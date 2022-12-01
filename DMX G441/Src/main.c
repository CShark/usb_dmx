#include "main.h"
#include "dmx_usart.h"
#include "usb.h"

static void Clock_Init(void);
static void GPIO_Init(void);
static void ReadPortConfig(void);

static char portConfig[] = {0, 0, 0, 0};

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    Clock_Init();
    GPIO_Init();

    ReadPortConfig();

    // USART_Init(portConfig);
    // USB_Init();

    while (1) {
    }
}

/**
 * @brief Get the configuration for each port based on the resistors on  the back
 * @details
 * No resistor = Disabled
 * GND = Output
 * 3.3V = Input
 */
static void ReadPortConfig() {
    // Turn on ADC
    ADC1->CR &= ~ADC_CR_DEEPPWD;
    ADC1->CR |= ADC_CR_ADVREGEN;
    // wait 20μs
    SysTick->LOAD = 2000;
    SysTick->VAL = 0;
    SysTick->CTRL = 1;
    while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0) {
    }
    SysTick->CTRL = 0;

    // ADC Calibration
    ADC1->CR &= ~ADC_CR_ADEN;
    ADC1->CR &= ~ADC_CR_ADCALDIF;
    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL) {
    }
    ADC1->CR |= ADC_CR_ADEN;

    // Patch ADC Channels
    ADC1->CFGR |= (2 << ADC_CFGR_RES_Pos);
    ADC1->SMPR1 |= (7 << ADC_SMPR1_SMP1_Pos) | (7 << ADC_SMPR1_SMP2_Pos) | (7 << ADC_SMPR1_SMP3_Pos) | (7 << ADC_SMPR1_SMP4_Pos);

    // Run ADC
    char data[12] = {0};

    for (int i = 0; i < 4; i++) {
        ADC1->SQR1 &= ~ADC_SQR1_SQ1_Msk;
        ADC1->SQR1 |= ((i + 1) << ADC_SQR1_SQ1_Pos);

        ADC1->CR |= ADC_CR_ADSTART;
        while (!(ADC1->ISR & ADC_ISR_EOC)) {
        }

        data[i] = ADC1->DR;
    }

    // Activate pulldowns
    GPIOA->PUPDR |= (2 << GPIO_PUPDR_PUPD0_Pos) | (2 << GPIO_PUPDR_PUPD1_Pos) | (2 << GPIO_PUPDR_PUPD2_Pos) | (2 << GPIO_PUPDR_PUPD3_Pos);

    // Redo measurements
    for (int i = 0; i < 4; i++) {
        ADC1->SQR1 &= ~ADC_SQR1_SQ1_Msk;
        ADC1->SQR1 |= ((i + 1) << ADC_SQR1_SQ1_Pos);

        ADC1->CR |= ADC_CR_ADSTART;
        while (!(ADC1->ISR & ADC_ISR_EOC)) {
        }

        data[i + 4] = ADC1->DR;
    }

    // deactivate pulldowns
    GPIOA->PUPDR &= ~0xFF;

    // redo measurements
    for (int i = 0; i < 4; i++) {
        ADC1->SQR1 &= ~ADC_SQR1_SQ1_Msk;
        ADC1->SQR1 |= ((i + 1) << ADC_SQR1_SQ1_Pos);

        ADC1->CR |= ADC_CR_ADSTART;
        while (!(ADC1->ISR & ADC_ISR_EOC)) {
        }

        data[i + 8] = ADC1->DR;
    }

    // detect configuration
    for (int i = 0; i < 4; i++) {
        if ((data[i] & 0xC0) == 0xC0 && data[i + 4] > 0 && (data[i + 8] & 0xC0) == 0xC0) {
            portConfig[i] = USART_INPUT;
        } else if (data[i] == 0 && data[i + 4] == 0 && data[i + 8] == 0) {
            portConfig[i] = USART_OUTPUT;
        } else {
            // unconnected port, don't patch.
            // If no resistor is present, value will float w/o pulldown
        }
    }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
static void Clock_Init(void) {
    RCC->CR |= RCC_CR_HSEON | RCC_CR_HSION;

    // Configure PLL (R=143.75, Q=47.92)
    RCC->CR &= ~RCC_CR_PLLON;
    while (RCC->CR & RCC_CR_PLLRDY) {
    }
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE | RCC_PLLCFGR_PLLM_0 | (23 << RCC_PLLCFGR_PLLN_Pos) | RCC_PLLCFGR_PLLQ_1;
    RCC->PLLCFGR |= RCC_PLLCFGR_PLLREN | RCC_PLLCFGR_PLLQEN;
    RCC->CR |= RCC_CR_PLLON;

    // Select PLL as main clock, AHB/2 > otherwise Bus Error Hard Fault
    RCC->CFGR |= RCC_CFGR_HPRE_3 | RCC_CFGR_SW_PLL;

    // Select & Enable IO Clocks (PLL > USB, ADC; PLLC (71.875) > UART)
    RCC->CCIPR = RCC_CCIPR_CLK48SEL_1 | RCC_CCIPR_ADC12SEL_1;
    RCC->AHB2ENR |= RCC_AHB2ENR_ADC12EN | RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN;
    RCC->APB1ENR1 |= RCC_APB1ENR1_USBEN | RCC_APB1ENR1_UART4EN | RCC_APB1ENR1_USART3EN | RCC_APB1ENR1_USART2EN | RCC_APB1ENR1_TIM2EN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

    // Enable DMAMUX & DMA1 Clock
    RCC->AHB1ENR |= RCC_AHB1ENR_DMAMUX1EN | RCC_AHB1ENR_DMA1EN;
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void GPIO_Init(void) {
    /*  PA
        11 & 12 > USB
        13 & 14 > SWDIO & SWCLK
        15 > DMX 1 Direction Output
     */
    GPIOA->MODER &= ~(GPIO_MODER_MODE15);
    GPIOA->MODER |= GPIO_MODER_MODE15_0;
    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD15);

    /* PB
      3 & 4 > DMX 4 TX,RX
      5,6,7 > DMX 3 DE,TX,RX
      8 > BOOT0
      10,11,12 > DMX 2 TX,RX,DE
     */
    GPIOB->AFR[0] = (7 << GPIO_AFRL_AFSEL3_Pos) | (7 << GPIO_AFRL_AFSEL4_Pos) | (7 << GPIO_AFRL_AFSEL6_Pos) | (7 << GPIO_AFRL_AFSEL7_Pos);
    GPIOB->AFR[1] = (7 << GPIO_AFRH_AFSEL10_Pos) | (7 << GPIO_AFRH_AFSEL11_Pos);
    GPIOB->MODER &= ~(GPIO_MODER_MODE3 | GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7 | GPIO_MODER_MODE8 | GPIO_MODER_MODE10 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12);
    GPIOB->MODER |= GPIO_MODER_MODE3_1 | GPIO_MODER_MODE4_1 | GPIO_MODER_MODE5_0 | GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1 | GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1 | GPIO_MODER_MODE12_0;
    GPIOB->PUPDR |= GPIO_PUPDR_PUPD4_0 | GPIO_PUPDR_PUPD7_0 | GPIO_PUPDR_PUPD11_0;

    /* PC
      10,11 > DMX 1 TX,RX
      12  > DMX 4 DE
     */
    GPIOC->AFR[1] = (5 << GPIO_AFRH_AFSEL10_Pos) | (5 << GPIO_AFRH_AFSEL11_Pos);
    GPIOC->MODER &= ~(GPIO_MODER_MODE10 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12);
    GPIOC->MODER |= GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1 | GPIO_MODER_MODE12_0;
    GPIOC->PUPDR |= GPIO_PUPDR_PUPD11_0;
}

void NMI_Handler() {
}

void HardFault_Handler() {
    while (1) {
    }
}
