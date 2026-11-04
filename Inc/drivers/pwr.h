/*
 * pwr.h
 *
 *  Created on: Nov 4, 2026
 *      Author: ADMIN
 */

#ifndef DRIVERS_PWR_H_
#define DRIVERS_PWR_H_

#include <stdint.h>

#include "memory_map.h"

typedef struct {
	volatile uint32_t CR;		/* PWR power control register 		 */
	volatile uint32_t CSR;		/* PWR power control/status register */
} PWR_TypeDef;

#define PWR			((PWR_TypeDef *) POWER_BASE)

typedef enum {
	VOS_SCALE_MODE_3	= 0x01U,	/* Scale 3 mode <= 64 MHz  				*/
	VOS_SCALE_MODE_2	= 0x02U,	/* Scale 2 mode (reset value) <= 84 MHz */
	VOS_SCALE_MODE_1	= 0x03U		/* Scale 1 mode <= 100 MHz				*/
} PWR_VOS_t;

// This function sets regulator voltage scaling output
void PWR_Set_Vos(PWR_VOS_t vos);

#endif /* DRIVERS_PWR_H_ */
