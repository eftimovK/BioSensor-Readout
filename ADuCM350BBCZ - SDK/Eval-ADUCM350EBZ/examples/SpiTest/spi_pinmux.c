/*
 **
 ** Source file generated on November 6, 2014 at 14:20:52.	
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
 ** SPI0 (SCLK, MISO, MOSI, CS)
 ** SPI1 (SCLK, MISO, MOSI, CS)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** P3_00, P3_01, P3_02, P3_03, P3_04, P3_05, P3_06, P3_07
 */

#include "device.h"

#define SPI0_SCLK_PORTP3_MUX  ((uint16_t) ((uint16_t) 1<<0))
#define SPI0_MISO_PORTP3_MUX  ((uint16_t) ((uint16_t) 1<<2))
#define SPI0_MOSI_PORTP3_MUX  ((uint16_t) ((uint16_t) 1<<4))
#define SPI0_CS_PORTP3_MUX  ((uint16_t) ((uint16_t) 1<<6))
#define SPI1_SCLK_PORTP3_MUX  ((uint16_t) ((uint16_t) 3<<8))
#define SPI1_MISO_PORTP3_MUX  ((uint16_t) ((uint16_t) 3<<10))
#define SPI1_MOSI_PORTP3_MUX  ((uint16_t) ((uint16_t) 3<<12))
#define SPI1_CS_PORTP3_MUX  ((uint16_t) ((uint16_t) 3<<14))



int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void) {
    /* Port Control MUX registers */
    *((volatile uint32_t *)REG_GPIO3_GPCON) = SPI0_SCLK_PORTP3_MUX | SPI0_MISO_PORTP3_MUX
     | SPI0_MOSI_PORTP3_MUX | SPI0_CS_PORTP3_MUX | SPI1_SCLK_PORTP3_MUX
     | SPI1_MISO_PORTP3_MUX | SPI1_MOSI_PORTP3_MUX | SPI1_CS_PORTP3_MUX;


    return 0;
}

