This is a simple test that uses the ADuCM350 SPI interface to test a SPI-Flash device.

The test performs various read/write operations over the SPIH device to verify
that the SPI-Flash device is accessable and working correctly.

The test requires a the ADuCM350 Audio Board expansion board and verifies connectivity
and function of the Microchip SST25LF020S 2-MBit SPI Flach device on that board.


OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".


TEST SETUP:
This example uses the SPI-Flash device on the "ADuCM350 Audio Board" daughter
board (Rev-A with jumper LK4 set to position "A"), plugged into the "Digital
Header 2" connector on the main ""EVAL-ADUCM350EBZ" evaluation board (Rev.B).

