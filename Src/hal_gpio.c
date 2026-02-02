#include <stdint.h>
#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_gpio.h>
/*
void My_HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
}
*/

/*
void My_HAL_GPIO_DeInit(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin)
{
}
*/

/*
GPIO_PinState My_HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    return -1;
}
*/

/*
void My_HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
}
*/

/*
void My_HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
}
*/
void hal_gpio_init_button_pa0_pulldown(void)
{
    // Enable GPIOA clock 
    HAL_RCC_GPIOA_CLK_Enable();

    GPIO_InitTypeDef init = {
        .Pin   = GPIO_PIN_0,
        .Mode  = GPIO_MODE_INPUT,
        .Pull  = GPIO_PULLDOWN,
        .Speed = GPIO_SPEED_FREQ_LOW
    };

    HAL_GPIO_Init(GPIOA, &init);
}

void hal_exti0_enable_rising(void)
{
    // Unmask EXTI0 
    EXTI->IMR |= (1u << 0);

    // Rising edge trigger
    EXTI->RTSR |= (1u << 0);
}
void hal_syscfg_route_pa0_to_exti0(void)
{
    // Enable SYSCFG peripheral clock
    __HALmake _RCC_SYSCFG_CLK_ENABLE();

    // Route PA0 to EXTI0 
    // EXTI0 selection is bits [3:0] of EXTICR[0]
    SYSCFG->EXTICR[0] &= ~(0xFu << 0);   // sets to 0000 which is port A
}