#include "eth/http_custom.h"
#include "config.h"
#include "dmx_usart.h"
#include <stdarg.h>

/* config values are just a bunch of unencoded ascii lines with the name of the input-element
output:%u
universe:%u
name:%s
description:%s
acn:%u
inputdisable:%u
rdm:%u
delta:%u
failover:%u

ipmode:%u
s_ip:%s
s_mask:%s
s_gw:%s
dhcp_en:%u
d_dev:%s
d_host:%s
d_mask:%s
a_ip:%s
a_mask:%s
a_gw:%s
*/

static const char *artnet_template = "output:%u\nuniverse:%u\nname:%s\ndescription:%s\nacn:%u\ninputdisable:%u\nrdm:%u\ndelta:%u\nfailover:%u";
static const char *ipconfig_template = "ipmode:%u\ns_ip:%s\ns_mask:%s\ns_gw:%s\ndhcp_en:%u\nd_dev:%s\nd_host:%s\nd_mask:%s\na_ip:%s\na_mask:%s\na_gw:%s";
static char file_buffer[300];
static struct netif *netif;
static unsigned int reset_timeout = 0;
static HttpPostData postMetadata;

static void httpc_parsePortConfig(struct pbuf *p);
static void httpc_recordFailover(struct pbuf *p);
static void httpc_setIpConfig(struct pbuf *p);

void httpc_init(struct netif *ncm_netif) {
    netif = ncm_netif;
}

void httpc_timeout() {
    if (reset_timeout != 0) {
        if ((sys_now() - reset_timeout) > 500) {
            NVIC_SystemReset();
        }
    }
}

int fs_open_custom(struct fs_file *file, const char *name) {
    const CONFIG *cfg = Config_GetActive();

    if (strncmp(name, "/config", 7) == 0) {
        unsigned char id = 0;

        switch (name[7]) {
        case '1':
            id = 1;
            break;
        case '2':
            id = 2;
            break;
        case '3':
            id = 3;
            break;
        default:
            id = 0;
            break;
        }

        file->len = snprintf(file_buffer, sizeof(file_buffer), artnet_template,
                             (cfg->ArtNet[id].PortDirection == USART_INPUT) ? 1 : 0,
                             ((cfg->ArtNet[id].Network & 0x7F) << 8) | ((cfg->ArtNet[id].Subnet & 0x0F) << 4) | cfg->ArtNet[id].Universe,
                             cfg->ArtNet[id].ShortName,
                             cfg->ArtNet[id].LongName,
                             cfg->ArtNet[id].AcnPriority,
                             (cfg->ArtNet[id].PortFlags & PORT_FLAG_INDISABLED) ? 1 : 0,
                             (cfg->ArtNet[id].PortFlags & PORT_FLAG_RDM) ? 1 : 0,
                             (cfg->ArtNet[id].PortFlags & PORT_FLAG_SINGLE) ? 1 : 0,
                             cfg->ArtNet[id].FailoverMode);
        file->index = file->len;
        file->data = file_buffer;

        return 1;
    } else if (strcmp(name, "/ipconfig") == 0) {
        char ip_buffer[9][IP4ADDR_STRLEN_MAX];
        ip4addr_ntoa_r(&cfg->StaticIp, ip_buffer[0], IP4ADDR_STRLEN_MAX);
        ip4addr_ntoa_r(&cfg->StaticSubnet, ip_buffer[1], IP4ADDR_STRLEN_MAX);
        ip4addr_ntoa_r(&cfg->StaticGateway, ip_buffer[2], IP4ADDR_STRLEN_MAX);
        ip4addr_ntoa_r(&cfg->DhcpServerSelf, ip_buffer[3], IP4ADDR_STRLEN_MAX);
        ip4addr_ntoa_r(&cfg->DhcpServerClient, ip_buffer[4], IP4ADDR_STRLEN_MAX);
        ip4addr_ntoa_r(&cfg->DhcpServerSubnet, ip_buffer[5], IP4ADDR_STRLEN_MAX);
        ip4addr_ntoa_r(&netif->ip_addr, ip_buffer[6], IP4ADDR_STRLEN_MAX);
        ip4addr_ntoa_r(&netif->netmask, ip_buffer[7], IP4ADDR_STRLEN_MAX);
        ip4addr_ntoa_r(&netif->gw, ip_buffer[8], IP4ADDR_STRLEN_MAX);

        file->len = snprintf(file_buffer, sizeof(file_buffer), ipconfig_template,
                             cfg->Mode,
                             ip_buffer[0],
                             ip_buffer[1],
                             ip_buffer[2],
                             cfg->DhcpServerEnable,
                             ip_buffer[3],
                             ip_buffer[4],
                             ip_buffer[5],
                             ip_buffer[6],
                             ip_buffer[7],
                             ip_buffer[8]);

        file->index = file->len;
        file->data = file_buffer;
        return 1;
    }

    return 0;
}

void fs_close_custom(struct fs_file *file) {
}

int fs_read_custom(struct fs_file *file, char *buffer, int count) {
    if (file->index < file->len) {
        if ((file->len - file->index) > count) {
            count = file->len - file->index;
        }

        memcpy(buffer, file_buffer + file->index, count);
        file->index += count;
        return count;
    } else {
        return FS_READ_EOF;
    }
}

err_t httpd_post_begin(void *connection, const char *uri, const char *http_request,
                       u16_t http_request_len, int content_len, char *response_uri,
                       u16_t response_uri_len, u8_t *post_auto_wnd) {
    postMetadata.connection = connection;
    postMetadata.redirectTo = NULL;
    postMetadata.postHander = NULL;

    if (strcmp(uri, "/reset-config") == 0) {
        Config_Reset();
        Config_ApplyNetwork();
        postMetadata.redirectTo = "/device.html";
        snprintf(response_uri, response_uri_len, postMetadata.redirectTo);
        return ERR_OK;
    } else if (strcmp(uri, "/reset-device") == 0) {
        postMetadata.redirectTo = "/device.html";
        snprintf(response_uri, response_uri_len, postMetadata.redirectTo);
        reset_timeout = sys_now();
        if (reset_timeout == 0)
            reset_timeout--;

        return ERR_OK;
    } else if (strcmp(uri, "/reset-dfu") == 0) {
        PWR->CR1 |= PWR_CR1_DBP;
        TAMP->BKP0R = 0xF0;
        PWR->CR1 &= ~PWR_CR1_DBP;
        postMetadata.redirectTo = "/dfu.html";
        snprintf(response_uri, response_uri_len, postMetadata.redirectTo);

        reset_timeout = sys_now();
        if (reset_timeout == 0)
            reset_timeout--;

        return ERR_OK;
    } else if (strcmp(uri, "/set-config") == 0) {
        postMetadata.redirectTo = "/";
        snprintf(response_uri, response_uri_len, postMetadata.redirectTo);

        postMetadata.postHander = httpc_parsePortConfig;
        *post_auto_wnd = 1;

        return ERR_OK;
    } else if (strcmp(uri, "/record-failover") == 0) {
        postMetadata.redirectTo = "/";
        snprintf(response_uri, response_uri_len, postMetadata.redirectTo);

        postMetadata.postHander = httpc_recordFailover;
        *post_auto_wnd = 1;

        return ERR_OK;
    } else if (strcmp(uri, "/ip-config") == 0) {
        postMetadata.redirectTo = "/device";
        snprintf(response_uri, response_uri_len, postMetadata.redirectTo);

        postMetadata.postHander = httpc_setIpConfig;
        *post_auto_wnd = 1;

        return ERR_OK;
    }

    return ERR_VAL;
}

err_t httpd_post_receive_data(void *connection, struct pbuf *p) {
    if (postMetadata.connection == connection) {
        if (postMetadata.postHander != NULL) {
            postMetadata.postHander(p);
        }
    }

    pbuf_free(p);
    return ERR_OK;
}

void httpd_post_finished(void *connection, char *response_uri, u16_t response_uri_len) {
    if (postMetadata.connection == connection) {
        if (postMetadata.redirectTo != NULL) {
            snprintf(response_uri, response_uri_len, postMetadata.redirectTo);
        }
    }
}

// Helpers
static void httpc_parseIp(char valBuf[65], unsigned char ip[4]) {
    char *ptr = valBuf;

    for (int i = 0; i < 4; i++) {
        ip[i] = strtol(ptr, &ptr, 10);
        ptr++;
    }
}

static void httpc_unescape(char valBuf[65]) {
    char orig[65];
    unsigned char target = 0;
    memcpy(orig, valBuf, 65);

    for (int i = 0; i < 65; i++) {
        if (orig[i] == 0) {
            valBuf[target] = 0;
            return;
        } else if (orig[i] == '+') {
            valBuf[target] = ' ';
            target++;
        } else if (orig[i] == '%') {
            char *ptr;
            long tmp = strtol(&orig[i + 1], &ptr, 16);
            i = ptr - orig - 1;
            valBuf[target] = (char)tmp;
            target++;
        } else {
            valBuf[target] = orig[i];
            target++;
        }
    }
}

static unsigned short httpc_findKey(struct pbuf *p, const char *key, int portId) {
    char buf[16];
    int bufLen = 0;

    if (portId >= 0 && portId < 4) {
        bufLen = snprintf(buf, sizeof(buf), "%s%i=", key, portId);
    } else {
        bufLen = snprintf(buf, sizeof(buf), "%s=", key);
    }

    unsigned short find = pbuf_memfind(p, buf, bufLen, 0);
    if (find != 0xFFFF) {
        find += bufLen;
    }
    return find;
}

static char httpc_findValue(struct pbuf *p, char valBuf[65], const char *key, int portId) {
    unsigned short keyPos = 0;
    unsigned short end = 0;
    char *buf;

    keyPos = httpc_findKey(p, key, portId);
    memclr(valBuf, 65);

    if (keyPos != 0xFFFF) {
        end = pbuf_memfind(p, "&", 1, keyPos);
        if (end == 0xFFFF) {
            end = p->tot_len;
        }

        if (end - keyPos > 64) {
            end = keyPos + 64;
        }

        buf = pbuf_get_contiguous(p, valBuf, 64, end - keyPos, keyPos);

        if (buf != valBuf) {
            memcpy(valBuf, buf, end - keyPos);
        }

        valBuf[end - keyPos] = 0;
        httpc_unescape(valBuf);

        return 1;
    } else {
        return 0;
    }
}

// Post-parsers
static void httpc_parsePortConfig(struct pbuf *p) {
    CONFIG *cfg = Config_GetActive();
    char valBuf[65];

    for (int i = 0; i < 4; i++) {
        // TODO: create artnet internal handler
        if (httpc_findKey(p, "output", i) == 0xFFFF) {
            cfg->ArtNet[i].PortDirection = USART_OUTPUT;
        } else {
            cfg->ArtNet[i].PortDirection = USART_INPUT;
        }

        if (httpc_findValue(p, valBuf, "name", i)) {
            strncpy(cfg->ArtNet[i].ShortName, valBuf, sizeof(cfg->ArtNet[i].ShortName));
        }

        if (httpc_findValue(p, valBuf, "description", i)) {
            strncpy(cfg->ArtNet[i].LongName, valBuf, sizeof(cfg->ArtNet[i].LongName));
        }

        if (httpc_findValue(p, valBuf, "acn", i)) {
            unsigned char acn = atoi(valBuf);
            if (acn >= 0 && acn < 0xFF) {
                cfg->ArtNet[i].AcnPriority = acn;
            }
        }

        if (httpc_findKey(p, "inputdisable", i) != 0xFFFF) {
            cfg->ArtNet[i].PortFlags |= PORT_FLAG_INDISABLED;
        } else {
            cfg->ArtNet[i].PortFlags &= ~PORT_FLAG_INDISABLED;
        }

        if (httpc_findKey(p, "rdm", i) != 0xFFFF) {
            cfg->ArtNet[i].PortFlags |= PORT_FLAG_RDM;
        } else {
            cfg->ArtNet[i].PortFlags &= ~PORT_FLAG_RDM;
        }

        if (httpc_findKey(p, "delta", i) != 0xFFFF) {
            cfg->ArtNet[i].PortFlags |= PORT_FLAG_SINGLE;
        } else {
            cfg->ArtNet[i].PortFlags &= ~PORT_FLAG_SINGLE;
        }

        if (httpc_findValue(p, valBuf, "failover", i)) {
            char failover = atoi(valBuf);

            if (failover >= 0 && failover < 4) {
                cfg->ArtNet[i].FailoverMode = failover;
            }
        }

        if (httpc_findValue(p, valBuf, "universe", i)) {
            unsigned short universe = atoi(valBuf);

            cfg->ArtNet[i].Universe = universe & 0x0F;
            cfg->ArtNet[i].Subnet = (universe >> 4) & 0x0F;
            cfg->ArtNet[i].Network = (universe >> 8) & 0x7F;
        }
    }

    Config_ApplyArtNet();
    Config_Store();
}

static void httpc_recordFailover(struct pbuf *p) {
    for (unsigned char i = 0; i < 4; i++) {
        if (httpc_findKey(p, "recFailover", i) != 0xFFFF) {
            unsigned char *buffer = USART_GetDmxBuffer(i);
            Config_StoreFailsafeScene(buffer, i);
        }
    }
}

static void httpc_setIpConfig(struct pbuf *p) {
    CONFIG *cfg = Config_GetActive();
    char valBuf[65];
    unsigned char ipAddr[4];

    if (httpc_findValue(p, valBuf, "ipmode", -1)) {
        char mode = atoi(valBuf);

        if (mode >= 0 && mode < 3) {
            Config_SetMode(mode);
        }
    }

    if (httpc_findValue(p, valBuf, "s_ip", -1)) {
        httpc_parseIp(valBuf, ipAddr);
        Config_SetIp(ipAddr);
    }

    if (httpc_findValue(p, valBuf, "s_mask", -1)) {
        httpc_parseIp(valBuf, ipAddr);
        Config_SetNetmask(ipAddr);
    }

    if (httpc_findValue(p, valBuf, "s_gw", -1)) {
        httpc_parseIp(valBuf, ipAddr);
        Config_SetGateway(ipAddr);
    }

    {
        char dhcp_en = 0;
        struct ip4_addr device;
        struct ip4_addr host;
        struct ip4_addr netmask;

        if (httpc_findKey(p, "dhcp_en", -1) != 0xFFFF) {
            dhcp_en = 1;
        }

        if (httpc_findValue(p, valBuf, "d_dev", -1)) {
            ip4addr_aton(valBuf, &device);
        } else {
            device = cfg->DhcpServerSelf;
        }

        if (httpc_findValue(p, valBuf, "d_host", -1)) {
            ip4addr_aton(valBuf, &host);
        } else {
            host = cfg->DhcpServerClient;
        }

        if (httpc_findValue(p, valBuf, "d_mask", -1)) {
            ip4addr_aton(valBuf, &netmask);
        } else {
            host = cfg->DhcpServerSubnet;
        }

        Config_DhcpServer(dhcp_en, device, host, netmask);
    }

    Config_ApplyNetwork();
    Config_Store();
}
