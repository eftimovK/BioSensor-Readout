
TEST PLAN
This is a basic LED blink test.  It blinks the DISPLAY and DISPLAY1 LEDs on the
Eval-ADUCM350EBZ board in a continuous binary count pattern.  The test runs in
an infinite loop and does not exit.  Any failures are reported to the output
device, typically STDIO, which can be captured via SWD/semi-hosting by the IDE's
terminal console.  A debugger is required to be attached to capture such output.

The example uses the ADI GPIO System Service to configure the pin electronics
and drive the LEDs.


PIN MULTIPLEXING JAVA PLUGIN
There is no pin multiplexing to manage for this example; it just uses generic
GPIO pin mapping, which is active by default.  Hence, there is no pinmux.c file
and no need to run the Pinmux Java Plugin.


HARDWARE SETUP
This example uses the built-in LED on the main ""EVAL-ADUCM350EBZ" evaluation
board (Rev.B).  There are no external hardware requirements.


RUNNING THE EXAMPLE
Open, build, download and run the example.  No user interaction is required
beyond that.


OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".
