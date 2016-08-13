#include "gpio.h"
 	    
//空闲I/O口初始化
void Gpio_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;

// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能PORTA时钟

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭jtag，使能SWD，可以用SWD模式调试
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_7 | GPIO_Pin_12;//PA7，PA12
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //设置成下拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA15
	
 
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;//PB8,PB9,PB12,PB13,PB14,PB15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0设置成上拉输入  
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOA.0
	
} 

