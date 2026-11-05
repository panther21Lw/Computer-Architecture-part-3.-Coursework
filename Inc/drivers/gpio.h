/*
 * gpio.h
 *
 *  Created on: Nov 3, 2026
 *      Author: ADMIN
 */

#ifndef DRIVERS_GPIO_H_
#define DRIVERS_GPIO_H_

#include <stdint.h>		/* Header for standard integer types (like uint32_t) */

#include "memory_map.h"

// GPIO Registers
typedef struct {
	volatile uint32_t MODER;		/* GPIO port mode register */
	volatile uint32_t OTYPER;		/* GPIO port output type register */
	volatile uint32_t OSPEEDR;		/* GPIO port output speed register */
	volatile uint32_t PUPDR;		/* GPIO port pull-up/pull-down register */
	volatile uint32_t IDR;			/* GPIO input data register */
	volatile uint32_t ODR;			/* GPIO port output data register */
	volatile uint32_t BSRR;			/* GPIO port bit set/reset register */
	volatile uint32_t LCKR;			/* GPIO port configuration lock register */
	volatile uint32_t AFRL;			/* GPIO alternate function low register */
	volatile uint32_t AFRH;			/* GPIO alternate function high register */
} GPIO_TypeDef;

#define 	GPIOA		((GPIO_TypeDef *) GPIOA_BASE)
#define		GPIOB		((GPIO_TypeDef *) GPIOB_BASE)

typedef enum {
	GPIO_PIN_0 =  0U,				/* Pin 0 */
	GPIO_PIN_1 =  1U,				/* Pin 1 */
	GPIO_PIN_2 =  2U,				/* Pin 2 */
	GPIO_PIN_3 =  3U,				/* Pin 3 */
	GPIO_PIN_4 =  4U,				/* Pin 4 */
	GPIO_PIN_5 =  5U,				/* Pin 5 */
	GPIO_PIN_6 =  6U,				/* Pin 6 */
	GPIO_PIN_7 =  7U,				/* Pin 7 */
	GPIO_PIN_8 =  8U,				/* Pin 8 */
	GPIO_PIN_9 =  9U,				/* Pin 9 */
	GPIO_PIN_10 = 10U,				/* Pin 10 */
	GPIO_PIN_11 = 11U,				/* Pin 11 */
	GPIO_PIN_12 = 12U,				/* Pin 12 */
	GPIO_PIN_13 = 13U,				/* Pin 13 */
	GPIO_PIN_14 = 14U,				/* Pin 14 */
	GPIO_PIN_15 = 15U				/* Pin 15 */
} GPIO_Pin_t;

typedef enum {
	GPIO_MODE_INPUT	    = 0x00U,	/* Input (reset state) */
	GPIO_MODE_OUTPUT    = 0x01U,	/* General purpose output mode */
	GPIO_MODE_ALTERNATE = 0x02U,	/* Alternate function mode */
	GPIO_MODE_ANALOG	= 0x03U		/* Analog mode */
} GPIO_Mode_t;

typedef enum {
	GPIO_OUTPUT_PUSHPULL  = 0U,		/* Output push-pull */
	GPIO_OUTPUT_OPENDRAIN = 1U		/* Output open-drain */
} GPIO_OType_t;

typedef enum {
	GPIO_SPEED_LOW		= 0x00U,	/* Low speed */
	GPIO_SPEED_MEDIUM	= 0x01U,	/* Medium speed */
	GPIO_SPEED_FAST		= 0x02U,	/* Fast speed */
	GPIO_SPEED_HIGH		= 0x03U		/* High speed */
} GPIO_Speed_t;

typedef enum {
	GPIO_PULL_NO		= 0x00U,	/* No pull-up, pull-down */
	GPIO_PULL_UP		= 0x01U,	/* Pull-up */
	GPIO_PULL_DOWN		= 0x02U		/* Pull-down */
	/* Option 0x03 is reserved */
} GPIO_Pull_t;

typedef enum {
	AF0 = 0x00U,	/* Alternate function 0 */
	AF1 = 0x01U,	/* Alternate function 1 */
	AF2 = 0x02U,	/* Alternate function 2 */
	AF3 = 0x03U,	/* Alternate function 3 */
	AF4 = 0x04U,	/* Alternate function 4 */
	AF5 = 0x05U,	/* Alternate function 5 */
	AF6 = 0x06U,	/* Alternate function 6 */
	AF7 = 0x07U,	/* Alternate function 7 */
	AF8 = 0x08U,	/* Alternate function 8 */
	AF9 = 0x09U,	/* Alternate function 9 */
	AF10= 0x0AU,	/* Alternate function 10 */
	AF11= 0x0BU,	/* Alternate function 11 */
	AF12 = 0x0CU,	/* Alternate function 12 */
	AF13 = 0x0DU,	/* Alternate function 13 */
	AF14 = 0x0EU,	/* Alternate function 14 */
	AF15 = 0x0FU,	/* Alternate function 15 */
} GPIO_Alt_func_t;

typedef enum {
	GPIO_PIN_RESET	= 0x00U,	/* Logical LOW value on the pin */
	GPIO_PIN_SET	= 0x01U,	/* Logical HIGH value on the pin */
} GPIO_PinState_t;

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
				GPIO_OType_t outputType, GPIO_Speed_t outputSpeed, GPIO_Pull_t pullUpDown);

/**
 * This function writes the specified value to the specified pin and port.
 */
void GPIO_Write_Pin(GPIO_TypeDef * port, GPIO_Pin_t pin, GPIO_PinState_t value);

/**
 * This function reads the specified pin on the specified port.
 */
GPIO_PinState_t GPIO_Read_Pin(GPIO_TypeDef * port, GPIO_Pin_t pin);

/**
 * This function selects specific alternate function for the specified pin.
 */
void GPIO_Set_Alternate_Function(GPIO_TypeDef * port, GPIO_Pin_t pin, GPIO_Alt_func_t altFunction);


#endif	/* DRIVERS_GPIO_H_ */
