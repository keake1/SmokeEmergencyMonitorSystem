/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : modbus_registers.c
  * @brief          : Modbus 寄存器映射实现
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "modbus_registers.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

/* USER CODE BEGIN 0 */

/* ==================== 线圈寄存器（位映射） ====================
 * 用两个 64 位整数分别存储在线状态和报警标志：
 *   在线: bit n   ← 传感器地址 n+1
 *   报警: bit n+63
 */
static uint64_t coil_online = 0;
static uint64_t coil_alarm  = 0;

/* ==================== 保持寄存器 ====================
 * 类型: 地址 1-63
 * 数据: 地址 64-504 (63 传感器 × 7 寄存器)
 * 为了方便访问, 类型数组用 [1..63], 数据数组用 [1..63][0..6]
 */
static uint8_t  reg_type[MODBUS_MAX_SLAVES + 1];                     /* [1..63] */
static uint16_t reg_data[MODBUS_MAX_SLAVES + 1][MODBUS_DATA_REGS_PER_SENSOR]; /* [1..63][0..6] */

/* 零地址出现次数统计（累计，供调试查看） */
static uint32_t zero_addr_count = 0;

/* 零地址检测：每轮计数 + 三轮历史（与地址重复检测机制一致） */
static uint32_t zero_addr_cycle_count;          /* 本轮零地址响应次数 */
static uint32_t zero_addr_cycle_hist[3];         /* 三轮历史 */
static uint8_t  zero_addr_present;               /* 综合结果：三轮累加 > 2 */

/* ==================== 离线检测 ====================
 * 每个地址 (0-63) 在哪个周期最后收到过响应。
 * 若连续三轮完整扫描(StepCycle)都没收到某地址的响应，
 * 则自动将其标记为离线。
 */
static uint32_t current_cycle = 0;
static uint32_t last_seen_cycle[MODBUS_MAX_SLAVES + 1] = {0};
static uint8_t  zero_addr_present = 0;    /* true: 0 地址传感器最近有过响应 */

/* ==================== 地址重复检测 ====================
 * 统计每轮 CRC 错误次数，三轮累计错误 > 2 次
 * 视为存在地址重复（多个从机争抢同一地址导致帧冲突）。
 */
#define ADDR_CONFLICT_CYCLES        3   /* 检测窗口：连续三轮 */
#define ADDR_CONFLICT_ERR_THRESHOLD 2   /* 三轮累计错误阈值 */

static uint8_t  crc_error_count = 0;                              /* 当前周期累计 CRC 错误数 */
static uint8_t  crc_error_history[ADDR_CONFLICT_CYCLES] = {0};    /* 过去三轮各自错误数 */
static uint8_t  crc_error_hist_idx = 0;                           /* 环形缓冲区写入索引 */
static uint8_t  addr_conflict_flag = 0;                           /* 地址重复标志位 */

/* ==================== 控板 DIP 地址 ==================== */
static uint8_t  board_hw_addr = 0;                                /* ID1-ID8 读取的硬件地址 */

/* ==================== 全局报警标志（线圈位 128） ==================== */
static uint8_t  global_alarm_flag = 0;                            /* 由 DWIN 状态帧更新时同步 */

/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* 检查从机地址是否有效 (1-63) */
static inline uint8_t addr_valid(uint8_t slave)
{
    return (slave >= 1 && slave <= MODBUS_MAX_SLAVES);
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* ==================== 线圈操作 ==================== */

void ModbusReg_SetOnline(uint8_t slave, uint8_t online)
{
    if (!addr_valid(slave)) return;
    taskENTER_CRITICAL();
    if (online)
        coil_online |=  (1ULL << (slave - 1));
    else
        coil_online &= ~(1ULL << (slave - 1));
    taskEXIT_CRITICAL();
}

uint8_t ModbusReg_GetOnline(uint8_t slave)
{
    if (!addr_valid(slave)) return 0;
    uint8_t bit;
    taskENTER_CRITICAL();
    bit = (coil_online >> (slave - 1)) & 1;
    taskEXIT_CRITICAL();
    return bit;
}

void ModbusReg_SetAlarm(uint8_t slave, uint8_t alarm)
{
    if (!addr_valid(slave)) return;
    taskENTER_CRITICAL();
    if (alarm)
        coil_alarm |=  (1ULL << (slave - 1));
    else
        coil_alarm &= ~(1ULL << (slave - 1));
    taskEXIT_CRITICAL();
}

uint8_t ModbusReg_GetAlarm(uint8_t slave)
{
    if (!addr_valid(slave)) return 0;
    uint8_t bit;
    taskENTER_CRITICAL();
    bit = (coil_alarm >> (slave - 1)) & 1;
    taskEXIT_CRITICAL();
    return bit;
}

/* ==================== 保持寄存器操作 ==================== */

void ModbusReg_SetType(uint8_t slave, uint8_t type)
{
    if (!addr_valid(slave)) return;
    reg_type[slave] = type;
}

uint8_t ModbusReg_GetType(uint8_t slave)
{
    if (!addr_valid(slave)) return 0;
    return reg_type[slave];
}

void ModbusReg_SetData(uint8_t slave, uint8_t index, uint16_t value)
{
    if (!addr_valid(slave)) return;
    if (index >= MODBUS_DATA_REGS_PER_SENSOR) return;
    reg_data[slave][index] = value;
}

uint16_t ModbusReg_GetData(uint8_t slave, uint8_t index)
{
    if (!addr_valid(slave)) return 0;
    if (index >= MODBUS_DATA_REGS_PER_SENSOR) return 0;
    return reg_data[slave][index];
}

/* ==================== 批量读取（按 Modbus 地址直接索引） ==================== */

/**
  * @brief  按保持寄存器地址读取值
  * @param  reg_addr  寄存器地址 (1-505)
  * @retval 寄存器值，地址越界返回 0
  */
uint16_t ModbusReg_ReadHolding(uint16_t reg_addr)
{
    if (reg_addr >= REG_ADDR_TYPE_START && reg_addr <= REG_ADDR_TYPE_END)
    {
        /* 类型区：地址 1-63 */
        return reg_type[reg_addr];
    }
    else if (reg_addr >= REG_ADDR_DATA_START && reg_addr <= REG_ADDR_DATA_END)
    {
        /* 数据区：地址 64-504 */
        uint16_t offset = reg_addr - REG_ADDR_DATA_START;            /* 0-440 */
        uint8_t slave   = (offset / MODBUS_DATA_REGS_PER_SENSOR) + 1; /* 1-63 */
        uint8_t index   = offset % MODBUS_DATA_REGS_PER_SENSOR;       /* 0-6 */
        return reg_data[slave][index];
    }
    else if (reg_addr == REG_ADDR_BOARD_ADDR)
    {
        /* 地址 505：控板硬件 DIP 地址 */
        return board_hw_addr;
    }
    return 0;
}

/**
  * @brief  按线圈地址读取位值
  * @param  coil_addr  线圈地址 (0-128)
  * @retval 0 或 1，越界返回 0
  */
uint8_t ModbusReg_ReadCoil(uint16_t coil_addr)
{
    if (coil_addr >= COIL_OFFSET_ONLINE && coil_addr < COIL_OFFSET_ONLINE + MODBUS_MAX_SLAVES)
    {
        /* 位 0-62: 在线状态 */
        return (coil_online >> (coil_addr - COIL_OFFSET_ONLINE)) & 1;
    }
    else if (coil_addr >= COIL_OFFSET_ALARM && coil_addr < COIL_OFFSET_ALARM + MODBUS_MAX_SLAVES)
    {
        /* 位 63-125: 报警标志 */
        return (coil_alarm >> (coil_addr - COIL_OFFSET_ALARM)) & 1;
    }
    else if (coil_addr == COIL_OFFSET_SYSTEM + 0)
    {
        /* 位 126: 零地址传感器存在 */
        return zero_addr_present;
    }
    else if (coil_addr == COIL_OFFSET_SYSTEM + 1)
    {
        /* 位 127: 地址重复 */
        return addr_conflict_flag;
    }
    else if (coil_addr == COIL_GLOBAL_ALARM)
    {
        /* 位 128: 全局报警 */
        return global_alarm_flag;
    }
    return 0;
}

/* ==================== 零地址检测 ==================== */

void ModbusReg_ResetZeroAddrCount(void)
{
    zero_addr_count = 0;
}

uint32_t ModbusReg_GetZeroAddrCount(void)
{
    return zero_addr_count;
}

/* 内部函数：零地址计数递增 （供接收任务调用） */
void ModbusReg_IncrementZeroAddr(void)
{
    zero_addr_count++;
}

/* ==================== 离线检测 ==================== */

/**
  * @brief  记录某地址收到响应
  * @note   在接收任务 CRC 校验通过后调用。
  *         同时负责将地址 1-63 的传感器标记为在线，
  *         以及更新 0 地址传感器存在标志。
  * @param  slave  收到响应的从机地址（0-63）
  */
void ModbusReg_RecordResponse(uint8_t slave)
{
    if (slave <= MODBUS_MAX_SLAVES)
    {
        /* 更新此地址最近有响应的周期号 */
        last_seen_cycle[slave] = current_cycle;
    }
    if (slave >= 1 && slave <= MODBUS_MAX_SLAVES)
    {
        /* 有效传感器 → 标记在线 */
        ModbusReg_SetOnline(slave, 1);
    }
    if (slave == 0)
    {
        /* 零地址有回复 → 累计本轮次数 */
        zero_addr_cycle_count++;
        zero_addr_count++;
    }
}

/**
  * @brief  推进一个完整扫描周期
  * @note   在轮询任务中，当 scan_addr 从 63 回绕到 0 时调用。
  *         检查每个传感器地址：若连续三轮周期没有响应，
  *         则自动标记离线；对 0 地址同样处理。
  *         同时检查 CRC 错误历史，若连续三轮每轮超过阈值，
  *         标记地址重复标志。
  */
void ModbusReg_StepCycle(void)
{
    current_cycle++;

    /* ---- 检查传感器 (1-63) ---- */
    for (uint8_t i = 1; i <= MODBUS_MAX_SLAVES; i++)
    {
        /* 连续三轮（含本轮）无响应 → 离线 */
        if (last_seen_cycle[i] + 3 <= current_cycle)
        {
            ModbusReg_SetOnline(i, 0);
        }
    }

    /* ---- 零地址检测：三轮累加 > 2 ---- */
    zero_addr_cycle_hist[(current_cycle - 1) % 3] = zero_addr_cycle_count;
    zero_addr_cycle_count = 0;
    zero_addr_present = (zero_addr_cycle_hist[0]
                      +  zero_addr_cycle_hist[1]
                      +  zero_addr_cycle_hist[2]) > 2;

    /* ---- 地址重复检测：记录本轮 CRC 错误数到环形历史 ---- */
    crc_error_history[crc_error_hist_idx] = crc_error_count;
    crc_error_hist_idx = (crc_error_hist_idx + 1) % ADDR_CONFLICT_CYCLES;
    crc_error_count = 0;

    /* 三轮累计错误数 > 2 → 地址重复标志 */
    addr_conflict_flag = (crc_error_history[0]
                       +  crc_error_history[1]
                       +  crc_error_history[2]) > ADDR_CONFLICT_ERR_THRESHOLD;
}

/**
  * @brief  查询 0 地址传感器当前是否存在
  * @retval 1  存在（最近三轮内有过响应）
  *         0  不存在（连续三轮无响应）
  */
uint8_t ModbusReg_GetZeroAddrPresent(void)
{
    return zero_addr_present;
}

/* ==================== 地址重复检测 ==================== */

/**
  * @brief  记录一次 CRC 校验错误
  * @note   在接收任务中 CRC 校验失败时调用。
  *         计数器在每轮 StepCycle 开头重置。
  */
void ModbusReg_RecordCrcError(void)
{
    crc_error_count++;
}

/**
  * @brief  查询是否存在地址重复
  * @retval 1  存在地址重复（连续三轮 CRC 错误数 > 2）
  *         0  无地址重复
  */
uint8_t ModbusReg_GetAddrConflict(void)
{
    return addr_conflict_flag;
}

/* ==================== 控板 DIP 地址 ==================== */

void ModbusReg_SetBoardAddr(uint8_t addr)
{
    board_hw_addr = addr;
}

uint8_t ModbusReg_GetBoardAddr(void)
{
    return board_hw_addr;
}

/* ==================== 全局报警标志（线圈位 128） ==================== */

void ModbusReg_SetGlobalAlarm(uint8_t alarm)
{
    global_alarm_flag = (alarm != 0) ? 1 : 0;
}

uint8_t ModbusReg_GetGlobalAlarmCoil(void)
{
    return global_alarm_flag;
}

/* USER CODE END 0 */
