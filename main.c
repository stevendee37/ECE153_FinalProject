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
#include "POLL.h"
#include "LED.h"
#include "LED_configs.h"
#include "ultrasonic.h"

#include <string.h>
#include <stdio.h> 
 
#include "stm32l476xx.h"

uint32_t volatile currentValue = 0;
uint32_t volatile lastValue = 0;
uint32_t volatile overflowCount = 0;
uint32_t volatile timeInterval = 0;
uint32_t volatile input = 0;
uint32_t volatile throttlePDL_dist;

uint32_t volatile currentValue_1 = 0;
uint32_t volatile lastValue_1 = 0;
uint32_t volatile overflowCount_1 = 0;
uint32_t volatile timeInterval_1 = 0;
uint32_t volatile input_1 = 0;
uint32_t volatile brakePDL_dist;

uint32_t volatile gear = 1;
uint32_t volatile RPM = 5000;
uint32_t volatile MAX_RPM = 12000;

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

int main(void) {	
	System_Clock_Init(); // System Clock = 80 MHz
	
	// Initialize Ultrasonic Sensor
	Input_Capture_Setup();
	// Trigger Setup
	Trigger_Setup();
	
	// Initialize UART
	Init_USARTx(1);
	
	// Initialize SPI
	SPI1_Init();
	SPI1_GPIO_Init();
	setup();
	
	// Initialize Polling
	POLL_Init();
	int i = 0;
	int previousInput = 1;
	
	// Pedal distances
	int DISTANCE_RESTING = 12;
	int DISTANCE_QUARTER = 10;
	int DISTANCE_HALF = 8;
	int DISTANCE_3QUARTERS = 6;
	int DISTANCE_FULL = 4;
	
	while(1) {
		if((timeInterval < 150) || (timeInterval > 25000)){
			throttlePDL_dist = 0;
			brakePDL_dist = 0;
		}
		else{
			////////// POLLING for input //////////
			uint32_t mask = 1UL<<13;
			uint32_t input = (GPIOC->IDR & mask) == mask;
			if((previousInput == 1) && (input == 0)){
				if(i == 1){
					i = 0;
				}
				else{
					i = 1;
				}
			}
			if(i == 0){
				// Do Nothing
			}
			else if (i == 1){
				if(throttlePDL_dist <= DISTANCE_RESTING){
					if(gear < 8){
						gear++;
					}
					if(RPM > 7000){
						RPM = RPM - 2000;
						}
					else{
						RPM = 5000;
					}
				i = 0;
				}
			}
			previousInput = input;
			
			////////// Distance calculations //////////
			throttlePDL_dist = timeInterval/58;
			brakePDL_dist = timeInterval_1/58;
			
			////////// RPM behavior based on pedal input //////////
			if(brakePDL_dist >= DISTANCE_RESTING){
				if(throttlePDL_dist < DISTANCE_FULL){
					RPM += 100;
					if(RPM > MAX_RPM){
						RPM = MAX_RPM;
					}
				}
				else if(throttlePDL_dist >= DISTANCE_FULL && throttlePDL_dist < DISTANCE_3QUARTERS){
					RPM += 90;
					if(RPM > 11125){
						RPM = 11125;
					}
				}
				else if(throttlePDL_dist >= DISTANCE_3QUARTERS && throttlePDL_dist < DISTANCE_HALF){
					RPM += 50;
					if(RPM > 10250){
						RPM = 10250;
					}
				}
				else if(throttlePDL_dist >= DISTANCE_HALF && throttlePDL_dist < DISTANCE_QUARTER){
					RPM += 10;
					if(RPM > 9375){
						RPM = 9375;
					}
				}
				else if(throttlePDL_dist >= DISTANCE_QUARTER && throttlePDL_dist < DISTANCE_RESTING){
					RPM += 10;
					if(RPM > 8500){
						RPM = 8500;
					}
				}
				else if(throttlePDL_dist >= DISTANCE_RESTING){
					RPM -= 10;
					if(RPM < 5000){
						if(gear > 1){
							RPM = 10000;
							gear--;
						}
						else{
							RPM = 5000;
						}	
					}
				}
			}
			if(brakePDL_dist < DISTANCE_FULL){
				RPM -= RPM/35;
				if(RPM < 5000){
					if(gear > 1){
						RPM = 10000;
						gear--;
						
					}
					else{
						RPM = 5000;
					}
				}
			}
			else if(brakePDL_dist >= DISTANCE_FULL && brakePDL_dist < DISTANCE_3QUARTERS){
				RPM -= RPM/45;
				if(RPM < 5000){
					if(gear > 1){
						RPM = 10000;
						gear--;
						
					}
					else{
						RPM = 5000;
					}
				}
			}
			else if(brakePDL_dist >= DISTANCE_3QUARTERS && brakePDL_dist < DISTANCE_HALF){
				RPM -= RPM/50;
				if(RPM < 5000){
					if(gear > 1){
						RPM = 10000;
						gear--;
						
					}
					else{
						RPM = 5000;
					}
				}
			}
			else if(brakePDL_dist >= DISTANCE_HALF && brakePDL_dist < DISTANCE_QUARTER){
				RPM -= RPM/75;
				if(RPM < 5000){
					if(gear > 1){
						RPM = 10000;
						gear--;
						
					}
					else{
						RPM = 5000;
					}
				}
			}
			else if(brakePDL_dist >= DISTANCE_QUARTER && brakePDL_dist < DISTANCE_RESTING){
				RPM -= RPM/85;
				if(RPM < 5000){
					if(gear > 1){
						RPM = 10000;
						gear--;
						
					}
					else{
						RPM = 5000;
					}
				}
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
