
This is a simple system clocks test that runs through the steps outlined
in section "Example Use Cases" of chapter "System Clocks" in the HRM.

The use cases currently handled are:
- change clock source to HFXTAL using polling
- change clock source to HFXTAL using interrupts
- change clock source to SPLL using polling
- change clock source to SPLL using interrupts

There is no special hardware setup needed to run the example code in the ASIC.

OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".
