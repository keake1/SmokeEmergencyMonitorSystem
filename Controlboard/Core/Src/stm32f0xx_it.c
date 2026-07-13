/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f0xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "uart1_modbus.h"
#include "uart2_modbus_slave.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* The handlers SVC_Handler, PendSV_Handler, and SysTick_Handler are provided
 * by the FreeRTOS port layer when FreeRTOS is enabled.
 * We make them weak here so FreeRTOS's strong definitions override them.
 * See: Drivers/Middlewares/FreeRTOS/portable/GCC/ARM_CM0/portasm.c
 *      Drivers/Middlewares/FreeRTOS/portable/GCC/ARM_CM0/port.c */
#pragma weak SVC_Handler
#pragma weak PendSV_Handler
#pragma weak SysTick_Handler

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M0 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVC_IRQn 0 */

  /* USER CODE END SVC_IRQn 0 */
  /* USER CODE BEGIN SVC_IRQn 1 */

  /* USER CODE END SVC_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* ---- RXNE：收到一个字节 ---- */
  if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE))
  {
      /* 从数据寄存器读取字节（读取操作自动清除 RXNE 标志） */
      uint8_t data = (uint8_t)(huart1.Instance->RDR & 0xFF);

      /* 交由 Modbus 驱动处理（存入内部缓冲区） */
      ModbusMaster_RxByteHandler(data);
  }

  /* ---- IDLE：总线空闲，一帧结束 ---- */
  if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE))
  {
      /* F0 系列（新版 USART IP）必须通过写 ICR 清除 IDLE 标志。
       * 注意：F1 的"读 SR → 读 DR"序列在 F0 上无效，
       * 若不写 ICR，IDLE 将持续置位导致中断风暴，饿死所有任务。 */
      __HAL_UART_CLEAR_IDLEFLAG(&huart1);

      /* 将完整帧送入原始响应队列 */
      ModbusMaster_RxIdleHandler();
  }

  /* ---- ORE：溢出错误，必须写 ICR 清除 ----
   * 若交给 HAL_UART_IRQHandler 处理，HAL 会视为阻塞性错误
   * 并关闭 RXNEIE，导致 Modbus 接收静默失效。 */
  if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE))
  {
      __HAL_UART_CLEAR_OREFLAG(&huart1);
      ModbusMaster_ResetRx();
  }

  /* ---- FE/NE/PE：帧/噪声/校验错误，必须写 ICR 清除 ----
   * 关键：EIE 未使能时 HAL 不会清除这些标志，但 HAL_UART_IRQHandler
   * 检测到 errorflags != 0 且 RXNEIE 使能时会走错误分支提前 return，
   * 永远到不了 TXE 处理分支 → TXEIE 无人关闭 → 中断风暴、
   * 中断发送（HAL_UART_Transmit_IT）永久卡死。 */
  if (huart1.Instance->ISR & (USART_ISR_FE | USART_ISR_NE | USART_ISR_PE))
  {
      __HAL_UART_CLEAR_FEFLAG(&huart1);
      __HAL_UART_CLEAR_NEFLAG(&huart1);
      __HAL_UART_CLEAR_PEFLAG(&huart1);
      /* 错误帧数据不可靠，丢弃当前接收缓冲 */
      ModbusMaster_ResetRx();
  }

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */

  /* ---- RXNE：收到一个字节 ---- */
  if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE))
  {
      uint8_t data = (uint8_t)(huart2.Instance->RDR & 0xFF);
      ModbusSlave_RxByteHandler(data);
  }

  /* ---- IDLE：总线空闲，一帧结束 ---- */
  if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_IDLE))
  {
      __HAL_UART_CLEAR_IDLEFLAG(&huart2);
      ModbusSlave_RxIdleHandler();
  }

  /* ---- ORE：溢出错误，必须写 ICR 清除 ----
   * 若交给 HAL_UART_IRQHandler 处理，HAL 会视为阻塞性错误
   * 并关闭 RXNEIE，导致 Modbus 接收静默失效。 */
  if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE))
  {
      __HAL_UART_CLEAR_OREFLAG(&huart2);
      ModbusSlave_ResetRx();
  }

  /* ---- FE/NE/PE：同 USART1，防止 HAL 错误分支导致 TXE 中断风暴 ---- */
  if (huart2.Instance->ISR & (USART_ISR_FE | USART_ISR_NE | USART_ISR_PE))
  {
      __HAL_UART_CLEAR_FEFLAG(&huart2);
      __HAL_UART_CLEAR_NEFLAG(&huart2);
      __HAL_UART_CLEAR_PEFLAG(&huart2);
      ModbusSlave_ResetRx();
  }

  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/**
  * @brief This function handles USART3 and USART4 global interrupts.
  */
void USART3_4_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_4_IRQn 0 */

  /* USER CODE END USART3_4_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_4_IRQn 1 */

  /* USER CODE END USART3_4_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
