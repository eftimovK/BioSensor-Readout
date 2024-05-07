This is a "stress" test that runs the WDT, GPT, and GPIO device drivers.
It conencts GPIO output pins setup as flags to GPIO input pins setup as
interrupt sources.  It also uses timers to capture and verify expected
timing as interrupts arrive.

The test requires hardware "Loopback" connections, and verifies pin
connectivity between output flags and interrupt inputs through the GPIO
ports.  Two timers and external interrupts are used, as well as the
watchdog timer (internal) interrupt, to generate and verfy expected
interrupt counts.  The WDT is configured to generate a timer interrupt
rather than the default timeout action of resetting the processor.

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

On EVAL-ADuCRF101MKxZ target board:
	P1.4 & P0.6
	P1.5 & P0.7
    
On EVAL-ADuM360EBZ target board:
	P1.2 & P0.3
	P1.3 & P0.5

on the Eval-ADUCM350EBZ:
This test requires use of the "ADuCM350B Breakout Board, Rev-0",
connected to main evaluation board "DIGITAL HEADER 1" (J6) to make
the pins accessable.  Install the breakout board and place the
following external jumpers across these pins on breakout board:
    P1.0 & P1.2 (pins J2-14 & J2-18 of ADuCM350 Breakout Board)
    P1.1 & P1.3 (pins J2-16 & J2-20 of ADuCM350 Breakout Board)
