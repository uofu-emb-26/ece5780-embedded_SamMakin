#include "stm32f0xx.h"
#include <stdint.h>

static void delay_ms(uint32_t t);

static void leds_init(void);
static void leds_off(void);
static void show_value_on_leds(uint8_t value);

static void adc_pin_init(void);
static void adc_init(void);

static void dac_pin_init(void);
static void dac_init(void);

static const uint8_t sine_table[32] = {
    127,151,175,197,216,232,244,251,
    254,251,244,232,216,197,175,151,
    127,102,78,56,37,21,9,2,
    0,2,9,21,37,56,78,102
};

int lab6_main(void)
{
    HAL_Init();
    SystemClock_Config();

    leds_init();

    adc_pin_init();
    adc_init();

    dac_pin_init();
    dac_init();

    uint32_t index = 0;

    while (1)
    {
        uint8_t adc_value = (uint8_t)(ADC1->DR & 0xFFu);
        show_value_on_leds(adc_value);

        DAC->DHR8R1 = sine_table[index];

        index++;
        if (index >= 32)
        {
            index = 0;
        }

        delay_ms(1);
    }
}

static void delay_ms(uint32_t t)
{
    for (uint32_t j = 0; j < t; j++)
    {
        for (volatile uint32_t i = 0; i < 8000; i++)
        {
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

static void leds_off(void)
{
    GPIOC->BSRR =
        (1u << (6 + 16)) |
        (1u << (7 + 16)) |
        (1u << (8 + 16)) |
        (1u << (9 + 16));
}

static void show_value_on_leds(uint8_t value)
{
    leds_off();

    if (value > 50)  GPIOC->BSRR = (1u << 6);
    if (value > 100) GPIOC->BSRR = (1u << 7);
    if (value > 150) GPIOC->BSRR = (1u << 8);
    if (value > 200) GPIOC->BSRR = (1u << 9);
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

static void dac_pin_init(void)
{
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    GPIOA->MODER |= (3u << (4 * 2));
    GPIOA->PUPDR &= ~(3u << (4 * 2));
}

static void dac_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_DACEN;

    DAC->CR &= ~(DAC_CR_TEN1 | DAC_CR_TSEL1);
    DAC->CR |= DAC_CR_EN1;
}