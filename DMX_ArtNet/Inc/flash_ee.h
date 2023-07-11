#ifndef __FLASH_EE_H_
#define __FLASH_EE_H_

#include "platform.h"
#include "config.h"

void EE_ReadConfig(CONFIG *ptr);
void EE_WriteConfig(CONFIG *config);
void EE_ClearConfig();
void EE_ReadFailover(unsigned char* buffer, int idx);
void EE_WriteFailover(const unsigned char *buffer, unsigned char art_port);
#endif