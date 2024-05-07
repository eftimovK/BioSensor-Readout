/*********************************************************************************

Copyright (c) 2012-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/


/*****************************************************************************
 * @file:    SysClkTest.c
 * @brief:   System Clocks Test for ADuCM350
 * @version: $Revision: 28525 $
 * @date:    $Date: 2014-11-12 14:51:26 -0500 (Wed, 12 Nov 2014) $
 *****************************************************************************/

/*! \addtogroup SysClk_Test System Clocks Test
 *  .
 *  @{
 */

/* Apply ADI MISRA Suppressions */
#define ASSERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"

#include <stddef.h>		/* for 'NULL' */
//#include <string.h>		/* for strlen */

#include "gpt.h"
#include "wut.h"

#include "test_common.h"

/* Timeout set for ~15ms          */
/* XTAL expected to lock in ~10ms */
/* SPLL expected to lock in ~30us */
//#define TEST_GPT_TIMEOUT       0x1F4
#define TEST_GPT_TIMEOUT       0x3F4
#define TEST_WUT_TIMEOUT       0x1F4

// FIXME: this is already defined in system.c
// but still needed here due to missing API
// To be removed when the new API is added
#define OSCKEY_UNLOCK                (0xCB14)

/* Device handles */
ADI_GPT_HANDLE          hGPT;
ADI_WUT_DEV_HANDLE      hWUT;

volatile bool_t         bCommonInterruptFlag;

void                    test_SystemClkXTAL_Poll     (void);
void                    test_SystemClkXTAL_IRQ      (void);
void                    test_SystemClkPLL_Poll      (ADI_SYS_CLOCK_MUX_ID sourceId);

ADI_GPT_RESULT_TYPE     test_InitializeTimer        (uint16_t timeout);
ADI_GPT_RESULT_TYPE     test_EnableTimer            (void);
ADI_GPT_RESULT_TYPE     test_DisableTimer           (void);

ADI_WUT_RESULT_TYPE     test_InitializeWakeupTimer  (void);
ADI_WUT_RESULT_TYPE     test_EnableWakeupTimer      (uint32_t timeout);
ADI_WUT_RESULT_TYPE     test_DisableWakeupTimer     (void);

void                    test_SysClockReset          (void);


/* callbacks */
void gptCallback (void *pCBParam, uint32_t Event, void *pArg);
void wutCallback (void *pCBParam, uint32_t Event, void *pArg);


/*!
 * @brief       Function 'main' for System Clocks example program
 *
 * @param       none
 * @return      int (Zero for success, non-zero for failure).
 *
 * @details     Simple application that calls the example functions in sequence.
 *
 */

int main(void)
{

    /* Initialize system */
    SystemInit();

    /* test system initialization */
    test_Init();

    test_InitializeTimer(TEST_GPT_TIMEOUT);
    test_InitializeWakeupTimer();


    /* The following functions correspond to the use cases */
    /* outlined in HRM section "Example Use Cases",        */
    /* chapter "System Clocks"                             */

    /* Set system clock source to XTAL using polling */
    test_SystemClkXTAL_Poll();

    /* Return to HFOSC as main clock source */
    test_SysClockReset();

    // Set system clock source to XTAL using IRQ
    test_SystemClkXTAL_IRQ();

    /* Return to HFOSC as main clock source */
    test_SysClockReset();

    /* Set system clock source to PLL (PLL source is HFOSC) */
    test_SystemClkPLL_Poll(ADI_SYS_CLOCK_MUX_SPLL_HF_OSC);

    /* Return to HFOSC as main clock source */
    test_SysClockReset();

    /* Set system clock source to PLL (PLL source is HFXTAL) */
    test_SystemClkPLL_Poll(ADI_SYS_CLOCK_MUX_SPLL_HF_XTAL);

    PASS();

}

void test_SysClockReset(void) {

    SetSystemClockMux (ADI_SYS_CLOCK_MUX_ROOT_HFOSC);
    /* Disable HFXTAL */
    pADI_PWR->OSCKEY = OSCKEY_UNLOCK;
    pADI_PWR->OSCCTRL &= ~BITM_PWR_OSCCTRL_HFXTALEN;

}

void test_SystemClkXTAL_Poll(void) {

    /* Step numbers correspond to figure 12 in HRMv1.04 */

    /* 1. Select the internal oscillator as clock source */
    SetSystemClockMux (ADI_SYS_CLOCK_MUX_ROOT_HFOSC);

    /* 2. Turn on the XTAL, the lock time is expected to be around 10 ms */

    /* Clear status register, HFXTAL bits */
    pADI_SYSCLK->CLKSTAT0 = ((1 << BITP_SYSCLK_CLKSTAT0_HFXTALNOK) |
                             (1 << BITP_SYSCLK_CLKSTAT0_HFXTALOK));

    /* Enable HKXTAL */
    pADI_PWR->OSCKEY = OSCKEY_UNLOCK;
    pADI_PWR->OSCCTRL |= BITM_PWR_OSCCTRL_HFXTALEN;

    /* 3. Setting up a timeout in case the XTAL does not lock. */
    if (test_EnableTimer() != ADI_GPT_SUCCESS) {
        FAIL("GPT SetLdVal");
    }

    /* 4-5. Poll the XTAL status bits. If the XTAL never locks, the code will be */
    /*      stuck here until the timer interrupt fires                           */

    /* Wait for HFXTAL to stabilize */
    while (!(pADI_SYSCLK->CLKSTAT0 & BITM_SYSCLK_CLKSTAT0_HFXTALOK))
        ;

    /* 6. Disable the timer */
    test_DisableTimer();

    /* 7.Change system clock to XTAL */
    SetSystemClockMux (ADI_SYS_CLOCK_MUX_ROOT_HFXTAL);

}

void test_SystemClkXTAL_IRQ(void) {

    /* Step numbers correspond to figure 13 in HRMv1.04 */

    /* 1. Select the internal oscillator as clock source */
    SetSystemClockMux(ADI_SYS_CLOCK_MUX_ROOT_HFOSC);

    /* 2. Turn on the XTAL with IRQ enabled */

    /* Clear status register, HFXTAL bits */
    pADI_SYSCLK->CLKSTAT0 = ((1 << BITP_SYSCLK_CLKSTAT0_HFXTALNOK) |
                             (1 << BITP_SYSCLK_CLKSTAT0_HFXTALOK));

    /* Enable HKXTAL */
    pADI_PWR->OSCKEY = OSCKEY_UNLOCK;
    pADI_PWR->OSCCTRL |= BITM_PWR_OSCCTRL_HFXTALEN;

    /* Enable HFXTAL interrupt */
    pADI_SYSCLK->CLKCON0 |= (1 << BITP_SYSCLK_CLKCON0_HFXTALIE);

    /* 3. Setting up a timeout in case the XTAL does not lock. */
    if (test_EnableWakeupTimer(TEST_WUT_TIMEOUT) != ADI_WUT_SUCCESS) {
        FAIL("test_EnableWakeupTimer failed");
    }

    /* Enable XTAL interrupt in NVIC */
    NVIC_EnableIRQ(XTAL_OSC_IRQn);

    /* 4. Go to sleep (CORE_SLEEP) */
    bCommonInterruptFlag = false;
    if (SystemEnterLowPowerMode(ADI_SYS_MODE_CORE_SLEEP, &bCommonInterruptFlag, 1)) {
        FAIL("SystemEnterLowPowerMode failed");
    }

    if (test_DisableWakeupTimer() != ADI_WUT_SUCCESS) {
        FAIL("test_DisableWakeupTimer failed");
    }

    /* Check if XTAL is locked */
    if (!(pADI_SYSCLK->CLKSTAT0 & BITM_SYSCLK_CLKSTAT0_HFXTALOK)) {
        FAIL("XTAL not locked!");
    }

    /* 5. Change system clock to XTAL */
    SetSystemClockMux (ADI_SYS_CLOCK_MUX_ROOT_HFXTAL);

}


void test_SystemClkPLL_Poll(ADI_SYS_CLOCK_MUX_ID sourceId) {

    if ((sourceId != ADI_SYS_CLOCK_MUX_SPLL_HF_OSC) && (sourceId != ADI_SYS_CLOCK_MUX_SPLL_HF_XTAL)) {
        FAIL("test_SystemClkPLL_Poll wrong parameter");
    }

    /* Step numbers correspond to figure 10 in HRMv1.04 */

    /* 1. Select the internal oscillator as clock source */
    SetSystemClockMux (ADI_SYS_CLOCK_MUX_ROOT_HFOSC);

    if (sourceId == ADI_SYS_CLOCK_MUX_SPLL_HF_XTAL) {

        /* 2. Select XTAL as PLL source */
        SetSystemClockMux(ADI_SYS_CLOCK_MUX_SPLL_HF_XTAL);

        /* 3. Turn on the XTAL, the lock time is expected to be around 10 ms */

        /* Clear status register, HFXTAL bits */
        pADI_SYSCLK->CLKSTAT0 = ((1 << BITP_SYSCLK_CLKSTAT0_HFXTALNOK) |
                                 (1 << BITP_SYSCLK_CLKSTAT0_HFXTALOK));

        /* Enable HFXTAL */
        pADI_PWR->OSCKEY = OSCKEY_UNLOCK;
        pADI_PWR->OSCCTRL |= BITM_PWR_OSCCTRL_HFXTALEN;
    }
    else {
        /* Select HFOSC as PLL source */
        SetSystemClockMux(ADI_SYS_CLOCK_MUX_SPLL_HF_OSC);
    }

    /* 4. Program SPLL */

    /* Clear status register, SPLL bits */
    pADI_SYSCLK->CLKSTAT0 = ((1 << BITP_SYSCLK_CLKSTAT0_SPLLUNLOCK) |
                             (1 << BITP_SYSCLK_CLKSTAT0_SPLLLOCK));

    /* Configure PLL */
    pADI_SYSCLK->CLKCON3 =  (0 << BITP_SYSCLK_CLKCON3_SPLLIE) |      /* Interrupt disabled*/
                            (1 << BITP_SYSCLK_CLKCON3_SPLLEN) |      /* SPLL enabled */
                            (1 << BITP_SYSCLK_CLKCON3_SPLLDIV2) |   /* Divide by 2 */
                            (2 << BITP_SYSCLK_CLKCON3_SPLLMSEL) |   /* M divider */
                            (8 << BITP_SYSCLK_CLKCON3_SPLLNSEL);    /* N multiplier */

    /* 5. Setting up a timeout in case the PLL does not lock. */
    if (test_EnableTimer() != ADI_GPT_SUCCESS) {
        FAIL("test_EnableTimer failed");
    }

    if (sourceId == ADI_SYS_CLOCK_MUX_SPLL_HF_XTAL) {
        /* 6. Poll the XTAL status bits.  If the XTAL never locks, the code will be */
        /*    stuck here until the timer interrupt fires                            */

        while (!(pADI_SYSCLK->CLKSTAT0 & BITM_SYSCLK_CLKSTAT0_HFXTALOK))
            ;
    }

    /* 7. Poll PLL status bits. If the PLL never locks, the code will be stuck */
    /*    here until the timer interrupt fires                                 */

    while (!(pADI_SYSCLK->CLKSTAT0 & BITM_SYSCLK_CLKSTAT0_SPLLLOCK))
        ;

    /* 8. Disable timer */
    test_DisableTimer();

    /* 9. Change system clock to PLL */
    SetSystemClockMux(ADI_SYS_CLOCK_MUX_ROOT_SPLL);

}

/**********************/
/*   GPT Functions    */
/**********************/

ADI_GPT_RESULT_TYPE test_InitializeTimer(uint16_t timeout) {
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;

    /* Initialize timer 0 */
    if (adi_GPT_Init(ADI_GPT_DEVID_0, &hGPT) != ADI_GPT_SUCCESS) {
        FAIL("GPT Init");
    }

    /* Program timeout value */
    result = adi_GPT_SetLdVal(hGPT, timeout);

    if (ADI_GPT_SUCCESS == result) {
    /* Set 32kHz clock as clock source */
    result = adi_GPT_SetClockSelect(hGPT, ADI_GPT_CLOCK_SELECT_32KHZ_INTERNAL_CLOCK);
    }

    if (ADI_GPT_SUCCESS == result) {
    /* Set prescaler value to 1 */
    result = adi_GPT_SetPrescaler(hGPT, ADI_GPT_PRESCALER_1);
    }

    if (ADI_GPT_SUCCESS == result) {
    /* Select count down mode */
    result = adi_GPT_SetCountMode(hGPT, ADI_GPT_COUNT_DOWN);
    }

    if (ADI_GPT_SUCCESS == result) {
        /* Register callback for timeout failure */
        result = adi_GPT_RegisterCallback(hGPT, gptCallback, NULL);
    }

    return result;
}

ADI_GPT_RESULT_TYPE test_EnableTimer(void) {
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;

    /* Enable timer interrupt in NVIC */
    NVIC_EnableIRQ(TIMER0_IRQn);

    result = adi_GPT_SetTimerEnable(hGPT, true);

    return result;
}

ADI_GPT_RESULT_TYPE test_DisableTimer(void) {
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;

    /* Disable interrupt in NVIC */
    NVIC_DisableIRQ(TIMER0_IRQn);

    /* Stop timer count (disable) */
    result = adi_GPT_SetTimerEnable(hGPT, false);

    return result;
}

/**********************/
/*   WUT Functions    */
/**********************/

ADI_WUT_RESULT_TYPE test_InitializeWakeupTimer(void){
    ADI_WUT_RESULT_TYPE result = ADI_WUT_SUCCESS;

    if( adi_WUT_Init(ADI_WUT_DEVID_0, &hWUT) != 0 ) {
         FAIL("adi_WUT_Init failed");
    }

    /* install WUT callback on all interrupts */
    if (adi_WUT_RegisterCallback(hWUT, wutCallback, ADI_WUT_TARGET_MASK)) {
        FAIL("adi_WUT_RegisterCallback failed");
    }

    /* select LFOSC clock source */
    if( adi_WUT_SetClockSelect(hWUT, ADI_WUT_CLK_LFOSC) != 0 ) {
        FAIL("adi_WUT_SetClockSelect failed");
    }

    /* No pre-scaling */
    if( adi_WUT_SetPrescaler(hWUT, ADI_WUT_PRE_DIV1) != 0 ) {
        FAIL("adi_WUT_SetPrescaler failed");
    }

    /* select periodic mode */
    if( adi_WUT_SetTimerMode(hWUT, ADI_WUT_MODE_PERIODIC) != 0 ) {
        FAIL("adi_WUT_SetTimerMode failed");
    }

    /* Enable wakeup */
    if( adi_WUT_SetWakeUpEnable(hWUT, true) != 0 ) {
        FAIL("adi_WUT_SetWakeUpEnable failed");
    }

    return result;
}

ADI_WUT_RESULT_TYPE test_EnableWakeupTimer(uint32_t timeout){
    ADI_WUT_RESULT_TYPE result = ADI_WUT_SUCCESS;

    /* Enable timer interrupt in NVIC */
    NVIC_EnableIRQ(WUT_IRQn);

    /* Program timeout value */
    result = adi_WUT_SetComparator(hWUT, ADI_WUT_COMPB, timeout);

    /* Enable the timer */
    result = adi_WUT_SetTimerEnable(hWUT, true);

    /* Enable interrupts from source B */
    result = adi_WUT_SetInterruptEnable(hWUT, ADI_WUT_COMPB, true);

    return result;
}

ADI_WUT_RESULT_TYPE test_DisableWakeupTimer(void){
    ADI_WUT_RESULT_TYPE result = ADI_WUT_SUCCESS;

    /* Disable interrupt in NVIC */
    NVIC_DisableIRQ(WUT_IRQn);

    /* Stop timer count (disable) */
    adi_WUT_SetWakeUpEnable(hWUT, false);

    /* Clear interrupt flag */
    adi_WUT_ClearInterruptFlag(hWUT, ADI_WUT_COMPB);

    return result;
}

/**************************/
/*   Interrupt Handlers   */
/**************************/

void gptCallback(void *pCBParam, uint32_t Event, void *pArg) {
    /* Disable interrupt */
    NVIC_DisableIRQ(TIMER0_IRQn);

    FAIL("Timeout error");
}

void wutCallback(void* hWut, uint32_t Event, void* pArg) {

    /* This can bring the part out of hibernate  */
    SystemExitLowPowerMode(&bCommonInterruptFlag);

    /* Disable interrupt */
    NVIC_DisableIRQ(WUT_IRQn);

}

void XTAL_Int_Handler(void) {

    SystemExitLowPowerMode(&bCommonInterruptFlag);

    /* Disable interrupt */
    NVIC_DisableIRQ(XTAL_OSC_IRQn);
}

void PLL_Int_Handler(void) {

    /* Disable interrupt */
    NVIC_DisableIRQ(PLL_IRQn);
}

/* Revert ADI MISRA Suppressions */
#define REVERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"

/*
** EOF
*/

/*@}*/
