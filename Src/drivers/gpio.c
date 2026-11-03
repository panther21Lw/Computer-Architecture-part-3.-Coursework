/*
 * gpio.c
 *
 *  Created on: Nov 3, 2026
 *      Author: ADMIN
 */

#include "drivers/gpio.h"

/**
 * This function initializes the GPIO pin.
 *
 * @param port: GPIO port: GPIOA, GPIOB and so on;
 * @param pin: number of pin listed in GPIO_Pin_t;
 * @param outputMode: pin mode listed in GPIO_Mode_t;
 * @param outputType: pin output type listed in GPIO_OType_t;
 * @param outputSpeed: pin output speed listed in GPIO_Speed_t;
 * @param pullUpDown: pin pull Up or pull Down option listed in GPIO_Pull_t;
 */
void GPIO_Init(GPIO_TypeDef * port, GPIO_Pin_t pin, GPIO_Mode_t outputMode,
				GPIO_OType_t outputType, GPIO_Speed_t outputSpeed, GPIO_Pull_t pullUpDown)
{
	port->MODER &= ~(0x3UL << (pin*2U));		// clearing corresponding bits in GPIO MODER register
	port->MODER |= ((uint32_t)outputMode << (pin*2U));	// setting corresponding output mode to GPIO MODER register

	port->OTYPER &= ~(1UL << pin);			// first cleaning corresponding bit in GPIO OTYPER register
	port->OTYPER |= ((uint32_t)outputType << pin);	// setting output type to GPIO OTYPER register

	port->OSPEEDR &= ~(0x3UL <<( pin*2U));	// first cleaning corresponding bits in GPIO OSPEEDR register
	port->OSPEEDR |= ((uint32_t)outputSpeed << (pin*2U)); // setting output speed to GPIO OSPEEDR register

	port->PUPDR &= ~(0x3UL << (pin*2U));		// first cleaning corresponding bits in GPIO PUPDR register
	port->PUPDR |= ((uint32_t)pullUpDown << (pin*2U));	// setting pull Up/Down configuration to corresponding bit in GPIO PUPDR
}

/**
 * This function writes the specified value to the specified pin and port.
 */
void GPIO_Write_Pin(GPIO_TypeDef * port, GPIO_Pin_t pin, GPIO_PinState_t value)
{
	if (value == GPIO_PIN_SET) {	// if value is 1 or GPIO_PIN_SET
		port->BSRR = (uint32_t)(1UL << pin);	// setting bit in BSx part of BSRR register
	} else {
		port->BSRR = (uint32_t)(1UL << (pin + 16U));	// setting bit in BRx part of BSRR register
	}
}

/**
 * This function reads the specified pin on the specified port.
 */
GPIO_PinState_t GPIO_Read_Pin(GPIO_TypeDef * port, GPIO_Pin_t pin)
{
	/**
	 * Shifting corresponding bit in Input Data Register to 0th place.
	 * Checking if bit at 0th place has value 0 or 1.
	 * If it is zero: returning GPIO_PIN_RESET
	 * If it is one : returning GPIO_PIN_SET
	 */
	return (GPIO_PinState_t)((port->IDR >> pin) & 1UL);
}

/**
 * This function selects specific alternate function for the specified pin.
 */
void GPIO_Set_Alternate_Function(GPIO_TypeDef * port, GPIO_Pin_t pin, GPIO_Alt_func_t altFunction)
{
	// If pin number < 8, then choose GPIOx_AFRL register
	if (pin < GPIO_PIN_8) {
		port->AFRL &= ~(0x0FUL << (pin*4U));	// first cleaning corresponding bits in AFRL
		port->AFRL |= ((uint32_t)altFunction << (pin*4U));	// setting corresponding alternate function to specified pin
	} else {
		port->AFRH &= ~(0x0FUL << ((pin-8U)*4U));	// first cleaning corresponding bits in AFRH
		port->AFRH |= ((uint32_t)altFunction << ((pin-8U)*4U));	// setting corresponding alternate function to specified pin
	}
}

