#ifndef __PLATFORM_H
#define __PLATFORM_H

#define STM32G441xx
#include "stm32g4xx.h"
#include "lwip/pbuf.h"

void *memcpy(void *destination, const void *source, unsigned int num);
void memcpy_pbuf(struct pbuf *destination, const void* source, unsigned int num);

#endif