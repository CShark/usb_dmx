#include "cdc_device.h"
#include "config.h"
#include "dmx_usart.h"
#include "platform.h"

static char buffer[64];
static char lineCoding[7];

static void CDC_SendID();
static void CDC_ResetConfig();

char CDC_SetupPacket(USB_SETUP_PACKET *setup, char *data, short length) {
    // Windows requires us to remember the line coding
    switch (setup->Request) {
    case CDC_CONFIG_CONTROLLINESTATE:
        // "passive detection" :D
        // Does not require the host to send any data, as that might mess with other
        // serial devices connected to the machine. Instead it will raise the RTS control flag
        // and wait for some response, which will be the same ID as send in the ArtPollReply.
        if (setup->Value & 0x01) { // DTR
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
    USB_Fetch(ep, buffer, &length);

    switch (buffer[0]) {
    case 0xA0:
        if (length == 2) {
            Config_SetMode(buffer[1]);
            Config_ApplyNetwork();
        }
        break;
    case 0xA1:
        if (length == 14) {
            ip_addr_t self, client, subnet;
            IP4_ADDR(&self, buffer[2], buffer[3], buffer[4], buffer[5]);
            IP4_ADDR(&client, buffer[6], buffer[7], buffer[8], buffer[9]);
            IP4_ADDR(&subnet, buffer[10], buffer[11], buffer[12], buffer[13]);

            Config_DhcpServer(buffer[1], self, client, subnet);
            Config_ApplyNetwork();
        }
        break;
    case 0xA2:
        if (length == 13) {
            Config_SetIp(&buffer[1]);
            Config_SetGateway(&buffer[5]);
            Config_SetNetmask(&buffer[9]);

            Config_ApplyNetwork();
        }
        break;

    case 0xB0:
        CDC_TransmitData(&Config_GetActive()->Mode, 1);
        break;
    case 0xB1: {
        char data[1 + 4 * 3];
        CONFIG *config = Config_GetActive();
        data[0] = config->DhcpServerEnable;
        for (int i = 0; i < 4; i++) {
            data[i + 1] = ip4_addr_get_byte(&config->DhcpServerSelf, i);
            data[i + 5] = ip4_addr_get_byte(&config->DhcpServerClient, i);
            data[i + 9] = ip4_addr_get_byte(&config->DhcpServerSubnet, i);
        }

        CDC_TransmitData(data, sizeof(data));
    } break;
    case 0xB2: {
        char data[4 * 3];
        CONFIG *config = Config_GetActive();
        for (int i = 0; i < 4; i++) {
            data[i] = ip4_addr_get_byte(&config->StaticIp, i);
            data[i + 4] = ip4_addr_get_byte(&config->StaticGateway, i);
            data[i + 8] = ip4_addr_get_byte(&config->StaticSubnet, i);
        }

        CDC_TransmitData(data, sizeof(data));
    } break;

    case 0xF0:
        PWR->CR1 |= PWR_CR1_DBP;
        TAMP->BKP0R = 0xF0;
        PWR->CR1 &= ~PWR_CR1_DBP;
    case 0xF1:
        NVIC_SystemReset();
        break;
    case 0xF2:
        Config_Reset();
        Config_ApplyNetwork();
        break;

    case 0xD0:
        if (length == 2 && buffer[1] >= 0 && buffer[1] < 4) {
            char *buffer = USART_GetDmxBuffer(buffer[1]);
            CDC_TransmitData(buffer, 512);
        }
        break;
    }
}

static void CDC_SendID() {
    USB_Transmit(4, UID->ID, sizeof(UID->ID));
}

void CDC_TransmitData(char *data, int len) {
    while (USB_IsTransmitPending(4)) {
    }

    USB_Transmit(4, data, len);
}
