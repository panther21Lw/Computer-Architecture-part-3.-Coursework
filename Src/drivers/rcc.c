/*
 * rcc.c
 *
 *  Created on: Nov 4, 2026
 *      Author: ADMIN
 */

#include "drivers/rcc.h"
#include "drivers/pwr.h"
#include "drivers/flash.h"

#define RCC_HSI_CLOCK 	(16000000UL)	/* HSI frequency 16 MHz. Source: Table 39, p. 87, DS	*/
#define RCC_HSE_CLOCK	(25000000UL)	/* HSE frequency 25 MHz. On-board Crystal				*/

#define RCC_HSE_T_SU_NS	(2*1000000UL)	/* HSE typical startup time (2 ms in ns). Source: Table 37, p. 85, DS */
#define RCC_HSI_T_SU_NS (4*	  1000UL)	/* HSI maximum startup time (4 us in ns). Source: Table 39, p. 87, DS */
#define RCC_PLL_LOCK_NS	(300* 1000UL)	/* PLL maximum lock time (300 us in ns).  Source: Table 41, p. 89, DS */

#define RCC_HSE_SU_MAX	(10 * RCC_HSE_T_SU_NS)	/* maximum timeout for setup HSERDY	*/
#define RCC_HSI_SU_MAX	(2  * RCC_HSI_T_SU_NS)	/* maximum timeout for setup HSIRDY	*/
#define RCC_PLL_SU_MAX	(2  * RCC_PLL_LOCK_NS)	/* maximum timeout for setup PLLRDY	*/


// This functions calculates number on CPU cycles to spend to achieve time, provided in ns
static inline uint32_t time_ns_to_cycles(uint32_t time_ns, uint32_t frequency)
{
	/**
	 * The formula is:
	 * 1 cycle  = N (seconds) <==> 1/frequency
	 * x cycles = M (seconds)
	 *
	 * Use proportion to solve for x:
	 * x = (M (seconds) * 1 cycle / N (seconds))
	 *
	 * Replacing N (seconds) to 1/frequency:
	 * x = (M (seconds) * 1 cycle * frequency) / 1 (second)
	 * So the answer in cycles.
	 *
	 * M provided in ns (nano seconds), so equation have to be divided by 1_000_000_000 (billion)
	 *
	 * The final formula is:
	 *
	 * x = (M (nanoseconds) * frequency) / 1000000000
	 */
	// + 999999999ULL used to avoid losing fraction part during integer division
	return (uint32_t)(((uint64_t)time_ns * frequency + 999999999ULL) / 1000000000ULL);

}

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

		/**
		 * PLL_P bits have values:
		 * 00: p = 2
		 * 01: p = 4
		 * 10: p = 6
		 * 11: p = 8
		 */
		PLL_P = (PLL_P + 1U) << 1U;	// multiplication changed by left shift by 1

		return (uint32_t)((uint64_t)pll_input_clock_freq * Pll_N / ((uint64_t)PLL_M * PLL_P));

	default:	// if switch status not 0x00 to 0x02 there is some error. Return 0
		return 0U;
	}
}

// This function sets SYSCLK to 25 MHz.
// Clock source for SYSCLK is PLL.
RCC_CLK_SET_t SYSCLK_HSE_25MHz(void) {

	// Getting current MCU Clock Frequency. There will be 0 if an error was occurred.
	uint32_t frequency = RCC_Get_SysClk();

	if (frequency == 0) { return RCC_CLK_SET_FAIL; }

	/* 1. Switching to HSI */
	RCC->CR |= (1UL);	// setting HSION

	// Waiting for HSI is ready (HSIRDY flag) or timeout exit if HSI failed
	uint32_t timeout = time_ns_to_cycles(RCC_HSI_SU_MAX, frequency);

	while(!(RCC->CR & 0x02UL)) {
		if (timeout == 0) {
			// Waiting time was expired. There are some problems with HSI.
			return RCC_CLK_SET_FAIL;
		}
		timeout--;
	}

	// Switching SW to HSI
	RCC->CFGR = (RCC->CFGR & ~(0x03UL)) | ((uint32_t)RCC_SYSCLK_SRC_HSI);

	// Waiting until HSI is used as system clock
	while(RCC->CFGR & 0x0CUL);

	/* 2. Turning off the PLL */
	RCC->CR &= ~(1UL << 24U);	// Clearing PLLON bit
	/**
	 * It is possible to execute some lines of code instead of waiting for PLL off.
	 */

	/* 3. Turning on the HSE */
	RCC->CR |= (1UL << 16U);	// setting HSEON bit

	// Check for PLL if off
	while(RCC->CR & (1UL << 25U));	// Waiting for PLLRDY bit reset

	// Check for HSE is ready
	timeout = time_ns_to_cycles(RCC_HSE_SU_MAX, frequency);

	// Waiting HSERDY flag is set
	while(!(RCC->CR & (1UL << 17U))) {
		if (timeout == 0) {
			//Waiting time was expired. There are some problems with HSE.
			return RCC_CLK_SET_FAIL;
		}
		timeout--;
	}

	/* 4. Enabling POWER interface */
	RCC->APB1ENR |= (1UL << 28U);	// Setting PWREN bit
	(void)RCC->APB1ENR;		// Making sure that the POWER interface is enabled

	/* 5. Apply VOS */
	PWR_Set_Vos(VOS_SCALE_MODE_3);

	/* 6. Configuring the PLL */
	// Cleaning M, N and P fields in PLL configuration register
	/**
	 * Setting PLL factors:
	 * M =  25;
	 * N = 200;
	 * P =   8;
	 */
	RCC->PLLCFGR = (RCC->PLLCFGR & ~(0x03FUL) & ~(0x1FFUL << 6U) & ~(0x03UL << 16U))
			| (25UL) | (200UL << 6U) | (3UL << 16U);

	RCC->PLLCFGR |= (1UL << 22U);	// Setting the PLLSRC bit: HSE is clock source for PLL

	/* There is no need to set APB1 prescalers because target SYSCLK (25 MHz) if less than 50 MHz max for APB1. */

	/* 7. Turning on the PLL */
	RCC->CR |= (1UL << 24U);	// Setting the PLLON bit

	/* 8. Flash latency */
	Flash_Set_Wait_States(FLASH_WS_0);	// Target frequency (25 MHz) less than 30 MHz. WS not needed.

	/* Waiting for PLL lock */
	timeout = time_ns_to_cycles(RCC_PLL_SU_MAX, frequency);

	while(!(RCC->CR & (1UL << 25U))) {
		if (timeout == 0) {
			// Waiting time was expired. There are some problems with PLL.
			return RCC_CLK_SET_FAIL;
		}
		timeout--;
	}

	// Waiting that new VOS value was applied by PWR controlled after PLL on
	while(!(PWR->CSR & (1UL << 14U)));

	// HSI is selected as SYSCLK source. No need to clean SW source bits
	RCC->CFGR |= RCC_SYSCLK_SRC_PLL;

	// Reading SWS bits to make sure SYSCLK source is taken into account
	while(((RCC->CFGR >> 2U) & 0x03UL) != RCC_SYSCLK_SRC_PLL);

	return RCC_CLK_SET_SUCCESS;
}

// This function sets SYSCLK to 100 MHz.
// Clock source for SYSCLK is HSE.
RCC_CLK_SET_t SYSCLK_HSE_100MHz(void) {
	// Getting current MCU Clock Frequency. There will be 0 if an error was occurred.
		uint32_t frequency = RCC_Get_SysClk();

		if (frequency == 0) { return RCC_CLK_SET_FAIL; }

		/* 1. Switching to HSI */
		RCC->CR |= (1UL);	// setting HSION

		// Waiting for HSI is ready (HSIRDY flag) or timeout exit if HSI failed
		uint32_t timeout = time_ns_to_cycles(RCC_HSI_SU_MAX, frequency);
		while(!(RCC->CR & 0x02UL)) {
			if (timeout == 0) {
				// Waiting time was expired. There are some problems with HSI.
				return RCC_CLK_SET_FAIL;
			}
			timeout--;
		}

		// Switching SW to HSI
		RCC->CFGR = (RCC->CFGR & ~(0x03UL)) | ((uint32_t)RCC_SYSCLK_SRC_HSI);

		// Waiting until HSI is used as system clock
		while(RCC->CFGR & 0x0CUL);

		/* 2. Turning off the PLL */
		RCC->CR &= ~(1UL << 24U);	// Clearing PLLON bit
		/**
		 * It is possible to execute some lines of code instead of waiting for PLL off.
		 */

		/* 3. Turning on the HSE */
		RCC->CR |= (1UL << 16U);	// setting HSEON bit

		// Check for PLL if off
		while(RCC->CR & (1UL << 25U));	// Waiting for PLLRDY bit reset

		// Check for HSE is ready
		timeout = time_ns_to_cycles(RCC_HSE_SU_MAX, frequency);

		// Waiting HSERDY flag is set
		while(!(RCC->CR & (1UL << 17U))) {
			if (timeout == 0) {
				// Waiting time was expired. There are some problems with HSE.
				return RCC_CLK_SET_FAIL;
			}
			timeout--;
		}

		/* 4. Enabling POWER interface */
		RCC->APB1ENR |= (1UL << 28U);	// Setting PWREN bit
		(void)RCC->APB1ENR;		// Making sure that the POWER interface is enabled

		/* 5. Apply VOS */
		PWR_Set_Vos(VOS_SCALE_MODE_1);

		/* 6. Configuring the PLL */
		// Cleaning M, N and P fields in PLL configuration register
		/**
		 * Setting PLL factors:
		 * M = 25;
		 * N = 200;
		 * P =  2;
		 */
		RCC->PLLCFGR = (RCC->PLLCFGR & ~(0x03FUL) & ~(0x1FFUL << 6U) & ~(0x03UL << 16U))
				| (25UL) | (200UL << 6U) | (0UL << 16U);

		RCC->PLLCFGR |= (1UL << 22U);	// Setting the PLLSRC bit: HSE is clock source for PLL

		/* Setting APB1 prescalers */
		RCC->CFGR = (RCC->CFGR & ~(7UL << 10U)) | (RCC_APB_PRE_2 << 10U);	// APB1 clock divided by 2

		/* 7. Turning on the PLL */
		RCC->CR |= (1UL << 24U);	// Setting the PLLON bit

		/* 8. Flash latency */
		Flash_Set_Wait_States(FLASH_WS_3);	// 3 Wait States for 100 MHz

		/* Waiting for PLL lock */
		timeout = time_ns_to_cycles(RCC_PLL_SU_MAX, frequency);
		while(!(RCC->CR & (1UL << 25U))) {
			if (timeout == 0) {
				// Waiting time was expired. There are some problems with PLL.
				return RCC_CLK_SET_FAIL;
			}
			timeout--;
		}

		// Waiting that new VOS value was applied by PWR controlled after PLL on
		while(!(PWR->CSR & (1UL << 14U)));

		// HSI is selected as SYSCLK source. No need to clean SW source bits
		RCC->CFGR |= RCC_SYSCLK_SRC_PLL;

		// Reading SWS bits to make sure SYSCLK source is taken into account
		while(((RCC->CFGR >> 2U) & 0x03UL) != RCC_SYSCLK_SRC_PLL);

		return RCC_CLK_SET_SUCCESS;
}
