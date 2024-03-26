// #ifndef __DMX_USART_H
// #define __DMX_USART_H

// #include "eth/artnet.h"
// #include "config.h"
// #include "platform.h"
// #include "rdm.h"

// typedef enum {
//     USART_DMX_STATE_UNCONFIGURED = 0,
//     USART_DMX_STATE_Pause = 1,
//     USART_DMX_STATE_DMX = 2,
//     USART_DMX_STATE_DMXPending = 3,
//     USART_DMX_STATE_Msk = 3,

//     USART_PORT_STATE_UNCONFIGURED = (0 << 5),
//     USART_PORT_STATE_Tx = (1 << 5),
//     USART_PORT_STATE_Rx = (2 << 5),
//     USART_PORT_STATE_Msk = (3 << 5),

//     USART_STATE_NewInput = (1 << 7),
//     USART_STATE_Break = (1 << 8),

//     USART_MODE_INPUT = (1 << 9),
//     USART_MODE_OUTPUT = (0 << 9),
//     USART_MODE_MSK = (1 << 9),
// } USART_DMX_State;

// typedef struct {
//     USART_TypeDef *Usart;
//     GPIO_TypeDef *DRPort;
//     GPIO_TypeDef *RxPort;
//     DMAMUX_Channel_TypeDef *DmaMux;
//     DMA_Channel_TypeDef *Dma;
//     unsigned char *DmxBuffer;

//     char DmaMux_RX;
//     char DmaMux_TX;
//     USART_DMX_State State;
//     ArtNet_Port_Flags Flags;
//     IRQn_Type Irq;

//     short DmaIFCR_Offset;
//     short DRPin;
//     short RxPin;
// } USART_DmxConfig;

// // For AHB/2
// // #define USART_BRR 287
// // #define USART_BRRBREAK 800

// #define USART_BRR (287 << 1)
// #define USART_BRRBREAK (800 << 1)

// void USART_Init(CONFIG *config);
// void USART_AlterPortFlags(unsigned char port, ArtNet_Port_Flags mask, char value);
// void USART_ChangePortDirection(unsigned char port, char direction);
// void USART_SetBuffer(unsigned char port, const unsigned char *buffer, unsigned short length);
// void USART_ClearBuffer(unsigned char port);
// unsigned char *USART_GetDmxBuffer(unsigned char port);
// unsigned char USART_IsInputNew(unsigned char port);
// void USART_ClearInputNew(unsigned char port);

// void USART_BusyCheck();

// void USART_Tick();

// void USART1_IRQHandler();
// void USART2_IRQHandler();
// void USART3_IRQHandler();
// void UART4_IRQHandler();
// #endif
