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

## Workspaces

A multi-project workspace is included within the Github repository as `GPStar-proton-pack.code-workspace`. Double click on the file to launch VSCode with the workspace in context.

## Libraries

Every project is configured with with a **platformio.ini** file which determines which external libraries are necessary for that project, down to the architecture level. This is necessary as ESP32 projects may rely on libraries which are either different from what ATMega requires, or the associated project has more components that needs to be utilized. It should not be necessary to explicitly download nor install libraries manually.

## Environments

Each project may consist of 1 or more environments. These may be in the form of architecture-specific environments (eg. ATMega 2560 vs. ESP32) or test vs. development. Pay attention to the PlatformIO Project Environment selection at the bottom of the VSCode UI to see which project and environment is currently in context for builds and uploads.

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

The file `.github/copilot-instructions.md` defines project-specific guidelines for Copilot, such as coding standards, conventions, and assumptions about how the project is structured. Copilot uses this file to better align its suggestions with each project’s expectations.

## Project-Aware Context (Optional)

In addition to standard Copilot behavior, this project includes an optional local Model Context Protocol (MCP) server configuration which allows Copilot to reference the project’s own documentation and source code when generating suggestions.

This works by:

- Breaking project files into small sections (aka. "chunks").
- Converting those sections into a searchable format (vector index).
- Allowing Copilot to query this local index for relevant information while you are coding.

This service runs locally and does not modify Copilot itself, it simply provides additional context when Copilot asks for it.

### Running the Local Context Server

To start the local context server, you will need Python and the following dependencies:

`pip install sentence-transformers numpy faiss-cpu`

Open the Extensions panel and look for the "MCP Servers - Installed" section at the bottom, which should list the `gpstar-rag` entry. Click on the gear icon and select the `Start Server` option from the context menu.
Once running, the server listens for requests from GitHub Copilot and returns relevant snippets from the project to improve suggestion quality. An output panel should open to show the status of the server as it starts. If you see an information line which reads "Discovered 1 tools" then the server has correctly started and is available for use by the chat agent.