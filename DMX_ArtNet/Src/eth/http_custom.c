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
f_v:%s
f_id:%u
*/

static const char *artnet_template = "output:%u\nuniverse:%u\nname:%s\ndescription:%s\nacn:%u\ninputdisable:%u\nrdm:%u\ndelta:%u\nfailover:%u";
static const char *ipconfig_template = "ipmode:%u\ns_ip:%s\ns_mask:%s\ns_gw:%s\ndhcp_en:%u\nd_dev:%s\nd_host:%s\nd_mask:%s\na_ip:%s\na_mask:%s\na_gw:%s\nf_v:%s\nf_id:%u";
static char file_buffer[300];
static struct netif *netif;
static unsigned int reset_timeout = 0;
static unsigned int apply_timeout = 0;
static HttpPostData postMetadata;

// 16 key, 64 value, 1x '=', 1x '\0'
static char post_buffer[16 + 64 + 2];

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

    if (apply_timeout != 0) {
        if ((sys_now() - apply_timeout) > 500) {
            Config_ApplyNetwork();
            apply_timeout = 0;
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
                             (cfg->ArtNet[id].PortDirection == ARTNET_INPUT) ? 1 : 0,
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
                             ip_buffer[8],
                             FIRMWARE_VER,
                             FIRMWARE_INT);

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
        postMetadata.redirectTo = "/device.html";
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
        if (postMetadata.postHander != NULL) {
            postMetadata.postHander(NULL);
        }

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

static unsigned short httpc_parseNextValue(struct pbuf *p, unsigned short *offset) {
    // find positon of next &
    // by definition, it cannot be in the post_buffer
    unsigned short endpos = 0;
    unsigned short buffer_len = 0;

    while (buffer_len < sizeof(post_buffer)) {
        if (post_buffer[buffer_len] == 0) {
            break;
        }

        buffer_len++;
    }

    for (endpos = *offset; endpos < p->tot_len; endpos++) {
        if (pbuf_get_at(p, endpos) == '&') {
            break;
        }
    }

    // check if found key-value pair is too large to be valid
    if (buffer_len + (endpos - *offset) > sizeof(post_buffer)) {
        *offset = endpos + 1;
        return 0;
    }

    if (pbuf_get_at(p, endpos) != '&') {
        if (post_buffer[0] != 0) {
            // our value is waaay to long, just discard it
            post_buffer[0] = 0;
        } else {
            if (p->tot_len - *offset < sizeof(post_buffer)) {
                pbuf_copy_partial(p, post_buffer, p->tot_len - *offset, *offset);
                post_buffer[p->tot_len - *offset] = 0;
            } else {
                // value does not fit temporary buffer, skip
            }
        }

        return 0;
    } else {
        pbuf_copy_partial(p, &post_buffer[buffer_len], endpos - *offset, *offset);

        post_buffer[buffer_len + (endpos - *offset)] = 0;
        *offset = endpos + 1;
        return 1;
    }
}

static signed char httpc_getValueKey(const char **keys, const unsigned char keyCount) {
    unsigned char start = 0;
    unsigned char end = keyCount - 1;
    unsigned char i = 0;

    // do a binary search over the keys
    for (i = 0; i < sizeof(post_buffer); i++) {
        if (post_buffer[i] == '\0' || post_buffer[i] == '=') {
            break;
        }

        if (keys[start][i] > post_buffer[i]) {
            return -1;
        }

        while (keys[start][i] < post_buffer[i] && start < end) {
            start++;
        }

        while (keys[end][i] > post_buffer[i] && end > start) {
            end--;
        }

        if (keys[start][i] != post_buffer[i]) {
            return -1;
        }
    }

    if (start == end) {
        if (post_buffer[i] == '=') {
            strncpy(post_buffer, &post_buffer[i + 1], sizeof(post_buffer) - i - 1);
            httpc_unescape(post_buffer);
        } else {
            post_buffer[0] = '1';
            post_buffer[1] = '\0';
        }
        return start;
    } else {
        return -1;
    }
}

// Post-parsers
static void httpc_parsePortConfig(struct pbuf *p) {
    CONFIG *cfg = Config_GetActive();
    unsigned short offset = 0;

    static const char *keys[] = {
        "acn0",
        "acn1",
        "acn2",
        "acn3",
        "delta0",
        "delta1",
        "delta2",
        "delta3",
        "description0",
        "description1",
        "description2",
        "description3",
        "failover0",
        "failover1",
        "failover2",
        "failover3",
        "inputdisable0",
        "inputdisable1",
        "inputdisable2",
        "inputdisable3",
        "name0",
        "name1",
        "name2",
        "name3",
        "output0",
        "output1",
        "output2",
        "output3",
        "rdm0",
        "rdm1",
        "rdm2",
        "rdm3",
        "universe0",
        "universe1",
        "universe2",
        "universe3",
    };
    static const unsigned char keyCount = 36;
    static short flags = 0; // Keep track which boolean attributes have already been set during this request, as false values are not transmitted at all

    while (1) {
        if (p != NULL) {
            if (httpc_parseNextValue(p, &offset) == 0) {
                break;
            }
        }

        signed char keyIdx = httpc_getValueKey(keys, keyCount);

        if (keyIdx >= 0) {
            unsigned char i = keyIdx % 4;

            switch (keyIdx / 4) {
            case 0: // acn
            {
                unsigned char acn = atoi(post_buffer);
                if (acn >= 0 && acn < 0xFF) {
                    cfg->ArtNet[i].AcnPriority = acn;
                }
                break;
            }
            case 1: // delta
                flags |= (1 << (i * 4));
                cfg->ArtNet[i].PortFlags |= PORT_FLAG_SINGLE;
                break;
            case 2: // description
                strncpy(cfg->ArtNet[i].LongName, post_buffer, sizeof(cfg->ArtNet[i].LongName));
                break;
            case 3: // failover
            {
                char failover = atoi(post_buffer);

                if (failover >= 0 && failover < 4) {
                    cfg->ArtNet[i].FailoverMode = failover;
                }
                break;
            }
            case 4: // input disable
                flags |= (2 << (i * 4));
                cfg->ArtNet[i].PortFlags |= PORT_FLAG_INDISABLED;
                break;
            case 5: // name
                strncpy(cfg->ArtNet[i].ShortName, post_buffer, sizeof(cfg->ArtNet[i].ShortName));
                break;
            case 6: // output
                flags |= (4 << (i * 4));
                cfg->ArtNet[i].PortDirection = ARTNET_INPUT;
                break;
            case 7: // rdm
                flags |= (8 << (i * 4));
                cfg->ArtNet[i].PortFlags |= PORT_FLAG_RDM;
                break;
            case 8: // universe
            {
                unsigned short universe = atoi(post_buffer);

                cfg->ArtNet[i].Universe = universe & 0x0F;
                cfg->ArtNet[i].Subnet = (universe >> 4) & 0x0F;
                cfg->ArtNet[i].Network = (universe >> 8) & 0x7F;
                break;
            }
            }
        }

        post_buffer[0] = 0;

        if (p == NULL) {
            // check flags for unset boolean values and reset them
            for (int i = 0; i < 4; i++) {
                if ((flags & (1 << (i * 4))) == 0) {
                    cfg->ArtNet[i].PortFlags &= ~PORT_FLAG_SINGLE;
                }

                if ((flags & (2 << (i * 4))) == 0) {
                    cfg->ArtNet[i].PortFlags &= ~PORT_FLAG_INDISABLED;
                }

                if ((flags & (4 << (i * 4))) == 0) {
                    cfg->ArtNet[i].PortDirection = ARTNET_OUTPUT;
                }

                if ((flags & (8 << (i * 4))) == 0) {
                    cfg->ArtNet[i].PortFlags &= ~PORT_FLAG_RDM;
                }
            }

            flags = 0;

            Config_ApplyArtNet();
            Config_Store();

            break;
        }
    }
}

static void httpc_recordFailover(struct pbuf *p) {
    unsigned short offset = 0;

    static const char *keys[] = {
        "recFailover0",
        "recFailover1",
        "recFailover2",
        "recFailover3"};

    static const unsigned char keyCount = 4;

    while (1) {
        if (p != NULL) {
            if (httpc_parseNextValue(p, &offset) == 0) {
                break;
            }
        }

        signed char keyIdx = httpc_getValueKey(keys, keyCount);

        if (keyIdx >= 0) {
            unsigned char *buffer = DMX_GetBuffer(keyIdx);
            Config_StoreFailsafeScene(buffer, keyIdx);
        }

        post_buffer[0] = 0;

        if (p == NULL) {
            break;
        }
    }
}

static void httpc_setIpConfig(struct pbuf *p) {
    CONFIG *cfg = Config_GetActive();
    unsigned short offset = 0;

    static const char *keys[] = {
        "d_dev",
        "d_host",
        "d_mask",
        "dhcp_en",
        "ipmode",
        "s_gw",
        "s_ip",
        "s_mask",
    };

    static const unsigned char keyCount = 8;
    static unsigned char flags = 0;
    unsigned char ipAddr[4];
    struct ip4_addr s_ipAddr;

    while (1) {
        if (p != NULL) {
            if (httpc_parseNextValue(p, &offset) == 0) {
                break;
            }
        }

        signed char keyIdx = httpc_getValueKey(keys, keyCount);

        if (keyIdx >= 0) {
            switch (keyIdx) {
            case 0: // d_dev
                ipaddr_aton(post_buffer, &s_ipAddr);
                Config_DhcpServer(cfg->DhcpServerEnable, s_ipAddr, cfg->DhcpServerClient, cfg->DhcpServerSubnet);
                break;
            case 1: // d_host
                ipaddr_aton(post_buffer, &s_ipAddr);
                Config_DhcpServer(cfg->DhcpServerEnable, cfg->DhcpServerSelf, s_ipAddr, cfg->DhcpServerSubnet);
                break;
            case 2: // d_mask
                ipaddr_aton(post_buffer, &s_ipAddr);
                Config_DhcpServer(cfg->DhcpServerEnable, cfg->DhcpServerSelf, cfg->DhcpServerClient, s_ipAddr);
                break;
            case 3: // dhcp_en
                flags |= 1;
                Config_DhcpServer(post_buffer[0], cfg->DhcpServerSelf, cfg->DhcpServerClient, cfg->DhcpServerSubnet);
                break;
            case 4: // ipmode
            {
                char mode = atoi(post_buffer);

                if (mode >= 0 && mode < 3) {
                    Config_SetMode(mode);
                }
                break;
            }
            case 5: // s_gw
                httpc_parseIp(post_buffer, ipAddr);
                Config_SetGateway(ipAddr);
                break;
            case 6: // s_ip
                httpc_parseIp(post_buffer, ipAddr);
                Config_SetIp(ipAddr);
                break;
            case 7: // s_mask
                httpc_parseIp(post_buffer, ipAddr);
                Config_SetNetmask(ipAddr);
                break;
            }
        }

        post_buffer[0] = 0;

        if (p == NULL) {
            if((flags & 1) == 0) {
                Config_DhcpServer(0, cfg->DhcpServerSelf, cfg->DhcpServerClient, cfg->DhcpServerSubnet);
            }

            flags = 0;

            // Defer Config_ApplyNetwork until after the post request is completely handled, otherwise hardfault
            apply_timeout = sys_now();
            if (apply_timeout == 0)
                apply_timeout--;

            Config_Store();
            break;
        }
    }
}
