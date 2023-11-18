# IDE Setup

The custom software needed for operation must be compiled and uploaded to your devices. This guide provides the list of required libraries for compilation and outlines the changes needed to support each board.

## Supported Devices

###Proton Pack:###

- GPStar Proton Pack PCB
- Arduino Mega 2560

###Neutrona Wand:###

- GPStar Neutrona Wand PCB
- DEPRECATED: Arduino Nano

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

For reference, the FQBN for builds is "arduino:avr:mega" for the GPStar PCB's while "arduino:avr:nano" would be used for a Nano controller as part of the DIY builds.

### MillisDelay

The MillisDelay library must be downloaded from the project GitHub page. Download the code as a zip use the `Sketch -> Add .ZIP Library` option to import the downloaded file.
[https://github.com/ansonhe97/millisDelay](https://github.com/ansonhe97/millisDelay)

No further configuration is needed for this library.

## +++ IMPORTANT WHEN FLASHING UPDATES +++

If you are flashing updates to your existing setup, make sure that both your Proton Pack and Neutrona Wand Micro SD Cards have all the latest sound effects from this repository.

### Important information for older DIY GPStar Proton Pack builds

If you are compiling the code to upload to an Arduino Mega with the original GPStar home built instructions. You want to use disabled GPSTAR&#95;PROTON&#95;PACK&#95;PCB which can be found at the very bottom of the `configuration.h` of the Proton Pack.

Example: `//#define GPSTAR_PROTON_PACK_PCB`

This is a legacy flag, for people who originally put the Cyclotron Lid detection on pin 51 and not pin 43.

- If your Cyclotron Lid detection is on pin 51, then comment/disable this define.
- If your home built GPStar Proton Pack was built with pin 43 for the Cyclotron Lid detection, then you can leave this enabled.

## For compiling code on the Arduino Nano:

The **final** release compatible with the Arduino Nano was **v2.2.0** in November of 2023. Releases after this date no longer support that device due primarily to system memory limitations. Please consider migrating to the official GPStar Neutrona Wand PCB to continue to receive additional software updates for your devices.

**In other words, since the software for the pack and wand must be updated in tandem, this implies that updates cannot be made to a Proton Pack (whether DIY or PCB) without the accompanying change to the Neutrona Wand.**

## Advanced User Configurations

To modify defaults within the software, please see the [Advanced Configuration](ADVCONFIG.md) guide.
