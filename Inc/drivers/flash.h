/*
 * flash.h
 *
 *  Created on: Nov 3, 2026
 *      Author: ADMIN
 */

#ifndef DRIVERS_FLASH_H_
#define DRIVERS_FLASH_H_

#include <stdint.h>		/* Header for standard integer types (like uint32_t) */

#include "memory_map.h"

//FLASH registers
typedef struct {
	volatile uint32_t ACR;		/* Flash access control register */
	volatile uint32_t KEYR;		/* Flash key register 			 */
	volatile uint32_t OPTKEYR;	/* Flash option key register 	 */
	volatile uint32_t SR;		/* Flash status register		 */
	volatile uint32_t CR;		/* Flash control register		 */
	volatile uint32_t OPTCR;	/* Flash option control register */
} FLASH_TypeDef;

#define FLASH		((FLASH_TypeDef *) FLASH_R_BASE)

typedef enum {
	FLASH_WS_0	= 0U,			/* 0 Wait states */
	FLASH_WS_1	= 1U,			/* 1 Wait state  */
	FLASH_WS_2	= 2U,			/* 2 Wait states */
	FLASH_WS_3	= 3U,			/* 3 Wait states */
	/* It is possible to have up to 6 WS but we will use only FLASH_WS_3 option */
} FLASH_WS_t;

// This function sets the number of Flash Wait States
void Flash_Set_Wait_States(FLASH_WS_t ws);

// This function enables Prefetch Buffer
void Flash_Prefetch_Enable(void);

// This function enables Instruction and Data Caches
void Flash_Enable_Caches(void);

// This function enables Instruction Cache
void Flash_Instruction_Cache_Enable(void);

// This function enables Data Cache
void Flash_Data_Cache_Enable(void);

#endif /* DRIVERS_FLASH_H_ */
