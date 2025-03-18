#include <stm32f0xx_hal.h>
#include "main.h"
#include <assert.h>

// I3G4250D Definitions
#define GYRO_ADDRESS   (0x69 << 1)  // 7-bit I2C address shifted left (for I2C communication)
#define WHO_AM_I_REG   0x0F         // WHO_AM_I register address
#define WHO_AM_I_VAL   0xD3         // Expected WHO_AM_I value from I3G4250D

// LED Pin Definitions (on GPIOC)
#define RED     GPIO_PIN_6
#define BLUE    GPIO_PIN_7
#define ORANGE  GPIO_PIN_8
#define GREEN   GPIO_PIN_9

// Additional definition for Part 2
#define TILT_THRESHOLD 2000           // Threshold for tilt detection
#define OUT_X_L        (0x28 | 0x80)    // Starting register address for angular rate data with auto-increment

// Function prototypes for second part
void Configure_Gyro(void);
void Read_Angular_Rate_And_Update_LEDs(void);

// ----------------- WHO_AM_I Check Code (Part 1) -----------------

// Function to read WHO_AM_I and update LED state
void Read_Register(void) {
    uint32_t timeout;

    // Configure I2C2 for a read: device address, read mode, 1 byte, and generate START
    I2C2->CR2 = GYRO_ADDRESS
              | (1 << I2C_CR2_RD_WRN_Pos)      // Read mode
              | (1 << I2C_CR2_NBYTES_Pos)      // 1 byte to read
              | (1 << I2C_CR2_START_Pos);      // Generate START condition

    // Wait for RXNE (data received) or NACKF (no acknowledge) with timeout
    timeout = 100000;
    while (!(I2C2->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF)) && --timeout);
    if (timeout == 0 || (I2C2->ISR & I2C_ISR_NACKF)) {
        // Error: Turn on RED LED, clear NACK flag, and exit
        HAL_GPIO_WritePin(GPIOC, RED, GPIO_PIN_SET);
        I2C2->ICR = I2C_ICR_NACKCF;
        return;
    }

    // Read the received WHO_AM_I value
    uint8_t received = I2C2->RXDR;

    // Update LED: GREEN if correct, ORANGE if incorrect
    if (received == WHO_AM_I_VAL) {
        HAL_GPIO_WritePin(GPIOC, GREEN, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOC, ORANGE, GPIO_PIN_SET);
    }

    // Generate STOP condition to end I2C communication
    I2C2->CR2 |= I2C_CR2_STOP;
}

// Function to write the WHO_AM_I register address and then initiate a read
void Write_Register(void) {
    uint32_t timeout;

    // Configure I2C2 for writing 1 byte: clear read mode, set device address, and generate START
    I2C2->CR2 = (GYRO_ADDRESS & ~(1 << I2C_CR2_RD_WRN_Pos))
              | (1 << I2C_CR2_NBYTES_Pos)      // 1 byte to write
              | (1 << I2C_CR2_START_Pos);      // Generate START condition

    // Wait for TXIS (transmit ready) or NACKF with timeout
    timeout = 100000;
    while (!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)) && --timeout);
    if (timeout == 0 || (I2C2->ISR & I2C_ISR_NACKF)) {
        // Error: Clear NACK flag, generate STOP, and turn on RED LED
        I2C2->ICR = I2C_ICR_NACKCF;
        I2C2->CR2 |= I2C_CR2_STOP;
        HAL_GPIO_WritePin(GPIOC, RED, GPIO_PIN_SET);
        return;
    }

    // Send the WHO_AM_I register address
    I2C2->TXDR = WHO_AM_I_REG;

    // Wait until the transmission is complete (TC flag) with timeout
    timeout = 100000;
    while (!(I2C2->ISR & I2C_ISR_TC) && --timeout);
    if (timeout == 0) {
        I2C2->CR2 |= I2C_CR2_STOP;
        HAL_GPIO_WritePin(GPIOC, RED, GPIO_PIN_SET);
        return;
    }

    // Initiate the read operation to get the WHO_AM_I value
    Read_Register();
}

// ----------------- Second Part Code: Sensor Configuration and Angular Rate Reading -----------------

// Function to configure the gyroscope into measurement mode by writing to CTRL_REG1
void Configure_Gyro(void) {
    uint8_t config = 0x0F;  // Example: normal mode, all axes enabled (check datasheet for proper value)
    uint32_t timeout;

    // Configure I2C2 for writing 2 bytes: first byte is CTRL_REG1 (0x20), second is config value
    I2C2->CR2 = (GYRO_ADDRESS & ~(1 << I2C_CR2_RD_WRN_Pos))
              | (2 << I2C_CR2_NBYTES_Pos)   // 2 bytes to write
              | (1 << I2C_CR2_START_Pos);     // Generate START condition

    timeout = 100000;
    while (!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)) && --timeout);
    if (timeout == 0 || (I2C2->ISR & I2C_ISR_NACKF)) {
        I2C2->ICR = I2C_ICR_NACKCF;
        I2C2->CR2 |= I2C_CR2_STOP;
        return;
    }
    // Send CTRL_REG1 register address (0x20)
    I2C2->TXDR = 0x20;

    timeout = 100000;
    while (!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)) && --timeout);
    if (timeout == 0 || (I2C2->ISR & I2C_ISR_NACKF)) {
        I2C2->ICR = I2C_ICR_NACKCF;
        I2C2->CR2 |= I2C_CR2_STOP;
        return;
    }
    // Send configuration value
    I2C2->TXDR = config;

    timeout = 100000;
    while (!(I2C2->ISR & I2C_ISR_TC) && --timeout);
    I2C2->CR2 |= I2C_CR2_STOP;
}

// Function to read angular rate data (X, Y, Z) and update LEDs based on tilt thresholds
void Read_Angular_Rate_And_Update_LEDs(void) {
    uint8_t buffer[6];
    uint32_t timeout;
    uint8_t idx = 0;

    // Step 1: Write starting register address (OUT_X_L with auto-increment)
    I2C2->CR2 = (GYRO_ADDRESS & ~(1 << I2C_CR2_RD_WRN_Pos))
              | (1 << I2C_CR2_NBYTES_Pos)   // 1 byte to write
              | (1 << I2C_CR2_START_Pos);     // Generate START condition
    timeout = 100000;
    while (!(I2C2->ISR & (I2C_ISR_TXIS | I2C_ISR_NACKF)) && --timeout);
    if (timeout == 0 || (I2C2->ISR & I2C_ISR_NACKF)) {
        I2C2->ICR = I2C_ICR_NACKCF;
        I2C2->CR2 |= I2C_CR2_STOP;
        return;
    }
    I2C2->TXDR = OUT_X_L; // Send OUT_X_L register address (with auto-increment)

    timeout = 100000;
    while (!(I2C2->ISR & I2C_ISR_TC) && --timeout);
    I2C2->CR2 |= I2C_CR2_STOP; // End write phase

    // Step 2: Read 6 bytes from sensor (X, Y, Z: each axis low and high bytes)
    I2C2->CR2 = GYRO_ADDRESS
              | (1 << I2C_CR2_RD_WRN_Pos)      // Set to read mode
              | (6 << I2C_CR2_NBYTES_Pos)      // 6 bytes to read
              | (1 << I2C_CR2_START_Pos);      // Generate START condition
    timeout = 100000;
    idx = 0;
    while (idx < 6) {
        while (!(I2C2->ISR & (I2C_ISR_RXNE | I2C_ISR_NACKF)) && --timeout);
        if (timeout == 0) break;
        buffer[idx++] = I2C2->RXDR;
    }
    I2C2->CR2 |= I2C_CR2_STOP; // End read phase

    // Step 3: Convert the 6 bytes into 16-bit signed values (little-endian order)
    int16_t x = (int16_t)((buffer[1] << 8) | buffer[0]);
    int16_t y = (int16_t)((buffer[3] << 8) | buffer[2]);
    int16_t z = (int16_t)((buffer[5] << 8) | buffer[4]);

    // Clear all LED outputs (optional)
    HAL_GPIO_WritePin(GPIOC, RED | BLUE | GREEN | ORANGE, GPIO_PIN_RESET);

    // Update LED status based on tilt thresholds:
    // For instance, if x, y, or z exceed TILT_THRESHOLD, turn on the corresponding LED.
    if (x > TILT_THRESHOLD) {
        HAL_GPIO_WritePin(GPIOC, RED, GPIO_PIN_SET);   // X tilt positive → RED LED
    }
    if (y > TILT_THRESHOLD) {
        HAL_GPIO_WritePin(GPIOC, BLUE, GPIO_PIN_SET);    // Y tilt positive → BLUE LED
    }
    if (z > TILT_THRESHOLD) {
        HAL_GPIO_WritePin(GPIOC, GREEN, GPIO_PIN_SET);   // Z tilt positive → GREEN LED
    }
}

// ----------------- Main Function -----------------

int lab5_main(void) {
    HAL_Init();
    SystemClock_Config();

    // Enable GPIOC clock and initialize LED pins
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIOBEN;
    GPIO_InitTypeDef ledInit = {RED | BLUE | ORANGE | GREEN, GPIO_MODE_OUTPUT_PP, GPIO_SPEED_FREQ_LOW, GPIO_NOPULL};
    HAL_GPIO_Init(GPIOC, &ledInit);

    // PB14 (Slave Address setting) HIGH setting
    GPIOB->MODER |= GPIO_MODER_MODER14_0;  
    GPIOB->OTYPER &= ~GPIO_OTYPER_OT_14; 
    GPIOB->BSRR = GPIO_PIN_14;

    // PC0 (I2C activate) HIGH setting
    GPIOC->MODER |= GPIO_MODER_MODER0_0;
    GPIOC->OTYPER &= ~GPIO_OTYPER_OT_0;
    GPIOC->BSRR = GPIO_PIN_0;

    // Configure I2C2 pins: SDA on PB11 and SCL on PB13
    GPIOB->MODER &= ~(GPIO_MODER_MODER11 | GPIO_MODER_MODER13);
    GPIOB->MODER |= (GPIO_MODER_MODER11_1 | GPIO_MODER_MODER13_1);
    GPIOB->OTYPER |= (1 << 11) | (1 << 13);
    // Set alternate functions: PB11 -> AF1 for SDA, PB13 -> AF5 for SCL
    GPIOB->AFR[1] |= (1 << ((11 - 8) * 4));
    GPIOB->AFR[1] |= (5 << ((13 - 8) * 4));

    // Enable I2C2 clock and initialize I2C2 in 100kHz standard mode
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;
    I2C2->TIMINGR = 0x00201D2B;  // Timing configuration for 100kHz
    I2C2->CR1 |= I2C_CR1_PE;      // Enable I2C2

    // Perform WHO_AM_I check (Part 1)
    Write_Register();
    HAL_Delay(1000);  // Wait to observe LED (GREEN should be on if WHO_AM_I is correct)

    // Configure sensor into measurement mode (Part 2)
    Configure_Gyro();
    HAL_Delay(1000);  // Allow sensor configuration to take effect

    // Main loop: read angular rate data and update LEDs based on tilt thresholds (Part 2)
    while (1) {
        // Clear LED status before each measurement
        HAL_GPIO_WritePin(GPIOC, RED | BLUE | GREEN | ORANGE, GPIO_PIN_RESET);
        
        // Read angular rate data and update LED status
        Read_Angular_Rate_And_Update_LEDs();
        
        HAL_Delay(1000);  // Delay to observe LED changes
    }
}
