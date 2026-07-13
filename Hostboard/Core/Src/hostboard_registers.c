/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : hostboard_registers.c
  * @brief          : 128 路 Controlboard 线圈数据存储 + 检测逻辑
  *
  * 存储由接收任务解析的 FC 0x02 响应数据，提供查询 API。
  * 维护零地址检测（三轮 OR）和地址重复检测（三轮错误计数累加 > 2）。
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "hostboard_registers.h"
#include <string.h>

/* USER CODE BEGIN 0 */

/* ==================== 零地址检测 ==================== */

/**
 * @brief  零地址检测状态
 *
 * 每轮完整扫描 (0-128) 中: zero_addr_count_this_cycle 累计收到 addr0 有效响应的次数
 * StepCycle() 时移入 zero_addr_history[3] 环形缓冲
 * 三轮累加 > 2 → zero_addr_present = 1
 * （与重复地址探测机制一致，避免单次瞬态误触发）
 */
static uint32_t zero_addr_count_this_cycle;          /* 本轮零地址响应计数 */
static uint32_t zero_addr_history[3];                /* 三轮历史 */
static uint8_t  zero_addr_present;                   /* 综合结果 */

/* ==================== 重复地址检测 ==================== */

/**
 * @brief  地址重复检测状态
 *
 * 每轮扫描中: crc_error_count 累计非超时错误
 * StepCycle() 时移入 crc_error_history[3] 环形缓冲
 * 三轮累加 > 2 → addr_conflict_flag = 1
 */
static uint32_t crc_error_count;                 /* 本轮错误计数 */
static uint32_t crc_error_history[3];            /* 三轮历史 */
static uint8_t  addr_conflict_flag;              /* 综合结果 */

/* ==================== 轮次跟踪 ==================== */

static uint32_t current_cycle;                   /* 递增周期计数器，用于在线检测 */

/* ==================== 线圈数据存储 ==================== */

/**
 * ctrl_boards[0] 预留（地址 0 有效响应不计入存储）
 * ctrl_boards[1..128] 对应 Controlboard 地址 1-128
 */
static CtrlBoardEntry_t ctrl_boards[MAX_CTRLBD_ADDR + 1];

/* USER CODE END 0 */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* USER CODE BEGIN 1 */

/* ==================== 数据存储/读取 API ==================== */

/**
 * @brief  存储 Controlboard 的线圈数据
 * @param  addr  Controlboard 地址 (1-128)
 * @param  data  线圈数据指针
 * @param  len   数据长度（上限 COIL_BYTE_COUNT）
 */
void HostReg_StoreCoilData(uint8_t addr, const uint8_t *data, uint8_t len)
{
    if (addr < 1 || addr > MAX_CTRLBD_ADDR)
        return;
    if (len > COIL_BYTE_COUNT)
        len = COIL_BYTE_COUNT;
    memcpy(ctrl_boards[addr].coil_data, data, len);

    /* 收到有效响应 → 更新在线标志和周期 */
    ctrl_boards[addr].online = 1;
    ctrl_boards[addr].last_seen_cycle = current_cycle;
}

/**
 * @brief  读取 Controlboard 线圈数据的一个字节
 * @param  addr       Controlboard 地址 (1-128)
 * @param  byte_idx   字节索引 (0-16)
 * @return 线圈数据字节，越界返回 0
 */
uint8_t HostReg_GetCoilByte(uint8_t addr, uint8_t byte_idx)
{
    if (addr < 1 || addr > MAX_CTRLBD_ADDR)
        return 0;
    if (byte_idx >= COIL_BYTE_COUNT)
        return 0;
    return ctrl_boards[addr].coil_data[byte_idx];
}

/**
 * @brief  读取 Controlboard 线圈数据的一个位
 * @param  addr       Controlboard 地址 (1-128)
 * @param  bit_idx    位索引 (0-129)
 * @return 线圈位值 (0/1)，越界返回 0
 */
uint8_t HostReg_GetCoilBit(uint8_t addr, uint16_t bit_idx)
{
    if (addr < 1 || addr > MAX_CTRLBD_ADDR)
        return 0;
    if (bit_idx >= COIL_BYTE_COUNT * 8)
        return 0;
    return (ctrl_boards[addr].coil_data[bit_idx / 8] >> (bit_idx % 8)) & 1;
}

/* ==================== 零地址检测 API ==================== */

/**
 * @brief  记录收到地址 0 的 CRC 有效响应
 */
void HostReg_RecordZeroAddrResponse(void)
{
    zero_addr_count_this_cycle++;
}

/**
 * @brief  查询零地址是否存在
 * @retval 1 存在 Controlboard 误设为地址 0
 */
uint8_t HostReg_GetZeroAddrPresent(void)
{
    return zero_addr_present;
}

/* ==================== 重复地址检测 API ==================== */

/**
 * @brief  记录一次非超时错误（CRC 失败/帧异常）
 */
void HostReg_RecordError(void)
{
    crc_error_count++;
}

/**
 * @brief  查询是否存在地址重复
 * @retval 1 存在地址重复
 */
uint8_t HostReg_GetAddrConflict(void)
{
    return addr_conflict_flag;
}

/* ==================== 在线检测 API ==================== */

/**
 * @brief  查询 Controlboard 在线状态
 * @param  addr  Controlboard 地址 (1-128)
 * @retval 1 在线，0 离线或地址越界
 */
uint8_t HostReg_IsOnline(uint8_t addr)
{
    if (addr < 1 || addr > MAX_CTRLBD_ADDR)
        return 0;
    return ctrl_boards[addr].online;
}

/* ==================== 局部位写入（精简轮询） ==================== */

/**
 * @brief  将响应数据按位写入 coil_data 的指定偏移
 * @param  addr       Controlboard 地址 (1-128)
 * @param  reg_addr   位起始地址（如 126）
 * @param  bit_count  位数（如 3）
 * @param  data       响应数据指针（大端序填充）
 *
 * 用于精简轮询场景：只读取 3 bits (126-128)，收到 1 字节后
 * 按位复制到 ctrl_boards[addr].coil_data 的正确位置。
 */
void HostReg_StorePartialBits(uint8_t addr, uint16_t reg_addr,
                              uint16_t bit_count, const uint8_t *data)
{
    if (addr < 1 || addr > MAX_CTRLBD_ADDR)
        return;

    for (uint16_t i = 0; i < bit_count; i++)
    {
        uint8_t  bit_val  = (data[i / 8] >> (i % 8)) & 1;
        uint16_t dst_bit  = reg_addr + i;
        uint8_t  byte_off = dst_bit / 8;
        uint8_t  bit_off  = dst_bit % 8;

        if (byte_off >= COIL_BYTE_COUNT)
            break;

        ctrl_boards[addr].coil_data[byte_off] &= ~(1 << bit_off);
        ctrl_boards[addr].coil_data[byte_off] |= (bit_val << bit_off);
    }

    /* 收到有效响应 → 更新在线标志 */
    ctrl_boards[addr].online = 1;
    ctrl_boards[addr].last_seen_cycle = current_cycle;
}

/* ==================== 周期更新 ==================== */

/**
 * @brief  每轮完整扫描结束时调用
 *
 * 1. 将本轮 CRC 错误数存入历史环形缓冲，清零计数
 * 2. 三轮累加 > 2 → addr_conflict_flag = 1
 * 3. 将本轮零地址发现标志移入历史，清零
 * 4. zero_addr_present = 三轮 OR
 */
void HostReg_StepCycle(void)
{
    current_cycle++;

    /* ---- 在线检测：检查所有 Controlboard (1-128) ---- */
    for (uint16_t addr = 1; addr <= MAX_CTRLBD_ADDR; addr++)
    {
        if (ctrl_boards[addr].last_seen_cycle + 3 <= current_cycle)
        {
            ctrl_boards[addr].online = 0;  /* 连续三轮无响应 → 离线 */
        }
    }

    /* ---- 重复地址检测：三轮错误计数累加 > 2 ---- */
    crc_error_history[(current_cycle - 1) % 3] = crc_error_count;
    crc_error_count = 0;

    uint32_t sum = crc_error_history[0] + crc_error_history[1] + crc_error_history[2];
    addr_conflict_flag = (sum > 2) ? 1 : 0;

    /* ---- 零地址检测：三轮累加 > 2 ---- */
    zero_addr_history[(current_cycle - 1) % 3] = zero_addr_count_this_cycle;
    zero_addr_count_this_cycle = 0;
    zero_addr_present = (zero_addr_history[0]
                      +  zero_addr_history[1]
                      +  zero_addr_history[2]) > 2;
}

/* ==================== 报警位快速打包 API ==================== */

/**
 * @brief  将控制器的报警相关位打包为 uint64_t
 * @param  addr  Controlboard 地址 (1-128)
 * @retval 64 位报警位图
 *         bit  0 = coil_data bit 129 (烟雾报警, sensorIdx 0)
 *         bit  1 = coil_data bit 63  (传感器 1 报警)
 *         bit  2 = coil_data bit 64  (传感器 2 报警)
 *         ... 以此类推 ...
 *         bit 63 = coil_data bit 125 (传感器 63 报警)
 * @note   __builtin_ctzll() 可直接得到 sensorIdx，无需映射
 */
uint64_t HostReg_GetAlarmBits64(uint8_t addr)
{
    if (addr < 1 || addr > MAX_CTRLBD_ADDR)
        return 0;

    const uint8_t *cd = ctrl_boards[addr].coil_data;
    uint64_t alarms = 0;

    /* 烟雾报警 bit 129 → bit 0 */
    if (cd[16] & 0x02)
        alarms |= 1ULL;

    /* 传感器 1-63 报警 bits 63-125 → bits 1-63 */
    alarms |= (uint64_t)(cd[7] >> 7)         << 1;
    alarms |= (uint64_t)(cd[8])              << 2;
    alarms |= (uint64_t)(cd[9])              << 10;
    alarms |= (uint64_t)(cd[10])             << 18;
    alarms |= (uint64_t)(cd[11])             << 26;
    alarms |= (uint64_t)(cd[12])             << 34;
    alarms |= (uint64_t)(cd[13])             << 42;
    alarms |= (uint64_t)(cd[14])             << 50;
    alarms |= (uint64_t)(cd[15] & 0x3F)      << 58;

    return alarms;
}

/* USER CODE END 1 */
