/***********************************************************************
 * Copyright(c) 2006 Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary and confidential.  By using this software
 * you agree to the terms of the associated Analog Devices License Agreement.
 *
 * Description:
 * Provides definitions for host and device for use with the hostapp.
 *
 ***********************************************************************/
#ifndef _HOSTAPP_H_
#define _HOSTAPP_H_

/*********************************************************************
 * defines for host and device
 *********************************************************************/

enum _USB_COMMAND
{
	NO_COMMAND,				/* nothing doing here... */
	GET_FW_VERSION,			/* get the firmware version */
	QUERY_SUPPORT,			/* query for support */
	QUERY_REPLY,			/* query reply */

	LOOPBACK,				/* run loopback on the device */

	MEMORY_READ,			/* read from specified memory on the device */
	MEMORY_WRITE,			/* write to specified memory on the device */

	USBIO_START,			/* run USB IO on this device */
	USBIO_STOP,				/* stop USB IO on this device */
	USBIO_OPEN,				/* open file on host */
	USBIO_CLOSE,			/* close file on host */
	USBIO_READ,				/* read file on host */
	USBIO_READ_REPLY,		/* read reply from host */
	USBIO_WRITE,			/* write file on host */
	USBIO_WRITE_REPLY,		/* write reply from host */
	USBIO_SEEK_CUR,			/* seek from current position of file on host */
	USBIO_SEEK_END,			/* seek from end of file on host */
	USBIO_SEEK_SET,			/* seek from beginning of file on host */
	USBIO_SEEK_REPLY,		/* seek reply from host */
	USBIO_FILEPTR,			/* sending file pointer */

    PHDC_GETDATA,           /* Retrieve PHDC data */
    PHDC_SETDATA,           /* Transmit PHDC data */

	CUSTOM_COMMAND,			/* custom command */

    REPEAT_OUT = 100,
    REPEAT_IN
};


enum _VERSION_STRINGS		/* version string info */
{
	FW_BUILD_DATE,			/* build date of firmware */
	FW_BUILD_TIME,			/* build time of firmware */
	FW_VERSION_NUMBER,		/* version number of firmware */
	FW_TARGET_PROC,			/* target processor of firmware */
	FW_APPLICATION_NAME,	/* application name of firmware */

	NUM_VERSION_STRINGS		/* number of version strings */
};

typedef struct _USBCB		/* USB command block */
{
  unsigned int u32_Command;		/* command to execute */
  unsigned int u32_Data;		/* generic data field */
  unsigned int u32_Count;		/* number of bytes to transfer */
} USBCB, *PUSBCB;

typedef struct app_phdc_data_row {
    unsigned long Date;
    unsigned long Time;
    unsigned long Data;
} APP_PHDC_DATA_ROW;

typedef enum app_phdc_data_type {
    PHDC_TEMPERATURE,
    PHDC_HEART_RATE,
    PHDC_BLOOD_PRESSURE,
    PHDC_BLOOD_GLUCOSE,
    PHDC_COMMAND
} APP_PHDC_DATA_TYPE;

#define APP_PHDC_MAX_UNITS_LEN 20

typedef struct app_phdc_opaque_def {
    unsigned long       DataLen;
    unsigned long       DataType;
    char                DataUnits[APP_PHDC_MAX_UNITS_LEN];
} APP_PHDC_OPAQUE_DEF;

#define	MAX_VERSION_STRING_LEN		32
#define VERSION_STRING_BLOCK_SIZE	(NUM_VERSION_STRINGS*MAX_VERSION_STRING_LEN)

#define LOOPBACK_HEADER_BYTES		4						/* bytes in header of loopback data */
#define MAX_DATA_BYTES_BULK	        (64*1024)			    /* max bytes to send */
#define MIN_DATA_BYTES_BULK 	    LOOPBACK_HEADER_BYTES	/* min bytes to send */

#define APP_PHDC_MAX_DATA_ROWS      10

#define FILE_OPEN_MODE_OFFSET		0						/* byte offset for mode for file open */
#define FILE_OPEN_FILENAME_OFFSET	4						/* byte offset for filename for file open */

#define USBIO_STDIN_FD 	            0		/* file descriptor for stdin on Device */
#define USBIO_STDOUT_FD             1		/* file descriptor for stdout on Device */
#define USBIO_STDERR_FD             2		/* file descriptor for stderr on Device */

/*********************************************************************
 * defines for host only
 *********************************************************************/

#ifdef _HOSTAPP_

enum _ERROR_VALUES			/* error values */
{
	OPERATION_PASSED = 0,
	UNSUPPORTED_COMMAND,
	IO_WRITE_USBCB_FAILED,
	IO_READ_USBCB_FAILED,
	IO_READ_DATA_FAILED,
	IO_WRITE_DATA_FAILED,
	OUT_OF_MEMORY_ON_HOST,
	ERROR_OPENING_FILE,
	ERROR_READING_FILE,
	NO_AVAILABLE_FILE_PTRS,
	COULD_NOT_CONNECT,
};

#endif	/* _HOSTAPP_ */

/*
*********************************************************************************************************
*                                        App_GetDataLengthFromOpaqueData()
*
* Description : Returns the value of the data length of the main data phase as encoded in the opaque
*               data.
*
* Argument(s) : none.
*
* Returns     : The data length
*               0, On Failure.
*
* Caller(s)   : App_Rd().
*
* Note(s)     : This will be specific to the protocol in use, and will need to be changed accordingly.
*
*********************************************************************************************************
*/

static unsigned long App_USBD_PHDC_GetDataLengthFromOpaqueData( unsigned char *pOpaqueData )
{
    return ((unsigned long*)pOpaqueData)[0];
}
/*
*********************************************************************************************************
*                                        App_SetDataLengthFromOpaqueData()
*
* Description : Sets the value of the data length of the main data phase in the opaque data.
*
* Argument(s) : none.
*
* Returns     : 0 on success
*               -1, On Failure.
*
* Caller(s)   : App_Rd().
*
* Note(s)     : This will be specific to the protocol in use, and will need to be changed accordingly.
*
*********************************************************************************************************
*/

static int App_USBD_PHDC_SetDataLengthFromOpaqueData( unsigned char *pOpaqueData, unsigned long DataLen )
{
    ((unsigned long*)pOpaqueData)[0] = DataLen;
    return 0;
}
/*********************************************************************
 * defines for device only
 *********************************************************************/

#ifndef _HOSTAPP_


#endif /* ! _HOSTAPP_ */


#endif /* _HOSTAPP_H_ */

