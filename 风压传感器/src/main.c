#include "board.h"
#include "config.h"
#include "display.h"
#include "pressure.h"
#include "sensor_modbus.h"
#include "uart.h"

void main(void)
{
    uint16_t heartbeat_tick = 0U;

    Board_Init();
    Uart_Init();
    Pressure_Init();
    SensorModbus_Init();
    Display_Init();
    Board_GreenLedToggle();

    /*
     * 注意：UART2 是 Modbus 总线，严禁主动发送任何调试帧，
     * 否则会与主站请求/其它传感器响应碰撞并污染总线。
     */
    for (;;) {  
        Pressure_ProcessRx();
        SensorModbus_Process(Board_ReadAddress(), Pressure_GetValue());
        Display_SetValue(Pressure_GetValue());
        Display_ScanOnce();

        /* 粗略 1ms 节拍（for 循环忙等），使心跳计数接近真实毫秒 */
        Board_DelayMs(1U);

        heartbeat_tick+=1U;
        if (heartbeat_tick >= HEARTBEAT_INTERVAL_MS) {
            heartbeat_tick = 0U;
            Board_GreenLedToggle();
        }
    }
}
