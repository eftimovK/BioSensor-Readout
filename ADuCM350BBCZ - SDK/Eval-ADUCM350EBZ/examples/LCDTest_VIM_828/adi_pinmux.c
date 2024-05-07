/*
 **
 ** Source file generated on March 7, 2013 at 13:30:39.	
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
 ** UART0_ALT (Tx, Rx)
 ** LCD (S1, S2, S3, S4, S5, S6, S7, S8, S9, S10, S11, S12, S13, S14, S15, S16, S17, S18, S19, S20, S21, S22, S23, S24, S25, S26, S27, S28, S29, S30, S31, S32, BPLANE0, BPLANE1, BPLANE2, BPLANE3)
 **
 ** GPIO (unavailable)
 ** ------------------
 ** P1_00, P1_01, P1_02, P1_03, P1_04, P1_05, P1_06, P1_07, P1_08, P1_09, P1_10, P1_11,
 ** P1_12, P1_13, P1_14, P1_15, P2_00, P2_01, P2_02, P2_03, P2_04, P2_05, P2_06, P2_07,
 ** P2_08, P2_09, P2_10, P2_11, P2_12, P2_13, P2_14, P2_15, P3_06, P3_07, P3_08, P3_09,
 ** P3_10, P3_11
 */

#include "device.h"

#define UART0_ALT_TX_PORTP3_MUX  ((uint16_t) ((uint16_t) 1<<12))
#define UART0_ALT_RX_PORTP3_MUX  ((uint16_t) ((uint16_t) 1<<14))
#define LCD_S1_PORTP2_MUX  ((uint16_t) ((uint16_t) 1<<8))
#define LCD_S2_PORTP2_MUX  ((uint16_t) ((uint16_t) 1<<10))
#define LCD_S3_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<0))
#define LCD_S4_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<2))
#define LCD_S5_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<4))
#define LCD_S6_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<6))
#define LCD_S7_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<8))
#define LCD_S8_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<10))
#define LCD_S9_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<12))
#define LCD_S10_PORTP1_MUX  ((uint16_t) ((uint16_t) 1<<14))
#define LCD_S11_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<16))
#define LCD_S12_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<18))
#define LCD_S13_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<20))
#define LCD_S14_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<22))
#define LCD_S15_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<24))
#define LCD_S16_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<26))
#define LCD_S17_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<28))
#define LCD_S18_PORTP1_MUX  ((uint32_t) ((uint32_t) 1<<30))
#define LCD_S19_PORTP2_MUX  ((uint16_t) ((uint16_t) 1<<12))
#define LCD_S20_PORTP2_MUX  ((uint16_t) ((uint16_t) 1<<14))
#define LCD_S21_PORTP2_MUX  ((uint32_t) ((uint32_t) 1<<16))
#define LCD_S22_PORTP2_MUX  ((uint32_t) ((uint32_t) 1<<18))
#define LCD_S23_PORTP2_MUX  ((uint32_t) ((uint32_t) 1<<20))
#define LCD_S24_PORTP2_MUX  ((uint32_t) ((uint32_t) 1<<22))
#define LCD_S25_PORTP2_MUX  ((uint32_t) ((uint32_t) 1<<24))
#define LCD_S26_PORTP2_MUX  ((uint32_t) ((uint32_t) 1<<26))
#define LCD_S27_PORTP2_MUX  ((uint32_t) ((uint32_t) 1<<28))
#define LCD_S28_PORTP2_MUX  ((uint32_t) ((uint32_t) 1<<30))
#define LCD_S29_PORTP3_MUX  ((uint32_t) ((uint32_t) 1<<16))
#define LCD_S30_PORTP3_MUX  ((uint32_t) ((uint32_t) 1<<18))
#define LCD_S31_PORTP3_MUX  ((uint32_t) ((uint32_t) 1<<20))
#define LCD_S32_PORTP3_MUX  ((uint32_t) ((uint32_t) 1<<22))
#define LCD_BPLANE0_PORTP2_MUX  ((uint16_t) ((uint16_t) 1<<0))
#define LCD_BPLANE1_PORTP2_MUX  ((uint16_t) ((uint16_t) 1<<2))
#define LCD_BPLANE2_PORTP2_MUX  ((uint16_t) ((uint16_t) 1<<4))
#define LCD_BPLANE3_PORTP2_MUX  ((uint16_t) ((uint16_t) 1<<6))



int32_t adi_initpinmux(void);

/*
 * Initialize the Port Control MUX Registers
 */
int32_t adi_initpinmux(void) {
    /* Port Control MUX registers */
    *((volatile uint32_t *)REG_GPIO1_GPCON) = LCD_S3_PORTP1_MUX | LCD_S4_PORTP1_MUX
     | LCD_S5_PORTP1_MUX | LCD_S6_PORTP1_MUX | LCD_S7_PORTP1_MUX
     | LCD_S8_PORTP1_MUX | LCD_S9_PORTP1_MUX | LCD_S10_PORTP1_MUX
     | LCD_S11_PORTP1_MUX | LCD_S12_PORTP1_MUX | LCD_S13_PORTP1_MUX
     | LCD_S14_PORTP1_MUX | LCD_S15_PORTP1_MUX | LCD_S16_PORTP1_MUX
     | LCD_S17_PORTP1_MUX | LCD_S18_PORTP1_MUX;
    *((volatile uint32_t *)REG_GPIO2_GPCON) = LCD_S1_PORTP2_MUX | LCD_S2_PORTP2_MUX
     | LCD_S19_PORTP2_MUX | LCD_S20_PORTP2_MUX | LCD_S21_PORTP2_MUX
     | LCD_S22_PORTP2_MUX | LCD_S23_PORTP2_MUX | LCD_S24_PORTP2_MUX
     | LCD_S25_PORTP2_MUX | LCD_S26_PORTP2_MUX | LCD_S27_PORTP2_MUX
     | LCD_S28_PORTP2_MUX | LCD_BPLANE0_PORTP2_MUX | LCD_BPLANE1_PORTP2_MUX
     | LCD_BPLANE2_PORTP2_MUX | LCD_BPLANE3_PORTP2_MUX;
    *((volatile uint32_t *)REG_GPIO3_GPCON) = UART0_ALT_TX_PORTP3_MUX | UART0_ALT_RX_PORTP3_MUX
     | LCD_S29_PORTP3_MUX | LCD_S30_PORTP3_MUX | LCD_S31_PORTP3_MUX
     | LCD_S32_PORTP3_MUX;


    return 0;
}

