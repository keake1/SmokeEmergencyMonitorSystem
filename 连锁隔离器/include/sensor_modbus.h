#ifndef SENSOR_MODBUS_H
#define SENSOR_MODBUS_H

#include <stdint.h>

void SensorModbus_Init(void);
void SensorModbus_Process(uint8_t my_addr, uint16_t level);
#endif

