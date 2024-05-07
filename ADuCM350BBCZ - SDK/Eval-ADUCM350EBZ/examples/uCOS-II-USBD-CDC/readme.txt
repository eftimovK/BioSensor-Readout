ADuCM350 USB Communication Device Class (CDC) Firmware Demo
====================================================

Description
-----------

This project demonstrates an echo operation where a user can attach a PC based 
terminal application to the Virtual Com port. What the user types will be echoed 
back by the device.

The default template version of lib_cfg.h (from Micrium) is overridden
by providing a customized app_cfg.h and switching the initial include
define at the top of lib_cfg.h to include it.


Build Instructions
------------------

Before this project can be opened in IAR tools, please ensure that the Windows 
System Environment variable MICRIUM_DIR is defined and points the location where 
your Micrium sources are based. Please note this variable must point to the 
Micrium/Software directory within the Micrium installation.

Once opened, simply build, download to the ADuCM350 device and run. Connect the 
standard A end of the micro USB cable to the host PC and the micro-B end to the 
Eval-ADUCM350EBZ micro-B connector (J4). Once the USB lead is connected to the 
host PC, you will see a new COM port named as "ADI/Micrium USB Virtual COM Device 
(CDC/ACM) (COMX)" under "Ports (COM & LPT)" in the Device Manager, where X is the 
assigned port number.

There are 2 build configurations provided:
1) Debug          : Debug aware; with OSAL; IVT relocated
2) Release        : Optimized; with OSAL; IVT relocated

Please note that the above configurations have the following properties:

1) "Defined symbols":
        RELOCATE_IVT

   Please further note that the Interrupt Vector Table (IVT) MUST be relocated 
   from ROM in order for the Interrupt manager to install replacement interrupt 
   handlers.

2) Library:
        $PROJ_DIR$\..\..\..\osal\uCOS-II\Lib\iar\Debug\Exe\libosal.a

The following table is comprised of the "Grand Total" lines from the c.map files 
for each configuration:

    Configuration                    ro code  ro data  rw data  rw data
                                                                 (abs)
    ------                           -------  -------  -------  -------
    Debug                             49 088   14 884   16 030   952
    Release                           37 111   12 749   11 314   952


Windows' Driver Installation
----------------------------

Before connecting the firmware application to a host PC, the user must ensure that 
the device driver is installed. The Windows driver is pre-installed upon installation 
of the ADuCM350 Device Drivers package. For reference, the driver is located in the 
..\host\driver directory along with installer executables, dpinst32.exe and dpinst64.exe.

On connecting the firmware application for the first time, a System Tray bubble 
should appear on the Windows 7 host PC informing that it is installing the driver. 
No user intervention is required. Eventually, it should report that it has 
successfully installed the driver.
If it is not successful:
 - Go to the device manager and under other device, CDC device.
 - Right click on it select Update Driver Software.
 - Select the Browse my computer for driver software. Browse to the 
   path \examples\uCOS-II-USBD-CDC\host
 - Click on Next. Driver Installation will start.

These actions may also occur on subsequent connection to a different USB port on 
the host PC.


Using the CDC Example
----------------------

The example is exactly the same as described in the µC/USB Device Documentation.
The user will need to use a PC terminal application on Windows to use the CDC 
ACM example. The Micriµm documentation discusses the PC based terminal application 
options in detail, but good results can be obtained using the TeraTerm or PuTTY
terminal emulator.

In the PC based terminal application, serial port settings must be as below -

 - Baud Rate: 9600
 - data: 8bit
 - parity: None
 - Stop: 1 bit
 - Flow Control: None

Run the CDC demo example on the ADuCM350 device and open the terminal application 
with the COM port shown by ADI/Micrium CDC Device (in Device Manager).
You will see following menu-


USB CDC ACM Serial Emulation Demo
---------------------------------

1. Echo 1 demo.
2. Echo N demo.
Option:

The first option enables character-by-character echo, whereas the second option 
can receive data up to 512 bytes. 

CTRL-C will take the user back to the main menu.


Connectivity Issues with Virtual COM port on PC
-----------------------------------------------
There is a known Windows issue that affects most applications. The virtual COM
port will be removed from the device manager when the USB cable is unplugged,
even if an application has the virtual COM port open.  The virtual COM port will
not be accessible even after the virtual serial device is reconnected unless the
virtual COM port is released by the application prior to the reconnection, which
may require closing the application first.  It is therefore recommended that the
application be closed before the USB device is unplugged.  This issue is also
apparent if the target is reset.
