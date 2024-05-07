##########################################################
#
# Readme.txt  - LCDTest example
#
##########################################################

Description:
-----------

	This example demonstrates how to use the LCD driver APIs to display 
    characters on the Varitronix VIM-828 8 x 14-Segment LCD display. 
    The test demonstrates the following:    
    1. Displaying the uppercase alphabet on display segments.
    2. Displaying numbers on display segments.
    3. Varying the LCD display contrast.
    4. Blinking the LCD between On and Off state using both hardware and software
       controlled mechanism.
       
Running the test:
----------------

	1. Build, download the test to an ADuCM350 target board.
    2. Follow the Hadware Setup section to connect the LCD display.
	3. Run the program.
	4. Observe the LCD for display of various characters.

Hardware Setup:
---------------
    - Connect the ADuCM350 Display Board (Rev. B) or ADuCM350 GPIO Test Header 
      Board (Rev-B) to "DIGITAL HEADER 1" (J6) and "CAP TOUCH" header (J21) 
      connectors on the Eval-ADUCM350EBZ motherboard (Rev.B).
    - Ensure that LK14 is open.

OUTPUT
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".
