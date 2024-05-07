
This is an example for using the Analog Generate CUSTOM_INTERRUPT to trigger an
interrupt from a sequence.

The AFE sequencer on the ADuCM350 can be used to perform cycle accurate measurements
using the 16-bit AFE. Sometimes, it may be necessary to trigger another peripheral
(e.g. GPIO pin, SPI, I2C etc.) as part of a measurement. The AFE sequence does not
have control of other peripherals, but it can use the Analog Generate interrupts
to wake-up/activate the core to perform a task in parallel with the sequence which
is being executed by the sequencer.

In this simple example, the sequence implements:
 - A short wait, simulating hardware setup/settling times
 - Triggers a CUSTOM_INTERRUPT
 - A longer wait, simulating a measurement
 - Triggers a CUSTOM_INTERRUPT

When the interrupt is triggered, the default interrupt handler for the Analog 
Generate interrupts calls the Custom Interrupt callback function, which turns on
an LED (DISPLAY) on the Eval-ADUCM350EBZ motherboard on the first interrupt and
extinguishes the LED on the second interrupt. 

OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".
