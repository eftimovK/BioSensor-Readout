/*
 **
 ** Source file generated on February 1, 2013 at 15:50:40.	
 **
 ** Copyright (C) 2013 Analog Devices Inc., All Rights Reserved.
 **
 ** This file is generated automatically based upon the options selected in 
 ** the Pin Multiplexing configuration editor. Changes to the Pin Multiplexing
 ** configuration should be made by changing the appropriate options rather
 ** than editing this file.
 **
 ** Selected Peripherals
 ** --------------------
 ** BEEPER (TONE_P, TONE_N)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** P3_12, P3_13
 */

#include "device.h"

#define BEEPER_TONE_P_PORTP3_MUX  ((uint32_t) ((uint32_t) 1<<24))
#define BEEPER_TONE_N_PORTP3_MUX  ((uint32_t) ((uint32_t) 1<<26))



int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void) {
    /* Port Control MUX registers */
    *((volatile uint32_t *)REG_GPIO3_GPCON) = BEEPER_TONE_P_PORTP3_MUX | BEEPER_TONE_N_PORTP3_MUX;


    return 0;
}

