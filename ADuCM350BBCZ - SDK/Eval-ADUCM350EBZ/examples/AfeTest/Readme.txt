
This is a simple AFE test that runs through the steps outlined in the chapter
"AFE Example Use Cases" in the Harware Reference Manual (HRM, UG-587).

Currently the test calls each function provided with the AFE example code library.
This is for illustration purpose.

There is no special hardware setup needed to run the example code in the ASIC.

OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".
