/*
��ģ������STM32��AWU���ܣ�����LSI��ΪRTCʱ�ӣ������Դ����ͻ��ѵ�Ƭ��
��ֲ��ģ�� :
1��ֻ�����RTC_Alarm_Configuration��������������
2���޸Ĺ���ʱ��WORK_TIMES������ʱ��STANDBY_TIMES����λ��s��
	���õ�Ϊ32Ϊ���ӼĴ�����0-4294967295s(71582788.25min)

*/
#ifndef __RTC_Alarm_H
#define __RTC_Alarm_H

//�ⲿ�ṩ�ӿ�


//�궨�壬��λs��0-4294967295s(71582788.25min)
//����ʱ��
#define WORK_TIMES 30
//����ʱ��
#define STANDBY_TIMES 2

extern void RTC_Alarm_Configuration(void);
void RTC_Off(void);

#endif 
