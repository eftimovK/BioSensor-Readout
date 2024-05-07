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
*                           USB DEVICE PHDC TEST APPLICATION (SINGLE TASK)
*
*                                              TEMPLATE
*
* Filename      : app_usbd_phdc.c
* Version       : V4.00
* Programmer(s) : JFD
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            INCLUDE FILES
*********************************************************************************************************
*/

#include <app_usbd.h>

#if (APP_CFG_USBD_PHDC_EN == DEF_ENABLED) && (APP_CFG_USBD_PHDC_BASIC == DEF_ENABLED)
#include <usbd_phdc.h>
#include <ucos_ii.h>
#include <hostapp.h>
#include <stdlib.h>
#include <string.h>
/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

#define  APP_USBD_PHDC_ITEM_DATA_LEN_MAX                  64    /* Should be the same value as host app.                */
#define  APP_USBD_PHDC_ITEM_DATA_LEN_MIN                   5
#define  APP_USBD_PHDC_ITEM_NBR_MAX                        1    /* Should be the same value as host app.                */
#define  APP_USBD_PHDC_ITEM_Q_NBR_MAX                     APP_USBD_PHDC_ITEM_NBR_MAX
#define  APP_USBD_PHDC_ITEM_DFLT_PERIOD                  100


/*
*********************************************************************************************************
*                                          LOCAL DATA TYPES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                        APPLICATION COMMANDS
*********************************************************************************************************
*/

typedef  enum  app_usbd_phdc_cmds {
    APP_USBD_PHDC_CMD_NONE     = 0,
    APP_USBD_PHDC_CMD_RESET    = 1,
    APP_USBD_PHDC_CMD_ADD_ITEM = 2,
    APP_USBD_PHDC_CMD_USBCB    = 3,
    APP_USBD_PHDC_CMD_SIGNOFF  = 4
} APP_USBD_PHDC_CMDS;


/*
*********************************************************************************************************
*                                          APPLICATION ITEM
*
* Note(s): (1) Data buffer of each item has the following mapping:
*
*                   +---------+---------------+-------------------+-----------------------------------+
*                   | Offset  |     Field     |        Size       |            Description            |
*                   +---------+---------------+-------------------+-----------------------------------+
*                   |    0    | Cmd           |         1         | Command from host.                |
*                   +---------+---------------+-------------------+-----------------------------------+
*                   |    1    | Item index    |         1         | Index of item.                    |
*                   +---------+---------------+-------------------+-----------------------------------+
*                   |    2    | Period (LSB)  |         1         | Periodicity of transfer.          |
*                   +---------+---------------+-------------------+-----------------------------------+
*                   |    3    | Period (MSB)  |         1         | Periodicity of transfer.          |
*                   +---------+---------------+-------------------+-----------------------------------+
*                   |    4    | Latency /     |         1         | Latency / reliability (QoS) of    |
*                   |         | Reliability   |                   | transfer.                         |
*                   +---------+---------------+-------------------+-----------------------------------+
*                   |    5    | Data          | [0..DATA_LEN_MAX] | Data.                             |
*                   +---------+---------------+-------------------+-----------------------------------+
*
*********************************************************************************************************
*/

typedef  struct  app_usbd_phdc_item {
    CPU_INT16U           Period;                                /* Periodicity of xfer.                                 */
    LATENCY_RELY_FLAGS   LatencyRely;                           /* Latency / rely of xfer.                              */
    CPU_INT08U           *Data;//[APP_USBD_PHDC_ITEM_DATA_LEN_MAX];
    CPU_INT08U           DataOpaque[sizeof(APP_PHDC_OPAQUE_DEF)];
    CPU_INT16U           DataLen;
    CPU_INT08U           DataOpaqueLen;
    OS_TMR              *Tmr;                                   /* Item's timer.                                        */
} APP_USBD_PHDC_ITEM;


/*
*********************************************************************************************************
*                                           LOCAL CONSTANTS
*********************************************************************************************************
*/

static  const  CPU_INT08U  App_USBD_PHDC_OpaqueDataRx[]           = {"Micrium PHDC Rx"};
static  const  CPU_INT08U  App_USBD_PHDC_OpaqueDataTx[]           = {"Micrium PHDC Tx"};
//static  const  CPU_INT08U  App_USBD_PHDC_OpaqueDataTxLowLatency[] = {"Micrium PHDC Tx low latency"};


/*
*********************************************************************************************************
*                                       LOCAL GLOBAL VARIABLES
*********************************************************************************************************
*/

static  OS_STK               App_USBD_PHDC_TxCommTaskStk[APP_CFG_USBD_PHDC_TASK_STK_SIZE];
static  OS_STK               App_USBD_PHDC_RxCommTaskStk[APP_CFG_USBD_PHDC_TASK_STK_SIZE];

static  CPU_BOOLEAN          App_USBD_PHDC_PreambleEn;
static  CPU_INT08U           App_USBD_PHDC_ClassNbr;

static  APP_USBD_PHDC_ITEM   App_USBD_PHDC_Items[APP_USBD_PHDC_ITEM_NBR_MAX];

static  OS_EVENT            *App_USBD_PHDC_ItemQ;
static  void                *App_USBD_PHDC_ItemQBuffer[APP_USBD_PHDC_ITEM_Q_NBR_MAX];

static	APP_USBD_PHDC_ITEM  *p_dflt_item;

#pragma data_alignment = 4

#define NUM_VERSION_STRINGS         5
#define MAX_VERSION_STRING_LEN      32

char FwVersionInfo[NUM_VERSION_STRINGS][MAX_VERSION_STRING_LEN] = {
                                                                    __DATE__,       /* build date       */
                                                                    __TIME__,       /* build time       */
                                                                    "01.00.00",     /* version number   */
#ifdef __ADSPBF527__
                                                                    "ADSP-BF527",   /* target processor */
#elif defined(__ADSPBF548__)
                                                                    "ADSP-BF548",   /* target processor */
#elif defined(__ADSPBF526__)
                                                                    "ADSP-BF526",   /* target processor */
#elif defined(__ADSPBF609__)
                                                                    "ADSP-BF609",   /* target processor */
#else
                                                                    "ADuCM350  ", /* target processor */

#endif
                                                                    "uC-PHDC"};    /* application name */

static APP_PHDC_OPAQUE_DEF TempOpaqueData;
static APP_PHDC_DATA_ROW App_PHDC_Data[APP_PHDC_MAX_DATA_ROWS];

/*
*********************************************************************************************************
*                                      LOCAL FUNCTION PROTOTYPES
*********************************************************************************************************
*/

        CPU_BOOLEAN  App_USBD_PHDC_Init         (CPU_INT08U    dev_nbr,
                                                 CPU_INT08U    cfg_hs,
                                                 CPU_INT08U    cfg_fs);

static  void         App_USBD_PHDC_RxCommTask   (void         *p_arg);

static  void         App_USBD_PHDC_TxCommTask   (void         *p_arg);

static  void         App_USBD_PHDC_SetPreambleEn(CPU_INT08U    class_nbr,
                                                 CPU_BOOLEAN   preamble_en);


static void App_USBD_PHDC_ProcessData(CPU_INT08U *data, CPU_INT32U data_len, CPU_INT08U *data_opaque, CPU_INT08U data_opaque_len);
static void App_USBD_PHDC_ProcessUSBCB(USBCB *pUSBCB);
static void App_USBD_PHDC_PostItem( APP_USBD_PHDC_ITEM *p_item);

static void App_USBD_PHDC_FillDataRow(APP_PHDC_DATA_ROW *pRow, unsigned long Date,unsigned long Time, unsigned long Data);
static unsigned long TempValue(void);
static void App_USBD_PHDC_FillDataArray(APP_PHDC_DATA_ROW *pData, CPU_INT32U NumRows);
static void App_USBD_PHDC_SendPHDCData(CPU_INT32U DataLen, APP_PHDC_DATA_TYPE DataType);


/*
*********************************************************************************************************
*                                     LOCAL CONFIGURATION ERRORS
*********************************************************************************************************
*/

#if  (OS_TMR_CFG_TICKS_PER_SEC < 100u)
#error  "OS_TMR_CFG_TICKS_PER_SEC              illegally #define'd in 'os_cfg.h'"
#error  "                                      [MUST be >= 100]                 "
#endif


/*
*********************************************************************************************************
*                                          GLOBAL FUNCTIONS
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                        App_USBD_PHDC_Init()
*
* Description : Add a PHDC interface to the USB device stack and initialize it.
*
* Argument(s) : dev_nbr     Device number.
*
*               cfg_hs      Index of high-speed configuration to which this interface will be added to.
*
*               cfg_fs      Index of high-speed configuration to which this interface will be added to.
*
* Return(s)   : DEF_OK,   if PHDC interface successfully added.
*
*               DEF_FAIL, otherwise.
*
* Caller(s)   : App_USBD_Create().
*
* Note(s)     : none.
*********************************************************************************************************
*/

CPU_BOOLEAN  App_USBD_PHDC_Init (CPU_INT08U  dev_nbr,
                                 CPU_INT08U  cfg_hs,
                                 CPU_INT08U  cfg_fs)
{
    CPU_BOOLEAN          valid_cfg_hs;
    CPU_BOOLEAN          valid_cfg_fs;
    USBD_ERR             err;
    INT8U                os_err;
    LATENCY_RELY_FLAGS   latency_rely_flags;


    USBD_PHDC_Init(&err);                                       /* ------------------- INIT PHDC ---------------------- */
    if (err != USBD_ERR_NONE) {
        return (DEF_FAIL);
    }
                                                                /* Add PHDC instance.                                   */
    App_USBD_PHDC_ClassNbr = USBD_PHDC_Add(DEF_NO,              /* Vendor-defined data format.                          */
                                           DEF_YES,             /* Preamble capable.                                    */
                                           App_USBD_PHDC_SetPreambleEn,
                                           10,
                                          &err);

    if (err != USBD_ERR_NONE) {
        return (DEF_FAIL);
    }

    latency_rely_flags = USBD_PHDC_LATENCY_VERYHIGH_RELY_BEST |
                         USBD_PHDC_LATENCY_HIGH_RELY_BEST     |
                         USBD_PHDC_LATENCY_MEDIUM_RELY_BEST;
    USBD_PHDC_RdCfg(App_USBD_PHDC_ClassNbr,                     /* Cfg rd xfers with all possible latency/rely flags.   */
                    latency_rely_flags,
                    App_USBD_PHDC_OpaqueDataRx,
                    sizeof(App_USBD_PHDC_OpaqueDataRx),
                   &err);

    if (err != USBD_ERR_NONE) {
        return (DEF_FAIL);
    }

    USBD_PHDC_WrCfg(App_USBD_PHDC_ClassNbr,
                    USBD_PHDC_LATENCY_VERYHIGH_RELY_BEST,
                    App_USBD_PHDC_OpaqueDataTx,
                    sizeof(App_USBD_PHDC_OpaqueDataTx),
                   &err);

    if (err != USBD_ERR_NONE) {
        return (DEF_FAIL);
    }

    if (cfg_hs != USBD_CFG_NBR_NONE) {
        valid_cfg_hs = USBD_PHDC_CfgAdd(App_USBD_PHDC_ClassNbr,
                                        dev_nbr,
                                        cfg_hs,
                                       &err);                   /* Add PHDC class to HS dflt cfg.                       */
    }

    if (cfg_fs != USBD_CFG_NBR_NONE) {
        valid_cfg_fs = USBD_PHDC_CfgAdd(App_USBD_PHDC_ClassNbr,
                                        dev_nbr,
                                        cfg_fs,
                                       &err);                   /* Add PHDC class to FS dflt cfg.                       */
    }

    if ((valid_cfg_fs == DEF_NO) &&
        (valid_cfg_hs == DEF_NO)) {
        return (DEF_FAIL);
    }

                                                                /* -------------------- INIT APP ---------------------- */
    p_dflt_item                = &App_USBD_PHDC_Items[0];      /* Init dflt item.                                      */
    p_dflt_item->Period        = APP_USBD_PHDC_ITEM_DFLT_PERIOD;
    p_dflt_item->LatencyRely   = USBD_PHDC_LATENCY_VERYHIGH_RELY_BEST;
    p_dflt_item->DataLen       = APP_USBD_PHDC_ITEM_DATA_LEN_MIN + 3;
    p_dflt_item->DataOpaqueLen = 0;

    App_USBD_PHDC_ItemQ = OSQCreate(&App_USBD_PHDC_ItemQBuffer[0],
                                     APP_USBD_PHDC_ITEM_Q_NBR_MAX);

    if (App_USBD_PHDC_ItemQ == (OS_EVENT *)0) {
        APP_TRACE_DBG(("Could not create item Queue \r\n"));
        return (DEF_FAIL);
    }

    App_USBD_PHDC_PreambleEn  = DEF_DISABLED;                   /* Metadata msg preamble disabled by default.           */

#if (OS_TASK_CREATE_EXT_EN == 1u)

#if (OS_STK_GROWTH == 1u)                                       /* Create task that will handle rd procedures.          */
    os_err = OSTaskCreateExt(        App_USBD_PHDC_RxCommTask,
                             (void *)0,
                                    &App_USBD_PHDC_RxCommTaskStk[APP_CFG_USBD_PHDC_TASK_STK_SIZE - 1u],
                                     APP_CFG_USBD_PHDC_RX_COMM_TASK_PRIO,
                                     APP_CFG_USBD_PHDC_RX_COMM_TASK_PRIO,
                                    &App_USBD_PHDC_RxCommTaskStk[0],
                                     APP_CFG_USBD_PHDC_TASK_STK_SIZE,
                             (void *)0,
                                     OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK);
#else
    os_err = OSTaskCreateExt(        App_USBD_PHDC_RxCommTask,
                             (void *)0,
                                    &App_USBD_PHDC_RxCommTaskStk[0],
                                     APP_CFG_USBD_PHDC_RX_COMM_TASK_PRIO,
                                     APP_CFG_USBD_PHDC_RX_COMM_TASK_PRIO,
                                    &App_USBD_PHDC_RxCommTaskStk[APP_CFG_USBD_PHDC_TASK_STK_SIZE - 1u],
                                     APP_CFG_USBD_PHDC_TASK_STK_SIZE,
                             (void *)0,
                                     OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK);
#endif

#else

#if (OS_STK_GROWTH == 1u)
    os_err = OSTaskCreate(        App_USBD_PHDC_RxCommTask,
                          (void *)0,
                                 &App_USBD_PHDC_RxCommTaskStk[APP_CFG_USBD_PHDC_TASK_STK_SIZE - 1u],
                                  APP_CFG_USBD_PHDC_RX_COMM_TASK_PRIO);
#else
    os_err = OSTaskCreate(        App_USBD_PHDC_RxCommTask,
                          (void *)0,
                                 &App_USBD_PHDC_RxCommTaskStk[0],
                                  APP_CFG_USBD_PHDC_RX_COMM_TASK_PRIO);
#endif

#endif

#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(APP_CFG_USBD_PHDC_RX_COMM_TASK_PRIO, (INT8U *)"USB Device PHDC RX comm", &os_err);
#endif

    if (os_err != OS_ERR_NONE) {
        APP_TRACE_DBG(("Could not add PHDC rx comm task w/err = %d\r\n", os_err));
        return (DEF_FAIL);
    }

#if (OS_TASK_CREATE_EXT_EN == 1u)

#if (OS_STK_GROWTH == 1u)                                       /* Create task that will handle wr procedures.          */
    os_err = OSTaskCreateExt(        App_USBD_PHDC_TxCommTask,
                             (void *)0,
                                    &App_USBD_PHDC_TxCommTaskStk[APP_CFG_USBD_PHDC_TASK_STK_SIZE - 1u],
                                     APP_CFG_USBD_PHDC_TX_COMM_TASK_PRIO,
                                     APP_CFG_USBD_PHDC_TX_COMM_TASK_PRIO,
                                    &App_USBD_PHDC_TxCommTaskStk[0],
                                     APP_CFG_USBD_PHDC_TASK_STK_SIZE,
                             (void *)0,
                                     OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK);
#else
    os_err = OSTaskCreateExt(        App_USBD_PHDC_TxCommTask,
                             (void *)0,
                                    &App_USBD_PHDC_TxCommTaskStk[0],
                                     APP_CFG_USBD_PHDC_TX_COMM_TASK_PRIO,
                                     APP_CFG_USBD_PHDC_TX_COMM_TASK_PRIO,
                                    &App_USBD_PHDC_TxCommTaskStk[APP_CFG_USBD_PHDC_TASK_STK_SIZE - 1u],
                                     APP_CFG_USBD_PHDC_TASK_STK_SIZE,
                             (void *)0,
                                     OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK);
#endif

#else

#if (OS_STK_GROWTH == 1u)
    os_err = OSTaskCreate(        App_USBD_PHDC_TxCommTask,
                          (void *)0,
                                 &App_USBD_PHDC_TxCommTaskStk[APP_CFG_USBD_PHDC_TASK_STK_SIZE - 1u],
                                  APP_CFG_USBD_PHDC_TX_COMM_TASK_PRIO);
#else
    os_err = OSTaskCreate(        App_USBD_PHDC_TxCommTask,
                          (void *)cnt,
                                 &App_USBD_PHDC_TxCommTaskStk[0],
                                  APP_CFG_USBD_PHDC_TX_COMM_TASK_PRIO);
#endif

#endif

#if (OS_TASK_NAME_EN > 0)
    OSTaskNameSet(APP_CFG_USBD_PHDC_TX_COMM_TASK_PRIO, (INT8U *)"USB Device PHDC TX comm", &os_err);
#endif

    /* Fill the Temperature data */
    App_USBD_PHDC_FillDataArray(App_PHDC_Data, APP_PHDC_MAX_DATA_ROWS);

    TempOpaqueData.DataLen = APP_PHDC_MAX_DATA_ROWS*sizeof(APP_PHDC_DATA_ROW);
    TempOpaqueData.DataType = (unsigned long)PHDC_TEMPERATURE;
    strcpy(TempOpaqueData.DataUnits, "deg C");
    strcat(TempOpaqueData.DataUnits, "\0");

    if (os_err != OS_ERR_NONE) {
        APP_TRACE_DBG(("Could not add PHDC tx comm task w/err = %d\r\n", os_err));
        return (DEF_FAIL);
    }

    return (DEF_OK);
}

static CPU_INT32U gnTimeout=0;

/*
*********************************************************************************************************
*                                     App_USBD_PHDC_RxCommTask()
*
* Description : Read PHDC data and opaque data from host. Then take action in function of the application
*               command sent by host.
*
* Argument(s) : p_arg       Argument passed to the task.
*
* Return(s)   : none.
*
* Caller(s)   : This is a Task.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_PHDC_RxCommTask (void *p_arg)
{
    CPU_BOOLEAN          is_conn;
    CPU_INT08U           data_opaque_len;
    CPU_INT08U           data_opaque[USBD_PHDC_CFG_DATA_OPAQUE_MAX_LEN];
    CPU_INT08U           data[APP_USBD_PHDC_ITEM_DATA_LEN_MAX];
    CPU_INT08U           nbr_xfer, i;
    CPU_INT16U           xfer_len;
    USBD_ERR             usb_err;
    CPU_INT32U           data_len;
    CPU_BOOLEAN          bSendAssocRqst = DEF_FALSE;

    (void)&p_arg;
    p_dflt_item = &App_USBD_PHDC_Items[0];

    while (DEF_TRUE) {
        is_conn = USBD_PHDC_IsConn(App_USBD_PHDC_ClassNbr);     /* Wait for configuration.                              */
        while (is_conn == DEF_NO) {
            OSTimeDlyHMSM(0u, 0u, 0u, 100u);
            is_conn = USBD_PHDC_IsConn(App_USBD_PHDC_ClassNbr);
            if (is_conn == DEF_YES) {
                bSendAssocRqst = DEF_TRUE;
            }
        }
#if defined(__TEST_ASSOC_RQST_TIMEOUT__)
        if (bSendAssocRqst == DEF_TRUE) {
            bSendAssocRqst = DEF_FALSE;
            /* send association request */
            USBD_PHDC_Wr( App_USBD_PHDC_ClassNbr,
                             (void *)data_opaque,
                                     32,
                                     USBD_PHDC_LATENCY_VERYHIGH_RELY_BEST,
                                     500u,
                                    &usb_err);

            if (usb_err == USBD_ERR_OS_TIMEOUT) {
                printf("send association request timeout\n");
            }
            else {

                USBD_PHDC_Rd( App_USBD_PHDC_ClassNbr,
                                 (void *)data_opaque,
                                         32,
                                         500u,
                                        &usb_err);

                if (usb_err == USBD_ERR_OS_TIMEOUT) {
                    printf("association request acknowledgement timeout\n");
                }
            }
        }
#endif

        if (App_USBD_PHDC_PreambleEn == DEF_ENABLED) {          /* Host decide of preamble state.                       */
            APP_TRACE_DBG(("Queueing Preamble packet\n"));
            data_opaque_len = USBD_PHDC_PreambleRd(        App_USBD_PHDC_ClassNbr,
                                                   (void *)data_opaque,
                                                           USBD_PHDC_CFG_DATA_OPAQUE_MAX_LEN,
                                                          &nbr_xfer,
                                                           0,
                                                          &usb_err);

            if (usb_err ) {
                if (usb_err != USBD_ERR_OS_ABORT) {             /* Host changed preamble state when xfer is aborted.    */
                    APP_TRACE_DBG(("Rd preamble failed w/err = %d\r\n", usb_err));
                    OSTimeDlyHMSM(0u, 0u, 0u, 250u);
                }
                continue;
            }
            APP_TRACE_DBG(("Preamble packet received, %d bytes, %d reps\n",data_opaque_len, nbr_xfer));
        }

        /*
         * Set data length size (first 32 bit word in opaque data)
         */
        if (App_USBD_PHDC_PreambleEn && data_opaque_len) {
            data_len = App_USBD_PHDC_GetDataLengthFromOpaqueData(data_opaque);
        }
        else {
            /* Preamble disabled, so we only accept a single byte command */
            data_len = 1;
            nbr_xfer = 1;
        }

        /* In the following, the same data buffer is used. It could be copied aside,
         * if required, in the App_USBD_PHDC_ProcessData() function, or this could be replaced with
         * a larger buffer that is incremented each time round the loop
        */
        for (i=0; i < nbr_xfer; i++)
        {
            APP_TRACE_DBG(("Queueing Data buffer, %d bytes\n",data_len));

            xfer_len = USBD_PHDC_Rd(App_USBD_PHDC_ClassNbr,
                                        data,
                                        data_len,
                                        0u,
                                       &usb_err);

            if (usb_err != USBD_ERR_NONE) {
                if (usb_err != USBD_ERR_OS_ABORT) {
                    APP_TRACE_DBG(("Rd failed w/err = %d\r\n", usb_err));
                    OSTimeDlyHMSM(0u, 0u, 0u, 250u);
                }
                if (usb_err == USBD_ERR_OS_TIMEOUT) {
                  gnTimeout++;
                }
                continue;
            }
            APP_TRACE_DBG(("Data buffer received, %d bytes\n",xfer_len));

            App_USBD_PHDC_ProcessData(data, data_len, data_opaque, data_opaque_len);
        }
    }
}

/*
*********************************************************************************************************
*                                     App_USBD_PHDC_TxCommTask()
*
* Description : Write next item data in queue along with its preamble.
*
* Argument(s) : p_arg       Argument passed to the task.
*
* Return(s)   : none.
*
* Caller(s)   : This is a Task.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_PHDC_TxCommTask (void *p_arg)
{
    CPU_BOOLEAN          is_conn;
    USBD_ERR             usb_err;
    INT8U                os_err;
    APP_USBD_PHDC_ITEM  *p_item;

    while (DEF_TRUE) {
        is_conn = USBD_PHDC_IsConn(App_USBD_PHDC_ClassNbr);     /* Wait for configuration.                              */
        while ((is_conn                  == DEF_NO      ) ||
               (App_USBD_PHDC_PreambleEn == DEF_DISABLED)) {
            OSTimeDlyHMSM(0u, 0u, 0u, 100u);
            is_conn = USBD_PHDC_IsConn(App_USBD_PHDC_ClassNbr);
        }
                                                                /* Wait for data to wr on Q.                            */
        p_item = (APP_USBD_PHDC_ITEM *)OSQPend(App_USBD_PHDC_ItemQ,
                                               0,
                                              &os_err);

        APP_TRACE_DBG(("Sending Item, datalen = %d, opaque datalen = %d\n",p_item->DataLen,p_item->DataOpaqueLen));
        if (os_err != OS_ERR_NONE) {
            APP_TRACE_DBG(("Queue pend failed w/err = %d\r\n", os_err));
            OSTimeDlyHMSM(0u, 0u, 0u, 100u);
            continue;
        }
        usb_err = USBD_ERR_NONE;                                /* Write preamble (if applicable) and data to host.     */
        if ((App_USBD_PHDC_PreambleEn == DEF_ENABLED                    ) &&
            (p_item->LatencyRely      != USBD_PHDC_LATENCY_LOW_RELY_GOOD)) {

            APP_TRACE_DBG(("Sending Preamble ...\n"));

            USBD_PHDC_PreambleWr(        App_USBD_PHDC_ClassNbr,
                                 (void *)p_item->DataOpaque,
                                         p_item->DataOpaqueLen,
                                         p_item->LatencyRely,
                                         1,
                                         0,
                                        &usb_err);

            if (usb_err != USBD_ERR_NONE) {
                APP_TRACE_DBG(("Could not write PHDC item #%d preamble w/err = %d\r\n",
                                p_item->Data[1],
                                usb_err));
            } else {
                APP_TRACE_DBG(("Done\n"));
            }

        }

        if (usb_err == USBD_ERR_NONE) {
            APP_TRACE_DBG(("Sending Data ...\n"));
            USBD_PHDC_Wr(        App_USBD_PHDC_ClassNbr,
                         (void *)p_item->Data,
                                 p_item->DataLen,
                                 p_item->LatencyRely,
                                 0,
                                &usb_err);

            if (usb_err != USBD_ERR_NONE) {
                APP_TRACE_DBG(("Could not write PHDC item #%d w/err = %d\r\n",
                               p_item->Data[1],
                               usb_err));
            } else {
                APP_TRACE_DBG(("Done\n"));
            }
        }
    }
}


/*
*********************************************************************************************************
*                                    App_USBD_PHDC_SetPreambleEn()
*
* Description : Callback called when host enable/disable preambles.
*
* Argument(s) : class_nbr       PHDC instance number.
*
*               preamble_en     DEF_ENABLED,  if preambles are enabled
*                               DEF_DISABLED, otherwise
*
* Return(s)   : none.
*
* Caller(s)   : PHDC.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_USBD_PHDC_SetPreambleEn (CPU_INT08U   class_nbr,
                                           CPU_BOOLEAN  preamble_en)
{
    (void)&class_nbr;

    App_USBD_PHDC_PreambleEn = preamble_en;
}


static void App_USBD_PHDC_PostItem( APP_USBD_PHDC_ITEM       *p_item)
{
    INT8U                     os_err;
    APP_TRACE_DBG(("Posting Item, datalen = %d, opaque datalen = %d\n",p_item->DataLen,p_item->DataOpaqueLen));

    os_err = OSQPost(        App_USBD_PHDC_ItemQ,               /* Post item on Q.                                      */
                     (void *)p_item);

    if (os_err != OS_ERR_NONE) {
        APP_TRACE_DBG(("Q post failed w/err %d\r\n", os_err));
        return;
    }

}

/*
*********************************************************************************************************
*                                     App_USBD_PHDC_ProcessData()
*
* Description : Process the USBCB structure
*
* Argument(s) : p_arg       Argument passed to the task.
*
* Return(s)   : none.
*
* Caller(s)   : This is a Task.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static void App_USBD_PHDC_ProcessData(CPU_INT08U *data, CPU_INT32U data_len, CPU_INT08U *data_opaque, CPU_INT08U data_opaque_len)
{
    INT8U           os_err;

    switch (data[0]) {                                      /* First data octets is the cmd.                        */
        case APP_USBD_PHDC_CMD_ADD_ITEM:                    /* Host added a new item.                               */
            break;

        case APP_USBD_PHDC_CMD_USBCB:                       /* Host sends a command                                 */
            App_USBD_PHDC_ProcessUSBCB((USBCB*)&data[4]);
            break;

        case APP_USBD_PHDC_CMD_SIGNOFF:                       /* Host send reset cmd when app start.                  */
            APP_TRACE_DBG(("APP_USBD_PHDC_CMD_SIGNOFF\n"));
            break;

        case APP_USBD_PHDC_CMD_RESET:                       /* Host send reset cmd when app start.                  */
            APP_TRACE_DBG(("APP_USBD_PHDC_CMD_RESET\n"));

            USBD_PHDC_Reset(App_USBD_PHDC_ClassNbr);       /* Reset PHDC instance.                                 */

             os_err = OSQFlush(App_USBD_PHDC_ItemQ);

             if (os_err != OS_ERR_NONE) {
                 APP_TRACE_DBG(("Could not flush queue w/err = %d\r\n", os_err));
             }

             /*
              * Await Preamble Enable Request
              */
             while (App_USBD_PHDC_PreambleEn != DEF_ENABLED) {
                 OSTimeDlyHMSM(0u, 0u, 0u, 100u);
             }
             break;


        default:
             break;
    }

}


static void App_USBD_PHDC_FillDataRow(APP_PHDC_DATA_ROW *pRow, unsigned long Date,unsigned long Time, unsigned long Data)
{
    pRow->Date = Date;
    pRow->Time = Time;
    pRow->Data = Data;
}
static unsigned long TempValue(void)
{
    unsigned long Temp = 32;
    unsigned long value = rand()*8/RAND_MAX;
    Temp +=  value;

    return Temp;
}

static void App_USBD_PHDC_FillDataArray(APP_PHDC_DATA_ROW *pData, CPU_INT32U NumRows)
{
    CPU_INT32U i;
    for (i=0; i < NumRows; i++)
    {
        App_USBD_PHDC_FillDataRow(&pData[i], i, 0, TempValue());
    }
}

static void App_USBD_PHDC_SendPHDCData(CPU_INT32U DataLen, APP_PHDC_DATA_TYPE DataType)
{
    p_dflt_item->Data                         = (CPU_INT08U*)App_PHDC_Data;
    p_dflt_item->DataLen                      = (CPU_INT16U)(DataLen&0x0000FFFF);
    p_dflt_item->DataOpaqueLen                = (CPU_INT08U)(sizeof(APP_PHDC_OPAQUE_DEF)&0x000000FF);
    memcpy(p_dflt_item->DataOpaque,(CPU_INT08U*)&TempOpaqueData, sizeof(APP_PHDC_OPAQUE_DEF));

    App_USBD_PHDC_PostItem(p_dflt_item);

}
/*
*********************************************************************************************************
*                                     App_USBD_PHDC_ProcessUSBCB()
*
* Description : Process the USBCB structure
*
* Argument(s) : p_arg       Argument passed to the task.
*
* Return(s)   : none.
*
* Caller(s)   : This is a Task.
*
* Note(s)     : none.
*********************************************************************************************************
*/
static void App_USBD_PHDC_ProcessUSBCB(USBCB *pUSBCB)
{
    APP_TRACE_DBG(("App_USBD_PHDC_ProcessUSBCB, cmd = %d, count = %d\n",pUSBCB->u32_Command,pUSBCB->u32_Count));

    switch (pUSBCB->u32_Command) {
        case GET_FW_VERSION:
            p_dflt_item->Data                        = (CPU_INT08U*)FwVersionInfo;
            p_dflt_item->DataLen                     = pUSBCB->u32_Count;
            p_dflt_item->DataOpaqueLen               = sizeof(CPU_INT32U);
            *(CPU_INT32U*)&p_dflt_item->DataOpaque[0] = pUSBCB->u32_Count;
            App_USBD_PHDC_PostItem(p_dflt_item);
            break;

        case MEMORY_WRITE:
            break;
        case MEMORY_READ:
            break;
        case PHDC_GETDATA:
            App_USBD_PHDC_SendPHDCData(
                                        pUSBCB->u32_Count,
                                        (APP_PHDC_DATA_TYPE)pUSBCB->u32_Data
                                      );
            break;

        case PHDC_SETDATA:
            break;
    }
}

void AppDeviceConnected(void)
{
    /* Post Semaphore */
    //OSSemPost(pConnectSemaphore);
}

void AppDeviceDisconnected(void)
{
    //OS_ERR err;
    /* Post Semaphore */
    //OSSemPost(pConnectSemaphore);
}

#endif


