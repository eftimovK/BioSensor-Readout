
This is an example for a unipolar current measurement.

After the initialization and calibration steps, a DC voltage is applied across 
the load (AFE4-AFE5) and measurements are performed. After a specified time, the 
DC voltage level is changed. The outputs of the LPF are then returned to the UART.

The example doesn't use floating-point, all the arithmetic is performed using
fixed-point.

The following parameters are programmable through macros:
 - switch matrix configuration
 - DC level 1 (mV)
 - DC level 2 (mV)
 - Duration of DC level 1 (us)
 - Duration of DC level 2 (us)
 - IVS switch closed time (DC level 1) (us)
 - IVS switch closed time (DC level 2) (us)
 - Switching current shunting (required/not required)
Note: there are no checks in the code that the values are within admissible ranges,
which needs to be ensured by the user.
Macros which are preceeded by a "DO NOT EDIT" comment should not be changed.

When using the Eval-ADuCM350EBZ board, the test needs a daughter board attached
to the evaluation board, with the relevant impedances (AFE4-AFE5) populated. It also
needs a level-shifting resistor (RL).

Once the test has finished, it sends a result message string to STDIO;
"PASS" for success and "FAIL" (plus failure message) for failures.

For Eval-ADUCM350EBZ boards, the results are returned to the PC/host via the 
UART-on-USB interface on the USB-SWD/UART-EMUZ (Rev.C) board to a listening 
PC based terminal application tuned to the corresponding virtual serial port. 
To ensure that data is returned using the UART, set the macro USE_UART_FOR_DATA = 1. 
Failure to do so will result in a failure, as the standard output is too slow to 
return ADC/LPF data. See the ADuCM350 Device Drivers Getting Started Guide for
information on drivers and configuring the PC based terminal application.
