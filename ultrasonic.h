#ifndef __STM32L476R_NUCLEO_ULTRASONIC_H
#define __STM32L476R_NUCLEO_ULTRASONIC_H

#include "stm32l476xx.h"

void Input_Capture_Setup(void);
void TIM4_IRQHandler(void);
void Trigger_Setup(void);

#endif 
