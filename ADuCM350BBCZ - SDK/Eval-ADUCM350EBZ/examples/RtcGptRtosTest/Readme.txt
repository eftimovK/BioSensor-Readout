##########################################################
#
# Readme.txt  - RtcGptRtosTest example
#
##########################################################

Description:
-----------
	This example demonstrates a simple threading test that spins up three
	threads running in an RTOS context using the RTC and GPT drivers.
        
        Interrupts will be handled via the OSAL second level dispatcher as the
        adi_global_config values are overriden. Specifically,
        
            ADI_CFG_ENABLE_RTOS_SUPPORT = 1



        
Running the test:
----------------

    1. Build, download the test to an ADuCM350 target board.
    2. Run the program.

OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".

Hardware Setup:
---------------

    No hardware setup is required.
