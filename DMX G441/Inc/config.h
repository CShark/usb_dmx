#ifndef __CONFIG_H
#define __CONFIG_H

#include "lwip/netif.h"

#define ARTNET_NET 0
#define ARTNET_SUB 0
#define ARTNET_UNI 0

#pragma pack(4) // 4 Byte alignment for flash storage
typedef enum {
    CONFIGIP_Auto = 0,
    CONFIGIP_Static = 1,
    CONFIGIP_DHCP = 2
} CONFIG_IP_MODE;

typedef struct {
    ip_addr_t DhcpServerSelf;
    ip_addr_t DhcpServerClient;
    ip_addr_t DhcpServerSubnet;

    ip_addr_t StaticIp;
    ip_addr_t StaticSubnet;
    ip_addr_t StaticGateway;

    CONFIG_IP_MODE Mode;
    char DhcpServerEnable;

    char ArtNetShortName[18];
    char ArtNetLongName[64];
    char ArtNetNetwork;
    char ArtNetSubnet;
    char ArtNetUniverse[8];
} CONFIG;
#pragma pack()

void Config_Init(struct netif *netif);

void Config_Reset();

void Config_ResetIp();
void Config_SetMode(CONFIG_IP_MODE mode);
void Config_DhcpServer(char enable, ip_addr_t host, ip_addr_t client, ip_addr_t subnet);
void Config_SetIp(const char *ip);
void Config_SetGateway(const char *gw);
void Config_SetNetmask(const char *net);

CONFIG* Config_GetActive();
void Config_ApplyNetwork();
void Config_Store();

CONFIG Config_GetDefault();

#endif