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
*
* File           : usbd_dev_cfg.c
* Version        : V4.04.01
* Programmer(s)  : FT
*                  FGK
*********************************************************************************************************
*/

#include  <usbd_core.h>
#include  <usbd_dev_cfg.h>
#include  <usbd_bsp_dev.h>

#define _PHDC_MULT_
//#define _MSC_
//#define _VENDOR_
/*
*********************************************************************************************************
*                                      USB DEVICE CONFIGURATION
*********************************************************************************************************
*/

USBD_DEV_CFG  USBD_DevCfg_ADI_Vendor = {
    0xFFFE,                                                     /* Vendor  ID.                                          */
#if defined(_MSC_)
    0x2208,                                                     /* Product ID.                                          */
#elif defined(_VENDOR_)
    0x1003,                                                     /* Product ID.                                          */
#elif (APP_CFG_USBD_PHDC_BASIC == DEF_ENABLED)
    0x1060,
#else
    0x1060,                                                     /* Product ID.                                          */
#endif
    0x0100,                                                     /* Device release number.                               */
   "OEM MANUFACTURER",                                          /* Manufacturer  string.                                */
   "OEM PRODUCT",                                               /* Product       string.                                */
   "1234567890ABCDEF",                                          /* Serial number string.                                */
    USBD_LANG_ID_ENGLISH_US                                     /* String language ID.                                  */
};

/*
*********************************************************************************************************
*                               ADI USB DEVICE CONTROLLER CONFIGURATION
*
*           (2) The EP information table is define in the USB device controller driver Board support
*               package BSP folder.
*********************************************************************************************************
*/

const  USBD_DRV_CFG  USBD_DrvCfg_ADI =  {
    0,                                                 			/* Base addr   of device controller hw registers.       */
    0x00000000,                                                 /* Base addr   of device controller dedicated mem.      */
    0u,                                                 /* Size        of device controller dedicated mem.      */

     USBD_DEV_SPD_FULL,                                         /* Speed       of device controller.                    */

     USBD_DrvEP_InfoTbl_MUSBMHDRC                                 /* EP Info tbl of device controller.                    */
};



