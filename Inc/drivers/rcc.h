/*
 * rcc.h
 *
 *  Created on: Nov 4, 2026
 *      Author: ADMIN
 */

#ifndef DRIVERS_RCC_H_
#define DRIVERS_RCC_H_

#include <stdint.h>

#include "memory_map.h"

// RCC registers
typedef struct {
	volatile uint32_t CR;				/* RCC clock control register     				*/
	volatile uint32_t PLLCFGR;			/* RCC PLL configuration register 				*/
	volatile uint32_t CFGR;				/* RCC clock configuration register 			*/
	volatile uint32_t CIR;				/* RCC clock interrupt register					*/
	volatile uint32_t AHB1RSTR;			/* RCC AHB1 peripheral reset register 			*/
	volatile uint32_t AHB2RSTR;			/* RCC AHB2 peripheral reset register			*/
	volatile uint32_t APB1RSTR;			/* RCC APB1 peripheral reset register			*/
	volatile uint32_t APB2RSTR;			/* RCC APB2 peripheral reset register			*/
	volatile uint32_t AHB1ENR;			/* RCC AHB1 peripheral clock enable register 	*/
	volatile uint32_t AHB2ENR;			/* RCC AHB2 peripheral clock enable register	*/
	volatile uint32_t APB1ENR;			/* RCC APB1 peripheral clock enable register	*/
	volatile uint32_t APB2ENR;			/* RCC APB2 peripheral clock enable register	*/
	volatile uint32_t AHB1LPENR;		/* RCC AHB1 peripheral clock enable in low power mode register  */
	volatile uint32_t AHB2LPENR;		/* RCC AHB2 peripheral clock enable in low power mode register  */
	volatile uint32_t APB1LPENR;		/* RCC APB1 peripheral clock enable in low power mode register  */
	volatile uint32_t APB2LPENR;		/* RCC APB2 peripheral clock enabled in low power mode register */
	volatile uint32_t BDCR;				/* RCC Backup domain control register			*/
	volatile uint32_t CSR;				/* RCC clock control & status register			*/
	volatile uint32_t SSCGR;			/* RCC spread spectrum clock generation register */
	volatile uint32_t PLLI2SCFGR;		/* RCC PLLI2S configuration register			*/
	volatile uint32_t DCKCFGR;			/* Dedicated Clocks Configuration Register		*/
} RCC_TypeDef;

#define RCC								((RCC_TypeDef *) RCC_BASE)

typedef enum {
	RCC_MCO2_SYSCLK	= 0x00UL,			/* System clock (SYSCLK) selected 	*/
	RCC_MCO2_PLLI2S	= 0x01UL,			/* PLLI2S clock selected			*/
	RCC_MCO2_HSE	= 0x02UL,			/* HSE oscillator clock selected	*/
	RCC_MCO2_PLL	= 0x03UL			/* PLL clock selected				*/
} RCC_MCO2_t;

typedef enum {
	RCC_MCO1_HSI	= 0x00UL,			/* HSI clock selected				*/
	RCC_MCO1_LSE	= 0x01UL,			/* LSE oscillator selected			*/
	RCC_MCO1_HSE	= 0x02UL,			/* HSE oscillator clock selected	*/
	RCC_MCO1_PLL	= 0x03UL			/* PLL clock selected				*/
} RCC_MCO1_t;

typedef enum {
	RCC_MCO_PRE_NO	= 0x00UL,			/* no division						*/
	RCC_MCO_PRE_2	= 0x04UL,			/* division by 2					*/
	RCC_MCO_PRE_3	= 0x05UL,			/* division by 3					*/
	RCC_MCO_PRE_4	= 0x06UL,			/* division by 4					*/
	RCC_MCO_PRE_5	= 0x07UL,			/* division by 5					*/
} RCC_MCO_PRE_t;

// This enumeration is used to set the APBx prescaler
typedef enum {
	RCC_APB_PRE_NO	= 0x00UL,			/* AHB clock not divided			*/
	RCC_APB_PRE_1	= 0x01UL,			/* AHB clock not divided			*/
	RCC_APB_PRE_2	= 0x04UL,			/* AHB clock divided by 2			*/
	RCC_APB_PRE_4	= 0x05UL,			/* AHB clock divided by 4			*/
	RCC_APB_PRE_8	= 0x06UL,			/* AHB clock divided by 8			*/
	RCC_APB_PRE_16	= 0x07UL			/* AHB clock divided by 16			*/
} RCC_APB_PRE_t;

typedef enum {
	RCC_SYSCLK_SRC_HSI	= 0x00UL,		/* HSI oscillator selected as system clock	*/
	RCC_SYSCLK_SRC_HSE	= 0x01UL,		/* HSE oscillator selected as system clock	*/
	RCC_SYSCLK_SRC_PLL	= 0x02UL		/* PLL selected as system clock				*/
} RCC_SYSCLK_SRC_t;

typedef enum {
	RCC_CLK_SET_SUCCESS = 0x00UL,		/* Setting the new clock frequency was SUCCESSFUL */
	RCC_CLK_SET_FAIL	= 0x01UL		/* Setting the new clock frequency 	   FAILED	  */
} RCC_CLK_SET_t;


// This function returns SYSCLK on the System Clock Switch
// This function may RETURN 0 in case of internal error
uint32_t RCC_Get_SysClk(void);

// This function sets SYSCLK to 25 MHz.
// Clock source for SYSCLK is HSE.
RCC_CLK_SET_t SYSCLK_HSE_25MHz(void);

// This function sets SYSCLK to 100 MHz.
// Clock source for SYSCLK is HSE.
RCC_CLK_SET_t SYSCLK_HSE_100MHz(void);

// This function enables clock to USART2.
static inline void RCC_USART2_Clock_EN(void)
{
	RCC->APB1ENR |= (1UL << 17UL);	// Setting the USART2EN bit
}

// This function enables clock to GPIOA
static inline void RCC_GPIOA_Clock_EN(void)
{
	RCC->AHB1ENR |= (1UL);	// Setting the GPIOAEN bit
}

// This function enables clock to GPIOB
static inline void RCC_GPIOB_Clock_EN(void)
{
	RCC->AHB1ENR |= (1UL << 1UL);	// Setting the GPIOBEN bit
}

#endif /* DRIVERS_RCC_H_ */
