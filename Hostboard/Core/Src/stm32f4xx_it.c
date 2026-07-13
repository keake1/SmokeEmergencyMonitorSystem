/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
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
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "uart1_modbus_master.h"
#include "dwin.h"       /* 新增 */
#include "FreeRTOS.h"
#include "task.h"
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

/* ---- FreeRTOS CM4F port 使用别名函数名，需要桥接到向量表 --------------- */
void vPortSVCHandler( void );
void xPortPendSVHandler( void );
void xPortSysTickHandler( void );

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
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
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
__attribute__((naked)) void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */
  __asm volatile ("b vPortSVCHandler");
  /* USER CODE END SVCall_IRQn 0 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
__attribute__((naked)) void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */
  __asm volatile ("b xPortPendSVHandler");
  /* USER CODE END PendSV_IRQn 0 */
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
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
  {
    xPortSysTickHandler();
  }
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
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
      /* F4 系列数据寄存器为 DR（而非 F0 的 RDR） */
      uint8_t data = (uint8_t)(huart1.Instance->DR & 0xFF);
      ModbusMaster_RxByteHandler(data);
  }

  /* ---- IDLE：总线空闲，一帧结束 ---- */
  if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_IDLE))
  {
      __HAL_UART_CLEAR_IDLEFLAG(&huart1);
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

  /* ---- FE/NE/PE：帧/噪声/校验错误，必须写 SR 清除 ----
   * HAL_UART_IRQHandler 检测到 errorflags != 0 且 RXNEIE 使能时
   * 会走错误分支提前 return，永远到不了 TXE 处理分支 →
   * TXEIE 无人关闭 → 中断风暴、中断发送永久卡死。 */
  if (huart1.Instance->SR & (USART_SR_FE | USART_SR_NE | USART_SR_PE))
  {
      __HAL_UART_CLEAR_FEFLAG(&huart1);
      __HAL_UART_CLEAR_NEFLAG(&huart1);
      __HAL_UART_CLEAR_PEFLAG(&huart1);
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

  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */

  /* USER CODE END USART2_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */

  /* ---- RXNE：收到一个字节，送入迪文屏状态机 ---- */
  if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE))
  {
      uint8_t data = (uint8_t)(huart3.Instance->DR & 0xFF);
      DwinRxByteHandler(data);
  }

  /* ---- ORE：溢出错误，清标志 + 重置 RX 状态机 ---- */
  if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_ORE))
  {
      __HAL_UART_CLEAR_OREFLAG(&huart3);
      DwinRxReset();
  }

  /* ---- FE/NE/PE：同 USART1，防止 HAL 错误分支导致 TXE 中断风暴 ---- */
  if (huart3.Instance->SR & (USART_SR_FE | USART_SR_NE | USART_SR_PE))
  {
      __HAL_UART_CLEAR_FEFLAG(&huart3);
      __HAL_UART_CLEAR_NEFLAG(&huart3);
      __HAL_UART_CLEAR_PEFLAG(&huart3);
      DwinRxReset();
  }

  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */

  /* USER CODE END USART3_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
