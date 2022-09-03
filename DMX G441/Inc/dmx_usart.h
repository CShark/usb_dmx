#ifndef __DMX_USART_H
#define __DMX_USART_H

#include "stm32g441xx.h"

typedef enum {
    USART_DMX_STATE_Pause = 0,
    USART_DMX_STATE_DMX = 1
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
    IRQn_Type Irq;
    
    short DmaIFCR_Offset;
    short DRPin;
} USART_DmxConfig;


void USART_Init();

void USART1_IRQHandler();
void USART2_IRQHandler();
void USART3_IRQHandler();
void UART4_IRQHandler();
#endif
