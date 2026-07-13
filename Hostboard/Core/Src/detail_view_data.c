/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : detail_view_data.c
  * @brief          : 详情界面数据存储实现
  *
  * 传感器型号存储：s_detail_types[sensor_idx]，索引 1-63 对应传感器 1-63
  * 线圈就绪标志：s_detail_coil_ready 表示当前选中控制器的线圈已采集
  * 选中控制器地址：s_detail_select_addr，用于切换时自动重置
  *****************@../docs/superpowers/specs/2026-06-11-hostboard-dwin-detail-view.md*************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "detail_view_data.h"
#include <string.h>

/* USER CODE BEGIN 0 */

static uint8_t s_detail_types[DETAIL_SENSOR_MAX];   /* 传感器型号 (索引 1-63) */
static uint8_t s_detail_coil_ready;                  /* 当前选中控制器的线圈已采集 */
static uint8_t s_detail_select_addr;                 /* 当前缓冲针对的控制器地址 */

/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 1 */

void DetailView_Reset(void)
{
    memset(s_detail_types, 0, sizeof(s_detail_types));
    s_detail_coil_ready = 0;
    s_detail_select_addr = 0;
}

void DetailView_SetType(uint8_t sensor_idx, uint8_t type)
{
    if (sensor_idx < 1 || sensor_idx >= DETAIL_SENSOR_MAX)
        return;
    s_detail_types[sensor_idx] = type;
}

uint8_t DetailView_GetType(uint8_t sensor_idx)
{
    if (sensor_idx < 1 || sensor_idx >= DETAIL_SENSOR_MAX)
        return 0;
    return s_detail_types[sensor_idx];
}

void DetailView_SetCoilReady(uint8_t ctrl_addr)
{
    s_detail_select_addr = ctrl_addr;
    s_detail_coil_ready = 1;
}

uint8_t DetailView_IsCoilReady(uint8_t ctrl_addr)
{
    return (s_detail_coil_ready && s_detail_select_addr == ctrl_addr) ? 1 : 0;
}

/* USER CODE END 1 */
