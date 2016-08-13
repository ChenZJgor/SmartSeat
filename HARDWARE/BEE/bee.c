#include "bee.h"

void Bee_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//使能PORTB时钟

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭jtag，使能SWD，可以用SWD模式调试
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;//PB1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING ; //设置开漏输出,改为浮空输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB0
	
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
