#include "usb.h"
#include "stm32g441xx.h"
#include "usb_config.h"

#define __USB_MEM __attribute__((section(".usbbuf")))
#define __USBBUF_BEGIN 0x40006000
#define __MEM2USB(X) (((int)X - __USBBUF_BEGIN))

typedef enum {
    USBCT_None = 0,
    USBCT_Setup = 1,
    USBCT_Transmit = 2,
    USBCT_Receive = 3,
    USBCT_Status = 4
} USB_CTRL_TransferState;

typedef enum {
    USBCT_ERROR = USB_EP_TX_STALL,
    USBCT_BUSY = USB_EP_TX_NAK,
    USBCT_ACK = USB_EP_TX_VALID
} USB_CTRL_Response;

typedef struct {
    unsigned short ADDR_TX;
    unsigned short COUNT_TX;
    unsigned short ADDR_RX;
    unsigned short COUNT_RX;
} USB_BTABLE_ENTRY;

typedef struct {
    unsigned char RequestType;
    unsigned char Request;
    union {
        unsigned short Value;
        struct {
            unsigned char DescriptorType;
            unsigned char DescriptorIndex;
        };
    };
    unsigned short Index;
    unsigned short Length;
} USB_SETUP_PACKET;

typedef struct {
    USB_SETUP_PACKET Setup;
    USB_CTRL_TransferState TransferState;
} USB_CTRL_Transfer;

static void USB_HandleCTR(char ep_id);
static void USB_HandleSetup();
static void USB_SetEP(short *ep, short value, short mask);
static void USB_ClearSRAM();
static void USB_SendStatus(USB_CTRL_Response status);

__ALIGNED(8)
__USB_MEM
static USB_BTABLE_ENTRY BTable[8] = {0};

__ALIGNED(2)
__USB_MEM
static char EP0_Buf[2][64] = {0};

static char USB_MsgBuf[512] = {0};
static USB_CTRL_Transfer ControlStatus = {0};
static char ActiveConfiguration = 0;

void USB_Init() {
    // Initialize all Interrupts
    NVIC_SetPriority(USB_HP_IRQn, 0);
    NVIC_SetPriority(USB_LP_IRQn, 0);
    NVIC_SetPriority(USBWakeUp_IRQn, 0);
    NVIC_EnableIRQ(USB_HP_IRQn);
    NVIC_EnableIRQ(USB_LP_IRQn);
    NVIC_EnableIRQ(USBWakeUp_IRQn);

    // Enable USB macrocell
    USB->CNTR &= ~USB_CNTR_PDWN;

    // Wait 1μs until clock is stable
    SysTick->LOAD = 100;
    SysTick->VAL = 0;
    SysTick->CTRL = 1;
    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0) {
    }
    SysTick->CTRL = 0;

    // Enable all interrupts & the internal pullup to put 1.5K on D+ for Fast-Speed USB
    USB->CNTR |= USB_CNTR_RESETM | USB_CNTR_CTRM | USB_CNTR_WKUPM | USB_CNTR_SUSPM | USB_CNTR_ESOFM | USB_CNTR_ERRM | USB_CNTR_L1REQM | USB_CNTR_SOFM;
    USB->BCDR |= USB_BCDR_DPPU;

    // Clear the USB Reset (D+ & D- low) to start enumeration
    USB->CNTR &= ~USB_CNTR_FRES;
}

void USB_HP_IRQHandler() {
}

void USB_LP_IRQHandler() {
    if ((USB->ISTR & USB_ISTR_RESET) != 0) {
        // On Reset, reininitalize the USB-SRAM and Control Endpoint
        USB->ISTR = ~USB_ISTR_RESET;

        USB_ClearSRAM();

        USB->BTABLE = __MEM2USB(BTable);

        BTable[0].ADDR_TX = __MEM2USB(EP0_Buf[1]);
        BTable[0].COUNT_TX = 0;
        BTable[0].ADDR_RX = __MEM2USB(EP0_Buf[0]);
        BTable[0].COUNT_RX = (1 << 15) | (1 << 10);

        // We expect to receive a setup, discard any transmit request
        USB_SetEP(&USB->EP0R, USB_EP_CONTROL | USB_EP_RX_VALID | USB_EP_TX_NAK, USB_EP_TYPE_MASK | USB_EP_RX_VALID | USB_EP_TX_VALID);
        // Turn on all the interrupts
        USB->CNTR = USB_CNTR_CTRM | USB_CNTR_RESETM | USB_CNTR_ERRM | USB_CNTR_WKUPM | USB_CNTR_SUSPM | USB_CNTR_SOFM | USB_CNTR_ESOFM | USB_CNTR_L1REQM;

        // Enable the USB peripheral and set the Address to 0
        USB->DADDR = USB_DADDR_EF;
    } else if ((USB->ISTR & USB_ISTR_CTR) != 0) {
        // A succesfull transfer occured, ISTR does not need to be reset
        USB_HandleCTR(USB->ISTR & USB_ISTR_EP_ID);
    } else if ((USB->ISTR & USB_ISTR_WKUP) != 0) {
        // The Wakeup-Request, return to normal operation.
        // Re-Enable everything if necessary.
        USB->CNTR &= ~USB_CNTR_FSUSP;
        USB->CNTR &= ~USB_CNTR_LPMODE;
        USB->ISTR = ~USB_ISTR_WKUP;
        USB_CONF_WakeUpDevice();
    } else if ((USB->ISTR & USB_ISTR_SUSP) != 0) {
        // The suspend request requires the device to go into suspend mode
        // If the device is bus powered, also activate LowPower Mode to further reduce power consumption
        USB->CNTR |= USB_CNTR_FSUSP;
        USB->ISTR = ~USB_ISTR_SUSP;
        USB->CNTR |= USB_CNTR_LPMODE;
        USB_CONF_SuspendDevice();
    } else if ((USB->ISTR & USB_ISTR_ESOF) != 0) {
        USB->ISTR = ~USB_ISTR_ESOF;
    } else if ((USB->ISTR & USB_ISTR_SOF) != 0) {
        USB->ISTR = ~USB_ISTR_SOF;
    } else if ((USB->ISTR & USB_ISTR_ERR) != 0) {
        USB->ISTR = ~USB_ISTR_ERR;
    } else if ((USB->ISTR & USB_ISTR_L1REQ) != 0) {
        USB->ISTR = ~USB_ISTR_L1REQ;
    }
}

void USBWakeUp_IRQHandler() {
}

/// @brief Clear the SRAM to have a better view in the memory tool
static void USB_ClearSRAM() {
    char *buffer = __USBBUF_BEGIN;

    for (int i = 0; i < 1024; i++) {
        buffer[i] = 0;
    }
}

/// @brief Set the endpoint register to a specified value. Necessary because of stupid toggle bits
/// @param ep Pointer to the Endpoint register EPnR
/// @param value The values to write
/// @param mask The bits which should be updated
static void USB_SetEP(short *ep, short value, short mask) {
    short toggle = 0b0111000001110000;
    short rc_w0 = 0b1000000010000000;
    short rw = 0b0000011100001111;

    short wr0 = rc_w0 & (~mask | value);
    short wr1 = (mask & toggle) & (*ep ^ value);
    short wr2 = rw & ((*ep & ~mask) | value);

    *ep = wr0 | wr1 | wr2;
}

static void USB_HandleCTR(char ep_id) {
    if (ep_id < 8) {
        short *EPnR = (&USB->EP0R) + 0x04 * ep_id;
        USB_BTABLE_ENTRY *btable = &BTable[ep_id];

        if (*EPnR & USB_EP_CTR_RX) {
            char *buffer = btable->ADDR_RX + __USBBUF_BEGIN;
            short length = btable->COUNT_RX & 0x1FF;

            if (ep_id == 0) {
                if (*EPnR & USB_EP_SETUP) {
                    ControlStatus.TransferState = USBCT_Setup;
                    ControlStatus.Setup = *((USB_SETUP_PACKET *)EP0_Buf[0]);
                    USB_HandleSetup();
                } else {
                }
            } else {
                for (int i = 0; i < length; i++) {
                    USB_MsgBuf[i] = buffer[i];
                }
            }

            USB_SetEP(EPnR, USB_EP_RX_VALID, USB_EP_CTR_RX | USB_EP_RX_VALID);
        }

        if (*EPnR & USB_EP_CTR_TX) {
            // If Controlendpoint & communication finished, check if it was to set the address.
            if (ep_id == 0) {
                if (ControlStatus.TransferState == USBCT_Status) {
                    if (ControlStatus.Setup.Request == 0x05) {
                        USB->DADDR = USB_DADDR_EF | ControlStatus.Setup.Value;
                    }
                }
            }

            USB_SetEP(EPnR, 0x00, USB_EP_CTR_TX);
        }
    }
}

/// @brief Handle USB Setup requests
static void USB_HandleSetup() {
    if ((ControlStatus.Setup.RequestType & USB_SETUP_TYPE_DEV) != 0) { // Device requests
        switch (ControlStatus.Setup.Request) {
        case 0x00: { // Get Status
        }
        case 0x01: { // Clear Feature
        }
        case 0x03: { // Set Feature
        }
        case 0x05: { // Set Address
            USB_SendStatus(USBCT_ACK);
        }
        case 0x06: { // Get Device Descriptor
            short *descriptor = USB_CONF_GetDeviceDescriptor();
            short length = sizeof(USB_CONFIG_DESCRIPTOR);

            short *mem = BTable[0].ADDR_TX + __USBBUF_BEGIN;
            for (int i = 0; i < length / sizeof(short); i++) {
                mem[i] = descriptor[i];
            }

            BTable[0].COUNT_TX = length;
            ControlStatus.TransferState = USBCT_Transmit;
            USB_SetEP(&(USB->EP0R), USB_EP_TX_VALID, USB_EP_TX_VALID);
            break;
        }
        case 0x07: { // Set Device Descriptor
        }
        case 0x08: { // Get Configuration
            ControlStatus.TransferState = USBCT_Transmit;
        }
        case 0x09: { // Set Configuration
            if (ControlStatus.Setup.Value == 0 ||
                ControlStatus.Setup.Value == 1) {
                ActiveConfiguration = ControlStatus.Setup.Value;
                USB_SendStatus(USBCT_ACK);
            } else {
                USB_SendStatus(USBCT_ERROR);
            }
        }
        }
    }
}

static void USB_SendStatus(USB_CTRL_Response status) {
    ControlStatus.TransferState = USBCT_Status;
    BTable[0].COUNT_TX = 0;
    USB_SetEP(&(USB->EP0R), status, USB_EP_TX_VALID);
}

static void USB_Transmit(char ep_idx, char* buffer, short lenght) {
    
}