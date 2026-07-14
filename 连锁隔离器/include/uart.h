#ifndef UART_H
#define UART_H

#include <stdint.h>

void Uart_Init(void);

void Uart2_SendByte(uint8_t value);
uint8_t Uart2_ReadByte(uint8_t *value);

void Uart2_Send(const uint8_t *data, uint8_t len);

#endif
