# Software Flashing Guide

Each supported device in the GPStar system (Proton Pack, Neutrona Wand, Attenuator, Single-shot Blaster, Belt Gizmo, Stream Effects) are separate devices and must be updated individually when software updates are available. The following guide focuses on the core system: the Proton Pack and Neutrona Wand. For updating the [Attenuator (aka. Wireless Adapter)](ATTENUATOR_FLASHING.md) or the [Single-Shot Blaster](SINGLESHOT.md), please see their respective guides.

> Note that the GPStar Audio may also require updates to firmware, though this will be rare. See the [SOUND](SOUND.md) guide for more information. As of version 6.1 of the software all devices with a web UI will accurately report the firmware version of the GPStar Audio device it controls. For the Attenuator and Proton Pack both will reflect the Proton Pack's audio device. Compare the reported version number to the latest firmware version to know if an update is necessary.

Please mind the following regardless of model or version of hardware in use:

1. The same software version should be uploaded for **ALL** interconnected devices (read: Pack, Wand, and optional Attenuator), even if the release changelog does not advertise modifications for a specific device. This ensures proper serial communication between these components.
1. When you are flashing updates to your devices, make sure that **BOTH** your Proton Pack and Neutrona Wand Micro SD Cards have all the latest sound effects from this repository as found in the [sounds](/sounds/) directory.
1. Accessories such as the Stream Effects and Belt Gizmo should also be updated to remain in sync with any changes to WebSocket functionality which may have been implemented for the Attenuator/Pack and Wand.

## Supported Devices

Please refer to the table below for a list of devices and their supported software release.

| Controller Device | v1.x | v2.x | v3.x | v4.x | v5.x | v6.0 | v6.1+ |
|-------------------|------|------|------|------|------|------|-------|
| <span class="logo-icon-tiny"></span> GPStar Proton Pack | Yes | Yes | Yes | Yes | Yes | Yes | Yes | Yes |
| <span class="logo-icon-tiny"></span> GPStar Neutrona Wand | Yes | Yes | Yes | Yes | Yes | Yes | Yes |
| <span class="logo-icon-tiny"></span> GPStar Single-shot Blaster | &mdash; | &mdash; | &mdash; | &mdash; | Yes | Yes | Yes |
| <span class="logo-icon-tiny"></span> GPStar Proton Pack II | &mdash; | &mdash; | &mdash; | &mdash; | &mdash; | &mdash; | Yes |
| <span class="logo-icon-tiny"></span> GPStar Neutrona Wand II | &mdash; | &mdash; | &mdash; | &mdash; | &mdash; | &mdash; | Yes |
| <span class="logo-icon-tiny"></span> GPStar Single-shot Blaster II | &mdash; | &mdash; | &mdash; | &mdash; | &mdash; | &mdash; | Yes |
| <sup>d1</sup> DIY Arduino Mega Proton Pack | Yes | Yes <sup>3</sup> | Yes <sup>3</sup> | Yes <sup>3</sup> | Yes <sup>3</sup> | Yes <sup>3</sup> | Yes <sup>3</sup> |
| <sup>d1</sup> DIY Arduino Mega Neutrona Wand | Yes | Yes <sup>2</sup> | Yes <sup>2</sup> | Yes <sup>2</sup> | Yes <sup>2</sup> | Yes <sup>2</sup> | Yes <sup>2</sup> |
| <sup>s1</sup> Attenuator (Arduino Nano) | Yes | Yes | Yes | Yes | Yes | No <sup>4</sup> | No <sup>4</sup> |
| <sup>s1</sup> Attenuator (ESP32) | &mdash; | &mdash; | Yes | Yes | Yes | Yes | Yes |
| <sup>s1</sup> Wireless Adapter (ESP32)  | &mdash; | &mdash; | Yes | Yes | Yes | Yes | Yes<sup>5</sup> |
| Belt Gizmo | &mdash; | &mdash; | &mdash; | &mdash; | &mdash; | Yes | Yes |
| Stream Effects | &mdash; | &mdash; | &mdash; | &mdash; | &mdash; | Yes | Yes |
| GPStar PSTT | &mdash; | &mdash; | &mdash; | &mdash; | &mdash; | &mdash; | Yes<sup>6</sup> |

<sup>d1</sup> These are now considered as "legacy" devices and have distinct end-of-life notes in later versions (see additional superscript notations).

<sup>s1</sup> These are referred to as "Serial1" devices as they attach to the GPStar Proton Pack PCB though they are considered separate devices and will be flashed independently. Please see the [ATTENUATOR_FLASHING](ATTENUATOR_FLASHING.md) guide for those instructions.

<sup>2</sup> Support for the Arduino Nano as a wand controller ended after the release of v2.2.0. That device **must** be replaced with a [Mega 2560 Pro Mini](https://www.amazon.com/s?k=Mega+2560+PRO+MINI) to support the later software releases.

<sup>3</sup> If paired with an Arduino Nano for the Neutrona Wand, this arrangement is deprecated. To continue using the DIY Arduino Mega you **must** upgrade the Neutrona Wand to use a [Mega 2560 Pro Mini](https://www.amazon.com/s?k=Mega+2560+PRO+MINI).

<sup>4</sup> Support for the Arduino Nano as an Attenuator controller ended as of the v6.0.0 release. That device must be replaced with an ESP-WROOM-32 module to support the later software releases.

<sup>5</sup> Use of an ESP32 as Wireless Adapter is still supported for GPStar I users, but is no longer necessary for GPStar II kits as they can provide built-in WiFi capability.

<sup>6</sup> Introduced in the v6.1.1 release and requires the latest software to be installed on a GPStar II Neutrona Wand or GPStar II Single-Shot Blaster.

## 📝 GPStar II Flashing
The GPStar Proton Pack II and GPStar Neutrona Wand II can be updated wirelessly using a WiFi connection and web browser to reach the firmware update screen. Before you begin, you must enable WiFi on all devices as the WiFi on the GPStar Neutrona Wand II is disabled for power saving measures when it is connected to a GPStar Proton Pack II. Similarly, if you have a Attenuator connected, the GPStar Proton Pack II will turn off its WiFi for power saving.

You can manually turn on or turn off the WiFi for each device via one of the following ways:

1. From the [Wand Operation Menu](OPERATION_MENUS.md) via Menu Level 3 which is only available when using a GPStar II Neutrona Wand.
1. From the highest level device (either Attenuator or GPStar II Proton Pack) via the Settings page. Example: From the Attenuator you can turn on WiFi for the pack or wand by using the toggle at the bottom of the Pack Settings or Wand Settings page and only saving the change (no need to save to EEPROM).

For best results it is advised that you update the directly-connected devices in the following order:

1. GPStar II Neutrona Wand
1. GPStar II Proton Pack
1. Attenuator (if connected)

For a complete view of the process please see the following flowchart:

```mermaid
flowchart TD
    A[Enable WiFi for Devices]
    B[Update GPStar II Neutrona Wand]
    C[Update GPStar II Proton Pack]
    D{Attenuator<br/>Connected?}
    E[Update Attenuator]
    F[Updates Complete]
    
    A --> B
    B --> C
    C --> D
    D -->|Yes| E
    D -->|No| F
    E --> F
```
<br/>

### Updating Over-the-Air

Once the WiFi has been enabled for all devices you will connect individually to update the firmware on each device:

1. Connect to the respective device's private WiFi network.
2. Open your web browser to the device and navigate to the settings tab in the web UI.
3. Click on the respective **"Update {device} Firmware"** link.

![](images/gpstar-ii-settings.jpg)

- Use the select file button and select the .bin firmware for the device that can be found in the `/binaries` directory.

![](images/gpstar-ii-update.jpg)

- The upload will begin immediately. Once at 100% the device will reboot.

### Connecting to GPStar II Controllers via USB

Under normal circumstances you should only ever have to update the GPStar II boards over-the-air via the web UI as above. As a backup, the GPStar II Proton Pack and Neutrona Wand have a built-in USB-C connector that can be used to connect any computer to the board for manually flashing firmware. Connect a USB-C cable to the device from your computer, then use the GPStar ESP32 Firmware Flasher to update the firmware. See [GPStar II & ESP32 USB Flashing](#gpstar-ii-usb-flashing) below.

## 📝 GPStar I Flashing

These devices do not offer over-the-air updates and **MUST** be updated by using a USB serial cable and the GPStar firmware flasher utility.

### Flashing Updates

After connecting your GPStar Proton Pack or Neutrona Wand board to your computer with the included FTDI to USB programming cable, download the GPStar firmware flasher from the [extras](/extras/) folder and run the program.

**As of the v6.1.1 release, new versions of the flasher utilities are available and are encouraged to be used instead of any older copies.** These provide more integity/size checks for selected files, ensuring you have selected a true binary file.

### GPStar I Pack/Wand Firmware Flasher

- [Windows (x86/x64)](/extras/gpstarFirmwareFlasher.exe?raw=true)
- [MacOS Intel/M1](/extras/GPStar-Firmware-Flasher-Mac.dmg?raw=true)

The latest pre-compiled firmware binaries can be found in the binaries folder of this repository.<br>
[Binaries Folder](/binaries/)

When downloading the binaries via GitHub, click on the file to open its webpage then click on the **`Download raw file`** button in the upper-right of the page.

![GPStar firmware flasher](images/flashDownload.png)

**You can also find the latest stable versions of the Firmware and Flashing software at:**

[https://gpstartechnologies.com/pages/support-downloads](https://gpstartechnologies.com/pages/support-downloads)

**IMPORTANT: It is REQUIRED to have both the Proton Pack and Neutrona Wand on the same version of their respective firmware. It is also recommended that this be the latest available firmware.**

### Connecting to GPStar I Proton Pack and Neutrona Wand PCBs

Use the included FTDI to USB programming cable that comes with the GPStar kits or use any other suitable FTDI 5V basic serial connector. The UART Pins on the PCB should align with with the standard wire order for FTDI-to-USB cables which use a single Dupont 6-pin connector. Observe these common colours and notes to ensure proper orientation:

- The ground pin will typically be a black wire, while VCC will typically be red.
- The DTR pin on the PCB will connect to a wire labelled either DTR or RTS.
- Any wire labelled CTS will be connected to the 2nd pin labelled GND on the PCB.
- **Be careful to not reverse the connector!**

![UART Connection](images/uart_pack.jpg)

📝 **Note:** For the Neutrona Wand, some users have adopted use of the [Hasbro Plasma Series Spengler's Neutrona Wand: Data Port Front Tube](https://www.etsy.com/listing/1756220009/) as offered by RADIsLAB, which includes a parts list for a dedicated programming cable which stay mounted inside of the device.

### Using the Flashing Software

### Windows

![GPStar firmware flasher Windows](images/flash-gpstar-1-firmware.png)

1. Open the firmware flasher, then select the firmware from the firmware selection box for the GPStar board you wish to flash.
1. Then select the Port for the connection to your GPStar Board. `Microsoft Windows users: The Default baud rate of 115200 should already be automatically selected.` `The Port will be different for every user and may not look like what you see in the screenshot image. If you attempt to flash and your GPStar board can not be found, please select another PORT from the drop down menu and try again.`
1. Click on the Upload button and wait for it to complete.

`Note: If connecting the programming cable for the first time to your computer, it may take Windows a few minutes to setup the USB/TTL drivers automatically before it starts working.`

### macOS

1. Open the firmware flasher by double-clicking the .DMG file. This will mount a new drive for the application.
1. Go to the mounted drive "GPSTAR-FIRMWARE-FLASHER" and double-click on the `GPStar-Firmware-Flasher.app` to start the program.
1. Select the firmware from the firmware selection box for the GPStar board you wish to flash.
1. Then select the PORT for the connection to your gpstar Board after connecting the gpstar board to your computer with the programming cable. `The PORT will be different for every user and may not look like what you see in the screenshot image.`
1. Click on the UPLOAD button and wait for it to complete.

## Flashing Update Instruction Video

[GPStar I Firmware Update Instruction Video](https://www.youtube.com/watch?v=Hbk-RCVR1ew) (YouTube)
[![GPStar I Proton Pack & Neutrona Wand Flashing Demonstration Video](https://img.youtube.com/vi/Hbk-RCVR1ew/maxresdefault.jpg)](https://www.youtube.com/watch?v=Hbk-RCVR1ew)

## GPStar II USB Flashing

### GPStar ESP32 Firmware Flasher

- [Windows](/extras/gpstarESP32FirmwareFlasher.exe?raw=true)

**You can always find the latest stable versions of the Firmware and Flashing software at:**

[https://gpstartechnologies.com/pages/support-downloads](https://gpstartechnologies.com/pages/support-downloads)

The latest pre-compiled firmware binaries can also be found in the binaries folder of the Github project repository: [Binaries Folder](/binaries/)

When downloading the binaries via GitHub, click on the file to open its webpage then click on the **`Download raw file`** button in the upper-right of the page. Alternatively, download the entire release as a .zip file which will guarantee the firmware files are available in the correct format, as well as all audio files.

![GPStar firmware flasher](images/flashDownload.png)

> **If your firmware obtained via GitHub fails to be flashed, check the contents of all downloaded file(s)!** If any file contains HTML then you downloaded the wrong form of the file--return to the Github location and make sure you use the `Download raw file` button.

**IMPORTANT: It is REQUIRED to have both the Proton Pack and Neutrona Wand on the same version of their respective firmware. It is also recommended that this be the latest available firmware.**

### Using the Flashing Software

![GPStar ESP32 firmware flasher Windows](images/flash-gpstar-2-firmware.png)

1. Open the firmware flasher, then select the bootloader and firmware in the appropriate selection boxes for the GPStar board you wish to flash.
1. Then select the Port for the connection to your GPStar Board. `Microsoft Windows users: The Default baud rate of 921600 or 115200 should already be automatically selected.` `The Port will be different for every user and may not look like what you see in the screenshot image. If you attempt to flash and your GPStar board can not be found, please select another Port from the drop down menu and try again.`
1. Click on the Upload button and wait for it to complete.

`Note: If connecting the programming cable for the first time to your computer, it may take Windows a few minutes to setup the USB/JTAG drivers automatically before it starts working.`

## [Advanced] Compile & Upload

If you prefer to make adjustments to the source code configuration options instead of flashing pre-compiled binaries, refer to the [Compiling and Flashing](COMPILING_FLASHING.md) guide.
