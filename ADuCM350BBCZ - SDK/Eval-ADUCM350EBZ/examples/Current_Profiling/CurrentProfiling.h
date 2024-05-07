#ifndef __CURRENTPROFILING_H__
#define __CURRENTPROFILING_H__

/* C++ linkage */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
  
/* Conditional Compilation Macros */
  
/* Set to print debug messages to the terminal/UART                         */
#define DEBUG_PRINT         (0)

/* Set to print AFE measurement results to the terminal/UART                */
#define RESULTS_PRINT       (0)

/* Set to enable Rotary Switch on ADuCM350 Test Header Board Rev.A to move  *
 * between states                                                           */
#define ROTARYSW_ENABLE     (0)

/* Set to enable Push-Botton Switches on ADuCM350 Test Header Board Rev.A   *
 * to move between states                                                   */
#define PUSHBUTTONSW_ENABLE (1)

/* Set to enable the RTC for all states (including State 6: Hibernate)      *
 * Note: the LFXTAL is required for RTC operation. By enabling the RTC, the *
 * LFXTAL will also be enabled for all states. If RTC_ENABLE is not set,    *
 * LFOSC will be enabled for LCD functionality.                             */
#define RTC_ENABLE          (0)

/* Set to retain the full 16kB of SRAM0 in Hibernate Mode.                  *
 * Clear to only retain the lower 8kB of SRAM0 in Hibernate Mode.           */
#define RAMRETENTION_ENABLE (0)

/* Set to keep the LFOSC enabled during State 6: Hibernate.                 *
 * Note: If RTC_ENABLE is not set, the LFOSC will be enabled for States 4   *
 * & 5 (required for LCD), regardless of the setting of LFOSC_ENABLE.       */
#define LFOSC_ENABLE        (0)

/*****************************************************************************
 *
 *  Defines
 *
 *****************************************************************************/

/* The number of results expected from the DFT as part of the AC measurement *
 * (in this case 4 for 2 complex results)                                    */
#define DFT_RESULTS_COUNT           (4u)

/* Memory address locations for data storage */
#define DFTRESULTS_LOCATION         (0x20000000u)
#define MAGNITUDERESULT_LOCATION    (0x20000008u)
#define PHASERESULT_LOCATION        (0x2000000Cu)
#define ACMEAS10KHZCPSEQ_LOCATION   (0x20000010u)

/* RCAL value, in ohms */
#define RCAL                        (1000u)

/* If both real and imaginary result are within the interval                *
 * (DFT_RESULTS_OPEN_MIN_THR, DFT_RESULTS_OPEN_MAX_THR), it is considered   *
 * an open circuit and results for both magnitude and phase will be 0.      */
#define DFT_RESULTS_OPEN_MAX_THR    (10)
#define DFT_RESULTS_OPEN_MIN_THR    (-10)

/* Fractional LSB size for the fixed32_t type defined below, used for       *
 * printing only.                                                           */
#define FIXED32_LSB_SIZE            (625)
#define MSG_MAXLEN                  (50)

/* LCD contrast values for use in State 4 and State 5                       */
#define LCD_MIN_CONTRAST            (0x00u)
#define LCD_MID_CONTRAST            (0x10u)
#define LCD_MAX_CONTRAST            (0x1Fu)
  
/* RAM 0 upper 8kB retention */
#define UPPER8K_RETAINED            (1u << BITP_PWR_PWRMOD_RAM0_RET)
#define UPPER8K_NOTRETAINED         (!UPPER8K_RETAINED)

/* Masks for the GPIO port for retrieving the user input to determine the   *
 * next state                                                               */
#define ROTARYMASK  (ADI_GPIO_PIN_0 + ADI_GPIO_PIN_1 + ADI_GPIO_PIN_2 + ADI_GPIO_PIN_3)
#define P04MASK     (ADI_GPIO_PIN_4)
#define P05MASK     (ADI_GPIO_PIN_5)
   
/* Leap-year compute macro (ignores leap-seconds)                           */
#define LEAP_YEAR(x)                (((0 == (x % 4)) && (0 != (x % 100))) || (0 == (x % 400)))

/* TypeDefs */
  
/* The numbering of the states matches the values found on the rotary       */
/* switch on the test header board. The position of the rotary switch (if   */
/* used) is decoded on GPIO port 0 and the decoded values also correspond   */
/* to the state numbers below.                                              */
typedef enum {
    ADI_CP_INIT_STATE                       = 0xFF,
    ADI_CP_READY_STATE                      = 0x01,
    ADI_CP_AFE_MEASUREMENT_STATE            = 0x02,
    ADI_CP_ACTIVE_CORE_STATE                = 0x03,
    ADI_CP_ACTIVE_DISPLAY_STATE             = 0x04,
    ADI_CP_STATIC_DISPLAY_SYS_SLEEP_STATE   = 0x05,
    ADI_CP_HIBERNATE_STATE                  = 0x06,
    ADI_CP_ERROR_STATE                      = 0x10
} ADI_CP_STATE_TYPE;

typedef enum {
    ADI_CP_SUCCESS                          = 0,
    ADI_CP_GENERIC_ERROR                    = 1,
    ADI_CP_INVALID_STATE_ERROR              = 2
} ADI_CP_ERROR_TYPE;

/* Custom fixed-point type used for final results, to keep track of the     *
 * decimal point position. Signed number with 28 integer bits and 4         *
 * fractional bits.                                                         */
typedef union {
    int32_t     full;
    struct 
    {
        uint8_t fpart:4;
        int32_t ipart:28;
    } parts;
} fixed32_t;

/* Typedef for LED functions                                                */
typedef struct {
    int32_t  port;
    int16_t  pin;
} ADI_LED_DEF;

extern  ADI_CP_STATE_TYPE       adi_CP_InitState                        ();
extern  ADI_CP_STATE_TYPE       adi_CP_ReadyState                       ();
extern  ADI_CP_STATE_TYPE       adi_CP_AfeMeasurementState              (int16_t    *dftResults);
extern  ADI_CP_STATE_TYPE       adi_CP_ActiveCoreState                  (int16_t    *dft_results,
                                                                         fixed32_t  *magnitude_result,
                                                                         fixed32_t  *phase_result);
extern  ADI_CP_STATE_TYPE       adi_CP_ActiveDisplayState               (fixed32_t  *magnitude_result,
                                                                         fixed32_t  *phase_result);
extern  ADI_CP_STATE_TYPE       adi_CP_StaticDisplaySystemSleepState    ();
extern  ADI_CP_STATE_TYPE       adi_CP_HibernateState                   ();

#endif /* __CURRENTPROFILING_H__ */
