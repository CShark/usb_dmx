#ifndef __DHCP_SERVER_H_
#define __DHCP__SERVER_H_
#include "lwip/netif.h"
#include "lwip/udp.h"

typedef enum {
    DHCP_Discover = 1,
    DHCP_Offer = 2,
    DHCP_Request = 3,
    DHCP_Decline = 4,
    DHCP_Ack = 5,
    DHCP_Nack = 6,
    DHCP_Release = 7,
    DHCP_Inform = 8,
    DHCP_ForceRenew = 9,
} DHCP_Type_Enum;

#define DHCP_LEASE_QUERY 10
#define DHCP_LEASE_UNASSIGNED 11
#define DHCP_LEASE_UNKNOWN 12
#define DHCP_LEASE_ACTIVE 13
#define DHCP_BULKLEASE_QUERY 14
#define DHCP_LEASE_QUERYDONE 15
#define DHCP_ACTIVELEASE_QUERY 16
#define DHCP_LEASE_QUERYSTATUS 17
#define DHCP_TLS 18

#pragma pack(1)
typedef struct {
    unsigned char Operation;
    unsigned char HardwareType;
    unsigned char HwAddressLength;
    unsigned char Hops;
    unsigned int TransactionID;
    unsigned short Seconds;
    unsigned short Flags;
    unsigned int IpClient;
    unsigned int IpYours;
    unsigned int IpServer;
    unsigned int IpGateway;
    unsigned char HwAddress[16];
    unsigned char __spare[192];
    unsigned int MagicCookie;
} DHCP_PACKET;

typedef struct {
    unsigned char Code;
    unsigned char Length;
} DHCP_Header;

typedef struct {
    DHCP_Header Header;
    DHCP_Type_Enum Type;
} DHCP_Type;

typedef struct {
    DHCP_Header Header;
    unsigned char RequestedItems[];
} DHCP_RequestList;

typedef struct {
    DHCP_Header Header;
    unsigned char Type;
    unsigned char Identifier[];
} DHCP_ClientIdentifier;

typedef struct {
    DHCP_Header Header;
    unsigned int Ip[];
} DHCP_IpOption;

typedef struct {
    DHCP_Header Header;
    unsigned int Ip;
} DHCP_IpOptionSingle;

typedef struct {
    DHCP_Header Header;
    unsigned int Value;
} DHCP_IntOption;

typedef struct {
    DHCP_PACKET *Header;

    DHCP_Type *Type;
    DHCP_RequestList *RequestList;
    DHCP_ClientIdentifier *ClientIdentifier;
    
    DHCP_IpOption *Subnet;
    DHCP_IpOption *Router;
    DHCP_IpOption *Broadcast;
    DHCP_IpOption *RequestedIp;
} DHCP_Options;

typedef struct {
    DHCP_PACKET Header;
    DHCP_Type Type;
    DHCP_IpOptionSingle Subnet;
    DHCP_IpOptionSingle Router;
    DHCP_IpOptionSingle DhcpServer;
    DHCP_IntOption TTL;

    unsigned char OptionEnd;
}DHCP_OfferResponse;
#pragma pack()

void DhcpServer_Init();
void DhcpServer_Configure(ip_addr_t *client, ip_addr_t *subnet);
void DhcpServer_Start(struct netif *netif);
void DhcpServer_Stop();
#endif