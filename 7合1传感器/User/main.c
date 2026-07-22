#include "uart1.h"
#include "uart2.h"
#include "delay.h"
#include "stdio.h"
#include "digital_cube.h"
#include "addr.h"
#include "ModeBusRTU.h"
#include "timer0.h"
// #include "SPL06_001.h"

unsigned int d_pressure = 0;
unsigned int dTest = 0;
float pressure = 0, origin_pressure = 0, temperature = 0;
void main()
{
	
	P0M1 = 0;   P0M0 = 0;   
	P1M1 = 0;   P1M0 = 0;   
	P2M1 = 0;   P2M0 = 0;   
	P3M1 = 0;   P3M0 = 0;   
	P5M1 = 0;   P5M0 = 0;   
	P2M0 |= 0x01 << 2;
	//LED推挽输出
	P2M1 &= ~(0x01 << 2);
	P3M0 |= 0x01 << 4;
	P3M1 &= ~(0x01) << 4;
	P22 = 1;
	P34 = 1;
	addr_init();
	// do {
		// spl0601_init();
		// temperature = user_spl0601_get_temperature();
		// origin_pressure = user_spl0601_get_presure();	
	// } while(origin_pressure < 90000 || temperature > 70);//异常值重新初始化
//	spl0601_init();
//		temperature = user_spl0601_get_temperature();
//		origin_pressure = user_spl0601_get_presure();	
	Delay_xms(1000);
	Delay_xms(1000);	
	Timer0_Config(STC_1T, 1000);
	digital_cube_init();
	address = addr_read();
	UART1_config(UART1_P30P31,11059200,9600);
	UART2_config(UART2_P10P11,11059200,9600);
	EA = 1;

	while(1) {

		Digital_Tube_ShowNum((unsigned int)dTest % 1000);
		//Digital_Tube_ShowNum((int)(CO_Value));
		//Digital_Tube_ShowNum(ad_value % 1000);

	}
}


void error_handle(void)
{
	if (Reg[0x0004] == 1) {
		P34 = 0;//P34——>LED红灯
	}
	else {
		P34 = 1;
	}
}


void Pressure_reg0002_write()
{
	float a;
	//获取气压计的值
	
	
	a = (float)(pressure - origin_pressure);
	
	if (a <= 0 )
		d_pressure = 0;
	else
		d_pressure = (uint16_t)a;
	Reg[0x0002] = (uint16_t)d_pressure;

}

void Pressure_Reg0003_Read()
{
	if (Reg[0x0003] == 0x0001) {
		Reg[0x0003] = 0x0000;
	}
}




void run_1ms(void)
{
	if (uart_send_flag) {
		UART2_sendArray(ModBusSendBuf_len, ModBusSendBuf);
		
		uart_send_flag = 0;
		//Delay_xms(5);
	}
	Digital_Tube_flash();
	//Time++;
	cnt++;
	if (cnt >= 10) {
		cnt = 0;
		//UART1_sendArray(Res_cnt, ModbusReceiveBuf);
		ModBus_ReadOneByte();
		
		
		//P34 = ~P34;
	}
}
/*
	绿灯闪烁
*/
void run_500ms(void)
{
//	char temp[40];
	P22 = ~P22;
//	sprintf(temp, "p:%.0f，d:%d\r\nt:%d\r\norigin:%.0f", pressure, d_pressure, (int)temperature, origin_pressure);
//	PrintString1(temp);
	
}

void run_10ms(void)
{
	
}

void run_100ms(void)
{
	error_handle();
	// Pressure_Reg0003_Read();
	// Pressure_reg0002_write();
	
}
void run_2000ms(void)
{
	dTest= parse_sensor_frame();
}
uint8 count10ms;
uint16 count500ms;
uint8 count100ms;
uint16 count2000ms;
//1ms1次
//void Timer2() interrupt TIMER2_VECTOR
void Timer0() interrupt TIMER0_VECTOR
{	
	run_1ms();
	count10ms++;
	count500ms++;
	count100ms++;
	count2000ms++;
	if (count10ms >= 10) {
		run_10ms();
		count10ms = 0;
	}
	if (count100ms >= 100) {
		run_100ms();
		count100ms = 0;
	}
 	
	if (count500ms >= 500) {
		run_500ms();
		count500ms = 0;
	}
	if (count2000ms >= 2000) {
		run_2000ms();
		count2000ms = 0;
	}
}

