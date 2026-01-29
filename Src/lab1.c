#include <stm32f0xx_hal.h>
#include <assert.h>

static uint8_t case3Stage = 0;

void lab1_main(void)
{
    static int checkOffCase = 2;
    HAL_Init();

    HAL_RCC_GPIOC_CLK_Enable();
    HAL_RCC_GPIOA_CLK_Enable();

    // Set PA0 as input: MODER0[1:0] = 00
    GPIOA->MODER &= ~(0x3u << (0 * 2));

    //Maybe no pull up?
    GPIOA->PUPDR &= ~(0x3u << (0 * 2));

    GPIO_InitTypeDef initStr = {
        .Pin   = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_6 | GPIO_PIN_7,
        .Mode  = GPIO_MODE_OUTPUT_PP,
        .Pull  = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW
    };

    HAL_GPIO_Init(GPIOC, &initStr);
    assert(((GPIOC->MODER >> (6u * 2u)) & 0x3u) == 0x1u);
    assert(((GPIOC->MODER >> (7u * 2u)) & 0x3u) == 0x1u);
    assert(((GPIOC->MODER >> (8u * 2u)) & 0x3u) == 0x1u);
    assert(((GPIOC->MODER >> (9u * 2u)) & 0x3u) == 0x1u);

    static uint8_t last = 0;

    while (1)
    {
        uint8_t new = (GPIOA->IDR >> 0) & 0x1u;

        if (new && !last) {      // rising edge
            HAL_Delay(20);       // debounce
            if ((GPIOA->IDR & 0x1u)) {
                case3Stage++;
                if (case3Stage > 4) case3Stage = 0;
            }
        }

        last = new;

        switch(checkOffCase)
        {
            case(1):
                // LED on PC8 ON, PC9 OFF
                GPIOC->BSRR = GPIO_PIN_8;           // set PC8
                GPIOC->BSRR = (GPIO_PIN_9 << 16);   // reset PC9
                HAL_Delay(100);

                // LED on PC8 OFF, PC9 ON
                GPIOC->BSRR = GPIO_PIN_9;           // set PC9
                GPIOC->BSRR = (GPIO_PIN_8 << 16);   // reset PC8
                HAL_Delay(100);
                break;

            case(2):
                // Turn all LEDs off first
                HAL_GPIO_WritePin(GPIOC,
                    GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9,
                    GPIO_PIN_RESET);
                assert((GPIOC->ODR & (GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9)) == 0u);

                // Red
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
                assert((GPIOC->ODR & GPIO_PIN_6) == GPIO_PIN_6);
                HAL_Delay(150);

                // Blue
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
                assert((GPIOC->ODR & GPIO_PIN_6) == 0u);
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
                assert((GPIOC->ODR & GPIO_PIN_7) == GPIO_PIN_7);
                HAL_Delay(150);

                // Orange
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
                assert((GPIOC->ODR & GPIO_PIN_7) == 0u);
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
                assert((GPIOC->ODR & GPIO_PIN_8) == GPIO_PIN_8);
                HAL_Delay(150);

                // Green
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
                assert((GPIOC->ODR & GPIO_PIN_8) == 0u);
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
                assert((GPIOC->ODR & GPIO_PIN_9) == GPIO_PIN_9);
                HAL_Delay(150);

                // Reset green before looping
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
                assert((GPIOC->ODR & GPIO_PIN_9) == 0u);
                break;

            case(3):
                case3running();
                break;

            default: {
                HAL_Delay(100);
                uint32_t prev = GPIOC->ODR;
                HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
                assert(((GPIOC->ODR ^ prev) & GPIO_PIN_8) == GPIO_PIN_8);

                HAL_Delay(100);
                prev = GPIOC->ODR;
                HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
                assert(((GPIOC->ODR ^ prev) & GPIO_PIN_9) == GPIO_PIN_9);
                break;
            }
        }
    }
}

void case3running(void)
{
    // Turn all LEDs off first
    HAL_GPIO_WritePin(GPIOC,
        GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9,
        GPIO_PIN_RESET);
    assert((GPIOC->ODR & (GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9)) == 0u);

    switch(case3Stage)
    {
        case(0):
            // Red
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
            assert((GPIOC->ODR & GPIO_PIN_6) == GPIO_PIN_6);

            break;

        case(1):
            // Blue
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);// need to turn off pin
            assert((GPIOC->ODR & GPIO_PIN_6) == 0u);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
            assert((GPIOC->ODR & GPIO_PIN_7) == GPIO_PIN_7);
            break;

        case(2):
            // Orange
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
            assert((GPIOC->ODR & GPIO_PIN_7) == 0u);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
            assert((GPIOC->ODR & GPIO_PIN_8) == GPIO_PIN_8);
            break;

        case(4):
            // Green
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
            assert((GPIOC->ODR & GPIO_PIN_8) == 0u);
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
            assert((GPIOC->ODR & GPIO_PIN_9) == GPIO_PIN_9);

            // Reset green before looping
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
            assert((GPIOC->ODR & GPIO_PIN_9) == 0u);
            break;

        default:
            break;
    }
}

