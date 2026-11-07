/*
 * fpu.h
 *
 *  Created on: Nov 7, 2026
 *      Author: ADMIN
 */

#ifndef DRIVERS_FPU_H_
#define DRIVERS_FPU_H_

#include <stdint.h>

// Coprocessor access control register
#define	 CPACR	(*(volatile uint32_t *) (0xE000ED88UL))

// This function calls Data synchronization barrier (DSB) assembler instruction
static inline void __dsb(void) {
	__asm__ __volatile__ ("dsb");
}

// This function calls Instruction synchronization barrier (ISB) assembler instruction
static inline void __isb(void) {
	__asm__ __volatile__ ("isb");
}

/**
 * This function provides FULL ACCESS to FPU
 * coprocessors 10th and 11th
 */
static inline void FPU_Init_FullAccess(void) {
	CPACR |= ((3UL << 11*2U) | (3UL << 10U*2U));
}

#endif /* DRIVERS_FPU_H_ */
