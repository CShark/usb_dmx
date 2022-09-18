#ifndef __DMX_USART_H
#define __DMX_USART_H

#include "stm32g441xx.h"

typedef enum {
    USART_DMX_STATE_UNCONFIGURED = 0,
    USART_DMX_STATE_Pause = 1,
    USART_DMX_STATE_DMX = 2    
} USART_DMX_State;

typedef struct {
    USART_TypeDef* Usart;
    GPIO_TypeDef* DRPort;
    DMAMUX_Channel_TypeDef* DmaMux;
    DMA_Channel_TypeDef* Dma;
    char* DmxBuffer;

    char DmaMux_RX;
    char DmaMux_TX;
    USART_DMX_State State;
    char IOType;
    IRQn_Type Irq;
    
    short DmaIFCR_Offset;
    short DRPin;
} USART_DmxConfig;


#define USART_OUTPUT 0x02
#define USART_INPUT 0x04

void USART_Init(char *portDirection);

void USART_SetPortState(char port, char enable);
void USART_SetBufferPage(char port, char page, char* buffer);

void USART1_IRQHandler();
void USART2_IRQHandler();
void USART3_IRQHandler();
void UART4_IRQHandler();
#endif
