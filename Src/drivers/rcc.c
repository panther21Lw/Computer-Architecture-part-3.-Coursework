/*
 * rcc.c
 *
 *  Created on: Nov 4, 2026
 *      Author: ADMIN
 */

#include "drivers/rcc.h"

#define RCC_HSI_CLOCK 	(16000000UL)	/* HSI frequency 16 MHz. Source: Table 39, p. 87, DS	*/
#define RCC_HSE_CLOCK	(25000000UL)	/* HSE frequency 25 MHz. On-board Crystal				*/


// This function returns SYSCLK on the System Clock Switch
// This function may RETURN 0 in case of internal error
uint32_t RCC_Get_SysClk(void) {
	uint32_t switch_status = (RCC->CFGR >> 2UL) & 0x03UL;	// getting Switch Status bits to get SYSCLK clock source

	switch (switch_status) {
	case 0x00UL:	// HSI is selected as the SYSCLK clock source
		return RCC_HSI_CLOCK;	// returning the HSI clock frequency value

	case 0x01UL:	// HSE is selected as the SYSCLK clock source
		return RCC_HSE_CLOCK;	// returning the HSSE clock frequency value

	case 0x02UL:	// PLL is selected as the SYSCLK clock source
		/**
		 * The frequency is generated with PLL (PLLCLK) derives from:
		 * 1. VCO_in  = PLL_SOURCE / PLL_M;
		 * 2. VCO_out = VCO_in * PLL_N;
		 * 3. PLLCLK  = VCO_out / PLL_P;
		 */
		uint32_t pll_input_clock_freq;

		if (RCC->PLLCFGR & (1UL << 22U)) {	// if the PLLSRC bit (bit 22) is set then the clock source is HSE
			pll_input_clock_freq = RCC_HSE_CLOCK;
		} else {
			pll_input_clock_freq = RCC_HSI_CLOCK;	// if the PLLSCR bit (bit 22) is reset then the clock source is HSI
		}

		uint32_t PLL_M = (RCC->PLLCFGR & 0x3FUL);	// bits 5:0 in PLLCFGR is PLL_M factor
		uint32_t Pll_N = ((RCC->PLLCFGR >> 6UL) & 0x1FFUL);	// bits 14:6 in PLLCFGR is PLL_N factor
		uint32_t PLL_P = ((RCC->PLLCFGR >> 16UL) & 0x03UL);	// bits 17:16 in PLLCFGR is PLL_P factor

		return (pll_input_clock_freq / PLL_M * Pll_N / PLL_P);

	default:	// if switch status not 0x00 to 0x02 there is some error. Return 0
		return 0;
	}
}

// This function sets SYSCLK to 25 MHz.
// Clock source for SYSCLK is HSE.
RCC_CLK_SET_t SYSCLK_HSE_25MHz(void);

// This function sets SYSCLK to 100 MHz.
// Clock source for SYSCLK is HSE.
RCC_CLK_SET_t SYSCLK_HSE_100MHz(void);
