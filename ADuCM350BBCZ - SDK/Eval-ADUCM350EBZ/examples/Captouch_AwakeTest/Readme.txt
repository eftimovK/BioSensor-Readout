
This is an example use of the CapTouch driver which configures the ADuCM350 to wake up 
the core from CORE_SLEEP and SYS_SLEEP power modes when a CapTouch button is touched. 

One single button is configured, the Up Button connected to the input B.

The Cortex starts in CORE_SLEEP Mode and when the button is touched the first time
it wakes up and goes to sleep again to SYS_SLEEP mode until a new touch event.

Once the Cortex wakes up from SYS_SLEEP mode, the test finishes.

OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".

HARDWARE REQUIREMENTS
This example requires the Eval-ADUCM350EBZ (Rev.B) motherboard and the "ADuCM350 
Display/CapTouch Board" (Rev-B) daughter board (connected to both "DIGITAL HEADER 1" 
(J6) and "CAPTOUCH HEADER" (J21) connectors on the motherboard). The example will 
build without the daughter board, but will not run successfully.

THIS EXAMPLE BUILDS WITHOPUT THE DISPLAY ADAPTER BOARD, BUT IT DOES NOT RUN SUCCESSFULLY.


