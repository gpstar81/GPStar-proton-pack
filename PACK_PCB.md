# Pack PCB Hookup

This guide is part of the kit approach to providing a minimally-invasive upgrade to the stock HasLab controllers. For the Proton Pack all stock connections can be made using the JST-XH connectors on all wiring.

IMPORTANT: This solution still requires 2 of the WavTrigger devices for each of the pack and wand, whether supplied as part of a kit or if obtained on your own. The design of the PCB's will allow for completing the power and communication connections.

![](images/PackPCB-Labels.png)

## Proton Pack

Connections for the pack should be made according to the table below.

- Ordering aligns with PCB labels or when viewed left-to-right with the connector keyhole at the bottom.
- Pins denoted D# correspond to the internal code and connection to the controller chip.
- Ground may be designated as "GND" or simply "-".

### Stock Connectors

| Label | Pins | Notes |
|-------|------|-------|
| BATTERY 5V-IN | +/\- | Power from battery. **This MUST be a regulated 5V source!** |
| Volume | D3/GND/D2 | Rotary encoder connection for pack volume adjusted via crank generator knob |
| SW1 | D31/GND | Stock connection for main switch under the Ion Arm |
| SW3/SW4 | GND/D27/D25/GND | Stock connection for cyclotron panel toggles, Vibration and Mode Switch |
| SW6 | GND/D23 | Stock connection for ribbon cable disconnection alarm (wire order does not matter) |
| M1 | VCC/GND | Stock connection for pack vibration motor (D45) |
| Power Cell | VCC/D53/GND/PCCYC | Stock connection for power cell LED's (continues to cyclotron lid) |
| Cyclotron | D43/GND/PCCYC/VCC | Stock connection cyclotron lens LED's (continues from power cell), includes lid sensor |

### Special Connectors

| Label | Pins | Notes |
|-------|------|-------|
| WAV TRIGGER | GND/NC/VCC/TX/RX/NC | Communication and Power for the pack's Wav Trigger |
| WAND 5V-OUT | +/\- | Power to the Neutrona Wand |
| WAND (Serial) | RX2/TX2 | Serial communication to the Neutrona Wand |
| ICSP | DO NOT USE! | Reserved header for bootloader updates (reserved) |
| UART | See Below | Programming header for software updates (optional) |

For connecting the UART pins, use a suitable FTDI chip such as the same **FTDI Basic 5V** chip used for programming the WavTrigger. Connect each pin to the respective label to the FTDI chip.

- If using an FTDI to USB cable, the RTS pin will connect to the DTR board pin.
- The CTS pin on any FTDI device will not be used (connects to GND on the board).

### Optional Connectors

| Label | Pins | Notes |
|-------|------|-------|
| SW-D | D29/GND | Cyclotron direction switch (wire order does not matter) |
| SM-T | GND/D37| Smoke Toggle switch to enable/disable smoke effects (wire order does not matter) |
| SM-1 | \-/+ | Smoke effects for N-Filter (D39) |
| FN-1 | \-/+ | Fan for N-Filter smoke (D33) |
| SM-2 | \-/+ | Smoke effects for Booster Tube (D35) |
| FN-2 | \-/+ | Fan for Booster Tube smoke (D50) |
| NEO-C | D13/VCC/GND | Neopixel LED ring for Inner Cyclotron "Cake" |
| P.RED | D4/GND/D5/GND | Cyclotron Panel LED's: 2x Red |
| P.YELLOW | D6/GND/D7/GND | Cyclotron Panel LED's: 2x Yellow |
| P.GREEN | D8/GND/D9/GND | Cyclotron Panel LED's: 2x Green |
| P.SWITCH | D10/GND/D11/GND | Cyclotron Panel LED's: 1x Green (Mode Indicator, D10) and 1x Yellow (Vibration Enabled, D11) |
| LED-W | D46/GND | Connection for a standalone white LED in N-Filter |
| 5V-OUT | +/\- | Power for additional accessories |
| SCL/SDA | SCL/SDA | Expansion serial port using I2C |
| RX1/TX1 | RX1/TX1 | Expansion serial port |

