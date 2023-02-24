# Wand Setup

## Arduino Nano - Pin Reference vs. Hasbro Controller

| Nano Pin | D12 | D11       | D10    | D9    | D8    | D7           | D6        | D5      | D4  | D3  | D2   | GND | RST | RX0       | TX1       |
|------------|-----|-----------|--------|-------|-------|--------------|-----------|---------|-----|-----|------|-----|-----|-----------|-----------|
| Hasbro Ref | D7  | MOTOR-NPN | J1 (R) | WT-RX | WT-TX | ROT (Yellow) | ROT (Red) | D9 & D1 | SW1 | SW4 | SW45 |     |     | TX (pack) | RX (pack) |

| Nano Pin | D13 | 3v3 | REF | A0  | A1 | A2 | A3 | A4 | A5 | A6  | A7  | 5V       | RST | GND          | VIN |
|------------|-----|-----|-----|-----|----|----|----|----|----|-----|-----|----------|-----|--------------|-----|
| Hasbro Ref | D8  |     |     | SW2 | D2 | D3 | D4 | D5 | D6 | SW6 | SW7 | 5V (PCB) |     | Ground (PCB) |

## Connection References

![](images/WandPCBs.png)

*[Wand Reference Diagrams](https://github.com/Tomdf/Ghostbusters/tree/main/Haslab%20Pack%20Illustrations/Neutrona_Wand_Illustration) by Tomdf*

| WAND-TUBE → J1 |    |              |    | Nano Pin | Hasbro Ref |
|----------------|----|--------------|----|----------|------------|
| Brown          | -> | Ground (PCB) |    |          | J1         |
| Red            | -> | 470Ω         | -> | Pin D10  | J1         |
| Yellow         | -> | 5V (PCB)     |    |          | J1         |


Note: If you want to replace LEDs, the bargraph uses 3mm LED diodes.

| BARGRAPH → J3 |  |          |     | Nano Pin | Hasbro Ref |
|----------------------|------------------------------------------------------------------|----------|-----|--------|-----|
| Blue                 | ->                                                               | 5V (PCB) |     |        | VDD |
| Green                | ->                                                               | 140 Ω    | ->  | Pin A5 | D6  |
| Yellow               | ->                                                               | 140 Ω    | ->  | Pin A4 | D5  |
| Light Yellow (beige) | ->                                                               | 140 Ω    | ->  | Pin A3 | D4  |
| Red                  | ->                                                               | 140 Ω    | ->  | Pin A2 | D3  |
| Dark Green (olive)   | ->                                                               | 140 Ω    | ->  | Pin A1 | D2  |

| VENT / OTHER SWITCH → SW2 / SW1 |    |              |     | Nano Pin | Hasbro Ref |                                                     |
|---------------------------------|----|--------------|-----|--------|-----|-----------------------------------------------------|
| ?                               | -> | Ground (PCB) |     |        | SW2 | Shouldn’t matter which wire goes where. (GND or A0) |
| ?                               | -> | ->           | ->  | Pin A0 | SW2 | Shouldn’t matter which wire goes where. (GND or A0) |
| Olive green?                    | -> | Ground (PCB) |     |        | SW1 | Shouldn’t matter which wire goes where. (GND or D4) |
| Olive green?                    | -> | ->           | ->  | Pin D4 | SW1 | Shouldn’t matter which wire goes where. (GND or D4) |

| ROTARY → SW3 | (Referenced as ROT on the chart at the top) |              |    | Nano Pin | Hasbro Ref |
|--------------|---------------------------------------------|--------------|----|----------|------------|
| Olive green  | ->                                          | Ground (PCB) |    |          | SW3 (V+) ? |
| Red          | ->                                          | ->           | -> | Pin D6   | SW3        |
| Yellow       | ->                                          | ->           | -> | Pin D7   | SW3        |

SW45 / SW4 Connector: SW45 = Intensify / SW4 = Activate

| WHITE    | WHITE       | RED      | RED         |
|----------|-------------|----------|-------------|
| (to gnd) | (to pin D2) | (to gnd) | (to pin D3) |

| MODE SWITCH → SW6 |    |              |    | Nano Pin | Hasbro Ref |                                        |
|-------------------|----|--------------|----|----------|------------|----------------------------------------|
| White             | -> | 5V (PCB)     |    |          | SW6        | Shouldn’t matter which wire goes where |
| White             | -> | 10k Ω        | -> | Pin A6   | SW6        | Shouldn’t matter which wire goes where |
|                   |    | Ground (PCB) |    |          |            |                                        |

| BARREL EXTENSION SWITCH → SW7 |    |              |    | Nano Pin | Hasbro Ref |                                        |
|-------------------------------|----|--------------|----|----------|------------|----------------------------------------|
| ?                             | -> | 5V (PCB)     |    |          | SW6        | Shouldn’t matter which wire goes where |
| ?                             | -> | 10k Ω        | -> | Pin A7   | SW6        | Shouldn’t matter which wire goes where |
|                               |    | Ground (PCB) |    |          |            |                                        |

| VIBRATION MOTOR |    |            | Motor Wires             | Nano Pin     | Notes                                                                   |
|-----------------|----|------------|-------------------------|--------------|-------------------------------------------------------------------------|
|                 |    | 1N4001 (s) | Red (wire from motor)   | 5V (PCB)     |                                                                         |
| NPN C           | -> | 1N4001     | Black (wire from motor) |              | NPN = PN2222 NPN Bipolar Transistor. Reference Pack page for more info. |
| NPN B           | -> | 330 Ω      | ->                      | Pin D11      | 1N4001 diode (s = the striped end of the diode)                         |
| NPN E           | -> | ->         | ->                      | Ground (PCB) |                                                                         |

| VENT LIGHT LED BOARD → D7 & D8 |    |          |    | Nano Pin | Hasbro Ref |                                        |
|--------------------------------|----|----------|----|----------|------------|----------------------------------------|
| Red                            | -> | 5V (PCB) |    |          | VDD        |                                        |
| White                          | -> | 90 Ω     | -> | Pin D12  | D7         | Blinking white led on top of the wand  |
| Black                          | -> | 90 Ω     | -> | Pin D13  | D8         | White led inside the wand (vent light) |

For Below: Regarding sharing 2 LED’s on 1 pin from the Nano. The spec sheet for the nano says a max 40mA draw on a pin. With 2 LEDs it is at that threshold. I have been running this setup for a month now and have not blown out the pin. I prefer this solution as it requires less space than using a transistor method. 

| SLO-BLO             |    |       |    | Nano Pin.    | Hasbro Ref | Notes |
|---------------------|----|-------|----|--------------|----|-------------------------------------|
| Red                 | -> | ->    | -> | Pin D5       | D1 | Nano pin shared with FRONT LEFT LED |
| Black               | -> | 140 Ω | -> | Ground (PCB) | D1 |                                     |

| FRONT LEFT LED → D9 |    |       |    | Nano Pin     | Hasbro Ref | Notes |
|---------------------|----|-------|----|--------------|----|-------------------------------------|
| Red                 | -> | 140 Ω | -> | Pin D5       | D9 | Nano pin shared with SLO-BLO        |
| Yellow              | -> | ->    | -> | Ground (PCB) | D9 |

OPTIONAL - See below for transistor method to drive the 2 LEDs from the same pin if you wish to use that method instead. See example breadboard setup photo below that you can replicate onto [a ¼ pcb board](https://www.adafruit.com/product/1608).

| Slo-Blo (B) | Front Left LED (Y) |              |
|-------------|--------------------|--------------|
|             |                    | 140 Ω        |
| NPN C       | ->                 | Ground       |
| NPN B       | ->                 | 1k Ω         |
| NPN E       | ->                 | Ground (PCB) |

![](images/SloBloAlt.jpg)

## Wav Trigger Connections

> 5V IN goes to the + rail on the PCB
> 
> GND goes to the ground rail on the PCB
> 
> WT-RX (RX Pin) goes to pin D9 on the Nano
> 
> WT-TX (TX Pin) goes to pin D8 on the Nano
> 
> SPKR+ goes to the + speaker wire
> 
> SPKR- goes to the – speaker wire
