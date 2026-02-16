#include <stm32f0xx_hal.h>
#include <assert.h>

void lab3_main(void)
{
    HAL_Init();// setup hal

    // clock enables
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();

    // Pin defs
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin   = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio.Pull  = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &gpio);

    // Initial LED state
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);

    TIM2->PSC = 7999; // 1 ms tick
    TIM2->ARR = 250; // 250 ms reset

    TIM2->DIER |= TIM_DIER_UIE; // Update event interupt

    TIM2->EGR |= TIM_EGR_UG;    // load PSC/ARR
    TIM2->SR  &= ~TIM_SR_UIF;   // clear pending update flag again

    NVIC_EnableIRQ(TIM2_IRQn);// wire interupt to CPU

    TIM2->CR1 |= TIM_CR1_CEN;   // start timer
}


//Handler
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF) {

        TIM2->SR &= ~TIM_SR_UIF;     //clear flag

        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8) == GPIO_PIN_SET) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
        }
    }
}
