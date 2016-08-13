#include "power.h"

void Power_Init()
{
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//使能PORTB时钟

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭jtag，使能SWD，可以用SWD模式调试
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;//PB5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING ; //设置开漏输出,改为浮空输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB0
	
}

void Power_Contrl(u8 sw)
{
	if(sw == POWER_ACTIVE){
		POWER_ON();
		POWER = 1;
	}
	else if(sw == POWER_NEGATIVE){
		POWER_OFF();
	}
}
