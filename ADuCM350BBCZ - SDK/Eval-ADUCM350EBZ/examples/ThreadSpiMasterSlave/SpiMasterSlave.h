/*********************************************************************************

Copyright (c) 2011-2014 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.  By using 
this software you agree to the terms of the associated Analog Devices Software 
License Agreement.

*********************************************************************************/


/** define size of data buffers */
#define BUFFERSIZE 250u
/* master prologue includes an additional dummy byte,
/  that allows the slave to process the received incoming command
/  and prepare for responding tranmission
 */
#define MASTERPROLOGUESIZE 3u
/* slave prologue does not include dummy byte */
#define SLAVEPROLOGUESIZE  2u

#define PROLOGUE1 0x17
#define PROLOGUE2 0x71
#define DUMMY     0xFF


   /* The Slave has to have higher priority than the master to allow it to */
   /* setup and submit a buffer to recieve data into                       */
   
#define ThreadSPIMaster_STK_SIZE 200
#define ThreadSPIMaster_PRIO 7


#define ThreadSPISlave_STK_SIZE 200
#define ThreadSPISlave_PRIO 6

extern    ADI_SPI_DEV_HANDLE                hDeviceM, 
                                            hDeviceS;


extern void ThreadSPIMasterRun(void* arg);
extern void ThreadSPISlaveRun(void* arg);



