/*********************************************************************************

Copyright (c) 2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/


/*****************************************************************************
 * @file:    UnipolarCurrentMeasurement.c
 * @brief:   Unipolar current measurement example.
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
#define VL2                         (100)
/* The duration (in us) of DC Level 1 voltage */
#define DURL1                       ((uint32_t)(2500000))
/* The duration (in us) of DC Level 2 voltage */
#define DURL2                       ((uint32_t)(2500000))

/* The duration (in us) which the IVS switch should remain closed (to shunt */
/* switching current) before changing the DC level.                         */
#define DURIVS1                     ((uint32_t)(100))
/* The duration (in us) which the IVS switch should remain closed (to shunt */
/* switching current) after changing the DC level.                          */
#define DURIVS2                     ((uint32_t)(100))

/* Is shunting of the switching current required? Required: 1, Not required: 0 */
#define SHUNTREQD                   (0)

/* DO NOT EDIT: DAC LSB size in mV, before attenuator (1.6V / (2^12 - 1)) */
#define DAC_LSB_SIZE                (0.39072)
/* DO NOT EDIT: DC Level 1 in DAC codes */
#define DACL1                       ((uint32_t)((((float)VL1) / (float)DAC_LSB_SIZE) + 0x800))
/* DO NOT EDIT: DC Level 2 in DAC codes */
#define DACL2                       ((uint32_t)((((float)VL2) / (float)DAC_LSB_SIZE) + 0x800))

/* DO NOT EDIT: Number of samples to be transferred by DMA, based on the duration of */
/* the sequence.                                                                     */
/* SAMPLE_COUNT = (Level 1 Duration + Level 2 Duration)us * (160k/178)samples/s      */
#define SAMPLE_COUNT                (uint32_t)((2 * (DURL1 + DURL2)) / 2225)

/* Size limit for each DMA transfer */
#define DMA_BUFFER_SIZE             (300u)

/* DO NOT EDIT: Maximum printed message length. Used for printing only. */
#define MSG_MAXLEN                  (50)

/* Resistor values, if they change, the ADC code values below need to be changed too */
#define RTIA                        (37400)
#define RCAL                        (1000)
#define RL                          (49900)

/* The min and max ADC codes depend on the value of RL and the input range.         */
/* Expected ADC code for 0uA input current:                                         */
/*   ADC_CODE_HIGH = 0x8000 + VBIAS / RL * RTIA * 1.5 / 3.6 * 2^16                  */
/* Expected ADC code for 44uA input current:                                        */
/*   ADC_CODE_LOW  = 0x8000 + (VBIAS / RL - 44e-6) * RTIA * 1.5 / 3.6 * 2^16        */
#define ADC_CODE_HIGH               ((uint32_t)(0x8000 + 1.1 / RL * RTIA * 1.5 / 3.6 * 65536 + 0.5))

/* Switch matrix state for TIA channel calibration */
/* Needs custom state configuration (MSB + LSB) because of the level shifting resistor         */
/* Depending on the application, it may be desirable to calibrate the RL offset by measuring   */
/* an open with one of the T switches closed (nothing connected to the corresponding AFE pin). */
#define ADI_SWM_MSB_TIACHANCAL      ((0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_PL) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_P8) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_P7) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_P6) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_P5) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_P4) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_P3) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_P2) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_PR1) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_D8) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_D7) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_D6) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_D5) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_D4) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_D3) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_D2) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_DR1))

#define ADI_SWM_LSB_TIACHANCAL      ((0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_IVS) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_NL) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_NR2) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_N7) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_N6) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_N5) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_N4) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_N3) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_N2) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_N1) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_TR2) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_T7) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_T6) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_T5) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_T4) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_T3) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_T2) | \
                                     (1 << BITP_AFE_AFE_SW_FULL_CFG_LSB_T1))

/* Switch matrix state for measurement (AFE4-AFE5) */
/* Needs custom state configuration (MSB + LSB) because of the level shifting resistor */
#define ADI_SWM_MSB_MEASUREMENT     ((0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_PL) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_P8) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_P7) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_P6) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_P5) | \
                                     (1 << BITP_AFE_AFE_SW_FULL_CFG_MSB_P4) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_P3) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_P2) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_PR1) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_D8) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_D7) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_D6) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_D5) | \
                                     (1 << BITP_AFE_AFE_SW_FULL_CFG_MSB_D4) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_D3) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_D2) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_MSB_DR1))

#define ADI_SWM_LSB_MEASUREMENT     ((0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_IVS) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_NL) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_NR2) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_N7) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_N6) | \
                                     (1 << BITP_AFE_AFE_SW_FULL_CFG_LSB_N5) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_N4) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_N3) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_N2) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_N1) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_TR2) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_T7) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_T6) | \
                                     (1 << BITP_AFE_AFE_SW_FULL_CFG_LSB_T5) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_T4) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_T3) | \
                                     (0 << BITP_AFE_AFE_SW_FULL_CFG_LSB_T2) | \
                                     (1 << BITP_AFE_AFE_SW_FULL_CFG_LSB_T1))

#pragma location="volatile_ram"
uint16_t        dmaBuffer[DMA_BUFFER_SIZE * 2];

/* Sequence for unipolar current measurement */
uint32_t seq_afe_unipolarcurrentmeasurement[] = {
    0x001A0000,   /*  0 - Safety word: bits 31:16 = command count, bits 7:0 = CRC                                            */
    0x84007818,   /*  1 - AFE_FIFO_CFG: DATA_FIFO_SOURCE_SEL = 11 (LPF)                                                      */
    0x8A000030,   /*  2 - AFE_WG_CFG: TYPE_SEL = 00                                                                          */
    0x88000F00,   /*  3 - AFE_DAC_CFG: DAC_ATTEN_EN = 0 (disable DAC attenuator)                                             */
    0xAA000000,   /*  4 - AFE_WG_DAC_CODE: DAC_CODE = 0x0 (DAC Level 1 placeholder, user programmable)                       */
    0xA0000002,   /*  5 - AFE_ADC_CFG: MUX_SEL = 00010, GAIN_OFFS_SEL = 00 (TIA)                                             */
    0xA2000000,   /*  6 - AFE_SUPPLY_LPF_CFG: BYPASS_SUPPLY_LPF = 0 (do not bypass) (default value, user programmable)       */
    0x00000C80,   /*  7 - Wait 200us                                                                                         */
    0x00000000,   /*  8 - Placeholder for switch matrix configuration                                                        */
    0x00000000,   /*  9 - Placeholder for switch matrix configuration                                                        */
    0x86020000,   /* 10 - AFE_SW_CFG: SW_SOURCE_SEL = 1                                                                      */
    0x00000640,   /* 11 - Wait 100us                                                                                         */
    0x80024EF0,   /* 12 - AFE_CFG: WG_EN = 1                                                                                 */
    0x00000C80,   /* 13 - Wait 200us                                                                                         */
    0x80034FF0,   /* 14 - AFE_CFG: ADC_CONV_EN = 1, SUPPLY_LPF_EN = 1                                                        */
    0x00090880,   /* 15 - Wait 37ms for LPF settling                                                                         */
    0x00000000,   /* 16 - Wait (DAC Level 1 duration - IVS duration 1) (placeholder, user programmable)                      */
    0x00000000,   /* 17 - Potentially IVS_STATE = 1 (close IVS switch, user programmable)                                    */
    0x86020000,   /* 18 - AFE_SW_CFG: SW_SOURCE_SEL = 1                                                                      */   
    0x00000000,   /* 19 - Wait IVS duration 1 (placeholder, user programmable)                                               */
    0xAA000000,   /* 20 - AFE_WG_DAC_CODE: DAC_CODE = 0x0 (DAC Level 2 placeholder, user programmable)                       */
    0x00000000,   /* 21 - Wait IVS duration 2 (placeholder, user programmable)                                               */
    0x00000000,   /* 22 - Potentially IVS_STATE = 0 (open IVS switch, user programmable)                                     */
    0x86020000,   /* 23 - AFE_SW_CFG: SW_SOURCE_SEL = 1                                                                      */   
    0x00000000,   /* 24 - Wait (DAC Level 2 duration - IVS duration 2) (placeholder, user programmable)                      */
    0x80020EF0,   /* 25 - AFE_CFG: WAVEGEN_EN = 0, ADC_CONV_EN = 0, SUPPLY_LPF_EN = 0                                        */
    0x82000002,   /* 26 - AFE_SEQ_CFG: SEQ_EN = 0                                                                            */
};


uint32_t seq_afe_unipolarcurrent_tiachancal1[] = {
    0x000A0064,   /*  0 - Safety word: bits 31:16 = command count, bits 7:0 = CRC    */
    0xA4000101,   /*  1 - AFE_SW_FULL_CFG_MSB: DR1 = 1, PR1 = 1                      */
    0xA6008081,   /*  2 - AFE_SW_FULL_CFG_LSB: T1 = 1, TR2 = 1, NR2 = 1              */
    0x86020000,   /*  3 - AFE_SW_CFG: SW_SOURCE_SEL = 1                              */
    0xAA000870,   /*  4 - AFE_WG_DAC_CODE: DAC_CODE = 0x870                          */
    0xA0000018,   /*  5 - AFE_ADC_CFG: MUX_SEL = 11000, GAIN_OFFS_SEL = 00 (TIA)     */
    0x00000640,   /*  6 - Wait 100us                                                 */
    0x80030FF0,   /*  7 - AFE_CFG: ADC_CONV_EN = 1, SUPPLY_LPF_EN = 1                */
    0x00090880,   /*  8 - Wait 37ms                                                  */
    0x80020EF0,   /*  9 - AFE_CFG: ADC_CONV_EN = 0, SUPPLY_LPF_EN = 0                */
    0x82000002,   /* 10 - AFE_SEQ_CFG: SEQ_EN = 0                                    */
};

uint32_t seq_afe_unipolarcurrent_tiachancal2[] = {
    0x000700AA,   /*  0 - Safety word: bits 31:16 = command count, bits 7:0 = CRC    */
    0xA0000000,   /*  1 - AFE_ADC_CFG: MUX_SEL = 00000                               */
    0xA0000019,   /*  2 - AFE_ADC_CFG: MUX_SEL = 11001                               */
    0x00000640,   /*  3 - Wait 100us                                                 */
    0x80030FF0,   /*  4 - AFE_CFG: ADC_CONV_EN = 1, SUPPLY_LPF_EN = 1                */
    0x00090880,   /*  5 - Wait 37ms                                                  */
    0x80020EF0,   /*  6 - AFE_CFG: ADC_CONV_EN = 0, SUPPLY_LPF_EN = 0                */
    0x82000002,   /*  7 - AFE_SEQ_CFG: SEQ_EN = 0                                    */
};

uint32_t seq_afe_unipolarcurrent_tiachancal3[] = {
    0x000A0000,   /*  0 - Safety word: bits 31:16 = command count, bits 7:0 = CRC    */
    0xA0000000,   /*  1 - AFE_ADC_CFG: MUX_SEL = 00000                               */
    0xA0000002,   /*  2 - AFE_ADC_CFG: MUX_SEL = 00010                               */
    0xA4000000 + ADI_SWM_MSB_TIACHANCAL,    /* 3 */
    0xA6000000 + ADI_SWM_LSB_TIACHANCAL,    /* 4 */
    0x86020000,   /*  5 - AFE_SW_CFG: SW_SOURCE_SEL = 1                              */
    0x00000640,   /*  6 - Wait 100us                                                 */
    0x80030FF0,   /*  7 - AFE_CFG: ADC_CONV_EN = 1, SUPPLY_LPF_EN = 1                */
    0x00090880,   /*  8 - Wait 37ms                                                  */
    0x80020EF0,   /*  9 - AFE_CFG: ADC_CONV_EN = 0, SUPPLY_LPF_EN = 0                */
    0x82000002,   /* 10 - AFE_SEQ_CFG: SEQ_EN = 0                                    */
};


/* Variables and functions needed for data output through UART */
ADI_UART_HANDLE     hUartDevice     = NULL;

/* Function prototypes */
void                    test_print                  (char *pBuffer);
ADI_UART_RESULT_TYPE    uart_Init                   (void);
ADI_UART_RESULT_TYPE    uart_UnInit                 (void);
extern int32_t          adi_initpinmux              (void);
void                    RxDmaCB                     (void *hAfeDevice, 
                                                     uint32_t length, 
                                                     void *pBuffer);
/* Modified TIA channel calibration routine for unipolar current measurement */
ADI_AFE_RESULT_TYPE     adi_AFE_CustomTiaChanCal    (ADI_AFE_DEV_HANDLE const hDevice);

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
    
    /* SPLL output is 32MHz, we need to divide by 2 for 16MHz PCLK */
    SetSystemClockDivider(ADI_SYS_CLOCK_UART, 2);
    
    /* Test initialization */
    test_Init();

    /* Initialize static pinmuxing */
    adi_initpinmux();

    /* Initialize the UART for transferring measurement data out */
    if (ADI_UART_SUCCESS != uart_Init())
    {
        FAIL("uart_Init");
    }

    /* Initialize the AFE API */
    if (ADI_AFE_SUCCESS != adi_AFE_Init(&hAfeDevice)) 
    {
        FAIL("adi_AFE_Init");
    }

    /* Set the correct values for RTIA and RCAL.                            */
    /* adi_AFE_Init will set RTIA and RCAL to default values of 7.5k and 1k */
    /* respectively. As the resistor values are different in this example,  */
    /* it is necessary to reflect this difference before running the        */
    /* calibration routines.                                                */
    if (ADI_AFE_SUCCESS != adi_AFE_SetRtia(hAfeDevice, RTIA))
    {
        FAIL("adi_AFE_SetRtia");
    }
    if (ADI_AFE_SUCCESS != adi_AFE_SetRcal(hAfeDevice, RCAL))
    {
        FAIL("adi_AFE_SetRcal");
    }

    /* AFE power up */
    if (ADI_AFE_SUCCESS != adi_AFE_PowerUp(hAfeDevice)) 
    {
        FAIL("adi_AFE_PowerUp");
    }

    /* Excitation Channel Power-Up */
    if (ADI_AFE_SUCCESS != adi_AFE_ExciteChanPowerUp(hAfeDevice)) 
    {
        FAIL("adi_AFE_ExciteChanPowerUp");
    }

    /* TIA Channel Calibration */
    /* This is custom calibration done for the configured used in this example */
    if (ADI_AFE_SUCCESS != adi_AFE_CustomTiaChanCal(hAfeDevice)) 
    {
        FAIL("adi_AFE_CustomTiaChanCal");
    }

    /* Excitation Channel Calibration, no attenuation                                     */
    /* Uses the standard calibration function from SDK, but this may change in the future */
    /* and be replaced with a custom function, similar to the TIA channel calibration.    */
    if (ADI_AFE_SUCCESS != adi_AFE_ExciteChanCalNoAtten(hAfeDevice)) 
    {
        FAIL("adi_AFE_ExciteChanCalNoAtten");
    }

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

    /* Set DAC Level 1 */
    seq_afe_unipolarcurrentmeasurement[4]   = SEQ_MMR_WRITE(REG_AFE_AFE_WG_DAC_CODE, DACL1);
    /* Set DAC Level 2 */
    seq_afe_unipolarcurrentmeasurement[20]  = SEQ_MMR_WRITE(REG_AFE_AFE_WG_DAC_CODE, DACL2);

    /* Set durations in ACLK periods */
    seq_afe_unipolarcurrentmeasurement[16]  = dur1 * 16;
    seq_afe_unipolarcurrentmeasurement[19]  = dur2 * 16;
    seq_afe_unipolarcurrentmeasurement[21]  = dur3 * 16;
    seq_afe_unipolarcurrentmeasurement[24]  = dur4 * 16;
    
    /* Switch matrix parameterization */
    seq_afe_unipolarcurrentmeasurement[8]   = SEQ_MMR_WRITE(REG_AFE_AFE_SW_FULL_CFG_MSB, ADI_SWM_MSB_MEASUREMENT);
    seq_afe_unipolarcurrentmeasurement[9]   = SEQ_MMR_WRITE(REG_AFE_AFE_SW_FULL_CFG_LSB, ADI_SWM_LSB_MEASUREMENT);
    
    seq_afe_unipolarcurrentmeasurement[17]  = SEQ_MMR_WRITE(REG_AFE_AFE_SW_FULL_CFG_LSB, (ADI_SWM_MSB_MEASUREMENT |
                                                                                          (SHUNTREQD << BITP_AFE_AFE_SW_FULL_CFG_LSB_IVS)));

    seq_afe_unipolarcurrentmeasurement[22]  = SEQ_MMR_WRITE(REG_AFE_AFE_SW_FULL_CFG_LSB, ADI_SWM_LSB_MEASUREMENT);
    
    /* Set the Rx DMA buffer sizes */
    if (ADI_AFE_SUCCESS != adi_AFE_SetDmaRxBufferMaxSize(hAfeDevice, DMA_BUFFER_SIZE, DMA_BUFFER_SIZE))
    {
        FAIL("adi_AFE_SetDmaRxBufferMaxSize");
    }
    
    /* Register Rx DMA Callback */
    if (ADI_AFE_SUCCESS != adi_AFE_RegisterCallbackOnReceiveDMA(hAfeDevice, RxDmaCB, 0))
    {
        FAIL("adi_AFE_RegisterCallbackOnReceiveDMA");
    }
        
    /* Recalculate CRC in software for the amperometric measurement */
    adi_AFE_EnableSoftwareCRC(hAfeDevice, true);

    /* Perform Uni-Polar Current Measurement */
    if (ADI_AFE_SUCCESS != adi_AFE_RunSequence(hAfeDevice, seq_afe_unipolarcurrentmeasurement, (uint16_t *)dmaBuffer, SAMPLE_COUNT)) 
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
    
    /* Uninitialize the UART */
    uart_UnInit();

    PASS();
}

/* Modified TIA channel calibration routine for unipolar current measurement */
ADI_AFE_RESULT_TYPE adi_AFE_CustomTiaChanCal(ADI_AFE_DEV_HANDLE const hDevice) {
    ADI_AFE_RESULT_TYPE     result = ADI_AFE_SUCCESS;
    int32_t                 offset_code;
    uint32_t                gain_code;
    int32_t                 channel_offset;
    int32_t                 yp;
    int64_t                 tmp64;
    
    adi_AFE_EnableSoftwareCRC(hDevice, true);
    
    /* Start the calibration routine with default gain and offset values */
    adi_AFE_WriteCalibrationRegister(hDevice, ADI_AFE_CAL_REG_ADC_GAIN_TIA, (uint32_t)0x4000);
    adi_AFE_WriteCalibrationRegister(hDevice, ADI_AFE_CAL_REG_ADC_OFFSET_TIA, (uint32_t)0x000);

    /* Step 1: measure 0V (P = Vbias, N = Vbias) */
    result = adi_AFE_RunSequence(hDevice, seq_afe_unipolarcurrent_tiachancal1, NULL, 0);

    if (ADI_AFE_SUCCESS == result) {

        /* Read measurement result, convert to signed format and invert the sign */
        offset_code = 32768 - pADI_AFE->AFE_SUPPLY_LPF_RESULT;
        /* Align to 0.25LSB resolution */
        offset_code = offset_code << 2;

        /* Write offset to the calibration register */
        adi_AFE_WriteCalibrationRegister(hDevice, ADI_AFE_CAL_REG_ADC_OFFSET_TIA, offset_code);
       
        /* Step 2: measure 0.7V (P = Vref, N = Vbias) */
        result = adi_AFE_RunSequence(hDevice, seq_afe_unipolarcurrent_tiachancal2, NULL, 0);
    }

    if (ADI_AFE_SUCCESS == result) {

        /* Read measurement result and convert to a signed number */
        gain_code = pADI_AFE->AFE_SUPPLY_LPF_RESULT - 32768;
        /* Gain code calculation: (0.7V * 1.5 / 3.6V) * 65535 */
        /* The gain code is stored with 14 fractional bits, hence the 16384 */
        gain_code = (16384 * 19114) / gain_code;

        /* Write gain to the calibration register */
        adi_AFE_WriteCalibrationRegister(hDevice, ADI_AFE_CAL_REG_ADC_GAIN_TIA, gain_code);

        /* Step 3: measure TIA channel offset (P = IVAO, N = IVAIM) */
        result = adi_AFE_RunSequence(hDevice, seq_afe_unipolarcurrent_tiachancal3, NULL, 0);
    }

    if (ADI_AFE_SUCCESS == result) {

        /* Read measurement result and convert to signed format */
        yp = pADI_AFE->AFE_SUPPLY_LPF_RESULT - 32768;
            
        /* Using 64-bit signed numbers for partial results simplifies the task */
        tmp64 = (ADC_CODE_HIGH - 32768) - (int64_t)yp;
       
        /* Increase resolution to account for:  */
        /* - 14 fractional bits of the gain     */
        /* - 0.25 LSB resolution of the offset  */
        tmp64 = tmp64 << 16;
        
        /* Divide by the previously calculated gain correction factor and round */
        tmp64 = (tmp64 << 1) / gain_code;
        tmp64 = tmp64 + 1;
        tmp64 = tmp64 >> 1;        
        
        channel_offset = tmp64 + offset_code;

        /* Write offset to the calibration register */
        adi_AFE_WriteCalibrationRegister(hDevice, ADI_AFE_CAL_REG_ADC_OFFSET_TIA, channel_offset);

    }
    
    adi_AFE_EnableSoftwareCRC(hDevice, false);
        
    return result;
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
    char                    msg[MSG_MAXLEN];
    uint32_t                i;
    uint16_t                *ppBuffer = (uint16_t*)pBuffer;

    /* Check if there are samples to be sent */
    if (length)
    {
        for (i = 0; i < length; i++)
        {
            sprintf(msg, "%u\r\n", *ppBuffer++);
            PRINT(msg);
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