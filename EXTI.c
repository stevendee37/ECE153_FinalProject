
/*
 * ECE 153B - Winter 2021
 *
 * Name(s):
 * Section:
 * Lab: 2A
 */

#include "EXTI.h"
#include "LED.h"



void EXTI_Init(void) {
	// Initialize User Button
	// [TODO]
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	GPIOC->MODER &= ~GPIO_MODER_MODE13;
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD13;
	// Configure SYSCFG EXTI
	// [TODO]
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	
	SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI13;
	SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;
	
	// Configure EXTI Trigger
	// [TODO]
	EXTI->RTSR1 &= ~EXTI_RTSR1_RT13;
	EXTI->FTSR1 |= EXTI_FTSR1_FT13;
	
	// Enable EXTI
	// [TODO]
	EXTI->IMR1 |= EXTI_IMR1_IM13;
	
	// Configure and Enable in NVIC
	// [TODO]
	NVIC_EnableIRQ(EXTI15_10_IRQn);
	NVIC_SetPriority(EXTI15_10_IRQn, 0);
}

// [TODO] Write Interrupt Handlers (look in startup_stm32l476xx.s to find the 
// interrupt handler names that you should use)

