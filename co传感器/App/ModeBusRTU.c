#include "ModeBusRTU.h"
#include "stdlib.h"
#include "string.h"
#include "uart2.h"
#include "uart1.h"
#include "digital_cube.h"
double Time = 0;
unsigned char Res_cnt = 0;
/*�ӻ���ַ*/
uint8_t address;
/*���ջ���*/
uint8_t ModbusReceiveBuf[20];
uint8_t ModBusSendBuf_len;
/*���ͻ���*/
uint8_t ModBusSendBuf[20];
/*ģ��Ĵ���*/
uint16_t Reg[6] = {0};

uint8_t uart_send_flag = 0;


uint8_t cnt = 0;
uint16_t ModBus_CRC(uint8_t *addr,uint8_t num)
{
	int i, j, temp;
	uint16_t crc=0xFFFF;	
	for(i = 0; i < num; i++) {
		crc = crc ^ (*addr);
		for( j = 0; j < 8; j++)
		{
			temp = crc & 0x0001;
			crc = crc>>1;
			if(temp) {
				crc = crc ^ 0xA001;
			}
		}
		addr++;
	}
	return crc;
}


void ModBus_Send(void)
{
	uint8_t i = 0, j = 0;
	uint16_t Reg_address = (uint16_t)((ModbusReceiveBuf[2] << 8) | ModbusReceiveBuf[3]);
	uint16_t Reg_len = (uint16_t)((ModbusReceiveBuf[4] << 8) | ModbusReceiveBuf[5]);
	uint16_t crc = 0;
	ModBusSendBuf[i++] = ModbusReceiveBuf[0];
	ModBusSendBuf[i++] = 0x03;
	ModBusSendBuf[i++] = ((Reg_len * 2) % 256) + 1;
	ModBusSendBuf[i++] = 0x01;//型号
	for (j = 0; j < Reg_len; j++) {
		ModBusSendBuf[i++] = Reg[0x0001] / 256;
		ModBusSendBuf[i++] = Reg[0x0001] % 256;
	}
	crc = ModBus_CRC(ModBusSendBuf, i);
	ModBusSendBuf[i++] = crc % 256;
	ModBusSendBuf[i++] = crc / 256;
	ModBusSendBuf_len = i;
	uart_send_flag = 1;




}


void ModBus_write(void)
{
	uint16_t Reg_address = (uint16_t)((ModbusReceiveBuf[2] << 8) | ModbusReceiveBuf[3]);
	uint16_t receive_data = (uint16_t)((ModbusReceiveBuf[4] << 8) | ModbusReceiveBuf[5]);
	Reg[Reg_address] = receive_data;
}

uint8_t RxState = 0;        // ����״̬
uint8_t pRxPacket = 0;      // ��ǰ���յ���λ��
void ModBus_Clear(void)
{
	RxState = 0;
	pRxPacket = 0;
	memset(ModbusReceiveBuf, 0, sizeof(ModbusReceiveBuf)); 
}

void ModBus_ReadOneByte()
{
	uint8_t CRC_H = 0, CRC_L = 0;
	if ((ModbusReceiveBuf[0] == address || ModbusReceiveBuf[0] == 0xff) &&  Res_cnt >= 7) {
		
		if (ModBus_CRC(ModbusReceiveBuf, 6) == (ModbusReceiveBuf[7] << 8 | ModbusReceiveBuf[6])) {
			ModBus_handle();
			Res_cnt = 0;
			//P34 = ~P34;
		}
	}
	
	Res_cnt = 0;
	
}

void ModBus_handle()
{

	switch (ModbusReceiveBuf[1])
	{
	case 0x03:
		if (ModbusReceiveBuf[0] != 0xFF)
			
			ModBus_Send();
		break;
	case 0x06:
		
		ModBus_write();
		break;
	
	default:
		break;
	}


}


void UART2_int (void) interrupt UART2_VECTOR
{
	unsigned char temp;	
	Digital_Tube_flash();
    if((S2CON & 1) != 0)
    {
        S2CON &= ~1;    //Clear Rx flag
        temp =  S2BUF;
		
		//ModBus_ReadOneByte(temp);
		//ModbusReceiveBuf[Res_cnt++] = temp;
		cnt = 0;
    }
    if((S2CON & 2) != 0)
    {
        S2CON &= ~2;    //Clear Tx flag
		
		B_TX2_Busy = 0;
    }
}




void UART1_int (void) interrupt 4
{
	unsigned char temp;	
	Digital_Tube_flash();
    if(RI)
    {
		
        RI = 0;
        temp = SBUF;
		ModbusReceiveBuf[Res_cnt++] = SBUF;
        cnt = 0;
    }
    if(TI)
    {
        TI = 0;
        B_TX1_Busy = 0;
    }
}
