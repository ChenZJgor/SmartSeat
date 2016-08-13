#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 
#define LED0_OFF()  {GPIOA->CRH&=0XFFFFFFF0;GPIOA->CRH|=0X00000004;}
#define LED0_ON() {GPIOA->CRH&=0XFFFFFFF0;GPIOA->CRH|=0x00000001;}

#define LED1_OFF()  {GPIOA->CRH&=0XFFFF0FFF;GPIOA->CRH|=0X00004000;}
#define LED1_ON() {GPIOA->CRH&=0XFFFF0FFF;GPIOA->CRH|=0x00001000;}

#define LED0 PAout(8)	// PA8
#define LED1 PAout(11)	// PA11	

void LED_Init(void);//��ʼ��

		 				    
#endif
