
This is test that compares LFXTAL with HFOSC, using general purpose timers.

GPT0 is programmed with LFXTAL as source. GPT1 is programmed with PCLK as source,
where PCLK has HFOSC as source (with a divider of 1). GPT0 events are captured
using GPT1 and captured values stored in an array.

There is no special hardware setup needed to run the example code in the ASIC.

OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".
