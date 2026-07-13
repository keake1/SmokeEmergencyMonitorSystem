/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : modbus_registers.h
  * @brief          : Modbus 寄存器映射 — 存储传感器在线/报警/类型/数据
  *
  * 寄存器布局（控板内存中维护）：
  *
  * 【线圈寄存器 —— 位操作】
  *   位 0-62:    传感器地址 1-63 在线状态 (1=在线, 0=离线)
  *   位 63-125:  传感器地址 1-63 报警标志 (1=报警, 0=正常)
  *   位 126:     零地址传感器存在 (1=存在)
  *   位 127:     地址重复 (1=重复)
  *
  * 【保持寄存器 —— 16 位操作】
  *   地址 1-63:      传感器 1-63 的类型字节 (0x01/0x02/0x03/0x04)
  *   地址 64-70:     传感器 1 数据 (7 个寄存器)
  *   地址 71-77:     传感器 2 数据
  *   ...
  *   地址 64+(n-1)*7 ~ 64+(n-1)*7+6: 传感器 n 数据
  *
  *   对于简单传感器 (CO/风压/余压):
  *     [0] = 数值, [1..6] = 0
  *   对于 7 合 1 传感器:
  *     [0]=eCO₂, [1]=eCH₂O, [2]=TVOC,
  *     [3]=PM2.5, [4]=PM10, [5]=温度, [6]=湿度
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MODBUS_REGISTERS_H__
#define __MODBUS_REGISTERS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported constants --------------------------------------------------------*/

/** @defgroup Register_Layout 寄存器布局常量 */
/** @{ */
#define MODBUS_MAX_SLAVES             63
#define MODBUS_DATA_REGS_PER_SENSOR    7   /* 每个传感器占用 7 个数据寄存器 */

#define REG_ADDR_TYPE_START     1
#define REG_ADDR_TYPE_END       MODBUS_MAX_SLAVES
#define REG_ADDR_DATA_START     (REG_ADDR_TYPE_END + 1)                     /* = 64 */
#define REG_ADDR_DATA_END       (REG_ADDR_DATA_START +                     \
                                 MODBUS_MAX_SLAVES *                       \
                                 MODBUS_DATA_REGS_PER_SENSOR - 1)          /* = 504 */

#define REG_ADDR_BOARD_ADDR     (REG_ADDR_DATA_END + 1)                   /* = 505 */

#define COIL_OFFSET_ONLINE      0      /* 位 0-62:   在线状态 */
#define COIL_OFFSET_ALARM       63     /* 位 63-125: 报警标志 */
#define COIL_OFFSET_SYSTEM      126    /* 位 126: 零地址存在 / 位 127: 地址重复 */
#define COIL_GLOBAL_ALARM       128    /* 位 128: 全局报警（任一传感器报警=1） */
#define COIL_SMOKE_ALARM        129    /* 位 129: 烟雾报警器报警标志（Isolator 引脚输入） */
/** @} */

/* USER CODE BEGIN EConst */

/* USER CODE END EConst */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported functions prototypes ---------------------------------------------*/

/** @defgroup Coil_Operations 线圈操作（在线/报警） */
/** @{ */
void    ModbusReg_SetOnline(uint8_t slave, uint8_t online);
uint8_t ModbusReg_GetOnline(uint8_t slave);
void    ModbusReg_SetAlarm(uint8_t slave, uint8_t alarm);
uint8_t ModbusReg_GetAlarm(uint8_t slave);
/** @} */

/** @defgroup Holding_Register_Operations 保持寄存器操作（类型/数据） */
/** @{ */
void     ModbusReg_SetType(uint8_t slave, uint8_t type);
uint8_t  ModbusReg_GetType(uint8_t slave);
void     ModbusReg_SetData(uint8_t slave, uint8_t index, uint16_t value);
uint16_t ModbusReg_GetData(uint8_t slave, uint8_t index);
/** @} */

/** @defgroup Bulk_Read 批量读取（供外部 Modbus 从站协议栈使用） */
/** @{ */
uint16_t ModbusReg_ReadHolding(uint16_t reg_addr);
uint8_t  ModbusReg_ReadCoil(uint16_t coil_addr);
/** @} */

/** @defgroup Board_Address 控板 DIP 地址 */
/** @{ */
void     ModbusReg_SetBoardAddr(uint8_t addr);
uint8_t  ModbusReg_GetBoardAddr(void);
/** @} */

/** @defgroup Zero_Address 零地址检测 */
/** @{ */
void     ModbusReg_ResetZeroAddrCount(void);
uint32_t ModbusReg_GetZeroAddrCount(void);
void     ModbusReg_IncrementZeroAddr(void);
/** @} */

/** @defgroup Offline_Detection 离线检测 — 连续三轮无响应自动标记离线 */
/** @{ */
void     ModbusReg_RecordResponse(uint8_t slave);   /* 收到传感器响应时调用 */
void     ModbusReg_StepCycle(void);                  /* 完整扫描周期完成时调用 */
uint8_t  ModbusReg_GetZeroAddrPresent(void);         /* 查询 0 地址传感器当前是否存在 */
/** @} */

/** @defgroup Addr_Conflict 地址重复检测 — 连续三轮 CRC 错误超阈值自动标记 */
/** @{ */
void     ModbusReg_RecordCrcError(void);             /* CRC 校验失败时调用 */
uint8_t  ModbusReg_GetAddrConflict(void);            /* 查询是否存在地址重复 */
/** @} */

/** @defgroup Global_Alarm 全局报警标志（线圈位 128） */
/** @{ */
void     ModbusReg_SetGlobalAlarm(uint8_t alarm);    /* DWIN 状态帧更新时同步写入 */
uint8_t  ModbusReg_GetGlobalAlarmCoil(void);         /* 查询全局报警标志 */
/** @} */

/** @defgroup Smoke_Alarm 烟雾报警器标志（线圈位 129，Isolator 引脚输入） */
/** @{ */
void     ModbusReg_SetSmokeAlarm(uint8_t alarm);     /* TaskDwinIcons 读取 Isolator 引脚后同步写入 */
uint8_t  ModbusReg_GetSmokeAlarm(void);              /* 查询烟雾报警器报警状态 */
/** @} */

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __MODBUS_REGISTERS_H__ */
