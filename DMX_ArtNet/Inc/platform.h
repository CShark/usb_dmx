#ifndef __PLATFORM_H
#define __PLATFORM_H

#ifndef STM32G484xx
#define STM32G484xx
#endif

#include "lwip/pbuf.h"
#include "stm32g4xx.h"

// String representation and continuous number for firmware revisions
#define FIRMWARE_VER "2.1"
#define FIRMWARE_INT 5

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

//#if BYTE_ORDER == LITTLE_ENDIAN
//#define UI16_LITTLE_ENDIAN(x) (x)
//#define UI16_BIG_ENDIAN(x) ((x & 0xFF00) >> 8 | (x & 0x00FF) << 8)
//#elif BYTE_ORDER == BIG_ENDIAN
#define UI16_BIG_ENDIAN(x) (x)
#define UI16_LITTLE_ENDIAN(x) ((x & 0xFF00) >> 8 | (x & 0x00FF) << 8)

#define UI32_BIG_ENDIAN(x) (x)
#define UI32_LITTLE_ENDIAN(x) ((x & 0xFF000000) >> 24 | (x & 0x00FF0000) >> 8 | (x & 0x0000FF00) << 8 | (x & 0x000000FF) << 24)
//#endif

// dummy define for compatibilty with autogen for arduino
#define PROGMEM

#define STM32_SYSMEM 0x1FFF0000
#define UID ((UID_TypeDef *)UID_BASE)

void *memcpy(void *destination, const void *source, unsigned int num);
void memcpy_pbuf(struct pbuf *destination, const void *source, unsigned int num);
char pbufcpy_mem(void *target, const struct pbuf *p, unsigned int maxlen);
void memclr(void *target, unsigned int len);
int memcmp(const void *a, const void *b, unsigned int num);

#endif
