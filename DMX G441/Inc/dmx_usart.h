#ifndef __DMX_USART_H
#define __DMX_USART_H

#include "stm32g441xx.h"

typedef enum {
    USART_DMX_STATE_UNCONFIGURED = 0,
    USART_DMX_STATE_Pause = 1,
    USART_DMX_STATE_DMX = 2
} USART_DMX_State;

typedef enum {
    PORT_FLAG_SINGLE = 0x01,
    PORT_FLAG_RDM = 0x02,
} USART_Port_Flags;

typedef struct {
    USART_TypeDef *Usart;
    GPIO_TypeDef *DRPort;
    GPIO_TypeDef *RxPort;
    DMAMUX_Channel_TypeDef *DmaMux;
    DMA_Channel_TypeDef *Dma;
    char *DmxBuffer;

    char DmaMux_RX;
    char DmaMux_TX;
    USART_DMX_State State;
    USART_Port_Flags Flags;
    char IOType;
    IRQn_Type Irq;

    short DmaIFCR_Offset;
    short DRPin;
    short RxPin;
    char BreakStatus;
} USART_DmxConfig;

#define USART_OUTPUT 0x02
#define USART_INPUT 0x01

// For AHB/2
//#define USART_BRR 287
//#define USART_BRRBREAK 800

#define USART_BRR (287 << 1)
#define USART_BRRBREAK (800 << 1)

void USART_Init();
void USART_InitPortDirections(char *portDirection);

void USART_SetPortState(char port, char enable);
void USART_AlterPortFlags(char port, USART_Port_Flags mask, char value);
void USART_ChangePortDirection(char port, char direction);
void USART_SetBuffer(char port, char *buffer, short length);
void USART_ClearBuffer(char port);
char *USART_GetDmxBuffer(char port);

void USART_BusyCheck();

void USART1_IRQHandler();
void USART2_IRQHandler();
void USART3_IRQHandler();
void UART4_IRQHandler();
#endif
