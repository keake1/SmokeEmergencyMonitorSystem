#ifndef __MODEBUSRTU_H__
#define __MODEBUSRTU_H__

#include "STC8H.h"

#define uint8_t unsigned char
#define uint16_t unsigned int


extern uint8_t ModbusReceiveBuf[20];
extern uint8_t ModBusSendBuf[20];
extern uint8_t ModBusSendBuf_len;
extern uint8_t uart_send_flag;
extern uint8_t address;
extern uint16_t Reg[6];
extern volatile double Time;
extern uint8_t cnt;
uint16_t ModBus_CRC(uint8_t *addr,uint8_t num);
void ModBus_Send(void);
void ModBus_ReadOneByte();
void ModBus_handle(void);
void ModBus_Clear(void);
extern unsigned char Res_cnt;
#endif