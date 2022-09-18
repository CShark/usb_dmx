#include "hid_device.h"
#include "dmx_usart.h"

static char buffer[33];

void HID_HandlePacket(char ep, short length) {
    USB_Fetch(ep, buffer, &length);

    if (buffer[0] == 0x10) {
        // config packet
        if ((buffer[1] & 0x06)) {
            // Output or Input
            USART_SetPortState(ep - 1, 1);
        } else {
            // disabled
            USART_SetPortState(ep - 1, 0);
        }
    } else if (buffer[0] < 0x10) {
        USART_SetBufferPage(ep - 1, buffer[0], &buffer[1]);
    }
}

char HID_SetupPacket(USB_SETUP_PACKET *setup, char *data, short length) {
    return USB_ERR;
}
