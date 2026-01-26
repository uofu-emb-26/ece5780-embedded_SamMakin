#include <stm32f0xx_hal.h>

void lab1_main(void)
{
    HAL_Init();

    HAL_RCC_GPIOC_CLK_Enable();

    GPIO_InitTypeDef initStr = {
        .Pin   = GPIO_PIN_8 | GPIO_PIN_9,
        .Mode  = GPIO_MODE_OUTPUT_PP,
        .Pull  = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW
    };

    HAL_GPIO_Init(GPIOC, &initStr);

    while (1)
    {
        HAL_Delay(300);
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9);
         HAL_Delay(50);
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9);
         HAL_Delay(50);
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9);

    }
}
