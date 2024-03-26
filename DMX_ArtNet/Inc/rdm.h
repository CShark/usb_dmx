#ifndef __RDM_H
#define __RDM_H

#include "usart.h"
#include "platform.h"

typedef enum {
    RDM_CMD_Discovery = 0x10,
    RDM_CMD_DiscoveryReply = 0x11,
    RDM_CMD_Get = 0x20,
    RDM_CMD_GetReply = 0x21,
    RDM_CMD_Set = 0x30,
    RDM_CMD_SetReply = 0x31
} RDM_CommandClass;

typedef enum {
    RDM_REPLY_ACK = 0x00,
    RDM_REPLY_ACK_TIMER = 0x01,
    RDM_REPLY_NACK = 0x02,
    RDM_REPLY_ACK_OVR = 0x03
} RDM_ReplyType;

typedef enum {
    RDM_DISCREPLY_EMPTY = -1,
    RDM_DISCREPLY_SINGLE = 0,
    RDM_DISCREPLY_MULTIPLE = -2,
} RDM_DiscoverReply;

// Endianness is switched
typedef enum {
    RDM_PARAM_DISC_UNIQUE_BRANCH = 0x0100,
    RDM_PARAM_DISC_MUTE = 0x0200,
    RDM_PARAM_DISC_UNMUTE = 0x0300,
    RDM_PARAM_PROX_DEV = 0x1000,
    RDM_PARAM_PROX_DEV_COUNT = 0x1100,
    RDM_PARAM_COMMS_STATUS = 0x1500
} RDM_ParamIDs;

typedef enum {
    RDM_COMM_Idle = 0,

    RDM_DISC_None = (0 << 1),
    RDM_DISC_Complete = (1 << 1),
    RDM_DISC_Full = (2 << 1),
    RDM_DISC_Inc = (3 << 1),
    RDM_DISC_Msk = (3 << 1)
} RDM_CommState;

typedef enum {
    //RDM_DISC_None = 0,
    RDM_Disc_UnMute = 1,
    RDM_Disc_Mute = 2,
    RDM_Disc_UniqueBranch = 3,
} RDM_DiscState;

typedef enum {
    RDM_IOREQ_NONE,
    RDM_IOREQ_IN,
    RDM_IOREQ_OUT
} RDM_IoReqType;

typedef struct {
    RDM_IoReqType Type;
    unsigned char *Buffer;
    unsigned int Length;
} RDM_IoRequest;

#pragma pack(1);
typedef struct {
    unsigned char StartCode;
    unsigned char SubCode;
    unsigned char MessageLength;
    unsigned char DestUID[6];
    unsigned char SrcUID[6];

    unsigned char Transaction;
    union {
        unsigned char PortID;
        unsigned char ResponseType;
    };
    unsigned char MessageCount;
    unsigned short SubDevice;
    unsigned char CommandClass;
    unsigned short ParamID;
    unsigned char ParamLength;
    unsigned char ParamData[233];
} RDM_Message;

typedef struct {
    unsigned char Separator;
    unsigned char EUID[12];
    unsigned char Checksum[4];
} RDM_DiscoverResponse;

typedef struct {
    RDM_CommState CommState;
    RDM_Message *RdmBuffer;
    unsigned char portId;
    unsigned char devices[512][6];
} RDM_State;
#pragma pack();

void RDM_Init(USART_PortConfig *port);
void RDM_SetEnabled(unsigned char i, unsigned char enabled);

char RDM_QueueMessage(RDM_Message *message, char port);
char RDM_Discover(char force);
RDM_IoRequest RDM_TickPort(char port);

#endif
