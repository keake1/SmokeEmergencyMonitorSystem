/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : modbus_slave_tasks.c
  * @brief          : Modbus RTU 从站任务实现
  *
  * 接收任务 (TaskSlaveRecv)：
  *   1. 阻塞等待 xSlaveRawRxQueue
  *   2. CRC 校验
  *   3. 检查功能码：FC 0x02 (读离散输入) / FC 0x03 (读保持寄存器)
  *   4. 从内部寄存器读取数据 (ModbusReg_ReadCoil / ReadHolding)
  *   5. 构造 Modbus 响应帧（含 CRC）
  *   6. 入队到 xSlaveTxQueue
  *
  * 发送任务 (TaskSlaveSend)：
  *   1. 阻塞等待 xSlaveTxQueue
  *   2. HAL_UART_Transmit 发送
  *   3. 3.5 字符帧间隔
  *
  * 从机地址：Controlboard DIP 开关读取的地址 (ModbusReg_GetBoardAddr())
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "modbus_slave_tasks.h"
#include "uart1_modbus.h"           /* Modbus_CRC16 */
#include "uart2_modbus_slave.h"
#include "modbus_registers.h"
#include "usart.h"
#include <string.h>

/* USER CODE BEGIN 0 */

/* 3.5 字符时间 @ 9600 8N1 ≈ 3.65 ms → 4 ticks */
#define SLAVE_3_5_CHAR_TICKS  pdMS_TO_TICKS(4)

/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  UART2 从站接收任务
  * @param  arg  未使用
  *
  * 解析主站请求（FC 0x02 / 0x03），读取控板寄存器，构造响应帧入队。
  * 只响应发给本机 DIP 地址的请求。
  */
void TaskSlaveRecv(void *arg)
{
    (void)arg;
    ModbusSlaveFrame_t raw;
    uint8_t  resp_buf[256];
    uint16_t crc;

    /* 根据寄存器表限制最大读取范围 */
#define MAX_COIL_BITS       (COIL_GLOBAL_ALARM + 1)   /* 129: 线圈最大地址+1 */
#define MAX_HOLDING_REGS    125                       /* 3+125*2+2=255 = 256 */

    for (;;)
    {
        /* ---- 1. 等待原始帧（IDLE 中断放入队列） ---- */
        if (ModbusSlave_DequeueRawFrame(&raw, portMAX_DELAY) != 0)
            continue;

        /* ---- 2. CRC 校验 ---- */
        if (raw.length < 4) continue;

        uint16_t calc_crc = Modbus_CRC16(raw.frame, raw.length - 2);
        uint16_t recv_crc = (uint16_t)raw.frame[raw.length - 2] |
                            ((uint16_t)raw.frame[raw.length - 1] << 8);
        if (calc_crc != recv_crc)
            continue;

        /* ---- 3. 解析请求 ---- */
        uint8_t  slave_addr = raw.frame[0];
        uint8_t  func_code  = raw.frame[1];
        uint8_t  own_addr   = ModbusReg_GetBoardAddr();

        /* 地址过滤：只响应发给本机地址的请求 */
        if (slave_addr != own_addr)
            continue;

        uint16_t resp_len = 0;

        /* ---- 4. 按功能码处理 ---- */
        switch (func_code)
        {
            case 0x02:  /* 读离散输入 (Read Discrete Inputs) */
            {
                uint16_t start_addr = ((uint16_t)raw.frame[2] << 8) | raw.frame[3];
                uint16_t bit_count  = ((uint16_t)raw.frame[4] << 8) | raw.frame[5];

                /* 限制最大位数：不超过线圈寄存器范围 */
                if (bit_count > MAX_COIL_BITS) bit_count = MAX_COIL_BITS;

                uint16_t byte_count = (bit_count + 7) / 8;

                resp_buf[0] = (uint8_t)own_addr;
                resp_buf[1] = 0x02;
                resp_buf[2] = (uint8_t)byte_count;

                for (uint16_t b = 0; b < byte_count; b++)
                {
                    uint8_t byte_val = 0;
                    for (uint8_t bit = 0; bit < 8; bit++)
                    {
                        uint16_t coil = start_addr + b * 8 + bit;
                        if (coil < start_addr + bit_count)
                            byte_val |= (ModbusReg_ReadCoil(coil) << bit);
                    }
                    resp_buf[3 + b] = byte_val;
                }
                resp_len = 3 + byte_count;
                break;
            }

            case 0x03:  /* 读保持寄存器 (Read Holding Registers) */
            {
                uint16_t start_addr = ((uint16_t)raw.frame[2] << 8) | raw.frame[3];
                uint16_t reg_count  = ((uint16_t)raw.frame[4] << 8) | raw.frame[5];

                /* 限制最大寄存器数避免缓冲区溢出 */
                if (reg_count > MAX_HOLDING_REGS) reg_count = MAX_HOLDING_REGS;

                resp_buf[0] = (uint8_t)own_addr;
                resp_buf[1] = 0x03;
                resp_buf[2] = (uint8_t)(reg_count * 2);

                for (uint16_t i = 0; i < reg_count; i++)
                {
                    uint16_t val = ModbusReg_ReadHolding(start_addr + i);
                    resp_buf[3 + i * 2]     = (uint8_t)(val >> 8);
                    resp_buf[3 + i * 2 + 1] = (uint8_t)(val & 0xFF);
                }
                resp_len = 3 + reg_count * 2;
                break;
            }

            default:
                /* 不支持的功能码 → 丢弃 */
                continue;
        }

        /* ---- 5. 追加 CRC ---- */
        crc = Modbus_CRC16(resp_buf, resp_len);
        resp_buf[resp_len]     = (uint8_t)(crc & 0xFF);
        resp_buf[resp_len + 1] = (uint8_t)(crc >> 8);
        resp_len += 2;

        /* ---- 6. 入队发送 ---- */
        ModbusSlaveFrame_t resp;
        resp.length = (resp_len < MODBUS_SLAVE_RX_BUF_SIZE) ? resp_len : MODBUS_SLAVE_RX_BUF_SIZE;
        memcpy(resp.frame, resp_buf, resp.length);
        xQueueSend(xSlaveTxQueue, &resp, portMAX_DELAY);
    }
}

/**
  * @brief  UART2 从站发送任务
  * @param  arg  未使用
  *
  * 从 xSlaveTxQueue 取响应帧，通过 UART2 发送，然后等待 3.5 字符帧间隔。
  */
void TaskSlaveSend(void *arg)
{
    (void)arg;
    ModbusSlaveFrame_t resp;

    for (;;)
    {
        /* ---- 1. 等待响应帧 ---- */
        if (xQueueReceive(xSlaveTxQueue, &resp, portMAX_DELAY) != pdPASS)
            continue;

        /* ---- 2. 中断发送 ---- */
        HAL_UART_Transmit_IT(&huart2, resp.frame, resp.length);
        xSemaphoreTake(xSlaveTxCompleteSem, pdMS_TO_TICKS(200));

        /* ---- 3. 3.5 字符帧间隔 ---- */
        vTaskDelay(SLAVE_3_5_CHAR_TICKS);
    }
}

/* USER CODE END 0 */
