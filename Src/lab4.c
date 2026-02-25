#include <stm32f0xx_hal.h>
#include <assert.h>

#define UART_BAUD 115200u

static volatile char g_rx_char = 0; //Recieved Char
static volatile uint8_t g_rx_ready = 0; //flag bit

static void gpio_usart3_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIOB->MODER &= ~((3u << (10 * 2)) | (3u << (11 * 2))); // clear mode bits
    GPIOB->MODER |=  ((2u << (10 * 2)) | (2u << (11 * 2))); // set AF mode (10)

    GPIOB->AFR[1] &= ~((0xFu << ((10 - 8) * 4)) | (0xFu << ((11 - 8) * 4))); // clear AF
    GPIOB->AFR[1] |=  ((4u   << ((10 - 8) * 4)) | (4u   << ((11 - 8) * 4))); // AF4 = USART3
}

static void usart3_init(void)
{
    __HAL_RCC_USART3_CLK_ENABLE();

    USART3->CR1 = 0;
    USART3->BRR = (uint16_t)(HAL_RCC_GetHCLKFreq() / UART_BAUD);

    USART3->CR1 |= USART_CR1_TE | USART_CR1_RE;  // Enable transmitter and receiver

    USART3->CR1 |= USART_CR1_RXNEIE;  // Enable RX-not-empty interrupt

    USART3->CR1 |= USART_CR1_UE; // Enable USART peripheral

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

// Setup 
static void leds_init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpio);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);
}

static void led_apply(char which, char action)
{
    uint16_t pin = 0;

    if (which == 'r') pin = GPIO_PIN_8;
    else if (which == 'g') pin = GPIO_PIN_9;
    else return;

    if (action == '0') HAL_GPIO_WritePin(GPIOC, pin, GPIO_PIN_RESET);
    else if (action == '1') HAL_GPIO_WritePin(GPIOC, pin, GPIO_PIN_SET);
    else if (action == 't') HAL_GPIO_TogglePin(GPIOC, pin);
}

void lab4_main(void)
{
    HAL_Init();
    leds_init();

    gpio_usart3_init();
    usart3_init();

    char first = 0;

    while (1)
    {
        if (!g_rx_ready) continue;

        g_rx_ready = 0;
        char c = g_rx_char;

        if (c == '\r' || c == '\n') continue;

        if (first == 0)
        {
            first = c;
        }
        else
        {
            char second = c;

            if ((first == 'g' || first == 'r') && (second == '0' || second == '1' || second == 't'))
            {
                led_apply(first, second);
            }

            first = 0;
        }
    }
}
