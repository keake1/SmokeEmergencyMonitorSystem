#ifndef _TIMER0_H
#define _TIMER0_H

#include "STC8H.h"

extern u16 T0_cnt;

#define STC_1T 	1
#define STC_12T 2

/*timer0 config*/
void Timer0_Config(u8 mode, u16 time);

#endif