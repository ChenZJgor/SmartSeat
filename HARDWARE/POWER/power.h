#ifndef __POWER_H
#define __POWER_H
#include "sys.h"

#define POWER_OFF()  {GPIOB->CRL&=0XFF0FFFFF;GPIOB->CRL|=0X00400000;}
#define POWER_ON() {GPIOB->CRL&=0XFF0FFFFF;GPIOB->CRL|=0x00100000;}

#define POWER    PBout(5) 

#define POWER_ACTIVE 1
#define POWER_NEGATIVE 0

void Power_Init(void);
void Power_Contrl(u8 sw);

#endif
