#include "ModeBusRTU.h"
#include "stdlib.h"
#include "string.h"
#include "uart2.h"
#include "uart1.h"
#include "digital_cube.h"

typedef struct {
    uint8_t valid;          // 是否校验成功
    uint16_t co2_ppm;    // CO2 浓度值
    uint8_t mode;        // 模式：1=主动输出，2=查询返回
} CO2_Data;
CO2_Data co2 = {0};
// 传感器数据结构体
typedef struct {
    uint16_t eCO2;
    uint16_t eCH2O;
    uint16_t TVOC;
    uint16_t PM2_5;
    uint16_t PM10;
    uint16_t Temperature;
    uint16_t Humidity;
    uint8_t valid; // 校验是否通过
} SensorData;
SensorData sensor = {0};
double Time = 0;
unsigned char Res_cnt = 0;
/*从机地址*/
uint8_t address;
/*接收缓存*/
uint8_t  ModbusReceiveBuf[20];
uint8_t ModBusSendBuf_len;
uint8_t sensorRawData[17];
/*发送缓存*/
uint8_t ModBusSendBuf[20];
/*模拟寄存器*/
uint16_t Reg[6] = {0};

uint8_t uart_send_flag = 0;

uint8_t cnt = 0;


/*
	co2传感器数据解析校验函数
*/
static uint8_t calc_sum_check(uint8_t *dd, uint8_t len)
{
    uint16_t sum = 0;
	uint8_t i = 0;
    for (i = 0; i < len - 1; i++)
        sum += dd[i];
    return (uint8_t)(sum & 0xFF);
}
// 校验计算函数（取低8位）7-1
uint8_t calc_checksum(const uint8_t *dd, int len) {
    uint16_t sum = 0;
	int i = 0;
    for (i = 0; i < len - 1; i++) {  // 不包含最后一位校验和
        sum += dd[i];
    }
    return (uint8_t)(sum & 0xFF); // 取低8位
}
uint16_t ModBus_CRC(uint8_t *addr,uint8_t num)
{
	
	int i, j, temp;
	
	uint16_t crc=0xFFFF;	
	EA = 0;
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
	EA = 1;
	return crc;
}

/*
	7合1传感器发送
*/
void ModBus_Send(void)
{
    uint8_t i = 0;
    uint16_t crc = 0;
		
    uint8_t func = 0x03;                  // 功能码：读保持寄存器
		// 温度和湿度：转为整数后发送（×10保留一位小数）
    uint16_t temp_send = sensor.Temperature;
    uint16_t humi_send = sensor.Humidity;
	
    // 写入地址和功能码
    ModBusSendBuf[i++] = address;
    ModBusSendBuf[i++] = func;

    // 数据字节数：每项2字节，共7项（eCO2、eCH2O、TVOC、PM2.5、PM10、温度、湿度）
    ModBusSendBuf[i++] = 15;
    ModBusSendBuf[i++] = 0x04; // 型号
    // ==== 依次写入各项传感器数据 ====
//		for(i=3;i<=16;i++)ModBusSendBuf[i]=sensorRawData[i-3];
    ModBusSendBuf[i++] = (uint8_t)(sensor.eCO2 >> 8);
    ModBusSendBuf[i++] = (uint8_t)(sensor.eCO2 & 0xFF);

    ModBusSendBuf[i++] = (uint8_t)(sensor.eCH2O >> 8);
    ModBusSendBuf[i++] = (uint8_t)(sensor.eCH2O & 0xFF);

    ModBusSendBuf[i++] = (uint8_t)(sensor.TVOC >> 8);
    ModBusSendBuf[i++] = (uint8_t)(sensor.TVOC & 0xFF);

    ModBusSendBuf[i++] = (uint8_t)(sensor.PM2_5 >> 8);
    ModBusSendBuf[i++] = (uint8_t)(sensor.PM2_5 & 0xFF);

    ModBusSendBuf[i++] = (uint8_t)(sensor.PM10 >> 8);
    ModBusSendBuf[i++] = (uint8_t)(sensor.PM10 & 0xFF);

    

    ModBusSendBuf[i++] = (uint8_t)(temp_send >> 8);
    ModBusSendBuf[i++] = (uint8_t)(temp_send & 0xFF);

    ModBusSendBuf[i++] = (uint8_t)(humi_send >> 8);
    ModBusSendBuf[i++] = (uint8_t)(humi_send & 0xFF);

    // ==== CRC校验 ====
    crc = ModBus_CRC(ModBusSendBuf, i);
    ModBusSendBuf[i++] = crc & 0xFF;   // CRC低字节
    ModBusSendBuf[i++] = crc >> 8;     // CRC高字节

    ModBusSendBuf_len = i;
    uart_send_flag = 1; // 通知主程序可以发送
}
/*
	co2传感器数据发送函数
*/
// void ModBus_Send(void)
// {
//     uint8_t i = 0;
//     uint16_t crc = 0;

//     uint8_t addr = ModbusReceiveBuf[0];   // 从请求帧获取设备地址
//     uint8_t func = 0x03;                  // 功能码：读保持寄存器

//     // ==== 1. 写入地址和功能码 ====
//     ModBusSendBuf[i++] = addr;
//     ModBusSendBuf[i++] = func;

//     // ==== 2. 数据字节数 ====
//     // 只发送一个16位数据（CO2浓度）
//     ModBusSendBuf[i++] = 2;

//     // ==== 3. 写入 CO2 数据 ====
//     ModBusSendBuf[i++] = (uint8_t)(co2.co2_ppm >> 8);   // 高字节
//     ModBusSendBuf[i++] = (uint8_t)(co2.co2_ppm & 0xFF); // 低字节

//     // ==== 4. CRC16 校验 ====
//     crc = ModBus_CRC(ModBusSendBuf, i);
//     ModBusSendBuf[i++] = crc & 0xFF;   // CRC低字节
//     ModBusSendBuf[i++] = crc >> 8;     // CRC高字节

//     // ==== 5. 更新发送标志 ====
//     ModBusSendBuf_len = i;
//     uart_send_flag = 1;  // 通知主循环可以发送
// }

void ModBus_write(void)
{
	uint16_t Reg_address = (uint16_t)((ModbusReceiveBuf[2] << 8) | ModbusReceiveBuf[3]);
	uint16_t receive_data = (uint16_t)((ModbusReceiveBuf[4] << 8) | ModbusReceiveBuf[5]);
	Reg[Reg_address] = receive_data;
}



uint8_t RxState = 0;        // 接收状态
uint8_t pRxPacket = 0;      // 当前接收到的位置
void ModBus_Clear(void)
{
	RxState = 0;
	pRxPacket = 0;
	memset(ModbusReceiveBuf, 0, sizeof(ModbusReceiveBuf)); 
}
void parse_co2_frame()
{
    co2.valid = 0;

    // -------------------------
    // 主动输出帧 (16 bytes)
    // -------------------------
    if (pRxPacket == 16 && sensorRawData[0] == 0x42 && sensorRawData[1] == 0x4D)
    {
        uint8_t sum = calc_sum_check(sensorRawData, 16);
        if (sum == sensorRawData[15])
        {
            co2.co2_ppm = (sensorRawData[6] << 8) | sensorRawData[7];
            co2.valid = 1;
            co2.mode = 1;  // 主动输出模式
        }
        else
        {
			return;
        }
    }

    // -------------------------
    // 查询返回帧 (14 bytes)
    // -------------------------
    else if (pRxPacket == 14 && sensorRawData[0] == 0x64 && sensorRawData[1] == 0x69)
    {
        // 提取CRC
        uint16_t crc_calc = calc_sum_check(sensorRawData, 12);
        uint16_t crc_recv = (sensorRawData[13] << 8) | sensorRawData[12];

        if (crc_calc == crc_recv)
        {
            co2.co2_ppm = (sensorRawData[5] << 8) | sensorRawData[4];
            co2.valid = 1;
            co2.mode = 2;  // 查询返回模式
        }
        else
        {
			return;
        }
    }
    else
    {
		return;
    }

}
unsigned int parse_sensor_frame() {
// 温度部分
    uint8_t temp_high = sensorRawData[12];
    uint8_t temp_low  = sensorRawData[13];
	uint8_t checksum = calc_checksum(sensorRawData, sizeof(sensorRawData));
	
	float temp = (float)temp_high + (temp_low / 10.0f);
    if (sizeof(sensorRawData) < 17 || sensorRawData[0] != 0x3C || sensorRawData[1] != 0x02) {
        sensor.valid = 0;
        pRxPacket=0;
        return 0;
    }

    if (checksum != sensorRawData[16]) {
        sensor.valid = 0;
        pRxPacket=0;
        return 0;
    }
		
    sensor.valid = 1;
    // 解析各项数据（高字节在前）
    sensor.eCO2  = (sensorRawData[2] << 8) | sensorRawData[3];
    sensor.eCH2O = (sensorRawData[4] << 8) | sensorRawData[5];
    sensor.TVOC  = (sensorRawData[6] << 8) | sensorRawData[7];
    sensor.PM2_5 = (sensorRawData[8] << 8) | sensorRawData[9];
    sensor.PM10  = (sensorRawData[10] << 8) | sensorRawData[11];
		
    
    
    if (temp_high & 0x80) { // bit7=1 表示负温度
        temp_high &= 0x7F;
        temp = -((float)temp_high + (temp_low / 10.0f));
    }
    sensor.Temperature = temp*10;
    // 湿度部分
    sensor.Humidity = (sensorRawData[14] + (sensorRawData[15] / 10.0f)) * 10;
    pRxPacket=0;
    return sensor.Humidity;
}

/*
	1ms处理串口接收到的消息
	modbus相关变量在此文件内作为全局变量，调用方便
*/
void ModBus_ReadOneByte()
{
	uint8_t CRC_H = 0, CRC_L = 0;
//	if ((ModbusReceiveBuf[0] == 1 || ModbusReceiveBuf[0] == 0xff) &&  Res_cnt >= 7) {
	if ((ModbusReceiveBuf[0] == address) &&  Res_cnt >= 7) {	
		if (ModBus_CRC(ModbusReceiveBuf, 6) == (ModbusReceiveBuf[7] << 8 | ModbusReceiveBuf[6])) {
			ModBus_handle();
			Res_cnt = 0;
		}
	}
	Res_cnt = 0;
}

void ModBus_handle()
{

	switch (ModbusReceiveBuf[1])
	{
	case 0x03:
		if (ModbusReceiveBuf[2] != 0x99)
		{
			ModBus_Send();
		}
		break;
	case 0x06:
		
		ModBus_write();
		break;
	
	default:
		break;
	}


}
/*
	实际使用中串口1，2只使用到一个，串口中断接受信息，填充到接收缓存/
	
*/
void UART2_int (void) interrupt UART2_VECTOR
{
	unsigned char temp;	
	Digital_Tube_flash();
    if((S2CON & 1) != 0)
    {
        S2CON &= ~1;    //Clear Rx flag
        temp =  S2BUF;
		//TX1_data(temp);
//		ModBus_ReadOneByte(temp);
			
		ModbusReceiveBuf[Res_cnt++] = temp;//注释掉一个
		// sensorRawData[pRxPacket++] = temp;
		
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
	Digital_Tube_flash();
    if(RI)
    {
		
        RI = 0;
//        temp = SBUF;
		//ModBus_ReadOneByte(temp);

		// ModbusReceiveBuf[Res_cnt++] = SBUF;//注释掉一个
        if(pRxPacket < sizeof(sensorRawData))
		{
			sensorRawData[pRxPacket++] = SBUF;
		}
        else pRxPacket=0;
		cnt = 0;
        
    }
    if(TI)
    {
        TI = 0;
        B_TX1_Busy = 0;
    }
}
