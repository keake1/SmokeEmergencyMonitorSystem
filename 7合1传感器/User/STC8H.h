#ifndef     __STC8H_H__
#define     __STC8H_H__

/////////////////////////////////////////////////

/////////////////////////////////////////////////
//STC8H系列芯片使用注意事项：
//除P3.0/P3.1外，其余所有I/O口上电后状态均为高阻输入状态，用户在使用I/O口时必须先设置I/O口模式
/////////////////////////////////////////////////

//包含本头文件后,不用另外再包含"REG51.H"
#include "intrins.h"


sfr         P0          =           0x80;
sbit        P00         =           P0^0;
sbit        P01         =           P0^1;
sbit        P02         =           P0^2;
sbit        P03         =           P0^3;
sbit        P04         =           P0^4;
sbit        P05         =           P0^5;
sbit        P06         =           P0^6;
sbit        P07         =           P0^7;
sfr         SP          =           0x81;
sfr         DPL         =           0x82;
sfr         DPH         =           0x83;
sfr         S4CON       =           0x84;
sfr         S4BUF       =           0x85;
sfr         PCON        =           0x87;
sfr         TCON        =           0x88;
sbit        TF1         =           TCON^7;
sbit        TR1         =           TCON^6;
sbit        TF0         =           TCON^5;
sbit        TR0         =           TCON^4;
sbit        IE1         =           TCON^3;
sbit        IT1         =           TCON^2;
sbit        IE0         =           TCON^1;
sbit        IT0         =           TCON^0;
sfr         TMOD        =           0x89;
sfr         TL0         =           0x8a;
sfr         TL1         =           0x8b;
sfr         TH0         =           0x8c;
sfr         TH1         =           0x8d;
sfr         AUXR        =           0x8e;
sfr         INTCLKO     =           0x8f;
sfr         P1          =           0x90;
sbit        P10         =           P1^0;
sbit        P11         =           P1^1;
sbit        P12         =           P1^2;
sbit        P13         =           P1^3;
sbit        P14         =           P1^4;
sbit        P15         =           P1^5;
sbit        P16         =           P1^6;
sbit        P17         =           P1^7;
sfr         P1M1        =           0x91;
sfr         P1M0        =           0x92;
sfr         P0M1        =           0x93;
sfr         P0M0        =           0x94;
sfr         P2M1        =           0x95;
sfr         P2M0        =           0x96;
sfr         SCON        =           0x98;
sbit        SM0         =           SCON^7;
sbit        SM1         =           SCON^6;
sbit        SM2         =           SCON^5;
sbit        REN         =           SCON^4;
sbit        TB8         =           SCON^3;
sbit        RB8         =           SCON^2;
sbit        TI          =           SCON^1;
sbit        RI          =           SCON^0;
sfr         SBUF        =           0x99;
sfr         S2CON       =           0x9a;
sfr         S2BUF       =           0x9b;
sfr         IRCBAND     =           0x9d;
sfr         LIRTRIM     =           0x9e;
sfr         IRTRIM      =           0x9f;
sfr         P2          =           0xa0;
sbit        P20         =           P2^0;
sbit        P21         =           P2^1;
sbit        P22         =           P2^2;
sbit        P23         =           P2^3;
sbit        P24         =           P2^4;
sbit        P25         =           P2^5;
sbit        P26         =           P2^6;
sbit        P27         =           P2^7;
sfr         P_SW1       =           0xa2;
sfr         IE          =           0xa8;
sbit        EA          =           IE^7;
sbit        ELVD        =           IE^6;
sbit        EADC        =           IE^5;
sbit        ES          =           IE^4;
sbit        ET1         =           IE^3;
sbit        EX1         =           IE^2;
sbit        ET0         =           IE^1;
sbit        EX0         =           IE^0;
sfr         SADDR       =           0xa9;
sfr         WKTCL       =           0xaa;
sfr         WKTCH       =           0xab;
sfr         S3CON       =           0xac;
sfr         S3BUF       =           0xad;
sfr         TA          =           0xae;
sfr         IE2         =           0xaf;
sfr         P3          =           0xb0;
sbit        P30         =           P3^0;
sbit        P31         =           P3^1;
sbit        P32         =           P3^2;
sbit        P33         =           P3^3;
sbit        P34         =           P3^4;
sbit        P35         =           P3^5;
sbit        P36         =           P3^6;
sbit        P37         =           P3^7;
sfr         P3M1        =           0xb1;
sfr         P3M0        =           0xb2;
sfr         P4M1        =           0xb3;
sfr         P4M0        =           0xb4;
sfr         IP2         =           0xb5;
sfr         IP2H        =           0xb6;
sfr         IPH         =           0xb7;
sfr         IP          =           0xb8;
sbit        PPCA        =           IP^7;
sbit        PLVD        =           IP^6;
sbit        PADC        =           IP^5;
sbit        PS          =           IP^4;
sbit        PT1         =           IP^3;
sbit        PX1         =           IP^2;
sbit        PT0         =           IP^1;
sbit        PX0         =           IP^0;
sfr         SADEN       =           0xb9;
sfr         P_SW2       =           0xba;
sfr         ADC_CONTR   =           0xbc;
sfr         ADC_RES     =           0xbd;
sfr         ADC_RESL    =           0xbe;
sfr         P4          =           0xc0;
sbit        P40         =           P4^0;
sbit        P41         =           P4^1;
sbit        P42         =           P4^2;
sbit        P43         =           P4^3;
sbit        P44         =           P4^4;
sbit        P45         =           P4^5;
sbit        P46         =           P4^6;
sbit        P47         =           P4^7;
sfr         WDT_CONTR   =           0xc1;
sfr         IAP_DATA    =           0xc2;
sfr         IAP_ADDRH   =           0xc3;
sfr         IAP_ADDRL   =           0xc4;
sfr         IAP_CMD     =           0xc5;
sfr         IAP_TRIG    =           0xc6;
sfr         IAP_CONTR   =           0xc7;
sfr         P5          =           0xc8;
sbit        P50         =           P5^0;
sbit        P51         =           P5^1;
sbit        P52         =           P5^2;
sbit        P53         =           P5^3;
sbit        P54         =           P5^4;
sbit        P55         =           P5^5;
sbit        P56         =           P5^6;
sbit        P57         =           P5^7;
sfr         P5M1        =           0xc9;
sfr         P5M0        =           0xca;
sfr         P6M1        =           0xcb;
sfr         P6M0        =           0xcc;
sfr         SPSTAT      =           0xcd;
sfr         SPCTL       =           0xce;
sfr         SPDAT       =           0xcf;
sfr         PSW         =           0xd0;
sbit        CY          =           PSW^7;
sbit        AC          =           PSW^6;
sbit        F0          =           PSW^5;
sbit        RS1         =           PSW^4;
sbit        RS0         =           PSW^3;
sbit        OV          =           PSW^2;
sbit        F1          =           PSW^1;
sbit        P           =           PSW^0;
sfr         T4T3M       =           0xd1;
sfr         T4H         =           0xd2;
sfr         T4L         =           0xd3;
sfr         T3H         =           0xd4;
sfr         T3L         =           0xd5;
sfr         T2H         =           0xd6;
sfr         T2L         =           0xd7;
sfr         USBCLK      =           0xdc;
sfr         ADCCFG      =           0xde;
sfr         IP3         =           0xdf;
sfr         ACC         =           0xe0;
sfr         P7M1        =           0xe1;
sfr         P7M0        =           0xe2;
sfr         DPS         =           0xe3;
sfr         DPL1        =           0xe4;
sfr         DPH1        =           0xe5;
sfr         CMPCR1      =           0xe6;
sfr         CMPCR2      =           0xe7;
sfr         P6          =           0xe8;
sfr         USBDAT      =           0xec;
sfr         IP3H        =           0xee;
sfr         AUXINTIF    =           0xef;
sfr         B           =           0xf0;
sfr         USBCON      =           0xf4;
sfr         IAP_TPS     =           0xf5;
sfr         P7          =           0xf8;
sfr         USBADR      =           0xfc;
sfr         RSTCFG      =           0xff;

//如下特殊功能寄存器位于扩展RAM区域
//访问这些寄存器,需先将P_SW2的BIT7设置为1,才可正常读写

/////////////////////////////////////////////////
//FE00H-FEFFH
/////////////////////////////////////////////////

#define     CKSEL       (*(unsigned char volatile xdata *)0xfe00)
#define     CLKDIV      (*(unsigned char volatile xdata *)0xfe01)
#define     HIRCCR      (*(unsigned char volatile xdata *)0xfe02)
#define     XOSCCR      (*(unsigned char volatile xdata *)0xfe03)
#define     IRC32KCR    (*(unsigned char volatile xdata *)0xfe04)
#define     MCLKOCR     (*(unsigned char volatile xdata *)0xfe05)
#define     IRCDB       (*(unsigned char volatile xdata *)0xfe06)
#define     X32KCR      (*(unsigned char volatile xdata *)0xfe08)

#define     P0PU        (*(unsigned char volatile xdata *)0xfe10)
#define     P1PU        (*(unsigned char volatile xdata *)0xfe11)
#define     P2PU        (*(unsigned char volatile xdata *)0xfe12)
#define     P3PU        (*(unsigned char volatile xdata *)0xfe13)
#define     P4PU        (*(unsigned char volatile xdata *)0xfe14)
#define     P5PU        (*(unsigned char volatile xdata *)0xfe15)
#define     P6PU        (*(unsigned char volatile xdata *)0xfe16)
#define     P7PU        (*(unsigned char volatile xdata *)0xfe17)
#define     P0NCS       (*(unsigned char volatile xdata *)0xfe18)
#define     P1NCS       (*(unsigned char volatile xdata *)0xfe19)
#define     P2NCS       (*(unsigned char volatile xdata *)0xfe1a)
#define     P3NCS       (*(unsigned char volatile xdata *)0xfe1b)
#define     P4NCS       (*(unsigned char volatile xdata *)0xfe1c)
#define     P5NCS       (*(unsigned char volatile xdata *)0xfe1d)
#define     P6NCS       (*(unsigned char volatile xdata *)0xfe1e)
#define     P7NCS       (*(unsigned char volatile xdata *)0xfe1f)
#define     P0SR        (*(unsigned char volatile xdata *)0xfe20)
#define     P1SR        (*(unsigned char volatile xdata *)0xfe21)
#define     P2SR        (*(unsigned char volatile xdata *)0xfe22)
#define     P3SR        (*(unsigned char volatile xdata *)0xfe23)
#define     P4SR        (*(unsigned char volatile xdata *)0xfe24)
#define     P5SR        (*(unsigned char volatile xdata *)0xfe25)
#define     P6SR        (*(unsigned char volatile xdata *)0xfe26)
#define     P7SR        (*(unsigned char volatile xdata *)0xfe27)
#define     P0DR        (*(unsigned char volatile xdata *)0xfe28)
#define     P1DR        (*(unsigned char volatile xdata *)0xfe29)
#define     P2DR        (*(unsigned char volatile xdata *)0xfe2a)
#define     P3DR        (*(unsigned char volatile xdata *)0xfe2b)
#define     P4DR        (*(unsigned char volatile xdata *)0xfe2c)
#define     P5DR        (*(unsigned char volatile xdata *)0xfe2d)
#define     P6DR        (*(unsigned char volatile xdata *)0xfe2e)
#define     P7DR        (*(unsigned char volatile xdata *)0xfe2f)
#define     P0IE        (*(unsigned char volatile xdata *)0xfe30)
#define     P1IE        (*(unsigned char volatile xdata *)0xfe31)
#define     P2IE        (*(unsigned char volatile xdata *)0xfe32)
#define     P3IE        (*(unsigned char volatile xdata *)0xfe33)
#define     P4IE        (*(unsigned char volatile xdata *)0xfe34)
#define     P5IE        (*(unsigned char volatile xdata *)0xfe35)
#define     P6IE        (*(unsigned char volatile xdata *)0xfe36)
#define     P7IE        (*(unsigned char volatile xdata *)0xfe37)

#define     I2CCFG      (*(unsigned char volatile xdata *)0xfe80)
#define     I2CMSCR     (*(unsigned char volatile xdata *)0xfe81)
#define     I2CMSST     (*(unsigned char volatile xdata *)0xfe82)
#define     I2CSLCR     (*(unsigned char volatile xdata *)0xfe83)
#define     I2CSLST     (*(unsigned char volatile xdata *)0xfe84)
#define     I2CSLADR    (*(unsigned char volatile xdata *)0xfe85)
#define     I2CTXD      (*(unsigned char volatile xdata *)0xfe86)
#define     I2CRXD      (*(unsigned char volatile xdata *)0xfe87)
#define     I2CMSAUX    (*(unsigned char volatile xdata *)0xfe88)
#define     TM2PS       (*(unsigned char volatile xdata *)0xfea2)
#define     TM3PS       (*(unsigned char volatile xdata *)0xfea3)
#define     TM4PS       (*(unsigned char volatile xdata *)0xfea4)
#define     ADCTIM      (*(unsigned char volatile xdata *)0xfea8)
#define     T3T4PIN     (*(unsigned char volatile xdata *)0xfeac)


#define PWMA_ETRPS (*(unsigned char volatile xdata *)0xfeb0)
#define PWMA_ENO   (*(unsigned char volatile xdata *)0xfeb1)
#define PWMA_PS    (*(unsigned char volatile xdata *)0xfeb2)
#define PWMA_IOAUX (*(unsigned char volatile xdata *)0xfeb3)
#define PWMB_ETRPS (*(unsigned char volatile xdata *)0xfeb4)
#define PWMB_ENO   (*(unsigned char volatile xdata *)0xfeb5)
#define PWMB_PS    (*(unsigned char volatile xdata *)0xfeb6)
#define PWMB_IOAUX (*(unsigned char volatile xdata *)0xfeb7)
#define PWMA_CR1   (*(unsigned char volatile xdata *)0xfec0)
#define PWMA_CR2   (*(unsigned char volatile xdata *)0xfec1)
#define PWMA_SMCR  (*(unsigned char volatile xdata *)0xfec2)
#define PWMA_ETR   (*(unsigned char volatile xdata *)0xfec3)
#define PWMA_IER   (*(unsigned char volatile xdata *)0xfec4)
#define PWMA_SR1   (*(unsigned char volatile xdata *)0xfec5)
#define PWMA_SR2   (*(unsigned char volatile xdata *)0xfec6)
#define PWMA_EGR   (*(unsigned char volatile xdata *)0xfec7)
#define PWMA_CCMR1 (*(unsigned char volatile xdata *)0xfec8)
#define PWMA_CCMR2 (*(unsigned char volatile xdata *)0xfec9)
#define PWMA_CCMR3 (*(unsigned char volatile xdata *)0xfeca)
#define PWMA_CCMR4 (*(unsigned char volatile xdata *)0xfecb)
#define PWMA_CCER1 (*(unsigned char volatile xdata *)0xfecc)
#define PWMA_CCER2 (*(unsigned char volatile xdata *)0xfecd)
#define PWMA_CNTR  (*(unsigned int volatile xdata *)0xfece)
#define PWMA_CNTRH (*(unsigned char volatile xdata *)0xfece)
#define PWMA_CNTRL (*(unsigned char volatile xdata *)0xfecf)
#define PWMA_PSCR  (*(unsigned int volatile xdata *)0xfed0)
#define PWMA_PSCRH (*(unsigned char volatile xdata *)0xfed0)
#define PWMA_PSCRL (*(unsigned char volatile xdata *)0xfed1)
#define PWMA_ARR   (*(unsigned int volatile xdata *)0xfed2)
#define PWMA_ARRH  (*(unsigned char volatile xdata *)0xfed2)
#define PWMA_ARRL  (*(unsigned char volatile xdata *)0xfed3)
#define PWMA_RCR   (*(unsigned char volatile xdata *)0xfed4)
#define PWMA_CCR1  (*(unsigned int volatile xdata *)0xfed5)
#define PWMA_CCR1H (*(unsigned char volatile xdata *)0xfed5)
#define PWMA_CCR1L (*(unsigned char volatile xdata *)0xfed6)
#define PWMA_CCR2  (*(unsigned int volatile xdata *)0xfed7)
#define PWMA_CCR2H (*(unsigned char volatile xdata *)0xfed7)
#define PWMA_CCR2L (*(unsigned char volatile xdata *)0xfed8)
#define PWMA_CCR3  (*(unsigned int volatile xdata *)0xfed9)
#define PWMA_CCR3H (*(unsigned char volatile xdata *)0xfed9)
#define PWMA_CCR3L (*(unsigned char volatile xdata *)0xfeda)
#define PWMA_CCR4  (*(unsigned int volatile xdata *)0xfedb)
#define PWMA_CCR4H (*(unsigned char volatile xdata *)0xfedb)
#define PWMA_CCR4L (*(unsigned char volatile xdata *)0xfedc)
#define PWMA_BRK   (*(unsigned char volatile xdata *)0xfedd)
#define PWMA_DTR   (*(unsigned char volatile xdata *)0xfede)
#define PWMA_OISR  (*(unsigned char volatile xdata *)0xfedf)

#define PWMB_CR1   (*(unsigned char volatile xdata *)0xfee0)
#define PWMB_CR2   (*(unsigned char volatile xdata *)0xfee1)
#define PWMB_SMCR  (*(unsigned char volatile xdata *)0xfee2)
#define PWMB_ETR   (*(unsigned char volatile xdata *)0xfee3)
#define PWMB_IER   (*(unsigned char volatile xdata *)0xfee4)
#define PWMB_SR1   (*(unsigned char volatile xdata *)0xfee5)
#define PWMB_SR2   (*(unsigned char volatile xdata *)0xfee6)
#define PWMB_EGR   (*(unsigned char volatile xdata *)0xfee7)
#define PWMB_CCMR1 (*(unsigned char volatile xdata *)0xfee8)
#define PWMB_CCMR2 (*(unsigned char volatile xdata *)0xfee9)
#define PWMB_CCMR3 (*(unsigned char volatile xdata *)0xfeea)
#define PWMB_CCMR4 (*(unsigned char volatile xdata *)0xfeeb)
#define PWMB_CCER1 (*(unsigned char volatile xdata *)0xfeec)
#define PWMB_CCER2 (*(unsigned char volatile xdata *)0xfeed)
#define PWMB_CNTR  (*(unsigned int volatile xdata *)0xfeee)
#define PWMB_CNTRH (*(unsigned char volatile xdata *)0xfeee)
#define PWMB_CNTRL (*(unsigned char volatile xdata *)0xfeef)
#define PWMB_PSCR  (*(unsigned int volatile xdata *)0xfef0)
#define PWMB_PSCRH (*(unsigned char volatile xdata *)0xfef0)
#define PWMB_PSCRL (*(unsigned char volatile xdata *)0xfef1)
#define PWMB_ARR   (*(unsigned int volatile xdata *)0xfef2)
#define PWMB_ARRH  (*(unsigned char volatile xdata *)0xfef2)
#define PWMB_ARRL  (*(unsigned char volatile xdata *)0xfef3)
#define PWMB_RCR   (*(unsigned char volatile xdata *)0xfef4)
#define PWMB_CCR5  (*(unsigned int volatile xdata *)0xfef5)
#define PWMB_CCR5H (*(unsigned char volatile xdata *)0xfef5)
#define PWMB_CCR5L (*(unsigned char volatile xdata *)0xfef6)
#define PWMB_CCR6  (*(unsigned int volatile xdata *)0xfef7)
#define PWMB_CCR6H (*(unsigned char volatile xdata *)0xfef7)
#define PWMB_CCR6L (*(unsigned char volatile xdata *)0xfef8)
#define PWMB_CCR7  (*(unsigned int volatile xdata *)0xfef9)
#define PWMB_CCR7H (*(unsigned char volatile xdata *)0xfef9)
#define PWMB_CCR7L (*(unsigned char volatile xdata *)0xfefa)
#define PWMB_CCR8  (*(unsigned int volatile xdata *)0xfefb)
#define PWMB_CCR8H (*(unsigned char volatile xdata *)0xfefb)
#define PWMB_CCR8L (*(unsigned char volatile xdata *)0xfefc)
#define PWMB_BRK   (*(unsigned char volatile xdata *)0xfefd)
#define PWMB_DTR   (*(unsigned char volatile xdata *)0xfefe)
#define PWMB_OISR  (*(unsigned char volatile xdata *)0xfeff)

/////////////////////////////////////////////////
//FD00H-FDFFH
/////////////////////////////////////////////////

#define     PWM0C       (*(unsigned int volatile xdata *)0xff00)
#define     PWM0CH      (*(unsigned char volatile xdata *)0xff00)
#define     PWM0CL      (*(unsigned char volatile xdata *)0xff01)
#define     PWM0CKS     (*(unsigned char volatile xdata *)0xff02)
#define     PWM0TADC    (*(unsigned int volatile xdata *)0xff03)
#define     PWM0TADCH   (*(unsigned char volatile xdata *)0xff03)
#define     PWM0TADCL   (*(unsigned char volatile xdata *)0xff04)
#define     PWM0IF      (*(unsigned char volatile xdata *)0xff05)
#define     PWM0FDCR    (*(unsigned char volatile xdata *)0xff06)
#define     PWM00T1     (*(unsigned int volatile xdata *)0xff10)
#define     PWM00T1H    (*(unsigned char volatile xdata *)0xff10)
#define     PWM00T1L    (*(unsigned char volatile xdata *)0xff11)
#define     PWM00T2     (*(unsigned int volatile xdata *)0xff12)
#define     PWM00T2H    (*(unsigned char volatile xdata *)0xff12)
#define     PWM00T2L    (*(unsigned char volatile xdata *)0xff13)
#define     PWM00CR     (*(unsigned char volatile xdata *)0xff14)
#define     PWM00HLD    (*(unsigned char volatile xdata *)0xff15)
#define     PWM01T1     (*(unsigned int volatile xdata *)0xff18)
#define     PWM01T1H    (*(unsigned char volatile xdata *)0xff18)
#define     PWM01T1L    (*(unsigned char volatile xdata *)0xff19)
#define     PWM01T2    	(*(unsigned int volatile xdata *)0xff1a)
#define     PWM01T2H    (*(unsigned char volatile xdata *)0xff1a)
#define     PWM01T2L    (*(unsigned char volatile xdata *)0xff1b)
#define     PWM01CR     (*(unsigned char volatile xdata *)0xff1c)
#define     PWM01HLD    (*(unsigned char volatile xdata *)0xff1d)
#define     PWM02T1     (*(unsigned int volatile xdata *)0xff20)
#define     PWM02T1H    (*(unsigned char volatile xdata *)0xff20)
#define     PWM02T1L    (*(unsigned char volatile xdata *)0xff21)
#define     PWM02T2    	(*(unsigned int volatile xdata *)0xff22)
#define     PWM02T2H    (*(unsigned char volatile xdata *)0xff22)
#define     PWM02T2L    (*(unsigned char volatile xdata *)0xff23)
#define     PWM02CR     (*(unsigned char volatile xdata *)0xff24)
#define     PWM02HLD    (*(unsigned char volatile xdata *)0xff25)
#define     PWM03T1    	(*(unsigned int volatile xdata *)0xff28)
#define     PWM03T1H    (*(unsigned char volatile xdata *)0xff28)
#define     PWM03T1L    (*(unsigned char volatile xdata *)0xff29)
#define     PWM03T2    	(*(unsigned int volatile xdata *)0xff2a)
#define     PWM03T2H    (*(unsigned char volatile xdata *)0xff2a)
#define     PWM03T2L    (*(unsigned char volatile xdata *)0xff2b)
#define     PWM03CR     (*(unsigned char volatile xdata *)0xff2c)
#define     PWM03HLD    (*(unsigned char volatile xdata *)0xff2d)
#define     PWM04T1    	(*(unsigned int volatile xdata *)0xff30)
#define     PWM04T1H    (*(unsigned char volatile xdata *)0xff30)
#define     PWM04T1L    (*(unsigned char volatile xdata *)0xff31)
#define     PWM04T2    	(*(unsigned int volatile xdata *)0xff32)
#define     PWM04T2H    (*(unsigned char volatile xdata *)0xff32)
#define     PWM04T2L    (*(unsigned char volatile xdata *)0xff33)
#define     PWM04CR     (*(unsigned char volatile xdata *)0xff34)
#define     PWM04HLD    (*(unsigned char volatile xdata *)0xff35)
#define     PWM05T1     (*(unsigned int volatile xdata *)0xff38)
#define     PWM05T1H    (*(unsigned char volatile xdata *)0xff38)
#define     PWM05T1L    (*(unsigned char volatile xdata *)0xff39)
#define     PWM05T2     (*(unsigned int volatile xdata *)0xff3a)
#define     PWM05T2H    (*(unsigned char volatile xdata *)0xff3a)
#define     PWM05T2L    (*(unsigned char volatile xdata *)0xff3b)
#define     PWM05CR     (*(unsigned char volatile xdata *)0xff3c)
#define     PWM05HLD    (*(unsigned char volatile xdata *)0xff3d)
#define     PWM06T1     (*(unsigned int volatile xdata *)0xff40)
#define     PWM06T1H    (*(unsigned char volatile xdata *)0xff40)
#define     PWM06T1L    (*(unsigned char volatile xdata *)0xff41)
#define     PWM06T2     (*(unsigned int volatile xdata *)0xff42)
#define     PWM06T2H    (*(unsigned char volatile xdata *)0xff42)
#define     PWM06T2L    (*(unsigned char volatile xdata *)0xff43)
#define     PWM06CR     (*(unsigned char volatile xdata *)0xff44)
#define     PWM06HLD    (*(unsigned char volatile xdata *)0xff45)
#define     PWM07T1     (*(unsigned int volatile xdata *)0xff48)
#define     PWM07T1H    (*(unsigned char volatile xdata *)0xff48)
#define     PWM07T1L    (*(unsigned char volatile xdata *)0xff49)
#define     PWM07T2     (*(unsigned int volatile xdata *)0xff4a)
#define     PWM07T2H    (*(unsigned char volatile xdata *)0xff4a)
#define     PWM07T2L    (*(unsigned char volatile xdata *)0xff4b)
#define     PWM07CR     (*(unsigned char volatile xdata *)0xff4c)
#define     PWM07HLD    (*(unsigned char volatile xdata *)0xff4d)
#define     PWM1C       (*(unsigned int volatile xdata *)0xff50)
#define     PWM1CH      (*(unsigned char volatile xdata *)0xff50)
#define     PWM1CL      (*(unsigned char volatile xdata *)0xff51)
#define     PWM1CKS     (*(unsigned char volatile xdata *)0xff52)
#define     PWM1IF      (*(unsigned char volatile xdata *)0xff55)
#define     PWM1FDCR    (*(unsigned char volatile xdata *)0xff56)
#define     PWM10T1     (*(unsigned int volatile xdata *)0xff60)
#define     PWM10T1H    (*(unsigned char volatile xdata *)0xff60)
#define     PWM10T1L    (*(unsigned char volatile xdata *)0xff61)
#define     PWM10T2     (*(unsigned int volatile xdata *)0xff62)
#define     PWM10T2H    (*(unsigned char volatile xdata *)0xff62)
#define     PWM10T2L    (*(unsigned char volatile xdata *)0xff63)
#define     PWM10CR     (*(unsigned char volatile xdata *)0xff64)
#define     PWM10HLD    (*(unsigned char volatile xdata *)0xff65)
#define     PWM11T1     (*(unsigned int volatile xdata *)0xff68)
#define     PWM11T1H    (*(unsigned char volatile xdata *)0xff68)
#define     PWM11T1L    (*(unsigned char volatile xdata *)0xff69)
#define     PWM11T2     (*(unsigned int volatile xdata *)0xff6a)
#define     PWM11T2H    (*(unsigned char volatile xdata *)0xff6a)
#define     PWM11T2L    (*(unsigned char volatile xdata *)0xff6b)
#define     PWM11CR     (*(unsigned char volatile xdata *)0xff6c)
#define     PWM11HLD    (*(unsigned char volatile xdata *)0xff6d)
#define     PWM12T1     (*(unsigned int volatile xdata *)0xff70)
#define     PWM12T1H    (*(unsigned char volatile xdata *)0xff70)
#define     PWM12T1L    (*(unsigned char volatile xdata *)0xff71)
#define     PWM12T2     (*(unsigned int volatile xdata *)0xff72)
#define     PWM12T2H    (*(unsigned char volatile xdata *)0xff72)
#define     PWM12T2L    (*(unsigned char volatile xdata *)0xff73)
#define     PWM12CR     (*(unsigned char volatile xdata *)0xff74)
#define     PWM12HLD    (*(unsigned char volatile xdata *)0xff75)
#define     PWM13T1     (*(unsigned int volatile xdata *)0xff78)
#define     PWM13T1H    (*(unsigned char volatile xdata *)0xff78)
#define     PWM13T1L    (*(unsigned char volatile xdata *)0xff79)
#define     PWM13T2     (*(unsigned int volatile xdata *)0xff7a)
#define     PWM13T2H    (*(unsigned char volatile xdata *)0xff7a)
#define     PWM13T2L    (*(unsigned char volatile xdata *)0xff7b)
#define     PWM13CR     (*(unsigned char volatile xdata *)0xff7c)
#define     PWM13HLD    (*(unsigned char volatile xdata *)0xff7d)
#define     PWM14T1     (*(unsigned int volatile xdata *)0xff80)
#define     PWM14T1H    (*(unsigned char volatile xdata *)0xff80)
#define     PWM14T1L    (*(unsigned char volatile xdata *)0xff81)
#define     PWM14T2     (*(unsigned int volatile xdata *)0xff82)
#define     PWM14T2H    (*(unsigned char volatile xdata *)0xff82)
#define     PWM14T2L    (*(unsigned char volatile xdata *)0xff83)
#define     PWM14CR     (*(unsigned char volatile xdata *)0xff84)
#define     PWM14HLD    (*(unsigned char volatile xdata *)0xff85)
#define     PWM15T1     (*(unsigned int volatile xdata *)0xff88)
#define     PWM15T1H    (*(unsigned char volatile xdata *)0xff88)
#define     PWM15T1L    (*(unsigned char volatile xdata *)0xff89)
#define     PWM15T2     (*(unsigned int volatile xdata *)0xff8a)
#define     PWM15T2H    (*(unsigned char volatile xdata *)0xff8a)
#define     PWM15T2L    (*(unsigned char volatile xdata *)0xff8b)
#define     PWM15CR     (*(unsigned char volatile xdata *)0xff8c)
#define     PWM15HLD    (*(unsigned char volatile xdata *)0xff8d)
#define     PWM16T1     (*(unsigned int volatile xdata *)0xff90)
#define     PWM16T1H    (*(unsigned char volatile xdata *)0xff90)
#define     PWM16T1L    (*(unsigned char volatile xdata *)0xff91)
#define     PWM16T2     (*(unsigned int volatile xdata *)0xff92)
#define     PWM16T2H    (*(unsigned char volatile xdata *)0xff92)
#define     PWM16T2L    (*(unsigned char volatile xdata *)0xff93)
#define     PWM16CR     (*(unsigned char volatile xdata *)0xff94)
#define     PWM16HLD    (*(unsigned char volatile xdata *)0xff95)
#define     PWM17T1     (*(unsigned int volatile xdata *)0xff98)
#define     PWM17T1H    (*(unsigned char volatile xdata *)0xff98)
#define     PWM17T1L    (*(unsigned char volatile xdata *)0xff99)
#define     PWM17T2     (*(unsigned int volatile xdata *)0xff9a)
#define     PWM17T2H    (*(unsigned char volatile xdata *)0xff9a)
#define     PWM17T2L    (*(unsigned char volatile xdata *)0xff9b)
#define     PWM17CR     (*(unsigned char volatile xdata *)0xff9c)
#define     PWM17HLD    (*(unsigned char volatile xdata *)0xff9d)
#define     PWM2C       (*(unsigned int volatile xdata *)0xffa0)
#define     PWM2CH      (*(unsigned char volatile xdata *)0xffa0)
#define     PWM2CL      (*(unsigned char volatile xdata *)0xffa1)
#define     PWM2CKS     (*(unsigned char volatile xdata *)0xffa2)
#define     PWM2TADC    (*(unsigned int volatile xdata *)0xffa3)
#define     PWM2TADCH   (*(unsigned char volatile xdata *)0xffa3)
#define     PWM2TADCL   (*(unsigned char volatile xdata *)0xffa4)
#define     PWM2IF      (*(unsigned char volatile xdata *)0xffa5)
#define     PWM2FDCR    (*(unsigned char volatile xdata *)0xffa6)
#define     PWM20T1     (*(unsigned int volatile xdata *)0xffb0)
#define     PWM20T1H    (*(unsigned char volatile xdata *)0xffb0)
#define     PWM20T1L    (*(unsigned char volatile xdata *)0xffb1)
#define     PWM20T2     (*(unsigned int volatile xdata *)0xffb2)
#define     PWM20T2H    (*(unsigned char volatile xdata *)0xffb2)
#define     PWM20T2L    (*(unsigned char volatile xdata *)0xffb3)
#define     PWM20CR     (*(unsigned char volatile xdata *)0xffb4)
#define     PWM20HLD    (*(unsigned char volatile xdata *)0xffb5)
#define     PWM21T1     (*(unsigned int volatile xdata *)0xffb8)
#define     PWM21T1H    (*(unsigned char volatile xdata *)0xffb8)
#define     PWM21T1L    (*(unsigned char volatile xdata *)0xffb9)
#define     PWM21T2     (*(unsigned int volatile xdata *)0xffba)
#define     PWM21T2H    (*(unsigned char volatile xdata *)0xffba)
#define     PWM21T2L    (*(unsigned char volatile xdata *)0xffbb)
#define     PWM21CR     (*(unsigned char volatile xdata *)0xffbc)
#define     PWM21HLD    (*(unsigned char volatile xdata *)0xffbd)
#define     PWM22T1     (*(unsigned int volatile xdata *)0xffc0)
#define     PWM22T1H    (*(unsigned char volatile xdata *)0xffc0)
#define     PWM22T1L    (*(unsigned char volatile xdata *)0xffc1)
#define     PWM22T2     (*(unsigned int volatile xdata *)0xffc2)
#define     PWM22T2H    (*(unsigned char volatile xdata *)0xffc2)
#define     PWM22T2L    (*(unsigned char volatile xdata *)0xffc3)
#define     PWM22CR     (*(unsigned char volatile xdata *)0xffc4)
#define     PWM22HLD    (*(unsigned char volatile xdata *)0xffc5)
#define     PWM23T1     (*(unsigned int volatile xdata *)0xffc8)
#define     PWM23T1H    (*(unsigned char volatile xdata *)0xffc8)
#define     PWM23T1L    (*(unsigned char volatile xdata *)0xffc9)
#define     PWM23T2     (*(unsigned int volatile xdata *)0xffca)
#define     PWM23T2H    (*(unsigned char volatile xdata *)0xffca)
#define     PWM23T2L    (*(unsigned char volatile xdata *)0xffcb)
#define     PWM23CR     (*(unsigned char volatile xdata *)0xffcc)
#define     PWM23HLD    (*(unsigned char volatile xdata *)0xffcd)
#define     PWM24T1     (*(unsigned int volatile xdata *)0xffd0)
#define     PWM24T1H    (*(unsigned char volatile xdata *)0xffd0)
#define     PWM24T1L    (*(unsigned char volatile xdata *)0xffd1)
#define     PWM24T2     (*(unsigned int volatile xdata *)0xffd2)
#define     PWM24T2H    (*(unsigned char volatile xdata *)0xffd2)
#define     PWM24T2L    (*(unsigned char volatile xdata *)0xffd3)
#define     PWM24CR     (*(unsigned char volatile xdata *)0xffd4)
#define     PWM24HLD    (*(unsigned char volatile xdata *)0xffd5)
#define     PWM25T1     (*(unsigned int volatile xdata *)0xffd8)
#define     PWM25T1H    (*(unsigned char volatile xdata *)0xffd8)
#define     PWM25T1L    (*(unsigned char volatile xdata *)0xffd9)
#define     PWM25T2     (*(unsigned int volatile xdata *)0xffda)
#define     PWM25T2H    (*(unsigned char volatile xdata *)0xffda)
#define     PWM25T2L    (*(unsigned char volatile xdata *)0xffdb)
#define     PWM25CR     (*(unsigned char volatile xdata *)0xffdc)
#define     PWM25HLD    (*(unsigned char volatile xdata *)0xffdd)
#define     PWM26T1     (*(unsigned int volatile xdata *)0xffe0)
#define     PWM26T1H    (*(unsigned char volatile xdata *)0xffe0)
#define     PWM26T1L    (*(unsigned char volatile xdata *)0xffe1)
#define     PWM26T2     (*(unsigned int volatile xdata *)0xffe2)
#define     PWM26T2H    (*(unsigned char volatile xdata *)0xffe2)
#define     PWM26T2L    (*(unsigned char volatile xdata *)0xffe3)
#define     PWM26CR     (*(unsigned char volatile xdata *)0xffe4)
#define     PWM26HLD    (*(unsigned char volatile xdata *)0xffe5)
#define     PWM27T1     (*(unsigned int volatile xdata *)0xffe8)
#define     PWM27T1H    (*(unsigned char volatile xdata *)0xffe8)
#define     PWM27T1L    (*(unsigned char volatile xdata *)0xffe9)
#define     PWM27T2     (*(unsigned int volatile xdata *)0xffea)
#define     PWM27T2H    (*(unsigned char volatile xdata *)0xffea)
#define     PWM27T2L    (*(unsigned char volatile xdata *)0xffeb)
#define     PWM27CR     (*(unsigned char volatile xdata *)0xffec)
#define     PWM27HLD    (*(unsigned char volatile xdata *)0xffed)

/////////////////////////////////////////////////
//FC00H-FCFFH
/////////////////////////////////////////////////

#define     PWM3C       (*(unsigned int volatile xdata *)0xfc00)
#define     PWM3CH      (*(unsigned char volatile xdata *)0xfc00)
#define     PWM3CL      (*(unsigned char volatile xdata *)0xfc01)
#define     PWM3CKS     (*(unsigned char volatile xdata *)0xfc02)
#define     PWM3IF      (*(unsigned char volatile xdata *)0xfc05)
#define     PWM3FDCR    (*(unsigned char volatile xdata *)0xfc06)
#define     PWM30T1     (*(unsigned int volatile xdata *)0xfc10)
#define     PWM30T1H    (*(unsigned char volatile xdata *)0xfc10)
#define     PWM30T1L    (*(unsigned char volatile xdata *)0xfc11)
#define     PWM30T2     (*(unsigned int volatile xdata *)0xfc12)
#define     PWM30T2H    (*(unsigned char volatile xdata *)0xfc12)
#define     PWM30T2L    (*(unsigned char volatile xdata *)0xfc13)
#define     PWM30CR     (*(unsigned char volatile xdata *)0xfc14)
#define     PWM30HLD    (*(unsigned char volatile xdata *)0xfc15)
#define     PWM31T1     (*(unsigned int volatile xdata *)0xfc18)
#define     PWM31T1H    (*(unsigned char volatile xdata *)0xfc18)
#define     PWM31T1L    (*(unsigned char volatile xdata *)0xfc19)
#define     PWM31T2     (*(unsigned int volatile xdata *)0xfc1a)
#define     PWM31T2H    (*(unsigned char volatile xdata *)0xfc1a)
#define     PWM31T2L    (*(unsigned char volatile xdata *)0xfc1b)
#define     PWM31CR     (*(unsigned char volatile xdata *)0xfc1c)
#define     PWM31HLD    (*(unsigned char volatile xdata *)0xfc1d)
#define     PWM32T1     (*(unsigned int volatile xdata *)0xfc20)
#define     PWM32T1H    (*(unsigned char volatile xdata *)0xfc20)
#define     PWM32T1L    (*(unsigned char volatile xdata *)0xfc21)
#define     PWM32T2     (*(unsigned int volatile xdata *)0xfc22)
#define     PWM32T2H    (*(unsigned char volatile xdata *)0xfc22)
#define     PWM32T2L    (*(unsigned char volatile xdata *)0xfc23)
#define     PWM32CR     (*(unsigned char volatile xdata *)0xfc24)
#define     PWM32HLD    (*(unsigned char volatile xdata *)0xfc25)
#define     PWM33T1     (*(unsigned int volatile xdata *)0xfc28)
#define     PWM33T1H    (*(unsigned char volatile xdata *)0xfc28)
#define     PWM33T1L    (*(unsigned char volatile xdata *)0xfc29)
#define     PWM33T2     (*(unsigned int volatile xdata *)0xfc2a)
#define     PWM33T2H    (*(unsigned char volatile xdata *)0xfc2a)
#define     PWM33T2L    (*(unsigned char volatile xdata *)0xfc2b)
#define     PWM33CR     (*(unsigned char volatile xdata *)0xfc2c)
#define     PWM33HLD    (*(unsigned char volatile xdata *)0xfc2d)
#define     PWM34T1     (*(unsigned int volatile xdata *)0xfc30)
#define     PWM34T1H    (*(unsigned char volatile xdata *)0xfc30)
#define     PWM34T1L    (*(unsigned char volatile xdata *)0xfc31)
#define     PWM34T2     (*(unsigned int volatile xdata *)0xfc32)
#define     PWM34T2H    (*(unsigned char volatile xdata *)0xfc32)
#define     PWM34T2L    (*(unsigned char volatile xdata *)0xfc33)
#define     PWM34CR     (*(unsigned char volatile xdata *)0xfc34)
#define     PWM34HLD    (*(unsigned char volatile xdata *)0xfc35)
#define     PWM35T1     (*(unsigned int volatile xdata *)0xfc38)
#define     PWM35T1H    (*(unsigned char volatile xdata *)0xfc38)
#define     PWM35T1L    (*(unsigned char volatile xdata *)0xfc39)
#define     PWM35T2     (*(unsigned int volatile xdata *)0xfc3a)
#define     PWM35T2H    (*(unsigned char volatile xdata *)0xfc3a)
#define     PWM35T2L    (*(unsigned char volatile xdata *)0xfc3b)
#define     PWM35CR     (*(unsigned char volatile xdata *)0xfc3c)
#define     PWM35HLD    (*(unsigned char volatile xdata *)0xfc3d)
#define     PWM36T1     (*(unsigned int volatile xdata *)0xfc40)
#define     PWM36T1H    (*(unsigned char volatile xdata *)0xfc40)
#define     PWM36T1L    (*(unsigned char volatile xdata *)0xfc41)
#define     PWM36T2     (*(unsigned int volatile xdata *)0xfc42)
#define     PWM36T2H    (*(unsigned char volatile xdata *)0xfc42)
#define     PWM36T2L    (*(unsigned char volatile xdata *)0xfc43)
#define     PWM36CR     (*(unsigned char volatile xdata *)0xfc44)
#define     PWM36HLD    (*(unsigned char volatile xdata *)0xfc45)
#define     PWM37T1     (*(unsigned int volatile xdata *)0xfc48)
#define     PWM37T1H    (*(unsigned char volatile xdata *)0xfc48)
#define     PWM37T1L    (*(unsigned char volatile xdata *)0xfc49)
#define     PWM37T2     (*(unsigned int volatile xdata *)0xfc4a)
#define     PWM37T2H    (*(unsigned char volatile xdata *)0xfc4a)
#define     PWM37T2L    (*(unsigned char volatile xdata *)0xfc4b)
#define     PWM37CR     (*(unsigned char volatile xdata *)0xfc4c)
#define     PWM37HLD    (*(unsigned char volatile xdata *)0xfc4d)
#define     PWM4C       (*(unsigned int volatile xdata *)0xfc50)
#define     PWM4CH      (*(unsigned char volatile xdata *)0xfc50)
#define     PWM4CL      (*(unsigned char volatile xdata *)0xfc51)
#define     PWM4CKS     (*(unsigned char volatile xdata *)0xfc52)
#define     PWM4TADC    (*(unsigned int volatile xdata *)0xfc53)
#define     PWM4TADCH   (*(unsigned char volatile xdata *)0xfc53)
#define     PWM4TADCL   (*(unsigned char volatile xdata *)0xfc54)
#define     PWM4IF      (*(unsigned char volatile xdata *)0xfc55)
#define     PWM4FDCR    (*(unsigned char volatile xdata *)0xfc56)
#define     PWM40T1     (*(unsigned int volatile xdata *)0xfc60)
#define     PWM40T1H    (*(unsigned char volatile xdata *)0xfc60)
#define     PWM40T1L    (*(unsigned char volatile xdata *)0xfc61)
#define     PWM40T2     (*(unsigned int volatile xdata *)0xfc62)
#define     PWM40T2H    (*(unsigned char volatile xdata *)0xfc62)
#define     PWM40T2L    (*(unsigned char volatile xdata *)0xfc63)
#define     PWM40CR     (*(unsigned char volatile xdata *)0xfc64)
#define     PWM40HLD    (*(unsigned char volatile xdata *)0xfc65)
#define     PWM41T1     (*(unsigned int volatile xdata *)0xfc68)
#define     PWM41T1H    (*(unsigned char volatile xdata *)0xfc68)
#define     PWM41T1L    (*(unsigned char volatile xdata *)0xfc69)
#define     PWM41T2     (*(unsigned int volatile xdata *)0xfc6a)
#define     PWM41T2H    (*(unsigned char volatile xdata *)0xfc6a)
#define     PWM41T2L    (*(unsigned char volatile xdata *)0xfc6b)
#define     PWM41CR     (*(unsigned char volatile xdata *)0xfc6c)
#define     PWM41HLD    (*(unsigned char volatile xdata *)0xfc6d)
#define     PWM42T1     (*(unsigned int volatile xdata *)0xfc70)
#define     PWM42T1H    (*(unsigned char volatile xdata *)0xfc70)
#define     PWM42T1L    (*(unsigned char volatile xdata *)0xfc71)
#define     PWM42T2     (*(unsigned int volatile xdata *)0xfc72)
#define     PWM42T2H    (*(unsigned char volatile xdata *)0xfc72)
#define     PWM42T2L    (*(unsigned char volatile xdata *)0xfc73)
#define     PWM42CR     (*(unsigned char volatile xdata *)0xfc74)
#define     PWM42HLD    (*(unsigned char volatile xdata *)0xfc75)
#define     PWM43T1     (*(unsigned int volatile xdata *)0xfc78)
#define     PWM43T1H    (*(unsigned char volatile xdata *)0xfc78)
#define     PWM43T1L    (*(unsigned char volatile xdata *)0xfc79)
#define     PWM43T2     (*(unsigned int volatile xdata *)0xfc7a)
#define     PWM43T2H    (*(unsigned char volatile xdata *)0xfc7a)
#define     PWM43T2L    (*(unsigned char volatile xdata *)0xfc7b)
#define     PWM43CR     (*(unsigned char volatile xdata *)0xfc7c)
#define     PWM43HLD    (*(unsigned char volatile xdata *)0xfc7d)
#define     PWM44T1     (*(unsigned int volatile xdata *)0xfc80)
#define     PWM44T1H    (*(unsigned char volatile xdata *)0xfc80)
#define     PWM44T1L    (*(unsigned char volatile xdata *)0xfc81)
#define     PWM44T2     (*(unsigned int volatile xdata *)0xfc82)
#define     PWM44T2H    (*(unsigned char volatile xdata *)0xfc82)
#define     PWM44T2L    (*(unsigned char volatile xdata *)0xfc83)
#define     PWM44CR     (*(unsigned char volatile xdata *)0xfc84)
#define     PWM44HLD    (*(unsigned char volatile xdata *)0xfc85)
#define     PWM45T1     (*(unsigned int volatile xdata *)0xfc88)
#define     PWM45T1H    (*(unsigned char volatile xdata *)0xfc88)
#define     PWM45T1L    (*(unsigned char volatile xdata *)0xfc89)
#define     PWM45T2     (*(unsigned int volatile xdata *)0xfc8a)
#define     PWM45T2H    (*(unsigned char volatile xdata *)0xfc8a)
#define     PWM45T2L    (*(unsigned char volatile xdata *)0xfc8b)
#define     PWM45CR     (*(unsigned char volatile xdata *)0xfc8c)
#define     PWM45HLD    (*(unsigned char volatile xdata *)0xfc8d)
#define     PWM46T1     (*(unsigned int volatile xdata *)0xfc90)
#define     PWM46T1H    (*(unsigned char volatile xdata *)0xfc90)
#define     PWM46T1L    (*(unsigned char volatile xdata *)0xfc91)
#define     PWM46T2     (*(unsigned int volatile xdata *)0xfc92)
#define     PWM46T2H    (*(unsigned char volatile xdata *)0xfc92)
#define     PWM46T2L    (*(unsigned char volatile xdata *)0xfc93)
#define     PWM46CR     (*(unsigned char volatile xdata *)0xfc94)
#define     PWM46HLD    (*(unsigned char volatile xdata *)0xfc95)
#define     PWM47T1     (*(unsigned int volatile xdata *)0xfc98)
#define     PWM47T1H    (*(unsigned char volatile xdata *)0xfc98)
#define     PWM47T1L    (*(unsigned char volatile xdata *)0xfc99)
#define     PWM47T2     (*(unsigned int volatile xdata *)0xfc9a)
#define     PWM47T2H    (*(unsigned char volatile xdata *)0xfc9a)
#define     PWM47T2L    (*(unsigned char volatile xdata *)0xfc9b)
#define     PWM47CR     (*(unsigned char volatile xdata *)0xfc9c)
#define     PWM47HLD    (*(unsigned char volatile xdata *)0xfc9d)
#define     PWM5C       (*(unsigned int volatile xdata *)0xfca0)
#define     PWM5CH      (*(unsigned char volatile xdata *)0xfca0)
#define     PWM5CL      (*(unsigned char volatile xdata *)0xfca1)
#define     PWM5CKS     (*(unsigned char volatile xdata *)0xfca2)
#define     PWM5IF      (*(unsigned char volatile xdata *)0xfca5)
#define     PWM5FDCR    (*(unsigned char volatile xdata *)0xfca6)
#define     PWM50T1     (*(unsigned int volatile xdata *)0xfcb0)
#define     PWM50T1H    (*(unsigned char volatile xdata *)0xfcb0)
#define     PWM50T1L    (*(unsigned char volatile xdata *)0xfcb1)
#define     PWM50T2     (*(unsigned int volatile xdata *)0xfcb2)
#define     PWM50T2H    (*(unsigned char volatile xdata *)0xfcb2)
#define     PWM50T2L    (*(unsigned char volatile xdata *)0xfcb3)
#define     PWM50CR     (*(unsigned char volatile xdata *)0xfcb4)
#define     PWM50HLD    (*(unsigned char volatile xdata *)0xfcb5)
#define     PWM51T1     (*(unsigned int volatile xdata *)0xfcb8)
#define     PWM51T1H    (*(unsigned char volatile xdata *)0xfcb8)
#define     PWM51T1L    (*(unsigned char volatile xdata *)0xfcb9)
#define     PWM51T2     (*(unsigned int volatile xdata *)0xfcba)
#define     PWM51T2H    (*(unsigned char volatile xdata *)0xfcba)
#define     PWM51T2L    (*(unsigned char volatile xdata *)0xfcbb)
#define     PWM51CR     (*(unsigned char volatile xdata *)0xfcbc)
#define     PWM51HLD    (*(unsigned char volatile xdata *)0xfcbd)
#define     PWM52T1     (*(unsigned int volatile xdata *)0xfcc0)
#define     PWM52T1H    (*(unsigned char volatile xdata *)0xfcc0)
#define     PWM52T1L    (*(unsigned char volatile xdata *)0xfcc1)
#define     PWM52T2     (*(unsigned int volatile xdata *)0xfcc2)
#define     PWM52T2H    (*(unsigned char volatile xdata *)0xfcc2)
#define     PWM52T2L    (*(unsigned char volatile xdata *)0xfcc3)
#define     PWM52CR     (*(unsigned char volatile xdata *)0xfcc4)
#define     PWM52HLD    (*(unsigned char volatile xdata *)0xfcc5)
#define     PWM53T1     (*(unsigned int volatile xdata *)0xfcc8)
#define     PWM53T1H    (*(unsigned char volatile xdata *)0xfcc8)
#define     PWM53T1L    (*(unsigned char volatile xdata *)0xfcc9)
#define     PWM53T2     (*(unsigned int volatile xdata *)0xfcca)
#define     PWM53T2H    (*(unsigned char volatile xdata *)0xfcca)
#define     PWM53T2L    (*(unsigned char volatile xdata *)0xfccb)
#define     PWM53CR     (*(unsigned char volatile xdata *)0xfccc)
#define     PWM53HLD    (*(unsigned char volatile xdata *)0xfccd)
#define     PWM54T1     (*(unsigned int volatile xdata *)0xfcd0)
#define     PWM54T1H    (*(unsigned char volatile xdata *)0xfcd0)
#define     PWM54T1L    (*(unsigned char volatile xdata *)0xfcd1)
#define     PWM54T2     (*(unsigned int volatile xdata *)0xfcd2)
#define     PWM54T2H    (*(unsigned char volatile xdata *)0xfcd2)
#define     PWM54T2L    (*(unsigned char volatile xdata *)0xfcd3)
#define     PWM54CR     (*(unsigned char volatile xdata *)0xfcd4)
#define     PWM54HLD    (*(unsigned char volatile xdata *)0xfcd5)
#define     PWM55T1     (*(unsigned int volatile xdata *)0xfcd8)
#define     PWM55T1H    (*(unsigned char volatile xdata *)0xfcd8)
#define     PWM55T1L    (*(unsigned char volatile xdata *)0xfcd9)
#define     PWM55T2     (*(unsigned int volatile xdata *)0xfcda)
#define     PWM55T2H    (*(unsigned char volatile xdata *)0xfcda)
#define     PWM55T2L    (*(unsigned char volatile xdata *)0xfcdb)
#define     PWM55CR     (*(unsigned char volatile xdata *)0xfcdc)
#define     PWM55HLD    (*(unsigned char volatile xdata *)0xfcdd)
#define     PWM56T1     (*(unsigned int volatile xdata *)0xfce0)
#define     PWM56T1H    (*(unsigned char volatile xdata *)0xfce0)
#define     PWM56T1L    (*(unsigned char volatile xdata *)0xfce1)
#define     PWM56T2     (*(unsigned int volatile xdata *)0xfce2)
#define     PWM56T2H    (*(unsigned char volatile xdata *)0xfce2)
#define     PWM56T2L    (*(unsigned char volatile xdata *)0xfce3)
#define     PWM56CR     (*(unsigned char volatile xdata *)0xfce4)
#define     PWM56HLD    (*(unsigned char volatile xdata *)0xfce5)
#define     PWM57T1     (*(unsigned int volatile xdata *)0xfce8)
#define     PWM57T1H    (*(unsigned char volatile xdata *)0xfce8)
#define     PWM57T1L    (*(unsigned char volatile xdata *)0xfce9)
#define     PWM57T2     (*(unsigned int volatile xdata *)0xfcea)
#define     PWM57T2H    (*(unsigned char volatile xdata *)0xfcea)
#define     PWM57T2L    (*(unsigned char volatile xdata *)0xfceb)
#define     PWM57CR     (*(unsigned char volatile xdata *)0xfcec)
#define     PWM57HLD    (*(unsigned char volatile xdata *)0xfced)

/////////////////////////////////////////////////
//FD00H-FDFFH
/////////////////////////////////////////////////

#define     P0INTE      (*(unsigned char volatile xdata *)0xfd00)
#define     P1INTE      (*(unsigned char volatile xdata *)0xfd01)
#define     P2INTE      (*(unsigned char volatile xdata *)0xfd02)
#define     P3INTE      (*(unsigned char volatile xdata *)0xfd03)
#define     P4INTE      (*(unsigned char volatile xdata *)0xfd04)
#define     P5INTE      (*(unsigned char volatile xdata *)0xfd05)
#define     P6INTE      (*(unsigned char volatile xdata *)0xfd06)
#define     P7INTE      (*(unsigned char volatile xdata *)0xfd07)
#define     P0INTF      (*(unsigned char volatile xdata *)0xfd10)
#define     P1INTF      (*(unsigned char volatile xdata *)0xfd11)
#define     P2INTF      (*(unsigned char volatile xdata *)0xfd12)
#define     P3INTF      (*(unsigned char volatile xdata *)0xfd13)
#define     P4INTF      (*(unsigned char volatile xdata *)0xfd14)
#define     P5INTF      (*(unsigned char volatile xdata *)0xfd15)
#define     P6INTF      (*(unsigned char volatile xdata *)0xfd16)
#define     P7INTF      (*(unsigned char volatile xdata *)0xfd17)
#define     P0IM0       (*(unsigned char volatile xdata *)0xfd20)
#define     P1IM0       (*(unsigned char volatile xdata *)0xfd21)
#define     P2IM0       (*(unsigned char volatile xdata *)0xfd22)
#define     P3IM0       (*(unsigned char volatile xdata *)0xfd23)
#define     P4IM0       (*(unsigned char volatile xdata *)0xfd24)
#define     P5IM0       (*(unsigned char volatile xdata *)0xfd25)
#define     P6IM0       (*(unsigned char volatile xdata *)0xfd26)
#define     P7IM0       (*(unsigned char volatile xdata *)0xfd27)
#define     P0IM1       (*(unsigned char volatile xdata *)0xfd30)
#define     P1IM1       (*(unsigned char volatile xdata *)0xfd31)
#define     P2IM1       (*(unsigned char volatile xdata *)0xfd32)
#define     P3IM1       (*(unsigned char volatile xdata *)0xfd33)
#define     P4IM1       (*(unsigned char volatile xdata *)0xfd34)
#define     P5IM1       (*(unsigned char volatile xdata *)0xfd35)
#define     P6IM1       (*(unsigned char volatile xdata *)0xfd36)
#define     P7IM1       (*(unsigned char volatile xdata *)0xfd37)
#define     P0WKUE      (*(unsigned char volatile xdata *)0xfd40)
#define     P1WKUE      (*(unsigned char volatile xdata *)0xfd41)
#define     P2WKUE      (*(unsigned char volatile xdata *)0xfd42)
#define     P3WKUE      (*(unsigned char volatile xdata *)0xfd43)
#define     P4WKUE      (*(unsigned char volatile xdata *)0xfd44)
#define     P5WKUE      (*(unsigned char volatile xdata *)0xfd45)
#define     P6WKUE      (*(unsigned char volatile xdata *)0xfd46)
#define     P7WKUE      (*(unsigned char volatile xdata *)0xfd47)
#define     PIN_IP      (*(unsigned char volatile xdata *)0xfd60)
#define     PIN_IPH     (*(unsigned char volatile xdata *)0xfd61)

/////////////////////////////////////////////////
//FC00H-FCFFH
/////////////////////////////////////////////////

#define     MD3         (*(unsigned char volatile xdata *)0xfcf0)
#define     MD2         (*(unsigned char volatile xdata *)0xfcf1)
#define     MD1         (*(unsigned char volatile xdata *)0xfcf2)
#define     MD0         (*(unsigned char volatile xdata *)0xfcf3)
#define     MD5         (*(unsigned char volatile xdata *)0xfcf4)
#define     MD4         (*(unsigned char volatile xdata *)0xfcf5)
#define     ARCON       (*(unsigned char volatile xdata *)0xfcf6)
#define     OPCON       (*(unsigned char volatile xdata *)0xfcf7)

/////////////////////////////////////////////////
//FB00H-FBFFH
/////////////////////////////////////////////////

#define     COMEN       (*(unsigned char volatile xdata *)0xfb00)
#define     SEGENL      (*(unsigned char volatile xdata *)0xfb01)
#define     SEGENH      (*(unsigned char volatile xdata *)0xfb02)
#define     LEDCTRL     (*(unsigned char volatile xdata *)0xfb03)
#define     LEDCKS      (*(unsigned char volatile xdata *)0xfb04)
#define     COM0_DA_L   (*(unsigned char volatile xdata *)0xfb10)
#define     COM1_DA_L   (*(unsigned char volatile xdata *)0xfb11)
#define     COM2_DA_L   (*(unsigned char volatile xdata *)0xfb12)
#define     COM3_DA_L   (*(unsigned char volatile xdata *)0xfb13)
#define     COM4_DA_L   (*(unsigned char volatile xdata *)0xfb14)
#define     COM5_DA_L   (*(unsigned char volatile xdata *)0xfb15)
#define     COM6_DA_L   (*(unsigned char volatile xdata *)0xfb16)
#define     COM7_DA_L   (*(unsigned char volatile xdata *)0xfb17)
#define     COM0_DA_H   (*(unsigned char volatile xdata *)0xfb18)
#define     COM1_DA_H   (*(unsigned char volatile xdata *)0xfb19)
#define     COM2_DA_H   (*(unsigned char volatile xdata *)0xfb1a)
#define     COM3_DA_H   (*(unsigned char volatile xdata *)0xfb1b)
#define     COM4_DA_H   (*(unsigned char volatile xdata *)0xfb1c)
#define     COM5_DA_H   (*(unsigned char volatile xdata *)0xfb1d)
#define     COM6_DA_H   (*(unsigned char volatile xdata *)0xfb1e)
#define     COM7_DA_H   (*(unsigned char volatile xdata *)0xfb1f)
#define     COM0_DC_L   (*(unsigned char volatile xdata *)0xfb20)
#define     COM1_DC_L   (*(unsigned char volatile xdata *)0xfb21)
#define     COM2_DC_L   (*(unsigned char volatile xdata *)0xfb22)
#define     COM3_DC_L   (*(unsigned char volatile xdata *)0xfb23)
#define     COM4_DC_L   (*(unsigned char volatile xdata *)0xfb24)
#define     COM5_DC_L   (*(unsigned char volatile xdata *)0xfb25)
#define     COM6_DC_L   (*(unsigned char volatile xdata *)0xfb26)
#define     COM7_DC_L   (*(unsigned char volatile xdata *)0xfb27)
#define     COM0_DC_H   (*(unsigned char volatile xdata *)0xfb28)
#define     COM1_DC_H   (*(unsigned char volatile xdata *)0xfb29)
#define     COM2_DC_H   (*(unsigned char volatile xdata *)0xfb2a)
#define     COM3_DC_H   (*(unsigned char volatile xdata *)0xfb2b)
#define     COM4_DC_H   (*(unsigned char volatile xdata *)0xfb2c)
#define     COM5_DC_H   (*(unsigned char volatile xdata *)0xfb2d)
#define     COM6_DC_H   (*(unsigned char volatile xdata *)0xfb2e)
#define     COM7_DC_H   (*(unsigned char volatile xdata *)0xfb2f)

#define     TSCHEN1     (*(unsigned char volatile xdata *)0xfb40)
#define     TSCHEN2     (*(unsigned char volatile xdata *)0xfb41)
#define     TSCFG1      (*(unsigned char volatile xdata *)0xfb42)
#define     TSCFG2      (*(unsigned char volatile xdata *)0xfb43)
#define     TSWUTC      (*(unsigned char volatile xdata *)0xfb44)
#define     TSCTRL      (*(unsigned char volatile xdata *)0xfb45)
#define     TSSTA1      (*(unsigned char volatile xdata *)0xfb46)
#define     TSSTA2      (*(unsigned char volatile xdata *)0xfb47)
#define     TSRT        (*(unsigned char volatile xdata *)0xfb48)
#define     TSDAT       (*(unsigned int  volatile xdata *)0xfb49)
#define     TSDATH      (*(unsigned char volatile xdata *)0xfb49)
#define     TSDATL      (*(unsigned char volatile xdata *)0xfb4A)
#define     TSTH00      (*(unsigned int  volatile xdata *)0xfb50)
#define     TSTH00H     (*(unsigned char volatile xdata *)0xfb50)
#define     TSTH00L     (*(unsigned char volatile xdata *)0xfb51)
#define     TSTH01      (*(unsigned int  volatile xdata *)0xfb52)
#define     TSTH01H     (*(unsigned char volatile xdata *)0xfb52)
#define     TSTH01L     (*(unsigned char volatile xdata *)0xfb53)
#define     TSTH02      (*(unsigned int  volatile xdata *)0xfb54)
#define     TSTH02H     (*(unsigned char volatile xdata *)0xfb54)
#define     TSTH02L     (*(unsigned char volatile xdata *)0xfb55)
#define     TSTH03      (*(unsigned int  volatile xdata *)0xfb56)
#define     TSTH03H     (*(unsigned char volatile xdata *)0xfb56)
#define     TSTH03L     (*(unsigned char volatile xdata *)0xfb57)
#define     TSTH04      (*(unsigned int  volatile xdata *)0xfb58)
#define     TSTH04H     (*(unsigned char volatile xdata *)0xfb58)
#define     TSTH04L     (*(unsigned char volatile xdata *)0xfb59)
#define     TSTH05      (*(unsigned int  volatile xdata *)0xfb5a)
#define     TSTH05H     (*(unsigned char volatile xdata *)0xfb5a)
#define     TSTH05L     (*(unsigned char volatile xdata *)0xfb5b)
#define     TSTH06      (*(unsigned int  volatile xdata *)0xfb5c)
#define     TSTH06H     (*(unsigned char volatile xdata *)0xfb5c)
#define     TSTH06L     (*(unsigned char volatile xdata *)0xfb5d)
#define     TSTH07      (*(unsigned int  volatile xdata *)0xfb5e)
#define     TSTH07H     (*(unsigned char volatile xdata *)0xfb5e)
#define     TSTH07L     (*(unsigned char volatile xdata *)0xfb5f)
#define     TSTH08      (*(unsigned int  volatile xdata *)0xfb60)
#define     TSTH08H     (*(unsigned char volatile xdata *)0xfb60)
#define     TSTH08L     (*(unsigned char volatile xdata *)0xfb61)
#define     TSTH09      (*(unsigned int  volatile xdata *)0xfb62)
#define     TSTH09H     (*(unsigned char volatile xdata *)0xfb62)
#define     TSTH09L     (*(unsigned char volatile xdata *)0xfb63)
#define     TSTH10      (*(unsigned int  volatile xdata *)0xfb64)
#define     TSTH10H     (*(unsigned char volatile xdata *)0xfb64)
#define     TSTH10L     (*(unsigned char volatile xdata *)0xfb65)
#define     TSTH11      (*(unsigned int  volatile xdata *)0xfb66)
#define     TSTH11H     (*(unsigned char volatile xdata *)0xfb66)
#define     TSTH11L     (*(unsigned char volatile xdata *)0xfb67)
#define     TSTH12      (*(unsigned int  volatile xdata *)0xfb68)
#define     TSTH12H     (*(unsigned char volatile xdata *)0xfb68)
#define     TSTH12L     (*(unsigned char volatile xdata *)0xfb69)
#define     TSTH13      (*(unsigned int  volatile xdata *)0xfb6a)
#define     TSTH13H     (*(unsigned char volatile xdata *)0xfb6a)
#define     TSTH13L     (*(unsigned char volatile xdata *)0xfb6b)
#define     TSTH14      (*(unsigned int  volatile xdata *)0xfb6c)
#define     TSTH14H     (*(unsigned char volatile xdata *)0xfb6c)
#define     TSTH14L     (*(unsigned char volatile xdata *)0xfb6d)
#define     TSTH15      (*(unsigned int  volatile xdata *)0xfb6e)
#define     TSTH15H     (*(unsigned char volatile xdata *)0xfb6e)
#define     TSTH15L     (*(unsigned char volatile xdata *)0xfb6f)

/////////////////////////////////////////////////
typedef struct TIM1_struct
{
    volatile unsigned char CR1;     /*!< control register 1 */
    volatile unsigned char CR2;     /*!< control register 2 */
    volatile unsigned char SMCR;    /*!< Synchro mode control register */
    volatile unsigned char ETR;     /*!< external trigger register */
    volatile unsigned char IER;     /*!< interrupt enable register*/
    volatile unsigned char SR1;     /*!< status register 1 */
    volatile unsigned char SR2;     /*!< status register 2 */
    volatile unsigned char EGR;     /*!< event generation register */
    volatile unsigned char CCMR1;   /*!< CC mode register 1 */
    volatile unsigned char CCMR2;   /*!< CC mode register 2 */
    volatile unsigned char CCMR3;   /*!< CC mode register 3 */
    volatile unsigned char CCMR4;   /*!< CC mode register 4 */
    volatile unsigned char CCER1;   /*!< CC enable register 1 */
    volatile unsigned char CCER2;   /*!< CC enable register 2 */
    volatile unsigned char CNTRH;   /*!< counter high */
    volatile unsigned char CNTRL;   /*!< counter low */
    volatile unsigned char PSCRH;   /*!< prescaler high */
    volatile unsigned char PSCRL;   /*!< prescaler low */
    volatile unsigned char ARRH;    /*!< auto-reload register high */
    volatile unsigned char ARRL;    /*!< auto-reload register low */
    volatile unsigned char RCR;     /*!< Repetition Counter register */
    volatile unsigned char CCR1H;   /*!< capture/compare register 1 high */
    volatile unsigned char CCR1L;   /*!< capture/compare register 1 low */
    volatile unsigned char CCR2H;   /*!< capture/compare register 2 high */
    volatile unsigned char CCR2L;   /*!< capture/compare register 2 low */
    volatile unsigned char CCR3H;   /*!< capture/compare register 3 high */
    volatile unsigned char CCR3L;   /*!< capture/compare register 3 low */
    volatile unsigned char CCR4H;   /*!< capture/compare register 3 high */
    volatile unsigned char CCR4L;   /*!< capture/compare register 3 low */
    volatile unsigned char BKR;     /*!< Break Register */
    volatile unsigned char DTR;     /*!< dead-time register */
    volatile unsigned char OISR;    /*!< Output idle register */
}TIM1_TypeDef;

#define TIM1_BaseAddress    0xFEC0
#define TIM1        ((TIM1_TypeDef xdata*)TIM1_BaseAddress)

/////////////////////////////////////////////////
/*   interrupt vector */
#define		INT0_VECTOR		0
#define		TIMER0_VECTOR	1
#define		INT1_VECTOR		2
#define		TIMER1_VECTOR	3
#define		UART1_VECTOR	4
#define		ADC_VECTOR		5
#define		LVD_VECTOR		6
#define		PCA_VECTOR		7
#define		UART2_VECTOR	8
#define		SPI_VECTOR		9
#define		INT2_VECTOR		10
#define		INT3_VECTOR		11
#define		TIMER2_VECTOR	12
#define		INT4_VECTOR		16
#define		UART3_VECTOR	17
#define		UART4_VECTOR	18
#define		TIMER3_VECTOR	19
#define		TIMER4_VECTOR	20
#define		CMP_VECTOR		21
#define		PWM0_VECTOR		22
#define		PWMFD_VECTOR	23
#define		I2C_VECTOR		24
#define		USB_VECTOR		25
#define		PWMA_VECTOR		26
#define		PWMB_VECTOR		27
#define		PWM1_VECTOR		28
#define		PWM2_VECTOR		29
#define		PWM3_VECTOR		30
#define		PWM4_VECTOR		31
#define		PWM5_VECTOR		32

/////////////////////////////////////////////////
typedef unsigned char	u8;
typedef unsigned int u16;
typedef unsigned long int u32;
typedef	signed char int8;
typedef	signed int int16;

typedef unsigned char uint8;
typedef unsigned int uint16;
typedef unsigned long int uint32;


#define	TRUE	1
#define	FALSE	0

//=============================================================

//========================================
#define	Polity_0			0	//中断优先级为 0 级（最低级）
#define	Polity_1			1	//中断优先级为 1 级（较低级）
#define	Polity_2			2	//中断优先级为 2 级（较高级）
#define	Polity_3			3	//中断优先级为 3 级（最高级）

//========================================
#define		ENABLE		1
#define		DISABLE		0

#endif

