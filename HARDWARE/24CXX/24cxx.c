#include "sys.h"
#include "includes.h"
#include "24cxx.h" 
//#include "delay.h" 
#include "delay.h"
#include "ds1302.h"
#include "usart3.h"	


u8 store_stack[29] = {0};	//储存缓冲区

//初始化IIC接口
void AT24CXX_Init(void)
{
	IIC_Init();
}
//在AT24CXX指定地址读出一个数据
//ReadAddr:开始读数的地址  
//返回值  :读到的数据
u8 AT24CXX_ReadOneByte(u16 ReadAddr)
{				  
	u8 temp=0;		  	    																 
    IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	   //发送写命令
		IIC_Wait_Ack();
		IIC_Send_Byte(ReadAddr>>8);//发送高地址
		IIC_Wait_Ack();		 
	}else IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //发送器件地址0XA0,写数据 	 

	IIC_Wait_Ack(); 
    IIC_Send_Byte(ReadAddr%256);   //发送低地址
	IIC_Wait_Ack();	    
	IIC_Start();  	 	   
	IIC_Send_Byte(0XA1);           //进入接收模式			   
	IIC_Wait_Ack();	 
    temp=IIC_Read_Byte(0);		   
    IIC_Stop();//产生一个停止条件	    
	return temp;
}
//在AT24CXX指定地址写入一个数据
//WriteAddr  :写入数据的目的地址    
//DataToWrite:要写入的数据
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{				   	  	    																 
    IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	    //发送写命令
		IIC_Wait_Ack();
		IIC_Send_Byte(WriteAddr>>8);//发送高地址
 	}else
	{
		IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //发送器件地址0XA0,写数据 
	}	 
	IIC_Wait_Ack();	   
    IIC_Send_Byte(WriteAddr%256);   //发送低地址
	IIC_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(DataToWrite);     //发送字节							   
	IIC_Wait_Ack();  		    	   
    IIC_Stop();//产生一个停止条件 
	delay_ms(10);	 
}
//在AT24CXX里面的指定地址开始写入长度为Len的数据
//该函数用于写入16bit或者32bit的数据.
//WriteAddr  :开始写入的地址  
//DataToWrite:数据数组首地址
//Len        :要写入数据的长度2,4
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{  	
	u8 t;
	for(t=0;t<Len;t++)
	{
		AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
	}												    
}

//在AT24CXX里面的指定地址开始读出长度为Len的数据
//该函数用于读出16bit或者32bit的数据.
//ReadAddr   :开始读出的地址 
//返回值     :数据
//Len        :要读出数据的长度2,4
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len)
{  	
	u8 t;
	u32 temp=0;
	for(t=0;t<Len;t++)
	{
		temp<<=8;
		temp+=AT24CXX_ReadOneByte(ReadAddr+Len-t-1); 	 				   
	} 
	return temp;												    
}
//检查AT24CXX是否正常
//这里用了24XX的最后一个地址(255)来存储标志字.
//如果用其他24C系列,这个地址要修改
//返回1:检测失败
//返回0:检测成功
u8 AT24CXX_Check(void)
{
	u8 temp;
	temp=AT24CXX_ReadOneByte(255);//避免每次开机都写AT24CXX			   
	if(temp==0X55)return 0;		   
	else//排除第一次初始化的情况
	{
		AT24CXX_WriteOneByte(255,0X55);
	    temp=AT24CXX_ReadOneByte(255);	  
		if(temp==0X55)return 0;
	}
	return 1;											  
}

//在AT24CXX里面的指定地址开始读出指定个数的数据
//ReadAddr :开始读出的地址 对24c02为0~255
//pBuffer  :数据数组首地址
//NumToRead:要读出数据的个数
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);	
		NumToRead--;
	}
}  
//在AT24CXX里面的指定地址开始写入指定个数的数据
//WriteAddr :开始写入的地址 对24c02为0~255
//pBuffer   :数据数组首地址
//NumToWrite:要写入数据的个数
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite)
{
	while(NumToWrite--)
	{
		AT24CXX_WriteOneByte(WriteAddr,*pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}

void Data_Store(u16 data)
{
	u8 i = 0,writeaddr = 0,houraddr = 0;
	u16 data_temp = 0, verify = 0;
	
	AT24CXX_Init();
	DS1302_GPIO_Init();
	Read_rtc();
	time_pros();
	DS1302_Off();
	i = AT24CXX_ReadOneByte(0);
	writeaddr = 1 + 29 * i;
	verify = AT24CXX_ReadLenByte(writeaddr, 2);
	data_temp = disp[0] * 100 + disp[1] * 40 + disp[2];
	
	if(verify > 0){
		if(data_temp > verify){
			writeaddr += 29;
			memset(store_stack,0,29);
			AT24CXX_Write(writeaddr,store_stack,29);
			
			store_stack[0] = data_temp & 0xff;
			store_stack[1] = (data_temp >> 8) & 0xff;
			store_stack[2] = disp[0];
			store_stack[3] = disp[1];
			store_stack[4] = disp[2];
		
			houraddr = 5 + disp[3];
			
			i = AT24CXX_ReadOneByte(writeaddr+houraddr);
			store_stack[houraddr] = data + i;
			//store_stack[houraddr] = data;
			AT24CXX_Write(writeaddr,store_stack,29);
			
			i = AT24CXX_ReadOneByte(0);
			i++;
			if(i>=8){
				i = 0;
				memset(store_stack,0,29);
				AT24CXX_Write(1,store_stack,29);
			}
			AT24CXX_WriteOneByte(0,i);
		}
		else if(data_temp == verify){
			houraddr = 5 + disp[3];
			i = AT24CXX_ReadOneByte(writeaddr+houraddr);
			store_stack[houraddr] = data + i;
			AT24CXX_WriteOneByte(writeaddr+houraddr,store_stack[houraddr]);
		}
		else if(data_temp < verify){
			if(READ_BLU)
				printf("date data err.\n");
		}
	}
	else if(verify == 0){
		memset(store_stack,0,29);
		AT24CXX_Write(writeaddr,store_stack,29);
			
		store_stack[0] = data_temp & 0xff;
		store_stack[1] = (data_temp >> 8) & 0xff;
		store_stack[2] = disp[0];
		store_stack[3] = disp[1];
		store_stack[4] = disp[2];
		
		houraddr = 5 + disp[3];
			
		i = AT24CXX_ReadOneByte(writeaddr+houraddr);
		store_stack[houraddr] = data + i;
		AT24CXX_Write(writeaddr,store_stack,29);
	}
	memset(disp,0,6);
	IIC_Off();
}
void Posture_Store(u8 posture)
{
	u8 i = 0,writeaddr = 0,houraddr = 0;
	u16 data_temp = 0, verify = 0;
	
	AT24CXX_Init();
	DS1302_GPIO_Init();
	Read_rtc();
	time_pros();
	DS1302_Off();
	i = AT24CXX_ReadOneByte(0);
	writeaddr = 1 + 29 * i;
	verify = AT24CXX_ReadLenByte(writeaddr, 2);
	data_temp = disp[0] * 100 + disp[1] * 40 + disp[2];
	
	if(verify > 0){
		if(data_temp > verify){
			writeaddr += 29;
			memset(store_stack,0,29);
			AT24CXX_Write(writeaddr,store_stack,29);
			
			store_stack[0] = data_temp & 0xff;
			store_stack[1] = (data_temp >> 8) & 0xff;
			store_stack[2] = disp[0];
			store_stack[3] = disp[1];
			store_stack[4] = disp[2];
		
			houraddr = 5 + disp[3];
			
			posture &= 0xc0;
			i = AT24CXX_ReadOneByte(writeaddr+houraddr);
			i &= ~0xc0;
			store_stack[houraddr] = i | posture;
			//store_stack[houraddr] = data;
			AT24CXX_Write(writeaddr,store_stack,29);
			
			i = AT24CXX_ReadOneByte(0);
			i++;
			if(i>=8){
				i = 0;
				memset(store_stack,0,29);
				AT24CXX_Write(1,store_stack,29);
			}
			AT24CXX_WriteOneByte(0,i);
		}
		else if(data_temp == verify){
			houraddr = 5 + disp[3];
			posture &= 0xc0;
			i = AT24CXX_ReadOneByte(writeaddr+houraddr);
			i &= ~0xc0;
			store_stack[houraddr] = i | posture;
			AT24CXX_WriteOneByte(writeaddr+houraddr,store_stack[houraddr]);
		}
		else if(data_temp < verify){
			if(READ_BLU)
				printf("date data err.\n");
		}
	}
	else if(verify == 0){
		memset(store_stack,0,29);
		AT24CXX_Write(writeaddr,store_stack,29);
			
		store_stack[0] = data_temp & 0xff;
		store_stack[1] = (data_temp >> 8) & 0xff;
		store_stack[2] = disp[0];
		store_stack[3] = disp[1];
		store_stack[4] = disp[2];
		
		houraddr = 5 + disp[3];
			
		posture &= 0xc0;
		i = AT24CXX_ReadOneByte(writeaddr+houraddr);
		i &= ~0xc0;
		store_stack[houraddr] = i | posture;
		AT24CXX_Write(writeaddr,store_stack,29);
	}
	memset(disp,0,6);
	IIC_Off();
}
