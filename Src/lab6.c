#include "stm32f0xx_hal.h"

// LED definitions (GPIOC)
#define RED     GPIO_PIN_6
#define BLUE    GPIO_PIN_7
#define ORANGE  GPIO_PIN_8
#define GREEN   GPIO_PIN_9

// ADC threshold values
#define ADC_THRESHOLD1  51
#define ADC_THRESHOLD2  102
#define ADC_THRESHOLD3  153
#define ADC_THRESHOLD4  204

// 8-bit sine wave table (32 samples)
const uint8_t sine_table[32] = {
    127, 151, 175, 197, 216, 232, 244, 251,
    254, 251, 244, 232, 216, 197, 175, 151,
    127, 102,  78,  56,  37,  21,   9,   2,
      0,   2,   9,  21,  37,  56,  78, 102
};

ADC_HandleTypeDef hadc;
DAC_HandleTypeDef hdac;

void SystemClock_Config(void);
void initLEDs(void);
void initADC(void);
void initDAC(void);

void initLEDs(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitStruct.Pin = RED | BLUE | ORANGE | GREEN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void initADC(void) {
    ADC_ChannelConfTypeDef sConfig = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();

    // Configure PC0 as analog input
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    hadc.Instance = ADC1;
    hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
    hadc.Init.Resolution = ADC_RESOLUTION_8B;
    hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
    hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    hadc.Init.ContinuousConvMode = ENABLE;
    hadc.Init.DiscontinuousConvMode = DISABLE;
    hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc.Init.DMAContinuousRequests = DISABLE;
    hadc.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;

    HAL_ADC_Init(&hadc);

    sConfig.Channel = ADC_CHANNEL_10;
    sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
    sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
    HAL_ADC_ConfigChannel(&hadc, &sConfig);

    HAL_ADC_Start(&hadc);
}

void initDAC(void) {
    DAC_ChannelConfTypeDef sConfig = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_DAC1_CLK_ENABLE();

    // Configure PA4 as analog output
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    hdac.Instance = DAC1;
    HAL_DAC_Init(&hdac);

    sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1);
    HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
}

int lab6_main(void) {
    HAL_Init();
    SystemClock_Config();

    initLEDs();
    initADC();
    initDAC();

    uint8_t adc_val;
    uint8_t sine_index = 0;

    while (1) {
        // Read ADC value
        HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);
        adc_val = (uint8_t)HAL_ADC_GetValue(&hadc);

        // LED control based on ADC thresholds
        HAL_GPIO_WritePin(GPIOC, RED,    (adc_val > ADC_THRESHOLD1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, BLUE,   (adc_val > ADC_THRESHOLD2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, ORANGE, (adc_val > ADC_THRESHOLD3) ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOC, GREEN,  (adc_val > ADC_THRESHOLD4) ? GPIO_PIN_SET : GPIO_PIN_RESET);

        // Output sine wave to DAC
        HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_8B_R, sine_table[sine_index]);
        sine_index = (sine_index + 1) % 32;

        HAL_Delay(1); // Delay 1ms for ~31Hz waveform
    }
}