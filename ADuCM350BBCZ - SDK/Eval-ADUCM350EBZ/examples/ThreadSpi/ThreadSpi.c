/*********************************************************************************

Copyright (c) 2011-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using
this software you agree to the terms of the associated Analog Devices Software
License Agreement.

*********************************************************************************/

/*!
 *****************************************************************************
 * @file:    ThreadSpi.c
 * @brief:   SPI Device Test for ADuCxxx
 * @version: $Revision: 29075 $
 * @date:    $Date: 2014-12-08 14:25:13 -0500 (Mon, 08 Dec 2014) $
 *****************************************************************************/

/*  ThreadSPI
 *  This example code demonstrates the SPI device driver running under RTOS.
 *  An external SPI data loopback is perfomed across the MOSI/MISO data lines.
 *  The example creates two tasks and one semaphore, the first task initiates an
 *  SPI transceiver transaction, the second task monitors for the condition when
 *  the SPI transaction has completed, after which it passs a semaphore back to
 *  the first task when the process can start again.
 *  Loopback which requires a hardware jumper between the MISO & MOSI pins of
 *  SPI1, as follows - pins P3.5 & P3.6 on EVAL-ADuM350-MOCZ/Rev0 board
 */

#include <string.h>
#include "test_common.h"
#include "gpio.h"
#include "spi.h"
#include "system.h"
#include "includes.h"
#include "app_cfg.h"

#define TASK1_PRIO 6
#define TASK1_STK_SIZE 200

#define TASK2_PRIO 7
#define TASK2_STK_SIZE 200

/** define size of data buffers */
#define BUFFERSIZE 250

/** define data for prologue testing */
#define PROLOGUE "Hello World"

/** define guard data for testing data over/under runs */
#define GUARD_BYTE (0xa5u)

/* external pinmux code */
extern int32_t adi_initpinmux(void);

/* allocate oversized buffers with guard data */
uint8_t overtx[BUFFERSIZE+4u];  /*!< Overallocated transmit data buffer with guard data */
uint8_t overrx[BUFFERSIZE+4u];  /*!< Overallocated receive data buffer with guard data */

/* the "usable" buffers within the overallocated buffers housing the guard data */
uint8_t* tx = &overtx[2];  /*!< Transmit data buffer */
uint8_t* rx = &overrx[2];  /*!< Receive data buffer */

ADI_SPI_TRANSCEIVE_TYPE transceiver;

ADI_SPI_DEV_HANDLE hDevice;
OS_EVENT *pSem;

uint32_t TimerCount;

/* local functions */
static CPU_STK Task1Stack[TASK1_STK_SIZE];
static CPU_STK Task2Stack[TASK2_STK_SIZE];
static void Task1(void* arg);
static void Task2(void* arg);
static void InitSpiTransceiver(void);
static uint32_t CheckSpiTransceiver(void);

/*!
 * @brief  Function 'main' for OSAL example program
 *
 * @param  none
 * @return
 *
 * OSAL.
 *
 */
int main(void)
{

    /* Clock initialization */
    SystemInit();

    /* test system initialization */
    test_Init();

    /* initialize the OS */
    OSInit();

    /* init the GPIO pins */
    adi_initpinmux();

    /* initialise SPI transcceiver test data */
    InitSpiTransceiver();

    /* task that submits a SPI transceiver */
    if (OSTaskCreate(Task1, NULL, &Task1Stack[0]+TASK1_STK_SIZE, TASK1_PRIO))
        FAIL("Failed on OSTaskCreate()");

    /* task that monitors when the SPI transceiver has completed */
    if (OSTaskCreate(Task2, NULL, &Task2Stack[0]+TASK2_STK_SIZE, TASK2_PRIO))
        FAIL("Failed on OSTaskCreate()");

    /* create the semaphore that is passed between task 1&2 */
    if (0 == (pSem = OSSemCreate(0u)))
        FAIL("Failed on OSSemCreate()");

    /* start the OS */
    OSStart();

    return 0;
}

static void Task1(void* arg)
{
   uint8_t err;

    while(1)
    {
        TimerCount = 0;

        /* Initialize SPI1, by default non-DMA, non-blocking mode */
        if (adi_SPI_MasterInit(ADI_SPI_DEVID_1, &hDevice))
            FAIL("Failed on adi_SPI_MasterInit()");

        /* enable blocking mode */
        if (adi_SPI_SetBlockingMode(hDevice, true))
            FAIL("Failed on adi_SPI_SetBlockingMode()");

        /* enable DMA mode */
        if (adi_SPI_SetDmaMode(hDevice, true))
            FAIL("Failed on adi_SPI_SetDmaMode()");

        /* throttle bitrate to something the controller can reach */
        if (adi_SPI_SetBitrate(hDevice, 250000))
            FAIL("Failed on adi_SPI_SetBitrate()");

        /* launch the transfer */
        if (adi_SPI_MasterTransfer(hDevice, &transceiver))
            FAIL("Failed on adi_SPI_MasterTransfer()");

        /* wait for a semaphore that signifies an SPI transceiver completion */
        OSSemPend(pSem, 0,&err);

        if (OS_ERR_NONE != err)
            FAIL("Failed on OSSemPend()");

        /* check SPI loopback data */
        if (CheckSpiTransceiver())
            FAIL("Failed on CheckSpiTransceiver()");

        /* shut it down */
        if (adi_SPI_UnInit(hDevice))
            FAIL("Failed on adi_SPI_UnInit()");

        PASS();
    }
}

static void Task2(void* arg)
{
    while(1)
    {

        while(TimerCount < 0xFFFF)
        {
            TimerCount++;
        }

        /* polling loop... */
        while (!adi_SPI_MasterComplete(hDevice));

        /* SPI transceiver is complete, pass semaphore */
        OSSemPost(pSem);
    }
}

static void InitSpiTransceiver(void)
{
    /* set SPI transceiver data */

    /* set guard data */
    overtx[0] = GUARD_BYTE;
    overtx[1] = GUARD_BYTE;
    overtx[BUFFERSIZE+2] = GUARD_BYTE;
    overtx[BUFFERSIZE+3] = GUARD_BYTE;
    overrx[0] = GUARD_BYTE;
    overrx[1] = GUARD_BYTE;
    overrx[BUFFERSIZE+2] = GUARD_BYTE;
    overrx[BUFFERSIZE+3] = GUARD_BYTE;

    /* initialize transceiver structure for data payload only */
    transceiver.pPrologue = PROLOGUE;
    transceiver.PrologueSize = strlen(PROLOGUE);

    /* link transceiver data size to the remaining count */
    transceiver.DataSize = BUFFERSIZE;

    /* initialize data attributes */
    transceiver.pTxData = tx;  transceiver.pRxData = rx;

    /* auto increment both buffers */
    transceiver.bTxIncrement = true;
    transceiver.bRxIncrement = true;

    /* initialize both buffers with bad data */
    for (unsigned int i = 0u; i < BUFFERSIZE; i++) {
      tx[i] = (unsigned char)i;
      rx[i] = (uint8_t)0xdd;
    }
}

static uint32_t CheckSpiTransceiver(void)
{

    uint32_t Result = 0u;

    /* verify the guard data */
    if(   (overtx[0] != GUARD_BYTE)            || (overtx[1] != GUARD_BYTE)
        || (overtx[BUFFERSIZE+2] != GUARD_BYTE) || (overtx[BUFFERSIZE+3u] != GUARD_BYTE)
        || (overrx[0] != GUARD_BYTE)            || (overrx[1] != GUARD_BYTE)
        || (overrx[BUFFERSIZE+2] != GUARD_BYTE) || (overrx[BUFFERSIZE+3u] != GUARD_BYTE)) {
        Result = 1u;
    }

    /* verify receive data */
    if (Result == ADI_SPI_SUCCESS) {
        for (unsigned int i = 0u; i < BUFFERSIZE; i++) {
            if (tx[i] != rx[i]) {
                Result = 1u;
                break;
            }
        }
    }
    return(Result);
}

