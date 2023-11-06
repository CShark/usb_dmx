#include "eth/dhcp_server.h"
#include "eth/global.h"
#include "lwip/dhcp.h"
#include "lwip/pbuf.h"
#include "platform.h"

// This is an NCM device. So we don't do true DHCP Server, but always offer the same IP,
// as there only ever will be one other device

static struct udp_pcb *dhcp;
static struct netif *dhcp_if;
static const short dhcp_port = 67;
static const int dhcp_magicCookie = 1666417251;

static ip_addr_t *dhcp_clientIp;
static ip_addr_t *dhcp_subnet;

static void DhcpServer_Receive(void *arg, struct udp_pcb *pcb, struct pbuf *p,
		const ip_addr_t *addr, u16_t port);
static void DhcpServer_Discover(DHCP_Options *options);
static void DhcpServer_Request(DHCP_Options *options);

void DhcpServer_Init() {
	dhcp = udp_new();
	udp_bind(dhcp, IP4_ADDR_ANY, dhcp_port);
	udp_recv(dhcp, DhcpServer_Receive, NULL);
	ip_set_option(dhcp, SOF_BROADCAST);
}

void DhcpServer_Configure(ip_addr_t *client, ip_addr_t *subnet) {
	dhcp_clientIp = client;
	dhcp_subnet = subnet;
}

void DhcpServer_Start(struct netif *netif) {
	dhcp_if = netif;
}

void DhcpServer_Stop() {
	dhcp_if = NULL;
}

static void DhcpServer_Receive(void *arg, struct udp_pcb *pcb, struct pbuf *p,
		const ip_addr_t *addr, u16_t port) {
	if (dhcp_if == NULL)
		return;

	unsigned short len = p->tot_len;
	unsigned short offset = 0;

	if (pbufcpy_mem(net_buffer, p, sizeof(net_buffer)) == 0) {
		pbuf_free(p);
		return;
	}

	pbuf_free(p);

	DHCP_PACKET *header = (DHCP_PACKET*) net_buffer;
	DHCP_Options options = { .Header = header };

	if (header->MagicCookie != dhcp_magicCookie) {
		return;
	}

	offset = sizeof(DHCP_PACKET);
	while (offset < len) {
		if (net_buffer[offset] == 0) {
			offset++;
		} else if (net_buffer[offset] == 255) {
			break;
		} else {
			switch (net_buffer[offset]) {
			case 53: // DHCP Message Type
				options.Type = (DHCP_Type*) &net_buffer[offset];
				break;
			case 50: // Request IP Address
				options.RequestedIp = (DHCP_IpOption*) &net_buffer[offset];
				break;
			case 61: // Client Identifier
				options.ClientIdentifier =
						(DHCP_ClientIdentifier*) &net_buffer[offset];
				break;
			case 55: // Parameter Request List
				options.RequestList = (DHCP_RequestList*) &net_buffer[offset];
				break;
			}

			// Read length and skip
			offset += net_buffer[offset + 1] + 2;
		}
	}

	if (!options.Type) {
		return;
	}

	switch (options.Type->Type) {
	case DHCP_Discover:
		DhcpServer_Discover(&options);
		break;
	case DHCP_Request:
		DhcpServer_Request(&options);
		break;
	default:
		// Ignore all other requests
		break;
	}
}

static void DhcpServer_Discover(DHCP_Options *options) {
	DHCP_OfferResponse response = { .Header.Operation = 2,
			.Header.HardwareType = 1, .Header.HwAddressLength = 6,

			.Header.IpYours = dhcp_clientIp->addr, .Header.IpServer =
					dhcp_if->ip_addr.addr,

			.Header.TransactionID = options->Header->TransactionID,
			.Header.MagicCookie = options->Header->MagicCookie };

	memcpy(response.Header.HwAddress, options->Header->HwAddress,
			sizeof(response.Header.HwAddress));

	response.Type.Header.Code = 53;
	response.Type.Header.Length = 1;
	response.Type.Type = DHCP_Offer;

	response.Subnet.Header.Code = 1;
	response.Subnet.Header.Length = 4;
	response.Subnet.Ip = dhcp_subnet->addr;

	response.Router.Header.Code = 3;
	response.Router.Header.Length = 4;
	response.Router.Ip = dhcp_if->ip_addr.addr;

	response.DhcpServer.Header.Code = 54;
	response.DhcpServer.Header.Length = 4;
	response.DhcpServer.Ip = dhcp_if->ip_addr.addr;

	response.TTL.Header.Code = 51;
	response.TTL.Header.Length = 4;
	response.TTL.Value = UI32_LITTLE_ENDIAN(86400);

	response.OptionEnd = 255;

	struct pbuf *p;
	p = pbuf_alloc(PBUF_TRANSPORT, sizeof(response), PBUF_POOL);
	memcpy_pbuf(p, &response, sizeof(response));
	udp_sendto(dhcp, p, IP4_ADDR_BROADCAST, 68);
	pbuf_free(p);
}

static void DhcpServer_Request(DHCP_Options *options) {
	DHCP_OfferResponse response = { .Header.Operation = 2,
			.Header.HardwareType = 1, .Header.HwAddressLength = 6,

			.Header.IpYours = options->RequestedIp->Ip[0], .Header.IpServer =
					dhcp_if->ip_addr.addr,

			.Header.TransactionID = options->Header->TransactionID,
			.Header.MagicCookie = options->Header->MagicCookie };

	memcpy(response.Header.HwAddress, options->Header->HwAddress,
			sizeof(response.Header.HwAddress));

	if (options->RequestedIp == NULL) {
		response.Header.IpYours = options->Header->IpClient;
	}

	if(options->Header->IpClient != NULL) {
		response.Header.IpClient = options->Header->IpClient;
	}

	response.Type.Header.Code = 53;
	response.Type.Header.Length = 1;
	response.Type.Type = DHCP_Ack;

	response.Subnet.Header.Code = 1;
	response.Subnet.Header.Length = 4;
	response.Subnet.Ip = dhcp_subnet->addr;

	response.Router.Header.Code = 3;
	response.Router.Header.Length = 4;
	response.Router.Ip = dhcp_if->ip_addr.addr;

	response.DhcpServer.Header.Code = 54;
	response.DhcpServer.Header.Length = 4;
	response.DhcpServer.Ip = dhcp_if->ip_addr.addr;

	response.TTL.Header.Code = 51;
	response.TTL.Header.Length = 4;
	response.TTL.Value = UI32_LITTLE_ENDIAN(86400);

	response.OptionEnd = 255;

	struct pbuf *p;
	p = pbuf_alloc(PBUF_TRANSPORT, sizeof(response), PBUF_POOL);
	memcpy_pbuf(p, &response, sizeof(response));
	if (options->Header->IpClient == NULL) {
		udp_sendto(dhcp, p, IP4_ADDR_BROADCAST, 68);
	} else {
		udp_sendto(dhcp, p, ip_2_ip4(&options->Header->IpClient), 68);
	}
	pbuf_free(p);
}
