#ifndef _UART1_H
#define _UART1_H

#include "STC8H.h"

/*************	本地常量声明	**************/
#define UART1_BUF_LENGTH    64

//串口1功能脚选择
#define UART1_P30P31	0x00
#define UART1_P36P37	0x40
#define UART1_P16P17	0x80


/*************	本地变量声明	**************/
extern u8  TX1_Cnt;    //发送计数
extern u8  RX1_Cnt;    //接收计数
extern bit B_TX1_Busy; //发送忙标志
extern u8 idata RX1_Buffer[UART1_BUF_LENGTH]; //接收缓冲


/*函数声明*/
void UART1_config(u8 UART_IOSEL,u32 FOSC,u32 BAUD1);
void TX1_data(u8 tx1dat);
void PrintString1(u8 *puts);
void UART1_sendArray(unsigned char count, unsigned char *p);
#endif