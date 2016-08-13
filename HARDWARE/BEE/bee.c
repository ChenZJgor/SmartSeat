#include "bee.h"

void Bee_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//ʹ��PORTBʱ��

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//�ر�jtag��ʹ��SWD��������SWDģʽ����
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;//PB1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING ; //���ÿ�©���,��Ϊ��������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB0
	
}

void Bee_Contrl(u8 sw)
{
	if(sw == BEE_ACTIVE){
		BEE_ON();
		BEE = 1;
	}
	else if(sw == BEE_NEGATIVE){
		BEE_OFF();
	}
}
