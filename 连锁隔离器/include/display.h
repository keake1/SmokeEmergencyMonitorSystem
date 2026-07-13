#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

void Display_Init(void);
void Display_SetValue(uint16_t value);
void Display_ScanOnce(void);
void Display_ShowRawDigit(uint8_t digit, uint8_t pattern);
void Display_TestAllOn(void);
void Display_TestPolarity(uint8_t mode);

#endif
