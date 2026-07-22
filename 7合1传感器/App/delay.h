#ifndef _DELAY_H
#define _DELAY_H

#include "STC8H.h"
#include "intrins.h"

//定义晶振频率
#define FOSC_11059200	1		//11.0592MHZ
#define FOSC_22118400 2		//22.1184MHZ
#define FOSC_24000000 3		//24MHZ

//选择使用的晶振频率
//当前默认使用11.0592MHZ
#define SYS_FOSC FOSC_11059200
//#define SYS_FOSC FOSC_22118400
//#define SYS_FOSC FOSC_24000000

//
void Delay_xms(u16 i);
void Delay_xus(u16 i);
#endif
