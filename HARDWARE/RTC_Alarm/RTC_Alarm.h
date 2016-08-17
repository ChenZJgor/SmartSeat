/*
该模块启动STM32的AWU功能，采用LSI作为RTC时钟，周期性待机和唤醒单片机
移植该模块 :
1、只需调用RTC_Alarm_Configuration配置启动函数，
2、修改工作时间WORK_TIMES、待机时间STANDBY_TIMES，单位秒s，
	设置的为32为闹钟寄存器，0-4294967295s(71582788.25min)

*/
#ifndef __RTC_Alarm_H
#define __RTC_Alarm_H

//外部提供接口


//宏定义，单位s，0-4294967295s(71582788.25min)
//运行时间
#define WORK_TIMES 30
//待机时间
#define STANDBY_TIMES 2

extern void RTC_Alarm_Configuration(void);
void RTC_Off(void);

#endif 

