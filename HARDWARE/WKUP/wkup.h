#ifndef __WKUP_H
#define __WKUP_H	 
#include "sys.h"
#include "stm32f10x_pwr.h"

void Sys_Enter_Standby(void);	//系统进入停机模式
void Sys_Enter_Shutdown(void);	//系统进入待机模式
#endif


