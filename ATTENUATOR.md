# Pack Attenuator Setup (Optional)

"If Egon had kept on using and modifying his pack and modifying it, what might it look like?"

That was the question posted by Jason Reitman to Adam Savage for the 2021 Ghostbusters Afterlife film. So in 2022 the Tested.com [channel released a video](https://www.youtube.com/watch?v=dloSR3a57IA) for a "1 Day Build" which featured propmaster Ben Eadie who joined Adam Savage to create one particular item which brings more life to the front of the Proton Pack.

This guide demonstrates the build process for replicating the "Pack Attenuator" device created by Adam and Ben which fully integrates into the gpstar electronics kit. The ability to control and receieve feedback from the devices makes this more than just an aesthetic improvement but a truly usable piece of the pack-wand combination.

## Bill of Materials

This device has it's own BOM which is separate from any other build items related to the pack or wand. This shares the same Arduino platform as the other controllers and we'll be using the same type of serial protocol as used between the pack and wand.

* [4 Pin Aviation Connector (to Proton Pack)](https://a.co/d/gPiOSzL)
* [20 AWG 4 Conductor Wire w/ PVC Jacket](https://a.co/d/iglELZR)
* [1/4 Inch Braided PET Wire Loom](https://a.co/d/b7pm6GU)
* [PG7 Rubber Strain Relief Connector](https://a.co/d/h1Hxh5T)
* [Single-Pixel Addressable RGB LED’s](https://a.co/d/90SO4AQ)
* [SPST Mini Toggle Switches](https://a.co/d/9DoDrgZ)
* [Rotary Encoder Knob w/ Switch](https://a.co/d/3iBps4P)
* [28-Segment Bargraph from Frutto Technology](https://fruttotechnology.com/ols/products/preorder-28-segment-bargraph-pcb-for-spengler-neutrona-wand)
* [Nano Terminal Adapter IO Shield](https://a.co/d/gnK7aza) (Optional)

## Dimensions

It is worth noting that the device is meant to attach to the left shoulder strap of a standard ALICE pack. The width of the strap is typically around 80mm while worn, so this device will fit near perfectly once the strap is pressed against your shoulder/chest.

	Width: 80mm
	Height: 115mm
	Depth: 38mm

## Arduino Nano - Pin Reference

The following is a diagram of the Arduino Nano pins from left and right, when oriented with the USB connection facing up (north).

| Connection    | Nano (L) | USB | Nano (R) | Connection    |
|---------------|----------|-----|----------|---------------|
|               | D13      |     | D12      |               |
|               | 3V3      |     | D11      |               |
|               | REF      |     | D10      | Neopixels (2) |
|               | A0       |     | D9       |               |
|               | A1       |     | D8       |               |
|               | A2       |     | D7       |               |
|               | A3       |     | D6       | Right Toggle  |
| SDA Bargraph  | A4       |     | D5       | Left Toggle   |
| SCL Bargraph  | A5       |     | D4       | Encoder Post  |
|               | A6       |     | D3       | Encoder A     |
|               | A7       |     | D2       | Encoder B     |
| To Bargraph   | 5V       |     | GND      | Common Ground |
|               | RST      |     | RST      |               |
| Ground (Pack) | GND      |     | RX0      | to Pack TX1   |
| +5V (Pack)    | VIN      |     | TX1      | to Pack RX1   |

When connecting to the pack, the following wiring scheme was used with the 4-pin connector:

	1 - GND (Black)
	2 - 5V (Red)
	3 - TX1 (White) to Pack RX1
	4 - RX0 (Yellow) to Pack TX1

### Connections by Component

Wire colors are suggestions, and meant to help differentiate the components. You may use your own scheme as desired.

**Toggles**

| LEFT TOGGLE         |   |        |     | Nano Pin | Notes |
|---------------------|---|--------|-----|----------|-------|
| <font color="yellow">Yellow</font> | → | Ground |   |        | Shouldn’t matter which wire goes where |
| <font color="yellow">Yellow</font> | → | →      | → | Pin D5 | Shouldn’t matter which wire goes where |

| Right TOGGLE         |   |        |     | Nano Pin | Notes |
|---------------------|---|--------|-----|----------|-------|
| <font color="green">Green</font> | → | Ground |   |        | Shouldn’t matter which wire goes where |
| <font color="green">Green</font> | → | →      | → | Pin D6 | Shouldn’t matter which wire goes where |

**Encoder**

The rotary encoder is similar to that used on the Proton Pack and Neutrona Wand. It requires a common ground connection and sends data via the A/B signal wires to indicate which direction it was turned.

One notable point is the stated part in the BOM also supports a momentary "push" action on the center post, so an additional ground and data pin will be used for that ability.

| ROTARY ENCODER (DIAL)      |    | Nano Pin |
|----------------------------|----|----------|
| <font color="blue">Blue</font>  | → | Pin D2 |
| <font color="gray">Black</font> | → | GND    |
| <font color="blue">Blue</font>  | → | Pin D3 |

| ROTARY ENCODER (POST)      |    | Nano Pin |
|----------------------------|----|----------|
| <font color="yellow">Yellow</font> | → | Pin D4 |
| <font color="gray">Black</font>    | → | GND    |

**Addressable LED's**

| NEOPIXEL LED'S |   | Component | Nano Pin |
|----------------|---|-----------|----------|
| <font color="red">Red</font>    | → |        | 5V      |
| <font color="blue">Blue</font>  | → | 470k Ω | Pin D10 |
| <font color="gray">Black</font> | → |        | GND     |

**Note:** It is advised to place a 100uf capacitor across the positive and negative connections to these devices, just to buffer any current fluctuations.

## Bargraph

This is a separate but critical device and should be a relatively easy connection when using the Frutto Technology packaging which has only 2 ports: 5V/GND and SDA/SCL. Power will be connected to the "5V" on the Arduino and a common ground (GND). Connect the SDA and SCL to the A4 and A5 pins, respectively.

## Pack Connection Cable

In order to connect to the pack you will need to create a custom cable for the device. This will consist of running the 4-conductor jacketed wire through the braided wire loom, and terminating with the 4-pin connector at one end and passing through the strain relief at the other end. The bare wires can be soldered directly to the Nano or use a Nano terminal shield.

Where the socket for this connection cable is mounted on the pack is up to you--this device is meant to integrate how you think it should. You will be able to use 2 existing JST-XH connections on the gpstar Pack PCB controller: 5V-OUT and TX1/RX1. Please refer to the wiring notes above for how to make this connections. Just remember that TX/RX from the Attenuator will go to RX/TX on the pack (read: the wires flip).

## Component Fitment

The dimensions of the project housing allows for easy access into all areas of the device. It should be no problem to fit the available components within the final device. Note that you will want to build the connection cable in full and run it through the strain relief before attached to the device housing.

## Operation

TBD - Will update once functionality is completed.

## Arduino Nano Pinout Reference

![](images/Arduino-nano-pinout.png)
