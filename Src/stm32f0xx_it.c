/**
  ******************************************************************************
  * @file    Templates/Src/stm32f0xx_it.c
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "main.h"
#include <stm32f0xx_hal.h>
#include <stm32f0xx_it.h>

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void) {
  HAL_IncTick();  // Increment global tick

  static uint32_t local_tick = 0;
  local_tick++;

  // Toggle Blue LED (PC7) every 200ms
  if (local_tick % 200 == 0) {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7);
  }
}


/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f0xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @brief  EXTI Interrupt Handler for PA0 button press.
  *         - Toggles PC8 (Green LED) and PC9 (Orange LED).
  *         - Simulates a long-running interrupt if needed.
  */
void EXTI0_1_IRQHandler(void) {
  if (__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) != RESET) {
    // Toggle Green (PC8) and Orange (PC9) LEDs
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);

    // Simulate a long-running interrupt (1.5 seconds delay)
    // Uncomment the below line when testing long-running interrupts
    for (volatile int i = 0; i < 6000000; i++);

    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
  }
}

