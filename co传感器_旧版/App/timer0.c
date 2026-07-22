#include "timer0.h"
#include "config.h"

//
u16 T0_cnt;

/*********************************************
/*函数名称：Timer0_Config(u8 mode, u16 time)
/*函数功能：Timer0配置函数
/*输入参数：mode:速度模式
/*					mode=1, 1T模式; mode=2,12T模式
/*					time:定时时间
/*	1T模式(max=5.9ms):1ms=1000,2ms=500,5ms=200
/*	12T模式(max=71ms):1ms=1000,10ms=100
/*返回值：无
*********************************************/
void Timer0_Config(u8 mode, u16 time)
{
	switch(mode)
	{
		case STC_1T:
		{
			AUXR |= 0x80;		//定时器0为1T模式
			TH0 = (65536-MAIN_Fosc/time)/256;
			TL0 = (65536-MAIN_Fosc/time)%256;
		}
		break;
		case STC_12T:
		{
			AUXR &= ~0x80;	//定时器0为12T模式
			TH0 = (65536-MAIN_Fosc/time/12)/256;
			TL0 = (65536-MAIN_Fosc/time/12)%256;
		}
	}
	TMOD &= 0xF0;		//设置定时器为模式0,16位自动重装载
	TR0 = 1;				//启动定时器
	ET0 = 1;				//打开定时器中断
	PX0 = 0;
}

////Timer0 Interrupt Routine
//void Timer0() interrupt TIMER0_VECTOR
//{
//	T0_cnt++;
//}

