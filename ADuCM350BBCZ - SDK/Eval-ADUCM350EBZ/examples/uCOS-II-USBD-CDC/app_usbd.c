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
*               You can find our product's user manual, API reference, release notes and
*               more information at: https://doc.micrium.com
*
*               You can contact us at: http://www.micrium.com
*
*               Portions Copyright (c) 2014 Analog Devices, Inc.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                   USB APPLICATION INITIALIZATION
*
*                                              TEMPLATE
*
* Filename      : app_usbd.c
* Version       : V4.04.01
* Programmer(s) : FGK
*                 FT
*                 OD
*                 JFD
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#define   APP_USBD_MODULE
#include  <app_usbd.h>

#if (APP_CFG_USBD_EN == DEF_ENABLED)
#include  <usbd_core.h>
#include  <usbd_dev_cfg.h>
#include  <usbd_drv_adi.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

//static CPU_BOOLEAN bMediaReady = DEF_FALSE;


/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

                                                                /* ---------- USB DEVICE CALLBACKS FUNCTIONS ---------- */
static  void  App_USBD_EventReset  (CPU_INT08U   dev_nbr);
static  void  App_USBD_EventSuspend(CPU_INT08U   dev_nbr);
static  void  App_USBD_EventResume (CPU_INT08U   dev_nbr);
static  void  App_USBD_EventCfgSet (CPU_INT08U   dev_nbr,
                                    CPU_INT08U   cfg_val);
static  void  App_USBD_EventCfgClr (CPU_INT08U   dev_nbr,
                                    CPU_INT08U   cfg_val);
static  void  App_USBD_EventConn   (CPU_INT08U   dev_nbr);
static  void  App_USBD_EventDisconn(CPU_INT08U   dev_nbr);

static  USBD_BUS_FNCTS  App_USBD_BusFncts = {
    App_USBD_EventReset,
    App_USBD_EventSuspend,
    App_USBD_EventResume,
    App_USBD_EventCfgSet,
    App_USBD_EventCfgClr,
    App_USBD_EventConn,
    App_USBD_EventDisconn
};

CPU_BOOLEAN App_USBD_DevStart(CPU_INT08U dev_nbr);

/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                           App_USBD_Init()
*
* Description : Initialize USB device stack.
*
* Argument(s) : none.
*
* Return(s)   : DEF_OK,   if the stack was created successfully.
*               DEF_FAIL, if the stack could not be created.
*
* Caller(s)   : App_USB_Init().
*
* Note(s)     : none.
*********************************************************************************************************
*/

static CPU_INT08U   cdc_dev_nbr;
static CPU_INT08U   cdc_cfg_nbr;


static CPU_BOOLEAN  App_USBD_CreateDevice(USBD_DEV_CFG *p_dev_cfg, CPU_INT08U *p_dev_nbr, CPU_INT08U *p_cfg_nbr);

CPU_BOOLEAN  App_USBD_Init ()
{
    CPU_BOOLEAN  ok;
    USBD_ERR     err;

    APP_TRACE_DBG(("\r\n"));
    APP_TRACE_DBG(("===================================================================\r\n"));
    APP_TRACE_DBG(("=                    USB DEVICE INITIALIZATION                    =\r\n"));
    APP_TRACE_DBG(("===================================================================\r\n"));
    APP_TRACE_DBG(("Initializing USB Device...\r\n"));

    USBD_Init(&err);

    if (err!= USBD_ERR_NONE) {
        APP_TRACE_DBG(("... init failed w/err = %d\r\n\r\n", err));
        return (DEF_FAIL);
    }

    ok = App_USBD_CreateDevice(&USBD_DevCfg_ADI,  &cdc_dev_nbr,  &cdc_cfg_nbr);
    if (ok != DEF_OK) {
        return (DEF_FAIL);
    }
    ok = App_USBD_CDC_Init(cdc_dev_nbr, USBD_CFG_NBR_NONE, cdc_cfg_nbr);
    if (ok != DEF_OK) {
        return (DEF_FAIL);
    }

    ok = App_USBD_DevStart(cdc_dev_nbr);
    if (ok != DEF_OK) {
        return (DEF_FAIL);
    }

    return (DEF_OK);
}


CPU_BOOLEAN  App_USBD_CreateDevice(USBD_DEV_CFG *p_dev_cfg, CPU_INT08U *p_dev_nbr, CPU_INT08U *p_cfg_nbr )
{
    CPU_INT08U   dev_nbr;
    USBD_ERR     err;

    APP_TRACE_DBG(("    Adding controller driver ... \r\n"));
                                                                     /* Add USB device instance.                           */
    dev_nbr = USBD_DevAdd((USBD_DEV_CFG     *)p_dev_cfg,
                          (USBD_BUS_FNCTS   *)&App_USBD_BusFncts,
                          (USBD_DRV_API     *)&USBD_DrvAPI_ADI,
                          (USBD_DRV_CFG     *)&USBD_DrvCfg_ADI,
                          (USBD_DRV_BSP_API *)&USBD_DrvBSP_ADI,
                          &err);

    *p_dev_nbr = dev_nbr;

    if (err != USBD_ERR_NONE) {
        APP_TRACE_DBG(("    ... could not add controller driver w/err =  %d\r\n\r\n", err));
        return (DEF_FAIL);
    }

    APP_TRACE_DBG(("    Adding FS configuration ... \r\n"));

    *p_cfg_nbr = USBD_CfgAdd( dev_nbr,                          /* Add FS configuration.                                */
                              USBD_DEV_ATTRIB_SELF_POWERED,
                              100u,
                              USBD_DEV_SPD_FULL,
                             "FS configuration",
                             &err);

    if (err != USBD_ERR_NONE) {
        APP_TRACE_DBG(("    ... could not add FS configuration w/err =  %d\r\n\r\n", err));
        return (DEF_FAIL);
    }

    return (DEF_OK);
}

CPU_BOOLEAN App_USBD_DevStop(CPU_INT08U dev_nbr)
{
    USBD_ERR     err;

    USBD_DevStop(dev_nbr, &err);
    if (err!=USBD_ERR_NONE) {
      return DEF_FAIL;
    }

    return (DEF_OK);
}

CPU_BOOLEAN App_USBD_DevStart(CPU_INT08U dev_nbr)
{
    USBD_ERR     err;

    USBD_DevStart(dev_nbr, &err);
    if (err!=USBD_ERR_NONE) {
      return DEF_FAIL;
    }

    return (DEF_OK);
}

/*
*********************************************************************************************************
*********************************************************************************************************
*                                            USB CALLBACKS
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        App_USBD_EventReset()
*
* Description : Bus reset event callback function.
*
* Argument(s) : dev_nbr     Device number.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_EventProcess() via 'p_bus_fnct->Reset()'.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_EventReset (CPU_INT08U  dev_nbr)
{
    (void)dev_nbr;
}


/*
*********************************************************************************************************
*                                       App_USBD_EventSuspend()
*
* Description : Bus suspend event callback function.
*
* Argument(s) : dev_nbr     Device number.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_EventProcess() via 'p_bus_fnct->Suspend()'.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_EventSuspend (CPU_INT08U  dev_nbr)
{
    /*
     * On disconnection, clear the media ready flag
    */
    //bMediaReady = DEF_FALSE;
}


/*
*********************************************************************************************************
*                                       App_USBD_EventResume()
*
* Description : Bus Resume event callback function.
*
* Argument(s) : dev_nbr     Device number.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_EventProcess() via 'p_bus_fnct->Resume()'.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_EventResume (CPU_INT08U  dev_nbr)
{
    (void)dev_nbr;
}


/*
*********************************************************************************************************
*                                       App_USBD_EventCfgSet()
*
* Description : Set configuration callback event callback function.
*
* Argument(s) : dev_nbr     Device number.
*
*               cfg_nbr     Active device configuration.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_CfgOpen() via 'p_dev->BusFnctsPtr->CfgSet()'.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_EventCfgSet (CPU_INT08U  dev_nbr,
                                    CPU_INT08U  cfg_val)
{
#if 0
    /* To prevent this function repeatedly cycling the USB device status,
     * only do if bMediaReady is not set
    */
    if (!bMediaReady)
    {
        /* Stop the device just enumerated
        */
        App_USBD_DevStop(dev_nbr);

        /* set media ready flag to prevent recursive calls
        */
        bMediaReady = DEF_TRUE;

        /* set off task to prepare the RAMDisk and restart the device
        */
        Create_RAMDiskPrepareTask(dev_nbr);
    }
#endif
}


/*
*********************************************************************************************************
*                                       App_USBD_EventCfgClr()
*
* Description : Clear configuration event callback function.
*
* Argument(s) : dev_nbr     Device number.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_CfgClose via 'p_dev->BusFnctsPtr->CfgClr()'.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_EventCfgClr (CPU_INT08U  dev_nbr,
                                    CPU_INT08U  cfg_val)
{
    (void)dev_nbr;
    (void)cfg_val;
}

/*
*********************************************************************************************************
*                                        App_USBD_EventConn()
*
* Description : Device connection event callback function.
*
* Argument(s) : dev_nbr     Device number.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_EventProcess() via 'p_bus_fnct->Conn()'.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_EventConn (CPU_INT08U  dev_nbr)
{
    (void)dev_nbr;
}


/*
*********************************************************************************************************
*                                       App_USBD_EventDisconn()
*
* Description : Device connection event callback function.
*
* Argument(s) : dev_nbr     USB device number.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_EventProcess() via 'p_bus_fnct->Disconn()'.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_EventDisconn (CPU_INT08U  dev_nbr)
{
    (void)dev_nbr;
}


/*
*********************************************************************************************************
*                                            USBD_Trace()
*
* Description : Function to output or log USB trace events.
*
* Argument(s) : p_str       Pointer to string containing the trace event information.
*
* Return(s)   : none.
*
* Caller(s)   : USBD_DbgTaskHandler().
*
* Note(s)     : none.
*********************************************************************************************************
*/

void  USBD_Trace (const  CPU_CHAR  *p_str)
{
    (void)p_str;
}
#endif
