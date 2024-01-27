# IDE Setup

The custom software needed for operation must be compiled and uploaded to your devices. This guide provides the list of required libraries for compilation and outlines the changes needed to support each board.

## Supported Devices

Please refer to the table below for a list of devices and their supported software release.

| Controller Device | v1.x | v2.x | v3.x |
|-------------------|------|------|------|
| <img src='images/gpstar_logo.png' width=20 align="left"/> GPStar Proton Pack PCB   | Yes | Yes | Yes |
| <img src='images/gpstar_logo.png' width=20 align="left"/> GPStar Neutrona Wand PCB | Yes | Yes | Yes |
| <sup>d1</sup> DIY Arduino Mega Proton Pack   | Yes | Yes <sup>2</sup> | Yes <sup>3</sup> |
| <sup>d1</sup> DIY Arduino Nano Neutrona Wand | Yes | Yes <sup>2</sup> | No |

<sup>d1</sup> These are now considered as "legacy" devices and have distinct end-of-life notes in later versions.

<sup>2</sup> Both devices are supported as of the last release at v2.2.0

<sup>3</sup> If paired with an Arduino Nano for the Neutrona Wand, this arrangement is deprecated. To continue using the DIY Arduino Mega you will need to upgrade the Neutrona Wand to use the GPStar Neutrona Wand PCB.

## Prerequisites

Download and install the Arduino IDE 2.x or higher. This will be used to compile and upload the code to your Proton Pack and Neutrona Wand.

[Arduino IDE 2.x](https://www.arduino.cc/en/software)

The following libraries are required to be installed. All can be found within the Arduino Library Manager with the app. Go to `Sketch -> Include Library -> Manage Libraries...` to access the Library Manager. Search for the libraries by name and install the latest version available.

- **ezButton** by ArduinoGetStarted.com
- **FastLED** by Daniel Garcia
- **Ramp** by Sylvain Garnavault
- **SafeString** by Matthew Ford
- **SerialTransfer** by PowerBroker2

## +++ IMPORTANT WHEN FLASHING UPDATES +++

If you are flashing updates to your existing setup, make sure that both your Proton Pack and Neutrona Wand Micro SD Cards have all the latest sound effects from this repository.

## Connection to your GPStar Proton Pack and Neutrona Wand PCB

Use the included FTDI to USB programming cable that comes with the GPStar kits or use any other suitable FTDI 5V basic serial connector. The UART Pins on the PCB should align with with the standard wire order for FTDI-to-USB cables which use a single Dupont 6-pin connector. Observe these common colours and notes to ensure proper orientation:

- The ground pin will typically be a black wire, while VCC will typically be red.
- The DTR pin on the PCB will connect to a wire labelled either DTR or RTS.
- Any wire labelled CTS will be connected to the 2nd pin labelled GND on the PCB.
- Be careful to not reverse the connector!

![UART Connection](images/uart_pack.jpg)

## Compiling and flashing the software

Once your have connected your GPStar board to your computer via the FTDI to USB cable, you will need to select it from your devices in the Arduino IDE and search for Mega.

![Board Selection](images/flash-gpstar-1.png)
![Board Selection Mega](images/flash-gpstar-2.png)

Next click on the UPLOAD button (arrow pointing to the right) and wait for it to complete.
![Board Selection Mega](images/flash-gpstar-3.png)

[Firmware Flashing Instructional Video](https://www.youtube.com/watch?v=J-P8rl3Hzck) (YouTube)
[![Firmware Flashing Instructional Video](https://img.youtube.com/vi/J-P8rl3Hzck/maxresdefault.jpg)](https://www.youtube.com/watch?v=J-P8rl3Hzck)

## Advanced User Configurations

To modify defaults within the software, please see the [Advanced Configuration](ADVCONFIG.md) guide.
