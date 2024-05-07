This example code demonstrates the SPI device driver running under RTOS.
It performs an external SPI data loopback across the MOSI/MISO data lines.
The example creates two tasks and one semaphore, the first task initiates an 
SPI transceiver transaction, the second task monitors for the condition when
the SPI transaction has completed, after which it passes a semaphore back to 
the first task when the process can start again.

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

This test depends on the OSAL library, which must be built as a prerequisite.
Please go to the OSAL "uCOS-II\Lib directory and build the library.  In turn,
the OSAL library build depends on the third-party Micrium sources being present
(see the release notes on using the Micrium RTOS).

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
	P3.5 & P3.6, i.e., J3:24 & J3:16 of breakout board on digital header 1 DH1
