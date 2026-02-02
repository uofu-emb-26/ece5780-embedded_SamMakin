#include <stm32f0xx_hal.h>
#include <assert.h>


void lab2_main(void)
{
    HAL_Init();// setup hal

    // clock enables
    HAL_RCC_GPIOC_CLK_Enable();
    HAL_RCC_GPIOA_CLK_Enable();

        // Configure LED pins
    GPIO_InitTypeDef led_init = {
        .Pin   = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9,
        .Mode  = GPIO_MODE_OUTPUT_PP,
        .Pull  = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW
    };

    HAL_GPIO_Init(GPIOC, &led_init);

    // Set green LED HIGH
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);

    // Enable peripheral interuprts
    hal_gpio_init_button_pa0_pulldown();

    assert((EXTI->IMR  & (1u << 0)) == 0);
    assert((EXTI->RTSR & (1u << 0)) == 0);

    hal_exti0_enable_rising();

    assert((EXTI->IMR  & (1u << 0)) != 0);
    assert((EXTI->RTSR & (1u << 0)) != 0);

    // 2.3
    uint32_t before = (SYSCFG->EXTICR[0] & 0xFu);
    assert(before <= 0x7u);   // sanity: port selector is in a valid range

    // Route PA0 to EXTI0
    hal_syscfg_route_pa0_to_exti0();

    assert((SYSCFG->EXTICR[0] & 0xFu) == 0x0u);// Check PA0 -> EXTI0


    // 2.4
    NVIC_SetPriority(EXTI0_1_IRQn, 1);
    NVIC_EnableIRQ(EXTI0_1_IRQn);

    
    while (1)
        {
                HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);  // red LED 
                  HAL_Delay(500);
        }


}

void EXTI0_1_IRQHandler(void)
{
        // Toggle orange (PC8) and green (PC9)
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9);

    // Clear pending flag for EXTI line 0 "task completed"
    EXTI->PR = (1u << 0);
}