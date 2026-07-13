#ifndef __DWIN_H
#define __DWIN_H

#include "main.h"
#include <stdint.h>

/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
/* USER CODE END Includes */

/* ============================================================
 *  NorFlash 存储布局
 * ============================================================
 *  备注（Tip）存储区：从 NorFlash 地址 0x000000 起
 *    每条备注占 DWIN_TIP_WORDS_PER_SLOT 个字（word = 2 字节）
 *    数据格式：[content_len(1B)] [content(最多32B)] [CRC16(2B)]
 *    共 35 字节，向上取偶数字对齐 = 18 word = 36 字节
 *
 *  地址计算：slot_index（0起）× DWIN_TIP_WORDS_PER_SLOT
 * ============================================================ */

/* NorFlash 操作寄存器地址（系统变量 0x0008） */
#define DWIN_NOR_CMD_ADDR           0x0008U

/* DWIN 变量区：写 NorFlash 前的数据中转缓冲区 */
#define DWIN_TIP_VAR_ADDR           0x1000U  
#define DWIN_TIP_WRITE_BUF_ADDR     0X3200U	 /* 备注写入暂存区（发往 NorFlash 前） */
#define DWIN_TIP_READ_BUF_ADDR      0x3220U  /* 备注读回暂存区（从 NorFlash 读回后） */

/* NorFlash 物理存储起始地址（字地址） */
#define DWIN_TIP_FLASH_BASE         0x000000UL

/* 每条备注占用的 word 数（必须为偶数）
 *   1B 长度 + 最多30B内容 + 2B CRC = 35B → 取偶数对齐 = 18 word */
#define DWIN_TIP_WORDS_PER_SLOT     18U
/* 每条备注数据区字节数 */
#define DWIN_TIP_BYTES_PER_SLOT     (DWIN_TIP_WORDS_PER_SLOT * 2U)  /* 36 字节 */
/* 内容最大字节数（去掉 1B 长度头 + 2B CRC） */
#define DWIN_TIP_CONTENT_MAX_LEN    30U

/* 备注槽位总数（根据实际迪文屏工程中备注变量数量填写） */
#define DWIN_TIP_SLOT_COUNT         128U

/* NorFlash 地址计算宏：由 slot 索引（0起）得到对应的 NorFlash 字地址 */
#define DWIN_TIP_FLASH_ADDR(slot)   (DWIN_TIP_FLASH_BASE + (uint32_t)(slot) * DWIN_TIP_WORDS_PER_SLOT)

/**
 * @brief 向迪文屏指定变量地址写入任意长度数据（0x82 指令）
 *        帧格式：5A A5 [LEN] 82 [AH] [AL] [D0]...[Dn]
 *        LEN = 3 + data_len
 * @param addr      目标变量起始地址
 * @param pData     待写入字节数组（调用方负责大端序）
 * @param data_len  数据字节数（最大 0xF9）
 */
void DWIN_WriteVar(uint16_t addr, const uint8_t *pData, uint8_t data_len);

/**
 * @brief 向迪文屏发送读变量请求（0x83 指令）
 *        帧格式：5A A5 04 83 [AH] [AL] [CNT]
 *        屏幕会返回：5A A5 [LEN] 83 [AH] [AL] [CNT] [DH] [DL]...
 * @param addr      起始变量地址（16位）
 * @param word_cnt  读取的 word 数量
 */
void DWIN_ReadVar(uint16_t addr, uint8_t word_cnt);

/**
 * @brief 将变量存储空间数据写入片内 NorFlash（0x08 寄存器，写操作 0xA5）
 *        帧格式：5A A5 0B 82 00 08 A5 [F2][F1][F0] [VH][VL] [LH][LL]
 * @param flash_addr  NorFlash 目标首地址（24位，必须为偶数）
 * @param var_addr    变量存储空间首地址（必须为偶数）
 * @param word_len    读写字长度（word 数，必须为偶数）
 */
void DWIN_NorFlashWrite(uint32_t flash_addr, uint16_t var_addr, uint16_t word_len);

/**
 * @brief 从片内 NorFlash 读取数据到变量存储空间（0x08 寄存器，读操作 0x5A）
 *        帧格式：5A A5 0B 82 00 08 5A [F2][F1][F0] [VH][VL] [LH][LL]
 * @note  发送后需等待 D7 自动清零（轮询或延时）再进行后续操作。
 * @param flash_addr  NorFlash 源首地址（24位，必须为偶数）
 * @param var_addr    变量存储空间目标首地址（必须为偶数）
 * @param word_len    读写字长度（word 数，必须为偶数）
 */
void DWIN_NorFlashRead(uint32_t flash_addr, uint16_t var_addr, uint16_t word_len);

/* 控制器状态图标变量区
 *   地址范围：0x1800 ~ 0x187F，共 128 个 word，下标 = 控制器地址 - 1
 *   图标值定义：
 *     0x0000 = 在线且报警（红色告警图标）
 *     0x0001 = 保留
 *     0x0002 = 离线（灰色图标）
 *     0x0003 = 在线且正常（绿色图标）*/
#define DWIN_CTRL_ICON_BASE_ADDR    0x1800U  /* 控制器状态图标区起始地址 */
#define DWIN_CTRL_ICON_COUNT        128U  /* 图标数量，对应地址 1~128 */

/* USER CODE BEGIN ET */

/**
 * @brief  迪文屏帧数据结构（发送/接收队列项）
 */
typedef struct {
    uint8_t  data[128];   /* 帧数据（含 0x5A 0xA5 帧头） */
    uint16_t length;       /* 帧长度（字节数） */
} DwinFrame_t;

/**
 * @brief  Hostboard 端 DWIN 屏解析状态（接收任务填充）
 */
typedef struct {
    /* RTC 时间（地址 0x0010 读回） */
    uint8_t rtcYear;
    uint8_t rtcMonth;
    uint8_t rtcDay;
    uint8_t rtcHour;
    uint8_t rtcMinute;
    uint8_t rtcSecond;
    uint8_t rtcReady;       /* 1 = RTC 已读回有效数据 */

    /* 屏幕选中的控制器地址（地址 0x4000 读回） */
    uint8_t selectAddr;
} DWIN_HostStatus_t;

/* USER CODE END ET */

#define DWIN_ICON_ALARM             0x0003U  /* 在线且报警 */
#define DWIN_ICON_TROUBLE           0x0002U  /* 在线且故障 */
#define DWIN_ICON_NORMAL            0x0001U  /* 在线且正常 */
#define DWIN_ICON_OFFLINE           0x0000U  /* 离线 */

/* 报警记录显示区
 *   起始地址 0x2000，每条记录占 4 个 word（8 字节）
 *   字节布局：[年(1)] [月(1)] [日(1)] [时(1)] [分(1)] [秒(1)] [控制器地址(1)] [保留(1)]
 *   最多 12 条，地址依次为 0x2000 / 0x2004 / ... / 0x202C，满后循环覆盖 */
#define DWIN_ALARM_BASE_ADDR        0x2000U  /* 报警记录区起始地址 */
#define DWIN_ALARM_RECORD_WORDS     4U       /* 每条记录占 4 个 word（8 字节）*/
#define DWIN_ALARM_MAX_RECORDS      12U      /* 最多保存 12 条 */

/* USER CODE BEGIN EConst */
#define DWIN_HOST_STATUS_ICON_ADDR  0x1881U  /* Hostboard 状态图标地址 */
#define DWIN_ICONS_PER_FRAME        32U       /* 每帧图标数（32 icons × 2 bytes = 64 bytes data）*/   
#define DWIN_SENSOR_STATUS_ICON_ADDR    0x1890U /* 传感器状态图标区*/
#define DWIN_SENSOR_DATA_ADDR       0x1900U  /* 传感器数据区（屏幕读回后存放处） */
#define SEWNSOR_NUM_PER_CTRL              64U       /* 每个控制器的传感器数量（对应图标数量）*/
#define DWIN_SELECT_CTRL_ADDR       0x3100U  /* 屏幕选中控制器地址 */

/* 详情界面常量 */
#define DWIN_DETAIL_WIN_ICON_ADDR      0x1882U  /* 窗状态（选中控制器） */
#define DWIN_DETAIL_GLB_ALARM_ADDR      0x1883U  /* 全局报警 */
#define DWIN_DETAIL_ZERO_ADDR_ADDR      0x1884U  /* 零地址存在 */
#define DWIN_DETAIL_CONF_ADDR_ADDR      0x1885U  /* 地址重复 */
#define DWIN_DETAIL_SENSOR_ICON_ADDR    0x1890U  /* 传感器图标起始（63 个 word） */
#define DWIN_DETAIL_SENSOR_DATA_ADDR    0x1900U  /* 传感器数据起始（每传感器 16 地址） */

/* ==================== USART3 迪文屏队列参数 ==================== */
#define DWIN_TX_FRAME_MAX   128
#define DWIN_RX_FRAME_MAX   128
#define DWIN_TX_QUEUE_LEN   8
#define DWIN_RX_QUEUE_LEN   8

/* USER CODE END EConst */

/**
 * @brief RTC 时间结构体（由迪文屏 RTC 读回解析后填充）
 */
typedef struct {
    uint8_t year;   /* 年（普通十进制，如 25 表示 2025 年，实际年份 = 2000 + year） */
    uint8_t month;  /* 月，01~12 */
    uint8_t day;    /* 日，01~31 */
    uint8_t week;   /* 星期（1~7，一般不用于显示）*/
    uint8_t hour;   /* 时，00~23 */
    uint8_t minute; /* 分，00~59 */
    uint8_t second; /* 秒，00~59 */
} DWIN_RTC_t;

/**
 * @brief 向迪文屏发送读取 RTC 时间的指令（0x83 指令，地址 0x0010，读 4 个 word）
 *
 * 发送帧：5A A5 04 83 00 10 04  共 7 字节
 * 屏幕应答（示例）：5A A5 0C 83 00 10 04 [YY] [MM] [DD] [WW] [HH] [mn] [SS] [00]
 *   第 8  字节（buf[7]）= 年
 *   第 9  字节（buf[8]）= 月
 *   第 10 字节（buf[9]）= 日
 *   第 11 字节（buf[10]）= 星期（忽略）
 *   第 12 字节（buf[11]）= 时
 *   第 13 字节（buf[12]）= 分
 *   第 14 字节（buf[13]）= 秒
 *   第 15 字节（buf[14]）= 无意义
 *
 * @note 调用后需等待 USART3_RxTask 解析到 0x83 + 地址 0x0010 的应答帧，
 *       再调用 DWIN_ParseRTC() 解析时间数据。
 */
void DWIN_ReadRTC(void);

/**
 * @brief 向迪文屏报警记录区写入一条报警记录（循环覆盖）
 *
 * 记录格式（8 字节，大端）：
 *   [年高字节(1)] [年低字节(1)] [月(1)] [日(1)] [时(1)] [分(1)] [控制器地址(1)] [传感器槽位(1)]
 *   年份 = 2000 + rtc->year（普通十进制值，如 25 → 2025），以 uint16_t 大端序存入前两字节
 *   例：rtc->year=25 → full_year=2025 → buf[0]=0x07, buf[1]=0xD9
 *   传感器槽位：控制器侧 g_sensor[] 的下标（0~63），0xFF 表示未知
 *
 * 最多保存 DWIN_ALARM_MAX_RECORDS（12）条，写满后从第 0 条开始覆盖。
 * 写入位置索引由函数内部静态变量维护。
 *
 * @param rtc        报警发生时的 RTC 时间
 * @param ctrlAddr   触发报警的控制器地址（1~128）
 * @param sensorIdx  报警传感器在控制器侧的槽位索引（0~63），未知时传 0xFF
 */
void DWIN_WriteAlarmRecord(const DWIN_RTC_t *rtc, uint8_t ctrlAddr, uint8_t sensorIdx);

/**
 * @brief 计算字节数组的 CRC16（多项式 0x8005，初值 0xFFFF）
 * @param pData  数据指针
 * @param len    数据字节数
 * @retval CRC16 校验值
 */
uint16_t DWIN_CalcCRC16(const uint8_t *pData, uint16_t len);

/* USER CODE BEGIN EFP */

/* ==================== USART3 队列与信号量 ==================== */
extern QueueHandle_t     xDwinTxQueue;         /* 发送队列（应用 → TX 任务） */
extern QueueHandle_t     xDwinRxQueue;         /* 接收队列（ISR → RX 任务） */
extern SemaphoreHandle_t xDwinTxCompleteSem;   /* USART3 TX 完成信号量 */

/* ==================== 驱动 API ==================== */
void UART3_Send(const uint8_t *buf, uint16_t len);
void DwinRxByteHandler(uint8_t data);
void DwinRxReset(void);
void Dwin_InitQueues(void);

/* ==================== 读应答解析状态 ==================== */
extern DWIN_HostStatus_t g_hostDwinStatus;

/* USER CODE END EFP */

#endif /* __DWIN_H */
