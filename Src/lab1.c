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
    // LED on PC8 ON, PC9 OFF
    GPIOC->BSRR = GPIO_PIN_8;           // set PC8
    GPIOC->BSRR = (GPIO_PIN_9 << 16);   // reset PC9
    HAL_Delay(100);

    // LED on PC8 OFF, PC9 ON
    GPIOC->BSRR = GPIO_PIN_9;           // set PC9
    GPIOC->BSRR = (GPIO_PIN_8 << 16);   // reset PC8
    HAL_Delay(100);
}
}
