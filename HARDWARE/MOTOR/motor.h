#ifndef __BEE_H
#define __BEE_H
#include "sys.h"

#define MOTOR_OFF()  {GPIOB->CRL&=0XFFFFFFF0;GPIOB->CRL|=0X00000004;}
#define MOTOR_ON() {GPIOB->CRL&=0XFFFFFFF0;GPIOB->CRL|=0x00000001;}

#define MOTOR    PBout(0) 

#define MOTOR_ACTIVE 1
#define MOTOR_NEGATIVE 0

void Motor_Init(void);
void Motor_Contrl(u8 sw);

#endif
