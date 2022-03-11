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

#include <string.h>
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
	// Set up PB8
		// Enable clock for GPIO Port B
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	
		// Configure PB8 to be used as alternative function
	GPIOB->MODER &= ~GPIO_MODER_MODE8;
	GPIOB->MODER |= GPIO_MODER_MODE8_1;
	
	GPIOB->AFR[1] &= ~GPIO_AFRH_AFSEL8;
	GPIOB->AFR[1] |= GPIO_AFRH_AFSEL8_1;
	
	// Set PB8 to no pull-up, no pull-down
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD8;
	
	// Enable Timer 4 in RCC_APB1ENRx
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM4EN;
	
	// Set prescaler to 79
	TIM4->PSC &= ~TIM_PSC_PSC;
	TIM4->PSC |= 0x4F;
	
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
	TIM1->PSC |= 0x4F;
	
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

//////////// USART ///////////

void Init_USARTx(int x) {
	if(x == 1) {
		UART1_Init();
		UART1_GPIO_Init();
		USART_Init(USART1);
	} else if(x == 2) {
		UART2_Init();
		UART2_GPIO_Init();
		USART_Init(USART2);
	} else {
		// Do nothing...
	}
}

// MAX7219 register address
uint8_t max7219_REG_noop = 0x00;
uint8_t max7219_REG_digit0 = 0x01;
uint8_t max7219_REG_digit1 = 0x02;
uint8_t max7219_REG_digit2 = 0x03;
uint8_t max7219_REG_digit3 = 0x04;
uint8_t max7219_REG_digit4 = 0x05;
uint8_t max7219_REG_digit5 = 0x06;
uint8_t max7219_REG_digit6 = 0x07;
uint8_t max7219_REG_digit7 = 0x08;
uint8_t max7219_REG_decodeMode = 0x09;
uint8_t max7219_REG_intensity = 0x0a;
uint8_t max7219_REG_scanLimit = 0x0b;
uint8_t max7219_REG_shutdown = 0x0c;
uint8_t max7219_REG_displayTest = 0x0f;


// Control one MAX7219 module
void MAX7219_1Unit(uint8_t reg_addr, uint8_t reg_data) {

 // Before sending data set the LOAD pin to LOW
 GPIOA->ODR &= ~GPIO_ODR_OD4;
 // First, send the register address
 SPI_SendByte(reg_addr << 8 | reg_data);
	
 // After transmission, set the LOAD pin to HIGH
 GPIOA->ODR |= GPIO_ODR_OD4;
}

uint8_t matrixData_8X8_4[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b00000000,
 0b00111110,
 0b00001000,
 0b00001000,
 0b00111000,
 0b00000000,
 0b00000000

};

uint8_t matrixData_8X8_8[8] = { // Matrix pattern

 0b00000000,// row 1, top to bottom
 0b01110110,
 0b10001001,
 0b10001001,
 0b10001001,
 0b10001001,
 0b01110110,
 0b00000000

};

void Draw (uint8_t *LED_matrix) // Display data to LED matrix
{
 MAX7219_1Unit(1, LED_matrix[0]);
 MAX7219_1Unit(2, LED_matrix[1]);
 MAX7219_1Unit(3, LED_matrix[2]);
 MAX7219_1Unit(4, LED_matrix[3]);
 MAX7219_1Unit(5, LED_matrix[4]);
 MAX7219_1Unit(6, LED_matrix[5]);
 MAX7219_1Unit(7, LED_matrix[6]);
 MAX7219_1Unit(8, LED_matrix[7]);
}

void setup ( ) {

 // Initialize MAX7219 registers
 MAX7219_1Unit(max7219_REG_scanLimit, 0x07);// set to scan all row
 // Disable decoding
 MAX7219_1Unit(max7219_REG_decodeMode, 0x00);
 // Not shutdown mode
 MAX7219_1Unit(max7219_REG_shutdown, 0x01);
 // Not test mode
 MAX7219_1Unit(max7219_REG_displayTest, 0x00);

 for(int i=1; i<=8; i++) {// Set all LED intensity to low
 MAX7219_1Unit(i,0);
 }

 // Set LED intensity range: 0x00 ~ 0x0f
 MAX7219_1Unit(max7219_REG_intensity, 0x0a);
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
	
	Draw(matrixData_8X8_4);
	
	while(1) {
		// [TODO] Store your measurements on Stack
		if((timeInterval < 150) || (timeInterval > 25000)){
			distance = 0;
		}
		else{
			distance = timeInterval/58;
			printf("%i\n", distance);
		}
	}
	
	
	
}
