This test exercises the on-chip flash memories of the ADuCM350, via the adi_FEE
device driver.

This is an extended test, running 10 tests on each of three seperate flash controllers.
Total test time runs about 10-15 minutes.

The basic sequence of each test iteration is:
- erase the parts of the flash region which are not 0xFF
- check that the whole region is 0xFF
- write a test pattern to a psuedo-randomly chosen section on the flash region
- verify that the chosen section contains the test pattern, and that the sections
  before and after it still contain 0xFF
- check that a forward signature check fails
- write the correct signature to flash
- check that a forward signature check passes
- check that the hardware-computed signature matches a software-computed version
- repeat the above 4 steps for reverse signature
- check that the forward and reverse signatures are different
- for flash controller 1, and the GP flash controller, mass-erase the flash region
- for Flash controller 0, selectively page-erase the flash region (so as not to
  erase the test program code)

Interrupts may also be set up to trigger aborts during flash operation, to verify that
the driver handles the aborts correctly.

For flash controller 0, parity checking is also briefly turned on, without correct parity
data being in place, to verify that the parity error interrupt occurs.

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
