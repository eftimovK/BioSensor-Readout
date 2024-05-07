/*
*********************************************************************************************************
*                                       EXAMPLE CODE
*
*               This file is provided as an example on how to use Micrium products.
*
*               Please feel free to use any application code labeled as 'EXAMPLE CODE' in
*               your application products.  Example code may be used as is, in whole or in
*               part, or may be used as a reference only. This file can be modified as
*               required to meet the end-product requirements.
*
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*
*               You can contact us at www.micrium.com.
*
*               Portions Copyright (c) 2014 Analog Devices, Inc.
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*
*                                       APPLICATION CONFIGURATION
*
*
* Filename      : app_cfg.h
* Version       : V1.00
* Programmer(s) : FT
*********************************************************************************************************
*/

#ifndef  APP_CFG_MODULE_PRESENT
#define  APP_CFG_MODULE_PRESENT

#include <stdio.h>

/*
*********************************************************************************************************
*                                       ADDITIONAL uC/MODULE ENABLES
*********************************************************************************************************
*/

#define  APP_CFG_SERIAL_EN                      DEF_ENABLED
#define  APP_CFG_USBD_EN                        DEF_ENABLED


/*
*********************************************************************************************************
*                                            TASK PRIORITIES
*********************************************************************************************************
*/
//#define __PHDC_SINGLE__
#ifdef __PHDC_SINGLE__
#define  APP_CFG_TASK_START_PRIO                           2u
#define  USBD_OS_CFG_CORE_TASK_PRIO                        3u
#define  USBD_OS_CFG_TRACE_TASK_PRIO                       4u

#define  APP_CFG_USBD_HID_MOUSE_TASK_PRIO                  7u
#define  USBD_HID_OS_CFG_TMR_TASK_PRIO                     9u

#define  OS_TASK_TMR_PRIO                        (OS_LOWEST_PRIO - 2)

#define  APP_CFG_USBD_PHDC_RX_COMM_TASK_PRIO               5u
#define  APP_CFG_USBD_PHDC_TX_COMM_TASK_PRIO               6u

/*
 * These are not required but need to be defined for the app to compile
*/
#define  APP_CFG_USBD_HID_READ_TASK_PRIO                   5u
#define  APP_CFG_USBD_HID_WRITE_TASK_PRIO                  5u

#else

#define  APP_CFG_TASK_START_PRIO                           5u
#define  USBD_OS_CFG_CORE_TASK_PRIO                        4u
#define  USBD_OS_CFG_TRACE_TASK_PRIO                       14u

#define  APP_CFG_USBD_VENDOR_ECHO_SYNC_TASK_PRIO           5u
#define  APP_CFG_USBD_VENDOR_ECHO_ASYNC_TASK_PRIO          6u

#define  APP_CFG_USBD_HID_MOUSE_TASK_PRIO                  7u
#define  APP_CFG_USBD_HID_READ_TASK_PRIO                   7u
#define  APP_CFG_USBD_HID_WRITE_TASK_PRIO                  8u
#define  USBD_HID_OS_CFG_TMR_TASK_PRIO                     9u

#define  APP_CFG_USBD_CDC_SERIAL_TASK_PRIO                 10u

#define  OS_TASK_TMR_PRIO                        (OS_LOWEST_PRIO - 2)

#define  APP_CFG_USBD_PHDC_RX_COMM_TASK_PRIO               11u
#define  APP_CFG_USBD_PHDC_TX_COMM_TASK_PRIO               12u
#define  USBD_PHDC_OS_CFG_SCHED_TASK_PRIO                  13u

#define APP_CFG_DISKPREP_TASK_PRIO                         3u
#endif


/*
*********************************************************************************************************
*                                            TASK STACK SIZES
*********************************************************************************************************
*/
#if 1

#define  APP_CFG_TASK_START_STK_SIZE                     256u

#define  USBD_OS_CFG_CORE_TASK_STK_SIZE                  256u
#define  USBD_OS_CFG_TRACE_TASK_STK_SIZE                 256u

#define  APP_CFG_USBD_VENDOR_TASK_STK_SIZE               256u
#define  APP_CFG_USBD_HID_TASK_STK_SIZE                  128u
#define  USBD_HID_OS_CFG_TMR_TASK_STK_SIZE               256u

#define  APP_CFG_USBD_CDC_SERIAL_TASK_STK_SIZE           128u

#define  USBD_PHDC_OS_CFG_SCHED_TASK_STK_SIZE            256u
#define  APP_CFG_USBD_PHDC_TASK_STK_SIZE                 512u

#else
#define  APP_CFG_TASK_START_STK_SIZE                    180u

#define  USBD_OS_CFG_CORE_TASK_STK_SIZE                 180u
#define  USBD_OS_CFG_TRACE_TASK_STK_SIZE                256u

#define  APP_CFG_USBD_VENDOR_TASK_STK_SIZE              256u
#define  APP_CFG_USBD_HID_TASK_STK_SIZE                 128u
#define  USBD_HID_OS_CFG_TMR_TASK_STK_SIZE              256u

#define  APP_CFG_USBD_CDC_SERIAL_TASK_STK_SIZE          256u

#define  USBD_PHDC_OS_CFG_SCHED_TASK_STK_SIZE           180u
#define  APP_CFG_USBD_PHDC_TASK_STK_SIZE                180u
#endif
/*
*********************************************************************************************************
*                                              BSP CONFIGURATION
*********************************************************************************************************
*/

#define  BSP_CFG_EXT_MEM_BUS_16                 DEF_ENABLED


/*
*********************************************************************************************************
*                                   uC/USB-DEVICE APPLICATION CONFIGURATION
*********************************************************************************************************
*/

#define  APP_CFG_USBD_TEST_EN                   DEF_DISABLED
#define  APP_CFG_USBD_VENDOR_EN                 DEF_DISABLED
#define  APP_CFG_USBD_CDC_EN                    DEF_DISABLED
#define  APP_CFG_USBD_HID_EN                    DEF_DISABLED
#define  APP_CFG_USBD_PHDC_EN                   DEF_DISABLED
#define  APP_CFG_USBD_MSC_EN                    DEF_ENABLED


#define  APP_CFG_USBD_VENDOR_ECHO_SYNC_EN       DEF_ENABLED
#define  APP_CFG_USBD_VENDOR_ECHO_ASYNC_EN      DEF_DISABLED


#define  APP_CFG_USBD_HID_TEST_MOUSE_EN         DEF_DISABLED


#define  APP_CFG_USBD_CDC_SERIAL_TEST_EN        DEF_DISABLED

#define  APP_CFG_USBD_VENDOR_MAX_BUF_SIZE       4*1024u

#define APP_CFG_USBD_PHDC_ITEM_DATA_LEN_MAX      128u
#define APP_CFG_USBD_PHDC_ITEM_NBR_MAX           5u
/*
*********************************************************************************************************
*                                             uC/LIB CONFIGURATION
*********************************************************************************************************
*/

#define  LIB_MEM_CFG_OPTIMIZE_ASM_EN            DEF_ENABLED
#define  LIB_MEM_CFG_ARG_CHK_EXT_EN             DEF_ENABLED
#define  LIB_MEM_CFG_ALLOC_EN                   DEF_ENABLED
#define  LIB_MEM_CFG_HEAP_SIZE                         644u
#define  LIB_MEM_CFG_HEAP_BASE_ADDR              0x20040000


/*
*********************************************************************************************************
*                                                 uC/SERIAL
*
* Note(s) : (1) Configure SERIAL_CFG_MAX_NBR_IF to the number of interfaces (i.e., UARTs) that will be
*               present.
*
*           (2) Configure SERIAL_CFG_RD_BUF_EN to enable/disable read buffer functionality.  The serial
*               core stores received data in the read buffer until the user requests it, providing a
*               reliable guarantee against receive overrun.
*
*           (3) Configure SERIAL_CFG_WR_BUF_EN to enable/disable write buffer functionality.  The serial
*               core stores line driver transmit data in the write buffer while the serial interface is
*               transmitting application data.
*
*           (4) Configure SERIAL_CFG_ARG_CHK_EXT_EN to enable/disable extended argument checking
*               functionality.
*
*           (5) Configure SERIAL_CFG_TX_DESC_NBR to allow multiple transmit operations (i.e., Serial_Wr,
*               Serial_WrAsync) to be queued.
*********************************************************************************************************
*/

#define  SERIAL_CFG_MAX_NBR_IF                            2u    /* See Note #1.                                         */
#define  SERIAL_CFG_RD_BUF_EN                   DEF_DISABLED    /* See Note #2.                                         */
#define  SERIAL_CFG_WR_BUF_EN                   DEF_DISABLED    /* See Note #3.                                         */
#define  SERIAL_CFG_ARG_CHK_EXT_EN              DEF_DISABLED    /* See Note #4.                                         */
#define  SERIAL_CFG_TX_DESC_NBR                           1u    /* See Note #5.                                         */


/*
*********************************************************************************************************
*                                    uC/SERIAL APPLICATION CONFIGURATION
*********************************************************************************************************
*/

#define  APP_SERIAL_CFG_TRACE_EN                DEF_ENABLED
#define  APP_SERIAL_CFG_TRACE_PORT_NAME         "UART0"


/*
*********************************************************************************************************
*                                       uC/PROBE APPLICATION CONFIGURATION
*********************************************************************************************************
*/

#define  APP_CFG_PROBE_SERIAL_PORT_NAME         "UART0"

/*
*********************************************************************************************************
*                                     TRACE / DEBUG CONFIGURATION
*********************************************************************************************************
*/

#define  TRACE_LEVEL_OFF                                  0u
#define  TRACE_LEVEL_INFO                                 1u
#define  TRACE_LEVEL_DBG                                  2u

#include <cpu.h>
void  App_SerPrintf  (CPU_CHAR *format, ...);
#define  APP_CFG_TRACE_LEVEL                    TRACE_LEVEL_OFF

#define  APP_CFG_TRACE                         printf

#define  APP_TRACE_INFO(x)                     ((APP_CFG_TRACE_LEVEL >= TRACE_LEVEL_INFO)  ? (void)(APP_CFG_TRACE x) : (void)0)
#define  APP_TRACE_DBG(x)                      ((APP_CFG_TRACE_LEVEL >= TRACE_LEVEL_DBG)   ? (void)(APP_CFG_TRACE x) : (void)0)

#endif
