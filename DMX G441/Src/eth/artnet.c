#include "eth/artnet.h"
#include "config.h"
#include "dmx_usart.h"
#include "eth/global.h"
#include "lwip/autoip.h"
#include "platform.h"
#include "systimer.h"

static struct udp_pcb *artnet;
static struct netif *artif;
static short artnet_port = 6454;

static char artnet_portConfig[4];
static char *initial_portConfig;
static char artnet_inputs = 0x0F;
static unsigned int artnet_timeout[4];

static CONFIG *config;

static void ArtNet_SendPollReply(const ip_addr_t *addr, u16_t port);
static void ArtNet_HandleIpProg(ArtNet_IpProg *p, const ip_addr_t *addr, u16_t port);
static void ArtNet_Receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
static void ArtNet_HandleAddress(ArtNet_Address *data, const ip_addr_t *addr, u16_t port);
static void ArtNet_HandleOutput(ArtNet_Dmx *data);
static void ArtNet_HandleInput(ArtNet_Input *data, const ip_addr_t *addr, u16_t port);
static void ArtNet_ApplyFailover(int idx);

void ArtNet_Init(struct netif *netif, char *portConfig) {
    initial_portConfig = portConfig;
    artif = netif;
    artnet = udp_new();
    udp_bind(artnet, IP4_ADDR_ANY, artnet_port);
    udp_recv(artnet, ArtNet_Receive, NULL);
    ip_set_option(artnet, SOF_BROADCAST);

    config = Config_GetActive();

    // Initial directions
    memcpy(artnet_portConfig, portConfig, 4);

    for (int i = 0; i < 4; i++) {
        if (config->ArtNetPortDirection & (1 << (i * 2))) {
            if (config->ArtNetPortDirection & (1 << (i * 2 + 1))) {
                artnet_portConfig[i] = USART_INPUT;
            } else {
                artnet_portConfig[i] = USART_OUTPUT;
            }
        }
    }

    USART_InitPortDirections(artnet_portConfig);

    for (int i = 0; i < 4; i++) {
        if (config->ArtNetPortFlags[i] & PORT_FLAG_RDM) {
            USART_AlterPortFlags(i, PORT_FLAG_RDM, 1);
        }

        if (config->ArtNetPortFlags[i] & PORT_FLAG_SINGLE) {
            USART_AlterPortFlags(i, PORT_FLAG_SINGLE, 1);
        }
    }
}

static void ArtNet_Receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    if (pbufcpy_mem(net_buffer, p, sizeof(net_buffer)) == 0) {
        pbuf_free(p);
        return;
    }

    pbuf_free(p);

    ArtNet_Header *art = net_buffer;
    art->ProtocolVersion = UI16_LITTLE_ENDIAN(art->ProtocolVersion);

    if (strcmp(art->Signature, "Art-Net") == 0 && art->ProtocolVersion >= 14) {
        switch (art->OpCode) {
        case ArtCode_OpPoll: {
            ArtNet_Poll *poll = net_buffer;

            if (poll->Flags & 0x20) {
                // Targeted mode, check addresses
                short base = (config->ArtNetNetwork & 0x7F) << 8;
                base += (config->ArtNetSubnet & 0x0F) << 4;

                for (int i = 0; i < 4; i++) {
                    if (artnet_portConfig[i] & USART_OUTPUT) {
                        short address = base + config->ArtNetUniverse[i];

                        if (poll->TargetPort[0] <= address && poll->TargetPort[1] >= address) {
                            ArtNet_SendPollReply(addr, port);
                            break;
                        }
                    }

                    if (artnet_portConfig[i] & USART_INPUT) {
                        short address = base + config->ArtNetUniverse[i + 4];

                        if (poll->TargetPort[0] <= address && poll->TargetPort[1] >= address) {
                            ArtNet_SendPollReply(addr, port);
                            break;
                        }
                    }
                }
            } else {
                ArtNet_SendPollReply(addr, port);
            }
        } break;
        case ArtCode_OpIpProg:
            ArtNet_HandleIpProg(net_buffer, addr, port);
            break;
        case ArtCode_OpAddress:
            ArtNet_HandleAddress(net_buffer, addr, port);
            break;
        case ArtCode_OpDmx:
            ArtNet_HandleOutput(net_buffer);
            break;
        }
    }
}

static void ArtNet_SendPollReply(const ip_addr_t *addr, u16_t port) {
    memclr(net_buffer, sizeof(net_buffer));

    ArtNet_PollReply *reply = net_buffer;
    memcpy(reply->Header.Signature, "Art-Net", 8);
    reply->Header.OpCode = ArtCode_OpPollReply;

    reply->IpAddress[0] = ip4_addr1(&artif->ip_addr);
    reply->IpAddress[1] = ip4_addr2(&artif->ip_addr);
    reply->IpAddress[2] = ip4_addr3(&artif->ip_addr);
    reply->IpAddress[3] = ip4_addr4(&artif->ip_addr);

    for (int i = 0; i < 6; i++) {
        reply->Mac[i] = artif->hwaddr[i];
    }

    reply->Port = UI16_BIG_ENDIAN(0x1936);

    reply->VersInfo = UI16_LITTLE_ENDIAN(0);
    reply->OEM = 0xFFFF;

    reply->NetSwitch = config->ArtNetNetwork;
    reply->SubSwitch = config->ArtNetSubnet;

    reply->NumPorts = UI16_LITTLE_ENDIAN(4);
    for (int i = 0; i < 4; i++) {
        reply->PortTypes[i] = artnet_portConfig[i] << 6;
        reply->SwOut[i] = config->ArtNetUniverse[i];
        reply->SwIn[i] = config->ArtNetUniverse[i + 4];
    }

    for (int i = 0; i < 4; i++) {
        reply->GoodInput[i] = (((artnet_inputs ^ 0xFF) & (1 << i)) >> i) << 3;

        if ((config->ArtNetPortFlags[i] & PORT_FLAG_RDM) == 0) {
            reply->GoodOutputB[i] |= (1 << 7);
        }

        if ((config->ArtNetPortFlags[i] & PORT_FLAG_SINGLE) == 0) {
            reply->GoodOutputB[i] |= (1 << 6);
        }
    }

    reply->Status1 = 0x20;
    reply->Status2 = (1 << 2) | (1 << 3) | (1 << 6);
    if (config->Mode == CONFIGIP_DHCP) {
        reply->Status2 |= (1 << 1);
    }
    reply->Status3 = (1 << 3) | (1 << 5);
    reply->Status3 |= (config->ArtNetFailoverMode << 6);

    reply->AcnPriority = config->AcnPriority;

    memcpy(reply->ShortName, config->ArtNetShortName, 18);
    memcpy(reply->LongName, config->ArtNetLongName, 64);

    memcpy(reply->NodeReport, "ID:", 3);
    memcpy(reply->NodeReport + 3, UID->ID, sizeof(UID->ID));

    struct pbuf *p;
    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(ArtNet_PollReply), PBUF_POOL);
    memcpy_pbuf(p, reply, sizeof(ArtNet_PollReply));
    udp_sendto(artnet, p, addr, port);
    pbuf_free(p);
}

static void ArtNet_HandleIpProg(ArtNet_IpProg *data, const ip_addr_t *addr, u16_t port) {
    if (data->Command & 0x80) {
        if (data->Command & 0x40) {
            Config_SetMode(CONFIGIP_DHCP);
        } else if (data->Command & 0x08) {
            Config_SetMode(CONFIGIP_Auto);
        } else {
            Config_SetMode(CONFIGIP_Static);

            if (data->Command & 0x10) {
                Config_SetGateway(data->ProgGateway);
            }

            if (data->Command & 0x04) {
                Config_SetIp(data->ProgIp);
            }

            if (data->Command & 0x02) {
                Config_SetNetmask(data->ProgSubnet);
            }
        }
    }

    Config_ApplyNetwork();

    memclr(net_buffer, sizeof(net_buffer));
    ArtNet_IpProgReply *reply = net_buffer;
    memcpy(reply->Header.Signature, "Art-Net", 8);
    reply->Header.OpCode = ArtCode_OpIpProgReply;
    reply->Header.ProtocolVersion = UI16_LITTLE_ENDIAN(14);

    reply->ProgGateway[0] = ip4_addr1(&artif->gw);
    reply->ProgGateway[1] = ip4_addr2(&artif->gw);
    reply->ProgGateway[2] = ip4_addr3(&artif->gw);
    reply->ProgGateway[3] = ip4_addr4(&artif->gw);

    reply->ProgIp[0] = ip4_addr1(&artif->ip_addr);
    reply->ProgIp[1] = ip4_addr2(&artif->ip_addr);
    reply->ProgIp[2] = ip4_addr3(&artif->ip_addr);
    reply->ProgIp[3] = ip4_addr4(&artif->ip_addr);

    reply->ProgSubnet[0] = ip4_addr1(&artif->netmask);
    reply->ProgSubnet[1] = ip4_addr2(&artif->netmask);
    reply->ProgSubnet[2] = ip4_addr3(&artif->netmask);
    reply->ProgSubnet[3] = ip4_addr4(&artif->netmask);

    reply->ProgPort = UI16_LITTLE_ENDIAN(artnet_port);

    struct pbuf *p;
    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(reply), PBUF_POOL);
    memcpy_pbuf(p, reply, sizeof(reply));
    udp_sendto(artnet, p, addr, port);
    pbuf_free(p);
}

static void ArtNet_HandleAddress(ArtNet_Address *data, const ip_addr_t *addr, u16_t port) {
    if (data->NetSwitch == 0) {
        config->ArtNetNetwork = ARTNET_NET;
    } else if (data->NetSwitch & 0x80) {
        config->ArtNetNetwork = data->NetSwitch & ~0x80;
    }

    if (data->ShortName[0] != 0) {
        memcpy(config->ArtNetShortName, data->ShortName, 18);
    }

    if (data->LongName[0] != 0) {
        memcpy(config->ArtNetLongName, data->LongName, 64);
    }

    if (data->SubSwitch == 0) {
        config->ArtNetSubnet = ARTNET_SUB;
    } else if (data->SubSwitch & 0x80) {
        config->ArtNetSubnet = data->SubSwitch & ~0x80;
    }

    if (data->AcnPriority >= 0 && data->AcnPriority < 200) {
        config->AcnPriority = data->AcnPriority;
    }

    for (int i = 0; i < 4; i++) {
        if (data->SwOut[i] & 0x80) {
            config->ArtNetUniverse[i] = data->SwOut[i] & ~0x80;
        } else if (data->SwOut[i] == 0) {
            config->ArtNetUniverse[i] = ARTNET_UNI + i;
        }

        if (data->SwIn[i] & 0x80) {
            config->ArtNetUniverse[i + 4] = data->SwIn[i] & ~0x80;
        } else if (data->SwIn[i] == 0) {
            config->ArtNetUniverse[i + 4] = ARTNET_UNI + i;
        }
    }

    char idx = data->Command & 0x0F;
    char cmd = data->Command & 0xF0;
    if (cmd > 0 && idx >= 0 && idx < 4) {
        switch (cmd) {
        case 0x90:
            USART_ClearBuffer(idx);
            break;

        case 0xA0:
            USART_AlterPortFlags(idx, PORT_FLAG_SINGLE, 1);
            config->ArtNetPortFlags[idx] |= PORT_FLAG_SINGLE;
            break;
        case 0xB0:
            USART_AlterPortFlags(idx, PORT_FLAG_SINGLE, 0);
            config->ArtNetPortFlags[idx] &= ~PORT_FLAG_SINGLE;
            break;
        case 0xC0:
            USART_AlterPortFlags(idx, PORT_FLAG_RDM, 1);
            config->ArtNetPortFlags[idx] |= PORT_FLAG_RDM;
            break;
        case 0xD0:
            USART_AlterPortFlags(idx, PORT_FLAG_RDM, 0);
            config->ArtNetPortFlags[idx] &= ~PORT_FLAG_RDM;
            break;

        case 0x20: // Output
            config->ArtNetPortDirection = (config->ArtNetPortDirection & ~(0x03 << (idx * 2))) | (0x01 << (idx * 2));
            USART_ChangePortDirection(idx, USART_OUTPUT);
            artnet_portConfig[idx] = USART_OUTPUT;
            break;
        case 0x30: // Input
            config->ArtNetPortDirection = (config->ArtNetPortDirection & ~(0x03 << (idx * 2))) | (0x03 << (idx * 2));
            USART_ChangePortDirection(idx, USART_INPUT);
            artnet_portConfig[idx] = USART_INPUT;
            break;
        }
    } else {
        switch (data->Command) {
            // Failover mode
        case 0x08:
            config->ArtNetFailoverMode = ArtFail_Hold;
            break;
        case 0x09:
            config->ArtNetFailoverMode = ArtFail_Zero;
            break;
        case 0x0A:
            config->ArtNetFailoverMode = ArtFail_Full;
            break;
        case 0x0B:
            config->ArtNetFailoverMode = ArtFail_Scene;
            break;
        case 0x0C: {
            char *buffers[4] = {
                USART_GetDmxBuffer(0),
                USART_GetDmxBuffer(1),
                USART_GetDmxBuffer(2),
                USART_GetDmxBuffer(3)};

            Config_StoreFailsafeScene(buffers);
        } break;
        }
    }

    Config_Store();
    ArtNet_SendPollReply(addr, port);
}

static void ArtNet_HandleInput(ArtNet_Input *data, const ip_addr_t *addr, u16_t port) {
    artnet_inputs = 0;
    for (int i = 0; i < 4; i++) {
        artnet_inputs |= ((data->Input[i] & 0x01) ^ 0x01) << i;
    }

    ArtNet_SendPollReply(addr, port);
}

static void ArtNet_HandleOutput(ArtNet_Dmx *data) {
    data->Length = UI16_LITTLE_ENDIAN(data->Length);
    char net = data->Net & 0x7F;
    char sub = (data->SubUni & 0xF0) >> 4;
    char uni = (data->SubUni & 0x0F);

    if (data->Length <= 512) {
        if (config->ArtNetSubnet == sub && config->ArtNetNetwork == net) {
            if (uni == config->ArtNetUniverse[0]) {
                USART_SetBuffer(0, data->Data, data->Length);
                artnet_timeout[0] = sys_now();
            }

            if (uni == config->ArtNetUniverse[1]) {
                USART_SetBuffer(1, data->Data, data->Length);
                artnet_timeout[1] = sys_now();
            }

            if (uni == config->ArtNetUniverse[2]) {
                USART_SetBuffer(2, data->Data, data->Length);
                artnet_timeout[2] = sys_now();
            }

            if (uni == config->ArtNetUniverse[3]) {
                USART_SetBuffer(3, data->Data, data->Length);
                artnet_timeout[3] = sys_now();
            }
        }
    }
}

static void ArtNet_ApplyFailover(int idx) {
    if (idx >= 0 && idx < 4) {
        switch (config->ArtNetFailoverMode) {
        case ArtFail_Hold:
            break;
        case ArtFail_Zero:
            memclr(USART_GetDmxBuffer(idx), 512);
            break;
        case ArtFail_Full: {
            char *buffer = USART_GetDmxBuffer(idx);
            for (int i = 0; i < 512; i++) {
                buffer[i] = 0xFF;
            }
        } break;
        case ArtFail_Scene:
            Config_LoadFailsafeScene(USART_GetDmxBuffer(idx), idx);
            break;
        }
    }
}

void ArtNet_InputTick() {
    for (int i = 0; i < 4; i++) {
        if (artnet_portConfig[i] & USART_INPUT && artnet_inputs & (1 << i)) {
            // Send input
            memclr(net_buffer, sizeof(net_buffer));
            ArtNet_Dmx *reply = net_buffer;
            memcpy(reply->Header.Signature, "Art-Net", 8);
            reply->Header.OpCode = ArtCode_OpDmx;
            reply->Header.ProtocolVersion = UI16_LITTLE_ENDIAN(14);

            reply->Net = config->ArtNetNetwork & 0x7F;
            reply->SubUni = ((config->ArtNetSubnet & 0x0F) << 4) | (config->ArtNetUniverse[i] & 0x0F);
            reply->Length = 512;

            char *buffer = USART_GetDmxBuffer(i);
            memcpy(reply->Data, buffer, 512);

            struct pbuf *p;
            p = pbuf_alloc(PBUF_TRANSPORT, sizeof(ArtNet_Dmx), PBUF_POOL);
            memcpy_pbuf(p, reply, sizeof(ArtNet_Dmx));
            udp_sendto(artnet, p, IP4_ADDR_BROADCAST, artnet_port);
            pbuf_free(p);
        }
    }
}

void ArtNet_TimeoutTick() {
    for (int i = 0; i < 4; i++) {
        if (sys_now() - artnet_timeout[i] > ARTNET_FAILTIMEOUT) {
            ArtNet_ApplyFailover(i);
        }
    }
}