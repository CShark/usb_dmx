#include "flash_ee.h"

/*
Category2 Device (128KB, no dual bank)
Virt-Addr | Value
*/
#define PAGE_SIZE 2048
#define PAGE_NUM 63
#define PAGE_OFFSET PAGE_NUM *PAGE_SIZE
#define FLASH_OFFSET 0x08000000

static unsigned int *FlashPage = FLASH_OFFSET + PAGE_OFFSET;
static unsigned int currentOffset = -1;

static void EE_ClearPage();
static void EE_UnlockFlash();
static void EE_LockFlash();
static void EE_BeginWrite();
static void EE_EndWrite();

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

static void EE_ClearPage() {
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
    FlashPage[0] = 0xA0A0A0A0;
    FlashPage[1] = 0x0123ABCD;

    while (FLASH->SR & FLASH_SR_BSY) {
    }

    FLASH->SR &= ~FLASH_SR_EOP;

    FLASH->CR &= FLASH_CR_PG;
    currentOffset = 2;
}

void EE_ReadConfig(CONFIG *ptr) {
    // Check Header
    if (FlashPage[0] != 0xA0A0A0A0 || FlashPage[1] != 0x0123ABCD) {
        // return default, format page
        EE_UnlockFlash();
        EE_ClearPage();
        EE_LockFlash();
    } else {
        // Get active config
        for (currentOffset = 2; currentOffset < PAGE_SIZE / sizeof(int); currentOffset += 2) {
            unsigned int addr = FlashPage[currentOffset];
            unsigned int val = FlashPage[currentOffset + 1];

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

    unsigned int *srcPtr = config;
    unsigned int *origPtr = &active;

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
            EE_ClearPage();
            active = Config_GetDefault();
        } 

        FLASH->CR |= FLASH_CR_PG;
        for (int i = 0; i < sizeof(CONFIG) / sizeof(int); i++) {
            if (srcPtr[i] != origPtr[i]) {
                FlashPage[currentOffset] = i;
                FlashPage[currentOffset + 1] = srcPtr[i];
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
