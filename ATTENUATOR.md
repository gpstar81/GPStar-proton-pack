# Pack Attenuator (Optional)

"If Egon had kept on using and modifying his pack and modifying it, what might it look like?"

That was the question posed by Jason Reitman to Adam Savage with regards to the 2021 Ghostbusters Afterlife film. So in 2022 [Adam Savage's Tested YouTube channel released a video](https://www.youtube.com/watch?v=dloSR3a57IA) for a "1 Day Build" which featured propmaster Ben Eadie to create a unique item which brings more life to the front of the Proton Pack.

![](images/Savage-Attenuator.jpg)
*Still image of Adam's creation.*

This guide demonstrates the build process for replicating the "Pack Attenuator" device created by Adam and Ben which fully integrates into the gpstar electronics kit. The ability to control and receive feedback from the devices makes this more than just an aesthetic improvement but a truly usable piece of the pack-wand combination.

### Potential Backstory

The device was rediscovered by Phoebe Spengler from notes in her grandfather’s underground workshop on the “Dirt Farm”. The device was theorized and prototyped but never put into service until now.

The attenuator device alters the magnetic field to maintain a correct “rate of feed” of positrons into the cyclotron, thus allowing the user to extend the time to ensnare a ghost by preventing an overheat event within the Proton Pack. Ahead of an overheat (aka. vent sequence) the device will provide physical and audiovisual cues as to the impending event. Interacting with the device will cancel the warning and stave off the venting sequence.

### Special Notes

Credit to [ShapeforgeProps](https://www.etsy.com/shop/ShapeforgeProps) for their excellent reproduction of this device as a 3D print and served as inspiration for taking this to a new level with the gpstar kit.

*Please note that this device is considered experimental and still under development. An electronics and housing kit is in the works which will integrate with the gpstar Proton Pack controller.*

[Attenuator Preview Demonstration](https://www.youtube.com/watch?v=k-u7S7Ctcbc) (YouTube, August 2023).
[![Attenuator Preview Demonstration Video](https://img.youtube.com/vi/k-u7S7Ctcbc/maxresdefault.jpg)](https://www.youtube.com/watch?v=k-u7S7Ctcbc)

[Attenuator Backstory and Updates](https://www.youtube.com/watch?v=BLHyUAcyqoI) (YouTube, October 2023).
[![Attenuator Backstory and Updates](https://img.youtube.com/vi/BLHyUAcyqoI/maxresdefault.jpg)](https://www.youtube.com/watch?v=BLHyUAcyqoI)

## Bill of Materials

This device has it's own BOM which is separate from any other build items related to the pack or wand. This shares the same Arduino platform as the other controllers and we'll be using the same type of serial protocol as used between the pack and wand.

**Core Electronics**

* [4 Pin Aviation Connector (to Proton Pack)](https://a.co/d/gPiOSzL)
* [20 AWG 4 Conductor Wire w/ PVC Jacket](https://a.co/d/iglELZR)
* [1/4 Inch Braided PET Wire Loom](https://a.co/d/b7pm6GU)
* [PG7 Rubber Strain Relief Connector](https://a.co/d/h1Hxh5T)
* [Single-Pixel Addressable RGB LEDs](https://a.co/d/90SO4AQ)
* [SPST Mini Toggle Switches](https://a.co/d/9DoDrgZ)
* [Rotary Encoder Knob w/ Switch](https://a.co/d/3iBps4P)
* [3V 10mm x 2mm Vibration Motor](https://a.co/d/8p7mP9x)
* [9mm Passive Piezo Buzzer](https://a.co/d/b39ELcm)
* [28-Segment Bargraph from Frutto Technology](https://fruttotechnology.com/ols/products/preorder-28-segment-bargraph-pcb-for-spengler-neutrona-wand)
* [Arduino Nano or Similar](https://a.co/d/ev1LPea)
* [Nano Terminal Adapter IO Shield](https://a.co/d/gnK7aza) (Optional)

**Decorations**

This device was designed to allow for certain "real" parts as opposed to being 100% 3D printed. To finish the look of the exterior the following are either required or suggested. These parts may be sourced from any vendor you prefer (in the US most ACE Hardware locations have extensive selections of metrics screws).

* M3x4 Button Hex Head Screws (Qty: 14) - Used for decoration on the sides of the body.
* M4x4 Fillister Head Slotted Screws (Qty: 4) - Used for decoration on the top of the body.
* M4x4 Button Hex Head Screws (Qty: 4) - Used to secure the base plate to the body.
* M4x6 Heat-set Inserts (Qty: 4) - Used to secure the base plate to the body.

Device labels can be obtained or created by any means desired. The sizes [offered by this product on GBFans](https://www.gbfans.com/shop/pack-vinyl-labels/) should be sufficient.


## Dimensions

It is worth noting that the device is meant to attach to the left shoulder strap of a standard ALICE pack. The width of the strap is typically around 80mm while worn, so this device will fit near perfectly once the strap is pressed against your shoulder/chest.

	Width: 80mm
	Height: 115mm
	Depth: 38mm

## Arduino Nano - Standard Pinout Reference

![](images/Arduino-nano-pinout.png)

## Arduino Nano - Pin Connections

The following is a diagram of the Arduino Nano pins from left and right, when oriented with the USB connection facing up (north).

| Connection    | Nano (L) | USB | Nano (R) | Connection    |
|---------------|----------|-----|----------|---------------|
|               | D13      |     | D12      |               |
| Vib. Motor +  | 3V3      |     | D11      | PN2222        |
|               | REF      |     | D10      | Piezo Buzzer  |
|               | A0       |     | D9       | Neopixels (2) |
|               | A1       |     | D8       |               |
|               | A2       |     | D7       |               |
|               | A3       |     | D6       | Right Toggle  |
| SDA Bargraph  | A4       |     | D5       | Left Toggle   |
| SCL Bargraph  | A5       |     | D4       | Encoder Post  |
|               | A6       |     | D3       | Encoder B     |
|               | A7       |     | D2       | Encoder A     |
| To Bargraph   | 5V       |     | GND      | Common Ground |
|               | RST      |     | RST      |               |
| Ground (Pack) | GND      |     | RX0      | to Pack TX1   |
| +5V (Pack)    | VIN      |     | TX1      | to Pack RX1   |

When connecting to the pack, the following wiring scheme was used with the recommended 4-pin connector:

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
| <font color="blue">Blue</font> | → | Pin D2 |
| Black                          | → | GND    |
| <font color="blue">Blue</font> | → | Pin D3 |

| ROTARY ENCODER (POST)      |    | Nano Pin |
|----------------------------|----|----------|
| <font color="yellow">Yellow</font> | → | Pin D4 |
| Black                              | → | GND    |

**Addressable LEDs**

| LED'S          |   | Component | Nano Pin |
|----------------|---|-----------|----------|
| <font color="red">Red</font>    | → | 100uf  | 5V     |
| <font color="blue">Blue</font>  | → | 470k Ω | Pin D9 |
| <font color="gray">Black</font> | → | 100uf  | GND    |

**Note:** It is advised to place a 100uf capacitor across the positive and negative connections to these devices, just to buffer any current fluctuations.

Addressable LEDs have a distinct data flow with solder pads labelled `DIN` and `DOUT`. It is crucial to chain these devices starting from the Arduino to an LED's `DIN` pad first, then the same device's `DOUT` pad to the next LED's `DIN` pad, and so on.

**Physical Feedback**

| PIEZO BUZZER               |    | Nano Pin |
|----------------------------|----|----------|
| <font color="red">Red</font> | → | Pin D10 |
| Black                        | → | GND     |

| VIBRATION MOTOR            | Component(s) | Nano Pin |
|----------------------------|--------------|----------|
| <font color="red">Red</font>     | →      | 3V3 |
| <font color="blue">Blue</font>   | NPN C  |     |
| <font color="green">Green</font> | NPN B → R 270 Ω | Pin D11 |
| Black                            | NPN E → 1N4001  | GND     |

**Note:** The vibration motor requires use of a transistor as the higher current draw exceeds the maximum 40mA recommended for the Arduino Nano pin.

## Bargraph

This is a separate but critical device and should be a relatively easy connection when using the Frutto Technology packaging which has only 2 ports: 5V/GND and SDA/SCL. Power will be connected to the "5V" on the Arduino and a common ground (GND). Connect the SDA and SCL to the A4 and A5 pins, respectively.

## Pack Connection Cable

In order to connect to the pack you will need to create a custom cable for the device. This will consist of running the 4-conductor jacketed wire through the braided wire loom, and terminating with the 4-pin connector at one end and passing through the strain relief at the other end. The bare wires can be soldered directly to the Nano or use a Nano terminal shield.

Where the socket for this connection cable is mounted on the pack is up to you--this device is meant to integrate how you think it should. You will be able to use 2 existing JST-XH connections on the gpstar Pack PCB controller: 5V-OUT and TX1/RX1. Please refer to the wiring notes above for how to make this connections. Just remember that TX/RX from the Attenuator will go to RX/TX on the pack (read: the wires flip).

## Component Fitment

The dimensions of the project housing allows for easy access into all areas of the device. It should be no problem to fit the available components within the final device. Note that you will want to build the connection cable in full and run it through the strain relief before attached to the device housing.

For assembly, the shell contains 4 holes meant to take heat-set inserts which provide metal-threaded junctions for screws to hold on the bottom plate. Because the plat must then be attached to the ALICE pack strap these should be secure but removable in the case of re-flashing software updates to the device.

## Operation

While not attached to a compatible Proton Pack (read: standalone mode) the device will simply provide some lights and effects. The left toggle switch will turn on the bargraph animations while the right toggle switch will turn on the LED's.

Ideally, the device should be connected to the gpstar Proton Pack Controller which will allow it to provide some extended functionality. Under normal use the main dial will allow adjusting the overall volume, starting/stopping music tracks, navigating to the next track, and adjusting the effects volume.

* Left Toggle: Turns the pack on or off, similar to use of the switch under the Ion Arm
* Right Toggle: Turns the LED's on the device on or off
* Main Dial - Long Press: Alternates between two modes of operation
	* Mode 1 (Default) - Indicated by a high buzzer tone
		* Main Dial - Short Press: Starts or stops the current music track
 		* Main Dial - Double Press: Mutes or unmutes all pack/wand audio
		* Main Dial - Turn CW/CCW: Adjusts the overall volume for pack/wand
	* Mode 2 - Indicated by a low buzzer tone
		* Main Dial - Short Press: Advances to the next music track
 		* Main Dial - Double Press: Move to the previous music track
		* Main Dial - Turn CW/CCW: Adjusts the effects volume for pack/wand

Note that during an overheat warning, the device will emit sounds and vibrations in addition to lighting effects as the pack reaches a critical state. At this time the pack operator can turn the primary dial either direction to cancel the current warning. If the warning time is allowed to expire the the pack will enter the vent sequence.
