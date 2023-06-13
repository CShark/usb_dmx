#ifndef __DMX_USART_H
#define __DMX_USART_H

#include "platform.h"
#include "eth/artnet.h"

typedef enum {
    USART_DMX_STATE_UNCONFIGURED = 0,
    USART_DMX_STATE_Pause = 1,
    USART_DMX_STATE_DMX = 2
} USART_DMX_State;

typedef struct {
    USART_TypeDef *Usart;
    GPIO_TypeDef *DRPort;
    GPIO_TypeDef *RxPort;
    DMAMUX_Channel_TypeDef *DmaMux;
    DMA_Channel_TypeDef *Dma;
    unsigned char *DmxBuffer;

    char DmaMux_RX;
    char DmaMux_TX;
    USART_DMX_State State;
    ArtNet_Port_Flags Flags;
    char IOType;
    IRQn_Type Irq;

    short DmaIFCR_Offset;
    short DRPin;
    short RxPin;
    char BreakStatus;

    char NewInput;
} USART_DmxConfig;

#define USART_OUTPUT 0x02
#define USART_INPUT 0x01

// For AHB/2
//#define USART_BRR 287
//#define USART_BRRBREAK 800

#define USART_BRR (287 << 1)
#define USART_BRRBREAK (800 << 1)

void USART_Init();
void USART_InitPortDirections(const unsigned char *portDirection);

void USART_SetPortState(unsigned char port, char enable);
void USART_AlterPortFlags(unsigned char port, ArtNet_Port_Flags mask, char value);
void USART_ChangePortDirection(unsigned char port, char direction);
void USART_SetBuffer(unsigned char port, const unsigned char *buffer, unsigned short length);
void USART_ClearBuffer(unsigned char port);
unsigned char *USART_GetDmxBuffer(unsigned char port);
unsigned char USART_IsInputNew(unsigned char port);
void USART_ClearInputNew(unsigned char port);

void USART_BusyCheck();

void USART1_IRQHandler();
void USART2_IRQHandler();
void USART3_IRQHandler();
void UART4_IRQHandler();
#endif
