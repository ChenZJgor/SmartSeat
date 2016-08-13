#include "ds1302.h"
#include "delay.h"
#include "usart.h"
u8 time_data[7]={16,2,8,9,19,05,00};
u8 read_data[7]={0};
u8 write_add[7]={0x8c,0x8a,0x88,0x86,0x84,0x82,0x80};
u8 read_add[7]={0x8d,0x8b,0x89,0x87,0x85,0x83,0x81};
u8 disp[6];

//DS1302写一个字节
void Write_DS1302_Byte(u8 dat)
{
	u8 i;
	
	DS1302_SDA_OUT();
	
	for(i=0;i<8;i++)
	{
		DS1302_SCLK = 0;
		DS1302_SDA = dat & 0x01;
		dat = dat >> 1;
		DS1302_SCLK = 1;
	}
}

//写DS1302
void Write_DS1302(u8 add,u8 dat)
{
	DS1302_SDA_OUT();
	DS1302_CE=0;
	
	delay_us(10);
	
	DS1302_SCLK=0;
	
	delay_us(10);
	
	DS1302_CE=1;
	
	delay_us(10);
	
	Write_DS1302_Byte(add);
	Write_DS1302_Byte(dat);
	DS1302_CE=0;
	
	delay_us(10);
	
	DS1302_SDA=1;
	DS1302_SCLK=1;
}

//读取DS1302
u8 Read_DS1302(u8 add)
{
	u8 i,value;
	
	DS1302_CE=0;
	delay_us(10);
	DS1302_SCLK=0;
	delay_us(10);
	DS1302_CE=1;
	delay_us(10);
	Write_DS1302_Byte(add);
	DS1302_SDA_IN();
	for(i=0;i<8;i++)
	{
		value=value>>1;
		DS1302_SCLK=0;
		if(DS1302_IN_SDA)
		value=value|0x80;
		DS1302_SCLK=1;
	}
	DS1302_CE=0;
	delay_us(10);
	DS1302_SCLK=0;
	delay_us(10);
	DS1302_SCLK=1;
	DS1302_SDA_OUT();
	DS1302_SDA=1;
	return value;
}

void Set_rtc(void)
{
 u8 i,j;
	
 for(i=0;i<7;i++)
 {
	j=time_data[i]/10;
	time_data[i]=time_data[i]%10;
  time_data[i]=time_data[i]+j*16;
 }
 Write_DS1302(0x8e,0x00);
 for(i=0;i<7;i++)
 {
  Write_DS1302(write_add[i],time_data[i]);
 }
 Write_DS1302(0x8e,0x80);
}

void Read_rtc(void)
{
 u8 i;
	
 for(i=0;i<7;i++)
 {
  read_data[i]=Read_DS1302(read_add[i]);
 }
}

void time_pros(void)
{
	u8 tmp;
	
 tmp = read_data[0]/16;									//年十进制转换
 disp[0] = read_data[0]%16 + tmp * 10;	
 
 tmp = read_data[2]/16;									//月十进制转换
 disp[1] = read_data[2]%16 + tmp * 10;	
 
 tmp = read_data[3]/16;									//日十进制转换
 disp[2] = read_data[3]%16 + tmp * 10;
 
 tmp = read_data[4]/16;									//时十进制转换
 disp[3] = read_data[4]%16 + tmp * 10;	
 
 tmp = read_data[5]/16;									//分十进制转换
 disp[4] = read_data[5]%16 + tmp * 10;	
 
 tmp = read_data[6]/16;									//秒十进制转换
 disp[5] = read_data[6]%16 + tmp * 10;	
}

/*
void display(void)
{
 printf("%d%d-%d%d-%d%d\n",disp[7],disp[6],disp[4],disp[3],disp[1],disp[0]);

  delay_ms(1000);
}
*/

void DS1302_GPIO_Init(void)
{
// GPIOA->CRL&=0XF000FFFF;
// GPIOA->CRL|=0X03330000;	
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void DS1302_Off(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING ;   //浮空输入

	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*
void DS1302_Init(void)
{
//	Set_rtc();
  Read_rtc();
	time_pros();
	//display();
}
*/
