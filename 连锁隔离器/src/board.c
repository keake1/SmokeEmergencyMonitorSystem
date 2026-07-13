#include "board.h"
#include "config.h"
#include "stc8h.h"

static void Board_SetQuasiBidirectional(void)
{
    P0M0 = 0x00U;
    P0M1 = 0x00U;
    P1M0 = 0x00U;
    P1M1 = 0x00U;
    P2M0 = 0x00U;
    P2M1 = 0x00U;
    P3M0 = 0x00U;
    P3M1 = 0x00U;

    /*
     * LED display pins must drive current, so use push-pull outputs.
     * STC mode bits: M1/M0 = 00 quasi, 01 push-pull.
     */
    P0M0 |= 0x02U;  /* P01 DIG3 */
    P2M0 |= 0xFFU;  /* P20..P27 segments, green LED, DIG1/DIG2 */
    P3M0 |= 0xF0U;  /* P34 red LED, P35/P36/P37 segments */
}

void Board_Init(void)
{
    Board_SetQuasiBidirectional();

    P02 = 1;
    P03 = 1;
    P12 = 1;
    P13 = 1;
    P14 = 1;
    P15 = 1;

    Board_RedLedSet(0U);
    P22 = 1;
}

uint8_t Board_ReadAddress(void)
{
    uint8_t addr = 0U;

    if (P15 == 0) { addr |= 0x20U; }
    if (P14 == 0) { addr |= 0x10U; }
    if (P13 == 0) { addr |= 0x08U; }
    if (P12 == 0) { addr |= 0x04U; }
    if (P03 == 0) { addr |= 0x02U; }
    if (P02 == 0) { addr |= 0x01U; }

    return addr;
}

void Board_RedLedSet(uint8_t on)
{
    P34 = (on != 0U) ? 0 : 1;
}

void Board_GreenLedToggle(void)
{
    P22 = !P22;
}

void Board_DelayMs(uint16_t ms)
{
    uint16_t i;
    uint8_t j;

    while (ms-- != 0U) {
        for (i = 0U; i < 500U; ++i) {
            for (j = 0U; j < 2U; ++j) {
                __asm nop __endasm;
            }
        }
    }
}
