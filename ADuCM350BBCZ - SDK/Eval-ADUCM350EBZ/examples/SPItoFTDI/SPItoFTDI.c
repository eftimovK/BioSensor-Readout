/*********************************************************************************

Copyright (c) 2011-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using
this software you agree to the terms of the associated Analog Devices Software
License Agreement.

*********************************************************************************/

/*!
 *****************************************************************************
 * @file:    SpiToFTDI.c
 * @brief:   SPI Device Test for ADuCxxx
 * @version: $Revision: 33017 $
 * @date:    $Date: 2015-12-15 11:35:38 -0500 (Tue, 15 Dec 2015) $
 *****************************************************************************/


/* Apply ADI MISRA Suppressions */
#define ASSERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"

#include <stddef.h>		/* for 'NULL' */
#include <string.h>		/* for strlen */

#include "test_common.h"
#include "spi.h"

extern int32_t adi_initpinmux(void);
extern void ftdi(void);

void spiTest(ADI_SPI_DEV_ID_TYPE const devID);

/*
 * Function 'main' for SPItoFTDI example program
 *
 *
 */
int main(void)
{
    /* Clock initialization */
    SystemInit();

    /* test system initialization */
    test_Init();

    /* Use static pinmuxing */
    adi_initpinmux();

    /* Call the routine that will interface to the FT4222 */
    ftdi();

    /* success if we get this far... */
    PASS();
}


/*
** EOF
*/

/*@}*/
