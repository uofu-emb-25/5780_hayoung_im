#include "main.h"
#include "stm32f0xx_hal.h"

// Global Timer handles
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

// Variables for PWM fade effect
volatile uint16_t pwm_value = 0;
volatile int pwm_direction = 1;  // 1: increase, -1: decrease

// Function prototypes
void TIM2_IRQHandler(void);
void TIM2_Init(void);
void lab3_PWM_Init(void);
void lab3_1_TimerInterrupt(void);
void lab3_3_PWMGeneration(void);
void TIM3_IRQHandler(void);

int lab3_main(void) {
    // Run Lab 3.1 - Timer Interrupt (4Hz LED Toggle)
    lab3_1_TimerInterrupt();

    // Run Lab 3.3 - PWM Generation using TIM3
    // lab3_3_PWMGeneration();

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
    htim2.Init.Prescaler = 2999; 
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 1999;      
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

// --------------- Lab 3.3: PWM Generation using TIM3 (Interrupt-Based) ---------------
void lab3_3_PWMGeneration(void) {
    HAL_Init();
    SystemClock_Config();    // System clock setup; assume 8MHz on APB1

    // Configure & start TIM3 in PWM mode on PC6/PC7 
    lab3_PWM_Init();

    // Enable TIM3 interrupt for PWM update
    HAL_NVIC_SetPriority(TIM3_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    while (1) {
        // No delay needed, PWM updates handled in TIM3_IRQHandler
    }
}

// Initialize TIM3 for PWM output on PC6 & PC7
void lab3_PWM_Init(void) {
    // Enable TIM3 clock & GPIOC clock
    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // Configure PC6 (CH1) & PC7 (CH2) as Alternate Function, Push-Pull
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;  // PC6 (CH1), PC7 (CH2)
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;   
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF0_TIM3;

    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    // Configure TIM3 for 800 Hz PWM
    htim3.Instance = TIM3;
    htim3.Init.Prescaler         = 59;
    htim3.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim3.Init.Period            = 999;
    htim3.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim3);

    // Configure PWM Channels
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode       = TIM_OCMODE_PWM2; 
    sConfigOC.Pulse        = 200;  // 20% duty cycle (initial)
    sConfigOC.OCPolarity   = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode   = TIM_OCFAST_DISABLE;

    // CH1 config (PC6)
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);

    // Switch to PWM1 for CH2 (PC7)
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2);

    // Start PWM on both channels with Interrupt
    HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_2);
}

// TIM3 Interrupt Handler: Gradually Adjust PWM Duty Cycle
void TIM3_IRQHandler(void) {
    if (__HAL_TIM_GET_FLAG(&htim3, TIM_FLAG_UPDATE) != RESET) {
        if (__HAL_TIM_GET_IT_SOURCE(&htim3, TIM_IT_UPDATE) != RESET) {
            __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);

            // Adjust PWM duty cycle gradually
            if (pwm_direction == 1) {
                pwm_value += 10;
                if (pwm_value >= 1000) pwm_direction = -1;  // Reverse direction at max
            } else {
                pwm_value -= 10;
                if (pwm_value <= 0) pwm_direction = 1;  // Reverse direction at min
            }

            // Apply new PWM duty cycle
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwm_value);
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pwm_value);
        }
    }
}
