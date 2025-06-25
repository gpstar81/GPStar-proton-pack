# Compiling & Flashing via IDE Setup

The custom software needed for operation must be compiled and uploaded to your devices which can be accomplished using an interactive development environment (IDE). This guide provides the list of required libraries for compilation and outlines the changes needed to support each board.

## Supported Devices

Please refer to the table below for a list of devices and their supported software release.

| Controller Device | v1.x | v2.x | v3.x | v4.x | v5.x | v6.x |
|-------------------|------|------|------|------|------|------|
| <img src="./images/gpstar_logo.png" width="20"/> GPStar Proton Pack PCB   | Yes | Yes | Yes | Yes | Yes | Yes |
| <img src="./images/gpstar_logo.png" width="20"/> GPStar Neutrona Wand PCB | Yes | Yes | Yes | Yes | Yes | Yes |
| <sup>d1</sup> DIY Arduino Mega Proton Pack   | Yes | Yes <sup>2</sup> | Yes <sup>3</sup> | Yes <sup>3</sup> | Yes <sup>3</sup> | Yes <sup>3</sup> |
| <sup>d1</sup> DIY Arduino Mega Neutrona Wand | Yes | Yes <sup>2</sup> | Yes <sup>2</sup> | Yes <sup>2</sup> | Yes <sup>2</sup> | Yes <sup>2</sup> |

<sup>d1</sup> These are now considered as "legacy" devices and have distinct end-of-life notes in later versions (see additional superscript notations).

<sup>2</sup> Support for the Arduino Nano as a wand controller ended after the release of v2.2.0. That device must be replaced with a [Mega 2560 Pro Mini](https://www.amazon.com/s?k=Mega+2560+PRO+MINI) to support the later software releases.

<sup>3</sup> If paired with an Arduino Nano for the Neutrona Wand, this arrangement is deprecated. To continue using the DIY Arduino Mega you will need to upgrade the Neutrona Wand to use the GPStar Neutrona Wand PCB or a [Mega 2560 Pro Mini](https://www.amazon.com/s?k=Mega+2560+PRO+MINI).

## Prerequisites

Please see our guide on installing VSCode with PlatformIO. This will be used to compile and upload the code to your Proton Pack and Neutrona Wand.

[VSCode + PlatformIO](VSCODE.md)

## +++ IMPORTANT WHEN FLASHING UPDATES +++

If you are flashing updates to your existing setup, make sure that both your Proton Pack and Neutrona Wand Micro SD Cards have all the latest sound effects from this repository.

## Connection to your GPStar Proton Pack and Neutrona Wand PCB

Use the included FTDI to USB programming cable that comes with your GPStar kit or use any other suitable FTDI 5V basic serial connector. The UART Pins on the PCB should align with with the standard wire order for FTDI-to-USB cables which use a single Dupont 6-pin connector. Observe these common colours and notes to ensure proper orientation:

- The ground pin will typically be a black wire, while VCC will typically be red.
- The DTR pin on the PCB will connect to a wire labelled either DTR or RTS.
- Any wire labelled CTS will be connected to the 2nd pin labelled GND on the PCB.
- Be careful to not reverse the connector!

![UART Connection](images/uart_pack.jpg)

## Compiling and Flashing the Software

TO BE ADDED

[Firmware Flashing Instructional Video](https://www.youtube.com/watch?v=J-P8rl3Hzck) (YouTube)
[![Firmware Flashing Instructional Video](https://img.youtube.com/vi/J-P8rl3Hzck/maxresdefault.jpg)](https://www.youtube.com/watch?v=J-P8rl3Hzck)

## Advanced User Configurations

To modify defaults within the software, please see the [Advanced Configuration](ADVCONFIG.md) guide.
