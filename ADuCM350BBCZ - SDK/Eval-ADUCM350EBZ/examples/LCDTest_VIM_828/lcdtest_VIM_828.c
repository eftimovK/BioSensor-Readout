/*********************************************************************************

Copyright (c) 2013-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/


/*****************************************************************************
 * @file:    LCDTest.c
 * @brief:   LCD testing
 * @version: $Revision: 29403 $
 * @date:    $Date: 2015-01-08 10:34:56 -0500 (Thu, 08 Jan 2015) $
 *****************************************************************************/

#include "test_common.h"
#include "lcd.h"
#include "lcd_VIM828.h"

extern int32_t adi_initpinmux(void);

/* Device handles */
ADI_GPT_HANDLE          hGPT;
ADI_LCD_DEV_HANDLE      hLCD;

/* Function Prototypes */
ADI_GPT_RESULT_TYPE     InitializeTimer     (void);
ADI_GPT_RESULT_TYPE     EnableTimer         (uint16_t timeout);
ADI_GPT_RESULT_TYPE     DisableTimer        (void);
void                    Delay               (uint16_t TimeOut);
void                    TestContrast        (void);
void                    TestDisplayString   (void);
void                    TestBlink           (bool_t bAutomatic);

/* Callback Function Prototypes */
void                    GPTimerCallback     (void *pCBParam, uint32_t Event, void *pArg);
void                    LCDCallback         (void *pCBParam,uint32_t nEvent, void *EventArg);

volatile    bool_t  timerOn;

/*!
 * @brief       Function 'main' for LCD example program
 *
 * @param       none
 * @return      int (Zero for success, non-zero for failure).
 *
 * @details     Simple application that calls the example functions in sequence.
 *
 */

int main(void)
{
    bool_t  bVLCDState;

    /* Initialize system */
    SystemInit();

    /* Test initialization */
    test_Init();

    /* Set the pin mux */
    adi_initpinmux();

    /* Initialize the timer required for the delay functions */
    InitializeTimer();

    /* Initialise lcd driver, by passing the configuration structure */
    if(ADI_LCD_SUCCESS != adi_LCD_Init (ADI_LCD_DEVID_0, &hLCD))
    {
        FAIL("adi_LCD_Init Failed");
    }

    /* Install the callback handler */
    if(ADI_LCD_SUCCESS != adi_LCD_RegisterCallback(hLCD, LCDCallback, NULL))
    {
        FAIL("Install the callback handler failed (adi_LCD_RegisterCallback)");
    }

    /* Enable LCD */
    if(ADI_LCD_SUCCESS != adi_LCD_Enable (hLCD, true))
    {
        FAIL("Enable LCD failed (adi_LCD_Enable)");
    }

    /* Wait until the charge pump reaches the bias level */
    do
    {
        if(ADI_LCD_SUCCESS != adi_LCD_GetVLCDStatus(hLCD, &bVLCDState))
        {
            FAIL("Charge pump failed to reach the bias level (adi_LCD_GetVLCDStatus)");
        }
    } while(bVLCDState != true);

    /* Test displaying the characters on segment display */
    TestDisplayString();

    /* Test the LCD contrast */
    uint8_t dispString1[] = {"CONTRAST"};
    if(adi_LCDVIM828_DisplayString(hLCD, ADI_LCD_SCREEN_0, dispString1))
    {
        FAIL("Contrast Test: adi_LCDVIM828_DisplayString failed");
    }
    TestContrast();

    /* Test hardware based blinking */
    uint8_t dispString2[] = {"HW BLINK"};
    if(adi_LCDVIM828_DisplayString(hLCD, ADI_LCD_SCREEN_0, dispString2))
    {
        FAIL("HW Blink Test: adi_LCDVIM828_DisplayString failed");
    }
    TestBlink(true);

    /* Test software based blinking */
    uint8_t dispString3[] = {"SW BLINK"};
    if(adi_LCDVIM828_DisplayString(hLCD, ADI_LCD_SCREEN_0, dispString3))
    {
        FAIL("SW Blink Test: adi_LCDVIM828_DisplayString failed");
    }
    TestBlink(false);

    /* Disable the timer */
    DisableTimer();

    /* Set the contrast level to 0 */
    if(ADI_LCD_SUCCESS != adi_LCD_SetContrast(hLCD, 0))
    {
        FAIL("Failed to set the contrast level to 0 (adi_LCD_SetContrast)");
    }

    /* Disable the charge pump */
    if(ADI_LCD_SUCCESS != adi_LCD_EnableChargePump(hLCD, false))
    {
        FAIL("Failed to disable the charge pump (adi_LCD_EnableChargePump)");
    }

    /* unitialize the LCD */
    if(ADI_LCD_SUCCESS != adi_LCD_UnInit(hLCD))
    {
        FAIL("Failed to unitialize the LCD (adi_LCD_UnInit)");
    }

    PASS();
}

/*!
 * @brief       Initialise Timer Function.
 *
 * @return      Status
 *              - #ADI_GPT_SUCCESS                      Call completed successfully.
 *
 * @details     This function initialises which clock is being used and which prescaler.
 *              It also sets whether the timer is to count up or down.
 *
 */
ADI_GPT_RESULT_TYPE InitializeTimer() {
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;

    /* Initialize timer 0 */
    if (ADI_GPT_SUCCESS != adi_GPT_Init(ADI_GPT_DEVID_0, &hGPT))
    {
        FAIL("InitializeTIMER: adi_GPT_Init");
    }

    /* Set to count down */
    if (ADI_GPT_SUCCESS != adi_GPT_SetCountMode(hGPT, ADI_GPT_COUNT_DOWN))
    {
        FAIL("InitializeTIMER: adi_GPT_SetCountMode");
    }

    /* Select PCLK as source */
    if (ADI_GPT_SUCCESS != adi_GPT_SetClockSelect(hGPT, ADI_GPT_CLOCK_SELECT_PCLK))
    {
        FAIL("InitializeTIMER: adi_GPT_SetClockSelect");
    }

    /* Set the prescaler */
    if (ADI_GPT_SUCCESS != adi_GPT_SetPrescaler(hGPT, ADI_GPT_PRESCALER_32768))
    {
        FAIL("InitializeTIMER: adi_GPT_SetPrescaler");
    }

    /* Register the call back */
    if (ADI_GPT_SUCCESS != adi_GPT_RegisterCallback(hGPT, GPTimerCallback, NULL))
    {
        FAIL("InitializeTIMER: adi_GPT_RegisterCallback");
    }

    return result;
}

/*!
 * @brief       Enable Timer Function.
 *
 * @return      Status
 *              - #ADI_GPT_SUCCESS              Call completed successfully.
 *
 * @details     This function sets the length of time of the delay.
 *
 */
ADI_GPT_RESULT_TYPE EnableTimer(uint16_t timeout) {
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;

    timerOn = false;

    /* Program timeout value */
    result = adi_GPT_SetLdVal(hGPT, timeout);

    /* Start the timer */
    result = adi_GPT_SetTimerEnable(hGPT, true);

    return result;
}

/*!
 * @brief       Disable Timer Function.
 *
 * @return      Status
 *              - #ADI_GPT_SUCCESS                      Call completed successfully.
 *
 * @details     This function disables the timer.
 *
 */
ADI_GPT_RESULT_TYPE DisableTimer(void) {
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;

    /* stop timer count (disable) */
    result = adi_GPT_SetTimerEnable(hGPT, false);

    return result;
}

/* Waits until the given timout reached. Uses the GP Timer for wait */
void Delay(uint16_t TimeOut)
{
    /* Enable the timer by setting the given timeout */
    EnableTimer(TimeOut);

    /* Wait until the timer expires */
    while(timerOn == false)
    {
        asm("nop; nop; nop;");
    }
}


/*!
 * @brief       Tests the LCD Contrast.
 *
 *              Scrolls through Bias Levels.
 *
 * @details     This function scolls up through the different contrast levels and subsequently
 *              scrolls back down through them to show they all work correctly.
 *
 */
void TestContrast(void)
{
    uint32_t i;
    bool_t   bVLCDState;

    for(i = 0; i < 32; i++)
    {
        /* Set the new contrast level */
        if(adi_LCD_SetContrast(hLCD, i) != ADI_LCD_SUCCESS)
        {
            FAIL("Failed to set the LCD contrast");
        }

        Delay(3);

        /* Wait until the charge pump reaches the new bias level */
        do
        {
            if(adi_LCD_GetVLCDStatus(hLCD, &bVLCDState) != ADI_LCD_SUCCESS)
            {
                FAIL("Failed to query the VLCD status");
            }
        } while(bVLCDState != true);
    }

    /* Set the contrast back to normal level */
    if(adi_LCD_SetContrast(hLCD, 15) != ADI_LCD_SUCCESS)
    {
        FAIL("Failed to set the LCD contrast");
    }

    /* Clear all segments */
    if(adi_LCD_Reset(hLCD) != ADI_LCD_SUCCESS)
    {
        FAIL("Failed to reset LCD Data registers");
    }

    return;
}


/*!
 * @brief       Tests segment character display.
 *
 *
 * @details     This function scrolls through upper case letters and numbers on
 *              segments of the display.
 *
 */
void TestDisplayString(void)
{
    uint8_t dispString[ADI_LCDVIM828_NUM_SEGMENTS];
    uint8_t dispChar;
    uint32_t i;

    /* Scroll through upper case letters */
    for(dispChar = 'A'; dispChar <= 'Z'; dispChar++)
    {
        for(i = 0; i < ADI_LCDVIM828_NUM_SEGMENTS; i++)
        {
            dispString[i] = dispChar;
        }

        /* Display the letter in the array */
        if(adi_LCDVIM828_DisplayString(hLCD, ADI_LCD_SCREEN_0, dispString) != ADI_LCDVIM828_SUCCESS)
        {
            FAIL("Display String");
        }

        Delay(10);
    }
    
    /* Scroll through numbers */
    for(dispChar = '0'; dispChar <= '9'; dispChar++)
    {
        for(i = 0; i < ADI_LCDVIM828_NUM_SEGMENTS; i++)
        {
            dispString[i] = dispChar;
        }

        /* Display the letter in the array */
        if(adi_LCDVIM828_DisplayString(hLCD, ADI_LCD_SCREEN_0, dispString) != ADI_LCDVIM828_SUCCESS)
        {
            FAIL("Display String");
        }

        Delay(10);
    }
    
    /* Display Decimal point */
    for(dispChar = '.'; dispChar <= '.'; dispChar++)
    {
        for(i = 0; i < ADI_LCDVIM828_NUM_SEGMENTS; i++)
        {
            dispString[i] = dispChar;
        }

        /* Display the letter in the array */
        if(adi_LCDVIM828_DisplayString(hLCD, ADI_LCD_SCREEN_0, dispString) != ADI_LCDVIM828_SUCCESS)
        {
            FAIL("Display String");
        }

        Delay(10);
    }

    /* At the end clear the measurement */
    if(adi_LCDVIM828_ClearDisplay(hLCD, ADI_LCD_SCREEN_0) != ADI_LCDVIM828_SUCCESS)
    {
        FAIL("Failed to clear display");
    }

    return;
}


/*!
 * @brief       Tests blinking between a screen and off state.
 *
 *
 * @details     This function tests switching screen 0/1 and off state.
 *
 */
void TestBlink(bool_t bAutomatic)
{
    /* Select the screen to blink */
    if(adi_LCD_SetScreenSelect(hLCD, ADI_LCD_SCREEN_0) != ADI_LCD_SUCCESS)
    {
        FAIL("Failed to set the screen select");
    }

    /* Enable switch between screen0 and off state */
    if(adi_LCD_SetAutoSwitch(hLCD, false) != ADI_LCD_SUCCESS)
    {
        FAIL("Failed to to set Auto Switch");
    }

    /* If hardware switching to be used */
    if(bAutomatic == true)
    {
        /* Set the blink frequency before enabling hardware based blink mode */
        if(adi_LCD_SetBlinkFreq(hLCD, ADI_LCD_BLINK_FREQ_1HZ) != ADI_LCD_SUCCESS)
        {
            FAIL("Failed to set the blink frequency");
        }

        /* Enable hardware switching */
        if(adi_LCD_SetBlinkMode(hLCD, ADI_LCD_BLINK_MODE_HARDWARE) != ADI_LCD_SUCCESS)
        {
            FAIL("Failed to enable hardware blink mode");
        }

        /* Wait to observe blinking */
        Delay(150);
    }
    /* software based blinking */
    else
    {
        uint32_t i;

        /* Enable software based switching */
        if(adi_LCD_SetBlinkMode(hLCD, ADI_LCD_BLINK_MODE_SOFTWARE) != ADI_LCD_SUCCESS)
        {
            FAIL("Failed to enable software based blink mode");
        }

        /* Switch between the screens */
        for(i = 0; i < 12; i++)
        {
            if(adi_LCD_ToggleLCD(hLCD) != ADI_LCD_SUCCESS)
            {
                FAIL("Failed to toggle the LCD");
            }

        /* Wait before switching to the other screen */
        Delay(15);
        }
    }

    /* Disable blinking */
    if(adi_LCD_SetBlinkMode(hLCD, ADI_LCD_BLINK_MODE_DISABLE) != ADI_LCD_SUCCESS)
    {
        FAIL("Failed to disable blink mode");
    }

    /* Clear all the symbols */
    if(adi_LCD_Reset(hLCD) != ADI_LCD_SUCCESS)
    {
        FAIL("Failed to reset LCD Data registers");
    }

    return;
}

/* Timer callback */
void GPTimerCallback(void *pCBParam, uint32_t Event, void *pArg)
{

    switch((ADI_GPT_EVENT_TYPE) Event)
    {
        case ADI_GPT_EVENT_TIMEOUT:
            /* Signal that the timer has expired */
            timerOn = true;

            /* Stop the timer */
            DisableTimer();

            break;

        default:
            FAIL("Unexpected event");
            break;
    }

    return;
}

/* LCD callback */
void LCDCallback(void *pCBParam,uint32_t nEvent, void *EventArg)
{
    /* Handle the events */
    switch((ADI_LCD_EVENT_TYPE) nEvent)
    {
        /* Charge pump good status went low */
        case ADI_LCD_EVENT_CP_GD:
            FAIL("Charge pump good status went low");
            break;

        /* Frame boundary interrupt */
        case ADI_LCD_EVENT_FRAME_BOUNDARY:
            break;
    }

    return;
}