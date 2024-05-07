This is the first test to try if having problems making things work.  No
other device drivers are needed to run this test.

The test verifies the most basic system functionality by using only the
intrinsic NVIC APIs to test basic core system operation.  The test programs
the SysTick interrupt interval and increments a counter in the SysTick
Interrupt handler to verify the expected number of interrupts.

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
