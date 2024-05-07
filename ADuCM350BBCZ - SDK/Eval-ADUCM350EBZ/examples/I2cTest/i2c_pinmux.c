/*
 **
 ** Source file generated on September 12, 2013 at 14:43:13.	
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
 ** I2C_ALT (SCL, SDA)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** P4_00, P4_01
 */

#include "device.h"

#define I2C_ALT_SCL_PORTP4_MUX  ((uint16_t) ((uint16_t) 1<<0))
#define I2C_ALT_SDA_PORTP4_MUX  ((uint16_t) ((uint16_t) 1<<2))



int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void) {
    /* Port Control MUX registers */
    *((volatile uint32_t *)REG_GPIO4_GPCON) = I2C_ALT_SCL_PORTP4_MUX | I2C_ALT_SDA_PORTP4_MUX;


    return 0;
}

