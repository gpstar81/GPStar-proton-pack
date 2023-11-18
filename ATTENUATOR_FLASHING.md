# Flashing the Attenuator

Separate firmware files exist for the Arduino Nano vs. the ESP32 version of the Attenuator, though the "Wireless Adapter" is simply an ESP32 controller without the inputs/outputs of an Attenuator. Therefore, the processes in this document will be the same for either use of an ESP32 controller.

For the Arduino Nano you may use the same flashing utility as the other gpstar PCB devices as outlined in the [FLASHING](FLASHING.md) guide. For the ESP32 that will require a different process as outlined below. Since both the Arduino Nano and ESP development board have their own USB connection it will **not** be necessary to use a separate UART programming cable.

## For ESP32

This device supports Over-The-Air (OTA) updates for firmware, meaning you will need to utilize a desktop web browser from a computer (not a mobile device) and the built-in WiFi access point provided by the controller.

1. Power up your Proton Pack and device,
1. Open the WiFi preferences on your computer and look for the SSID which begins "ProtonPack_".
	* If this is your first connection to the access point, use the default password "555-2368".
1. Navigate to the URL: [http://192.168.1.2/update](http://192.168.1.2/update)
1. Use the "Select File" button and select the [Attenuator-ESP32.bin](binaries/attenuator/Attenuator-ESP32.bin) file from the `/binaries/attenuator` directory.
1. The upload will begin immediately. Once at 100% the device will reboot.
1. Navigate to [http://192.168.1.2](http://192.168.1.2) to confirm that the device is able to communicate with the Proton Pack PCB.

![](images/WebUI-Update1.jpg)

![](images/WebUI-Update2.jpg)

![](images/WebUI-Update3.jpg)

### Setting a WiFi Password

Once you are able to reach the web UI at [http://192.168.1.2](http://192.168.1.2) scroll to the bottom of the page to find the "Change WiFi Password" link. Follow the instructions on the page to set a new password for your device. This will be unique to the ESP32 controller and will persist as the new default even if the device is power-cycled.

![](images/WebUI-Password.jpg)

### Forgot Your WiFi Password?

Since you won't have the ability to use the OTA update process above, you will need to follow a manual process using a USB cable and a utility for your OS of choice:

**For Windows:**

1. Download the [Flash Download Tools](https://www.espressif.com/en/support/download/other-tools) from Espressif Systems.
1. Locate the [Attenuator-ESP32-Reset.bin](binaries/attenuator/Attenuator-ESP32-Reset.bin) file from the `/binaries/attenuator` directory.
1. Use the utility to upload the .bin file to the device via USB.

**For Linux/MacOS:**

1. Download [Python](https://www.python.org/downloads/) and install for your operating system.
1. From a terminal (command line) environment run the following:
	* `curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py`
	* `python get-pip.py` or `python3 get-pip.py`
	* `pip install esptool`
1. Locate the [Attenuator-ESP32-Reset.bin](binaries/attenuator/Attenuator-ESP32-Reset.bin) file from the `/binaries/attenuator` directory.
1. Run `esptool.py -p <PORT> Attenuator-ESP32.bin` where `<PORT>` is your ESP32 controller as a serial (USB) device.

Once flashed, this will allow you to get back into the web UI using the default password ("555-2368") and change to your choice of password. Once changed, you will need to re-flash the device using the standard firmware--otherwise, the device will always use the default WiFi password!

## For Arduino Nano

Just as you used the gpstar flashing utility for Windows or MacOS to upload to your Proton Pack or Neutrona Wand, you will do the same for this device. Plug in your device using a standard USB cable and note the serial COM port used. Select the "Attenuator-Nano.hex" file from the `/binaries/attenuator` directory and upload to the attached device.

---

## Software Development Requirements

To build or edit the code for this device you must have an ArduinoIDE environment set up similar to what is needed for the pack/wand software. Download and install the Arduino IDE 2.x or higher. This will be used to compile and upload the code to your Proton Pack and Neutrona Wand.

[Arduino IDE 2.x](https://www.arduino.cc/en/software)

The following libraries are required to be installed. All but the MillisDelay library can be found within the Arduino Library Manager with the app. Go to `Sketch -> Include Library -> Manage Libraries...` to access the Library Manager. Search for the libraries by name and install the latest version available.

### Common Libraries

- **FastLED** by Daniel Garcia
- **ezButton** by ArduinoGetStarted.com
- **simple ht16k33 library** by lpaseen
- **SerialTransfer** by PowerBroker2
- **millisDelay** `See Below`

### ESP32 Libraries

- **ArduinoJSON** by Benoit Blanchon
- **AsyncTCP** by dvarrel
- **ESPAsyncTCP** by dvarrel
- **ESPAsyncWebSrv** by dvarrel
- **Preferences** by Volodymyr Shymanskyy
- **ElegantOTA** `See Below`
- **ESPAsyncWebServer** `See Below`

To build for the ESP32 controller you will need to use the `Boards Manager` to install the `esp32 by Expressif Systems` package. When selecting a board for compilation and upload, simply use the board `ESP32 Dev Module` for satisfactory results. For reference, the FQBN for builds is "esp32:esp32:esp32".

### ElegantOTA

The ElegantOTA library must be enabled to utilize the Asynchronous Web Server.

1. Go to your Arduino libraries directory
1. Open `ElegantOTA` folder and then open `src` folder
1. Locate the `ELEGANTOTA_USE_ASYNC_WEBSERVER` macro in the `ElegantOTA.h` file, and set it to 1:
	`#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1`
1. Save the changes to the `ElegantOTA.h` file.

### ESPAsyncWebServer

The ESPAsyncWebServer library must be downloaded from the project GitHub page. Download the code as a zip use the `Sketch -> Add .ZIP Library` option to import the downloaded file.
[https://github.com/me-no-dev/ESPAsyncWebServer.git](https://github.com/me-no-dev/ESPAsyncWebServer.git)

No further configuration is needed for this library.

### MillisDelay

The MillisDelay library must be downloaded from the project GitHub page. Download the code as a zip use the `Sketch -> Add .ZIP Library` option to import the downloaded file.
[https://github.com/ansonhe97/millisDelay.git](https://github.com/ansonhe97/millisDelay.git)

No further configuration is needed for this library.