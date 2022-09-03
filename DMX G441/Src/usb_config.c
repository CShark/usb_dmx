#include "usb_config.h"

#define USB_NUM_INTERFACES 1

static USB_DEVICE_DESCRIPTOR deviceDescriptor = {
    .Length = 18,
    .Type = 0x01,
    .VersionUSB = 0x0200,
    .DeviceClass = 0xFF,
    .DeviceSubClass = 0xFF,
    .DeviceProtocol = 0xFF,
    .MaxPacketSize = 64,
    .IdVendor = 0xDEAD,
    .IdProduct = 0xBEEF,
    .VersionDevice = 0x0001,
    .strManufacturer = 0,
    .strProduct = 0,
    .strSerialNumber = 0,
    .Configurations = 1};

static USB_CONFIG_DESCRIPTOR configDescriptor = {
    .Length = 9,
    .Type = 0x02,
    .Interfaces = 1,
    .ConfigValue = 1,
    .strConfiguration = 0,
    .Attributes = 0x80,
    .MaxPower = 50,
    .TotalLength = 9};

static USB_INT_DESCRIPTOR interfaceDescriptors[USB_NUM_INTERFACES] = {
    {.Length = 9,
     .Type = 0x04,
     .InterfaceId = 1,
     .AlternateSetting = 0,
     .Endpoints = 1}};

USB_DEVICE_DESCRIPTOR *USB_CONF_GetDeviceDescriptor() {
    return &deviceDescriptor;
}

USB_CONFIG_DESCRIPTOR *USB_CONF_GetConfigDescriptor() {
    return &configDescriptor;
}

__weak void USB_CONF_SuspendDevice() {}
__weak void USB_CONF_WakeUpDevice() {}