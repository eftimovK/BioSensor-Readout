/*********************************************************************************

Copyright(c) 2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary and confidential.  By using this software you agree
to the terms of the associated Analog Devices License Agreement.
*********************************************************************************/

/*
    Project Name:    Power_On_Self_Test

    Hardware:        ADcCM350 EZ-Board

    Description:     This file tests the SPI-Flash on the EZ-Board.

    This file performs the Serial Flash portion of the ADcCM350 Evaluation Board
    Power-On Self Test (POST).  It tests basic read/write access of the external
    SPI-Flash device (Microchip 2-MBit SST25LF020A) on the Audio Extender Board (revA)
    (U3) in single-bit serial mode to verify connectivity.

    The test employs the SPI Device Driver for talking to the SPI-Flash device over
    the SPIH device instance.

    This is not an exhaustive SPI-Flash device memory test or SPI device driver test;
    it is only meant to use the SPI device driver to communicate with and verify correct
    connectivity between the ADcCM350 processor on the EZ-Board to the SPI-Flach device.
*/

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rules_all:"Suppress all rules for other header files")
#endif

/* standard includes */
#include <stdio.h>
#include <adi_types.h>
#include <stdlib.h>

/* driver includes */
#include <adi_int.h>
#include <spi.h>

/* test interface include */
#include "test_common.h"

/* test loop mode for robustness testing (undefine for normal, one-time executiuon) */
//#define LOOP_MODE

/* return codes */
#define FAILURE true
#define SUCCESS false

#ifdef _MISRA_RULES
#pragma diag(push)
#pragma diag(suppress:misra_rule_17_4:"Array indexing shall be the only allowed form of pointer arithmetic.")
#endif /* _MISRA_RULES */

/* SPI device Info */
#define SPI_DEVICE_NUM          ADI_SPI_DEVID_H
#define SPI_CS_PORT             ADI_GPIO_PORT_0
#define SPI_CS_PIN              ADI_GPIO_PIN_15
#define SPI_CLOCK               7999999

/* SPI-Flash chip info (Microchip SST25LF020A) */
#define MANUF_ID                0xbfu
#define DEVICE_ID               0x43u

#define BIT_SIZE                0x200000u     /* 2-MBits */
#define BYTE_SIZE               (BIT_SIZE>>3) /* 1/4-MBytes */
#define SECTOR_SIZE             0x1000u       /* 4-KByte/sector */
#define NUM_SECTORS             (BYTE_SIZE/SECTOR_SIZE)

/* test address (last sector base address of the 2-MBIT flash address range) */
#define TEST_ADDRESS            ((NUM_SECTORS-1)*SECTOR_SIZE)

/* device status bits */
#define MAX_TIMEOUT             0x0fffu   /* ~20x wait-on-busy limit */
#define SR1_BUSY_BIT            (1<<0)    /* bit0 is device busy bit */
#define SR1_WEL_BIT             (1<<1)    /* bit1 is write enable bit */
#define SR1_BP0_BIT             (1<<2)    /* bit2 is block protect bit0 */
#define SR1_BP1_BIT             (1<<3)    /* bit3 is block protect bit1 */

/* Select SPI Flash Commands (from Microchip SST25LF020A, 2-MBit serial flash device data sheet) */
#define CMD_SR1_WRITE           0x01u
#define CMD_SINGLE_MODE_WRITE   0x02u
#define CMD_SINGLE_MODE_READ    0x03u
#define CMD_WRITE_DISABLE       0x04u
#define CMD_SR1_READ            0x05u
#define CMD_WRITE_ENABLE        0x06u
#define CMD_SECTOR_ERASE        0x20u
#define CMD_SR1_WRITE_ENABLE    0x50u
#define CMD_CHIP_ERASE          0x60u
#define CMD_ID_READ             0x90u
#define CMD_AUTO_WRITE          0xafu

/* Buffer sizes */
#define PROLOGUE_SIZE           8u
#define TRANSFER_SIZE           1024u
#define CHIPID_SIZE             2u

/* global data buffers (force 32-bit alignment) */
/* GNU syntax is: __attribute__((aligned(32))) ... */
/* IAR syntax is: #pragma data_alignment=4 ... */
#pragma data_alignment=4
uint8_t gPrologueBuffer[PROLOGUE_SIZE];

#pragma data_alignment=4
uint8_t gTransmitBuffer[TRANSFER_SIZE];

#pragma data_alignment=4
uint8_t gReceiveBuffer[TRANSFER_SIZE];

#pragma data_alignment=4
uint8_t gGarbageBuffer[TRANSFER_SIZE];

/* error messages */
char gPrintString[128];


/* prototypes */

/* pinmux API */
extern int32_t adi_initpinmux(void);

/* test entry point */
int SpiFlashTest (void);

/* main test interface */
bool RunTest                    (ADI_SPI_DEV_HANDLE hSpi,
                                bool (*pTestFunction)(ADI_SPI_DEV_HANDLE, uint32_t, uint32_t),
                                uint32_t Addr,
                                uint32_t ByteCount);

/* flash I/O tests (write-then-read) */
bool TestSingleModeIO           (ADI_SPI_DEV_HANDLE hSpi, uint32_t Addr, uint32_t ByteCount);

/* flash write-only utilities (dual-mode write does not exist) */
bool SingleModeWrite            (ADI_SPI_DEV_HANDLE hSpi, uint32_t Address, uint32_t ByteCount);

/* flash read-only utilities */
bool SingleModeRead             (ADI_SPI_DEV_HANDLE hSpi, uint32_t Address, uint32_t ByteCount);

/* data functions */
void InitializeData             (uint32_t ByteCount);
bool ValidateData               (uint32_t ByteCount);

/* misc functions */
bool ConfigureSPI               (ADI_SPI_DEV_HANDLE hSpi);
bool FlashBusyWait              (ADI_SPI_DEV_HANDLE hSpi);
bool FlashErase                 (ADI_SPI_DEV_HANDLE hSpi);
bool FlashReadID                (ADI_SPI_DEV_HANDLE hSpi);
bool FlashWriteSR1              (ADI_SPI_DEV_HANDLE hSpi, uint8_t bits);
bool FlashTestSR1               (ADI_SPI_DEV_HANDLE hSpi, uint8_t bit, bool* pIsSet);
bool FlashWriteEnable           (ADI_SPI_DEV_HANDLE hSpi, bool bEnable);
bool FlashEraseSector           (ADI_SPI_DEV_HANDLE hSpi, uint32_t Address);


/* entry point for flash test */
int main (void)
{
    /* Clock initialization */
    SystemInit();

	/* jack up the core and peripheral clocks to 16MHz so we can test the SST25LF020A
       as fast as possible, i.e., less than 1/2 core clock or 8MHz; not hitting the
       20MHz max read clock spec... but as fast as the 350 can push it */

    /* Set PCLK frequency to 16MHz for better resolution */
    /* Need to set HCLK frequency to 16MHz first */
    if (SetSystemClockDivider(ADI_SYS_CLOCK_CORE, 1))
		FAIL();
    if (SetSystemClockDivider(ADI_SYS_CLOCK_PCLK, 1))
		FAIL();

    /* test system initialization */
    test_Init();

    /* initialize static pinmuxing */
    adi_initpinmux();

    /* install and enable DMA error interrupt handeling with OSAL */
    ADI_INSTALL_HANDLER(DMA_ERR_IRQn, DMA_Err_Int_Handler);
    ADI_ENABLE_INT(DMA_ERR_IRQn);

    /* run the test */
    SpiFlashTest();
}


/* entry point for flash test */
int SpiFlashTest (void)
{
#ifdef LOOP_MODE
    uint32_t testCount = 1;
#endif
    ADI_SPI_DEV_HANDLE hSpi;
    bool bIsSet;


    /* test "loop" for easy breakout */
    while (1) {

        PERF( "SPI Flash Test" );

        /* open SPI */
        if (adi_SPI_MasterInit(SPI_DEVICE_NUM, &hSpi)) {
            FAIL("adi_SPI_MasterInit() failure...");
        }

        /* configure SPI */
        if (ConfigureSPI(hSpi)) {
            FAIL("ConfigureSPI failure...");
        }

        /* verify expected manuf and chip IDs */
        PERF("Checking flash ID...");
        if (FlashReadID(hSpi)) {
            PERF("FlashReadID() failure!");
            break;
        }
        PERF("Flash ID check passed.");

        /* disable block write protection bits */
        PERF("Disabeling block write protection bits...");
        if (FlashWriteSR1(hSpi, 0)) {
            PERF("FlashWriteSR1() failure!");
            break;
        }
        /* verify block write protection bits are clear */
        PERF("Verifying block write protection bits...");
        if (FlashTestSR1(hSpi, SR1_BP0_BIT, &bIsSet)) {
            PERF("FlashTestSR1() failure!");
            break;
        }
        if (bIsSet) {
            PERF("Block Protection Bit0 not clear!");
            break;
		}
        if (FlashTestSR1(hSpi, SR1_BP1_BIT, &bIsSet)) {
            PERF("FlashTestSR1() failure!");
            break;
        }
        if (bIsSet) {
            PERF("Block Protection Bit1 not clear!");
            break;
		}
        PERF("Block write protection bits disabled & verified...");

        /* erase the entire flash */
        PERF("Bulk erasing flash...");
        if (FlashErase(hSpi)) {
            PERF("FlashErase() failure!");
            break;
        }
        PERF("Bulk erase passed.");

        /* Single mode tests */
        PERF("Testing single-bit access...");
        if (RunTest(hSpi, &TestSingleModeIO, TEST_ADDRESS, TRANSFER_SIZE)) {
            PERF("TestSingleModeIO() failure!");
            break;
        }
        PERF("Single-bit mode testing passed.");

        /* close SPI */
        if (adi_SPI_UnInit(hSpi)) {
            PERF("adi_spi_Close() failure!");
            break;
        }

#ifdef LOOP_MODE
        /* log test iteration and result */
        sprintf(gPrintString, "SPI Flash Test Success, iteration: %d\n", testCount++);
        PERF(gPrintString);
#else
        /* success */
        PASS();
#endif
    } /* end while */

    /* failure */
    FAIL("\nTEST_SPI_FLASH failed.\n");

    /* make compiler happy */
    return 0;
}


/* tests the device via a function pointer to a test case function and its associated test parameters */
bool RunTest(ADI_SPI_DEV_HANDLE hSpi,
             bool (*pTestFunction)(ADI_SPI_DEV_HANDLE, uint32_t, uint32_t),
             uint32_t Addr,
             uint32_t ByteCount)
{
    /* initialize transmit and receive data to a known state */
    InitializeData(ByteCount);

    /* dispatch the test and pass params through the function pointer */
    if (pTestFunction(hSpi, Addr, ByteCount))
        return FAILURE;

    /* check/return data validation result */
    PERF("Validating readback...");
    return (ValidateData(ByteCount));
}


/* write to a page then read it back test (all in single-bit mode) */
bool TestSingleModeIO(ADI_SPI_DEV_HANDLE hSpi, uint32_t Addr, uint32_t ByteCount)
{
    while (1) {

        PERF("Erasing test sector...");
        if (FlashEraseSector(hSpi, Addr))
            break;

        PERF("Writing test sector...");
        if (SingleModeWrite(hSpi, Addr, ByteCount))
            break;

        PERF("Reading test sector...");
        if (SingleModeRead(hSpi, Addr, ByteCount))
            break;

        /* success */
        return SUCCESS;
    }

    /* failure */
    PERF("TestSingleModeIO() failure!");
    return FAILURE;
}


bool SingleModeWrite(ADI_SPI_DEV_HANDLE hSpi, uint32_t Address, uint32_t ByteCount)
{
    ADI_SPI_TRANSCEIVE_TYPE xfr;
    bool bIsSet;
    bool firstIteration = true;
    uint8_t *pTx = gTransmitBuffer;

    while (1) {

        /* assert flash write enable state */
        if (FlashWriteEnable(hSpi, true))
            break;

        /* use auto-address-increment instruction for multi-byte writes */
        gPrologueBuffer[0]       = CMD_AUTO_WRITE;
        gPrologueBuffer[1]       = (uint8_t)(Address >> 16);
        gPrologueBuffer[2]       = (uint8_t)(Address >> 8);
        gPrologueBuffer[3]       = (uint8_t)Address;
        xfr.pRxData              = gGarbageBuffer;
        xfr.DataSize             = 1;
        xfr.bTxIncrement         = true;
        xfr.bRxIncrement         = false;

        /* Auto-Address Instruction (AAI) requires CS to be cycled between data byte, hence the loop... */
        /* this precludes a block-mode DMA transfer, so stay in interrupt-driven mode */
        while (ByteCount--) {

            /* reset the prologue pointer and size each iteration */
            xfr.pPrologue = gPrologueBuffer;
            xfr.PrologueSize = firstIteration ? 4 : 1;
            firstIteration = false;

			/* update transmit data */
            xfr.pTxData          = pTx++;

            /* submit transaction */
            if (adi_SPI_MasterTransfer(hSpi, &xfr))
                break;
        }

        /* disable flash write state */
        if (FlashWriteEnable(hSpi, false))
            break;

        /* verify flash is write-disabled */
        if (FlashTestSR1(hSpi, SR1_WEL_BIT, &bIsSet))
            break;

        if (bIsSet) {
            PERF("Write Enable bit not clear!");
            break;
		}

        /* success */
        return SUCCESS;

    } /* end while */

    /* failure */
    PERF("SingleModeWrite() failure!");
    return FAILURE;
}


bool SingleModeRead(ADI_SPI_DEV_HANDLE hSpi, uint32_t Address, uint32_t ByteCount)
{
    ADI_SPI_TRANSCEIVE_TYPE xfr;

    while (1) {

        /* verify not busy */
        if (FlashBusyWait(hSpi))
            break;

        /* single-mode read sequence */
        gPrologueBuffer[0]       = CMD_SINGLE_MODE_READ;
        gPrologueBuffer[1]       = (uint8_t)(Address >> 16);
        gPrologueBuffer[2]       = (uint8_t)(Address >> 8);
        gPrologueBuffer[3]       = (uint8_t)Address;
        xfr.pPrologue            = gPrologueBuffer;
        xfr.PrologueSize         = 4;
        xfr.pTxData              = gGarbageBuffer;
        xfr.pRxData              = gReceiveBuffer;
        xfr.DataSize             = ByteCount;
        xfr.bTxIncrement         = false;
        xfr.bRxIncrement         = true;

        /* switch to DMA mode for more efficient block read */
        /* depends on blocking mode having already been asserted. */
        if (adi_SPI_SetDmaMode(hSpi, true))
            break;

        /* submit transaction */
        if (adi_SPI_MasterTransfer(hSpi, &xfr))
            break;

        /* switch back to non-DMA mode */
        if (adi_SPI_SetDmaMode(hSpi, false))
            break;

        /* success */
        return SUCCESS;
    }

    /* failure */
    PERF("SingleModeRead() failure!");
    return FAILURE;
}


/* erase the flash */
bool FlashErase(ADI_SPI_DEV_HANDLE hSpi)
{
    ADI_SPI_TRANSCEIVE_TYPE xfr;

    while (1) {

        /* verify not busy */
        if (FlashBusyWait(hSpi))
            break;

        /* reset the flash */
        gPrologueBuffer[0]       = CMD_CHIP_ERASE;
        xfr.pPrologue            = gPrologueBuffer;
        xfr.PrologueSize         = 1;
        xfr.pTxData              = gGarbageBuffer;
        xfr.pRxData              = gGarbageBuffer;
        xfr.DataSize             = 0;
        xfr.bTxIncrement         = false;
        xfr.bRxIncrement         = false;

        /* submit transaction */
        if (adi_SPI_MasterTransfer(hSpi, &xfr))
            break;

        /* wait with timeout */
        if (FlashBusyWait(hSpi))
            break;

        /* success */
        return SUCCESS;
    }

    /* failure */
    PERF("FlashErase() failure!");
    return FAILURE;
}


/* verify manuf and chip IDs */
bool FlashReadID(ADI_SPI_DEV_HANDLE hSpi)
{
    ADI_SPI_TRANSCEIVE_TYPE xfr;
    uint8_t chipID[CHIPID_SIZE];
    uint8_t mid = 0xff;
    uint16_t did = 0xffff;

    while (1) {

        /* verify not busy */
        if (FlashBusyWait(hSpi))
            break;

        /* single-mode read sequence */
        gPrologueBuffer[0]       = CMD_ID_READ;
        gPrologueBuffer[1]       = 0;
        gPrologueBuffer[2]       = 0;
        gPrologueBuffer[3]       = 0;
        xfr.pPrologue            = gPrologueBuffer;
        xfr.PrologueSize         = 4;
        xfr.pTxData              = gGarbageBuffer;
        xfr.pRxData              = chipID;
        xfr.DataSize             = CHIPID_SIZE;
        xfr.bTxIncrement         = false;
        xfr.bRxIncrement         = true;

        /* submit transaction */
        if (adi_SPI_MasterTransfer(hSpi, &xfr))
            break;

        /* extract IDs */
        mid = chipID[0];
        did = chipID[1];

        /* verify */
        if (MANUF_ID != mid)
            break;
        if (DEVICE_ID != did)
            break;

        /* success */
        sprintf(gPrintString, "SPI Flash ID check passed: manufacturer, device ID: 0x%02x,  0x%02x", mid, did);
        PERF(gPrintString);
        return SUCCESS;
    }

    /* failure */
    PERF("FlashReadID() failure!");
    sprintf(gPrintString, "...unexpected SPI Flash manufacturer ID: 0x%02x and/or device ID: 0x%02x", mid, did);
    PERF(gPrintString);
    return FAILURE;
}


/* write the flash SR1 register */
bool FlashWriteSR1 (ADI_SPI_DEV_HANDLE hSpi, uint8_t bits)
{
    ADI_SPI_TRANSCEIVE_TYPE xfr;
    bool bIsSet;

    while (1) {

        /* issue SR1 write enable */
        gPrologueBuffer[0]       = CMD_SR1_WRITE_ENABLE;
        gPrologueBuffer[1]       = bits;
        xfr.pPrologue            = gPrologueBuffer;
        xfr.PrologueSize         = 2;
        xfr.pTxData              = gGarbageBuffer;
        xfr.pRxData              = gGarbageBuffer;
        xfr.DataSize             = 0;
        xfr.bTxIncrement         = false;
        xfr.bRxIncrement         = false;

        /* submit transaction */
        if (adi_SPI_MasterTransfer(hSpi, &xfr))
            break;

        /* write SR1 */
        gPrologueBuffer[0]       = CMD_SR1_WRITE;
        gPrologueBuffer[1]       = bits;
        xfr.pPrologue            = gPrologueBuffer;
        xfr.PrologueSize         = 2;
        xfr.pTxData              = gGarbageBuffer;
        xfr.pRxData              = gGarbageBuffer;
        xfr.DataSize             = 0;
        xfr.bTxIncrement         = false;
        xfr.bRxIncrement         = false;

        /* submit transaction */
        if (adi_SPI_MasterTransfer(hSpi, &xfr))
            break;

        /* verify flash is write-disabled */
        if (FlashTestSR1(hSpi, SR1_WEL_BIT, &bIsSet))
            break;

        if (bIsSet) {
            PERF("Write Enable bit not clear!");
            break;
		}

        /* success */
        return SUCCESS;
    }

    /* failure */
    PERF("FlashWriteSR1() failure!");
    return FAILURE;

}


/* return flash SR1 bit status as a boolean */
bool FlashTestSR1 (ADI_SPI_DEV_HANDLE hSpi, uint8_t bit, bool *pIsSet)
{
    ADI_SPI_TRANSCEIVE_TYPE xfr;

    while (1) {

        /* read SR1 */
        gPrologueBuffer[0]       = CMD_SR1_READ;
        xfr.pPrologue            = gPrologueBuffer;
        xfr.PrologueSize         = 1;
        xfr.pTxData              = gGarbageBuffer;
        xfr.pRxData              = gReceiveBuffer;
        xfr.DataSize             = 1;  /* one byte for SR1 readback */
        xfr.bTxIncrement         = false;
        xfr.bRxIncrement         = true;

        /* submit transaction */
        if (adi_SPI_MasterTransfer(hSpi, &xfr))
            break;

        /* success, return bit set state */
        *pIsSet = (gReceiveBuffer[0] & bit);
        return SUCCESS;
    }

    /* failure */
    PERF("FlashTestSR1() failure!");
    return FAILURE;
}


/* prepare the flash for writing */
bool FlashWriteEnable(ADI_SPI_DEV_HANDLE hSpi, bool bEnable)
{
    ADI_SPI_TRANSCEIVE_TYPE xfr;
    bool bIsSet;

    while (1) {

        /* verify not busy */
        if (FlashBusyWait(hSpi))
            break;

        gPrologueBuffer[0]       = bEnable ? CMD_WRITE_ENABLE : CMD_WRITE_DISABLE;
        xfr.pPrologue            = gPrologueBuffer;
        xfr.PrologueSize         = 1;
        xfr.pTxData              = gGarbageBuffer;
        xfr.pRxData              = gGarbageBuffer;
        xfr.DataSize             = 0;
        xfr.bTxIncrement         = false;
        xfr.bRxIncrement         = false;

        /* submit transaction */
        if (adi_SPI_MasterTransfer(hSpi, &xfr))
            break;

        /* verify flash write enablement */
        if (FlashTestSR1(hSpi, SR1_WEL_BIT, &bIsSet))
            break;

        if (bEnable != bIsSet) {
            PERF("Incorrect Write Enable bit!");
            break;
		}

        /* success */
        return SUCCESS;
    }

    /* failure */
    PERF("FlashWriteEnable() failure!");
    return FAILURE;
}


/* erase a 4k-byte sector on the flash, assumes address is aligned to sector start boundary */
bool FlashEraseSector(ADI_SPI_DEV_HANDLE hSpi, uint32_t Address)
{
    ADI_SPI_TRANSCEIVE_TYPE xfr;
    bool bIsSet;

    while (1) {

        /* assert flash write enable state */
        if (FlashWriteEnable(hSpi, true))
            break;

        /* sector erase sequence */
        gPrologueBuffer[0]       = CMD_SECTOR_ERASE;
        gPrologueBuffer[1]       = (uint8_t)(Address >> 16);
        gPrologueBuffer[2]       = (uint8_t)(Address >> 8);
        gPrologueBuffer[3]       = (uint8_t)Address;
        xfr.pPrologue            = gPrologueBuffer;
        xfr.PrologueSize         = 4;
        xfr.pTxData              = gGarbageBuffer;
        xfr.pRxData              = gGarbageBuffer;
        xfr.DataSize             = 0;
        xfr.bTxIncrement         = false;
        xfr.bRxIncrement         = false;

        /* submit transaction */
        if (adi_SPI_MasterTransfer(hSpi, &xfr))
            break;

        /* wait with timeout */
        if (FlashBusyWait(hSpi))
            break;

        /* verify flash is auto-write-disabled */
        if (FlashTestSR1(hSpi, SR1_WEL_BIT, &bIsSet))
            break;

        if (bIsSet) {
            PERF("Write Enable bit not clear!");
            break;
		}

        /* success */
        return SUCCESS;
    }

    /* failure */
    PERF("FlashEraseSector() failure!");
    return FAILURE;
}


/* busy wait with timeout */
bool FlashBusyWait(ADI_SPI_DEV_HANDLE hSpi)
{
    uint16_t timeout;
    volatile bool bIsSet;

    /* poll on the busy bit until it clears or we time out */
    for (timeout = 0; timeout < MAX_TIMEOUT; timeout++) {

		/* read busy bit */
        if (FlashTestSR1(hSpi, SR1_BUSY_BIT, (bool *)&bIsSet)) {
			PERF("FlashTestSR1() failure during FlashBusyWait()!");
            break;
		}

		/* break out when busy bit clears */
        if (!bIsSet)
        	break;
    }

    /* return timeout status */
    return (MAX_TIMEOUT == timeout);
}


/* initialize data */
void InitializeData(uint32_t ByteCount)
{
    int i;
    uint8_t *pTx = gTransmitBuffer;
    uint8_t *pRx = gReceiveBuffer;

       for (i = 0; i < ByteCount; i++) {
        *pTx++ = rand();
        *pRx++ = 0x77;  // more identifiable than just zero...
       }
}


/* verify results */
bool ValidateData(uint32_t ByteCount)
{
    int i;
    uint8_t *pTx = gTransmitBuffer;
    uint8_t *pRx = gReceiveBuffer;

    for(i = 0; i < ByteCount; i++) {
        if (*pTx++ != *pRx++) {
            /* falure */
            sprintf(gPrintString, "SPI Flash data validation failed at index: %d", i);
            PERF(gPrintString);
            return FAILURE;
        }
    }
    return SUCCESS;
}


bool ConfigureSPI(ADI_SPI_DEV_HANDLE hSpi)
{

    while (1) {

        /* disable DMA (initially) */
        if (adi_SPI_SetDmaMode(hSpi, false))                         break;

        /* set blocking mode so DMA transfers complete without need to poll */
        if (adi_SPI_SetBlockingMode(hSpi, true))                     break;

        /* send zeros if tx SPI underflows*/
        if (adi_SPI_SetTransmitUnderflow(hSpi, true))                break;

        /* data transitions on falling edge of clock */
        if (adi_SPI_SetClockPhase(hSpi, false))                      break;

        /* run the SST25LF020A up to 1/2 core clock */
        if (adi_SPI_SetBitrate(hSpi, SPI_CLOCK))                     break;

        /* success */
        return SUCCESS;

    } /* end while */

    /* failure */
    PERF("ConfigureSPI() failure!");
    return FAILURE;
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

    /* get the descriptor error channel bits from the DMA controller */
    uint32_t invalidDescriptorBits  = pADI_DMA->DMAINVALIDDESCCLR;

    /* if an expected SPI channel has a descriptor error, clear it and proceed */
    if (SPI_DMA_CHANNELS & invalidDescriptorBits) {
        pADI_DMA->DMAINVALIDDESCCLR = invalidDescriptorBits;
        return;
    }

    /* otherwise, it is an unexpected dma error, a bus fault, some other
       more generic DMA error, etc., all of which we want to trap on.
    */
    while(1)
        ;
}
