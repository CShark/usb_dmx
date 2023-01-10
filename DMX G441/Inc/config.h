#ifndef __CONFIG_H
#define __CONFIG_H

#include "lwip/netif.h"

void Config_Init(struct netif *netif);

void Config_Reset();

void Config_ResetIp();
void Config_EnableDhcp();
void Config_SetIp(const char *ip);
void Config_SetGateway(const char *gw);
void Config_SetNetmask(const char *net);

#endif