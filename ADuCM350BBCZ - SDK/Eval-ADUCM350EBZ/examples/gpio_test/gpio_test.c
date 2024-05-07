#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

#include "test_common.h"
#include "gpio.h"

#define ITERATIONS   100
#define TIMEOUT      100000

volatile unsigned char MagicNumber;
#define MAGIC_NUMBER     0x77
#define MAGIC_NUMBER_A   0xAA
#define MAGIC_NUMBER_B   0xBB

void Test_GPIO_Polling(void);
void Test_GPIO_Interrupt(void);

void Test_GPIO_GroupInterrupt(void);

/***************************************************************************
                   !!!HARDWARE JUMPERS REQUIRED!!!


        External jumpers on EVAL-ADuCRF101MKxZ must be placed across:
            P1.2 & P0.3
            P1.3 & P0.4
        External jumpers on Eval-ADUCM350-EBZ must be placed across:
            Attach ADuCM350 Breakout Board to Digital Header 2
            P4.0 & P4.2 (pins J3-30 & J3-34 of ADuCM350 Breakout Board)
        External jumpers on EVAL-ADuM360EBZ must be placed across:
            P1.2 & P0.3
            P1.3 & P0.5
        External jumpers on EVAL-ADuM320EBZ must be placed across:
	        P0.6 & P0.5
	        P0.4 & P0.3
***************************************************************************/

typedef struct {
    ADI_GPIO_PORT_TYPE Port;
    ADI_GPIO_MUX_TYPE  Muxing;
    ADI_GPIO_DATA_TYPE Pins;
} PinMap;

#define         INTERRUPT_ID        EINT0_IRQn

PinMap Output = { ADI_GPIO_PORT_4, (ADI_GPIO_P42), (ADI_GPIO_PIN_2) };
PinMap Input =  { ADI_GPIO_PORT_4, (ADI_GPIO_P40), (ADI_GPIO_PIN_0) };

static void CallbackHandler (void *pCBParam, uint32_t Event, void *pArg);

int main(void)
{

    /* Clock initialization */
    SystemInit();

     /* initialize the test console */
    test_Init();

    /* initialize the GPIO */
    if (adi_GPIO_Init()) {
        FAIL("main: adi_GPIO_Init failed");
    }

    if (adi_GPIO_SetOutputEnable(Output.Port, Output.Pins, true)) {
        FAIL("main: adi_GPIO_SetOutputEnable failed");
    }

    /* ADuCM350 has dedicated input and output enables that are seperate */
    if (adi_GPIO_SetInputEnable(Input.Port, Input.Pins, true)) {
        FAIL("main: adi_GPIO_SetInputEnable failed");
    }

    /* Test pin configuration by polling */
    Test_GPIO_Polling();

    /* Test pin configuration by interrupt */
    Test_GPIO_Interrupt();

    Test_GPIO_GroupInterrupt();

    if (adi_GPIO_UnInit()) {
        FAIL("main: adi_GPIO_UnInit failed");
    }

    /* If we get this far, then the test passes */
    PASS();
}


void Test_GPIO_Polling(void)
{
    unsigned int i,j;
    ADI_GPIO_DATA_TYPE Data;

    /* Un-register the external pin callback handler */
    if(adi_GPIO_RegisterCallback(INTERRUPT_ID, NULL, NULL)) {
        FAIL("Test_GPIO_Polling: adi_GPIO_RegisterCallback failed");
    }

    for(j=0; j<ITERATIONS; j++) {

        /* Set outputs low */
        if (adi_GPIO_SetLow(Output.Port, Output.Pins)) {
            FAIL("Test_GPIO_Polling: adi_GPIO_SetLow failed");
        }

        /* Wait for the pin to stabilise */
        for( i=0; i<0xFFF; i++ ) ;

        /* Do a port readback */
        Data = adi_GPIO_GetData(Input.Port) & Input.Pins;
        if ( Data != 0 ) {
            FAIL("Test_GPIO_Polling: data mismatch, check jumpers");
        }

        /* Set outputs high */
        if (adi_GPIO_SetHigh(Output.Port, Output.Pins)) {
            FAIL("Test_GPIO_Polling: adi_GPIO_SetHigh failed");
        }

        /* Wait for the pin to stabilise */
        for( i=0; i<0xFFF; i++ );

        /* Do a port readback */
        Data = adi_GPIO_GetData(Input.Port) & Input.Pins;
        if ( Data != Input.Pins ) {
            FAIL("Test_GPIO_Polling: data mismatch, check jumpers");
        }
    }
}

/* uses External interrupt to test GPIO interrupting */
void Test_GPIO_Interrupt(void)
{
    unsigned int i;
    ADI_GPIO_DATA_TYPE Data;

    /* Initialise output pins to a known state  */
        if (adi_GPIO_SetLow(Output.Port, Output.Pins)) {
            FAIL("Test_GPIO_Interrupt: adi_GPIO_SetLow failed");
    }

    /* Ext interrupt test #1 - test for low-to-high transition */
    MagicNumber = 0;


    /* Register the callback */
    if(adi_GPIO_RegisterCallback(INTERRUPT_ID, CallbackHandler, NULL)) {
        FAIL("Test_GPIO_Interrupt: adi_GPIO_RegisterCallback failed");
    }


    /* enable rising-edge interrupt */
    if (adi_GPIO_EnableIRQ(INTERRUPT_ID, ADI_GPIO_IRQ_RISING_EDGE)) {
        FAIL("Test_GPIO_Interrupt: adi_GPIO_EnableIRQ failed");
    }

    /* spawn the interrupt */
    if (adi_GPIO_SetHigh(Output.Port, Output.Pins)) {
        FAIL("Test_GPIO_Interrupt: adi_GPIO_SetHigh failed");
    }

    /* Wait for interrupt to occur but timeout when there's no interrupts */
    for(i=0; i<TIMEOUT; i++){
        if (MagicNumber == MAGIC_NUMBER)
           break;
    }

	/* Check if an interrupt has occured */
	if ( MagicNumber == MAGIC_NUMBER ) {
		/* Verify that the input pins are high */
		Data = adi_GPIO_GetData(Input.Port) & Input.Pins;

		if ( Data != Input.Pins ) {
			FAIL("Test_GPIO_Interrupt: data mismatch, check jumpers");
		}
	} else {
		FAIL("Test_GPIO_Interrupt: no interrupt occured");
	}


    /* Ext interrupt test #2 - test for high-to-low transition */
    MagicNumber = 0;

    /* switch to rising-edge interrupt */
    if (adi_GPIO_EnableIRQ(INTERRUPT_ID, ADI_GPIO_IRQ_FALLING_EDGE)) {
	FAIL("Test_GPIO_Interrupt: adi_GPIO_EnableIRQ failed");
    }

	/* set outputs low */
    if (adi_GPIO_SetLow(Output.Port, Output.Pins)) {
        FAIL("Test_GPIO_Interrupt: adi_GPIO_SetLow failed");
    }

    /* Wait for interrupt to occur but timeout when there's no interrupts */
    for( i=0; i<TIMEOUT; i++ ){
        if ( MagicNumber == MAGIC_NUMBER )
            break;
    }

    /* Check if an interrupt has occured */
    if ( MagicNumber == MAGIC_NUMBER ) {
        /* Verify that the input pins are low */
        Data = adi_GPIO_GetData(Input.Port) & Input.Pins;

        if ( Data != 0x00 ) {
            FAIL("Test_GPIO_Interrupt: data mismatch, check jumpers");
        }
        } else {
        FAIL("Test_GPIO_Interrupt: no interrupt occured");
    }

    if(adi_GPIO_DisableIRQ (INTERRUPT_ID)) {
        FAIL("Test_GPIO_Interrupt: adi_GPIO_DisableIRQ failed");
    }
}

/* Uses Group interrupt to test GPIO interrupting */
void Test_GPIO_GroupInterrupt(void)
{
    unsigned int i;
    ADI_GPIO_DATA_TYPE Data;

    /* Initialise output pins to a known state  */
    if (adi_GPIO_SetLow(Output.Port, Output.Pins)) {
        FAIL("Test_GPIO_GroupInterrupt: adi_GPIO_SetLow failed");
    }

    /* Register the callback for Group A */
    if(adi_GPIO_RegisterCallback (GPIOA_IRQn, CallbackHandler, NULL)) {
      FAIL("Test_GPIO_GroupInterrupt: adi_GPIO_RegisterCallback failed");
    }

    /* Register the callback Group B*/
    if(adi_GPIO_RegisterCallback (GPIOB_IRQn, CallbackHandler, NULL)) {
      FAIL("Test_GPIO_GroupInterrupt: adi_GPIO_RegisterCallback failed");
    }

    /* configure to capture low-to-high capture (only one edge or the other avaliable here; not both) */
    if (adi_GPIO_SetGroupInterruptPolarity(Input.Port, Input.Pins)) {
    	FAIL("Test_GPIO_GroupInterrupt: adi_GPIO_SetGroupInterruptPolarity failed");
    }

    /* enable the GPIO GroupA interrupt */
    NVIC_EnableIRQ(GPIOA_IRQn);

    /* enable the GPIO GroupB interrupt */
    NVIC_EnableIRQ(GPIOB_IRQn);


    /* Group interrupt test #1 - Test mapping input port pin to generate Group A interrupt */
    MagicNumber = 0;

    /* set the input port's GroupA interrupt pin mask */
    if (adi_GPIO_SetGroupInterruptPins(Input.Port, GPIOA_IRQn, Input.Pins)) {
    	FAIL("Test_GPIO_GroupInterrupt: adi_GPIO_SetGroupInterruptPins failed");
    }

    /* spawn the interrupt */
    if (adi_GPIO_SetHigh(Output.Port, Output.Pins)) {
        FAIL("Test_GPIO_GroupInterrupt: adi_GPIO_SetHigh failed");
    }

    /* Wait for interrupt to occur but timeout when there's no interrupts */
    for(i=0; i<TIMEOUT; i++){
       if (MagicNumber != 0)
          break;
    }

    /* Check if an interrupt has occured */
    if ( MagicNumber == MAGIC_NUMBER_A ) {

        /* Verify that the input pins are high */
        Data = adi_GPIO_GetData(Input.Port) & Input.Pins;

        if ( Data != Input.Pins ) {
            FAIL("Test_GPIO_GroupInterrupt: data mismatch, check jumpers");
        }

    } else {
        FAIL("Test_GPIO_GroupInterrupt: no interrupt occured");
    }


    /* Group interrupt test #1 - Test mapping input port pin to generate Group B interrupt */
    MagicNumber = 0;

    /* Initialise output pins to a known state  */
    if (adi_GPIO_SetLow(Output.Port, Output.Pins)) {
        FAIL("Test_GPIO_GroupInterrupt: adi_GPIO_SetLow failed");
    }

    /* set the input port's GroupB interrupt pin mask */
    if (adi_GPIO_SetGroupInterruptPins(Input.Port, GPIOB_IRQn, Input.Pins)) {
    	FAIL("Test_GPIO_GroupInterrupt: adi_GPIO_SetGroupInterruptPins failed");
    }

    /* spawn the interrupt */
    if (adi_GPIO_SetHigh(Output.Port, Output.Pins)) {
        FAIL("Test_GPIO_GroupInterrupt: adi_GPIO_SetHigh failed");
    }

    /* Wait for interrupt to occur but timeout when there's no interrupts */
    for(i=0; i<TIMEOUT; i++){
       if (MagicNumber != 0)
          break;
    }

    /* Check if an interrupt has occured */
    if ( MagicNumber == MAGIC_NUMBER_B ) {

        /* Verify that the input pins are high */
        Data = adi_GPIO_GetData(Input.Port) & Input.Pins;

        if ( Data != Input.Pins ) {
            FAIL("Test_GPIO_GroupInterrupt: data mismatch, check jumpers");
        }

    } else {
        FAIL("Test_GPIO_GroupInterrupt: no interrupt occured");
    }
}


/* Common GPIO callhack handler shared by all interrupts */
static void CallbackHandler (void *pCBParam, uint32_t interruptID, void *pArg)
{
    /* process callback by type */
    switch (interruptID) {

        /* dedicated external pin interrupts */
        case EINT0_IRQn:
        case EINT1_IRQn:
        case EINT2_IRQn:
        case EINT3_IRQn:
        case EINT4_IRQn:
        case EINT5_IRQn:
        case EINT6_IRQn:
        case EINT7_IRQn:
        case EINT8_IRQn:

        	/* clear the pin (singular, and add the offset for IRQn) */
			adi_GPIO_ClearIRQ(interruptID);

			MagicNumber = MAGIC_NUMBER;
        	break;

        case GPIOA_IRQn: /* shared GroupA interrupt */


        	/* set the flag */
		MagicNumber = MAGIC_NUMBER_A;
        	break;

        case GPIOB_IRQn: /* shared GroupB interrupt */

        	/* set the flag */
   	        MagicNumber = MAGIC_NUMBER_B;
        	break;

        default:

            FAIL("CallbackHandler: unexpected GPIO interrupt callback");
        	break;
    }
}
