#include "board.h"
#include "config.h"
#include "sensor_modbus.h"
#include "uart.h"

void main(void)
{
    uint16_t heartbeat_tick = 0U;
    uint16_t level;

    Board_Init();
    Uart_Init();
    SensorModbus_Init();
    Board_GreenLedToggle();

    for (;;) {
        level = Board_ReadP00Level();
        SensorModbus_Process(Board_ReadAddress(), level);

        Board_DelayMs(1U);

        heartbeat_tick += 1U;
        if (heartbeat_tick >= HEARTBEAT_INTERVAL_MS) {
            heartbeat_tick = 0U;
            Board_GreenLedToggle();
        }
    }
}
