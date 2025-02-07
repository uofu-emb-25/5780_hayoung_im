#include "main.h"

// Global tick counter for SysTick timing
volatile uint32_t tick_count = 0;

/**
 * @brief Initializes the SysTick timer and toggles LEDs based on time.
 *        - SysTick triggers every 1ms.
 *        - The Red LED (PC6) toggles every 500ms in the main loop.
 *        - The Blue LED (PC7) toggles every 200ms inside the SysTick interrupt.
 */
void lab2_SysTick_Timer(void) {
    HAL_Init();
    SystemClock_Config();

    __HAL_RCC_GPIOC_CLK_ENABLE();

    // Corrected function usage
    MY_HAL_GPIO_Init(GPIOC, GPIO_PIN_6 | GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

    while (1) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
        HAL_Delay(500);
    }
}


/**
 * @brief Configures EXTI (External Interrupt) to toggle LEDs when a button is pressed.
 *        - PA0 (Button) is set as an external interrupt input.
 *        - PC8 (Green LED) and PC9 (Orange LED) toggle when the button is pressed.
 *        - EXTI interrupt is enabled and configured.
 */
void lab2_Interrupt_Toggle(void) {
    HAL_Init();
    SystemClock_Config();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    // Configure PA0 as input with EXTI
    MY_HAL_GPIO_Init(GPIOA, GPIO_PIN_0, GPIO_MODE_IT_RISING, GPIO_PULLDOWN, 0);

    // Configure PC8 and PC9 as output LEDs
    MY_HAL_GPIO_Init(GPIOC, GPIO_PIN_8 | GPIO_PIN_9, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);

    HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

    while (1) {
        // Interrupt toggles LEDs
    }
}


/**
 * @brief Changes interrupt priorities for SysTick and EXTI to analyze behavior.
 *        - SysTick handles Blue LED (PC7) toggling every 200ms.
 *        - Red LED (PC6) blinks every 500ms in the main loop.
 *        - EXTI (button) toggles Green LED (PC8) and Orange LED (PC9).
 */
void lab2_Interrupt_Priority(void) {
    HAL_Init();
    SystemClock_Config();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    // Configure PA0 as input with EXTI
    MY_HAL_GPIO_Init(GPIOA, GPIO_PIN_0, GPIO_MODE_IT_RISING, GPIO_PULLDOWN, 0);

    // Configure all LEDs
    MY_HAL_GPIO_Init(GPIOC, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
    HAL_NVIC_SetPriority(SysTick_IRQn, 2, 0);
    HAL_NVIC_SetPriority(EXTI0_1_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

    while (1) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
        HAL_Delay(500);
    }
}





/**
 * @brief Main function entry point for Lab 2.
 *        - Uncomment the relevant function to run either the SysTick Timer version
 *          or the External Interrupt version.
 */
int lab2_main(void) {
    // Uncomment the following line to test the SysTick Timer:
    //lab2_SysTick_Timer();

    // Uncomment the following line to test the EXTI Interrupt:
    //lab2_Interrupt_Toggle();

    // Uncomment the following line to test long-running Interrupt Priority:
    lab2_Interrupt_Priority();
}
