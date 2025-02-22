#include "main.h"
#include "stm32f0xx_hal.h"
#include <string.h>

// UART and GPIO handles
UART_HandleTypeDef huart1;

// LED pin definitions
#define RED_LED_PIN    GPIO_PIN_6  // PC6
#define BLUE_LED_PIN   GPIO_PIN_7  // PC7
#define LED_PORT       GPIOC

// Global buffer for received data
uint8_t received_data[2];
volatile uint8_t data_ready = 0;

// Forward declarations
void SystemClock_Config(void);
void GPIO_Init(void);
void UART1_GPIO_Init(void);
void UART1_Init(void);
void Blocking_Reception(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void Process_Command(uint8_t color, uint8_t action);

int lab4_main(void)
{
    // 1) HAL and clock initialization
    HAL_Init();
    SystemClock_Config();

    // 2) Initialize GPIO for LEDs (PC6, PC7)
    GPIO_Init();

    // 3) Initialize UART1
    UART1_Init();

    // 4) blocking reception
    Blocking_Reception();

}


/* -------------------------------------------------------
   GPIO_Init: Initialize LED pins (PC6, PC7) as outputs.
   ------------------------------------------------------- */
void GPIO_Init(void)
{
    // Enable clock for GPIOC
    My_HAL_RCC_GPIOC_CLK_ENABLE();

    // Use a custom function to set up PC6 and PC7 as outputs
    MY_HAL_GPIO_Init(LED_PORT,
                     RED_LED_PIN | BLUE_LED_PIN,
                     GPIO_MODE_OUTPUT_PP,
                     GPIO_NOPULL,
                     GPIO_SPEED_FREQ_LOW);

    My_HAL_GPIO_WritePin(LED_PORT, RED_LED_PIN, GPIO_PIN_RESET);
    My_HAL_GPIO_WritePin(LED_PORT, BLUE_LED_PIN, GPIO_PIN_RESET);
}

/* -------------------------------------------------------
   UART1_GPIO_Init: Configure PA9 (TX), PA10 (RX) for USART1
   in AF mode. Adjust to match your MCU/board pin mapping.
   ------------------------------------------------------- */
void UART1_GPIO_Init(void)
{
    // Enable clock for GPIOA (assuming PA9, PA10 for USART1)
    My_HAL_RCC_GPIOA_CLK_ENABLE(); 

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // TX pin = PA9
    GPIO_InitStruct.Pin       = GPIO_PIN_9;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART1; // STM32F0: USART1 often AF1
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // RX pin = PA10
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Alternate = GPIO_AF1_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* -------------------------------------------------------
   UART1_Init: Set up USART1 at 115200 8N1, etc.
   ------------------------------------------------------- */
void UART1_Init(void)
{
    // Enable clock for USART1
    __HAL_RCC_USART1_CLK_ENABLE();

    // Configure TX/RX pins as alternate function
    UART1_GPIO_Init();

    // Set up the UART peripheral
    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = 115200;
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_UART_Init(&huart1);

    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

/* -------------------------------------------------------
   Blocking_Reception: Waits for a single character and
   toggles an LED based on 'r' or 'b'.
   ------------------------------------------------------- */
   void Blocking_Reception(void)
   {
       uint8_t input_char;
       HAL_Delay(100);
   
       while (1) {
        HAL_UART_Transmit(&huart1, (uint8_t *)"\r\nEnter command: ", 17, HAL_MAX_DELAY);
           HAL_UART_Receive(&huart1, &input_char, 1, HAL_MAX_DELAY);
           HAL_UART_Transmit(&huart1, &input_char, 1, HAL_MAX_DELAY);
           HAL_UART_Transmit(&huart1, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY);
   
           if (input_char == 'r') {
               My_HAL_GPIO_TogglePin(LED_PORT, RED_LED_PIN);
           } else if (input_char == 'b') {
               My_HAL_GPIO_TogglePin(LED_PORT, BLUE_LED_PIN);
           } else {
               HAL_UART_Transmit(&huart1, (uint8_t *)"Invalid Input\r\n", 15, HAL_MAX_DELAY);
           }
       }
   }
   