# IDE Setup

The custom software needed for operation must be compiled and uploaded to your devices. This guide provides the list of required libraries for compilation and outlines the changes needed to support each board.

## Supported Devices
###Proton Pack:###
 
- <img src='images/gpstar_logo.png' width=30 align="left" /> gpstar Proton Pack PCB

###Neutrona Wand:###
- <img src='images/gpstar_logo.png' width=30 align="left" /> gpstar Neutrona Wand PCB

## Prerequisites

Download and install the Arduino IDE 2.x or higher. This will be used to compile and upload the code to your Proton Pack and Neutrona Wand.

[Arduino IDE 2.x](https://www.arduino.cc/en/software)

The following libraries are required to be installed. All but the WavTrigger and MillisDelay library can be found within the Arduino Library Manager with the app. Go to `Sketch -> Include Library -> Manage Libraries...` to access the Library Manager. Search for the libraries by name and install the latest version available.

- **FastLED** by Daniel Garcia
- **ezButton** by ArduinoGetStarted.com
- **Ramp** by Sylvain Garnavault
- **AltSoftSerial** by Paul Stoffregen
- **simple ht16k33 library** by lpaseen
- **SerialTransfer** by PowerBroker2
- **millisDelay** `See Below`
- **WavTrigger** `See Below`

### MillisDelay

The MillisDelay library must be downloaded from the project GitHub page. Download the code as a zip use the `Sketch -> Add .ZIP Library` option to import the downloaded file.
[https://github.com/ansonhe97/millisDelay](https://github.com/ansonhe97/millisDelay)

No further configuration is needed for this library.

### WavTrigger

The WavTrigger library must be downloaded from the project GitHub page. Download the code as a zip use the `Sketch -> Add .ZIP Library` option to import the downloaded file.
[https://github.com/robertsonics/WAV-Trigger-Arduino-Serial-Library](https://github.com/robertsonics/WAV-Trigger-Arduino-Serial-Library)

## +++ IMPORTANT +++
Documented on both the Proton Pack and Neutron Wand code at the top of each file, is instructions on modifying the wavTrigger.h file. **You will need to open the wavTrigger.h file and comment out the neccessary #define required for the Proton Pack and Neutrona Wand.**

**(see For compiling code on the gpstar Proton Pack and gpstar Neutrona Wand below for more information)**. 

The **wavTrigger.h** file can be located in your `Arduino/Libraries/<wav trigger folder>` (MacOS) or `C:\Arduino\Libraries` (Windows). See the special notes below for compiling the code for the gpstar Proton Pack and Neutrona Wand boards.

## For compiling code for the gpstar Proton Pack PCB and gpstar Neutrona Wand PCB:

You need to enable `__WT_USE_SERIAL_3__` by uncommenting this line inside the **wavTrigger.h** file. Be sure to comment out any other serial class option previously in use.

![WavTrigger Serial Class for gpstar PCB boards](images/wt_serial_mega.png)

[Firmware Flashing Instructional Video](https://www.youtube.com/watch?v=J-P8rl3Hzck) (YouTube)
[![Firmware Flashing Instructional Video](https://img.youtube.com/vi/J-P8rl3Hzck/maxresdefault.jpg)](https://www.youtube.com/watch?v=J-P8rl3Hzck)

## Advanced User Configurations

To modify defaults within the software, please see the [Advanced Configuration](ADVCONFIG.md) guide.
