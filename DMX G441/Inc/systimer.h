#ifndef __SYSTIMER_H
#define __SYSTIMER_H

#include "platform.h"

void Systick_Init();
unsigned int sys_jiffies();
unsigned int sys_now();
void delay_ms(unsigned int ms);

#endif