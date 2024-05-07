##########################################################
#
# Readme.txt  - ThreadTest Example
#
##########################################################

Description:
-----------

	This example demonstrates the use of the SPI APIs in both the slave
        and Master modes of operation. The test is an RTOS based test with two
        threads. A SPI Master thread and a SPI Slave thread.
        
        Note: the slave side must always be started first so that master side
        transactions are recognized. To accomplish this the SPI SLave thread
        is assigned a higher priority.

        The SPI_DMA define may be used to run the test in either DMA or 
        interrupt modes.
        
         The MASTER_SLAVE_WITH_PROLOGUE define controlls whether the test 
         employs transaction block prologue data or not.   
         
         If MASTER_SLAVE_WITH_PROLOGUE is defined, prologue data is used
         and test behaivor is:
	    a. Master sends a two byte prologue plus 1 guard byte.
	    b. Slave waits to receive the two byte prologue then
		responds by sending a data transaction back to the       
   		master.                                                  
	    c. Master receives the slave data and compares it with
		the expected data to issue a PASS/FAIL test result.  
                                                            
          If MASTER_SLAVE_WITH_PROLOGUE is undefined, prologue data is not
          used and test behaivor is:
	    a. Master sends no data and just clocks the slave.          
	    b. Slave sends a data transaction back to the master.       
	    c. Master receives the slave data and compares it with
		the expected data to issue a PASS/FAIL test result.

        In all cases, it is the MASTER SIDE that reports PASS/FAIL test result.


Running the test:
----------------

	1. Build, download and run the test on the ADuCM350 evaluation board.
	2. This test is designed to run forever; it does not exit by itself.


OUTPUT:
------
Test output is printed to STDIO, which is reported to the debugger console via the
semi-hosting facility (see the "Capturing STDIO from Examples" section of the
"ADuCM350BBCZ Software Users Guide").  Note: semi-hosting is enabled by default in
most examples.  Typically, each example prints "PASS!" or "FAIL: <filename,
linenumber, message>" messages, depending on success or failure of the example run.
Some tests provide additional performance data or message reports during the test
run.  Performance messages are formatted as "PERF: <message>".


TEST SETUP:
----------
Expansion Board Cable Strappings
  Attach an ADuCM350 Breakout Board to Digital Header 2 with LK1 in "B" position.
  
  Master Side Connections                 Slave Side Connections
  SPI1:MOSI [P3.6]/[J3.16]  ------------> SPI0:MOSI [P3.2]/[J3.12]            
  SPI1:MISO [P3.5]/[J3.24]  <------------ SPI0:MISO [P3.1]/[J3.28]            
  SPI1:CLK  [P3.4]/[J3.14]  ------------> SPI0:CLK  [P3.0]/[J3.30]            
  SPI1:SSEL [P3.7]/[J3.22]  ------------> SPI0:SSEL [P3.3]/[J3.26]            

