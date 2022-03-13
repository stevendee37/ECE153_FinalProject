/*
 * ECE 153B - Winter 2022
 *
 * Name(s):
 * Section:
 * Lab: 3B
 */
#include "SysClock.h"
#include "UART.h"
#include "SPI.h"
#include "EXTI.h"
#include "LED.h"

#include <string.h>
#include <stdio.h> 
 
#include "stm32l476xx.h"

uint32_t volatile currentValue = 0;
uint32_t volatile lastValue = 0;
uint32_t volatile overflowCount = 0;
uint32_t volatile timeInterval = 0;
uint32_t volatile input = 0;
uint32_t volatile distance;

uint32_t volatile currentValue_1 = 0;
uint32_t volatile lastValue_1 = 0;
uint32_t volatile overflowCount_1 = 0;
uint32_t volatile timeInterval_1 = 0;
uint32_t volatile input_1 = 0;
uint32_t volatile distance_1;



uint32_t volatile gear = 1;
uint32_t volatile RPM = 5000;

void Input_Capture_Setup() {
	// [TODO]
	// Set up PB8
		// Enable clock for GPIO Port B
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
		// Configure PB8 to be used as alternative function
	GPIOB->MODER &= ~GPIO_MODER_MODE8;
	GPIOB->MODER |= GPIO_MODER_MODE8_1;
	
	GPIOA->MODER &= ~GPIO_MODER_MODE0;
	GPIOA->MODER |= GPIO_MODER_MODE0_1;
	
	GPIOB->AFR[1] &= ~GPIO_AFRH_AFSEL8;
	GPIOB->AFR[1] |= GPIO_AFRH_AFSEL8_1;
	
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL0;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL0_0;
	
	// Set PB8 to no pull-up, no pull-down
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD8;
	
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD0;
	
	// Enable Timer 4 in RCC_APB2ENRx
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;
	
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	
	// Set prescaler to 79
	TIM4->PSC &= ~TIM_PSC_PSC;
	TIM4->PSC |= 0x4F;
	
	TIM2->PSC &= ~TIM_PSC_PSC;
	TIM2->PSC |= 0x4F;
	
	// Enable auto reload preload in control register
	TIM4->CR1 |= TIM_CR1_ARPE;
	
	TIM2->CR1 |= TIM_CR1_ARPE;
	
	// Set auto reload value to maximum value
	TIM4->ARR |= TIM_ARR_ARR;
	
	TIM2->ARR |= TIM_ARR_ARR;
	
	// Set input capture mode bits such that input capture mode bits
	// is mapped to timer input 1 in capture/compare mode register
	TIM4->CCMR2 &= ~TIM_CCMR2_CC3S;
	TIM4->CCMR2 |= TIM_CCMR2_CC3S_0;
	
	TIM2->CCMR1 &= ~TIM_CCMR1_CC1S;
	TIM2->CCMR1 |= TIM_CCMR1_CC1S_0;
	
	// Set bits to capture both rising/falling edges
	TIM4->CCER |= TIM_CCER_CC3P;
	TIM4->CCER |= TIM_CCER_CC3NP;
	
	TIM2->CCER |= TIM_CCER_CC1P;
	TIM2->CCER |= TIM_CCER_CC1NP;
	
	// Enable Capturing
	TIM4->CCER |= TIM_CCER_CC3E;
	
	TIM2->CCER |= TIM_CCER_CC1E;
	
	// Enable interrupt, DMA requests, and update interrupt 
	// in DMA/Interrupt enable register
	TIM4->DIER |= TIM_DIER_CC3IE;
	TIM4->DIER |= TIM_DIER_CC3DE;
	TIM4->DIER |= TIM_DIER_UIE;
	
	TIM2->DIER |= TIM_DIER_CC1IE;
	TIM2->DIER |= TIM_DIER_CC1DE;
	TIM2->DIER |= TIM_DIER_UIE;
	
	// Enable update generation in the event generation register
	TIM4->EGR |= TIM_EGR_UG;
	
	TIM2->EGR |= TIM_EGR_UG;
	
	// Clear the update interupt flag
	TIM4->SR &= ~TIM_SR_UIF;
	
	TIM2->SR &= ~TIM_SR_UIF;
	
	// Set direction of the counter (upcounter)
	TIM4->CR1 &= ~TIM_CR1_DIR;
	
	TIM2->CR1 &= ~TIM_CR1_DIR;
	
	// Enable counter
	TIM4->CR1 |= TIM_CR1_CEN;
	
	TIM2->CR1 |= TIM_CR1_CEN;
	
	// Enable interrupt TIM4_IRQn, set priority to 2
	NVIC_EnableIRQ(TIM4_IRQn);
	NVIC_SetPriority(TIM4_IRQn, 2);
	
	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_SetPriority(TIM2_IRQn, 2);
}

void TIM4_IRQHandler(void) {
	input = (GPIOB->IDR & GPIO_IDR_ID8) == GPIO_IDR_ID8;
	
	// If overflow flag is raised:
	if((TIM4->SR & TIM_SR_UIF) == TIM_SR_UIF){
		overflowCount++;
		TIM4->SR &= ~TIM_SR_UIF;
	}
	if((TIM4->SR & TIM_SR_CC3IF) != 0){
		// On rising edge:
		if(input == 1){
			currentValue = TIM4->CCR3;
		}
	
		// On falling edge:
		else if(input == 0){
			lastValue = currentValue;
			currentValue = TIM4->CCR3;
			if(overflowCount == 0){
				timeInterval = currentValue - lastValue;
			}
			else{
				timeInterval = currentValue - lastValue;
				timeInterval += overflowCount * TIM_ARR_ARR;
				overflowCount =0;
		
			}
		}
	}
}

void TIM2_IRQHandler(void) {
	input_1 = (GPIOA->IDR & GPIO_IDR_ID0) == GPIO_IDR_ID0;
	
	// If overflow flag is raised:
	if((TIM2->SR & TIM_SR_UIF) == TIM_SR_UIF){
		overflowCount_1++;
		TIM2->SR &= ~TIM_SR_UIF;
	}
	if((TIM2->SR & TIM_SR_CC1IF) != 0){
		// On rising edge:
		if(input_1 == 1){
			currentValue_1 = TIM2->CCR1;
		}
	
		// On falling edge:
		else if(input_1 == 0){
			lastValue_1 = currentValue_1;
			currentValue_1 = TIM2->CCR1;
			if(overflowCount_1 == 0){
				timeInterval_1 = currentValue_1 - lastValue_1;
			}
			else{
				timeInterval_1 = currentValue_1 - lastValue_1;
				timeInterval_1 += overflowCount_1 * TIM_ARR_ARR;
				overflowCount_1 = 0;
		
			}
		}
	}
}


void Trigger_Setup() {
	// [TODO]
	
	// Set up PA9
		// Enable clock for GPIO Port A
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
		// Configure PA9 to Alternative Function Mode
	GPIOA->MODER &= ~GPIO_MODER_MODE9;
	GPIOA->MODER |= GPIO_MODER_MODE9_1;
	
	GPIOA->MODER &= ~GPIO_MODER_MODE2;
	GPIOA->MODER |= GPIO_MODER_MODE2_1;
	
		// Configure and Select the Alternative Function for PA9
	GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL9;
	GPIOA->AFR[1] |= GPIO_AFRH_AFSEL9_0;
	
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL2;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL2_1;
	
		// Configure PA9 to No Pull-Up, No Pull-Down
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD9;
	
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD2;
	
		// Set the output type of PA9 to push-pull
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT9;
	
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT2;
	
		// Configure PA9 to Very High Output Speed
	GPIOA->OSPEEDR  &= ~GPIO_OSPEEDR_OSPEED9;
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED9;
	
	GPIOA->OSPEEDR  &= ~GPIO_OSPEEDR_OSPEED2;
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED2;
	
	// Enable Timer 1 in RCC_APB2ENR
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN;
	
	// Set prescaler to 15
	TIM1->PSC &= ~TIM_PSC_PSC;
	TIM1->PSC |= 0x4F;
	
	TIM5->PSC &= ~TIM_PSC_PSC;
	TIM5->PSC |= 0x4F;
	
	// Enable Auto Reload Preload
	TIM1->CR1 |= TIM_CR1_ARPE;
	
	TIM5->CR1 |= TIM_CR1_ARPE;
	
	// Set auto reload value to its maximal value
	TIM1->ARR |= TIM_ARR_ARR;
	
	TIM5->ARR |= TIM_ARR_ARR;
	
	// Set CCR value that will trigger the sensor
	TIM1->CCR2 = 0xA; // 26.4.14
	
	TIM5->CCR2 = 0xA; // 26.4.14
	
	// Set the output compare mode such that timer operates in
	// PWM mode 1
	TIM1->CCMR1 &= ~TIM_CCMR1_OC2M;
	TIM1->CCMR1 |= TIM_CCMR1_OC2M_1;
	TIM1->CCMR1 |= TIM_CCMR1_OC2M_2;
	
	TIM5->CCMR2 &= ~TIM_CCMR2_OC3M;
	TIM5->CCMR2 |= TIM_CCMR2_OC3M_1;
	TIM5->CCMR2 |= TIM_CCMR2_OC3M_2;
	
	// Enable output compare preload
	TIM1->CCMR1 |= TIM_CCMR1_OC2PE;
	
	TIM5->CCMR2 |= TIM_CCMR2_OC3PE;
	
	// Enable output in capture/compare enable register
	TIM1->CCER |= TIM_CCER_CC2E;
	
	TIM5->CCER |= TIM_CCER_CC3E;
	
	// Set bits for main output enable in break and dead-time register
	TIM1->BDTR |= ( TIM_BDTR_MOE | TIM_BDTR_OSSR );
	
	TIM5->BDTR |= ( TIM_BDTR_MOE | TIM_BDTR_OSSR );
	
	// Enable update generation in the event generation register
	TIM1->EGR |= TIM_EGR_UG;
	
	TIM5->EGR |= TIM_EGR_UG;
	
	// Enable update interrupt
	TIM1->DIER |= TIM_DIER_UIE;
	
	TIM5->DIER |= TIM_DIER_UIE;
	
	// Clear the update interrupt flag
	TIM1->SR &= ~TIM_SR_UIF;
	
	TIM5->SR &= ~TIM_SR_UIF;
	
	// Set direction of counter (upcounter)
	TIM1->CR1 &= ~TIM_CR1_DIR;
	
	TIM5->CR1 &= ~TIM_CR1_DIR;
	
	// Enable counter
	TIM1->CR1 |= TIM_CR1_CEN;
	
	TIM5->CR1 |= TIM_CR1_CEN;
}

//////////// USART ///////////

void Init_USARTx(int x) {
	if(x == 1) {
		UART1_Init();
		UART1_GPIO_Init();
		USART_Init(USART1);
	} else {
		// Do nothing...
	}
}

/////////// GEAR 1 
uint8_t matrixData_8X8_1_0[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00000010,
 0b00111110,
 0b00010010,
 0b00000000,
 0b00000000,
 0b00000000

};

uint8_t matrixData_8X8_1_1[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00000010,
 0b00111110,
 0b00010010,
 0b00000000,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_1_2[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00000010,
 0b00111110,
 0b10010010,
 0b10000000,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_1_3[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b10000010,
 0b10111110,
 0b10010010,
 0b10000000,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_1_4[8] = { // Matrix pattern

 0b10000000,// row 1, top to bottom
 0b10000000,
 0b10000010,
 0b10111110,
 0b10010010,
 0b10000000,
 0b10000000,
 0b10000000

};

////////// GEAR 2
uint8_t matrixData_8X8_2_0[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00111010,
 0b00101010,
 0b00101010,
 0b00101110,
 0b00000000,
 0b00000000

};

uint8_t matrixData_8X8_2_1[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00111010,
 0b00101010,
 0b00101010,
 0b00101110,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_2_2[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00111010,
 0b00101010,
 0b10101010,
 0b10101110,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_2_3[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b10111010,
 0b10101010,
 0b10101010,
 0b10101110,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_2_4[8] = { // Matrix pattern

 0b10000000,// row 1, top to bottom
 0b10000000,
 0b10111010,
 0b10101010,
 0b10101010,
 0b10101110,
 0b10000000,
 0b10000000

};

///////// GEAR 3
uint8_t matrixData_8X8_3_0[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00111110,
 0b00101010,
 0b00101010,
 0b00101010,
 0b00000000,
 0b00000000

};

uint8_t matrixData_8X8_3_1[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00111110,
 0b00101010,
 0b00101010,
 0b00101010,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_3_2[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00111110,
 0b00101010,
 0b10101010,
 0b10101010,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_3_3[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b10111110,
 0b10101010,
 0b10101010,
 0b10101010,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_3_4[8] = { // Matrix pattern

 0b10000000,// row 1, top to bottom
 0b10000000,
 0b10111110,
 0b10101010,
 0b10101010,
 0b10101010,
 0b10000000,
 0b10000000

};

/////////// GEAR 4
uint8_t matrixData_8X8_4_0[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00111110,
 0b00001000,
 0b00001000,
 0b00111000,
 0b00000000,
 0b00000000

};

uint8_t matrixData_8X8_4_1[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00111110,
 0b00001000,
 0b00001000,
 0b00111000,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_4_2[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00111110,
 0b00001000,
 0b10001000,
 0b10111000,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_4_3[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b10111110,
 0b10001000,
 0b10001000,
 0b10111000,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_4_4[8] = { // Matrix pattern

 0b10000000,// row 1, top to bottom
 0b10000000,
 0b10111110,
 0b10001000,
 0b10001000,
 0b10111000,
 0b10000000,
 0b10000000

};

///////// GEAR 5
uint8_t matrixData_8X8_5_0[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00101110,
 0b00101010,
 0b00101010,
 0b00111010,
 0b00000000,
 0b00000000

};

uint8_t matrixData_8X8_5_1[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00101110,
 0b00101010,
 0b00101010,
 0b00111010,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_5_2[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00101110,
 0b00101010,
 0b10101010,
 0b10111010,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_5_3[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b10101110,
 0b10101010,
 0b10101010,
 0b10111010,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_5_4[8] = { // Matrix pattern

 0b10000000,// row 1, top to bottom
 0b10000000,
 0b10101110,
 0b10101010,
 0b10101010,
 0b10111010,
 0b10000000,
 0b10000000

};

///////// GEAR 6
uint8_t matrixData_8X8_6_0[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00101110,
 0b00101010,
 0b00101010,
 0b00111110,
 0b00000000,
 0b00000000

};

uint8_t matrixData_8X8_6_1[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00101110,
 0b00101010,
 0b00101010,
 0b00111110,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_6_2[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00101110,
 0b00101010,
 0b10101010,
 0b10111110,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_6_3[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b10101110,
 0b10101010,
 0b10101010,
 0b10111110,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_6_4[8] = { // Matrix pattern

 0b10000000,// row 1, top to bottom
 0b10000000,
 0b10101110,
 0b10101010,
 0b10101010,
 0b10111110,
 0b10000000,
 0b10000000

};

////////// GEAR 7
uint8_t matrixData_8X8_7_0[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00110000,
 0b00101000,
 0b00100100,
 0b00100010,
 0b00000000,
 0b00000000

};

uint8_t matrixData_8X8_7_1[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00110000,
 0b00101000,
 0b00100100,
 0b00100010,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_7_2[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00110000,
 0b00101000,
 0b10100100,
 0b10100010,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_7_3[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b10110000,
 0b10101000,
 0b10100100,
 0b10100010,
 0b10000000,
 0b10000000

};

uint8_t matrixData_8X8_7_4[8] = { // Matrix pattern

 0b10000000,// row 1, top to bottom
 0b10000000,
 0b10110000,
 0b10101000,
 0b10100100,
 0b10100010,
 0b10000000,
 0b10000000

};

////////// GEAR 8
uint8_t matrixData_8X8_8_0[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00111110,
 0b00101010,
 0b00101010,
 0b00111110,
 0b00000000,
 0b00000000
};

uint8_t matrixData_8X8_8_1[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00111110,
 0b00101010,
 0b00101010,
 0b00111110,
 0b10000000,
 0b10000000
};

uint8_t matrixData_8X8_8_2[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00111110,
 0b00101010,
 0b10101010,
 0b10111110,
 0b10000000,
 0b10000000
};

uint8_t matrixData_8X8_8_3[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b10111110,
 0b10101010,
 0b10101010,
 0b10111110,
 0b10000000,
 0b10000000
};

uint8_t matrixData_8X8_8_4[8] = { // Matrix pattern

 0b10000000,// row 1, top to bottom
 0b10000000,
 0b10111110,
 0b10101010,
 0b10101010,
 0b10111110,
 0b10000000,
 0b10000000
};

void EXTI15_10_IRQHandler(void) {
	// Clear interrupt pending bit
	EXTI->PR1 |= EXTI_PR1_PIF13;
	// Define behavior that occurs when interrupt occurs
	if(gear < 8){
		gear++;
		RPM = 5000 + 50*(gear - 1);
	}
}

int main(void) {	
	System_Clock_Init(); // Switch System Clock = 80 MHz
	
	// Input Capture Setup
	Input_Capture_Setup();
	// Trigger Setup
	Trigger_Setup();
	
	
	// Initialize UART -- change the argument depending on the part you are working on
	Init_USARTx(1);
	
	SPI1_Init();
	SPI1_GPIO_Init();
	setup();
	
	EXTI_Init();
	
	// Draw(matrixData_8X8_8);
	
	while(1) {
		// [TODO] Store your measurements on Stack
		if((timeInterval < 150) || (timeInterval > 25000)){
			distance = 0;
			distance_1 = 0;
		}
		else{
			distance = timeInterval/58;
			distance_1 = timeInterval_1/58;
			if(distance < 2){
				RPM += 250;
				if(RPM > 12000){
					RPM = 12000;
				}
			}
			else if(distance >=2 && distance < 5){
				RPM += 100;
				if(RPM > 12000){
					RPM = 12000;
				}
			}
			else if(distance >=5 && distance < 8){
				RPM += 50;
				if(RPM > 11000){
					RPM = 11000;
				}
			}
			else if(distance >=8 && distance < 11){
				RPM += 10;
				if(RPM > 10000){
					RPM = 10000;
				}
			}
			else if(distance >=11){
				RPM -= 10;
				if(RPM < 5000){
					RPM = 5000;
					if(gear > 1){
						gear--;
						RPM = 12000;
					}		
				}
			}
			if(distance_1 < 2){
				RPM -= RPM/5;
				if(RPM < 5000){
					RPM = 5000;
					if(gear > 1){
						gear--;
						RPM = 12000;
					}
				}
			}
			else if(distance_1 >=2 && distance_1 < 5){
				RPM -= RPM/15;
				if(RPM < 5000){
					RPM = 5000;
					if(gear > 1){
						gear--;
						RPM = 12000;
					}
				}
			}
			else if(distance_1 >=5 && distance_1 < 8){
				RPM -= RPM/25;
				if(RPM < 5000){
					RPM = 5000;
					if(gear > 1){
						gear--;
						RPM = 12000;
					}
				}
			}
			else if(distance_1 >=8 && distance_1 < 11){
				RPM -= RPM/75;
				if(RPM < 5000){
					RPM = 5000;
					if(gear > 1){
						gear--;
						RPM = 12000;
					}
				}
			}
			else if(distance_1 >=11){
			}
			
			printf("%i", RPM);
			printf("  GEAR: %i\n", gear);
			
			if(RPM <= 8000){
				if(gear == 1){
					Draw(matrixData_8X8_1_0);
				}
				else if(gear == 2){
					Draw(matrixData_8X8_2_0);
				}
				else if(gear == 3){
					Draw(matrixData_8X8_3_0);
				}
				else if(gear == 4){
					Draw(matrixData_8X8_4_0);
				}
				else if(gear == 5){
					Draw(matrixData_8X8_5_0);
				}
				else if(gear == 6){
					Draw(matrixData_8X8_6_0);
				}
				else if(gear == 7){
					Draw(matrixData_8X8_7_0);
				}
				else if(gear == 8){
					Draw(matrixData_8X8_8_0);
				}

			}
			else if (RPM > 8000 && RPM  <= 9000){
				if(gear == 1){
					Draw(matrixData_8X8_1_1);
				}
				else if(gear == 2){
					Draw(matrixData_8X8_2_1);
				}
				else if(gear == 3){
					Draw(matrixData_8X8_3_1);
				}
				else if(gear == 4){
					Draw(matrixData_8X8_4_1);
				}
				else if(gear == 5){
					Draw(matrixData_8X8_5_1);
				}
				else if(gear == 6){
					Draw(matrixData_8X8_6_1);
				}
				else if(gear == 7){
					Draw(matrixData_8X8_7_1);
				}
				else if(gear == 8){
					Draw(matrixData_8X8_8_1);
				}
			}
			else if(RPM > 9000 && RPM <= 10000){
				if(gear == 1){
					Draw(matrixData_8X8_1_2);
				}
				else if(gear == 2){
					Draw(matrixData_8X8_2_2);
				}
				else if(gear == 3){
					Draw(matrixData_8X8_3_2);
				}
				else if(gear == 4){
					Draw(matrixData_8X8_4_2);
				}
				else if(gear == 5){
					Draw(matrixData_8X8_5_2);
				}
				else if(gear == 6){
					Draw(matrixData_8X8_6_2);
				}
				else if(gear == 7){
					Draw(matrixData_8X8_7_2);
				}
				else if(gear == 8){
					Draw(matrixData_8X8_8_2);
				}
			}				
			else if(RPM > 10000 && RPM <= 11000){
				if(gear == 1){
					Draw(matrixData_8X8_1_3);
				}
				else if(gear == 2){
					Draw(matrixData_8X8_2_3);
				}
				else if(gear == 3){
					Draw(matrixData_8X8_3_3);
				}
				else if(gear == 4){
					Draw(matrixData_8X8_4_3);
				}
				else if(gear == 5){
					Draw(matrixData_8X8_5_3);
				}
				else if(gear == 6){
					Draw(matrixData_8X8_6_3);
				}
				else if(gear == 7){
					Draw(matrixData_8X8_7_3);
				}
				else if(gear == 8){
					Draw(matrixData_8X8_8_3);
				}
			}	
			else if(RPM > 11000 && RPM <= 12000){
				if(gear == 1){
					Draw(matrixData_8X8_1_4);
				}
				else if(gear == 2){
					Draw(matrixData_8X8_2_4);
				}
				else if(gear == 3){
					Draw(matrixData_8X8_3_4);
				}
				else if(gear == 4){
					Draw(matrixData_8X8_4_4);
				}
				else if(gear == 5){
					Draw(matrixData_8X8_5_4);
				}
				else if(gear == 6){
					Draw(matrixData_8X8_6_4);
				}
				else if(gear == 7){
					Draw(matrixData_8X8_7_4);
				}
				else if(gear == 8){
					Draw(matrixData_8X8_8_4);
				}
			}				
		}
	}
	
	
	
}
