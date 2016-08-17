#include "sys.h"
#include "usart3.h"	  
#include "24cxx.h"
#include "adc.h"
#include "stm32f10x_adc.h"
#include "ds1302.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用os,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os 使用	  
#endif

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 0
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*使用microLib的方法*/

u8 USART3_TX_BUF[USART3_TX_LEN];
  
int SendaByte(int ch, FILE *f)
{
	while((USART3->SR&0X40)==0);//循环发送,直到发送完毕   
    USART3->DR = (u8) ch;      
	return ch;
}
int GetKey (void)  { 

    while (!(USART3->SR & USART_FLAG_RXNE));

    return ((int)(USART3->DR & 0x1FF));
}

void SendString(u8 *string)
{
	while(*string){
		SendaByte(*string,0);
		string++;
	}
}
extern u16 tmr_active_push;
extern u16 tmr_active_count;
extern u16 seattime;
extern float tmr_active_correct;

#if EN_USART3_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART3_RX_BUF[USART3_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART3_RX_STA=0;       //接收状态标记	 

//初始化IO 串口3 
//bound:波特率
void uart3_init(u32 bound){
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能USART3，GPIOB时钟
 	USART_DeInit(USART3);  //复位串口1
	 //USART3_TX   PB.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB.10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;	//复用推挽输出,改下拉
    GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PB10
   
    //USART3_RX	  PB.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);  //初始化PB11

   //Usart3 NVIC 配置

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx;	//收发模式，改接收模式

    USART_Init(USART3, &USART_InitStructure); //初始化串口
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(USART3, ENABLE);                    //使能串口 

}

void USART3_IRQHandler(void)                	//串口3中断服务程序
	{
	u8 Res;
#ifdef SYSTEM_SUPPORT_OS	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
		Res =USART_ReceiveData(USART3);//(USART3->DR);	//读取接收到的数据
		
		if((USART3_RX_STA&0x8000)==0)//接收未完成
			{				
				if(Res==0x0a)USART3_RX_STA|=0x4000;
				else
					{
					USART3_RX_BUF[USART3_RX_STA&0X3FFF]=Res ;
					USART3_RX_STA++;
					if(USART3_RX_STA>(USART3_REC_LEN-1)){
						USART3_RX_STA=0;//接收数据错误,重新开始接收	 
					}
					}
			if(USART3_RX_STA&0x4000)//接收到了0x0a
				{
					USART3_RX_STA|=0x8000;	//接收完成了 
				}						
				
			}   		 
     } 
#ifdef SYSTEM_SUPPORT_OS	 	//如果时钟节拍数定义了,说明要使用ucosII了.
	OSIntExit();  											 
#endif
} 
#endif	

void usart_scan(void)
{
	u8 t,len;
	u8 USART3_TEMP[50];
	u8 datatemp[53] = {0};
	u16 temp = 0,date_temp = 0;
	float correct_temp = 0;
	if(USART3_RX_STA&0x8000)//串口接收完成？
	{					   
			len=USART3_RX_STA&0x3fff;//得到此次接收到的数据长度
			for(t=0;t<len;t++)
			{
				USART3_TEMP[t]=USART3_RX_BUF[t];
			}
			USART3_TEMP[len] = 0;
			if(strncmp((char*)USART3_TEMP,"DATA",4) == 0){
				len = (USART3_TEMP[4] - 0x30) * 29 + 1;
				AT24CXX_Init();
				AT24CXX_Read(len,datatemp,29);
				for(t = 0; t < 29; t++)
					if(READ_BLU)
						printf("data %d = %d  \n",t,datatemp[t]);
				IIC_Off();
			}
			else if(strncmp((char*)USART3_TEMP,"BATTERY",7) == 0){
				temp = Get_Adc_Average(ADC_Channel_3,10);
				if(READ_BLU)
					printf("battery = %d\n",temp);
			}
			else if(strncmp((char*)USART3_TEMP,"PULL",4) == 0){
				tmr_active_push += tmr_active_count;
				correct_temp = (float)tmr_active_count / 60.0;
				temp = tmr_active_count / 60;
				tmr_active_correct = correct_temp - (float)temp;
				Data_Store(tmr_active_count / 60);
				tmr_active_count = 0;
				
				AT24CXX_Init();
				for(t = 0; t < 8; t++){
					AT24CXX_Read(1+t*29,datatemp,29);
					if(READ_BLU)
						printf("DATA%d=",t);
					for(len = 0;len < 29; len++){
						if(READ_BLU){
							printf("%d,",datatemp[len]);
							//USART1->DR=datatemp[len];
							//while((USART1->SR&0X40)==0);//等待发送结束
						}
					}
					if(READ_BLU)
						printf("\n");
				}
				IIC_Off();
			}
			else if(strncmp((char*)USART3_TEMP,"TIME",4) == 0){
				time_data[0] = (USART3_TEMP[4] - 0x30) * 10 + (USART3_TEMP[5] - 0x30);
				time_data[2] = (USART3_TEMP[6] - 0x30) * 10 + (USART3_TEMP[7] - 0x30);
				time_data[3] = (USART3_TEMP[8] - 0x30) * 10 + (USART3_TEMP[9] - 0x30);
				time_data[4] = (USART3_TEMP[10] - 0x30) * 10 + (USART3_TEMP[11] - 0x30);
				time_data[5] = (USART3_TEMP[12] - 0x30) * 10 + (USART3_TEMP[13] - 0x30);
				time_data[6] = (USART3_TEMP[14] - 0x30) * 10 + (USART3_TEMP[15] - 0x30);
				temp = time_data[0] * 559 + time_data[2] * 745 +time_data[3] * 24 +time_data[4] ;
				DS1302_GPIO_Init();
				Read_rtc();
				time_pros();
				DS1302_Off();
				date_temp = disp[0] * 559 + disp[1] * 745 + disp[2] * 24 + disp[3];
				if(temp != date_temp){
					DS1302_GPIO_Init();
					Set_rtc();
					DS1302_Off();
				}
				DS1302_GPIO_Init();
				Read_rtc();
				time_pros();
				DS1302_Off();
				if(READ_BLU){
					for(t = 0;t<6;t++)
						printf("--%d--",disp[t]);
					printf("\n");
				}
				memset(disp,0,6);
			}
			else if(strncmp((char*)USART3_TEMP,"SEATTIME",8) == 0){
				len = (USART3_TEMP[8] - 0x30)*100 + (USART3_TEMP[9] - 0x30)*10 + (USART3_TEMP[10] - 0x30);
				AT24CXX_Init();
				AT24CXX_WriteOneByte(240,len);
				IIC_Off();
				seattime = len * 60;
				if(READ_BLU)
					printf("Time set\n");
			}
			USART3_RX_STA=0;    
	}
}
