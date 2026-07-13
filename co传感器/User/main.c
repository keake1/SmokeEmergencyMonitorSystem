#include "uart1.h"
#include "uart2.h"
#include "delay.h"
#include "stdio.h"
#include "adc.h"
#include "digital_cube.h"
#include "addr.h"
#include "ModeBusRTU.h"
#include "timer0.h"

float CO_Value = 0;
u16 ad_value = 0;

void main()
{
	
	P0M1 = 0;   P0M0 = 0;   
	P1M1 = 0;   P1M0 = 0;   
	P2M1 = 0;   P2M0 = 0;   
	P3M1 = 0;   P3M0 = 0;   
	P5M1 = 0;   P5M0 = 0;   
	P2M0 |= 0x01 << 2;
	//LED�������
	P2M1 &= ~(0x01 << 2);
	P3M0 |= 0x01 << 4;
	P3M1 &= ~(0x01) << 4;
	P22 = 1;
	P34 = 1;
	addr_init();
	ADC_Init();
	ADC_IO_Config();
	Timer0_Config(STC_1T, 1000);
	
	
	//Delay_xms(2000);	
	digital_cube_init();
	address = addr_read();

	UART1_config(UART1_P30P31,11059200,9600);
	UART2_config(UART2_P10P11,11059200,9600);
	EA = 1;
	while(1) {
		
		
		Digital_Tube_ShowNum((int)(CO_Value)%1000);
		//Digital_Tube_ShowNum(ad_value % 1000);

	}
}

void CO_Reg0001_write(void)
{

	ad_value = Get_ADC10bitResult(8);
	CO_Value = ad_value - 372 / 4.0;
	CO_Value = CO_Value * 5 / 1024.0 * 100;
	if (CO_Value < 0)
		CO_Value = 0;
	Reg[0x0001] = (unsigned int)(CO_Value * 100);
}

void error_handle(void)
{
	if (Reg[0x0004] == 1) {
		P34 = 0;
	}
	else {
		P34 = 1;
	}
}

void run_1ms(void)
{
	if (uart_send_flag) {
		
		UART2_sendArray(ModBusSendBuf_len, ModBusSendBuf);
		UART1_sendArray(ModBusSendBuf_len, ModBusSendBuf);
		uart_send_flag = 0;
	}
	Digital_Tube_flash();
	//Time++;
	cnt++;
	if (cnt >= 10) {
		cnt = 0;
		ModBus_ReadOneByte();
	}
}

void run_500ms(void)
{
	///char temp[20];
	P22 = ~P22;
//	PrintString2("Hello World\r\n");
//	sprintf(temp, "ad:%d, co:%f", ad_value, CO_Value);
//	PrintString2(temp);
	
}

void run_10ms(void)
{

}

void run_100ms(void)
{
	error_handle();
	CO_Reg0001_write();
}

uint8 count10ms;
uint16 count500ms;
uint8 count100ms;

//1ms1��
//void Timer2() interrupt TIMER2_VECTOR
void Timer0() interrupt TIMER0_VECTOR
{	
	run_1ms();
	count10ms++;
	count500ms++;
	count100ms++;
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
}

