#include "sensor_modbus.h"
#include "board.h"
#include "config.h"
#include "crc16.h"
#include "stc8h.h"
#include "uart.h"

/*
 * Modbus RTU 从站（UART2，9600 8N1）
 *
 * 帧定界：Timer 0 轮询检测 3.5 字符（约 4ms）总线空闲间隔。
 *   每收到一字节就重装并启动 Timer 0；缓冲非空且 TF0 溢出
 *   即认为一帧结束。总线上混有主站请求（8 字节）和其它
 *   传感器的响应（简单传感器 8 字节 / 7合1 20 字节），
 *   只有按空闲间隔定界才能保证不错位。
 *
 * 支持的指令（与 Controlboard modbus_tasks.c 对应）：
 *   读：  addr 03 00 00 00 01 CRC
 *         回复 addr 03 03 <型号 0x07> <电平高> <电平低> CRC
 *   报警：addr 06 00 04 00 <00/01> CRC → 控制红灯
 *         不回复——主站不等待写响应，且主站解析不区分功能码，
 *         回显帧的 frame[3]=0x04 会被误认为 7合1 型号字节。
 */

/* 缓冲需容纳总线上最长的帧（7合1 响应 20 字节）*/
#define MB_RX_BUF_SIZE 24U

/* Timer 0 重装值：12T 模式下 FOSC/12 = 921.6kHz，
 * 4ms ≈ 3686 个计数 → 65536 - 3686 = 61850 = 0xF19A */
#define MB_GAP_RELOAD_H 0xF1U
#define MB_GAP_RELOAD_L 0x9AU

#define TCON_TR0 0x10U
#define TCON_TF0 0x20U

static uint8_t g_rx[MB_RX_BUF_SIZE];
static uint8_t g_rx_len;
static uint8_t g_rx_overflow;

void SensorModbus_Init(void)
{
    g_rx_len = 0U;
    g_rx_overflow = 0U;

    /* Timer 0：模式 1（16 位）、12T，仅作空闲间隔计时，不开中断 */
    TMOD = (TMOD & 0xF0U) | 0x01U;
    AUXR &= (uint8_t)~0x80U;            /* T0x12 = 0：12T 模式 */
    TCON &= (uint8_t)~(TCON_TR0 | TCON_TF0);
}

/* 收到一字节后重启空闲计时器 */
static void SensorModbus_RestartGapTimer(void)
{
    TCON &= (uint8_t)~(TCON_TR0 | TCON_TF0);
    TH0 = MB_GAP_RELOAD_H;
    TL0 = MB_GAP_RELOAD_L;
    TCON |= TCON_TR0;
}

static void SensorModbus_ReplyLevel(uint8_t addr, uint16_t level)
{
    uint8_t tx[8];

    tx[0] = addr;
    tx[1] = 0x03U;
    tx[2] = 0x03U;                               /* 字节数：型号 + 2 字节数据 */
    tx[3] = SENSOR_TYPE_CHAIN_ISOLATOR;           /* 0x07 */
    tx[4] = (uint8_t)(level >> 8);
    tx[5] = (uint8_t)level;
    Crc16_Append(tx, 6U);
    Uart2_Send(tx, sizeof(tx));
}

static void SensorModbus_HandleFrame(uint8_t my_addr, uint16_t level)
{
    /* 主站请求固定 8 字节；其它长度（如别的传感器响应）直接忽略 */
    if (g_rx_len != 8U) {
        return;
    }

    if (g_rx[0] != my_addr) {
        return;
    }

    if (Crc16_Check(g_rx, 8U) == 0U) {
        return;
    }

    /* 读保持寄存器：寄存器 0x0000，数量 0x0001 */
    if ((g_rx[1] == 0x03U) &&
        (g_rx[2] == 0x00U) && (g_rx[3] == 0x00U) &&
        (g_rx[4] == 0x00U) && (g_rx[5] == 0x01U)) {
        SensorModbus_ReplyLevel(my_addr, level);
        return;
    }

    /* 写单寄存器：寄存器 0x0004 = 报警标志（不回复，见文件头注释） */
    if ((g_rx[1] == 0x06U) &&
        (g_rx[2] == 0x00U) && (g_rx[3] == 0x04U) &&
        (g_rx[4] == 0x00U)) {
        Board_RedLedSet((g_rx[5] != 0U) ? 1U : 0U);
    }
}

void SensorModbus_Process(uint8_t my_addr, uint16_t level)
{
    uint8_t b;

    /* 取走当前所有已到字节 */
    while (Uart2_ReadByte(&b) != 0U) {
        if (g_rx_len < MB_RX_BUF_SIZE) {
            g_rx[g_rx_len++] = b;
        } else {
            g_rx_overflow = 1U;          /* 超长帧：整帧作废 */
        }
        SensorModbus_RestartGapTimer();
    }

    /* 缓冲非空且空闲超过 3.5 字符 → 一帧结束 */
    if ((g_rx_len != 0U) && ((TCON & TCON_TF0) != 0U)) {
        TCON &= (uint8_t)~(TCON_TR0 | TCON_TF0);
        if (g_rx_overflow == 0U) {
            SensorModbus_HandleFrame(my_addr, level);
        }
        g_rx_len = 0U;
        g_rx_overflow = 0U;
    }
}
