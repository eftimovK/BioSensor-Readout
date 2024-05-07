/*********************************************************************************

Copyright (c) 2011-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using
this software you agree to the terms of the associated Analog Devices Software
License Agreement.

*********************************************************************************/

#include "misra.h"

#include <stddef.h>		/* for 'NULL' */
#include <string.h>		/* for strlen */
#include <stdio.h>


#include "test_common.h"
#include "spi.h"

//FTDI 4222 protocol related defines
#define FTDI_ACK_SIZE                    7 
#define FTDI_SYNC_BYTE                0x5A
#define FTDI_HEADER_SIZE              0x05
#define FTDI_MASTER_TRANSFER          0x80
#define FTDI_SLAVE_TRANSFER           0x81
#define FTDI_SHORT_MASTER_NO_CHKSUM   0x82
#define FTDI_SHORT_SLAVE_WR_NO_CHKSUM 0x83
#define FTDI_ACK_COMMAND              0x84

//FTDI Windows Application realted defines
#define HOST_CMD_USER_WRITE_REQ      0x4a
#define HOST_CMD_USER_READ_REQ       0x4b

#define FTDI_SN 0x60
#define ACK_SN  0x80

/*
 * The SPI APIs require the use of a ADI_SPI_TRANSCEIVE_TYPE data structure */
static ADI_SPI_TRANSCEIVE_TYPE     xfr;

/*
 * When transmitting data over the SPI interface received data will also need 
 * to be collected, even if it is not used. Declare a "garabage" buffer for this
 * purpose
 */
#pragma data_alignment=8
static uint8_t gRecieveBuffer[1024u];


/*
 * To send the string "123" over the SPIH to the FT4222 the following
 * character buffer needs to be transmitted
 */
#pragma data_alignment=8
static uint8_t TxBuffer123[] = {
  FTDI_SYNC_BYTE,
  FTDI_SHORT_MASTER_NO_CHKSUM,
  FTDI_SN,                      /* Serial Number */
  0x00,                         /* Size MSB */
  0x5,                          /* Size LSB */
  HOST_CMD_USER_WRITE_REQ,      /* Command to windows to "write" i.e. rcv */
  0x31,                         
  0x32,
  0x33,
  0x00
  };

/*
 * To send the string "ABC" over the SPIH to the FT4222 the following
 * character buffer needs to be transmitted
 */
#pragma data_alignment=8
static uint8_t TxBufferABC[] = {
  FTDI_SYNC_BYTE,
  FTDI_SHORT_MASTER_NO_CHKSUM,
  FTDI_SN + 1,                  /* Serial Number */
  0x00,                         /* Size MSB */
  0x5,                          /* Size LSB */
  HOST_CMD_USER_WRITE_REQ,      /* Command to windows to "write" i.e. rcv */
  0x41,
  0x42,
  0x43,
  0x00
  };

#define SZ_TX_NO_DATA sizeof(TxNoData)/sizeof(TxNoData[0])

ADI_SPI_RESULT_TYPE         spihResult;
ADI_SPI_DEV_HANDLE          hSpihDevice     = 0;
ADI_SPI_DEV_GENERIC_SETTINGS_TYPE settings;

#define SPI_CLOCK               250000 
/* --------------------------------------------------------------------------
 *
 */
static void openSPIH(void)
{
  /* Initialize SPI */
  if (ADI_SPI_SUCCESS != adi_SPI_MasterInit(ADI_SPI_DEVID_H, &hSpihDevice)) {
      FAIL("Failed to init SPI driver");
  }

  /* throttle bitrate to something the controller can reach */
  if (ADI_SPI_SUCCESS != adi_SPI_SetBitrate(hSpihDevice, SPI_CLOCK)) {
      FAIL("adi_SPI_SetBitrate");
  }

      
  /* disble blocking mode */
  if (ADI_SPI_SUCCESS != adi_SPI_SetBlockingMode(hSpihDevice, true)) {
      FAIL("adi_SPI_SetBlockingMode");
    }


  /* enable dma */
   if (ADI_SPI_SUCCESS != adi_SPI_SetDmaMode(hSpihDevice, true)) {
      FAIL("adi_SPI_SetDmaMode");
    }
  
    if (ADI_SPI_SUCCESS != adi_SPI_SetHWChipSelect( hSpihDevice, true)) {
        FAIL("adi_SPI_SetHWChipSelecthSpihDevice");
    }


  
    /* data transitions on falling edge of clock */
    if (adi_SPI_SetClockPhase(hSpihDevice, false)) 
      FAIL("adi_SPI_SetClockPhase");
    
    /* eliminate clock stalls between bytes */
    if (adi_SPI_SetContinousMode(hSpihDevice, true))
      FAIL("adi_SPI_SetContinousMode");
}



/**************************************************************************
 *
 * Transmit the two buffers defined above over SPIH to the FT4222
 *
 ***************************************************************************/



void ftdi(void)
{

    openSPIH();

    xfr.pPrologue = NULL;    
    xfr.PrologueSize = 0;
    
    xfr.pRxData              = gRecieveBuffer;
    xfr.DataSize             = sizeof(TxBuffer123)/sizeof(TxBuffer123[0]);
    // one too many is resulting in the Win32 Event not being triggered. 
    xfr.bTxIncrement         = true;
    xfr.bRxIncrement         = true; 
    xfr.pTxData              = TxBuffer123;
    
    /* transmit the characters to the SPI port*/
    spihResult = adi_SPI_MasterTransfer(hSpihDevice, &xfr);
    if (ADI_SPI_SUCCESS != spihResult){
      printf("SPIH Tx error(%d)\n",spihResult);
    }
    
    
    if (ADI_SPI_SUCCESS != adi_SPI_SetDmaMode(hSpihDevice, true)) {
        FAIL("adi_SPI_SetDmaMode");
    }
     

    xfr.pRxData              = gRecieveBuffer;
    xfr.DataSize             = sizeof(TxBufferABC)/sizeof(TxBufferABC[0]);
    xfr.bTxIncrement         = true;
    xfr.bRxIncrement         = true; 
    xfr.pTxData              = TxBufferABC;
   
     spihResult = adi_SPI_MasterTransfer(hSpihDevice, &xfr);
     if (ADI_SPI_SUCCESS != spihResult){
        printf("SPIH Tx error(%d)\n",spihResult);
     }
     
     if (ADI_SPI_SUCCESS != adi_SPI_SetDmaMode(hSpihDevice, false)) {
        FAIL("adi_SPI_SetDmaMode");
     }    
}
