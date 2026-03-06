#include "stm32f0xx.h"
#include <stdint.h>

#define GYRO_ADDR        0x69u
#define WHO_AM_I_REG     0x0Fu
#define EXPECTED_WHO1    0xD3u
#define EXPECTED_WHO2    0xD4u

#define I2C2_TIMING_100KHZ  0x10420F13u

static void leds_off(void)
{
    GPIOC->BSRR = (1u << (6u + 16u)) |
                  (1u << (7u + 16u)) |
                  (1u << (8u + 16u)) |
                  (1u << (9u + 16u));
}

static void all_leds_on(void)
{
    GPIOC->BSRR = (1u << 6u) |
                  (1u << 7u) |
                  (1u << 8u) |
                  (1u << 9u);
}

static void led_set(uint32_t pin)
{
    leds_off();
    GPIOC->BSRR = (1u << pin);
}

static void delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms * 800u; i++) {
        __asm__("nop");
    }
}

static int wait_txis(void)
{
    while (1) {
        uint32_t isr = I2C2->ISR;

        if (isr & I2C_ISR_NACKF) {
            I2C2->ICR = I2C_ICR_NACKCF;
            I2C2->CR2 |= I2C_CR2_STOP;
            return 0;
        }

        if (isr & I2C_ISR_TXIS) {
            return 1;
        }
    }
}

static int wait_rxne(void)
{
    while (1) {
        uint32_t isr = I2C2->ISR;

        if (isr & I2C_ISR_NACKF) {
            I2C2->ICR = I2C_ICR_NACKCF;
            I2C2->CR2 |= I2C_CR2_STOP;
            return 0;
        }

        if (isr & I2C_ISR_RXNE) {
            return 1;
        }
    }
}

static void wait_tc(void)
{
    while (!(I2C2->ISR & I2C_ISR_TC)) {
    }
}

static void cr2_setup(uint8_t addr7, uint8_t nbytes, uint8_t read_mode)
{
    I2C2->CR2 &= ~((0x3FFu << 0) |
                   (0xFFu  << 16) |
                   I2C_CR2_RD_WRN |
                   I2C_CR2_AUTOEND |
                   I2C_CR2_START |
                   I2C_CR2_STOP);

    I2C2->CR2 |= ((uint32_t)addr7 << 1);
    I2C2->CR2 |= ((uint32_t)nbytes << 16);

    if (read_mode) {
        I2C2->CR2 |= I2C_CR2_RD_WRN;
    }
}

static int gyro_read_reg(uint8_t reg, uint8_t *buf, uint8_t len)
{
    cr2_setup(GYRO_ADDR, 1, 0);
    I2C2->CR2 |= I2C_CR2_START;

    if (!wait_txis()) return 0;
    I2C2->TXDR = reg;

    wait_tc();

    cr2_setup(GYRO_ADDR, len, 1);
    I2C2->CR2 |= I2C_CR2_START;

    for (uint8_t i = 0; i < len; i++) {
        if (!wait_rxne()) return 0;
        buf[i] = (uint8_t)I2C2->RXDR;
    }

    wait_tc();
    I2C2->CR2 |= I2C_CR2_STOP;

    return 1;
}

void lab5_main(void)
{
    RCC->AHBENR  |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

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

    all_leds_on();
    delay_ms(500);
    leds_off();
    delay_ms(250);

    GPIOB->MODER &= ~((3u << (11u * 2u)) | (3u << (13u * 2u)));
    GPIOB->MODER |=  ((2u << (11u * 2u)) | (2u << (13u * 2u)));
    GPIOB->OTYPER |= (1u << 11u) | (1u << 13u);

    GPIOB->AFR[1] &= ~((0xFu << ((11u - 8u) * 4u)) |
                       (0xFu << ((13u - 8u) * 4u)));
    GPIOB->AFR[1] |=  ((1u << ((11u - 8u) * 4u)) |
                       (1u << ((13u - 8u) * 4u)));

    GPIOB->PUPDR &= ~((3u << (11u * 2u)) | (3u << (13u * 2u)));

    GPIOB->MODER &= ~(3u << (14u * 2u));
    GPIOB->MODER |=  (1u << (14u * 2u));
    GPIOB->OTYPER &= ~(1u << 14u);
    GPIOB->ODR |= (1u << 14u);

    GPIOC->MODER &= ~(3u << (0u * 2u));
    GPIOC->MODER |=  (1u << (0u * 2u));
    GPIOC->OTYPER &= ~(1u << 0u);
    GPIOC->ODR |= (1u << 0u);

    I2C2->TIMINGR = I2C2_TIMING_100KHZ;
    I2C2->CR1 |= I2C_CR1_PE;
    I2C2->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF;

    delay_ms(50);

    {
        uint8_t who = 0;

        if (!gyro_read_reg(WHO_AM_I_REG, &who, 1)) {
            while (1) {
                led_set(8u);
                delay_ms(200);
                leds_off();
                delay_ms(200);
            }
        }

        if (who == EXPECTED_WHO1 || who == EXPECTED_WHO2) {
            while (1) {
                led_set(6u);
            }
        } else {
            while (1) {
                led_set(9u);
                delay_ms(500);
                leds_off();
                delay_ms(500);
            }
        }
    }
}