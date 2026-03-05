#include <stm32f0xx_hal.h>
#include <assert.h>

void lab3_main(void)
{
    HAL_Init();// setup hal

    // clock enables
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();

    //LED alt funciton
    GPIOC->MODER &= ~(GPIO_MODER_MODER6_Msk | GPIO_MODER_MODER7_Msk);
    GPIOC->MODER |=  (GPIO_MODER_MODER6_1   | GPIO_MODER_MODER7_1);

    GPIOC->AFR[0] &= ~((0xF << (6 * 4)) | (0xF << (7 * 4)));

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

    //3.1
    TIM2->PSC = 7999;
    TIM2->ARR = 250;

    TIM2->DIER |= TIM_DIER_UIE;

    TIM2->EGR |= TIM_EGR_UG;
    TIM2->SR  &= ~TIM_SR_UIF;

    NVIC_EnableIRQ(TIM2_IRQn);

    TIM2->CR1 |= TIM_CR1_CEN;


    //3.2
    TIM3->PSC = 9;
    TIM3->ARR = 1000;

    TIM3->EGR |= TIM_EGR_UG; //update
    TIM3->SR  &= ~TIM_SR_UIF; //clear flag  

   
    TIM3->CCMR1 = 0; //starting bit

    TIM3->CCMR1 |= (0 << TIM_CCMR1_CC1S_Pos); // enCH1
    TIM3->CCMR1 |= (7 << TIM_CCMR1_OC1M_Pos); // pwm mode 2
    TIM3->CCMR1 |= TIM_CCMR1_OC1PE;

    TIM3->CCMR1 |= (0 << TIM_CCMR1_CC2S_Pos);// enCH2
    TIM3->CCMR1 |= (6 << TIM_CCMR1_OC2M_Pos);
    TIM3->CCMR1 |= TIM_CCMR1_OC2PE;

    //connect pins
  TIM3->CCER |= TIM_CCER_CC1E;
TIM3->CCER |= TIM_CCER_CC2E;

uint16_t StartingVal1 = 0;
uint16_t StartingVal2 = 1000;

TIM3->CCR2 = StartingVal1;
TIM3->CCR1 = StartingVal2;

TIM3->EGR |= TIM_EGR_UG;
TIM3->SR  &= ~TIM_SR_UIF;

TIM3->CR1 |= TIM_CR1_CEN;

while (1)
{
    if (StartingVal1 < 1000) {
        StartingVal1++;
    } else {
        StartingVal1 = 0;
    }
    TIM3->CCR2 = StartingVal1;

    if (StartingVal2 > 0) {
        StartingVal2--;
    } else {
        StartingVal2 = 1000;
    }
    TIM3->CCR1 = StartingVal2;

    HAL_Delay(5);
}

    TIM3->CR1 |= TIM_CR1_CEN;

}


//Handler
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF) {

        TIM2->SR &= ~TIM_SR_UIF;

        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8) == GPIO_PIN_SET) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
        }
    }
}
