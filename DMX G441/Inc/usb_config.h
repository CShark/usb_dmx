#ifndef __USB_CONFIG_H
#define __USB_CONFIG_H

#ifndef __weak
#define __weak __attribute__((weak))
#endif

typedef struct {
    unsigned char Length;
    unsigned char Type;
    unsigned short VersionUSB;
    unsigned char DeviceClass;
    unsigned char DeviceSubClass;
    unsigned char DeviceProtocol;
    unsigned char MaxPacketSize;
    unsigned short IdVendor;
    unsigned short IdProduct;
    unsigned short VersionDevice;
    unsigned char strManufacturer;
    unsigned char strProduct;
    unsigned char strSerialNumber;
    unsigned char Configurations;
} USB_DEVICE_DESCRIPTOR;

typedef struct {
    unsigned char Length;
    unsigned char Type;
    unsigned short TotalLength;
    unsigned char Interfaces;
    unsigned char ConfigValue;
    unsigned char strConfiguration;
    unsigned char Attributes;
    unsigned char MaxPower;
} USB_CONFIG_DESCRIPTOR;

typedef struct {
    unsigned char Length;
    unsigned char Type;
    unsigned char InterfaceId;
    unsigned char AlternateSetting;
    unsigned char Endpoints;
    unsigned char InterfaceClass;
    unsigned char InterfaceSubClass;
    unsigned char InterfaceProtocol;
    unsigned char strInterface;
} USB_INT_DESCRIPTOR;

typedef struct {
    unsigned char Length;
    unsigned char Type;
    unsigned char Address;
    unsigned char Attributes;
    unsigned short MaxPacketSize;
    unsigned char Interval;
} USB_EP_DESCRIPTOR;

USB_DEVICE_DESCRIPTOR *USB_CONF_GetDeviceDescriptor();
USB_CONFIG_DESCRIPTOR *USB_CONF_GetConfigDescriptor();

void USB_CONF_SuspendDevice();
void USB_CONF_WakeUpDevice();

#endif