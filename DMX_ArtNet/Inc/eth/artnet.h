#ifndef __ARTNET_H
#define __ARTNET_H

#include "lwip/netif.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"
#include "arch/cc.h"

#define ARTNET_FAILTIMEOUT 5000

typedef enum {
    ArtCode_OpPoll = 0x2000,
    ArtCode_OpPollReply = 0x2100,
    ArtCode_OpDiagData = 0x2300,
    ArtCode_OpCommand = 0x2400,

    ArtCode_OpDmx = 0x5000,
    ArtCode_OpNzs = 0x5100,
    ArtCode_OpSync = 0x5200,
    ArtCode_OpAddress = 0x6000,
    ArtCode_OpInput = 0x7000,

    ArtCode_OpIpProg = 0xF800,
    ArtCode_OpIpProgReply = 0xF900,
} ArtNet_OpCodes;

typedef enum {
    ArtFail_Hold = 0b00,
    ArtFail_Zero = 0b01,
    ArtFail_Full = 0b10,
    ArtFail_Scene = 0b11,
} ArtNet_Failover;

typedef enum {
    PORT_FLAG_SINGLE = 0x01,
    PORT_FLAG_RDM = 0x02,
    PORT_FLAG_INDISABLED = 0x04,
} ArtNet_Port_Flags;

#pragma pack(1)

typedef struct {
    unsigned char Signature[8];
    unsigned short OpCode;
} ArtNet_HeaderShort;

typedef struct {
    char Signature[8];
    unsigned short OpCode;
    unsigned short ProtocolVersion;
} ArtNet_Header;

typedef struct {
    ArtNet_Header Header;
    unsigned char Flags;
    unsigned char DiagPriority;
    unsigned short TargetPort[2];
} ArtNet_Poll;

typedef struct {
    ArtNet_HeaderShort Header;
    unsigned char IpAddress[4];
    unsigned short Port;
    unsigned short VersInfo;
    unsigned char NetSwitch;
    unsigned char SubSwitch;
    unsigned short OEM;
    unsigned char UbeaVersion;
    unsigned char Status1;
    unsigned char EstaMan[2];
    unsigned char ShortName[18];
    unsigned char LongName[64];
    unsigned char NodeReport[64];
    unsigned short NumPorts;
    unsigned char PortTypes[4];
    unsigned char GoodInput[4];
    unsigned char GoodOutputA[4];
    unsigned char SwIn[4];
    unsigned char SwOut[4];
    unsigned char AcnPriority;
    unsigned char SwMacro;
    unsigned char SwRemote;
    unsigned char __spare[3];
    unsigned char Style;
    unsigned char Mac[6];
    unsigned char BindIp[4];
    unsigned char BindIndex;
    unsigned char Status2;
    unsigned char GoodOutputB[4];
    unsigned char Status3;
    unsigned char DefaultRespUID[6];
    unsigned char __filler[15];
} ArtNet_PollReply;

typedef struct {
    ArtNet_Header Header;
    unsigned char __filler1[2];
    unsigned char Command;
    unsigned char __filler2;
    unsigned char ProgIp[4];
    unsigned char ProgSubnet[4];
    unsigned char ProgGateway[4];
    unsigned char __spare[4];
} ArtNet_IpProg;

typedef struct {
    ArtNet_Header Header;
    unsigned char __filler1[4];
    unsigned char ProgIp[4];
    unsigned char ProgSubnet[4];
    unsigned short ProgPort;
    unsigned char Status;
    unsigned char __filler2;
    unsigned char ProgGateway[4];
    unsigned char __filler3[2];
} ArtNet_IpProgReply;

typedef struct {
    ArtNet_Header Header;
    unsigned char NetSwitch;
    unsigned char BindIndex;
    unsigned char ShortName[18];
    unsigned char LongName[64];
    unsigned char SwIn[4];
    unsigned char SwOut[4];
    unsigned char SubSwitch;
    unsigned char AcnPriority;
    unsigned char Command;
} ArtNet_Address;

typedef struct {
    ArtNet_Header Header;
    unsigned char Sequence;
    unsigned char Physical;
    unsigned char SubUni;
    unsigned char Net;
    unsigned short Length;
    unsigned char Data[512];
} ArtNet_Dmx;

typedef struct {
    ArtNet_Header Header;
    unsigned char __filler;
    unsigned char BindIndex;
    unsigned short NumPorts;
    unsigned char Input[4];
} ArtNet_Input;
#pragma pack()

void ArtNet_Init(struct netif *netif);
void ArtNet_InputTick(char forceTransmit);
void ArtNet_TimeoutTick();

#endif
