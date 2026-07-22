#ifndef _RI2C_H
#define _RI2C_H

#include "STC8H.h"

//
sbit I2C_SDA = P3^3; //定义SDA
sbit I2C_SCL = P3^2; //定义SCL

//I2C操作指令
#define I2C_Write		0x5A
#define I2C_Read		0x5B 


/*函数声明*/
void Wait();
void Start();
void SendData(char dat);
void RecvACK();
char RecvData();
void SendACK();
void SendNAK();
void Stop();
void WriteNbyte(u8 addr, u8 *p, u8 number);
void ReadNbyte( u8 addr, u8 *p, u8 number);
u8 IIC_Write_1Byte(u8 SlaveAddress,u8 REG_Address,u8 REG_data);
u8 IIC_Read_1Byte(u8 SlaveAddress,u8 REG_Address,u8 *REG_data);
#endif

