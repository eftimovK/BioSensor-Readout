Installation of CDC windows driver
===================================

There is one INF file (usbser.inf) associated with one PID (0x7824).

You can use dpinst32.exe/dpinst64.exe to pre-install the WinUSB driver on 32-bit and 64-bit 
architectures respectively. Either open a command window and navigate to this directory and 
use (e.g.)


.\dpinst64.exe 

and follow the insructions. You can also just double-click on the appropriate dpinst file 
from Windows Explorer.

IMPORTANT NOTE:

	Windows 7 does not use the "Found New Hardware" Wizard and will fail to install 
	the driver if dpinst is not used beforehand. In that instance you have to open the
	Device Manager, locate the offending item, right-click and select 
	"Update Driver Software ...".
	
	This behavior is consistent with other USB products, where the manufacturers state
	"do not connect your device until you have run the setup.exe". 
	
	On Windows XP, the "Found New Hardware" Wizard will display and you can Browse 
	to this directory. If you use dpinst, you should simpley select the Search and 
	Install option.

You can also clean out all driver instances based on the usbser.inf by using 

.\dpinst64.exe -u usbser.inf

from a command line. 