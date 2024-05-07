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
* File          : usbd_dev_cfg.h
* Version       : V4.04.01
* Programmer(s) : FGK
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                               MODULE
*
* Note(s) : (1) This USB device configuration header file is protected from multiple pre-processor
*               inclusion through use of the USB device configuration module present pre-processor
*               macro definition.
*********************************************************************************************************
*/

#ifndef  USBD_DEV_CFG_MODULE_PRESENT                            /* See Note #1.                                         */
#define  USBD_DEV_CFG_MODULE_PRESENT


/*
*********************************************************************************************************
*                                      USB DEVICE CONFIGURATION
*********************************************************************************************************
*/

extern  USBD_DEV_CFG  USBD_DevCfg_ADI_Vendor;


/*
*********************************************************************************************************
*                                   USB DEVICE DRIVER CONFIGURATION
*********************************************************************************************************
*/

extern  USBD_DRV_CFG  USBD_DrvCfg_ADI;


/*
*********************************************************************************************************
*                                             MODULE END
*********************************************************************************************************
*/

#endif
