#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include "test_common.h"
#include "wdt.h"
#include "wut.h"
#include "gpt.h"


/* device handles */
ADI_WUT_DEV_HANDLE hWUT;
ADI_GPT_HANDLE     hGPT;

/* GPT evet capture indexes */
enum CAPTURES {CAPA = 0, CAPB, CAPC, CAPD, CAPR, NUM_CAPTURES};

/* pick some 16-bit comparator values for the
   WUT (even thoguh the WUT is 32-bit), because
   the GPT resolution is only 16-bit */
#define LOADB   0x80
#define LOADC  0x800
#define LOADD 0x8000

// GPT captures will slightly exceed actual WUT comparator
// values by <= DELAY because GPT is started before WUT
#define DELAY 0x10
#define LIMITB LOADB + DELAY
#define LIMITC LOADC + DELAY
#define LIMITD LOADD + DELAY

/* GP timer capture assets */
uint16_t captureValue[NUM_CAPTURES] = {0};
int captureIndex = CAPB;
bool_t captureComplete = false;

/* prototypes */
void RunTest(void);
void InitializeGPT(void);
void InitializeWUT(void);
void Test_WUT_Compare(void);

/* callbacks */
void WUT_Callback  (void *pCBParam, uint32_t Event, void *pArg);
void GPT0_Callback  (void *pCBParam, uint32_t Event, void *pArg);


int main(int argc, char *argv[])
{
    SystemInit();

    test_Init();

    /* Initialize the GP timer */
    InitializeGPT();

    /* Initialize the wakeup timer */
    InitializeWUT();

    /* run the test */
    RunTest();

    /* If we get this far, then the test passes */
    PASS();
}


void RunTest(void)
{
    /* start the timers */
    if( adi_GPT_SetTimerEnable(hGPT, true) != 0 ) {
        FAIL("adi_GPT_SetTimerEnable failed");
    }
    if( adi_WUT_SetTimerEnable(hWUT, true) != 0 ) {
        FAIL("adi_WUT_SetTimerEnable failed");
    }

	/* wait on capture flag */
    while(!captureComplete);

	/* stop the timers */
    if( adi_GPT_SetTimerEnable(hGPT, false) != 0 ) {
        FAIL("adi_GPT_SetTimerEnable failed");
    }
    if( adi_WUT_SetTimerEnable(hWUT, false) !=  0 ) {
        FAIL("adi_WUT_SetTimerEnable failed");
    }

    /* test the  captured event values... */
    if (captureValue[CAPB] < LOADB || captureValue[CAPB] > LIMITB) {
        FAIL("COMPB captured value out of range");
    }
    if (captureValue[CAPC] < LOADC || captureValue[CAPC] > LIMITC) {
        FAIL("COMPC captured value out of range");
    }
    if (captureValue[CAPD] < LOADD || captureValue[CAPD] > LIMITD) {
        FAIL("COMPD captured value out of range");
    }

    return;
}


void InitializeGPT(void)
{

    ADI_GPT_CLOCK_SELECT_TYPE clock =
#if defined (ADI_ADUCRF101)
    ADI_GPT_CLOCK_SELECT_UCLK;
#elif defined (ADI_ADUCM350) ||  defined (ADI_ADUCM320)
    ADI_GPT_CLOCK_SELECT_PCLK;
#else
#error "Unsupported test target."
#endif

    /* initialise timer 0 */
    if( adi_GPT_Init(ADI_GPT_DEVID_0, &hGPT ) != 0 ) {
        FAIL("adi_GPT_Init failed");
    }

    /* install callback */
    if (adi_GPT_RegisterCallback(hGPT, GPT0_Callback, hGPT)) {
        FAIL("adi_GPT_RegisterCallback failed");
    }

    /* which event to cpature */
    if( adi_GPT_SetEventToCapture( hGPT, ADI_GPT_0_CAPTURE_EVENT_WAKEUP_TIMER ) != 0 ) {
        FAIL("adi_GPT_SetEventToCapture failed");
    }

    /* Enable event capture */
    if( adi_GPT_SetCaptureEventEnable( hGPT, true) != 0 ) {
        FAIL("adi_GPT_SetCaptureEventEnable failed");
    }

    /* Enalbe timer0 in NVIC */
    NVIC_EnableIRQ(TIMER0_IRQn);

    /* select timer clock source */
    if( adi_GPT_SetClockSelect(hGPT, clock) != 0 ) {
        FAIL("adi_GPT_SetClockSelect failed");
    }

    /* Set timer prescaller */
    if( adi_GPT_SetPrescaler(hGPT, ADI_GPT_PRESCALER_1) != 0 ) {
        FAIL("adi_GPT_SetPrescaler failed");
    }

    /* set timer count direction */
    if( adi_GPT_SetCountMode(hGPT, ADI_GPT_COUNT_UP) != 0 ) {
        FAIL("adi_GPT_SetCountMode failed");
    }

    /* Set timer to free running mode */
    if( adi_GPT_SetFreeRunningMode( hGPT ) != 0 ) {
        FAIL("adi_GPT_SetFreeRunningMode failed");
    }
}


void InitializeWUT(void)
{
    /* Initialise the wakeup timer */
    if( adi_WUT_Init(ADI_WUT_DEVID_0, &hWUT) != 0 ) {
        FAIL("adi_WUT_Init failed");
    }

    /* install WUT callback on all interrupts */
    if (adi_WUT_RegisterCallback(hWUT, WUT_Callback, ADI_WUT_TARGET_MASK)) {
        FAIL("adi_WUT_RegisterCallback failed");
    }

    /* Enable Wakeup timer in NVIC */
    NVIC_EnableIRQ(WUT_IRQn);

}


static void WUT_Callback(void* hWut, uint32_t Event, void* pArg)
{
    uint16_t status = adi_WUT_GetStatusRegister(hWut);

    /* check, clear and adjust timer capture index for expected interrupts */
    if (status & T2STA_WUFB) {
        adi_WUT_SetInterruptEnable(hWut, ADI_WUT_COMPB, false);
        pADI_WUT->T2CLRI |= T2CLRI_WUFB_CLR;
        captureIndex = CAPB;
    }

    if (status & T2STA_WUFC) {
        adi_WUT_SetInterruptEnable(hWut, ADI_WUT_COMPC, false);
        pADI_WUT->T2CLRI |= T2CLRI_WUFC_CLR;
        captureIndex = CAPC;
    }

    if (status & T2STA_WUFD) {
        adi_WUT_SetInterruptEnable(hWut, ADI_WUT_COMPD, false);
        pADI_WUT->T2CLRI |= T2CLRI_WUFD_CLR;
        captureIndex = CAPD;
    }

    /* Ensure that above register write is fully complete before
       returning from the interrupt */
    __DSB();  /* Data Syncronization barrier instruction */

}


static void GPT0_Callback(void* hGpt, uint32_t Event, void* pArg)
{
    /* store the captured value */
    adi_GPT_GetCapturedValue(hGpt, &captureValue[captureIndex]);

    /* Clear the interrupt */
    adi_GPT_ClearCapturedEventStatus(hGpt);

    /* set completion flag after COMPD capture */
    if (captureIndex == CAPD)
    	captureComplete = true;

    return;
}
