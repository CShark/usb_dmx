#ifndef __PLATFORM_H
#define __PLATFORM_H

#define STM32G441xx
#include "lwip/pbuf.h"
#include "stm32g4xx.h"

#pragma pack(1)
typedef struct {
    union {
        __IO unsigned char ID[12];

        struct {
            __IO unsigned short Coordinates[2];
            __IO unsigned char Wafer;
            __IO unsigned char Lot[7];
        };
    };
} UID_TypeDef;
#pragma pack()

#define STM32_SYSMEM 0x1FFF0000
#define UID ((UID_TypeDef *)UID_BASE)

void *memcpy(void *destination, const void *source, unsigned int num);
void memcpy_pbuf(struct pbuf *destination, const void *source, unsigned int num);
void memclr(void *target, unsigned int len);

#endif