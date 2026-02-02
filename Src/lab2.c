#include <stm32f0xx_hal.h>
#include <assert.h>


void lab2_main(void)
{
    HAL_Init();// setup hal

    // clock enables
    HAL_RCC_GPIOC_CLK_Enable();
    HAL_RCC_GPIOA_CLK_Enable();

        while (1)
    {
        // main loop work (red LED toggle later)
    }


}