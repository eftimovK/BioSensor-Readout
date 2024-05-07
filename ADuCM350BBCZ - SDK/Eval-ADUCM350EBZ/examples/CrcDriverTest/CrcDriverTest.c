/*********************************************************************************

Copyright (c) 2012-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*****************************************************************************
 * @file:    CRCTest.c
 * @brief:   CRC Test for ADuCM350
 * @version: $Revision: 28525 $
 * @date:    $Date: 2014-11-12 14:51:26 -0500 (Wed, 12 Nov 2014) $
 *****************************************************************************/

/*! \addtogroup CRC_Test CRC Test
 *  .
 *  @{
 */

/* Apply ADI MISRA Suppressions */
#define ASSERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"
#include <stddef.h>		/* for 'NULL' */
#include <stdio.h>
#include <string.h>		/* for strlen */
#include <stdlib.h>
#include "dma.h"
#include "crc.h"
#include "test_common.h"
#include "config/adi_crc_config.h"

#define TEST_BUFFER_SIZE  1024*2

static void                     crc32               (uint32_t *crc, uint32_t *data);
static uint32_t                 crcBlock            (uint32_t *startAddr, uint32_t endAddr);
unsigned char test[TEST_BUFFER_SIZE* 4];
uint32_t softwareCRC, hardwareCRC;

#define         POLYNOMIAL      0x04C11DB7
#if (ADI_CFG_ENABLE_CALLBACK_SUPPORT == 1)
volatile bool_t bResultFlag;

/* CRC callback */
void  CRCCallback(void *pCBParam, uint32_t Event, void *pArg)
{

  switch((ADI_CRC_EVENT_TYPE) Event)
  {
    case ADI_CRC_EVENT_RESULT_READY:
      bResultFlag = true;
      result = (uint32_t)(pArg);
      break;

    default:
      FAIL("Unexpected event");
      break;
  }

  return;
}
#endif

/*!
 * @brief       Function 'main' for CRC example program
 *
 * @param       none
 * @return      int (Zero for success, non-zero for failure).
 *
 * @details
 *
 */

int main(void)
{
	char errString[255];
    uint32_t *startAddr;
    uint32_t i;
    bool_t bDone=false;
    ADI_CRC_RESULT_TYPE errCode;
    ADI_CRC_DEV_HANDLE hDevice;

    /* Initialize system */
    SystemInit();

    /* test system initialization */
    test_Init();

    memset(test,0,sizeof(test));
    for(i=0;i<( TEST_BUFFER_SIZE *4 );i++) {
      test[i]=(rand())&0XFF;
    }

    startAddr =(uint32_t *) &test[0];
	if ((uint32_t)startAddr & 0x3) {
		FAIL("This test needs word-aligned addresses.");
	}

	if (ADI_CRC_SUCCESS != (errCode = adi_CRC_Init(ADI_CRC_DEVID_0, &hDevice))) {
		sprintf(errString, " Init Failed \t%08X", errCode);
		PERF(errString);
		FAIL();
	}

	if (ADI_CRC_SUCCESS != (errCode = adi_CRC_SetAutoReset(hDevice,ADI_CRC_CKSUM_FFFFFFFF))) {
		sprintf(errString, " Auto Reset  Failed \t%08X", errCode);
		PERF(errString);
		FAIL();
	}

	if (ADI_CRC_SUCCESS != (errCode = adi_CRC_SetBitMirroring(hDevice, false))) {
		sprintf(errString, "  Set Bit mirroring  Failed \t%08X", errCode);
		PERF(errString);
		FAIL();
	}

	if (ADI_CRC_SUCCESS != (errCode = adi_CRC_SetByteMirroring(hDevice, false))) {
		sprintf(errString, "  Set Byte mirroring  Failed \t%08X", errCode);
		PERF(errString);
		FAIL();
	}

	if (ADI_CRC_SUCCESS != (errCode = adi_CRC_SetWordSwap(hDevice, false))) {
		sprintf(errString, "  Set Word swap Failed \t%08X", errCode);
		PERF(errString);
		FAIL();
	}

#if (ADI_CRC_CFG_ENABLE_DMA_SUPPORT == 1)

	if (ADI_CRC_SUCCESS != (errCode = adi_CRC_SetDmaMode(hDevice, true))) {
		sprintf(errString, "  Set DMA Failed \t%08X", errCode);
		PERF(errString);
		FAIL();
	}

#if (ADI_CFG_ENABLE_CALLBACK_SUPPORT == 1)

	bResultFlag = false;
	if (ADI_CRC_SUCCESS != (errCode = adi_CRC_RegisterCallback(hDevice, CRCCallback, hDevice))) {
		sprintf(errString, "  Call back Registration  Failed \t%08X", errCode);
		PERF(errString);
		FAIL();
	}

#endif /* ADI_CFG_ENABLE_CALLBACK_SUPPORT */

	if (ADI_CRC_SUCCESS != (errCode = adi_CRC_BufferSubmit(hDevice,(uint32_t*)startAddr, TEST_BUFFER_SIZE))) {
		sprintf(errString, "  Buffer submission failed \t%08X", errCode);
		PERF(errString);
		FAIL();
	}

	if (ADI_CRC_SUCCESS != (errCode = adi_CRC_Enable(hDevice, true))) {
		sprintf(errString, "  Failed to enable the CRC engine \t%08X", errCode);
		PERF(errString);
		FAIL();
	}

#if (ADI_CFG_ENABLE_CALLBACK_SUPPORT == 1)

	/* Wait for the call back */
	while (bResultFlag == false)
		;

#else

	while (bDone == false) {
		errCode =  adi_CRC_IsResultAvailable(hDevice, &bDone);
	}

	/* Wait for the CRC result  */
	errCode = adi_CRC_GetResult(hDevice, &hardwareCRC);

#endif /* ADI_CFG_ENABLE_CALLBACK_SUPPORT */

	if (ADI_CRC_SUCCESS != (errCode = adi_CRC_Enable(hDevice, false))) {
		sprintf(errString, "  Failed to disable  the CRC engine \t%08X", errCode);
		PERF(errString);
		FAIL();
	}

	if (ADI_CRC_SUCCESS != (errCode = adi_CRC_UnInit(hDevice))) {
		sprintf(errString, "  Failed to uninit device \t%08X", errCode);
		PERF(errString);
		FAIL();
	}

#else /* ADI_CRC_CFG_ENABLE_DMA_SUPPORT */

	if (ADI_CRC_SUCCESS != (errCode = adi_CRC_BufferSubmit(hDevice,(uint32_t*)startAddr, TEST_BUFFER_SIZE))) {
		sprintf(errString, "  Buffer submission failed \t%08X", errCode);
		PERF(errString);
		FAIL();
	}

	if (ADI_CRC_SUCCESS != (errCode = adi_CRC_Enable(hDevice, true))) {
		sprintf(errString, "  Failed to enable the CRC engine \t%08X", errCode);
		PERF(errString);
		FAIL();
	}

	/* Get CRC result. In the core mode, Enable will compute the CRC and core will   */
	errCode = adi_CRC_GetResult(hDevice, &hardwareCRC);

#endif /* ADI_CRC_CFG_ENABLE_DMA_SUPPORT */

	/* compute the CRC using mathematical formula */
	softwareCRC = crcBlock(startAddr, TEST_BUFFER_SIZE);

	/* compare the result */
	if (softwareCRC != hardwareCRC) {
		FAIL("CRC results mismatch.");
	}

	/* print the result */
	PASS();
}
/* Software implementation of the hardware CRC computation */
static void crc32(uint32_t *crc, uint32_t *data) {
    uint8_t i;

    for (i = 0; i < 32; i++) {

        if ((*data & 0x80000000) ^ (*crc & 0x80000000)) {
            *crc = *crc << 1;
            *crc = *crc ^ POLYNOMIAL;
        } else {
            *crc = *crc << 1;
        }

        *data = *data << 1;
    }

}
/* Assumes start and end addresses are word-aligned */
static uint32_t crcBlock(uint32_t *startAddr, uint32_t nSize) {
    uint32_t crc;
    uint32_t data;
    uint32_t i;
    uint32_t *addr = startAddr;
    /* Initialize CRC with 0xFFFFFFFF: there is scope to parametrize that */
    crc = 0xFFFFFFFF;

    /* FIXME: check if it's inside or outside Flash */
    for (i=0; i < nSize; i ++) {
        data = *addr++;
        crc32(&crc, &data);
    }

    return crc;
}

/* Revert ADI MISRA Suppressions */
#define REVERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"

/*
** EOF
*/

/*@}*/
