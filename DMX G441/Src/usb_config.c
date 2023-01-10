#include "usb_config.h"
#include "cdc_device.h"
#include "ncm_device.h"
#include "stm32g441xx.h"

// Example definition for a NCM-Device
static const USB_DESCRIPTOR_DEVICE DeviceDescriptor = {
    .Length = 18,
    .Type = 0x01,
    .USBVersion = 0x0200,
    .DeviceClass = 0xEF,
    .DeviceSubClass = 0x02,
    .DeviceProtocol = 0x01,
    .MaxPacketSize = 64,
    .VendorID = 0x16C0,
    .ProductID = 0x088B,
    .DeviceVersion = 0x0100,
    .strManufacturer = 1,
    .strProduct = 2,
    .strSerialNumber = 3,
    .Configurations = 1};

static const USB_DESCRIPTOR_CONFIG ConfigDescriptor = {
    .Length = 9,
    .Type = 0x02,
    .TotalLength = 155,
    .Interfaces = 4,
    .ConfigurationID = 1,
    .strConfiguration = 0,
    .Attributes = (1 << 7),
    .MaxPower = 50};

static const USB_FUNC_IAD NCM_IAD = {
    .Length = 8,
    .DescriptorType = 0x0B,
    .FirstInterface = 0,
    .InterfaceCount = 2,
    .Class = 0x02,
    .SubClass = 0x0D,
    .Protocol = 0x00,
    .strFunction = 0};

static const USB_DESCRIPTOR_INTERFACE NCMInterface = {
    .Length = 9,
    .Type = 0x04,
    .InterfaceID = 0,
    .AlternateID = 0,
    .Endpoints = 1,
    .Class = 0x02,
    .SubClass = 0x0D,
    .Protocol = 0x00,
    .strInterface = 4};

static const USB_DESC_FUNC_HEADER NCMFuncHeader = {
    .Length = 5,
    .Type = CS_INTERFACE,
    .SubType = FUNC_HEADER,
    .CDCVersion = 0x0110};

static const USB_DESC_FUNC_UNION1 NCMFuncUnion = {
    .Length = 5,
    .Type = CS_INTERFACE,
    .SubType = FUNC_UNION,
    .ControlInterface = 0,
    .SubInterface0 = 1};

static const USB_DESC_FUNC_ECM NCMFuncETH = {
    .Length = 13,
    .Type = CS_INTERFACE,
    .SubType = FUNC_ECM,
    .MaxSegmentSize = 1514,
    .strMacAddress = 20};

static const USB_DESC_FUNC_NCM NCMFuncNCM = {
    .Length = 6,
    .Type = CS_INTERFACE,
    .SubType = FUNC_NCM,
    .NcmVersion = 0x0100,
    .NetworkCapabilities = 0b10000};

static const USB_DESCRIPTOR_INTERFACE DataInterfaces[2] = {
    {
        .Length = 9,
        .Type = 0x04,
        .InterfaceID = 1,
        .AlternateID = 0,
        .Endpoints = 0,
        .Class = 0x0A,
        .SubClass = 0x00,
        .Protocol = 0x01,
        .strInterface = 0,
    },
    {.Length = 9,
     .Type = 0x04,
     .InterfaceID = 1,
     .AlternateID = 1,
     .Endpoints = 2,
     .Class = 0x0A,
     .SubClass = 0x00,
     .Protocol = 0x01,
     .strInterface = 0}};

static const USB_DESCRIPTOR_ENDPOINT NCM_Endpoints[3] = {
    {.Length = 7,
     .Type = 0x05,
     .Address = (1 << 7) | 1,
     .Attributes = 0x03,
     .MaxPacketSize = 16,
     .Interval = 50},
    {.Length = 7,
     .Type = 0x05,
     .Address = 2,
     .Attributes = 0x02,
     .MaxPacketSize = 64,
     .Interval = 1},
    {.Length = 7,
     .Type = 0x05,
     .Address = (1 << 7) | 2,
     .Attributes = 0x02,
     .MaxPacketSize = 64,
     .Interval = 0}};

static const USB_FUNC_IAD CDC_IAD = {
    .Length = 8,
    .DescriptorType = 0x0B,
    .FirstInterface = 2,
    .InterfaceCount = 2,
    .Class = 0x02,
    .SubClass = 0x02,
    .Protocol = 0x01,
    .strFunction = 0};

static const USB_DESCRIPTOR_INTERFACE CDCInterfaces[2] = {
    {.Length = 9,
     .Type = 0x04,
     .InterfaceID = 2,
     .AlternateID = 0,
     .Endpoints = 1,
     .Class = 0x02,
     .SubClass = 0x02,
     .Protocol = 0x01,
     .strInterface = 0},
    {.Length = 9,
     .Type = 0x04,
     .InterfaceID = 3,
     .AlternateID = 0,
     .Endpoints = 2,
     .Class = 0x0A,
     .SubClass = 0x00,
     .Protocol = 0x00,
     .strInterface = 0}};

static const USB_DESC_FUNC_HEADER CDCFuncHeader = {
    .Length = 5,
    .Type = 0x24,
    .SubType = 0x00,
    .CDCVersion = 0x0110};

static const USB_DESC_FUNC_ACM CDCFuncACM = {
    .Length = 4,
    .Type = 0x24,
    .SubType = 0x02,
    .Capabilities = (1 << 1)};

static const USB_DESC_FUNC_UNION1 CDCFuncUnion = {
    .Length = 5,
    .Type = 0x24,
    .SubType = 0x06,
    .ControlInterface = 2,
    .SubInterface0 = 3};

static const USB_DESCRIPTOR_ENDPOINT CDCEndpoints[3] = {
    {.Length = 7,
     .Type = 0x05,
     .Address = (1 << 7) | 3,
     .Attributes = 0x03,
     .MaxPacketSize = 8,
     .Interval = 50},
    {.Length = 7,
     .Type = 0x05,
     .Address = 4,
     .Attributes = 0x02,
     .MaxPacketSize = 64,
     .Interval = 0},
    {.Length = 7,
     .Type = 0x05,
     .Address = (1 << 7) | 4,
     .Attributes = 0x02,
     .MaxPacketSize = 64,
     .Interval = 0}};

// Buffer holding the complete descriptor (except the device one) in the correct order
static char ConfigurationBuffer[155] = {0};

/// @brief A Helper to add a descriptor to the configuration buffer
/// @param data The raw descriptor data
/// @param offset The offset in the configuration buffer
static void AddToDescriptor(char *data, short *offset);

USB_DESCRIPTOR_DEVICE *USB_GetDeviceDescriptor() {
    return &DeviceDescriptor;
}

static void AddToDescriptor(char *data, short *offset) {
    short length = data[0];

    for (int i = 0; i < length; i++) {
        ConfigurationBuffer[i + *offset] = data[i];
    }

    *offset += length;
}

char *USB_GetConfigDescriptor(short *length) {
    if (ConfigurationBuffer[0] == 0) {
        short offset = 0;
        AddToDescriptor(&ConfigDescriptor, &offset);

        AddToDescriptor(&NCM_IAD, &offset);
        AddToDescriptor(&NCMInterface, &offset);
        AddToDescriptor(&NCMFuncHeader, &offset);
        AddToDescriptor(&NCMFuncUnion, &offset);
        AddToDescriptor(&NCMFuncETH, &offset);
        AddToDescriptor(&NCMFuncNCM, &offset);
        AddToDescriptor(&NCM_Endpoints[0], &offset);
        AddToDescriptor(&DataInterfaces[0], &offset);
        AddToDescriptor(&DataInterfaces[1], &offset);
        AddToDescriptor(&NCM_Endpoints[1], &offset);
        AddToDescriptor(&NCM_Endpoints[2], &offset);

        AddToDescriptor(&CDC_IAD, &offset);
        AddToDescriptor(&CDCInterfaces[0], &offset);
        AddToDescriptor(&CDCFuncHeader, &offset);
        AddToDescriptor(&CDCFuncACM, &offset);
        AddToDescriptor(&CDCFuncUnion, &offset);
        AddToDescriptor(&CDCEndpoints[0], &offset);
        AddToDescriptor(&CDCInterfaces[1], &offset);
        AddToDescriptor(&CDCEndpoints[1], &offset);
        AddToDescriptor(&CDCEndpoints[2], &offset);
    }

    *length = sizeof(ConfigurationBuffer);
    return ConfigurationBuffer;
}

char *USB_GetString(char index, short lcid, short *length) {
    // Strings need to be in unicode (thus prefixed with u"...")
    // The length is double the character count + 2 — or use VSCode which will show the number of bytes on hover
    if (index == 1) {
        *length = 20;
        return u"Housemade";
    } else if (index == 2) {
        *length = 24;
        return u"ArtNet-Node";
    } else if (index == 3) {
        *length = 22;
        return u"01234-6786";
    } else if (index == 4) {
        *length = 28;
        return u"DMX Interface";
    } else if (index == 20) {
        *length = 26;
        return u"445BBD24A371";
    }

    return 0;
}

char *USB_GetOSDescriptor(short *length) {
    return 0;
}

void USB_ConfigureEndpoints() {
    // Configure all endpoints and route their reception to the functions that need them
    USB_CONFIG_EP NCMIntEP = {
        .EP = 1,
        .RxBufferSize = 0,
        .TxBufferSize = 16,
        .TxCallback = NCM_ControlTransmit,
        .Type = USB_EP_INTERRUPT};

    USB_SetEPConfig(NCMIntEP);

    USB_CONFIG_EP NCMDataEp = {
        .EP = 2,
        .RxBufferSize = 64,
        .TxBufferSize = 64,
        .RxCallback = NCM_HandlePacket,
        .TxCallback = NCM_BufferTransmitted,
        .Type = USB_EP_BULK};

    USB_SetEPConfig(NCMDataEp);

    USB_CONFIG_EP CDCIntEp = {
        .EP = 3,
        .RxBufferSize = 0,
        .TxBufferSize = 8,
        .Type = USB_EP_INTERRUPT};

    USB_SetEPConfig(CDCIntEp);

    USB_CONFIG_EP CDCDataEp = {
        .EP = 4,
        .RxBufferSize = 64,
        .TxBufferSize = 64,
        .RxCallback = CDC_HandlePacket,
        .Type = USB_EP_BULK};

    USB_SetEPConfig(CDCDataEp);
}

char USB_HandleClassSetup(USB_SETUP_PACKET *setup, char *data, short length) {
    // Route the setup packets based on the Interface / Class Index
    if (setup->Index == 0) {
        NCM_SetupPacket(setup, data, length);
    } else if (setup->Index == 2) {
        CDC_SetupPacket(setup, data, length);
    }
}

void USB_ResetClass(char interface, char alternateId) {
    if (interface == 1) {
        NCM_Reset(interface, alternateId);
    }
}

__weak void USB_SuspendDevice() {
}

__weak void USB_WakeupDevice() {
}
