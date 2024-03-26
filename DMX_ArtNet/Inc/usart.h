#ifndef __USART_H__
#define __USART_H__

#include "config.h"
#include "platform.h"

#define USART_SC_DMX 0x00
#define USART_SC_RDM 0xCC
#define USART_SC_SUB_MESSAGE 0x01

typedef struct USART_PortConfig USART_PortConfig;

typedef enum {
    DMX_Single,
    DMX_InDisabled,
    DMX_ForceSend,
    DMX_NewInput
} DMX_Flags;

typedef struct {
    unsigned char *Buffer;
    DMX_Flags Flags;
} DMX_PortMetadata;

typedef enum {
    USART_PORT_Initialized = 1,

    USART_DIR_Input = (0 << 1),
    USART_DIR_Output = (1 << 1),
    USART_DIR_IOMsk = (1 << 1),

    USART_RxTx_Active = (1 << 2),
    USART_RxTx_Disabled = (0 << 3),
    USART_RxTx_Rx = (1 << 3),
    USART_RxTx_Tx = (2 << 3),
    USART_RxTx_Msk = (3 << 3)
} USART_PortStatus;

typedef enum {
    USART_RxTxState_Idle,
    USART_RxTxState_Break,
    USART_RxTxState_Data,
} USART_RxTxStatus;

typedef struct {
    USART_RxTxStatus Status;
    unsigned char *Buffer;
    unsigned short Length;
    void (*Callback)(USART_PortConfig *);
} USART_RxTxMetadata;

struct USART_PortConfig {
    USART_TypeDef *Usart;
    GPIO_TypeDef *DRPort;
    GPIO_TypeDef *RxPort;
    DMAMUX_Channel_TypeDef *DmaMux;
    DMA_Channel_TypeDef *Dma;

    char DmaMux_RX;
    char DmaMux_TX;
    IRQn_Type Irq;

    short DmaIFCR_Offset;
    short DRPin;
    short RxPin;

    USART_RxTxMetadata RxTxMetadata;
    USART_PortStatus Status;
    DMX_PortMetadata DmxData;

    unsigned char PortID;
};

void USART_Init(CONFIG *config);
void USART_Tick();

void USART_Transmit(USART_PortConfig *port, const unsigned char *buffer, unsigned short length, void (*callback)(USART_PortConfig *));

void USART_ChangeDirection(unsigned char i, unsigned char artnet_direction);
void USART_UpdateInputEnabled(unsigned char i);

void USART1_IRQHandler();
void USART2_IRQHandler();
void USART3_IRQHandler();
void UART4_IRQHandler();

#endif
