/*
 * pwr.c
 *
 *  Created on: Nov 4, 2026
 *      Author: ADMIN
 */


#include "drivers/pwr.h"

// This function sets regulator voltage scaling output
void PWR_Set_Vos(PWR_VOS_t vos)
{
	// clearing 15 and 14 bits in Power control register: VOS bits
	// Setting corresponding VOS value to Power control register
	PWR->CR = (PWR->CR & ~(0x03UL << 14UL)) | ((uint32_t)vos << 14UL);
}
