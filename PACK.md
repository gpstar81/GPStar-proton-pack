# Pack Setup

## Reference

PN2222  - NPN Bipolar Transistor

| Name | Description |
|---|---|
| NPN E  | Emitter |
| NPN B | Base |
| NPN C | Collector |

![](images/NPNTransistor.png)

| Code | Description |
|---|---|
| 1N4001 (Diode) | S = striped end |
| SW1 | Red power switch. |
| SW6 | Alarm ribbon cable switch |
| SW4 | Vibration Switch (enable or disable vibration motors) |
| SW3 | Mode Switch (1984/2021) |
| SW-CYC | Cyclotron Direction Switch |
| SW-SMOKE | Smoke Switch (enable or disable smoke) |
| JP3 | PowerCell LEDs |
| JP4 | Cyclotron Lid LEDs |
| ROT | Rotary Encoder. Volume control from the crank generator knob |
| JP4-BR | Cyclotron Lid Switch |
| M1 | Vibration motor (pack) |
| LED-R | Red LED for cyclotron switch plate (OPTIONAL) (2 of them) |
| LED-Y | Yellow LED for cyclotron switch plate (OPTIONAL) (2 of them) |
| LED-G | Green LED for cyclotron switch plate (OPTIONAL) (2 of them) |
| LED-GSW | Green LED for mode year switch (OPTIONAL) |
| LED-YSW | Yellow LED for vibration switch (OPTIONAL) |
| NEO-CYC | 8 NeoPixel jewels chained together (OPTIONAL) (8 jewels) |
| SMOKE1 | This is a standard 5V Air/Vacuum pump motor I use. (see below for more info) |
| SMOKE2 | This is a standard 5V Air/Vacuum pump motor I use. (see below for more info) |

![](images/PackController.jpg)

**Wire colour reference on this page:**

> (B) = Black wire
> 
> (R) = Red wire
> 
> (Y) = Yellow wire
>   
> (G) = Green wire
>   
> (O) = Orange wire
>   
> (BL) = Blue wire
>   
> (BR) = Brown wire
>   
> (+) = Referencing a positive voltage wire
>   
> (-) = Referencing a ground wire.

**Connector wire references from the Haslab wires in the pack.**

|  |  |  |  |  |  |  |
|---|---|---|---|---|---|---|
| Haslab Connector Colour | (white) | (red) | (blue) | (white) | (red) | (green) |
| Haslab Name | SW1 | SW6 | SW4-SW3 | JP3 | JP4 | M1 |
| Wire colours & order | R-B | R-B | R-G-BL-G | O-Y-B-G | R-Y-B-BR | R-B |

## Circuit Schematic

Half size Bread Board ([1609 Adafruit](https://www.adafruit.com/product/1609)) or any other brand.

| **-** | **+** | **-** | **A** | **B** | **C** | **D** | **E** |  | **F** | **G** | **H** | **I** | **J** | **+** | **-** |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| **1** | B | D |  | B | WAND (+) | BAT (+) | A |  | D | BAT (-) | WAND (-) | C |  | A | C |
| **2** | Wav Trig (5v in) | Wav Trig (-) |  |  |  |  |  |  |  |  |  |  |  |  |  |
| **3** |  | I | I | SW1 (B) | SW6 (B) | SW4 (R) | SW3(G) |  |  |  |  |  |  |  |  |
| **4** |  | J | J | SW-CYC (B) | SW-SMOKE (B) | ROT (2) |  |  |  |  |  |  |  |  |  |
| **5** |  | K | K | LED-R (-) | LED-R (-) | LED-Y (-) | LED-Y (-) |  |  |  |  |  |  |  |  |
| **6** |  | L | L | LED-G (-) | LED-G (-) | LED-GSW (-) | LED-YSW (-) |  |  |  |  |  |  |  |  |
| **7** | Mega 5V + | Mega Ground |  |  |  |  |  |  |  |  |  |  |  |  |  |
| **8** | JP3 (R) | JP3 (B) | JP3 (Y) |  |  |  | 470 Ω | ↔ | 470 Ω | PIN 53 |  |  |  |  |  |
| **9** |  |  | JP3 (G) | JP4 (Y) |  |  |  |  |  |  |  |  |  |  |  |
| **10** | JP4 (R) | JP4 (B) |  |  | JP4 (BR) | PIN 51 |  |  |  |  |  |  |  |  |  |
| **11** | NEO-CYC (pwr) | NEO-CYC (gnd) | NEO-CYC (in) |  |  |  | 470 Ω | ↔ | 470 Ω | PIN 13 |  |  |  |  |  |
| **12** |  |  |  |  |  |  |  |  |  |  |  |  |  | Q |  |
| **13** |  |  |  |  | 1N4001 (s) | UNUSED | Q |  |  |  | 1N4001 (s) | M1 (R) | E | E |  |
| **14** |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| **15** |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| **16** |  |  | NPN C |  | 1N4001 | UNUSED |  |  | NPN C |  | 1N4001 | M1 (B) |  |  |  |
| **17** |  |  | NPN B |  | 330 Ω (?) |  |  |  | NPN B |  | 330 Ω |  |  |  |  |
| **18** |  |  | NPN E |  |  |  | P |  | NPN E |  |  |  | F |  | F |
| **19** |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| **20** |  |  |  |  | 330 Ω (?) |  | PIN 33 |  |  |  | 330 Ω |  | PIN 45 |  |  |
| **21** |  |  |  |  |  |  |  |  |  |  |  |  |  | M | P |
| **22** |  |  |  |  | 1N4001 (s) | SMOKE2 (+) | M |  |  |  | 1N4001 (s) | SMOKE1 (+) | G | G |  |
| **23** |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| **24** |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| **25** |  |  | NPN C |  | 1N4001 | SMOKE2 (-) |  |  | NPN C |  | 1N4001 | SMOKE1 (-) |  |  |  |
| **26** |  |  | NPN B |  | 330 Ω |  |  |  | NPN B |  | 330 Ω |  |  |  |  |
| **27** |  |  | NPN E |  |  |  | O |  | NPN E |  |  |  | H |  | H |
| **28** |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
| **29** |  |  |  |  | 330 Ω |  | PIN 35 |  |  |  | 330 Ω |  | PIN 39 |  |  |
| **30** |  |  |  |  |  |  |  |  |  |  |  |  |  |  | O |

|  |  |
|---|---|
| BAT (+) | 5V from your power source |
| BAT (-) | Ground from your 5V power source. |
| WAND (+) | Runs to your wand (5V) + power rail [on the ¼ board](https://www.adafruit.com/product/1608) (see wand section) |
| WAND (-) | Runs to your wand ground rail on the ¼ board (see wand section) |

Bread board diagram reference. 
When you see a letter or code in a cells above, it references where it needs to be connected or what needs to be connected there.

Reference the photos below for more information.

Example #1:
You see the letter B in +1 on the breadboard and again on B1. You need to connect a wire to these 2 points.

Example #2:
E1 you have a A, you need to run a wire from E1 to +1 on the other side of the breadboard on that + rail with the A reference in the cell above.

Example #3:
E8 you have one end of 470ohm resistor. You need to put the other end of this resistor onto F8 on the board.

Example #4:
H16 you have one end of a 1n4001 diode. You need to then put the STRIPED end of the diode to H13 on the board.

Example #5:
C29 I have one end of a 330 resistor, and the other end runs to C26.

Example #6:
G10 says PIN 53. You need to run a wire from this point to PIN 53 to the Arduino Mega.

![](images/Breadboard.jpg)

## Wires to the Arduino MEGA

|  |  |  |  |
|---|---|---|---|
| SW1 (R) → Pin 31 |  |  |  |
|  | SW6 (R) → Pin 23 |  |  |
|  | SW4 (G) → Pin 27 |  |  |
|  | SW3 (BL) → Pin 25 |  |  |
| (Optional) | SW-CYC (R) → Pin 29 | (Optional) | LED-R (+) → 140 Ω → Pin 4 |
| (Optional) | SW-SMOKE (R) → Pin 37 | (Optional) | LED-R (+) → 140 Ω → Pin 5 |
|  | ROT (3) → Pin 2 | (Optional) | LED-Y (+) → 140 Ω → Pin 6 |
|  | ROT (1) → Pin 3 | (Optional) | LED-Y (+) → 140 Ω → Pin 7 |
|  | MEGA TX3 → WAV TRIGGER RX | (Optional) | LED-G (+) → 90 Ω → Pin 8 |
|  | MEGA RX3 → WAV TRIGGER TX | (Optional) | LED-G (+) → 90 Ω → Pin 9 |
|  | MEGA TX2 → WAND RX | (Optional) | LED-GSW (+) → 90 Ω → Pin 10 |
|  | MEGA RX2 → WAND TX | (Optional) | LED-YSW (+) → 140 Ω → Pin 11 |
| (Optional) | Pin 33 → Goes 5V high, is timed to go off during over heat sequence. You can hook up any device to this. I used a fan for testing purposes. Future plan is run a DC motor off this that is connected to the n-filter cone. See YELLOW SECTION on the chart above for reference. What you put in this section depends on what device you hook up to the Mega if anything at all. DO NOT DRAW MORE THAN 40MA from a pin. Use a transistor setup if you need more power. | NOTE: If you change LED colours, use the appropriate resistor. The resistors I reference here are the minimum required for these particular 5mm LED diodes. You can go up to the nearest resistor value if you do not have a 140. For example I used 150 since I do not have any 140’s in my resistor kit, and 100 instead of 90 for my green LEDs. |  |
| (Optional) | Pin 35 → Goes 5V high, is timed to go off occasionally during the wand firing sequences. You can hook up any device to this. I used DC pump motor for drawing smoke from a e-vape pen and push it out of the booster tube. It is referenced as the smoke_booster in the pack code. ‘See matching colour section in chart above. What you hook up here depends on what device you intend to run. DO NOT DRAW MORE THAN 40MA from a pin. Use a transistor setup if you need more power. |  |  |
| (Optional) | Pin 39 → Goes 5V high, is timed to go off during over heat sequence and occasionally during the wand firing sequences. You can hook up any device to this. I used DC pump motor for drawing smoke from a e-vape pen and run it to the n-filter. This is referenced as smoke_pin in the pack code. See matching colour section in chart above. What you hook up here depends on what device you intend to run. DO NOT DRAW MORE THAN 40MA from a pin. Use a transistor setup if you need more power. |  |  |

## Rotary Encoder

Rotary encoder pin layout reference to be used for volume control.

ROT: 1-2-3

(viewed from 3 pins facing you) (See rotary encoder photo below for more detail)

![](images/RotaryEncoder.jpg)

## Pack Options

The following are *OPTIONAL* builds for use within the Proton Pack.

 - [Cyclotron Lights](CYCLOTRON.md)
 - [N-Filter Vent Light](NFILTER.md)
