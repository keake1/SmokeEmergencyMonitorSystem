/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : uart1_modbus.h
  * @brief          : Modbus RTU master driver — 发送队列 + ISR 接收
  *
  * 架构：
  *   应用请求入发送队列 → 发送任务发帧 → 传感器回复 → IDLE 中断收帧
  *   → 原始队列 → 接收任务 CRC 校验 + 解析 → 存入寄存器 (modbus_registers)
  *
  *   不再有"已解析队列"，接收任务将数据直接写入寄存器映射表。
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __UART1_MODBUS_H__
#define __UART1_MODBUS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported constants --------------------------------------------------------*/

/** @defgroup Modbus_Function_Codes 功能码 */
/** @{ */
#define MODBUS_FUNC_READ_HOLDING_REGISTERS    0x03
#define MODBUS_FUNC_WRITE_SINGLE_REGISTER     0x06
/** @} */

/** @defgroup Modbus_Return_Codes 返回值 */
/** @{ */
#define MODBUS_OK              0
#define MODBUS_ERR_TMO        (-1)
#define MODBUS_ERR_CRC        (-2)
#define MODBUS_ERR_EXCEPTION  (-4)
/** @} */

/** @defgroup Sensor_Model_Bytes 传感器型号字节 */
/** @{ */
#define SENSOR_MODEL_CO       0x01
#define SENSOR_MODEL_WIND     0x02  /* 预留 */
#define SENSOR_MODEL_PRESSURE 0x03
#define SENSOR_MODEL_7IN1     0x04
#define SENSOR_MODEL_TH       0x05  /* 温湿度传感器 */
#define SENSOR_MODEL_CO2      0x06  /* CO2 传感器 */
/** @} */

/** @defgroup Buffer_Sizes */
/** @{ */
#define MODBUS_RX_BUF_SIZE         64
#define MODBUS_SEND_QUEUE_LEN      8
#define MODBUS_RAW_RX_QUEUE_LEN    4
/** @} */

/* USER CODE BEGIN EConst */

/* USER CODE END EConst */

/* Exported types ------------------------------------------------------------*/

/**
  * @brief 发送队列项
  */
typedef struct {
    uint8_t  slave_addr;   /* 从机地址 1~247 */
    uint8_t  func_code;    /* 0x03 / 0x06 */
    uint16_t reg_addr;     /* 寄存器地址 */
    uint16_t reg_value;    /* 读：寄存器数量 / 写：写入值 */
} ModbusRequest_t;

/**
  * @brief 原始响应帧（ISR → 接收任务）
  */
typedef struct {
    uint8_t  frame[MODBUS_RX_BUF_SIZE];
    uint16_t length;
} ModbusResponse_t;

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported queue/semaphore handles -----------------------------------------*/

extern QueueHandle_t      xModbusSendQueue;     /* 发送队列 */
extern SemaphoreHandle_t  xModbusTxSemaphore;   /* 事务完成信号量 */
extern SemaphoreHandle_t  xModbusTxCompleteSem; /* USART1 TX 完成信号量 */

/* Exported functions prototypes ---------------------------------------------*/

void     ModbusMaster_InitQueues(void);
void     ModbusMaster_StartRx(void);
void     ModbusMaster_ResetRx(void);
int      ModbusMaster_EnqueueRequest(const ModbusRequest_t *req);
void     ModbusMaster_RxByteHandler(uint8_t data);
void     ModbusMaster_RxIdleHandler(void);
uint16_t Modbus_CRC16(const uint8_t *data, uint16_t len);

/* 供接收任务调用 */
int ModbusMaster_DequeueRawFrame(ModbusResponse_t *raw, TickType_t timeout);

/* 同步入队封装（仅塞队列，不等待） */
int ModbusMaster_ReadRegisters(UART_HandleTypeDef *huart,
                               uint8_t slave, uint16_t reg_addr, uint16_t reg_cnt,
                               uint8_t *rx_buf, uint16_t *rx_len, uint32_t timeout);
int ModbusMaster_WriteRegister(UART_HandleTypeDef *huart,
                               uint8_t slave, uint16_t reg_addr, uint16_t value,
                               uint32_t timeout);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __UART1_MODBUS_H__ */
