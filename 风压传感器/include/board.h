#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

void Board_Init(void);
uint8_t Board_ReadAddress(void);
void Board_RedLedSet(uint8_t on);
void Board_GreenLedToggle(void);
void Board_DelayMs(uint16_t ms);

#endif

