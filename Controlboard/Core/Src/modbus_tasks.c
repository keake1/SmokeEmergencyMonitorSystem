/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : modbus_tasks.c
  * @brief          : Modbus 发送任务和接收任务实现
  *
  * 发送任务 (TaskModbusSend)：
  *   1. 阻塞等待 xModbusSendQueue
  *   2. 构造 Modbus 帧（含 CRC），HAL_UART_Transmit 发送
  *   3. 读指令（0x03）等 xModbusTxSemaphore → 写指令（0x06）跳过
  *   4. 3.5 字符帧间隔
  *
  * 接收任务 (TaskModbusReceive)：
  *   1. 阻塞等待 xModbusRawRxQueue（IDLE 中断填充）
  *   2. CRC 校验
  *   3. 解析帧：提取从机地址、型号、数据
  *   4. 存入寄存器映射表 (modbus_registers)
  *   5. 报警阈值判定（CO/7合1），状态变化时发 0x06 通知从机
  *   6. 释放 xModbusTxSemaphore
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "modbus_tasks.h"
#include "uart1_modbus.h"
#include "modbus_registers.h"
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "dwin.h"

/* 3.5 字符时间 @ 9600 8N1 ≈ 3.65 ms → 4 ticks */
#define MODBUS_3_5_CHAR_TICKS  pdMS_TO_TICKS(4)

/* 等待响应超时 */
#define MODBUS_RESP_TIMEOUT    pdMS_TO_TICKS(30)

/* 调试用：接收任务处理完一帧时的系统节拍（1 tick = 1ms） */
/* Live Watch 配合 poll_debug_tx_tick 观察轮询往返时间 */
volatile uint32_t poll_debug_rx_tick = 0;

/* ==================== 调试：串口1 最近接收帧 ====================
 * Live Watch 添加以下变量即可直观看到每帧内容：
 *   debug_rx_frame  - 最近一帧原始字节（未用部分清 0）
 *   debug_rx_len    - 该帧长度
 *   debug_rx_count  - 累计接收帧数（观察是否在持续收帧）
 */
volatile uint8_t  debug_rx_frame[MODBUS_RX_BUF_SIZE] = {0};
volatile uint16_t debug_rx_len   = 0;
volatile uint32_t debug_rx_count = 0;

/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* ==================== 发送任务 ==================== */

void TaskModbusSend(void *arg)
{
    (void)arg;
    ModbusRequest_t req;
    uint8_t tx_buf[8];
    uint16_t crc;

    for (;;)
    {
        /* ---- 1. 等待发送队列有请求 ---- */
        if (xQueueReceive(xModbusSendQueue, &req, portMAX_DELAY) != pdPASS)
            continue;

        /* ---- 2. 构造 Modbus RTU 帧 (8 字节) ---- */
        tx_buf[0] = req.slave_addr;
        tx_buf[1] = req.func_code;
        tx_buf[2] = (uint8_t)(req.reg_addr >> 8);
        tx_buf[3] = (uint8_t)(req.reg_addr & 0xFF);
        tx_buf[4] = (uint8_t)(req.reg_value >> 8);
        tx_buf[5] = (uint8_t)(req.reg_value & 0xFF);

        crc = Modbus_CRC16(tx_buf, 6);
        tx_buf[6] = (uint8_t)(crc & 0xFF);
        tx_buf[7] = (uint8_t)(crc >> 8);

        /* ---- 3. 中断发送（不阻塞任务） ---- */
        HAL_UART_Transmit_IT(&huart1, tx_buf, 8);
        xSemaphoreTake(xModbusTxCompleteSem, pdMS_TO_TICKS(50));

        /* ---- 4. 读指令等待响应，写指令跳过 ---- */
        if (req.func_code == MODBUS_FUNC_READ_HOLDING_REGISTERS)
            xSemaphoreTake(xModbusTxSemaphore, MODBUS_RESP_TIMEOUT);

        /* ---- 5. 帧间隔：3.5 字符时间 ---- */
        vTaskDelay(MODBUS_3_5_CHAR_TICKS);
    }
}

/* ==================== 接收任务 ==================== */

void TaskModbusReceive(void *arg)
{
    (void)arg;
    ModbusResponse_t raw;

    for (;;)
    {
        /* ---- 1. 等待原始帧（IDLE 中断放入队列） ---- */
        if (ModbusMaster_DequeueRawFrame(&raw, portMAX_DELAY) != 0)
            continue;

        /* ---- 调试：保存最近一帧到调试数组 ---- */
        for (uint16_t i = 0; i < MODBUS_RX_BUF_SIZE; i++)
            debug_rx_frame[i] = (i < raw.length) ? raw.frame[i] : 0;
        debug_rx_len = raw.length;
        debug_rx_count++;

        /* ---- 2. CRC 校验 ---- */
        uint8_t crc_ok = 0;
        if (raw.length >= 4)
        {
            uint16_t calc_crc = Modbus_CRC16(raw.frame, raw.length - 2);
            uint16_t recv_crc = (uint16_t)raw.frame[raw.length - 2] |
                                ((uint16_t)raw.frame[raw.length - 1] << 8);
            crc_ok = (calc_crc == recv_crc);
        }

        if (crc_ok)
        {
            /* ---- 3. 解析帧 ---- */
            uint8_t  slave = raw.frame[0];

            /* ---- 4. 记录响应（在线状态 + 离线检测周期追踪） ---- */
            ModbusReg_RecordResponse(slave);

            /* ---- 5. 零地址出现次数统计（总线异常监控） ---- */
            if (slave == 0)
            {
                ModbusReg_IncrementZeroAddr();
            }

            /* ---- 6. 有效传感器地址 (1-63) — 存类型、数据和报警判定 ---- */
            if (slave >= 1 && slave <= MODBUS_MAX_SLAVES)
            {
                uint8_t byte_count = raw.frame[2];
                uint8_t model;
                uint8_t data_offset;
                uint16_t data_bytes;

                /* ---- 兼容旧版 CO 传感器（无型号字节） ----
                 * byte_count == 2  → 旧格式：无型号，默认 CO，数据从 frame[3] 起
                 * byte_count >= 3  → 新格式：含型号字节，数据从 frame[4] 起 */
                if (byte_count == 2)
                {
                    model       = SENSOR_MODEL_CO;
                    data_offset = 3;
                    data_bytes  = 2;
                }
                else
                {
                    model       = raw.frame[3];
                    data_offset = 4;
                    data_bytes  = (byte_count >= 1) ? (byte_count - 1) : 0;
                }

                /* 记录传感器类型 */
                ModbusReg_SetType(slave, model);

                /* 解析数据 */
                for (uint8_t i = 0; i < MODBUS_DATA_REGS_PER_SENSOR; i++)
                {
                    /* 每 2 字节一个 uint16，不足则跳出 */
                    if ((i * 2 + 1) >= data_bytes) break;

                    uint16_t val = ((uint16_t)raw.frame[data_offset + i * 2] << 8) |
                                    raw.frame[data_offset + i * 2 + 1];
                    ModbusReg_SetData(slave, i, val);
                }

                /* ---- 7. 报警阈值判定 ---- */
                uint8_t cur_alarm = ModbusReg_GetAlarm(slave);
                uint8_t new_alarm = cur_alarm;

                switch (model)
                {
                    case SENSOR_MODEL_CO:
                    {
                        uint16_t co_val = ModbusReg_GetData(slave, 0);
                        if (cur_alarm == 0 && co_val > 2500)
                            new_alarm = 1;
                        else if (cur_alarm == 1 && co_val < 2000)
                            new_alarm = 0;
                        break;
                    }
                    case SENSOR_MODEL_7IN1:
                    {
                        /* [0]=eCO₂, [1]=eCH₂O, [2]=TVOC,
                         * [3]=PM2.5, [4]=PM10, [5]=温度, [6]=湿度
                         * 温湿度不参与报警判定 */
                        uint16_t co2   = ModbusReg_GetData(slave, 0);
                        uint16_t ch2o  = ModbusReg_GetData(slave, 1);
                        uint16_t tvoc  = ModbusReg_GetData(slave, 2);
                        uint16_t pm25  = ModbusReg_GetData(slave, 3);
                        uint16_t pm10  = ModbusReg_GetData(slave, 4);

                        if (cur_alarm == 0)
                        {
                            /* 任何一项超过报警阈值 → 报警 */
                            if (co2 > 1000 || ch2o > 80 || tvoc > 600 ||
                                pm25 > 35 || pm10 > 50)
                                new_alarm = 1;
                        }
                        else
                        {
                            /* 必须所有项都低于恢复阈值才解除报警 */
                            if (co2 < 800  && ch2o < 70 && tvoc < 500 &&
                                pm25 < 25  && pm10 < 40)
                                new_alarm = 0;
                        }
                        break;
                    }
                    case SENSOR_MODEL_WIND:
                    {
                        uint16_t press = ModbusReg_GetData(slave, 0);
                        if (cur_alarm == 0 && press > 400)
                            new_alarm = 1;
                        else if (cur_alarm == 1 && press < 300)
                            new_alarm = 0;
                        break;
                    }
                    case SENSOR_MODEL_CO2:
                    {
                        uint16_t co2 = ModbusReg_GetData(slave, 0);
                        if (cur_alarm == 0 && co2 > 1000)
                            new_alarm = 1;
                        else if (cur_alarm == 1 && co2 < 800)
                            new_alarm = 0;
                        break;
                    }
                    case SENSOR_MODEL_TH:
                    {
                        /* 温度不参与报警，只判湿度 */
                        uint16_t hum = ModbusReg_GetData(slave, 1);
                        if (cur_alarm == 0 && hum > 700)   /* > 70% */
                            new_alarm = 1;
                        else if (cur_alarm == 1 && hum < 600)  /* < 60% */
                            new_alarm = 0;
                        break;
                    }
                    case SENSOR_MODEL_CHAIN_ISOLATOR:
                    {
                        /* 连锁隔离器：P00=1 → 报警；P00=0 → 正常 */
                        uint16_t level = ModbusReg_GetData(slave, 0);
                        if (cur_alarm == 0 && level >= 1)
                            new_alarm = 1;
                        else if (cur_alarm == 1 && level == 0)
                            new_alarm = 0;
                        break;
                    }
                    default:
                        break;
                }

                /* 报警状态变化时：更新线圈位 + 发 0x06 指令通知从机 */
                if (new_alarm != cur_alarm)
                {
                    ModbusReg_SetAlarm(slave, new_alarm);

                    ModbusRequest_t alarm_req;
                    alarm_req.slave_addr = slave;
                    alarm_req.func_code  = MODBUS_FUNC_WRITE_SINGLE_REGISTER;
                    alarm_req.reg_addr   = 0x0004;
                    alarm_req.reg_value  = new_alarm ? 0x0001 : 0x0000;
                    ModbusMaster_EnqueueRequest(&alarm_req);
                }

                /* ---- 8. 更新迪文屏显示 ---- */
                DWIN_UpdateSensor(slave, model);
            }
        }
        /* CRC 失败：记录错误计数用于地址重复检测 */
        else
        {
            ModbusReg_RecordCrcError();
        }

        /* ---- 9. 仅 CRC 通过时释放信号量，通知发送任务收到响应 ---- */
        if (crc_ok)
        {
            xSemaphoreGive(xModbusTxSemaphore);
        }

        /* 调试：记录收到并处理完一帧的时间 */
        poll_debug_rx_tick = xTaskGetTickCount();
    }
}

/* USER CODE END 0 */
