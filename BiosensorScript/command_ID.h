
/*!
 *****************************************************************************
   @file:    command_ID.h
   @brief:   Command IDs (8-bit !) for the command set between the GUI and microcontroller. 
   @date:    $Date: 2024-27-06 $
*****************************************************************************/

#ifndef __COMMAND_ID__
#define __COMMAND_ID__

/* Set codes for commands that are received through UART (e.g. sent by a GUI)   */
#define CMD_START_CONST   1   /*!< start measurement with const voltage excitation                  */
#define CMD_START_CV      2   /*!< start measurement with cyclic voltammetry (CV) excitation        */
#define CMD_START_EIS     3   /*!< start measurement with AC excitation for impedance spectroscopy  */
#define CMD_STOP          0   /*!< stop measurement */
#define CMD_START_CONFIG  5
#define CMD_STOP_CONFIG   6
#define CMD_ABORT  -1

/* Pameters for configuration */
#define PARAM_VOLTAGE_STEP     100  /*!< Set voltage level for const excitation */
#define PARAM_VOLTAGE1_CV      101  /*!< Set voltage level 1 for CV excitation */
#define PARAM_VOLTAGE2_CV      102  /*!< Set voltage level 2 for CV excitation */
#define PARAM_SLOPE_CV         103  /*!< Set slope duration  for CV excitation */

#endif /* __COMMAND_ID__ */
