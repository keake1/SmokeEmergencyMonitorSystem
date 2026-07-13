/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    freertos_hooks.c
  * @brief   FreeRTOS 钩子函数实现
  *
  * 栈溢出钩子：
  *   configCHECK_FOR_STACK_OVERFLOW 设为 2 时，FreeRTOS 在每次上下文切换时
  *   检查任务栈末尾的标记模式（0xA5A5A5A5）是否被破坏。
  *   若被破坏则调用此钩子。
  *
  *   调试方法：在 `__disable_irq()` 行打断点，
  *   观察 pcTaskName 参数即可知道哪个任务溢出了。
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* USER CODE BEGIN 1 */

/**
  * @brief  栈溢出钩子
  * @note   当 configCHECK_FOR_STACK_OVERFLOW > 0 时，FreeRTOS 在检测到
  *         栈溢出后调用此函数。
  * @param  xTask      溢出任务句柄
  * @param  pcTaskName 溢出任务名称
  */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;

    /* 停所有中断，卡住 —— 调试器看 pcTaskName 即知哪个任务溢出 */
    taskDISABLE_INTERRUPTS();
    for (;;);
}

/* USER CODE END 1 */
