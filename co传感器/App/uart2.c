#include "uart2.h"

u8  TX2_Cnt;    //发送计数
u8  RX2_Cnt;    //接收计数
bit B_TX2_Busy; //发送忙标志
u8 idata RX2_Buffer[UART2_BUF_LENGTH]; //接收缓冲

//========================================================================
// 函数: void TX2_data(u8 tx2dat)
// 描述: 串口2发送数据函数
// 参数: tx2dat: 要发送的数据
// 返回: none
// 备注: 
//========================================================================
void TX2_data(u8 tx2dat)
{
	S2BUF = tx2dat;
	B_TX2_Busy = 1;
	while(B_TX2_Busy);
}

//========================================================================
// 函数: void PrintString2(u8 *puts)
// 描述: 串口2发送字符串函数
// 参数: puts:  字符串指针
// 返回: none
// 备注: 
//========================================================================
void PrintString2(u8 *puts)
{
	for (; *puts != 0;  puts++)	//到停止符0结束
	{
		TX2_data(*puts);
	}
}

//========================================================================
// 函数: void UART2_config(u8 UART_IOSEL,u32 FOSC,u32 BAUD2)
// 描述: UART2初始化函数
// 参数: FOSC:设置晶振时钟频率，BAUD2:设置波特率
//			 UART_IOSEL=0x00:P1.0/P1.1
// 返回: none
// 备注: 
//========================================================================
void UART2_config(u8 UART_IOSEL,u32 FOSC,u32 BAUD2)
{
	AUXR &= ~(1<<4);    //Timer stop
	AUXR &= ~(1<<3);    //Timer2 set As Timer
	AUXR |=  (1<<2);    //Timer2 set as 1T mode
	T2H = (65536UL - (FOSC / 4) / BAUD2) / 256;
	T2L = (65536UL - (FOSC / 4) / BAUD2) % 256;
	IE2  &= ~(1<<2);    //禁止中断
	AUXR |=  (1<<4);    //Timer run enable
	S2CON &= ~(1<<7);   // 8位数据, 1位起始位, 1位停止位, 无校验
	
	IP2 |= 1;
	IP2H |= 1;
	
	IE2   |= 1;         //允许中断
	S2CON |= (1<<4);    //允许接收
	P_SW2 |= UART_IOSEL;	//UART2 switch to: 0: P1.0 P1.1
	B_TX2_Busy = 0;
	TX2_Cnt = 0;
	RX2_Cnt = 0;
}


void UART2_sendArray(unsigned char count, unsigned char *p)
{
	unsigned char i = 0;
	for (i = 0; i < count; i++){
		TX2_data(p[i]);
	}
}

//========================================================================
// 函数: void UART2_int (void) interrupt UART2_VECTOR
// 描述: UART2中断函数
// 参数: none
// 返回: none
// 备注: 
//========================================================================
// void UART2_int (void) interrupt UART2_VECTOR
// {
//     if((S2CON & 1) != 0)
//     {
//         S2CON &= ~1;    //Clear Rx flag
//         RX2_Buffer[RX2_Cnt] = S2BUF;
//         if(++RX2_Cnt >= UART2_BUF_LENGTH)   RX2_Cnt = 0;
//     }
//     if((S2CON & 2) != 0)
//     {
//         S2CON &= ~2;    //Clear Tx flag
//         B_TX2_Busy = 0;
//     }
// }