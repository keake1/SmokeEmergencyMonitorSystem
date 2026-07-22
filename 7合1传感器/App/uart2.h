#ifndef _UART2_H
#define _UART2_H

#include "STC8H.h"

/*************	本地常量声明	**************/
#define UART2_BUF_LENGTH    40

//串口1功能脚选择
#define UART2_P10P11	0x00


/*************	本地变量声明	**************/
extern u8  TX2_Cnt;    //发送计数
extern u8  RX2_Cnt;    //接收计数
extern bit B_TX2_Busy; //发送忙标志
extern u8 idata RX2_Buffer[UART2_BUF_LENGTH]; //接收缓冲


/*函数声明*/
void UART2_config(u8 UART_IOSEL,u32 FOSC,u32 BAUD2);
void TX2_data(u8 tx2dat);
void PrintString2(u8 *puts);
void UART2_sendArray(unsigned char count, unsigned char *p);

#endif