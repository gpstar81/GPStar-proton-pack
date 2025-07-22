# Stream Effects

What good is a Proton Pack that doesn't throw a stream? This device aims to provide a true interactive experience with your Neutrona Wand by offering a light show of epic proportions, not unlike the spectacle of a giant Twinkie.

This depends on the presence of an Attenuator or WiFi Adapter in order to provide a wireless integration with your equipment.

## External Shell

To help make your final device look more polished, a simple project enclosure is available to encapsulate your ESP32 Mini as will be used in this build.

- [ESP32 Mini Enclosure by Dustin Grau](https://www.tinkercad.com/things/39nAaTEvdFY-esp32-mini-enclosure?sharecode=MeckPb0Cxp0IQtvAr5UArlMZEFwcrhd_EKvVeLHaFFM)

## Bill of Materials

Assembly of this device WILL require SOLDERING skills and is considered a DIY approach at this time. The exact list of parts below were chosen for their availability from common sources and/or available support across the Internet.

* [ESP32 Mini Dev Board](https://a.co/d/dMZEs5r)
* [5M Round Reticulate RGB LED Rope Light](https://a.co/d/j8L33dh) [May be found cheaper from AliExpress]

## ESP32 - Pin Connections

The following is a diagram of the **ESP32 pins** from left and right, when oriented with the USB connection facing down (south) like the pinout diagram above. Again, this table is based on the expected pin labels on the ESP32 itself--your terminal shield may differ slightly and should be checked for proper orientation when you insert the ESP32 into the socket.

| Connection    | ESP32 (L) |     | ESP32 (R) | Connection    |
|---------------|-----------|-----|-----------|---------------|
|               | EN        |     | GPIO23    |               |
|               | GPIO36    |     | GPIO22    |               |
|               | GPIO39    |     | GPIO1     |               |
|               | GPIO34    |     | GPIO3     |               |
|               | GPIO35    |     | GPIO21    |               |
|               | GPIO32    |     | GPIO19    |               |
|               | GPIO33    |     | GPIO18    |               |
|               | GPIO25    |     | GPIO5     |               |
|               | GPIO26    |     | GPIO17    | to Pack RX1   |
|               | GPIO27    |     | GPIO16    | to Pack TX1   |
|               | GPIO14    |     | GPIO4     |               |
|               | GPIO12    |     | GPIO2     |               |
|               | GPIO13    |     | GPIO15    |               |
| to 5V-OUT -   | GND       |     | GND       |               |
| to 5V-OUT +   | VIN       |     | 3.3V      |               |
|               |         | **USB** |         |               |

## Firmware Flashing



## Operation



**Security Notice**


