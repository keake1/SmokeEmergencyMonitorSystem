/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : modbus_polling.h
  * @brief          : Modbus 轮询任务 — 定时读取所有 Controlboard 离散输入
  *
  * 轮询内容：FC 0x02 读取 0-129 位离散输入寄存器，获取每个 Controlboard
  * 的传感器在线/报警状态、零地址/重复地址/全局报警标志。
  *
  * 轮询地址范围：0-128（含地址 0 用于零地址检测）
  * 轮询间隔：50ms（每个地址之间固定延时）
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MODBUS_POLLING_H__
#define __MODBUS_POLLING_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported functions prototypes ---------------------------------------------*/

void TaskModbusPoll(void *arg);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __MODBUS_POLLING_H__ */
