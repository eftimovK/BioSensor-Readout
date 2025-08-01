/*********************************************************************************

Copyright (c) 2011-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*!
 *****************************************************************************
 * @file:    I2cPerf.c
 * @brief:   I2C Device Performance Test for ADuCxxx
 * @version: $Revision: 28525 $
 * @date:    $Date: 2014-11-12 14:51:26 -0500 (Wed, 12 Nov 2014) $
 *****************************************************************************/

#include <stddef.h>		/* for 'NULL' */
#include <string.h>		/* for strlen */
#include <stdio.h>      /* for sprintf */

#include "test_common.h"
#include "i2c.h"

// This test requires ADI_I2C_LOOPBACK preprocessor
// macro to be defined in the project options.
// This causes the required loopback API to be
// included in the I2C device driver compile.

/** define test size */
#define MAXBYTES 128  // takes a minute or so to pass

/** define guard data for testing data over/under runs */
#define GUARD_BYTE (0xa5u)

/* allocate oversized buffers with guard data */
uint8_t overtx[MAXBYTES+2u];  /*!< Overallocated transmit data buffer with guard data */
uint8_t overrx[MAXBYTES+2u];  /*!< Overallocated receive data buffer with guard data */

/* the "usable" buffers within the overallocated buffers housing the guard data */
uint8_t* tx = &overtx[1];  /*!< Transmit data buffer */
uint8_t* rx = &overrx[1];  /*!< Receive data buffer */

#if (ADI_I2C_CFG_ENABLE_CALLBACK_SUPPORT==1)

    /* callback prototype */
    void cbHandler (void *pcbParam, uint32_t Event, void *pArg);

    /* callback event collector */
    static uint16_t callback_count = 0;
    static uint16_t callback_events = 0;

#endif /* ADI_I2C_CFG_ENABLE_CALLBACK_SUPPORT */

/*  Anything faster than ~30 kHz is physically non-realizable with 1 MHz pclk,
    which is desirable for low-power concerns.  With a 16 MHz pclk, faster I2C
    serial clocks are possible, at the cost of higher power consumption.
*/
#define MASTER_CLOCK 30000

/* manditory call to pinmux plugin generated API */
extern int32_t adi_initpinmux(void);

/*
 * @brief  Function 'main' for I2C example program
 *
 * @param  none
 * @return int (Zero for success, non-zero for failure).
 *
 * A simple application that sets up device I2C0 in loopback
 * mode to test if the I2C controller and basic infastructure
 * is working.
 *
 */
int main(void)
{
    ADI_I2C_DEV_HANDLE hDevice;
    uint16_t slaveID = 0x77;
    uint16_t bytesRemaining, bytecount;

    /* Clock initialization */
    SystemInit();

    /* test system initialization */
    test_Init();

    // init the guard data
    overtx[0] = GUARD_BYTE;
    overrx[0] = GUARD_BYTE;
    overtx[MAXBYTES+1u] = GUARD_BYTE;
    overrx[MAXBYTES+1u] = GUARD_BYTE;

    // init transmit data
    for (unsigned int i = 0u; i < MAXBYTES; i++) {
        tx[i] = (unsigned char)i;
    }

    /* Take HCLK/PCLK down to 1MHz for better power utilization */
    /* Need to set PCLK frequency first, because HCLK frequency */
    /* needs to be greater than or equal to the PCLK frequency  */
    /* at all times.                                            */
    SetSystemClockDivider(ADI_SYS_CLOCK_PCLK, 16);
    SetSystemClockDivider(ADI_SYS_CLOCK_CORE, 16);

    /* Initialize I2C driver */
    if (ADI_I2C_SUCCESS != adi_I2C_MasterInit(ADI_I2C_DEVID_0, &hDevice)) {
        FAIL("adi_I2C_MasterInit");
    }

#if (ADI_I2C_CFG_ENABLE_CALLBACK_SUPPORT==1)

    /* setup a callback */
    if (ADI_I2C_SUCCESS != adi_I2C_RegisterCallback(hDevice, cbHandler, hDevice)) {
        FAIL("adi_I2C_MasterInit");
    }

#endif /* ADI_I2C_CFG_ENABLE_CALLBACK_SUPPORT */

	/* NOTE: adi_I2C_SetMuxing() is formally deprecated.
	   Use of pinmux plugin is now manditory.
	*/
	adi_initpinmux();

    /* select serial bit rate (~30 kHz max with 1MHz pclk)*/
    if (ADI_I2C_SUCCESS != adi_I2C_SetMasterClock(hDevice, MASTER_CLOCK)) {
        FAIL("adi_I2C_SetMasterClock");
    }

    /* disable blocking mode... i.e., poll for completion */
    if (ADI_I2C_SUCCESS != adi_I2C_SetBlockingMode(hDevice, false)) {
        FAIL("adi_I2C_SetBlockingMode");
    }


    // test loop (MAXBYTES iterations, with increasing block size each pass)
	for (bytecount = 1; bytecount < MAXBYTES; bytecount++) {

        // reinit receive buffer with known bad data
        for (unsigned int i = 0u; i < MAXBYTES; i++) {
            rx[i] = 0xdd;
        }

        bytesRemaining = bytecount;

        // transmit and receive the data through the loopback API
        if (ADI_I2C_SUCCESS != (adi_I2C_MasterSlaveLoopback(hDevice, slaveID, tx, rx, bytecount)))
            FAIL("adi_I2C_MasterSlaveLoopback");

        // polling loop...
		// do application-level blocking (if in non-blocking
		// mode -- sse adi_I2C_SetBlockingMode() API) to discover
		// when transaction is complete and during which time,
		// the application can service other tasks...
        while (bytesRemaining) {
            adi_I2C_GetNonBlockingStatus(hDevice, &bytesRemaining);
            // insert other application tasks here, as needed...
        }

		/* verify receive data */
		for (int i = 0; i < bytecount; i++) {
			if (tx[i] != rx[i]) {
				FAIL("Corrupt receive data");
			}
		}

        /* verify guard data */
		if (   (overtx[0] != GUARD_BYTE)
			|| (overrx[0] != GUARD_BYTE)
			|| (overtx[MAXBYTES+1u] != GUARD_BYTE)
			|| (overrx[MAXBYTES+1u] != GUARD_BYTE)) {
				FAIL("Corrupt guard data");
		}
	}

#if (ADI_I2C_CFG_ENABLE_CALLBACK_SUPPORT==1)

    /* report what callback events we collected */
    char str[100];
    sprintf(str, "%d callbacks captured, collectively spanning I2C events: 0x%04x", callback_count, callback_events);
    PERF(str);

#endif /* ADI_I2C_CFG_ENABLE_CALLBACK_SUPPORT */

    /* shut it down */
    if (ADI_I2C_SUCCESS != adi_I2C_UnInit(hDevice)) {
        FAIL("adi_I2C_UnInit");
    }

    /* success if we get this far... */
    PASS();
}

#if (ADI_I2C_CFG_ENABLE_CALLBACK_SUPPORT==1)

/* Callback Handler */
void cbHandler (void *pcbParam, uint32_t Event, void *pArg)
{
    /* collect the events */
    callback_count++;
    callback_events |= Event;
}

#endif /* ADI_I2C_CFG_ENABLE_CALLBACK_SUPPORT */

/*
** EOF
*/
