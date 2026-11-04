/*
 * nvic.c
 *
 *  Created on: Nov 5, 2026
 *      Author: ADMIN
 */

#include "drivers/nvic.h"

/**
 * This function return the priority of the interrupt line.
 */
uint32_t NVIC_GetPriority(IRQn_t irq_n) {
	return (uint32_t)(NVIC->IPR[(uint32_t)irq_n] >> 4U);
}
