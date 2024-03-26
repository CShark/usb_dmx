// #include "dmx_usart.h"
// #include "platform.h"

// static unsigned char dmx_buffer[4][513] = {{0}, {0}, {0}, {0}};

// USART_DmxConfig dmx_config[] = {
//     {.Usart = USART1,
//      .Irq = USART1_IRQn,
//      .DRPort = GPIOB,
//      .DRPin = 5,
//      .RxPort = GPIOB,
//      .RxPin = 7,

//      .DmxBuffer = dmx_buffer[0],

//      .DmaMux = DMAMUX1_Channel0,
//      .DmaMux_RX = 24,
//      .DmaMux_TX = 25,
//      .Dma = DMA1_Channel1,
//      .DmaIFCR_Offset = DMA_IFCR_CGIF3_Pos},

//     {.Usart = USART2,
//      .Irq = USART2_IRQn,
//      .DRPort = GPIOC,
//      .DRPin = 12,
//      .RxPort = GPIOB,
//      .RxPin = 4,

//      .DmxBuffer = dmx_buffer[1],

//      .DmaMux = DMAMUX1_Channel1,
//      .DmaMux_RX = 26,
//      .DmaMux_TX = 27,
//      .Dma = DMA1_Channel2,
//      .DmaIFCR_Offset = DMA_IFCR_CGIF2_Pos},

//     {.Usart = UART4,
//      .Irq = UART4_IRQn,
//      .DRPort = GPIOA,
//      .DRPin = 15,
//      .RxPort = GPIOC,
//      .RxPin = 11,

//      .DmxBuffer = dmx_buffer[2],

//      .DmaMux = DMAMUX1_Channel2,
//      .DmaMux_RX = 30,
//      .DmaMux_TX = 31,
//      .Dma = DMA1_Channel3,
//      .DmaIFCR_Offset = DMA_IFCR_CGIF3_Pos},

//     {.Usart = USART3,
//      .Irq = USART3_IRQn,
//      .DRPort = GPIOB,
//      .DRPin = 12,
//      .RxPort = GPIOB,
//      .RxPin = 11,

//      .DmxBuffer = dmx_buffer[3],

//      .DmaMux = DMAMUX1_Channel3,
//      .DmaMux_RX = 28,
//      .DmaMux_TX = 29,
//      .Dma = DMA1_Channel4,
//      .DmaIFCR_Offset = DMA_IFCR_CGIF4_Pos}};

// static void USART_Disable(USART_DmxConfig *dmx);
// static void USART_DisableDma(USART_DmxConfig *dmx);
// static void USART_TxEn(USART_DmxConfig *dmx);
// static void USART_TxDma(USART_DmxConfig *dmx, const char *buffer, unsigned short length);
// static void USART_RxEn(USART_DmxConfig *dmx);
// static void USART_RxDma(USART_DmxConfig *dmx, char *buffer, unsigned short length);
// static void USART_SendBreak(USART_DmxConfig *dmx);

// static void USART_HandleIRQ(USART_DmxConfig *dmx);
// static void USART_HandleIRQInput(USART_DmxConfig *dmx);
// static void USART_HandleIRQOutput(USART_DmxConfig *dmx);

// void USART_Init(CONFIG *config) {
//     // Initialize DMX Buffer
//     for (int i = 0; i < 4; i++) {
//         for (int b = 0; b < 513; b++) {
//             dmx_buffer[i][b] = 0x00;
//         }
//         if (i == 0)
//             dmx_buffer[i][1] = 0xEA;
//         if (i == 1)
//             dmx_buffer[i][1] = 0xBF;

//         NVIC_SetPriority(dmx_config[i].Irq, 1);
//         NVIC_EnableIRQ(dmx_config[i].Irq);

//         if (config->ArtNet[i].PortFlags & PORT_FLAG_RDM) {
//             USART_AlterPortFlags(i, PORT_FLAG_RDM, 1);
//         }

//         if (config->ArtNet[i].PortFlags & PORT_FLAG_SINGLE) {
//             USART_AlterPortFlags(i, PORT_FLAG_SINGLE, 1);
//         }

//         if (config->ArtNet[i].PortFlags & PORT_FLAG_INDISABLED) {
//             USART_AlterPortFlags(i, PORT_FLAG_INDISABLED, 1);
//         }

//         dmx_config[i].State &= ~USART_MODE_MSK;
//         if (config->ArtNet[i].PortDirection == ARTNET_OUTPUT) {
//             dmx_config[i].State |= USART_MODE_OUTPUT;
//             dmx_config[i].State &= ~USART_DMX_STATE_Msk;
//             dmx_config[i].State |= USART_DMX_STATE_DMX;
//             USART_TxEn(&dmx_config[i]);
//             USART_SendBreak(&dmx_config[i]);
//         } else {
//             if ((dmx_config[i].Flags & PORT_FLAG_INDISABLED) == 0) {
//                 dmx_config[i].State |= USART_MODE_INPUT;
//                 dmx_config[i].State &= ~USART_DMX_STATE_Msk;
//                 dmx_config[i].State |= USART_DMX_STATE_DMX;
//                 USART_RxEn(&dmx_config[i]);
//             }
//         }
//     }
// }

// void USART_AlterPortFlags(unsigned char port, ArtNet_Port_Flags mask, char value) {
//     if (port < 4) {
//         if (value) {
//             dmx_config[port].Flags |= mask;
//         } else {
//             dmx_config[port].Flags &= ~mask;
//         }
//     }
// }

// void USART_ChangePortDirection(unsigned char port, char direction) {
//     if (port < 4) {
//         if (direction == ARTNET_OUTPUT || direction == ARTNET_INPUT) {
//             if ((dmx_config[port].State & USART_MODE_MSK) != direction) {
//                 USART_Disable(&dmx_config[port]);

//                 // Clear buffer
//                 memclr(dmx_buffer[port], 513);

//                 // Change direction
//                 dmx_config[port].State &= ~USART_MODE_MSK;
//                 if (direction == ARTNET_OUTPUT) {
//                     dmx_config[port].State |= USART_MODE_OUTPUT;
//                     dmx_config[port].State |= USART_DMX_STATE_DMXPending;
//                 } else {
//                     dmx_config[port].State |= USART_MODE_INPUT;
//                     USART_RxEn(&dmx_config[port]);
//                 }
//             }
//         }
//     }
// }

// void USART_SetBuffer(unsigned char port, const unsigned char *buffer, unsigned short length) {
//     if (port < 4) {
//         if (length <= 512) {
//             for (int i = 0; i < length; i++) {
//                 dmx_buffer[port][i + 1] = buffer[i];
//             }

//             if ((dmx_config[port].Flags & PORT_FLAG_SINGLE) && (dmx_config[port].State & USART_MODE_MSK) == USART_MODE_OUTPUT) {
//                 USART_StartTransmitDmx(&dmx_config[port]);
//             }
//         }
//     }
// }

// void USART_ClearBuffer(unsigned char port) {
//     if (port < 4) {
//         memclr(&dmx_buffer[port], 513);
//     }
// }

// unsigned char *USART_GetDmxBuffer(unsigned char port) {
//     if (port < 4) {
//         return dmx_buffer[port] + 1;
//     }

//     return 0;
// }

// unsigned char USART_IsInputNew(unsigned char port) {
//     if (port < 4) {
//         return dmx_config[port].State & USART_STATE_NewInput;
//     }

//     return 0;
// }

// void USART_ClearInputNew(unsigned char port) {
//     if (port < 4) {
//         dmx_config[port].State &= ~USART_STATE_NewInput;
//     }
// }

// static void USART_Disable(USART_DmxConfig *dmx) {
//     USART_DisableDma(dmx);
//     dmx->Usart->CR1 &= ~(USART_CR1_TCIE | USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE);
//     dmx->Usart->ICR = 0xFF;
//     dmx->Usart->CR1 &= ~USART_CR1_UE;
//     DMA1->IFCR = (1 << dmx->DmaIFCR_Offset);
// }

// static void USART_DisableDma(USART_DmxConfig *dmx) {
//     dmx->Usart->CR3 &= ~(USART_CR3_DMAR | USART_CR3_DMAT);
//     dmx->Dma->CCR &= ~DMA_CCR_EN;
// }

// static void USART_TxEn(USART_DmxConfig *dmx) {
//     if (dmx->State & USART_PORT_STATE_Msk != USART_PORT_STATE_Tx) {
//         USART_Disable(dmx);

//         // Set default speed
//         dmx->Usart->BRR = USART_BRR;
//         dmx->Usart->CR2 = (0x02 << USART_CR2_STOP_Pos);
//         dmx->Usart->CR3 = USART_CR3_DDRE | USART_CR3_OVRDIS;
//         dmx->Usart->CR1 |= USART_CR1_UE;

//         // Configure driver & DMA
//         dmx->DRPort->BSRR = 1 << dmx->DRPin;
//         dmx->Dma->CPAR = &(dmx->Usart->TDR);
//         dmx->Dma->CCR = DMA_CCR_MINC | DMA_CCR_DIR | DMA_CCR_TCIE;
//         dmx->DmaMux->CCR = dmx->DmaMux_TX;

//         // Set port state
//         dmx->State &= ~USART_PORT_STATE_Msk;
//         dmx->State |= USART_PORT_STATE_Tx;
//     }
// }

// static void USART_RxEn(USART_DmxConfig *dmx) {
//     if (dmx->State & USART_PORT_STATE_Msk != USART_PORT_STATE_Rx) {
//         USART_Disable(dmx);

//         // Set default speed
//         dmx->Usart->BRR = USART_BRR;
//         dmx->Usart->CR2 = (0x02 << USART_CR2_STOP_Pos);
//         dmx->Usart->CR3 = USART_CR3_DDRE | USART_CR3_EIE | USART_CR3_OVRDIS;
//         dmx->Usart->CR1 |= USART_CR1_UE;

//         // Configure Driver & DMA
//         dmx->DRPort->BSRR = (1 << dmx->DRPin) << 16;
//         dmx->Dma->CPAR = &(dmx->Usart->RDR);
//         dmx->Dma->CCR = DMA_CCR_MINC | DMA_CCR_TCIE;
//         dmx->DmaMux->CCR = dmx->DmaMux_RX;

//         // Set Port state
//         dmx->State &= ~USART_PORT_STATE_Msk;
//         dmx->State |= USART_PORT_STATE_Rx;

//         dmx->Usart->CR1 |= USART_CR1_RE | USART_CR1_RXNEIE;
//     }
// }

// static void USART_RxDma(USART_DmxConfig *dmx, char *buffer, unsigned short length) {
//     dmx->Dma->CMAR = buffer;
//     dmx->Dma->CNDTR = length;

//     dmx->Usart->CR3 |= USART_CR3_DMAR;
//     dmx->Dma->CCR |= DMA_CCR_EN;
// }

// static void USART_TxDma(USART_DmxConfig *dmx, const char *buffer, unsigned short length) {
//     dmx->Dma->CMAR = buffer;
//     dmx->Dma->CNDTR = length;

//     dmx->Usart->CR1 |= USART_CR1_TE;

//     dmx->Usart->ICR = USART_ICR_TCCF;
//     dmx->Usart->CR3 |= USART_CR3_DMAT;
//     dmx->Dma->CCR |= DMA_CCR_EN;

//     dmx->Usart->CR1 |= USART_CR1_TCIE;
// }

// static void USART_SendBreak(USART_DmxConfig *dmx) {
//     dmx->Usart->CR3 &= ~USART_CR3_DMAT;
//     dmx->Dma->CCR &= ~DMA_CCR_EN;

//     dmx->Usart->CR1 &= ~(USART_CR1_UE | USART_CR1_TE);
//     dmx->Usart->BRR = USART_BRRBREAK;
//     dmx->Usart->CR1 |= USART_CR1_UE;
//     dmx->Usart->CR1 |= USART_CR1_TE;

//     dmx->State |= USART_STATE_Break;
//     dmx->Usart->TDR = 0x00;

//     dmx->Usart->CR1 |= USART_CR1_TCIE;
// }

// static void USART_HandleIRQInput(USART_DmxConfig *dmx) {
//     if ((dmx->State & USART_DMX_STATE_Msk) == USART_DMX_STATE_DMX) {
//         if (dmx->Usart->ISR & USART_ISR_FE) {
//             dmx->Usart->ICR = USART_ICR_FECF;

//             // Break detected, start reception
//             USART_DisableDma(dmx);
//             dmx->State |= USART_STATE_Break;
//         } else {
//             dmx->Usart->RQR = USART_RQR_RXFRQ;
//             if (dmx->State & USART_STATE_Break) {
//                 // First byte after break
//                 volatile char data = dmx->Usart->RDR;

//                 if (data == 0x00) {
//                     // DMX Frame
//                     USART_RxDma(dmx, dmx->DmxBuffer, 513);
//                     dmx->State |= USART_STATE_NewInput;
//                 }

//                 dmx->State &= ~USART_STATE_Break;
//             }
//         }
//     }
// }

// static void USART_HandleIRQOutput(USART_DmxConfig *dmx) {
//     dmx->Usart->CR1 &= ~USART_CR1_TCIE;
//     dmx->Usart->ICR = USART_ICR_TCCF;

//     if ((dmx->State & USART_DMX_STATE_Msk) == USART_DMX_STATE_DMX) {
//         if (dmx->State & USART_STATE_Break) {
//             // Return to normal speed & send buffer
//             dmx->State &= ~USART_STATE_Break;
//             dmx->Usart->CR1 &= ~(USART_CR1_UE | USART_CR1_TE);
//             dmx->Usart->BRR = USART_BRR;
//             dmx->Usart->CR1 |= USART_CR1_UE;

//             USART_TxDma(dmx, dmx->DmxBuffer, 513);
//         } else {
//             // Send next break
//             dmx->State &= ~USART_DMX_STATE_Msk;
//             if (dmx->Flags & PORT_FLAG_SINGLE) {
//                 USART_Disable(dmx);
//                 dmx->State |= USART_DMX_STATE_Pause;
//             } else {
//                 dmx->State |= USART_DMX_STATE_DMXPending;
//             }
//         }
//     }
// }

// static void USART_HandleIRQ(USART_DmxConfig *dmx) {
//     if ((dmx->State & USART_DMX_STATE_Msk) == USART_DMX_STATE_Pause) {
//         USART_Disable(dmx);
//         return;
//     }

//     if ((dmx->State & USART_MODE_MSK) == USART_MODE_OUTPUT) {
//         USART_HandleIRQOutput(dmx);
//     } else {
//         USART_HandleIRQInput(dmx);
//     }

//     dmx->Usart->ICR = 0xFF;
// }

// void USART_Tick() {
//     for (int i = 0; i < 4; i++) {
//         if (dmx_config[i].State & USART_MODE_MSK == USART_MODE_OUTPUT) {
//             if (dmx_config[i].Flags & PORT_FLAG_RDM) {
//                 // check if rdm is taking over
//                 RDM_IoRequest rdm_req = RDM_TickPort(i);
//                 if (rdm_req.Type != RDM_IOREQ_NONE) {
//                     continue;
//                 }
//             }

//             if (dmx_config[i].State & USART_DMX_STATE_Msk == USART_DMX_STATE_DMXPending) {
//                 dmx_config[i].State &= ~USART_DMX_STATE_Msk;
//                 dmx_config[i].State |= USART_DMX_STATE_DMX;

//                 USART_TxEn(&dmx_config[i]);
//                 USART_SendBreak(&dmx_config[i]);
//             }
//         } else {
//         }
//     }
// }

// void USART1_IRQHandler() {
//     USART_HandleIRQ(&dmx_config[0]);
// }

// void USART2_IRQHandler() {
//     USART_HandleIRQ(&dmx_config[1]);
// }

// void USART3_IRQHandler() {
//     USART_HandleIRQ(&dmx_config[3]);
// }

// void UART4_IRQHandler() {
//     USART_HandleIRQ(&dmx_config[2]);
// }
