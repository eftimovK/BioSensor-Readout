/*********************************************************************************

Copyright (c) 2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*****************************************************************************
 * @file:    AmperometricMeasurement.c
 * @brief:   Amperometric measurement example.
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "test_common.h"

#include "afe.h"
#include "afe_lib.h"
#include "uart.h"

/* Macro to enable the returning of AFE data using the UART */
/*      1 = return AFE data on UART                         */
/*      0 = return AFE data on SW (Std Output)              */
#define USE_UART_FOR_DATA           (1)

/* Helper macro for printing strings to UART or Std. Output */
#define PRINT(s)                    test_print(s)

/****************************************************************************/
/*  <----------- DURL1 -----------><----------- DURL2 ----------->          */
/*                  <-- DURIVS1 --><-- DURIVS2 -->                          */
/*                                 _______________________________ <--- VL2 */
/*                                |                                         */
/*  ______________________________|                                <--- VL1 */
/*                                                                          */
/*  <---- dur1 ----><--- dur2 ---><---- dur3 ----><---- dur4 ---->          */
/****************************************************************************/

/* DC Level 1 voltage in mV (range: -0.8V to 0.8V) */
#define VL1                         (0)
/* DC Level 2 voltage in mV (range: -0.8V to 0.8V) */
#define VL2                         (300)
/* The duration (in us) of DC Level 1 voltage */
#define DURL1                       ((uint32_t)(1000000))
/* The duration (in us) of DC Level 2 voltage */
#define DURL2                       ((uint32_t)(3000000))
/* The duration (in us) which the IVS switch should remain closed (to shunt */
/* switching current) before changing the DC level.                         */
#define DURIVS1                     ((uint32_t)(100))
/* The duration (in us) which the IVS switch should remain closed (to shunt */
/* switching current) after changing the DC level.                          */
#define DURIVS2                     ((uint32_t)(100))
/* Is shunting of the switching current required? Required: 1, Not required: 0 */
#define SHUNTREQD                   (1)

/* RCAL value, in ohms                                              */
/* Default value on ADuCM350 Switch Mux Config Board Rev.0 is 1k    */
#define RCAL                        (1000)
/* RTIA value, in ohms                                              */
/* Default value on ADuCM350 Switch Mux Config Board Rev.0 is 7.5k  */
#define RTIA                        (7500)

/* DO NOT EDIT: DAC LSB size in mV, before attenuator (1.6V / (2^12 - 1)) */
#define DAC_LSB_SIZE                (0.39072)
/* DO NOT EDIT: DC Level 1 in DAC codes */
#define DACL1                       ((uint32_t)(((float)VL1 / (float)DAC_LSB_SIZE) + 0x800))
/* DO NOT EDIT: DC Level 2 in DAC codes */
#define DACL2                       ((uint32_t)(((float)VL2 / (float)DAC_LSB_SIZE) + 0x800))

#define DECIMATION                  (uint8_t)(80)  // decimation (downsampling) factor

/* DO NOT EDIT: Number of samples to be transferred by DMA, based on the duration of */
/* the sequence.                                                                     */
/* SAMPLE_COUNT = (Level 1 Duration + Level 2 Duration)us * (160k/178)samples/s      */
#define SAMPLE_COUNT                (uint32_t)(8000) // ADC_SPS / DECIMATION * DURATION = max_SAMPLE_COUNT; (ADC_SPS = 160kSPS)

/* Size limit for each DMA transfer (max 1024) */
#define DMA_BUFFER_SIZE             (1024u)  // should be a multiple of DECIMATION! else, it gets rounded to the nearest multiple

/* DO NOT EDIT: Maximum printed message length. Used for printing only. */
#define MSG_MAXLEN                  (50)

#pragma location="nonvolatile_ram"  // RAM0 is non-volatile (see file ADuCM350BBCZ_CP.icf)
uint16_t        dmaBuffer[DMA_BUFFER_SIZE * 2];


#pragma location="volatile_ram"     // store in RAM1
uint16_t        adc[SAMPLE_COUNT];

uint16_t adc_count = 0;

/* Sequence for Amperometric measurement */
uint32_t seq_afe_ampmeas[] = {
    0x00160065,   /*  0 - Safety Word, Command Count = 15, CRC = 0x1C                                       */
    0x84003818, // DATA_FIFO_SOURCE_SEL = 0b01 (ADC); OLD CODE: 0x84007818,   /*  1 - AFE_FIFO_CFG: DATA_FIFO_SOURCE_SEL = 0b11 (LPF)                                   */
    0x8A000030,   /*  2 - AFE_WG_CFG: TYPE_SEL = 0b00                                                       */
    0x88000F00,   /*  3 - AFE_DAC_CFG: DAC_ATTEN_EN = 0 (disable DAC attenuator)                            */
    0xAA000800,   /*  4 - AFE_WG_DAC_CODE: DAC_CODE = 0x800 (DAC Level 1 placeholder, user programmable)    */
    0xA0000002,   /*  5 - AFE_ADC_CFG: MUX_SEL = 0b00010, GAIN_OFFS_SEL = 0b00 (TIA)                        */
    0xA2000000,   /*  6 - AFE_SUPPLY_LPF_CFG: BYPASS_SUPPLY_LPF = 0 (do not bypass)                         */
    0x86006655,   /*  7 - DMUX_STATE = 5, PMUX_STATE = 5, NMUX_STATE = 6, TMUX_STATE = 6                    */
    0x0001A900,   /*  8 - Wait: 6.8ms (based on load RC = 6.8kOhm * 1uF)                                    */
    0x80024EF0,   /*  9 - AFE_CFG: WG_EN = 1                                                                */
    0x00000C80,   /* 10 - Wait: 200us                                                                       */
    0x80034FF0,   /* 11 - AFE_CFG: ADC_CONV_EN = 1, SUPPLY_LPF_EN = 1                                       */
    0x00090880,   /* 12 - Wait: 37ms  for LPF settling                                                      */
    0x00000000,   /* 13 - Wait: (DAC Level 1 duration - IVS duration 1) (placeholder, user programmable)    */
    0x86016655,   /* 14 - IVS_STATE = 1 (close IVS switch, user programmable)                               */
    0x00000000,   /* 15 - Wait: IVS duration 1 (placeholder, user programmable)                             */
    0xAA000800,   /* 16 - AFE_WG_DAC_CODE: DAC_CODE = 0x800 (DAC Level 2 placeholder, user programmable)    */
    0x00000000,   /* 17 - Wait: IVS duration 2 (placeholder, user programmable)                             */
    0x86006655,   /* 18 - IVS_STATE = 0 (open IVS switch)                                                   */
    0x00000000,   /* 19 - Wait: (DAC Level 2 duration - IVS duration 2) (placeholder, user programmable)    */
    0x00000000,   /* 20 - Wait: finish DMA transfers (placeholder, programmed below)                        */
    0x80020EF0,   /* 21 - AFE_CFG: WAVEGEN_EN = 0, ADC_CONV_EN = 0, SUPPLY_LPF_EN = 0                       */
    0x82000002,   /* 22 - AFE_SEQ_CFG: SEQ_EN = 0                                                           */
};

/* Variables and functions needed for data output through UART */
ADI_UART_HANDLE     hUartDevice     = NULL;

/* Function prototypes */
void                    test_print                  (char *pBuffer);
ADI_UART_RESULT_TYPE    uart_Init                   (void);
ADI_UART_RESULT_TYPE    uart_UnInit                 (void);
extern int32_t          adi_initpinmux              (void);
void        RxDmaCB         (void *hAfeDevice, 
                             uint32_t length, 
                             void *pBuffer);

int main(void) {
    ADI_AFE_DEV_HANDLE  hAfeDevice;
    uint32_t            dur1;
    uint32_t            dur2;
    uint32_t            dur3;
    uint32_t            dur4;
    
    /* Initialize system */
    SystemInit();

    /* Change the system clock source to HFXTAL and change clock frequency to 16MHz     */
    /* Requirement for AFE (ACLK)                                                       */
    SystemTransitionClocks(ADI_SYS_CLOCK_TRIGGER_MEASUREMENT_ON);
    
    /* SPLL with 32MHz used, need to divide by 2 */
    SetSystemClockDivider(ADI_SYS_CLOCK_UART, 2);
    
    /* Test initialization */
    test_Init();

    /* initialize static pinmuxing */
    adi_initpinmux();

    /* Initialize the UART for transferring measurement data out */
    if (ADI_UART_SUCCESS != uart_Init())
    {
        FAIL("uart_Init");
    }

    /* Initialize the AFE API */
    if (ADI_AFE_SUCCESS != adi_AFE_Init(&hAfeDevice)) 
    {
        FAIL("Init");
    }

    /* Set RCAL Value */
    if (ADI_AFE_SUCCESS != adi_AFE_SetRcal(hAfeDevice, RCAL))
    {
        FAIL("Set RCAL");
    }

    /* Set RTIA Value */
    if (ADI_AFE_SUCCESS != adi_AFE_SetRtia(hAfeDevice, RTIA))
    {
        FAIL("Set RTIA");
    }
    
    /* Set decimation factor */
    if (ADI_AFE_SUCCESS != adi_AFE_SetDecFactor(hAfeDevice, DECIMATION))
    {
        FAIL("Set decFactor");
    }

    /* AFE power up */
    if (ADI_AFE_SUCCESS != adi_AFE_PowerUp(hAfeDevice)) 
    {
        FAIL("PowerUp");
    }

    /* Excitation Channel Power-Up */
    if (ADI_AFE_SUCCESS != adi_AFE_ExciteChanPowerUp(hAfeDevice)) 
    {
        FAIL("ExciteChanCalAtten");
    }

    /* TIA Channel Calibration */
    if (ADI_AFE_SUCCESS != adi_AFE_TiaChanCal(hAfeDevice)) 
    {
        FAIL("TiaChanCal");
    }

    /* Excitation Channel (no attenuation) Calibration */
    if (ADI_AFE_SUCCESS != adi_AFE_ExciteChanCalNoAtten(hAfeDevice)) 
    {
            FAIL("adi_AFE_ExciteChanCalNoAtten");
        }

    /* Amperometric Measurement */
    /* Set the user programmable portions of the sequence */
    /* Set the duration values */
    if (SHUNTREQD) 
    {
    dur1 = DURL1 - DURIVS1;
    dur2 = DURIVS1;
    dur3 = DURIVS2;
    dur4 = DURL2 - DURIVS2;
    }
    else 
    {
        dur1 = DURL1;
        dur2 = 0;
        dur3 = 0;
        dur4 = DURL2;
    }

    /* Set durations in ACLK periods */
        seq_afe_ampmeas[13] = dur1 * 16;
        seq_afe_ampmeas[15] = dur2 * 16;
        seq_afe_ampmeas[17] = dur3 * 16;
        seq_afe_ampmeas[19] = dur4 * 16;
        
        // If needed, wait before turning off ADC_CONV_EN in the sequencer
        // seq_afe_ampmeas[20] = 0.05*(DURL1 + DURL2) * 16; // wait 5% of the total measurement duration
                                                         // to make sure the dma transfers are complete

    /* Set DAC Level 1 */
    seq_afe_ampmeas[4]  = SEQ_MMR_WRITE(REG_AFE_AFE_WG_DAC_CODE, DACL1);
    /* Set DAC Level 2 */
    seq_afe_ampmeas[16] = SEQ_MMR_WRITE(REG_AFE_AFE_WG_DAC_CODE, DACL2);
    
    if (!SHUNTREQD)
    {
        /* IVS switch remains open */
        seq_afe_ampmeas[14] &= 0xFFFEFFFF;
    }
    
#if (ADI_AFE_CFG_ENABLE_RX_DMA_DUAL_BUFFER_SUPPORT == 1)   
    /* Set the Rx DMA buffer sizes */
    if (ADI_AFE_SUCCESS != adi_AFE_SetDmaRxBufferMaxSize(hAfeDevice, DMA_BUFFER_SIZE, DMA_BUFFER_SIZE))
    {
        FAIL("adi_AFE_SetDmaRxBufferMaxSize");
    }
#endif /* ADI_AFE_CFG_ENABLE_RX_DMA_DUAL_BUFFER_SUPPORT == 1 */
    
    /* Register Rx DMA Callback */
    if (ADI_AFE_SUCCESS != adi_AFE_RegisterCallbackOnReceiveDMA(hAfeDevice, RxDmaCB, 0))
    {
        FAIL("adi_AFE_RegisterCallbackOnReceiveDMA");
    }
        
    /* Recalculate CRC in software for the amperometric measurement */
    adi_AFE_EnableSoftwareCRC(hAfeDevice, true);

    /* Perform the Amperometric measurement(s) */
    if (ADI_AFE_SUCCESS != adi_AFE_RunSequence(hAfeDevice, seq_afe_ampmeas, (uint16_t *) dmaBuffer, SAMPLE_COUNT)) 
    {
        FAIL("adi_AFE_RunSequence");   
    }

    /* Restore to using default CRC stored with the sequence */
    adi_AFE_EnableSoftwareCRC(hAfeDevice, false);
    
    /* AFE Power Down */
    if (ADI_AFE_SUCCESS != adi_AFE_PowerDown(hAfeDevice)) 
    {
        FAIL("adi_AFE_PowerDown");
    }

    /* Unregister Rx DMA Callback */
    if (ADI_AFE_SUCCESS != adi_AFE_RegisterCallbackOnReceiveDMA(hAfeDevice, NULL, 0))
        {
        FAIL("adi_AFE_RegisterCallbackOnReceiveDMA (unregister)");
        }

    /* Uninitialize the AFE API */
    if (ADI_AFE_SUCCESS != adi_AFE_UnInit(hAfeDevice)) 
    {
        FAIL("adi_AFE_UnInit");
    }
        
    // EDIT: send data to serial port after measurement
    int16_t i;
    char                    msg[MSG_MAXLEN];
    
    for (i=0; i < SAMPLE_COUNT; i++)
    {
      sprintf(msg, "%u\r\n", adc[i]);
      PRINT(msg);
    }
    
    /* Uninitialize the UART */
    adi_UART_UnInit(hUartDevice);
    
    PASS();
}

/*!
 * @brief       AFE Rx DMA Callback Function.
 *
 * @param[in]   hAfeDevice  Device handle obtained from adi_AFE_Init()
 *              length      Number of U16 samples received from the DMA
 *              pBuffer     Pointer to the buffer containing the LPF results
 *              
 *
 * @details     16-bit results are converted to bytes and transferred using the UART
 *
 */
void RxDmaCB(void *hAfeDevice, uint32_t length, void *pBuffer)
{
#if (1 == USE_UART_FOR_DATA)
    uint32_t                i;
    uint16_t                *ppBuffer = (uint16_t*)pBuffer;
    uint8_t                 decCounter = 0; // counts to DECIMATION, replaces modulus
    uint32_t                adc_sum = 0;

    /* Check if there are samples to be sent */
    if (length)
    {
        for (i = 1; i <= length; i++)
        {
            adc_sum = adc_sum + *ppBuffer++;    // add current ADC value
            decCounter++;
            
            if (decCounter == DECIMATION)
            {
                adc[adc_count] = (uint16_t)(adc_sum / DECIMATION);
                adc_count++;            
                adc_sum = 0;
                decCounter = 0;
            }
        }
    }

#elif (0 == USE_UART_FOR_DATA)
    FAIL("Std. Output is too slow for ADC/LPF data. Use UART instead.");
    
#endif /* USE_UART_FOR_DATA */
}

/* Helper function for printing a string to UART or Std. Output */
void test_print (char *pBuffer) {
#if (1 == USE_UART_FOR_DATA)
    int16_t size;
    /* Print to UART */
    size = strlen(pBuffer);
    adi_UART_BufTx(hUartDevice, pBuffer, &size);

#elif (0 == USE_UART_FOR_DATA)
    /* Print  to console */
    printf(pBuffer);

#endif /* USE_UART_FOR_DATA */
}

/* Initialize the UART, set the baud rate and enable */
ADI_UART_RESULT_TYPE uart_Init (void) {
    ADI_UART_RESULT_TYPE    result = ADI_UART_SUCCESS;
    
    /* Open UART in blocking, non-intrrpt mode by supplying no internal buffs */
    if (ADI_UART_SUCCESS != (result = adi_UART_Init(ADI_UART_DEVID_0, &hUartDevice, NULL)))
    {
        return result;
    }

    /* Set UART baud rate to 115200 */
    if (ADI_UART_SUCCESS != (result = adi_UART_SetBaudRate(hUartDevice, ADI_UART_BAUD_115200)))
    {
        return result;
    }
    
    /* Enable UART */
    if (ADI_UART_SUCCESS != (result = adi_UART_Enable(hUartDevice,true)))
    {
        return result;
    }
    
    return result;
}

/* Uninitialize the UART */
ADI_UART_RESULT_TYPE uart_UnInit (void) {
    ADI_UART_RESULT_TYPE    result = ADI_UART_SUCCESS;
    
  /* Uninitialize the UART API */
    if (ADI_UART_SUCCESS != (result = adi_UART_UnInit(hUartDevice)))
    {
        return result;
    }
    
    return result;
}