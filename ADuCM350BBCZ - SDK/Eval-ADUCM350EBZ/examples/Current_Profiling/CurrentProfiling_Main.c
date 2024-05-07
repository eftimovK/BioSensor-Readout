/*********************************************************************************

Copyright (c) 2013-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*****************************************************************************
 * @file:    CurrentProfiling.c
 * @brief:   Application examples of low power us of ADuCM350 to allow for 
 *           profiling of the supply current
 * @version: $Revision: 29403 $
 * @date:    $Date: 2015-01-08 10:34:56 -0500 (Thu, 08 Jan 2015) $
 *****************************************************************************/

#include "test_common.h"
#include "CurrentProfiling.h"

/*****************************************************************************
 *
 *  Local Global Variables
 *
 *****************************************************************************/

static      ADI_CP_STATE_TYPE   nextState = ADI_CP_INIT_STATE;
#pragma location = "nonvolatile_ram"
static      int16_t             dftResults[DFT_RESULTS_COUNT];
#pragma location = "nonvolatile_ram"
static      fixed32_t           magnitude_result[DFT_RESULTS_COUNT / 2 - 1];
#pragma location = "nonvolatile_ram"
static      fixed32_t           phase_result[DFT_RESULTS_COUNT / 2 - 1];

int main (void)
{

    /* Initialize system */
    SystemInit();
    
    /* Test initialization */
    test_Init();

    /* Initialisation State. This state is entered once at the beginning    */
    /* and is not re-entered.                                               */
    nextState = adi_CP_InitState();

    while (1) {
        /* Move to the next state */
        switch (nextState)
        {
            case ADI_CP_READY_STATE:
                nextState = adi_CP_ReadyState();
                break;
                
            case ADI_CP_AFE_MEASUREMENT_STATE:
                nextState = adi_CP_AfeMeasurementState(dftResults);
                break;
                
            case ADI_CP_ACTIVE_CORE_STATE:
                nextState = adi_CP_ActiveCoreState(dftResults, magnitude_result, phase_result);
                break;
                
            case ADI_CP_ACTIVE_DISPLAY_STATE:
                nextState = adi_CP_ActiveDisplayState(magnitude_result, phase_result);
                break;
                
            case ADI_CP_STATIC_DISPLAY_SYS_SLEEP_STATE:
                nextState = adi_CP_StaticDisplaySystemSleepState();
                break;
                
            case ADI_CP_HIBERNATE_STATE:
                nextState = adi_CP_HibernateState();
                break;
                
            case ADI_CP_INIT_STATE:
                /* Should not be re-entered */
                FAIL("INIT_STATE should not be re-entered");
                break;

            default:
                if (((0xF >= nextState) && (0x7 <= nextState)) || (0x0 == nextState))
                {
                    /* Unused rotary switch values, to be treated as Hibernate State */
                    nextState = adi_CP_HibernateState();
                }
                else
                {
                    /* Invalid State */
                    FAIL("Invalid State");
                }
                break;
        }
    }
}

