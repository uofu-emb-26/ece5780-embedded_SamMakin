#include <stm32f0xx_hal.h>
#include <assert.h>

#define I2C2_TIMING_100KHZ 0x2000090E

void lab5_main(void)
{
    //enable clocks
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN;

    // Say PB11 and PB13 are in alternate fucntion mode not driven by GPIO
    GPIOB->MODER &= ~((3u << (11u * 2u)) | (3u << (13u * 2u)));
    GPIOB->MODER |=  ((2u << (11u * 2u)) | (2u << (13u * 2u)));

    // PB11 and PB13 are open drain type because if left at 0 its push pull
    GPIOB->OTYPER |= (1u << 11u) | (1u << 13u);

    // These go te the I2C2 SDA and SCL wires
    GPIOB->AFR[1] &= ~((0xFu << ((11u - 8u) * 4u)) | (0xFu << ((13u - 8u) * 4u)));
    GPIOB->AFR[1] |=  ((0x1u << ((11u - 8u) * 4u)) | (0x1u << ((13u - 8u) * 4u)));

    // This block set the address pin on the gryo high so it responds
    GPIOB->MODER &= ~(3u << (14u * 2u));
    GPIOB->MODER |=  (1u << (14u * 2u));  
    GPIOB->OTYPER &= ~(1u << 14u);         
    GPIOB->BSRR = (1u << 14u);            

    // Sets the gyro to be in the I2C
    GPIOC->MODER &= ~(3u << (0u * 2u));
    GPIOC->MODER |=  (1u << (0u * 2u));  
    GPIOC->OTYPER &= ~(1u << 0u);      //PCo to push pull    
    GPIOC->BSRR = (1u << 0u);             

    // Sets I2C to 100 kilo bits
    I2C2->TIMINGR = I2C2_TIMING_100KHZ;

    // Enable I2C peripheral
    I2C2->CR1 |= I2C_CR1_PE;

}