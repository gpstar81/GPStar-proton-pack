# VSCode + PlatformIO

This guide will outline how to begin coding and compiling using Visual Studio Code and PlatformIO instead of ArduinoIDE.

## Prerequisites

Start with downloading the VSCode IDE for your operating system and get the PlatformIO extension added.

- [Visual Studio Code for Windows, macOS, and Linux](https://code.visualstudio.com/download)
- [How to install PlatformIO for VSCode](https://platformio.org/install/ide?install=vscode)

Once PIO is available you can get to the Platforms and Libraries tabs to begin adding support for our typical microcontrollers and to manage the libraries independently and with specific version requirements. We need support for the correct platforms first, so we'll need to access PlatformIO and begin adding those.

1. Select the PlatformIO tab on the left-hand panel
1. Go to **PIO Home > Platforms**
1. Install the following for hardware support:
	- **Atmel megaAVR** for ATMega 2560
	- **Espressif 32** for ESP32

## Libraries

Before installing a library you'll need a project in context. Alternatively, once a project is configured with libraries they will be downloaded automatically and kept up to date per an associated **platformio.ini** file.

1. Go to **PIO Home > Libraries**
1. Search for the library you need to install
1. Select the project for which the library is needed

Once associated with a project, all libraries will be downloaded by PlatformIO upon opening the project.

## Command Line Tools

To compile a PlatformIO project via shell script, you will need the "pio" utility which can be installed via [Python v3.11 or higher](https://www.python.org/downloads/).

After installing Python3 run the following:

`pip install platformio`

If you need to upgrade pip that can be done using the following:

`pip install --upgrade pip`

Once the `pio` utility is available, the included scripts in the project's `.github/` folder may be used to compile code.

## Uploading Firmware

Each project's `platformio.ini` file specifies the upload speed and a default upload port for the device based on typical device names used to flash the hardware. This may need to be adjusted for your local machine if using a non-standard FTDI flashing cable. You can use the PIO home tab in VSCode to view a list of connected Devices. If your device cannot be identified from the list, try unplugging the device, refresh the page, plug it in again, and refresh again to see if there are any changes. Remember that if your OS cannot see the device you may need to install a USB driver.

## Code Assistance

This project is enabled for use with Github CoPilot for development assistance. The file `.github/copilot-instructions.md` serves as a template to the AI assistant to set standards for coding and fulfill assumptions about the project.
