#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>

uint16_t Crc16_Modbus(const uint8_t *data, uint8_t len);
uint8_t Crc16_Check(const uint8_t *frame, uint8_t len);
void Crc16_Append(uint8_t *frame, uint8_t payload_len);

#endif

