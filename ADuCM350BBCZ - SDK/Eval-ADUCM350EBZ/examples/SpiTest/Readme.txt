This is a simple test that runs data through the ADuCRF101 SPI interface.

The test requires a hardware "Loopback" connection, and verifies data integrity
as it is passed through the SPI controller.  It tests all four combinations of
transfer modes: blocking vs. non-blocking vs. DMA vs. non-DMA.

OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".


TEST SETUP:

This test relies on lookback jumpers across the following (MISO/MOSI) pins...

On EVAL-ADuCRF101MKxZ target board:
	J14:2 & J14:4
On EVAL-ADuCRF101MKxZ/Rev A target board:
	P0.0 & P0.2
On EVAL-ADuM360EBZ target board:
	P0.0 & P0.2
On ADuCM320 EVAL  target board:
	P1.5 & P1.6

On ADuCM350EBZ target board:
	(SPI0) P3.1 & P3.2, i.e., J3:12 & J3:28 of breakout board on digital header 1 DH1
	(SPI1) P3.5 & P3.6, i.e., J3:24 & J3:16 of breakout board on digital header 1 DH1
