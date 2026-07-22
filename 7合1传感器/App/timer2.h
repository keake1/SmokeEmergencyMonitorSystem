#ifndef _TIMER2_H
#define _TIMER2_H

#include "config.h"

extern u16 T2_cnt;

#define STC_1T 	1
#define STC_12T 2

/*timer2 config*/
void Timer2_Config(u8 mode, u16 time);

#endif