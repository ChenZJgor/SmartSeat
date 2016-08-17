#include "sys.h"
#include "usart3.h"	  
#include "24cxx.h"
#include "adc.h"
#include "stm32f10x_adc.h"
#include "ds1302.h"
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��os,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//os ʹ��	  
#endif

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 0
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

/*ʹ��microLib�ķ���*/

u8 USART3_TX_BUF[USART3_TX_LEN];
  
int SendaByte(int ch, FILE *f)
{
	while((USART3->SR&0X40)==0);//ѭ������,ֱ���������   
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

#if EN_USART3_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART3_RX_BUF[USART3_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART3_RX_STA=0;       //����״̬���	 

//��ʼ��IO ����3 
//bound:������
void uart3_init(u32 bound){
    //GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��USART3��GPIOBʱ��
 	USART_DeInit(USART3);  //��λ����1
	 //USART3_TX   PB.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB.10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;	//�����������,������
    GPIO_Init(GPIOB, &GPIO_InitStructure); //��ʼ��PB10
   
    //USART3_RX	  PB.11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(GPIOB, &GPIO_InitStructure);  //��ʼ��PB11

   //Usart3 NVIC ����

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx;	//�շ�ģʽ���Ľ���ģʽ

    USART_Init(USART3, &USART_InitStructure); //��ʼ������
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ��� 

}

void USART3_IRQHandler(void)                	//����3�жϷ������
	{
	u8 Res;
#ifdef SYSTEM_SUPPORT_OS	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
		{
		Res =USART_ReceiveData(USART3);//(USART3->DR);	//��ȡ���յ�������
		
		if((USART3_RX_STA&0x8000)==0)//����δ���
			{				
				if(Res==0x0a)USART3_RX_STA|=0x4000;
				else
					{
					USART3_RX_BUF[USART3_RX_STA&0X3FFF]=Res ;
					USART3_RX_STA++;
					if(USART3_RX_STA>(USART3_REC_LEN-1)){
						USART3_RX_STA=0;//�������ݴ���,���¿�ʼ����	 
					}
					}
			if(USART3_RX_STA&0x4000)//���յ���0x0a
				{
					USART3_RX_STA|=0x8000;	//��������� 
				}						
				
			}   		 
     } 
#ifdef SYSTEM_SUPPORT_OS	 	//���ʱ�ӽ�����������,˵��Ҫʹ��ucosII��.
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
	if(USART3_RX_STA&0x8000)//���ڽ�����ɣ�
	{					   
			len=USART3_RX_STA&0x3fff;//�õ��˴ν��յ������ݳ���
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
							//while((USART1->SR&0X40)==0);//�ȴ����ͽ���
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
