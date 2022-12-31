#include "eth/artnet.h"
#include "lwip/autoip.h"
#include "platform.h"

static struct udp_pcb *artnet;
static struct netif *artif;
static short artnet_port = 6454;
static char artnet_buffer[512 + 18];

static void ArtNet_SendPollReply(const ip_addr_t *addr, u16_t port);
static void ArtNet_HandleIpProg(ArtNet_IpProg *p, const ip_addr_t *addr, u16_t port);
static void ArtNet_Receive(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

void ArtNet_Init(struct netif *netif) {
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

    if (strcmp(art->Signature, "Art-Net") == 0 && art->ProtocolVersion >= 14) {
        switch (art->OpCode) {
        case ArtCode_OpPoll:
            ArtNet_SendPollReply(addr, port);
            break;
        case ArtCode_OpIpProg:
            ArtNet_HandleIpProg(artnet_buffer, addr, port);
            break;
        }
    }
}

static void ArtNet_SendPollReply(const ip_addr_t *addr, u16_t port) {
    ArtNet_PollReply reply = {
        .Header.Signature = "Art-Net",
        .Header.OpCode = ArtCode_OpPollReply,
        .IpAddress = {
            ip4_addr1(&artif->ip_addr),
            ip4_addr2(&artif->ip_addr),
            ip4_addr3(&artif->ip_addr),
            ip4_addr4(&artif->ip_addr),
        },
        .Mac = {artif->hwaddr[0], artif->hwaddr[1], artif->hwaddr[2], artif->hwaddr[3], artif->hwaddr[4], artif->hwaddr[5]},
        .Port = UI16_BIG_ENDIAN(0x1936),

        .VersInfo = UI16_LITTLE_ENDIAN(1),
        .OEM = 0xFFFF,

        .NetSwitch = 0,
        .SubSwitch = 0,

        .ShortName = "ArtNet-Node",
        .LongName = "Custom ArtNet-Node",
        .NodeReport = "",

        .NumPorts = UI16_LITTLE_ENDIAN(4),
        .PortTypes = {
            0b10000000,
            0b10000000,
            0b10000000,
            0b01000000,
        },
        .GoodInput = {0, 0, 0, 0},
        .GoodOutputA = {0, 0, 0, 0},
        .GoodOutputB = {0, 0, 0, 0},

        .SwIn = {0, 1, 2, 3},
        .SwOut = {0, 1, 2, 3},

        .Status1 = 0xE0,
        .Status2 = (1 << 3),
        .Status3 = 0};

    struct pbuf *p;
    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(reply), PBUF_POOL);

    memcpy_pbuf(p, &reply, sizeof(reply));

    udp_sendto(artnet, p, IP4_ADDR_BROADCAST, port);

    pbuf_free(p);
}

static void ArtNet_HandleIpProg(ArtNet_IpProg *data, const ip_addr_t *addr, u16_t port) {
    ip_addr_t newIp;

    if (data->Command & 0x80) {
        if (data->Command & 0x40) {
            // Enable DHCP
        } else if (data->Command & 0x08) {
            // Reset to default
        } else {
            if (data->Command & 0x10) {
                // Set Gateway
                IP4_ADDR(&newIp, data->ProgGateway[0], data->ProgGateway[1], data->ProgGateway[2], data->ProgGateway[3]);
                netif_set_gw(artif, &newIp);
            }

            if (data->Command & 0x04) {
                // Set IP-Address
                IP4_ADDR(&newIp, data->ProgIp[0], data->ProgIp[1], data->ProgIp[2], data->ProgIp[3]);
                netif_set_ipaddr(artif, &newIp);
            }

            if (data->Command & 0x02) {
                // Set Subnet mask
                IP4_ADDR(&newIp, data->ProgSubnet[0], data->ProgSubnet[1], data->ProgSubnet[2], data->ProgSubnet[3]);
                netif_set_netmask(artif, &newIp);
            }
        }
    }

    ArtNet_IpProgReply reply = {
        .Header.Signature = "Art-Net",
        .Header.ProtocolVersion = UI16_LITTLE_ENDIAN(14),
        .ProgGateway = {
            ip4_addr1(&artif->gw),
            ip4_addr2(&artif->gw),
            ip4_addr3(&artif->gw),
            ip4_addr4(&artif->gw)},
        .ProgIp = {ip4_addr1(&artif->ip_addr), ip4_addr2(&artif->ip_addr), ip4_addr3(&artif->ip_addr), ip4_addr4(&artif->ip_addr)},
        .ProgSubnet = {ip4_addr1(&artif->netmask), ip4_addr2(&artif->netmask), ip4_addr3(&artif->netmask), ip4_addr4(&artif->netmask)},
        .ProgPort = UI16_LITTLE_ENDIAN(artnet_port)};

    struct pbuf *p;
    p = pbuf_alloc(PBUF_TRANSPORT, sizeof(reply), PBUF_POOL);
    memcpy_pbuf(p, &reply, sizeof(reply));
    udp_sendto(artnet, p, addr, port);
    pbuf_free(p);
}

static void ArtNet_HandleAddress() {

}
