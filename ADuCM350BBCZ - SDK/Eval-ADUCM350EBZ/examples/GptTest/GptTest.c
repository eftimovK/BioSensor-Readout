/*********************************************************************************

Copyright (c) 2011-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/******************************************************************************
 * @file:    GptTest.c
 * @brief:   GPT Device Test for ADuCxxx
 * @version: $Revision: 28525 $
 * @date:    $Date: 2014-11-12 14:51:26 -0500 (Wed, 12 Nov 2014) $
*****************************************************************************/

/** \addtogroup GPT_Test GPT Test
 *  Example code demonstrating use of the GPIO functions.
 *  @{
 */

#include <stdint.h>

#include "test_common.h"
#include "gpt.h"

#ifdef ADI_ADUCM320
#define ADI_GPT_CLOCK_SELECT_32KHZ_INTERNAL_CLOCK ADI_GPT_CLOCK_SELECT_LFOSC
#endif
static ADI_GPT_RESULT_TYPE test_GetSet_LD			(ADI_GPT_DEV_ID_TYPE);
static ADI_GPT_RESULT_TYPE test_GPT_countUP			(ADI_GPT_DEV_ID_TYPE);
static ADI_GPT_RESULT_TYPE test_GPT_countDOWN		(ADI_GPT_DEV_ID_TYPE);
static ADI_GPT_RESULT_TYPE test_EventCapture		();
static ADI_GPT_RESULT_TYPE start_timer_freeRunning	(ADI_GPT_DEV_ID_TYPE timerID, ADI_GPT_HANDLE *);
void   GPTimerCallback                              (void *pCBParam, uint32_t Event, void *pArg);
static volatile uint16_t gpt_ISR_fired;


int main(void)
{
    /* Clock initialization */
    SystemInit();

    /* test system initialization */
    test_Init();

    if (ADI_GPT_SUCCESS != test_EventCapture()) {
        FAIL ("test_EventCapture() Failed");
    }
    if (ADI_GPT_SUCCESS != test_GetSet_LD(ADI_GPT_DEVID_0)) {
        FAIL ("test_GetSet_LD(ADI_GPT_DEVID_0) Failed");
    }

    if (ADI_GPT_SUCCESS != test_GetSet_LD(ADI_GPT_DEVID_1)) {
        FAIL ("test_GetSet_LD(ADI_GPT_DEVID_1) Failed");
    }

    if (ADI_GPT_SUCCESS != test_GPT_countUP(ADI_GPT_DEVID_0)) {
        FAIL ("test_GPT_countUP(ADI_GPT_DEVID_0) Failed");
    }

    if (ADI_GPT_SUCCESS != test_GPT_countUP(ADI_GPT_DEVID_1)) {
        FAIL ("test_GPT_countUP(ADI_GPT_DEVID_1) Failed");
    }

    if (ADI_GPT_SUCCESS != test_GPT_countDOWN(ADI_GPT_DEVID_0)) {
        FAIL ("test_GPT_countDOWN(ADI_GPT_DEVID_0) Failed");
    }

    if (ADI_GPT_SUCCESS != test_GPT_countDOWN(ADI_GPT_DEVID_1)) {
        FAIL ("test_GPT_countDOWN(ADI_GPT_DEVID_1) Failed");
    }

    /* success if we get this far... */
    PASS();
}

/* Does the timer have to be enabled to get and set ? */
static
ADI_GPT_RESULT_TYPE test_GetSet_LD(ADI_GPT_DEV_ID_TYPE timerID)
{
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;
    ADI_GPT_HANDLE hTimer;
    uint16_t ui_set;

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_Init(timerID, &hTimer );

    if (ADI_GPT_SUCCESS == result) {
        for (ui_set=0; ui_set < 0xFFFF; ui_set++)
        {
            uint16_t ui_get;

            if (ADI_GPT_SUCCESS == result)
                result = adi_GPT_SetLdVal(hTimer, ui_set );

            if (ADI_GPT_SUCCESS == result)
                result = adi_GPT_GetLdVal(hTimer, &ui_get );

            if (ADI_GPT_SUCCESS != result)
                break;

            if( ui_set != ui_get )
              result = ADI_GPT_ERR_UNKNOWN_ERROR;
        }
    }

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_UnInit(hTimer);

    return result;
}


static
ADI_GPT_RESULT_TYPE test_GPT_countUP(ADI_GPT_DEV_ID_TYPE timerID)
{
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;
    ADI_GPT_HANDLE hTimer;

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_Init(timerID, &hTimer );

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_RegisterCallback(hTimer, GPTimerCallback, NULL);

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_SetPrescaler(hTimer, ADI_GPT_PRESCALER_16);

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_SetClockSelect(hTimer, ADI_GPT_CLOCK_SELECT_32KHZ_INTERNAL_CLOCK);

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_SetPeriodicMode( hTimer, true, 0xff00);

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_SetCountMode(hTimer, ADI_GPT_COUNT_UP);

    gpt_ISR_fired = 0;

    if (ADI_GPT_SUCCESS == result)
        adi_GPT_SetTimerEnable(hTimer, true);

    if (ADI_GPT_SUCCESS == result)
        while( gpt_ISR_fired == 0);

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_UnInit(hTimer);

    return result;
}


static
ADI_GPT_RESULT_TYPE test_GPT_countDOWN(ADI_GPT_DEV_ID_TYPE timerID)
{
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;
    ADI_GPT_HANDLE hTimer;

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_Init(timerID, &hTimer );

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_RegisterCallback(hTimer, GPTimerCallback, NULL);

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_SetLdVal(hTimer, 0x10 );

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_SetCountMode(hTimer, ADI_GPT_COUNT_DOWN);

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_SetPrescaler(hTimer, ADI_GPT_PRESCALER_32768);

    gpt_ISR_fired = 0;

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_SetTimerEnable(hTimer, true );

    if (ADI_GPT_SUCCESS == result)
        while( gpt_ISR_fired == 0);

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_UnInit(hTimer);

    return result;
}

static ADI_GPT_RESULT_TYPE test_EventCapture()
{
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;
    ADI_GPT_HANDLE      hT0, hT1;
    uint16_t            capturedValue;


    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_Init(ADI_GPT_DEVID_0, &hT0 );

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_SetTimerEnable(hT0, true );

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_SetEventToCapture( hT0, ADI_GPT_0_CAPTURE_EVENT_TIMER1);

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_SetCaptureEventEnable( hT0, true);

    if (ADI_GPT_SUCCESS == result)
        result = start_timer_freeRunning(ADI_GPT_DEVID_1, &hT1);

    if (ADI_GPT_SUCCESS == result)
        while (adi_GPT_GetCaptureEventPending(hT0) == false);

    /* Shut down timer 1 to stop the capture from occuring */
    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_UnInit(hT1);

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_ClearCapturedEventStatus(hT0);

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_GetCapturedValue( hT0, &capturedValue);

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_UnInit(hT0);

    return result;
}


static
ADI_GPT_RESULT_TYPE start_timer_freeRunning(ADI_GPT_DEV_ID_TYPE timerID, ADI_GPT_HANDLE *phTimer)
{
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_Init(timerID, phTimer );

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_RegisterCallback(*phTimer, GPTimerCallback, NULL);

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_SetCountMode(*phTimer, ADI_GPT_COUNT_UP);

    /* Because this timer generates the event to be captured, */
    /* the prescaler value directly impacts the test time.    */
    /* Adjusting down the prescaler to speed up the test.     */
    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_SetPrescaler(*phTimer, ADI_GPT_PRESCALER_16);

    gpt_ISR_fired = 0;

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_SetTimerEnable(*phTimer, true);

    return result;
}


/**
 * @brief  Callback for GPT test program
 *
 * @return none
 *
 */
static void GPTimerCallback(void *pCBParam, uint32_t Event, void *pArg)
{
  gpt_ISR_fired++;
}



/*
** EOF
*/

/*@}*/

