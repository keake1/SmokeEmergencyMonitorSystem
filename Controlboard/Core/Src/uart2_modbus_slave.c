/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : uart2_modbus_slave.c
  * @brief          : Modbus RTU 从站驱动 — UART2
  *
  * 与 uart1_modbus.c (主站) 共用同一 CRC16 算法。
  * 从站接收走独立队列 xSlaveRawRxQueue，不与主站冲突。
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "uart2_modbus_slave.h"
#include "uart1_modbus.h"          /* 复用 Modbus_CRC16 */
#include "usart.h"
#include <string.h>

/* USER CODE BEGIN 0 */

/* ========== 内部接收缓冲区 ==========
 * RXNE 中断逐字节存入，IDLE 中断到来时作为完整一帧送入原始帧队列。
 */
static uint8_t  rx_buffer[MODBUS_SLAVE_RX_BUF_SIZE];
static volatile uint16_t rx_index = 0;

/* ========== 队列定义 ========== */
QueueHandle_t xSlaveTxQueue = NULL;
SemaphoreHandle_t xSlaveTxCompleteSem = NULL;

/* 原始响应队列（ISR → 接收任务，不与主站共享） */
static QueueHandle_t xSlaveRawRxQueue = NULL;

/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 1 */

/* ==================== 队列/信号量初始化 ==================== */

/**
  * @brief  初始化 UART2 从站队列
  * @note   在 main 初始化中、任务创建之前调用。
  */
void ModbusSlave_InitQueues(void)
{
    if (xSlaveRawRxQueue == NULL)
        xSlaveRawRxQueue = xQueueCreate(MODBUS_SLAVE_RX_QUEUE_LEN, sizeof(ModbusSlaveFrame_t));

    if (xSlaveTxQueue == NULL)
        xSlaveTxQueue = xQueueCreate(MODBUS_SLAVE_TX_QUEUE_LEN, sizeof(ModbusSlaveFrame_t));

    if (xSlaveTxCompleteSem == NULL)
        xSlaveTxCompleteSem = xSemaphoreCreateBinary();
}

/* ==================== 中断接收启动 ==================== */

/**
  * @brief  开启 USART2 RXNE + IDLE 中断
  * @note   在任务创建之后、调度器启动之前调用。
  */
void ModbusSlave_StartRx(void)
{
    rx_index = 0;

    /* 使能 USART2 RXNE 中断 */
    SET_BIT(huart2.Instance->CR1, USART_CR1_RXNEIE);

    /* 使能 USART2 IDLE 中断 */
    SET_BIT(huart2.Instance->CR1, USART_CR1_IDLEIE);
}

/**
  * @brief  重置接收缓冲区索引（ORE 溢出时调用）
  * @note   在 ISR 的 ORE 处理中调用，丢弃可能已损坏的接收数据。
  */
void ModbusSlave_ResetRx(void)
{
    rx_index = 0;
}

/* ==================== 中断处理函数（ISR 中调用） ==================== */

/**
  * @brief  USART2 字节接收中断处理
  * @param  data  收到的字节
  */
void ModbusSlave_RxByteHandler(uint8_t data)
{
    if (rx_index < MODBUS_SLAVE_RX_BUF_SIZE)
    {
        rx_buffer[rx_index++] = data;
    }
}

/**
  * @brief  USART2 空闲中断处理
  * @note   将 rx_buffer 中的完整帧送入原始响应队列（xSlaveRawRxQueue）。
  */
void ModbusSlave_RxIdleHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ModbusSlaveFrame_t resp;

    if (rx_index > 0)
    {
        resp.length = (rx_index < MODBUS_SLAVE_RX_BUF_SIZE) ? rx_index : MODBUS_SLAVE_RX_BUF_SIZE;
        memcpy(resp.frame, (const void *)rx_buffer, resp.length);

        xQueueSendFromISR(xSlaveRawRxQueue, &resp, &xHigherPriorityTaskWoken);
    }

    rx_index = 0;

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* ==================== 原始帧取出（接收任务调用） ==================== */

/**
  * @brief  从原始帧队列取一帧（阻塞）
  * @param  raw     输出帧
  * @param  timeout 超时 tick
  * @retval 0 成功，-1 失败/超时
  */
int ModbusSlave_DequeueRawFrame(ModbusSlaveFrame_t *raw, TickType_t timeout)
{
    if (xSlaveRawRxQueue == NULL) return -1;

    if (xQueueReceive(xSlaveRawRxQueue, raw, timeout) == pdPASS)
        return 0;

    return -1;
}

/* USER CODE END 1 */
