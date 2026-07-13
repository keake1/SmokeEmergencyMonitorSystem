/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : uart1_modbus.c
  * @brief          : Modbus RTU master driver — 队列驱动架构
  *
  * 设计概要：
  *   1. ModbusMaster_EnqueueRequest()  /  ModbusMaster_GetResponse()
  *      应用层入队/出队接口，通过 FreeRTOS 队列与任务通信。
  *   2. ModbusMaster_ReadRegisters()  /  ModbusMaster_WriteRegister()
  *      同步阻塞封装，内部调用队列 API + 等待响应，保持向后兼容。
  *   3. ModbusMaster_RxIdleHandler()
  *      在 USART1_IRQHandler 中检测到 IDLE 标志时调用，
  *      将接收缓冲区中的完整一帧送入原始响应队列。
  *   4. CRC 校验在接收任务（TaskModbusReceive）中完成。
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "uart1_modbus.h"
#include "usart.h"
#include <string.h>

/* USER CODE BEGIN 0 */

/* ========== 内部接收缓冲区 ==========
 * RXNE 中断将接收到的字节依次存入此缓冲区，
 * IDLE 中断到来时将缓冲区内容作为完整一帧送入原始响应队列。
 */
static uint8_t  rx_buffer[MODBUS_RX_BUF_SIZE];
static volatile uint16_t rx_index = 0;

/* ========== 队列/信号量定义 ========== */
QueueHandle_t      xModbusSendQueue     = NULL;
SemaphoreHandle_t  xModbusTxSemaphore   = NULL;
SemaphoreHandle_t  xModbusTxCompleteSem = NULL;

/* 原始响应队列（内部使用，ISR→接收任务，不在 .h 中暴露） */
static QueueHandle_t xModbusRawRxQueue = NULL;

/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 1 */

/**
  * @brief  计算 Modbus RTU CRC16
  * @param  data 数据指针
  * @param  len  数据长度
  * @return CRC16 值（低字节在前）
  */
uint16_t Modbus_CRC16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    uint16_t i, j;

    for (i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
    }
    return crc;
}

/* USER CODE END 1 */

/* USER CODE BEGIN 2 */

/* ==================== 队列/信号量初始化 ==================== */

/**
  * @brief  初始化 Modbus 队列和信号量
  * @note   必须在创建 FreeRTOS 任务之前调用。
  */
void ModbusMaster_InitQueues(void)
{
    if (xModbusSendQueue == NULL)
        xModbusSendQueue = xQueueCreate(MODBUS_SEND_QUEUE_LEN, sizeof(ModbusRequest_t));

    if (xModbusRawRxQueue == NULL)
        xModbusRawRxQueue = xQueueCreate(MODBUS_RAW_RX_QUEUE_LEN, sizeof(ModbusResponse_t));

    if (xModbusTxSemaphore == NULL)
        xModbusTxSemaphore = xSemaphoreCreateBinary();

    if (xModbusTxCompleteSem == NULL)
        xModbusTxCompleteSem = xSemaphoreCreateBinary();
}

/* ==================== 中断接收启动 ==================== */

/**
  * @brief  开启 USART1 RXNE + IDLE 中断
  * @note   RXNE 中断逐字节接收，IDLE 中断判断一帧结束。
  *         此函数通常在 main 初始化末尾调用一次。
  */
void ModbusMaster_StartRx(void)
{
    rx_index = 0;

    /* 使能 USART1 RXNE 中断 — 每收到一个字节触发 */
    SET_BIT(huart1.Instance->CR1, USART_CR1_RXNEIE);

    /* 使能 USART1 IDLE 中断 — 总线空闲（一帧结束）触发 */
    SET_BIT(huart1.Instance->CR1, USART_CR1_IDLEIE);
}

/**
  * @brief  重置接收缓冲区索引（ORE 溢出时调用）
  * @note   在 ISR 的 ORE 处理中调用，丢弃可能已损坏的接收数据。
  */
void ModbusMaster_ResetRx(void)
{
    rx_index = 0;
}

/* ==================== 中断处理函数（ISR 中调用） ==================== */

/**
  * @brief  USART1 字节接收中断处理
  * @note   在 USART1_IRQHandler 的 RXNE 分支中调用。
  *         从机回复的每个字节依次存入内部缓冲区。
  * @param  data  收到的字节
  */
void ModbusMaster_RxByteHandler(uint8_t data)
{
    /* 缓冲区防溢出 */
    if (rx_index < MODBUS_RX_BUF_SIZE)
    {
        rx_buffer[rx_index++] = data;
    }
}

/**
  * @brief  USART1 空闲中断处理函数
  * @note   在 USART1_IRQHandler 中检测到 IDLE 标志后调用。
  *         将 rx_buffer 中的完整帧送入原始响应队列（xModbusRawRxQueue）。
  *
  *         此函数在 ISR 上下文中执行，使用 FromISR 版本的队列 API。
  */
void ModbusMaster_RxIdleHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ModbusResponse_t resp;

    /* 索引 > 0 才是一帧有效数据 */
    if (rx_index > 0)
    {
        resp.length = (rx_index < MODBUS_RX_BUF_SIZE) ? rx_index : MODBUS_RX_BUF_SIZE;
        memcpy(resp.frame, (const void *)rx_buffer, resp.length);

        /* 送入原始响应队列（ISR 安全版本） */
        xQueueSendFromISR(xModbusRawRxQueue, &resp, &xHigherPriorityTaskWoken);
    }

    /* 重置缓冲区，准备接收下一帧 */
    rx_index = 0;

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* ==================== 应用层 API ==================== */

/**
  * @brief  将一次 Modbus 请求加入发送队列
  */
int ModbusMaster_EnqueueRequest(const ModbusRequest_t *req)
{
    if (xModbusSendQueue == NULL) return -1;

    if (xQueueSend(xModbusSendQueue, req, portMAX_DELAY) == pdPASS)
        return 0;

    return -1;
}


/* ==================== 同步阻塞封装（向后兼容） ==================== */

/**
  * @brief  向传感器发送读保持寄存器指令（仅入队，不等待响应）
  *
  * 响应需要通过 ModbusMaster_GetResponse() 单独获取。
  *
  * @param  huart    未使用，保留兼容
  * @param  slave    从机地址
  * @param  reg_addr 寄存器起始地址
  * @param  reg_cnt  寄存器数量
  * @param  rx_buf   未使用（仅在同步模式需要）
  * @param  rx_len   未使用
  * @param  timeout  未使用
  * @retval 0 入队成功，-1 入队失败
  */
int ModbusMaster_ReadRegisters(UART_HandleTypeDef *huart,
                               uint8_t slave,
                               uint16_t reg_addr,
                               uint16_t reg_cnt,
                               uint8_t *rx_buf,
                               uint16_t *rx_len,
                               uint32_t timeout)
{
    (void)huart;
    (void)rx_buf;
    (void)rx_len;
    (void)timeout;

    ModbusRequest_t req;

    req.slave_addr = slave;
    req.func_code  = MODBUS_FUNC_READ_HOLDING_REGISTERS;
    req.reg_addr   = reg_addr;
    req.reg_value  = reg_cnt;

    return ModbusMaster_EnqueueRequest(&req);
}

/**
  * @brief  向传感器写单个寄存器（仅入队，不等待响应）
  *
  * @param  huart    未使用，保留兼容
  * @param  slave    从机地址
  * @param  reg_addr 寄存器地址
  * @param  value    写入值
  * @param  timeout  未使用
  * @retval 0 入队成功，-1 入队失败
  */
int ModbusMaster_WriteRegister(UART_HandleTypeDef *huart,
                               uint8_t slave,
                               uint16_t reg_addr,
                               uint16_t value,
                               uint32_t timeout)
{
    (void)huart;
    (void)timeout;

    ModbusRequest_t req;

    req.slave_addr = slave;
    req.func_code  = MODBUS_FUNC_WRITE_SINGLE_REGISTER;
    req.reg_addr   = reg_addr;
    req.reg_value  = value;

    return ModbusMaster_EnqueueRequest(&req);
}

/* USER CODE END 2 */

/* USER CODE BEGIN 3 */

/* ==================== 原始帧取出（接收任务调用） ==================== */

/**
  * @brief  从原始响应队列取一帧（阻塞）
  * @param  raw     输出：原始响应帧
  * @param  timeout 超时 tick
  * @retval 0 成功，-1 失败/超时
  */
int ModbusMaster_DequeueRawFrame(ModbusResponse_t *raw, TickType_t timeout)
{
    if (xModbusRawRxQueue == NULL) return -1;

    if (xQueueReceive(xModbusRawRxQueue, raw, timeout) == pdPASS)
        return 0;

    return -1;
}

/* USER CODE END 3 */
