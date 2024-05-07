This test verifies that the watchdog timer is working correctly.  The test uses
timer 0 to capture the length of time the watchdog takes to trigger and then
compares it with the programmed timeout value in the watchdog register.

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
