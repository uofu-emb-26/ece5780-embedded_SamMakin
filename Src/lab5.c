#include <stm32f0xx_hal.h>
#include <assert.h>
#include <stdint.h>

#define I2C2_TIMING_100KHZ 0x10420F13
#define I2C2_AF_NUM        0x1u

static void leds_off(void)
{
    GPIOC->BSRR = (1u << (6u + 16u)) |
                  (1u << (7u + 16u)) |
                  (1u << (8u + 16u)) |
                  (1u << (9u + 16u));
}

static void led_on(uint32_t pin)
{
    GPIOC->BSRR = (1u << pin);
}

static void short_delay(uint32_t count)
{
    for (volatile uint32_t i = 0; i < count; i++)
    {
    }
}

static void i2c2_load_start(uint8_t addr, uint8_t nbytes, uint8_t read_mode)
{
    I2C2->CR2 &= ~((0x3FFu << 0) |
                   (0xFFu << 16) |
                   I2C_CR2_RD_WRN |
                   I2C_CR2_AUTOEND |
                   I2C_CR2_START |
                   I2C_CR2_STOP);

    I2C2->CR2 |= ((uint32_t)addr << 1);
    I2C2->CR2 |= ((uint32_t)nbytes << 16);

    if (read_mode)
    {
        I2C2->CR2 |= I2C_CR2_RD_WRN;
    }
}

void lab5_main(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    RCC->AHBENR  |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;

    RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C2RST;

    GPIOC->MODER &= ~((3u << (6u * 2u)) |
                      (3u << (7u * 2u)) |
                      (3u << (8u * 2u)) |
                      (3u << (9u * 2u)));

    GPIOC->MODER |=  ((1u << (6u * 2u)) |
                      (1u << (7u * 2u)) |
                      (1u << (8u * 2u)) |
                      (1u << (9u * 2u)));

    GPIOC->OTYPER &= ~((1u << 6u) |
                       (1u << 7u) |
                       (1u << 8u) |
                       (1u << 9u));

    leds_off();

    GPIOB->MODER &= ~((3u << (11u * 2u)) | (3u << (13u * 2u)));
    GPIOB->MODER |=  ((2u << (11u * 2u)) | (2u << (13u * 2u)));

    GPIOB->OTYPER |= (1u << 11u) | (1u << 13u);

    GPIOB->PUPDR &= ~((3u << (11u * 2u)) | (3u << (13u * 2u)));

    GPIOB->AFR[1] &= ~((0xFu << ((11u - 8u) * 4u)) |
                       (0xFu << ((13u - 8u) * 4u)));
    GPIOB->AFR[1] |=  ((I2C2_AF_NUM << ((11u - 8u) * 4u)) |
                       (I2C2_AF_NUM << ((13u - 8u) * 4u)));

    GPIOB->MODER &= ~(3u << (14u * 2u));
    GPIOB->MODER |=  (1u << (14u * 2u));
    GPIOB->OTYPER &= ~(1u << 14u);
    GPIOB->BSRR = (1u << 14u);

    GPIOC->MODER &= ~(3u << (0u * 2u));
    GPIOC->MODER |=  (1u << (0u * 2u));
    GPIOC->OTYPER &= ~(1u << 0u);
    GPIOC->BSRR = (1u << 0u);

    I2C2->TIMINGR = I2C2_TIMING_100KHZ;
    I2C2->CR1 |= I2C_CR1_PE;

    I2C2->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF;

    short_delay(50000u);

    i2c2_load_start(0x6B, 1, 0);
    I2C2->CR2 |= I2C_CR2_START;

    led_on(6u);

    short_delay(50000u);

    if (I2C2->ISR & I2C_ISR_BUSY)
    {
        led_on(7u);
    }

    if ((GPIOB->IDR & (1u << 11u)) == 0u)
    {
        led_on(8u);
    }

    if ((GPIOB->IDR & (1u << 13u)) == 0u)
    {
        led_on(9u);
    }

    while (1)
    {
    }
}