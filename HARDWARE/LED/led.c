#include "led.h" 
#include "project.h"
#include "delay.h"

//��ʼ��PA8��PA11Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //ʹ��PA�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //LED0-->PA.8 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 //����������ĸ�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOA.8
// GPIO_ResetBits(GPIOA,GPIO_Pin_8);						 //PA.8 �����

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;	    		 //LED1-->PA.11 �˿�����, �������
 GPIO_Init(GPIOA, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
// GPIO_ResetBits(GPIOA,GPIO_Pin_11); 						 //PA.11�����
}

void LED_Red_Blink(void)
{
		LED1_ON();
		LED1 = ON;							 
		delay_ms(100);
		LED1_OFF();
		delay_ms(100);							 
		LED1_ON();
		LED1 = ON;
		delay_ms(100);							 
		LED1_OFF();	
		delay_ms(100);								 
		LED1_ON();
		LED1 = ON;
		delay_ms(100);								 
		LED1_OFF();	
}
