/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : modbus_master_tasks.c
  * @brief          : Modbus RTU 主站发送/接收任务实现
  *
  * 发送任务 (TaskModbusSend)：
  *   1. 阻塞等待 xMasterSendQueue
  *   2. 构造 Modbus 帧（含 CRC），HAL_UART_Transmit 发送
  *   3. 打开接收窗口 → 等信号量（30ms 超时）→ 关闭接收窗口
  *   4. 3.5 字符帧间隔
  *
  * 接收任务 (TaskModbusRecv)：
  *   1. 阻塞等待 xMasterRawRxQueue（IDLE 中断填充）
  *   2. CRC 校验
  *   3. 解析帧（验证地址和功能码）
  *   4. 释放 xMasterRxSem 通知发送任务
  *
  * 参考 Controlboard/Core/Src/modbus_tasks.c 架构。
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "modbus_master_tasks.h"
#include "uart1_modbus_master.h"
#include "hostboard_registers.h"
#include "usart.h"

/* USER CODE BEGIN 0 */
#include <string.h>
#include "detail_view_data.h"
#include "dwin.h"

/* 3.5 字符时间 @ 9600 8N1 ≈ 3.65 ms → 4 ticks */
#define MODBUS_3_5_CHAR_TICKS  pdMS_TO_TICKS(4)

/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

static TickType_t CalcModbusTimeout(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* ==================== 详情数据推送 ==================== */

static void DetailPushToDwin(uint8_t sensor_idx, uint8_t type, const uint16_t *regs)
{
    uint8_t buf[32];
    memset(buf, 0, sizeof(buf));
    uint16_t addr = DWIN_DETAIL_SENSOR_DATA_ADDR + (uint16_t)(sensor_idx - 1) * 16;

    switch (type)
    {
        case 0x01:  /* CO 传感器 */
        {
            float val = (float)(regs[0]) / 100.0f;
            uint32_t *p = (uint32_t *)&val;
            buf[0] = (uint8_t)(*p >> 24);
            buf[1] = (uint8_t)(*p >> 16);
            buf[2] = (uint8_t)(*p >> 8);
            buf[3] = (uint8_t)(*p);
            break;
        }
        case 0x02:  /* 风压传感器 — 与 Controlboard 标准布局一致 (word 2-3) */
        {
            float val = (float)(regs[0]);
            uint32_t *p = (uint32_t *)&val;
            buf[4] = (uint8_t)(*p >> 24);
            buf[5] = (uint8_t)(*p >> 16);
            buf[6] = (uint8_t)(*p >> 8);
            buf[7] = (uint8_t)(*p);
            break;
        }
        case 0x03:  /* 余压传感器 — 与 Controlboard 标准布局一致 (word 4) */
        {
            buf[8] = (uint8_t)(regs[0] >> 8);
            buf[9] = (uint8_t)(regs[0] & 0xFF);
            break;
        }
        case 0x04:  /* 7 合 1 传感器 */
        {
            buf[10] = (uint8_t)(regs[0] >> 8);  buf[11] = (uint8_t)(regs[0] & 0xFF);
            buf[12] = (uint8_t)(regs[1] >> 8);  buf[13] = (uint8_t)(regs[1] & 0xFF);
            buf[14] = (uint8_t)(regs[2] >> 8);  buf[15] = (uint8_t)(regs[2] & 0xFF);
            { float v = (float)(regs[3]); uint32_t *p = (uint32_t *)&v;
              buf[16] = (uint8_t)(*p >> 24); buf[17] = (uint8_t)(*p >> 16);
              buf[18] = (uint8_t)(*p >> 8);  buf[19] = (uint8_t)(*p); }
            { float v = (float)(regs[4]); uint32_t *p = (uint32_t *)&v;
              buf[20] = (uint8_t)(*p >> 24); buf[21] = (uint8_t)(*p >> 16);
              buf[22] = (uint8_t)(*p >> 8);  buf[23] = (uint8_t)(*p); }
            { float v = (float)(int16_t)(regs[5]) / 10.0f; uint32_t *p = (uint32_t *)&v;
              buf[24] = (uint8_t)(*p >> 24); buf[25] = (uint8_t)(*p >> 16);
              buf[26] = (uint8_t)(*p >> 8);  buf[27] = (uint8_t)(*p); }
            { float v = (float)(regs[6]) / 10.0f; uint32_t *p = (uint32_t *)&v;
              buf[28] = (uint8_t)(*p >> 24); buf[29] = (uint8_t)(*p >> 16);
              buf[30] = (uint8_t)(*p >> 8);  buf[31] = (uint8_t)(*p); }
            break;
        }
        case 0x05:  /* 温湿度传感器 — 温度/湿度到偏移 12-15（与 7 合 1 位置一致） */
        {
            { float v = (float)(int16_t)(regs[0]) / 10.0f; uint32_t *p = (uint32_t *)&v;
              buf[24] = (uint8_t)(*p >> 24); buf[25] = (uint8_t)(*p >> 16);
              buf[26] = (uint8_t)(*p >> 8);  buf[27] = (uint8_t)(*p); }
            { float v = (float)(regs[1]) / 10.0f; uint32_t *p = (uint32_t *)&v;
              buf[28] = (uint8_t)(*p >> 24); buf[29] = (uint8_t)(*p >> 16);
              buf[30] = (uint8_t)(*p >> 8);  buf[31] = (uint8_t)(*p); }
            break;
        }
        case 0x06:  /* CO2 传感器 — CO2 浓度到偏移 10-11（复用 7 合 1 的 eCO₂ 位置） */
        {
            buf[10] = (uint8_t)(regs[0] >> 8);
            buf[11] = (uint8_t)(regs[0] & 0xFF);
            break;
        }
        case 0x07:  /* 连锁隔离器 — P00 电平值到偏移 8-9（复用余压位置） */
        {
            buf[8] = (uint8_t)(regs[0] >> 8);
            buf[9] = (uint8_t)(regs[0] & 0xFF);
            break;
        }
        default:
            return;
    }

    DWIN_WriteVar(addr, buf, 32);
}

/* ==================== 动态超时计算 ==================== */

/**
  * @brief  根据当前在飞请求计算响应超时
  * @note   FC 0x03：每个寄存器回复 2 字节，估计帧长 5 + reg_count*2 字节
  *         FC 0x02：每 8 位回复 1 字节，估计帧长 5 + ceil(reg_count/8) 字节
  *         @9600 baud ≈ 1.04 ms/字节，加处理裕量按 2 ms/字节计算
  * @retval 超时 Tick 数
  */
static TickType_t CalcModbusTimeout(void)
{
    uint16_t resp_bytes;

    if (g_host_last_req.func_code == 0x02U)
    {
        /* FC 0x02 响应：reg_count 是位数 */
        resp_bytes = 5U + ((uint16_t)(g_host_last_req.reg_count + 7U) >> 3);
    }
    else
    {
        /* FC 0x03 响应：reg_count 是寄存器数，每寄存 2 字节 */
        resp_bytes = 5U + (uint16_t)(g_host_last_req.reg_count * 2U);
    }

    /* 基值 15ms + 每字节 2ms 裕量 */
    return pdMS_TO_TICKS(15U + resp_bytes * 2U);
}

/* ==================== 发送任务 ==================== */

void TaskModbusSend(void *arg)
{
    (void)arg;
    ModbusMasterRequest_t req;
    uint8_t tx_buf[8];
    uint16_t crc;

    for (;;)
    {
        /* ---- 1. 等待发送队列有请求 ---- */
        if (xQueueReceive(xMasterSendQueue, &req, portMAX_DELAY) != pdPASS)
            continue;

        /* 清除可能残留的信号量（上次超时后接收任务可能仍会 Give） */
        xSemaphoreTake(xMasterRxSem, 0);

        /* 记录当前请求信息（供接收任务解析响应数据的位置） */
        g_host_last_req.slave_addr = req.slave_addr;
        g_host_last_req.func_code  = req.func_code;
        g_host_last_req.reg_addr   = req.reg_addr;
        g_host_last_req.reg_count  = req.reg_value;

        /* ---- 3. 构造 Modbus RTU 帧 (8 字节) ---- */
        tx_buf[0] = req.slave_addr;
        tx_buf[1] = req.func_code;
        tx_buf[2] = (uint8_t)(req.reg_addr >> 8);
        tx_buf[3] = (uint8_t)(req.reg_addr & 0xFF);
        tx_buf[4] = (uint8_t)(req.reg_value >> 8);
        tx_buf[5] = (uint8_t)(req.reg_value & 0xFF);

        crc = ModbusMaster_CRC16(tx_buf, 6);
        tx_buf[6] = (uint8_t)(crc & 0xFF);
        tx_buf[7] = (uint8_t)(crc >> 8);

        /* ---- 4. 中断发送（发送完成后再开接收窗，避免自回声触发假 IDLE） ---- */
        HAL_UART_Transmit_IT(&huart1, tx_buf, 8);
        xSemaphoreTake(xMasterTxCompleteSem, pdMS_TO_TICKS(50));

        /* ---- 5. 发送完成 → 开启接收窗口，等待响应 ---- */
        ModbusMaster_StartRx();
        xSemaphoreTake(xMasterRxSem, CalcModbusTimeout());

        /* ---- 6. 关闭接收中断窗口 ---- */
        ModbusMaster_DisableRx();

        /* ---- 7. 帧间隔：3.5 字符时间 ---- */
        vTaskDelay(MODBUS_3_5_CHAR_TICKS);
    }
}

/* ==================== 接收任务 ==================== */

void TaskModbusRecv(void *arg)
{
    (void)arg;
    ModbusMasterFrame_t raw;

    for (;;)
    {
        /* ---- 1. 等待原始帧（IDLE 中断放入队列） ---- */
        if (ModbusMaster_DequeueRawFrame(&raw, portMAX_DELAY) != 0)
            continue;

        /* ---- 2. CRC 校验 ---- */
        uint8_t crc_ok = 0;
        if (raw.length >= 4)
        {
            uint16_t calc_crc = ModbusMaster_CRC16(raw.frame, raw.length - 2);
            uint16_t recv_crc = (uint16_t)raw.frame[raw.length - 2] |
                                ((uint16_t)raw.frame[raw.length - 1] << 8);
            crc_ok = (calc_crc == recv_crc);
        }

        if (crc_ok)
        {
            uint8_t  slave_addr = raw.frame[0];
            uint8_t  func_code  = raw.frame[1];
            uint8_t  byte_cnt   = raw.frame[2];

            /* 地址 0：任何 CRC 有效的响应均视为零地址存在 */
            if (slave_addr == 0)
            {
                HostReg_RecordZeroAddrResponse();
            }
            /* 地址 1-128：验证是 FC 0x02 响应且数据长度合理后存入寄存器 */
            else if (slave_addr >= 1 && slave_addr <= MAX_CTRLBD_ADDR
                     && func_code == 0x02
                     && byte_cnt > 0
                     && (3 + byte_cnt + 2) <= raw.length)
            {
                if (byte_cnt >= COIL_BYTE_COUNT)
                {
                    /* 全量 17 字节响应 → 完整覆盖 */
                    HostReg_StoreCoilData(slave_addr, &raw.frame[3], byte_cnt);

                    /* 如果是选中控制器的 FC 0x02 响应 → 标记线圈就绪 */
                    if (slave_addr == g_hostDwinStatus.selectAddr)
                    {
                        DetailView_SetCoilReady(slave_addr);
                    }
                }
                else if (byte_cnt > 0 && slave_addr == g_host_last_req.slave_addr)
                {
                    /* 精简响应（如 67 bits/9 字节轮询）→ 按位写入指定偏移 */
                    HostReg_StorePartialBits(slave_addr,
                                             g_host_last_req.reg_addr,
                                             g_host_last_req.reg_count,
                                             &raw.frame[3]);
                }
                /* else: 其他长度 → 忽略（不应发生） */
            }

            /* ── FC 0x03 响应：传感器型号或数据（详情采集用） ── */
            else if (func_code == 0x03
                     && slave_addr == g_host_last_req.slave_addr
                     && byte_cnt > 0
                     && (3 + byte_cnt + 2) <= raw.length)
            {
                uint16_t reg_addr = g_host_last_req.reg_addr;
                uint8_t  reg_count = byte_cnt / 2;
                uint16_t regs[7];
                uint8_t  i;

                if (reg_count > 7) reg_count = 7;

                for (i = 0; i < reg_count; i++)
                {
                    regs[i] = ((uint16_t)raw.frame[3 + i * 2] << 8)
                            | raw.frame[3 + i * 2 + 1];
                }

                if (reg_addr >= 1 && reg_addr <= 63)
                {
                    /* Controlboard 将 uint8 型号扩展为 uint16，型号在低字节 */
                    DetailView_SetType((uint8_t)reg_addr, (uint8_t)(regs[0] & 0xFF));
                }
                else if (reg_addr >= 64)
                {
                    uint8_t sensor_idx = (uint8_t)((reg_addr - 64) / 7 + 1);
                    uint8_t type = DetailView_GetType(sensor_idx);
                    if (type != 0)
                    {
                        DetailPushToDwin(sensor_idx, type, regs);
                    }
                }
            }
            /* else: 功能码异常或格式错误 → 忽略不计 */

            /* ---- 3. 仅 CRC 通过时释放信号量：通知发送任务可以发下一帧 ---- */
            xSemaphoreGive(xMasterRxSem);
        }
        else
        {
            /* CRC 失败且收到过数据 → 计入重复地址检测 */
            if (raw.length > 0)
                HostReg_RecordError();
        }
    }
}

/* USER CODE END 0 */
