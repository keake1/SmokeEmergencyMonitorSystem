/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : hostboard_registers.h
  * @brief          : Hostboard 寄存器模块 — 存储 Controlboard 线圈数据
  *
  * 管理 128 个 Controlboard (地址 1-128) 的离散输入寄存器数据，
  * 以及零地址检测和地址重复检测的全局标志。
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __HOSTBOARD_REGISTERS_H__
#define __HOSTBOARD_REGISTERS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/

/* USER CODE BEGIN ET */

/**
 * @brief  Controlboard 信息结构体
 *
 * 每个地址 (1-128) 对应一个结构体实例，合并存储线圈数据、
 * 在线标志和离线检测追踪信息。
 */
typedef struct {
    uint8_t  coil_data[17];               /* 17 字节原始线圈数据 (COIL_BYTE_COUNT) */
    uint32_t last_seen_cycle;              /* 最近收到正确响应时的周期号 */
    uint8_t  online;                       /* 1=在线, 0=离线 */
} CtrlBoardEntry_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

/** @defgroup HostReg_Constants 地址范围和线圈布局 */
/** @{ */
#define MAX_CTRLBD_ADDR     128     /* 最大 Controlboard 地址 */
#define COIL_BYTE_COUNT     17      /* 130 bits (0-129) = 16.25 → 17 字节 */

/* 线圈位偏移定义（对应 Controlboard 的 coil 寄存器映射） */
#define COIL_OFFSET_ONLINE      0    /* 位 0-62:  传感器 1-63 在线状态 */
#define COIL_OFFSET_ALARM       63   /* 位 63-125: 传感器 1-63 报警标志 */
#define COIL_OFFSET_ZERO_ADDR   126  /* 位 126:  零地址传感器存在 */
#define COIL_OFFSET_ADDR_CONF   127  /* 位 127:  地址重复标志 */
#define COIL_GLOBAL_ALARM       128  /* 位 128:  全局报警（任一传感器报警） */
#define COIL_SMOKE_ALARM        129  /* 位 129:  烟雾报警器状态 */
/** @} */

/* USER CODE BEGIN EConst */

/* USER CODE END EConst */

/* Exported functions prototypes ---------------------------------------------*/

/** @defgroup HostReg_Data_Storage 线圈数据存储与读取 */
/** @{ */
void    HostReg_StoreCoilData(uint8_t addr, const uint8_t *data, uint8_t len);
uint8_t HostReg_GetCoilByte(uint8_t addr, uint8_t byte_idx);
uint8_t HostReg_GetCoilBit(uint8_t addr, uint16_t bit_idx);
/** @} */

/** @defgroup HostReg_Zero_Addr 零地址检测 */
/** @{ */
void    HostReg_RecordZeroAddrResponse(void);
uint8_t HostReg_GetZeroAddrPresent(void);
/** @} */

/** @defgroup HostReg_Conflict 地址重复检测 */
/** @{ */
void    HostReg_RecordError(void);
void    HostReg_StepCycle(void);
uint8_t HostReg_GetAddrConflict(void);
/** @} */

/** @defgroup HostReg_Online 在线检测 */
/** @{ */
uint8_t HostReg_IsOnline(uint8_t addr);
/** @} */

/** @defgroup HostReg_Partial 局部位写入（精简轮询） */
/** @{ */
void HostReg_StorePartialBits(uint8_t addr, uint16_t reg_addr,
                              uint16_t bit_count, const uint8_t *data);
/** @} */

/** @defgroup HostReg_Alarm 报警位快速打包 */
/** @{ */
uint64_t HostReg_GetAlarmBits64(uint8_t addr);
/** @} */

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __HOSTBOARD_REGISTERS_H__ */
