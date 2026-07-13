/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : uart1_modbus_master.h
  * @brief          : Modbus RTU 主站驱动 — USART1 (F407)
  *
  * 架构：
  *   应用请求入发送队列 → 发送任务发帧 → 从站回复 → IDLE 中断收帧
  *   → 原始队列 → 接收任务 CRC 校验 + 解析
  *
  * 参考 Controlboard 的 uart1_modbus.c，移植到 STM32F407。
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __UART1_MODBUS_MASTER_H__
#define __UART1_MODBUS_MASTER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported constants --------------------------------------------------------*/

/** @defgroup Modbus_Function_Codes 功能码 */
/** @{ */
#define MODBUS_FUNC_READ_DISCRETE_INPUTS    0x02
#define MODBUS_FUNC_READ_HOLDING_REGISTERS  0x03
/** @} */

/** @defgroup Buffer_Sizes 缓冲区大小 */
/** @{ */
#define MODBUS_MASTER_RX_BUF_SIZE       64
#define MODBUS_MASTER_SEND_QUEUE_LEN    8
#define MODBUS_MASTER_RAW_RX_QUEUE_LEN  4
/** @} */

/* USER CODE BEGIN EConst */

/* USER CODE END EConst */

/* Exported types ------------------------------------------------------------*/

/**
  * @brief 发送队列项
  */
typedef struct {
    uint8_t  slave_addr;   /* 从机地址 1~247 */
    uint8_t  func_code;    /* 0x02 / 0x03 */
    uint16_t reg_addr;     /* 寄存器起始地址 */
    uint16_t reg_value;    /* 读取数量 */
} ModbusMasterRequest_t;

/**
  * @brief 原始响应帧（ISR → 接收任务）
  */
typedef struct {
    uint8_t  frame[MODBUS_MASTER_RX_BUF_SIZE];
    uint16_t length;
} ModbusMasterFrame_t;

/* USER CODE BEGIN ET */

/**
 * @brief  最近一次发送的 Modbus 请求信息
 * @note   发送任务在发帧前写入，接收任务据此解析响应数据的位置。
 *         同一时刻只有一帧在飞行，不存在竞态。
 */
typedef struct {
    uint8_t  slave_addr;   /* 请求的从机地址 */
    uint8_t  func_code;    /* 功能码 (0x02/0x03) */
    uint16_t reg_addr;     /* 寄存器起始地址 */
    uint16_t reg_count;    /* 请求的寄存器/位数数量 */
} HostboardLastReq_t;

/* USER CODE END ET */

/* Exported queue/semaphore handles -----------------------------------------*/

extern QueueHandle_t      xMasterSendQueue;        /* 发送队列 */
extern SemaphoreHandle_t  xMasterRxSem;            /* 接收完成信号量 */
extern SemaphoreHandle_t  xMasterTxCompleteSem;    /* USART1 TX 完成信号量 */
extern HostboardLastReq_t g_host_last_req;         /* 当前在飞的请求信息 */

/* Exported functions prototypes ---------------------------------------------*/

void     ModbusMaster_InitQueues(void);
void     ModbusMaster_ResetRx(void);
void     ModbusMaster_StartRx(void);
void     ModbusMaster_DisableRx(void);
void     ModbusMaster_RxByteHandler(uint8_t data);
void     ModbusMaster_RxIdleHandler(void);
uint16_t ModbusMaster_CRC16(const uint8_t *data, uint16_t len);
int      ModbusMaster_EnqueueRequest(const ModbusMasterRequest_t *req);
int      ModbusMaster_DequeueRawFrame(ModbusMasterFrame_t *raw, TickType_t timeout);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __UART1_MODBUS_MASTER_H__ */
