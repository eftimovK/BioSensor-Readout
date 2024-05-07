This test verifies that the wakeup timer is working correctly.  The test uses
timer 0 to capture the length of time the wakeup timer takes to generate an
interrupt, we can then compare the captured time with that programmed in the
wakeup timer register to verify that the wakeup timer is working correctly.

OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".

NOTE:
This test contains an override of the factory default WUT configuration file,
adi_wut_config.h, normally found in the .../inc/config directory.
This override enables static configuration and sets initialization constants
differently than the facotry defaults for the purposes of this test case.


TEST SETUP:
This test has no external hardware requirements.
