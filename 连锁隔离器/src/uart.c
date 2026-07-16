#include "uart.h"
#include "config.h"
#include "stc8h.h"

/*
 * UART2 是 Modbus 总线（9600 8N1），使用 Timer 2 作波特率发生器。
 * STC8H1K28 的 UART2 固定使用 Timer 2。
 */

static uint16_t Uart_Reload(uint32_t baud)
{
    return (uint16_t)(65536UL - (FOSC_HZ / 4UL / baud));
}

void Uart_Init(void)
{
    /*
     * Timer 2 → UART2 @ 9600（1T mode, 16-bit auto-reload）
     */
    uint16_t reload2 = Uart_Reload(CONTROLLER_BAUD);

    S2CON = S2CON_REN;

    T2H = (uint8_t)(reload2 >> 8);
    T2L = (uint8_t)reload2;
    /* T2x12=1 | T2R=1：1T 模式，启动 Timer 2 */
    AUXR = (AUXR & (uint8_t)~0x01U) | 0x14U;

    S2CON &= (uint8_t)~(S2CON_RI | S2CON_TI);
}

void Uart2_SendByte(uint8_t value)
{
    S2BUF = value;
    while ((S2CON & S2CON_TI) == 0U) {
    }
    S2CON &= (uint8_t)~S2CON_TI;
}

uint8_t Uart2_ReadByte(uint8_t *value)
{
    if ((S2CON & S2CON_RI) == 0U) {
        return 0U;
    }

    S2CON &= (uint8_t)~S2CON_RI;
    *value = S2BUF;
    return 1U;
}

void Uart2_Send(const uint8_t *data, uint8_t len)
{
    uint8_t i;
    for (i = 0U; i < len; ++i) {
        Uart2_SendByte(data[i]);
    }
}
