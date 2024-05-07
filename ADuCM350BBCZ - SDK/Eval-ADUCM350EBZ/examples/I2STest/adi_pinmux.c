/*
 **
 ** Source file generated on January 31, 2013 at 11:12:07.	
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
 ** UART0 (Tx, Rx)
 ** I2S (BIT_CLOCK, DATA, LR_CLK)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** P0_06, P0_07, P3_12, P3_13, P3_14, P4_00, P4_01
 */

#include "device.h"

#define I2C_ALT_SCL_PORTP4_MUX  ((uint16_t) ((uint16_t) 1<<0))
#define I2C_ALT_SDA_PORTP4_MUX  ((uint16_t) ((uint16_t) 1<<2))
#define UART0_TX_PORTP0_MUX  ((uint16_t) ((uint16_t) 2<<12))
#define UART0_RX_PORTP0_MUX  ((uint16_t) ((uint16_t) 2<<14))
#define I2S_BIT_CLOCK_PORTP3_MUX  ((uint32_t) ((uint32_t) 2<<24))
#define I2S_DATA_PORTP3_MUX  ((uint32_t) ((uint32_t) 2<<26))
#define I2S_LR_CLK_PORTP3_MUX  ((uint32_t) ((uint32_t) 2<<28))



int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void) {
    /* Port Control MUX registers */
    *((volatile uint32_t *)REG_GPIO0_GPCON) = UART0_TX_PORTP0_MUX | UART0_RX_PORTP0_MUX;
    *((volatile uint32_t *)REG_GPIO3_GPCON) = I2S_BIT_CLOCK_PORTP3_MUX | I2S_DATA_PORTP3_MUX
     | I2S_LR_CLK_PORTP3_MUX;
    *((volatile uint32_t *)REG_GPIO4_GPCON) = I2C_ALT_SCL_PORTP4_MUX | I2C_ALT_SDA_PORTP4_MUX;


    return 0;
}

