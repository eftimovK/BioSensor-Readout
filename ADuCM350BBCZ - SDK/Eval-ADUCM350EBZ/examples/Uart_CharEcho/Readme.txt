
This example uses the UART driver to do character echo onto a PC based terminal
application.

USAGE
1) Determine the PC COM port number (see test setup).
2) Open a terminal application on the PC (PuTTY, TeraTerm, ...)
3) Configure the terminal for 9600-8-N-1
4) Download and run the example.
5) Type characters on the PC terminal.
6) The example receives the characters and transmits the characters back to the 
   terminal.
7) Press the return key to stop the program.

For Eval-ADUCM350EBZ boards, the UART echo is sent to / recevied from the PC/host 
via the UART-on-USB interface on the USB-SWD/UART-EMUZ (Rev.C) board to a listening 
PC based terminal application tuned to the corresponding virtual serial port. 
See the ADuCM350 Device Drivers Getting Started Guide for information on drivers 
and configuring the PC based terminal application.

Once the test has finished, it sends a result message string to STDIO;
"PASS" for success and "FAIL" (plus failure message) for failures.
