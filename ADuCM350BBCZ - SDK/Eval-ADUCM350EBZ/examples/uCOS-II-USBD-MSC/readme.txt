ADuCM350 USB Mass Storage Class Device Firmware Demo
====================================================

Description
-----------

This project exports a small RAMDisk on the USB MSC interface, which can be accessed
by a host PC that supports the standard USB MSC class.

The default template version of lib_cfg.h (from Micrium) is overridden
by providing a customized app_cfg.h and switching the initial include
define at the top of lib_cfg.h to include it.
        
Build Instructions
------------------

Before this project can be opened in IAR tools, please ensure that the Windows System Environment
variable MICRIUM_DIR is defined and points the the location where your Micrium sources are
based. Please note this variable will point to the Micrium/Software directory within
the Micrium installation.

Once opened, simply build, load and run. Connect the standard A end of your USB cable to the host PC and
the micro-B end to the EZ-BOARD micro-B connector. Once the USB lead is connected to the host PC,
you will see a new drive letter under "Devices with Removable Storage" in Windows' Explorer. It will
also be listed in the Device Manager as "USB Mass Storage Device".

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
    Debug                             51 618   15 098   15 481   11 192
    Release                           36 596   12 753   10 765   11 192


NOTES
-----
The RAMdisk is very small and does not support reformatting; this will fail.
The RAMdisk supports small file write/read/modify, etc.
