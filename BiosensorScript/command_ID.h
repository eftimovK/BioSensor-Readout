
/*!
 *****************************************************************************
   @file:    command_ID.h
   @brief:   Command IDs (8-bit !) for the command set between the GUI and microcontroller. 
   @date:    $Date: 2024-27-06 $
*****************************************************************************/

#ifndef __COMMAND_ID__
#define __COMMAND_ID__

/* Set codes for commands that are received through UART (e.g. by a GUI)              */
#define CMD_START   1   /*!< measurement start */
#define CMD_STOP    0   /*!< measurement stop */
#define CMD_START_CONFIG  5
#define CMD_STOP_CONFIG   6
#define CMD_ABORT  -1

/* Pameters for configuration */
#define PARAM_VOLTAGE_STEP     100  /*!< Set voltage level for const excitation */
#define PARAM_VOLTAGE1_CV      101  /*!< Set voltage level 1 for CV excitation */
#define PARAM_VOLTAGE2_CV      102  /*!< Set voltage level 2 for CV excitation */

#endif /* __COMMAND_ID__ */
