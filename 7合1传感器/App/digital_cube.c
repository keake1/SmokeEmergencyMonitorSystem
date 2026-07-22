#include "digital_cube.h"

unsigned char display_buf[3] = {0};

code unsigned char data_[] = {
	0x3F,  //"0"
	0x06,  //"1"
	0x5B,  //"2"
	0x4F,  //"3"
	0x66,  //"4"
	0x6D,  //"5"
	0x7D,  //"6"
	0x07,  //"7"
	0x7F,  //"8"
	0x6F,  //"9"
	0x77,  //"A"
	0x7C,  //"B"
	0x39,  //"C"
	0x5E,  //"D"
	0x79,  //"E"
	0x71,  //"F"
	0x76,  //"H"
	0x38,  //"L"
	0x37,  //"n"
	0x3E,  //"u"
	0x73,  //"P"
	0x5C,  //"o"
	0x40,  //"-"
	0x00   //空
};


void digital_cube_init(void)
{


    // 配置P0.1为推挽输出
    P0M0 |= 0x01 << 1;
    P0M1 &= ~(0x01 << 1);
    P01 = 0;  // DIG3

    // 配置P2.6为推挽输出
    P2M0 |= 0x01 << 6;
    P2M1 &= ~(0x01 << 6);
    P26 = 0;  // DIG2

    // 配置P2.3为推挽输出
    P2M0 |= 0x01 << 3;
    P2M1 &= ~(0x01 << 3);
    P23 = 0;  // DIG1

    // 配置P2.4为推挽输出
    P2M0 |= 0x01 << 4;
    P2M1 &= ~(0x01 << 4);
    P24 = 0;  // LEDA

    // 配置P2.7为推挽输出
    P2M0 |= 0x01 << 7;
    P2M1 &= ~(0x01 << 7);
    P27 = 0;  // LEDB

    // 配置P3.6为推挽输出
    P3M0 |= 0x01 << 6;
    P3M1 &= ~(0x01 << 6);
    P36 = 0;  // LEDC

    // 配置P2.0为推挽输出
    P2M0 |= 0x01 << 0;
    P2M1 &= ~(0x01 << 0);
    P20 = 0;  // LEDD

    // 配置P2.1为推挽输出
    P2M0 |= 0x01 << 1;
    P2M1 &= ~(0x01 << 1);
    P21 = 0;  // LEDE

    // 配置P2.5为推挽输出
    P2M0 |= 0x01 << 5;
    P2M1 &= ~(0x01 << 5);
    P25 = 0;  // LEDF

    // 配置P3.5为推挽输出
    P3M0 |= 0x01 << 5;
    P3M1 &= ~(0x01 << 5);
    P35 = 0;  // LEDG

    P37 = 0;

}

void digital_cube_ShowOne(unsigned char num)
{
    // 根据 data[num] 的每一位设置引脚
    P24 = (data_[num] & 0x01) ;
    P27 = (data_[num] & 0x02) ;
    P36 = (data_[num] & 0x04) ;
    P20 = (data_[num] & 0x08) ;
    P21 = (data_[num] & 0x10) ;
    P25 = (data_[num] & 0x20) ;
    P35 = (data_[num] & 0x40) ;
}


void Digital_Tube_Show(unsigned char num, unsigned char loc)
{
	
	P01 = 1;
    P26 = 1;
    P23 = 1;
    switch (loc)
    {
    case 0:
        P01 = 1;
		P26 = 1;
		P23 = 0;
        break;
    case 1:
        P01 = 1;
		P26 = 0;
		P23 = 1;
        break;
    case 2:
        P01 = 0;
		P26 = 1;
		P23 = 1;
        break;
    
    default:
        break;
    }
    digital_cube_ShowOne(num);
}

void Digital_Tube_ShowNum(int num)
{
	if (num >= 0) {
		display_buf[0] = (num / 100);
		display_buf[1] = (num / 10) % 10;
		display_buf[2] = (num % 10);
	}
	else {
		display_buf[0] = 22;
		display_buf[1] = -num / 10;
		display_buf[2] = -num % 10;
	}
		
}


void Digital_Tube_flash(void)
{
	
	static unsigned char p = 0;
	Digital_Tube_Show(display_buf[p], p);
	p++;
	if (p >= 3)
		p = 0;

}

