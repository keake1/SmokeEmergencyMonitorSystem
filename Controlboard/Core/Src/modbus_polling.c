/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : modbus_polling.c
  * @brief          : Modbus 轮询任务实现
  *
  * 轮询策略：
  *   [顺序轮询] 依次扫描地址 0 → 1 → 2 → ... → 63 → 0 → ...
  *              使用 0x03 读保持寄存器，寄存器 0x0000，数量 1。
  *   [在线穿插] 每进行 POLL_SEQUENTIAL_BETWEEN_ONLINE 次顺序地址轮询，
  *              插入一次已标记在线的传感器轮询，提高其数据更新频率。
  *
  * 任务流：
  *   TaskModbusPoll
  *     └── ModbusMaster_EnqueueRequest()  →  [xModbusSendQueue]
  *           └── 发送任务发帧 → 传感器响应 → 接收任务解析入库
  *
  * 注意：
  *   - 本任务仅将请求塞入发送队列，不等待响应。
  *   - 发送队列有 8 个槽位，队列满时 EnqueueRequest 自动阻塞，
  *     因此轮询节奏由总线实际吞吐量自然调节。
  *   - 地址 0 无传感器应答，接收任务仅做零地址计数。
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "modbus_polling.h"
#include "uart1_modbus.h"
#include "modbus_registers.h"
#include "gpio.h"

/* USER CODE BEGIN 0 */
#include "dwin.h"

/* 每 N 次顺序地址轮询穿插 1 次在线传感器轮询 */
#define POLL_SEQUENTIAL_BETWEEN_ONLINE  8

/* 轮询间隔：50ms */
#define POLL_INTERVAL_TICKS             pdMS_TO_TICKS(50)

/* 调试用：每次轮询请求入队时的系统节拍（1 tick = 1ms） */
/* Live Watch 配合 poll_debug_rx_tick 观察轮询往返时间 */
volatile uint32_t poll_debug_tx_tick = 0;
/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
  * @brief  查找下一个在线传感器地址
  * @param  last_idx  [in/out] 上一次轮询到的地址，函数内部自增并回绕
  * @retval 0 ~ MODBUS_MAX_SLAVES  在线传感器地址（0 = 无在线传感器）
  */
static uint8_t FindNextOnline(uint8_t *last_idx)
{
    for (uint8_t i = 1; i <= MODBUS_MAX_SLAVES; i++)
    {
        (*last_idx)++;
        if (*last_idx > MODBUS_MAX_SLAVES)
            *last_idx = 1;

        if (ModbusReg_GetOnline(*last_idx))
            return *last_idx;
    }
    return 0;   /* 无在线传感器 */
}

/* USER CODE END 0 */

/* ==================== 轮询任务 ==================== */

void TaskModbusPoll(void *arg)
{
    (void)arg;

    /* 启动延迟：等待系统初始化完成后开始轮询 */
    vTaskDelay(pdMS_TO_TICKS(100));

    uint8_t scan_addr   = 0;    /* 顺序扫描地址 0~63 */
    uint8_t poll_count  = 0;    /* 自上次在线轮询后的顺序轮询计数 */
    uint8_t online_idx  = 0;    /* 在线传感器轮询游标 */

    ModbusRequest_t req;

    /* 请求模板：功能码 0x03，寄存器 0x0000，读 1 个寄存器 */
    req.func_code = MODBUS_FUNC_READ_HOLDING_REGISTERS;
    req.reg_addr  = 0x0000;
    req.reg_value = 0x0001;

    /* GREEN_LED 闪烁计数器（低电平点亮） */
    uint8_t led_counter = 0;

    for (;;)
    {
        poll_debug_tx_tick = xTaskGetTickCount();
        /* ---- 轮询间隔 ---- */
        vTaskDelay(POLL_INTERVAL_TICKS);

        /* ---- 排空迪文屏更新队列（若 UART3 空闲） ---- */
        DWIN_FlushQueue();

        /* ---- GREEN_LED 闪烁：每 10 次翻转一次（≈1Hz） ---- */
        led_counter++;
        if (led_counter >= 10)
        {
            led_counter = 0;
            HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin);
        }

        /* ---- 判断本次轮询类型 ---- */
        if (poll_count >= POLL_SEQUENTIAL_BETWEEN_ONLINE)
        {
            /* --- 在线传感器轮询 --- */
            uint8_t online_slave = FindNextOnline(&online_idx);

            if (online_slave != 0)
            {
                req.slave_addr = online_slave;
                ModbusMaster_EnqueueRequest(&req);
                poll_count = 0;         /* 重置计数，下轮回到顺序扫描 */
                continue;               /* 跳过下方的顺序路径 */
            }
            /* 无在线传感器：退化为顺序轮询，不清空计数 */
        }

        /* --- 顺序地址轮询 --- */
        req.slave_addr = scan_addr;
        ModbusMaster_EnqueueRequest(&req);

        /* 推进扫描地址 */
        scan_addr++;
        if (scan_addr > MODBUS_MAX_SLAVES)
        {
            scan_addr = 0;

            /* ---- 新一轮开始前读取 ID1-ID8 DIP 开关（低电平=1） ---- */
            uint8_t dip = 0;
            if (HAL_GPIO_ReadPin(ID8_GPIO_Port, ID8_Pin) == GPIO_PIN_RESET) dip |= 0x80;
            if (HAL_GPIO_ReadPin(ID7_GPIO_Port, ID7_Pin) == GPIO_PIN_RESET) dip |= 0x40;
            if (HAL_GPIO_ReadPin(ID6_GPIO_Port, ID6_Pin) == GPIO_PIN_RESET) dip |= 0x20;
            if (HAL_GPIO_ReadPin(ID5_GPIO_Port, ID5_Pin) == GPIO_PIN_RESET) dip |= 0x10;
            if (HAL_GPIO_ReadPin(ID4_GPIO_Port, ID4_Pin) == GPIO_PIN_RESET) dip |= 0x08;
            if (HAL_GPIO_ReadPin(ID3_GPIO_Port, ID3_Pin) == GPIO_PIN_RESET) dip |= 0x04;
            if (HAL_GPIO_ReadPin(ID2_GPIO_Port, ID2_Pin) == GPIO_PIN_RESET) dip |= 0x02;
            if (HAL_GPIO_ReadPin(ID1_GPIO_Port, ID1_Pin) == GPIO_PIN_RESET) dip |= 0x01;
            ModbusReg_SetBoardAddr(dip);

            /* 完成一轮完整扫描 → 推进离线检测周期 */
            ModbusReg_StepCycle();
        }

        poll_count++;
    }
}

/* USER CODE END 1 */
