#include <stm32f0xx_hal.h>

static uint8_t case3Stage = 0;
void lab1_main(void)
{
    static int checkOffCase = 3;
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

                // Red
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
                HAL_Delay(150);

                // Blue
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
                HAL_Delay(150);

                // Orange
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
                HAL_Delay(150);

                // Green
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
                HAL_Delay(150);

                // Reset green before looping
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
                break;

            case(3):
            case3running();
            break;


            default:
            HAL_Delay(100);
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
            HAL_Delay(100);
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
            break;
        }

    }




}

    void case3running(void)
    {
                // Turn all LEDs off first
                HAL_GPIO_WritePin(GPIOC,
                    GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9,
                    GPIO_PIN_RESET);
            switch(case3Stage)
            {
                case(0):
                // Red
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
                HAL_Delay(150);
                
                case(1):
                // Blue
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
                
                break;

                case(2):
                // Orange
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
                
                break;
                case(4):
                // Green
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
                

                // Reset green before looping
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
                break;
                
                default:
                break;
            }


    }
