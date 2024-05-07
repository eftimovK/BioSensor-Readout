/*
 **
 ** Source file generated on December 8, 2014 at 14:20:19.	
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
 ** SPI1 (SCLK, MISO, MOSI)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** P3_04, P3_05, P3_06
 */

#include "device.h"

#define SPI1_SCLK_PORTP3_MUX  ((uint16_t) ((uint16_t) 3<<8))
#define SPI1_MISO_PORTP3_MUX  ((uint16_t) ((uint16_t) 3<<10))
#define SPI1_MOSI_PORTP3_MUX  ((uint16_t) ((uint16_t) 3<<12))



int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void) {
    /* Port Control MUX registers */
    *((volatile uint32_t *)REG_GPIO3_GPCON) = SPI1_SCLK_PORTP3_MUX | SPI1_MISO_PORTP3_MUX
     | SPI1_MOSI_PORTP3_MUX;


    return 0;
}

