# Compiling & Flashing via IDE Setup (DIY)

The custom software needed for operation must be compiled and uploaded to your devices. This guide provides the list of required libraries for compilation and outlines the changes needed to support each board.

**Note:** This is a slightly modified version of the standard [COMPILING_FLASHING](COMPILING_FLASHING.md) guide.

## Supported Devices

Please refer to the table below for a list of devices and their supported software release.

| Controller Device | v1.x | v2.x | v3.x | v4.x | v5.x |
|-------------------|------|------|------|------|------|
| <img src='images/gpstar_logo.png' width=20 align="left"/> GPStar Proton Pack PCB   | Yes | Yes | Yes | Yes | Yes |
| <img src='images/gpstar_logo.png' width=20 align="left"/> GPStar Neutrona Wand PCB | Yes | Yes | Yes | Yes | Yes |
| <sup>d1</sup> DIY Arduino Mega Proton Pack   | Yes | Yes <sup>2</sup> | Yes <sup>3</sup> | Yes <sup>3</sup> | Yes <sup>3</sup> |
| <sup>d1</sup> DIY Arduino Mega Neutrona Wand | Yes | Yes <sup>2</sup> | Yes <sup>2</sup> | Yes <sup>2</sup> | Yes <sup>2</sup> |

<sup>d1</sup> These are now considered as "legacy" devices and have distinct end-of-life notes in later versions (see additional superscript notations).

<sup>2</sup> Support for the Arduino Nano as a wand controller ended after the release of v2.2.0. That device must be replaced with a [Mega 2560 Pro Mini](https://www.amazon.com/s?k=Mega+2560+PRO+MINI) to support the later software releases.

<sup>3</sup> If paired with an Arduino Nano for the Neutrona Wand, this arrangement is deprecated. To continue using the DIY Arduino Mega you will need to upgrade the Neutrona Wand to use the GPStar Neutrona Wand PCB or a [Mega 2560 Pro Mini](https://www.amazon.com/s?k=Mega+2560+PRO+MINI).

## Prerequisites

Download and install the Arduino IDE. This will be used to compile and upload the code to your Proton Pack and Neutrona Wand.

[Arduino IDE](https://www.arduino.cc/en/software)

The following libraries are required to be installed. All can be found within the Arduino Library Manager with the app. Go to `Sketch -> Include Library -> Manage Libraries...` to access the Library Manager. Search for the libraries by name and install the latest version available.

- **ADS1115_WE** by Wolfgang Ewald
- **CRC32** by Christopher Baker
- **digitalWriteFast** by Watterott and Armin Joachimsmeyer
- **ezButton** by ArduinoGetStarted.com
- **FastLED** by Daniel Garcia
- **Ramp** by Sylvain Garnavault
- **SafeString** by Matthew Ford
- **SerialTransfer** by PowerBroker2
- **Simple ht16k33 Library** by Ipaseen
- **Switch** by Albert van Dalen

## +++ IMPORTANT WHEN FLASHING UPDATES +++

If you are flashing updates to your existing setup, make sure that both your Proton Pack and Neutrona Wand Micro SD Cards have all the latest sound effects from this repository.

### Important information for older DIY GPStar Proton Pack builds

If you are compiling the code to upload to an Arduino Mega with the original GPStar home built instructions, you need to disable GPSTAR&#95;PROTON&#95;PACK&#95;PCB which can be found at the very bottom of the `Configuration.h` of the Proton Pack.

Example: `//#define GPSTAR_PROTON_PACK_PCB`

This is a legacy flag, for people who originally put the Cyclotron Lid detection on pin 51 and not pin 43.

- If your Cyclotron Lid detection is on pin 51, then comment/disable this define.
- If your home built GPStar Proton Pack was built with pin 43 for the Cyclotron Lid detection, leave this enabled/uncommented.

## For compiling code on the Arduino Nano:

The **final** release compatible with the Arduino Nano was **v2.2.0** in November of 2023. Releases after this date no longer support that device due primarily to system memory limitations. Please consider migrating to the official GPStar Neutrona Wand PCB to continue to receive additional software updates for your devices. Otherwise, you must replace the Arduino Nano with a [Mega 2560 Pro Mini](https://www.amazon.com/s?k=Mega+2560+PRO+MINI).

**In other words: since the software for the pack and wand must be updated in tandem, this implies that updates cannot be made to a Proton Pack (whether DIY or PCB) without the accompanying change to the Neutrona Wand hardware.**

## Advanced User Configurations

To modify defaults within the software, please see the [Advanced Configuration](ADVCONFIG.md) guide.
