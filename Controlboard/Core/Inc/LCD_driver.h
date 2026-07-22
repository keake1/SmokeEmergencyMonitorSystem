#ifndef __LCD_DRIVER_H__
#define __LCD_DRIVER_H__
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

#define OLED_EN_DIS_BUFFER  0

typedef unsigned char uchar;
typedef unsigned int uint;

/* ==================== 引脚宏（适配 CubeMX 命名） ==================== */

/* RES — 复位（PA1） */
#define LCD_RES_H()  do { HAL_GPIO_WritePin(LCD_RES_GPIO_Port, LCD_RES_Pin, GPIO_PIN_SET);   vTaskDelay(pdMS_TO_TICKS(1)); } while(0)
#define LCD_RES_L()  do { HAL_GPIO_WritePin(LCD_RES_GPIO_Port, LCD_RES_Pin, GPIO_PIN_RESET); vTaskDelay(pdMS_TO_TICKS(1)); } while(0)

/* CS — 片选（PA4） */
#define LCD_CS_H()  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)
#define LCD_CS_L()  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)

/* CLK — SPI 时钟（PA5） */
#define LCD_CLK_H() HAL_GPIO_WritePin(LCD_SCL_GPIO_Port, LCD_SCL_Pin, GPIO_PIN_SET)
#define LCD_CLK_L() HAL_GPIO_WritePin(LCD_SCL_GPIO_Port, LCD_SCL_Pin, GPIO_PIN_RESET)

/* A0 — 数据/命令选择（PA6，高=数据，低=命令） */
#define LCD_RS_H()  HAL_GPIO_WritePin(LCD_A0_GPIO_Port, LCD_A0_Pin, GPIO_PIN_SET)
#define LCD_RS_L()  HAL_GPIO_WritePin(LCD_A0_GPIO_Port, LCD_A0_Pin, GPIO_PIN_RESET)

/* SID — SPI 数据（PA7） */
#define LCD_SDA_H() HAL_GPIO_WritePin(LCD_SID_GPIO_Port, LCD_SID_Pin, GPIO_PIN_SET)
#define LCD_SDA_L() HAL_GPIO_WritePin(LCD_SID_GPIO_Port, LCD_SID_Pin, GPIO_PIN_RESET)

#define LCD_Delayms(x)  vTaskDelay(pdMS_TO_TICKS(x))

#define REG_DATA	1
#define REG_CMD		0


void LCD_Init(void);
void LCD_Clear(void);
void LCD_Clear_2(void);
void LCD_Set_Buffer(int   x, int   y , uint8_t *dat , uint16_t size);
void LCD_DispChar8x16(uint8_t _ucPage,uint8_t _ucColumn,uint8_t *p);
void LCD_DispStr8x16(uint8_t _ucPage,uint8_t _ucColumn,char *str);
void LCD_DispNum(uint8_t _ucPage,uint8_t _ucColumn,uint16_t num,uint8_t _ucLenth);

void LCD_Dis_String(int  x,int   y,   char disMode, char  *string);
void LCD_Dis_Date(int  x,int   y,   char disMode, char  *date , uint16_t  len);
void LCD_Printf(int  x,int  y, char disMode , char *p,... );
void OLED_ShowCN(uint8_t Line, uint8_t Column, uint8_t Num);
#endif
