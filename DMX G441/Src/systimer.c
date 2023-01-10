#include "systimer.h"

static volatile unsigned long globalTime_ms = 0;

void Systick_Init() {
    unsigned int loadVal = SystemCoreClock / 1000 / 8;

    if (loadVal == 0) {
        loadVal = SystemCoreClock / 1000;
        SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;
    } else {
        SysTick->CTRL &= ~SysTick_CTRL_CLKSOURCE_Msk;
    }

    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    SysTick->LOAD = loadVal;
    SysTick->VAL = 0;

    NVIC_SetPriority(SysTick_IRQn, 0);
    NVIC_EnableIRQ(SysTick_IRQn);

    SysTick->CTRL |= 1;
}

void SysTick_Handler() {
    globalTime_ms++;
}

unsigned int sys_jiffies() {
    return globalTime_ms;
}

unsigned int sys_now() {
    return globalTime_ms;
}

void delay_ms(unsigned int ms) {
    unsigned long time = globalTime_ms;

    while (globalTime_ms - time < ms) {
    }
}