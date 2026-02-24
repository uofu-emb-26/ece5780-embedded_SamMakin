#include <stm32f0xx_hal.h>
#include <assert.h>

static void gpio_usart3_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIOB->MODER &= ~((3u << (10 * 2)) | (3u << (11 * 2)));
    GPIOB->MODER |=  ((2u << (10 * 2)) | (2u << (11 * 2)));

    GPIOB->AFR[1] &= ~((0xFu << ((10 - 8) * 4)) | (0xFu << ((11 - 8) * 4)));
    GPIOB->AFR[1] |=  ((4u   << ((10 - 8) * 4)) | (4u   << ((11 - 8) * 4)));
}

void lab4_main(void)
{
    HAL_Init();
    gpio_usart3_init();

    while (1) { }
}