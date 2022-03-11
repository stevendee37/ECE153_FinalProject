#include "SPI.h"

// MOSI and NSS
// PA4 PA5 PA7

// Note: When the data frame size is 8 bit, "SPIx->DR = byte_data;" works incorrectly. 
// It mistakenly send two bytes out because SPIx->DR has 16 bits. To solve the program,
// we should use "*((volatile uint8_t*)&SPIx->DR) = byte_data";

void SPI1_GPIO_Init(void) {
	// TODO: initialize SPI1 GPIO pins
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
	// Configure pins to operate in alternate funciton mode
	GPIOA->MODER &= ~GPIO_MODER_MODER4;
	GPIOA->MODER &= ~GPIO_MODER_MODER5;
	GPIOA->MODER &= ~GPIO_MODER_MODER7;
	GPIOA->MODER |= GPIO_MODER_MODER4_1;
	GPIOA->MODER |= GPIO_MODER_MODER5_1;
	GPIOA->MODER |= GPIO_MODER_MODER7_1;
	
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL4_2;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL4_0;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL5_2;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL5_0;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL7_2;
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL7_0;
	
	// Set output type to push-pull
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT4;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT5;
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT7;
	
	// Set output speed to low
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED4;
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED5;
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED7;
	
	// Set no pull up/down
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD4;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5;
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD7;
}

void SPI1_Init(void){
	// TODO: initialize SPI1 peripheral
	
	// Enable SPI1 Clock
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	
	// Set and clear SPI1 Reset
	RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;
	RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;
	
	// Disable SPI1
	SPI1->CR1 &= ~SPI_CR1_SPE;
	
	SPI1->CR1 &= ~SPI_CR1_RXONLY;
	
	SPI1->CR1 &= ~SPI_CR1_BIDIOE;
	SPI1->CR2 |= SPI_CR2_NSSP;
	SPI1->CR2 |= SPI_CR2_FRXTH;
	SPI1->CR2 |= SPI_CR2_SSOE;
	
	// Set SPI1 to master mode
	SPI1->CR1 |= SPI_CR1_MSTR;
	
	// Set SPI1 to 2-line
	SPI1->CR1 &= ~SPI_CR1_BIDIMODE;
	
	// Set SPI1 Data Size to 16 bit
	SPI1->CR2 |= SPI_CR2_DS;
	
	// Set SPI1 Polarity to Low (0)
	SPI1->CR1 &= ~SPI_CR1_CPOL;
	
	// Set SPI1 Clock Phase to 1 Edge
	SPI1->CR1 &= ~SPI_CR1_CPHA;
	
	// Enable software SSM
	SPI1->CR1 |= SPI_CR1_SSM;
	
	// Set baud rate to 256
	SPI1->CR1 |= SPI_CR1_BR;
	// BR_1 and BR_0
	
	// Set MSB first
	SPI1->CR1 &= ~SPI_CR1_LSBFIRST;
	
	// Set NOT TI Mode
	SPI1->CR2 &= ~SPI_CR2_FRF;
	
	// SPI_CR1_SPE
	SPI1->CR1 |= SPI_CR1_SPE;
	
}
 
void SPI_SendByte(uint16_t write_data) {
	// TODO: perform SPI transfer
	while((SPI1->SR & SPI_SR_TXE) == 0);
	
	*((volatile uint16_t*)&SPI1->DR) = write_data;
	
	while((SPI1->SR & SPI_SR_BSY) != 0);
}