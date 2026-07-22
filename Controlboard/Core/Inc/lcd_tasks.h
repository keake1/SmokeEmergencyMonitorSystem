/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : lcd_tasks.h
  * @brief          : LCD 显示任务 — 128×64 点阵屏（ST7565R）驱动封装
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __LCD_TASKS_H__
#define __LCD_TASKS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  LCD 显示任务（FreeRTOS）
  * @param  arg  未使用
  * @note  在 main.c 中创建：
  *          xTaskCreate(TaskLcdDisplay, "LcdDisp", 256, NULL, 1, NULL);
  */
void TaskLcdDisplay(void *arg);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __LCD_TASKS_H__ */
