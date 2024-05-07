
TEST PLAN
This is basic Beeper driver functional test.  It plays a playlist of tone
frequency and duration pairs.  This example uses the beeper "sequence" mode,
not the "single-tone" mode.  This is done so that as each tone completes, the
controller does not auto-disable (as it will after each tone completes in
single-tone mode).  Single-tone play mode is not a problem and works fine, it
is just that it introduces a noticeable pause between tones as a new single-tone
is programmed to play... one after the other.  The sequence play mode is used to
minimize the gap between tones because the pre-programmed A/B tones are
automatically switched without each channel auto-disabling and needing to
re-enabled for each new tone.

The actual play tones are driven on tone channel A (ToneA), while the tone B
channel (ToneB) is only used to keep the controller active as it auto-switches
between ToneA/ToneB paired plays.  In this case, we preload ToneB only once with
a short-lived (1) duration silence (rest) tone frequency.  As each tone A/B pair
is played sequentially, the target tone is heard over the ToneA channel and a very
short intervening silence is heard between ToneA plays on the ToneB channel as the
sequencer switches back and forth between the two.

As each ToneA tone start interrupt fires, we reprogram the next target tone into
ToneA register (leaving the ToneB channel alone, which never changes), while the
current ToneA value (from the previous load) is still playing.  Each ToneA start
interrupt lets us program the next tone in the sequence until all tones have been
played.  After the last tone is loaded and no more tones are to be loaded, we stop
processing interrupts and wait in the main loop for last tone to complete by
monitoring the beeper BUSY bit to clear before exiting.

OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".


PIN MULTIPLEXING JAVA PLUGIN
The beep example employs the Java pinmux plugin. listed under the IAR IDE "Tools" menu.
This plugin generated a single pin multiplex API that configures all pin muxing at once.
The output file is in the same directory as the beeper example code and is named
"beep_pinmux.c".


HARDWARE SETUP
This example uses the piezoelectric transducer to play tones on the "ADuCM350 Audio
Board" daughter board (Rev-A with jumper LK4 set to position "A"), plugged into the
"Digital Header 2" connector on the main ""EVAL-ADUCM350EBZ" evaluation board
(Rev.0/Rev.B).
