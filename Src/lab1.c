#include "main.h"
#include "stm32f0xx.h"

// Initialize SysTick Timer for accurate delays
void SysTick_Init(void) {
    SysTick->LOAD = (SystemCoreClock / 1000) - 1;  // 1ms interval
    SysTick->VAL  = 0;                             // Clear current value
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk |    // Use processor clock
                    SysTick_CTRL_ENABLE_Msk;        // Enable SysTick Timer
}

// Implement Manual Delay Using SysTick
void delay_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++) {
        SysTick->VAL = 0;  // Reset SysTick value
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) {
            // Wait until COUNTFLAG is set
        }
    }
}


uint8_t is_button_pressed(void) {
    static uint8_t last_state = 1;  // Button starts unpressed (logic high)
    uint8_t current_state = (GPIOA->IDR & GPIO_IDR_0) ? 1 : 0;
    
    // Detect falling edge: transition from high (1) to low (0)
    if (last_state == 1 && current_state == 0) { 
        delay_ms(50); // Debounce delay
        current_state = (GPIOA->IDR & GPIO_IDR_0) ? 1 : 0; // Re-check after delay
        if (current_state == 0) { // Confirmed press
            last_state = 0;
            return 1;
        }
    }
    // Detect rising edge: update last_state when the button is released
    else if (last_state == 0 && current_state == 1) { 
        delay_ms(50); // Debounce delay
        current_state = (GPIOA->IDR & GPIO_IDR_0) ? 1 : 0; // Re-check after delay
        if (current_state == 1) { // Confirmed release
            last_state = 1;
        }
    }
    return 0;
}

// Function - Blinking LEDs Example
void lab1_blink_example(void) {
     HAL_Init(); // Reset of all peripherals, init the Flash and Systick
    SystemClock_Config(); //Configure the system clock

    __HAL_RCC_GPIOC_CLK_ENABLE(); // Enable the GPIOC clock in theRCC
    // Set up aconfiguration struct to passtothe initialization function
    GPIO_InitTypeDef initStr ={GPIO_PIN_8 |GPIO_PIN_9,
    GPIO_MODE_OUTPUT_PP,
    GPIO_SPEED_FREQ_LOW,
    GPIO_NOPULL};
    HAL_GPIO_Init(GPIOC, &initStr); // Initialize pins PC8 & PC9
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);//Start PC8 high
     while (1) {
        HAL_Delay(200);// Delay 200ms
        // Toggle the output state of both PC8and PC9
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8 | GPIO_PIN_9);
    }
}

// Function - Blinking LEDs Using Register-Based GPIO Access (unchanged)
void lab1_blinking_leds(void) {
    // Enable GPIOC Clock (LEDs on PC6 & PC7)
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    // Configure GPIOC Pins 6 & 7 as Output Push-Pull
    GPIOC->MODER   &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7);  
    GPIOC->MODER   |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0);
    GPIOC->OTYPER  &= ~(GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7); 
    GPIOC->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7);
    GPIOC->PUPDR   &= ~(GPIO_PUPDR_PUPDR6 | GPIO_PUPDR_PUPDR7);

    // Initialize LED States: PC6 (Red LED) ON, PC7 (Blue LED) OFF
    GPIOC->BSRR = GPIO_BSRR_BS_6 | GPIO_BSRR_BR_7; 

    SysTick_Init();

    while (1) {
        delay_ms(200);
        GPIOC->BSRR = GPIO_BSRR_BS_7 | GPIO_BSRR_BR_6;  // Blue ON, Red OFF
        delay_ms(200);
        GPIOC->BSRR = GPIO_BSRR_BS_6 | GPIO_BSRR_BR_7;  // Red ON, Blue OFF
    }
}

// Function - Toggle LEDs Using Button Press (Corrected for PA0 active-low)
void lab1_button_toggle(void) {
    // Enable Clocks for GPIOC (LEDs) and GPIOA (Button on PA0)
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;  // For LED control on PC6 & PC7
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;  // For Button input on PA0

    // Configure LEDs on PC6 (Red) and PC7 (Blue) as Outputs (Push-Pull, Low Speed)
    GPIOC->MODER   &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7);
    GPIOC->MODER   |= (GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0);
    GPIOC->OTYPER  &= ~(GPIO_OTYPER_OT_6 | GPIO_OTYPER_OT_7);
    GPIOC->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7);

    // Configure PA0 (Button) as Input with Internal Pull-Up Resistor
    GPIOA->MODER   &= ~GPIO_MODER_MODER0;      // Set PA0 to input mode
    GPIOA->PUPDR   &= ~(GPIO_PUPDR_PUPDR0);      // Clear any existing pull configuration
    GPIOA->PUPDR   |= GPIO_PUPDR_PUPDR0_0;       // Enable pull-up resistor (01)

    // Initial LED State: Red ON (PC6 high), Blue OFF (PC7 low)
    GPIOC->BSRR = GPIO_BSRR_BS_6 | GPIO_BSRR_BR_7;

    SysTick_Init();

    while(1) {
        if (is_button_pressed()) {
            // Toggle LEDs: If Red (PC6) is ON, turn it OFF and turn Blue (PC7) ON, and vice versa.
            if (GPIOC->ODR & GPIO_ODR_6) {  
                GPIOC->BSRR = GPIO_BSRR_BR_6 | GPIO_BSRR_BS_7;  // Blue ON, Red OFF
            } else {                            
                GPIOC->BSRR = GPIO_BSRR_BS_6 | GPIO_BSRR_BR_7;  // Red ON, Blue OFF
            }
        }
        delay_ms(10); // Short delay to stabilize debounce checks
    }
}

// Main Entry Function
int lab1_main(void) {

    // Uncomment the following line to run the example blink version:
    lab1_blink_example();

    // Uncomment the following line to run the LED blinking version:
    //lab1_blinking_leds();

    // Uncomment the following line to run the button toggle version:
    //lab1_button_toggle();
    
}
