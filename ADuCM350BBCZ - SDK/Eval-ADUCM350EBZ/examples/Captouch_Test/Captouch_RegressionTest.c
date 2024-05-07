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
 * @details     This is a simple Captouch application that configures the CT using the driver
 *              and finishes with a PASS message if any function return an errror.              
 *
 */



/* CT device Handle */
ADI_CT_DEV_HANDLE hDevice = NULL;
uint8_t  CaptouchEvents;
bool_t CTEventFlag=false;

/* prototypes */
extern int32_t adi_initpinmux (void);
ADI_CT_RESULT_TYPE ct_Init(ADI_CT_DEV_HANDLE const hCaptouchDevice);


int main(void)
{


    /* Initialize system */
    SystemInit();

    /* initialize the test console */
    test_Init();


    SetSystemClockDivider(ADI_SYS_CLOCK_CORE, 1);
    
    adi_initpinmux();

    /* Initialize the CT API */
    if(adi_CT_Init(ADI_CT_DEV_ID_0,&hDevice)) {
        FAIL("Init");
    }

    /* Configure the Captouch */
    if(ct_Init(hDevice)) {
        FAIL("Init");
    }
    /* Power Up the Captouch  */
    if(adi_CT_SetPowerStatus(hDevice,true)){
        FAIL("Power UP");
    }

    PASS();


}

ADI_CT_RESULT_TYPE ct_Init(ADI_CT_DEV_HANDLE const hCaptouchDevice)

{

  #define ADI_CT_DEFAULT_SELF_TIMER_VALUE               10
  #define ADI_CT_DEFAULT_IIR_FILTER_WEIGHT              8
  #define ADI_CT_DEFAULT_C2V_HOLDTIME                   16
  #define ADI_CT_DEFAULT_PHASE13                        7
  #define ADI_CT_DEFAULT_PHASE24                        63
  #define ADI_CT_DEFAULT_AVG_MIN_POS_GROUP1             14
  #define ADI_CT_DEFAULT_AVG_MIN_POS_GROUP2             0
  #define ADI_CT_DEFAULT_FAST_PROX_VALUE                50
  #define ADI_CT_DEFAULT_SLOW_PROX_VALUE                10
  #define ADI_CT_DEFAULT_SLOW_FILTER_WEIGHT             15
  #define ADI_CT_DEFAULT_FAST_FILTER_WEIGHT             1
  #define ADI_CT_DEFAULT_TOUCH_UPPER_THR                450
  #define ADI_CT_DEFAULT_TOUCH_LOWER_THR                400
  #define ADI_CT_DEFAULT_TOUCH_MIN_DURATION             10
  #define ADI_CT_DEFAULT_REL_UPPER_THR                  450
  #define ADI_CT_DEFAULT_REL_LOWER_THR                  400
  #define ADI_CT_DEFAULT_REL_MIN_DURATION               10
  #define ADI_CT_DEFAULT_MAIN_DAC_VALUE                 0
  #define ADI_CT_DEFAULT_SUB_DAC_VALUE                  0


  ADI_CT_RESULT_TYPE result = ADI_CT_RESULT_SUCCESS;

  uint16_t  SelfTimerValue  = ADI_CT_DEFAULT_SELF_TIMER_VALUE;
  uint8_t   IIRFilterWeight = ADI_CT_DEFAULT_IIR_FILTER_WEIGHT;
  uint16_t  EnabledStages   = ADI_CT_STAGE1_BIT | ADI_CT_STAGE2_BIT | ADI_CT_STAGE3_BIT | ADI_CT_STAGE4_BIT | ADI_CT_STAGE5_BIT;


  /*-----------------------------------------------------------------------------
 *   CT C2V Timing Data
 *----------------------------------------------------------------------------*/

  static ADI_CT_C2V_TIMING_TYPE C2V_TimingSettings =
{

  ADI_CT_AUTOZERO_DUTY_2us,            /*!< Autozero Duty       */
  ADI_CT_DEFAULT_C2V_HOLDTIME,         /*!< C2VHoldTime     */
  ADI_CT_DEFAULT_PHASE13,              /*!< Phase13         */
  ADI_CT_DEFAULT_PHASE24,               /*!< Phase24         */

};



/*-----------------------------------------------------------------------------
 *   CT Average Data
 *----------------------------------------------------------------------------*/

static ADI_CT_AVERAGE_TYPE AverageSettings =
{

  ADI_CT_RANKING_32_SAMPLES,                  /*!< Number of Samples to be Ranked                       */
  ADI_CT_AVG_OUTPUT_GROUP1,                   /*!< Select the Final Average Value                       */
  ADI_CT_AVG_SAMPLES_8_SAMPLES,               /*!< Number of Samples in Average Group1                  */
  ADI_CT_AVG_SAMPLES_NO_AVG,                  /*!< Number of Samples in Average Group2                  */
  ADI_CT_DEFAULT_AVG_MIN_POS_GROUP1,          /*!< Minimum rank position to be Average in Group1        */
  ADI_CT_DEFAULT_AVG_MIN_POS_GROUP2,          /*!< Minimum rank position to be Average in Group2        */
};


/*-----------------------------------------------------------------------------
 *   CT Baseline Data
 *----------------------------------------------------------------------------*/

static ADI_CT_BASELINE_TYPE BaselineSettings =
{

  ADI_CT_BASELINE_CAL_DELAY_FAST_FILTER_UPDATE_8,       /*!< Delay before updating the Baseline         */
  ADI_CT_FAST_FILTER_UPDATE_2ND_CONVERSION_CYCLE,       /*!< Update for the fast filter                 */
  ADI_CT_DEFAULT_FAST_PROX_VALUE,                       /*!< Fast Proximity Value                       */
  ADI_CT_DEFAULT_SLOW_PROX_VALUE,                       /*!< Slow Proximity Value                       */
  ADI_CT_DEFAULT_SLOW_FILTER_WEIGHT,                    /*!< Slow Filter new sample Weight              */
  ADI_CT_DEFAULT_FAST_FILTER_WEIGHT,                    /*!< Fast Filter new sample Weight              */
};


/*-----------------------------------------------------------------------------
 *   CT Touch and Release Data
 *----------------------------------------------------------------------------*/

static ADI_CT_TOUCH_AND_RELEASE_TYPE TouchSettings =
{

  ADI_CT_INDIVIDUAL_SENSOR_THR_CFGx_DISABLE,                    /*!< To use CT_SENSOR_THR_CFGx as threshold     */
  ADI_CT_TCH_REL_EVENT_ON_HIGH_INPUT_INA_INB_DIFF_GREATER_THR,  /*!< Type of rising edge detection              */
  ADI_CT_TCH_REL_INPUT_AMBIENT_VALUE,                           /*!< Input A                                    */
  ADI_CT_TCH_REL_INPUT_CDC_VALUE,                               /*!< Input B                                    */
  ADI_CT_DEFAULT_TOUCH_UPPER_THR,                               /*!< Upper Threshold                            */
  ADI_CT_DEFAULT_TOUCH_LOWER_THR,                               /*!< Lower Threshold                            */
  ADI_CT_DEFAULT_TOUCH_MIN_DURATION,                            /*!< Minimum duration to detect Touch/Release   */
};

static ADI_CT_TOUCH_AND_RELEASE_TYPE ReleaseSettings =
{

  ADI_CT_INDIVIDUAL_SENSOR_THR_CFGx_DISABLE,                    /*!< To use CT_SENSOR_THR_CFGx as threshold     */
  ADI_CT_TCH_REL_EVENT_ON_HIGH_INPUT_INA_INB_DIFF_LESSER_THR,   /*!< Type of rising edge detection              */
  ADI_CT_TCH_REL_INPUT_AMBIENT_VALUE,                           /*!< Input A                                    */
  ADI_CT_TCH_REL_INPUT_CDC_VALUE,                               /*!< Input B                                    */
  ADI_CT_DEFAULT_REL_UPPER_THR,                                 /*!< Upper Threshold                            */
  ADI_CT_DEFAULT_REL_LOWER_THR,                                 /*!< Lower Threshold                            */
  ADI_CT_DEFAULT_REL_MIN_DURATION,                              /*!< Minimum duration to detect Touch/Release   */
};

/*-----------------------------------------------------------------------------
 *   Stage Type Data
 *----------------------------------------------------------------------------*/
static ADI_STAGE_TYPE StageSettings =
{

  true,                                 /*!<   Bypass PGA                       */
  ADI_CT_PGA_GAIN_2,                    /*!<   PGA Gain                         */
  ADI_CT_C2VRANGE_2_91pF,               /*!<   C2V Range                        */
  false,                                /*!<   Enable PK2PK Measurements        */
  false,                                /*!<   Disable Signal Excitation        */
  true,                                 /*!<   CapDac Enable                    */
  ADI_CT_DEFAULT_MAIN_DAC_VALUE,        /*!<   MAIN DAC Value                   */
  ADI_CT_DEFAULT_SUB_DAC_VALUE,         /*!<   SUB DAC Value                    */

};




  /* Enable the Captouch */
  if(adi_CT_SetEnable(hCaptouchDevice,true))
   {
      return(ADI_CT_RESULT_ERR);
   }


  /*ANALOG TO DIGITAL CONVERSION*/

  /* Set the number of Integration Cycles */
  if(adi_CT_SetIntCycles(hCaptouchDevice, ADI_CT_INT_CYCLES_2))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Set the Unmatched Pads */
  if(adi_CT_SetUnmatchedPads(hCaptouchDevice, ADI_CT_UNMATCHED_PADS_FLOATED))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Set the Timings Settings */
  if(adi_CT_SetTimingsCfg(hCaptouchDevice,&C2V_TimingSettings))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Disable the PGA */
  if(adi_CT_SetByPassGain(hCaptouchDevice,true))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Set the Internal Buffer Bias Current */
  if(adi_CT_SetIntBufferBiasCurrent(hCaptouchDevice,ADI_CT_INT_BUFFER_BIAS_2_5UA))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Set the PGA Bias Current */
  if(adi_CT_SetPGABiasCurrent(hCaptouchDevice,ADI_CT_PGA_BIAS_3UA))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Set the C2V Bias Current */
  if(adi_CT_SetC2VBiasCurrent(hCaptouchDevice,ADI_CT_C2V_BIAS_6UA))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Enable the C2V Low Pass Filter */
  if(adi_CT_SetC2VLowPassFilterEnable(hCaptouchDevice,true))
   {
      return(ADI_CT_RESULT_ERR);
   }


  /* SELF TIMER */

  /* Set the Self Timer Value */
  if(adi_CT_SetSelfTimerValue(hCaptouchDevice,SelfTimerValue))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Enable the Self Timer  */
  if(adi_CT_SetSelfTimerEnable(hCaptouchDevice,true))
   {
      return(ADI_CT_RESULT_ERR);
   }


   /* FILTERING AND AVERAGING SETTINGS */

  /* Set the IIR Filter Weight */
  if(adi_CT_SetIIRFilterWeight(hCaptouchDevice,IIRFilterWeight))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Enable the IIR Filter Weight */
  if(adi_CT_SetIIRFilterEnable(hCaptouchDevice,true))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Set the Average Settings */
 if(adi_CT_SetAverageCfg(hCaptouchDevice,&AverageSettings))
   {
      return(ADI_CT_RESULT_ERR);
   }



   /* BASELINE SETTINGS */

   /* Set the Baseline Settings */
  if(adi_CT_SetBaselineCfg(hCaptouchDevice,&BaselineSettings))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /*STAGE CONFIGURATION */

  /* Set Stage 1 configuration */
  if(adi_CT_SetStageGenericCfg(hCaptouchDevice,ADI_CT_STAGE1_NUMBER,ADI_CT_INPUT_B,&StageSettings))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Set Stage 2 configuration */
  if(adi_CT_SetStageGenericCfg(hCaptouchDevice,ADI_CT_STAGE2_NUMBER,ADI_CT_INPUT_C,&StageSettings))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Set Stage 3 configuration */
  if(adi_CT_SetStageGenericCfg(hCaptouchDevice,ADI_CT_STAGE3_NUMBER,ADI_CT_INPUT_D,&StageSettings))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Set Stage 4 configuration */
  if(adi_CT_SetStageGenericCfg(hCaptouchDevice,ADI_CT_STAGE4_NUMBER,ADI_CT_INPUT_E,&StageSettings))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Set Stage 5 configuration */
  if(adi_CT_SetStageGenericCfg(hCaptouchDevice,ADI_CT_STAGE5_NUMBER,ADI_CT_INPUT_F,&StageSettings))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Set the Stages to be Enabled */
  if(adi_CT_SetStagesEnable(hCaptouchDevice,EnabledStages,true))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Enable the calibration in the set Stages */
  if(adi_CT_SetStagesCalibrationEnable(hCaptouchDevice,EnabledStages,true))
   {
      return(ADI_CT_RESULT_ERR);
   }


  /* TOUCH AND RELEASE DETECTION SETTINGS */

  /* Set Touch Settings */
  if(adi_CT_SetTouchCfg(hCaptouchDevice,&TouchSettings))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Set Release Settings */
  if(adi_CT_SetReleaseCfg(hCaptouchDevice,&ReleaseSettings))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Enable Touch Detection */
  if(adi_CT_SetTouchEnable(hCaptouchDevice,true))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Enable Release Detection */
  if(adi_CT_SetReleaseEnable(hCaptouchDevice,true))
   {
      return(ADI_CT_RESULT_ERR);
   }


  /*INTERRUPTS*/

  /* Enable the Touch and Release Interrupts */
  if(adi_CT_SetInterruptsEnable(hCaptouchDevice,ADI_CT_INTERRUPT_RELEASE_DETECTED | ADI_CT_INTERRUPT_TOUCH_DETECTED,true))
   {
      return(ADI_CT_RESULT_ERR);
   }


  /* Auto CapDAC adjustfor Stage 1 */
  if(adi_CT_Stage_AutoCAPDAC_Adjustment(hCaptouchDevice,ADI_CT_STAGE1_NUMBER,ADI_CT_DEFAULT_BASELINE_VALUE))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Auto CapDAC adjustfor Stage 2 */
  if(adi_CT_Stage_AutoCAPDAC_Adjustment(hCaptouchDevice,ADI_CT_STAGE2_NUMBER,ADI_CT_DEFAULT_BASELINE_VALUE))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Auto CapDAC adjustfor Stage 3 */
  if(adi_CT_Stage_AutoCAPDAC_Adjustment(hCaptouchDevice,ADI_CT_STAGE3_NUMBER,ADI_CT_DEFAULT_BASELINE_VALUE))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Auto CapDAC adjustfor Stage 4 */
  if(adi_CT_Stage_AutoCAPDAC_Adjustment(hCaptouchDevice,ADI_CT_STAGE4_NUMBER,ADI_CT_DEFAULT_BASELINE_VALUE))
   {
      return(ADI_CT_RESULT_ERR);
   }

  /* Auto CapDAC adjustfor Stage 5 */
  if(adi_CT_Stage_AutoCAPDAC_Adjustment(hCaptouchDevice,ADI_CT_STAGE5_NUMBER,ADI_CT_DEFAULT_BASELINE_VALUE))
   {
      return(ADI_CT_RESULT_ERR);
   }

  if(adi_CT_SetAutoForceBaselineCal(hDevice))
   {
      return(ADI_CT_RESULT_ERR);
   }

  return result;

}



/* Revert ADI MISRA Suppressions */
#define REVERT_ADI_MISRA_SUPPRESSIONS
#include "misra.h"

/*
** EOF
*/

/*@}*/
