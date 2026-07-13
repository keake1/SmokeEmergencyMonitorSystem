#include "dwin.h"
#include <string.h>

/* USER CODE BEGIN 0 */

/* ==================== 队列与信号量定义 ==================== */
QueueHandle_t     xDwinTxQueue         = NULL;
QueueHandle_t     xDwinRxQueue         = NULL;
SemaphoreHandle_t xDwinTxCompleteSem   = NULL;

/* ==================== USART3 RX 状态机组装 ==================== */
#define DWIN_RX_BUF_SIZE  128

/* 状态机状态 */
#define DWIN_RX_WAIT_5A   0   /* 等待帧头 0x5A */
#define DWIN_RX_WAIT_A5   1   /* 已收 0x5A，等待 0xA5 */
#define DWIN_RX_WAIT_LEN  2   /* 已收帧头，等待 LEN 字节 */
#define DWIN_RX_DATA      3   /* 正在收集帧数据 */

static volatile uint8_t  dwin_rx_state;        /* 状态机当前状态 */
static volatile uint8_t  dwin_rx_buf[DWIN_RX_BUF_SIZE];  /* 接收缓冲区 */
static volatile uint16_t dwin_rx_idx;          /* 缓冲区当前写入位置 */
static volatile uint16_t dwin_rx_expected;     /* 预期总长度（含帧头+LEN） */

/* 报警记录环形写入索引（DWIN_WriteAlarmRecord 使用） */
static uint8_t s_alarmIdx = 0;

/* USER CODE END 0 */

/**
 * @brief 向迪文屏指定变量地址写入任意长度数据（0x82 指令）
 *
 * 帧格式：
 *   5A A5 [LEN] 82 [AH] [AL] [D0] [D1] ... [Dn]
 *   LEN = 3 + data_len（指令码1 + 地址2 + 数据n）
 *
 * 示例：
 *   写 1 个 word(2字节) → 5A A5 05 82 10 00 00 02
 *   写 2 个 word(4字节) → 5A A5 07 82 10 01 00 0A 00 0B
 *
 * @param addr      目标变量起始地址
 * @param pData     待写入的原始字节数据（调用方负责大端序）
 * @param data_len  数据字节数（最大 0xF9，即 LEN 上限 0xFF - 3 固定字节）
 */
void DWIN_WriteVar(uint16_t addr, const uint8_t *pData, uint8_t data_len)
{
    /* 当前项目中最大数据长度为 36 字节 (DWIN_TIP_BYTES_PER_SLOT)，
     * 将其限制为 64 字节非常安全，并可节省约 200 字节的任务栈空间 */
    if (pData == NULL || data_len == 0 || data_len > 64)
    {
        return;
    }

    /* 总帧长 = 帧头(2) + LEN字段(1) + LEN内容(3+data_len) */
    uint8_t buf[3 + 1 + 2 + 64];          /* 优化：减小最大帧缓冲限制以防止栈溢出 */
    buf[0] = 0x5A;
    buf[1] = 0xA5;
    buf[2] = 3 + data_len;                  /* LEN */
    buf[3] = 0x82;                          /* 写变量指令 */
    buf[4] = (uint8_t)(addr >> 8);          /* 地址高字节 */
    buf[5] = (uint8_t)(addr & 0xFF);        /* 地址低字节 */
    memcpy(&buf[6], pData, data_len);       /* 数据段 */

    UART3_Send(buf, 6 + data_len);
}

/**
 * @brief 向迪文屏发送读变量请求（0x83 指令）
 *
 * 发送帧格式（共 7 字节）：
 *   5A A5 04 83 [AH] [AL] [CNT]
 *   ├─帧头──┤ │  │  ├─地址──┤  └─读取word数
 *             LEN CMD
 *   LEN = 0x04：从指令码起计 4 字节（83 AH AL CNT）
 *
 * 示例：读 0x1000 处 1 个 word → 5A A5 04 83 10 00 01
 * 屏幕应答：5A A5 06 83 10 00 01 00 02
 *            ├─帧头──┤ │  │  ├─地址──┤ │   ├─数据──┤
 *                      LEN CMD         CNT
 */
void DWIN_ReadVar(uint16_t addr, uint8_t word_cnt)
{
    uint8_t buf[7];
    buf[0] = 0x5A;
    buf[1] = 0xA5;
    buf[2] = 0x04;                          /* LEN */
    buf[3] = 0x83;                          /* 读变量指令 */
    buf[4] = (uint8_t)(addr >> 8);          /* 地址高字节 */
    buf[5] = (uint8_t)(addr & 0xFF);        /* 地址低字节 */
    buf[6] = word_cnt;                      /* 读取 word 数量（最大 0x7C）*/
    UART3_Send(buf, sizeof(buf));
}

/**
 * @brief 将变量存储空间数据写入片内 NorFlash 数据库（0x08 寄存器，写操作）
 *
 * 帧格式（共 12 字节）：
 *   5A A5 0B 82 00 08 A5 [F2] [F1] [F0] [VH] [VL] [LH] [LL]
 *                        └─操作模式=写(0xA5)─┘
 *   0x0008 : NOR_FLASH_RW_CMD 寄存器地址
 *   D7(1字节)  = 0xA5  操作模式：写
 *   D6:4(3字节)= flash_addr  NorFlash 数据库首地址（偶数，范围 0x000000~0x03FFFE）
 *   D3:2(2字节)= var_addr    变量存储空间首地址（偶数）
 *   D1:0(2字节)= word_len    读写字长度（偶数，单位 word）
 *
 * 示例：将变量 0x1002 处 2 个 word 写入 NorFlash 0x000002
 *   → 5A A5 0B 82 00 08 A5 00 00 02 10 02 00 02
 *
 * @param flash_addr  NorFlash 目标首地址（24位，必须为偶数）
 * @param var_addr    变量存储空间首地址（必须为偶数）
 * @param word_len    读写字长度（word 数，必须为偶数）
 */
void DWIN_NorFlashWrite(uint32_t flash_addr, uint16_t var_addr, uint16_t word_len)
{
    uint8_t buf[14];
    buf[0]  = 0x5A;
    buf[1]  = 0xA5;
    buf[2]  = 0x0B;                              /* LEN */
    buf[3]  = 0x82;                              /* 写变量指令 */
    buf[4]  = 0x00;                              /* 寄存器地址高字节 */
    buf[5]  = 0x08;                              /* 寄存器地址低字节（0x0008）*/
    buf[6]  = 0xA5;                              /* D7：操作模式 = 写 */
    buf[7]  = (uint8_t)((flash_addr >> 16) & 0xFF); /* D6：flash 地址高字节 */
    buf[8]  = (uint8_t)((flash_addr >>  8) & 0xFF); /* D5：flash 地址中字节 */
    buf[9]  = (uint8_t)( flash_addr        & 0xFF); /* D4：flash 地址低字节 */
    buf[10] = (uint8_t)(var_addr >> 8);          /* D3：变量地址高字节 */
    buf[11] = (uint8_t)(var_addr & 0xFF);        /* D2：变量地址低字节 */
    buf[12] = (uint8_t)(word_len >> 8);          /* D1：字长高字节 */
    buf[13] = (uint8_t)(word_len & 0xFF);        /* D0：字长低字节 */
    UART3_Send(buf, sizeof(buf));
}

/**
 * @brief 向迪文屏发送读取 RTC 时间的指令（0x83 指令，地址 0x0010，读 4 个 word）
 *
 * 发送帧（7 字节）：5A A5 04 83 00 10 04
 * 屏幕应答（15 字节）：
 *   5A A5 0C 83 00 10 04 [YY][MM][DD][WW][HH][mn][SS][00]
 *   buf[7]=年  buf[8]=月  buf[9]=日  buf[10]=星期
 *   buf[11]=时 buf[12]=分 buf[13]=秒 buf[14]=无意义
 */
void DWIN_ReadRTC(void)
{
    uint8_t buf[7];
    buf[0] = 0x5AU;
    buf[1] = 0xA5U;
    buf[2] = 0x04U;   /* LEN：指令码(1)+地址(2)+word数(1) */
    buf[3] = 0x83U;   /* 读变量指令 */
    buf[4] = 0x00U;   /* RTC 寄存器地址高字节 */
    buf[5] = 0x10U;   /* RTC 寄存器地址低字节（0x0010）*/
    buf[6] = 0x04U;   /* 读 4 个 word（8 字节）= 年月日星期时分秒保留 */
    UART3_Send(buf, sizeof(buf));
}

/**
 * @brief 向迪文屏报警记录区写入一条报警记录（环形覆盖）
 *
 * 地址布局：
 *   起始地址 DWIN_ALARM_BASE_ADDR（0x2000），每条记录 4 个 word（8 字节）
 *   第 n 条记录地址 = 0x2000 + n * 4，n = 0~11
 *
 * 记录字节格式（Big-Endian）：
 *   [年高(1)][年低(1)][月(1)][日(1)][时(1)][分(1)][控制器地址(1)][传感器槽位(1)]
 *
 * @param rtc        报警发生时的 RTC 时间（由 DWIN_ParseRTC 填充）
 * @param ctrlAddr   触发报警的控制器地址（1~128）
 * @param sensorIdx  报警传感器在控制器侧的槽位索引（0~63），未知时传 0xFF
 */
void DWIN_WriteAlarmRecord(const DWIN_RTC_t *rtc, uint8_t ctrlAddr, uint8_t sensorIdx)
{
    if (rtc == NULL)
    {
        return;
    }

    /* 环形写入索引（文件 scope，在 InitQueues 中归零） */

    /* 计算本条记录的变量地址 */
    uint16_t addr = (uint16_t)(DWIN_ALARM_BASE_ADDR
                    + (uint16_t)s_alarmIdx * DWIN_ALARM_RECORD_WORDS);

    /* 组装 8 字节记录
     * year 为普通十进制值（如 25 表示2025年），实际年份 = 2000 + year */
    uint16_t full_year = (uint16_t)(2000U + rtc->year);

    uint8_t buf[8];
    buf[0] = (uint8_t)(full_year >> 8U);   /* 年高字节（固定为 0x07）*/
    buf[1] = (uint8_t)(full_year & 0xFFU); /* 年低字节（如 0xD9 = 2025）*/
    buf[2] = rtc->month;
    buf[3] = rtc->day;
    buf[4] = rtc->hour;
    buf[5] = rtc->minute;
    buf[6] = ctrlAddr;                     /* 控制器地址（原 buf[7]，秒字段移除）*/
    buf[7] = sensorIdx;                    /* 报警传感器槽位索引（0~63)*/

    DWIN_WriteVar(addr, buf, sizeof(buf));

    /* 索引递增，超出上限则回绕到 0 */
    s_alarmIdx++;
    if (s_alarmIdx >= DWIN_ALARM_MAX_RECORDS)
    {
        s_alarmIdx = 0U;
    }
}

/****
 * @brief 计算字节数组的 CRC16（多项式 0x8005，初值 0xFFFF）
 *
 * 用于备注数据的完整性校验：
 *   保存时：在数据末尾附加 CRC16（大端序，高字节在前）
 *   读回时：对 [content_len + content] 重新计算并与末尾存储值比对
 *
 * @param pData  数据指针
 * @param len    数据字节数
 * @retval CRC16 校验值
 */
uint16_t DWIN_CalcCRC16(const uint8_t *pData, uint16_t len)
{
    uint16_t crc = 0xFFFFU;
    for (uint16_t i = 0U; i < len; i++)
    {
        crc ^= (uint16_t)pData[i] << 8U;
        for (uint8_t bit = 0U; bit < 8U; bit++)
        {
            if (crc & 0x8000U)
            {
                crc = (uint16_t)((crc << 1U) ^ 0x8005U);
            }
            else
            {
                crc <<= 1U;
            }
        }
    }
    return crc;
}

/*
 * @brief 从片内 NorFlash 数据库读取数据到变量存储空间（0x08 寄存器，读操作）
 *
 * 帧格式（共 12 字节）：
 *   5A A5 0B 82 00 08 5A [F2] [F1] [F0] [VH] [VL] [LH] [LL]
 *                        └─操作模式=读(0x5A)─┘
 *   D7(1字节)  = 0x5A  操作模式：读
 *   D6:4(3字节)= flash_addr  NorFlash 数据库首地址（偶数）
 *   D3:2(2字节)= var_addr    变量存储空间首地址（偶数）
 *   D1:0(2字节)= word_len    读写字长度（偶数，单位 word）
 *
 * 示例：从 NorFlash 0x000002 读 2 个 word 到变量 0x2000
 *   → 5A A5 0B 82 00 08 5A 00 00 02 20 00 00 02
 *
 * @note 发送后需等待 D7 自动清零（轮询或延时）再进行其他操作。
 *
 * @param flash_addr  NorFlash 源首地址（24位，必须为偶数）
 * @param var_addr    变量存储空间目标首地址（必须为偶数）
 * @param word_len    读写字长度（word 数，必须为偶数）
 */
void DWIN_NorFlashRead(uint32_t flash_addr, uint16_t var_addr, uint16_t word_len)
{
    uint8_t buf[14];
    buf[0]  = 0x5A;
    buf[1]  = 0xA5;
    buf[2]  = 0x0B;                              /* LEN */
    buf[3]  = 0x82;                              /* 写变量指令 */
    buf[4]  = 0x00;                              /* 寄存器地址高字节 */
    buf[5]  = 0x08;                              /* 寄存器地址低字节（0x0008）*/
    buf[6]  = 0x5A;                              /* D7：操作模式 = 读 */
    buf[7]  = (uint8_t)((flash_addr >> 16) & 0xFF); /* D6：flash 地址高字节 */
    buf[8]  = (uint8_t)((flash_addr >>  8) & 0xFF); /* D5：flash 地址中字节 */
    buf[9]  = (uint8_t)( flash_addr        & 0xFF); /* D4：flash 地址低字节 */
    buf[10] = (uint8_t)(var_addr >> 8);          /* D3：变量地址高字节 */
    buf[11] = (uint8_t)(var_addr & 0xFF);        /* D2：变量地址低字节 */
    buf[12] = (uint8_t)(word_len >> 8);          /* D1：字长高字节 */
    buf[13] = (uint8_t)(word_len & 0xFF);        /* D0：字长低字节 */
    UART3_Send(buf, sizeof(buf));
}

/* USER CODE BEGIN 1 */

/* ==================== 队列/信号量初始化 ==================== */

void Dwin_InitQueues(void)
{
    if (xDwinTxQueue == NULL)
        xDwinTxQueue = xQueueCreate(DWIN_TX_QUEUE_LEN, sizeof(DwinFrame_t));

    if (xDwinRxQueue == NULL)
        xDwinRxQueue = xQueueCreate(DWIN_RX_QUEUE_LEN, sizeof(DwinFrame_t));

    if (xDwinTxCompleteSem == NULL)
        xDwinTxCompleteSem = xSemaphoreCreateBinary();

    /* 重置报警记录环形索引（重启后从头写入） */
    s_alarmIdx = 0;
}

/* ==================== USART3 发送 ==================== */

void UART3_Send(const uint8_t *buf, uint16_t len)
{
    if (buf == NULL || len == 0 || len > DWIN_TX_FRAME_MAX)
        return;
    if (xDwinTxQueue == NULL)
        return;

    DwinFrame_t frame;
    frame.length = len;
    for (uint16_t i = 0; i < len; i++)
        frame.data[i] = buf[i];

    xQueueSend(xDwinTxQueue, &frame, pdMS_TO_TICKS(100));
}

/* ==================== USART3 RX 状态机 ==================== */

void DwinRxByteHandler(uint8_t data)
{
    BaseType_t xWoken = pdFALSE;

    switch (dwin_rx_state)
    {
        case DWIN_RX_WAIT_5A:
            if (data == 0x5A)
            {
                dwin_rx_buf[0] = 0x5A;
                dwin_rx_idx = 1;
                dwin_rx_state = DWIN_RX_WAIT_A5;
            }
            break;

        case DWIN_RX_WAIT_A5:
            if (data == 0xA5)
            {
                dwin_rx_buf[1] = 0xA5;
                dwin_rx_idx = 2;
                dwin_rx_state = DWIN_RX_WAIT_LEN;
            }
            else if (data == 0x5A)
            {
                dwin_rx_buf[0] = 0x5A;
                dwin_rx_idx = 1;
            }
            else
            {
                dwin_rx_state = DWIN_RX_WAIT_5A;
                dwin_rx_idx = 0;
            }
            break;

        case DWIN_RX_WAIT_LEN:
            dwin_rx_buf[2] = data;
            dwin_rx_idx = 3;
            if (data > 0 && data <= (DWIN_RX_BUF_SIZE - 3))
            {
                dwin_rx_expected = 3 + data;
                dwin_rx_state = DWIN_RX_DATA;
            }
            else
            {
                dwin_rx_state = DWIN_RX_WAIT_5A;
                dwin_rx_idx = 0;
            }
            break;

        case DWIN_RX_DATA:
            if (dwin_rx_idx < DWIN_RX_BUF_SIZE)
                dwin_rx_buf[dwin_rx_idx] = data;
            dwin_rx_idx++;

            if (dwin_rx_idx >= dwin_rx_expected)
            {
                DwinFrame_t frame;
                frame.length = (dwin_rx_idx <= DWIN_RX_FRAME_MAX)
                               ? (uint16_t)dwin_rx_idx : DWIN_RX_FRAME_MAX;
                for (uint16_t i = 0; i < frame.length; i++)
                    frame.data[i] = dwin_rx_buf[i];

                if (xDwinRxQueue != NULL)
                    xQueueSendFromISR(xDwinRxQueue, &frame, &xWoken);

                dwin_rx_state = DWIN_RX_WAIT_5A;
                dwin_rx_idx = 0;
            }
            break;
    }

    portYIELD_FROM_ISR(xWoken);
}

/* ==================== USART3 RX 状态机重置 ==================== */

void DwinRxReset(void)
{
    dwin_rx_state = DWIN_RX_WAIT_5A;
    dwin_rx_idx = 0;
}

/* USER CODE END 1 */
