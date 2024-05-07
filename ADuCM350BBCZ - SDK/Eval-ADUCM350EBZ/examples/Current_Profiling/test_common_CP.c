/*********************************************************************************

Copyright (c) 2011-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*!
 *****************************************************************************
 * @file:   test_common.c
 * @brief:  Common utilities for testing
 * @version: $Revision: 29403 $
 * @date:    $Date: 2015-01-08 10:34:56 -0500 (Thu, 08 Jan 2015) $
 *****************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

/* This source file was modified from the original specifically for the Current
 * Profiling example. The only change is that the DEBUG_PRINT conditional       
 * compilation macro is used to ensure that the UART and/or semi-hosting is not 
 * used unintentionally.
 */
   
/* TEST OUTPUT
   define the TEST_COMMON_USES_UART macro in project settings to redirect semi-hosted
   output to the debug console (the default sevice) to the hardware UART device.
*/

#include "CurrentProfiling.h"
#include "test_common.h"

#ifdef TEST_COMMON_USES_UART
ADI_UART_HANDLE hUartDevice = NULL;
#endif

static void quit(void);

/**
 * Initialize the test system, including SystemInit and Wdog
 *
 * @param  none
 * @return none
 *
 * @brief  Set up the test system
 *         Send output to wherever output should go
 */
void test_Init()
{
    ADI_WDT_DEV_HANDLE hWatchdog;

    /* Disable WDT for testing*/
    if( adi_WDT_Init(ADI_WDT_DEVID_0, &hWatchdog) != 0)
    {
        test_Fail("adi_WDT_Init failed, you're probably calling it twice");
    }

    if( adi_WDT_SetEnable(hWatchdog, false) != 0 )
    {
        test_Fail("adi_WDT_SetEnable failed");
    }

    if( adi_WDT_UnInit(hWatchdog) != 0 )
    {
        test_Fail("adi_WDT_UnInit failed");
    }

#ifdef TEST_COMMON_USES_UART
    /* initialize GPIO & UART */
    /* ignore return codes since there's nothing we can do if it fails */
    adi_GPIO_Init();

    /* open UART in blocking, non-intrrpt mode by supplying no internal buffs */
    adi_UART_Init(ADI_UART_DEVID_0, &hUartDevice, NULL);

    /* enable UART */
    adi_UART_Enable(hUartDevice,true);
#endif

}



/**
 * Passing result
 *
 * @param  none
 * @return none
 *
 * @brief  Report a passing test result
 */
void test_Pass()
{
    char pass[] = "PASS!\n\r";

#ifdef TEST_COMMON_USES_UART
    char *p;
    int16_t size;

    p = pass;
    size = strlen(p);

    /* ignore return codes since there's nothing we can do if it fails */
    adi_UART_BufTx(hUartDevice, p, &size);

#else
    printf(pass);
#endif

    /* Once the result is reported, do an abrupt termination */
    quit();
}


/**
 * Failing result
 *
 * @param  none
 * @return none
 *
 * @brief  Report a failing test result
 */
void test_Fail(char *FailureReason)
{
#if (DEBUG_PRINT == 1)
    char fail[] = "FAIL: ";
    char term[] = "\n\r";

#ifdef TEST_COMMON_USES_UART
    int16_t size;

    size = strlen(fail);

    /* ignore return codes since there's nothing we can do if it fails */
    adi_UART_BufTx(hUartDevice, fail, &size);

    size = strlen(FailureReason);

    /* ignore return codes since there's nothing we can do if it fails */
    adi_UART_BufTx(hUartDevice, FailureReason, &size);

    size = strlen(term);

    /* ignore return codes since there's nothing we can do if it fails */
    adi_UART_BufTx(hUartDevice, term, &size);

#else

    printf(fail);
    printf(FailureReason);
    printf(term);

#endif
    /* Once the result is reported, do an abrupt termination */
    quit();
#endif /* DEBUG_PRINT */
}


/**
 * Info
 *
 * @param  none
 * @return none
 *
 * @brief  Report test info
 */
void test_Perf(char *InfoString)
{
    char info[] = "PERF: ";
    char term[] = "\n\r";

#ifdef TEST_COMMON_USES_UART
    int16_t size;

    size = strlen(info);

    /* ignore return codes since there's nothing we can do if it fails */
    adi_UART_BufTx(hUartDevice, info, &size);

    size = strlen(InfoString);

    /* ignore return codes since there's nothing we can do if it fails */
    adi_UART_BufTx(hUartDevice, InfoString, &size);

    size = strlen(term);

    /* ignore return codes since there's nothing we can do if it fails */
    adi_UART_BufTx(hUartDevice, term, &size);

#else

    printf(info);
    printf(InfoString);
    printf(term);

#endif

    /* do not quit... */
}


static void quit(void)
{
#ifdef TEST_COMMON_USES_UART
   adi_UART_BufFlush(hUartDevice);
#endif


#if defined ( __CC_ARM   )
	_sys_exit(0);  /* Keil retargeted implimentation for MicroLib */
#else
   exit(0);
#endif
}
