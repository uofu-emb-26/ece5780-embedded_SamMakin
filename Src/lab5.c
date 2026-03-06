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
    for (volatile uint32_t i = 0; i < count; i++) {
    }
}

static int wait_txis(void)
{
    while (1) {
        uint32_t isr = I2C2->ISR;

        // if slave did not answer just stop out
        if (isr & I2C_ISR_NACKF) {
            I2C2->ICR = I2C_ICR_NACKCF;
            I2C2->CR2 |= I2C_CR2_STOP;
            return 0;
        }

        // ready to send next byte
        if (isr & I2C_ISR_TXIS) {
            return 1;
        }
    }
}

static int wait_rxne(void)
{
    while (1) {
        uint32_t isr = I2C2->ISR;

        // if no ack just bail
        if (isr & I2C_ISR_NACKF) {
            I2C2->ICR = I2C_ICR_NACKCF;
            I2C2->CR2 |= I2C_CR2_STOP;
            return 0;
        }

        // byte came in
        if (isr & I2C_ISR_RXNE) {
            return 1;
        }
    }
}

static void wait_tc(void)
{
    // wait until transfer fully done
    while (!(I2C2->ISR & I2C_ISR_TC)) {
    }
}

static void i2c2_load_start(uint8_t addr, uint8_t nbytes, uint8_t read_mode)
{
    // clear old transaction fields
    I2C2->CR2 &= ~((0x3FFu << 0) |
                   (0xFFu << 16) |
                   I2C_CR2_RD_WRN |
                   I2C_CR2_AUTOEND |
                   I2C_CR2_START |
                   I2C_CR2_STOP);

    I2C2->CR2 |= ((uint32_t)addr << 1);
    I2C2->CR2 |= ((uint32_t)nbytes << 16);

    if (read_mode) {
        I2C2->CR2 |= I2C_CR2_RD_WRN;
    }
}

static int gyro_write_reg(uint8_t addr7, uint8_t reg, uint8_t val)
{
    i2c2_load_start(addr7, 2, 0);
    I2C2->CR2 |= I2C_CR2_START;

    if (!wait_txis()) return 0;
    I2C2->TXDR = reg;

    if (!wait_txis()) return 0;
    I2C2->TXDR = val;

    wait_tc();
    I2C2->CR2 |= I2C_CR2_STOP;
    return 1;
}

static int gyro_read_reg(uint8_t addr7, uint8_t reg, uint8_t *buf, uint8_t len)
{
    // first write register addres
    i2c2_load_start(addr7, 1, 0);
    I2C2->CR2 |= I2C_CR2_START;

    if (!wait_txis()) return 0;
    I2C2->TXDR = reg;

    wait_tc();

    // then repeated start and read back
    i2c2_load_start(addr7, len, 1);
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

    // led pins
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

    // PB11 SDA, PB13 SCL
    GPIOB->MODER &= ~((3u << (11u * 2u)) | (3u << (13u * 2u)));
    GPIOB->MODER |=  ((2u << (11u * 2u)) | (2u << (13u * 2u)));
    GPIOB->OTYPER |= (1u << 11u) | (1u << 13u);

    GPIOB->AFR[1] &= ~((0xFu << ((11u - 8u) * 4u)) |
                       (0xFu << ((13u - 8u) * 4u)));
    GPIOB->AFR[1] |=  ((I2C2_AF_NUM << ((11u - 8u) * 4u)) |
                       (I2C2_AF_NUM << ((13u - 8u) * 4u)));

    GPIOB->PUPDR &= ~((3u << (11u * 2u)) | (3u << (13u * 2u)));

    // gyro addres select high
    GPIOB->MODER &= ~(3u << (14u * 2u));
    GPIOB->MODER |=  (1u << (14u * 2u));
    GPIOB->OTYPER &= ~(1u << 14u);
    GPIOB->BSRR = (1u << 14u);

    // put gyro in i2c mode
    GPIOC->MODER &= ~(3u << (0u * 2u));
    GPIOC->MODER |=  (1u << (0u * 2u));
    GPIOC->OTYPER &= ~(1u << 0u);
    GPIOC->BSRR = (1u << 0u);

    I2C2->TIMINGR = I2C2_TIMING_100KHZ;
    I2C2->CR1 |= I2C_CR1_PE;
    I2C2->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF;

    short_delay(50000u);

    {
        uint8_t temp = 0;

        // just test if one register read works
        if (gyro_read_reg(0x69, 0x0F, &temp, 1)) {
            led_on(6u);
        } else {
            led_on(8u);
        }
    }

    while (1) {
    }
}