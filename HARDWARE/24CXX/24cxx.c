#include "sys.h"
#include "includes.h"
#include "24cxx.h" 
//#include "delay.h" 
#include "delay.h"
#include "ds1302.h"
#include "usart3.h"	


u8 store_stack[29] = {0};	//���滺����

//��ʼ��IIC�ӿ�
void AT24CXX_Init(void)
{
	IIC_Init();
}
//��AT24CXXָ����ַ����һ������
//ReadAddr:��ʼ�����ĵ�ַ  
//����ֵ  :����������
u8 AT24CXX_ReadOneByte(u16 ReadAddr)
{				  
	u8 temp=0;		  	    																 
    IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	   //����д����
		IIC_Wait_Ack();
		IIC_Send_Byte(ReadAddr>>8);//���͸ߵ�ַ
		IIC_Wait_Ack();		 
	}else IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //����������ַ0XA0,д���� 	 

	IIC_Wait_Ack(); 
    IIC_Send_Byte(ReadAddr%256);   //���͵͵�ַ
	IIC_Wait_Ack();	    
	IIC_Start();  	 	   
	IIC_Send_Byte(0XA1);           //�������ģʽ			   
	IIC_Wait_Ack();	 
    temp=IIC_Read_Byte(0);		   
    IIC_Stop();//����һ��ֹͣ����	    
	return temp;
}
//��AT24CXXָ����ַд��һ������
//WriteAddr  :д�����ݵ�Ŀ�ĵ�ַ    
//DataToWrite:Ҫд�������
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{				   	  	    																 
    IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	    //����д����
		IIC_Wait_Ack();
		IIC_Send_Byte(WriteAddr>>8);//���͸ߵ�ַ
 	}else
	{
		IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //����������ַ0XA0,д���� 
	}	 
	IIC_Wait_Ack();	   
    IIC_Send_Byte(WriteAddr%256);   //���͵͵�ַ
	IIC_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(DataToWrite);     //�����ֽ�							   
	IIC_Wait_Ack();  		    	   
    IIC_Stop();//����һ��ֹͣ���� 
	delay_ms(10);	 
}
//��AT24CXX�����ָ����ַ��ʼд�볤��ΪLen������
//�ú�������д��16bit����32bit������.
//WriteAddr  :��ʼд��ĵ�ַ  
//DataToWrite:���������׵�ַ
//Len        :Ҫд�����ݵĳ���2,4
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{  	
	u8 t;
	for(t=0;t<Len;t++)
	{
		AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
	}												    
}

//��AT24CXX�����ָ����ַ��ʼ��������ΪLen������
//�ú������ڶ���16bit����32bit������.
//ReadAddr   :��ʼ�����ĵ�ַ 
//����ֵ     :����
//Len        :Ҫ�������ݵĳ���2,4
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
//���AT24CXX�Ƿ�����
//��������24XX�����һ����ַ(255)���洢��־��.
//���������24Cϵ��,�����ַҪ�޸�
//����1:���ʧ��
//����0:���ɹ�
u8 AT24CXX_Check(void)
{
	u8 temp;
	temp=AT24CXX_ReadOneByte(255);//����ÿ�ο�����дAT24CXX			   
	if(temp==0X55)return 0;		   
	else//�ų���һ�γ�ʼ�������
	{
		AT24CXX_WriteOneByte(255,0X55);
	    temp=AT24CXX_ReadOneByte(255);	  
		if(temp==0X55)return 0;
	}
	return 1;											  
}

//��AT24CXX�����ָ����ַ��ʼ����ָ������������
//ReadAddr :��ʼ�����ĵ�ַ ��24c02Ϊ0~255
//pBuffer  :���������׵�ַ
//NumToRead:Ҫ�������ݵĸ���
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);	
		NumToRead--;
	}
}  
//��AT24CXX�����ָ����ַ��ʼд��ָ������������
//WriteAddr :��ʼд��ĵ�ַ ��24c02Ϊ0~255
//pBuffer   :���������׵�ַ
//NumToWrite:Ҫд�����ݵĸ���
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
