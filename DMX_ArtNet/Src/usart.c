#include "usart.h"
#include "dmx.h"

#define USART_BRR (287 << 1)
#define USART_BRRBREAK (800 << 1)

USART_PortConfig usart_config[] = {
    {.Usart = USART1,
     .Irq = USART1_IRQn,
     .DRPort = GPIOB,
     .DRPin = 5,
     .RxPort = GPIOB,
     .RxPin = 7,

     .DmaMux = DMAMUX1_Channel0,
     .DmaMux_RX = 24,
     .DmaMux_TX = 25,
     .Dma = DMA1_Channel1,
     .DmaIFCR_Offset = DMA_IFCR_CGIF3_Pos,

     .PortID = 0},

    {.Usart = USART2,
     .Irq = USART2_IRQn,
     .DRPort = GPIOC,
     .DRPin = 12,
     .RxPort = GPIOB,
     .RxPin = 4,

     .DmaMux = DMAMUX1_Channel1,
     .DmaMux_RX = 26,
     .DmaMux_TX = 27,
     .Dma = DMA1_Channel2,
     .DmaIFCR_Offset = DMA_IFCR_CGIF2_Pos,

     .PortID = 1},

    {.Usart = UART4,
     .Irq = UART4_IRQn,
     .DRPort = GPIOA,
     .DRPin = 15,
     .RxPort = GPIOC,
     .RxPin = 11,

     .DmaMux = DMAMUX1_Channel2,
     .DmaMux_RX = 30,
     .DmaMux_TX = 31,
     .Dma = DMA1_Channel3,
     .DmaIFCR_Offset = DMA_IFCR_CGIF3_Pos,

     .PortID = 2},

    {.Usart = USART3,
     .Irq = USART3_IRQn,
     .DRPort = GPIOB,
     .DRPin = 12,
     .RxPort = GPIOB,
     .RxPin = 11,

     .DmaMux = DMAMUX1_Channel3,
     .DmaMux_RX = 28,
     .DmaMux_TX = 29,
     .Dma = DMA1_Channel4,
     .DmaIFCR_Offset = DMA_IFCR_CGIF4_Pos,

     .PortID = 3}};

static void USART_HandleIRQ(USART_PortConfig *port);
static void USART_DisableDma(USART_PortConfig *port);
static void USART_Disable(USART_PortConfig *port);
static void USART_SetTx(USART_PortConfig *port);
static void USART_TxDma(USART_PortConfig *port, const unsigned char *buffer, unsigned short length);
static void USART_SetRx(USART_PortConfig *port);
static void USART_RxDma(USART_PortConfig *port, unsigned char *buffer, unsigned short length);
static void USART_SendBreak(USART_PortConfig *port);

void USART_Init(CONFIG *config) {
    for (int i = 0; i < 4; i++) {
        NVIC_SetPriority(usart_config[i].Irq, 1);
        NVIC_EnableIRQ(usart_config[i].Irq);

        if (config->ArtNet[i].PortDirection == ARTNET_OUTPUT) {
            usart_config[i].Status |= USART_DIR_Output;
        }
    }

    DMX_Init(usart_config, config);
}

void USART_Tick() {
    for (int i = 0; i < 4; i++) {
        if ((usart_config[i].Status & USART_RxTx_Active) == 0) {
            DMX_Tick(&usart_config[i]);
        }
    }
}

void USART_ChangeDirection(unsigned char i, unsigned char artnet_direction) {
    if (i >= 0 && i < 4) {
        if ((artnet_direction == ARTNET_OUTPUT && (usart_config[i].Status & USART_DIR_IOMsk) != USART_DIR_Output) ||
            (artnet_direction == ARTNET_INPUT && (usart_config[i].Status & USART_DIR_IOMsk) != USART_DIR_Input)) {

            USART_Disable(&usart_config[i]);
            usart_config[i].Status &= ~USART_DIR_IOMsk;

            if (artnet_direction == ARTNET_OUTPUT) {
                usart_config[i].Status |= USART_DIR_Output;
                USART_SetTx(&usart_config[i]);
            } else {
                usart_config[i].Status |= USART_DIR_Input;
                USART_UpdateInputEnabled(i);
            }
        }
    }
}

void USART_UpdateInputEnabled(unsigned char i) {
    if (i >= 0 && i < 4) {
        if ((usart_config[i].Status & USART_DIR_IOMsk) == USART_DIR_Input) {
            if (DMX_IsInputEnabled(&usart_config[i].DmxData)) {
                USART_SetRx(&usart_config[i]);
            } else {
                USART_Disable(&usart_config[i]);
            }
        }
    }
}

void USART_Transmit(USART_PortConfig *port, const unsigned char *buffer, unsigned short length, void (*callback)(USART_PortConfig *)) {
    USART_SetTx(port);
    port->RxTxMetadata.Buffer = buffer;
    port->RxTxMetadata.Length = length;
    port->RxTxMetadata.Status = USART_RxTxState_Break;
    port->RxTxMetadata.Callback = callback;
    port->Status |= USART_RxTx_Active;

    USART_SendBreak(port);
}

void USART_Receive(USART_PortConfig *port, unsigned char *buffer, unsigned short length) {
}

static void USART_HandleIRQ(USART_PortConfig *port) {
    port->Usart->CR1 &= ~USART_CR1_TCIE;
    port->Usart->ICR = USART_ICR_TCCF;

    if ((port->Status & USART_RxTx_Msk) == USART_RxTx_Tx) {
        if (port->RxTxMetadata.Status == USART_RxTxState_Break) {
            port->Usart->CR1 &= ~(USART_CR1_UE | USART_CR1_TE);
            port->Usart->BRR = USART_BRR;
            port->Usart->CR1 |= USART_CR1_UE;

            // Send Data
            port->RxTxMetadata.Status = USART_RxTxState_Data;
            USART_TxDma(port, port->RxTxMetadata.Buffer, port->RxTxMetadata.Length);
        } else if (port->RxTxMetadata.Status == USART_RxTxState_Data) {
            // Callback
            port->Status &= ~USART_RxTx_Active;
            port->RxTxMetadata.Status = USART_RxTxState_Idle;
            port->RxTxMetadata.Buffer = NULL;
            port->RxTxMetadata.Length = 0;
        }
    } else {
        if (port->Usart->ISR & USART_ISR_FE) {
            port->Usart->ICR = USART_ICR_FECF;

            if (port->RxTxMetadata.Status == USART_RxTxState_Data) {
                if (port->RxTxMetadata.Callback != NULL) {
                    port->RxTxMetadata.Callback(port);
                    port->RxTxMetadata.Callback = NULL;
                }
            }

            // Break detected, start reception
            USART_DisableDma(port);
            port->RxTxMetadata.Status = USART_RxTxState_Break;
        } else {
            port->Usart->RQR = USART_RQR_RXFRQ;
            if (port->RxTxMetadata.Status == USART_RxTxState_Break) {
                // First byte after break
                volatile char data = port->Usart->RDR;

                if (data == USART_SC_DMX) {
                    // DMX Frame, accept only if we are in input mode
                    if ((port->Status & USART_DIR_IOMsk) == USART_DIR_Input) {
                        port->RxTxMetadata.Buffer = port->DmxData.Buffer;
                        port->RxTxMetadata.Length = 513;
                        port->RxTxMetadata.Status = USART_RxTxState_Data;
                        port->RxTxMetadata.Callback = DMX_InputCallback;

                        USART_RxDma(port, port->RxTxMetadata.Buffer, port->RxTxMetadata.Length);
                    }
                } else if (data == USART_SC_RDM) {
                }
            } else if (port->RxTxMetadata.Status == USART_RxTxState_Data) {
                if (DMA1->ISR & (DMA_ISR_TCIF1 << port->DmaIFCR_Offset)) {
                    DMA1->IFCR = DMA_IFCR_CTCIF1 << port->DmaIFCR_Offset;
                    port->RxTxMetadata.Status = USART_RxTxState_Idle;

                    if (port->RxTxMetadata.Callback != NULL) {
                        port->RxTxMetadata.Callback(port);
                        port->RxTxMetadata.Callback = NULL;
                    }
                }
            }
        }
    }

    port->Usart->ICR = 0xFF;
}

static void USART_DisableDma(USART_PortConfig *port) {
    port->Usart->CR3 &= ~(USART_CR3_DMAR | USART_CR3_DMAT);
    port->Dma->CCR &= ~DMA_CCR_EN;
}

static void USART_Disable(USART_PortConfig *port) {
    USART_DisableDma(port);
    port->Usart->CR1 &= ~(USART_CR1_TCIE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE);
    port->Usart->ICR = 0xFF;
    port->Usart->CR1 &= ~USART_CR1_UE;
    DMA1->IFCR = (1 << port->DmaIFCR_Offset);

    port->Status &= ~USART_RxTx_Msk;
    port->Status |= USART_RxTx_Disabled;
}

static void USART_SetTx(USART_PortConfig *port) {
    if ((port->Status & USART_RxTx_Msk) != USART_RxTx_Tx) {
        USART_Disable(port);

        // Set default speed
        port->Usart->BRR = USART_BRR;
        port->Usart->CR2 = (0x02 << USART_CR2_STOP_Pos);
        port->Usart->CR3 = USART_CR3_DDRE | USART_CR3_OVRDIS;
        port->Usart->CR1 |= USART_CR1_UE;

        // Configure driver & DMA
        port->DRPort->BSRR = 1 << port->DRPin;
        port->Dma->CPAR = &(port->Usart->TDR);
        port->Dma->CCR = DMA_CCR_MINC | DMA_CCR_DIR;
        port->DmaMux->CCR = port->DmaMux_TX;

        // Set port state
        port->Status &= ~USART_RxTx_Msk;
        port->Status |= USART_RxTx_Tx;
    }
}

static void USART_SetRx(USART_PortConfig *port) {
    if ((port->Status & USART_RxTx_Msk) != USART_RxTx_Rx) {
        USART_Disable(port);

        // Set default speed
        port->Usart->BRR = USART_BRR;
        port->Usart->CR2 = (0x02 << USART_CR2_STOP_Pos);
        port->Usart->CR3 = USART_CR3_DDRE | USART_CR3_EIE | USART_CR3_OVRDIS;
        port->Usart->CR1 |= USART_CR1_UE;

        // Configure Driver & DMA
        port->DRPort->BSRR = (1 << port->DRPin) << 16;
        port->Dma->CPAR = &(port->Usart->RDR);
        port->Dma->CCR = DMA_CCR_MINC | DMA_CCR_TCIE;
        port->DmaMux->CCR = port->DmaMux_RX;

        // Set Port state
        port->Status &= ~USART_RxTx_Msk;
        port->Status |= USART_RxTx_Rx;

        port->Usart->CR1 |= USART_CR1_RE | USART_CR1_RXNEIE;
    }
}

static void USART_TxDma(USART_PortConfig *port, const unsigned char *buffer, unsigned short length) {
    port->Dma->CMAR = buffer;
    port->Dma->CNDTR = length;

    port->Usart->CR1 |= USART_CR1_TE;

    port->Usart->ICR = USART_ICR_TCCF;
    port->Usart->CR3 |= USART_CR3_DMAT;
    port->Dma->CCR |= DMA_CCR_EN;

    port->Usart->CR1 |= USART_CR1_TCIE;
}

static void USART_RxDma(USART_PortConfig *port, unsigned char *buffer, unsigned short length) {
    port->Dma->CMAR = buffer;
    port->Dma->CNDTR = length;

    port->Usart->CR3 |= USART_CR3_DMAR;
    port->Dma->CCR |= DMA_CCR_EN;
}

static void USART_SendBreak(USART_PortConfig *port) {
    port->Usart->CR3 &= ~USART_CR3_DMAT;
    port->Dma->CCR &= ~DMA_CCR_EN;

    port->Usart->CR1 &= ~(USART_CR1_UE | USART_CR1_TE);
    port->Usart->BRR = USART_BRRBREAK;
    port->Usart->CR1 |= USART_CR1_UE;
    port->Usart->CR1 |= USART_CR1_TE;

    port->Usart->TDR = 0x00;
    port->Usart->CR1 |= USART_CR1_TCIE;
}

void USART1_IRQHandler() {
    USART_HandleIRQ(&usart_config[0]);
}

void USART2_IRQHandler() {
    USART_HandleIRQ(&usart_config[1]);
}

void USART3_IRQHandler() {
    USART_HandleIRQ(&usart_config[3]);
}

void UART4_IRQHandler() {
    USART_HandleIRQ(&usart_config[2]);
}

void DMA1_Channel1_IRQHandler() {
    USART_HandleIRQ(&usart_config[0]);
}

void DMA1_Channel2_IRQHandler() {
    USART_HandleIRQ(&usart_config[1]);
}

void DMA1_Channel3_IRQHandler() {
    USART_HandleIRQ(&usart_config[2]);
}

void DMA1_Channel4_IRQHanlder() {
    USART_HandleIRQ(&usart_config[3]);
}
