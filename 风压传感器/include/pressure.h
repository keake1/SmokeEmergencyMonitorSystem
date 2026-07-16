#ifndef PRESSURE_H
#define PRESSURE_H

#include <stdint.h>

void Pressure_Init(void);
void Pressure_OnByte(uint8_t b);
void Pressure_ProcessRx(void);
void Pressure_ProcessControllerRx(void);
void Pressure_PollBlocking(void);
uint16_t Pressure_GetValue(void);

#endif
