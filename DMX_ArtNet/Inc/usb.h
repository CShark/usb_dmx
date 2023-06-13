#ifndef __USB_H
#define __USB_H

typedef struct {
    unsigned char RequestType;
    unsigned char Request;
    union {
        unsigned short Value;
        struct {
            unsigned char DescriptorIndex;
            unsigned char DescriptorType;
        };
    };
    unsigned short Index;
    unsigned short Length;
} USB_SETUP_PACKET;

typedef struct {
    unsigned char EP;
    unsigned char RxBufferSize;
    unsigned char TxBufferSize;
    unsigned short Type;
    void (*RxCallback)(unsigned char ep, short length);
    void (*TxCallback)(unsigned char ep, short length);
} USB_CONFIG_EP;

#define USB_OK 0
#define USB_BUSY 1
#define USB_ERR 2

// Disable this define to disable the timeout feature
#define USB_TXTIMEOUT 50

/// @brief Initialize all USB related stuff
void USB_Init();
/// @brief Low Priority handler for most interrupts
void USB_LP_IRQHandler();
/// @brief High Priority handler for Burst and Isochronous transfers
void USB_HP_IRQHandler();

/// @brief Transmit some data of arbitrary length
/// @param ep The endpoint id
/// @param buffer A pointer to the buffer containing the data
/// @param length The number of bytes to sent
/// @remark Will automatically split the transmission into multiple chunks if necessary
void USB_Transmit(unsigned char ep, const unsigned char* buffer, short length);
/// @brief Whether there is currently any unfinished transfer running
/// @param ep The endpoint to check
/// @remark Do not busy-wait on this during reception. It will stall the USB-ISR
char USB_IsTransmitPending(unsigned char ep);
/// @brief Get data out of the reception buffers
/// @param ep The endpoint id to fetch data from
/// @param buffer The target buffer to write to
/// @param length The length of the buffer. Will contain the number of bytes read
void USB_Fetch(unsigned char ep, unsigned char* buffer, short *length);
/// @brief Configure an endpoint
void USB_SetEPConfig(USB_CONFIG_EP config);

#endif