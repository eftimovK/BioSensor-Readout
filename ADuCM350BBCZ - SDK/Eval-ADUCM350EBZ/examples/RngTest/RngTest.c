/*********************************************************************************

Copyright (c) 2012-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*****************************************************************************
 * @file:    RngTest.c
 * @brief:   Random Number Generator (RNG) Test for ADuCM350
 * @version: $Revision: 28669 $
 * @date:    $Date: 2014-11-18 13:17:38 -0500 (Tue, 18 Nov 2014) $
 *****************************************************************************/

#include <time.h>
#include <stddef.h>  // for 'NULL'
#include <stdio.h>   // for scanf
#include <string.h>  // for strncmp

#include "rng.h"
#include "gpio.h"

#include "test_common.h"

/* Number of Random numbers required to be generated for successfull completion of the test */
#define ADI_RNG_NUM_RANDOM_NUMS             5

/* Macros to setup Random Number Generator */
/* Reload value for the sample counter ( accumulate so many samples) */
#define ADI_RNG_SAMPLE_LEN_RELOAD           50
/* Prescaler for sample counter reload value (sample length reload value scaled by 2^PRESCALER) */
#define ADI_RNG_SAMPLE_LEN_PRESCALER        9

/* Device handle */
ADI_RNG_HANDLE hRNG = NULL;
volatile bool_t bRngReadyFlag;
volatile bool_t bHibernateExitFlag;
volatile uint32_t RandomNumCount;

/* prototypes */
void rng_Init(void);
void rng_TestRandom (void);
void rng_TestTimer(void);

/* RNG interrupt callback prototype */
void rngCallback (void *pCBParam, uint32_t nEvent, void *EventArg);

int main (void)
{
    /* Initialize system */
    SystemInit();

    /* test system initialization */
    test_Init();

    /* initialize driver */
    rng_Init();

    /* test random number generation mode */
    rng_TestRandom();

    /* test one-shot timer mode */
    rng_TestTimer();

    PASS();
}


void rng_Init (void) {

    /* Init RNG */
    if (ADI_RNG_SUCCESS != adi_RNG_Init(ADI_RNG_DEVID_0, &hRNG))
        FAIL("adi_RNG_Init failed");

    /* Disable RNG */
    if (ADI_RNG_SUCCESS != adi_RNG_Enable(hRNG, false))
        FAIL("adi_RNG_Enable failed");

    /*
     * Enable oscillator counter
     * Oscillator not required for random number generation
     * Here it is enabled only for test purpose
     */
    if (ADI_RNG_SUCCESS != adi_RNG_EnableCounter(hRNG, true))
        FAIL("adi_RNG_EnableCounter failed");

    /* Set sample counter reload value */
    if (ADI_RNG_SUCCESS != adi_RNG_SetLenReload(hRNG, ADI_RNG_SAMPLE_LEN_RELOAD))
        FAIL("adi_RNG_SetLenReload failed");

    /* Set sample counter reload prescaler value */
    if (ADI_RNG_SUCCESS != adi_RNG_SetLenPrescaler(hRNG, ADI_RNG_SAMPLE_LEN_PRESCALER))
        FAIL("adi_RNG_SetLenPrescaler failed");

    /* register callback handler for all interrupts */
    if (ADI_RNG_SUCCESS != adi_RNG_RegisterCallback (hRNG,  rngCallback, ADI_RNG_INTERRUPT_RNG_RDY)) {
        FAIL("adi_RNG_RegisterCallback failed");
    }
}

void rng_TestRandom (void) {

    /* Reset the number of random numbers generated count */
    RandomNumCount = 0;

    /* Set device in RNG mode */
    if (ADI_RNG_SUCCESS != adi_RNG_SetMode(hRNG, false))
        FAIL("adi_RNG_SetMode failed");

    /* go to sleep and await RNG Data Ready interrupt */
    PERF("Random Number Generation test start");

    /* Enable RNG */
    if (ADI_RNG_SUCCESS != adi_RNG_Enable(hRNG, true))
        FAIL("adi_RNG_Enable failed");

	/* allow time for uart output to clear before hibernate cuts clock */
	for (int i=0; i<1000; i++);

	/* enter full hibernate mode with wakeup flag and no masking */
	bHibernateExitFlag = false;
	if (SystemEnterLowPowerMode(ADI_SYS_MODE_HIBERNATE, &bHibernateExitFlag, 0))
		FAIL("SystemEnterLowPowerMode failed");

    PERF("Random Number Generation test complete\n");

    /* verify expected results */
    if (RandomNumCount < ADI_RNG_NUM_RANDOM_NUMS)
        FAIL("rng_TestRandom failed to generated the expected number of Random numbers");

    /* Disable RNG */
    if (ADI_RNG_SUCCESS != adi_RNG_Enable(hRNG, false))
        FAIL("adi_RNG_Enable failed");
}

void rng_TestTimer (void) {

    bRngReadyFlag = false;

    /* Set device in Timer mode */
    if (ADI_RNG_SUCCESS != adi_RNG_SetMode(hRNG, true))
        FAIL("adi_RNG_SetMode failed");

    /* go to sleep and await RNG Data Ready interrupt */
    PERF("One-shot Timer mode test start");

    /* Enable RNG */
    if (ADI_RNG_SUCCESS != adi_RNG_Enable(hRNG, true))
        FAIL("adi_RNG_Enable failed");

	/* allow time for uart output to clear before hibernate cuts clock */
	for (int i=0; i<1000; i++);

	/* enter full hibernate mode with wakeup flag and no masking */
	bHibernateExitFlag = false;
	if (SystemEnterLowPowerMode(ADI_SYS_MODE_HIBERNATE, &bHibernateExitFlag, 0))
		FAIL("SystemEnterLowPowerMode failed");

    PERF("One-shot Timer mode complete\n");

    /* verify expected results */
    if (!bRngReadyFlag) FAIL("rng_TestTimer failed to get RNG Data  Ready Interrupt");

    /* Disable RNG */
    if (ADI_RNG_SUCCESS != adi_RNG_Enable(hRNG, false))
        FAIL("adi_RNG_Enable failed");
}

/* RNG Callback handler */
void rngCallback (void *pCBParam, uint32_t nEvent, void *EventArg) {

    char        buffer [128];
    uint16_t    RandomNum;
    uint32_t    OscCount;

    /* process RNG interrupts (cleared by driver) */
    if (ADI_RNG_INTERRUPT_RNG_RDY & (ADI_RNG_INTERRUPT_TYPE) nEvent) {

        PERF("got RNG interrupt callback with ADI_RNG_INTERRUPT_RNG_RDY status");

        /* RNG data ready interrupt received */
        bRngReadyFlag = true;

        /* Update random numbers generated counter */
        RandomNumCount++;

        /* Get the Oscillator count before reading the RNG data */
        if (ADI_RNG_SUCCESS != adi_RNG_GetOscCount (hRNG, &OscCount))
            FAIL("adi_RNG_GetOscCount failed");

        /* covert random number to string */
        sprintf (buffer, "Oscillator Count = %d", OscCount);
        PERF(buffer);

        /* Read the RNG data to start new random number generation */
        if (ADI_RNG_SUCCESS != adi_RNG_GetRngData (hRNG, &RandomNum))
            FAIL("adi_RNG_GetRngData failed");

        /* covert random number to string */
        sprintf (buffer, "Random Number generated = %d", RandomNum);
        PERF(buffer);

        /* IF (Enough random numbers generated) */
        if (RandomNumCount >= ADI_RNG_NUM_RANDOM_NUMS)
        {
            /* Disable RNG */
            if (ADI_RNG_SUCCESS != adi_RNG_Enable(hRNG, false))
                FAIL("adi_RNG_Enable failed");
            
            bHibernateExitFlag = true;  // exit hibernation on return from interrupt
        }
        /* ELSE (More random numbers needed) */
    }
}
