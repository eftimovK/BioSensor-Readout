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

/* slave buffers */
static uint8_t slavetx[BUFFERSIZE];
static uint8_t slaverx[BUFFERSIZE];


/* slave buffers */
static uint8_t overtx[BUFFERSIZE];
static uint8_t overrx[BUFFERSIZE];
static uint8_t overpro[5];

static ADI_SPI_DEV_HANDLE hDeviceS;

static ADI_SPI_RESULT_TYPE run_the_slave_test(ADI_SPI_DEV_HANDLE hDevice);
static ADI_SPI_RESULT_TYPE run_the_slave_rx_prologue_then_tx_test(ADI_SPI_DEV_HANDLE hDevice);

static ADI_SPI_DEV_GENERIC_SETTINGS_TYPE settings;

void ThreadSPISlaveRun(void* arg)
{
    /* Initialize SPI1 - Slave */
    if (ADI_SPI_SUCCESS != adi_SPI_SlaveInit(ADI_SPI_DEVID_0, &hDeviceS)) {
        FAIL("Failed to init SPI driver");
    }
    /* throttle bitrate to something the controller can reach */
   if (ADI_SPI_SUCCESS != adi_SPI_SetBitrate(hDeviceS, 10000)) {
        FAIL("adi_SPI_SetBitrate");
    }

    /* test generic settings getter */
    if (ADI_SPI_SUCCESS != adi_SPI_GetGenericSettings(hDeviceS, &settings)) {
        FAIL("Failed to get SPI settings");
    }

    /* blocking */
    if (ADI_SPI_SUCCESS != adi_SPI_SetBlockingMode(hDeviceS, true)) {
        FAIL("Failed to get SPI settings");
    }
#ifdef SPI_DMA
    if (ADI_SPI_SUCCESS != adi_SPI_SetDmaMode(hDeviceS, true)) {
#else
    if (ADI_SPI_SUCCESS != adi_SPI_SetDmaMode(hDeviceS, false)) {
#endif
        FAIL("Failed to get SPI settings");
    }
    
    
#ifdef MASTER_SLAVE_WITH_PROLOGUE
    if (ADI_SPI_SUCCESS != run_the_slave_rx_prologue_then_tx_test(hDeviceS)){
      FAIL("Slave - Data failure");
    }
#else
    if (ADI_SPI_SUCCESS != run_the_slave_test(hDeviceS)){
      FAIL("Slave - Data failure");
    }
#endif

}



ADI_SPI_RESULT_TYPE run_the_slave_rx_prologue_then_tx_test(ADI_SPI_DEV_HANDLE hDevice)
{
    ADI_SPI_RESULT_TYPE result = ADI_SPI_SUCCESS;  /* assume the best */
    ADI_SPI_TRANSCEIVE_TYPE transceive;

    /* initialize both buffers with bad data */
    for (unsigned int i = 0u; i < BUFFERSIZE; i++) {
        slavetx[i] = (unsigned char)i;
        slaverx[i] = (uint8_t)0xdd;
    }

    for (unsigned int i = 0u; i < SLAVEPROLOGUESIZE; i++) {
        overpro[i] = 0x00;
    }

    /* initialize prologue rx */
    transceive.pPrologue = NULL;
    transceive.PrologueSize = 0;
    transceive.DataSize = SLAVEPROLOGUESIZE;
    transceive.pTxData = &overtx[0];
    transceive.pRxData = &overpro[0];
    transceive.bTxIncrement = true;
    transceive.bRxIncrement = true;

    /* prologue receive in non-DMA mode */
    adi_SPI_SetDmaMode(hDevice, false);
    /* receive prologue data */
    adi_SPI_SlaveTransfer(hDevice, &transceive);

    
    /* check slave rx for prologue and respond with tx data in DMA mode */
    if((overpro[0] == PROLOGUE1) && (overpro[1] == PROLOGUE2)){
#ifdef SPI_DMA
        adi_SPI_SetDmaMode(hDevice, true);
#endif
        /* initialize slave tx */
        transceive.pPrologue = NULL;
        transceive.PrologueSize = 0;
        transceive.DataSize = BUFFERSIZE;
        transceive.pTxData = &slavetx[0];
        transceive.pRxData = &slaverx[0];
        transceive.bTxIncrement = true;
        transceive.bRxIncrement = true;
        adi_SPI_SlaveTransfer(hDevice, &transceive);

    }
    return result;
}




ADI_SPI_RESULT_TYPE run_the_slave_test(ADI_SPI_DEV_HANDLE hDevice)
{
    ADI_SPI_RESULT_TYPE result = ADI_SPI_SUCCESS;  /* assume the best */
    ADI_SPI_TRANSCEIVE_TYPE transceive;

    /* initialize both buffers with bad data */
    for (unsigned int i = 0u; i < BUFFERSIZE; i++) {
        slavetx[i] = (unsigned char)i;
        slaverx[i] = (uint8_t)0xdd;
    }

    /* initialize transceive structure for data payload only */
    transceive.pPrologue = NULL;
    transceive.PrologueSize = 0;

    /* link transceive data size to the remaining count */
    transceive.DataSize = BUFFERSIZE;

    /* initialize data attributes */
    transceive.pTxData = &slavetx[0];
    transceive.pRxData = &slaverx[0];

    /* auto increment both buffers */
    transceive.bTxIncrement = true;
    transceive.bRxIncrement = true;

    result = adi_SPI_SlaveTransfer(hDevice, &transceive);

    
    return result;
}