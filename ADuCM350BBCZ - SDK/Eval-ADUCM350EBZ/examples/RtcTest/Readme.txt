
This is basic RTC test.


There is no special hardware setup needed to run the example code in the ASIC.

Use the "ADI_RTC_RESET" macro to force the RTC clock to be reset to the
latest build timestamp.

Use the "ADI_RTC_CALIBRATE" macro to enable calibration mode, in which the
low frequency crystal used by the RTC is brought out for measurement by an
external frequency counter.  This allows computation of crystal error with
which RTC trim settings may be selected.

The main testing is twofold: a short hibernation test followed by an extended
hibernation test.

The short test sets an RTC ALARM five seconds hence and then puts the part
into hibernation mode.  The ALARM wakeup reports success to the test console
and the program proceeds to the extended test.

Use the "ADI_RTC_EXTENDED_TEST" macro to enable RTC extended test.
The extended test puts the core into hibernation state and stays there until
the pushbutton is pressed (P0.10).  This allows monitoring of hibernation
battery current and verification of RTC clock accuracy over time, while in
low power standby mode.  The core will stay in hibernation until the button
is pressed... even for weeks.  Make sure the battery is installed for extended
testing.

USE THE P0.10 PUSHBUTTON TO BRING THE TEST OUT OF EXTENDED HIBERNATION MODE.

WARNING: While the processor is in hibernation, do not press the target "reset"
button.  The initial version of ADuCM350 silicon will go into a "high current"
lockout mode rendering the board unresponsive.  If this happens, the board may
be recovered by connecting an emulator, pulling power, and then -- while holding
the "Serial Download" pushbutton -- reapply power.  That should bring the
processor out of lockout mode.

Five build-time preprocessor macros are available to configure the build:

ADI_RTC_RESET
This macro forces the part to reset the RTC internal clock, using the latest
build timestamp as a reference.

ADI_RTC_USE_IAR_SYSTEM_TIME_HOOKS
This macro controls inclusion of ADI-provided IAR runtime library override
functions that "hook" the RTC into the standard C library <time.h> library,
allowing use of C programming standard time(), ctime(), clock(), etc., functions.
The <time.h> interface hooking is done within the RTC device driver and is
demonstrated in the RTC example code.

ADI_RTC_CALIBRATE
This macro supports calibration code in the example.  The RTC external crystal
is multiplexed out onto external pin P1.7 for measurement.  The code illustrates
selection and assertion of suitable RTC trim values.

ADI_RTC_EXTENDED_TEST
This macro enables RTC extended test.

OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".
