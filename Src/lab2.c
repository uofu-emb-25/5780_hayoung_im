#include "main.h"

// Global tick counter for SysTick timing
volatile uint32_t tick_count = 0;

/**
 * @brief Initializes the SysTick timer and toggles LEDs based on time.
 *        - SysTick triggers every 1ms.
 *        - Red LED (PC6) toggles every 500ms in the main loop.
 *        - Blue LED (PC7) toggles every 200ms inside the SysTick interrupt.
 */
void lab2_SysTick_Timer(void) {
    // Initialize the HAL library
    HAL_Init();

    // Configure the system clock
    SystemClock_Config();

    // Enable GPIOC clock
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // Configure PC6 (Red LED) and PC7 (Blue LED) as output
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    // Configure SysTick to trigger every 1ms
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

    while (1) {
        // Toggle Red LED (PC6) every 500ms
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
        HAL_Delay(500);  // 500ms delay
    }
}


void lab2_Interrupt_Toggle(void) {

}

// Main function entry point for Lab 2
int lab2_main(void) {
    // Uncomment the following line to run the SysTick Timer version:
    lab2_SysTick_Timer();

    // Uncomment the following line to run the Interrupt Toggle version:
    // lab2_Interrupt_Toggle();
}
