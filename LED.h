#ifndef __STM32L476R_NUCLEO_LED_H
#define __STM32L476R_NUCLEO_LED_H

#include "stm32l476xx.h"

void MAX7219_1Unit(uint8_t reg_addr, uint8_t reg_data);

void Draw (uint8_t *LED_matrix);
void setup(void);

#endif
