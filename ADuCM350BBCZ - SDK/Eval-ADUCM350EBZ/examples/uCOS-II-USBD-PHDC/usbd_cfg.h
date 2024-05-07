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
*                                    USB DEVICE CONFIGURATION FILE
*
*                                              TEMPLATE
*
* File          : usbd_cfg.h
* Version       : V4.04.01
* Programmer(s) : FT
*                 FGK
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                  USB DEVICE GENERIC CONFIGURATION
*
* Note(s) : (1) Configure USBD_CFG_OPTIMIZE_SPD with the desired USB stack performance optimization.
*
*               DEF_ENABLED      Optimizes USB stack for speed performance.
*               DEF_DISABLED     Optimizes USB stack for memory footprint.
*
*           (2) Configure USBD_CFG_DEV_NBR_MAX with the total number of USB device controllers
*               used by your application.
*********************************************************************************************************
*/

#define  USBD_CFG_OPTIMIZE_SPD                  DEF_ENABLED     /* Speed optimization (see Note #1).                    */

#define  USBD_CFG_MAX_NBR_DEV                             1u    /* Configure maximum number of devices (see Note #2).   */

#define USBD_CFG_BUF_ALIGN_OCTETS                         4u    /* Set memory alignment for DMA address register */
#define USBD_CFG_ERR_ARG_CHK_EXT_EN             DEF_DISABLED    /* Argument checking */

/*
*********************************************************************************************************
*                                      USB DEVICE CONFIGURATIONS
*********************************************************************************************************
*/

#define  USBD_CFG_MAX_NBR_CFG                             1u    /* Configure maximum number of configurations ...       */
                                                                /*  ... supported.                                      */

/*
*********************************************************************************************************
*                                 USB DEVICE INTERFACES CONFIGURATION
*********************************************************************************************************
*/

#define  USBD_CFG_MAX_NBR_IF                             1u    /* Configure maximum number of interfaces ...           */
                                                                /* ... per configuration.                               */

#define  USBD_CFG_MAX_NBR_IF_ALT                         1u    /* Configure maximum number of alternative ...          */
                                                                /* ... interfaces per interface.                        */
#define  USBD_CFG_MAX_NBR_IF_GRP                         1u

#define  USBD_CFG_MAX_NBR_EP_DESC                        5u    /* Configure maximum number of endpoints ...            */
                                                                /* ... (including controls).                            */

#define  USBD_CFG_MAX_NBR_EP_OPEN                        5u    /* Maximum number of opened endpoints.                  */

#define  USBD_CFG_MAX_NBR_URB_EXTRA                      0u
                                                               /* Must be between 0u and 255u.                         */

/*
*********************************************************************************************************
*                                   USB DEVICE STRING CONFIGURATION
*********************************************************************************************************
*/

#define  USBD_CFG_MAX_NBR_STR                            10u    /* Maximum number of string descriptors supported.      */


/*
*********************************************************************************************************
*                                   USB DEVICE DEBUG CONFIGURATION
*********************************************************************************************************
*/

#define  USBD_CFG_DBG_TRACE_EN                  DEF_DISABLED
#define  USBD_CFG_DBG_TRACE_NBR_EVENTS                   20u


/*
*********************************************************************************************************
*                                      VENDOR CLASS CONFIGURATION
*********************************************************************************************************
*/

#define  USBD_VENDOR_CFG_MAX_NBR_DEV                      2u    /* Max nbr of class instances.                          */
#define  USBD_VENDOR_CFG_MAX_NBR_CFG                      2u    /* Max nbr of cfg per dev.                              */


/*
*********************************************************************************************************
*                                       HID CLASS CONFIGURATION
*********************************************************************************************************
*/

#define  USBD_HID_CFG_MAX_NBR_DEV                         2u    /* Max nbr of class instances.                          */
#define  USBD_HID_CFG_MAX_NBR_CFG                         2u    /* Max nbr of cfg per dev.                              */


/*
*********************************************************************************************************
*                                       HID CLASS CONFIGURATION
*
* Note(s) : (1) The HID class specification indicates that the Interrupt OUT endpoint is optional (target
*               read API). See 'Device Class Definition for Human Interface Devices (HID), 6/27/01,
*               Version 1.11', section '4.4 Interfaces' for more details about the use of the Interrupt
*               OUT endpoint.
*********************************************************************************************************
*/

#define  USBD_HID_CFG_RD_EN                     DEF_ENABLED     /* Configure optional read API (see Note #1).           */
#define  USBD_HID_CFG_MAX_NBR_REPORT_ID                   16u
#define  USBD_HID_CFG_MAX_NBR_REPORT_PUSHPOP               0u

/*
*********************************************************************************************************
*                          USB DEVICE MASS STORAGE CLASS (MSC) CONFIGURATION
*********************************************************************************************************
*/

#define USBD_MSC_OS_CFG_TASK_PRIO                6u
#define USBD_MSC_OS_CFG_TASK_STK_SIZE            256u
#define USBD_MSC_CFG_MAX_LUN                     1u
#define USBD_MSC_CFG_DATA_LEN                    512u
//#define USBD_MSC_CFG_DATA_LEN                    4096u
#define USBD_MSC_CFG_MAX_NBR_DEV                 1u
#define USBD_MSC_CFG_MAX_NBR_CFG                 2u
#define USBD_RAMDISK_CFG_BLK_SIZE                512u
#define USBD_RAMDISK_CFG_NBR_BLKS                48u
//#define USBD_RAMDISK_CFG_NBR_BLKS                4096u
#define USBD_RAMDISK_CFG_NBR_UNITS               1u

/*
*********************************************************************************************************
*                                       CDC CLASS CONFIGURATION
*********************************************************************************************************
*/

#define  USBD_CDC_CFG_MAX_NBR_DEV                        2u     /* Max nbr of class instances.                          */
#define  USBD_CDC_CFG_MAX_NBR_CFG                        2u     /* Max nbr of cfg per dev.                              */
#define  USBD_CDC_CFG_MAX_NBR_DATA_IF                    2u     /* Max nbr of CDC data IFs.                             */


/*
*********************************************************************************************************
*                                 CDC ACM SERIAL CLASS CONFIGURATION
*********************************************************************************************************
*/

#define  USBD_ACM_SERIAL_CFG_MAX_NBR_DEV                 1u     /* Max nbr of class instances.                          */


/*
*********************************************************************************************************
*                          USB DEVICE MICROSOFT OS DESCRIPTOR CONFIGURATION
*
* Note(s) : (1) Configure USBD_CFG_MS_OS_DESC_EN to enable or disable Microsoft OS descriptor.
*
*               (a) When DEF_ENABLED,  Microsoft descriptors and MS OS string descriptor are enabled.
*               (b) When DEF_DISABLED, Microsoft descriptors and MS OS string descriptor are disabled.
*********************************************************************************************************
*/

#define  USBD_CFG_MS_OS_DESC_EN                 DEF_DISABLED    /* Configure Microsoft descriptor feature (see Note #1).*/


/*
*********************************************************************************************************
*                      USB DEVICE PERSONAL HEALTHCARE DEVICE (PHDC) CONFIGURATION
*********************************************************************************************************
*/

#define  USBD_PHDC_CFG_MAX_NBR_DEV                         1u
#define  USBD_PHDC_CFG_MAX_NBR_CFG                         1u

#define  USBD_PHDC_CFG_DATA_OPAQUE_MAX_LEN                43u

#if (APP_CFG_USBD_PHDC_BASIC == DEF_ENABLED)
#define  USBD_PHDC_OS_CFG_SCHED_EN                       DEF_DISABLED
#else
#define  USBD_PHDC_OS_CFG_SCHED_EN                       DEF_DISABLED
#endif
#if 0
/*
*********************************************************************************************************
*                          USB DEVICE MASS STORAGE CLASS (MSC) CONFIGURATION
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                          USB DEVICE MASS STORAGE CLASS (MSC) CONFIGURATION
*********************************************************************************************************
*/

#define USBD_MSC_OS_CFG_TASK_PRIO                6u
#define USBD_MSC_OS_CFG_TASK_STK_SIZE            1000u
#define USBD_MSC_CFG_MAX_LUN                     1u
#define USBD_MSC_CFG_DATA_LEN                    4096u
#define USBD_MSC_CFG_MAX_NBR_DEV                 1u
#define USBD_MSC_CFG_MAX_NBR_CFG                 2u
#define USBD_RAMDISK_CFG_BLK_SIZE                512u
//#define USBD_RAMDISK_CFG_BASE_ADDR               0x00000000u
#define USBD_RAMDISK_CFG_NBR_BLKS                1u
//#define USBD_RAMDISK_CFG_NBR_BLKS                4096u
#define USBD_RAMDISK_CFG_NBR_UNITS               1u

/*
*********************************************************************************************************
*                                USB DEVICE AUDIO CLASS CONFIGURATION
*********************************************************************************************************
*/

#define  USBD_AUDIO_ASYNC                       DEF_DISABLED
#define  USBD_AUDIO_SYNC                        DEF_ENABLED

#define  USBD_AUDIO_CFG_MAX_BUF_SIZE              (13 * 1024)
#endif

