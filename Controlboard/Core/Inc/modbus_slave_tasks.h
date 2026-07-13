/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : modbus_slave_tasks.h
  * @brief          : Modbus RTU 从站接收/发送任务声明
  *
  * TaskSlaveRecv：等待原始帧队列 → CRC 校验 → 解析 FC 0x02/0x03
  *                → 从寄存器读取数据 → 构造响应 → 入队发送队列
  * TaskSlaveSend：等待发送队列 → UART2 发送 → 3.5 字符间隔
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MODBUS_SLAVE_TASKS_H__
#define __MODBUS_SLAVE_TASKS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported functions prototypes ---------------------------------------------*/

void TaskSlaveRecv(void *arg);
void TaskSlaveSend(void *arg);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __MODBUS_SLAVE_TASKS_H__ */
