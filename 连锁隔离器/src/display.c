#include "display.h"
#include "board.h"
#include "pressure.h"
#include "sensor_modbus.h"
#include "stc8h.h"

static uint8_t g_digits[3];
static uint8_t g_scan_index;

static const uint8_t g_seg_table[10] = {
    0x3FU, 0x06U, 0x5BU, 0x4FU, 0x66U,
    0x6DU, 0x7DU, 0x07U, 0x7FU, 0x6FU
};

static void Display_AllDigitsOff(void)
{
    P23 = 1;
    P26 = 1;
    P01 = 1;
}

static void Display_SetSegments(uint8_t pattern)
{
    P24 = (pattern & 0x01U) ? 1 : 0;
    P27 = (pattern & 0x02U) ? 1 : 0;
    P36 = (pattern & 0x04U) ? 1 : 0;
    P20 = (pattern & 0x08U) ? 1 : 0;
    P21 = (pattern & 0x10U) ? 1 : 0;
    P25 = (pattern & 0x20U) ? 1 : 0;
    P35 = (pattern & 0x40U) ? 1 : 0;
    P37 = 0;
}

static void Display_EnableDigit(uint8_t index)
{
    if (index == 0U) {
        P23 = 0;
    } else if (index == 1U) {
        P26 = 0;
    } else {
        P01 = 0;
    }
}

static void Display_WaitAndPollRx(uint16_t count)
{
    while (count-- != 0U) {
        Pressure_ProcessRx();
        SensorModbus_Process(Board_ReadAddress(), Pressure_GetValue());
        __asm nop __endasm;
    }
}

void Display_Init(void)
{
    g_digits[0] = 0U;
    g_digits[1] = 0U;
    g_digits[2] = 0U;
    g_scan_index = 0U;
    Display_AllDigitsOff();
    Display_SetSegments(0U);
}

void Display_SetValue(uint16_t value)
{
    if (value > 999U) {
        value = 999U;
    }

    g_digits[0] = (uint8_t)(value / 100U);
    g_digits[1] = (uint8_t)((value / 10U) % 10U);
    g_digits[2] = (uint8_t)(value % 10U);
}

void Display_ScanOnce(void)
{
    Display_AllDigitsOff();
    Display_SetSegments(0U);
    Display_WaitAndPollRx(10U);

    Display_SetSegments(g_seg_table[g_digits[g_scan_index]]);
    Display_WaitAndPollRx(10U);

    Display_EnableDigit(g_scan_index);
    Display_WaitAndPollRx(600U);
    Display_AllDigitsOff();

    g_scan_index++;
    if (g_scan_index >= 3U) {
        g_scan_index = 0U;
    }
}

void Display_ShowRawDigit(uint8_t digit, uint8_t value)
{
    if (value > 9U) {
        value = 9U;
    }

    Display_AllDigitsOff();
    Display_SetSegments(0U);
    Display_SetSegments(g_seg_table[value]);
    Display_EnableDigit(digit);
}

void Display_TestAllOn(void)
{
    P24 = 1;
    P27 = 1;
    P36 = 1;
    P20 = 1;
    P21 = 1;
    P25 = 1;
    P35 = 1;
    P37 = 1;

    P23 = 0;
    P26 = 0;
    P01 = 0;
}

void Display_TestPolarity(uint8_t mode)
{
    uint8_t seg_on = (mode & 0x01U) ? 1U : 0U;
    uint8_t dig_on = (mode & 0x02U) ? 1U : 0U;

    P24 = seg_on;
    P27 = seg_on;
    P36 = seg_on;
    P20 = seg_on;
    P21 = seg_on;
    P25 = seg_on;
    P35 = seg_on;
    P37 = seg_on;

    P23 = dig_on;
    P26 = dig_on;
    P01 = dig_on;
}
