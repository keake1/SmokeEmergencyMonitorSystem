#include "uart1.h"
#include "delay.h"
u8  TX1_Cnt;    //发送计数
u8  RX1_Cnt;    //接收计数
bit B_TX1_Busy; //发送忙标志
u8 idata RX1_Buffer[UART1_BUF_LENGTH]; //接收缓冲

//========================================================================
// 函数: void TX1_data()
// 描述: 串口1发送数据函数
// 参数: tx1dat: 要发送的数据
// 返回: none
// 备注: 
//========================================================================
void TX1_data(u8 tx1dat)
{
	SBUF = tx1dat;
	B_TX1_Busy = 1;
	
	while(B_TX1_Busy);

}

//========================================================================
// 函数: void PrintString1(u8 *puts)
// 描述: 串口1发送字符串函数
// 参数: puts:  字符串指针
// 返回: none
// 备注: 
//========================================================================
void PrintString1(u8 *puts)
{
	for (; *puts != 0;  puts++)     //到停止符0结束
	{
		TX1_data(*puts);
	}
}

//========================================================================
// 函数: void UART1_config(u8 UART_IOSEL,u32 FOSC,u32 BAUD1)
// 描述: UART1初始化函数
// 参数: FOSC:设置晶振时钟频率，BAUD1:设置波特率
//			 UART_IOSEL=0x00:P3.0/P3.1
//			 UART_IOSEL=0x40:P3.2/P3.3
//			 UART_IOSEL=0x80:P5.4/P5.5
// 返回: none
// 备注: 
//========================================================================
void UART1_config(u8 UART_IOSEL,u32 FOSC,u32 BAUD1)
{
	TR1 = 0;
	AUXR &= ~0x01;      //S1 BRT Use Timer1;
	AUXR |=  (1<<6);    //Timer1 set as 1T mode
	TMOD &= ~(1<<6);    //Timer1 set As Timer
	TMOD &= ~0x30;      //Timer1_16bitAutoReload;
	TH1 = (u8)((65536UL - (FOSC / 4) / BAUD1) / 256);
	TL1 = (u8)((65536UL - (FOSC / 4) / BAUD1) % 256);
	ET1 = 0;    //禁止中断
	TR1  = 1;
	SCON = (SCON & 0x3f) | 0x40;    //UART1模式, 0x00: 同步移位输出, 0x40: 8位数据,可变波特率, 0x80: 9位数据,固定波特率, 0xc0: 9位数据,可变波特率
    PS  = 1;    //高优先级中断
	IPH  |=  0x01 << 4;
	ES  = 1;    //允许中断
	REN = 1;    //允许接收
	P_SW1 |= UART_IOSEL;
	//  PCON2 |=  (1<<4);   //内部短路RXD与TXD, 做中继, ENABLE,DISABLE
	B_TX1_Busy = 0;
	TX1_Cnt = 0;
	RX1_Cnt = 0;
}

//========================================================================
// 函数: void UART1_int (void) interrupt UART1_VECTOR
// 描述: UART1中断函数
// 参数: none
// 返回: none
// 备注: 
//========================================================================
//void UART1_int (void) interrupt 4
//{
//    if(RI)
//    {
//        RI = 0;
//        RX1_Buffer[RX1_Cnt] = SBUF;
//        if(++RX1_Cnt >= UART1_BUF_LENGTH)   RX1_Cnt = 0;
//    }
//    if(TI)
//    {
//        TI = 0;
//        B_TX1_Busy = 0;
//    }
//}

void UART1_sendArray(unsigned char count, unsigned char *p)
{
	unsigned char i = 0;
	for (i = 0; i < count; i++){
		
		TX1_data(p[i]);
	}
}

