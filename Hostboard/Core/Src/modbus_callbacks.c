/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : modbus_callbacks.c
  * @brief          : HAL UART TX/RX 完成回调 (Hostboard)
  *
  * USART1 TX 完成回调：释放 xMasterTxCompleteSem 信号量，
  * 通知发送任务可以继续执行。
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dwin_tasks.h"
/* USER CODE END Includes */
#include "uart1_modbus_master.h"
#include "usart.h"

/* USER CODE BEGIN 0 */
#include "dwin.h"

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        BaseType_t xWoken = pdFALSE;
        xSemaphoreGiveFromISR(xMasterTxCompleteSem, &xWoken);
        portYIELD_FROM_ISR(xWoken);
    }
    /* ---- USART3 迪文屏 TX 完成 ---- */
    else if (huart == &huart3)
    {
        BaseType_t xWoken = pdFALSE;
        xSemaphoreGiveFromISR(xDwinTxCompleteSem, &xWoken);
        portYIELD_FROM_ISR(xWoken);
    }
    /* ---- USART2 打印机 TX 完成 ---- */
    else if (huart == &huart2)
    {
        BaseType_t xWoken = pdFALSE;
        xSemaphoreGiveFromISR(xPrinterTxCompleteSem, &xWoken);
        portYIELD_FROM_ISR(xWoken);
    }
}

/* USER CODE END 0 */
