/*********************************************************************************

Copyright (c) 2012-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*****************************************************************************
 * @file:    Captouch_5ButtonsTest.c
 * @brief:   CT Test for ADuCM350
 * @version: $Revision: 28525 $
 * @date:    $Date: 2014-11-12 14:51:26 -0500 (Wed, 12 Nov 2014) $
 *****************************************************************************/

/*! \addtogroup CT_Test CT Test
 *  .
 *  @{
 */

/* Apply ADI MISRA Suppressions */
#define ASSERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"

#include <stddef.h>		/* for 'NULL' */
#include <string.h>		/* for strlen */

#include "test_common.h"

#include "captouch.h"
#include "captouch_lib.h"

/*!
 * @brief       Function 'main' for CT example program
 *
 * @param       none
 * @return      int (Zero for success, non-zero for failure).
 *
 * @details     This is a simple Captouch application that configures 5 Captouch Buttons.

                The Up, Down, Left, Right and Enter Buttons are configured to generate Touch and
                Release Interrupts. So, the Touch and Release Detection Algorithm is enable.

                To detect the Touch Event Event with the current configuration, the user must press
                the button around 90ms (it can be modified).

                Anytime a Touch or a Release Interrupt happen the system check the Button Status using a
                function defined in the captouch driver, adi_CT_GetTchAndRelAlgorithmStatus.

                The system sends a message by UART anytime one button is touched or released,
                indicating which button generated the event.

                To finish the test the user must touch three buttons at the same time.
 *
 */



/* CT device Handle */
ADI_CT_DEV_HANDLE hDevice = NULL;
uint8_t  CaptouchEvents;
bool_t CTEventFlag=false;

/* prototypes */
extern int32_t adi_initpinmux (void);
void ct_AutoCAPDAC_adjust(ADI_CT_DEV_HANDLE const hCaptouchDevice);

void ctCallback (void *pCBParam, uint32_t status, void *pArg);

#define BUTTON_UP      ADI_CT_STATUS_BIT_B
#define BUTTON_LEFT    ADI_CT_STATUS_BIT_C
#define BUTTON_ENTER   ADI_CT_STATUS_BIT_D
#define BUTTON_RIGHT   ADI_CT_STATUS_BIT_E
#define BUTTON_DOWN    ADI_CT_STATUS_BIT_F



int main(void)
{


    ADI_CT_INTERRUPT_TYPE callbacks = (ADI_CT_INTERRUPT_TYPE)
                                      ( ADI_CT_INTERRUPT_CONV_SET_COMPLETE
                                      | ADI_CT_INTERRUPT_TOUCH_DETECTED
                                      | ADI_CT_INTERRUPT_RELEASE_DETECTED);



    uint8_t  ButtonStatus               = 0;
    uint8_t  PreviousButtonStatus       = 0;
    uint8_t  TouchButtonStatus          = 0;
    uint8_t  TouchCounter               = 0;

    /* Initialize system */
    SystemInit();

    /* initialize the test console */
    test_Init();

    SetSystemClockDivider(ADI_SYS_CLOCK_CORE, 1);
    
    adi_initpinmux();

    /* Initialize the CT API */
    if (adi_CT_Init(ADI_CT_DEV_ID_0,&hDevice)) {
        FAIL("Init");
    }

    /* register callback handler for CT interrupts */
    adi_CT_RegisterCallback(hDevice, ctCallback, callbacks);
    
    /* Auto baseline adjustment using the CAPDAC*/
    ct_AutoCAPDAC_adjust(hDevice);
    

    /* Power Up the Captouch  */
    if(adi_CT_SetPowerStatus(hDevice,true)){
        FAIL("Power UP");
    }

    /* START THE EXAMPLE */
    PERF("Start the Test: 5 BUTTONS EXAMPLE \n");
    PERF("Touch 3 buttons at the same time to exit from Test\n");


    while(1)

    {

      /*  Wait to a Captouch Event */
      if(CTEventFlag)
      {

        CTEventFlag=false;

        /* Check what type of event happened */
        if((CaptouchEvents & ADI_CT_INTERRUPT_TOUCH_DETECTED) || (CaptouchEvents & ADI_CT_INTERRUPT_RELEASE_DETECTED))
        {
          /* TOUCH EVENT or RELEASE EVENT */

          /* Read the Current Button Status */
          adi_CT_GetTchAndRelAlgorithmStatus(hDevice,&ButtonStatus);

          /* If the previous status is Touch and the current status is Release, it means we have a Touch and Release Event */
          TouchButtonStatus = (~(ButtonStatus) & PreviousButtonStatus);


          /* Detect if any button is being touched */
          if((ButtonStatus & BUTTON_UP) & ~(PreviousButtonStatus))
          {
            PERF("UP      is being Touched\n");
            TouchCounter++;
          }

          if((ButtonStatus & BUTTON_DOWN) & ~(PreviousButtonStatus))
          {
            PERF("DOWN    is being Touched\n");
            TouchCounter++;
          }

          if((ButtonStatus & BUTTON_ENTER) & ~(PreviousButtonStatus))
          {

            PERF("ENTER    is being Touched\n");
            TouchCounter++;
          }

          if((ButtonStatus & BUTTON_RIGHT) & ~(PreviousButtonStatus))
          {
            PERF("RIGHT   is being Touched\n");
            TouchCounter++;
          }

          if((ButtonStatus & BUTTON_LEFT) & ~(PreviousButtonStatus))
          {
            PERF("LEFT   is being Touched\n");
            TouchCounter++;
          }

          /* Detect if any button has been released */
          if(TouchButtonStatus & BUTTON_UP)
          {
            PERF("UP      has been Released\n");
            TouchCounter--;
          }

          if(TouchButtonStatus & BUTTON_DOWN)
          {
            PERF("DOWN    has been Released\n");
            TouchCounter--;
          }

          if(TouchButtonStatus & BUTTON_ENTER)
          {

            PERF("ENTER    has been Released\n");
            TouchCounter--;
          }

          if(TouchButtonStatus & BUTTON_RIGHT)
          {
            PERF("RIGHT   has been Released\n");
            TouchCounter--;
          }

          if(TouchButtonStatus & BUTTON_LEFT)
          {
            PERF("LEFT   has been Released\n");
            TouchCounter--;
          }


          /* If 3 or more buttons are pressed at the same time the test finishes*/
          if(TouchCounter >= 3)
          {
            PASS();
          }

          /* Save the button status as the new previous status */
          PreviousButtonStatus = ButtonStatus;

        }

      }

    }

}

void ct_AutoCAPDAC_adjust(ADI_CT_DEV_HANDLE const hCaptouchDevice)

{

    /* Auto CapDAC adjustfor Stage 1 */
    adi_CT_Stage_AutoCAPDAC_Adjustment(hCaptouchDevice,ADI_CT_STAGE1_NUMBER,ADI_CT_DEFAULT_BASELINE_VALUE);

    /* Auto CapDAC adjustfor Stage 2 */
    adi_CT_Stage_AutoCAPDAC_Adjustment(hCaptouchDevice,ADI_CT_STAGE2_NUMBER,ADI_CT_DEFAULT_BASELINE_VALUE);

    /* Auto CapDAC adjustfor Stage 3 */
    adi_CT_Stage_AutoCAPDAC_Adjustment(hCaptouchDevice,ADI_CT_STAGE3_NUMBER,ADI_CT_DEFAULT_BASELINE_VALUE);

    /* Auto CapDAC adjustfor Stage 4 */
    adi_CT_Stage_AutoCAPDAC_Adjustment(hCaptouchDevice,ADI_CT_STAGE4_NUMBER,ADI_CT_DEFAULT_BASELINE_VALUE);

    /* Auto CapDAC adjustfor Stage 5 */
    adi_CT_Stage_AutoCAPDAC_Adjustment(hCaptouchDevice,ADI_CT_STAGE5_NUMBER,ADI_CT_DEFAULT_BASELINE_VALUE);

    adi_CT_SetAutoForceBaselineCal(hCaptouchDevice);


}


/* Callback handler */
void ctCallback (void *pCBParam,uint32_t status, void *pArg){

    /* process CT interrupts (cleared by driver) */

    if ((ADI_CT_INTERRUPT_TOUCH_DETECTED | ADI_CT_INTERRUPT_RELEASE_DETECTED) & status) {

        CTEventFlag    = true;
        CaptouchEvents = status;
        adi_CT_ClearInterruptFlag(hDevice);

    }


}

/* Revert ADI MISRA Suppressions */
#define REVERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"

/*
** EOF
*/

/*@}*/
