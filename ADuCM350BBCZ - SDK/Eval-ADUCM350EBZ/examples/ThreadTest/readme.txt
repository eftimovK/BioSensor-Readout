##########################################################
#
# Readme.txt  - ThreadTest Example
#
##########################################################

Description:
-----------

	This example demonstrates a simple threading test that spins up three
	threads that post/pend on events to eachother in a continuous manner.

	Print output is directed via semi-hosting to the debugger Terminal I/O
	window.  The build configuration requires the "Semihosted" low-level
	library interface option to be set, as well as catching "stdout/stderr"
	"Via semihosting".  This is done under the "General Options" "Library
	Configuration" project settings dialogue.


Running the test:
----------------

	1. Build, download and run the test on the ADuCM350 evaluation board.
	2. This test is designed to run forever; it does not exit by itself.


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
