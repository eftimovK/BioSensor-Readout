
This is an example use of the CapTouch driver to be used for driver testing. The
CapTouch registers (MMR) are configured and if the configuration is successful, the
test passes.

OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".

HARDWARE REQUIREMENTS
This example requires the Eval-ADUCM350EBZ (Rev.B) motherboard and the "ADuCM350 
Display/CapTouch Board" (Rev-B) daughter board (connected to both "DIGITAL HEADER 1" 
(J6) and "CAPTOUCH HEADER" (J21) connectors on the motherboard). The example will 
build without the daughter board, but will not run successfully.