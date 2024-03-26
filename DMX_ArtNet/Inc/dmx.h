#ifndef __DMX_H__
#define __DMX_H__

#include "config.h"
#include "platform.h"
#include "usart.h"

void DMX_Init(USART_PortConfig *port, CONFIG *config);
char DMX_Tick(USART_PortConfig *port);

void DMX_SetSingleMode(unsigned char i, unsigned char enabled);
void DMX_SetInDisabled(unsigned char i, unsigned char disabled);
void DMX_InputCallback(USART_PortConfig *port);

char DMX_IsInputEnabled(DMX_PortMetadata *port);

unsigned char* DMX_GetBuffer(unsigned char i);
void DMX_SetBuffer(unsigned char i, const unsigned char *data, unsigned short len);
void DMX_ClearBuffer(unsigned char i);
char DMX_IsInputNew(unsigned char i);


#endif
