This is a simple test that runs data through the ADuCRF101/ADuCM320 I2C interface.

The test uses an internal "Loopback" connection, and verifies data integrity
as it is passed through the I2C controller.  It tests all four combinations of
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

I2C cannot run at either of the standard I2C serial bit rates (100kHz or the
400kHz) with only a 1MHz input clock; this test requires the 16MHz core clock.
Alternatively, the 1MHz core clock can sustain up to ~45kHz I2C serial bit rate.

This test uses the internal on-chip I2C loopback mode
and dedicated test API, adi_I2C_MasterSlaveLoopback(),
to validate basic driver functionality.  This API is
normally not included in production builds.  It is enabled
by setting the "ADI_I2C_LOOPBACK" preprocessor define so that
all files get it.  The adi_I2C_MasterSlaveLoopback() API
run the I2C controller in both Master and Slave mode
simultaneously.

GPIO is also brought out to the I2C alternate pin muxing (port4) for
obvservation over the "Digital Header 2" expansion interface.

No any external loopback connections are required.
