#include "dmx_usart.h"
#include "platform.h"

static char dmx_buffer[4][513] = {{0}, {0}, {0}, {0}};

USART_DmxConfig dmx_config[] = {
    {.Usart = USART1,
     .Irq = USART1_IRQn,
     .DRPort = GPIOB,
     .DRPin = 5,
     .RxPort = GPIOB,
     .RxPin = 7,

     .DmxBuffer = dmx_buffer[0],

     .DmaMux = DMAMUX1_Channel0,
     .DmaMux_RX = 24,
     .DmaMux_TX = 25,
     .Dma = DMA1_Channel1},

    {.Usart = USART2,
     .Irq = USART2_IRQn,
     .DRPort = GPIOC,
     .DRPin = 12,
     .RxPort = GPIOB,
     .RxPin = 4,

     .DmxBuffer = dmx_buffer[1],

     .DmaMux = DMAMUX1_Channel1,
     .DmaMux_RX = 26,
     .DmaMux_TX = 27,
     .Dma = DMA1_Channel2},

    {.Usart = UART4,
     .Irq = UART4_IRQn,
     .DRPort = GPIOA,
     .DRPin = 15,
     .RxPort = GPIOC,
     .RxPin = 11,

     .DmxBuffer = dmx_buffer[2],

     .DmaMux = DMAMUX1_Channel2,
     .DmaMux_RX = 30,
     .DmaMux_TX = 31,
     .Dma = DMA1_Channel3},

    {.Usart = USART3,
     .Irq = USART3_IRQn,
     .DRPort = GPIOB,
     .DRPin = 12,
     .RxPort = GPIOB,
     .RxPin = 11,

     .DmxBuffer = dmx_buffer[3],

     .DmaMux = DMAMUX1_Channel3,
     .DmaMux_RX = 28,
     .DmaMux_TX = 29,
     .Dma = DMA1_Channel4}};

static void USART_ConfigTransmit(USART_DmxConfig *dmx);
static void USART_ConfigReceive(USART_DmxConfig *dmx);
static void USART_StartTransmitDmx(USART_DmxConfig *dmx);
static void USART_StartReceiveDMX(USART_DmxConfig *dmx);

void USART_Init() {
    // Initialize DMX Buffer
    for (int i = 0; i < 4; i++) {
        for (int b = 0; b < 512; b++) {
            dmx_buffer[i][b] = 0x00;
        }
    }
}

void USART_InitPortDirections(char *portDirection) {
    for (int i = 0; i < 4; i++) {
        // 16MHz HSI16, 250kbps = 64 USARTDIV, OVER8=0
        NVIC_SetPriority(dmx_config[i].Irq, 1);
        NVIC_EnableIRQ(dmx_config[i].Irq);

        if (portDirection[i] == USART_OUTPUT) {
            USART_ConfigTransmit(&dmx_config[i]);
        } else if (portDirection[i] == USART_INPUT) {
            USART_ConfigReceive(&dmx_config[i]);
        }

        USART_SetPortState(i, 1);
    }
}

void USART_SetPortState(unsigned char port, char enable) {
    if (port < 4) {
        if (dmx_config[port].State != USART_DMX_STATE_UNCONFIGURED) {
            if (dmx_config[port].State == USART_DMX_STATE_Pause && enable != 0) {
                if (dmx_config[port].IOType & USART_OUTPUT) {
                    USART_StartTransmitDmx(&dmx_config[port]);
                } else {
                    USART_StartReceiveDMX(&dmx_config[port]);
                }
            } else if (dmx_config[port].State == USART_DMX_STATE_DMX && enable == 0) {
                dmx_config[port].State = USART_DMX_STATE_Pause;
            }
        }
    }
}

void USART_AlterPortFlags(unsigned char port, ArtNet_Port_Flags mask, char value) {
    if (port < 4) {
        if (value) {
            dmx_config[port].Flags |= mask;
        } else {
            dmx_config[port].Flags &= ~mask;
        }
    }
}

void USART_ChangePortDirection(unsigned char port, char direction) {
    if (port < 4) {
        if (direction == USART_INPUT || direction == USART_OUTPUT) {
            if (dmx_config[port].IOType != direction) {
                dmx_config[port].State = USART_DMX_STATE_Pause;

                // Disable USART & DMA
                dmx_config[port].Dma->CCR &= ~DMA_CCR_EN;
                dmx_config[port].Usart->CR1 &= ~(USART_CR1_RE | USART_CR1_TE);
                dmx_config[port].Usart->CR1 &= ~USART_CR1_UE;

                // Clear buffer
                memclr(dmx_buffer[port], 513);

                // Change direction
                if (direction == USART_INPUT) {
                    USART_ConfigReceive(&dmx_config[port]);
                    USART_StartReceiveDMX(&dmx_config[port]);
                } else {
                    USART_ConfigTransmit(&dmx_config[port]);
                    USART_StartTransmitDmx(&dmx_config[port]);
                }
            }
        }
    }
}

void USART_SetBuffer(unsigned char port, char *buffer, short length) {
    if (port < 4) {
        if (length <= 512) {
            for (int i = 0; i < length; i++) {
                dmx_buffer[port][i + 1] = buffer[i];
            }

            if (dmx_config[port].Flags & PORT_FLAG_SINGLE && dmx_config[port].IOType & USART_OUTPUT) {
                USART_StartTransmitDmx(&dmx_config[port]);
            }
        }
    }
}

void USART_ClearBuffer(unsigned char port) {
    if (port < 4) {
        memclr(&dmx_buffer[port], 513);
    }
}

char *USART_GetDmxBuffer(unsigned char port) {
    if (port < 4) {
        return dmx_buffer[port] + 1;
    }

    return 0;
}

char USART_IsInputNew(unsigned char port) {
    if (port < 4) {
        return dmx_config[port].NewInput;
    }

    return 0;
}

void USART_ClearInputNew(unsigned char port) {
    if (port < 4) {
        dmx_config[port].NewInput = 0;
    }
}

static void USART_ConfigTransmit(USART_DmxConfig *dmx) {
    dmx->Usart->BRR = USART_BRR;
    dmx->Usart->CR2 |= (0x02 << USART_CR2_STOP_Pos);
    dmx->Usart->CR3 |= USART_CR3_EIE | USART_CR3_DDRE;
    dmx->Usart->CR1 |= USART_CR1_UE;

    dmx->DRPort->BSRR = 1 << dmx->DRPin;
    dmx->Dma->CPAR = &(dmx->Usart->TDR);
    dmx->Dma->CMAR = (uint32_t)dmx->DmxBuffer;
    dmx->Dma->CNDTR = 513;
    dmx->Dma->CCR = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TCIE | DMA_CCR_TEIE;
    dmx->DmaMux->CCR = dmx->DmaMux_TX;

    dmx->State = USART_DMX_STATE_Pause;
    dmx->IOType = USART_OUTPUT;
}

static void USART_ConfigReceive(USART_DmxConfig *dmx) {
    dmx->Usart->BRR = USART_BRR;
    dmx->Usart->CR2 |= (0x02 << USART_CR2_STOP_Pos);
    dmx->Usart->CR3 |= USART_CR3_DDRE | USART_CR3_EIE | USART_CR3_OVRDIS;
    dmx->Usart->CR1 |= USART_CR1_UE;

    dmx->DRPort->BSRR = (1 << dmx->DRPin) << 16;
    dmx->Dma->CPAR = &(dmx->Usart->RDR);
    dmx->Dma->CMAR = dmx->DmxBuffer;
    dmx->Dma->CNDTR = 512;
    dmx->Dma->CCR = DMA_CCR_MINC | DMA_CCR_TCIE | DMA_CCR_TEIE;
    dmx->DmaMux->CCR = dmx->DmaMux_RX;

    dmx->State = USART_DMX_STATE_Pause;
    dmx->IOType = USART_INPUT;
}

static void USART_StartReceiveDMX(USART_DmxConfig *dmx) {
    dmx->State = USART_DMX_STATE_DMX;

    dmx->Usart->CR1 |= USART_CR1_RXNEIE;
    dmx->Usart->CR1 |= USART_CR1_RE;

    dmx->Dma->CCR &= ~DMA_CCR_EN;
    dmx->Usart->CR3 &= ~USART_CR3_DMAT;
}

static void USART_StartTransmitDmx(USART_DmxConfig *dmx) {
    if (dmx->State == USART_DMX_STATE_Pause) {
        dmx->State = USART_DMX_STATE_DMX;

        dmx->Dma->CCR &= ~DMA_CCR_EN;
        dmx->Usart->CR3 &= ~USART_CR3_DMAT;

        dmx->Usart->CR1 &= ~USART_CR1_UE;

        dmx->Usart->BRR = USART_BRRBREAK;

        dmx->Usart->CR1 |= USART_CR1_UE;
        dmx->Usart->CR1 |= USART_CR1_TE;
        dmx->Usart->TDR = 0x00;
        dmx->Usart->CR1 |= USART_CR1_TCIE;
    }
}

static void USART_HandleIrqResponse(USART_DmxConfig *dmx) {
    if (dmx->State == USART_DMX_STATE_DMX) {
        if (dmx->IOType & USART_OUTPUT) {
            dmx->Usart->CR1 &= ~USART_CR1_TCIE;

            if (dmx->Usart->BRR != USART_BRR) {
                dmx->Usart->CR1 &= ~(USART_CR1_UE | USART_CR1_TE);
                dmx->Usart->ICR |= USART_ICR_TCCF;

                dmx->Usart->BRR = USART_BRR;

                dmx->Usart->CR1 |= USART_CR1_UE;
                dmx->Usart->CR3 |= USART_CR3_DMAT;
                dmx->Dma->CNDTR = 513;
                dmx->Dma->CMAR = dmx->DmxBuffer;
                dmx->Usart->ICR |= USART_ICR_TCCF;
                dmx->Dma->CCR |= DMA_CCR_EN;

                dmx->Usart->CR1 |= USART_CR1_TE;
                dmx->Usart->CR1 |= USART_CR1_TCIE;
            } else {
                if ((dmx->Flags & PORT_FLAG_SINGLE) == 0) {
                    dmx->Usart->CR3 &= ~USART_CR3_DMAT;
                    dmx->Dma->CCR &= ~DMA_CCR_EN;
                    dmx->Usart->CR1 &= ~(USART_CR1_UE | USART_CR1_TE);
                    dmx->Usart->ICR |= USART_ICR_TCCF;

                    dmx->Usart->BRR = USART_BRRBREAK;

                    dmx->Usart->CR1 |= USART_CR1_UE;
                    dmx->Usart->CR1 |= USART_CR1_TE;
                    dmx->Usart->TDR = 0x00;
                    dmx->Usart->CR1 |= USART_CR1_TCIE;
                } else {
                    dmx->State = USART_DMX_STATE_Pause;
                }
            }
        } else {
            volatile char data = dmx->Usart->RDR;

            dmx->Usart->ICR |= USART_ICR_NECF | USART_ICR_PECF;

            if (dmx->Usart->ISR & USART_ISR_FE) {
                // Break, start new reception
                dmx->Usart->CR3 &= ~USART_CR3_DMAR;
                dmx->Dma->CCR &= ~DMA_CCR_EN;
                dmx->Usart->ICR |= USART_ICR_FECF;
                dmx->BreakStatus = 1;
            } else if ((dmx->Usart->CR3 & USART_CR3_DMAR) == 0) {
                // check for 0 byte
                if (dmx->BreakStatus) {
                    if (data == 0x00) {
                        dmx->Usart->CR3 |= USART_CR3_DMAR;
                        dmx->Dma->CMAR = dmx->DmxBuffer + 1;
                        dmx->Dma->CNDTR = 512;
                        dmx->Dma->CCR |= DMA_CCR_EN;
                        dmx->NewInput = 1;
                    } else {
                        dmx->BreakStatus = 0;
                    }
                }
            }
        }
    } else {
        dmx->Usart->CR1 &= ~(USART_CR1_TE | USART_CR1_RE | USART_CR1_TCIE);
        dmx->Usart->ICR |= USART_ICR_TCCF;
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
