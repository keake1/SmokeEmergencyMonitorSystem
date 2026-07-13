/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : modbus_master_tasks.h
  * @brief          : Modbus RTU 主站发送/接收任务声明
  *
  * TaskModbusSend：  取请求 → 构造帧 → 发送 → 等响应
  * TaskModbusRecv：  取原始帧 → CRC → 解析 → 通知发送任务
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MODBUS_MASTER_TASKS_H__
#define __MODBUS_MASTER_TASKS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported functions prototypes ---------------------------------------------*/

void TaskModbusSend(void *arg);
void TaskModbusRecv(void *arg);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __MODBUS_MASTER_TASKS_H__ */
