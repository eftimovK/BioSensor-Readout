/*********************************************************************************

Copyright (c) 2011-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using
this software you agree to the terms of the associated Analog Devices Software
License Agreement.

*********************************************************************************/

/*!
 *****************************************************************************
 * @file:    SpiTest.c
 * @brief:   SPI Device Test for ADuCxxx
 * @version: $Revision: 33012 $
 * @date:    $Date: 2015-12-14 15:51:59 -0500 (Mon, 14 Dec 2015) $
 *****************************************************************************/

/*! \addtogroup SPI_Test SPI Test
 *  Example code demonstrating use of the SPI device driver.
 *  @{
 */

/* Apply ADI MISRA Suppressions */
#define ASSERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"

#include <stddef.h>		/* for 'NULL' */
#include <string.h>		/* for strlen */

#include "test_common.h"
#include "spi.h"


/* testing is done using SPI0 and SPI1 device                  */
/* define the following macro for testing external (off-chip)  */
/* loopback which requires a hardware jumper between the       */
/* MISO & MOSI pins for SPI0 and SPI1, as follows:             */
/* On ADuCM350EBZ target board:                                */
/*	(SPI0) P3.1 & P3.2, i.e., J3:12 & J3:28 of breakout board on digital header 1 DH1  */
/*	(SPI1) P3.5 & P3.6, i.e., J3:24 & J3:16 of breakout board on digital header 1 DH1  */
/*                                                             */
/* otherwise undefine and internal loopback will be used       */
#define EXTERNAL_LOOPBACK

/** define size of data buffers, must be even-sized for DMA. */
#define BUFFERSIZE 1024u

/** define data for prologue testing */
#define PROLOGUE "Hello World"

/** define guard data for testing data over/under runs */
#define GUARD_BYTE (0xa5u)

/* allocate oversized buffers with guard data */
uint8_t overtx[BUFFERSIZE+4u];  /*!< Overallocated transmit data buffer with guard data */
uint8_t overrx[BUFFERSIZE+4u];  /*!< Overallocated receive data buffer with guard data */

/* the "usable" buffers within the overallocated buffers housing the guard data */
uint8_t* tx = &overtx[2];  /*!< Transmit data buffer */
uint8_t* rx = &overrx[2];  /*!< Receive data buffer */

/* the tests... */
ADI_SPI_RESULT_TYPE test_blocking_mode(ADI_SPI_DEV_HANDLE hDevice);
ADI_SPI_RESULT_TYPE test_non_blocking_mode(ADI_SPI_DEV_HANDLE hDevice);
ADI_SPI_RESULT_TYPE test_dma_blocking_mode(ADI_SPI_DEV_HANDLE hDevice);
ADI_SPI_RESULT_TYPE test_dma_non_blocking_mode(ADI_SPI_DEV_HANDLE hDevice);

ADI_SPI_RESULT_TYPE run_the_test(ADI_SPI_DEV_HANDLE hDevice);

extern int32_t adi_initpinmux(void);
extern void ftdi(void);

void spiTest(ADI_SPI_DEV_ID_TYPE const devID);

/*!
 * @brief  Function 'main' for SPI example program
 *
 * @param  none
 * @return int (Zero for success, non-zero for failure).
 *
 * A simple application that sets up device SPI in loopback
 * mode to test if the SPI controller and basic infastructure
 * is working.
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

    /* install and enable DMA error handeling with OSAL */
    ADI_INSTALL_HANDLER(DMA_ERR_IRQn, DMA_Err_Int_Handler);
    ADI_ENABLE_INT(DMA_ERR_IRQn);


    spiTest(ADI_SPI_DEVID_0); /* SPI 0 */
    spiTest(ADI_SPI_DEVID_1); /* SPI 1 */


    
    /* success if we get this far... */
    PASS();
}

/*!
 * @brief  Function 'spiTest' for SPI example program
 *
 * @param  devID SPI device to test
 * @return
 *
 * A simple application that sets up device SPI in loopback
 * mode to test if the SPI controller and basic infastructure
 * is working.
 *
 */
void spiTest(ADI_SPI_DEV_ID_TYPE const devID)
{
    ADI_SPI_DEV_HANDLE hDevice;
    ADI_SPI_DEV_GENERIC_SETTINGS_TYPE settings;

    /* Initialize SPI */
    if (ADI_SPI_SUCCESS != adi_SPI_MasterInit(devID, &hDevice)) {
        FAIL("Failed to init SPI driver");
    }

    /* throttle bitrate to something the controller can reach */
    if (ADI_SPI_SUCCESS != adi_SPI_SetBitrate(hDevice, 250000)) {
        FAIL("adi_SPI_SetBitrate");
    }

    if (ADI_SPI_SUCCESS != adi_SPI_SetChipSelect(hDevice, ADI_GPIO_PORT_3, ADI_GPIO_PIN_3)) {
        FAIL("adi_SPI_SetChipSelect");
    }

    /* test generic settings getter */
    if (ADI_SPI_SUCCESS != adi_SPI_GetGenericSettings(hDevice, &settings)) {
        FAIL("Failed to get SPI settings");
    }

    /* test non-dma blocking mode */
    if (ADI_SPI_SUCCESS != test_blocking_mode(hDevice)) {
        FAIL("SPI interrupt-driven blocking mode test failed");
    }

    /* test non-dma non-blocking mode */
    if (ADI_SPI_SUCCESS != test_non_blocking_mode(hDevice)) {
        FAIL("SPI interrupt-driven non-blocking mode test failed");
    }

    /* test dma-based blocking mode */
    if (ADI_SPI_SUCCESS != test_dma_blocking_mode(hDevice)) {
        FAIL("SPI dma-driven blocking mode test failed");
    }

    /* test dma-based non-blocking mode */
    if (ADI_SPI_SUCCESS != test_dma_non_blocking_mode(hDevice)) {
        FAIL("SPI dma-driven non-blocking mode test failed");
    }

    /* shut it down */
    if (ADI_SPI_SUCCESS != adi_SPI_UnInit(hDevice)) {
        FAIL("Failed to uninit SPI driver");
    }
}


/*!
 * @brief  Setup for non-dma blocking mode test
 *
 * @param[in]      hDevice Device handle obtained from adi_SPI_Init().
 * @return         ADI_SPI_RESULT_TYPE.
 *
 * Configure SPI and test non-dma, blocking mode data transfers.
 *
 * @sa        run_the_test().
 */
ADI_SPI_RESULT_TYPE test_blocking_mode (ADI_SPI_DEV_HANDLE hDevice)
{
    ADI_SPI_RESULT_TYPE result = ADI_SPI_SUCCESS;  /* assume the best */

#ifndef EXTERNAL_LOOPBACK
    /* enable internal loopback mode */
    if (!result)
      result = adi_SPI_SetLoopback(hDevice, true);
#endif

    /* enable blocking mode */
    if (!result) {
        result = adi_SPI_SetBlockingMode(hDevice, true);
    }

    /* disable dma */
    if (!result) {
        result = adi_SPI_SetDmaMode(hDevice, false);
    }

    if (!result) {
        result = run_the_test(hDevice);
    }

    return result;
}


/*!
 * @brief  Setup for non-dma non-blocking mode test
 *
 * @param[in]      hDevice Device handle obtained from adi_SPI_Init().
 * @return         ADI_SPI_RESULT_TYPE.
 *
 * Configure SPI and test non-dma, non-blocking mode data transfers.
 *
 * @sa        run_the_test().
 */
ADI_SPI_RESULT_TYPE test_non_blocking_mode(ADI_SPI_DEV_HANDLE hDevice)
{
    ADI_SPI_RESULT_TYPE result = ADI_SPI_SUCCESS;  /* assume the best */

#ifndef EXTERNAL_LOOPBACK
    /* enable internal loopback mode */
    if (!result)
      result = adi_SPI_SetLoopback(hDevice, true);
#endif

    /* disble blocking mode */
    if (!result) {
        result = adi_SPI_SetBlockingMode(hDevice, false);
    }

    /* disable dma */
    if (!result) {
        result = adi_SPI_SetDmaMode(hDevice, false);
    }

    if (!result) {
      result = run_the_test(hDevice);
    }

    return result;
}


/*!
 * @brief  Setup for dma-based blocking mode test
 *
 * @param[in]      hDevice Device handle obtained from adi_SPI_Init().
 * @return         ADI_SPI_RESULT_TYPE.
 *
 * Configure SPI and test dma-based, blocking mode data transfers.
 *
 * @sa        run_the_test().
 */
ADI_SPI_RESULT_TYPE test_dma_blocking_mode (ADI_SPI_DEV_HANDLE hDevice)
{
    ADI_SPI_RESULT_TYPE result = ADI_SPI_SUCCESS;  /* assume the best */

#ifndef EXTERNAL_LOOPBACK
    /* enable internal loopback mode */
    if (!result)
      result = adi_SPI_SetLoopback(hDevice, true);
#endif

    /* enable blocking mode */
    if (!result) {
        result = adi_SPI_SetBlockingMode(hDevice, true);
    }

    /* enable dma */
    if (!result) {
        result = adi_SPI_SetDmaMode(hDevice, true);
    }

    /* eliminate clock stalls between bytes */
    adi_SPI_SetContinousMode(hDevice, true);

    if (!result) {
        result = run_the_test(hDevice);
    }

    /* disable dma */
    if (!result) {
        result = adi_SPI_SetDmaMode(hDevice, false);
    }

    return result;
}


/*!
 * @brief  Setup for dma-based non-blocking mode test
 *
 * @param[in]      hDevice Device handle obtained from adi_SPI_Init().
 * @return         ADI_SPI_RESULT_TYPE.
 *
 * Configure SPI and test dma-based, non-blocking mode data transfers.
 *
 * @sa        run_the_test().
 */
ADI_SPI_RESULT_TYPE test_dma_non_blocking_mode(ADI_SPI_DEV_HANDLE hDevice)
{
    ADI_SPI_RESULT_TYPE result = ADI_SPI_SUCCESS;  /* assume the best */

#ifndef EXTERNAL_LOOPBACK
    /* enable internal loopback mode */
    if (!result) {
        result = adi_SPI_SetLoopback(hDevice, true);
    }
#endif

    /* disble blocking mode */
    if (!result) {
        result = adi_SPI_SetBlockingMode(hDevice, false);
    }

    /* enable dma */
    if (!result) {
        result = adi_SPI_SetDmaMode(hDevice, true);
    }

    /* eliminate clock stalls between bytes */
    adi_SPI_SetContinousMode(hDevice, true);

    if (!result) {
        result = run_the_test(hDevice);
    }

    /* disable dma */
    if (!result) {
        result = adi_SPI_SetDmaMode(hDevice, false);
    }

    return result;
}


/*!
 * @brief  Shared routine to run a previously configured test.
 *
 * @param[in]      hDevice Device handle obtained from adi_SPI_Init().
 * @return         ADI_SPI_RESULT_TYPE.
 *
 * Execute an SPI data transfer based on current test configuration.
 *
 * @sa        test_blocking_mode().
 * @sa        test_non_blocking_mode().
 * @sa        test_dma_blocking_mode().
 * @sa        test_dma_non_blocking_mode().
 */
ADI_SPI_RESULT_TYPE run_the_test(ADI_SPI_DEV_HANDLE hDevice)
{
    ADI_SPI_RESULT_TYPE result = ADI_SPI_SUCCESS;  /* assume the best */
    ADI_SPI_TRANSCEIVE_TYPE transceive;

    /* set guard data */
    overtx[0] = GUARD_BYTE;
    overtx[1] = GUARD_BYTE;
    overtx[BUFFERSIZE+2] = GUARD_BYTE;
    overtx[BUFFERSIZE+3] = GUARD_BYTE;
    overrx[0] = GUARD_BYTE;
    overrx[1] = GUARD_BYTE;
    overrx[BUFFERSIZE+2] = GUARD_BYTE;
    overrx[BUFFERSIZE+3] = GUARD_BYTE;

    /* initialize both buffers with bad data */
    for (unsigned int i = 0u; i < BUFFERSIZE; i++) {
        tx[i] = (unsigned char)i;
        rx[i] = (uint8_t)0xdd;
    }

    /* initialize transceive structure for data payload only */
    transceive.pPrologue = PROLOGUE;
    transceive.PrologueSize = strlen(PROLOGUE);

    /* link transceive data size to the remaining count */
    transceive.DataSize = BUFFERSIZE;

    /* initialize data attributes */
    transceive.pTxData = tx;
    transceive.pRxData = rx;

    /* auto increment both buffers */
    transceive.bTxIncrement = true;
    transceive.bRxIncrement = true;

    // submit the call
	if (ADI_SPI_SUCCESS != (result = adi_SPI_MasterTransfer(hDevice, &transceive)))
		return result;

    // poll on non-blocking tests...
    if (!adi_SPI_GetBlockingMode(hDevice)) {

    	if (adi_SPI_GetDmaMode(hDevice)) {

    		// DMA polling loop...
       		while (!adi_SPI_GetDmaTxComplete(hDevice) || !adi_SPI_GetDmaRxComplete(hDevice)) ;

       	} else {

       		// non-DMA polling loop...
       		while (!adi_SPI_MasterComplete(hDevice)) ;
       	}
    }

    /* verify the guard data */
    if (   (overtx[0] != GUARD_BYTE)            || (overtx[1] != GUARD_BYTE)
        || (overtx[BUFFERSIZE+2] != GUARD_BYTE) || (overtx[BUFFERSIZE+3u] != GUARD_BYTE)
        || (overrx[0] != GUARD_BYTE)            || (overrx[1] != GUARD_BYTE)
        || (overrx[BUFFERSIZE+2] != GUARD_BYTE) || (overrx[BUFFERSIZE+3u] != GUARD_BYTE)) {
          result = ADI_SPI_ERR_UNKNOWN;
    }

    /* verify receive data */
    if (!result) {
        for (unsigned int i = 0u; i < BUFFERSIZE; i++) {
            if (tx[i] != rx[i]) {
                result = ADI_SPI_ERR_UNKNOWN;  /* failure */
                break;
            }
        }
    }

    return result;
}


/***************************DMA ERROR HANDLER***************************\
| DMA error handler to process expected DMA error interrupts resulting  |
| from "invalid" DMA descriptors that are expressly intended to mark    |
| completion of DMA ping-pong descriptor sequences for large blocks of  |
| data.                                                                 |
\***********************************************************************/
ADI_INT_HANDLER(DMA_Err_Int_Handler) {

/* all DMA channels in use by SPI */
#define SPI_DMA_CHANNELS ( 1 << SPIH_TX_CHANn | 1 << SPIH_RX_CHANn \
                         | 1 << SPI0_TX_CHANn | 1 << SPI0_RX_CHANn \
                         | 1 << SPI1_TX_CHANn | 1 << SPI1_RX_CHANn )

    /* get the channel descriptor error bits from the DMA controller */
    uint32_t invalidDescriptorBits  = pADI_DMA->DMAINVALIDDESCCLR & SPI_DMA_CHANNELS;

        /* if an expected SPI channel has a descriptor error, clear it and proceed */
    if (invalidDescriptorBits) {
        pADI_DMA->DMAINVALIDDESCCLR = invalidDescriptorBits;
    } else {
        /* otherwise, it is an unexpected non-SPI dma descriptor error, a bus fault,
           some other more generic DMA error, etc., all of which we want to trap on.
        */
        while(1)
            ;
    }
}

/* Revert ADI MISRA Suppressions */
#define REVERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"

/*
** EOF
*/

/*@}*/
