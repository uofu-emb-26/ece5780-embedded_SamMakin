#pragma once
#include <stdint.h>

void SystemClock_Config(void);
int lab1_main(void);
int lab2_main(void);
int lab3_main(void);
int lab4_main(void);
int lab5_main(void);
int lab6_main(void);
int lab7_main(void);

// Functions -Sam Makin
void HAL_RCC_GPIOC_CLK_Enable(void);
void HAL_RCC_GPIOA_CLK_Enable(void);


void hal_gpio_init_button_pa0_pulldown(void);
void hal_exti0_enable_rising(void);

