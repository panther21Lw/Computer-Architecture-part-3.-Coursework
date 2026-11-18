/*
 * usart.h
 *
 *  Created on: Nov 5, 2026
 *      Author: ADMIN
 */

#ifndef DRIVERS_USART_H_
#define DRIVERS_USART_H_

#include <stdint.h>

#include "memory_map.h"


typedef struct {
	volatile uint32_t SR;		/* Status register						*/
	volatile uint32_t DR;		/* Data register						*/
	volatile uint32_t BRR;		/* Baud rate register					*/
	volatile uint32_t CR1;		/* Control register 1					*/
	volatile uint32_t CR2;		/* Control register 2					*/
	volatile uint32_t CR3;		/* Control register 3					*/
	volatile uint32_t GTPR;		/* Guard time and prescaler register	*/
} USART_TypeDef;

#define USART2		((USART_TypeDef *) USART2_BASE)

typedef enum {
	USART_STOP_BIT_1	= 0x00UL,	/* 1 Stop bit	*/
	USART_STOP_BIT_05	= 0x01UL,	/* 0.5 Stop bit	*/
	USART_STOP_BIT_2	= 0x02UL,	/* 2 Stop bits	*/
	USART_STOP_BIT_15	= 0x03UL	/* 1.5 Stop bit	*/
} USART_STOP_BIT_t;

typedef enum {
	USART_OSAMP_16	= 0x00UL,		/* oversampling by 16, OVER8 bit is reset	*/
	USART_OSAMP_8	= 0x01UL		/* oversampling by 8,  OVER8 bit is set		*/
} USART_OSAMP_t;


typedef enum {
	USART_PARITY_OFF 	= 0x00UL,		/* Parity control disabled	*/
	USART_PARITY_EVEN	= 0x01UL,		/* Even parity selected		*/
	USART_PARITY_ODD	= 0x02UL		/* Odd parity selected		*/
} USART_PARITY_t;

typedef enum {
	USART_WORD_8	= 0x00UL,			/* 8-bit word length */
	USART_WORD_9	= 0x01UL			/* 9-bit word length */
} USART_WORD_t;

// This function initializes USART2
// Function activates Transmitter and Receiver.
// Function enables interrupts for events: RXNE.
void USART2_Init(uint32_t baudRate, USART_OSAMP_t oversampling,
		USART_PARITY_t parity, USART_WORD_t wordLength, USART_STOP_BIT_t nStopBits);

// This function enables USART2 interrupt on Transmit data register empty event
static inline void USART2_TXE_Interrupt_Enable(void){
	USART2->CR1 |= (1UL << 7U);	// interrupt on Transmit data register empty
}

// This function disables USART2 interrupt on Transmit data register empty event
static inline void USART2_TXE_Interrupt_Disable(void){
	USART2->CR1 &= ~(1UL << 7U); // disable interrupt on TXE event
}

// This function enables USART2 interrupt on Transmission complete event
static inline void USART2_TC_Interrupt_Enable(void){
	USART2->CR1 |= (1UL << 6U);	// interrupt on Transmission complete
}

// This function disables USART2 interrupt on Transmission complete event
static inline void USART2_TC_Interrupt_Disable(void){
	USART2->CR1 &= ~(1UL << 6U); // disable interrupt on TC event
}

#endif /* DRIVERS_USART_H_ */
