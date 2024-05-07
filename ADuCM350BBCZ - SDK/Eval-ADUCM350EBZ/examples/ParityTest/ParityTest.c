/*********************************************************************************

Copyright (c) 2013-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*!
 *****************************************************************************
 * @file:    ParityTest.c
 * @brief:   ParityTest Device Example for ADuCM350
 * @version: $Revision: 29403 $
 * @date:    $Date: 2015-01-08 10:34:56 -0500 (Thu, 08 Jan 2015) $
 *****************************************************************************/

/** \addtogroup Systick_Example SysTick Example
 *  Example code demonstrating use of the SysTick Interrupt.
 *  @{
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifndef _ILONG
#error Needs _ILONG
#endif

#include "wut.h"

#include "test_common.h"
#include <flash.h>

#include <ADuCM350_device.h>

#if defined ( __ICCARM__ )  // IAR compiler...
/* Apply ADI MISRA Suppressions */
#define ASSERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"
#endif

static char strbuf[256]; /* buffer for logging messages */


static void checkInitialSignature(ADI_FEE_DEV_HANDLE  hDevice, uint32_t baseAddr, uint32_t flashSize, ADI_FEE_SIGN_DIR direction)
{
    ADI_FEE_RESULT_TYPE feeResult;
    uint32_t startPage, endPage;
    uint32_t signature = 0u;
    uint32_t endAddr = baseAddr + flashSize - 4u;

    feeResult = adi_FEE_GetPageNumber(hDevice,
                            baseAddr,
                            &startPage);

    if (ADI_FEE_SUCCESS != feeResult)
    {
        sprintf(strbuf, "adi_FEE_GetPageNumber(addr = 0x%08x) failed with %d\n", baseAddr, feeResult);
        PERF(strbuf);
        FAIL("Failed in checkInitialSignature()");
    }

    feeResult = adi_FEE_GetPageNumber(hDevice,
                            endAddr,
                            &endPage);

    if (ADI_FEE_SUCCESS != feeResult)
    {
        sprintf(strbuf, "adi_FEE_GetPageNumber(addr = 0x%08x) failed with %d\n", endAddr, feeResult);
        PERF(strbuf);
        FAIL("Failed in checkInitialSignature()");
    }

    feeResult = adi_FEE_VerifySignature(hDevice,
                                        direction,
                                        startPage,
                                        endPage,
                                        &signature);

    if (ADI_FEE_SUCCESS != feeResult)
    {
        sprintf(strbuf, "adi_FEE_Sign(%ld, %ld) != ADI_FEE_SUCCESS\n", startPage, endPage);
        PERF(strbuf);
        FAIL("Failed in checkInitialSignature()");
    }

    PERF("Initial signature is correct");
}


static void testParityChecking(uint32_t flashBase, uint32_t flashStart, uint32_t nWords, uint32_t parityBase)
{
    uint32_t   offset = (flashStart & 0x1Fu);
    uint32_t   addr   = (flashStart >> 3u) | parityBase;
    uint32_t *pFlashWord = (uint32_t*)flashStart;
    uint32_t wordsProcessed = 0u;

    /* Now check that the parity information in flash is correct.
    */
    while (wordsProcessed < nWords)    /* For each word in specified region */
    {
        uint32_t nParityWord = 0xFFFFFFFFu;       /* aligned word for checking */

        /* Prepare the next aligned word to check */
        while((offset < 32u) && (wordsProcessed < nWords))
        {
            uint32_t dataWord = *pFlashWord++;
            uint32_t parityBit = 1u;

            /* For every set bit in dataWord, toggle the parity bit */
            while(0u != dataWord)
            {
                parityBit ^= (1u & dataWord); /* no-op if bottom bit is zero */
                dataWord >>= 1;               /* shift the next bit down */
            }

            /* Now use the parity bit to toggle the appropriate bit in the parity word.
             * The parity word starts out as all-1s (so that unchanged bits have no effect
             * when written) and gets toggled to zero if parityBit is 1. This is why
             * parityBit is initialised to 1, above. If there are an even number of set bits
             * dataWord then parityBit will still be 1 and will toggle the bit in nParityWord
             * to zero, i.e. the hardware expects *even* parity.
             */
            nParityWord ^= (parityBit << offset);
            wordsProcessed++;
            offset++;
        }

        offset = 0; /* start at the beginning of the next word */

        /* Verify that the calculated parity word matches what is in memory */
        if ((*(uint32_t*)addr) != nParityWord)
        {
            PERF("Parity word doesn't match contents of flash");
        	FAIL("Failed in testParityChecking()");
        }

        /* If the word has been successfully checked then increment the destination address */
        addr += 4;
    }

    PERF("Parity array is valid");
}


static void enableParityChecking(ADI_FEE_DEV_HANDLE  hDevice)
{
    /* Turn parity checking back on for the remainder of program execution */
    if (ADI_FEE_SUCCESS != adi_FEE_SetParityChecking(hDevice, ADI_FEE_PARITY_ENABLE_INTERRUPT))
    {
        PERF("adi_FEE_SetParityChecking(ON) failed\n");
        FAIL("Failed in enableParityChecking()");
    }

    PERF("Parity checking now enabled");
}


void HardFault_Handler(void)
{
    FAIL("Hard fault");
}

void BusFault_Handler(void)
{
    FAIL("Bus fault");
}

void Parity_Int_Handler(void)
{
	uint32_t errAddrLo = pADI_FEE0->FEEPARADRL;
	uint32_t errAddrHi = pADI_FEE0->FEEPARADRH;

	sprintf(strbuf, "FLASH parity error, address = 0x%04x%04x", errAddrHi, errAddrLo);
	PERF(strbuf);
	FAIL("Parity fault");
}

void testSignaturesAndParity(ADI_FEE_DEV_ID_TYPE const devID, uint32_t baseAddr, uint32_t flashSize, uint32_t nWords, uint32_t parityBase)
{
    ADI_FEE_DEV_HANDLE  hDevice;

    /* Open the flash device */
    if (ADI_FEE_SUCCESS != adi_FEE_Init(devID, true, &hDevice))
    {
      FAIL("adi_FEE_Init() failed\n");
    }

    /* Check the forward signature */
    checkInitialSignature(hDevice, baseAddr, flashSize, ADI_FEE_SIGN_FORWARD);
    /* Check the reverse signature */
    checkInitialSignature(hDevice, baseAddr, flashSize, ADI_FEE_SIGN_REVERSE);
    /* Verify the parity array (optional) */
    testParityChecking(baseAddr, baseAddr, nWords, parityBase);
    /* Enable parity checking */
    enableParityChecking(hDevice);

    /* Close the flash device */
    if (ADI_FEE_SUCCESS != adi_FEE_UnInit(hDevice))
    {
        FAIL("adi_FEE_UnInit() failed\n");
    }
}

/**
 * @brief  Function 'main' for flash signature/parity test program.
 *
 * @return int (Zero for success, non-zero for failure).
 *
 *
 *
 *
 */
int main()
{
    /* Clock initialization */
    SystemInit();

    // Change HCLK clock divider to 1 for a 16MHz clock
    if (ADI_SYS_SUCCESS != SetSystemClockDivider(ADI_SYS_CLOCK_CORE, 1))
        return 1;    /* Without the UART we have no way of reporting failure */

    // Change PCLK clock divider to 1 for a 16MHz clock
    if (ADI_SYS_SUCCESS != SetSystemClockDivider (ADI_SYS_CLOCK_UART, 1))
        return 1;    /* Without the UART we have no way of reporting failure */

    /* test system initialization */
    test_Init();

    /* NVIC initialization */
    NVIC_SetPriorityGrouping(12);

    PERF("Testing flash controller 0");
    testSignaturesAndParity(ADI_FEE_DEVID_0, 0x00000000u, 0x00040000, 0xF800u, 0x0003E000);

    PERF("Testing flash controller 1");
    testSignaturesAndParity(ADI_FEE_DEVID_1, 0x00040000u, 0x00020000, 0x7C00u, 0x0005F000);

    /* If we got here then nothing has failed */
    PASS();

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
