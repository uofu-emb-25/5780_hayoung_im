#include "main.h"

// Global TIM2 handle
TIM_HandleTypeDef htim2;

// Function prototypes
void TIM2_IRQHandler(void);
void TIM2_Init(void);


int lab3_main(void) {
    // Run Lab 3.1 - Timer Interrupt (4Hz LED Toggle)
    lab3_1_TimerInterrupt();

    
    return 0;
}

// --------------- Lab 3.1: Timer Interrupt (4Hz LED Toggle) ------------------
void lab3_1_TimerInterrupt(void) {
    // Initialize HAL and system clock
    HAL_Init();
    SystemClock_Config();

    // Enable GPIOC clock
    My_HAL_RCC_GPIOC_CLK_ENABLE();

    // Initialize PC8 and PC9 as output (LEDs)
    MY_HAL_GPIO_Init(GPIOC, GPIO_PIN_8 | GPIO_PIN_9, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW);

    // Initialize TIM2 (4Hz interrupt)
    TIM2_Init();

    // Infinite loop (LEDs will toggle in the interrupt handler)
    while (1) {
        // No operations in the main loop
    }
}

// TIM2 Initialization function
void TIM2_Init(void) {
    // Enable TIM2 clock
    __HAL_RCC_TIM2_CLK_ENABLE();

    // Configure TIM2 parameters
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 999;    // 8MHz / (999+1) = 8kHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 1999;      // 8kHz / (1999+1) = 4Hz (0.25s interrupt)
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim2);

    // Enable TIM2 interrupt
    HAL_NVIC_SetPriority(TIM2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    // Start TIM2 in interrupt mode
    HAL_TIM_Base_Start_IT(&htim2);
}

// TIM2 Interrupt Handler (executes every 0.25s)
void TIM2_IRQHandler(void) {
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET) {
        if (__HAL_TIM_GET_IT_SOURCE(&htim2, TIM_IT_UPDATE) != RESET) {
            __HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);

            // Toggle PC8 and PC9 using custom HAL functions
            My_HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
            My_HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
        }
    }
}

