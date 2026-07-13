/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : modbus_polling.h
  * @brief          : Modbus 轮询任务声明
  *
  * 轮询策略：
  *   1. 依次扫描地址 0-63（功能码 0x03，寄存器 0x0000，数量 1）
  *   2. 每进行 8 次顺序地址轮询，穿插 1 次在线传感器轮询
  *      —— 提高在线传感器的数据更新频率
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
  * @brief  Modbus 轮询任务
  * @note   依次轮询地址 0-63，每 8 次顺序轮询穿插 1 次
  *         在线传感器轮询。请求入发送队列后即返回，
  *         由发送/接收任务完成实际收发和解析。
  *
  * @param  arg  未使用
  */
void TaskModbusPoll(void *arg);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __MODBUS_POLLING_H__ */
