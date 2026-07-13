/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : modbus_tasks.h
  * @brief          : Modbus 发送任务和接收任务的声明
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MODBUS_TASKS_H__
#define __MODBUS_TASKS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Modbus 发送任务
  * @note   从 xModbusSendQueue 取出请求，构造 Modbus 帧并发送。
  *         发送后等待 xModbusTxSemaphore（表示接收完毕），
  *         然后等待 3.5 字符帧间隔再处理下一个请求。
  *
  * @param  arg  未使用
  */
void TaskModbusSend(void *arg);

/**
  * @brief  Modbus 接收任务
  * @note   从 xModbusRawRxQueue 取出原始帧，CRC 校验，
  *         有效帧放入 xModbusResponseQueue 并释放 xModbusTxSemaphore。
  *
  * @param  arg  未使用
  */
void TaskModbusReceive(void *arg);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __MODBUS_TASKS_H__ */
