#include "dmx_usart.h"

__PACKED
char dmx_buffer[4][512] = {{0}, {0}, {0}, {0}};

USART_DmxConfig dmx_config[] = {
    {.Usart = USART1,
     .Irq = USART1_IRQn,
     .DRPort = GPIOB,
     .DRPin = 5,

     .DmxBuffer = dmx_buffer[0],

     .DmaMux = DMAMUX1_Channel0,
     .DmaMux_RX = 24,
     .DmaMux_TX = 25,
     .Dma = DMA1_Channel1},

    {.Usart = USART2,
     .Irq = USART2_IRQn,
     .DRPort = GPIOC,
     .DRPin = 12,

     .DmaMux = DMAMUX1_Channel1,
     .DmaMux_RX = 26,
     .DmaMux_TX = 27,
     .Dma = DMA1_Channel2},

    {.Usart = UART4,
     .Irq = UART4_IRQn,
     .DRPort = GPIOA,
     .DRPin = 15,

     .DmaMux = DMAMUX1_Channel2,
     .DmaMux_RX = 30,
     .DmaMux_TX = 31,
     .Dma = DMA1_Channel3},

    {.Usart = USART3,
     .Irq = USART3_IRQn,
     .DRPort = GPIOB,
     .DRPin = 12,

     .DmaMux = DMAMUX1_Channel3,
     .DmaMux_RX = 28,
     .DmaMux_TX = 29,
     .Dma = DMA1_Channel4}};

static void USART_ConfigTransmit(USART_DmxConfig *dmx);
static void USART_ConfigReceive(USART_DmxConfig *dmx);
static void USART_StartTransmitDmx(USART_DmxConfig *dmx);

void USART_Init(char *portDirection) {
    // Initialize DMX Buffer
    for (int i = 0; i < 4; i++) {
        for (int b = 0; b < 512; b++) {
            dmx_buffer[i][b] = 0x00;
        }
    }

    for (int i = 0; i < 4; i++) {
        // 16MHz HSI16, 250kbps = 64 USARTDIV, OVER8=0
        NVIC_SetPriority(dmx_config[i].Irq, 0);
        NVIC_EnableIRQ(dmx_config[i].Irq);

        if (portDirection[i] == USART_OUTPUT) {
            USART_ConfigTransmit(&dmx_config[i]);
        } else if (portDirection[i] == USART_INPUT) {
            USART_ConfigReceive(&dmx_config[i]);
        }
    }

    // USART_StartTransmitDmx(&dmx_config[0]);
}

void USART_SetPortState(char port, char enable) {
    if (port >= 0 && port < 4) {
        if (dmx_config[port].State != USART_DMX_STATE_UNCONFIGURED) {
            if (dmx_config[port].State == USART_DMX_STATE_Pause && enable != 0) {
                if (dmx_config[port].IOType == USART_OUTPUT) {
                    USART_StartTransmitDmx(&dmx_config[port]);
                } else {
                }
            } else if (dmx_config[port].State == USART_DMX_STATE_DMX && enable == 0) {
                dmx_config[port].State = USART_DMX_STATE_Pause;
            }
        }
    }
}

void USART_SetBufferPage(char port, char page, char *buffer) {
    if (port >= 0 && port < 4) {
        if (page >= 0 && page < 0x10) {
            for (int i = 0; i < 32; i++) {
                dmx_buffer[port][page * 32 + i] = buffer[i];
            }
        }
    }
}

static void USART_ConfigTransmit(USART_DmxConfig *dmx) {
    dmx->Usart->BRR = 64;
    dmx->Usart->CR2 |= (0x02 << USART_CR2_STOP_Pos);
    dmx->Usart->CR3 |= USART_CR3_EIE | USART_CR3_DDRE | USART_CR3_DMAT;
    dmx->Usart->CR1 |= USART_CR1_UE;

    dmx->DRPort->BSRR = 1 << dmx->DRPin;
    dmx->Dma->CPAR = &(dmx->Usart->TDR);
    dmx->Dma->CMAR = dmx->DmxBuffer;
    dmx->Dma->CNDTR = 512;
    dmx->Dma->CCR = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TCIE | DMA_CCR_TEIE;
    dmx->DmaMux->CCR = dmx->DmaMux_TX;

    dmx->State = USART_DMX_STATE_Pause;
    dmx->IOType = USART_OUTPUT;
}

static void USART_ConfigReceive(USART_DmxConfig *dmx) {
    dmx->Usart->BRR = 64;
    dmx->Usart->CR2 |= (0x02 << USART_CR2_STOP_Pos);
}

static void USART_StartTransmitDmx(USART_DmxConfig *dmx) {
    if (dmx->State == USART_DMX_STATE_Pause) {
        dmx->State = USART_DMX_STATE_DMX;

        dmx->Usart->ICR |= USART_ICR_TCCF;
        dmx->Usart->CR3 &= ~USART_CR3_DMAT;
        dmx->Usart->CR1 |= USART_CR1_TCIE;
        dmx->Usart->CR1 &= ~USART_CR1_UE;
        dmx->Usart->BRR = 128;

        dmx->Usart->CR1 |= USART_CR1_UE;
        dmx->Usart->TDR = 0x00;
        dmx->Usart->CR1 |= USART_CR1_TE;
    }
}

static void USART_HandleIrqResponse(USART_DmxConfig *dmx) {
    dmx->Usart->ICR |= USART_ICR_TCCF;

    if (dmx->IOType == USART_OUTPUT) {
        if (dmx->Usart->BRR != 64) {
            // Break & MAB, send control byte
            dmx->Usart->CR1 &= ~USART_CR1_UE;
            dmx->Usart->BRR = 64;
            dmx->Usart->CR1 |= USART_CR1_UE;
            dmx->Usart->TDR = 0x00;
            dmx->Usart->CR1 |= USART_CR1_TE;
        } else if (dmx->State == USART_DMX_STATE_DMX) {
            if ((dmx->Usart->CR3 & USART_CR3_DMAT) == 0) {
                // initiate dma
                dmx->Usart->CR3 |= USART_CR3_DMAT;
                dmx->Dma->CNDTR = 512;
                dmx->Dma->CMAR = dmx->DmxBuffer;
                dmx->Dma->CCR |= DMA_CCR_EN;
                dmx->Usart->CR1 |= USART_CR1_TE;
            } else {
                if (dmx->State == USART_DMX_STATE_DMX) {
                    // Initiate next DMX Frame
                    dmx->Dma->CCR &= ~DMA_CCR_EN;
                    dmx->Usart->CR3 &= ~USART_CR3_DMAT;
                    dmx->Usart->CR1 &= ~(USART_CR1_UE | USART_CR1_TCIE);

                    dmx->Usart->BRR = 128;
                    dmx->Usart->CR1 |= USART_CR1_UE;
                    dmx->Usart->ICR |= USART_ICR_TCCF;
                    dmx->Usart->CR1 |= USART_CR1_TCIE;
                    dmx->Usart->TDR = 0x00;
                    dmx->Usart->CR1 |= USART_CR1_TE;
                }
            }
        }
    }
}

void USART1_IRQHandler() {
    USART_HandleIrqResponse(&dmx_config[0]);
}

void USART2_IRQHandler() {
    USART_HandleIrqResponse(&dmx_config[1]);
}

void USART3_IRQHandler() {
    USART_HandleIrqResponse(&dmx_config[3]);
}

void UART4_IRQHandler() {
    USART_HandleIrqResponse(&dmx_config[2]);
}
