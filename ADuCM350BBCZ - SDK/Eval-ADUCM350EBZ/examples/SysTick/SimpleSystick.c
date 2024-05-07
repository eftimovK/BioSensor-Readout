/*********************************************************************************

Copyright (c) 2011-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using
this software you agree to the terms of the associated Analog Devices Software
License Agreement.

*********************************************************************************/

/*!
 *****************************************************************************
 * @file:    SimpleSystick.c
 * @brief:   SimpleSystick Device Example for ADuCxxx
 * @version: $Revision: 29237 $
 * @date:    $Date: 2014-12-15 12:42:39 -0500 (Mon, 15 Dec 2014) $
 *****************************************************************************/

/** \addtogroup Systick_Example SysTick Example
 *  Example code demonstrating use of the SysTick Interrupt.
 *  @{
 */

#include <stdint.h>
#include <stdio.h>

#include "test_common.h"

#if defined ( __ICCARM__ )  // IAR compiler...
/* Apply ADI MISRA Suppressions */
#define ASSERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"
#endif

#define COUNT 26
static uint32_t Count = 0;

/**
 * @brief  Function 'main' for SysTick example program.
 *
 * @return int (Zero for success, non-zero for failure).
 *
 * This is a simple application that sets up the SysTick to generate
 * a periodic interrupt, which is then used to increment a counter and print
 * the count onto the Terminal IO.
 */
int main()
{
	int i;

    /*
     * Clock initialization
     */
    SystemInit();                       /* CMSIS API */

    /* test system initialization */
    test_Init();

    /*
     * SysTick initialization
     */
    SysTick_Config(/*SystemFrequency*/20000/2);  /* CMSIS API */

    /* timeout and also wait for COUNT */
    for (i=0; i<100000; i++) {
    	if (COUNT == Count)
    		PASS();
    }

	FAIL("Systick interrupt count failed or timed-out waiting");
}


/**
 * @brief  Interrupt handler for SysTick example program
 *
 * @return none
 *
 * SysTick Handler is called whenever SysTick interrupt occurs.
 * In this example, the handler increments the interrupt count
 */
void SysTick_Handler(void)
{
    /* bump Count up to COUNT  */
    if (COUNT != Count)
        Count += 1;
}


#if defined ( __ICCARM__ )  // IAR compiler...
/* Revert ADI MISRA Suppressions */
#define REVERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"
#endif


/*
** EOF
*/

/*@}*/
