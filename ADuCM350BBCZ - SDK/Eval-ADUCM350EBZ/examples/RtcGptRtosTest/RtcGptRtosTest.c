/*********************************************************************************

Copyright (c) 2013-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using
this software you agree to the terms of the associated Analog Devices Software
License Agreement.

*********************************************************************************/

/*****************************************************************************
 * @file:    RtosTest.c
 * @brief:   Verify that drivers build in RTOS mode
 * @version: $Revision: 29403 $
 * @date:    $Date: 2015-01-08 10:34:56 -0500 (Thu, 08 Jan 2015) $
 *****************************************************************************/

#include <time.h>
#include <stddef.h>  // for 'NULL'
#include <stdio.h>   // for scanf
#include <string.h>  // for strncmp

#include <ucos_ii.h>


#include "system.h"
#include "rtc.h"
#include "gpio.h"
#include "rtc.h"

#include "test_common.h"

/*
 * RTOS definitions
 */
#define THREAD_RTC_STK_SIZE 2048u
#define THREAD_RTC_PRIO 5

#define THREAD_GPT_STK_SIZE 2048u
#define THREAD_GPT_PRIO 6

#define THREAD_SYNC_STK_SIZE 2048u
#define THREAD_SYNC_PRIO 4

// leap-year compute macro (ignores leap-seconds)
#define LEAP_YEAR(x) (((0==x%4)&&(0!=x%100))||(0==x%400))

/* device and board specific values selected according to computed trim measurement */
/* THESE VALUES ARE UNIQUE TO THE EVAL-ADUCM350EBZ REV. 0 BOARD, SERIAL#: AVAS 35070 */
#define ADI_RTC_TRIM_INTERVAL    (uint32_t)ADI_RTC_TRIM_INTERVAL_14
#define ADI_RTC_TRIM_DIRECTION   (uint32_t)ADI_RTC_TRIM_SUB
#define ADI_RTC_TRIM_VALUE       (uint32_t)ADI_RTC_TRIM_1

uint32_t BuildSeconds(void);
extern void rtcCallback  (void *pCBParam, uint32_t nEvent, void *EventArg);


char    g_Thread_RTC_Stack[THREAD_RTC_STK_SIZE];
//OS_TCB  g_Thread_RTC_TCB;

char    g_Thread_GPT_Stack[THREAD_GPT_STK_SIZE];
//OS_TCB  g_Thread_GPT_TCB;

char    g_Thread_SYNC_Stack[THREAD_GPT_STK_SIZE];
//OS_TCB  g_Thread_SYNC_TCB;


/* RTOS object definitions  */
OS_EVENT    *g_Sem_RTC;
OS_EVENT    *g_Sem_GPT;
OS_FLAG_GRP *g_Flag_AllDone;


#define RTC_THREAD_DONE 0x1
#define GPT_THREAD_DONE 0x2
/*
* The RTC thread simply waits for a three second event and then notifies
* the SYNC thread that it is done
*/

#define RTC_ALARM_OFFSET 5

ADI_RTC_HANDLE hRTC = NULL;
/* Number of RTC alarms required to be registered for successfull completion of the test */
#define ADI_RTC_NUM_ALARMS      3


void Thread_RTC_Run(void* arg)
{

    ADI_RTC_RESULT_TYPE result;
    INT8U               err;
    INT32U              rtcCount;


     /* callbacks */
    ADI_RTC_INT_SOURCE_TYPE callbacks = (ADI_RTC_INT_SOURCE_TYPE)
                                       ( ADI_RTC_INT_SOURCE_WRITE_PEND
                                       | ADI_RTC_INT_SOURCE_WRITE_SYNC
                                       | ADI_RTC_INT_SOURCE_WRITE_PENDERR
                                       | ADI_RTC_INT_SOURCE_ISO_DONE
                                       | ADI_RTC_INT_SOURCE_LCD_UPDATE
                                       | ADI_RTC_INT_SOURCE_ALARM
                                       | ADI_RTC_INT_SOURCE_FAIL);

    result = adi_RTC_Init(ADI_RTC_DEVID_0, &hRTC);

    /* retry on failsafe */
    if (ADI_RTC_ERR_CLOCK_FAILSAFE == result) {
        uint32_t buildTime = BuildSeconds();

        /* clear the failsafe */
        adi_RTC_ClearFailSafe();

        /* un-init RTC for a clean restart, but ignore failure */
        adi_RTC_UnInit(hRTC);

        /* re-init RTC */
        if (ADI_RTC_SUCCESS != adi_RTC_Init(ADI_RTC_DEVID_0, &hRTC))
            FAIL("Double fault on adi_RTC_Init");

        PERF("Resetting clock and trim values after init failure");

        /* set clock to latest build time */
        if (ADI_RTC_SUCCESS != adi_RTC_SetCount(hRTC, buildTime))
            FAIL("adi_RTC_SetCount failed");

        /* apply pre-computed calibration BOARD-SPECIFIC trim values */
        if (adi_RTC_SetTrim(hRTC, ADI_RTC_TRIM_INTERVAL | ADI_RTC_TRIM_DIRECTION | ADI_RTC_TRIM_VALUE))
            FAIL("adi_RTC_SetTrim failed");

        /* enable trimming */
        if (adi_RTC_EnableTrim(hRTC, true))
            FAIL("adi_RTC_EnableTrim failed");

    /* catch all other open failures */
    } else {
    if (result != ADI_RTC_SUCCESS)
		  FAIL("Generic failure to initialize the RTC");
    }

    /* RTC opened successfully... */

    /* disable alarm */
    if (ADI_RTC_SUCCESS != adi_RTC_EnableAlarm(hRTC, false))
        FAIL("adi_RTC_EnableAlarm failed");


    /* register callback handler for all interrupts */
    if (ADI_RTC_SUCCESS != adi_RTC_RegisterCallback (hRTC,  rtcCallback, callbacks))
    {
        FAIL("adi_RTC_RegisterCallback failed");
    }

    /* enable RTC */
    if (ADI_RTC_SUCCESS != adi_RTC_EnableDevice(hRTC, true))
    {
        FAIL("adi_RTC_EnableDevice failed");
    }


    /* get the current count */
    if (ADI_RTC_SUCCESS != adi_RTC_GetCount(hRTC, &rtcCount))
        FAIL("adi_RTC_GetCount failed");

    /* set RTC alarm */
    if (ADI_RTC_SUCCESS != adi_RTC_SetAlarm(hRTC, rtcCount+RTC_ALARM_OFFSET))
        FAIL("adi_RTC_SetAlarm failed");

        /* disable alarm */
    if (ADI_RTC_SUCCESS != adi_RTC_EnableAlarm(hRTC, true))
        FAIL("adi_RTC_EnableAlarm failed");


    /* enable alarm interrupting */
    if (ADI_RTC_SUCCESS != adi_RTC_EnableInterrupts(hRTC, ADI_RTC_INT_ENA_ALARM, true))
        FAIL("adi_RTC_EnableInterrupts failed");

    OSSemPend(g_Sem_RTC, 0, &err);
    if (err != OS_ERR_NONE)
    {

        FAIL("Error pending on RTC semaphore\n");
    }

      /* disable alarm */
    if (ADI_RTC_SUCCESS != adi_RTC_EnableAlarm(hRTC, false))
        FAIL("adi_RTC_EnableAlarm failed");


    if( ADI_RTC_SUCCESS != adi_RTC_UnInit(hRTC))
         FAIL("adi_RTC_UnInit failed");

    /*
    * Tell the SYNC thread that the RTC is all done
    */
    OSFlagPost(g_Flag_AllDone,
               RTC_THREAD_DONE,
               OS_FLAG_SET,
               &err);
     if (err != OS_ERR_NONE)
    {
        FAIL("OSFlagPost failed in RTC thread\n");
    }

    //err = OSTaskDel(THREAD_RTC_PRIO);
    while(1);


}


/*
* GPT thread
*/

extern void   GPTimerCallback (void *pCBParam, uint32_t Event, void *pArg);

void Thread_GPT_Run(void* arg)
{

    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;
    ADI_GPT_DEV_ID_TYPE timerID = ADI_GPT_DEVID_0;
    ADI_GPT_HANDLE      hTimer;
    INT8U               err;


    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_Init(timerID, &hTimer );

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_RegisterCallback(hTimer, GPTimerCallback, NULL);


    if (ADI_GPT_SUCCESS == result)
         result = adi_GPT_SetLdVal(hTimer, 0xf);


    if (ADI_GPT_SUCCESS == result)
        adi_GPT_SetTimerEnable(hTimer, true);

    if (ADI_GPT_SUCCESS == result)
    {
      OSSemPend(g_Sem_GPT,
                0,
                &err);
      if (err != OS_ERR_NONE)
      {
          FAIL("Error pending on GPT semaphore\n");
      }
    }

    if (ADI_GPT_SUCCESS == result)
        result = adi_GPT_UnInit(hTimer);

    if (ADI_GPT_SUCCESS != result)
      FAIL("Error in GPT thread");

    /*
    * Tell the SYNC thread that the GPT is all done
    */
    OSFlagPost(g_Flag_AllDone,
               GPT_THREAD_DONE,
               OS_FLAG_SET,
               &err);
     if (err != OS_ERR_NONE)
    {
        FAIL("OSFlagPost failed in GPT thread\n");
    }

    //err = OSTaskDel(THREAD_GPT_PRIO);
    while(1);

}


void Thread_SYNC_Run(void* arg)
{

    INT8U     err;


    OSFlagPend(g_Flag_AllDone,
                       RTC_THREAD_DONE | GPT_THREAD_DONE,
                       OS_FLAG_WAIT_SET_ALL + OS_FLAG_CONSUME,
                       0,
                       &err);

    if( err != OS_ERR_NONE)
      FAIL("OSFlagPend failed");



    PASS();

    //err = OSTaskDel(THREAD_SYNC_PRIO);


}


int main (void)
{
    INT8U    OSRetVal;

  /* Initialize system */
    SystemInit();

    /* test system initialization */
    test_Init();


    OSInit();

    /* Create the RTC thread */
    OSRetVal = OSTaskCreate (Thread_RTC_Run,
                NULL,
               (void *)(g_Thread_RTC_Stack + THREAD_RTC_STK_SIZE),
               THREAD_RTC_PRIO);

    if (OSRetVal != OS_ERR_NONE)
    {
    	FAIL("Error creating RTC thread1\n");
    	return 1;
    }

    /* Create the GPT Thread */
        /* Create the RTC thread */
    OSRetVal = OSTaskCreate (Thread_GPT_Run,
                NULL,
               (void *)(g_Thread_GPT_Stack + THREAD_GPT_STK_SIZE),
               THREAD_GPT_PRIO);

    if (OSRetVal != OS_ERR_NONE)
    {
    	FAIL("Error creating GPT thread\n");
    	return 1;
    }


    /* Create the SYNC thread */
        /* Create the RTC thread */
    OSRetVal = OSTaskCreate (Thread_SYNC_Run,
                NULL,
               (void *)(g_Thread_SYNC_Stack + THREAD_SYNC_STK_SIZE),
               THREAD_SYNC_PRIO);

    if (OSRetVal != OS_ERR_NONE)
    {
    	FAIL("Error creating SYNC thread\n");
    	return 1;
    }

    /* Create the RTOS objects */
    g_Sem_RTC      = OSSemCreate(0u);
    g_Sem_GPT      = OSSemCreate(0u);
    g_Flag_AllDone = OSFlagCreate(0x00, &OSRetVal);

    if( OSRetVal != OS_ERR_NONE)
      FAIL("OSFlagCreate failed");

    OSStart();

    printf("Error starting the RTOS\n");
    return 0;



}


uint32_t BuildSeconds(void)
{
    /* count up seconds from the epoc (1/1/70) to the most recient build time */

    char timestamp[] = __DATE__ " " __TIME__;
    int month_days [] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint32_t days, month, date, year, hours, minutes, seconds;
    char Month[4];

    // parse the build timestamp
    sscanf(timestamp, "%s %d %d %d:%d:%d", Month, &date, &year, &hours, &minutes, &seconds);

    // parse ASCII month to a value
    if     ( !strncmp(Month, "Jan", 3 )) month = 1;
    else if( !strncmp(Month, "Feb", 3 )) month = 2;
    else if( !strncmp(Month, "Mar", 3 )) month = 3;
    else if( !strncmp(Month, "Apr", 3 )) month = 4;
    else if( !strncmp(Month, "May", 3 )) month = 5;
    else if( !strncmp(Month, "Jun", 3 )) month = 6;
    else if( !strncmp(Month, "Jul", 3 )) month = 7;
    else if( !strncmp(Month, "Aug", 3 )) month = 8;
    else if( !strncmp(Month, "Sep", 3 )) month = 9;
    else if( !strncmp(Month, "Oct", 3 )) month = 10;
    else if( !strncmp(Month, "Nov", 3 )) month = 11;
    else if( !strncmp(Month, "Dec", 3 )) month = 12;

    // count days from prior years
    days=0;
    for (int y=1970; y<year; y++) {
        days += 365;
        if (LEAP_YEAR(y))
            days += 1;
    }

    // add days for current year
    for (int m=1; m<month; m++)
        days += month_days[m-1];

    // adjust if current year is a leap year
    if ( (LEAP_YEAR(year) && ( (month > 2) || ((month == 2) && (date == 29)) ) ) )
        days += 1;

    // add days this month (not including current day)
    days += date-1;

    return (days*24*60*60 + hours*60*60 + minutes*60 + seconds);
}


/* RTC Callback handler */
void rtcCallback (void *pCBParam, uint32_t nEvent, void *EventArg)
{
    INT8U err;

    if (ADI_RTC_INT_SOURCE_ALARM & (ADI_RTC_INT_SOURCE_TYPE) nEvent)
    {
           err = OSSemPost(g_Sem_RTC);
 	    if (err != OS_ERR_NONE)
	    {
	    	FAIL("Error posting RTC semaphore from RTC callback\n");
	    }
   }

}


/**
 * Callback for GPT
 *
 */
void GPTimerCallback(void *pCBParam, uint32_t Event, void *pArg)
{
    INT8U err;

    err = OSSemPost(g_Sem_GPT);
    if (err != OS_ERR_NONE)
    {
        FAIL("Error posting GPT semaphore from GPT callback\n");
    }

}

