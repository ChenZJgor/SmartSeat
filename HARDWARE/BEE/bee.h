#ifndef __MOTOR_H
#define __MOTOR_H
#include "sys.h"

#define BEE_OFF()  {GPIOB->CRL&=0XFFFFFF0F;GPIOB->CRL|=0X00000040;}
#define BEE_ON() {GPIOB->CRL&=0XFFFFFF0F;GPIOB->CRL|=0x00000010;}

#define BEE    PBout(1) 

#define BEE_ACTIVE 1
#define BEE_NEGATIVE 0

void Bee_Init(void);
void Bee_Contrl(u8 sw);

#endif
