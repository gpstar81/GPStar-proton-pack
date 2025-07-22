# Belt Gizmo

While the original "gizmo" was never given an on-screen purpose it has endured as a standard piece of kit for any uniform. This guide shows how to an interactive light show to any such device produced by the community, with some light modifications.

This depends on the presence of an Attenuator or WiFi Adapter in order to provide a wireless integration with your equipment.

## External Shell

Many models of this device exist in the wild and you can pick any that you prefer as your base. Or if you have an existing Belt Gizmo you may be able to modify it to incorporate the new electronics.

- [Belt Gizmo by Dave W](https://makerworld.com/en/models/1481024-ghostbusters-belt-gizmo-customisable-readout#profileId-1546412)
- [Belt Gizmo by MRKIOU](https://cults3d.com/en/3d-model/gadget/ghostbusters-belt-gizmo?srsltid=AfmBOoodHZSokoh6WtUnP8dFB2FAfHBY5bJPWOEOunehdjz8OXQWtP-q)

A special holder for the electronics will be necessary to keep things clean and compact. This _should_ work with most Belt Gizmo designs, though it was created specifically for the MRKIOU (Q) design linked above.

- [Belt Gizmo Back by Dustin Grau](https://www.tinkercad.com/things/hXboxGGXGLa-belt-gizmo-back?sharecode=ha-4RQ-0age-2QNgxU-ZpJ5jofNhaNWqsaZMZCeECbs)

## Bill of Materials

Assembly of this device WILL require SOLDERING skills and is considered a DIY approach at this time. The exact list of parts below were chosen for their availability from common sources and/or available support across the Internet.

* [Waveshare ESP32-S3-Zero Mini Dev Board](https://a.co/d/0TzJcFc)
* [USB-C 5V Boost Converter and Lithium Battery Charger](https://a.co/d/9K8KTmT)
* [2500mAh 3.7V Lithium Polymer Battery Pack](https://a.co/d/cKEqCLi)
* [SPDT 2-Position Micro Slide Switch](https://a.co/d/08XEYrL)
* [SPST Momentary Push Button Switch](https://a.co/d/7ySDNmg)
* [28AWG Stranded Silicone Hookup Wire](https://a.co/d/8uN87y4)

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


