#include "hid_device.h"
#include "dmx_usart.h"

static char buffer[33];
static char inputState[4];
__PACKED
static char dmxBuffer[4][512 + 16];

void HID_HandlePacket(char ep, short length) {
    USB_Fetch(ep, buffer, &length);

    if (buffer[0] == 0x10) {
        // config packet
        if ((buffer[1] & 0x06)) {
            // Output or Input
            USART_SetPortState(ep - 1, 1);

            // Input
            if (buffer[1] & 0x04) {
                inputState[ep - 1] = 1;
                HID_HandleTXComplete(ep, 0);
            }
        } else {
            // disabled
            inputState[ep - 1] = 0;
            USART_SetPortState(ep - 1, 0);
        }
    } else if (buffer[0] < 0x10) {
        USART_SetBufferPage(ep - 1, buffer[0], &buffer[1]);
    }
}

void HID_HandleTXComplete(char ep, short length) {
    if (ep > 0 && ep <= 4 && inputState[ep - 1]) {
        char *buffer = USART_GetDmxBuffer(ep - 1);

        if (buffer) {
            for (int i = 0; i < 16; i++) {
                dmxBuffer[ep - 1][i * 33] = i;

                for (int j = 0; j < 32; j++) {
                    dmxBuffer[ep - 1][i * 33 + 1 + j] = buffer[i * 32 + j];
                }
            }

            USB_Transmit(ep, dmxBuffer[ep - 1], 512 + 16);
        }
    }
}

char HID_SetupPacket(USB_SETUP_PACKET *setup, char *data, short length) {
    return USB_ERR;
}
