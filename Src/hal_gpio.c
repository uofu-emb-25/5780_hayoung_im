#include <stdint.h>
#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_gpio.h>
#include "main.h"

void My_HAL_RCC_GPIOC_CLK_ENABLE(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

}


void MY_HAL_GPIO_Init(GPIO_TypeDef *GPIOx, uint32_t PinMask, uint32_t Mode, uint32_t Pull, uint32_t Speed) {
    for (uint8_t pin = 0; pin < 16; pin++) {
        if (PinMask & (1 << pin)) {
            GPIOx->MODER &= ~(0b11 << (pin * 2));
            GPIOx->MODER |= ((Mode & 0b11) << (pin * 2));

            if (Mode == GPIO_MODE_AF_OD) {
                GPIOx->OTYPER |= (1 << pin);
            } else {
                GPIOx->OTYPER &= ~(1 << pin);
            }


            GPIOx->OSPEEDR &= ~(0b11 << (pin * 2));
            GPIOx->OSPEEDR |= ((Speed & 0b11) << (pin * 2));

            GPIOx->PUPDR &= ~(0b11 << (pin * 2));
            GPIOx->PUPDR |= ((Pull & 0b11) << (pin * 2));

            if ((Mode == GPIO_MODE_AF_PP) || (Mode == GPIO_MODE_AF_OD)) {
                if (pin < 8) {
                    GPIOx->AFR[0] &= ~(0xF << (pin * 4));
                    GPIOx->AFR[0] |=  (0x1 << (pin * 4));  // AF1
                } else {
                    GPIOx->AFR[1] &= ~(0xF << ((pin - 8) * 4));
                    GPIOx->AFR[1] |=  (0x1 << ((pin - 8) * 4)); // AF1
                }
            }

            if (Mode == GPIO_MODE_IT_RISING 
                || Mode == GPIO_MODE_IT_FALLING 
                || Mode == GPIO_MODE_IT_RISING_FALLING) {

                uint8_t portSource = 0;
                if (GPIOx == GPIOA) portSource = 0;
                else if (GPIOx == GPIOB) portSource = 1;
                else if (GPIOx == GPIOC) portSource = 2;

                SYSCFG->EXTICR[pin / 4] &= ~(0xF << (4 * (pin % 4)));
                SYSCFG->EXTICR[pin / 4] |= (portSource << (4 * (pin % 4)));
            }
        }
    }

    // EXTI Rising
    if (Mode == GPIO_MODE_IT_RISING) {
        EXTI->RTSR |= PinMask;
        EXTI->IMR  |= PinMask;
    }
}

/*
void My_HAL_GPIO_DeInit(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin)
{
}
*/


GPIO_PinState My_HAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    return (GPIOx->IDR & GPIO_Pin) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}


void My_HAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    if (PinState == GPIO_PIN_SET) {
        GPIOx->BSRR = GPIO_Pin;
    } else {
        GPIOx->BSRR = (uint32_t)GPIO_Pin << 16;
    }
}

void My_HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    GPIOx->ODR ^= GPIO_Pin;
}

