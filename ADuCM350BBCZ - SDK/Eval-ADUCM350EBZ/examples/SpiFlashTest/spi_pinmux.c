/*
 **
 ** Source file generated on November 19, 2014 at 15:03:08.	
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
 ** SPIH (SCLK, MISO, MOSI, CS)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** P0_12, P0_13, P0_14, P0_15
 */

#include "device.h"

#define SPIH_SCLK_PORTP0_MUX  ((uint32_t) ((uint32_t) 1<<24))
#define SPIH_MISO_PORTP0_MUX  ((uint32_t) ((uint32_t) 1<<26))
#define SPIH_MOSI_PORTP0_MUX  ((uint32_t) ((uint32_t) 1<<28))



int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void) {
    /* Port Control MUX registers */
    *((volatile uint32_t *)REG_GPIO0_GPCON) = SPIH_SCLK_PORTP0_MUX | SPIH_MISO_PORTP0_MUX
     | SPIH_MOSI_PORTP0_MUX;


    return 0;
}

