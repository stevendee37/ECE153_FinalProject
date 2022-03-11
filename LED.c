#include "LED.h"

void LED_Init(void) {
	// [TODO]
	// Enable GPIO Clocks
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
	// Initialize Green LED
	GPIOA->MODER &= ~GPIO_MODER_MODE5;
	GPIOA->MODER  |= GPIO_MODER_MODE5_0;
	
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT5;
	
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5;
}

void Green_LED_Off(void) {
	// [TODO]
	GPIOA->ODR &= ~GPIO_ODR_OD5;
}

void Green_LED_On(void) {
	// [TODO]
	GPIOA->ODR |= GPIO_ODR_OD5;
}

void Green_LED_Toggle(void) {
	// [TODO]
	GPIOA-> ODR ^= GPIO_ODR_OD5;
}
