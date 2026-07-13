/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : uart1_modbus_master.c
  * @brief          : Modbus RTU Master driver — 队列驱动架构 (STM32F407)
  *
  * 参考 Controlboard/Core/Src/uart1_modbus.c 移植。
  *
  * 设计概要：
  *   1. ModbusMaster_EnqueueRequest()  — 应用层入队接口
  *   2. ModbusMaster_RxIdleHandler()   — ISR 中检测 IDLE 标志时调用
  *   3. CRC 校验在接收任务中完成
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "uart1_modbus_master.h"
#include "usart.h"
#include <string.h>

/* USER CODE BEGIN 0 */

/* ========== 内部接收缓冲区 ==========
 * RXNE 中断将收到的字节依次存入此缓冲区，
 * IDLE 中断到来时将缓冲区内容作为完整一帧送入原始队列。
 */
static uint8_t  rx_buffer[MODBUS_MASTER_RX_BUF_SIZE];
static volatile uint16_t rx_index = 0;

/* ========== 队列/信号量定义 ========== */
QueueHandle_t      xMasterSendQueue    = NULL;
SemaphoreHandle_t  xMasterRxSem        = NULL;
SemaphoreHandle_t  xMasterTxCompleteSem = NULL;

/* 最近一次发送的 Modbus 请求信息（供接收任务解析） */
HostboardLastReq_t g_host_last_req;

/* 原始响应队列（ISR → 接收任务） */
static QueueHandle_t xMasterRawRxQueue = NULL;

/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 1 */

/**
  * @brief  Modbus RTU CRC16 计算
  * @param  data  数据指针
  * @param  len   数据长度
  * @return CRC16 值（低字节在前）
  */
uint16_t ModbusMaster_CRC16(const uint8_t *data, uint16_t len)
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

/* ==================== 队列/信号量初始化 ==================== */

/**
  * @brief  初始化 Modbus 队列和信号量
  * @note   必须在创建 FreeRTOS 任务之前调用。
  */
void ModbusMaster_InitQueues(void)
{
    if (xMasterSendQueue == NULL)
        xMasterSendQueue = xQueueCreate(MODBUS_MASTER_SEND_QUEUE_LEN, sizeof(ModbusMasterRequest_t));

    if (xMasterRawRxQueue == NULL)
        xMasterRawRxQueue = xQueueCreate(MODBUS_MASTER_RAW_RX_QUEUE_LEN, sizeof(ModbusMasterFrame_t));

    if (xMasterRxSem == NULL)
        xMasterRxSem = xSemaphoreCreateBinary();

    if (xMasterTxCompleteSem == NULL)
        xMasterTxCompleteSem = xSemaphoreCreateBinary();
}

/* ==================== 复位接收缓冲 ==================== */

/**
  * @brief  复位接收缓冲区索引
  * @note   在 ORE/FE/NE/PE 等错误导致数据不可靠时调用。
  */
void ModbusMaster_ResetRx(void)
{
    rx_index = 0;
}

/* ==================== 中断接收控制 ==================== */

/**
  * @brief  开启 USART1 RXNE + IDLE 中断
  */
void ModbusMaster_StartRx(void)
{
    rx_index = 0;

    SET_BIT(huart1.Instance->CR1, USART_CR1_RXNEIE);
    SET_BIT(huart1.Instance->CR1, USART_CR1_IDLEIE);
}

/**
  * @brief  关闭 USART1 RXNE + IDLE 中断
  * @note   在不需要接收时关闭，防止总线噪声触发无限中断。
  */
void ModbusMaster_DisableRx(void)
{
    CLEAR_BIT(huart1.Instance->CR1, USART_CR1_RXNEIE);
    CLEAR_BIT(huart1.Instance->CR1, USART_CR1_IDLEIE);
}

/* ==================== 中断处理函数（ISR 中调用） ==================== */

/**
  * @brief  USART1 字节接收中断处理
  * @param  data  收到的字节
  */
void ModbusMaster_RxByteHandler(uint8_t data)
{
    if (rx_index < MODBUS_MASTER_RX_BUF_SIZE)
    {
        rx_buffer[rx_index++] = data;
    }
}

/**
  * @brief  USART1 空闲中断处理
  * @note   将 rx_buffer 送入原始响应队列（ISR 安全版本）。
  */
void ModbusMaster_RxIdleHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    ModbusMasterFrame_t resp;

    if (rx_index > 0)
    {
        resp.length = (rx_index < MODBUS_MASTER_RX_BUF_SIZE) ? rx_index : MODBUS_MASTER_RX_BUF_SIZE;
        memcpy(resp.frame, (const void *)rx_buffer, resp.length);

        xQueueSendFromISR(xMasterRawRxQueue, &resp, &xHigherPriorityTaskWoken);
    }

    rx_index = 0;

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* ==================== 原始帧取出（接收任务调用） ==================== */

/**
  * @brief  从原始响应队列取一帧（阻塞）
  * @param  raw     输出帧
  * @param  timeout 超时 tick
  * @retval 0 成功，-1 失败/超时
  */
int ModbusMaster_DequeueRawFrame(ModbusMasterFrame_t *raw, TickType_t timeout)
{
    if (xMasterRawRxQueue == NULL) return -1;

    if (xQueueReceive(xMasterRawRxQueue, raw, timeout) == pdPASS)
        return 0;

    return -1;
}

/* ==================== 应用层 API ==================== */

/**
  * @brief  将一次 Modbus 请求加入发送队列
  * @retval 0 成功，-1 失败
  */
int ModbusMaster_EnqueueRequest(const ModbusMasterRequest_t *req)
{
    if (xMasterSendQueue == NULL) return -1;

    if (xQueueSend(xMasterSendQueue, req, portMAX_DELAY) == pdPASS)
        return 0;

    return -1;
}

/* USER CODE END 1 */
