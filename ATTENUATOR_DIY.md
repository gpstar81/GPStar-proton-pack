# Pack Attenuator - DIY Guide

This guide is intended to document the parts and processes required to build the internals of an Attenuator device for your Proton Pack. Please note that while both the Arduino Nano is supported, there is much more functionality and room for improvements by using an ESP32 controller. This document supports both paths in their own sub-sections.

## Bill of Materials

This device has it's own BOM which is separate from any other build items related to the pack or wand. This utilizes the same ArduinoIDE for writing software as the other controllers and we'll be using the same type of serial protocol as used between the pack and wand.

**Core Electronics**

* [4 Pin Aviation Connector (to Proton Pack)](https://a.co/d/gPiOSzL)
* [20 AWG 4 Conductor Wire w/ PVC Jacket](https://a.co/d/iglELZR)
* [1/4 Inch Braided PET Wire Loom](https://a.co/d/cDQPSk0)
* [PG7 Rubber Strain Relief Connector](https://a.co/d/h1Hxh5T)
* [Single-Pixel 5V WS2812B RGB Addressable RGB LEDs](https://a.co/d/90SO4AQ) (You will use 3 pixels from this set)
* [Pre-soldered SPST Mini Toggle Switches](https://a.co/d/8SfEpVH)
* [Rotary Encoder Knob w/ Switch](https://a.co/d/3iBps4P)
* [3V 10mm x 2mm Vibration Motor](https://a.co/d/8p7mP9x)
* [9mm Passive Piezo Buzzer](https://a.co/d/b39ELcm)
* [28-Segment Bargraph from Frutto Technology](https://fruttotechnology.com/ols/products/preorder-28-segment-bargraph-pcb-for-spengler-neutrona-wand)

The preferred processor for this device is an ESP32 though you may also use an Arduino Nano though this may be phased out in the future. **Note that wireless capabilities are only available with the ESP32.**

* [ESP-WROOM-32 Dev Board w/ Terminal Shield](https://a.co/d/hDxXluE) or [ESP-WROOM-32 Dev Board w/out Headers](https://a.co/d/62ywBNo)

or

* [Arduino Nano Microcontroller](https://a.co/d/ev1LPea) plus [Arduino Nano Terminal Shield](https://a.co/d/7xNKJtO)

**Decorations**

This device was designed to allow for certain "real" parts as opposed to being 100% 3D printed. To finish the look of the exterior the following are either required or suggested. These parts may be sourced from any vendor you prefer (in the US most ACE Hardware locations have extensive selections of metrics screws).

* [M3x4 Button Hex Head Screws](https://a.co/d/h5No0N9) (Qty: 14) - Used for decoration on the sides of the body.
* [M4x4 Fillister Head Slotted Screws](https://a.co/d/es5eXBC) (Qty: 4) - Used for decoration on the top of the body.

The back plate will be attached using either M5 or M4 screws depending on the material used. For screws, your best value is to purchase an assorted set as this cost less than buying a single size of screw.

* [M5x8 Button Hex Head Screws](https://a.co/d/0fcUDN5) (Qty: 4) - Used to secure the base plate to metal body.

or

* [M4x8 Button Hex Head Screws](https://a.co/d/0fcUDN5) (Qty: 4) - Used to secure the base plate to plastic body.
* [M4x6 Heat-set Inserts](https://a.co/d/dMBuRqA) (Qty: 4) - Used to secure the base plate to a plastic body.

Device labels can be obtained or created by any means desired. The sizes [offered by this product on GBFans](https://www.gbfans.com/shop/pack-vinyl-labels/) should be sufficient.

* [Clear Dome Lens](https://www.digikey.com/en/products/detail/carclo-technical-plastics/10403/2641656)
* [Clip-in Lens SML_190_CTP](https://www.digikey.com/en/products/detail/visual-communications-company-vcc/SML-190-CTP/4515623)

## Dimensions

It is worth noting that the device is meant to attach to the left shoulder strap of a standard ALICE pack. The width of the strap is typically around 80mm while worn, so this device will fit near perfectly once the strap is pressed against your shoulder/chest. These are the overall dimensions if you wish to approximate the needed space or design your own enclosure. Maintaining these dimensions ensures the components above will fit within the enclosure.

	Width: 80mm
	Height: 115mm
	Depth: 38mm

## WiFi Connectivity

When using the ESP32 controller it is possible to connect to the device via WiFi. The SSID (Network Name) will be broadcast as **"ProtonPack_####"** where "####" is the last 4 hexadecimal values for the MAC address of the WiFi interface. This will differ for each ESP32 device, making each network unique to the attached pack. The default password is **"555-2368"** and can (and should) be changed via the web interface after successfully connecting to the network. The IP address for the device will be hardcoded as "192.168.1.2" with the web interface accessible at `http://192.168.1.2`.

**Security Notice**

Once you have successfully paired with the WiFi network for the Proton pack, you are HIGHLY encouraged to change the default password. This is accessible via a link at the bottom of the main device page as noted above. Follow the prompts to enter and confirm a new password then re-join your pack's WiFi network with the new credentials.

For instructions on using the web interface to control your equipment, please see the [Wireless Operations](WIRELESS_OPERATION.md) guide.

## ESP32 - Standard Pinout Reference

![](images/ESP32-pinout.png)

## ESP32 - Pin Connections

The following is a diagram of the ESP32 pins from left and right, when oriented with the USB connection facing down (south) like the pinout diagram above.

| Connection    | ESP32 (L) |     | ESP32 (R) | Connection    |
|---------------|-----------|-----|-----------|---------------|
|               | EN        |     | GPIO23    | Neopixels (2) |
|               | GPIO36    |     | GPIO22    | SCL Bargraph  |
|               | GPIO39    |     | GPIO1     |               |
| Left Toggle   | GPIO34    |     | GPIO3     |               |
| Right Toggle  | GPIO35    |     | GPIO21    | SDA Bargraph  |
| Encoder A     | GPIO32    |     | GPIO19    | PN2222        |
| Encoder B     | GPIO33    |     | GPIO18    | Piezo Buzzer  |
|               | GPIO25    |     | GPIO5     |               |
|               | GPIO26    |     | GPIO17    | to Pack RX1   |
|               | GPIO27    |     | GPIO16    | to Pack TX1   |
|               | GPIO14    |     | GPIO4     | Encoder Post  |
|               | GPIO12    |     | GPIO2     |               |
|               | GPIO13    |     | GPIO15    |               |
| Ground (Pack) | GND       |     | GND       | Common Ground |
| +5V (Pack)    | VIN       |     | 3.3V      | Vib. Motor +  |
|               |         | **USB** |         |               |

When connecting to the pack, the following wiring scheme was used with the recommended 4-pin connector:

	1 - GND (Black)
	2 - VIN (Red)
	3 - TX2 (White) to Pack RX1
	4 - RX2 (Yellow) to Pack TX1

It is advised to add a 330uF capacitor to the VIN+GND pins to help regulate power which will be shared with the controller, bargraph, and addressable LED's.

**Note:** Bargraph power (+5V) should be split from the VIN terminal which delivers power from the Proton Pack.

## Arduino Nano - Standard Pinout Reference

![](images/Arduino-nano-pinout.png)

## Arduino Nano - Pin Connections

The following is a diagram of the Arduino Nano pins from left and right, when oriented with the USB connection facing down (south) like the pinout diagram above.

| Connection    | Nano (L) |     | Nano (R) | Connection    |
|---------------|----------|-----|----------|---------------|
| to Pack RX1   | TX1      |     | VIN      | +5V (Pack)    |
| to Pack TX1   | RX0      |     | GND      | Ground (Pack) |
|               | RST      |     | RST      |               |
| Common Ground | GND      |     | 5V       | To Bargraph   |
| Encoder A     | D2       |     | A7       |               |
| Encoder B     | D3       |     | A6       |               |
| Encoder Post  | D4       |     | A5       | SCL Bargraph  |
| Left Toggle   | D5       |     | A4       | SDA Bargraph  |
| Right Toggle  | D6       |     | A3       |               |
|               | D7       |     | A2       |               |
|               | D8       |     | A1       |               |
| Neopixels (2) | D9       |     | A0       |               |
| Piezo Buzzer  | D10      |     | REF      |               |
| PN2222        | D11      |     | 3V3      | Vib. Motor +  |
|               | D12      |     | D13      |               |
|               |        | **USB** |        |               |

When connecting to the pack, the following wiring scheme was used with the recommended 4-pin connector:

	1 - GND (Black)
	2 - VIN (Red)
	3 - TX1 (White) to Pack RX1
	4 - RX0 (Yellow) to Pack TX1

It is advised to add a 330uF capacitor to the VIN+GND pins to help regulate power which will be shared with the controller, bargraph, and addressable LED's.

### Connections by Component

Wire colors are suggestions, and meant to help differentiate the components. You may use your own scheme as desired. Microcontroller (MCU) pins are labelled using their Nano (D#) or ESP (GPIO#) designations. Common pins will retain a singular name.

**Toggles - Arduino Nano**

| LEFT TOGGLE                    |   | MCU Pin | Notes |
|--------------------------------|---|---------|-------|
| <font color="blue">Blue</font> | → | GND     | Shouldn’t matter which wire goes where |
| <font color="blue">Blue</font> | → | D5      | Shouldn’t matter which wire goes where |

| Right TOGGLE                     |   | MCU Pin | Notes |
|----------------------------------|---|---------|-------|
| <font color="green">Green</font> | → | GND     | Shouldn’t matter which wire goes where |
| <font color="green">Green</font> | → | D6      | Shouldn’t matter which wire goes where |

**Toggles - ESP32**

For the ESP32 controller, GPIO pins 34 and 35 are input-only and do not have internal pull-up resistors. In order to get an accurate reading from the state of the switch it is necessary to add a 3.8k Ω resistor between the 3.3V (3V3) pin and the respective GPIO pin where the toggle switch connects to the controller. This is essentially a wire from the V+ pin to the noted GPIO pin, with the stated resistor on the wire. Both wires for the pull-up resistor and toggle switch will terminate at the same point on the controller.

| LEFT TOGGLE                    |   |        | MCU Pin | Notes |
|--------------------------------|---|--------|---------|-------|
| <font color="blue">Blue</font> | → |   →    | GND     | Shouldn’t matter which wire goes where |
| <font color="blue">Blue</font> | → |   →    | GPIO34  | Shouldn’t matter which wire goes where |
| <font color="red">Red</font>   | ↑ | 3.8K Ω | 3V3     | External Pull-up Resistor |

| Right TOGGLE                     |   |        | MCU Pin | Notes |
|----------------------------------|---|--------|---------|-------|
| <font color="green">Green</font> | → |   →    | GND     | Shouldn’t matter which wire goes where |
| <font color="green">Green</font> | → |   →    | GPIO35  | Shouldn’t matter which wire goes where |
| <font color="red">Red</font>     | ↑ | 3.8K Ω | 3V3     | External Pull-up Resistor |

**Rotary Encoder w/ Switch**

The rotary encoder is similar to that used on the Proton Pack and Neutrona Wand. It requires a common ground connection and sends data via the A/B signal wires to indicate which direction it was turned.

**Note: For this connection the post is oriented to the left.**

| ROTARY ENCODER (DIAL)          |   | MCU Pin   |
|--------------------------------|---|-----------|
| <font color="blue">Blue</font> | → | D2/GPIO33 |
| Black                          | → | GND       |
| <font color="blue">Blue</font> | → | D3/GPIO32 |

One notable point in the stated part noted in the BOM is that this encoder MUST also support a momentary "push" action on the center post, so an additional ground and data pin will be used for that momentary switch.

**Note: It does not matter which pin on the rotary encoder goes to which MCU pin.**

| ROTARY ENCODER (POST)            |   | MCU Pin  |
|----------------------------------|---|----------|
| <font color="green">Green</font> | → | D4/GPIO4 |
| Black                            | → | GND      |

**Addressable LEDs**

| LED'S                           |   | Component | MCU Pin |
|---------------------------------|---|-----------|---------|
| <font color="red">Red</font>    | → | 100uf | 5V/VIN    |
| <font color="green">Green</font>  | → | 470 Ω | D9/GPIO23 |
| <font color="gray">Black</font> | → | 100uf | GND       |

**Note:** It is advised to place a 100uf capacitor across the positive and negative connections to these devices, just to buffer any current fluctuations.

Addressable LEDs have a distinct data flow with solder pads labelled `DIN` and `DOUT`. It is crucial to chain these devices starting from the Arduino to an LED's `DIN` pad first, then the same device's `DOUT` pad to the next LED's `DIN` pad, and so on.

**Audio &amp; Physical Feedback**

| PIEZO BUZZER                 |   | MCU Pin    |
|------------------------------|---|------------|
| <font color="red">Red</font> | → | D10/GPIO18 |
| Black                        | → | GND        |

| VIBRATION MOTOR                | Component(s) | Component(s) | MCU Pin |
|--------------------------------|--------------|--------------|---------|
| <font color="red">Red</font>   | 1N4001 (s)   | →     | 3V3/3.3V |
| <font color="blue">Blue</font> | 1N4001       | NPN C |          |
|                                |              | NPN B | R 270 Ω → D11/GPIO19 |
|                                |              | NPN E | GND      |

**Note:** The vibration motor (60mA) requires use of a transistor as the higher current draw exceeds the maximum 40mA recommended for the microcontroller's pin. To safely deliver power we will use an 2N2222/2N5551, an NPN Bipolar Transistor (BJT). A diode is recommended as this is a motor and thus could cause a small induction current in reverse when power is removed.

![](images/VibrationMotor.jpg)

## Bargraph

This is a separate but critical device and should be a relatively easy connection when using the Frutto Technology packaging which has only 2 ports: 5V/GND and SDA/SCL. Power will be connected to the "5V" on the Arduino and a common ground (GND). F0r the Arduino Nano connect the SDA and SCL to the A4 and A5 pins, respectively. For the ESP32 these are GPIO pins 21 and 22 for SDA and SCL, respectively.

**Note:** For the ESP32 controller, GPIO pins 21 (SDA) and 22 (SCL) do not have internal pull-up resistors. In order to detect attached devices it is necessary to add a 3.8k Ω resistor between the 3.3V (3V3) pin and the respective GPIO pin where the bargraph connects to the controller. This is essentially a wire from the V+ pin to the noted GPIO pin, with the stated resistor on the wire. Both wires for the pull-up resistor and data line will terminate at the same point on the controller.

| Bargraph                        |     |   |        | MCU Pin | Notes |
|---------------------------------|-----|---|--------|---------|-------|
| <font color="red">Red</font>    |     | ↓ | 3.8K Ω | 3V3    | External Pull-up Resistor     |
| <font color="gray">White</font> | SDA | → |   →    | GPIO21 | Check wire labels on bargraph |
| <font color="gray">White</font> | SCL | → |   →    | GPIO22 | Check wire labels on bargraph |
| <font color="red">Red</font>    |     | ↑ | 3.8K Ω | 3V3    | External Pull-up Resistor     |

## ESP32 Break-out Board

To simplify installation it is suggested to create a break-out board for connections when using the ESP32 microcontroller. This is due to the need for pull-up resistors on select pins and can be accomplished by using a solderable ½ size protoboard.

|        | **A** | **B** | **C** | **D** | **E** |   | **F** | **G** | **H** | **I** | **J** |
|--------|-------|-------|-------|-------|-------|---|-------|-------|-------|-------|-------|
| **1**  | <font color="green">GPIO23</font> |  |  | R 470 Ω | ↔ | ↔ | ↔ |  R 470 Ω |  |  | Addressable&nbsp;LED's |
| **2**  |  |  |  |  |  |  |  |  |  |  |  |
| **3**  | <font color="red">3V3</font> | <font color="red">Z+</font> |  | R&nbsp;3.8K&nbsp;Ω | ↔ | ↔ | ↔ | R&nbsp;3.8K&nbsp;Ω |  | <font color="blue">GPIO21</font> | Bargraph&nbsp;SDA (JST-PH) |
| **4**  | | <font color="red">Z+</font> | <font color="red">Y+</font> | R&nbsp;3.8K&nbsp;Ω | ↔ | ↔ | ↔ | R&nbsp;3.8K&nbsp;Ω |  | <font color="green">GPIO22</font> | Bargraph&nbsp;SCL (JST-PH) |
| **5**  |  |  |  |  |  |  |  |  |  |  |  |
| **6**  |  | <font color="red">X+</font> | <font color="red">Y+</font> | R&nbsp;3.8K&nbsp;Ω | ↔ | ↔ | ↔ | R&nbsp;3.8K&nbsp;Ω |  | <font color="blue">GPIO34</font> | <font color="blue">Left Toggle</font> |
| **7**  |  |  |  |  |  |  |    | X- |  |  | Left Toggle (-) |
| **8**  |  |  |  |  |  |  | Y- | X- |  |  | Right Toggle (-) |
| **9**  |  | <font color="red">X+</font> | <font color="red">W+</font> | R&nbsp;3.8K&nbsp;Ω | ↔ | ↔ | ↔ | R&nbsp;3.8K&nbsp;Ω |  | <font color="green">GPIO35</font> | <font color="green">Right Toggle</font> |
| **10** |  |  |  |  |  |  |  |  |  |  |  |
| **11** |  |  |  |  |  |  |  |  | <font color="red">W+</font> | 1N4001 (s) | <font color="red">Vib. Motor (+)</font> |
| **12** | Buzzer&nbsp;(&dash;) |  | Z- |  | Y- |  |  |  |  | ↕ |  |
| **13** | <font color="red">Buzzer&nbsp;(+)</font> |  | <font color="green">GPIO18</font> |  |  |  |  |  |  | ↕ |  |
| **14** |  |  |  |  |  |  |  |  | NPN&nbsp;C | 1N4001 | <font color="blue">Vib. Motor (-)</font> |
| **15** | <font color="green">GPIO19</font> |  |  | R 270 Ω | ↔ | ↔ | ↔ | R 270 Ω | NPN&nbsp;B |  |  |
| **16** | GND (-) | V- |  |  | W- |  | W- |  | NPN&nbsp;E |  |  |
| **17** |  | V- | Rot.&nbsp;Enc.&nbsp;(&dash;) | Enc.&nbsp;Post&nbsp;(&dash;) | Z- |  |  |  |  |  |  |

## Pack Connection Cable

In order to connect to the pack you will need to create a custom cable for the device. This will consist of running the 4-conductor jacketed wire through the braided wire loom, and terminating with the 4-pin connector at one end and passing through the strain relief at the other end. The bare wires can be soldered directly to the microcontroller or use a terminal shield.

Where the socket for this connection cable is mounted on the pack is up to you--this device is meant to integrate how you think it should. You will be able to use 2 existing JST-XH connections on the gpstar Pack PCB controller: 5V-OUT and TX1/RX1. Please refer to the wiring notes above for how to make this connections. Just remember that TX/RX from the Attenuator will go to RX/TX on the pack (read: the wires flip).

Please see the notes above for the correct pin names for connections based on your choice of microcontroller.

## Component Fitment

The dimensions of the project housing allows for easy access into all areas of the device. It should be no problem to fit the available components within the final device. Note that you will want to build the connection cable in full and run it through the strain relief before attached to the device housing.

For assembly, the shell contains 4 holes meant to take heat-set inserts which provide metal-threaded junctions for screws to hold on the bottom plate. Because the plat must then be attached to the ALICE pack strap these should be secure but removable in the case of re-flashing software updates to the device.