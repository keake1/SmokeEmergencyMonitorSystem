/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : dwin.c
  * @brief          : 迪文屏 DGUS 通信驱动实现
  *
  * 将 Modbus 接收任务解析出的传感器数据写入迪文屏变量区。
  * 通过 UART3（115200 8N1）使用 0x82 指令传输，中断发送。
  *
  * 统一队列框架：
  *   所有发送（传感器数据、状态帧、控板地址）统一经过队列。
  *   调用方入队后调用 DWIN_ProcessQueue()，若 UART3 空闲则出队构造帧并发送。
  *   TX 完成后回调仅清除忙标志；下一帧由任务上下文驱动（DWIN_UpdateSensor
  *   或 DWIN_FlushQueue 每 50ms）。
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "dwin.h"
#include "usart.h"
#include "uart1_modbus.h"
#include "uart2_modbus_slave.h"
#include "modbus_registers.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

/* USER CODE BEGIN 0 */

/* ==================== 队列条目类型 ==================== */

#define DWIN_ITEM_SENSOR_DATA   0
#define DWIN_ITEM_STATUS        1
#define DWIN_ITEM_BOARD_ADDR    2

/* ==================== 更新队列 ==================== */

#define DWIN_QUEUE_SIZE     16

typedef struct {
    uint8_t type;             /* DWIN_ITEM_* */
    uint8_t slave;
    uint8_t model;
} DWIN_QueueItem_t;

static DWIN_QueueItem_t   dwin_queue[DWIN_QUEUE_SIZE];
static uint8_t            dwin_q_wr = 0;
static uint8_t            dwin_q_rd = 0;
static volatile uint8_t   dwin_tx_busy = 0;         /* UART3 中断发送中 */

static uint8_t DWIN_QueueEmpty(void)
{
    return (dwin_q_wr == dwin_q_rd);
}

static uint8_t DWIN_QueueFull(void)
{
    return ((dwin_q_wr + 1) % DWIN_QUEUE_SIZE == dwin_q_rd);
}

static void DWIN_Enqueue(uint8_t type, uint8_t slave, uint8_t model)
{
    taskENTER_CRITICAL();
    if (DWIN_QueueFull())
    {
        taskEXIT_CRITICAL();
        return;          /* 队列满 → 丢弃 */
    }
    dwin_queue[dwin_q_wr].type  = type;
    dwin_queue[dwin_q_wr].slave = slave;
    dwin_queue[dwin_q_wr].model = model;
    dwin_q_wr = (dwin_q_wr + 1) % DWIN_QUEUE_SIZE;
    taskEXIT_CRITICAL();
}

static uint8_t DWIN_Dequeue(DWIN_QueueItem_t *item)
{
    uint8_t ret = 0;
    taskENTER_CRITICAL();
    if (!DWIN_QueueEmpty())
    {
        *item = dwin_queue[dwin_q_rd];
        dwin_q_rd = (dwin_q_rd + 1) % DWIN_QUEUE_SIZE;
        ret = 1;
    }
    taskEXIT_CRITICAL();
    return ret;
}

/* ==================== 辅助函数 ==================== */

/**
  * @brief  将 float 转换为 4 字节大端序（IEEE 754）
  * @note   STM32F0 为小端 CPU，DWIN 屏要求大端字节序
  */
static void FloatToBigEndian(float val, uint8_t *out)
{
    uint32_t tmp;
    memcpy(&tmp, &val, sizeof(tmp));
    out[0] = (uint8_t)(tmp >> 24);
    out[1] = (uint8_t)(tmp >> 16);
    out[2] = (uint8_t)(tmp >> 8);
    out[3] = (uint8_t)(tmp);
}

/* ==================== 发送帧缓冲区 ====================
 * 所有帧类型共用同一个 static 缓冲区。
 * 同一时刻只有一帧在发送（dwin_tx_busy 互斥），因此安全。
 *
 * 最大帧 = 状态帧：6 字节帧头 + 69 地址 × 2 字节 = 144 字节
 */
static uint8_t  tx_frame_buf[6 + 69 * 2];
static uint16_t tx_frame_len;

/* ==================== 全局报警标志 ====================
 * 由 DWIN_BuildStatusFrame 每次扫描更新，DWIN_GetGlobalAlarm 查询。
 */
static uint8_t dwin_global_alarm = 0;

/* ==================== 帧构造函数 ====================
 * 各函数填充 tx_frame_buf 并设置 tx_frame_len，
 * 由 DWIN_ProcessQueue() 统一调用 HAL_UART_Transmit_IT 发送。
 */

/**
  * @brief  构造传感器数据帧（0x82 写 16 地址 = 32 字节数据）
  */
static void DWIN_BuildSensorFrame(uint8_t slave, uint8_t model)
{
    uint16_t dwin_addr = DWIN_VAR_BASE + (slave - 1) * DWIN_ADDRS_PER_SENSOR;
    uint16_t idx = 0;
    uint16_t raw;
    float    fval;

    /* ---- 帧头 ---- */
    tx_frame_buf[idx++] = DWIN_HEADER_1;
    tx_frame_buf[idx++] = DWIN_HEADER_2;
    tx_frame_buf[idx++] = 3 + DWIN_ADDRS_PER_SENSOR * 2;   /* 3 + 32 = 35 */
    tx_frame_buf[idx++] = DWIN_CMD_WRITE_VAR;
    tx_frame_buf[idx++] = (uint8_t)(dwin_addr >> 8);
    tx_frame_buf[idx++] = (uint8_t)(dwin_addr & 0xFF);

    /* ---- 数据区（32 字节，零填充） ---- */
    uint8_t *data = &tx_frame_buf[idx];
    memset(data, 0, DWIN_ADDRS_PER_SENSOR * 2);

    switch (model)
    {
        case SENSOR_MODEL_CO:
            raw  = ModbusReg_GetData(slave, 0);
            fval = (float)raw / 100.0f;
            FloatToBigEndian(fval, &data[DWIN_OFFSET_CO * 2]);
            break;

        case SENSOR_MODEL_WIND:
            raw  = ModbusReg_GetData(slave, 0);
            fval = (float)raw;
            FloatToBigEndian(fval, &data[DWIN_OFFSET_WIND * 2]);
            break;

        case SENSOR_MODEL_PRESSURE:
            raw = ModbusReg_GetData(slave, 0);
            data[DWIN_OFFSET_PRESSURE * 2 + 0] = (uint8_t)(raw >> 8);
            data[DWIN_OFFSET_PRESSURE * 2 + 1] = (uint8_t)(raw);
            break;

        case SENSOR_MODEL_7IN1:
            /* uint16 字段：eCO₂ [0], eCH₂O [1], TVOC [2] */
            raw = ModbusReg_GetData(slave, 0);
            data[DWIN_OFFSET_CO2 * 2 + 0] = (uint8_t)(raw >> 8);
            data[DWIN_OFFSET_CO2 * 2 + 1] = (uint8_t)(raw);

            raw = ModbusReg_GetData(slave, 1);
            data[DWIN_OFFSET_CH2O * 2 + 0] = (uint8_t)(raw >> 8);
            data[DWIN_OFFSET_CH2O * 2 + 1] = (uint8_t)(raw);

            raw = ModbusReg_GetData(slave, 2);
            data[DWIN_OFFSET_TVOC * 2 + 0] = (uint8_t)(raw >> 8);
            data[DWIN_OFFSET_TVOC * 2 + 1] = (uint8_t)(raw);

            /* float 字段：PM2.5 [3], PM10 [4] */
            raw  = ModbusReg_GetData(slave, 3);
            fval = (float)raw;
            FloatToBigEndian(fval, &data[DWIN_OFFSET_PM25 * 2]);

            raw  = ModbusReg_GetData(slave, 4);
            fval = (float)raw;
            FloatToBigEndian(fval, &data[DWIN_OFFSET_PM10 * 2]);

            /* 温度 [5] -- raw÷10，有符号 */
            {
                int16_t s_temp = (int16_t)ModbusReg_GetData(slave, 5);
                fval = (float)s_temp / 10.0f;
            }
            FloatToBigEndian(fval, &data[DWIN_OFFSET_TEMP * 2]);

            /* 湿度 [6] -- raw÷10，无符号 */
            raw = ModbusReg_GetData(slave, 6);
            fval = (float)raw / 10.0f;
            FloatToBigEndian(fval, &data[DWIN_OFFSET_HUMID * 2]);
            break;

        case SENSOR_MODEL_TH:
            /* 温度 reg_data[0]（×10，有符号）→ float 到偏移 12-13 */
            {
                int16_t s_temp = (int16_t)ModbusReg_GetData(slave, 0);
                fval = (float)s_temp / 10.0f;
            }
            FloatToBigEndian(fval, &data[DWIN_OFFSET_TEMP * 2]);

            /* 湿度 reg_data[1]（×10）→ float 到偏移 14-15 */
            raw = ModbusReg_GetData(slave, 1);
            fval = (float)raw / 10.0f;
            FloatToBigEndian(fval, &data[DWIN_OFFSET_HUMID * 2]);
            break;

        case SENSOR_MODEL_CO2:
            /* CO2 浓度 reg_data[0] → uint16 到偏移 10-11（复用 7 合 1 的 eCO₂ 位置） */
            raw = ModbusReg_GetData(slave, 0);
            data[DWIN_OFFSET_CO2 * 2 + 0] = (uint8_t)(raw >> 8);
            data[DWIN_OFFSET_CO2 * 2 + 1] = (uint8_t)(raw & 0xFF);
            break;

        case SENSOR_MODEL_CHAIN_ISOLATOR:
            /* P00 电平值 reg_data[0] → uint16 到偏移 8-9（余压位置，型号互斥） */
            raw = ModbusReg_GetData(slave, 0);
            data[DWIN_OFFSET_PRESSURE * 2 + 0] = (uint8_t)(raw >> 8);
            data[DWIN_OFFSET_PRESSURE * 2 + 1] = (uint8_t)(raw);
            break;

        default:
            return;
    }

    tx_frame_len = idx + DWIN_ADDRS_PER_SENSOR * 2;
}

/**
  * @brief  构造状态帧（图标 + 全局报警 + 零地址 + 地址重复 + 烟雾报警）
  *
  * 一次性写入 0x1400-0x1444（69 地址 = 138 字节数据）：
  *   0x1400-0x143F: 64 路传感器图标（uint16/路）
  *   0x1440:        开窗图标（全局报警 OR 烟雾报警，与 WS 引脚一致）
  *   0x1441:        全局报警图标（仅传感器报警）
  *   0x1442:        零地址存在
  *   0x1443:        地址重复
  *   0x1444:        烟雾报警器图标
  */
static void DWIN_BuildStatusFrame(void)
{
    uint16_t idx = 0;

    /* ---- 帧头 ---- */
    tx_frame_buf[idx++] = DWIN_HEADER_1;
    tx_frame_buf[idx++] = DWIN_HEADER_2;
    tx_frame_buf[idx++] = 3 + 68 * 2;                   /* 3 + 136 = 139 */
    tx_frame_buf[idx++] = DWIN_CMD_WRITE_VAR;
    tx_frame_buf[idx++] = (uint8_t)(DWIN_ICON_BASE >> 8);
    tx_frame_buf[idx++] = (uint8_t)(DWIN_ICON_BASE & 0xFF);

    /* ---- 数据区（68 × uint16 = 136 字节） ---- */
    uint8_t *data = &tx_frame_buf[idx];
    memset(data, 0, 69 * 2);

    dwin_global_alarm = 0;

    for (uint8_t i = 1; i <= MODBUS_MAX_SLAVES; i++)
    {
        uint8_t online = ModbusReg_GetOnline(i);
        uint8_t alarm  = ModbusReg_GetAlarm(i);
        uint16_t icon_val;

        if (!online)
            icon_val = DWIN_ICON_OFFLINE;
        else if (alarm)
        {
            icon_val = DWIN_ICON_ALARM;
            dwin_global_alarm = 1;
        }
        else
            icon_val = DWIN_ICON_NORMAL;

        data[(i - 1) * 2 + 0] = (uint8_t)(icon_val >> 8);
        data[(i - 1) * 2 + 1] = (uint8_t)(icon_val);
    }

    /* 开窗标志 = 全局报警标志（含 PB1 报警，同步到 0x1440 开窗图标，与 WS 引脚一致） */
    uint8_t window_open = dwin_global_alarm || ModbusReg_GetSmokeAlarm();

    /* 线圈位 128 = 全局报警（传感器 OR PB1） */
    ModbusReg_SetGlobalAlarm(window_open);

    /* 0x1440：开窗图标（与 WS 引脚一致） */
    data[64 * 2 + 0] = 0;
    data[64 * 2 + 1] = window_open;

    /* 0x1441：全局报警图标（仅传感器报警） */
    data[65 * 2 + 0] = 0;
    data[65 * 2 + 1] = dwin_global_alarm;

    /* 零地址存在 → 0x1442 */
    data[66 * 2 + 0] = 0;
    data[66 * 2 + 1] = ModbusReg_GetZeroAddrPresent();

    /* 地址重复 → 0x1443 */
    data[67 * 2 + 0] = 0;
    data[67 * 2 + 1] = ModbusReg_GetAddrConflict();

    tx_frame_len = idx + 68 * 2;
}

/**
  * @brief  构造控板 DIP 地址帧（0x82 写 1 地址 = 2 字节数据到 0x1500）
  */
static void DWIN_BuildBoardAddrFrame(void)
{
    uint8_t addr_val = ModbusReg_GetBoardAddr();
    uint16_t idx = 0;

    tx_frame_buf[idx++] = DWIN_HEADER_1;
    tx_frame_buf[idx++] = DWIN_HEADER_2;
    tx_frame_buf[idx++] = 5;                              /* LEN = 3 + 2 */
    tx_frame_buf[idx++] = DWIN_CMD_WRITE_VAR;
    tx_frame_buf[idx++] = (uint8_t)(DWIN_BOARD_ADDR >> 8);
    tx_frame_buf[idx++] = (uint8_t)(DWIN_BOARD_ADDR & 0xFF);
    tx_frame_buf[idx++] = 0;                              /* uint16 高字节 */
    tx_frame_buf[idx++] = addr_val;                       /* uint16 低字节 */

    tx_frame_len = idx;
}

/* ==================== 统一队列处理 ==================== */

/**
  * @brief  出队一项，构造帧并通过 UART3 中断发送
  * @note   若 UART3 忙则直接返回，不操作。
  *         HAL_UART_Transmit_IT 的唯一调用点。
  */
static void DWIN_ProcessQueue(void)
{
    if (dwin_tx_busy)
        return;

    DWIN_QueueItem_t item;
    if (!DWIN_Dequeue(&item))
        return;

    /* 根据类型构造帧 → 填充 tx_frame_buf 和 tx_frame_len */
    switch (item.type)
    {
        case DWIN_ITEM_SENSOR_DATA:
            DWIN_BuildSensorFrame(item.slave, item.model);
            break;

        case DWIN_ITEM_STATUS:
            DWIN_BuildStatusFrame();
            break;

        case DWIN_ITEM_BOARD_ADDR:
            DWIN_BuildBoardAddrFrame();
            break;

        default:
            return;
    }

    /* 启动中断发送 */
    dwin_tx_busy = 1;
    if (HAL_UART_Transmit_IT(&huart3, tx_frame_buf, tx_frame_len) != HAL_OK)
        dwin_tx_busy = 0;
}

/* ==================== 对外接口 ==================== */

/**
  * @brief  请求更新某传感器在迪文屏上的显示数据
  * @param  slave  传感器地址 (1-63)
  * @param  model  传感器型号字节
  * @note   入队后立即尝试发送；若 UART3 忙则排队等待后续驱动。
  */
void DWIN_UpdateSensor(uint8_t slave, uint8_t model)
{
    DWIN_Enqueue(DWIN_ITEM_SENSOR_DATA, slave, model);
    DWIN_ProcessQueue();
}

/**
  * @brief  排空更新队列：若 UART3 空闲则出队发送一帧
  * @note   轮询任务每 50ms 调用一次，作为队列的后备驱动。
  */
void DWIN_FlushQueue(void)
{
    DWIN_ProcessQueue();
}

/* ==================== 全局报警标志查询 ==================== */

uint8_t DWIN_GetGlobalAlarm(void)
{
    return dwin_global_alarm;
}

/* ==================== 图标更新任务 ==================== */

/**
  * @brief  迪文屏状态更新任务
  * @param  arg  未使用
  * @note   每 500ms 执行一轮：入队 STATUS 和 BOARD_ADDR 两帧，
  *         由 DWIN_ProcessQueue 驱动发送。
  */
void TaskDwinIcons(void *arg)
{
    (void)arg;
    TickType_t last_wake = xTaskGetTickCount();
    const TickType_t interval = pdMS_TO_TICKS(500);

    for (;;)
    {
        vTaskDelayUntil(&last_wake, interval);

        /* 帧 1：图标 + 全局报警 + 零地址 + 地址重复 */
        DWIN_Enqueue(DWIN_ITEM_STATUS, 0, 0);
        DWIN_ProcessQueue();

        /* 帧 2：控板 DIP 地址 */
        DWIN_Enqueue(DWIN_ITEM_BOARD_ADDR, 0, 0);
        DWIN_ProcessQueue();

        /* ---- PB1 报警输入（高电平=报警） ---- */
        ModbusReg_SetSmokeAlarm(
            HAL_GPIO_ReadPin(Isolator_GPIO_Port, Isolator_Pin) != GPIO_PIN_RESET);

        /* 同步 PB1 报警到全局报警标志（红灯、线圈位 128） */
        if (ModbusReg_GetSmokeAlarm()) {
            ModbusReg_SetGlobalAlarm(1);
        }

        /* ---- 报警输出（高电平有效） ---- */
        /* WS (PB0): 全局报警 或 PB1 报警 → 高电平 */
        HAL_GPIO_WritePin(WS_GPIO_Port, WS_Pin,
            (DWIN_GetGlobalAlarm() || ModbusReg_GetSmokeAlarm()) ? GPIO_PIN_SET : GPIO_PIN_RESET);

        /* ---- LED 指示（低电平点亮） ---- */
        /* RED_LED (PB9): 全局报警 → 点亮 */
        if (DWIN_GetGlobalAlarm())
            HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);
        else
            HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);

        /* ORANGE_LED (PB8): 零地址存在 或 地址重复 → 点亮 */
        if (ModbusReg_GetZeroAddrPresent() || ModbusReg_GetAddrConflict())
            HAL_GPIO_WritePin(ORANGE_LED_GPIO_Port, ORANGE_LED_Pin, GPIO_PIN_RESET);
        else
            HAL_GPIO_WritePin(ORANGE_LED_GPIO_Port, ORANGE_LED_Pin, GPIO_PIN_SET);
    }
}

/**
  * @brief  UART 发送完成回调（覆盖 HAL 弱符号）
  * @note   仅清除忙标志。不在此处调用 HAL_UART_Transmit_IT，
  *         因为 ISR 中 HAL gState 可能仍为 BUSY，不可靠。
  *         下一帧由任务上下文中的 DWIN_ProcessQueue 驱动。
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart3)
    {
        dwin_tx_busy = 0;
    }
    else if (huart == &huart1)
    {
        BaseType_t xWoken = pdFALSE;
        xSemaphoreGiveFromISR(xModbusTxCompleteSem, &xWoken);
        portYIELD_FROM_ISR(xWoken);
    }
    else if (huart == &huart2)
    {
        BaseType_t xWoken = pdFALSE;
        xSemaphoreGiveFromISR(xSlaveTxCompleteSem, &xWoken);
        portYIELD_FROM_ISR(xWoken);
    }
}

/* USER CODE END 0 */
