/*********************************************************************************

Copyright (c) 2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*****************************************************************************
 * @file:    AfeCustomInterruptExample.c
 * @brief:   Simple example triggering the CUSTOM_INTERRUPT from a sequence
 * @version: $Revision: 28525 $
 * @date:    $Date: 2014-11-12 14:51:26 -0500 (Wed, 12 Nov 2014) $
 *****************************************************************************/

#include <stdio.h>

#include "afe.h"
#include "afe_lib.h"
#include "gpio.h"

#include "test_common.h"

static void CustomInterruptCallback (void *pCBParam, uint32_t Event, void *pArg);

uint32_t seq_trigger_custom_int[] = {
    0x0005009A,     /*  0 - Safety word: bits 31:16 = command count, bits 7:0 = CRC             */
    0x00000C80,     /*  1 - Wait 200us. Simulating hardware setup.                              */
    0xD2000008,     /*  2 - AFE_ANALOG_GEN_INT: CUSTOM_INT = 1. Trigger interrupt.              */
    0x01E84800,     /*  3 - Wait 2s. Simulating measurement. LED will light during this time.   */
    0xD2000008,     /*  4 - AFE_ANALOG_GEN_INT: CUSTOM_INT = 1. Trigger interrupt.              */
    0x82000002,     /*  5 - AFE_SEQ_CFG: SEQ_EN = 0                                             */
};

int main(void) {
    ADI_AFE_DEV_HANDLE  hDevice;

    /* Initialize system */
    SystemInit();

    /* Change the system clock source to HFXTAL and change clock frequency to 16MHz     */
    /* Requirement for AFE sequencer clock (ACLK)                                       */
    SystemTransitionClocks(ADI_SYS_CLOCK_TRIGGER_MEASUREMENT_ON);

    /* Test initialization */
    test_Init();

    /* Initialize GPIO */
    if (ADI_GPIO_SUCCESS != adi_GPIO_Init())
    {
        FAIL("adi_GPIO_Init");
    }

    /* Enable GPIO output on P0.11 (DISPLAY LED) */
    if (ADI_GPIO_SUCCESS != adi_GPIO_SetOutputEnable(ADI_GPIO_PORT_0, ADI_GPIO_PIN_11, true))
    {
        FAIL("adi_GPIO_SetOutputEnable");
    }

    /* Turn off DISPLAY LED */
    if (ADI_GPIO_SUCCESS != adi_GPIO_SetHigh(ADI_GPIO_PORT_0, ADI_GPIO_PIN_11))
    {
        FAIL("adi_GPIO_SetHigh");
    }

    /* Initialize the AFE API */
    if (ADI_AFE_SUCCESS != adi_AFE_Init(&hDevice)) 
    {
        FAIL("adi_AFE_Init");
    }

    /* AFE power up */
    if (ADI_AFE_SUCCESS != adi_AFE_PowerUp(hDevice)) 
    {
        FAIL("adi_AFE_PowerUp");
    }
    
    /* Register callback for CUSTOM_INTERRUPT interrupts */
    if (ADI_AFE_SUCCESS != adi_AFE_RegisterAfeCallback(hDevice, ADI_AFE_INT_GROUP_GENERATE, CustomInterruptCallback, BITM_AFE_AFE_ANALOG_GEN_INT_CUSTOM_INT))
    {
        FAIL("adi_AFE_RegisterAfeCallback");
    }
    
    /* Clear CUSTOM_INTERRUPT interrupts */
    if (ADI_AFE_SUCCESS != adi_AFE_ClearInterruptSource(hDevice, ADI_AFE_INT_GROUP_GENERATE, BITM_AFE_AFE_ANALOG_GEN_INT_CUSTOM_INT))
    {
        FAIL("adi_AFE_ClearInterruptSource");
    }

    /* Enable CUSTOM_INTERRUPT interrupt */
    if (ADI_AFE_SUCCESS != adi_AFE_EnableInterruptSource(hDevice, ADI_AFE_INT_GROUP_GENERATE, BITM_AFE_AFE_ANALOG_GEN_IEN_CUSTOM_INT_IEN, true))
    {
        FAIL("adi_AFE_EnableInterruptSource");
    }
    
    /* Run the sequence */
    if (adi_AFE_RunSequence(hDevice, seq_trigger_custom_int, NULL, 0)) 
    {
        FAIL("adi_AFE_RunSequence");
    }

    /* Disable CUSTOM_INTERRUPT interrupt */
    if (ADI_AFE_SUCCESS != adi_AFE_EnableInterruptSource(hDevice, ADI_AFE_INT_GROUP_GENERATE, BITM_AFE_AFE_ANALOG_GEN_IEN_CUSTOM_INT_IEN, false))
    {
        FAIL("adi_AFE_EnableInterruptSource");
    }
    
    /* AFE Power Down */
    if (adi_AFE_PowerDown(hDevice)) {
        FAIL("PowerDown");
    }

    /* Uninitialize the AFE API */
    if (adi_AFE_UnInit(hDevice)) {
        FAIL("Init");
    }

    /* Uninitialize the GPIO API */
    if (ADI_GPIO_SUCCESS != adi_GPIO_UnInit())
    {
        FAIL("adi_GPIO_UnInit");
    }

    PASS();
}

/* CUSTOM_INTERRUPT callback function */
static void CustomInterruptCallback (void *pCBParam, uint32_t Event, void *pArg) {
    /* Toggle DISPLAY LED  */
    adi_GPIO_Toggle(ADI_GPIO_PORT_0, ADI_GPIO_PIN_11);
}
