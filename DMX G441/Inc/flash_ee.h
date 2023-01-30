#ifndef __FLASH_EE_H_
#define __FLASH_EE_H_

#include "platform.h"
#include "config.h"

void EE_ReadConfig(CONFIG *ptr);
void EE_WriteConfig(CONFIG *config);

#endif