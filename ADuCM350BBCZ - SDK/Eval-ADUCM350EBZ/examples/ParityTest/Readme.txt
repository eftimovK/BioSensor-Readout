This test verifies that signatures and parity data are being correctly
added to the test executable by the ADI-modified IElfTool utility.

A detailed description of the CRC and parity features is provided in the Software User's Guide,
sections 4.2 - 4.4.

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
