/*
 * usart.h
 *
 *  Created on: Nov 7, 2026
 *      Author: ADMIN
 */


#include "drivers/usart.h"
#include "drivers/rcc.h"


// This function initializes USART2
// Function activates Transmitter and Receiver.
// Function enables interrupts for events: RXNE.
void USART2_Init(uint32_t baudRate, USART_OSAMP_t oversampling,
		USART_PARITY_t parity, USART_WORD_t wordLength, USART_STOP_BIT_t nStopBits)
{
	// Enabling clock for USART2
	RCC_USART2_Clock_EN();

	uint32_t pclk = RCC_Get_APBxClk(RCC_APB1);	// Obtain APB1 clock

	if (pclk == 0 || baudRate == 0) {
		return;
	}

	float usartDiv = (float) pclk / ((float)baudRate * 8U * (2U - (float)oversampling));	// Calculation of USARTDIV

	uint32_t mantissa = (uint32_t)usartDiv;		// Mantissa is integer part of calculated USARTDIV

	usartDiv -= mantissa;	// Raw fraction part of USARTDIV

	usartDiv *= (2U - oversampling) * 8U;	// Fraction part of USARTDIV in integer value

	uint32_t fraction = (uint32_t)usartDiv;	// Fraction part value that will be written to BRR

	uint32_t tmp = (uint32_t)(usartDiv * 10U);	//  Value for rounding fraction part

	/**
	 * Rounding the Fraction part using rule:
	 * if 1st number after floating point is bigger than 4,
	 * so integer part of the number have to be incremented by 1;
	 * else integer part remains as is.
	 */
	if((tmp % 10U) > 4U) {
		fraction += 1UL;
		// If fraction part equals to divider, mantissa have to be incremented by 1
		// and fraction part has to be 0
		if (fraction == (8U * (2U - oversampling))) {
			mantissa += 1U;
			fraction = 0U;
		}
	}

	// Enabling the USART2
	USART2->CR1 |= (1UL << 13U);	// Setting the UE bit

	// Setting the Word length
	USART2->CR1 &= ~(1UL << 12U);	// Clearing the M bit
	USART2->CR1 |= (((uint32_t)wordLength & 0x01UL) << 12U);	// Setting the M bit

	// Setting the number of stop bits
	USART2->CR2 = (USART2->CR2 & ~(0x03UL << 12U)) | (((uint32_t)nStopBits & 0x03UL) << 12U);

	// Setting Mantissa and Fraction part to Baud Rate Register
	// Clearing Mantissa and Fraction parts of BRR
	USART2->BRR &= ~(0xFFFFUL);

	if (oversampling == USART_OSAMP_8) {
		// If oversampling by 8 is used, the DIV_Fraction[3] bit must be kept cleared
		USART2->BRR = (fraction & 0x07UL);
	} else {
		USART2->BRR = (fraction & 0x0FUL);	// Setting all 4 bits of the Fraction part
	}

	USART2->BRR |= (mantissa & 0xFFFUL) << 4U;	// Setting the Mantissa part

	// Setting the parity
	if (parity != USART_PARITY_OFF) {
		// Enabling the Parity control
		USART2->CR1 |= (1UL << 10U);	// Setting the PCE bit

		USART2->CR1 &= ~(1UL << 9U);	// Clearing the PS bit: even parity selected
		if (parity == USART_PARITY_ODD) {
			USART2->CR1 |= (1UL << 9U); // Setting the PS bit: odd parity selected
		}
	} else {
		// Disabling parity control
		USART2->CR1 &= ~(1UL << 10U);	// Clearing the PCE bit
	}

	// Setting oversampling method
	if (oversampling == USART_OSAMP_16) {
		USART2->CR1 &= ~(1UL << 15U);	// Clearing the OVER8 bit: oversampling by 16
	} else {
		USART2->CR1 |= (1UL << 15U);	// Setting the OVER8 bit: oversampling by 8
	}

	// Enabling interrupts
	USART2->CR1 |= (1UL << 5U);	// interrupt on Receiver data register not empty

	// Enabling the Transmitter
	USART2->CR1 |= (1UL << 3U);	// Setting the TE bit

	// Enabling the Receiver
	USART2->CR1 |= (1UL << 2U);	// Setting the RE bit

}
