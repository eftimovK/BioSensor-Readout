#include <stdio.h>

#include "test_common.h"
#include "wdt.h"
#include "gpt.h"


/*
 * This test will run with the following clock values
 *
 * WDT      32 KHz  Prescaler = 1
 * Timer0   16 MHz  Prescaler = 256
 *
 * Timer 0 will have an effective clock 64KHz
 *
 * Therefore, Timer0 should have 2x the number of WDT ticks when the WDT
 * expires.
 *
 * The WDT expired event is caught by TIMER0 (as a captured event)
 */

#define LOAD_COUNT 0x1000

/* result range varies by board due to +-20% tolerance on HFOSC crystal used by timer */
/* Note that high optimizations reduce the interrupt overhead and capture a smaller count */
#define MEAN_COUNT 0x2000
#define VARIANCE   0x0666
#define MAX_COUNT  MEAN_COUNT + VARIANCE
#define MIN_COUNT  MEAN_COUNT - VARIANCE

/* GPT clock mux */
#if defined (ADI_ADUCRF101)
#define SIXTEEN_MEG_CLOCK_SOURCE    ADI_GPT_CLOCK_SELECT_UCLK
#elif defined (ADI_ADUCM350)
#define SIXTEEN_MEG_CLOCK_SOURCE    ADI_GPT_CLOCK_SELECT_HFOSC
#elif defined (ADI_ADUCM320)
#define SIXTEEN_MEG_CLOCK_SOURCE   ADI_GPT_CLOCK_SELECT_HFXTAL
#else
#pragma message("Processor unsupported")
#endif

/* measured WDT timeout value captured by GPT0 */
volatile uint16_t TimerCaptureVal = 0;

/* GPT0 device handle */
ADI_GPT_HANDLE     hGPT0;

/* prototypes */
void Initialise_Timer0 (void);
void GPT0_Callback     (void *pCBParam, uint32_t Event, void *pArg);

/*
    This test only works from a hardware reset scenario, where the WDT is fresh.
    If you try to use the debugger "Reset" (which only resets the PC) to rerun
    this test, it will fail.  Always run using a Flash burn/reset/run sequence.
*/


int main(int argc, char *argv[])
{
    ADI_WDT_DEV_HANDLE hWDT;
    int32_t i;

    /* Clock initialization */
    SystemInit();

    /* Initlaise the UART driver to display results to the user. */
    test_Init();

    /* Initialise WDT so we can setup the test */
    if (adi_WDT_Init(ADI_WDT_DEVID_0, &hWDT)) {
        FAIL("adi_WDT_Init failed");
    }

    /* Allow time for watchdog to sync */
    for(i=0; i<10000; i++);

    /* Initilaise GP timer 0 to capture the WDT timeput event */
    Initialise_Timer0();

    /* start GP timer0 to measure the WDT timeout */
    if (adi_GPT_SetTimerEnable(hGPT0, true)) {
        FAIL("adi_GPT_SetTimerEnable failed");
    }

    /* start the WDT */
    if (adi_WDT_SetEnable(hWDT, true)) {
        FAIL("adi_WDT_SetEnable failed");
    }

    /* spin until we capture a WDT timeout value */
    while (!TimerCaptureVal)
    	;

    /* The captured value will not be exactly the same as the watchdog timer */
    /* value becuase the GP timer and WDT uses different clocks and slightly */
    /* different start points, so allow for a bit of slop in the compare.    */
    if (TimerCaptureVal > MAX_COUNT || TimerCaptureVal < MIN_COUNT) {
        FAIL("Timer capture compare failed");
    }
    /* start GP timer0 to measure the WDT timeout */
    if (adi_GPT_SetTimerEnable(hGPT0, false)) {
        FAIL("Disabling GPT-0 failed");
    }
    /* start the WDT */
    if (adi_WDT_SetEnable(hWDT, false)) {
        FAIL("Disabling WDT  failed");
    }

    PASS();

}


void Initialise_Timer0(void)
{
    int32_t i;

    /* initialise timer 0 */
    if (adi_GPT_Init(ADI_GPT_DEVID_0, &hGPT0)) {
        FAIL("adi_GPT_Init failed");
    }

    /* install callback */
    if (adi_GPT_RegisterCallback(hGPT0, GPT0_Callback, hGPT0)) {
        FAIL("adi_GPT_RegisterCallback failed");
    }

    /* Allow some time for the GPT to synch */
    for(i=0; i<10000; i++);

    /* Program GP Timer0 to capture on the the Watch Dog Timer interrupt event */
    if (adi_GPT_SetEventToCapture(hGPT0, ADI_GPT_0_CAPTURE_EVENT_WDTIMER)) {
        FAIL("adi_GPT_SetEventToCapture failed");
    }

    /* Enable timer0 in NVIC */
    NVIC_EnableIRQ(TIMER0_IRQn);

    /* Enable capture event */
    if (adi_GPT_SetCaptureEventEnable(hGPT0, true)) {
        FAIL("adi_GPT_SetCaptureEventEnable failed");
    }

    /* Timer 0 to count up */
    if (adi_GPT_SetCountMode(hGPT0, ADI_GPT_COUNT_UP)) {
        FAIL("adi_GPT_SetCountMode failed");
    }

    /* Set Timer0 to free running */
    if (adi_GPT_SetFreeRunningMode(hGPT0)) {
        FAIL("adi_GPT_SetFreeRunningMode failed");
    }

    /* select 16MHz internal clock olcillator */
    if (adi_GPT_SetClockSelect(hGPT0, SIXTEEN_MEG_CLOCK_SOURCE)) {
        FAIL("adi_GPT_SetClockSelect failed");
    }

    if (adi_GPT_SetPrescaler(hGPT0, ADI_GPT_PRESCALER_256)) {
        FAIL("adi_GPT_SetPrescaler failed");
    }
}


static void GPT0_Callback(void* hTimer, uint32_t Event, void* pArg)
{
    uint16_t captured;

	/* verify the handle */
	if (hTimer != hGPT0)
		FAIL("GPT0_Callback failed... bad handle");

	/* what happened? */
	switch (Event) {

		case ADI_GPT_EVENT_TIMEOUT:
			/* timeout occurred */
			FAIL("GPT0_Callback failed... unexpected timeout");
			break;

		case ADI_GPT_EVENT_CAPTURED:
			/* timer triggerred on event capture */
			adi_GPT_GetCapturedValue(hTimer, &captured);
            TimerCaptureVal = captured;
			break;

		default:
			FAIL("GPT0_Callback failed... unexpected event");
			break;
	}
    return;
}
