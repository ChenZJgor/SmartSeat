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
#include "RTC_Alarm.h"
#include "iwdg.h"
#include "stm32f10x_iwdg.h"


//将这些优先级分配给了UCOSIII的5个系统内部任务
//优先级0：中断服务服务管理任务 OS_IntQTask()
//优先级1：时钟节拍任务 OS_TickTask()
//优先级2：定时任务 OS_TmrTask()
//优先级OS_CFG_PRIO_MAX-2：统计任务 OS_StatTask()
//优先级OS_CFG_PRIO_MAX-1：空闲任务 OS_IdleTask()
//技术支持：www.openedv.com
//淘宝店铺：http://eboard.taobao.com  
//广州市星翼电子科技有限公司  
//作者：正点原子 @ALIENTEK

//任务优先级
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		512
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);

//任务优先级
#define CORE_TASK_PRIO		4
//任务堆栈大小	
#define CORE_STK_SIZE 		128
//任务控制块
OS_TCB CORETaskTCB;
//任务堆栈	
CPU_STK CORE_TASK_STK[CORE_STK_SIZE];
void core_task(void *p_arg);

//任务优先级
#define TIMEOUT_TASK_PRIO		5
//任务堆栈大小	
#define TIMEOUT_STK_SIZE 		128
//任务控制块
OS_TCB TIMEOUTTaskTCB;
//任务堆栈	
CPU_STK TIMEOUT_TASK_STK[TIMEOUT_STK_SIZE];
void timeout_task(void *p_arg);

//任务优先级
#define ACTIVE_TASK_PRIO		6
//任务堆栈大小	
#define ACTIVE_STK_SIZE 		128
//任务控制块
OS_TCB ACTIVETaskTCB;
//任务堆栈	
CPU_STK ACTIVE_TASK_STK[ACTIVE_STK_SIZE];
void active_task(void *p_arg);

//任务优先级
#define NEGATIVE_TASK_PRIO		6
//任务堆栈大小	
#define NEGATIVE_STK_SIZE 		128
//任务控制块
OS_TCB NEGATIVETaskTCB;
//任务堆栈	
CPU_STK NEGATIVE_TASK_STK[NEGATIVE_STK_SIZE];
void negative_task(void *p_arg);

//任务优先级
#define BALANCE_TASK_PRIO		7
//任务堆栈大小	
#define BALANCE_STK_SIZE 		128
//任务控制块
OS_TCB BALANCETaskTCB;
//任务堆栈	
CPU_STK BALANCE_TASK_STK[BALANCE_STK_SIZE];
void balance_task(void *p_arg);

//任务优先级
#define BLUETOOTH_TASK_PRIO		8
//任务堆栈大小	
#define BLUETOOTH_STK_SIZE 		128
//任务控制块
OS_TCB BLUETOOTHTaskTCB;
//任务堆栈	
CPU_STK BLUETOOTH_TASK_STK[BLUETOOTH_STK_SIZE];
void bluetooth_task(void *p_arg);

//任务优先级
#define BATTERY_TASK_PRIO		9
//任务堆栈大小	
#define BATTERY_STK_SIZE 		128
//任务控制块
OS_TCB BATTERYTaskTCB;
//任务堆栈	
CPU_STK BATTERY_TASK_STK[BATTERY_STK_SIZE];
void battery_task(void *p_arg);


////////////////////////////////////////////////////////

#define STRAIN_LEFT 400	//第一应变片开启电压
#define STRAIN_RIGHT 200	//第二应变片开启电压

#define ACTIVE_TIME seattime //最长静坐时间，N*1s
#define NEGETIVE_TIME 45 //休眠关机时间，N*1s
#define TIMEOUT_TIME 30	//超时处理时，电机最长震动时间
#define SPORT_TIME 15	//最少运动时间

#define EXCELLENT 0
#define GOOD 1
#define OK 2
#define BAD 3
#define SERIOUS 4

#define ON 1
#define OFF 0 

u16 seattime = 1800;	//最长静坐时间，N*1s

u8 power_flag = 0;	//电源标志位
u8 active_flag = 0;	//应变片激活标志位
u8 negative_flag = 0;	//应变片休眠标志位
u8 timeout_flag = 0;	//静坐时间超时标志位
u8 sport_flag = 0;	//运动计时器标志位
u8 strain_on = 0;	//压电传感器标志位
u8 bluetooth_on = 0;	//蓝牙连接标志位 
u8 motor_flag = 0;	//振动电机状态标志位
u8 bee_flag = 0;	//蜂鸣器状态标志位
u8 lowpower_flag = 0;		//低电量标志位
u8 led_green_flag = 0;	//绿色LED灯状态标志位
u8 led_red_flag = 0;		//红色LED灯状态标志位
u8 store_hour_flag = 0;	//小时储存标志
u8 system_init_flag = 0;	//系统初始化标志位
u8 balance_hour_flag = 0;
//u8 time_set_flag = 0;

float tmr_active_correct = 0;

u16 balance = 0;
u16 balance_level = 0;
u8 posture_store = 0;
//u8 balance_level[5] = {0};	

////////////////////////////////////////////////////////
OS_TMR 	tmr_active;		//激活定时器
void tmr_active_callback(void *p_tmr, void *p_arg); 	//激活定时器回调函数
u16 tmr_active_count = 0;
u16 tmr_active_push = 0;
u8 tmr_active_timeout = 0;

OS_TMR tmr_negative;	//休眠定时器
void tmr_negative_callback(void *p_tmr, void *p_arg);	//休眠定时器回调函数
u8 tmr_negative_count = 0;
u8 tmr_negative_timeout = 0;

OS_TMR tmr_timeout;	//超时定时器
void tmr_timeout_callback(void *p_tmr, void *p_arg);	//超时定时器回调函数
u8 tmr_timeout_count = 0;
u8 tmr_timeout_timeout = 0;

OS_TMR tmr_sport;	//运动定时器
void tmr_sport_callback(void *p_tmr, void *p_arg);	//超时定时器回调函数
u8 tmr_sport_count = 0;
u8 tmr_sport_timeout = 0;

OS_TMR tmr_store;	//储存定时器
void tmr_store_callback(void *p_tmr, void *p_arg);	//超时定时器回调函数
u16 tmr_store_count = 0;
u8 tmr_store_timeout = 0;

////////////////////////////////////////////////////////
OS_SEM ActiveSem;		//激活信号量
OS_SEM NegativeSem;	//休眠信号量
	
int main(void)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	
	delay_init();       //延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //中断分组配置
	uart_init(9600);    //串口波特率设置
	uart3_init(9600);		//串口3波特率设置
	Gpio_Init();
	LED_Init();         //LED初始化
	Adc_Init();					//ADC初始化
	//AT24CXX_Init();			//EEPROM
	IIC_Off();					//关闭IIC
	Motor_Init();				//振动电机初始化
	Bee_Init();					//蜂鸣器初始化
	Power_Init();				//应变片电源初始化
	//DS1302_GPIO_Init();	//DS1302初始化
	DS1302_Off();					//关闭DS1302
	EXTIX_Init();		//外部中断初始化
	IWDG_Init(IWDG_Prescaler_256,0x0fff);
	RTC_Alarm_Configuration();
	
	OSInit(&err);		//初始化UCOSIII
	OS_CRITICAL_ENTER();//进入临界区
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);  //开启UCOSIII
	while(1);
}

//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
		//创建应变片激活定时器
	OSTmrCreate((OS_TMR		*)&tmr_active,		//激活定时器
                (CPU_CHAR	*)"tmr active",		//定时器名字
                (OS_TICK	 )0,			//0
                (OS_TICK	 )100,          //100*10=1000ms
                (OS_OPT		 )OS_OPT_TMR_PERIODIC, //周期模式
                (OS_TMR_CALLBACK_PTR)tmr_active_callback,//激活定时器回调函数
                (void	    *)0,			//参数为0
                (OS_ERR	    *)&err);		//返回的错误码
								
		//创建应变片休眠定时器
	OSTmrCreate((OS_TMR		*)&tmr_negative,		//休眠定时器
                (CPU_CHAR	*)"tmr negative",		//定时器名字
                (OS_TICK	 )0,			//0
                (OS_TICK	 )100,          //100*10=1000ms
                (OS_OPT		 )OS_OPT_TMR_PERIODIC, //周期模式
                (OS_TMR_CALLBACK_PTR)tmr_negative_callback,//休眠定时器回调函数
                (void	    *)0,			//参数为0
                (OS_ERR	    *)&err);		//返回的错误码
								
		//创建超时定时器
	OSTmrCreate((OS_TMR		*)&tmr_timeout,		//超时定时器
                (CPU_CHAR	*)"tmr timeout",		//定时器名字
                (OS_TICK	 )0,			//0
                (OS_TICK	 )100,          //100*10=1000ms
                (OS_OPT		 )OS_OPT_TMR_PERIODIC, //周期模式
                (OS_TMR_CALLBACK_PTR)tmr_timeout_callback,//休眠定时器回调函数
                (void	    *)0,			//参数为0
                (OS_ERR	    *)&err);		//返回的错误码
								
		//创建运动定时器
	OSTmrCreate((OS_TMR		*)&tmr_sport,		//运动定时器
                (CPU_CHAR	*)"tmr sport",		//定时器名字
                (OS_TICK	 )0,			//0
                (OS_TICK	 )100,          //100*10=1000ms
                (OS_OPT		 )OS_OPT_TMR_PERIODIC, //周期模式
                (OS_TMR_CALLBACK_PTR)tmr_sport_callback,//休眠定时器回调函数
                (void	    *)0,			//参数为0
                (OS_ERR	    *)&err);		//返回的错误码		

		//创建储存定时器
	OSTmrCreate((OS_TMR		*)&tmr_store,		//储存定时器
                (CPU_CHAR	*)"tmr store",		//定时器名字
                (OS_TICK	 )0,			//0
                (OS_TICK	 )100,          //100*10=1000ms
                (OS_OPT		 )OS_OPT_TMR_PERIODIC, //周期模式
                (OS_TMR_CALLBACK_PTR)tmr_store_callback,//休眠定时器回调函数
                (void	    *)0,			//参数为0
                (OS_ERR	    *)&err);		//返回的错误码									
				
	OS_CRITICAL_ENTER();	//进入临界区
								
	OSSemCreate ((OS_SEM*)	&ActiveSem,				//创建应变片激活状态信号量
                   (CPU_CHAR*) "ActiveSem",
                   (OS_SEM_CTR)0,
                   (OS_ERR*)	&err);
								
	OSSemCreate ((OS_SEM*)	&NegativeSem,			//创建应变片休眠状态信号量
                   (CPU_CHAR*) "NegativeSem",
                   (OS_SEM_CTR)0,
                   (OS_ERR*)	&err);									
								
	//创建系统核心任务
	OSTaskCreate((OS_TCB 	* )&CORETaskTCB,		
				 (CPU_CHAR	* )"core task", 		
                 (OS_TASK_PTR )core_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )CORE_TASK_PRIO,     
                 (CPU_STK   * )&CORE_TASK_STK[0],	
                 (CPU_STK_SIZE)CORE_STK_SIZE/10,	
                 (CPU_STK_SIZE)CORE_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);
	
	//创建超时处理任务
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
								 
	//创建应变片激活任务
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
						
	//创建应变片休眠任务
	OSTaskCreate((OS_TCB 	* )&NEGATIVETaskTCB,		
				 (CPU_CHAR	* )"negative task", 		
                 (OS_TASK_PTR )negative_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )NEGATIVE_TASK_PRIO,     	
                 (CPU_STK   * )&NEGATIVE_TASK_STK[0],	
                 (CPU_STK_SIZE)NEGATIVE_STK_SIZE/10,	
                 (CPU_STK_SIZE)NEGATIVE_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )2,	//2个时间片，既2*5=10ms					
                 (void   	* )0,				
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, 
                 (OS_ERR 	* )&err);						

	//创建平衡检测任务
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

	//创建蓝牙通讯任务
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
								 
	//创建电池电量检测任务
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
				 			 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//挂起开始任务			 
	OS_CRITICAL_EXIT();	//进入临界区								 
}

//核心任务函数
void core_task(void *p_arg)
{

	u16 adc_left = 0,adc_right = 0, adc_diff = 0;
	u16 temp = 0,temp_date = 0;
	float correct_temp = 0;
//	u8 datatemp[100]={0};
//	u8 i ;
//	u8 bt05_flag = 0;	
	OS_ERR err;
	p_arg = p_arg;
	
	while(1)
	{
//调试代码
//////////////////////////////////////////////		
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
/*		Posture_Store('A');
		AT24CXX_Init();
		AT24CXX_Read(0,datatemp,54);
		for(i = 0; i < 54; i++)
			printf("data %d = %d  \n",i,datatemp[i]);
		IIC_Off();
*/
//		AT24CXX_WriteOneByte(255,66);
//		i = AT24CXX_ReadOneByte(255);
//		printf("data = %d  \n",i);
		
//		printf("adc running\n");
//////////////////////////////////////////////
		if(system_init_flag == 0){			
			if((Get_Adc_Average(ADC_Channel_3,10)) < 869){				
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
				
				Sys_Enter_Shutdown();
			}
//			RTC_Off();
			AT24CXX_Init();							 
			seattime = (AT24CXX_ReadLenByte(239,2)) * 60;									//读取坐下提醒时间
			IIC_Off();
											 
			ADC_Cmd(ADC1, DISABLE);			//关闭ADC1
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);			//关闭串口1通道时钟
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE); 		//关闭串口3通道时钟
			USART_Cmd(USART1, DISABLE);	//关闭串口1
			USART_Cmd(USART3, DISABLE);	//关闭串口3
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1	, DISABLE );	  //关闭ADC1通道时钟
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, DISABLE);			//关闭GPIOA通道时钟
			RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, DISABLE );		//关闭GPIOB通道时钟
			system_init_flag = 1;
			Sys_Enter_Standby();
		}
		
		if(strain_on == 0 && bluetooth_on == 0){
			Sys_Enter_Standby();
		}

		if(strain_on){
			EXTA15_Off();	//GPIOA.15	  关闭A.15外部中断			
			
			Adc_Init();	//开启ADC1
			
			if(power_flag == OFF){
				Power_Contrl(POWER_ACTIVE);					//开启应变片激励电源
				power_flag = ON;
			}
			
			if(lowpower_flag == 0){
				if(led_green_flag == OFF){					//绿色LED闪烁
					LED0_ON();
					LED0 = ON;
					led_green_flag = ON;
				}
				else{
					LED0_OFF();
					led_green_flag = OFF;
				}
			}
			if(store_hour_flag == 0){							//检测系统时间，每小时记录一次数据
				OSTmrStop (&tmr_store,OS_OPT_TMR_NONE,0,&err);
				tmr_store_count = 0;
				DS1302_GPIO_Init();
				Read_rtc();
				time_pros();
				DS1302_Off();
				if((disp[4] == 59) && (disp[5] == 59)){
					Posture_Store(posture_store);
					DS1302_GPIO_Init();
					Read_rtc();
					time_pros();
					DS1302_Off();
					temp = disp[0] * 559 + disp[1] * 745 + disp[2] * 24 + disp[3];
					AT24CXX_Init();	
					AT24CXX_WriteLenByte(241,temp,2);
					AT24CXX_WriteLenByte(243,balance,2);
					IIC_Off();
					balance_hour_flag = 0;			
					
					tmr_active_push += tmr_active_count;
					correct_temp = (float)tmr_active_count / 60.0;
					temp = tmr_active_count / 60;
					tmr_active_correct = correct_temp - (float)temp;
					Data_Store(tmr_active_count / 60);
					tmr_active_count = 0;
					OSTmrStart(&tmr_store,&err);
					store_hour_flag = 1;
				}
			}
			
			adc_left = Get_Adc_Average(ADC_Channel_1,10);
			adc_right = Get_Adc_Average(ADC_Channel_2,10);
//			printf("adc green = %d, adc blue = %d\n",adc_left,adc_right);
			if(adc_left - adc_right > 0)
				adc_diff = adc_left - adc_right;	//获取两组应变片之间的输出电压差
			else if(adc_left - adc_right < 0)
				adc_diff = adc_right - adc_left;
			else
				adc_diff = 0;
			if(balance_hour_flag == 0){
				DS1302_GPIO_Init();
				Read_rtc();
				time_pros();
				DS1302_Off();
				temp = AT24CXX_ReadLenByte(241, 2);
				temp_date = disp[0] * 559 + disp[1] * 745 + disp[2] * 24 + disp[3];
				if(temp == temp_date){
					balance = AT24CXX_ReadLenByte(243, 2);
				}
				else{
					balance = adc_diff;
				}
				balance_hour_flag = 1;
			}				
			balance = (balance + adc_diff) / 2;
			
			if(adc_left >= STRAIN_LEFT || adc_right >= STRAIN_RIGHT){
				OSSemPost (&ActiveSem, OS_OPT_POST_1, &err);	//发送激活信号量
//				if(READ_BLU)
//					printf("adc diff = %d\n",adc_diff);
				if(balance >= 370){
					balance_level = balance - 370;
//				if(READ_BLU)
//					printf("diff level more than 500 = %d\n\r",balance_level);
				}
				else if(balance < 370){
					balance_level = 370 - balance;
//				if(READ_BLU)
//					printf("diff level less than 500 = %d\n\r",adc_diff_level);
				}
			}
			else if(adc_left < STRAIN_LEFT && adc_right < STRAIN_RIGHT){
				OSSemPost (&NegativeSem, OS_OPT_POST_1, &err);		//发送休眠信号量
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
		if(tmr_active_timeout){																//激活定时器超时处理
			if(timeout_flag == 0){
				Motor_Contrl(MOTOR_ACTIVE);												//开启电机
				Bee_Contrl(BEE_ACTIVE);														//开启蜂鸣器
				delay_ms(10);
				Bee_Contrl(BEE_NEGATIVE);
				motor_flag = ON;																	//电机标记为开启状态	
				bee_flag = ON;																		//蜂鸣器标记为开启状态
				tmr_negative_count = 0;														//休眠计数清零
				tmr_negative_timeout = 0;													//休眠超时标记清零
				OSTmrStart(&tmr_timeout,&err);										//开启超时计时器
				OSTmrStop (&tmr_active,OS_OPT_TMR_NONE,0,&err);		//关闭应变片激活计时器
				
				data_store = ((u16)(tmr_active_correct + 0.6)) + tmr_active_count / 60;
				Data_Store(data_store);
				tmr_active_push = 0;
				tmr_active_count = 0;															//应变片激活计时器清零
				tmr_active_correct = 0;														//时间校准变量清零

				Posture_Store(posture_store);
				DS1302_GPIO_Init();
				Read_rtc();
				time_pros();
				DS1302_Off();
				data_store = disp[0] * 559 + disp[1] * 745 + disp[2] * 24 + disp[3];
				AT24CXX_Init();	
				AT24CXX_WriteLenByte(241,data_store,2);
				AT24CXX_WriteLenByte(243,balance,2);
				IIC_Off();
				balance_hour_flag = 0;		
				
				timeout_flag = 1;																	//超时计时标志置1
			}
			if(negative_flag){																	//当人离开坐垫
				if(motor_flag){
					Motor_Contrl(MOTOR_NEGATIVE);										//关闭电机
					motor_flag = OFF;																//电机标记为关闭状态
//					printf("motor off\n");
				}
				if(bee_flag){
					Bee_Contrl(BEE_NEGATIVE);												//关闭蜂鸣器
					bee_flag = OFF;
//					printf("bee off\n");
				}
				if(sport_flag == 0){
					OSTmrStart(&tmr_sport,&err);										//启动运动计时器
					sport_flag = 1;
				}
			}
			if(active_flag){																		//当人在坐垫上
				if(motor_flag == OFF){
					Motor_Contrl(MOTOR_ACTIVE);											//启动电机
					motor_flag = ON;

//					printf("motor going again\n");
				}
				if(bee_flag == OFF){															//启动蜂鸣器
					bee_flag = ON;
				}
				if(bee_flag){
					Bee_Contrl(BEE_ACTIVE);
					delay_ms(10);
					Bee_Contrl(BEE_NEGATIVE);
				}
				if(sport_flag){
					OSTmrStop (&tmr_sport,OS_OPT_TMR_NONE,0,&err);	//关闭运动定时器
					sport_flag = 0;																	//运动计时标志清零
				}
			}
/*			if(tmr_timeout_count >= 10){		
				if(bee_flag){
					Bee_Contrl(BEE_NEGATIVE);																						//大于10秒关闭蜂鸣器
					bee_flag = OFF;
					printf("more than 10s, bee off\n");
				}
			}*/
			if(tmr_sport_timeout){															//运动计时器计时完毕
//				printf("finish sport, more than 5 mins, motor off\n");
				Motor_Contrl(MOTOR_NEGATIVE);											//关闭电机
				motor_flag = OFF;
				Bee_Contrl(BEE_NEGATIVE);													//关闭蜂鸣器
				bee_flag = OFF;

				OSTmrStop (&tmr_timeout,OS_OPT_TMR_NONE,0,&err);	//关闭超时计时器
				tmr_timeout_count = 0;														//超时计时器计数清零
				tmr_timeout_timeout = 0;													//超时计时器计数超时标志清零
				timeout_flag = 0;																	//超时任务执行标志
				OSTmrStop (&tmr_sport,OS_OPT_TMR_NONE,0,&err);		//关闭运动计时器
				sport_flag = 0;																		//运动计时标志清零
				tmr_sport_count = 0;															//运动计时器计数清零
				tmr_sport_timeout = 0;														//运动计时器标志位清零
				
				active_flag = 0;																	//应变片激活标志位清零
				tmr_active_timeout = 0;														//应变片激活计时器计数超时标志位清零
			}
			if(tmr_timeout_timeout){														//超过一定时间未运动，关闭提醒
//				printf("more than 10 mins, turn off motor\n");
				Motor_Contrl(MOTOR_NEGATIVE);
				motor_flag = OFF;
				Bee_Contrl(BEE_NEGATIVE);																					//关闭蜂鸣器
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
			Motor_Contrl(MOTOR_NEGATIVE);											//关闭电机
			motor_flag = OFF;
			Bee_Contrl(BEE_NEGATIVE);													//关闭蜂鸣器
			bee_flag = OFF;
			OSTmrStop (&tmr_active,OS_OPT_TMR_NONE,0,&err);
			OSTmrStop (&tmr_negative,OS_OPT_TMR_NONE,0,&err);
			
			data_store = ((u16)(tmr_active_correct + 0.6)) + tmr_active_count / 60;
			Data_Store(data_store);
			tmr_active_push = 0;
			tmr_active_count = 0;															//应变片激活计时器清零
			tmr_active_correct = 0;
			store_hour_flag = 0;															//小时储存标志位清零
			
			Posture_Store(posture_store);
			DS1302_GPIO_Init();
			Read_rtc();
			time_pros();
			DS1302_Off();
			data_store = disp[0] * 559 + disp[1] * 745 + disp[2] * 24 + disp[3];
			AT24CXX_Init();	
			AT24CXX_WriteLenByte(241,data_store,2);
			AT24CXX_WriteLenByte(243,balance,2);
			IIC_Off();
			balance_hour_flag = 0;	

			strain_on = 0;
			tmr_negative_timeout = 0;
			tmr_negative_count = 0;
			LED0_OFF();
			led_green_flag = OFF;
			LED1_OFF();
			led_red_flag = OFF;
			Power_Contrl(POWER_NEGATIVE);					//关闭应变片电源
			power_flag = OFF;
			negative_flag = 0;
			
			ADC_Cmd(ADC1, DISABLE);								//关闭ADC
			USART_Cmd(USART1, DISABLE);						//关闭串口1
			USART_Cmd(USART3, DISABLE);						//关闭串口3
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1	, DISABLE );	  //关闭ADC1通道时钟
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);			//关闭串口1通道时钟
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE); 		//关闭串口3通道时钟								
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, DISABLE);			//关闭GPIOA通道时钟
			RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, DISABLE );		//关闭GPIOB通道时钟	
			
			//GPIOA.15	  开启A.15中断
			EXTIX_Init();												//开启中断
			
			Sys_Enter_Standby();								//进入停机模式
			
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
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1s
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
		OSTimeDlyHMSM(0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err); //延时1s
	}
}

//平衡检测任务
void balance_task(void *p_arg)
{	
	OS_ERR err;
//	u8 max,i,balance_name=0;
	p_arg = p_arg;
	
	while(1){
		if(balance_level <= 40){
			posture_store &= 0x00;
			posture_store |= 0x00;
			//printf("The balance is excellent\n");
			if(active_flag && tmr_active_timeout == 0){
				Bee_Contrl(BEE_NEGATIVE);	
			}
		}
		else if(balance_level >40 && balance_level <= 100){
			posture_store &=  0x00;
			posture_store |= 0x40;
			if(active_flag && tmr_active_timeout == 0){
				Bee_Contrl(BEE_ACTIVE);
				delay_ms(5);
				Bee_Contrl(BEE_NEGATIVE);
			}
		}	
		else if(balance_level >100){
			posture_store &= 0x00;
			posture_store |= 0x80;
			if(active_flag && tmr_active_timeout == 0){
				Bee_Contrl(BEE_ACTIVE);
				delay_ms(5);
				Bee_Contrl(BEE_NEGATIVE);
				delay_ms(50);
				Bee_Contrl(BEE_ACTIVE);
				delay_ms(5);
				Bee_Contrl(BEE_NEGATIVE);
			}
		}
		OSTimeDlyHMSM (0,0,5,0,OS_OPT_TIME_PERIODIC,&err);
	}
}

//蓝牙通讯任务
void bluetooth_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	
	while(1){
		
		if(bluetooth_on){
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
					Adc_Init();		//开启ADC1
					usart_scan();
				}
				else if(READ_BLU == 0){
					bluetooth_on = 0;
					LED1_OFF();
					led_red_flag = OFF;
					LED0_OFF();
					led_green_flag = OFF;
					
					ADC_Cmd(ADC1, DISABLE);								//关闭ADC
					USART_Cmd(USART1, DISABLE);						//关闭串口1
					USART_Cmd(USART3, DISABLE);						//关闭串口3
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1	, DISABLE );	  //关闭ADC1通道时钟
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);			//关闭串口1通道时钟
					RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE); 		//关闭串口3通道时钟								
					RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, DISABLE);			//关闭GPIOA通道时钟
					RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, DISABLE );		//关闭GPIOB通道时钟	
					
					Sys_Enter_Standby();
				}
			}
		}
		OSTimeDlyHMSM (0,0,1,0,OS_OPT_TIME_HMSM_STRICT,&err);
	}
}

//电池电量检测任务
void battery_task(void *p_arg)
{
	u16 battery = 0,data_store = 0;
	OS_ERR err;
	p_arg = p_arg;
	
	while(1){
		battery = Get_Adc_Average(ADC_Channel_3,10);
		if(battery < 869){
			Motor_Contrl(MOTOR_NEGATIVE);											//关闭电机
			motor_flag = OFF;
			Bee_Contrl(BEE_NEGATIVE);													//关闭蜂鸣器
			bee_flag = OFF;
			OSTmrStop (&tmr_active,OS_OPT_TMR_NONE,0,&err);
			OSTmrStop (&tmr_negative,OS_OPT_TMR_NONE,0,&err);
			
			data_store = ((u16)(tmr_active_correct + 0.6)) + tmr_active_count / 60;
			Data_Store(data_store);
			tmr_active_push = 0;
			tmr_active_count = 0;															//应变片激活计时器清零
			tmr_active_correct = 0;
			store_hour_flag = 0;															//小时储存标志位清零
			
			Posture_Store(posture_store);
			DS1302_GPIO_Init();
			Read_rtc();
			time_pros();
			DS1302_Off();
			data_store = disp[0] * 559 + disp[1] * 745 + disp[2] * 24 + disp[3];
			AT24CXX_Init();	
			AT24CXX_WriteLenByte(241,data_store,2);
			AT24CXX_WriteLenByte(243,balance,2);
			IIC_Off();
			balance_hour_flag = 0;	

			strain_on = 0;
			tmr_negative_timeout = 0;
			tmr_negative_count = 0;
			LED0_OFF();
			led_green_flag = OFF;
			LED1_OFF();
			led_red_flag = OFF;
			Power_Contrl(POWER_NEGATIVE);					//关闭应变片电源
			power_flag = OFF;
			negative_flag = 0;
			
			ADC_Cmd(ADC1, DISABLE);								//关闭ADC
			USART_Cmd(USART1, DISABLE);						//关闭串口1
			USART_Cmd(USART3, DISABLE);						//关闭串口3
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1	, DISABLE );	  //关闭ADC1通道时钟
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);			//关闭串口1通道时钟
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE); 		//关闭串口3通道时钟								
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, DISABLE);			//关闭GPIOA通道时钟
			RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, DISABLE );		//关闭GPIOB通道时钟	
			
			//GPIOA.15	  开启A.15中断
			EXTIX_Init();												//开启中断
//			RTC_Alarm_Configuration();					//开启RTC闹钟
			Sys_Enter_Shutdown();								//进入待机模式
		}
		if(battery < 880){
			lowpower_flag = 1;
			LED0_OFF();
			led_green_flag = OFF;
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

//激活定时器的回调函数
void tmr_active_callback(void *p_tmr, void *p_arg)
{
	tmr_active_count++;
	if((tmr_active_count + tmr_active_push) >= ACTIVE_TIME){
		tmr_active_timeout = 1;
	}
}

//休眠定时器的回调函数
void tmr_negative_callback(void *p_tmr, void *p_arg)
{
	tmr_negative_count++;
	if(tmr_negative_count >= NEGETIVE_TIME){
		tmr_negative_timeout = 1;
		tmr_negative_count = 0;
	}
}

//超时定时器回调函数
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

void tmr_store_callback(void *p_tmr, void *p_arg)
{
	tmr_store_count++;
	if(tmr_store_count >= 3595){
		tmr_store_timeout = 1;
		tmr_store_count = 0;
		store_hour_flag = 0;
	}
}
