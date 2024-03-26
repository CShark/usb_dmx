#include "rdm.h"

static RDM_State rdm_state[] = {
    {
        .portId = 1,
    },
    {
        .portId = 2,
    },
    {
        .portId = 3,
    },
    {
        .portId = 4,
    }};

static void RDM_InitMessage(RDM_State *rdm) {
    RDM_Message *msg = rdm->RdmBuffer;

    msg->StartCode = USART_SC_RDM;
    msg->SubCode = USART_SC_SUB_MESSAGE;
    msg->Transaction++;
    msg->PortID = rdm->portId;
    msg->MessageCount = 0;
}

static unsigned short RDM_UnmuteAll(RDM_State *rdm) {
    RDM_InitMessage(rdm);
    
    
}

void RDM_SetEnabled(unsigned char i, unsigned char enabled) {

}

RDM_IoRequest RDM_TickPort(char port) {
    RDM_IoRequest req;
    req.Type = RDM_IOREQ_NONE;

    if (port >= 0 && port < 4) {
        if ((rdm_state[port].CommState & RDM_DISC_Msk) == RDM_DISC_None) {
            rdm_state[port].CommState &= ~RDM_DISC_Msk;
            rdm_state[port].CommState |= RDM_DISC_Full;

            // start by unmuting all devices
            req.Length = RDM_UnmuteAll(&rdm_state[port]);
            req.Type = RDM_IOREQ_OUT;
            req.Buffer = rdm_state[port].RdmBuffer;
        }
    }

    return req;
}