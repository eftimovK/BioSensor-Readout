ADuCM350 USB PHDC Demo Firmware and Host application
====================================================

Description
-----------

This project provides an implementation of the USB PHDC class.

There are two modes of operation: 'basic' and 'multiple' Quality of Service (QoS).
For each mode there is a firmware build and host application pair. Both
host applications use the same WinUSB driver installation. Please refer to the
"Windows' Driver Installation" section below for details of its installation.

The default template version of lib_cfg.h (from Micrium) is overridden
by providing a customized app_cfg.h and switching the initial include
define at the top of lib_cfg.h to include it.
        
Build Instructions
------------------

Which mode is built is determined by the APP_CFG_USBD_PHDC_BASIC macro defined at
line 48 of .\app_cfg.h:

#define APP_CFG_USBD_PHDC_BASIC                 DEF_ENABLED

Set this line to

#define APP_CFG_USBD_PHDC_BASIC                 DEF_DISABLED

for multiple QoS mode.

The default configuration is for the 'basic' mode of operation.
Once project is opened, simply build, load and run. Connect the standard A end of your
USB cable to the host PC and the micro-B end to the EZ-BOARD micro-B connector.

There is also some test code defined in app_usbd_phdc_basic.c to demonnstrate that association requests
timeout properly. This is disabled by default, but can be enabled by defining the
__TEST_ASSOC_RQST_TIMEOUT__ macro in the project options. For correct operation with the host application,
this code should be disabled.

When enabled, an error message will be output to the "Terminal I/O" window.

There are 2 build configurations provided:
1) Debug          : Debug aware; with OSAL; IVT relocated
2) Release        : Optimized; with OSAL; IVT relocated

Please note that the above confugurations have the following properties:

1) "Defined symbols":
        RELOCATE_IVT

   Please further note that the Interrupt vector table (IVT) MUST be relocated from ROM in order
   for the Interrupt manager to install replacement interrupt handlers.

2) Library:
        $PROJ_DIR$\..\..\..\osal\uCOS-II\Lib\iar\Debug\Exe\libosal.a

The following table is comprised of the "Grand Total" lines from the c.map files for each configuration:

    Configuration                    ro code  ro data  rw data  rw data
                                                                 (abs)
    ------                           -------  -------  -------  -------
    Debug                             51 084   14 896   14 261      520
    Release                           36 796   12 940   14 270      520


Windows' Driver Installation
----------------------------

Before connecting the firmware application to a host PC, you need to ensure that the device
driver is installed. The WinUSB driver is pre-installed upon installation of the ADuCM350 Device Drivers
package. For reference, the driver is located in the .\host\driver directory along with installer
executables, dpinst32.exe and dpinst64.exe.

On connecting the firmware application for the first time, a System Tray bubble should appear
on your Windows 7 host PC informing you that it is installing the driver. No user intervention
is required. Eventually, it should report that it has successfully installed the driver.
If it is not successful:
 - Go to the device manager and under other device, PHDC device.
 - Right click on it select Update Driver Software.
 - Select the Browse my computer for driver software. Browse to the path \examples\uCOS-II-USBD-PHDC\host
 - Click on Next. Driver Installation will start.

These actions may also occur on subsequent connection to a different USB port on the host PC.


Host Applications
-----------------

Windows' executables are provided for both the 'basic' and 'multiple' modes of operation.


Basic (Single QoS) mode
-----------------------

The 'basic' mode of operation uses the app_usbd_phdc_basic.c file and implements a single
High Latency/Reliable Quality of Service transfer to transfer either the firmware details
or a set of pseudo temperature values.

The host application for this mode can be found under $PROJ_DIR\..\host\basic and can be run
from the command line as follows;

.\phdc.exe -[v|d]

where
	-v	displays the firmware information
	-d  displays the pseudo temperature data


Multiple QoS mode
-----------------

When configured to build in 'multiple' QoS mode the project uses the app_usbd_phdc_multiple.c file
and implements a Low Latency/Good QoS transfer to transfer the CPU percentage usage statistic to
the host applicaiton, .\host\multiple\phdc.exe.

This configuration behaves exactly as described in the Micrium documentation for the
uc/USB Device (TM) Class PHDC product:

	$_MICRIUM_DIR_$\uC-USB-Device-V4\Doc\uC-USB-Device-V4 User Manual.pdf

Important Note: APP_ITEM_DATA_OPAQUE_LEN_MAX defines the length of the Opaque data.
An assertion will be seen if the string size is more than that.
Please refer to the Micrium documentation mentioned above for further details.

Upto 5 QoS transfers can be run simultaneously.
