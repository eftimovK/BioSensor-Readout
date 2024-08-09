/*********************************************************************************

Copyright (c) 2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*****************************************************************************
 * @file:    BiosensorScript.c
 * @brief:   Firmware for biosensor measurement.
 *****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "test_common.h"

#include "afe.h"
#include "afe_lib.h"
#include "uart.h"
#include "command_ID.h"

/* Macro to enable the returning of AFE data using the UART */
/*      1 = return AFE data on UART                         */
/*      0 = return AFE data on SW (Std Output)              */
#define USE_UART_FOR_DATA           (1)

/* Helper macro for printing strings to UART or Std. Output */
#define PRINT(s)                    test_print(s)

// TODO: reorganize all necessary macros and sequences in a config/header file

/****************************************************************************/
/*  <----------- DURL1 -----------><----------- DURL2 ----------->          */
/*                  <-- DURIVS1 --><-- DURIVS2 -->                          */
/*                                 _______________________________ <--- VL2 */
/*                                |                                         */
/*  ______________________________|                                <--- VL1 */
/*                                                                          */
/*  <---- dur1 ----><--- dur2 ---><---- dur3 ----><---- dur4 ---->          */
/****************************************************************************/

/* Const voltage excitation; adapted from the step voltage sequence from the amp. measurement example

    Configurable parameters from GUI: 
        Voltage Level 2 [mV]

    Configurable parameters from macros (in this script):
        Voltage Level 1 (VL1)   [mV]
        Duration DURL1          [us]
        Duration DURL2          [us]
        Duration DURIVS1        [us]
        Duration DURIVS2        [us]

    Important hard-coded settings:
        ADC output (DATA_FIFO_SOURCE_SEL = 0b01)
        4-wire electrode pins: (see switch matrix command)
            CE   == AFE7
            REF+ == AFE6
            REF- == AFE2
            WE   == AFE3

*/
uint32_t seq_stepVoltage[] = {
    0x00150065,   /*  0 - Safety Word, Command Count = 15, CRC = 0x1C                                       */  // adjust command count accordingly!
    0x84003818,   /*  1 - AFE_FIFO_CFG: DATA_FIFO_SOURCE_SEL = 0b01 (ADC)                                   */
    0x8A000030,   /*  2 - AFE_WG_CFG: TYPE_SEL = 0b00                                                       */
    0x88000F00,   /*  3 - AFE_DAC_CFG: DAC_ATTEN_EN = 0 (disable DAC attenuator)                            */
    0xAA000800,   /*  4 - AFE_WG_DAC_CODE: DAC_CODE = 0x800 (DAC Level 1 placeholder, user programmable)    */
    0xA0000002,   /*  5 - AFE_ADC_CFG: MUX_SEL = 0b00010, GAIN_OFFS_SEL = 0b00 (TIA)                        */
    0xA2000000,   /*  6 - AFE_SUPPLY_LPF_CFG: BYPASS_SUPPLY_LPF = 0 (do not bypass)                         */
    // 0x86003456,   /*  7_(4-wire) - AFE_SW_CFG: DMUX_STATE = 6 PMUX_STATE = 5 NMUX_STATE = 4 TMUX_STATE = 3  */
    0x86003366,   /*  7_(2-wire) - DMUX_STATE = 6, PMUX_STATE = 6, NMUX_STATE = 3, TMUX_STATE = 3           */  // pins should be same as for IVS switch below
    0x0001A900,   /*  8 - Wait: 6.8ms (based on load RC = 6.8kOhm * 1uF)                                    */
    0x80024EF0,   /*  9 - AFE_CFG: WG_EN = 1                                                                */
    0x00000C80,   /* 10 - Wait: 200us                                                                       */
    0x80034FF0,   /* 11 - AFE_CFG: ADC_CONV_EN = 1, SUPPLY_LPF_EN = 1                                       */
    0x00090880,   /* 12 - Wait: 37ms  for LPF settling                                                      */
    0x00000000,   /* 13 - Wait: (DAC Level 1 duration - IVS duration 1) (placeholder, user programmable)    */
    0x86013366,   /* 14 - IVS_STATE = 1 (close IVS switch, user programmable)                               */
    0x00000000,   /* 15 - Wait: IVS duration 1 (placeholder, user programmable)                             */
    0xAA000800,   /* 16 - AFE_WG_DAC_CODE: DAC_CODE = 0x800 (DAC Level 2 placeholder, user programmable)    */
    0x00000000,   /* 17 - Wait: IVS duration 2 (placeholder, user programmable)                             */
    0x86003366,   /* 18 - IVS_STATE = 0 (open IVS switch)                                                   */
    0x00000000,   /* 19 - Wait: (DAC Level 2 duration - IVS duration 2) (placeholder, user programmable)    */
//    0x80020EF0,   /* 20 - AFE_CFG: WAVEGEN_EN = 0, ADC_CONV_EN = 0, SUPPLY_LPF_EN = 0                     */   // comment this command out if CONTINUOUS_MEASUREMENT=1 and use the one below
    0x00000000,   /* 20_CONTINUOUS - placeholder for the 20th command (no need to change command count above) - Wait: 0 us             */
    0x82000002,   /* 21 - AFE_SEQ_CFG: SEQ_EN = 0                                                           */
};

/* 
    Configurable parameters from macros (in this script)
*/
/* DC Level 1 voltage in mV (range: -0.8V to 0.8V) */
#define VL1                         (0)
/* The duration (in us) of DC Level 1 voltage */
#define DURL1                       ((uint32_t)(100))   // set to match DURIVS1
/* The duration (in us) of DC Level 2 voltage */
#define DURL2                       ((uint32_t)(0))     // set to 0 since we are running in continuous mode

/* DO NOT EDIT: DAC LSB size in mV, before attenuator (1.6V / (2^12 - 1)) */
#define DAC_LSB_SIZE                (0.39072)
/* DO NOT EDIT: DC Level 1 in DAC codes */
#define DACL1                       ((uint32_t)(((float)VL1 / (float)DAC_LSB_SIZE) + 0x800))

/* The duration (in us) which the IVS switch should remain closed (to shunt */
/* switching current) before changing the DC level.                         */
#define DURIVS1                     ((uint32_t)(100))
/* The duration (in us) which the IVS switch should remain closed (to shunt */
/* switching current) after changing the DC level.                          */
#define DURIVS2                     ((uint32_t)(100))
/* Is shunting of the switching current required? Required: 1, Not required: 0 */
#define SHUNTREQD                   (1)

/* 
    Configurable variables from GUI
*/
/* DC Level 2 voltage in mV (range: -0.8V to 0.8V) */
int32_t    voltageLevel2 = 100;
/* DC Level 2 in DAC codes */
uint32_t    voltageLevel2_DAC = ((uint32_t)(((float)0 / (float)DAC_LSB_SIZE) + 0x800));


/****************************************************************************/
/*  <-------------------------DURCV---------------------------->            */
/*                ______                    ______                <--- VL2  */
/*               /      \                  /      \                         */
/*              /        \                /        \                        */
/*             /          \              /          \                       */
/*            /            \            /            \                      */
/*           /              \          /              \                     */
/*  ________/                \________/                \________  <--- VL1  */
/*                                                                          */
/*  <--D1--><-S1-><-D2-><-S2->                                              */
/* 
    D1  = TRAP_DELAY1
    D2  = TRAP_DELAY2
    S1  = TRAP_SLOPE_1
    S2  = TRAP_SLOPE_2
    VL1 = TRAP_DC_LEVEL_1
    VL2 = TRAP_DC_LEVEL_2
*/
/****************************************************************************/

// NOTE: the delay and slope times are calculated in units of DAC rate (320kHz)
//       --> this means, to do a 1sec. slope time, we should convert 1s*320kHz =320000 =4E200 to hexadecimal code
// 
// Things to try out for debugging: (+ means worked well)
// + *use 0x9200FFFF instead of 0x920FFFFF for the Slope time (because max. is 1.6s)
// + *continuous measurement --> leave cmd 15 out! do not disable WAVEGEN and ADC
//  
//  *disabling DAC atten. after 2nd cmd, by 0x88000F00, /*  3 - AFE_DAC_CFG: DAC_ATTEN_EN = 0 (disable DAC attenuator)                            */
//  *right before enabling the generator, 0xA2000000, /*  13 - AFE_SUPPLY_LPF_CFG: BYPASS_SUPPLY_LPF = 0 (do not bypass)                         */
//  *try with the script from the forum (https://ez.analog.com/microcontrollers/precision-microcontrollers/w/documents/2338/continuously-amperometric-measurement-example)

/* Cyclic voltammetry excitation; adapted from the trapezoid sequence in afe_sequences.h

    Configurable parameters from GUI: 
        Voltage Level 1 [mV]
        Voltage Level 2 [mV]
        Slope time      [us] (refers to both TRAP_SLOPE 1 and 2)
        
    Configurable parameters from macros (in this script):
        TRAP_DELAY_1    [us]
        TRAP_DELAY_2    [us]
        Duration        [us]

    Important hard-coded settings:
        ADC output (DATA_FIFO_SOURCE_SEL = 0b01)
        4-wire electrode pins: (see switch matrix command)
            CE   == AFE7
            REF+ == AFE6
            REF- == AFE2
            WE   == AFE3

*/
uint32_t seq_cv[] = {
    0x001000B4, /*  0 - Safety Word, Command Count = 16, CRC = 0xB4                                                                                 */
    0x84003818, /*  1 - AFE_FIFO_CFG: DATA_FIFO_DMA_REQ_EN = 1 DATA_FIFO_SOURCE_SEL = 0b01 (ADC) CMD_FIFO_DMA_REQ_EN = 1 CMD_FIFO_EN = 1 DATA_FIFO_EN = 1 */
    0x8A000037, /*  2_edit AFE_WG_CFG: WG_TRAP_RESET=1 (0x37 instead of 0x36), TYPE_SEL = 0b11                                                           */
    0x8C000800, /*  3 - AFE_WG_DCLEVEL_1: TRAP_DC_LEVEL_1 = 0x800 (placeholder, user programmable)                                           */
    0x8E000C80, /*  4 - AFE_WG_DCLEVEL_2: TRAP_DC_LEVEL_2 = 0xC80 (placeholder, user programmable)                                           */
    0x90000140, /*  5_edit: 1ms=0x90000140 - AFE_WG_DELAY_1: TRAP_DELAY_1 = 0x7F8D (placeholder, user programmable)                                      */
    0x9207D000, /*  6_edit: 1.6s=0x7D000   - AFE_WG_SLOPE_1: TRAP_SLOPE_1 = 0x0CC1           (placeholder, user programmable)                                      */
    0x0000063E, /*  7 - Wait: 100 us                                                                                                                */
                /* This wait is needed because all the commands before this one are MMR writes. They are executed in a single clock cycle, and the  */
                /* DMA controller cannot keep up, resulting in the sequencer being starved (triggering a command FIFO underflow). 100us is          */
                /* sufficiently large and could be adjusted, depending on system load.                                                              */
    0x94000140, /*  8_edit: 1ms=0x94000140 - AFE_WG_DELAY_2: TRAP_DELAY_2 = 0x7F8D (placeholder, user programmable)                                 */
    0x9607D000, /*  9_edit: 1.6s - AFE_WG_SLOPE_2: TRAP_SLOPE_2 = 0x0CC1           (placeholder, user programmable)                                 */
    // 0x86003456, /*  10_(4-wire) - AFE_SW_CFG: DMUX_STATE = 6 PMUX_STATE = 5 NMUX_STATE = 4 TMUX_STATE = 3                                            */
    0x86003366,   /*  10_(2-wire) - DMUX_STATE = 6, PMUX_STATE = 6, NMUX_STATE = 3, TMUX_STATE = 3                    */
    0xA0000002, /* 11 - AFE_ADC_CFG: MUX_SEL = 0b10 GAIN_OFFS_SEL = 0b00 (TIA)                                                                      */
    0x0000063E, /* 12 - Wait: 100 us                                                                                                                */
    0x80034FF0, /* 13 - AFE_CFG: WAVEGEN_EN = 1 ADC_CONV_EN = 1 SUPPLY_LPF_EN = 1                                                                   */
    // set waiting according to the cycles of the signal; CYCLES = WAIT_DURATION / (SLOPE_1+SLOPE_2)
    0x0000063E, /* 14_edit - Wait: (CV duration) 3.2s=0x030D4000; 3.2*10=0x1E848000; 1s=0x00F42400; 100us=0x0000063E (placeholder, user programmable)                           */
    // 0x80020EF0, /* 15 - AFE_CFG: WAVEGEN_EN = 0 ADC_CONV_EN = 0 SUPPLY_LPF_EN = 0                                                                */  // comment this command out if CONTINUOUS_MEASUREMENT=1 and use the one below
    0x00000000, /* 15_CONTINUOUS: placeholder for the 15th command (no need to change command count above) - Wait: 0 us                             */
    0x82000002  /* 16 - AFE_SEQ_CFG: SEQ_EN = 0                                                                                                     */
};

/* 
    Configurable parameters from macros (in this script)
*/
/* The duration (in us) of DC Level 1 voltage (MAX. 1.6 seconds !) */
#define TRAP_DELAY_1                       ((uint32_t)(0))    // set to 0 for a triangular instead of trapezoidal signal
/* The duration (in us) of DC Level 2 voltage (MAX. 1.6 seconds !) */
#define TRAP_DELAY_2                       ((uint32_t)(0))    // set to 0 for a triangular instead of trapezoidal signal
/* The duration (in us) of the CV excitation signal */
#define DURCV                              ((uint32_t)(0))    // set to 0 since we are running in continuous mode

/* 
    Configurable variables from GUI
*/
/* DC Level 1 voltage in mV (range: -0.8V to 0.8V) */
int32_t    voltageLevelCV1 = 0;
/* DC Level 1 voltage in mV (range: -0.8V to 0.8V) */
int32_t    voltageLevelCV2 = 100;
/* Slope time between voltage levels in us (MAX. 1.6 seconds !) */
int32_t    slopeTimeCV = 1000000;
/* DC Level 1 in DAC codes */
uint32_t   voltageLevelCV1_DAC = ((uint32_t)(((float)0 / (float)DAC_LSB_SIZE) + 0x800));
/* DC Level 1 in DAC codes */
uint32_t   voltageLevelCV2_DAC = ((uint32_t)(((float)0 / (float)DAC_LSB_SIZE) + 0x800));

/* AC sinusoidal excitation (used for impedance spectroscopy); adapted from the impedance measurement example

    Configurable parameters from GUI: 
        Frequency       [Hz]
        Voltage         [mV]
        
    Configurable parameters from macros (in this script):
        Duration        [us]

    Important hard-coded settings:
        ADC output (DATA_FIFO_SOURCE_SEL = 0b01)
        4-wire electrode pins: (see switch matrix command)
            CE   == AFE4
            REF+ == AFE4
            REF- == AFE5
            WE   == AFE5

*/
uint32_t seq_ac[] = {
    0x00130043,   /* 0 - Safety word: bits 31:16 = command count, bits 7:0 = CRC */
    0x84005818,   /* 1 - AFE_FIFO_CFG: DATA_FIFO_SOURCE_SEL = 10 (DFT) */
    0x8A000034,   /* 2 - AFE_WG_CFG: TYPE_SEL = 10 (sinusoid) */
    0x98000000,   /* 3 - AFE_WG_CFG: SINE_FCW = 0 (placeholder, user programmable) */
    0x9E000000,   /* 4 - AFE_WG_AMPLITUDE: SINE_AMPLITUDE = 0 (placeholder, user programmable) */
    0x88000F01,   /* 5 - AFE_DAC_CFG: DAC_ATTEN_EN = 1 */
    0xA0000002,   /* 6 - AFE_ADC_CFG: MUX_SEL = 00010, GAIN_OFFS_SEL = 00 */
    /* RCAL (needed for calibration) */
    0x86008811,   /* 7 - DMUX_STATE = 1, PMUX_STATE = 1, NMUX_STATE = 8, TMUX_STATE = 8 */
    0x00000640,   /* 8 - Wait 100us */
    0x80034EF0,   /* 9 - AFE_CFG: WAVEGEN_EN = 1, SUPPLY_LPF_EN = 1 */
    0x00090880,   /* 10 - Wait: 37ms  for LPF settling                                                      */
    0x8002CFF0,   /* 11 - AFE_CFG: ADC_CONV_EN = 1, DFT_EN = 1 */
    0x00032340,   /* 12 - Wait 13ms */
    0x80024EF0,   /* 13 - AFE_CFG: ADC_CONV_EN = 0, DFT_EN = 0 */
    /* AFE4 - AFE5 */
    0x86003366,   /* 14 - DMUX_STATE = 6, PMUX_STATE = 6, NMUX_STATE = 3, TMUX_STATE = 3 */
    0x00000640,   /* 15 - Wait 100us */
    0x8002CFF0,   /* 16 - AFE_CFG: ADC_CONV_EN = 1, DFT_EN = 1 */
    0x00032340,   /* 17 - Wait: (AC signal duration) (placeholder, user programmable)    */
    0x80020EF0,   /* 18 - AFE_CFG: WAVEGEN_EN = 0, ADC_CONV_EN = 0, DFT_EN = 0, SUPPLY_LPF_EN = 0 */

    0x82000002,   /* 19 - AFE_SEQ_CFG: SEQ_EN = 0 */
};

/* 
    Configurable parameters from macros (in this script)
*/
/* The duration (in us) of the AC excitation signal */
#define DURAC                              ((uint32_t)(2000000)) // TODO: set minimum of AC duration correctly 

/* 
    Configurable variables from GUI
*/
/* AC peak voltage level in uV (range: 10 uV to 20 000 uV) */
int32_t    voltageLevelAC = 1000;
/* AC signal frequency in Hz (range: 100 Hz to 80 kHz) */
int32_t    signalFrequencyAC = 1000;

/* Sine amplitude in DAC codes */
uint32_t  voltageLevelAC_DAC = ((uint32_t)((float)(0 * 40) / 1000 / DAC_LSB_SIZE + 0.5)); // TODO: explain this equation
/* FCW = FREQ * 2^26 / 16e6 */
uint32_t signalFrequencyAC_SEQ = ((uint32_t)(((uint64_t)0 << 26) / 16000000 + 0.5));

/* Number of frequencies of impedance spectroscopy (up to MAX_NUM_EIS_FREQ) */
int32_t    numFrequencyEIS = 1;

/* 
    Sequence for turning off the wave generator, adc conversion and LPF.
    Called when a measurement in continuous mode is stopped.
*/
uint32_t seq_turnOff[] = {
    0x00030065,   /*  0 - Safety Word, Command Count = 15, CRC = 0x1C                                       */  // adjust command count accordingly!
    0x80020EF0,   /* 1 - AFE_CFG: WAVEGEN_EN = 0, ADC_CONV_EN = 0, SUPPLY_LPF_EN = 0                        */
    0x00000C80,   /* 2 - Wait: 200us                                                                        */
    0x82000002,   /* 3 - AFE_SEQ_CFG: SEQ_EN = 0                                                            */
};

uint8_t restBytes = 0;

/* RCAL value, in ohms                                              */
/* Default value on ADuCM350 Switch Mux Config Board Rev.0 is 1k    */
#define RCAL                        (1000)
/* RTIA value, in ohms                                              */
/* Default value on ADuCM350 Switch Mux Config Board Rev.0 is 7.5k  */
#define RTIA                        (7500)

/* Set decimation (downsampling) factor and whether to run dma transfers indefinitely */
#define DECIMATION                  (uint8_t)(160)
#define CONTINUOUS_MEASUREMENT      (bool_t)(1)     // sequence is stopped by sending a stop cmd (from the GUI)

// TODO: remove hard-coded SAMPLE_COUNT, but set it 
//      1) based on sequence duration, when not running in continuous mode
//      2) > 0 is sufficient when running in continuous mode 
/* Number of samples to be transferred by DMA, based on the duration of the sequence. */
#define SAMPLE_COUNT                (uint32_t)(8000) // ADC_SPS / DECIMATION * DURATION = max_SAMPLE_COUNT; (ADC_SPS = 160kSPS)

/* Maximum number of frequencies (data points) in a EIS measurement. */
#define MAX_NUM_EIS_FREQ            (uint32_t)(200)

/* The number of results expected from the DFT (== number of complex results times two) */
#define DFT_RESULTS_COUNT           (4)

/* Size limit for each DMA transfer (max 1024) */
#define DMA_BUFFER_SIZE             (1024u)  // should be a multiple of DECIMATION! else, it gets rounded to the nearest multiple

/* Maximum printed message length. Used for printing only. */
#define MSG_MAXLEN                  (50)

#pragma location="nonvolatile_ram"  // RAM0 is non-volatile (see file ADuCM350BBCZ_CP.icf)
uint16_t        dmaBuffer[DMA_BUFFER_SIZE * 2];

#pragma location="volatile_ram"     // store in RAM1
int32_t        frequenciesEIS[MAX_NUM_EIS_FREQ] = {1000};

// TODO: program sequence for the testing event
/* Amperometric measuring sequence for the creatinine samples for the testing event */
uint32_t seq_sampleMeasurement[] = {
    0x00150065,   /*  0 - Safety Word, Command Count = 15, CRC = 0x1C                                       */  // adjust command count accordingly!
    0x84003818,   /* 1 - AFE_FIFO_CFG: DATA_FIFO_SOURCE_SEL = 0b01 (ADC)                                    */
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
//    0x80020EF0,   /* 21 - AFE_CFG: WAVEGEN_EN = 0, ADC_CONV_EN = 0, SUPPLY_LPF_EN = 0                       */   // comment out, adjust command count above, and set CONTINUOUS_MEASUREMENT=1 to run indefinitely
    0x82000002,   /* 22 - AFE_SEQ_CFG: SEQ_EN = 0                                                           */
};

/* Variables and functions needed for data output through UART */
ADI_UART_HANDLE     hUartDevice     = NULL;
ADI_AFE_DEV_HANDLE  hAfeDevice;

/* Function prototypes */
void                    test_print                  (char *pBuffer);
ADI_UART_RESULT_TYPE    uart_setup                  (void);
ADI_UART_RESULT_TYPE    uart_UnInit                 (void);
void                    afe_setup                   (void);
void                    afe_ac_setup                (void);
void                    seq_stepVoltage_setup       (void);
void                    seq_cv_setup                (void);
void                    seq_ac_setup                (void);
void                    afe_postMeasurement         (void);
extern int32_t          adi_initpinmux              (void);
void        RxDmaCB         (void *hAfeDevice, 
                             uint32_t length, 
                             void *pBuffer);

int main(void) 
{
    bool_t              stopProgram = 0;
    int16_t             dft_results[DFT_RESULTS_COUNT];
    
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

    
    /* Setup the UART for transferring measurement data out */
    if (ADI_UART_SUCCESS != uart_setup())
    {
        FAIL("uart_setup");
    }

    // TODO:
    /* Set initial values for global variables explicitly */
    
    while(!stopProgram)
    {
        /* Read one character */
        int16_t rxSize = 1;
        int8_t cmdRx[1];   // buffer for the command received

        ADI_UART_RESULT_TYPE uartRxResult = adi_UART_BufRx(hUartDevice, cmdRx, &rxSize);
        if (ADI_UART_SUCCESS != uartRxResult)
        {
            test_Fail("adi_UART_BufRx() failed");
        }

        if (cmdRx[0] == CMD_START_CONST)
        {

            /* Setup AFE before running the sequence */
            afe_setup();

            /* Set sequence-specific programmable parameters */
            seq_stepVoltage_setup();
            
            /* Recalculate CRC in software for the amperometric measurement */
            adi_AFE_EnableSoftwareCRC(hAfeDevice, true);

            /* Perform the Amperometric measurement(s) */
            if (ADI_AFE_SUCCESS != adi_AFE_RunSequence(hAfeDevice, seq_stepVoltage, (uint16_t *) dmaBuffer, SAMPLE_COUNT)) 
            {
                FAIL("adi_AFE_RunSequence - step voltage excitation");   
            }

            /* Turn off AFE_CFG: WAVEGEN_EN = 0, ADC_CONV_EN = 0, SUPPLY_LPF_EN = 0 (in case of continuous measurement) by running a short turn-off sequence */
            if (CONTINUOUS_MEASUREMENT)
            {
                if (ADI_AFE_SUCCESS != adi_AFE_RunSequence(hAfeDevice, seq_turnOff, (uint16_t *) dmaBuffer, 0))
                {
                    FAIL("adi_AFE_RunSequence - turn off");
                }
            }

            /* Restore to using default CRC stored with the sequence */
            adi_AFE_EnableSoftwareCRC(hAfeDevice, false);
            
            /* Post measurement AFE deregistering */
            afe_postMeasurement();
        }
        else if (cmdRx[0] == CMD_START_CV)
        {

            /* Setup AFE before running the sequence */
            afe_setup();

            /* Set sequence-specific programmable parameters */
            seq_cv_setup();
            
            /* Recalculate CRC in software for the amperometric measurement */
            adi_AFE_EnableSoftwareCRC(hAfeDevice, true);

            /* Perform the Amperometric measurement(s) */
            if (ADI_AFE_SUCCESS != adi_AFE_RunSequence(hAfeDevice, seq_cv, (uint16_t *) dmaBuffer, SAMPLE_COUNT)) 
            {
                FAIL("adi_AFE_RunSequence - cv excitation");   
            }

            /* Turn off AFE_CFG: WAVEGEN_EN = 0, ADC_CONV_EN = 0, SUPPLY_LPF_EN = 0 (in case of continuous measurement) by running a short turn-off sequence */
            if (CONTINUOUS_MEASUREMENT)
            {
                if (ADI_AFE_SUCCESS != adi_AFE_RunSequence(hAfeDevice, seq_turnOff, (uint16_t *) dmaBuffer, 0))
                {
                    FAIL("adi_AFE_RunSequence - turn off");
                }
            }

            /* Restore to using default CRC stored with the sequence */
            adi_AFE_EnableSoftwareCRC(hAfeDevice, false);
            
            /* Post measurement AFE deregistering */
            afe_postMeasurement();
        }
        else if (cmdRx[0] == CMD_START_EIS)
        {
            /* Setup AFE before running the sequence */
            afe_ac_setup();    // the AC sequence needs a different AFE setup

            /* Recalculate CRC in software for the amperometric measurement */
            adi_AFE_EnableSoftwareCRC(hAfeDevice, true);

            for (uint32_t i = 0; i < numFrequencyEIS; i++)
            {
                signalFrequencyAC = frequenciesEIS[i];

                /* Set sequence-specific programmable parameters */
                seq_ac_setup(); // update frequency in the sequence

                /* Perform the Impedance measurement */
                if (ADI_AFE_SUCCESS != adi_AFE_RunSequence(hAfeDevice, seq_ac, (uint16_t *)dft_results, DFT_RESULTS_COUNT)) 
                {
                    FAIL("adi_AFE_RunSequence - ac excitation");   
                }

                // Send data to UART (raw DFT data which will be processed in the GUI)
                // dft_results[0] == RCAL real part
                // dft_results[1] == RCAL imaginary part
                char                    temp[MSG_MAXLEN];
                sprintf(temp, "%d:%d:%d:%d:%d:%d\r\n", DATA_MEAS_EIS, signalFrequencyAC, dft_results[0], dft_results[1], dft_results[2], dft_results[3] );
                PRINT(temp);
            }

            /* Restore to using default CRC stored with the sequence */
            adi_AFE_EnableSoftwareCRC(hAfeDevice, false);
            
            /* Post measurement AFE deregistering */
            afe_postMeasurement();
        }
        else if (cmdRx[0] == CMD_STOP)
        {
            stopProgram = 1;
        }
        else if (cmdRx[0] == CMD_ABORT)
        {
            stopProgram = 1;
        }
        else if (cmdRx[0] == CMD_START_CONFIG)
        {
            int8_t temp;
            bool_t configSuccess = 1;
            bool_t stopConfig    = 0;            

            while(!stopConfig)    // breaks if the stop config or an unknown cmd ID was sent
            {
                int16_t rxByteSize = 1;      // byte size of receive command
                ADI_UART_RESULT_TYPE uartRxResult = adi_UART_BufRx(hUartDevice, cmdRx, &rxByteSize);

                int32_t *targetParameter = NULL;  // Pointer to store address of the parameter to be updated
                temp = cmdRx[0];

                switch (cmdRx[0])
                {
                    case CMD_STOP_CONFIG:
                        stopConfig = true;
                        break;       // refers to the loop
                    case PARAM_VOLTAGE_STEP:
                        rxByteSize = 4;
                        targetParameter = &voltageLevel2;
                        break;
                    case PARAM_VOLTAGE1_CV:
                        rxByteSize = 4;
                        targetParameter = &voltageLevelCV1;
                        break;
                    case PARAM_VOLTAGE2_CV:
                        rxByteSize = 4;
                        targetParameter = &voltageLevelCV2;
                        break;
                    case PARAM_SLOPE_CV:
                        rxByteSize = 4;
                        targetParameter = &slopeTimeCV;
                        break;
                    case PARAM_VOLTAGE_AC:
                        rxByteSize = 4;
                        targetParameter = &voltageLevelAC;
                        break;
                    case PARAM_NUM_FREQ_EIS:
                        rxByteSize = 4;
                        targetParameter = &numFrequencyEIS;
                        break;
                    
                    default:    // tried to configure a parameter that is not known
                        configSuccess = 0;
                        stopConfig = true;
                        restBytes = 0;
                       
                        /* TODO: handle case where few more bytes were sent after the unknown cmd ID
                                 the problem is, that the while-condition adi_UART_GetNumRxBytes(hUartDevice)
                                 gets checked faster than the bytes get received, thus skipping the read() that is inside the function;
                                 a simple wait would do the trick; currently the printf() does this by slowing the execution...
                         */
                        printf("Unknown cmd ID : %d\n", temp);

                        /* ignore the bytes sent after an unrecognized cmd ID */
                        while ( adi_UART_GetNumRxBytes(hUartDevice) )
                        {
                                rxByteSize = 1;
                                int8_t dummyRx[1];
                                adi_UART_BufRx(hUartDevice, dummyRx, &rxByteSize);
                                restBytes += 1;
                        }
//                        printf("restBytes = %d\n", restBytes);
                        break;
                }

                if (!stopConfig)
                {
                    /* read and set the parameter value */

                    int8_t paramValueRx[4]; // buffer for the parameter value (set to max 4 bytes)

                    uartRxResult = adi_UART_BufRx(hUartDevice, paramValueRx, &rxByteSize);
                    if (ADI_UART_SUCCESS != uartRxResult)
                    {
                        FAIL("adi_UART_BufRx() for parameter value failed");
                    }

                    int32_t *paramValue_ptr = (int32_t *)&paramValueRx[0];
                    *targetParameter = *paramValue_ptr;

                    if (cmdRx[0] == PARAM_NUM_FREQ_EIS)
                    {
                        // prevent overshooting max. array index
                        if (numFrequencyEIS > MAX_NUM_EIS_FREQ)
                        { numFrequencyEIS = MAX_NUM_EIS_FREQ; } 

                        // read numFrequencyEIS number of int32, to fill in the frequency array for EIS
                        for (uint32_t i = 0; i < numFrequencyEIS; i++)
                        {
                            int8_t freqValueRx[4]; // buffer for the frequency value (set to max 4 bytes)
                            rxByteSize = 4;

                            uartRxResult = adi_UART_BufRx(hUartDevice, freqValueRx, &rxByteSize);
                            if (ADI_UART_SUCCESS != uartRxResult)
                            {
                                FAIL("adi_UART_BufRx() for a frequency value failed");
                            }

                            int32_t *freqValue_ptr = (int32_t *)&freqValueRx[0];
                            frequenciesEIS[i] = *freqValue_ptr;
                        }
                    }
                }

                /* send back success status */
                int16_t txSize = 1u;
                ADI_UART_RESULT_TYPE uartResult = adi_UART_BufTx(hUartDevice, &configSuccess, &txSize);
                if (ADI_UART_SUCCESS != uartResult)
                {
                    test_Fail("adi_UART_BufTx() failed");
                }
            }
        }

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
    char                    msg[MSG_MAXLEN];
    ADI_AFE_DEV_HANDLE      hAfeDeviceTemp = (ADI_AFE_DEV_HANDLE)hAfeDevice;

    /* Check if a cmd was sent to the port */
    if ( adi_UART_GetNumRxBytes(hUartDevice) )
    {
            /* Read one character to check if the STOP command was sent */
            int16_t rxSize = 1;
            int8_t cmdRx[1];
            ADI_UART_RESULT_TYPE uartRxResult = adi_UART_BufRx(hUartDevice, cmdRx, &rxSize);
            if (ADI_UART_SUCCESS != uartRxResult)
            {
                FAIL("adi_UART_BufRx() inside RxDmaCb failed");
            }

            if (cmdRx[0] == CMD_STOP)
            {
                /* Stop dma transfers */
                if (ADI_AFE_SUCCESS != adi_AFE_SetDmaTransfersZero(hAfeDeviceTemp) )
                {
                    FAIL("AFE: Set remaining dma transfers to zero");
                }
            }
    }

    /* Check if there are samples to be sent */
    if (length)
    {
        for (i = 1; i <= length; i++)
        {
            adc_sum = adc_sum + *ppBuffer++;    // add current ADC value
            decCounter++;
            
            if (decCounter == DECIMATION)
            {
                sprintf(msg, "%d:%u\r\n", DATA_MEAS_CONST, (uint16_t)(adc_sum / DECIMATION));
                PRINT(msg);
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

/* Initialize, configure, and enable/power-up the AFE */
void afe_setup (void)
{
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

    /* Set indefinite (continuous) measurement */
    if (ADI_AFE_SUCCESS != adi_AFE_SetIndefiniteMeasurement(hAfeDevice, CONTINUOUS_MEASUREMENT))
    {
        FAIL("Set indefiniteMeasurement");
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

    /*  DMA configuration */    
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

}

/* Initialize, configure, and enable/power-up the AFE for the AC measurement sequence */
void afe_ac_setup (void)
{
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
    
    /* Remove decimation (using DFT results) */
    if (ADI_AFE_SUCCESS != adi_AFE_SetDecFactor(hAfeDevice, 1))
    {
        FAIL("Set decFactor");
    }

    /* Set finite (non-continuous) measurement */
    if (ADI_AFE_SUCCESS != adi_AFE_SetIndefiniteMeasurement(hAfeDevice, false))
    {
        FAIL("Set indefiniteMeasurement");
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

    /* Excitation Channel Calibration (Attenuation Enabled) */
    if (ADI_AFE_SUCCESS != adi_AFE_ExciteChanCalAtten(hAfeDevice)) 
    {
        FAIL("adi_AFE_ExciteChanCalAtten");
    }

    /* No DMA configuration needed ? */    
    
    /* Unregister Rx DMA Callback */
    if (ADI_AFE_SUCCESS != adi_AFE_RegisterCallbackOnReceiveDMA(hAfeDevice, NULL, 0))
    {
        FAIL("adi_AFE_RegisterCallbackOnReceiveDMA (unregister)");
    }
}

/* Set sequence-specific programmable parameters */
void seq_stepVoltage_setup(void)
{
    /* Set the user programmable portions of the sequence */
    /* Set the duration values */
    uint32_t            dur1;
    uint32_t            dur2;
    uint32_t            dur3;
    uint32_t            dur4;

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
        seq_stepVoltage[13] = dur1 * 16;
        seq_stepVoltage[15] = dur2 * 16;
        seq_stepVoltage[17] = dur3 * 16;
        seq_stepVoltage[19] = dur4 * 16;
        

    // TODO: remove hard coded indexing in the sequence array; ideally use seq_stepVoltage[V_ind]=SEQ_MMR_WRITE(...) for modifying the voltage
    /* Set DAC Level 1 */
    seq_stepVoltage[4]  = SEQ_MMR_WRITE(REG_AFE_AFE_WG_DAC_CODE, DACL1);
    /* Set DAC Level 2 */
    voltageLevel2_DAC = ((uint32_t)(((float)voltageLevel2 / (float)DAC_LSB_SIZE) + 0x800)); // update value
    seq_stepVoltage[16] = SEQ_MMR_WRITE(REG_AFE_AFE_WG_DAC_CODE, voltageLevel2_DAC);
    
    if (!SHUNTREQD)
    {
        /* IVS switch remains open */
        seq_stepVoltage[14] &= 0xFFFEFFFF;
    }
}

/* Set sequence-specific programmable parameters */
void seq_cv_setup(void)
{
    /* Set the user programmable portions of the sequence */
    
    // TODO: remove hard coded indexing in the sequence array; e.g. use seq_cv[V_ind]=SEQ_MMR_WRITE(...) for modifying the voltage
            
    /* Set trapezoid delays in DAC periods (DAC update rate =320kHz) */
    seq_cv[5] = SEQ_MMR_WRITE(REG_AFE_AFE_WG_DELAY_1, (uint32_t)(TRAP_DELAY_1 * 0.32));
    seq_cv[8] = SEQ_MMR_WRITE(REG_AFE_AFE_WG_DELAY_2, (uint32_t)(TRAP_DELAY_2 * 0.32));

    /* Set trapezoid slope durations in DAC periods (DAC update rate =320kHz) */
    seq_cv[6] = SEQ_MMR_WRITE(REG_AFE_AFE_WG_SLOPE_1, (uint32_t)(slopeTimeCV * 0.32));
    seq_cv[9] = SEQ_MMR_WRITE(REG_AFE_AFE_WG_SLOPE_2, (uint32_t)(slopeTimeCV * 0.32));

    /* Set duration of signal in ACLK periods */
    seq_cv[14] = DURCV * 16;
    
    /* Set voltage levels */
    /* Set DAC Level 1 */
    voltageLevelCV1_DAC = ((uint32_t)(((float)voltageLevelCV1 / (float)DAC_LSB_SIZE) + 0x800)); // update value
    seq_cv[3]  = SEQ_MMR_WRITE(REG_AFE_AFE_WG_DCLEVEL_1, voltageLevelCV1_DAC);
    /* Set DAC Level 2 */
    voltageLevelCV2_DAC = ((uint32_t)(((float)voltageLevelCV2 / (float)DAC_LSB_SIZE) + 0x800)); // update value
    seq_cv[4] = SEQ_MMR_WRITE(REG_AFE_AFE_WG_DCLEVEL_2, voltageLevelCV2_DAC);
}

/* Set sequence-specific programmable parameters */
void seq_ac_setup(void)
{
    /* Set the user programmable portions of the sequence */
    
    // TODO: remove hard coded indexing in the sequence array; e.g. use seq_ac[V_ind]=SEQ_MMR_WRITE(...) for modifying the voltage
            
    /* Set duration of signal in ACLK periods */
    seq_ac[17] = DURAC * 16;

    /* Set voltage level (amplitude) */
    voltageLevelAC_DAC = ((uint16_t)((float)(voltageLevelAC * 40) / 1000 / DAC_LSB_SIZE + 0.5));    // divide by 1000 to get to mV
    seq_ac[4] = SEQ_MMR_WRITE(REG_AFE_AFE_WG_AMPLITUDE, voltageLevelAC_DAC);

    /* Set frequency of sinusoid     */
    signalFrequencyAC_SEQ = ((uint32_t)(((uint64_t)(signalFrequencyAC) << 26) / 16000000 + 0.5));
    seq_ac[3] = SEQ_MMR_WRITE(REG_AFE_AFE_WG_FCW, signalFrequencyAC_SEQ);
}

/* Power down, un-initialization and deregistering needed for AFE after the measurement */
void afe_postMeasurement(void)
{
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
}

/* Initialize, configure, and enable the UART */
ADI_UART_RESULT_TYPE uart_setup (void) {
    ADI_UART_RESULT_TYPE    result = ADI_UART_SUCCESS;
    
    ADI_UART_GENERIC_SETTINGS_TYPE  Settings;
    
    /* Open UART in blocking, non-intrrpt mode by supplying no internal buffs */
    if (ADI_UART_SUCCESS != (result = adi_UART_Init(ADI_UART_DEVID_0, &hUartDevice, NULL)))
    {
        return result;
    }
    
    Settings.BaudRate = ADI_UART_BAUD_115200;
    Settings.bBlockingMode = true;

    // use polling mode; if using interrupt, the last arg passed to UART_Init() must not be NULL
    Settings.bInterruptMode = 0;
    Settings.Parity = ADI_UART_PARITY_NONE;
    Settings.WordLength = ADI_UART_WLS_8;
    Settings.bDmaMode = false;
          
    /* Configure UART */
    if (ADI_UART_SUCCESS != (result = adi_UART_SetGenericSettings(hUartDevice, &Settings)))
    {
        return result;
    }

    /* Set UART baud rate to 115200 explicitly */
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