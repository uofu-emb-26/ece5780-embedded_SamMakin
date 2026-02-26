#include <stm32f0xx_hal.h>
#include <assert.h>

#define UART_BAUD 115200u

static volatile char    g_rx_char  = 0;
static volatile uint8_t g_rx_ready = 0;

static void gpio_usart3_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIOB->MODER &= ~((3u << (10u * 2u)) | (3u << (11u * 2u)));
    GPIOB->MODER |=  ((2u << (10u * 2u)) | (2u << (11u * 2u)));

    GPIOB->AFR[1] &= ~((0xFu << ((10u - 8u) * 4u)) | (0xFu << ((11u - 8u) * 4u)));
    GPIOB->AFR[1] |=  ((4u   << ((10u - 8u) * 4u)) | (4u   << ((11u - 8u) * 4u)));
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

static void usart3_tx_str(const char *s)
{
    while (*s) { usart3_tx_char(*s++); }
}

void USART3_4_IRQHandler(void)
{
    if ((USART3->ISR & USART_ISR_RXNE) != 0)
    {
        g_rx_char  = (char)USART3->RDR;
        g_rx_ready = 1;
    }
}

static void leds_init(void)
{
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIOC->MODER &= ~((3u << (6u * 2u)) | (3u << (7u * 2u)) | (3u << (8u * 2u)) | (3u << (9u * 2u)));
    GPIOC->MODER |=  ((1u << (6u * 2u)) | (1u << (7u * 2u)) | (1u << (8u * 2u)) | (1u << (9u * 2u)));

    GPIOC->AFR[0] &= ~((0xFu << (6u * 4u)) | (0xFu << (7u * 4u)));
    GPIOC->AFR[1] &= ~((0xFu << ((8u - 8u) * 4u)) | (0xFu << ((9u - 8u) * 4u)));

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpio);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);
}

static uint16_t led_pin_from_color(char which)
{
    if (which == 'r') return GPIO_PIN_6;
    if (which == 'b') return GPIO_PIN_7;
    if (which == 'o') return GPIO_PIN_8;
    if (which == 'g') return GPIO_PIN_9;
    return 0;
}

static void led_apply(char which, char action)
{
    uint16_t pin = led_pin_from_color(which);
    if (pin == 0) return;

    if (action == '0')      HAL_GPIO_WritePin(GPIOC, pin, GPIO_PIN_RESET);
    else if (action == '1') HAL_GPIO_WritePin(GPIOC, pin, GPIO_PIN_SET);
    else if (action == 't') HAL_GPIO_TogglePin(GPIOC, pin);
}

static void cmd_prompt(void)
{
    usart3_tx_str("\r\nCMD? ");
}

static void bad_cmd(void)
{
    usart3_tx_str("\r\nERR: use r/b/o/g + 0/1/2\r\n");
    cmd_prompt();
}

void lab4_main(void)
{
    HAL_Init();

    leds_init();
    gpio_usart3_init();
    usart3_init();

    usart3_tx_str("\r\nReady. r/b/o/g then 0/1/t\r\n");
    cmd_prompt();

    char first = 0;

    while (1)
    {
        if (!g_rx_ready) continue;

        g_rx_ready = 0;
        char c = g_rx_char;

        if (c == '\r' || c == '\n') continue;

        usart3_tx_char(c);  // echo

        if (first == 0)
        {
            first = c;
        }
        else
        {
            char second = c;

            if (led_pin_from_color(first) != 0 &&
                (second == '0' || second == '1' || second == 't'))
            {
                led_apply(first, second);
                cmd_prompt();
            }
            else
            {
                bad_cmd();
            }

            first = 0;
        }
    }
}