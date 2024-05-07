
This is an example use of the CapTouch driver which configures 5 CapTouch Buttons.

The Up, Down, Left, Right and Enter Buttons on the ADuCM350 Display/CapTouch Board
are configured to generate touch and release interrupts.

Up    --> Connected to Input B

Left  --> Connected to Input C

Right --> Connected to Input E

Down  --> Connected to Input F

Enter --> Connected to Input D


To detect a touch event with this configuration, the user must hold the button for 
90ms (this value can be modified).

Anytime a touch or release interrupt fires, the system checks the button status using a
CapTouch driver API; adi_CT_GetTchAndRelAlgorithmStatus.

The system sends an STDIO message each time a button is touched or released, indicating 
which button generated the event. 

To finish the test the user must touch three buttons simultaneously. 

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

NOTE: On the Eval-ADUCM350EBZ (rev.B) motherboard, P0.2 and P0.4 are connected to push-
button switches, with capacitors for debounce. This will prevent the Left and Right 
CapTouch button presses and releases from being detected. To allow these to be detected,
remove 0r resistors R63 and R65 from the motherboard.



