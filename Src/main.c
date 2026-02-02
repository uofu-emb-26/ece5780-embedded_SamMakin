#include "main.h"
#include <stm32f0xx_hal.h>

#define ACTIVE_LAB 2   // ← change this to 1–7

int main(void)
{
    switch (ACTIVE_LAB)
    {
        case 1:
            lab1_main();
            break;

        case 2:
            lab2_main();
            break;

        case 3:
            lab3_main();
            break;

        case 4:
            lab4_main();
            break;

        case 5:
            lab5_main();
            break;

        case 6:
            lab6_main();
            break;

        case 7:
            lab7_main();
            break;

        default:
            // Trap if an invalid lab number is selected
            while (1) { }
    }
}
