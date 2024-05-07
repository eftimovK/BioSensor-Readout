/*********************************************************************************

Copyright (c) 2011-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*****************************************************************************/
/*!
  @file:    GpioTest.c
  @brief:   GPIO Example for ADuCxxx
  @version: $Revision: 29226 $
  @date:    $Date: 2014-12-15 09:17:48 -0500 (Mon, 15 Dec 2014) $
*/
/*****************************************************************************/

/** \addtogroup GPIO_Test GPIO Test
 *  Example code demonstrating use of the GPIO functions.
 *  @{
 */

#include <stdint.h>
#include <stdio.h>

#include "test_common.h"
#include "gpio.h"

/* 
 *  Setup for ADuCM350EBZ target board:
 *  Install jumper LK14
 */

static ADI_GPIO_RESULT_TYPE TestGPIOPins           (const uint32_t LoopCount);
static ADI_GPIO_RESULT_TYPE TestInterruptFunctions (const uint32_t LoopCount);

#ifdef ADI_DEBUG
// these tests do negative testing that depend on ADI_DEBUG code, so don't run them in release mode
static ADI_GPIO_RESULT_TYPE InitializationTests    (const uint32_t LoopCount);
static ADI_GPIO_RESULT_TYPE ParameterCheckingTests (const uint32_t LoopCount);
#endif

#define LOOPCOUNT (1)   /*!< number of times to execute the tests */

/* GPIOEntry and GPIOs are only used in TestGPIOPins() */
/* They are defined as globals because the size is     */
/* too big to be allocated on the stack.               */
typedef struct {
    ADI_GPIO_PORT_TYPE Port;
    ADI_GPIO_MUX_TYPE  MuxValue;
    ADI_GPIO_DATA_TYPE PinValue;
} GPIOEntry;

GPIOEntry   GPIOs[] = {

    { ADI_GPIO_PORT_0,  ADI_GPIO_P00,  ADI_GPIO_PIN_0 },
    { ADI_GPIO_PORT_0,  ADI_GPIO_P01,  ADI_GPIO_PIN_1 },
    { ADI_GPIO_PORT_0,  ADI_GPIO_P02,  ADI_GPIO_PIN_2 },
    { ADI_GPIO_PORT_0,  ADI_GPIO_P03,  ADI_GPIO_PIN_3 },
    { ADI_GPIO_PORT_0,  ADI_GPIO_P04,  ADI_GPIO_PIN_4 },
    { ADI_GPIO_PORT_0,  ADI_GPIO_P05,  ADI_GPIO_PIN_5 },
#if 0  // don't test emulator pins, or we lose connection
    { ADI_GPIO_PORT_0,  ADI_GPIO_P06,  ADI_GPIO_PIN_6 },
    { ADI_GPIO_PORT_0,  ADI_GPIO_P07,  ADI_GPIO_PIN_7 },
    { ADI_GPIO_PORT_0,  ADI_GPIO_P08,  ADI_GPIO_PIN_8 },
    { ADI_GPIO_PORT_0,  ADI_GPIO_P09,  ADI_GPIO_PIN_9 },
#endif
    { ADI_GPIO_PORT_0,  ADI_GPIO_P010, ADI_GPIO_PIN_10 },
    { ADI_GPIO_PORT_0,  ADI_GPIO_P011, ADI_GPIO_PIN_11 },
    { ADI_GPIO_PORT_0,  ADI_GPIO_P012, ADI_GPIO_PIN_12 },
    { ADI_GPIO_PORT_0,  ADI_GPIO_P013, ADI_GPIO_PIN_13 },
    { ADI_GPIO_PORT_0,  ADI_GPIO_P014, ADI_GPIO_PIN_14 },
    { ADI_GPIO_PORT_0,  ADI_GPIO_P015, ADI_GPIO_PIN_15 },

    { ADI_GPIO_PORT_1,  ADI_GPIO_P10,  ADI_GPIO_PIN_0 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P11,  ADI_GPIO_PIN_1 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P12,  ADI_GPIO_PIN_2 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P13,  ADI_GPIO_PIN_3 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P14,  ADI_GPIO_PIN_4 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P15,  ADI_GPIO_PIN_5 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P16,  ADI_GPIO_PIN_6 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P17,  ADI_GPIO_PIN_7 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P18,  ADI_GPIO_PIN_8 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P19,  ADI_GPIO_PIN_9 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P110, ADI_GPIO_PIN_10 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P111, ADI_GPIO_PIN_11 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P112, ADI_GPIO_PIN_12 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P113, ADI_GPIO_PIN_13 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P114, ADI_GPIO_PIN_14 },
    { ADI_GPIO_PORT_1,  ADI_GPIO_P115, ADI_GPIO_PIN_15 },

    { ADI_GPIO_PORT_2,  ADI_GPIO_P20,  ADI_GPIO_PIN_0 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P21,  ADI_GPIO_PIN_1 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P22,  ADI_GPIO_PIN_2 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P23,  ADI_GPIO_PIN_3 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P24,  ADI_GPIO_PIN_4 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P25,  ADI_GPIO_PIN_5 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P26,  ADI_GPIO_PIN_6 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P27,  ADI_GPIO_PIN_7 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P28,  ADI_GPIO_PIN_8 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P29,  ADI_GPIO_PIN_9 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P210, ADI_GPIO_PIN_10 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P211, ADI_GPIO_PIN_11 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P212, ADI_GPIO_PIN_12 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P213, ADI_GPIO_PIN_13 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P214, ADI_GPIO_PIN_14 },
    { ADI_GPIO_PORT_2,  ADI_GPIO_P215, ADI_GPIO_PIN_15 },

    { ADI_GPIO_PORT_3,  ADI_GPIO_P30,  ADI_GPIO_PIN_0 },
    { ADI_GPIO_PORT_3,  ADI_GPIO_P31,  ADI_GPIO_PIN_1 },
    { ADI_GPIO_PORT_3,  ADI_GPIO_P32,  ADI_GPIO_PIN_2 },
    { ADI_GPIO_PORT_3,  ADI_GPIO_P33,  ADI_GPIO_PIN_3 },
    { ADI_GPIO_PORT_3,  ADI_GPIO_P34,  ADI_GPIO_PIN_4 },
    { ADI_GPIO_PORT_3,  ADI_GPIO_P35,  ADI_GPIO_PIN_5 },
    { ADI_GPIO_PORT_3,  ADI_GPIO_P36,  ADI_GPIO_PIN_6 },
    { ADI_GPIO_PORT_3,  ADI_GPIO_P37,  ADI_GPIO_PIN_7 },
    { ADI_GPIO_PORT_3,  ADI_GPIO_P38,  ADI_GPIO_PIN_8 },
    { ADI_GPIO_PORT_3,  ADI_GPIO_P39,  ADI_GPIO_PIN_9 },
    { ADI_GPIO_PORT_3,  ADI_GPIO_P310, ADI_GPIO_PIN_10 },
    { ADI_GPIO_PORT_3,  ADI_GPIO_P311, ADI_GPIO_PIN_11 },
    { ADI_GPIO_PORT_3,  ADI_GPIO_P312, ADI_GPIO_PIN_12 },
    { ADI_GPIO_PORT_3,  ADI_GPIO_P313, ADI_GPIO_PIN_13 },
    { ADI_GPIO_PORT_3,  ADI_GPIO_P314, ADI_GPIO_PIN_14 },

    { ADI_GPIO_PORT_4,  ADI_GPIO_P40,  ADI_GPIO_PIN_0 },
    { ADI_GPIO_PORT_4,  ADI_GPIO_P41,  ADI_GPIO_PIN_1 },
    { ADI_GPIO_PORT_4,  ADI_GPIO_P42,  ADI_GPIO_PIN_2 },

};

/*! ***************************************************************************

    @brief      Function 'main' for GPIO example program

    @return     Status
                    - ADI_GPIO_SUCCESS if successful

    @details    Illustrates/tests the GPIO functions

    @note       These tests are destructive and leave the ports in a messy state.
                Therefore, regression test failures must wait until end of run
                to init the test interface (UART port) and log results.

    @warning

    @sa         main
******************************************************************************/

int main()
{
    ADI_GPIO_RESULT_TYPE Result;

    /* ever the optimist */
    Result = ADI_GPIO_SUCCESS;

    /* Clock initialization */
    SystemInit();

    /* initialization tests */
#ifdef ADI_DEBUG
    Result = InitializationTests(LOOPCOUNT);
#endif

    /* initialize the GPIO */
    if (ADI_GPIO_SUCCESS == Result) {
        Result = adi_GPIO_Init();
    }

    /* parameter checking tests */
#ifdef ADI_DEBUG
    if (ADI_GPIO_SUCCESS == Result) {
        Result = ParameterCheckingTests(LOOPCOUNT);
    }
#endif

    /* test GPIO pins */
    if (ADI_GPIO_SUCCESS == Result) {
        Result = TestGPIOPins (LOOPCOUNT);
    }

    /* test interrupt functions */
    if (ADI_GPIO_SUCCESS == Result) {
        Result = TestInterruptFunctions (LOOPCOUNT);
    }

    /* terminate the GPIO */
    if (ADI_GPIO_SUCCESS == Result) {
        Result = adi_GPIO_UnInit();
    }

     /* reinitialize UART and log result */
    test_Init();
    
    if (ADI_GPIO_SUCCESS == Result) {
        PASS();
    } else {
	FAIL("GPIO test failure...");
    }

    /* return */
    return (Result);
}



#ifdef ADI_DEBUG
/*! ***************************************************************************

    @brief      Initialization Tests

    @return     Status
                    - ADI_GPIO_SUCCESS if successful

    @details    Verifies API functions fail if not initialized properly

    @note

    @warning

    @sa         InitializationTests
******************************************************************************/

static ADI_GPIO_RESULT_TYPE InitializationTests(const uint32_t LoopCount)
{
    ADI_GPIO_RESULT_TYPE Result;
    uint32_t             i;

    /* FOR (as many times as we're told) */
    for (i = 0; i < LoopCount; i++) {

        /* verify each of these functions fail because we're not initialized yet */
        if ((Result = adi_GPIO_ResetToPowerUp()) != ADI_GPIO_ERR_NOT_INITIALIZED) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_EnableIRQ(EINT0_IRQn, ADI_GPIO_IRQ_RISING_EDGE)) != ADI_GPIO_ERR_NOT_INITIALIZED) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_DisableIRQ(EINT0_IRQn)) != ADI_GPIO_ERR_NOT_INITIALIZED) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_SetOutputEnable(ADI_GPIO_PORT_0, ADI_GPIO_PIN_0, true)) != ADI_GPIO_ERR_NOT_INITIALIZED) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_SetPullUpEnable(ADI_GPIO_PORT_0, ADI_GPIO_PIN_0, true)) != ADI_GPIO_ERR_NOT_INITIALIZED) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }

        if ((Result = adi_GPIO_SetHigh(ADI_GPIO_PORT_0, ADI_GPIO_PIN_0)) != ADI_GPIO_ERR_NOT_INITIALIZED) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_SetLow(ADI_GPIO_PORT_0, ADI_GPIO_PIN_0)) != ADI_GPIO_ERR_NOT_INITIALIZED) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_Toggle(ADI_GPIO_PORT_0, ADI_GPIO_PIN_0)) != ADI_GPIO_ERR_NOT_INITIALIZED) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_SetData(ADI_GPIO_PORT_0, ADI_GPIO_PIN_0)) != ADI_GPIO_ERR_NOT_INITIALIZED) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }

        /* verify init and uninit work as directed */
        /* allow multiple inits for multiple clients */

        /* init1 */
        if ((Result = adi_GPIO_Init()) != ADI_GPIO_SUCCESS) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        /* init2 */
        if ((Result = adi_GPIO_Init()) != ADI_GPIO_SUCCESS) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        /* uninit1 */
        if ((Result = adi_GPIO_UnInit()) != ADI_GPIO_SUCCESS) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        /* uninit2 */
        if ((Result = adi_GPIO_UnInit()) != ADI_GPIO_SUCCESS) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        /* uninit3... should fail */
        if ((Result = adi_GPIO_UnInit()) != ADI_GPIO_ERR_NOT_INITIALIZED) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }

        /* looks like everything is passing */
        Result = ADI_GPIO_SUCCESS;

    /* ENDFOR */
    }

    /* return */
    return (Result);
}
#endif




#ifdef ADI_DEBUG
/*! ***************************************************************************

    @brief      ParameterChecking Tests

    @return     Status
                    - ADI_GPIO_SUCCESS if successful

    @details    Verifies API functions fail if parameters are not correct

    @note

    @warning

    @sa         ParameterCheckingTests
******************************************************************************/

static ADI_GPIO_RESULT_TYPE ParameterCheckingTests(const uint32_t LoopCount)
{
    ADI_GPIO_RESULT_TYPE Result;
    uint32_t             i;

    /* FOR (as many times as we're told) */
    for (i = 0; i < LoopCount; i++) {

        /* verify parameters */
        if ((Result = adi_GPIO_EnableIRQ(WUT_IRQn, ADI_GPIO_IRQ_RISING_EDGE)) != ADI_GPIO_ERR_INVALID_INTERRUPT) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_EnableIRQ(EINT0_IRQn, (ADI_GPIO_IRQ_TRIGGER_CONDITION_TYPE)(0xffff))) != ADI_GPIO_ERR_INVALID_TRIGGER) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_SetOutputEnable(0xffff, ADI_GPIO_PIN_0, true)) != ADI_GPIO_ERR_INVALID_PORT) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_SetOutputEnable(ADI_GPIO_PORT_0, 0, true)) != ADI_GPIO_ERR_INVALID_PINS) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_SetPullUpEnable(0xffff, ADI_GPIO_PIN_0, true)) != ADI_GPIO_ERR_INVALID_PORT) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_SetPullUpEnable(ADI_GPIO_PORT_0, 0, true)) != ADI_GPIO_ERR_INVALID_PINS) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }

        if ((Result = adi_GPIO_SetHigh(0xffff, ADI_GPIO_PIN_0)) != ADI_GPIO_ERR_INVALID_PORT) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_SetHigh(ADI_GPIO_PORT_0, 0)) != ADI_GPIO_ERR_INVALID_PINS) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_SetLow(0xffff, ADI_GPIO_PIN_0)) != ADI_GPIO_ERR_INVALID_PORT) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_SetLow(ADI_GPIO_PORT_0, 0)) != ADI_GPIO_ERR_INVALID_PINS) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_Toggle(0xffff, ADI_GPIO_PIN_0)) != ADI_GPIO_ERR_INVALID_PORT) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_Toggle(ADI_GPIO_PORT_0, 0)) != ADI_GPIO_ERR_INVALID_PINS) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_SetData(0xffff, ADI_GPIO_PIN_0)) != ADI_GPIO_ERR_INVALID_PORT) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }
        if ((Result = adi_GPIO_SetData(ADI_GPIO_PORT_0, 0)) != ADI_GPIO_ERR_INVALID_PINS) {
            Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
            break;
        }

        /* looks like everything is passing */
        Result = ADI_GPIO_SUCCESS;

    /* ENDFOR */
    }

    /* return */
    return (Result);
}
#endif


/*! ***************************************************************************

    @brief      Function Test Interrupt Functions

    @return     Status
                    - ADI_GPIO_SUCCESS if successful

    @details    Tests and verifies the GPIO interrupt functions

    @note

    @warning

    @sa         Test Interrupt Functions
******************************************************************************/

static ADI_GPIO_RESULT_TYPE TestInterruptFunctions(const uint32_t LoopCount) {

    typedef struct {
        IRQn_Type   Irq;
        volatile uint16_t *pConfigReg;
        uint8_t ShiftCount;
    } IRQENTRY;

    IRQENTRY IrqTable[] = {
        {   EINT0_IRQn, &pADI_IDU->EI0CFG, 0  },
        {   EINT1_IRQn, &pADI_IDU->EI0CFG, 4  },
        {   EINT2_IRQn, &pADI_IDU->EI0CFG, 8  },
        {   EINT3_IRQn, &pADI_IDU->EI0CFG, 12 },
        {   EINT4_IRQn, &pADI_IDU->EI1CFG, 0  },
        {   EINT5_IRQn, &pADI_IDU->EI1CFG, 4  },
        {   EINT6_IRQn, &pADI_IDU->EI1CFG, 8  },
        {   EINT7_IRQn, &pADI_IDU->EI1CFG, 12 },
    };
    ADI_GPIO_RESULT_TYPE Result;
    uint32_t             i;
    uint32_t             j;
    IRQENTRY            *pIrqEntry;
    ADI_GPIO_CONFIG_TYPE ConfigReg;

    /* FOR (as many times as we're told) */
    for (i = 0; i < LoopCount; i++) {

        /* FOR (each interrupt) */
        for (j = 0, pIrqEntry = IrqTable; j < ((sizeof(IrqTable))/(sizeof(IrqTable[0]))); j++, pIrqEntry++) {

            /* enable the interrupt */
            if ((Result = adi_GPIO_EnableIRQ(pIrqEntry->Irq, ADI_GPIO_IRQ_FALLING_EDGE)) != ADI_GPIO_SUCCESS) {
                return (Result);
            }

            /* verify the interrupt was enabled */
            ConfigReg = *pIrqEntry->pConfigReg;
            ConfigReg >>= pIrqEntry->ShiftCount;
            if ((ConfigReg & 0x08) != 0x08) {
                return (ADI_GPIO_ERR_UNKNOWN_ERROR);
            }

            /* disable the interrupt */
            if ((Result = adi_GPIO_DisableIRQ(pIrqEntry->Irq)) != ADI_GPIO_SUCCESS) {
                return (Result);
            }

            /* verify the interrupt was disabled */
            ConfigReg = *pIrqEntry->pConfigReg;
            ConfigReg >>= pIrqEntry->ShiftCount;
            if ((ConfigReg & 0x08) != 0x00) {
                return (ADI_GPIO_ERR_UNKNOWN_ERROR);
            }

        /* ENDFOR */
        }

    /* ENDFOR */
    }

    /* return */
    return (Result);
}


/*! ***************************************************************************

    @brief      Function Test GPIO Pins

    @return     Status
                    - ADI_GPIO_SUCCESS if successful

    @details    Tests and verifies individual pin functions

    @note

    @warning

    @sa         Test GPIO Pins
******************************************************************************/

static ADI_GPIO_RESULT_TYPE TestGPIOPins(const uint32_t LoopCount) {
    ADI_GPIO_RESULT_TYPE Result;
    uint32_t             i;
    uint32_t             j;
    ADI_GPIO_DATA_TYPE   RegData;
    GPIOEntry            *pGPIOEntry;

    /* ever the optimist */
    Result = ADI_GPIO_SUCCESS;

    /* reset to power up defaults */
    if ((Result = adi_GPIO_ResetToPowerUp()) != ADI_GPIO_SUCCESS) {
        return (Result);
    }

    /* FOR (as many times as we're told) */
    for (i = 0; i < LoopCount; i++) {

        /* FOR (each GPIO to test) */
        for (j = 0, pGPIOEntry = GPIOs; j < ((sizeof(GPIOs))/(sizeof(GPIOs[0]))); j++, pGPIOEntry++) {

            /* configure the pin as an output */
            if ((Result = adi_GPIO_SetOutputEnable(pGPIOEntry->Port, pGPIOEntry->PinValue, true)) != ADI_GPIO_SUCCESS) {
                break;
            }

            /* verify the pin is configured as an output */
            RegData = adi_GPIO_GetOutputEnable(pGPIOEntry->Port);
            if ((RegData & pGPIOEntry->PinValue) == 0x0) {
                Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
                break;
            }

            /* configure the pin as an input also */
            if ((Result = adi_GPIO_SetInputEnable(pGPIOEntry->Port, pGPIOEntry->PinValue, true)) != ADI_GPIO_SUCCESS) {
                break;
            }

            /* verify the pin is configured as an input */
            RegData = adi_GPIO_GetInputEnable(pGPIOEntry->Port);
            if ((RegData & pGPIOEntry->PinValue) == 0x0) {
                Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
                break;
            }

            /* drive the pin high */
            if ((Result = adi_GPIO_SetHigh(pGPIOEntry->Port, pGPIOEntry->PinValue)) != ADI_GPIO_SUCCESS) {
                break;
            }

            /* verify that the pin is reading back high */
            RegData = adi_GPIO_GetData(pGPIOEntry->Port);
            if ((RegData & pGPIOEntry->PinValue) == 0x0) {
                Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
                break;
            }

            /* drive the pin low */
            if ((Result = adi_GPIO_SetLow(pGPIOEntry->Port, pGPIOEntry->PinValue)) != ADI_GPIO_SUCCESS) {
                break;
            }

            /* verify that the pin is reading back low */
            RegData = adi_GPIO_GetData(pGPIOEntry->Port);
            if ((RegData & pGPIOEntry->PinValue) != 0x0) {
                Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
                break;
            }

            /* toggle the pin */
            if ((Result = adi_GPIO_Toggle(pGPIOEntry->Port, pGPIOEntry->PinValue)) != ADI_GPIO_SUCCESS) {
                break;
            }

            /* verify that the pin is reading back high */
            RegData = adi_GPIO_GetData(pGPIOEntry->Port);
            if ((RegData & pGPIOEntry->PinValue) == 0x0) {
                Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
                break;
            }

            /* toggle the pin again */
            if ((Result = adi_GPIO_Toggle(pGPIOEntry->Port, pGPIOEntry->PinValue)) != ADI_GPIO_SUCCESS) {
                break;
            }

            /* verify that the pin is reading back low */
            RegData = adi_GPIO_GetData(pGPIOEntry->Port);
            if ((RegData & pGPIOEntry->PinValue) != 0x0) {
                Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
                break;
            }


            /* configure the pin as an input */
            if ((Result = adi_GPIO_SetOutputEnable(pGPIOEntry->Port, pGPIOEntry->PinValue, false)) != ADI_GPIO_SUCCESS) {
                break;
            }

            /* verify the pin is configured as an input */
            RegData = adi_GPIO_GetOutputEnable(pGPIOEntry->Port);
            if ((RegData & pGPIOEntry->PinValue) != 0x0) {
                Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
                break;
            }

            /* enable the pull up for the pin */
            if ((Result = adi_GPIO_SetPullUpEnable(pGPIOEntry->Port, pGPIOEntry->PinValue, true)) != ADI_GPIO_SUCCESS) {
                break;
            }

            /* verify that the pin is being pulled up */
            RegData = adi_GPIO_GetPullUpEnable(pGPIOEntry->Port);
            if ((RegData & pGPIOEntry->PinValue) == 0x0) {
                Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
                break;
            }

            /* disable the pull up for the pin */
            if ((Result = adi_GPIO_SetPullUpEnable(pGPIOEntry->Port, pGPIOEntry->PinValue, false)) != ADI_GPIO_SUCCESS) {
                break;
            }

            /* verify that the pin is no longer being pulled up */
            RegData = adi_GPIO_GetPullUpEnable(pGPIOEntry->Port);
            if ((RegData & pGPIOEntry->PinValue) != 0x0) {
                Result = ADI_GPIO_ERR_UNKNOWN_ERROR;
                break;
            }

        /* ENDFOR */
        }

        /* break if any errors */
        if (Result != ADI_GPIO_SUCCESS) {
            break;
        }

    /* ENDFOR */
    }

    /* return */
    return (Result);
}

/*
** EOF
*/

/*@}*/
