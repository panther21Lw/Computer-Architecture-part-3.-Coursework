/*
 * flash.c
 *
 *  Created on: Nov 3, 2026
 *      Author: ADMIN
 */

#include "drivers/flash.h"

// This function sets the number of Flash Wait States
void Flash_Set_Wait_States(FLASH_WS_t ws)
{
	FLASH->ACR &= ~(0x0FUL);		// first clearing LATENCY bits in FLASH_ACR register
	FLASH->ACR |= ((uint32_t)ws);	// setting LATENCY bits with WS value of Wait States

	// reading FLASH ACR register to ensure the configuration is applied
	(void)FLASH->ACR;
}

// This function enables Instruction and Data Caches
void Flash_Enable_Caches(void)
{
	Flash_Instruction_Cache_Enable();	// enabling Instruction Cache
	Flash_Data_Cache_Enable();			// enabling Data Cache
}

// This function enables Instruction Cache
void Flash_Instruction_Cache_Enable(void)
{
	FLASH->ACR |= (1UL << 9UL);		// enabling the I Cache: bit 9 ICEN: Instruction cache enable
}

// This function enables Data Cache
void Flash_Data_Cache_Enable(void)
{
	FLASH->ACR |= (1UL << 10UL); 	// enabling the D Cache: bit 10 DCEN: Data cache enable
}

// This function enables Prefetch Buffer
void Flash_Prefetch_Enable(void)
{
	FLASH->ACR |= (1UL << 8UL);		// enabling the PreFetch: bit 8 PRFTEN: Prefetch enable
}
