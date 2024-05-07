/*****************************************************************************
 * RTOS_example.c
 *****************************************************************************/

#include "system.h"
#include <ucos_ii.h>
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "spi.h"

#include "SpiMasterSlave.h"



/* RTOS required data structures for thread creation */
char g_ThreadSPIMasterStack[ThreadSPIMaster_STK_SIZE];
OS_TCB g_ThreadSPIMaster_TCB;

char g_ThreadSPISlaveStack[ThreadSPISlave_STK_SIZE];
OS_TCB g_ThreadSPISlave_TCB;


int32_t adi_initpinmux(void);

int main(void)
{
  INT8U    OSRetVal;
  
  // system init first...
  SystemInit();
  /* test system initialization */
  test_Init();
  /* increase processor speed */
  SetSystemClockDivider(ADI_SYS_CLOCK_CORE, 1);
  SystemCoreClockUpdate();
  adi_initpinmux();
  
  OSInit();
  
  OSRetVal = OSTaskCreate (ThreadSPIMasterRun,
                           NULL,
                           (void *)(g_ThreadSPIMasterStack + ThreadSPIMaster_STK_SIZE),
                           ThreadSPIMaster_PRIO);
  
  if (OSRetVal != OS_ERR_NONE)
  {
    printf("Error creating ThreadSPIMaster\n");
    exit(1);
  }
  
  OSRetVal = OSTaskCreate (ThreadSPISlaveRun,
                           NULL,
                           (void *)(g_ThreadSPISlaveStack + ThreadSPISlave_STK_SIZE),
                           ThreadSPISlave_PRIO);
  
  if (OSRetVal != OS_ERR_NONE)
  {
    printf("Error creating ThreadSPISlave\n");
    exit(1);
  }
  
  
  OSStart();
  
  printf("Error starting the RTOS\n");
  return 0;
}

