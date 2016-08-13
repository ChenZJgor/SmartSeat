#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "includes.h"
#include "adc.h"
#include "exti.h"
#include "stm32f10x_adc.h"
#include "24cxx.h"
#include "key.h"
#include "bee.h"
#include "motor.h"
#include "power.h"
#include "ds1302.h"
#include "usart3.h"
#include "stm32f10x_exti.h"
#include "wkup.h"
#include "gpio.h"


//����Щ���ȼ��������UCOSIII��5��ϵͳ�ڲ�����
//���ȼ�0���жϷ������������� OS_IntQTask()
//���ȼ�1��ʱ�ӽ������� OS_TickTask()
//���ȼ�2����ʱ���� OS_TmrTask()
//���ȼ�OS_CFG_PRIO_MAX-2��ͳ������ OS_StatTask()
//���ȼ�OS_CFG_PRIO_MAX-1���������� OS_IdleTask()
//����֧�֣�www.openedv.com
//�Ա����̣�http://eboard.taobao.com  
//�������������ӿƼ����޹�˾  
//���ߣ�����ԭ�� @ALIENTEK

//�������ȼ�
#define START_TASK_PRIO		3
//�����ջ��С	
#define START_STK_SIZE 		512
//������ƿ�
OS_TCB StartTaskTCB;
//�����ջ	
CPU_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *p_arg);

//�������ȼ�
#define ADC_TASK_PRIO		4
//�����ջ��С	
#define ADC_STK_SIZE 		128
//������ƿ�
OS_TCB ADCTaskTCB;
//�����ջ	
CPU_STK ADC_TASK_STK[ADC_STK_SIZE];
void adc_task(void *p_arg);

//�������ȼ�
#define TIMEOUT_TASK_PRIO		5
//�����ջ��С	
#define TIMEOUT_STK_SIZE 		128
//������ƿ�
OS_TCB TIMEOUTTaskTCB;
//�����ջ	
CPU_STK TIMEOUT_TASK_STK[TIMEOUT_STK_SIZE];
void timeout_task(void *p_arg);

//�������ȼ�
#define ACTIVE_TASK_PRIO		6
//�����ջ��С	
#define ACTIVE_STK_SIZE 		128
//������ƿ�
OS_TCB ACTIVETaskTCB;
//�����ջ	
CPU_STK ACTIVE_TASK_STK[ACTIVE_STK_SIZE];
void active_task(void *p_arg);

//�������ȼ�
#define NEGATIVE_TASK_PRIO		6
//�����ջ��С	
#define NEGATIVE_STK_SIZE 		128
//������ƿ�
OS_TCB NEGATIVETaskTCB;
//�����ջ	
CPU_STK NEGATIVE_TASK_STK[NEGATIVE_STK_SIZE];
void negative_task(void *p_arg);

//�������ȼ�
#define BALANCE_TASK_PRIO		7
//�����ջ��С	
#define BALANCE_STK_SIZE 		128
//������ƿ�
OS_TCB BALANCETaskTCB;
//�����ջ	
CPU_STK BALANCE_TASK_STK[BALANCE_STK_SIZE];
void balance_task(void *p_arg);

//�������ȼ�
#define BLUETOOTH_TASK_PRIO		8
//�����ջ��С	
#define BLUETOOTH_STK_SIZE 		128
//������ƿ�
OS_TCB BLUETOOTHTaskTCB;
//�����ջ	
CPU_STK BLUETOOTH_TASK_STK[BLUETOOTH_STK_SIZE];
void bluetooth_task(void *p_arg);

//�������ȼ�
#define BATTERY_TASK_PRIO		9
//�����ջ��С	
#define BATTERY_STK_SIZE 		128
//������ƿ�
OS_TCB BATTERYTaskTCB;
//�����ջ	
CPU_STK BATTERY_TASK_STK[BATTERY_STK_SIZE];
void battery_task(void *p_arg);


////////////////////////////////////////////////////////

#define STRAIN_LEFT 400	//��һӦ��Ƭ������ѹ
#define STRAIN_RIGHT 200	//�ڶ�Ӧ��Ƭ������ѹ

#define ACTIVE_TIME 1800 //�����ʱ�䣬N*1s
#define NEGETIVE_TIME 45 //���߹ػ�ʱ�䣬N*1s
#define TIMEOUT_TIME 30	//��ʱ����ʱ��������ʱ��
#define SPORT_TIME 15	//�����˶�ʱ��

#define B_SAMPLES 10	//ƽ���������

#define EXCELLENT 0
#define GOOD 1
#define OK 2
#define BAD 3
#define SERIOUS 4

#define ON 1
#define OFF 0 

u8 power_flag = 0;	//��Դ��־λ
u8 active_flag = 0;	//Ӧ��Ƭ�����־λ
u8 negative_flag = 0;	//Ӧ��Ƭ���߱�־λ
u8 timeout_flag = 0;	//����ʱ�䳬ʱ��־λ
u8 sport_flag = 0;	//�˶���ʱ����־λ
u8 strain_on = 0;	//ѹ�紫������־λ
u8 bluetooth_on = 0;	//�������ӱ�־λ
u8 motor_flag = 0;	//�񶯵��״̬��־λ
u8 bee_flag = 0;	//������״̬��־λ
u8 adc_count_flag = 0;	//ƽ�������ɱ�־λ
u8 lowpower_flag = 0;		//�͵�����־λ
u8 led_green_flag = 0;	//��ɫLED��״̬��־λ
u8 led_red_flag = 0;		//��ɫLED��״̬��־λ

float tmr_active_correct = 0;

u8 balance_level[5] = {0};	

////////////////////////////////////////////////////////
OS_TMR 	tmr_active;		//���ʱ��
void tmr_active_callback(void *p_tmr, void *p_arg); 	//���ʱ���ص�����
u16 tmr_active_count = 0;
u16 tmr_active_push = 0;
u8 tmr_active_timeout = 0;

OS_TMR tmr_negative;	//���߶�ʱ��
void tmr_negative_callback(void *p_tmr, void *p_arg);	//���߶�ʱ���ص�����
u8 tmr_negative_count = 0;
u8 tmr_negative_timeout = 0;

OS_TMR tmr_timeout;	//��ʱ��ʱ��
void tmr_timeout_callback(void *p_tmr, void *p_arg);	//��ʱ��ʱ���ص�����
u8 tmr_timeout_count = 0;
u8 tmr_timeout_timeout = 0;

OS_TMR tmr_sport;	//�˶���ʱ��
void tmr_sport_callback(void *p_tmr, void *p_arg);	//��ʱ��ʱ���ص�����
u8 tmr_sport_count = 0;
u8 tmr_sport_timeout = 0;

////////////////////////////////////////////////////////
OS_SEM ActiveSem;		//�����ź���
OS_SEM NegativeSem;	//�����ź���
	
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init();       //��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�жϷ�������
	uart_init(9600);    //���ڲ���������
	uart3_init(9600);		//����3����������
	Gpio_Init();
	LED_Init();         //LED��ʼ��
	Adc_Init();					//ADC��ʼ��
	//AT24CXX_Init();			//EEPROM
	IIC_Off();					//�ر�IIC
	Motor_Init();				//�񶯵����ʼ��
	Bee_Init();					//��������ʼ��
	Power_Init();				//Ӧ��Ƭ��Դ��ʼ��
	//DS1302_GPIO_Init();	//DS1302��ʼ��
	DS1302_Off();					//�ر�DS1302
	EXTIX_Init();		//�ⲿ�жϳ�ʼ��
	
	OSInit(&err);		//��ʼ��UCOSIII
	OS_CRITICAL_ENTER();//�����ٽ���
	//������ʼ����
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//������ƿ�
				 (CPU_CHAR	* )"start task", 		//��������
                 (OS_TASK_PTR )start_task, 			//������
                 (void		* )0,					//���ݸ��������Ĳ���
                 (OS_PRIO	  )START_TASK_PRIO,     //�������ȼ�
                 (CPU_STK   * )&START_TASK_STK[0],	//�����ջ����ַ
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//�����ջ�����λ
                 (CPU_STK_SIZE)START_STK_SIZE,		//�����ջ��С
                 (OS_MSG_QTY  )0,					//�����ڲ���Ϣ�����ܹ����յ������Ϣ��Ŀ,Ϊ0ʱ��ֹ������Ϣ
                 (OS_TICK	  )0,					//��ʹ��ʱ��Ƭ��תʱ��ʱ��Ƭ���ȣ�Ϊ0ʱΪĬ�ϳ��ȣ�
                 (void   	* )0,					//�û�����Ĵ洢��
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //����ѡ��
                 (OS_ERR 	* )&err);				//��Ÿú�������ʱ�ķ���ֵ
	OS_CRITICAL_EXIT();	//�˳��ٽ���	 
	OSStart(&err);  //����UCOSIII
	while(1);
}

//��ʼ������
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//ͳ������                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//���ʹ���˲����жϹر�ʱ��
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //��ʹ��ʱ��Ƭ��ת��ʱ��
	 //ʹ��ʱ��Ƭ��ת���ȹ���,ʱ��Ƭ����Ϊ1��ϵͳʱ�ӽ��ģ���1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
		//����Ӧ��Ƭ���ʱ��
	OSTmrCreate((OS_TMR		*)&tmr_active,		//���ʱ��
                (CPU_CHAR	*)"tmr active",		//��ʱ������
                (OS_TICK	 )0,			//0
                (OS_TICK	 )100,          //100*10=1000ms
                (OS_OPT		 )OS_OPT_TMR_PERIODIC, //����ģʽ
                (OS_TMR_CALLBACK_PTR)tmr_active_callback,//���ʱ���ص�����
                (void	    *)0,			//����Ϊ0
                (OS_ERR	    *)&err);		//���صĴ�����
								
		//����Ӧ��Ƭ���߶�ʱ��
	OSTmrCreate((OS_TMR		*)&tmr_negative,		//���߶�ʱ��
                (CPU_CHAR	*)"tmr negative",		//��ʱ������
                (OS_TICK	 )0,			//0
                (OS_TICK	 )100,          //100*10=1000ms
                (OS_OPT		 )OS_OPT_TMR_PERIODIC, //����ģʽ
                (OS_TMR_CALLBACK_PTR)tmr_negative_callback,//���߶�ʱ���ص�����
                (void	    *)0,			//����Ϊ0
                (OS_ERR	    *)&err);		//���صĴ�����
								
		//������ʱ��ʱ��
	OSTmrCreate((OS_TMR		*)&tmr_timeout,		//��ʱ��ʱ��
                (CPU_CHAR	*)"tmr timeout",		//��ʱ������
                (OS_TICK	 )0,			//0
                (OS_TICK	 )100,          //100*10=1000ms
                (OS_OPT		 )OS_OPT_TMR_PERIODIC, //����ģʽ
                (OS_TMR_CALLBACK_PTR)tmr_timeout_callback,//���߶�ʱ���ص�����
                (void	    *)0,			//����Ϊ0
                (OS_ERR	    *)&err);		//���صĴ�����
								
		//�����˶���ʱ��
	OSTmrCreate((OS_TMR		*)&tmr_sport,		//�˶���ʱ��
                (CPU_CHAR	*)"tmr sport",		//��ʱ������
                (OS_TICK	 )0,			//0
                (OS_TICK	 )100,          //100*10=1000ms
                (OS_OPT		 )OS_OPT_TMR_PERIODIC, //����ģʽ
                (OS_TMR_CALLBACK_PTR)tmr_sport_callback,//���߶�ʱ���ص�����
                (void	    *)0,			//����Ϊ0
                (OS_ERR	    *)&err);		//���صĴ�����																			
				
	OS_CRITICAL_ENTER();	//�����ٽ���
								
	OSSemCreate ((OS_SEM*)	&ActiveSem,				//����Ӧ��Ƭ����״̬�ź���
                   (CPU_CHAR*) "ActiveSem",
                   (OS_SEM_CTR)0,
                   (OS_ERR*)	&err);
								
	OSSemCreate ((OS_SEM*)	&NegativeSem,			//����Ӧ��Ƭ����״̬�ź���
                   (CPU_CHAR*) "NegativeSem",
                   (OS_SEM_CTR)0,
                   (OS_ERR*)	&err);									
								
	//����ADC�������
	OSTaskCreate((OS_TCB 	* )&ADCTaskTCB,		
				 (CPU_CHAR	* )"adc task", 		
                 (OS_TASK_PTR )adc_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )ADC_TASK_PRIO,     
                 (CPU_STK   * )&ADC_TASK_STK[0],	
                 (CPU_STK_SIZE)ADC_STK_SIZE/10,	
                 (CPU_STK_SIZE)ADC_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
	
	//������ʱ��������
	OSTaskCreate((OS_TCB 	* )&TIMEOUTTaskTCB,		
				 (CPU_CHAR	* )"timeout task", 		
                 (OS_TASK_PTR )timeout_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )TIMEOUT_TASK_PRIO,     
                 (CPU_STK   * )&TIMEOUT_TASK_STK[0],	
                 (CPU_STK_SIZE)TIMEOUT_STK_SIZE/10,	
                 (CPU_STK_SIZE)TIMEOUT_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);								 
								 
	//����Ӧ��Ƭ��������
	OSTaskCreate((OS_TCB 	* )&ACTIVETaskTCB,		
				 (CPU_CHAR	* )"active task", 		
                 (OS_TASK_PTR )active_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )ACTIVE_TASK_PRIO,     
                 (CPU_STK   * )&ACTIVE_TASK_STK[0],	
                 (CPU_STK_SIZE)ACTIVE_STK_SIZE/10,	
                 (CPU_STK_SIZE)ACTIVE_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )2,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);		
						
	//����Ӧ��Ƭ��������
	OSTaskCreate((OS_TCB 	* )&NEGATIVETaskTCB,		
				 (CPU_CHAR	* )"negative task", 		
                 (OS_TASK_PTR )negative_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )NEGATIVE_TASK_PRIO,     	
                 (CPU_STK   * )&NEGATIVE_TASK_STK[0],	
                 (CPU_STK_SIZE)NEGATIVE_STK_SIZE/10,	
                 (CPU_STK_SIZE)NEGATIVE_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )2,	//2��ʱ��Ƭ����2*5=10ms					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);						

	//����ƽ��������
	OSTaskCreate((OS_TCB 	* )&BALANCETaskTCB,		
				 (CPU_CHAR	* )"balance task", 		
                 (OS_TASK_PTR )balance_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )BALANCE_TASK_PRIO,     	
                 (CPU_STK   * )&BALANCE_TASK_STK[0],	
                 (CPU_STK_SIZE)BALANCE_STK_SIZE/10,	
                 (CPU_STK_SIZE)BALANCE_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,			
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);

	//��������ͨѶ����
	OSTaskCreate((OS_TCB 	* )&BLUETOOTHTaskTCB,		
				 (CPU_CHAR	* )"bluetooth task", 		
                 (OS_TASK_PTR )bluetooth_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )BLUETOOTH_TASK_PRIO,     	
                 (CPU_STK   * )&BLUETOOTH_TASK_STK[0],	
                 (CPU_STK_SIZE)BLUETOOTH_STK_SIZE/10,	
                 (CPU_STK_SIZE)BLUETOOTH_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,			
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);		
								 
	//������ص����������
	OSTaskCreate((OS_TCB 	* )&BATTERYTaskTCB,		
				 (CPU_CHAR	* )"battery task", 		
                 (OS_TASK_PTR )battery_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )BATTERY_TASK_PRIO,     	
                 (CPU_STK   * )&BATTERY_TASK_STK[0],	
                 (CPU_STK_SIZE)BATTERY_STK_SIZE/10,	
                 (CPU_STK_SIZE)BATTERY_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,			
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);										 
				 			 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//����ʼ����			 
	OS_CRITICAL_EXIT();	//�����ٽ���						 
								 
	ADC_Cmd(ADC1, DISABLE);			//�ر�ADC1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);			//�رմ���1ͨ��ʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE); 		//�رմ���3ͨ��ʱ��
	USART_Cmd(USART1, DISABLE);	//�رմ���1
	USART_Cmd(USART3, DISABLE);	//�رմ���3
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1	, DISABLE );	  //�ر�ADC1ͨ��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, DISABLE);			//�ر�GPIOAͨ��ʱ��
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, DISABLE );		//�ر�GPIOBͨ��ʱ��
	Sys_Enter_Standby();
}

//adc������
void adc_task(void *p_arg)
{
	static u16 adc_count = 0;	
	u16 adc_left = 0,adc_right = 0, adc_diff = 0;
	u8 adc_diff_level = 0;
//	u8 datatemp[100]={0};
//	u8 i ;
//	u8 bt05_flag = 0;
	
	OS_ERR err;
	EXTI_InitTypeDef EXTI_InitStructure;
	p_arg = p_arg;
	
	while(1)
	{
/*		
		if(bt05_flag == 0){
			memset(USART3_TX_BUF,0,200);
			strcpy((char*)USART3_TX_BUF,"AT\r\n");
			SendString(USART3_TX_BUF);
			bt05_flag = 1;
			printf("cmd send\n");
		}*/

		//printf("AT\n\r");
		//Bee_Contrl(BEE_ACTIVE);
		//Motor_Contrl(MOTOR_ACTIVE);	
/*		if(time_init_flag == 0){
			AT24CXX_Init();
			AT24CXX_Write(146,datatemp,87);
			time_init_flag = 1;
		}*/
/*		AT24CXX_Init();
		Data_Store(1);
		AT24CXX_Read(30,datatemp,29);
		for(i = 0; i < 30; i++)
			printf("data %d = %d  \n",i,datatemp[i]);
		IIC_Off();
*/
//		AT24CXX_WriteOneByte(255,66);
//		i = AT24CXX_ReadOneByte(255);
//		printf("data = %d  \n",i);
		
//		printf("adc running\n");
		if(strain_on){			
			//GPIOA.15	  �ر�A.15�ⲿ�ж�
			GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource15);

			EXTI_InitStructure.EXTI_Line=EXTI_Line15;
			EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
			EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
			EXTI_InitStructure.EXTI_LineCmd = DISABLE;
			EXTI_Init(&EXTI_InitStructure);	  	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
			
			Adc_Init();	//����ADC1
			
			if(power_flag == OFF){
				Power_Contrl(POWER_ACTIVE);					//����Ӧ��Ƭ������Դ
				power_flag = ON;
			}
			
			if(lowpower_flag == 0){
				if(led_green_flag == OFF){					//��ɫLED��˸
					LED0_ON();
					LED0 = ON;
					led_green_flag = ON;
				}
				else{
					LED0_OFF();
					led_green_flag = OFF;
				}
			}
			adc_left = Get_Adc_Average(ADC_Channel_1,10);
			adc_right = Get_Adc_Average(ADC_Channel_2,10);
//			printf("adc green = %d, adc blue = %d\n",adc_left,adc_right);
			adc_diff = adc_left - adc_right;	//��ȡ����Ӧ��Ƭ֮��������ѹ��
			if(adc_left >= STRAIN_LEFT || adc_right >= STRAIN_RIGHT){
				OSSemPost (&ActiveSem, OS_OPT_POST_1, &err);	//���ͼ����ź���
				
				if(adc_count_flag == 0){											//ƽ����ֵͳ��
//					printf("adc diff = %d\n",adc_diff);
					if(adc_diff >= 376){
						adc_diff_level = adc_diff - 376;
//						if(READ_BLU)
//							printf("diff level more than 500 = %d\n\r",adc_diff_level);
						if(adc_diff_level <=15)
							balance_level[0]++;
						else if(adc_diff_level >15 && adc_diff_level <= 24)
							balance_level[1]++;
						else if(adc_diff_level >24 && adc_diff_level <= 36)
							balance_level[2]++;
						else if(adc_diff_level >36 && adc_diff_level <= 150)
							balance_level[3]++;
						else if(adc_diff_level >150)
							balance_level[4]++;
					}
					else if(adc_diff < 376){
						adc_diff_level = 376 - adc_diff;
//						if(READ_BLU)
//							printf("diff level less than 500 = %d\n\r",adc_diff_level);
						if(adc_diff_level <=15)
							balance_level[0]++;
						else if(adc_diff_level >15 && adc_diff_level <= 24)
							balance_level[1]++;
						else if(adc_diff_level >24 && adc_diff_level <= 36)
							balance_level[2]++;
						else if(adc_diff_level >36 && adc_diff_level <= 150)
							balance_level[3]++;
						else if(adc_diff_level >150)
							balance_level[4]++;
					}
					adc_count++;
				}
			}
			else if(adc_left < STRAIN_LEFT && adc_right < STRAIN_RIGHT){
				OSSemPost (&NegativeSem, OS_OPT_POST_1, &err);		//���������ź���
			}
			
			if(adc_count >= B_SAMPLES){													//ƽ��ͳ�ƽ���												
				adc_count = 0;
				adc_count_flag = 1;
			}
		}
		OSTimeDlyHMSM (0,0,1,0,OS_OPT_TIME_PERIODIC,&err);
	}
}

void timeout_task(void *p_arg)
{
	u16 data_store;
	OS_ERR err;
	p_arg = p_arg;
	
	while(1){
		if(tmr_active_timeout){																//���ʱ����ʱ����
			if(timeout_flag == 0){
				Motor_Contrl(MOTOR_ACTIVE);												//�������
				Bee_Contrl(BEE_ACTIVE);														//����������
				motor_flag = ON;																	//������Ϊ����״̬	
				bee_flag = ON;																		//���������Ϊ����״̬
				tmr_negative_count = 0;														//���߼�������
				tmr_negative_timeout = 0;													//���߳�ʱ�������
				OSTmrStart(&tmr_timeout,&err);										//������ʱ��ʱ��
				OSTmrStop (&tmr_active,OS_OPT_TMR_NONE,0,&err);		//�ر�Ӧ��Ƭ�����ʱ��
				//AT24CXX_Init();
				//DS1302_GPIO_Init();
				//Read_rtc();
				//time_pros();
				//DS1302_Off();
				//date_temp = disp[0] * 608 + disp[1] * 808 + disp[2]*26 + disp[3];	
				//if(date_temp > date_mark){
				//	date_mark = date_temp;
				//	tmr_active_push = 0;
			//	}
			//	tmr_active_push += tmr_active_count;
				data_store = ((u16)(tmr_active_correct + 0.5)) + tmr_active_count / 60;
				Data_Store(data_store);
				tmr_active_push = 0;
				tmr_active_count = 0;															//Ӧ��Ƭ�����ʱ������
				tmr_active_correct = 0;
				//IIC_Off();														

				
				timeout_flag = 1;																	//��ʱ��ʱ��־��1
			}
			if(negative_flag){																	//�����뿪����
				if(motor_flag){
					Motor_Contrl(MOTOR_NEGATIVE);										//�رյ��
					motor_flag = OFF;																//������Ϊ�ر�״̬
//					printf("motor off\n");
				}
				if(bee_flag){
					Bee_Contrl(BEE_NEGATIVE);																					//�رշ�����
					bee_flag = OFF;
//					printf("bee off\n");
				}
				if(sport_flag == 0){
					OSTmrStart(&tmr_sport,&err);										//�����˶���ʱ��
					sport_flag = 1;
				}
			}
			if(active_flag){																		//������������
				if(motor_flag == OFF){
					Motor_Contrl(MOTOR_ACTIVE);											//�������
					motor_flag = ON;

//					printf("motor going again\n");
				}
				if(bee_flag == OFF){
					bee_flag = ON;
				}
				if(bee_flag){
					Bee_Contrl(BEE_ACTIVE);
					delay_ms(10);
					Bee_Contrl(BEE_NEGATIVE);
				}
				if(sport_flag){
					OSTmrStop (&tmr_sport,OS_OPT_TMR_NONE,0,&err);	//�ر��˶���ʱ��
					sport_flag = 0;																	//�˶���ʱ��־����
				}
			}
/*			if(tmr_timeout_count >= 10){		
				if(bee_flag){
					Bee_Contrl(BEE_NEGATIVE);																						//����10��رշ�����
					bee_flag = OFF;
					printf("more than 10s, bee off\n");
				}
			}*/
			if(tmr_sport_timeout){															//�˶���ʱ����ʱ���
//				printf("finish sport, more than 5 mins, motor off\n");
				Motor_Contrl(MOTOR_NEGATIVE);											//�رյ��
				motor_flag = OFF;
				Bee_Contrl(BEE_NEGATIVE);													//�رշ�����
				bee_flag = OFF;

				OSTmrStop (&tmr_timeout,OS_OPT_TMR_NONE,0,&err);	//�رճ�ʱ��ʱ��
				tmr_timeout_count = 0;														//��ʱ��ʱ����������
				tmr_timeout_timeout = 0;													//��ʱ��ʱ��������ʱ��־����
				timeout_flag = 0;																	//��ʱ��ʱ��־����
				OSTmrStop (&tmr_sport,OS_OPT_TMR_NONE,0,&err);		//�ر��˶���ʱ��
				sport_flag = 0;																		//�˶���ʱ��־����
				tmr_sport_count = 0;															//�˶���ʱ����������
				tmr_sport_timeout = 0;														//�˶���ʱ����־λ����
				
				active_flag = 0;																	//Ӧ��Ƭ�����־λ����
				tmr_active_timeout = 0;														//Ӧ��Ƭ�����ʱ��������ʱ��־λ����
			}
			if(tmr_timeout_timeout){
//				printf("more than 10 mins, turn off motor\n");
				Motor_Contrl(MOTOR_NEGATIVE);
				motor_flag = OFF;
				Bee_Contrl(BEE_NEGATIVE);																					//�رշ�����
				bee_flag = OFF;

				OSTmrStop (&tmr_timeout,OS_OPT_TMR_NONE,0,&err);
				tmr_timeout_count = 0;
				timeout_flag = 0;
				tmr_timeout_timeout = 0;
				OSTmrStop (&tmr_sport,OS_OPT_TMR_NONE,0,&err);
				sport_flag = 0;
				tmr_sport_count = 0;
				tmr_sport_timeout = 0;
				
				active_flag = 0;
				tmr_active_timeout = 0;
			}
		}
		if(tmr_negative_timeout){
//			printf("negative timeout\n");
			Motor_Contrl(MOTOR_NEGATIVE);											//�رյ��
			motor_flag = OFF;
			Bee_Contrl(BEE_NEGATIVE);													//�رշ�����
			bee_flag = OFF;
			OSTmrStop (&tmr_active,OS_OPT_TMR_NONE,0,&err);
			OSTmrStop (&tmr_negative,OS_OPT_TMR_NONE,0,&err);
			//AT24CXX_Init();
			//DS1302_GPIO_Init();
			//Read_rtc();
			//time_pros();
			//DS1302_Off();
			//date_temp = disp[0] * 608 + disp[1] * 808 + disp[2]*26 + disp[3];	
			//if(date_temp > date_mark){
			//	date_mark = date_temp;
			//	tmr_active_push = 0;
		//	}
			//tmr_active_push += tmr_active_count;
			
			data_store = ((u16)(tmr_active_correct + 0.5)) + tmr_active_count / 60;
			Data_Store(data_store);
			tmr_active_push = 0;
			tmr_active_count = 0;															//Ӧ��Ƭ�����ʱ������
			tmr_active_correct = 0;
			//IIC_Off();
			strain_on = 0;
			tmr_negative_timeout = 0;
			tmr_negative_count = 0;
			LED0_OFF();
			led_green_flag = OFF;
			LED1_OFF();
			led_red_flag = OFF;
			Power_Contrl(POWER_NEGATIVE);					//�ر�Ӧ��Ƭ��Դ
			power_flag = OFF;
			negative_flag = 0;
			
			ADC_Cmd(ADC1, DISABLE);								//�ر�ADC
			USART_Cmd(USART1, DISABLE);						//�رմ���1
			USART_Cmd(USART3, DISABLE);						//�رմ���3
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1	, DISABLE );	  //�ر�ADC1ͨ��ʱ��
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);			//�رմ���1ͨ��ʱ��
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE); 		//�رմ���3ͨ��ʱ��								
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, DISABLE);			//�ر�GPIOAͨ��ʱ��
			RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, DISABLE );		//�ر�GPIOBͨ��ʱ��	
			
			//GPIOA.15	  ����A.15�ж�
			EXTIX_Init();												//�����ж�
			
			Sys_Enter_Standby();								//����ͣ��ģʽ
			
		}
		OSTimeDlyHMSM (0,0,1,0,OS_OPT_TIME_PERIODIC,&err);
	}
}

void active_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	
	while(1){
		OSSemPend (&ActiveSem,0,OS_OPT_PEND_BLOCKING,0,&err);
//		printf("tmr active going\n");
		if(active_flag == 0){
			if(timeout_flag == 0){
				OSTmrStart(&tmr_active, &err);
			}
			OSTmrStop (&tmr_negative,OS_OPT_TMR_NONE,0,&err);
			tmr_negative_count = 0;
			active_flag = 1;
			negative_flag = 0;
		}
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ1s
	}
}

void negative_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	
	while(1){
		OSSemPend (&NegativeSem,0,OS_OPT_PEND_BLOCKING,0,&err);
//		printf("tmr_negative going\n");
		if(negative_flag == 0){
			OSTmrStop (&tmr_active,OS_OPT_TMR_NONE,0,&err);
			OSTmrStart(&tmr_negative, &err);
			negative_flag = 1;
			active_flag = 0;
		}
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //��ʱ1s
	}
}

//ƽ��������
void balance_task(void *p_arg)
{	
	OS_ERR err;
	u8 max,i,balance_name=0;
	p_arg = p_arg;
	
	while(1){
		if(adc_count_flag){
//			printf("balance task is running\n");						
			max = balance_level[0];
			for(i=1; i<5; i++){
				if(balance_level[i] > max)
					max = balance_level[i];
			}
			for(i=0; i<5; i++){
				if(max == balance_level[i])
					balance_name = i;
			}
			if(balance_name == EXCELLENT){
				if(balance_level[EXCELLENT] >= B_SAMPLES/2){
//					printf("The balance is excellent\n");
						Bee_Contrl(BEE_NEGATIVE);					
				}
				else{
//					printf("THE balance is good\n");
						Bee_Contrl(BEE_NEGATIVE);	
				}
			}
			else if(balance_name == GOOD){
				if(balance_level[GOOD] >= B_SAMPLES/2){
//					printf("The balance is good\n");
						Bee_Contrl(BEE_NEGATIVE);					
				}
				else{
//					printf("THE balance is OK\n");
						Bee_Contrl(BEE_NEGATIVE);					
				}
			}
			else if(balance_name == OK){
				if(balance_level[OK] >= B_SAMPLES/2){
//					printf("The balance is OK\n");
						Bee_Contrl(BEE_NEGATIVE);						
				}
				else{
//					printf("THE balance is bad\n");
						Bee_Contrl(BEE_ACTIVE);
						delay_ms(10);
						Bee_Contrl(BEE_NEGATIVE);
				}
			}
			else if(balance_name == BAD){
				if(balance_level[BAD] >= B_SAMPLES/2){
//					printf("The balance is bad\n");
						Bee_Contrl(BEE_ACTIVE);
						delay_ms(10);
						Bee_Contrl(BEE_NEGATIVE);						
				}
				else{
//					printf("THE balance is serious\n");
						Bee_Contrl(BEE_ACTIVE);
						delay_ms(10);
						Bee_Contrl(BEE_NEGATIVE);					
				}
			}
			else if(balance_name == SERIOUS){
//				printf("THE balance is serious\n");
					Bee_Contrl(BEE_ACTIVE);
					delay_ms(10);
					Bee_Contrl(BEE_NEGATIVE);				
			}
			memset(balance_level,0,5);
			adc_count_flag = 0;
		}
		OSTimeDlyHMSM (0,0,1,0,OS_OPT_TIME_PERIODIC,&err);
	}
}

//����ͨѶ����
void bluetooth_task(void *p_arg)
{
//	u16 date_temp = 0;
	OS_ERR err;
	p_arg = p_arg;
	
	while(1){
/*		if(data_push_flag == 1){
			DS1302_GPIO_Init();
			Read_rtc();
			time_pros();
			DS1302_Off();
			date_temp = disp[0] * 608 + disp[1] * 808 + disp[2]*26 + disp[3];	
			if(date_temp > date_mark){
				date_mark = date_temp;
				tmr_active_push = 0;
				data_push_flag = 0;
			}			
		}*/
		if(bluetooth_on || READ_BLU){
			if(strain_on){
				if(READ_BLU){
					usart_scan();
				}
				else if(READ_BLU == 0){
					bluetooth_on = 0;
				}
			}
			else if(strain_on == 0){
				if(READ_BLU){
					Adc_Init();		//����ADC1
					usart_scan();
				}
				else if(READ_BLU == 0){
					bluetooth_on = 0;
					LED1_OFF();
					led_red_flag = OFF;
					LED0_OFF();
					led_green_flag = OFF;
					
					ADC_Cmd(ADC1, DISABLE);								//�ر�ADC
					USART_Cmd(USART1, DISABLE);						//�رմ���1
					USART_Cmd(USART3, DISABLE);						//�رմ���3
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1	, DISABLE );	  //�ر�ADC1ͨ��ʱ��
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);			//�رմ���1ͨ��ʱ��
					RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE); 		//�رմ���3ͨ��ʱ��								
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, DISABLE);			//�ر�GPIOAͨ��ʱ��
					RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, DISABLE );		//�ر�GPIOBͨ��ʱ��	
					
					Sys_Enter_Standby();
				}
			}
		}
		OSTimeDlyHMSM (0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}

//��ص����������
void battery_task(void *p_arg)
{
	u16 battery;
	OS_ERR err;
	p_arg = p_arg;
	
	while(1){
		battery = Get_Adc_Average(ADC_Channel_3,10);
		if(battery < 865){
			lowpower_flag = 1;
			if(led_red_flag == OFF){
				LED1_ON();
				LED1 = ON;
				led_red_flag = ON;
			}
			else{
				LED1_OFF();
				led_red_flag = OFF;
			}
		}
		else{
			lowpower_flag = 0;
			LED1_OFF();
			led_red_flag = OFF;
		}
		OSTimeDlyHMSM (0,0,5,0,OS_OPT_TIME_HMSM_STRICT,&err);
	}
	
}

//���ʱ���Ļص�����
void tmr_active_callback(void *p_tmr, void *p_arg)
{
	tmr_active_count++;
	if((tmr_active_count + tmr_active_push) >= ACTIVE_TIME){
		tmr_active_timeout = 1;
	}
}

//���߶�ʱ���Ļص�����
void tmr_negative_callback(void *p_tmr, void *p_arg)
{
	tmr_negative_count++;
	if(tmr_negative_count >= NEGETIVE_TIME){
		tmr_negative_timeout = 1;
		tmr_negative_count = 0;
	}
}

//��ʱ��ʱ���ص�����
void tmr_timeout_callback(void *p_tmr, void *p_arg)
{
	tmr_timeout_count++;
	if(tmr_timeout_count >= TIMEOUT_TIME){
		tmr_timeout_timeout = 1;
		tmr_timeout_count = 0;
	}
}

void tmr_sport_callback(void *p_tmr, void *p_arg)
{
	tmr_sport_count++;
	if(tmr_sport_count >= SPORT_TIME){
		tmr_sport_timeout = 1;
		tmr_sport_count = 0;
	}
}