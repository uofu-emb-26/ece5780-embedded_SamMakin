#include "stm32f0xx.h"
#include <stdint.h>

static void leds_init(void);
static void adc_pin_init(void);
static void adc_init(void);

int lab6_main(void)
{
    HAL_Init();
    SystemClock_Config();

    leds_init();
    adc_pin_init();
    adc_init();

    while (1)
    {
        uint8_t adc_value = (uint8_t)(ADC1->DR & 0xFFu);

        if (adc_value > 127)
        {
            GPIOC->BSRR = (1u << 6);
        }
        else
        {
            GPIOC->BSRR = (1u << (6 + 16));
        }
    }
}

static void leds_init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    GPIOC->MODER &= ~(
        (3u << (6 * 2)) |
        (3u << (7 * 2)) |
        (3u << (8 * 2)) |
        (3u << (9 * 2))
    );

    GPIOC->MODER |= (
        (1u << (6 * 2)) |
        (1u << (7 * 2)) |
        (1u << (8 * 2)) |
        (1u << (9 * 2))
    );
}

static void adc_pin_init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    GPIOC->MODER |= (3u << (0 * 2));
    GPIOC->PUPDR &= ~(3u << (0 * 2));
}

static void adc_init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    if (ADC1->CR & ADC_CR_ADEN)
    {
        ADC1->CR |= ADC_CR_ADDIS;
        while (ADC1->CR & ADC_CR_ADEN)
        {
        }
    }

    ADC1->CFGR1 = 0;
    ADC1->CFGR1 |= ADC_CFGR1_CONT;
    ADC1->CFGR1 |= ADC_CFGR1_RES_1;

    ADC1->CHSELR = ADC_CHSELR_CHSEL10;

    ADC1->CR |= ADC_CR_ADCAL;
    while (ADC1->CR & ADC_CR_ADCAL)
    {
    }

    ADC1->CR |= ADC_CR_ADEN;
    while ((ADC1->ISR & ADC_ISR_ADRDY) == 0)
    {
    }

    ADC1->CR |= ADC_CR_ADSTART;
}