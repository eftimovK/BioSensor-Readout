/*********************************************************************************

Copyright (c) 2011-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*!
 *****************************************************************************
 * @file:    UartTest.c
 * @brief:   UartTest Device Example for ADuCxxx
 * @version: $Revision: 28544 $
 * @date:    $Date: 2014-11-13 13:09:44 -0500 (Thu, 13 Nov 2014) $
 *****************************************************************************/

/** \addtogroup Uart_Example UART Example
 *  Example code demonstrating use of the UART device driver.
 *  @{
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#ifndef _ILONG
#error Needs _ILONG
#endif

#include "wut.h"

#include "test_common.h"
#include <uart.h>

#include <ADuCM350_device.h>

#if defined ( __ICCARM__ )  // IAR compiler...
/* Apply ADI MISRA Suppressions */
#define ASSERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"
#endif

extern int32_t adi_initpinmux(void);

#define XITERATIONS 10
#define YITERATIONS 10
#define ZITERATIONS 16

static const char vsTestString[] = "TEST String";


static void runTest(ADI_UART_HANDLE hDevice, uint32_t iterations, bool bInterrupt, bool bBlocking)
{
    char vRecvBuf[ZITERATIONS] = "";
    uint32_t i;
    ADI_UART_RESULT_TYPE uartResult;

    uartResult = adi_UART_SetInterruptMode(hDevice, bInterrupt);

    if (ADI_UART_SUCCESS != uartResult)
    {
        test_Fail("testInterruptNonblockingMode: adi_UART_SetInterruptMode() failed");
    }

    uartResult = adi_UART_SetBlockingMode(hDevice, bBlocking);

    if (ADI_UART_SUCCESS != uartResult)
    {
        test_Fail("testInterruptNonblockingMode: adi_UART_SetBlockingMode() failed");
    }

    uartResult = adi_UART_Enable(hDevice, true);

    if (ADI_UART_SUCCESS != uartResult)
    {
        test_Fail("testInterruptNonblockingMode: adi_UART_Enable(true) failed");
    }

    /* Repeat the data transfer for the specified number of iterations
     */
    for (i = 0;
         i < iterations;
         ++i)
    {
        int16_t numChars = sizeof(vsTestString);
        uint32_t charsTxed = 0u;
        uint32_t charsRxed = 0u;

        /* Loop until all bytes have been sent and received */
        do
        {
          int16_t txSize = numChars - charsTxed;
          int16_t rxSize = numChars - charsRxed;

          if (txSize > 0)
          {
              uartResult = adi_UART_BufTx(hDevice, vsTestString + charsTxed, &txSize);

              if (ADI_UART_SUCCESS != uartResult)
              {
                  test_Fail("testInterruptNonblockingMode: adi_UART_BufTx() failed");
              }

              charsTxed += txSize;
          }

          uartResult = adi_UART_BufRx(hDevice, vRecvBuf + charsRxed, &rxSize);

          if (ADI_UART_SUCCESS != uartResult)
          {
              test_Fail("testInterruptNonblockingMode: adi_UART_BufRx() failed");
          }

          charsRxed += rxSize;

        } while((charsTxed < numChars) || (charsRxed < numChars));
    }

    if (0 != strcmp(vsTestString, vRecvBuf))
    {
        test_Fail("testInterruptNonblockingMode: Rx string doesn't match Tx string");
    }

    uartResult = adi_UART_Enable(hDevice, false);

    if (ADI_UART_SUCCESS != uartResult)
    {
        test_Fail("testInterruptNonblockingMode: adi_UART_Enable(false) failed");
    }
}


/**
 * @brief  Function 'main' for the UART test program.
 *
 * @return int (Zero for success, non-zero for failure).
 *
 *
 *
 *
 */
int main()
{
    ADI_UART_HANDLE      hDevice;
    ADI_UART_RESULT_TYPE uartResult;
    ADI_UART_INIT_DATA   initData;
    static uint8_t rxBuffer[ZITERATIONS];
    static uint8_t txBuffer[ZITERATIONS];
    int y,z;

    /* Clock initialization */
    SystemInit();

    /* NVIC initialization */
    NVIC_SetPriorityGrouping(12);

    // Change HCLK clock divider to 1 for a 16MHz clock
    if (ADI_SYS_SUCCESS != SetSystemClockDivider(ADI_SYS_CLOCK_CORE, 1))
    {
            test_Fail("SetSystemClockDivider() failed");
    }
    /* PCLK is running as 20MHz for ADuCM320. This is done in System Init function */
    // Change PCLK clock divider to 1 for a 16MHz clock
    if (ADI_SYS_SUCCESS != SetSystemClockDivider (ADI_SYS_CLOCK_UART, 1))
    {
            test_Fail("SetSystemClockDivider() failed");
    }

    /* Use static pinmuxing */
    adi_initpinmux();

    for (z = sizeof(vsTestString); // buffer sizes must be big enough for the test string
         z <= ZITERATIONS;
         ++z)
    {
        initData.pRxBufferData = rxBuffer;
        initData.RxBufferSize = z;
        initData.pTxBufferData = txBuffer;
        initData.TxBufferSize = z;

        uartResult = adi_UART_Init(ADI_UART_DEVID_0, &hDevice, &initData);

        if (ADI_UART_SUCCESS != uartResult)
        {
            test_Fail("adi_UART_Init() failed");
        }

        uartResult = adi_UART_SetLoopback(hDevice, true);

        if (ADI_UART_SUCCESS != uartResult)
        {
            test_Fail("adi_UART_SetLoopback() failed");
        }

        for(y = 0;
            y < YITERATIONS;
            ++y)
        {
            ADI_UART_BAUDRATE_TYPE baudRate =
                (ADI_UART_BAUDRATE_TYPE)(y % ADI_UART_BAUD_MAX_ENTRIES);
            static uint32_t vBaudCheckTable[ADI_UART_BAUD_MAX_ENTRIES] =
            {
                9600u,
                19200,
                38400,
                57600,
                115200,
                230400,
                460800
            };
            static uint32_t checkBaud;
            static uint32_t diff;

            uartResult = adi_UART_SetBaudRate(hDevice, baudRate);

            if (ADI_UART_SUCCESS != uartResult)
            {
                test_Fail("adi_UART_SetBaudRate() failed");
            }

            // Interrupt mode
            runTest(hDevice, XITERATIONS, true, false); // non-blocking
            runTest(hDevice, XITERATIONS, true, true);  // blocking

            // Polled mode
            runTest(hDevice, XITERATIONS, false, false); // non-blocking
            // The test can't be run in polled-blocking mode

            checkBaud = adi_UART_GetBaudRate(hDevice);
            diff = abs((int)vBaudCheckTable[baudRate] - (int)checkBaud);

            if (diff > (vBaudCheckTable[baudRate] / 100u))
            {
                test_Fail("Baud rate doesn't match check table");
            }
        }

        uartResult = adi_UART_UnInit(hDevice);

        if (ADI_UART_SUCCESS != uartResult)
        {
            test_Fail("adi_UART_UnInit");
        }
    }

    // If we've got here then the test has passed
    test_Pass();

    return 0;
}

#if defined ( __ICCARM__ )  // IAR compiler...
/* Revert ADI MISRA Suppressions */
#define REVERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"
#endif


/*
** EOF
*/

/*@}*/
