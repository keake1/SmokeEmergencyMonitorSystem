#ifndef _ADC_H
#define _ADC_H

#include "STC8H.h"
#include "delay.h"

//////////////////////////////////////////////////////
#define ADC_POWER   0x80            //ADC电源控制位
#define ADC_FLAG    0x20            //ADC转换结束标志
#define ADC_START   0x40            //ADC启动控制位
#define ADC_EPWMT   0x10            //使能PWM实时触发ADC功能位

//注意:ADC 的第15通道只能用于检测内部1.19V参考信号源
//ADC_CHS[3...0]	ADC通道选择
#define ADC_CH0			0x00						//ADC通道0—P1.0
#define ADC_CH1			0x01						//ADC通道1—P1.1
#define ADC_CH2			0x02						//ADC通道1—P1.2
#define ADC_CH3			0x03						//ADC通道1—P1.3
#define ADC_CH4			0x04						//ADC通道1—P1.4
#define ADC_CH5			0x05						//ADC通道1—P1.5
#define ADC_CH6			0x06						//ADC通道1—P1.6
#define ADC_CH7			0x07						//ADC通道1—P1.7
#define ADC_CH8			0x08						//ADC通道8—P0.0
#define ADC_CH9			0x09						//ADC通道9—P0.1
#define ADC_CH10		0x0a						//ADC通道10—P0.2
#define ADC_CH11		0x0b						//ADC通道11—P0.3
#define ADC_CH15		0x0f						//ADC通道15-1.19V

//ADC_CFG	
//B7	B6	B5			B4	B3	B2	B1	B0
//-		-		RESFMT	-		 SPEED[3...0]
//SPEED[3...0]
//F[ADC]=SYSclk/2/(SPEED+1)
#define ADC_SPEED0  0x00          	//32个时钟
#define ADC_SPEED1  0x01          	//64个时钟
#define ADC_SPEED2  0x02          	//96个时钟
#define ADC_SPEED3	0x03         		//128个时钟
#define ADC_SPEED4	0x04        	  //160个时钟
#define ADC_SPEED5	0x05          	//192个时钟
#define ADC_SPEED6	0x06          	//224个时钟
#define ADC_SPEED7	0x07          	//256个时钟
#define ADC_SPEED8	0x08          	//288个时钟
#define ADC_SPEED9 	0x09          	//320个时钟
#define ADC_SPEED10 0x0a          	//352个时钟
#define ADC_SPEED11 0x0b          	//384个时钟
#define ADC_SPEED12 0x0c          	//416个时钟
#define ADC_SPEED13 0x0d          	//448个时钟
#define ADC_SPEED14 0x0e          	//480个时钟
#define ADC_SPEED15 0x0f          	//512个时钟

//ADC结果格式	0: 左对齐, ADC_RES: D11 D10 D9 D8 D7 D6 D5 D4, ADC_RESL: D3 D2 D1 D0 0  0  0  0
//ADCCFG				1: 右对齐, ADC_RES: 0  0  0  0 D11 D10 D9 D8,  ADC_RESL: D7 D6 D5 D4 D3 D2 D1 D0
//#define	ADC_CFG_RESFMT_L()	RESFMT = 0;	//ADC转换结果左对齐
//#define	ADC_CFG_RESFMT_R()	RESFMT = 1;	//ADC转换结果右对齐
#define ADC_RESFMTL 0x00
#define ADC_RESFMTR 0x20

//ADCTIM
//ADC时序控制
//B7				B6	B5				B4	B3	B2	B1	B0
//CSSETUP		CSHOLD[1:0]		   SMPDUTY[4:0]	
//CSSETUP:ADC通道选择时间
//=0,占用1个ADC工作时钟(默认值); =1,占用2个ADC工作时钟
//CSHOLD[1:0]:ADC通道选择保持时间
//00:占用1个工作时钟
//01:占用2个工作时钟(默认值)
//10:占用3个工作时钟
//11:占用4个工作时钟
//SMPDUTY[4:0]:ADC模拟信号采样时间(注意：一定不要设置小于01010B[默认值])
//建议选择最大值


/*函数声明*/
void ADC_IO_Config();
void ADC_Init();
u16 Get_ADC10bitResult(u8 ch);

#endif

