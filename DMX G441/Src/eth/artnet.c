#include "eth/artnet.h"
#include "dmx_usart.h"
#include "lwip/autoip.h"
#include "platform.h"
#include "config.h"

static struct udp_pcb *artnet;
static struct netif *artif;
static short artnet_port = 6454;
static char artnet_buffer[512 + 18];

static char artnet_net = ART_NET;
static char artnet_subnet = ART_SUBNET;
static char artnet_universe[8] = {ART_UNIVERSE, ART_UNIVERSE + 1, ART_UNIVERSE + 2, ART_UNIVERSE + 3, ART_UNIVERSE + 4, ART_UNIVERSE + 5, ART_UNIVERSE + 6, ART_UNIVERSE + 7};
static char artnet_shortName[18] = "ArtNet-Node";
static char artnet_longName[64] = "Custom Art-Net Node";
static char *artnet_portConfig;
static char artnet_inputs = 0x0F;

static void ArtNet_SendPollReply(const ip_addr_t *addr, u16_t port);
static void ArtNet_HandleIpProg(ArtNet_IpProg *p, const ip_addr_t *addr, u16_t port);
static void ArtNet_Receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
static void ArtNet_HandleAddress(ArtNet_Address *data, const ip_addr_t *addr, u16_t port);
static void ArtNet_HandleOutput(ArtNet_Dmx *data);
static void ArtNet_HandleInput(ArtNet_Input *data, const ip_addr_t *addr, u16_t port);

void ArtNet_Init(struct netif *netif, char *portConfig) {
    artnet_portConfig = portConfig;
    artif = netif;
    artnet = udp_new();
    udp_bind(artnet, IP4_ADDR_ANY, artnet_port);
    udp_recv(artnet, ArtNet_Receive, NULL);
    ip_set_option(artnet, SOF_BROADCAST);
}

static void ArtNet_Receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    unsigned int offset = 0;
    unsigned short length = p->tot_len;
    struct pbuf *q;

    for (q = p; q != NULL; q = q->next) {
        memcpy(artnet_buffer + offset, q->payload, q->len);
        offset += q->len;

        if (q->len == q->tot_len) {
            break;
        }
    }

    pbuf_free(p);

    ArtNet_Header *art = artnet_buffer;
    art->ProtocolVersion = UI16_LITTLE_ENDIAN(art->ProtocolVersion);

    if (strcmp(art->Signature, "Art-Net") == 0 && art->ProtocolVersion >= 14) {
        switch (art->OpCode) {
        case ArtCode_OpPoll: {
            ArtNet_Poll *poll = artnet_buffer;

            if (poll->Flags & 0x20) {
                // Targeted mode, check addresses
                short base = (artnet_net & 0x7F) << 8;
                base += (artnet_subnet & 0x0F) << 4;

                for (int i = 0; i < 4; i++) {
                    if (artnet_portConfig[i] & USART_OUTPUT) {
                        short address = base + artnet_universe[i];

                        if (poll->TargetPort[0] <= address && poll->TargetPort[1] >= address) {
                            ArtNet_SendPollReply(addr, port);
                            break;
                        }
                    }

                    if (artnet_portConfig[i] & USART_INPUT) {
                        short address = base + artnet_universe[i + 4];

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
            ArtNet_HandleIpProg(artnet_buffer, addr, port);
            break;
        case ArtCode_OpAddress:
            ArtNet_HandleAddress(artnet_buffer, addr, port);
            break;
        case ArtCode_OpDmx:
            ArtNet_HandleOutput(artnet_buffer);
            break;
        }
    }
}

static void ArtNet_SendPollReply(const ip_addr_t *addr, u16_t port) {
    memclr(artnet_buffer, sizeof(artnet_buffer));

    ArtNet_PollReply *reply = artnet_buffer;
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

    reply->NetSwitch = artnet_net;
    reply->SubSwitch = artnet_subnet;

    reply->NumPorts = UI16_LITTLE_ENDIAN(4);
    for (int i = 0; i < 4; i++) {
        reply->PortTypes[i] = artnet_portConfig[i] << 6;
        reply->SwOut[i] = artnet_universe[i];
        reply->SwIn[i] = artnet_universe[i + 4];
    }

    for (int i = 0; i < 4; i++) {
        reply->GoodInput[i] = (((artnet_inputs ^ 0x01) & (1 << i)) >> i) << 3;
    }

    reply->Status1 = 0xE0;
    reply->Status2 = (1 << 3);
    reply->Status3 = 0;

    memcpy(reply->ShortName, artnet_shortName, sizeof(artnet_shortName));
    memcpy(reply->LongName, artnet_longName, sizeof(artnet_longName));

    struct pbuf *p;
    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(ArtNet_PollReply), PBUF_POOL);
    memcpy_pbuf(p, reply, sizeof(ArtNet_PollReply));
    udp_sendto(artnet, p, addr, port);
    pbuf_free(p);
}

static void ArtNet_HandleIpProg(ArtNet_IpProg *data, const ip_addr_t *addr, u16_t port) {
    if (data->Command & 0x80) {
        if (data->Command & 0x40) {
            // Enable DHCP
            autoip_start(artif);
        } else if (data->Command & 0x08) {
            Config_ResetIp();
        } else {
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

    memclr(artnet_buffer, sizeof(artnet_buffer));
    ArtNet_IpProgReply *reply = artnet_buffer;
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
        artnet_net = ART_NET;
    } else if (data->NetSwitch & 0x80) {
        artnet_net = data->NetSwitch;
    }

    if (data->ShortName[0] != 0) {
        memcpy(artnet_shortName, data->ShortName, sizeof(artnet_shortName));
    }

    if (data->LongName[0] != 0) {
        memcpy(artnet_longName, data->LongName, sizeof(artnet_longName));
    }

    if (data->SubSwitch == 0) {
        artnet_subnet = ART_SUBNET;
    } else if (data->SubSwitch & 0x80) {
        artnet_subnet = data->SubSwitch;
    }

    for (int i = 0; i < 4; i++) {
        if (data->SwOut[i] & 0x80) {
            artnet_universe[i] = data->SwOut[i];
        } else if (data->SwOut[i] == 0) {
            artnet_universe[i] = ART_UNIVERSE + i;
        }

        if (data->SwIn[i] & 0x80) {
            artnet_universe[i + 4] = data->SwIn[i];
        } else if (data->SwIn[i] == 0) {
            artnet_universe[i + 4] = ART_UNIVERSE + i + 4;
        }
    }

    char idx = data->Command & 0x0F;
    char cmd = data->Command & 0xF0;
    if (idx >= 0 && idx < 4) {
        switch (cmd) {
        case 0x90:
            USART_ClearBuffer(idx);
            break;
        case 0xA0:
            USART_AlterPortFlags(idx, PORT_FLAG_SINGLE, 1);
            break;
        case 0xB0:
            USART_AlterPortFlags(idx, PORT_FLAG_SINGLE, 0);
            break;
        case 0xC0:
            USART_AlterPortFlags(idx, PORT_FLAG_RDM, 1);
            break;
        case 0xD0:
            USART_AlterPortFlags(idx, PORT_FLAG_RDM, 0);
            break;
        }
    }

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
    char sub = (data->SubUni & 0xF0) >> 8;
    char uni = (data->SubUni & 0x0F);

    if (data->Length <= 512) {
        if (artnet_subnet == sub && artnet_net == net) {
            if (uni == artnet_universe[0]) {
                USART_SetBuffer(0, data->Data, data->Length);
            }

            if (uni == artnet_universe[1]) {
                USART_SetBuffer(1, data->Data, data->Length);
            }

            if (uni == artnet_universe[2]) {
                USART_SetBuffer(2, data->Data, data->Length);
            }

            if (uni == artnet_universe[3]) {
                USART_SetBuffer(3, data->Data, data->Length);
            }
        }
    }
}

void ArtNet_InputTick() {
    for (int i = 0; i < 4; i++) {
        if (artnet_portConfig[i] & USART_INPUT && artnet_inputs & (1 << i)) {
            // Send input
            memclr(artnet_buffer, sizeof(artnet_buffer));
            ArtNet_Dmx *reply = artnet_buffer;
            memcpy(reply->Header.Signature, "Art-Net", 8);
            reply->Header.OpCode = ArtCode_OpDmx;
            reply->Header.ProtocolVersion = UI16_LITTLE_ENDIAN(14);

            reply->Net = artnet_net & 0x7F;
            reply->SubUni = ((artnet_subnet & 0x0F) << 4) | (artnet_universe[i] & 0x0F);
            reply->Length = 512;

            char *buffer = USART_GetDmxBuffer(i);
            memcpy(reply->Data, buffer, 512);

            struct pbuf *p;
            p = pbuf_alloc(PBUF_TRANSPORT, sizeof(reply), PBUF_POOL);
            memcpy_pbuf(p, reply, sizeof(reply));
            udp_sendto(artnet, p, IP4_ADDR_BROADCAST, artnet_port);
            pbuf_free(p);
        }
    }
}