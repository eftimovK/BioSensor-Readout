
/*!
 *****************************************************************************
   @file:    command_ID.h
   @brief:   Command IDs (8-bit !) for the command set between the GUI and microcontroller. 
   @date:    $Date: 2024-27-06 $
*****************************************************************************/

#ifndef __COMMAND_ID__
#define __COMMAND_ID__

/* Set codes for commands that are received through UART (e.g. by a GUI)              */
#define CMD_START   1  /*!< measurement start */
#define CMD_STOP    0
#define CMD_START_CONFIG  5
#define CMD_STOP_CONFIG   6
#define CMD_ABORT  -1

/* Pameters for configuration */
#define PARAMETERS_VOLTAGE     100  /*!< Set voltage level */
#define PARAMETERS_DUR         101  /*!< Set duration      */

#endif /* __COMMAND_ID__ */
