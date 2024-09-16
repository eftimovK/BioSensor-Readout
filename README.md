# Project Information

## Brief Description
As part of the [SensUs competition](https://www.sensus.org/), the goal is to develop a readout system for the biosensor of team AixSense at the IWE1 institute, RWTH Aachen. 

This repository and the corresponding GUI should contain (apart from the IAR Embedded Workbench IDE) all necessary resources and code required for the readout system, using the [ADuCM350 device](https://www.analog.com/en/products/aducm350.html). 

## Run & Debug
During the project, an emulator board attached to the [evaluation board](https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/eval-aducm350.html#eb-overview) (which includes the microcontroller) was used for connecting to a PC via USB.

**The USB driver for J-Link is necessary in order to successfully communicate with the board.**
Usually the driver should be installed together with the SDK, but can be downloaded separately according to the *'Getting Started Guide'*-manual found in *'ADuCM350BBCZ - SDK\Eval-ADUCM350EBZ\doc'*

If the firmware is already on the microcontroller, connect PC to the emulator board via USB and run the GUI. 
Otherwise, beforing attempting to connect within the GUI for any measurements, open the **BiosensorScript** project within the IAR environment, then build, and download the firmware. \
**For this, make sure the SDK is installed.** If the files in this repository do not work, obtain the SDK installer from the [ADuCM350 website](https://www.analog.com/en/lp/001/aducm350-design-resources.html). 

## Processing measurement data
After a successfull measurement using the GUI, the data is saved in a text file. For example, for an EIS measurement, the measured impedances are saved row-wise in the format:	

    real_part	imag_part	frequency

Data processing isn't currently supported within the GUI (only displaying). Thus, a MATLAB script can be used for post-analysis; see ***'readEISmeasurement.m'***.

## Setup within the IAR environment
The IAR project files for **BiosensorScript** should include the configuration. However, if a manual project configuration is necessary, follow the steps below. 
Open the 'Options' window of the current project and check the following settings:

- **_General Options --> Target_** to choose the ADuCM350 device
- **_C/C++ Compiler --> Preprocessor_** to add the path of _core_cm3.h_, if not found when compiling
- **_J-Link/J-Trace --> Setup_** and set the Reset to "Halt after bootloader"

Currently questionable setting:
- **_Debugger --> Download_** to uncheck _"Verify download"_

For more info, check chapter 3.2 in the Software User's Guide.

## Important Links

- Comprehensive forum for ADuCM350: https://ez.analog.com/analog-microcontrollers/precision-microcontrollers/w/documents/2432/aducm350-faq
- AFE daughter board (Eval-ADuCM350 BIO3Z): https://ez.analog.com/analog-microcontrollers/precision-microcontrollers/w/documents/2424/bioimpedance-measurement-using-aducm350