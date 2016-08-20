#include "includes.h"
#include "sys.h"
#include "exti.h"
#include "key.h"
#include "delay.h"
#include "usart.h"
#include "usart3.h"
#include "stm32f10x_exti.h"
#include "motor.h"
#include "wkup.h"
 
 
//�ⲿ�жϳ�ʼ������
void EXTIX_Init(void)
{
 
 	  EXTI_InitTypeDef EXTI_InitStructure;
 	  NVIC_InitTypeDef NVIC_InitStructure;

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//�ⲿ�жϣ���Ҫʹ��AFIOʱ��

	  KEY_Init();//��ʼ��������Ӧioģʽ

    //GPIOA.15	  �ж����Լ��жϳ�ʼ������
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource15);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line15;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	  	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

    //GPIOA.0	  �ж����Լ��жϳ�ʼ������
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource7);

   	EXTI_InitStructure.EXTI_Line=EXTI_Line7;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);		//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���


 
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//ʹ�ܰ������ڵ��ⲿ�ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//�����ȼ�1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure);  	  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
 
 
   	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//ʹ�ܰ������ڵ��ⲿ�ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2�� 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//�����ȼ�1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure); 
 
}

//�ر�A.15�ⲿ�ж�
void EXTA15_Off(void)
{
	 	EXTI_InitTypeDef EXTI_InitStructure;
 	  NVIC_InitTypeDef NVIC_InitStructure;
	
		GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource15);

		EXTI_InitStructure.EXTI_Line=EXTI_Line15;
		EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
		EXTI_InitStructure.EXTI_LineCmd = DISABLE;
		EXTI_Init(&EXTI_InitStructure);	  	
		NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;							
		NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;								
		NVIC_Init(&NVIC_InitStructure); 
}

 
void EXTI9_5_IRQHandler(void)
{
	OSIntEnter(); 
	
	SystemInit(); 
	//printf("WK_UP=A0\n");
	uart_init(9600);    //���ڲ���������
	uart3_init(9600);		//����3����������
	bluetooth_on = 1;
	EXTI_ClearITPendingBit(EXTI_Line7);  //���EXTI0��·����λ
	OSIntExit();
}


void EXTI15_10_IRQHandler(void)
{
	OSIntEnter();

	SystemInit(); 
	//printf("KEY1=A15\n");
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );
	strain_on = 1;
	EXTI_ClearITPendingBit(EXTI_Line15);  //���LINE15��·����λ
	
	OSIntExit();
}
