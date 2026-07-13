/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : uart2_modbus_slave.h
  * @brief          : Modbus RTU 从站驱动 — UART2
  *
  * 架构：
  *   Hostboard(主站) ← RS485 ← Controlboard UART2(从站)
  *
  *   从站地址 = Controlboard DIP 开关读取的地址 (ModbusReg_GetBoardAddr())
  *   协议：FC 0x02 (读离散输入) + FC 0x03 (读保持寄存器)
  *
  * ISR → xSlaveRawRxQueue → TaskSlaveRecv → xSlaveTxQueue → TaskSlaveSend → UART TX
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __UART2_MODBUS_SLAVE_H__
#define __UART2_MODBUS_SLAVE_H__

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

/** @defgroup Slave_Buffer_Sizes 缓冲区大小 */
/** @{ */
#define MODBUS_SLAVE_RX_BUF_SIZE    64
#define MODBUS_SLAVE_RX_QUEUE_LEN   4
#define MODBUS_SLAVE_TX_QUEUE_LEN   4
/** @} */

/* USER CODE BEGIN EConst */

/* USER CODE END EConst */

/* Exported types ------------------------------------------------------------*/

/**
  * @brief 原始帧/发送帧类型
  */
typedef struct {
    uint8_t  frame[MODBUS_SLAVE_RX_BUF_SIZE];
    uint16_t length;
} ModbusSlaveFrame_t;

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported queue/semaphore handles -----------------------------------------*/

extern QueueHandle_t      xSlaveTxQueue;         /* 发送队列（接收任务 → 发送任务） */
extern SemaphoreHandle_t  xSlaveTxCompleteSem;    /* UART2 TX 完成信号量 */

/* Exported functions prototypes ---------------------------------------------*/

void     ModbusSlave_InitQueues(void);
void     ModbusSlave_StartRx(void);
void     ModbusSlave_ResetRx(void);
void     ModbusSlave_RxByteHandler(uint8_t data);
void     ModbusSlave_RxIdleHandler(void);
int      ModbusSlave_DequeueRawFrame(ModbusSlaveFrame_t *raw, TickType_t timeout);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __UART2_MODBUS_SLAVE_H__ */
