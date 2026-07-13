#include "RI2C.h"
#include "delay.h"


#undef SUCCESS
#define SUCCESS 0
#undef FAILED
#define FAILED  1

/*************************************
/*函数名称：Wait()
/*函数功能：等待延时
/*输入参数：无
/*返回值：	无
*************************************/
void Wait()
{
	Delay_xus(2);
}

/*************************************
/*函数名称：Start()
/*函数功能：LED初始化函数
/*输入参数：无
/*返回值：	无
*************************************/
void I2C_Start(void)	//start the I2C, SDA High-to-low when SCL is high
{
	I2C_SDA = 1;
	Wait();
	I2C_SCL = 1;
	Wait();
	I2C_SDA = 0;
	Wait();
	I2C_SCL = 0;
	Wait();
}

/*************************************
/*函数名称：I2C_WriteAbyte(u8 dat)
/*函数功能：向I2C设备写入一个字节数据
/*输入参数：dat:写入的数据
/*返回值：	无
*************************************/
void I2C_WriteAbyte(u8 dat)
{
	u8 i;
	i = 8;
	do
	{
		if(dat & 0x80)  I2C_SDA = 1;
		else            I2C_SDA = 0;
		dat <<= 1;
		Wait();
		I2C_SCL = 1;
		Wait();
		I2C_SCL = 0;
		Wait();
	}
	while(--i);
}

/*************************************
/*函数名称：I2C_Check_ACK()
/*函数功能：LED初始化函数
/*					Check ACK, If F0=0, then right, if F0=1, then error
/*输入参数：无	
/*返回值：	无
*************************************/
void I2C_Check_ACK(void)
{
	I2C_SDA = 1;
	Wait();
	I2C_SCL = 1;
	Wait();
	CY  = I2C_SDA;
	I2C_SCL = 0;
	Wait();
}

/*************************************
/*函数名称：I2C_ReadAbyte(void)
/*函数功能：从I2C设备读一个字节
/*输入参数：无
/*返回值：	从I2C读取到的数据
*************************************/
u8 I2C_ReadAbyte(void)
{
	u8 i,dat;
	i = 8;
	I2C_SDA = 1;
	do
	{
		I2C_SCL = 1;
		Wait();
		dat <<= 1;
		if(I2C_SDA)		dat++;
		I2C_SCL  = 0;
		Wait();
	}
	while(--i);
	return(dat);
}

/*************************************
/*函数名称：SendACK()
/*函数功能：发送应答指令, Send ACK (LOW)
/*输入参数：无
/*返回值：	无
*************************************/
void SendACK(void)
{
	I2C_SDA = 0;
	Wait();
	I2C_SCL = 1;
	Wait();
	I2C_SCL = 0;
	Wait();
}

/*************************************
/*函数名称：SendNAK()
/*函数功能：发送无应答, Send No ACK (High)
/*输入参数：无
/*返回值：	无
*************************************/
void SendNAK(void)
{
	I2C_SDA = 1;
	Wait();
	I2C_SCL = 1;
	Wait();
	I2C_SCL = 0;
	Wait();
}

/*************************************
/*函数名称：I2C_Stop()
/*函数功能：I2C停止
/*输入参数：无
/*返回值：	无
*************************************/
void I2C_Stop(void)	//STOP the I2C, SDA Low-to-high when SCL is high
{
	I2C_SDA = 0;
	Wait();
	I2C_SCL = 1;
	Wait();
	I2C_SDA = 1;
	Wait();
}

/*************************************
/*函数名称：WriteNbyte(u8 addr, u8 *p, u8 number)
/*函数功能：LED初始化函数
/*输入参数：
/*返回值：	无
*************************************/
void WriteNbyte(u8 addr, u8 *p, u8 num)  /*  WordAddress,First Data Address,Byte lenth   */
{
	I2C_Start();							//发送起始命令
	I2C_WriteAbyte(I2C_Write);			//发送设备地址+写命令
	I2C_Check_ACK();
	I2C_WriteAbyte(addr);			//发送存储地址
	I2C_Check_ACK();
	do
	{
		I2C_WriteAbyte(*p);
		p++;
		I2C_Check_ACK();
	}
	while(--num);
	I2C_Stop();								//发送停止命令
}

/*************************************
/*函数名称：ReadNbyte(u8 addr, u8 *p, u8 number)
/*函数功能：LED初始化函数
/*输入参数：addr:
/*					p
/*返回值：	无
*************************************/
void ReadNbyte(u8 addr, u8 *p, u8 num)       /*  WordAddress,First Data Address,Byte lenth   */
{
	I2C_Start();									//发送起始命令
	I2C_WriteAbyte(I2C_Write);					//发送设备地址+写命令
	I2C_Check_ACK();
	I2C_WriteAbyte(addr);					//发送存储地址
	I2C_Check_ACK();
	I2C_Start();									//发送起始命令
	I2C_WriteAbyte(I2C_Read);					//发送设备地址+读命令
	I2C_Check_ACK();
	do
	{
		*p = I2C_ReadAbyte();	p++;
		if(num != 1)	SendACK();			//send ACK
	}
	while(--num);
	SendNAK();										//send no ACK
	I2C_Stop();										//发送停止命令
}


u8 IIC_Write_1Byte(u8 SlaveAddress,u8 REG_Address,u8 REG_data)
{
	I2C_Start();
    I2C_WriteAbyte(SlaveAddress<<1);
    I2C_Check_ACK();
    I2C_WriteAbyte(REG_Address);
    I2C_Check_ACK();
    I2C_WriteAbyte(REG_data);
    I2C_Check_ACK();
	I2C_Stop();
    return SUCCESS;
}


u8 IIC_Read_1Byte(u8 SlaveAddress,u8 REG_Address,u8 *REG_data)
{
    I2C_Start();
    I2C_WriteAbyte(SlaveAddress<<1);
    I2C_Check_ACK();
    I2C_WriteAbyte(REG_Address);
    I2C_Check_ACK();
	I2C_Stop();
    I2C_Start();
	I2C_WriteAbyte((SlaveAddress<<1)+1);
    I2C_Check_ACK();
	*REG_data = I2C_ReadAbyte();
	SendNAK();
	I2C_Stop();
	return SUCCESS;
}















