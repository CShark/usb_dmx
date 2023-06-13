#include "config.h"
#include "eth/dhcp_server.h"
#include "flash_ee.h"
#include "lwip/autoip.h"
#include "lwip/dhcp.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"

static struct netif *netif;
static CONFIG activeConfig;

void Config_Init(struct netif *net) {
    netif = net;

    Config_Reset();
    EE_ReadConfig(&activeConfig);
    Config_ApplyNetwork();
}

CONFIG *Config_GetActive() {
    return &activeConfig;
}

void Config_Reset() {
    Config_ResetIp();
}

void Config_ResetIp() {
    activeConfig = Config_GetDefault();
}

void Config_DhcpServer(char enable, ip_addr_t host, ip_addr_t client, ip_addr_t subnet) {
    activeConfig.DhcpServerEnable = enable;
    activeConfig.DhcpServerSelf = host;
    activeConfig.DhcpServerClient = client;
    activeConfig.DhcpServerSubnet = subnet;
}

void Config_SetMode(CONFIG_IP_MODE mode) {
    activeConfig.Mode = mode;
}

void Config_SetIp(const char *ip) {
    IP4_ADDR(&activeConfig.StaticIp, ip[0], ip[1], ip[2], ip[3]);
}

void Config_SetGateway(const char *gw) {
    IP4_ADDR(&activeConfig.StaticGateway, gw[0], gw[1], gw[2], gw[3]);
}

void Config_SetNetmask(const char *net) {
    IP4_ADDR(&activeConfig.StaticSubnet, net[0], net[1], net[2], net[3]);
}

void Config_ApplyNetwork() {
    dhcp_stop(netif);
    autoip_stop(netif);

    DhcpServer_Configure(&activeConfig.DhcpServerClient, &activeConfig.DhcpServerSubnet);

    if (activeConfig.DhcpServerEnable) {
        DhcpServer_Start(netif);
    } else {
        DhcpServer_Stop(netif);
    }

    switch (activeConfig.Mode) {
    case CONFIGIP_Auto:
        netif_set_addr(netif, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY);
        autoip_start(netif);
        break;
    case CONFIGIP_Static:
        netif_set_addr(netif, &activeConfig.StaticIp, &activeConfig.StaticSubnet, &activeConfig.StaticGateway);
        break;
    case CONFIGIP_DHCP:
        if (activeConfig.DhcpServerEnable) {
            netif_set_addr(netif, &activeConfig.DhcpServerSelf, &activeConfig.DhcpServerSubnet, &activeConfig.DhcpServerSelf);
        } else {
            netif_set_addr(netif, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY);
            dhcp_start(netif);
        }
        break;
    }

    Config_Store();
}

void Config_Store() {
    EE_WriteConfig(&activeConfig);
}

void Config_StoreFailsafeScene(char *buffer, char art_port) {
    EE_WriteFailover(buffer, art_port);
}
void Config_LoadFailsafeScene(char *target, int index) {
    EE_ReadFailover(target, index);
}

CONFIG Config_GetDefault() {
    CONFIG cfg = {.Mode = CONFIGIP_DHCP,
                  .DhcpServerEnable = 1};

    IP4_ADDR(&cfg.DhcpServerSelf, 10, 0, 0, 1);
    IP4_ADDR(&cfg.DhcpServerClient, 10, 0, 0, 2);
    IP4_ADDR(&cfg.DhcpServerSubnet, 255, 0, 0, 0);

    IP4_ADDR(&cfg.StaticIp, 192, 168, 10, 10);
    IP4_ADDR(&cfg.StaticGateway, 192, 168, 10, 1);
    IP4_ADDR(&cfg.StaticSubnet, 255, 255, 255, 0);

    for (int i = 0; i < 4; i++) {
        sprintf(cfg.ArtNet[i].ShortName, "ArtNet-Port %i", i);
        sprintf(cfg.ArtNet[i].LongName, "Custom Art-Net Node");

        cfg.ArtNet[i].Network = ARTNET_NET;
        cfg.ArtNet[i].Subnet = ARTNET_SUB;

        cfg.ArtNet[i].Universe = ARTNET_UNI + i;

        cfg.ArtNet[i].PortFlags = 0;

        cfg.ArtNet[i].FailoverMode = ArtFail_Hold;
        cfg.ArtNet[i].PortDirection = 0x00;

        cfg.ArtNet[i].AcnPriority = 100;
    }

    return cfg;
}