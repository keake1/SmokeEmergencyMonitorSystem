/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : dwin.h
  * @brief          : 迪文屏 DGUS 通信驱动 — 0x82 写变量指令
  *
  * 通信参数：UART3, 115200 8N1
  *
  * 变量区分配（0x1000 - 0x13FF）：
  *   每传感器固定 16 个地址，共 64 传感器 × 16 = 1024 个地址
  *   地址 64 不使用（最后 16 地址闲置）
  *
  * 每传感器 16 地址布局：
  *   偏移 0-1:  CO 值（float，raw÷100）
  *   偏移 2-3:  风压值（float，raw 直转）
  *   偏移 4:    余压值（uint16，raw 直写）
  *   偏移 5:    eCO₂（uint16，raw 直写）
  *   偏移 6:    eCH₂O（uint16，raw 直写）
  *   偏移 7:    TVOC（uint16，raw 直写）
  *   偏移 8-9:  PM2.5（float，raw 直转）
  *   偏移 10-11: PM10（float，raw 直转）
  *   偏移 12-13: 温度（float，raw÷10）
  *   偏移 14-15: 湿度（float，raw÷10）
  *
  * 0x82 写变量指令帧格式：
  *   5A A5 [LEN] 82 [AH] [AL] [D0] [D1] ... [Dn]
  *   LEN = 3 + data_len（指令码1 + 地址2 + 数据n 字节数）
  *
  * 数据以大端序传输（调用方负责转换）。
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __DWIN_H__
#define __DWIN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported constants --------------------------------------------------------*/

/** @defgroup DWIN_Protocol 迪文屏协议常量 */
/** @{ */
#define DWIN_HEADER_1           0x5A
#define DWIN_HEADER_2           0xA5
#define DWIN_CMD_WRITE_VAR      0x82    /* 写变量指令 */
/** @} */

/** @defgroup DWIN_VarArea 迪文屏变量地址映射 */
/** @{ */
#define DWIN_VAR_BASE           0x1000               /* 传感器数据变量区基地址 */
#define DWIN_ADDRS_PER_SENSOR   16                   /* 每传感器占用地址数 */

/* 字段偏移（相对本传感器的起始地址） */
#define DWIN_OFFSET_CO          0                    /* 2 addr, float */
#define DWIN_OFFSET_WIND        2                    /* 2 addr, float */
#define DWIN_OFFSET_PRESSURE    4                    /* 1 addr, uint16 */
#define DWIN_OFFSET_CO2         5                    /* 1 addr, uint16 */
#define DWIN_OFFSET_CH2O        6                    /* 1 addr, uint16 */
#define DWIN_OFFSET_TVOC        7                    /* 1 addr, uint16 */
#define DWIN_OFFSET_PM25        8                    /* 2 addr, float */
#define DWIN_OFFSET_PM10        10                   /* 2 addr, float */
#define DWIN_OFFSET_TEMP        12                   /* 2 addr, float */
#define DWIN_OFFSET_HUMID       14                   /* 2 addr, float */
/** @} */

/** @defgroup DWIN_VarArea_Status 迪文屏状态变量区 */
/** @{ */
#define DWIN_ICON_BASE          0x1400               /* 传感器状态图标基地址 */
#define DWIN_ICON_COUNT         64                   /* 图标数量（地址 64 不用） */
#define DWIN_ICON_OFFLINE       0                    /* 离线 → 图标值 0 */
#define DWIN_ICON_ALARM         1                    /* 在线+报警 → 图标值 1 */
#define DWIN_ICON_NORMAL        2                    /* 在线+正常 → 图标值 2 */

#define DWIN_FLAG_ALARM         0x1440               /* 全局报警标志（2 地址） */
#define DWIN_FLAG_ZERO_ADDR     0x1442               /* 零地址存在标志 */
#define DWIN_FLAG_CONFLICT      0x1443               /* 地址重复标志 */
#define DWIN_BOARD_ADDR         0x1500               /* 控板 DIP 地址 */
/** @} */

/** @defgroup DWIN_DataLen 各字段数据字节数 */
/** @{ */
#define DWIN_BYTES_FLOAT        4                    /* float 占 4 字节 */
#define DWIN_BYTES_UINT16       2                    /* uint16 占 2 字节 */
/** @} */

/* USER CODE BEGIN EConst */

/* USER CODE END EConst */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  更新某传感器在迪文屏上的全部显示数据
  * @param  slave  传感器地址 (1-63)
  * @param  model  传感器型号字节 (SENSOR_MODEL_CO / _WIND / _PRESSURE / _7IN1)
  * @note   根据型号从寄存器读取对应数据，通过 UART3 写入迪文屏变量区。
  *         只在收到 CRC 正确的传感器响应后调用。
  */
void DWIN_UpdateSensor(uint8_t slave, uint8_t model);

/**
  * @brief  排空更新队列：出队一项并发送（若 UART3 空闲）
  * @note   每轮轮询周期调用一次，确保突发响应结束后的队列积压能及时排空。
  *         队列空或忙时不操作，无开销。
  */
void DWIN_FlushQueue(void);

/**
  * @brief  获取全局报警标志
  * @retval 1  至少一个传感器报警
  *         0  全部传感器正常
  */
uint8_t DWIN_GetGlobalAlarm(void);

/**
  * @brief  迪文屏图标/标志更新任务
  * @param  arg  未使用
  * @note   每 500ms 更新：传感器图标、全局报警、零地址/重复地址标志、控板地址。
  */
void TaskDwinIcons(void *arg);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __DWIN_H__ */
