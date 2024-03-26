#include "eth/artnet.h"
#include "config.h"
#include "dmx.h"
#include "eth/global.h"
#include "lwip/autoip.h"
#include "platform.h"
#include "profiling.h"
#include "systimer.h"

static struct udp_pcb *artnet;
static struct netif *artif;
static short artnet_port = 6454;

static unsigned int artnet_timeout[4];
static unsigned char artnet_failover_state[4];

static CONFIG *config;

static void ArtNet_SendPollReply(const ip_addr_t *addr, u16_t port, unsigned char art_port);
static void ArtNet_HandleIpProg(ArtNet_IpProg *p, const ip_addr_t *addr, u16_t port);
static void ArtNet_Receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
static void ArtNet_HandleAddress(ArtNet_Address *data, const ip_addr_t *addr, u16_t port);
static void ArtNet_HandleOutput(ArtNet_Dmx *data);
static void ArtNet_HandleInput(ArtNet_Input *data, const ip_addr_t *addr, u16_t port);
static void ArtNet_ApplyFailover(int idx);

void ArtNet_Init(struct netif *netif) {
    artif = netif;
    artnet = udp_new();
    udp_bind(artnet, IP4_ADDR_ANY, artnet_port);
    udp_recv(artnet, ArtNet_Receive, NULL);
    ip_set_option(artnet, SOF_BROADCAST);

    config = Config_GetActive();
}

static void ArtNet_Receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    if (pbufcpy_mem(net_buffer, p, sizeof(net_buffer)) == 0) {
        pbuf_free(p);
        return;
    }

    pbuf_free(p);

    ArtNet_Header *art = (ArtNet_Header *)net_buffer;
    art->ProtocolVersion = UI16_LITTLE_ENDIAN(art->ProtocolVersion);

    if (strcmp(art->Signature, "Art-Net") == 0 && art->ProtocolVersion >= 14) {
        switch (art->OpCode) {
        case ArtCode_OpPoll: {
            ArtNet_Poll *poll = (ArtNet_Poll *)net_buffer;

            if (poll->Flags & 0x20) {
                // Targeted mode, check addresses
                for (int i = 0; i < 4; i++) {
                    short base = (config->ArtNet[i].Network & 0x7F) << 8;
                    base += (config->ArtNet[i].Subnet & 0x0F) << 4;

                    for (int i = 0; i < 4; i++) {
                        short address = base + config->ArtNet[i].Universe;

                        if (poll->TargetPort[0] <= address && poll->TargetPort[1] >= address) {
                            ArtNet_SendPollReply(addr, port, i);
                            break;
                        }
                    }
                }
            } else {
                for (int i = 0; i < 4; i++) {
                    ArtNet_SendPollReply(addr, port, i);
                }
            }
        } break;
        case ArtCode_OpIpProg:
            ArtNet_HandleIpProg((ArtNet_IpProg *)net_buffer, addr, port);
            break;
        case ArtCode_OpAddress:
            ArtNet_HandleAddress((ArtNet_Address *)net_buffer, addr, port);
            break;
        case ArtCode_OpDmx:
            ArtNet_HandleOutput((ArtNet_Dmx *)net_buffer);
            break;
        case ArtCode_OpInput:
            ArtNet_HandleInput((ArtNet_Input *)net_buffer, addr, port);
            break;
        }
    }
}

static void ArtNet_SendPollReply(const ip_addr_t *addr, u16_t port, unsigned char art_port) {
    if (art_port < 0 || art_port >= 4)
        return;

    memclr(net_buffer, sizeof(net_buffer));

    ArtNet_PollReply *reply = (ArtNet_PollReply *)net_buffer;
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

    reply->VersInfo = UI16_LITTLE_ENDIAN(FIRMWARE_INT);
    reply->OEM = 0xFFFF;

    reply->NetSwitch = config->ArtNet[art_port].Network;
    reply->SubSwitch = config->ArtNet[art_port].Subnet;

    reply->NumPorts = UI16_LITTLE_ENDIAN(1);
    reply->PortTypes[0] = config->ArtNet[art_port].PortDirection << 6;
    reply->SwOut[0] = config->ArtNet[art_port].Universe;
    reply->SwIn[0] = config->ArtNet[art_port].Universe;

    reply->BindIndex = art_port;

    if (config->ArtNet[art_port].PortFlags & PORT_FLAG_INDISABLED) {
        reply->GoodInput[0] = 1 << 3;
    }

    if ((config->ArtNet[art_port].PortFlags & PORT_FLAG_RDM) == 0) {
        reply->GoodOutputB[0] |= (1 << 7);
    }

    if ((config->ArtNet[art_port].PortFlags & PORT_FLAG_SINGLE) == 0) {
        reply->GoodOutputB[0] |= (1 << 6);
    }

    reply->Status1 = 0x20;
    reply->Status2 = (1 << 0) | (1 << 2) | (1 << 3) | (1 << 6) | (1 << 7);
    if (config->Mode == CONFIGIP_DHCP) {
        reply->Status2 |= (1 << 1);
    }
    reply->Status3 = (1 << 3) | (1 << 5);
    reply->Status3 |= (config->ArtNet[art_port].FailoverMode << 6);

    reply->AcnPriority = config->ArtNet[art_port].AcnPriority;

    memcpy(reply->ShortName, config->ArtNet[art_port].ShortName, 18);
    memcpy(reply->LongName, config->ArtNet[art_port].LongName, 64);

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
    Config_Store();

    memclr(net_buffer, sizeof(net_buffer));
    ArtNet_IpProgReply *reply = (ArtNet_IpProgReply *)net_buffer;
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
    if (data->BindIndex < 0 || data->BindIndex >= 4)
        return;

    if (data->NetSwitch == 0) {
        config->ArtNet[data->BindIndex].Network = ARTNET_NET;
    } else if (data->NetSwitch & 0x80) {
        config->ArtNet[data->BindIndex].Network = data->NetSwitch & ~0x80;
    }

    if (data->ShortName[0] != 0) {
        memcpy(config->ArtNet[data->BindIndex].ShortName, data->ShortName, 18);
    }

    if (data->LongName[0] != 0) {
        memcpy(config->ArtNet[data->BindIndex].LongName, data->LongName, 64);
    }

    if (data->SubSwitch == 0) {
        config->ArtNet[data->BindIndex].Subnet = ARTNET_SUB;
    } else if (data->SubSwitch & 0x80) {
        config->ArtNet[data->BindIndex].Subnet = data->SubSwitch & ~0x80;
    }

    if (data->AcnPriority >= 0 && data->AcnPriority < 200) {
        config->ArtNet[data->BindIndex].AcnPriority = data->AcnPriority;
    }

    char idx = data->Command & 0x0F;
    char cmd = data->Command & 0xF0;
    if (cmd > 0 && idx == 0) {
        switch (cmd) {
        case 0x90:
            DMX_ClearBuffer(data->BindIndex);
            break;
        case 0xA0:
            config->ArtNet[data->BindIndex].PortFlags |= PORT_FLAG_SINGLE;
            break;
        case 0xB0:
            config->ArtNet[data->BindIndex].PortFlags &= ~PORT_FLAG_SINGLE;
            break;
        case 0xC0:
            config->ArtNet[data->BindIndex].PortFlags |= PORT_FLAG_RDM;
            break;
        case 0xD0:
            config->ArtNet[data->BindIndex].PortFlags &= ~PORT_FLAG_RDM;
            break;

        case 0x20: // Output
            config->ArtNet[data->BindIndex].PortDirection = ARTNET_OUTPUT;
            break;
        case 0x30: // Input
            config->ArtNet[data->BindIndex].PortDirection = ARTNET_INPUT;
            break;
        }
    } else {
        switch (data->Command) {
            // Failover mode
        case 0x08:
            config->ArtNet[data->BindIndex].FailoverMode = ArtFail_Hold;
            break;
        case 0x09:
            config->ArtNet[data->BindIndex].FailoverMode = ArtFail_Zero;
            break;
        case 0x0A:
            config->ArtNet[data->BindIndex].FailoverMode = ArtFail_Full;
            break;
        case 0x0B:
            config->ArtNet[data->BindIndex].FailoverMode = ArtFail_Scene;
            break;
        case 0x0C:
            Config_StoreFailsafeScene(DMX_GetBuffer(data->BindIndex), data->BindIndex);
            break;
        }
    }

    if (config->ArtNet[data->BindIndex].PortDirection == ARTNET_OUTPUT) {
        if (data->SwOut[0] & 0x80) {
            config->ArtNet[data->BindIndex].Universe = data->SwOut[0] & ~0x80;
        } else if (data->SwOut[0] == 0) {
            config->ArtNet[data->BindIndex].Universe = ARTNET_UNI + data->BindIndex;
        }
    } else {
        if (data->SwIn[0] & 0x80) {
            config->ArtNet[data->BindIndex].Universe = data->SwIn[0] & ~0x80;
        } else if (data->SwIn[0] == 0) {
            config->ArtNet[data->BindIndex].Universe = ARTNET_UNI + data->BindIndex;
        }
    }

    Config_ApplyArtNet();
    Config_Store();
    ArtNet_SendPollReply(addr, port, data->BindIndex);
}

static void ArtNet_HandleInput(ArtNet_Input *data, const ip_addr_t *addr, u16_t port) {
    if (data->Input[data->BindIndex] & 0x01) {
        config->ArtNet[data->BindIndex].PortFlags |= PORT_FLAG_INDISABLED;
    } else {
        config->ArtNet[data->BindIndex].PortFlags &= ~PORT_FLAG_INDISABLED;
    }

    ArtNet_SendPollReply(addr, port, data->BindIndex);
}

static void ArtNet_HandleOutput(ArtNet_Dmx *data) {
    data->Length = UI16_LITTLE_ENDIAN(data->Length);
    char net = data->Net & 0x7F;
    char sub = (data->SubUni & 0xF0) >> 4;
    char uni = (data->SubUni & 0x0F);

    if (data->Length <= 512) {
        for (int i = 0; i < 4; i++) {
            if (config->ArtNet[i].Subnet == sub && config->ArtNet[i].Network == net) {
                if (uni == config->ArtNet[i].Universe) {
                    DMX_SetBuffer(i, data->Data, data->Length);
                    artnet_timeout[i] = sys_now();
                    artnet_failover_state[i] = 0;
                }
            }
        }
    }
}

static void ArtNet_ApplyFailover(int idx) {
    if (idx >= 0 && idx < 4) {
        switch (config->ArtNet[idx].FailoverMode) {
        case ArtFail_Hold:
            break;
        case ArtFail_Zero:
            DMX_ClearBuffer(idx);
            break;
        case ArtFail_Full: {
            unsigned char *buffer = DMX_GetBuffer(idx);
            for (int i = 0; i < 512; i++) {
                buffer[i] = 0xFF;
            }
        } break;
        case ArtFail_Scene:
            Config_LoadFailsafeScene(DMX_GetBuffer(idx), idx);
            break;
        }
    }
}

void ArtNet_InputTick(char forceTransmit) {
    for (int i = 0; i < 4; i++) {
        if ((config->ArtNet[i].PortDirection == ARTNET_INPUT) && ((config->ArtNet[i].PortFlags & PORT_FLAG_INDISABLED) == 0)) {
            if (DMX_IsInputNew(i) || forceTransmit) {
                // Send input
                memclr(net_buffer, sizeof(net_buffer));
                ArtNet_Dmx *reply = (ArtNet_Dmx *)net_buffer;
                memcpy(reply->Header.Signature, "Art-Net", 8);
                reply->Header.OpCode = ArtCode_OpDmx;
                reply->Header.ProtocolVersion = UI16_LITTLE_ENDIAN(14);

                reply->Net = config->ArtNet[i].Network & 0x7F;
                reply->SubUni = ((config->ArtNet[i].Subnet & 0x0F) << 4) | (config->ArtNet[i].Universe & 0x0F);
                reply->Length = UI16_LITTLE_ENDIAN(512);

                unsigned char *buffer = DMX_GetBuffer(i);
                memcpy(reply->Data, buffer, 512);

                struct pbuf *p;
                p = pbuf_alloc(PBUF_TRANSPORT, sizeof(ArtNet_Dmx), PBUF_POOL);
                memcpy_pbuf(p, reply, sizeof(ArtNet_Dmx));
                udp_sendto(artnet, p, IP4_ADDR_BROADCAST, artnet_port);
                pbuf_free(p);
            }
        }
    }
}

void ArtNet_TimeoutTick() {
    for (int i = 0; i < 4; i++) {
        if (sys_now() - artnet_timeout[i] > ARTNET_FAILTIMEOUT) {
            if (artnet_failover_state[i] == 0) {
                ArtNet_ApplyFailover(i);
                artnet_failover_state[i] = 1;
            }
        }
    }
}
