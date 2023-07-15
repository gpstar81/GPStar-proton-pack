# Wand PCB Hookup

This guide is part of the kit approach to providing a minimally-invasive upgrade to the stock HasLab controllers. For the Neutrona Wand all available connections for JST-PH wiring is present, though a significant amount of wire-cutting will be required to separate the stock controller and re-attach using terminal blocks on the new PCB.

IMPORTANT: This solution still requires 2 of the WavTrigger devices for each of the pack and wand, whether supplied as part of a kit or if obtained on your own. The design of the PCB's will allow for completing the power and communication connections.

![](images/WandPCB-Labels.png)

## Neutrona Wand

Connections for the wand should be made according to the table below.

- Ordering aligns with PCB labels or when viewed left-to-right with the connector keyhole at the bottom.
- Pins denoted A#/D# correspond to the internal code and connection to the controller chip.
- Ground may be designated as "GND" or simply "-".

### Stock Connectors (JST-PH)

| Label | Pins | Notes |
|-------|------|-------|
| 5V-IN | +/\- | Power from Proton Pack. **This MUST be a regulated 5V source!** |
| Q2 | VCC/D10/GND | Stock connection for addressable barrel LED's |
| SW45/SW4 | GND/D2/GND/D3 | Stock connection for the Intensify button and Activate toggle |
| SW6 | GND/A6 | Stock connection for the orange wand-end mode/alt switch |

### Stock Connectors (Terminal Blocks)

| Label/Pin | Color | Notes |
|-----------|-------|-------|
| A7 | <font color="orange">Orange</font> | Barrel extension switch (wire order does not matter) |
| GND | <font color="orange">Orange</font> | Barrel extension switch (wire order does not matter) |
| D8 | <font color="red">Red</font> | Slo-Blo VCC |
| GND | Black | Slo-Blo GND |
| D4 | <font color="brown">Brown</font> | Lower-right Toggle (wire order does not matter) |
| GND | <font color="brown">Brown</font> | Lower-right Toggle (wire order does not matter) |
| A0 | <font color="red">Red</font> | Upper-right Toggle (wire order does not matter) |
| GND | <font color="red">Red</font> | Upper-right Toggle (wire order does not matter) |
| D9 | <font color="red">Red</font> | Clippard LED (Top Left) VCC |
| GND | <font color="yellow">Yellow</font> | Clippard LED (Top Left) GND |
| R+ | <font color="red">Red</font> | Rumble (vibration) motor VCC |
| R- | Black | Rumble (vibration) motor GND |
| VCC | <font color="blue">Blue</font> | Power for stock bar graph |
| A5 | <font color="green">Green</font> | Stock bar graph LED 5 |
| A4 | <font color="yellow">Yellow</font> | Stock bar graph LED 4 |
| A3 | <font color="orange">Orange</font> | Stock bar graph LED 3 |
| A2 | <font color="red">Red</font> | Stock bar graph LED 2 |
| A1 | <font color="brown">Brown</font> | Stock bar graph LED 1 |
| D12 | <font color="gray">White</font> | Blinking top right LED |
| D13 | Black | White vent light LED |
| VL+ | <font color="red">Red</font> | VCC for top/vent lights |
| D7 | <font color="orange">Orange</font> | Rotary encoder B |
| D6 | <font color="red">Red</font> | Rotary encoder A |
| ROT- | <font color="brown">Brown</font> | Ground for rotary encoder |

### Special Connectors

| Label | Pins | Notes |
|-------|------|-------|
| PACK (Serial) | TX1/RX1 | Serial communication to the Proton Pack |
| AUDIO BOARD | GND/NC/VCC/TX/RX/NC | Communication and Power for the wands's Wav Trigger |
| ICSP | DO NOT USE! | Programming header for bootloader updates (reserved) |
| UART | See Below | Programming header for software updates (optional) |

For connecting the UART pins, use a suitable FTDI chip such as the same **FTDI Basic 5V** chip used for programming the WavTrigger. Connect each pin to the respective label to the FTDI chip.

- If using an FTDI to USB cable, the RTS pin will connect to the DTR board pin.
- The CTS pin on any FTDI device will not be used (connects to GND on the board).

### Optional Connectors

| Label | Pins | Notes |
|-------|------|-------|
| HAT1 | GND/D22 | Connection for top of the wand box hat LED.<br><br>The left (top in the photo) pin is GND, the right (bottom in the photo) pin is D22 which provides 5V and has a 140Ω resistor connected to it.<br><br>`Connector type: JST-PH`<br><br>`Do not draw more than 40mA from this connector.` |
| HAT2 | GND/D23 | Connection for barrel tip hat LED.<br><br>The left (top in the photo) pin is GND, the right (bottom in the photo) pin is D23 provides 5V and has a 140Ω resistor connected to it.<br><br>`Connector type: JST-PH`<br><br>`Do not draw more than 40mA from this connector.`|
| BARREL-LED | GND/D24 | Connection for white wand tip light.<br><br>The left (top in the photo) pin is GND, the right (bottom in the photo) pin is D24 which provides 5V and has a 100Ω resistor connected to it.<br><br>`Connector type: JST-PH`<br><br>`Do not draw more than 40mA from this connector.` |
| 5V-OUT | +/\- | Power for additional accessories.<br><br>`Connector type: JST-PH` |
| SCL/SDA | SCL/SDA | Expansion serial port using I2C.<br><br>`Connector type: JST-PH`|

**Note:** If using the [28-segment bargraph](BARGRAPH.md), power may be delivered via the 5V-OUT while the SCL/SDA serial connection will provide data/control.
