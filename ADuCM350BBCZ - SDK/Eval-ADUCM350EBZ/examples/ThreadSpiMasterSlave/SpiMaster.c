/*********************************************************************************

Copyright (c) 2011-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

#include "system.h"
#include <ucos_ii.h>
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "spi.h"
#include "SpiMasterSlave.h"

/* master buffers */
static uint8_t overtx[BUFFERSIZE];
static uint8_t overrx[BUFFERSIZE];
static uint8_t overpro[BUFFERSIZE];

static ADI_SPI_DEV_HANDLE hDeviceM;

static ADI_SPI_RESULT_TYPE run_the_master_test(ADI_SPI_DEV_HANDLE hDevice);

static ADI_SPI_DEV_GENERIC_SETTINGS_TYPE settings;

void ThreadSPIMasterRun(void* arg)
{
    /* Initialize SPI1 - Master */
    if (ADI_SPI_SUCCESS != adi_SPI_MasterInit(ADI_SPI_DEVID_1, &hDeviceM)) {
        FAIL("Failed to init SPI driver");
    }
    /* throttle bitrate to something the controller can reach */
   if (ADI_SPI_SUCCESS != adi_SPI_SetBitrate(hDeviceM, 10000)) {
        FAIL("adi_SPI_SetBitrate");
    }

    /* test generic settings getter */
    if (ADI_SPI_SUCCESS != adi_SPI_GetGenericSettings(hDeviceM, &settings)) {
        FAIL("Failed to get SPI settings");
    }

    /* blocking */
    if (ADI_SPI_SUCCESS != adi_SPI_SetBlockingMode(hDeviceM, true)) {
        FAIL("Failed to get SPI settings");
    }
#ifdef SPI_DMA
    if (ADI_SPI_SUCCESS != adi_SPI_SetDmaMode(hDeviceM, true)) {
#else
    if (ADI_SPI_SUCCESS != adi_SPI_SetDmaMode(hDeviceM, false)) {
#endif
        FAIL("Failed to get SPI settings");
    }
    
    
    if (ADI_SPI_SUCCESS != run_the_master_test(hDeviceM)){
      FAIL("Master - Data failure");
    }
    
    /* This will print "pass" and exit the program */
    test_Pass();

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
static ADI_SPI_RESULT_TYPE run_the_master_test(ADI_SPI_DEV_HANDLE hDevice)
{
    ADI_SPI_RESULT_TYPE result = ADI_SPI_SUCCESS;  /* assume the best */
    ADI_SPI_TRANSCEIVE_TYPE transceive;

    /* initialize both buffers with bad data */
    for (unsigned int i = 0u; i < BUFFERSIZE+2; i++) {
        overtx[i] = (unsigned char)i;
        overrx[i] = (uint8_t)0xdd;
    }

    overpro[0] = PROLOGUE1;
    overpro[1] = PROLOGUE2;
    overpro[2] = DUMMY;
    overpro[3] = DUMMY;
    overpro[4] = DUMMY;

    /* initialize transceive structure for data payload only */
#ifdef MASTER_SLAVE_WITH_PROLOGUE
    transceive.pPrologue = &overpro[0];
    transceive.PrologueSize = MASTERPROLOGUESIZE;
#else
    transceive.pPrologue = NULL;
    transceive.PrologueSize = 0;
#endif

    /* transceive data size, must be at least one byte greater than slave in order to flush last byte from slave */
    transceive.DataSize = BUFFERSIZE +2;

    /* initialize data attributes */
    transceive.pTxData = &overtx[0];
    transceive.pRxData = &overrx[0];

    /* auto increment both buffers */
    transceive.bTxIncrement = true;
    transceive.bRxIncrement = true;

    // submit the call
    if (ADI_SPI_SUCCESS != (result = adi_SPI_MasterTransfer(hDevice, &transceive)))
        return result;

    
    /* verify results */
    for (unsigned int i = 0u; i < BUFFERSIZE; i++) {
      if(overrx[i] != (unsigned char)(i)){
        result = ADI_SPI_ERR_UNKNOWN;
      }
    }
    return result;
}

