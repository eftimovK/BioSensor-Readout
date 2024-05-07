
This is an example for an ADC measurement(s) using an auxilliary channel.

After the initialization and calibration steps, ADC measurements are taken for a
specified time on a specified auxilliary channel. The outputs of the LPF are 
then returned to the UART.

Macros are provided to facilitate the setting of the measurement duration and the 
desired Aux Channel. Macros which are preceeded by a "DO NOT EDIT" comment should 
not be changed.

Once the test has finished, it sends a result message string to STDIO;
"PASS" for success and "FAIL" (plus failure message) for failures.

For Eval-ADUCM350EBZ boards, the results are returned to the PC/host via the 
UART-on-USB interface on the USB-SWD/UART-EMUZ (Rev.C) board to a listening 
PC based terminal application tuned to the corresponding virtual serial port. 
To ensure that data is returned using the UART, set the macro USE_UART_FOR_DATA = 1. 
Failure to do so will result in a failure, as the standard output is too slow to 
return ADC/LPF data. See the ADuCM350 Device Drivers Getting Started Guide for
information on drivers and configuring the PC based terminal application.
