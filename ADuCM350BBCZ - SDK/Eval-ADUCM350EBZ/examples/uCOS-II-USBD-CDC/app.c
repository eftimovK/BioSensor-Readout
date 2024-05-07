/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
*                                       ADI ADuCM350 Cortex M3 
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>
#include <usbd_core.h>
#include <startup.h>

#include "test_common.h"

/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/


/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_STK   App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE];

#if (LIB_MEM_CFG_HEAP_BASE_ADDR != 0u)
SECTION_PLACE(KEEP_VAR( CPU_INT08U __MemoryPool[LIB_MEM_CFG_HEAP_SIZE]), LIB_MEM_CFG_HEAP_BASE_ADDR);
#endif

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  App_TaskCreate(void);
static  void  App_ObjCreate (void);

static  void  App_TaskStart (void  *p_arg);

/*
*********************************************************************************************************
*                                         EXTERNALS
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : Startup Code.
*
* Note(s)     : none.
*********************************************************************************************************
*/
int  main (void)
{
#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_ERR     cpu_err;
#endif
#if (OS_TASK_NAME_EN > 0u)
    CPU_INT08U  os_err;
#endif

    SystemInit();                                               /* Initialize the system software                      */
	
    test_Init();                                                /* test system initialization                          */

    Mem_Init();                                                 /* Initialize the uC/LIB Memory pool see LIB_MEM_CFG_HEAP_SIZE in app_cfg.h */

    CPU_Init();                                                 /* Initialize the uC/CPU services                       */

#if (CPU_CFG_NAME_EN == DEF_ENABLED)
    CPU_NameSet("AduCM350", &cpu_err);
#endif

    OSInit();                                                   /* Init uC/OS-II.                                       */

    OSTaskCreateExt(                   App_TaskStart,           /* Create the start task                                */
                    (void           *) 0,
                                      &App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE - 1],
                                       APP_CFG_TASK_START_PRIO,
                                       APP_CFG_TASK_START_PRIO,
                                      &App_TaskStartStk[0],
                                       APP_CFG_TASK_START_STK_SIZE,
                    (void           *) 0,
                                      OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);

#if (OS_TASK_NAME_EN > 0u)
    OSTaskNameSet(APP_CFG_TASK_START_PRIO, "Start", &os_err);
#endif


    OSStart();                                                  /* Start multitasking (i.e. give control to uC/OS-II).  */
}

/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  App_TaskStart (void *p_arg)
{
    CPU_INT32U  cnts;
    CPU_INT32U  cpu_clk_freq;
    (void)p_arg;

    /* Determine SysTick reference freq */
    cpu_clk_freq = SystemGetClockFrequency(ADI_SYS_CLOCK_CORE);

    /* Determine nbr SysTick increments */
    cnts = cpu_clk_freq / (CPU_INT32U)OS_TICKS_PER_SEC;

    SysTick_Config (cnts);                                      /* Start system tick */

#if (OS_TASK_STAT_EN > 0u)
    OSStatInit();                                               /* Determine CPU capacity                               */
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

#if defined(ADI_SYSTEM_CLOCK_TRANSITION)
    /* Transition clocks to power up the USB controller
    */
    SystemTransitionClocks(ADI_SYS_CLOCK_TRIGGER_USB_ON);
#endif
                                                                /* --------------- MODULES INITIALIZATION ------------- */
    /* Initialize USB MSC device
     */
    App_USBD_Init();

    /* The following are currently not used, but are included as they form part of
     * the Micrium application framework template
    */
    App_ObjCreate();                                            /* Create Applicaiton kernel objects                    */
    App_TaskCreate();                                           /* Create Application tasks                             */

    /* delete task as it is no longer required
    */
    OSTaskDel(APP_CFG_TASK_START_PRIO);

}

/*
*********************************************************************************************************
*                                          AppTaskCreate()
*
* Description : Create application tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_TaskCreate (void)
{

}

/*
*********************************************************************************************************
*                                          App_ObjCreate()
*
* Description : Create application kernel objects tasks.
*
* Argument(s) : none
*
* Return(s)   : none
*
* Caller(s)   : AppTaskStart()
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  App_ObjCreate (void)
{

}
