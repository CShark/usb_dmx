
#ifndef __USB_H
#define __USB_H

#define USB_SETUP_DIR_Msk 0x80
#define USB_SETUP_TYPE_Msk 0x60
#define USB_SETUP_REC_Msk 0x1F

#define USB_SETUP_DIR_RX 0x00
#define USB_SETUP_DIR_TX 0x80
#define USB_SETUP_TYPE_DEV 0x00
#define USB_SETUP_TYPE_CLS 0x20
#define USB_SETUP_TYPE_VEN 0x40
#define USB_SETUP_REC_DEV 0x00
#define USB_SETUP_REC_INT 0x01
#define USB_SETUP_REC_EP 0x02
#define USB_SETUP_REC_OTHER 0x03

void USB_Init();

void USB_HP_IRQHandler();
void USB_LP_IRQHandler();
void USBWakeUp_IRQHandler();
#endif