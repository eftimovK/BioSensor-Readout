# Project Information

## Brief Description
As part of the [SensUs competition](https://www.sensus.org/), the goal is to develop a readout system for the biosensor of team AixSense at the IWE1 institute, RWTH Aachen. 

This repository contains therefore all necessary resources and code required for the readout system, using the [ADuCM350 device](https://www.analog.com/en/products/aducm350.html). 

## Quick Setup within the IAR environment
Before compiling and debugging, open the 'Options' window of the current project and check the following settings:

- **_General Options --> Target_** to choose the ADuCM350 device
- **_C/C++ Compiler --> Preprocessor_** to add the path of _core_cm3.h_, if not found when compiling
- **_J-Link/J-Trace --> Setup_** and set the Reset to "Halt after bootloader"

Currently questionable setting:
- **_Debugger --> Download_** to uncheck _"Verify download"_

For more info, check chapter 3.2 in the Software User's Guide.

## Important Links

- Comprehensive forum for ADuCM350: https://ez.analog.com/analog-microcontrollers/precision-microcontrollers/w/documents/2432/aducm350-faq
- AFE daughter board (Eval-ADuCM350 BIO3Z): https://ez.analog.com/analog-microcontrollers/precision-microcontrollers/w/documents/2424/bioimpedance-measurement-using-aducm350