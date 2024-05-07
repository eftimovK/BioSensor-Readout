/*****************************************************************************
 * RTOS_example.c
 *****************************************************************************/

#include "system.h"
#include <ucos_ii.h>
#include <stdio.h>
#include <stdlib.h>

#define THREAD1_STK_SIZE 200
#define THREAD1_PRIO 5
#define NUM_MSG 10

char g_Thread1Stack[THREAD1_STK_SIZE];
OS_TCB g_Thread1_TCB;

OS_EVENT *g_Sem1;
int MsgQArea[NUM_MSG];
OS_EVENT *g_MsgQ1;
void Thread1Run(void* arg)
{
	INT8U err;

	g_Sem1 = OSSemCreate(0u);
	while (1)
	{
		printf("In thread1\n");
	    OSSemPend(g_Sem1,
	              0,
	              &err);
	    if (err != OS_ERR_NONE)
	    {
	    	printf("Error pending on semaphore\n");
	    	exit(1);
	    }
	}

}

#define THREAD2_STK_SIZE 200
#define THREAD2_PRIO 6

char g_Thread2Stack[THREAD2_STK_SIZE];
OS_TCB g_Thread2_TCB;

void Thread2Run(void* arg)
{
	INT8U err;
	char *received_message;
	while (1)
	{
		printf("In thread2\n");
		received_message = OSQPend(g_MsgQ1,
	                     0,
	                     &err);
	    if (err != OS_ERR_NONE)
	    {
	    	printf("Error pending on queue\n");
	    	exit(1);
	    }
	    else
	    {
	    	printf("The message received was %s\n",received_message);
	    }

	    err = OSSemPost(g_Sem1);
	    if (err != OS_ERR_NONE)
	    {
	    	printf("Error posting semaphore\n");
	    	exit(1);
	    }

	}

}

#define THREAD3_STK_SIZE 200
#define THREAD3_PRIO 7

char g_Thread3Stack[THREAD2_STK_SIZE];
OS_TCB g_Thread3_TCB;
void Thread3Run(void* arg)
{
	INT8U err;
	int counter=0;
	char * my_str= malloc(10);

	while (1)
	{
		sprintf(my_str,"str%d",counter);
		counter++;
	    err = OSQPost(g_MsgQ1,
	    		my_str);

		printf("In thread3\n");

	    if (err != OS_ERR_NONE)
	    {
	    	printf("Error posting message\n");
	    	exit(1);
	    }
	}

}

int main(void)
{
  INT8U    OSRetVal;
  
  // system init first...
  SystemInit();
  
	OSInit();

      OSRetVal = OSTaskCreate (Thread1Run,
                NULL,
               (void *)(g_Thread1Stack + THREAD1_STK_SIZE),
               THREAD1_PRIO);
    
     if (OSRetVal != OS_ERR_NONE)
    {
    	printf("Error creating thread1\n");
    	exit(1);
    }

    OSRetVal = OSTaskCreate (Thread2Run,
                NULL,
               (void *)(g_Thread2Stack + THREAD2_STK_SIZE),
               THREAD2_PRIO);

    if (OSRetVal != OS_ERR_NONE)
    {
    	printf("Error creating thread2\n");
    	exit(1);
    }

    OSRetVal = OSTaskCreate (Thread3Run,
                NULL,
               (void *)(g_Thread3Stack + THREAD3_STK_SIZE),
               THREAD3_PRIO);
    
    if (OSRetVal != OS_ERR_NONE)
    {
    	printf("Error creating thread3\n");
    	exit(1);
    }
    g_MsgQ1 = OSQCreate((void**)&MsgQArea, 10 );
 
    OSStart();

    printf("Error starting the RTOS\n");
    return 0;
}
