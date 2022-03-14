#include "ultrasonic.h"
#include <stdio.h> 
 
#include "stm32l476xx.h"

void Input_Capture_Setup() {
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
