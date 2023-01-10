#include "config.h"
#include "lwip/autoip.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"

static struct netif *netif;

void Config_Init(struct netif *net) {
    netif = net;

    autoip_start(netif);
}

void Config_Reset() {
    Config_ResetIp();
}

void Config_ResetIp() {
    dhcp_stop(netif);
    netif_set_addr(netif, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY);
    autoip_start(netif);
}

void Config_EnableDhcp() {
    autoip_stop(netif);
    netif_set_addr(netif, IP4_ADDR_ANY, IP4_ADDR_ANY, IP4_ADDR_ANY);
    dhcp_start(netif);
}

void Config_SetIp(const char *ip) {
    autoip_stop(netif);
    dhcp_stop(netif);
    ip_addr_t addr;
    IP4_ADDR(&addr, ip[0], ip[1], ip[2], ip[3]);
    netif_set_ipaddr(netif, &addr);
}

void Config_SetGateway(const char *gw) {
    autoip_stop(netif);
    dhcp_stop(netif);
    ip_addr_t addr;
    IP4_ADDR(&addr, gw[0], gw[1], gw[2], gw[3]);
    netif_set_ipaddr(netif, &addr);
}

void Config_SetNetmask(const char *net) {
    autoip_stop(netif);
    dhcp_stop(netif);
    ip_addr_t addr;
    IP4_ADDR(&addr, net[0], net[1], net[2], net[3]);
    netif_set_ipaddr(netif, &addr);
}