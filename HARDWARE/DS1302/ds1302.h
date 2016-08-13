#ifndef _DS1302_H
#define _DS1302_H
#include "sys.h"
#define u8 unsigned char
#define DS1302_SDA_IN()		{GPIOA->CRL &= 0xFF0FFFFF;GPIOA->CRL |=0X00800000;}
#define DS1302_SDA_OUT()	{GPIOA->CRL &= 0xFF0FFFFF;GPIOA->CRL |=0X00300000;}

#define DS1302_CE			PAout(6)	//CE
#define DS1302_SCLK			PAout(4)	//SCLK
#define DS1302_SDA			PAout(5)	//I/O
#define DS1302_IN_SDA		PAin(5)	//读取I/O上数据用到

void Write_DS1302_Byte(u8 dat);
void Write_DS1302(u8 add,u8 dat);
u8 Read_DS1302(u8 add);
void Set_rtc(void);
void Read_rtc(void);
void time_pros(void);
//void DS1302_Init(void);
void DS1302_GPIO_Init(void);
void DS1302_Off(void);

extern u8 disp[6];
#endif
