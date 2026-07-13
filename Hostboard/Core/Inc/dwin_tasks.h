/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : dwin_tasks.h
  * @brief          : USART3 迪文屏发送/接收 FreeRTOS 任务声明
  *
  * TaskDwinTx： 从发送队列取帧 → HAL_UART_Transmit_IT → 等 TX 完成信号量
  * TaskDwinRx： 从接收队列取帧 → 将来解析帧内容
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __DWIN_TASKS_H__
#define __DWIN_TASKS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"

/* USER CODE BEGIN Includes */
#include "semphr.h"
#include "queue.h"
/* USER CODE END Includes */

/* Exported functions prototypes ---------------------------------------------*/

void TaskDwinTx(void *arg);
void TaskDwinRx(void *arg);
void TaskDwinIcons(void *arg);
void TaskAlarmMonitor(void *arg);
void TaskPrinterTx(void *arg);    /* 新增 */

/* USER CODE BEGIN EFP */

/* 迪文屏初始化完成信号量（Counting，最大 2，初值 0） */
extern SemaphoreHandle_t xDwinInitDoneSem;
void TaskDwinInit(void *arg);

/* 打印机 TX 完成信号量 */
extern SemaphoreHandle_t xPrinterTxCompleteSem;

/* 打印机发送队列 */
extern QueueHandle_t     xPrinterTxQueue;

/* 打印机作业结构体 */
typedef struct {
    uint8_t     ctrlAddr;   /* 控制器地址 1-128 */
    uint8_t     sensorIdx;  /* 传感器索引 0=烟雾, 1-63=传感器编号 */
    uint8_t     rtcYear;
    uint8_t     rtcMonth;
    uint8_t     rtcDay;
    uint8_t     rtcHour;
    uint8_t     rtcMinute;
    uint8_t     rtcSecond;
} printer_job_t;

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DWIN_TASKS_H__ */
