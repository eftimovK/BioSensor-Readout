
/*****************************************************************************/
#include <stdio.h>

#include "test_common.h"
#include "wdt.h"
#include "gpt.h"
#include "gpio.h"


/***************************************************************/
/* testing depends on hardware jumpers to conenct GPIO outputs */
/* setup as flags to GPIO inputs setup as interrupts.          */
/* Make the following external pin connections to run          */
/* EVAL-ADuCRF101MKxZ/Rev C board (RevD Silicon):              */
/*      connect P1.4 & P0.6                                    */
/*      connect P1.5 & P0.7                                    */
/*	Eval-ADUCM350-MOCZ must be placed across:                        */
/*      P1.0 & P1.2 (pins J2-14 & J2-18 of ADuCM350 Breakout Board)  */
/*      P1.1 & P1.3 (pins J2-16 & J2-20 of ADuCM350 Breakout Board)  */
/* EVAL-ADuM360EBZ/Rev A board:                                */
/*      connect P1.2 & P0.3                                    */
/*      connect P1.3 & P0.5                                    */
/***************************************************************/


/* actually, the number of GPT interrupts allowed before test termination     */
#define TEST_DURATION   3      /* Duration the stress test should run in secs */

ADI_GPT_HANDLE     hGPT0;   /* Handle to timer 0                             */
ADI_GPT_HANDLE     hGPT1;   /* Handle to timer 1                             */
ADI_WDT_DEV_HANDLE hWDT;

uint8_t  HiLo = 0;
uint16_t GPT1_InterruptCount = 0;

uint32_t edgeCount = 0;         /* Track the number of output edges           */

/* interrupt counters */
uint32_t irq2Count = 0;         /* IRQ2 interrupt count                       */
uint32_t irq3Count = 0;         /* IRQ3 interrupt count                       */

uint16_t GPT0_CaptureValue;       /* Storage for capturing WDT event value      */
uint8_t  GPT0_InterruptFlag = 0;  /* Flag to indicate GPT0 has triggered        */
uint32_t GPT0_InterruptCount = 0; /* Number of GPT0 interrupt timeouts          */

void Initialise_Timer0 (void);
void Initialise_Timer1 (void);
void Initialise_WDT    (void);
void Initialise_GPIO   (void);

/* callbacks */
void GPT0_Callback     (void *pCBParam, uint32_t Event, void *pArg);
void GPT1_Callback     (void *pCBParam, uint32_t Event, void *pArg);

typedef struct {
    ADI_GPIO_PORT_TYPE  Port;
    ADI_GPIO_MUX_TYPE   Muxing;
    ADI_GPIO_DATA_TYPE  Pins;
} PinMap;

// arrange inputs (interrupts) and outputs (flags)
PinMap Output = { ADI_GPIO_PORT_1, (ADI_GPIO_P10 | ADI_GPIO_P11), (ADI_GPIO_PIN_0 | ADI_GPIO_PIN_1) };

/* external interrupt inputs on ASDuCM350 are not avaliable on breakout board, so use Port1 pin interrupt group A */
PinMap Input =  { ADI_GPIO_PORT_1, (ADI_GPIO_P12 | ADI_GPIO_P13), (ADI_GPIO_PIN_2 | ADI_GPIO_PIN_3) };

void GroupA_Callback (void *pCBParam, uint32_t Event, void *pEventArg);

int main(int argc, char *argv[])
{
    int32_t i;

    /* Clock initialization */
    SystemInit();

    /* initialize the test console (depends on GPIO init) */
    test_Init();

    /* Initialize the GPIO service */
    if (adi_GPIO_Init()) {
        FAIL("adi_GPIO_Init failed");
    }

    /* Initialise WDT so we can setup the test */
    if (adi_WDT_Init(ADI_WDT_DEVID_0, &hWDT)) {
        FAIL("adi_WDT_Init failed");
    }

    /* Allow time for watchdog to sync */
    for (i=0; i<10000; i++);

    if (adi_WDT_SetEnable(hWDT, false)) {
        FAIL("adi_WDT_SetEnable failed");
    }

    /* Initialise general IO pins for testing */
    Initialise_GPIO();

    /* Initialise GP timer 0 to track the WDT */
    Initialise_Timer0();

    /* Initilaise GP timer 1 to track test time */
    Initialise_Timer1();

    /* Initialise and start the WDT */
    Initialise_WDT();

    /* Start the tests! */
    while (GPT1_InterruptCount < TEST_DURATION) {

		/* spawn an interrupt pair */
        if (adi_GPIO_SetHigh(Output.Port, Output.Pins)) {
			FAIL("adi_GPIO_SetHigh failed");
        }

        edgeCount++;

        for ( i=0; i<1000; i++ ) ;

		/* spawn another interrupt pair */
        if (adi_GPIO_SetLow(Output.Port, Output.Pins)) {
			FAIL("adi_GPIO_SetLow failed");
        }

        edgeCount++;

        for ( i=0; i<1000; i++  );
    }

    /* Check the results */
    if (edgeCount != irq2Count) {
    	FAIL("IRQ2 interrupt count failed");
    }

    if (edgeCount != irq3Count) {
    	FAIL("IRQ3 interrupt count failed");
    }

    /* If we get this far, then the test passes */
    PASS();
}


void Initialise_GPIO(void)
{
    if (adi_GPIO_SetOutputEnable(Output.Port, Output.Pins, true)) {
    	FAIL("Initialise_GPIO: adi_GPIO_SetOutputEnable failed");
    }
    if (adi_GPIO_SetLow(Output.Port, Output.Pins)) {
    	FAIL("Initialise_GPIO: adi_GPIO_SetLow failed");
    }

    /* ADuCM350 has dedicated input and output enables/disables */
    if (adi_GPIO_SetInputEnable(Input.Port, Input.Pins, true)) {
    	FAIL("Initialise_GPIO: adi_GPIO_SetInputEnable failed");
    }
    /* and disable output */
    if (adi_GPIO_SetOutputEnable(Input.Port, Input.Pins, false)) {
    	FAIL("Initialise_GPIO: adi_GPIO_SetOutputEnable failed");
    }

    /* for ADuCM350, configure singular GPIO GroupA interrupt to capture both input pins */

    /* this is because Port0 (external interrupts) are not avaliable on the break-out board */

    /* set the input port's GroupA interrupt pin mask */
    if (adi_GPIO_SetGroupInterruptPins(Input.Port, GPIOA_IRQn, Input.Pins)) {
    	FAIL("Initialise_GPIO: adi_GPIO_SetGroupInterruptPins failed");
    }

    /* configure to capture low-to-high capture (only one edge or the other avaliable here; not both) */
    if (adi_GPIO_SetGroupInterruptPolarity(Input.Port, Input.Pins)) {
    	FAIL("Initialise_GPIO: adi_GPIO_SetGroupInterruptPolarity failed");
    }

    if(adi_GPIO_RegisterCallback(GPIOA_IRQn, GroupA_Callback, NULL)) {
      FAIL("Initialise_GPIO: adi_GPIO_RegisterCallback failed");
    }

    /* enable the GPIO GroupA interrupt */
    NVIC_EnableIRQ(GPIOA_IRQn);
}


void Initialise_Timer0(void)
{
    int32_t i;

    /* initialise timer 0 */
    if (adi_GPT_Init(ADI_GPT_DEVID_0, &hGPT0)) {
    	FAIL("Initialise_Timer0: adi_GPT_Init failed");
    }

	/* register GPT0 callback */
	if (adi_GPT_RegisterCallback(hGPT0, GPT0_Callback, hGPT0)) {
    	FAIL("Initialise_Timer0: adi_GPT_RegisterCallback failed");
	}

    /* Allow some time for the GPT to synch */
    for (i=0; i<10000; i++);

    /* Enalbe timer0 in NVIC */
    NVIC_EnableIRQ(TIMER0_IRQn);

    /* Set GP Timer0 to capture the WDT */
    if (adi_GPT_SetEventToCapture( hGPT0, ADI_GPT_0_CAPTURE_EVENT_WDTIMER)) {
    	FAIL("Initialise_Timer0: adi_GPT_SetEventToCapture failed");
    }

    if (adi_GPT_SetCaptureEventEnable( hGPT0, true)) {
    	FAIL("Initialise_Timer0: adi_GPT_SetCaptureEventEnable failed");
    }

    /* Timer 0 to count up */
    if (adi_GPT_SetCountMode(hGPT0, ADI_GPT_COUNT_UP)) {
    	FAIL("Initialise_Timer0: adi_GPT_SetCountMode failed");
    }

    /* Set Timer0 to free running */
    if (adi_GPT_SetFreeRunningMode(hGPT0)) {
    	FAIL("Initialise_Timer0: adi_GPT_SetFreeRunningMode failed");
    }

    /* select 32kHz clock source to be the same as the wdt source*/
    if (adi_GPT_SetClockSelect(hGPT0, ADI_GPT_CLOCK_SELECT_32KHZ_INTERNAL_CLOCK)) {
    	FAIL("Initialise_Timer0: adi_GPT_SetClockSelect failed");
    }

    if (adi_GPT_SetPrescaler(hGPT0, ADI_GPT_PRESCALER_1)) {
    	FAIL("Initialise_Timer0: adi_GPT_SetPrescaler failed");
    }

    if (adi_GPT_SetTimerEnable(hGPT0, true)) {
    	FAIL("Initialise_Timer0: adi_GPT_SetTimerEnable failed");
    }
}


void Initialise_Timer1(void)
{
    int32_t i;

    /* initialise timer 0 */
    if (adi_GPT_Init(ADI_GPT_DEVID_1, &hGPT1)) {
    	FAIL("Initialise_Timer1: adi_GPT_Init failed");
    }

	/* register GPT1 callback */
	if (adi_GPT_RegisterCallback(hGPT1, GPT1_Callback, hGPT1)) {
    	FAIL("Initialise_Timer0: adi_GPT_RegisterCallback failed");
	}

    /* Allow some time for the GPT to synch */
    for (i=0; i<10000; i++);

    /* Enalbe timer1 in NVIC */
    NVIC_EnableIRQ(TIMER1_IRQn);

    /* Timer 1 to count up */
    if (adi_GPT_SetCountMode(hGPT1, ADI_GPT_COUNT_DOWN)) {
    	FAIL("Initialise_Timer1: adi_GPT_SetCountMode failed");
    }

    /* select UCLK clock source */
    if (adi_GPT_SetClockSelect(hGPT1, ADI_GPT_CLOCK_SELECT_PCLK)) {
    	FAIL("Initialise_Timer1: adi_GPT_SetClockSelect failed");
    }

    if (adi_GPT_SetPrescaler(hGPT1, ADI_GPT_PRESCALER_256)) {
    	FAIL("Initialise_Timer1: adi_GPT_SetClockSelect failed");
    }

    if (adi_GPT_SetPeriodicMode(hGPT1, true, 0xF423)) {
    	FAIL("Initialise_Timer1: adi_GPT_SetPeriodicMode failed");
    }

    if (adi_GPT_SetTimerEnable(hGPT1, true)) {
    	FAIL("Initialise_Timer1: adi_GPT_SetTimerEnable failed");
    }
}


void Initialise_WDT(void)
{
    /* Enable IRQ mode, for testing (since we don't want it to really reset the processor!) */
    if (adi_WDT_SetIRQMode(hWDT, true)) {
    	FAIL("Initialise_WDT: adi_WDT_SetIRQMode failed");
    }

    /* the following settings give a maximum timeout of 2 seconds */
    if (adi_WDT_SetPrescale(hWDT, ADI_WDT_PRESCALE_1)) {
    	FAIL("Initialise_WDT: adi_WDT_SetPrescale failed");
    }

    if (adi_WDT_SetLoadCount(hWDT, 0x007F)) {
    	FAIL("Initialise_WDT: adi_WDT_SetLoadCount failed");
    }

    /* Enable the WDT */
    if (adi_WDT_SetEnable(hWDT, true)) {
    	FAIL("Initialise_WDT: adi_WDT_SetEnable failed");
    }
}


void GroupA_Callback (void *pCBParam, uint32_t Event, void *pEventArg)
{
    /* handle the shared GPIO GroupA interrupt */
    ADI_GPIO_DATA_TYPE pins;

    /* read the interrupt status */
    pins = adi_GPIO_GetGroupInterruptStatus(Input.Port);

    /* clear the interrupt status */
    adi_GPIO_ClrGroupInterruptStatus(Input.Port, pins);

    /* bump the counter(s) for the expected interrupts */
    /* double-count because we're only catching one edge in Group A/B mode */
    if (ADI_GPIO_PIN_2 & pins) irq2Count+=2;
    if (ADI_GPIO_PIN_3 & pins) irq3Count+=2;
}

/* WDT interrupt callback */
static void GPT0_Callback( void *hDevice, uint32_t Event, void *pArg )
{
    /* Check what caused the interrupt */
    if (adi_GPT_GetCaptureEventPending(hDevice)) {

        if (adi_GPT_GetCapturedValue(hDevice, &GPT0_CaptureValue)) {
    		FAIL("GPT0_Callback: adi_GPT_GetCapturedValue failed");
        }

        GPT0_InterruptFlag = 1;

        if (adi_GPT_ClearCapturedEventStatus(hDevice)) {
    		FAIL("GPT0_Callback: adi_GPT_ClearCapturedEventStatus failed");
        }

		/* bump the interrupt count */
        GPT0_InterruptCount++;
    }
}


/* GPT1 interrupt callback */
static void GPT1_Callback( void *hDevice, uint32_t Event, void *pArg )
{
	/* bump the interrupt count */
    GPT1_InterruptCount++;
}
