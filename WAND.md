# Neutrona Wand Setup

This guide begins with some explanations for common items you will encounter during the upgrade process. Note that it will be necessary to upgrade the hose and conductors between the pack and wand. This exercise is left to individual preferences on connectors and style of replacement, though you will need 4 conductors at a minimum (+5V, Ground, and TX/RX for communications.

The space within the Neutron Wand is extremely limited, though multiple components must be fit into this space. To maximize what is available, some wires from the original controller (PCB) will be directly soldered onto the Arduino Nano. Where resistors or inline components are required, these may be soldered between the wire and Arduino Nano to reduce need for an additional protoboard. If needed, a [a ¼ protoboard from Adafruit](https://www.adafruit.com/product/1608) or any similar size from another manufacturer will be used.

You will find that the wand poses some challenges due to confined spaces and fragile wires to all the stock components. The following are suggestions to help you minimize damage to existing wiring, unless you plan to remove all stock component and run new wires.

- Remove what you can of any plastic covers over the switches internally, and use a hot glue gun to dribble some glue over the ends of the toggles, rotary encoder, and any other wire connections to reinforce them as they stand.
- Gently separate any wires that are bonded together, allowing about 1" of excess for stripping.
- Use a mechanical wire stripper to ensure you quickly strip back the sheathing for each.
- Use silicone-coated stranded wiring (22AWG down to 26AWG) to allow for maximum flexibility, and solder on extensions to every wire in the wand.
- Use heat-shrink tubing to reinforce the connections and to make a better bond with the stock wire to prevent breakage.
- Continue the extended wire to either a PCB or the Nano for soldering, making sure that ALL of the strands of wire make it through the holes.
- It's too easy to have some strands that get missed which could cause a short--check your connections after soldering. A magnifying glass and good lighting is your friend!

## Original Connection References

You may refer to this PCB drawing and the following table to help identify components in the Neutrona Wand.

![](images/WandPCBs.png)

*[Wand Reference Diagrams](https://github.com/Tomdf/Ghostbusters/tree/main/Haslab%20Pack%20Illustrations/Neutrona_Wand_Illustration) by Tomdf*

| Name | Socket | Color | Haslab Pin | Notes |
|------|--------|-------|------------|-------|
| J3   |        | <font color="brown">Brown</font> | D2 | Bar Graph LED's (5) |
|      |        | <font color="red">Red</font> | D3 |  |
|      |        | <font color="orange">Orange</font> | D4 |  |
|      |        | <font color="yellow">Yellow</font> | D5 |  |
|      |        | <font color="green">Green</font> | D6 |  |
|      |        | <font color="blue">Blue</font> | VDD |  |
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
| Q2   | JST-PH | <font color="blue">Blue</font> | VCC | Neopixels for wand tip effects (5 total) |
|      |        | <font color="yellow">Yellow</font> | Data |  |
|      |        | <font color="red">Red</font> | Gnd |  |

## WavTrigger Connections

These are the same connections which are used for the FTDI breakout board to program the WavTrigger. Use pins and sockets for an easy connection should a firmware upgrade be required in the future.

| WavTrigger | Connection |
|------------|------------|
| 5Vin       | +5V        |
| GND        | Ground     |
| RX         | PIN D9     |
| TX         | PIN D8     |
| SPKR+      | Speaker +  |
| SPKR-      | Speaker -  |

## Arduino Nano - Pin Reference vs. Hasbro Controller

The following is a diagram of the Arduino Nano pins from left and right, when oriented with the USB connection facing up (north).

| Connection    | Nano (L) |   | Nano (R) | Connection |
|---------------|----------|---|----------|------------|
| D8 (Red)      | D13  |   | D12  | D7 (Red)      |
|               | 3V3  |   | D11  | Motor-NPN     |
|               | REF  |   | D10  | Q2 (Yellow)   |
| SW2 (Red)     | A0   |   | D9   | WavTrigger RX |
| D2 (Brown)    | A1   |   | D8   | WavTrigger TX |
| D3 (Red)      | A2   |   | D7   | ROT (Yellow)  |
| D4 (Orange)   | A3   |   | D6   | ROT (Red)     |
| D5 (Yellow)   | A4   |   | D5   | D1 & D9 (Red) |
| D6 (Green)    | A5   |   | D4   | SW1 (Brown)   |
| SW6 (White)   | A6   |   | D3   | SW4 (Red)     |
| SW7 (Orange)  | A7   |   | D2   | SW45 (White)  |
|               | 5V   |   | GND  |               |
|               | RST  |   | RST  |               |
| Ground (Pack) | GND  |   | RX0  | TX (to Pack)  |
| +5V (Pack)    | VIN  |   | TX1  | RX (to Pack)  |

### Connections by Component

**Toggles and Buttons**

| LOWER RIGHT TOGGLE → SW1         |   |        |     | Nano Pin | Hasbro Ref | Notes |
|----------------------------------|---|--------|-----|----------|-----|----------------------------------------|
| <font color="brown">Brown</font> | → | Ground |     |          | SW1 | Shouldn’t matter which wire goes where |
| <font color="brown">Brown</font> | → | →      | →   | Pin D4   | SW1 | Shouldn’t matter which wire goes where |

| UPPER RIGHT TOGGLE → SW2         |   |        |     | Nano Pin | Hasbro Ref | Notes |
|----------------------------------|---|--------|-----|----------|-----|----------------------------------------|
| <font color="red">Red</font>     | → | Ground |     |          | SW2 | Shouldn’t matter which wire goes where |
| <font color="red">Red</font>     | → | →      | →   | Pin A0   | SW2 | Shouldn’t matter which wire goes where |

| SW45/SW4 Connector: SW45 = Intensify / SW4 = Activate |   |        |    | Nano Pin | Hasbro Ref |
|-------------------------------------------------------|---|--------|----|----------|------------|
| <font color="gray">White</font> | → | Ground |    |        | SW45 |
| <font color="gray">White</font> | → | →      | →  | Pin D2 | SW45 |
| <font color="red">Red</font>    | → | Ground |    |        | SW4  |
| <font color="Red"> Red </font>  | → | →      | →  | Pin D3 | SW4  |

**Special Switches**

These switches utilize a "pull down" resistor to measure the difference in voltage when the button os switch is depressed. Each switch will get +5V power on one wire, while the other wire will connect to both a 10K resistor and an Arduino Nano pin. The other side of the resistor will connect to ground. This allows the pin to go to ground (low) when power is not supplied (read: the switch is not depressed), and the pin gets when +5V is present when the switch is depressed.

| MODE SWITCH → SW6 |    |            |    | Nano Pin | Hasbro Ref |                                       |
|-------------------|----|------------|----|----------|------------|---------------------------------------|
| <font color="gray">White</font> | → | +5V    |   |        | SW6 | Shouldn’t matter which wire goes where |
| <font color="gray">White</font> | → | 10k Ω  | → | Pin A6 | SW6 | Shouldn’t matter which wire goes where |
|                                 |   | ↕      |   |        |     |                                        |
|                                 |   | Ground |   |        |     | Need to pull to ground (for reference) |

| BARREL EXTENSION SWITCH → SW7 |   |        |    | Nano Pin | Hasbro Ref |                                   |
|-------------------------------|---|--------|----|----------|------------|-----------------------------------|
| <font color="orange">Orange</font> | → | +5V    |   |        | SW7 | Shouldn’t matter which wire goes where |
| <font color="orange">Orange</font> | → | 10k Ω  | → | Pin A7 | SW7 | Shouldn’t matter which wire goes where |
|                                    |   | ↕      |   |        |     |                                        |
|                                    |   | Ground |   |        |     | Need to pull to ground (for reference) |

The rotary encoder is similar to that used on the Proton Pack. It requires a common ground connection and sends data via the A/B signal wires to indicate which direction it was turned.

| ROTARY ENCODER (ROT) → SW3 |   |              |    | Nano Pin | Hasbro Ref |
|----------------------------|---|--------------|----|----------|------------|
| <font color="brown">Brown</font>   | → | Ground |    |        | V+ |
| <font color="red">Red</font>       | → | →      | →  | Pin D6 | SW3 |
| <font color="orange">Orange</font> | → | →      | →  | Pin D7 | SW3 |

**Motors**

Considered optional as this can be left out if desired, and without impact to normal operation.

| VIBRATION&nbsp;MOTOR |   |            | Motor Wire | Nano&nbsp;Pin | Notes                                     |
|----------------------|---|------------|------------|---------|-------------------------------------------------|
|                      |   | 1N4001 (s) | <font color="red">Red</font> | +5V     | 1N4001 diode (s = striped end of the diode) |
| NPN C                | → | 1N4001     | Black                        |         | NPN* = PN2222                               |
| NPN B                | → | 330 Ω      | →                            | Pin D11 |                                             |
| NPN E                | → | →          | →                            | Ground  |                                             |

**LED's**

| WAND-TUBE → Q2 |   |              |   | Nano Pin | Hasbro Ref | Notes                                  |
|----------------|---|--------------|---|----------|------------|----------------------------------------|
| <font color="blue">Blue</font>     | → | +5V      |   |         | Q2 | Power to 5 wand LED's           |
| <font color="yellow">Yellow</font> | → | 470Ω     | → | Pin D10 | Q2 | Data input for addressing LED's |
| <font color="red">Red</font>       | → | Ground   |   |         | Q2 | Ground to 5 wand LED's          |

| BARGRAPH → J3                      |   |       |    | Nano Pin | Hasbro Ref |
|------------------------------------|---|-------|----|----------|------------|
| <font color="blue">Blue</font>     | → | +5V   |    |          | VDD  |
| <font color="green">Green</font>   | → | 140 Ω | →  | Pin A5   | D6   |
| <font color="yellow">Yellow</font> | → | 140 Ω | →  | Pin A4   | D5   |
| <font color="orange">Orange</font> | → | 140 Ω | →  | Pin A3   | D4   |
| <font color="red">Red</font>       | → | 140 Ω | →  | Pin A2   | D3   |
| <font color="brown">Brown</font>   | → | 140 Ω | →  | Pin A1   | D2   |

Note for Bargraph: If you want to replace LEDs, the bargraph uses 3mm LED diodes.
**Use the appropriate resistor for your replacement LED's!**

| VENT LIGHT LED BOARD → D7 & D8  |   |      |    | Nano Pin | Hasbro Ref |                                        |
|---------------------------------|---|------|----|----------|------------|----------------------------------------|
| <font color="red">Red</font>    | → | +5V  |    |          | VDD        |                                        |
| <font color="gray">White</font> | → | 90 Ω | →  | Pin D12  | D7         | Blinking white led on top of the wand  |
| Black                           | → | 90 Ω | →  | Pin D13  | D8         | White led inside the wand (vent light) |

*NPN Bipolar Transistor. Reference Pack page for more info.

For the SLO-BLO and Front-Left LED which share on 1 pin from the Nano. The spec sheet for the nano states a max 40mA draw on a pin. With 2 LEDs it is at that threshold. I have been running this setup for a month now and have not blown out the pin. I prefer this solution as it requires less space than using a transistor method.

| SLO-BLO → D1         |    |       |    | Nano Pin     | Hasbro Ref | Notes |
|----------------------|----|-------|----|--------------|----|-------------------------------------|
| <font color="red">Red</font> | → | 140 Ω | → | Pin D5 | D1 | Nano pin shared with FRONT LEFT LED |
| Black                        | → | →     | → | Ground | D1 |                                     |

| FRONT LEFT LED → D9 |    |       |    | Nano Pin     | Hasbro Ref | Notes |
|---------------------|----|-------|----|--------------|----|-------------------------------------|
| <font color="red">Red</font>       | → | 140 Ω | → | Pin D5 | D9 | Nano pin shared with SLO-BLO |
| <font color="yellow">Yellow</font> | → | →     | → | Ground | D9 |                              |

OPTIONAL - See below for transistor method to drive the 2 LEDs from the same pin if you wish to use that method instead. See example breadboard setup photo below that you can replicate onto [a ¼ protoboard](https://www.adafruit.com/product/1608).

| NPN* = PN2222 |   | Slo-Blo (B) | Front Left LED (Y)| Nano Pin |
|---------------|---|-------------|-------------------|----------|
|               |   | 140 Ω       | 140 Ω             |          |
|               |   | ↕           | ↕                 |          |
| NPN C         | → | Ground      | ← (Ground)        |          |
| NPN B         | → | 1k Ω        | →                 | Pin D5   |
| NPN E         | → | Ground      |                   |          |

*NPN Bipolar Transistor. Reference Pack page for more info.

![](images/SloBloAlt.jpg)

## Component Fitment

As noted at the start of this guide the space inside of the gun box for the wand is extremely limited. Creative placement of any protoboards, the Arduino, WavTrigger, and speaker (if upgraded) will be left to each individual. As a matter of advice, placing the WavTrigger under the wand barrel is an advised option with other items "stacked" above and beside the spring-loaded mechanism. As an alternative option, the WavTrigger can also fit in the opposite half of the gun box if the barrel jack is removed--this is to avoid the spring which attaches to the wand barrel.

## Arduino Nano Pinout Reference

![](images/Arduino-nano-pinout.png)