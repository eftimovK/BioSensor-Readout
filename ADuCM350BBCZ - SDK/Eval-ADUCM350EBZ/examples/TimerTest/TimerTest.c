/*********************************************************************************

Copyright (c) 2012-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/


/*****************************************************************************
 * @file:    TimerTest.c
 * @brief:   Timer Test for ADuCM350
 * @version: $Revision: 28525 $
 * @date:    $Date: 2014-11-12 14:51:26 -0500 (Wed, 12 Nov 2014) $
 *****************************************************************************/

/*! \addtogroup Timer_Test Timer Test
 *  .
 *  @{
 */

/* Apply ADI MISRA Suppressions */
#define ASSERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"

#include <stddef.h>		/* for 'NULL' */
//#include <string.h>		/* for strlen */

#include "gpt.h"

#include "test_common.h"

#ifdef ADI_ADUCM320
#define GPT_EVENT_DEVICE_ID ADI_GPT_DEVID_2
#define GPT_CLOCK_SOURCE    ADI_GPT_CLOCK_SELECT_PCLK
#define GPT_IRQn_NUM        TIMER2_IRQn
#define GPT_EVENT_SOURCE    ADI_GPT_1_CAPTURE_EVENT_TIMER2
#else
#define GPT_EVENT_DEVICE_ID  ADI_GPT_DEVID_0
#define GPT_CLOCK_SOURCE     ADI_GPT_CLOCK_SELECT_EXTERNAL_CLOCK
#define GPT_IRQn_NUM         TIMER0_IRQn
#define GPT_EVENT_SOURCE     ADI_GPT_1_CAPTURE_EVENT_TIMER0
#endif

// FIXME: this is already defined in system.c
// but still needed here due to missing API
// To be removed when the new API is added
#define OSCKEY_UNLOCK                (0xCB14)

/* The number of capture events to be recorded */
#define                 MAX_CAPTURE_EVENTS      10

/* The timeout for GPT0 (number of LFXTAL periods that generate an event) */
#define                 TEST_LFXTAL_TIMER_VAL   0x100

volatile uint16_t       countCaptureEvents = 0;

/* Additional 16-bit range for GPT1, extending GPT1 to 32-bit timer. */
volatile uint16_t       gpt1CountExt;

/* GPT1 capture values, lower half if the captured value and */
/* upper half is the 32-bit extension for the timer          */
volatile uint32_t       gpt1CaptureVal[MAX_CAPTURE_EVENTS];

/* Device handles */
ADI_GPT_HANDLE          hGPT0;
ADI_GPT_HANDLE          hGPT1;

ADI_GPT_RESULT_TYPE     test_GPT0_Init          (void);
ADI_GPT_RESULT_TYPE     test_GPT0_Enable        (void);
ADI_GPT_RESULT_TYPE     test_GPT0_Disable       (void);

ADI_GPT_RESULT_TYPE     test_GPT1_Init          (void);
ADI_GPT_RESULT_TYPE     test_GPT1_Enable        (void);
ADI_GPT_RESULT_TYPE     test_GPT1_Disable       (void);

void Timer1_Callback(void *pCBParam, uint32_t Event, void *EventArg);


/*!
 * @brief       Function 'main' for timer example program
 *
 * @param       none
 * @return      int (Zero for success, non-zero for failure).
 *
 * @details     Compare LFXTAL with HFOSC.
 *              Use LFXTAL as clock source for GPT0, use HFOSC (PCLK) as clock source for GPT1
 *              and capture GPT0 events using GPT1.
 *
 */


int main(void)
{
    int i;

    /* Initialize system */
    SystemInit();
    
    /* test system initialization */
    test_Init();
#ifndef ADI_ADUCM320
    /* Set PCLK frequency to 16MHz for better resolution */
    /* Need to set HCLK frequency to 16MHz first */
    SetSystemClockDivider(ADI_SYS_CLOCK_CORE, 1);
    SetSystemClockDivider(ADI_SYS_CLOCK_PCLK, 1);


    /* Turn on LFXTAL */
    SystemEnableClockSource(ADI_SYS_CLOCK_SOURCE_LFXTAL, true);

    /* Wait for LFXTAL to stabilize */
    while (!(pADI_SYSCLK->CLKSTAT0 & BITM_SYSCLK_CLKSTAT0_LFXTALSTATUS))
        ;
#endif
    if (test_GPT0_Init()) {
        FAIL("GPT0 Init Failed");
    }

    if (test_GPT1_Init()) {
        FAIL("GPT1 Init Failed");
    }

    /* GPT1 enabled first to be ready for capture */
    if (test_GPT1_Enable()) {
        FAIL("GPT1 Enable Failed");
    }
    if (test_GPT0_Enable()) {
        FAIL("GPT0 Enable Failed");
    }

    while (countCaptureEvents < MAX_CAPTURE_EVENTS)
        ;

    /* Replace the timer captures with deltas           */
    /* First element in the array needs to be discarded */
    for (i = MAX_CAPTURE_EVENTS - 1; i > 0; i--) {
        gpt1CaptureVal[i] = gpt1CaptureVal[i] - gpt1CaptureVal[i - 1];
    }
    
    if (test_GPT0_Disable()) {
        FAIL("GPT0 disable Failed");
    }
    if (test_GPT1_Disable()) {
        FAIL("GPT1 disable Failed");
    }

    // FIXME: add self-checking capabilities, maybe deltas with a +/-1 range?

    PASS();

}

/**********************/
/*   GPT Functions    */
/**********************/

/*
 * Timer setup for LFXTAL.
 * Programming GPT0 with LFXTAL as clock source
 *
 */
ADI_GPT_RESULT_TYPE test_GPT0_Init(void) {
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;

    /* Initialize timer 0 */
    if ((result = adi_GPT_Init(GPT_EVENT_DEVICE_ID, &hGPT0)) != ADI_GPT_SUCCESS) {
        return result;
    }

    /* Set periodic mode */
    if ((result = adi_GPT_SetPeriodicMode(hGPT0, true, TEST_LFXTAL_TIMER_VAL)) != ADI_GPT_SUCCESS) {
        return result;
    }

    /* Set LFXTAL as clock source */
    if ((result = adi_GPT_SetClockSelect(hGPT0, GPT_CLOCK_SOURCE)) != ADI_GPT_SUCCESS) {
        return result;
    }

    /* Set prescaler value to 1 */
    if ((result = adi_GPT_SetPrescaler(hGPT0, ADI_GPT_PRESCALER_1)) != ADI_GPT_SUCCESS) {
        return result;
    }

    /* Select count down mode */
    if ((result = adi_GPT_SetCountMode(hGPT0, ADI_GPT_COUNT_DOWN)) != ADI_GPT_SUCCESS) {
        return result;
    }

    return result;
}

ADI_GPT_RESULT_TYPE test_GPT0_Enable(void) {
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;

    /* Enable timer interrupt in NVIC */
    NVIC_EnableIRQ(GPT_IRQn_NUM);

    if ((result = adi_GPT_SetTimerEnable(hGPT0, true)) != ADI_GPT_SUCCESS) {
        return result;
    }

    return result;
}

ADI_GPT_RESULT_TYPE test_GPT0_Disable(void) {
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;

    /* Enable timer interrupt in NVIC */
    NVIC_DisableIRQ(GPT_IRQn_NUM);

    if ((result = adi_GPT_SetTimerEnable(hGPT0, false)) != ADI_GPT_SUCCESS) {
        return result;
    }

    return result;
}


/*
 * Timer setup for HFOSC.
 * Programming GPT1 with PCLK as clock source (PCLK is clocked by HFOSC).
 *
 */
ADI_GPT_RESULT_TYPE test_GPT1_Init(void) {
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;

    /* Initialize timer 0 */
    if ((result = adi_GPT_Init(ADI_GPT_DEVID_1, &hGPT1)) != ADI_GPT_SUCCESS) {
        return result;
    }

    if((result = adi_GPT_RegisterCallback(hGPT1, Timer1_Callback, NULL)) != ADI_GPT_SUCCESS) {
        return result;
    }

    /* Set free running mode */
    if ((result = adi_GPT_SetFreeRunningMode(hGPT1)) != ADI_GPT_SUCCESS) {
        return result;
    }

    /* Set PCLK as clock source */
    if ((result = adi_GPT_SetClockSelect(hGPT1, ADI_GPT_CLOCK_SELECT_PCLK)) != ADI_GPT_SUCCESS) {
        return result;
    }

    /* Set prescaler value to 1 (which means 4 for PCLK) */
    if ((result = adi_GPT_SetPrescaler(hGPT1, ADI_GPT_PRESCALER_1)) != ADI_GPT_SUCCESS) {
        return result;
    }

    /* Select count up mode */
    if ((result = adi_GPT_SetCountMode(hGPT1, ADI_GPT_COUNT_UP)) != ADI_GPT_SUCCESS) {
        return result;
    }

    /* Capture GPT0 events */
    if ((result = adi_GPT_SetEventToCapture(hGPT1, GPT_EVENT_SOURCE)) != ADI_GPT_SUCCESS) {
        return result;
    }

    /* Enable capture */
    if ((result = adi_GPT_SetCaptureEventEnable(hGPT1, true)) != ADI_GPT_SUCCESS) {
        return result;
    }

    return result;


}

ADI_GPT_RESULT_TYPE test_GPT1_Enable(void) {
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;

    /* Enable timer interrupt in NVIC */
    NVIC_EnableIRQ(TIMER1_IRQn);

    gpt1CountExt = 0;

    if ((result = adi_GPT_SetTimerEnable(hGPT1, true)) != ADI_GPT_SUCCESS) {
        return result;
    }

    return result;
}

ADI_GPT_RESULT_TYPE test_GPT1_Disable(void) {
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;

    /* Enable timer interrupt in NVIC */
    NVIC_DisableIRQ(TIMER1_IRQn);

    if ((result = adi_GPT_SetTimerEnable(hGPT1, false)) != ADI_GPT_SUCCESS) {
        return result;
    }

    return result;
}



/**************************/
/*   Interrupt Handlers   */
/**************************/

void Timer1_Callback(void *pCBParam, uint32_t Event, void *EventArg) {
    uint16_t capval;

    switch(Event)
    {
        case ADI_GPT_EVENT_TIMEOUT:
          gpt1CountExt++;
          break;

        case ADI_GPT_EVENT_CAPTURED:
           adi_GPT_GetCapturedValue(hGPT1, &capval);
           gpt1CaptureVal[countCaptureEvents] = (gpt1CountExt << 16) + capval;
           countCaptureEvents++;
           break;

    }
}

/* Revert ADI MISRA Suppressions */
#define REVERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"

/*
** EOF
*/

/*@}*/
