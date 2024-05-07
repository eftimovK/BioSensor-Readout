/*
 **
 ** Source file generated on July 9, 2013 at 11:39:56.	
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
 ** CAPTOUCH (ONOFF, Left, Right, Enter, Up, Down)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** P0_00, P0_01, P0_02, P0_03, P0_04, P0_05
 */

#include "device.h"

#define CAPTOUCH_ONOFF_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<0))
#define CAPTOUCH_LEFT_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<2))
#define CAPTOUCH_RIGHT_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<4))
#define CAPTOUCH_ENTER_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<6))
#define CAPTOUCH_UP_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<8))
#define CAPTOUCH_DOWN_PORTP0_MUX  ((uint16_t) ((uint16_t) 1<<10))



int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void) {
    /* Port Control MUX registers */
    *((volatile uint32_t *)REG_GPIO0_GPCON) = CAPTOUCH_ONOFF_PORTP0_MUX | CAPTOUCH_LEFT_PORTP0_MUX
     | CAPTOUCH_RIGHT_PORTP0_MUX | CAPTOUCH_ENTER_PORTP0_MUX | CAPTOUCH_UP_PORTP0_MUX
     | CAPTOUCH_DOWN_PORTP0_MUX;


    return 0;
}

