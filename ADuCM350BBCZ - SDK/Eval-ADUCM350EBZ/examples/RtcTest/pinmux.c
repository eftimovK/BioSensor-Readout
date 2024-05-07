/*
 **
 ** Source file generated on November 13, 2014 at 10:50:31.	
 **
 ** Copyright (C) 2014 Analog Devices Inc., All Rights Reserved.
 **
 ** This file is generated automatically based upon the options selected in 
 ** the Pin Multiplexing configuration editor. Changes to the Pin Multiplexing
 ** configuration should be made by changing the appropriate options rather
 ** than editing this file.
 **
 ** Selected Peripherals
 ** --------------------
 ** SYS_CLK_OUT (CLOCK_OUT)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** P1_07
 */

#include "device.h"

#define SYS_CLK_OUT_CLOCK_OUT_PORTP1_MUX  ((uint16_t) ((uint16_t) 3<<14))



int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void) {
    /* Port Control MUX registers */
    *((volatile uint32_t *)REG_GPIO1_GPCON) = SYS_CLK_OUT_CLOCK_OUT_PORTP1_MUX;


    return 0;
}

