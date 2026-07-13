/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : detail_view_data.h
  * @brief          : 详情界面数据存储 — 传感器型号 + 采集状态
  *
  * 存储选中控制器中各传感器的型号信息，以及线圈数据是否已就绪的标志。
  * 选中控制器切换时调用 DetailView_Reset() 清空所有缓冲。
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __DETAIL_VIEW_DATA_H__
#define __DETAIL_VIEW_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported constants --------------------------------------------------------*/

/* USER CODE BEGIN EConst */

#define DETAIL_SENSOR_MAX   64U   /* 传感器索引 1-63，下标 0 预留 */

/* USER CODE END EConst */

/* Exported functions prototypes ---------------------------------------------*/

void    DetailView_Reset(void);
void    DetailView_SetType(uint8_t sensor_idx, uint8_t type);
uint8_t DetailView_GetType(uint8_t sensor_idx);
void    DetailView_SetCoilReady(uint8_t ctrl_addr);
uint8_t DetailView_IsCoilReady(uint8_t ctrl_addr);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DETAIL_VIEW_DATA_H__ */
