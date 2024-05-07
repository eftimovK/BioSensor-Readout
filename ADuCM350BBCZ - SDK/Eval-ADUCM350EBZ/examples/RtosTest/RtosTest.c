/*********************************************************************************

Copyright (c) 2013-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/

/*****************************************************************************
 * @file:    RtosTest.c
 * @brief:   Verify that drivers build in RTOS mode
 * @version: $Revision: 29403 $
 * @date:    $Date: 2015-01-08 10:34:56 -0500 (Thu, 08 Jan 2015) $
 *****************************************************************************/

#include <time.h>
#include <stddef.h>  // for 'NULL'
#include <stdio.h>   // for scanf
#include <string.h>  // for strncmp

#include <ucos_ii.h>

#include "dma.h"
#include "system.h"
#include "rtc.h"
#include "wut.h"
#include "gpio.h"
#include "afe.h"
#include "beep.h"
#include "captouch.h"
#include "flash.h"
#include "i2c.h"
#include "i2s.h"
#include "lcd.h"
#include "rng.h"
#include "rtc.h"
#include "spi.h"
#include "uart.h"
#include "wdt.h"
#include "wut.h"
#include "gpio.h"


#include "test_common.h"



ADI_AFE_RESULT_TYPE  rafe;
ADI_AFE_DEV_HANDLE   hafe;
ADI_BEEP_RESULT_TYPE rbeep;
ADI_BEEP_HANDLE      hbeep;
ADI_CT_RESULT_TYPE   rct;
ADI_CT_DEV_HANDLE    hct;
ADI_FEE_RESULT_TYPE  rfee;
ADI_FEE_DEV_HANDLE   hfee;
ADI_I2S_RESULT_TYPE  ri2s;
ADI_I2S_DEV_HANDLE   hi2s;
ADI_LCD_RESULT_TYPE  rlcd;
ADI_LCD_DEV_HANDLE   hlcd;
ADI_I2C_RESULT_TYPE  ri2c;
ADI_I2C_DEV_HANDLE   hi2c;
ADI_RNG_RESULT_TYPE  rrng;
ADI_RNG_HANDLE       hrng;
ADI_RTC_RESULT_TYPE  rrtc;
ADI_RTC_HANDLE       hrtc;
ADI_SPI_RESULT_TYPE  rspi;
ADI_SPI_DEV_HANDLE   hspi;
ADI_UART_RESULT_TYPE ruart;
ADI_UART_HANDLE      huart;
ADI_WDT_RESULT_TYPE  rwdt;
ADI_WDT_DEV_HANDLE   hwdt;
ADI_WUT_RESULT_TYPE  rwut;
ADI_WUT_DEV_HANDLE   hwut;
                                   

uint32_t buffer[1];
uint32_t length;


/*
 * RTOS definitions
 */
#define THREAD1_STK_SIZE 200
#define THREAD1_PRIO 5
#define NUM_MSG 10
char    g_Thread1Stack[THREAD1_STK_SIZE];
OS_TCB  g_Thread1_TCB;


void Thread1Run(void* arg)
{
  
  int i;
  
#if 0
    /* Change the system clock source to HFXTAL and change clock frequency to 16MHz     */
    /* Requirement for AFE (ACLK)                                                       */
    SystemTransitionClocks(ADI_SYS_CLOCK_TRIGGER_MEASUREMENT_ON);
    
    rafe = adi_AFE_Init(&hafe);
    if( rafe != ADI_AFE_SUCCESS)
    {
      FAIL("adi_AFE_Init failed");
    }
#endif
    
    for(i=0; i<2; i++)
    {
    
      /*
       * Beep
       */

      rbeep = adi_Beep_Init(ADI_BEEP_DEVID_0, &hbeep);
      if( rbeep != ADI_BEEP_SUCCESS)
      {
        FAIL("adi_Beep_Init failed");
      }
      rbeep = adi_Beep_UnInit(hbeep);
       if( rbeep != ADI_BEEP_SUCCESS)
      {
        FAIL("adi_Beep_UnInit failed");
      }
     
      
       /*
        * Cap Touch 
        */
      
       rct = adi_CT_Init( ADI_CT_DEV_ID_0, &hct);
       if( rct != ADI_CT_RESULT_SUCCESS)   
       {
        FAIL("adi_CT_Init failed");
       }
       rct = adi_CT_UnInit( hct );
       if( rct != ADI_CT_RESULT_SUCCESS)
       {
        FAIL("adi_CT_UnInit failed");
       }
       
     
       /*
        * Flash
        */
       
        rfee = adi_FEE_Init( ADI_FEE_DEVID_0, 0, &hfee);
        if( rfee != ADI_FEE_SUCCESS)
        {
          FAIL("adi_FEE_Init failed");
        }
         rfee = adi_FEE_UnInit( hfee);
         if( rfee != ADI_FEE_SUCCESS)
         {
          FAIL("adi_FEE_UnInit failed");
         }
    

         /*
         *    I2S
         */
         
         ri2s = adi_I2S_Init(ADI_I2S_DEVID_0, &hi2s);
         if( ri2s != ADI_I2S_SUCCESS)
         {
          FAIL("adi_I2S_Init failed");
         }
         ri2s = adi_I2S_UnInit( hi2s);
         if( ri2s != ADI_I2S_SUCCESS)
         {
          FAIL("adi_I2S_UnInit failed");
         }
    
         /*
         * LCD
         */
         
        rlcd = adi_LCD_Init(ADI_LCD_DEVID_0, &hlcd);
         if( rlcd != ADI_LCD_SUCCESS)
         {
          FAIL("adi_LCD_Init failed");
         }
         rlcd = adi_LCD_UnInit( hlcd);
         if( rlcd != ADI_LCD_SUCCESS)
         {
          FAIL("adi_LCD_UnInit failed");
         }
         
         
         /*
         * I2C
         */
         ri2c = adi_I2C_MasterInit(ADI_I2C_DEVID_0, &hi2c);
         if( ri2c != ADI_I2C_SUCCESS)
         {
          FAIL("adi_I2C_MasterInit failed");
         }
         ri2c = adi_I2C_UnInit( hi2c);
         if( ri2c != ADI_I2C_SUCCESS)
         {
          FAIL("adi_I2C_UnInit failed");
         }
          ri2c = adi_I2C_SlaveInit(ADI_I2C_DEVID_0, &hi2c);
         if( ri2c != ADI_I2C_SUCCESS)
         {
          FAIL("adi_I2C_SlaveInit failed");
         }
         ri2c = adi_I2C_UnInit( hi2c);
         if( ri2c != ADI_I2C_SUCCESS)
         {
          FAIL("adi_I2C_UnInit failed");
         }
         
         
         /*
         * RNG
         */
         rrng = adi_RNG_Init(ADI_RNG_DEVID_0, &hrng);
         if( rrng != ADI_RNG_SUCCESS)
         {
          FAIL("adi_RNG_Init failed");
         }
         rrng = adi_RNG_UnInit( hrng);
         if( rrng != ADI_RNG_SUCCESS)
         {
          FAIL("adi_RNG_UnInit failed");
         }
         
         /*
         * RTC
         */
         rrtc = adi_RTC_Init(ADI_RTC_DEVID_0, &hrtc);
         if( rrtc != ADI_RTC_SUCCESS)
         {
          FAIL("adi_RTC_Init failed");
         }
         rrtc = adi_RTC_UnInit( hrtc);
         if( rrtc != ADI_RTC_SUCCESS)
         {
          FAIL("adi_RTC_UnInit failed");
         }
         
         
         /*
         * SPI
         */
         rspi = adi_SPI_MasterInit (ADI_SPI_DEVID_0 , &hspi);
         if( rspi != ADI_SPI_SUCCESS)
         {
          FAIL("adi_SPI_MasterInit failed");
         }
         rspi = adi_SPI_UnInit( hspi);
         if( rspi != ADI_SPI_SUCCESS)
         {
          FAIL("adi_SPI_UnInit failed");
         }
         rspi = adi_SPI_SlaveInit (ADI_SPI_DEVID_0 , &hspi);
         if( rspi != ADI_SPI_SUCCESS)
         {
          FAIL("adi_SPI_SlaveInit failed");
         }
         rspi = adi_SPI_UnInit( hspi);
         if( rspi != ADI_SPI_SUCCESS)
         {
          FAIL("adi_SPI_UnInit failed");
         }
        
#if 0
         /*
         * UART  - already being used to print PASS / FAIL
         */
         
         ruart = adi_UART_Init(ADI_UART_DEVID_0,&huart, 0);
         if( ruart != ADI_UART_SUCCESS)
         {
          FAIL("adi_UART_Init failed");
         }
         ruart = adi_UART_UnInit( huart);
         if( ruart != ADI_UART_SUCCESS)
         {
          FAIL("adi_UART_UnInit failed");
         }
#endif
         
         /*
         * WDT
         */
         rwdt = adi_WDT_Init(ADI_WDT_DEVID_0, &hwdt);
         if( rwdt != ADI_WDT_SUCCESS)
         {
          FAIL("adi_WDT_Init failed");
         }
         rwdt = adi_WDT_UnInit( hwdt);
         if( rwdt != ADI_WDT_SUCCESS)
         {
          FAIL("adi_WDT_UnInit failed");
         }
         
         /*
         * WUT
         */
         rwut = adi_WUT_Init (ADI_WUT_DEVID_0, &hwut);
         if( rwut != ADI_WUT_SUCCESS)
         {
          FAIL("adi_WUT_Init failed");
         }
         rwut = adi_WUT_UnInit( hwut);
         if( rwut != ADI_WUT_SUCCESS)
         {
          FAIL("adi_WUT_UnInit failed");
         }
        
    }
      
    PASS();
}



int main (void)
{
    INT8U    OSRetVal;
  
  /* Initialize system */
    SystemInit();

    /* test system initialization */
    test_Init();
    
    
    OSInit();

    OSRetVal = OSTaskCreate (Thread1Run,
                NULL,
               (void *)(g_Thread1Stack + THREAD1_STK_SIZE),
               THREAD1_PRIO);
    
    if (OSRetVal != OS_ERR_NONE)
    {
    	FAIL("Error creating thread1\n");
    	return 1;
    }

    OSStart();
    
	/* should never get here... */
    FAIL("Error starting the RTOS\n");
    
    return 0;

}


