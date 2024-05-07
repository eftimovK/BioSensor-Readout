This test verifies GPIO device driver operation.  The test includes:
	- negative testing of device driver API return codes,
	- driver APIs set various GPIO control registers correctly,
	- pin interrupt configurations,
	- indivigual pin programming,
	- multiplexing,
	- output enables,
	- pin state,
	- pin toggles,
	- open circuit enable,
	- output enables,
	- pull-up enables.

The test validates GPIO driver support and operation of both interrupt and
polling modes.

OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".


TEST SETUP:

ADuCM350EBZ target board:
  Install jumper LK14

