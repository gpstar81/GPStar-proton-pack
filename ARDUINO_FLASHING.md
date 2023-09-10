# IDE Setup

The custom software needed for operation must be compiled and uploaded to your devices. This guide provides the list of required libraries for compilation and outlines the changes needed to support each board.

## Supported Devices
###Proton Pack:###
 
- Arduino Mega 2560

###Neutrona Wand:###
- Arduino Nano

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

### Important information for older DIY gpstar Proton Pack builds
If you are compiling the code to upload to an Arduino Mega with the original gpstar home built instructions. You want to use disabled GPSTAR&#95;PROTON&#95;PACK&#95;PCB which can be found at the very bottom of the `configuration.h` of the Proton Pack. 

Example: `//#define GPSTAR_PROTON_PACK_PCB`

This is a legacy flag, for people who originally put the Cyclotron Lid detection on pin 51 and not pin 43. If your Cyclotron Lid detection is on pin 51, then comment/disable this define. If your home built gpstar Proton Pack was built with pin 43 for the Cyclotron Lid detection, then you can leave this enabled.

## For compiling code on the Arduino Nano:

**NOTE: Before uploading to the Arduino Nano, be sure to either disconnect the wand from the pack or otherwise disconnect the TX/RX connections as these will interfere with the USB interface when still connected to the Arduino Mega in the pack.**

### Important for Arduino Nano builds:
  * You will need to open Packet.h located in your Arduino/Libraries/SerialTransfer folder and on line #34 and change the max packet size to 0x40
  * When building for your Mega, you can switch it back to 0xFE

  * Before:
  const uint8&#95;t MAX_PACKET_SIZE = 0xFE; // Maximum allowed payload bytes per packet

  * After:
  const uint8&#95;t MAX_PACKET_SIZE = 0x40; // Maximum allowed payload bytes per packet

![Serial build for Arduino Nano](images/Arduino_nano_serial_build.jpg)

## Advanced User Configurations

To modify defaults within the software, please see the [Advanced Configuration](ADVCONFIG.md) guide.