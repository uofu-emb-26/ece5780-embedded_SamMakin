#include <stm32f0xx_hal.h>
#include <assert.h>

#define UART_BAUD 115200u

static volatile char g_rx_char = 0;
static volatile uint8_t g_rx_ready = 0;

static void gpio_usart3_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIOB->MODER &= ~((3u << (10 * 2)) | (3u << (11 * 2)));
    GPIOB->MODER |=  ((2u << (10 * 2)) | (2u << (11 * 2)));

    GPIOB->AFR[1] &= ~((0xFu << ((10 - 8) * 4)) | (0xFu << ((11 - 8) * 4)));
    GPIOB->AFR[1] |=  ((4u   << ((10 - 8) * 4)) | (4u   << ((11 - 8) * 4)));
}

static void usart3_init(void)
{
    __HAL_RCC_USART3_CLK_ENABLE();

    USART3->CR1 = 0;
    USART3->BRR = (uint16_t)(HAL_RCC_GetHCLKFreq() / UART_BAUD);
    USART3->CR1 |= USART_CR1_TE | USART_CR1_RE;
    USART3->CR1 |= USART_CR1_RXNEIE;
    USART3->CR1 |= USART_CR1_UE;

    NVIC_SetPriority(USART3_4_IRQn, 1);
    NVIC_EnableIRQ(USART3_4_IRQn);
}

static void usart3_tx_char(char c)
{
    while ((USART3->ISR & USART_ISR_TXE) == 0) { }
    USART3->TDR = (uint8_t)c;
}

void USART3_4_IRQHandler(void)
{
    if ((USART3->ISR & USART_ISR_RXNE) != 0)
    {
        g_rx_char = (char)USART3->RDR;
        g_rx_ready = 1;
    }
}

void lab4_main(void)
{
    HAL_Init();

    gpio_usart3_init();
    usart3_init();

    while (1)
    {
        if (g_rx_ready)
        {
            g_rx_ready = 0;
            usart3_tx_char(g_rx_char);
        }
    }
}