This test verifies GPT device driver operation.  It tests the General
Purpose Timer APIs and functionality, including:
	- Multiple timers
	- Event Capture
	- Timer set/get load value
	- Count up/down
	- Interrupt capture
	- Clock pre-scale and clock source selectability
	- Periodic and free-running modes

Once the test has finished, it sends a result message string to the UART/Terminal 
I/O output; "PASS" for success and "FAIL" (plus failure message) for failures.

!!! NOTE: THIS TEST TAKES A MINUTE OR SO TO COMPLETE !!!

OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".


TEST SETUP:

This test has no external hardware requirements.
