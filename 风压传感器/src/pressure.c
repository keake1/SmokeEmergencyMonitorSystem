#include "pressure.h"
#include "config.h"
#include "crc16.h"
#include "uart.h"
#include "board.h"

static uint16_t g_pressure;
static uint8_t g_frame_pos;
static uint8_t g_pressure_hi;
static uint8_t g_pressure_lo;

void Pressure_OnByte(uint8_t b)
{
    if (b == 0xFEU) {
        g_frame_pos = 1U;
        return;
    }

    switch (g_frame_pos) {
    case 0:
        break;

    case 1:
        g_pressure_hi = b;
        g_frame_pos = 2U;
        break;

    case 2:
        g_pressure_lo = b;
        g_frame_pos = 3U;
        break;

    case 3:
        g_frame_pos = 4U;
        break;

    case 4:
        g_frame_pos = 5U;
        break;

    default:
        if (b == 0xDCU) {
            uint16_t pressure = ((uint16_t)g_pressure_hi << 8) | g_pressure_lo;
            if (pressure <= 999U) {
                g_pressure = pressure;
            }
        }
        g_frame_pos = 0U;
        break;
    }
}

void Pressure_Init(void)
{
    g_pressure = 0U;
    g_frame_pos = 0U;
    g_pressure_hi = 0U;
    g_pressure_lo = 0U;
}

uint16_t Pressure_GetValue(void)
{
    return g_pressure;
}

void Pressure_ProcessRx(void)
{
    uint8_t b;

    while (Uart1_ReadByte(&b) != 0U) {
        Pressure_OnByte(b);
    }
}

void Pressure_ProcessControllerRx(void)
{
    uint8_t b;

    while (Uart2_ReadByte(&b) != 0U) {
        Pressure_OnByte(b);
    }
}

void Pressure_PollBlocking(void)
{
    uint8_t req[8];
    uint8_t resp[7];
    uint8_t i = 0U;
    uint16_t wait;
    uint8_t b;

    req[0] = PRESSURE_MODULE_ADDR;
    req[1] = 0x03U;
    req[2] = 0x00U;
    req[3] = 0x00U;
    req[4] = 0x00U;
    req[5] = 0x01U;
    Crc16_Append(req, 6U);

    Uart1_Send(req, sizeof(req));

    for (wait = 0U; wait < 30000U; ++wait) {
        if (Uart1_ReadByte(&b) != 0U) {
            if (i < sizeof(resp)) {
                resp[i++] = b;
            }
            if (i >= sizeof(resp)) {
                break;
            }
        }
    }

    if ((i == sizeof(resp)) &&
        (resp[0] == PRESSURE_MODULE_ADDR) &&
        (resp[1] == 0x03U) &&
        (resp[2] == 0x02U) &&
        (Crc16_Check(resp, sizeof(resp)) != 0U)) {
        g_pressure = ((uint16_t)resp[3] << 8) | resp[4];
    }
}
