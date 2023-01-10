#include "cdc_device.h"
#include "config.h"
#include "platform.h"

static char buffer[64];
static char lineCoding[7];

static void CDC_SendID();
static void CDC_ResetConfig();

char CDC_SetupPacket(USB_SETUP_PACKET *setup, char *data, short length) {
    // Windows requires us to remember the line coding
    switch (setup->Request) {
    case CDC_CONFIG_CONTROLLINESTATE:
        if (setup->Value & 0x01) {
            CDC_SendID();
        }
        break;
    case CDC_CONFIG_GETLINECODING:
        USB_Transmit(0, lineCoding, 7);
        break;
    case CDC_CONFIG_SETLINECODING:
        for (int i = 0; i < 7; i++) {
            lineCoding[i] = data[i];
        }
        return USB_OK;
        break;
    }
}

void CDC_HandlePacket(char ep, short length) {
    // Just mirror the text
    USB_Fetch(ep, buffer, &length);

    switch (buffer[0]) {
    case 0xF0:
        PWR->CR1 |= PWR_CR1_DBP;
        TAMP->BKP0R = 0xF0;
        PWR->CR1 &= ~PWR_CR1_DBP;
    case 0xF1:
        NVIC_SystemReset();
        break;
    case 0xF2:
        Config_Reset();
        break;
    }
}

static void CDC_SendID() {
    USB_Transmit(4, UID->ID, sizeof(UID->ID));
}