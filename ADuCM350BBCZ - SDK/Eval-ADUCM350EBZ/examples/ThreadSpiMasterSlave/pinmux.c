/*********************************************************************************

Copyright (c) 2011-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

#include <stdint.h>
#include "system.h"

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
