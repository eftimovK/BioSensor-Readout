/*********************************************************************************

Copyright (c) 2011-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*****************************************************************************/
/*!
  @file:    GpioTest.c
  @brief:   GPIO Example for ADuCxxx
  @version: $Revision: 28525 $
  @date:    $Date: 2014-11-12 14:51:26 -0500 (Wed, 12 Nov 2014) $
 *****************************************************************************/

/** \addtogroup GPIO_Test GPIO Test
 *  Example code demonstrating use of the GPIO functions.
 *  @{
 */

#include <stdint.h>
#include <stdio.h>

#include "test_common.h"
#include "gpio.h"
#include "i2s.h"
#include "i2c.h"

/* Number of bits per sample. Set this to either 16 or 24 */
#define BITS_PER_SAMPLE  16

/* Number of channels */
#define NUM_CHANNELS     2

/* Number of iterations each test will run */
#define NUMBER_LOOPS     10

/* Audio Codec's I2C address */
#define CODEC_ADDRESS   0x34

/* Enable this to initialize the I2S registers statically during the initialization.
   When enabled none of the configuration APIs will be called.
*/
#define STATIC_CONFIGURATION




#if (NUM_CHANNELS == 1)

#if (BITS_PER_SAMPLE == 16)

short audioData[] = {
  #include "sine_mono16.txt"
};

#else
int32_t audioData[] = {
#include "sine_mono24.txt"
};

#endif /* BITS_PER_SAMPLE */


#else

#if (BITS_PER_SAMPLE == 16)

short audioData[] = {
#include "sine_stereo16.txt"
};

#else
int32_t audioData[] = {
#include "sine_stereo24.txt"
};

#endif /* BITS_PER_SAMPLE */

#endif /* NUM_CHANNELS */

/* i2S device handle */
static  ADI_I2S_DEV_HANDLE   hI2S;

/* Iterations counter */
volatile uint32_t iterations;



/* Writes the given register data into the given register address */
void CodecWrite(ADI_I2C_DEV_HANDLE hDevice, uint8_t RegAddr, uint8_t RegData)
{
    if (ADI_I2C_SUCCESS != adi_I2C_MasterTransmit(hDevice,
                                                  CODEC_ADDRESS,
                                                  RegAddr,
                                                  ADI_I2C_8_BIT_DATA_ADDRESS_WIDTH,
                                                  (uint8_t *)&RegData,
                                                  1,
                                                  true))
    {
        FAIL();
    }

    return;
}

/* Configures the codec */
void ConfigureCodec()
{
  ADI_I2C_DEV_HANDLE hDevice;

   do {
          /* Initialize I2C driver */
          if (ADI_I2C_SUCCESS != adi_I2C_MasterInit(ADI_I2C_DEVID_0, &hDevice)) {
            FAIL();
          }

          /*
             NOTE: former call to deprecated adi_I2C_SetMuxing() removed in
             lieu of required pinmux plugin configuration, adi_initpinmux().
          */

          /* select clock rate */
          if (ADI_I2C_SUCCESS != adi_I2C_SetMasterClock(hDevice, 400000)) {
            FAIL();
          }

          /* enable blocking mode */
          if (ADI_I2C_SUCCESS != adi_I2C_SetBlockingMode(hDevice, true)) {
            FAIL();
          }
#if (BITS_PER_SAMPLE == 16)
          // Power up and set the Master Clock Select (256xfs)
          CodecWrite(hDevice, 0x00, 0x24);

          // Set 16 bit clocks per TDM slot
          CodecWrite(hDevice, 0x03, 0x08);

#else
          // Power up and set the Master Clock Select (384xfs)
          CodecWrite(hDevice, 0x00, 0x28);

          // Set 24 bit clocks per TDM slot
          CodecWrite(hDevice, 0x03, 0x00);
#endif

          // Set sample rate to 8khz and 16-slot TDM mode
          CodecWrite(hDevice, 0x02, 0x10);

          // Unmute the channels
          CodecWrite(hDevice, 0x07, 0x80);

          // Unmute the channels
          CodecWrite(hDevice, 0x0A, 0x00);

          // Set volume for left channel
          CodecWrite(hDevice, 0x06, 0x9F);

          // Set volume for right channel
          CodecWrite(hDevice, 0x05, 0x9F);

   } while(0);

}

/* I2S Callback */
static void I2SCallback(void *pCBParam, uint32_t Event, void *pArg)
{
  switch((ADI_I2S_EVENT) Event)
  {
      case ADI_I2S_EVENT_BUFFER_PROCESSED:
        iterations++;
        adi_I2S_Transmit(hI2S, (uint8_t *)audioData, sizeof(audioData));
        break;

      default:
        break;

  }

  return;
}

extern int32_t adi_initpinmux(void);

void main()
{
    ADI_I2S_RESULT_TYPE         i2s_ret;
    int                         count;

    count = sizeof(audioData);

    /* Initialize system */
    SystemInit();

    /* Set PCLK frequency to 16MHz for better resolution */
    /* Need to set HCLK frequency to 16MHz first */
    if (ADI_SYS_SUCCESS != SetSystemClockDivider(ADI_SYS_CLOCK_CORE, 1)) {
		FAIL();
    }
    if (ADI_SYS_SUCCESS !=SetSystemClockDivider(ADI_SYS_CLOCK_PCLK, 1)) {
		FAIL();
    }

    /* test system initialization */
    test_Init();

    /* Set statically generated pin-muxing */
    if (ADI_I2C_SUCCESS != adi_initpinmux()) {
		FAIL();
	}

    /* Initialize GPIO service */
    if (ADI_GPIO_SUCCESS != adi_GPIO_Init()) {
		FAIL();
    }

    /* Configure the Codec registers via I2C */
    ConfigureCodec();

   do
    {
      /* I2S device initialization */
      i2s_ret = adi_I2S_Init( ADI_I2S_DEVID_0, &hI2S);
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

#ifdef STATIC_CONFIGURATION

     /* Static configuration is set to 16 bit, so if we are testing 24 bit
        the configuration has to be changed dynamically */
#if (BITS_PER_SAMPLE == 24)

      i2s_ret = adi_I2S_SlotWidth( hI2S, ADI_I2S_SLOT_WIDTH_24_BIT );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_DataWidth( hI2S, ADI_I2S_DATA_WIDTH_24_BIT );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret =  adi_I2S_SetTranMode ( hI2S,  ADI_I2S_TRAN_MODE_24);
     if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_SetBLCKRate( hI2S, ADI_I2S_BCLK_RATE_32_BCLKS );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

#endif /* BITS_PER_SAMPLE */



#else

      i2s_ret = adi_I2S_SetDataFormat( hI2S, ADI_I2S_DATA_FORMAT_DEFAULT );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_SetChannelFormat( hI2S, ADI_I2S_CHANNEL_FORMAT_TDM16 );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_FrameRate( hI2S, ADI_I2S_FRAME_RATE_8_KHZ);
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

#if (BITS_PER_SAMPLE == 16)
      i2s_ret = adi_I2S_SlotWidth( hI2S, ADI_I2S_SLOT_WIDTH_16_BIT );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_DataWidth( hI2S, ADI_I2S_DATA_WIDTH_16_BIT );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret =  adi_I2S_SetTranMode ( hI2S,  ADI_I2S_TRAN_MODE_16);
     if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_SetBLCKRate( hI2S, ADI_I2S_BCLK_RATE_16_BCLKS );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

#else
      i2s_ret = adi_I2S_SlotWidth( hI2S, ADI_I2S_SLOT_WIDTH_24_BIT );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_DataWidth( hI2S, ADI_I2S_DATA_WIDTH_24_BIT );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret =  adi_I2S_SetTranMode ( hI2S,  ADI_I2S_TRAN_MODE_24);
     if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_SetBLCKRate( hI2S, ADI_I2S_BCLK_RATE_32_BCLKS );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

#endif

      i2s_ret = adi_I2S_SetLRCLKMode( hI2S, ADI_I2S_LR_MODE_50_DUTY_CYCLE);
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_SetEndianFormat( hI2S, ADI_I2S_ENDIAN_MSB_FIRST );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_SetDriveUnusedBits (hI2S, true);
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();


      i2s_ret = adi_I2S_SetMasterSlave( hI2S, ADI_I2S_MASTER_SLAVE_SELECT_MASTER );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_SetLRCLKPolarity( hI2S, ADI_I2S_LRCLK_POLARITY_NO_INVERT );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_SetBLCKEdge  ( hI2S, ADI_I2S_BLK_EDGE_FALLING );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_SetChannel_2_Drive( hI2S, ADI_I2S_CHANNEL_2_DRIVE_EN );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_SetChannel_1_Drive( hI2S, ADI_I2S_CHANNEL_1_DRIVE_EN );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_SetChannel_2_OutputMap( hI2S, ADI_I2S_CHANNEL_MAP_SLOT1 );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_SetChannel_1_OutputMap( hI2S, ADI_I2S_CHANNEL_MAP_SLOT0 );
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

     i2s_ret =  adi_I2S_SetFifoReset          ( hI2S,   ADI_I2S_FIFO_RESET_NORMAL );
     if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

     if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();
     i2s_ret =  adi_I2S_SetIncrReset           ( hI2S,  ADI_I2S_INCR_RESET_NORMAL);

     if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

     i2s_ret =  adi_I2S_SetReqEnable           ( hI2S, true);
     if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

     i2s_ret =  adi_I2S_SetStatEnable          ( hI2S,  false);
     if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

     i2s_ret =  adi_I2S_SetAutoIncrEnable      ( hI2S, false);
     if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

     i2s_ret =  adi_I2S_SetChanSel_2_Enable    ( hI2S,  true);
     if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

     i2s_ret =  adi_I2S_SetChanSel_1_Enable    ( hI2S,  true);
     if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();
#endif /* STATIC_CONFIGURATION */

#if (NUM_CHANNELS == 1)

     i2s_ret = adi_I2S_SetChannel_2_Drive( hI2S, ADI_I2S_CHANNEL_2_DRIVE_DIS );
     if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

     i2s_ret =  adi_I2S_SetChanSel_2_Enable    ( hI2S,  false);
     if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

#endif /* NUM_CHANNELS */

      i2s_ret = adi_I2S_SetBlockingMode( hI2S, false);
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret =  adi_I2S_Enable( hI2S, true );
      if( i2s_ret != ADI_I2S_SUCCESS)
         FAIL();

      /******************************    PIO NON-BLOCKING *******************/
      iterations = 0;
      do
      {
        uint16_t pBytesRemaining;

        i2s_ret =  adi_I2S_Transmit(hI2S, (uint8_t *)audioData, count);

        if( i2s_ret != ADI_I2S_SUCCESS)
          FAIL();

        do
        {

          i2s_ret = adi_I2S_GetNonBlockingStatus( hI2S, &pBytesRemaining);
          if( i2s_ret != ADI_I2S_SUCCESS)
            FAIL();

        } while( pBytesRemaining );

      } while( (iterations++ < NUMBER_LOOPS) && i2s_ret == ADI_I2S_SUCCESS);

      /******************************    DMA NON-BLOCKING *******************/

      i2s_ret = adi_I2S_SetDmaMode( hI2S, true);
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      iterations = 0;
      do
      {
        i2s_ret =  adi_I2S_Transmit(hI2S, (uint8_t *)audioData, count);

        if( i2s_ret != ADI_I2S_SUCCESS)
          FAIL();

        /* Wait until the completion of the DMA */
        while(adi_I2S_GetDmaCompletion(hI2S) == false);

      } while( (iterations++ < NUMBER_LOOPS) && i2s_ret == ADI_I2S_SUCCESS);

      /******************************    PIO BLOCKING *******************/

      i2s_ret = adi_I2S_SetDmaMode( hI2S, false);
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_SetBlockingMode( hI2S, true);
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();


      iterations = 0;
      do
      {
        i2s_ret =  adi_I2S_Transmit(hI2S, (uint8_t *)audioData, count);
        if( i2s_ret != ADI_I2S_SUCCESS)
          FAIL();

      } while( (iterations++ < NUMBER_LOOPS) && i2s_ret == ADI_I2S_SUCCESS);


      /* Non blocking callback mode */

      i2s_ret =  adi_I2S_RegisterCallback(hI2S, I2SCallback, NULL);
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret = adi_I2S_SetBlockingMode( hI2S, false);
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();


      /* PIO mode test with callback */

      i2s_ret = adi_I2S_SetDmaMode( hI2S, false);
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();


      iterations = 0;

      i2s_ret =  adi_I2S_Transmit(hI2S, (uint8_t *)audioData, count);
      if( i2s_ret != ADI_I2S_SUCCESS)
          FAIL();

      while(iterations < NUMBER_LOOPS);

      /* DMA mode test with callback */

      i2s_ret = adi_I2S_SetDmaMode( hI2S, true);
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      iterations = 0;

      i2s_ret =  adi_I2S_Transmit(hI2S, (uint8_t *)audioData, count);
      if( i2s_ret != ADI_I2S_SUCCESS)
          FAIL();

      while(iterations < NUMBER_LOOPS);

      i2s_ret = adi_I2S_SetDmaMode( hI2S, false);
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();

      i2s_ret =  adi_I2S_UnInit(hI2S);
      if( i2s_ret != ADI_I2S_SUCCESS)
        FAIL();


    } while(0);

    PASS();
}


