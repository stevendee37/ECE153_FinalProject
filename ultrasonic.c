#include "ultrasonic.h"
#include <stdio.h> 
 
#include "stm32l476xx.h"

uint32_t volatile currentValue = 0;
uint32_t volatile lastValue = 0;
uint32_t volatile overflowCount = 0;
uint32_t volatile timeInterval = 0;

uint32_t volatile previousInput = 1;
uint32_t volatile input = 0;
uint32_t volatile distance;
uint32_t volatile convertedTime;

uint32_t volatile test = 0;

void Input_Capture_Setup() {
	// [TODO]
	
	// Set up PB6
		// Enable clock for GPIO Port B
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	
		// Configure PB6 to be used as alternative function
	GPIOB->MODER &= ~GPIO_MODER_MODE8;
	GPIOB->MODER |= GPIO_MODER_MODE8_1;
	
	GPIOB->AFR[1] &= ~GPIO_AFRH_AFSEL8;
	GPIOB->AFR[1] |= GPIO_AFRH_AFSEL8_1;
	
	// Set PB6 to no pull-up, no pull-down
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD8;
	
	// Enable Timer 4 in RCC_APB1ENRx
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;
	
	// Set prescaler to 15
	TIM4->PSC &= ~TIM_PSC_PSC;
	TIM4->PSC |= 0xF;
	
	// Enable auto reload preload in control register
	TIM4->CR1 |= TIM_CR1_ARPE;
	
	// Set auto reload value to maximum value
	TIM4->ARR |= TIM_ARR_ARR;
	
	// Set input capture mode bits such that input capture mode bits
	// is mapped to timer input 1 in capture/compare mode register
	TIM4->CCMR2 &= ~TIM_CCMR2_CC3S;
	TIM4->CCMR2 |= TIM_CCMR2_CC3S_0;
	
	// Set bits to capture both rising/falling edges
	TIM4->CCER |= TIM_CCER_CC3P;
	TIM4->CCER |= TIM_CCER_CC3NP;
	
	// Enable Capturing
	TIM4->CCER |= TIM_CCER_CC3E;
	
	// Enable interrupt, DMA requests, and update interrupt 
	// in DMA/Interrupt enable register
	TIM4->DIER |= TIM_DIER_CC3IE;
	TIM4->DIER |= TIM_DIER_CC3DE;
	TIM4->DIER |= TIM_DIER_UIE;
	
	// Enable update generation in the event generation register
	TIM4->EGR |= TIM_EGR_UG;
	
	// Clear the update interupt flag
	TIM4->SR &= ~TIM_SR_UIF;
	
	// Set direction of the counter (upcounter)
	TIM4->CR1 &= ~TIM_CR1_DIR;
	
	// Enable counter
	TIM4->CR1 |= TIM_CR1_CEN;
	
	// Enable interrupt TIM4_IRQn, set priority to 2
	NVIC_EnableIRQ(TIM4_IRQn);
	NVIC_SetPriority(TIM4_IRQn, 2);
}

void TIM4_IRQHandler(void) {
	// [TODO]
	
	/*
	uint32_t volatile currentValue = 0;
	uint32_t volatile lastValue = 0;
	uint32_t volatile overflowCount = 0;
	uint32_t volatile timeInterval = 0;
	*/
	input = (GPIOB->IDR & GPIO_IDR_ID8) == GPIO_IDR_ID8;
	
	// If overflow flag is raised:
	if((TIM4->SR & TIM_SR_UIF) == TIM_SR_UIF){
		overflowCount++;
		TIM4->SR &= ~TIM_SR_UIF;
	}
	if((TIM4->SR & TIM_SR_CC3IF) != 0){
		// On rising edge:
		
		test = 1;
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

void Trigger_Setup() {
	// [TODO]
	
	// Set up PA9
		// Enable clock for GPIO Port A
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
		// Configure PA9 to Alternative Function Mode
	GPIOA->MODER &= ~GPIO_MODER_MODE9;
	GPIOA->MODER |= GPIO_MODER_MODE9_1;
	
		// Configure and Select the Alternative Function for PA9
	GPIOA->AFR[1] &= ~GPIO_AFRH_AFSEL9;
	GPIOA->AFR[1] |= GPIO_AFRH_AFSEL9_0;
	
		// Configure PA9 to No Pull-Up, No Pull-Down
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD9;
	
		// Set the output type of PA9 to push-pull
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT9;
	
		// Configure PA9 to Very High Output Speed
	GPIOA->OSPEEDR  &= ~GPIO_OSPEEDR_OSPEED9;
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED9;
	
	// Enable Timer 1 in RCC_APB2ENR
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
	
	// Set prescaler to 15
	TIM1->PSC &= ~TIM_PSC_PSC;
	TIM1->PSC |= 0xF;
	
	// Enable Auto Reload Preload
	TIM1->CR1 |= TIM_CR1_ARPE;
	
	// Set auto reload value to its maximal value
	TIM1->ARR |= TIM_ARR_ARR;
	
	// Set CCR value that will trigger the sensor
	TIM1->CCR2 = 0xA; // 26.4.14
	
	// Set the output compare mode such that timer operates in
	// PWM mode 1
	TIM1->CCMR1 &= ~TIM_CCMR1_OC2M;
	TIM1->CCMR1 |= TIM_CCMR1_OC2M_1;
	TIM1->CCMR1 |= TIM_CCMR1_OC2M_2;
	
	// Enable output compare preload
	TIM1->CCMR1 |= TIM_CCMR1_OC2PE;
	
	// Enable output in capture/compare enable register
	TIM1->CCER |= TIM_CCER_CC2E;
	
	// Set bits for main output enable in break and dead-time register
	TIM1->BDTR |= ( TIM_BDTR_MOE | TIM_BDTR_OSSR );
	
	// Enable update generation in the event generation register
	TIM1->EGR |= TIM_EGR_UG;
	
	// Enable update interrupt
	TIM1->DIER |= TIM_DIER_UIE;
	
	// Clear the update interrupt flag
	TIM1->SR &= ~TIM_SR_UIF;
	
	// Set direction of counter (upcounter)
	TIM1->CR1 &= ~TIM_CR1_DIR;
	
	// Enable counter
	TIM1->CR1 |= TIM_CR1_CEN;
}