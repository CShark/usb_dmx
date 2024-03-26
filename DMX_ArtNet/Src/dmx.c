#include "dmx.h"

static unsigned char dmx_buffer[4][513] = {{0}, {0}, {0}, {0}};
static USART_PortConfig *ports;

void DMX_Init(USART_PortConfig *port, CONFIG *config) {
    ports = port;

    for (int i = 0; i < 4; i++) {
        port[i].DmxData.Buffer = dmx_buffer[i];

        if (config->ArtNet[i].PortFlags & PORT_FLAG_SINGLE) {
            port[i].DmxData.Flags |= DMX_Single;
        }

        DMX_SetInDisabled(i, config->ArtNet[i].PortFlags & PORT_FLAG_INDISABLED);
    }
}

char DMX_Tick(USART_PortConfig *port) {
    if ((port->Status & USART_DIR_IOMsk) == USART_DIR_Output) {
        if ((port->DmxData.Flags & DMX_Single) == 0 || (port->DmxData.Flags & DMX_ForceSend)) {
            USART_Transmit(port, port->DmxData.Buffer, 513, NULL);
            port->DmxData.Flags &= ~DMX_ForceSend;
        }
    } else {
    }
}

void DMX_SetSingleMode(unsigned char i, unsigned char enabled) {
    if (i >= 0 && i < 4) {
        if (enabled) {
            ports[i].DmxData.Flags |= DMX_Single;
        } else {
            ports[i].DmxData.Flags &= ~DMX_Single;
        }
    }
}

void DMX_SetInDisabled(unsigned char i, unsigned char disabled) {
    if (i >= 0 && i < 4) {
        if (disabled) {
            ports[i].DmxData.Flags |= DMX_InDisabled;
        } else {
            ports[i].DmxData.Flags &= ~DMX_InDisabled;
        }

        USART_UpdateInputEnabled(i);
    }
}

void DMX_InputCallback(USART_PortConfig *port) {
    port->DmxData.Flags |= DMX_NewInput;
}

char DMX_IsInputEnabled(DMX_PortMetadata *port) {
    return (port->Flags & DMX_InDisabled) == 0;
}

unsigned char *DMX_GetBuffer(unsigned char i) {
    if (i >= 0 && i < 4) {
        return &dmx_buffer[i][1];
    }
}

void DMX_SetBuffer(unsigned char i, const unsigned char *data, unsigned short len) {
    if (i >= 0 && i < 4 && len <= 512) {
        memcpy(&dmx_buffer[i][1], data, len);
    }
}

void DMX_ClearBuffer(unsigned char i) {
    if (i >= 0 && i < 4) {
        memclr(dmx_buffer[i], 513);
    }
}

char DMX_IsInputNew(unsigned char i) {
    if (i >= 0 && i < 4) {
        if (ports[i].DmxData.Flags & DMX_NewInput) {
            ports[i].DmxData.Flags &= ~DMX_NewInput;
            return 1;
        }
    }

    return 0;
}
