#include "LED.h"
#include "SPI.h"

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
