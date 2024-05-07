/*********************************************************************************

Copyright (c) 2012-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*!
 *****************************************************************************
   @file:    adi_wut_config.h
   @brief:   Configuration options for WUT driver.
             This is specific to the WUT driver and will be included by the driver.
             It is not required for the application to include this header file.
   @version: $Revision: 28525 $
   @date:    $Date: 2014-11-12 14:51:26 -0500 (Wed, 12 Nov 2014) $
*****************************************************************************/

#ifndef __ADI_WUT_CONIG_H__
#define __ADI_WUT_CONIG_H__


/********************************************************************************\
|* NOTE: This is a modified override of the factory default adi_wut_config.h    *|
|* file, normally resolved from the .../inc/config directory.  The modificatios *|
|* are to the various initialization constants used to enable static            *|
|* of the WUT controller registers for the purposes of the "wut_test" example.  *|
\********************************************************************************/


/************* WUT Driver configurations ***************/

/*!
   Enable static configuration support.
   1 -  To have the static configuration support code in the driver.
   0 -  To eliminate static configuration code.
*/
#define ADI_WUT_CFG_ENABLE_STATIC_CONFIG_SUPPORT                     1


#if (ADI_WUT_CFG_ENABLE_STATIC_CONFIG_SUPPORT == 1)

/************* WUT controller static configurations ***************/

/* NOTE: the setting configured here are for the "wut_test.c" example
   and need to be modified to suite the application needs.
*/

/*! WUT Control Register STOP_WUFA bit.
    Controls enable for FieldA register update by FieldA interval value.
    0 - WUT FieldA updates are enabled (default).
    1 - WUT FieldA updates are disabled.
*/
#define WUT_CONTROL_DISABLE_FIELDA_UPDATES                           0

/*! WUT Control Register CLK value.
    Clock select.
    0 - WUT clock source is PCLK (default).
    1 - WUT clock source is external 32kHz oscillator.
    2 - WUT clock source is internal 32kHz oscilator.
    3 - WUT clock source is external GPIO pin.
*/
#define WUT_CONTROL_CLOCK_SELECT                                     0

/*! WUT WAKEUP enable bit.
    Controls enable for wakeup timer when core clock is off.
    0 - WUT core clock off wakeup is disabled (default).
    1 - WUT core clock off wakeup is enabled.
*/
#define WUT_CONTROL_WAKEUP_ENABLE                                    0

/*! WUT TIMER enable bit.
    Controls enable for timer.
    0 - WUT is disabled (default).
    1 - WUT is enabled.
*/
#define WUT_CONTROL_TIMER_ENABLE                                     0

/*! WUT timer MODE bit.
    Controls operational mode for timer.
    0 - WUT runs in periodic mode.
    1 - WUT runs in free-running mode (default).
*/
#define WUT_CONTROL_TIMER_MODE                                       1


/*! WUT FREEZE bit.
    Controls assertion of a temporaty lock on the two timer
    value read registers, insuring the 32-bit result does
    not reflect possible carries or borrows between the two
    successive 16-bit reads required to get the 32-bit value.
    0 - WUT frozen read is disabled (default).
    1 - WUT frozen read is enabled.
*/
#define WUT_CONTROL_FREEZE_READ                                      0

/*! WUT PRE bit.
    Controls clock source prescaler (clock divide).
    0 - WUT clock source is divided by 1 (default).
    1 - WUT clock source is divided by 16.
    2 - WUT clock source is divided by 256.
    3 - WUT clock source is divided by 32,768.
*/
#define WUT_CONTROL_TIMER_PRESCALE                                   0

/*! WUT FieldA INCREMENT value.
    FieldA increment value.
    Default value is 0x00C8.
    0x0000-0x0FFF - valid increment values (12-bit range).
    0x1000-0xFFFF - invalid increment values.
*/
#define WUT_FIELDA_INCREMENT_VALUE                              0x00C8

/*! WUT Field COMPARE values.
    Field comparator values for comparators A/B/C/D.
    Default values are:
    	FieldA = 0x00001900.
    	FieldB = 0x00001FFF.
    	FieldC = 0x00002FFF.
    	FieldD = 0x00003FFF.
    0x00000000-0xFFFFFFFF - valid update values (32-bit range).
*/
#define WUT_COMPARE_FIELDA_VALUE                            0x00001900
#define WUT_COMPARE_FIELDB_VALUE                            0x00000080
#define WUT_COMPARE_FIELDC_VALUE                            0x00000800
#define WUT_COMPARE_FIELDD_VALUE                            0x00008000

/*! WUT ROLL interrupt enable.
    Controls the counter rollover interrupt.
    0 - ROLL interrupt disabled (default).
    1 - ROLL interrupt enabled.
*/
#define WUT_INTERRUPT_ROLL_ENABLE                                    0

/*! WUT comparator interrupt enables.
    Controls Fieldx (x=A/B/C/D) comparator interrupts.
    0 - comparator interrupt disabled (default).
    1 - comparator interrupt enabled.
*/
#define WUT_INTERRUPT_FIELDA_ENABLE                                  0
#define WUT_INTERRUPT_FIELDB_ENABLE                                  1
#define WUT_INTERRUPT_FIELDC_ENABLE                                  1
#define WUT_INTERRUPT_FIELDD_ENABLE                                  1

/*! WUT interrupt flag clear bits.
    Controls clearing of WUT interrupt flags.
    0 - interrupt flag not cleared (default).
    1 - interrupt flag cleared.
*/
#define WUT_INTERRUPT_ROLL_CLEAR                                     1
#define WUT_INTERRUPT_FIELDA_CLEAR                                   1
#define WUT_INTERRUPT_FIELDB_CLEAR                                   1
#define WUT_INTERRUPT_FIELDC_CLEAR                                   1
#define WUT_INTERRUPT_FIELDD_CLEAR                                   1


/************** Macro validation *****************************/

#if ( WUT_CONTROL_DISABLE_FIELDA_UPDATES  > 1 )
#error "Invalid configuration"
#endif

#if ( WUT_CONTROL_CLOCK_SELECT > 1 )
#error "Invalid configuration"
#endif

#if ( WUT_CONTROL_WAKEUP_ENABLE > 1 )
#error "Invalid configuration"
#endif

#if ( WUT_CONTROL_TIMER_ENABLE > 3 )
#error "Invalid configuration"
#endif

#if ( WUT_CONTROL_TIMER_MODE > 1 )
#error "Invalid configuration"
#endif

#if ( WUT_CONTROL_FREEZE_READ > 1 )
#error "Invalid configuration"
#endif

#if ( WUT_CONTROL_TIMER_PRESCALE > 3 )
#error "Invalid configuration"
#endif

#if ( WUT_FIELDA_INCREMENT_VALUE > 0X0FFF )
#error "Invalid configuration"
#endif

#if ( WUT_COMPARE_FIELDA_VALUE > 0xFFFFFFFF )
#error "Invalid configuration"
#endif

#if ( WUT_COMPARE_FIELDB_VALUE > 0xFFFFFFFF )
#error "Invalid configuration"
#endif

#if ( WUT_COMPARE_FIELDC_VALUE > 0xFFFFFFFF )
#error "Invalid configuration"
#endif

#if ( WUT_COMPARE_FIELDD_VALUE > 0xFFFFFFFF )
#error "Invalid configuration"
#endif

#if ( WUT_INTERRUPT_ROLL > 1 )
#error "Invalid configuration"
#endif

#if ( WUT_INTERRUPT_FIELDA_ENABLE > 1 )
#error "Invalid configuration"
#endif

#if ( WUT_INTERRUPT_FIELDB_ENABLE > 1 )
#error "Invalid configuration"
#endif

#if ( WUT_INTERRUPT_FIELDC_ENABLE > 1 )
#error "Invalid configuration"
#endif

#if ( WUT_INTERRUPT_FIELDD_ENABLE > 1 )
#error "Invalid configuration"
#endif

#if ( WUT_INTERRUPT_ROLL_CLEAR > 1 )
#error "Invalid configuration"
#endif

#if ( WUT_INTERRUPT_FIELDA_CLEAR > 1 )
#error "Invalid configuration"
#endif

#if ( WUT_INTERRUPT_FIELDB_CLEAR > 1 )
#error "Invalid configuration"
#endif

#if ( WUT_INTERRUPT_FIELDC_CLEAR > 1 )
#error "Invalid configuration"
#endif

#if ( WUT_INTERRUPT_FIELDD_CLEAR > 1 )
#error "Invalid configuration"
#endif

#endif  /* #if (ADI_WUT_CFG_ENABLE_STATIC_CONFIG_SUPPORT == 1) */

#endif /* __ADI_WUT_CONIG_H__ */
