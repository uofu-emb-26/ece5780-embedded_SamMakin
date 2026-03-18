#include "stm32f0xx.h"
#include <stdint.h>

#define LAB_MODE           2u      // 1 = 5.1 WHO_AM_I demo, 2 = 5.2 motion LED demo

#define IMU_I2C_ADDR       0x69u

#define WHO_REG            0x0Fu
#define CTRL1_REG          0x20u
#define XLOW_REG           0xA8u
#define YLOW_REG           0xAAu

#define WHO_OK_A           0xD3u
#define WHO_OK_B           0xD4u

#define CTRL1_ENABLE_VAL   0x0Bu
#define MOTION_LIMIT       500

#define LED_A              6u
#define LED_B              7u
#define LED_C              8u
#define LED_D              9u


static void wait_ms(uint32_t t)
{
    for (uint32_t i = 0; i < (t * 800u); i++) {
        __asm__("nop");
    }
}

static void leds_off_all(void)
{
    GPIOC->BSRR =
        (1u << (LED_A + 16u)) |
        (1u << (LED_B + 16u)) |
        (1u << (LED_C + 16u)) |
        (1u << (LED_D + 16u));
}

static void leds_apply(uint32_t mask)
{
    leds_off_all();
    GPIOC->BSRR = mask;
}

static void led_single(uint32_t pin)
{
    leds_apply(1u << pin);
}

static void led_pair(uint32_t p1, uint32_t p2)
{
    leds_apply((1u << p1) | (1u << p2));
}

static void leds_full(void)
{
    GPIOC->BSRR =
        (1u << LED_A) |
        (1u << LED_B) |
        (1u << LED_C) |
        (1u << LED_D);
}

static void startup_led_flash(void)
{
    leds_full();
    wait_ms(1000);
    leds_off_all();
    wait_ms(500);
}

static void led_gpio_init(void)
{
    GPIOC->MODER &= ~(
        (3u << (LED_A * 2u)) |
        (3u << (LED_B * 2u)) |
        (3u << (LED_C * 2u)) |
        (3u << (LED_D * 2u))
    );

    GPIOC->MODER |= (
        (1u << (LED_A * 2u)) |
        (1u << (LED_B * 2u)) |
        (1u << (LED_C * 2u)) |
        (1u << (LED_D * 2u))
    );

    GPIOC->OTYPER &= ~(
        (1u << LED_A) |
        (1u << LED_B) |
        (1u << LED_C) |
        (1u << LED_D)
    );
}

static void imu_mode_pins(void)
{
    GPIOB->MODER  &= ~(3u << (14u * 2u));
    GPIOB->MODER  |=  (1u << (14u * 2u));
    GPIOB->OTYPER &= ~(1u << 14u);
    GPIOB->ODR    |=  (1u << 14u);

    GPIOC->MODER  &= ~(3u << (0u * 2u));
    GPIOC->MODER  |=  (1u << (0u * 2u));
    GPIOC->OTYPER &= ~(1u << 0u);
    GPIOC->ODR    |=  (1u << 0u);
}

static void i2c_pin_setup(void)
{
    GPIOB->MODER &= ~((3u << (11u * 2u)) | (3u << (13u * 2u)));
    GPIOB->MODER |=  ((2u << (11u * 2u)) | (2u << (13u * 2u)));

    GPIOB->OTYPER |= (1u << 11u) | (1u << 13u);

    GPIOB->AFR[1] &= ~((0xFu << 12u) | (0xFu << 20u));
    GPIOB->AFR[1] |=  ((1u << 12u) | (5u << 20u));

    GPIOB->PUPDR &= ~((3u << (11u * 2u)) | (3u << (13u * 2u)));
}

static void i2c2_enable(void)
{
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C2RST;

    I2C2->CR1 = 0;
    I2C2->CR2 = 0;
    I2C2->TIMINGR = 0;

    I2C2->CR1 |= I2C_CR1_PE;

    I2C2->TIMINGR |= (1u << 28);
    I2C2->TIMINGR |= (0x13u);
    I2C2->TIMINGR |= (0xFu << 8);
    I2C2->TIMINGR |= (2u << 16);
    I2C2->TIMINGR |= (4u << 20);

    I2C2->ICR = I2C_ICR_NACKCF | I2C_ICR_STOPCF;
}

static void hw_init_all(void)
{
    RCC->AHBENR  |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

    led_gpio_init();
    leds_off_all();

    i2c_pin_setup();
    imu_mode_pins();
    i2c2_enable();
}

static int wait_tx(void)
{
    while (1) {
        uint32_t s = I2C2->ISR;

        if (s & I2C_ISR_NACKF) {
            I2C2->ICR = I2C_ICR_NACKCF;
            I2C2->CR2 |= I2C_CR2_STOP;
            return 0;
        }

        if (s & I2C_ISR_TXIS) {
            return 1;
        }
    }
}

static int wait_rx(void)
{
    while (1) {
        uint32_t s = I2C2->ISR;

        if (s & I2C_ISR_NACKF) {
            I2C2->ICR = I2C_ICR_NACKCF;
            I2C2->CR2 |= I2C_CR2_STOP;
            return 0;
        }

        if (s & I2C_ISR_RXNE) {
            return 1;
        }
    }
}

static void wait_done(void)
{
    while (!(I2C2->ISR & I2C_ISR_TC)) {
    }
}

static void i2c_start_cfg(uint8_t addr7, uint8_t bytes, uint8_t rd)
{
    uint32_t r = I2C2->CR2;

    r &= ~((0x3FFu << 0) |
           (0xFFu << 16) |
           I2C_CR2_RD_WRN |
           I2C_CR2_AUTOEND |
           I2C_CR2_START |
           I2C_CR2_STOP);

    r |= ((uint32_t)addr7 << 1);
    r |= ((uint32_t)bytes << 16);

    if (rd) {
        r |= I2C_CR2_RD_WRN;
    }

    I2C2->CR2 = r;
}

static int imu_write(uint8_t reg, uint8_t val)
{
    i2c_start_cfg(IMU_I2C_ADDR, 2u, 0u);
    I2C2->CR2 |= I2C_CR2_START;

    if (!wait_tx()) return 0;
    I2C2->TXDR = reg;

    if (!wait_tx()) return 0;
    I2C2->TXDR = val;

    wait_done();
    I2C2->CR2 |= I2C_CR2_STOP;

    return 1;
}

static int imu_read(uint8_t reg, uint8_t *buf, uint8_t n)
{
    i2c_start_cfg(IMU_I2C_ADDR, 1u, 0u);
    I2C2->CR2 |= I2C_CR2_START;

    if (!wait_tx()) return 0;
    I2C2->TXDR = reg;

    wait_done();

    i2c_start_cfg(IMU_I2C_ADDR, n, 1u);
    I2C2->CR2 |= I2C_CR2_START;

    for (uint8_t i = 0; i < n; i++) {
        if (!wait_rx()) return 0;
        buf[i] = (uint8_t)I2C2->RXDR;
    }

    wait_done();
    I2C2->CR2 |= I2C_CR2_STOP;

    return 1;
}

static int16_t join_bytes(uint8_t lo, uint8_t hi)
{
    uint16_t tmp = ((uint16_t)hi << 8) | lo;
    return (int16_t)tmp;
}

static int who_id_ok(uint8_t id_val)
{
    if (id_val == WHO_OK_A) return 1;
    if (id_val == WHO_OK_B) return 1;
    return 0;
}

static void show_motion(int16_t x, int16_t y)
{
    if ((x > MOTION_LIMIT) && (y > MOTION_LIMIT))
        led_pair(LED_A, LED_C);
    else if ((x > MOTION_LIMIT) && (y < -MOTION_LIMIT))
        led_pair(LED_A, LED_D);
    else if ((x < -MOTION_LIMIT) && (y > MOTION_LIMIT))
        led_pair(LED_B, LED_C);
    else if ((x < -MOTION_LIMIT) && (y < -MOTION_LIMIT))
        led_pair(LED_B, LED_D);
    else if (x > MOTION_LIMIT)
        led_single(LED_A);
    else if (x < -MOTION_LIMIT)
        led_single(LED_B);
    else if (y > MOTION_LIMIT)
        led_single(LED_C);
    else if (y < -MOTION_LIMIT)
        led_single(LED_D);
    else
        leds_off_all();
}

static void fail_comm(void)
{
    while (1) {
        led_single(LED_C);
        wait_ms(200);
        leds_off_all();
        wait_ms(200);
    }
}

static void fail_wrong_id(void)
{
    while (1) {
        led_single(LED_D);
    }
}

static void pass_51_demo(void)
{
    while (1) {
        led_pair(LED_A, LED_B);
        wait_ms(250);
        leds_off_all();
        wait_ms(250);
    }
}

static void run_lab_51(void)
{
    uint8_t id_val = 0;

    if (!imu_read(WHO_REG, &id_val, 1u)) {
        fail_comm();
    }

    if (!who_id_ok(id_val)) {
        fail_wrong_id();
    }

    pass_51_demo();
}

static void run_lab_52(void)
{
    uint8_t id_val = 0;
    uint8_t axis_raw[2];

    if (!imu_read(WHO_REG, &id_val, 1u)) {
        fail_comm();
    }

    if (!who_id_ok(id_val)) {
        fail_wrong_id();
    }

    if (!imu_write(CTRL1_REG, CTRL1_ENABLE_VAL)) {
        fail_comm();
    }

    while (1) {
        int16_t x_axis = 0;
        int16_t y_axis = 0;

        if (imu_read(XLOW_REG, axis_raw, 2u)) {
            x_axis = join_bytes(axis_raw[0], axis_raw[1]);
        }

        if (imu_read(YLOW_REG, axis_raw, 2u)) {
            y_axis = join_bytes(axis_raw[0], axis_raw[1]);
        }

        show_motion(x_axis, y_axis);
        wait_ms(100);
    }
}

void lab5_main(void)
{
    hw_init_all();
    startup_led_flash();

    if (LAB_MODE == 1u) {
        run_lab_51();
    } else {
        run_lab_52();
    }
}