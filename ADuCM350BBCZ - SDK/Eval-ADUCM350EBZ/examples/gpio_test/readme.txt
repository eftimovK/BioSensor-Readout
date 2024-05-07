This test verifies GPIO device driver operation.  Two GPIO pins are driven as
outputs and two pins are read back as inputs.  The test relies on two jumpers
to be placed externally across the output and input pin pairs.

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

This test relies on lookback jumpers across the following pins...

on the EVAL-ADuCRF101EBZ:
	Short pins 7 to 11 on J12
	Short pins 6 to 10 on J12

on the EVAL-ADuCRF101MKxZ:
	Short pins P1.2 to P0.3
	Short pins P1.3 to P0.4

on the EVAL-ADuCM320:
	Short pins P0.6 to P0.5
	Short pins P0.4 to P0.3

on the Eval-ADUCM350EBZ:
This test requires use of the "ADuCM350 Breakout Board, Rev-0",
connected to main evaluation board "DIGITAL HEADER 2" (J1) to make
the pins accessable.  Install the breakout board and place the
following external jumpers across these pins on breakout board:
    P4.0 & P4.2 (pins J3-30 & J3-34 of ADuCM350 Breakout Board)
