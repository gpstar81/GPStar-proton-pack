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

The following libraries are required to be installed. All but the MillisDelay library can be found within the Arduino Library Manager with the app. Go to `Sketch -> Include Library -> Manage Libraries...` to access the Library Manager. Search for the libraries by name and install the latest version available.

- **FastLED** by Daniel Garcia
- **ezButton** by ArduinoGetStarted.com
- **Ramp** by Sylvain Garnavault
- **AltSoftSerial** by Paul Stoffregen
- **simple ht16k33 library** by lpaseen
- **SerialTransfer** by PowerBroker2
- **millisDelay** `See Below`

### MillisDelay

The MillisDelay library must be downloaded from the project GitHub page. Download the code as a zip use the `Sketch -> Add .ZIP Library` option to import the downloaded file.
[https://github.com/ansonhe97/millisDelay](https://github.com/ansonhe97/millisDelay)

No further configuration is needed for this library.

## +++ IMPORTANT WHEN FLASHING UPDATES +++
If you are flashing updates to your existing setup, make sure that both your Proton Pack and Neutrona Wand Micro SD Cards have all the latest sound effects from this repository.

## Connection to your gpstar Proton Pack and Neutrona Wand PCB
Use the included FTDI to USB programming cable that comes with the gpstar kits or use any other suitable FTDI 5V basic serial connector. The UART Pins on the PCB should align with with the standard wire order for FTDI-to-USB cables which use a single Dupont 6-pin connector. Observe these common colors and notes to ensure proper orientation:

- The ground pin will typically be a black wire, while VCC will typically be red.
- The DTR pin on the PCB will connect to a wire labelled either DTR or RTS.
- Any wire labelled CTS will be connected to the 2nd pin labelled GND on the PCB.
- Be careful to not reverse the connector!

![UART Connection](images/uart_pack.jpg)

## Flashing The Software
Once your have connected your gpstar Board to your computer via the FTDI to USB cable, you will need to select it from your devices in the Arduino IDE and search for Mega.

![Board Selection](images/flash-gpstar-1.png)
![Board Selection Mega](images/flash-gpstar-2.png)

Next click on the UPLOAD button (arrow pointing to the right) and wait for it to complete.
![Board Selection Mega](images/flash-gpstar-3.png)

[Firmware Flashing Instructional Video](https://www.youtube.com/watch?v=J-P8rl3Hzck) (YouTube)
[![Firmware Flashing Instructional Video](https://img.youtube.com/vi/J-P8rl3Hzck/maxresdefault.jpg)](https://www.youtube.com/watch?v=J-P8rl3Hzck)

## Advanced User Configurations

To modify defaults within the software, please see the [Advanced Configuration](ADVCONFIG.md) guide.
