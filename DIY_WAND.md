# Neutrona Wand Setup

**As of November 2023 the last release to support software builds for the Arduino Nano was v2.2.0. Due to memory limitations of this device no further software updates are possible. In order to make use of this guide you will need to upgrade to either an official GPStar Neutrona Wand controller PCB or use a Mega2560 Pro Mini as linked in the [DIY_BOM](DIY_BOM.md) document.**

This guide begins with some explanations for common items you will encounter during the upgrade process. Note that it will be necessary to upgrade the hose and conductors between the pack and wand. This exercise is left to individual preferences on connectors and style of replacement, though you will need 4 conductors at a minimum (+5V, Ground, and TX/RX for communications).

The space within the Neutrona Wand is extremely limited, though multiple components must be fit into this space. To maximize what is available, some wires from the original controller (PCB) can be directly soldered onto the microcontroller. Where resistors or inline components are required, these may be soldered between the wire and microcontroller to reduce need for an additional protoboard. If needed, a [a ¼ protoboard from Adafruit](https://www.adafruit.com/product/1608) or any similar size from another manufacturer will be used.

## Special Cautions and Warnings

You will find that the wand poses some challenges due to confined spaces and fragile wires to all the stock components. The following are suggestions to help you minimize damage to existing wiring, unless you plan to remove all the stock components and connect new wires.

- Remove what you can of any plastic covers over the switches internally, and use a hot glue gun to carefully drip glue over the soldered ends of the toggles, rotary encoder, and any other wire connections to reinforce them as they stand.
- Gently separate any wires that are bonded together, allowing about 1" of excess for stripping.
- Use a mechanical wire stripper to ensure you quickly strip back the sheathing for each wire.
- Use silicone-coated **stranded** wiring (22AWG down to 26AWG) to allow for maximum flexibility.
- Solder on extensions to every wire in the wand and use heat-shrink tubing to reinforce extended connections to make a better bond with the stock wire to prevent breakage.
- Continue the extended wire to either a PCB or the microcontroller for soldering, making sure that ALL of the strands of wire make it through the holes. Twist your bare wires tightly, and you can pre-tin the ends with a bit of solder to keep everything together as you fit the wire through any PCB holes (this also works well for making breadboard connections).
- Optionally, this is where you may consider a full replacement of some wires if they break off at the component you are connecting.
- It's too easy to have some strands that get missed which could cause a short--check your connections after soldering. A magnifying glass and good lighting is your friend!

## Original Connection References

You may wish to refer to this PCB drawing and the following table to help identify the original components in the Neutrona Wand.

![Stock PCB Layout](images/WandPCBs.png)

*[Wand Reference Diagrams](https://github.com/Tomdf/Ghostbusters/tree/main/Haslab%20Pack%20Illustrations/Neutrona_Wand_Illustration) by Tomdf*

| Name | Socket | Color | Hasbro Pin | Notes |
|------|--------|-------|------------|-------|
| J3   |        | <font color="brown">Brown</font> | D2 | Bar Graph LEDs (5) |
|      |        | <font color="red">Red</font> | D3 |  |
|      |        | <font color="orange">Orange</font> | D4 |  |
|      |        | <font color="yellow">Yellow</font> | D5 |  |
|      |        | <font color="green">Green</font> | D6 |  |
|      |        | <font color="blue">Blue</font> | VDD |  |
| P6   |        | <font color="red">Red</font> | V4.5 | 4.5V connection from battery compartment |
| GND  |        | Black | GND | Main power ground |
| M1   |        | <font color="red">Red</font> | VCC | Wand vibration motor |
|      |        | Black | GND |  |
| SW1  |        | <font color="brown">Brown</font> | SPDT Toggle | Lower Right Toggle (Main Power) |
|      |        | <font color="brown">Brown</font> |  |  |
| SW2  |        | <font color="red">Red</font> | SPDT Toggle | Upper Right Toggle (Bargraph Power)  |
|      |        | <font color="red">Red</font> |  |  |
| SW3  |        | <font color="brown">Brown</font> | V+ | Rotary encoder (intensity) on top of wand |
|      |        | <font color="red">Red</font> | A  |  |
|      |        | <font color="orange">Orange</font> | B |  |
| SW7  |        | <font color="orange">Orange</font> | SPST Switch | Wand tip retraction/extension sensor |
|      |        | <font color="orange">Orange</font> |  |  |
| SW4  | JST-PH | <font color="red">Red</font> | SPDT Toggle | Activate toggle on left control box |
|      |        | <font color="red">Red</font> |  |  |
| SW5  | JST-PH | <font color="gray">White</font> | SPST Button | Intensify button on left control box |
|      |        | <font color="gray">White</font> |  |  |
| SW6  | JST-PH | <font color="gray">White</font> | SPST Button | Orange button at end of wand, changes modes |
|      |        | <font color="gray">White</font> |  |  |
| D1   |        | <font color="red">Red</font> | VCC | SLO-BLO LED |
|      |        | Black | GND | |
| NA   |        | <font color="red">Red</font> | VDD | Power to D7 & D8 |
| D7   |        | <font color="gray">White</font> | GND | Top Right LED |
| D8   |        | Black | GND | Top Vent LED |
| D9   |        | <font color="red">Red</font> | VCC | Clippard LED (Top Left) |
|      |        | <font color="yellow">Yellow</font> | GND |  |
| LS1  |        | <font color="yellow">Yellow</font> | Speaker + | Wire closest to barrel LED connector is Speaker + |
|      |        | <font color="yellow">Yellow</font> | Speaker - | Wire furthest from barrel LED connector is Speaker - |

# (Important) Neutrona Wand Barrel LEDs wire colour differences.

It appears some of the silver tip generation 1 Neutrona Wands and orange tip generation 2 Neutrona Wands may have different wire colour combinations for the barrel LED lights:

## Generation 1 (Silver tip Neutrona Wands)
| Name | Socket | Color | Hasbro Pin | Notes |
|------|--------|-------|------------|-------|
| Q2   | JST-PH | <font color="yellow">Yellow</font> | VCC | Addressable LEDs for wand tip effects (5 total) |
|      |        | <font color="red">Red</font> | Data |  |
|      |        | <font color="red">Brown</font> | GND |  |

## Generation 2 (Orange tip Neutrona Wands)
| Name | Socket | Color | Hasbro Pin | Notes |
|------|--------|-------|------------|-------|
| Q2   | JST-PH | <font color="blue">Blue</font> | VCC | Addressable LEDs for wand tip effects (5 total) |
|      |        | <font color="yellow">Yellow</font> | Data |  |
|      |        | <font color="red">Red</font> | GND |  |

## WAV Trigger Connections

These are the same connections which are used for the FTDI breakout board to program the WAV Trigger. Use pins and sockets for an easy connection should a firmware upgrade be required in the future.

| WAV Trigger | Connection |
|------------|------------|
| 5Vin       | +5V        |
| GND        | Ground     |
| RX         | PIN D14    |
| TX         | PIN D15    |
| SPKR+      | Speaker +  |
| SPKR-      | Speaker -  |

## Mega2560 Pro Mini - Pin Reference vs. Hasbro Controller

The following is a diagram of the Mega2560 Pro Mini pins from left and right, when oriented with the USB connection facing up (north). As this board has 2 rows of pins on each side it is crucial to re-check labels before connection. Please see the diagram at the bottom of the page if necessary.

Please note that the Mega2560 Pro Mini uses a completely different pinout from the old Arduino Nano DIY build!

| Mega (L1) | Mega (L2) | USB | Mega (R1) | Mega (R2) |
|-----------|-----------|-----|-----------|-----------|
| VIN &rarr; +5V (Pack) | VIN |     |     |     |
| GND &rarr; Ground (Pack) | GND |     |     |     |
| 5V   | 5V   |     |     |     |
| 3.3V | 3.3V |     |     |     |
| RST  | AREF |     |     |     |
| TX0 &rarr; RX (to Pack) | RX0 &rarr; TX (to Pack) |     | A1 &rarr; D2 (Brown) | A0 &rarr; SW2 (Red) |
| D3 &rarr; SW4 (Red) | D2 &rarr; SW45 (White) |     | A3 &rarr; D4 (Orange) | A2 &rarr; D3 (Red) |
| D5 | D4 &rarr; SW1 (Brown) |     | A5 &rarr; D6 (Green) | A4 &rarr; D5 (Yellow) |
| D7 &rarr; ROT (Yellow) | D6 &rarr; ROT (Red) |     | A7 &rarr; SW7 (Orange) | A6 &rarr; SW6 (White) |
| D9 &rarr; D9 (Red) | D8 &rarr; D1 (Red) |     | A9   | A8   |
| D11 &rarr; Motor-NPN | D10 &rarr; Q2<sup>!</sup> (Yellow) or (Red) |     | A11  | A10  |
| D13 &rarr; D8 (Red) | D12 &rarr; D7 (Red) |     | A13  | A12  |
| D15 &rarr; WAV Trigger TX | D14 &rarr; WAV Trigger RX |     | A15  | A14  |
| D17  | D16  |     | D33  | D32  |
| D19  | D18  |     | D35  | D34  |
| D21  | D20  |     | D37  | D36  |
| D23  | D22  |     | D39  | D38  |
| D25  | D24  |     | D41  | D40  |
| D27  | D26  |     | D43  | D42  |
| D29  | D28  |     | D45  | D44  |
| D31  | D30  |     | D47  | D46  |

<sup>!</sup> **See note above about Barrel LED wire colour differences:** Generation 1 Neutrona Wands (silver tip) data line is (Red) while generation 2 Neutrona Wands (orange tip) data line is (Yellow)

### Connections by Component

**Toggles and Buttons**

| LOWER RIGHT TOGGLE → SW1         |   |        |     | Mega Pin | Hasbro Ref | Notes |
|----------------------------------|---|--------|-----|----------|-----|----------------------------------------|
| <font color="brown">Brown</font> | → | Ground |     |          | SW1 | Shouldn’t matter which wire goes where |
| <font color="brown">Brown</font> | → | →      | →   | Pin D4   | SW1 | Shouldn’t matter which wire goes where |

| UPPER RIGHT TOGGLE → SW2         |   |        |     | Mega Pin | Hasbro Ref | Notes |
|----------------------------------|---|--------|-----|----------|-----|----------------------------------------|
| <font color="red">Red</font>     | → | Ground |     |          | SW2 | Shouldn’t matter which wire goes where |
| <font color="red">Red</font>     | → | →      | →   | Pin A0   | SW2 | Shouldn’t matter which wire goes where |

| SW45/SW4 Connector: SW45 = Intensify / SW4 = Activate |   |        |    | Mega Pin | Hasbro Ref |
|-------------------------------------------------------|---|--------|----|----------|------------|
| <font color="gray">White</font> | → | Ground |    |        | SW45 |
| <font color="gray">White</font> | → | →      | →  | Pin D2 | SW45 |
| <font color="red">Red</font>    | → | Ground |    |        | SW4  |
| <font color="Red"> Red </font>  | → | →      | →  | Pin D3 | SW4  |

| MODE SWITCH → SW6 |    |            |    | Mega Pin | Hasbro Ref |                                       |
|-------------------|----|------------|----|----------|------------|---------------------------------------|
| <font color="gray">White</font> | → | Ground |   |        | SW6 | Shouldn’t matter which wire goes where |
| <font color="gray">White</font> | → | →      | → | Pin A6 | SW6 | Shouldn’t matter which wire goes where |

| BARREL EXTENSION SWITCH → SW7 |   |        |    | Mega Pin | Hasbro Ref |                                   |
|-------------------------------|---|--------|----|----------|------------|-----------------------------------|
| <font color="orange">Orange</font> | → | Ground |   |        | SW7 | Shouldn’t matter which wire goes where |
| <font color="orange">Orange</font> | → | →      | → | Pin A7 | SW7 | Shouldn’t matter which wire goes where |

The rotary encoder is similar to that used on the Proton Pack. It requires a common ground connection and sends data via the A/B signal wires to indicate which direction it was turned.

| ROTARY ENCODER (ROT) → SW3 |   |              |    | Mega Pin | Hasbro Ref |
|----------------------------|---|--------------|----|----------|------------|
| <font color="brown">Brown</font>   | → | Ground |    |        | V+ |
| <font color="red">Red</font>       | → | →      | →  | Pin D6 | SW3 |
| <font color="orange">Orange</font> | → | →      | →  | Pin D7 | SW3 |

**Motors**

Considered optional as this can be left out if desired, and without impact to normal operation.

| VIBRATION&nbsp;MOTOR |   |            | Motor Wire | Mega&nbsp;Pin | Notes                                     |
|----------------------|---|------------|------------|---------|-------------------------------------------------|
|                      |   | 1N4001 (s) | <font color="red">Red</font> | +5V     | 1N4001 diode (s = striped end of the diode) |
| NPN C                | → | 1N4001     | Black                        |         | NPN* = PN2222                               |
| NPN B                | → | 330 Ω      | →                            | Pin D11 |                                             |
| NPN E                | → | →          | →                            | Ground  |                                             |

*NPN Bipolar Transistor. Reference Pack page for more info.

### Hasbro Bargraph
| BARGRAPH → J3                      |   |       |    | Mega Pin | Hasbro Ref |
|------------------------------------|---|-------|----|----------|------------|
| <font color="blue">Blue</font>     | → | +5V   |    |          | VDD  |
| <font color="green">Green</font>   | → | 140 Ω | →  | Pin A5   | D6   |
| <font color="yellow">Yellow</font> | → | 140 Ω | →  | Pin A4   | D5   |
| <font color="orange">Orange</font> | → | 140 Ω | →  | Pin A3   | D4   |
| <font color="red">Red</font>       | → | 140 Ω | →  | Pin A2   | D3   |
| <font color="brown">Brown</font>   | → | 140 Ω | →  | Pin A1   | D2   |

Note for Bargraph: If you want to replace LEDs, the bargraph uses 3mm LED diodes.
**Use the appropriate resistor for your replacement LEDs based on their forward voltage given a 5V source!**

### Wand LEDs ###

| VENT LIGHT LED BOARD → D7 & D8  |   |      |    | Mega Pin | Hasbro Ref |                                        |
|---------------------------------|---|------|----|----------|------------|----------------------------------------|
| <font color="red">Red</font>    | → | +5V  |    |          | VDD        |                                        |
| <font color="gray">White</font> | → | 90 Ω | →  | Pin D12  | D7         | Blinking white LED on top of the wand  |
| Black                           | → | 90 Ω | →  | Pin D13  | D8         | White LED inside the wand (vent light) |

| SLO-BLO → D1         |    |       |    | Mega Pin     | Hasbro Ref | Notes |
|----------------------|----|-------|----|--------------|----|-------------------------------------|
| <font color="red">Red</font> | → | 140 Ω | → | Pin D8 | D1 |                                     |
| Black                        | → | →     | → | Ground | D1 |                                     |

| FRONT LEFT LED → D9 |    |       |    | Mega Pin     | Hasbro Ref | Notes |
|---------------------|----|-------|----|--------------|----|-------------------------------------|
| <font color="red">Red</font>       | → | 140 Ω | → | Pin D9 | D9 | |
| <font color="yellow">Yellow</font> | → | →     | → | Ground | D9 | |

## Wand Barrel LEDs

**\*See note near the top of this page about the Neutrona Wand barrel LED wire colours differences between the silver tip and orange tip Neutrona Wands\***

**Generation 1 (Silver Tip Wands)**

| WAND-TUBE → Q2 |   |              |   | Mega Pin | Hasbro Ref | Notes                                  |
|----------------|---|--------------|---|----------|------------|----------------------------------------|
| <font color="yellow">Yellow</font>     | → | +5V    |    |         | Q2 | Power to 5 wand LEDs                |
|                                    |   | ↕      | CAP 100uf |  |    | Capacitor across the +/- power wires |
| <font color="brown">Brown</font>       | → | Ground |    |         | Q2 | Ground to 5 wand LEDs               |
| <font color="red">Red</font> | → | 470Ω   | →  | Pin D10 | Q2 | Data input for addressing LEDs      |

**Generation 2 (Orange Tip Wands)**

| WAND-TUBE → Q2 |   |              |   | Mega Pin | Hasbro Ref | Notes                                  |
|----------------|---|--------------|---|----------|------------|----------------------------------------|
| <font color="blue">Blue</font>     | → | +5V    |    |         | Q2 | Power to 5 wand LEDs                |
|                                    |   | ↕      | CAP 100uf |  |    | Capacitor across the +/- power wires |
| <font color="red">Red</font>       | → | Ground |    |         | Q2 | Ground to 5 wand LEDs               |
| <font color="yellow">Yellow</font> | → | 470Ω   | →  | Pin D10 | Q2 | Data input for addressing LEDs      |

## Component Fitment

As noted at the start of this guide the space inside of the gun box for the wand is extremely limited. Creative placement of any protoboards, the Arduino, WAV Trigger, and speaker (if upgraded) will be left to each individual. As a matter of advice, placing the WAV Trigger under the wand barrel is an advised option with other items "stacked" above and beside the spring-loaded mechanism.

### (See the alternate component fitment section below for more information on the stacking option) ###

For another fitment option, the WAV Trigger can also fit in the opposite half of the gun box if the barrel jack is removed--this is to avoid the spring which attaches to the wand barrel.

### (See the Audio Output addendum for more information on the opposite half gun box mounting solution)
[Audio Output Addendum](SOUND.md)

Below is a view of the wand internals showing the microcontroller, a PCB for connections, and many of the wires which extend the stock wiring. This solution is using several of the included STL files: a small spacer is meant to fit under the barrel if you removed some of the plastic from the old controller mount; another 3D printed part is meant to protect the wiring from intruding on the barrel latching mechanism while still allowing the wires to connect to the barrel. And lastly, a mini USB cable has been routed through the casing to allow updating of the Arduino without removing the cover--in this case the vibration motor had already been removed to save space and make this area more accessible.

![](images/WandUSB.jpg)

## Alternate component fitment

This solution requires removing plastic within the wand, and printing some of the included STL mount files and stacking the PCB board then microcontroller on top of the WAV Trigger. Everything is then screwed into one of the remaining holes in the wand body.
![](images/WandMountAlt1.jpg)
![](images/WandMountAlt2.jpg)
![](images/WandMountAlt3.jpg)

## Mega2560 Pro Mini Pinout Reference

![](images/Mega-2560-Pro-Pinout.jpg)
