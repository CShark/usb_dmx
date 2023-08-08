#include "flash_ee.h"

/*
Category3 Device (512KB, dual bank)
Virt-Addr | Value
*/
// #define PAGE_SIZE 2048
#define PAGE_SIZE 4096
#define PAGE_NUM 126
#define FLASH_OFFSET 0x08000000

static volatile unsigned int *FlashConfigPage = (volatile unsigned int *)(FLASH_OFFSET + (PAGE_NUM * PAGE_SIZE));
static volatile unsigned int *FlashFailoverPage = (volatile unsigned int *)(FLASH_OFFSET + ((PAGE_NUM + 1) * PAGE_SIZE));
static unsigned int currentOffset = -1;

static void EE_ClearConfigPage();
static void EE_ClearFailoverPage();
static void EE_UnlockFlash();
static void EE_LockFlash();

static void EE_UnlockFlash() {
    while (FLASH->SR & FLASH_SR_BSY) {
    }

    FLASH->KEYR = 0x45670123;
    FLASH->KEYR = 0xCDEF89AB;
}

static void EE_LockFlash() {
    while (FLASH->SR & FLASH_SR_BSY) {
    }

    FLASH->CR |= FLASH_CR_LOCK;
}

static void EE_ClearConfigPage() {
    while (FLASH->SR & FLASH_SR_BSY) {
    }

    FLASH->SR = FLASH_SR_PGSERR;
    FLASH->CR |= FLASH_CR_PER;
    FLASH->CR &= ~FLASH_CR_PNB_Msk;
    FLASH->CR |= (PAGE_NUM << FLASH_CR_PNB_Pos);

    FLASH->CR |= FLASH_CR_STRT;

    while (FLASH->SR & FLASH_SR_BSY) {
    }

    FLASH->CR &= ~FLASH_CR_PER;

    if (FLASH->SR & FLASH_SR_PGSERR) {
        // Failed to erase page
        return;
    }

    FLASH->CR |= FLASH_CR_PG;
    FlashConfigPage[0] = 0xA0A0A0A0;
    FlashConfigPage[1] = 0x0123ABCD;

    while (FLASH->SR & FLASH_SR_BSY) {
    }

    FLASH->SR &= ~FLASH_SR_EOP;

    FLASH->CR &= ~FLASH_CR_PG;
    currentOffset = 2;
}

static void EE_ClearFailoverPage() {
    while (FLASH->SR & FLASH_SR_BSY) {
    }

    FLASH->SR = FLASH_SR_PGSERR;
    FLASH->CR |= FLASH_CR_PER;
    FLASH->CR &= ~FLASH_CR_PNB_Msk;
    FLASH->CR |= ((PAGE_NUM + 1) << FLASH_CR_PNB_Pos);

    FLASH->CR |= FLASH_CR_STRT;

    while (FLASH->SR & FLASH_SR_BSY) {
    }

    FLASH->CR &= ~FLASH_CR_PER;
}

void EE_ReadConfig(CONFIG *ptr) {
    // Check Header
    if (FlashConfigPage[0] != 0xA0A0A0A0 || FlashConfigPage[1] != 0x0123ABCD) {
        // return default, format page
        EE_UnlockFlash();
        EE_ClearConfigPage();
        EE_ClearFailoverPage();
        EE_LockFlash();
    } else {
        // Get active config
        for (currentOffset = 2; currentOffset < PAGE_SIZE / sizeof(int); currentOffset += 2) {
            unsigned int addr = FlashConfigPage[currentOffset];
            unsigned int val = FlashConfigPage[currentOffset + 1];

            if (addr == 0xFFFFFFFF) {
                return;
            }

            *(((unsigned int *)ptr) + addr) = val;
        }
    }
}

void EE_WriteConfig(CONFIG *config) {
    CONFIG active = Config_GetDefault();
    EE_ReadConfig(&active);

    unsigned int *srcPtr = (unsigned int *)config;
    unsigned int *origPtr = (unsigned int *)&active;

    unsigned int changes = 0;

    for (int i = 0; i < sizeof(CONFIG) / sizeof(int); i++) {
        if (srcPtr[i] != origPtr[i]) {
            changes++;
        }
    }

    unsigned int free = ((PAGE_SIZE / sizeof(int)) - currentOffset) / 2;

    if (changes > 0) {
        EE_UnlockFlash();
        if (free < changes) {
            EE_ClearConfigPage();
            active = Config_GetDefault();
        }

        FLASH->CR |= FLASH_CR_PG;
        for (int i = 0; i < sizeof(CONFIG) / sizeof(int); i++) {
            if (srcPtr[i] != origPtr[i]) {
                FlashConfigPage[currentOffset] = i;
                FlashConfigPage[currentOffset + 1] = srcPtr[i];
                currentOffset += 2;

                while (FLASH->SR & FLASH_SR_BSY) {
                }
                FLASH->SR &= ~FLASH_SR_EOP;
            }
        }
        FLASH->CR &= ~FLASH_CR_PG;

        EE_LockFlash();
    }
}

void EE_ClearConfig() {
    EE_UnlockFlash();

    EE_ClearConfigPage();
    EE_ClearFailoverPage();

    EE_LockFlash();
}

void EE_ReadFailover(unsigned char *buffer, int idx) {
    if (idx >= 0 && idx < 4) {
        memcpy(buffer, ((unsigned char *)FlashFailoverPage) + idx * 512, 512);
    }
}

void EE_WriteFailover(const unsigned char *buffer, unsigned char art_port) {
    char buffers[4][512];
    memcpy(buffers, (unsigned char *)FlashFailoverPage, 4 * 512);
    memcpy(buffers[art_port], buffer, 512);

    EE_UnlockFlash();
    EE_ClearFailoverPage();

    FLASH->CR |= FLASH_CR_PG;

    for (int i = 0; i < 4; i++) {
        // change to 4 byte width
        unsigned int *page = (unsigned int *)buffers[i];

        for (int x = 0; x < 128; x += 2) {
            FlashFailoverPage[i * 128 + x] = page[x];
            FlashFailoverPage[i * 128 + x + 1] = page[x + 1];

            while (FLASH->SR & FLASH_SR_BSY) {
            }
            FLASH->SR &= ~FLASH_SR_EOP;
        }
    }

    FLASH->CR &= ~FLASH_CR_PG;
    EE_LockFlash();
}
