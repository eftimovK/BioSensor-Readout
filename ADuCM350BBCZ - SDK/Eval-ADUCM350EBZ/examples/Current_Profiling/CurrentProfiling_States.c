#define ASSERT_ADI_MISRA_SUPPRESSIONS  /*!< Apply ADI MISRA Suppressions */
#include "misra.h"

#include <stdio.h>

#include "arm_math.h"

#include "CurrentProfiling.h"
#include "test_common.h"
#include "afe.h"
#include "afe_lib.h"
#include "lcd.h"
#include "lcd_VIM828.h"
#include "rtc.h"

/* Global Variables */
volatile    bool_t              bCommonInterruptFlag;
static      ADI_CP_STATE_TYPE   previousState;
static      ADI_CP_STATE_TYPE   currentState;
static      ADI_CP_STATE_TYPE   nextState = ADI_CP_READY_STATE;
static      char                currentDisplayString[4][ADI_LCDVIM828_NUM_SEGMENTS + 1];
static      uint8_t             currDSIncr = 0;

/* Device handles */
ADI_AFE_DEV_HANDLE      hAFE = NULL;
ADI_GPT_HANDLE          hGPT = NULL;
ADI_LCD_DEV_HANDLE      hLCD = NULL;
ADI_RTC_HANDLE          hRTC = NULL;

#pragma location = "nonvolatile_ram"
uint32_t            seq_afe_acmeas10khz_CP[] = {
                        0x0015006D,   /* Safety word: bits 31:16 = command count, bits 7:0 = CRC            */
                        0x84005818,   /* AFE_FIFO_CFG: DATA_FIFO_SOURCE_SEL = 10 (DFT)                      */
                        0x8A000034,   /* AFE_WG_CFG: TYPE_SEL = 10                                          */
                        0x9800A3D7,   /* AFE_WG_CFG: SINE_FCW = 0x0A3D7                                     */
                        0x9E000517,   /* AFE_WG_AMPLITUDE: SINE_AMPLITUDE = 0x517                           */
                        0x88000F01,   /* AFE_DAC_CFG: DAC_ATTEN_EN = 1                                      */
                        0x86008811,   /* DMUX_STATE = 1, PMUX_STATE = 1, NMUX_STATE = 8, TMUX_STATE = 8     */
                        0xA0000002,   /* AFE_ADC_CFG: MUX_SEL = 00010, GAIN_OFFS_SEL = 00 (TIA)             */
                        0x00000640,   /* Wait 100us                                                         */
                        0x80024EF0,   /* AFE_CFG: WAVEGEN_EN = 1                                            */
                        0x00000C80,   /* Wait 200us                                                         */
                        0x8002CFF0,   /* AFE_CFG: ADC_CONV_EN = 1, DFT_EN = 1                               */
                        0x00032340,   /* Wait 13ms ( -148us to stop at midscale)                            */
                        0x80020EF0,   /* AFE_CFG: WAVEGEN_EN = 0, ADC_CONV_EN = 0, DFT_EN = 0               */
                        0x86004433,   /* DMUX_STATE = 3, PMUX_STATE = 3, NMUX_STATE = 4, TMUX_STATE = 4     */
                        0x00000640,   /* Wait 100us                                                         */
                        0x80024EF0,   /* AFE_CFG: WAVEGEN_EN = 1                                            */
                        0x00000C80,   /* Wait 200us                                                         */
                        0x8002CFF0,   /* AFE_CFG: ADC_CONV_EN = 1, DFT_EN = 1                               */
                        0x00032340,   /* Wait 13ms ( -148us to stop at midscale)                            */
                        0x80020EF0,   /* AFE_CFG: WAVEGEN_EN = 0, ADC_CONV_EN = 0, DFT_EN = 0               */
                        0x82000002,   /* AFE_SEQ_CFG: SEQ_EN = 0                                            */
                    };

/* Function Prototypes */
extern      int32_t     adi_initpinmux_setUART              (void);
extern      int32_t     adi_initpinmux_clearUART            (void);
extern      int32_t     adi_initpinmux_setLCD               (void);
extern      int32_t     adi_initpinmux_clearLCD             (void);

void                    adi_CP_configureGpio                (void);
#if (RTC_ENABLE == 1)
void                    adi_CP_configureRtc                 (void);
#endif /* RTC_ENABLE */
q15_t                   arctan                              (q15_t              imag, 
                                                             q15_t              real);
void                    convert_dft_results                 (int16_t            *dft_results, 
                                                             q15_t              *dft_results_q15, 
                                                             q31_t              *dft_results_q31);
fixed32_t               calculate_magnitude                 (q31_t              magnitude_rcal, 
                                                             q31_t              magnitude_z);
fixed32_t               calculate_phase                     (q15_t              phase_rcal, 
                                                             q15_t              phase_z);
void                    sprintf_fixed32                     (char               *out, 
                                                             fixed32_t          in);
#if (RESULTS_PRINT == 1)    
void                    print_results                       (char               *text, 
                                                             fixed32_t          magnitude, 
                                                             fixed32_t          phase);
#endif /* RESULTS_PRINT */
void                    clockGatePeripherals                (void);
ADI_LCD_RESULT_TYPE     adi_CP_EnableLcd                    (ADI_LCD_DEV_HANDLE hLCD, 
                                                             bool_t             bFlag);
ADI_GPT_RESULT_TYPE     InitializeTimer                     (void);
ADI_GPT_RESULT_TYPE     EnableTimer                         (uint16_t           timeout);
ADI_GPT_RESULT_TYPE     DisableTimer                        (void);
void                    Delay                               (uint16_t           TimeOut);
#if (RTC_ENABLE == 1)
uint32_t                BuildSeconds                        (void);
#endif /* RTC_ENABLE */
#if (DEBUGLEDS_ENABLE == 1)
void                    BSP_LED_On                          (uint8_t            led);
void                    BSP_LED_Off                         (uint8_t            led);
#endif /* DEBUGLEDS_ENABLE */
void                    GPTimerCallback                     (void               *pCBParam, 
                                                             uint32_t           Event, 
                                                             void               *pArg);
void                    LCDCallback                         (void               *pCBParam,
                                                             uint32_t           nEvent, 
                                                             void               *EventArg);
void                    ExtIntCallback                      (void               *pCBParam, 
                                                             uint32_t           Event, 
                                                             void               *pArg);

/* State Functions */

/* Initialization State
 *          - This state is entered once at the beginning and is
 *            not re-entered
 *          - Configure GPIOs and external interrupts
 *          - Enable LFXTAL and initialize RTC (if required)
 *          - Clock Gate all peripherals
 */
ADI_CP_STATE_TYPE adi_CP_InitState () {
    
    currentState = nextState;
    
#ifndef TEST_COMMON_USES_UART
    /* UART won't be used, so change UART pins to GPIO to allow disabling */
    adi_initpinmux_clearUART();
#endif /* TEST_COMMON_USES_UART */

    /* Configure GPIOs and external interrupts */
    adi_CP_configureGpio();
    
#if (RTC_ENABLE == 1)
    /* Configure the RTC and LFXTAL */
    adi_CP_configureRtc();
#endif /* RTC_ENABLE */

    /* Clock Gate all peripherals to save power */
    clockGatePeripherals();
    
    previousState = currentState;
    nextState = ADI_CP_READY_STATE;
    
    return nextState;
}
   
/* State 1: Ready State
 *          - Internal RC oscillator enabled, 1MHz
 *          - PCLK gated
 *          - Flash not clocked
 *          - SRAM off
 *          - Clock tree off
 */
ADI_CP_STATE_TYPE adi_CP_ReadyState () {
    currentState = nextState;
    
    /* If previous state used LCD */ 
    if ((previousState == ADI_CP_ACTIVE_DISPLAY_STATE) || 
        (previousState == ADI_CP_STATIC_DISPLAY_SYS_SLEEP_STATE))
    {
        /* Clear LCD pin muxing */
        adi_initpinmux_clearLCD();
        /* Configure external interrupts and disable GPIOs */
        adi_CP_configureGpio();
    }

    /* NOTE: Care must be taken when changing the HCLK and PLCK dividers.   */
    /* HCLK must always be greater than or equal to PCLK, so the order in   */
    /* which they are changed will vary.                                    */

    /* Change HCLK frequency to 1MHz */
    if (SetSystemClockDivider (ADI_SYS_CLOCK_CORE, 16))
    {
        FAIL("SetSystemClockDivider (HCLK) failed");
    }

    /* Change PCLK frequency to 1MHz */
    if (SetSystemClockDivider (ADI_SYS_CLOCK_PCLK, 16))
    {
        FAIL("SetSystemClockDivider (PCLK) failed");
    }

    bCommonInterruptFlag = false;

    /* Enter CORE_SLEEP until a state chage is requested */
    if (SystemEnterLowPowerMode(ADI_SYS_MODE_CORE_SLEEP, &bCommonInterruptFlag, 0)) 
    {
        FAIL("SystemEnterLowPowerMode (CORE_SLEEP) failed");
    }
    
    previousState = currentState;
    
    /* Interrupt has been received, return and transition to next state */
    return nextState;
}

/* State 2: AFE Measurement State
 *          - Internal RC oscillator enabled, 16MHz
 *          - DMA active
 *          - PCLK gated
 *          - Run the AFE sequence from SRAM1
 *          - Flash not clocked
 */
ADI_CP_STATE_TYPE adi_CP_AfeMeasurementState (int16_t *dft_results) {
#if (RESULTS_PRINT == 1)
    char                msg[MSG_MAXLEN];
#endif /* RESULTS_PRINT */
    
    currentState = nextState;

    /* If previous state used LCD */ 
    if ((previousState == ADI_CP_ACTIVE_DISPLAY_STATE) || 
        (previousState == ADI_CP_STATIC_DISPLAY_SYS_SLEEP_STATE))
    {
        /* Clear LCD pin muxing */
        adi_initpinmux_clearLCD();
        /* Configure external interrupts and disable GPIOs */
        adi_CP_configureGpio();
    }

    /* NOTE: Care must be taken when changing the HCLK and PLCK dividers.   */
    /* HCLK must always be greater than or equal to PCLK, so the order in   */
    /* which they are changed will vary.                                    */

    /* Change HCLK (AFE_ADC_CLK) frequency to 16MHz */
    if (SetSystemClockDivider(ADI_SYS_CLOCK_AFE, 1))
    {
        FAIL("SetSystemClockDivider (HCLK) failed");
    }
    
    /* Change PCLK frequency to 1MHz */
    if (SetSystemClockDivider(ADI_SYS_CLOCK_PCLK, 16))
    {
        FAIL("SetSystemClockDivider (PCLK) failed");
    }

    /* Initialize the AFE API */
    if (adi_AFE_Init(&hAFE)) 
    {
        FAIL("AFE_Init");
    }

    /* AFE power up */
    if (adi_AFE_PowerUp(hAFE)) 
    {
        FAIL("AFE_PowerUp");
    }

    /* Excitation Channel Power-Up */
    if (adi_AFE_ExciteChanPowerUp(hAFE)) 
    {
        FAIL("AFE_ExciteChanCalAtten");
    }

    /* TIA Channel Calibration */
    if (adi_AFE_TiaChanCal(hAFE)) 
    {
        FAIL("TiaChanCal");
    }

    /* Excitation Channel Calibration (Attenuation Enabled) */
    if (adi_AFE_ExciteChanCalAtten(hAFE)) 
    {
        FAIL("AFE_ExciteChanCalAtten");
    }

    /* Loop until a state change is requested */
    while (nextState == currentState)
    {
        /* AC Measurement */
        if (adi_AFE_RunSequence(hAFE, seq_afe_acmeas10khz_CP, (uint16_t *)dft_results, DFT_RESULTS_COUNT)) {
            FAIL("AC Measurement");
        }
    }
    
#if (RESULTS_PRINT == 1)    
    PERF("DFT results (real, imaginary):");
    sprintf(msg, "    RCAL        = (%6d, %6d)", dft_results[0], dft_results[1]);
    PERF(msg);
    sprintf(msg, "    AFE2 - AFE1 = (%6d, %6d)", dft_results[2], dft_results[3]);
    PERF(msg);
#endif /* RESULTS_PRINT */

    /* AFE Power Down */
    if (adi_AFE_PowerDown(hAFE)) 
    {
        FAIL("PowerDown");
    }

    /* Uninitialize the AFE API */
    if (adi_AFE_UnInit(hAFE)) 
    {
        FAIL("Init");
    }

    previousState = currentState;
    
    /* Interrupt has been received, return and transition to next state */
    return nextState;
}

/* State 3: Active Core State
 *          - The core is used to process AFE measurement results
 *              - SRAM0 used
 *              - Clocked at 16MHz
 *          - PCLK gated
 *          - DMA disabled
 */
ADI_CP_STATE_TYPE adi_CP_ActiveCoreState (int16_t *dft_results, fixed32_t *magnitude_result, fixed32_t *phase_result) {
    q15_t               dft_results_q15[DFT_RESULTS_COUNT];
    q31_t               dft_results_q31[DFT_RESULTS_COUNT];
    q31_t               magnitude[DFT_RESULTS_COUNT / 2];
    q15_t               phase[DFT_RESULTS_COUNT / 2];
    uint8_t             i;
    
    currentState = nextState;

    /* If previous state used LCD */ 
    if ((previousState == ADI_CP_ACTIVE_DISPLAY_STATE) || 
        (previousState == ADI_CP_STATIC_DISPLAY_SYS_SLEEP_STATE))
    {
        /* Clear LCD pin muxing */
        adi_initpinmux_clearLCD();
        /* Configure external interrupts and disable GPIOs */
        adi_CP_configureGpio();
    }

    /* NOTE: Care must be taken when changing the HCLK and PLCK dividers.   */
    /* HCLK must always be greater than or equal to PCLK, so the order in   */
    /* which they are changed will vary.                                    */

    /* Change Core clock frequency to 16MHz */
    if (SetSystemClockDivider (ADI_SYS_CLOCK_CORE, 16))
    {
        FAIL("SetSystemClockDivider (HCLK) failed");
    }
    
     /* Change PCLK frequency to 1MHz */
    if (SetSystemClockDivider (ADI_SYS_CLOCK_PCLK, 16))
    {
        FAIL("SetSystemClockDivider (PCLK) failed");
    }
    
    /* Loop until a state change is requested */
    while (nextState == currentState)
    {   
        /* Convert DFT results to 1.15 and 1.31 formats.  */
        convert_dft_results(dft_results, dft_results_q15, dft_results_q31);

        /* Magnitude calculation */
        /* Use CMSIS function */
        arm_cmplx_mag_q31(dft_results_q31, magnitude, DFT_RESULTS_COUNT / 2);

        /* Calculate final magnitude values, calibrated with RCAL. */
        for (i = 0; i < DFT_RESULTS_COUNT / 2 - 1; i++) 
        {
            magnitude_result[i] = calculate_magnitude(magnitude[0], magnitude[i + 1]);
        }

        /* Phase calculation */
        /* RCAL first */
        phase[0] = arctan(dft_results[1], dft_results[0]);
        for (i = 0; i < DFT_RESULTS_COUNT / 2 - 1; i++) 
        {
            /* No need to calculate the phase if magnitude is 0 (open circuit) */
            if (magnitude_result[i].full) 
            {
                /* First the measured phase. */
                phase[i + 1]         = arctan(dft_results[2 * (i + 1) + 1], dft_results[2 * (i + 1)]);
                /* Then the phase calibrated with RCAL. */
                phase_result[i]      = calculate_phase(phase[0], phase[i + 1]);
            }
            else 
            {
                phase[i + 1]         = 0;
                phase_result[i].full = 0;
            }
        } 
    }

#if (RESULTS_PRINT == 1)    
    PERF("Final results (magnitude, phase):");
    print_results("AFE2 - AFE1", magnitude_result[0], phase_result[0]);
#endif /* RESULTS_PRINT */

    previousState = currentState;
    
    /* Interrupt has been received, return and transition to next state */
    return nextState;
}

/* State 4: Active Display State
 *          - Actively display results on an LCD
 *          - PCLK enabled
 *              - Only for one GPT and LCD
 *          - Run RC oscillator at 1MHz
 *          - Clock gate all other peripherals
 *          - DMA disabled
 */

ADI_CP_STATE_TYPE adi_CP_ActiveDisplayState (fixed32_t *magnitude_result, fixed32_t *phase_result) {
    bool_t  bVLCDState;
    char    magnitudeLabel[ADI_LCDVIM828_NUM_SEGMENTS + 1]  = {"MAG.    "};
    char    phaseLabel[ADI_LCDVIM828_NUM_SEGMENTS + 1]      = {"PHASE   "};
    char    magnitudeString[ADI_LCDVIM828_NUM_SEGMENTS + 1] = {"00000.00"};
    char    phaseString[ADI_LCDVIM828_NUM_SEGMENTS + 1]     = {"00000.00"};
  
    currentState = nextState;

    /* If previous state did not use LCD, set LCD pin muxing */
    if (previousState != ADI_CP_STATIC_DISPLAY_SYS_SLEEP_STATE)
    {
        adi_initpinmux_setLCD();
    }

    /* NOTE: Care must be taken when changing the HCLK and PLCK dividers.   */
    /* HCLK must always be greater than or equal to PCLK, so the order in   */
    /* which they are changed will vary.                                    */

    /* Change HCLK frequency to 1MHz */
    if (SetSystemClockDivider (ADI_SYS_CLOCK_CORE, 16))
    {
        FAIL("SetSystemClockDivider (HCLK) failed");
    }
    /* Change PCLK frequency to 1MHz */
    if (SetSystemClockDivider (ADI_SYS_CLOCK_PCLK, 16))
    {
        FAIL("SetSystemClockDivider (PCLK) failed");
    }

/* If the LFXTAL (required for RTC) is not enabled, then the LFOSC is required for LCD */
#if (RTC_ENABLE == 0)
    /* If LFOSC is not enabled and stable... */
    if (ADI_SYS_CLOCK_SOURCE_ENABLED_STABLE != GetClockSourceStatus(ADI_SYS_CLOCK_SOURCE_LFOSC))
    {
        /* ... enable it */
#if (PRES3_SILICON == 1)
        SystemClockSourcePowerUp(ADI_SYS_CLOCK_SOURCE_LFOSC, ADI_SYS_CLOCK_TIMEOUT_USE_DEFAULT);
#else
        if (ADI_SYS_SUCCESS != SystemClockSourcePowerUp(ADI_SYS_CLOCK_SOURCE_LFOSC, ADI_SYS_CLOCK_TIMEOUT_USE_DEFAULT))
        {
            FAIL("SystemClockSourcePowerUp (LFOSC) failed");
        }
#endif /* PRES3_SILICON */
    }
#endif /* RTC_ENABLE */
    
    /* Initialise the timer for the display delays */
    if (ADI_GPT_SUCCESS != InitializeTimer())
    {
        FAIL("InitializeTimer Fail");
    }
  
    /* Initialise lcd driver, by passing the configuration structure */
    if(ADI_LCD_SUCCESS != adi_LCD_Init (ADI_LCD_DEVID_0, &hLCD))
    {
        FAIL("adi_LCD_Init Failed");
    }

    /* Install the callback handler */
    if(ADI_LCD_SUCCESS != adi_LCD_RegisterCallback(hLCD, LCDCallback, NULL))
    {
        FAIL("adi_LCD_RegisterCallback Fail");
    }
    
    /* Set the LCD contrast (min by default) */
    if(ADI_LCD_SUCCESS != adi_LCD_SetContrast(hLCD, LCD_MIN_CONTRAST))
    {
        FAIL("adi_LCD_SetContrast Fail");
    }
    
    /* Set the LCD frame rate (min by default) */
    if(ADI_LCD_SUCCESS != adi_LCD_SetFrameRate(hLCD, ADI_LCD_FRAME_RATE_26_9))
    {
        FAIL("adi_LCD_SetFrameRate Fail");
    }
    
    /* Enable LCD */
    if(ADI_LCD_SUCCESS != adi_LCD_Enable (hLCD, true))
    {
        FAIL("adi_LCD_Enable Fail");
    }

    /* Wait until the charge pump reaches the bias level */
    do
    {
        if(ADI_LCD_SUCCESS != adi_LCD_GetVLCDStatus(hLCD, &bVLCDState))
        {
            FAIL("Charge pump failed to reach the bias level (adi_LCD_GetVLCDStatus)");
        }
    } while(bVLCDState != true);
    
    /* Convert DFT magnitude result to string for display */
    sprintf_fixed32(magnitudeString, magnitude_result[0]);
    /* Convert DFT phase result to string for display */
    sprintf_fixed32(phaseString, phase_result[0]);
    
    /* Copy display strings to a global variable so that they can be accessed */
    /* from other states. */
    for (currDSIncr = 0; 
         currDSIncr < ADI_LCDVIM828_NUM_SEGMENTS; 
         currDSIncr++)
    {
        currentDisplayString[0][currDSIncr] = magnitudeLabel[currDSIncr];
        currentDisplayString[1][currDSIncr] = magnitudeString[currDSIncr];
        currentDisplayString[2][currDSIncr] = phaseLabel[currDSIncr];
        currentDisplayString[3][currDSIncr] = phaseString[currDSIncr];
    }
    
    currDSIncr = 0;
    
    /* While waiting for a state change, display the results from State 3 */
    while (nextState == currentState)
    {
        if(adi_LCDVIM828_DisplayString(hLCD, ADI_LCD_SCREEN_0, (uint8_t const*) currentDisplayString[currDSIncr]))
        {
            FAIL("adi_LCDVIM828_DisplayString failed");
        }

        /* GPT configured with PCLK as clock source and a pre-scaler of 32768.  */
        /* PCLK is set to 1MHz, so each unit of the delay is equal to 32.768ms. */
        /* For a value of 40 (as below) this gives a display time of ~1.3s.     */
        Delay(40);

        if (currDSIncr == 3)
        {
            currDSIncr = 0;
        }
        else
        {
            currDSIncr++;
        }
    }
    
    /* If next state is not State 5, disable:   */
    /*  - LCD Controller                        */
    /*  - LCD Charge Pump Interrupt             */
    /*  - LCD Charge Pump                       */
    if (nextState != ADI_CP_STATIC_DISPLAY_SYS_SLEEP_STATE)
    {
        if (ADI_LCD_SUCCESS != adi_CP_EnableLcd (hLCD, false))
        {
            FAIL("adi_CP_EnableLcd (disable) failed");
        }
        adi_initpinmux_clearLCD();
    }

    previousState = currentState;
    
    /* Interrupt has been received, return and transition to next state */
    return nextState;
}

/* State 5: Static Display / System Sleep State
 *          - Last result retained on LCD
 *          - Enter System Sleep mode
 *          - Alternatively, display time, updating each second/minute
 */

ADI_CP_STATE_TYPE adi_CP_StaticDisplaySystemSleepState () {
  
    currentState = nextState;
    /* If previous state cleared LCD pin mux, set LCD pin muxing */
    if (previousState != ADI_CP_ACTIVE_DISPLAY_STATE) 
    {
        adi_initpinmux_setLCD();
    }

    /* NOTE: Care must be taken when changing the HCLK and PLCK dividers.   */
    /* HCLK must always be greater than or equal to PCLK, so the order in   */
    /* which they are changed will vary.                                    */

    /* Change HCLK frequency to 1MHz */
    if (SetSystemClockDivider (ADI_SYS_CLOCK_CORE, 16))
    {
        FAIL("SetSystemClockDivider (HCLK) failed");
    }
    /* Change PCLK frequency to 1MHz */
    if (SetSystemClockDivider (ADI_SYS_CLOCK_PCLK, 16))
    {
        FAIL("SetSystemClockDivider (PCLK) failed");
    }
    
/* If the LFXTAL (required for RTC) is not enabled, then the LFOSC is required for LCD */
#if (RTC_ENABLE == 0)
    /* If LFOSC is not enabled and stable... */
    if (ADI_SYS_CLOCK_SOURCE_ENABLED_STABLE != GetClockSourceStatus(ADI_SYS_CLOCK_SOURCE_LFOSC))
    {
        /* ... enable it */
        if (ADI_SYS_SUCCESS != SystemClockSourcePowerUp(ADI_SYS_CLOCK_SOURCE_LFOSC, ADI_SYS_CLOCK_TIMEOUT_USE_DEFAULT))
        {
            FAIL("SystemClockSourcePowerUp (LFOSC) failed");
        }
    }
#endif /* RTC_ENABLE */
    
    /* If the previous state did not have the LCD controller enabled, enable it */
    if (previousState != ADI_CP_ACTIVE_DISPLAY_STATE)
    {
        if (ADI_LCD_SUCCESS != adi_CP_EnableLcd (hLCD, true))
        {
            FAIL("adi_CP_EnableLcd (disable) failed");
        }
    }
    
    bCommonInterruptFlag = false;
    
    /* Enter SYS_SLEEP until a state chage is requested */
    if (SystemEnterLowPowerMode(ADI_SYS_MODE_SYS_SLEEP, &bCommonInterruptFlag, 0)) 
    {
        FAIL("SystemEnterLowPowerMode (SYS_SLEEP) failed");
    }

    /* If next state is not State 4 disable:  *
     *  - LCD Controller                            *
     *  - LCD Charge Pump Interrupt                 *
     *  - LCD Charge Pump                           */
    if (nextState != ADI_CP_ACTIVE_DISPLAY_STATE)
    {
        if (ADI_LCD_SUCCESS != adi_CP_EnableLcd (hLCD, false))
        {
            FAIL("adi_CP_EnableLcd (disable) failed");
        }
        adi_initpinmux_clearLCD();
    }

    previousState = currentState;
    
    /* Interrupt has been received, return and transition to next state */
    return nextState;
}


/* State 6: Hibernate State
 *          - Enter Hibernate mode
 */
ADI_CP_STATE_TYPE adi_CP_HibernateState () {
    uint16_t    temp;
  
    currentState = nextState;

#if (LFOSC_ENABLE == 0)    
    /* Powerdown LFOSC */
    SystemClockSourcePowerDownIfUnused(ADI_SYS_CLOCK_SOURCE_LFOSC);
#endif /* LFOSC_ENABLE */

    pADI_PWR->PWRKEY = PWRKEY_VALUE_KEY1;
    pADI_PWR->PWRKEY = PWRKEY_VALUE_KEY2;

    /* critical region */
    ADI_ENTER_CRITICAL_REGION();
    
        /* Read into a temporary variable */
        temp = pADI_PWR->PWRMOD;
#if (RAMRETENTION_ENABLE == 1)
        /* Upper 8KB of SRAM0 is retained */
        temp |= UPPER8K_RETAINED;
#elif (RAMRETENTION_ENABLE == 0)
        /* Upper 8KB of SRAM0 is not retained */
        temp &= UPPER8K_NOTRETAINED;
#endif /* RAMRETENTION_ENABLE */
        /* Write to register */
        pADI_PWR->PWRMOD = temp;
    
    /* exit critical region */
    ADI_EXIT_CRITICAL_REGION();
    
    bCommonInterruptFlag = false;
    
    /* Enter Hibernate until change of state is requested */
    if (SystemEnterLowPowerMode(ADI_SYS_MODE_HIBERNATE, &bCommonInterruptFlag, 0)) 
    {
        FAIL("SystemEnterLowPowerMode (HIBERNATE) failed");
    }

    previousState = currentState;
    
    /* Interrupt has been received, return and transition to next state */
    return nextState;
}

/* Support Functions */

void adi_CP_configureGpio (void) {
    
    /* Initialize GPIO driver */
    if (ADI_GPIO_SUCCESS != adi_GPIO_Init())
    {
        FAIL("adi_CP_configureGpio: adi_GPIO_Init failed");
    }
    
#ifndef TEST_COMMON_USES_UART
    /* Disable UART pin (P0.6 & P0.7) pull-ups */
    if (ADI_GPIO_SUCCESS != adi_GPIO_SetPullUpEnable(ADI_GPIO_PORT_0, 
                                                     (ADI_GPIO_PIN_6 | ADI_GPIO_PIN_7), 
                                                     false))
    {
        FAIL("adi_CP_configureGpio: adi_GPIO_SetPullUpEnable (UART pins) failed");
    }
#endif /* TEST_COMMON_USES_UART */

#if (ROTARYSW_ENABLE == 1)
    /* Enable required GPIO inputs */
    if (ADI_GPIO_SUCCESS != adi_GPIO_SetInputEnable(ADI_GPIO_PORT_0, 
                                                    (ADI_GPIO_PIN_0 | ADI_GPIO_PIN_1 | 
                                                     ADI_GPIO_PIN_2 | ADI_GPIO_PIN_3), 
                                                    true))
    {
        FAIL("adi_CP_configureGpio: adi_GPIO_SetInputEnable failed");
    }
 
    if (ADI_GPIO_SUCCESS != adi_GPIO_SetInputEnable(ADI_GPIO_PORT_3, 
                                                    ADI_GPIO_PIN_4, 
                                                    true))
    {
        FAIL("adi_CP_configureGpio: adi_GPIO_SetInputEnable failed");
    }

    /* Enable P3.4 pull-up */
    if (ADI_GPIO_SUCCESS != adi_GPIO_SetPullUpEnable(ADI_GPIO_PORT_3, 
                                                     ADI_GPIO_PIN_4,
                                                     true))
    {
        FAIL("adi_CP_configureGpio: adi_GPIO_SetPullUpEnable (P3.4) failed");
    }
        
    /* Install the CommonInterruptHandler for EINT7 */
    ADI_INSTALL_HANDLER(ADI_INT_EINT7, CommonInterruptHandler);
    
    /* Register the GPIO Callback for EINT7 */
    if (ADI_GPIO_SUCCESS != adi_GPIO_RegisterCallback(EINT7_IRQn, ExtIntCallback, NULL))
    {
        FAIL("adi_CP_configureGpio: adi_GPIO_RegisterCallback failed");
    }
    
    /* Enable the EINT7 pin (P3.4) */
    if (ADI_GPIO_SUCCESS != adi_GPIO_EnableIRQ(EINT7_IRQn, ADI_GPIO_IRQ_EITHER_EDGE))
    {
        FAIL("adi_CP_configureGpio: adi_GPIO_EnableIRQ failed");
    }
    
    ADI_ENABLE_INT(EINT7_IRQn);
#endif /* ROTARYSW_ENABLE */

#if (PUSHBUTTONSW_ENABLE == 1)
    /* Enable required GPIO inputs */
    if (ADI_GPIO_SUCCESS != adi_GPIO_SetInputEnable(ADI_GPIO_PORT_0, 
                                                    (ADI_GPIO_PIN_4 |ADI_GPIO_PIN_5), 
                                                    true))
    {
        FAIL("adi_CP_configureGpio: adi_GPIO_SetInputEnable failed");
    }
    
    /* Enable P0.4 and P0.5 pull-ups */
    if (ADI_GPIO_SUCCESS != adi_GPIO_SetPullUpEnable(ADI_GPIO_PORT_0, 
                                                     (ADI_GPIO_PIN_4 | ADI_GPIO_PIN_5 ),
                                                     true))
    {
        FAIL("adi_CP_configureGpio: adi_GPIO_SetPullUpEnable (P0.4 & P0.5) failed");
    }
    
    /* Install the CommonInterruptHandler for EINT5 and EINT6 */
    ADI_INSTALL_HANDLER(ADI_INT_EINT5, CommonInterruptHandler);
    ADI_INSTALL_HANDLER(ADI_INT_EINT6, CommonInterruptHandler);
    
    /* Register the GPIO Callback for EINT5 */
    if (ADI_GPIO_SUCCESS != adi_GPIO_RegisterCallback(EINT5_IRQn, ExtIntCallback, NULL))
    {
        FAIL("adi_CP_configureGpio: adi_GPIO_RegisterCallback failed");
    }
    
    /* Register the GPIO Callback for EINT6 */
    if (ADI_GPIO_SUCCESS != adi_GPIO_RegisterCallback(EINT6_IRQn, ExtIntCallback, NULL))
    {
        FAIL("adi_CP_configureGpio: adi_GPIO_RegisterCallback failed");
    }
    
    /* Enable the EINT5 pin (P0.4) */
    if (ADI_GPIO_SUCCESS != adi_GPIO_EnableIRQ(EINT5_IRQn, ADI_GPIO_IRQ_RISING_EDGE))
    {
        FAIL("adi_CP_configureGpio: adi_GPIO_EnableIRQ failed");
    }
    
    /* Enable the EINT6 pin (P0.5) */
    if (ADI_GPIO_SUCCESS != adi_GPIO_EnableIRQ(EINT6_IRQn, ADI_GPIO_IRQ_RISING_EDGE))
    {
        FAIL("adi_CP_configureGpio: adi_GPIO_EnableIRQ failed");
    }
    
    ADI_ENABLE_INT(EINT5_IRQn);
    ADI_ENABLE_INT(EINT6_IRQn);
#endif /* PUSHBUTTONSW_ENABLE */
}

#if (RTC_ENABLE == 1)
/* Configure the RTC and LFXTAL (which is required for the RTC) */
void adi_CP_configureRtc (void) {
    uint32_t            buildTime  = BuildSeconds();
    ADI_RTC_RESULT_TYPE rtcInitResult;
  
    /* Enable LFXTAL (required for RTC) */
    if (ADI_SYS_SUCCESS != (SystemClockSourcePowerUp(ADI_SYS_CLOCK_SOURCE_LFXTAL, ADI_SYS_CLOCK_TIMEOUT_USE_DEFAULT)))
    {
        FAIL("SystemClockSourcePowerUp (LFXTAL) failed");
    }
    
    /* If the LFCLK Mux is not set to use LFXTAL... */
    if (ADI_SYS_CLOCK_MUX_LFCLK_LFXTAL != GetSystemClockMux(ADI_SYS_CLOCK_MUX_LFCLK))
    {
        /* ... set the LFCLK Mux to use the LFXTAL */
        SetSystemClockMux(ADI_SYS_CLOCK_MUX_LFCLK_LFXTAL);
    }
    
    /* Powerdown LFOSC */
    SystemClockSourcePowerDownIfUnused(ADI_SYS_CLOCK_SOURCE_LFOSC);
    
    /* Initialise RTC */
    if (ADI_RTC_SUCCESS != (rtcInitResult = adi_RTC_Init(ADI_RTC_DEVID_0, &hRTC)))
    {
        PERF("adi_RTC_Init (initial attempt) failed");
        if (ADI_RTC_ERR_CLOCK_FAILSAFE == rtcInitResult)
        {
            /* Clear the failsafe */
            adi_RTC_ClearFailSafe();

            /* Un-init RTC for a clean restart, but ignore failure */
            adi_RTC_UnInit(hRTC);

            /* Re-init RTC */
            if (ADI_RTC_SUCCESS != adi_RTC_Init(ADI_RTC_DEVID_0, &hRTC))
            {
                FAIL("Double failure on adi_RTC_Init");
            }

            PERF("Resetting clock values after init failure");

            /* Set clock to latest build time */
            if (ADI_RTC_SUCCESS != adi_RTC_SetCount(hRTC, buildTime))
            {
                FAIL("adi_RTC_SetCount failed");
            }
        }
    }
    else
    {
        /* Set the clock to the latest build time */
        if (ADI_RTC_SUCCESS != adi_RTC_SetCount(hRTC, buildTime))
        {
            FAIL("adi_RTC_SetCount failed");
        }
    }

    /* Disable alarm */
    if (ADI_RTC_SUCCESS != adi_RTC_EnableAlarm(hRTC, false))
    {
        FAIL("adi_RTC_EnableAlarm failed");
    }

    /* Enable RTC */
    if (ADI_RTC_SUCCESS != adi_RTC_EnableDevice(hRTC, true))
    {
        FAIL("adi_RTC_EnableDevice failed");
    }
}
#endif /* RTC_ENABLE */

/* Arctan Implementation                                                                                    
 * =====================                                                                                    
 * Arctan is calculated using the formula:                                                                  
 *                                                                                                          
 *      y = arctan(x) = 0.318253 * x + 0.003314 * x^2 - 0.130908 * x^3 + 0.068542 * x^4 - 0.009159 * x^5    
 *                                                                                                          
 * The angle in radians is given by (y * pi)                                                                
 *                                                                                                          
 * For the fixed-point implementation below, the coefficients are quantized to 16-bit and                   
 * represented as 1.15                                                                                      
 * The input vector is rotated until positioned between 0 and pi/4. After the arctan                       
 * is calculated for the rotated vector, the initial angle is restored.                                     
 * The format of the output is 1.15 and scaled by PI. To find the angle value in radians from the output    
 * of this function, a multiplication by PI is needed.
 */

const q15_t coeff[5] = {
    (q15_t)0x28BD,     /*  0.318253 */
    (q15_t)0x006D,     /*  0.003314 */
    (q15_t)0xEF3E,     /* -0.130908 */
    (q15_t)0x08C6,     /*  0.068542 */
    (q15_t)0xFED4,     /* -0.009159 */
};

q15_t arctan(q15_t imag, q15_t real) {
    q15_t       t;
    q15_t       out;
    uint8_t     rotation; /* Clockwise, multiples of PI/4 */
    int8_t      i;

    if ((q15_t)0 == imag) 
    {
        /* Check the sign */
        if (real & (q15_t)0x8000) 
        {
            /* Negative, return -PI */
            return (q15_t)0x8000;
        }
        else 
        {
            return (q15_t)0;
        }
    }
    else 
    {
        rotation = 0;
        /* Rotate the vector until it's placed in the first octant (0..PI/4) */
        if (imag < 0) 
        {
            imag      = -imag;
            real      = -real;
            rotation += 4;
        }
        if (real <= 0) 
        {
            /* Using 't' as temporary storage before its normal usage */
            t         = real;
            real      = imag;
            imag      = -t;
            rotation += 2;
        }
        if (real <= imag) 
        {
            /* The addition below may overflow, drop 1 LSB precision if needed. *
             * The subtraction cannot underflow.                                */
            t = real + imag;
            if (t < 0) 
            {
                /* Overflow */
                t         = imag - real;
                real      = (q15_t)(((q31_t)real + (q31_t)imag) >> 1);
                imag      = t >> 1;
            }
            else 
            {
                t         = imag - real;
                real      = (real + imag);
                imag      = t;              
            }
            rotation += 1;
        }

        /* Calculate tangent value */
        t = (q15_t)((q31_t)(imag << 15) / real);

        out = (q15_t)0;

        for (i = 4; i >=0; i--) 
        {
            out += coeff[i];
            arm_mult_q15(&out, &t, &out, 1);
        }
        
        /* Rotate back to original position, in multiples of pi/4           *
         * We're using 1.15 representation, scaled by pi, so pi/4 = 0x2000  */
        out += (rotation << 13);

        return out;
    }
}

/* This function performs dual functionality:                                   *
 *      - open circuit check: the real and imaginary parts can be non-zero but  * 
 *        very small due to noise. If they are within the defined thresholds,   *
 *        overwrite them with 0s, this will indicate an open.                   *
 *      - convert the int16_t to q15_t and q31_t formats, needed for the        *
 *        magnitude and phase calculations.                                     */
void convert_dft_results(int16_t *dft_results, q15_t *dft_results_q15, q31_t *dft_results_q31) {
    int8_t      i;

    for (i = 0; i < (DFT_RESULTS_COUNT / 2); i++) 
    {
        if ((dft_results[i] < DFT_RESULTS_OPEN_MAX_THR) &&
            (dft_results[i] > DFT_RESULTS_OPEN_MIN_THR) &&              /* real part */
            (dft_results[2 * i + 1] < DFT_RESULTS_OPEN_MAX_THR) &&
            (dft_results[2 * i + 1] > DFT_RESULTS_OPEN_MIN_THR))        /* imaginary part */
        {

            /* Open circuit, force both real and imaginary parts to 0 */
            dft_results[i]           = 0;
            dft_results[2 * i + 1]   = 0;
        }
    }

    /*  Convert to 1.15 format */
    for (i = 0; i < DFT_RESULTS_COUNT; i++) 
    {
        dft_results_q15[i] = (q15_t)dft_results[i];
    }

    /*  Convert to 1.31 format */
    arm_q15_to_q31(dft_results_q15, dft_results_q31, DFT_RESULTS_COUNT);
}

/* Calculates calibrated magnitude.                                             *
 * The input values are the measured RCAL magnitude (magnitude_rcal) and the    *
 * measured magnitude of the unknown impedance (magnitude_z).                   *
 * Performs the calculation:                                                    *
 *      magnitude = magnitude_rcal / magnitude_z * RCAL                         *
 * Output in custom fixed-point format (28.4).                                  */
fixed32_t calculate_magnitude(q31_t magnitude_rcal, q31_t magnitude_z) {
    q63_t       magnitude;
    fixed32_t   out;

    magnitude = (q63_t)0;
    if ((q63_t)0 != magnitude_z) 
    {
        magnitude = (q63_t)magnitude_rcal * (q63_t)RCAL;
        /* Shift up for additional precision and rounding */
        magnitude = (magnitude << 5) / (q63_t)magnitude_z;
        /* Rounding */
        magnitude = (magnitude + 1) >> 1;
    }

    /* Saturate if needed */
    if (magnitude &  0xFFFFFFFF00000000) 
    {
        /* Cannot be negative */
        out.full = 0x7FFFFFFF;
    }
    else 
    {
        out.full = magnitude & 0xFFFFFFFF;
    }
        
    return out;
}

/* Calculates calibrated phase.                                                 *
 * The input values are the measured RCAL phase (phase_rcal) and the measured   *
 * phase of the unknown impedance (magnitude_z).                                *
 * Performs the calculation:                                                    *
 *      phase = (phase_z - phase_rcal) * PI / (2 * PI) * 180                    *
 *            = (phase_z - phase_rcal) * 180                                    *
 * Output in custom fixed-point format (28.4).                                  */
fixed32_t calculate_phase(q15_t phase_rcal, q15_t phase_z) {
    q63_t       phase;
    fixed32_t   out;

    /* Multiply by 180 to convert to degrees */
    phase = ((q63_t)(phase_z - phase_rcal) * (q63_t)180);
    /* Round and convert to fixed32_t */
    out.full = ((phase + (q63_t)0x400) >> 11) & 0xFFFFFFFF;

    return out;
}

/* Simple conversion of a fixed32_t variable to string format. */
void sprintf_fixed32(char *out, fixed32_t in) {
    fixed32_t   tmp;
    
    if (in.full < 0) 
    {
        tmp.parts.fpart = (16 - in.parts.fpart) & 0x0F;
        tmp.parts.ipart = in.parts.ipart;
        if (0 != in.parts.fpart) 
        {
            tmp.parts.ipart++;
        }
        if (0 == tmp.parts.ipart) 
        {
            sprintf(out, "      -0.%02d", tmp.parts.fpart * FIXED32_LSB_SIZE);
        }
        else 
        {
            sprintf(out, "%d.%02d", tmp.parts.ipart, tmp.parts.fpart * FIXED32_LSB_SIZE);
        }
    }
    else 
    {
        sprintf(out, "%d.%02d", in.parts.ipart, in.parts.fpart * FIXED32_LSB_SIZE);
    }

}

#if (RESULTS_PRINT == 1)    
/* Helper function for printing AFE results to the terminal/UART  */
void print_results(char *text, fixed32_t magnitude, fixed32_t phase) {
    char                msg[MSG_MAXLEN];
    char                tmp[MSG_MAXLEN];

    sprintf(msg, "    %s = (", text);
    /* Magnitude */
    sprintf_fixed32(tmp, magnitude);
    strcat(msg, tmp);
    strcat(msg, ", ");
    /* Phase */
    sprintf_fixed32(tmp, phase);
    strcat(msg, tmp);
    strcat(msg, ")");

    PERF(msg);
}
#endif /* RESULTS_PRINT */

/* Clock gate all (unused) peripheral clocks to save power. */
/* This function uses multiple calls to the SystemEnableClock function to   */
/* gate the peripheral clocks. It is worth noting that this task could be   */
/* performed more efficiently using a single register write.                */
void clockGatePeripherals(void) {
    /* Clock Gate CTCLK */
    if (SystemEnableClock (ADI_SYS_CLOCK_GATE_CTCLK, false))
    {
        FAIL("SystemEnableClock (CTCLK disable) failed");
    }
    
     /* Clock Gate ACLK */
    if (SystemEnableClock (ADI_SYS_CLOCK_GATE_ACLK, false))
    {
        FAIL("SystemEnableClock (ACLK disable) failed");
    }
    
     /* Clock Gate UCLK_I2S */
    if (SystemEnableClock (ADI_SYS_CLOCK_GATE_I2S, false))
    {
        FAIL("SystemEnableClock (UCLK_I2S disable) failed");
    }
    
     /* Clock Gate UCLK_UART */
    if (SystemEnableClock (ADI_SYS_CLOCK_GATE_UART, false))
    {
        FAIL("SystemEnableClock (UCLK_UART disable) failed");
    }
    
     /* Clock Gate UCLK_I2C */
    if (SystemEnableClock (ADI_SYS_CLOCK_GATE_I2C, false))
    {
        FAIL("SystemEnableClock (UCLK_I2C disable) failed");
    }
    
     /* Clock Gate UCLK_SPIH */
    if (SystemEnableClock (ADI_SYS_CLOCK_GATE_SPIH, false))
    {
        FAIL("SystemEnableClock (UCLK_SPIH disable) failed");
    }
    
     /* Clock Gate UCLK_SPI1 */
    if (SystemEnableClock (ADI_SYS_CLOCK_GATE_SPI1, false))
    {
        FAIL("SystemEnableClock (UCLK_SPI1 disable) failed");
    }
    
     /* Clock Gate UCLK_SPI0 */
    if (SystemEnableClock (ADI_SYS_CLOCK_GATE_SPI0, false))
    {
        FAIL("SystemEnableClock (UCLK_SPI0 disable) failed");
    }
}

ADI_LCD_RESULT_TYPE adi_CP_EnableLcd (ADI_LCD_DEV_HANDLE hLCD, bool_t bFlag){
    ADI_LCD_RESULT_TYPE result = ADI_LCD_SUCCESS;
    bool_t              bVLCDState;

    if (bFlag)
    {
        /* Enable the LCD controller */
        if (result = adi_LCD_Enable(hLCD, true))
        {
            return result;
        }

        /* Enable the LCD charge pump */
        if (result = adi_LCD_EnableChargePump(hLCD, true))
        {
            return result;
        }

        /* Wait until the charge pump reaches the bias level */
        do
        {
            if (result = adi_LCD_GetVLCDStatus(hLCD, &bVLCDState))
            {
                return result;
            }
        } while (bVLCDState != true);

        /* Enable the LCD charge pump interrupt */
        if (result = adi_LCD_SetChargePumpIntEnable(hLCD, true))
        {
            return result;
        }
    }
    else
    {
        /* Disable the LCD controller */
        if (result = adi_LCD_Enable(hLCD, false))
        {
            return result;
        }

        /* Disable the LCD charge pump */
        if (result = adi_LCD_EnableChargePump(hLCD, false))
        {
            return result;
        }

        /* Disable the LCD charge pump interrupt */
        if (result = adi_LCD_SetChargePumpIntEnable(hLCD, false))
        {
            return result;
        }
    }

    return result;
}

/*!
 * @brief       Initialize Timer Function.
 *
 * @return      Status
 *              - #ADI_GPT_SUCCESS              Call completed successfully.
 *
 * @details     This function initializes timer0.
 *
 */
ADI_GPT_RESULT_TYPE InitializeTimer() {
    ADI_GPT_RESULT_TYPE result = ADI_GPT_SUCCESS;

    /* Initialize timer0 */
    if (ADI_GPT_SUCCESS != (result = adi_GPT_Init(ADI_GPT_DEVID_0, &hGPT)))
    {
        return result;
    }

    /* Set to count down */
    if (ADI_GPT_SUCCESS != (result = adi_GPT_SetCountMode(hGPT, ADI_GPT_COUNT_DOWN)))
    {
        return result;
    }

    /* Select PCLK as source */
    if (ADI_GPT_SUCCESS != (result = adi_GPT_SetClockSelect(hGPT, ADI_GPT_CLOCK_SELECT_PCLK)))
    {
        return result;
    }

    /* Set the prescaler */
    if (ADI_GPT_SUCCESS != (result = adi_GPT_SetPrescaler(hGPT, ADI_GPT_PRESCALER_32768)))
    {
        return result;
    }

    /* Register the call back */
    if (ADI_GPT_SUCCESS != (result = adi_GPT_RegisterCallback(hGPT, GPTimerCallback, NULL)))
    {
        return result;
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

    /* Program timeout value */
    if (ADI_GPT_SUCCESS != (result = adi_GPT_SetLdVal(hGPT, timeout)))
    {
        return result;
    }

    /* Start the timer */
    if (ADI_GPT_SUCCESS != (result = adi_GPT_SetTimerEnable(hGPT, true)))
    {
        return result;
    }

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

    /* Stop timer count (disable) */
    if (ADI_GPT_SUCCESS != (result = adi_GPT_SetTimerEnable(hGPT, false)))
    {
        return result;
    }

    return result;
}

/*!
 * @brief       Delay Function.
 *
 * @return      Status
 *              - #ADI_GPT_SUCCESS                      Call completed successfully.
 *
 * @details     This function enters Core Sleep mode until the given timout is 
 *              reached. Uses timer0 for wait.
 *
 */
void Delay(uint16_t TimeOut)
{
    /* Enable the timer by setting the given timeout */
    if (ADI_GPT_SUCCESS != EnableTimer(TimeOut))
    {
        FAIL("EnableTimer() failed");
    }

    bCommonInterruptFlag = false;
    
    SystemEnterLowPowerMode(ADI_SYS_MODE_CORE_SLEEP, &bCommonInterruptFlag, 0);
}

#if (RTC_ENABLE == 1)
/*!
 * @brief       Build Seconds Function.
 *
 * @return      Number of seconds
 *
 * @details     This function calculates the number of seconds from epoc (1/1/70) 
 *              to the most recient build time for the RTC.
 *
 */
uint32_t BuildSeconds(void)
{
    char        timestamp[]     = __DATE__ " " __TIME__;
    int         month_days[]    = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    uint32_t    days, month, date, year, hours, minutes, seconds;
    char        Month[4];

    /* Parse the build timestamp */
    sscanf(timestamp, "%s %d %d %d:%d:%d", Month, &date, &year, &hours, &minutes, &seconds);

    /* Parse ASCII month to a value */
    if     ( !strncmp(Month, "Jan", 3 )) month = 1;
    else if( !strncmp(Month, "Feb", 3 )) month = 2;
    else if( !strncmp(Month, "Mar", 3 )) month = 3;
    else if( !strncmp(Month, "Apr", 3 )) month = 4;
    else if( !strncmp(Month, "May", 3 )) month = 5;
    else if( !strncmp(Month, "Jun", 3 )) month = 6;
    else if( !strncmp(Month, "Jul", 3 )) month = 7;
    else if( !strncmp(Month, "Aug", 3 )) month = 8;
    else if( !strncmp(Month, "Sep", 3 )) month = 9;
    else if( !strncmp(Month, "Oct", 3 )) month = 10;
    else if( !strncmp(Month, "Nov", 3 )) month = 11;
    else if( !strncmp(Month, "Dec", 3 )) month = 12;

    /* Count days from prior years */
    days = 0;
    for (int y = 1970; y < year; y++) 
    {
        days += 365;
        if (LEAP_YEAR(y))
        {
            days += 1;
        }
    }

    /* Add days for current year */
    for (int m = 1; m < month; m++)
    {
        days += month_days[m - 1];
    }

    /* Adjust if current year is a leap year */
    if ((LEAP_YEAR(year) && ((month > 2) || ((month == 2) && (date == 29)))))
    {
        days += 1;
    }

    /* Add days this month (not including current day) */
    days += date - 1;

    return ((days * 24 * 60 * 60) + (hours * 60 * 60) + (minutes * 60) + seconds);
}
#endif /* RTC_ENABLE */

/* Interrupt Handler Callbacks */
    
/* EINT Callback */
void ExtIntCallback (void *pCBParam, uint32_t Event, void *pArg) {
    ADI_GPIO_DATA_TYPE  port0Value = 0;
    ADI_GPIO_DATA_TYPE  port3Value = 0;
    IRQn_Type           currentIRQ = (IRQn_Type) Event;
    
    /* Exit low power mode */
    SystemExitLowPowerMode(&bCommonInterruptFlag);

    adi_GPIO_ClearIRQ(currentIRQ);
        
    /* Delay to allow GPIO value to settle */
    for (uint16_t delay = 0; delay < 0x02FF; delay++);
    
    /* Check the value of the GPIO input */
    port0Value = adi_GPIO_GetData(ADI_GPIO_PORT_0);
    port3Value = adi_GPIO_GetData(ADI_GPIO_PORT_3);
    switch(currentIRQ)
    {
        case EINT7_IRQn:
            /* Assign next state only if Pushbutton P3.4 is pressed */
            if (port3Value  & P04MASK)
            {
                nextState = (ADI_CP_STATE_TYPE)(port0Value  & ROTARYMASK);
            }
            break;
      
        case EINT5_IRQn:
            /* Decrement nextState */
            if (port0Value  & P04MASK)
            {
                if (ADI_CP_READY_STATE == nextState)
                {
                    nextState = ADI_CP_HIBERNATE_STATE;
                }
                else
                {
                    nextState--;
                }
            }
            break;
            
        case EINT6_IRQn:
            /* Increment nextState */
            if (port0Value  & P05MASK)
            {
                if (ADI_CP_HIBERNATE_STATE == nextState)
                {
                    nextState = ADI_CP_READY_STATE;
                }
                else
                {
                    nextState++;
                }
            }
            break;
            
        default:
            FAIL("Unexpected event (ExtIntCallback)");
            break;
    }
    
    return;
}

/* GP Timer callback */
void GPTimerCallback(void *pCBParam, uint32_t Event, void *pArg)
{
    switch((ADI_GPT_EVENT_TYPE) Event)
    {
        case ADI_GPT_EVENT_TIMEOUT:
            /* Exit low power mode */
            SystemExitLowPowerMode(&bCommonInterruptFlag);
        
            /* Stop the timer */
            DisableTimer();

            break;

        default:
            FAIL("Unexpected event (GPTimerCallback)");
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
