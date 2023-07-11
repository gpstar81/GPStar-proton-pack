# Bill of Materials

This guide should provide a listing of minimal/required components as well as any optional items needed for the conversion.

Note that just about everything can be sourced at [Mouser](https://www.mouser.com/) or [Digi-Key](https://www.digikey.com/) among usual sites such as [Amazon](https://www.amazon.com/) or [Adafruit](https://www.adafruit.com/). The HasLab equipment makes use of both [JST-XH and JST-PH](https://en.wikipedia.org/wiki/JST_connector) connectors, so buy accordingly for your headers and plugs if you wish to continue using the stock connectors.

## Tools & Connectors

These items are considered optional but useful. If you are using the ready-to-use PCB's you may only be interested in the pin crimping tool and JST connectors. Otherwise, if soldering is in your skillset you may consider the TS101 as a worthwhile upgrade for easier soldering.

| Desc | Use | Link |
|------|-----|------|
| Pin Crimping Tool | Necessary for crimping wires for additional JST connectors which may be needed for optional components. | [https://a.co/d/3M5z2kk](https://a.co/d/3M5z2kk) |
| JST-XH Connector Set | Assorted JST-XH connectors for optional components in the Proton Pack. | [https://a.co/d/7dkYd1O](https://a.co/d/7dkYd1O) |
| JST-PH Connector Set | Pre-crimpted 2-pin connectors for optional components in the Neutrona Wand. | [https://a.co/d/0ckywqT](https://a.co/d/0ckywqT) or [https://a.co/d/8fkU1py](https://a.co/d/8fkU1py) |
| TS101 Soldering Iron | Incredibly good and modern soldering iron. Heats up in seconds and fully customizible with auto-sleep function. Absolutely worth it for tight spaces and ease of use. | [Mini Electric Soldering Iron Kit TS101](https://a.co/d/irkoB22) |
| Lead-free Solder | You will need good quality solder and Kester makes a narrow diameter which is perfect for small spaces. | [Kester Lead Free Solder, 0.031" Diameter, 0.43 oz.](https://a.co/d/205d0rz) |
| Solder Sucker | Very helpful to clean up and desolder pins etc | [https://www.adafruit.com/product/1597](https://www.adafruit.com/product/1597) |

## Common Components

| Qty | Desc | Use | Link |
|-----|------|-----|------|
| 2   | Robertsonics WavTrigger | REQUIRED - 1 for the pack and 1 for the wand. You can find these on mouser as well.                                                     | [From SparkFun](https://www.sparkfun.com/products/13660)<br/>[From Amazon](https://a.co/d/dNLmVyo) |
| 2   | Micro SD Cards. | REQUIRED - I recommend Sandisk Extreme A2 U3 V30 cards. I am using a 64gb card in the Wav Triggers, a 32GB or 16GB is more than enough. | [https://www.amazon.fr/SanDisk-Extreme-microSDXC-mobile-Performance/dp/B09X7D3Y59/](https://www.amazon.fr/SanDisk-Extreme-microSDXC-mobile-Performance/dp/B09X7D3Y59/) [https://www.amazon.com/SanDisk-Extreme-Mobile-Gaming-microSD/dp/B089DNFTQF/](https://www.amazon.com/SanDisk-Extreme-Mobile-Gaming-microSD/dp/B089DNFTQF/) |
| 1   | FTDI Basic (5V) | RECOMMENDED - Used to update the WavTrigger firmware | [https://www.sparkfun.com/products/9716](https://www.sparkfun.com/products/9716) |
| 1&nbsp;(Package) | 1n4001 Diode (10 Pack) | Minimum 2 are REQUIRED, one for each vibration motor (pack and wand). Though it's good to have additional on-hand for other additions. | [https://www.adafruit.com/product/755](https://www.adafruit.com/product/755) |
| 1&nbsp;(Package) | 100uF 16V Electrolytic Capacitors (10 Pack) (or equivalent) | Minimum 3 are RECOMMENDED, one for the PowerCell, Cyclotron Lid LEDs and Wand Barrel LEDs. Though it's good to have additional on-hand for other additions. | [https://www.adafruit.com/product/2193](https://www.adafruit.com/product/2193) |
| 1&nbsp;(Package) | Resistors (Various) | REQUIRED - Various resistors for the project | [https://www.amazon.fr/gp/product/B08FD1XVL6/](https://www.amazon.fr/gp/product/B08FD1XVL6/) [https://www.amazon.com/BOJACK-Values-Resistor-Resistors-Assortment/dp/B08FD1XVL6/](https://www.amazon.com/BOJACK-Values-Resistor-Resistors-Assortment/dp/B08FD1XVL6/) |
| 1&nbsp;(Package) | NPN Bipolar Transistors (PN2222) (10 Pack) | Minimum 2 are REQUIRED, one for each vibration motor (pack and wand) | [https://www.adafruit.com/product/756](https://www.adafruit.com/product/756) |
| 1&nbsp;(Package) | Heat Shrink Tubing | RECOMMENDED - To protect exposed wire soldering work. | [https://www.amazon.fr/gp/product/B071D7LJ31/](https://www.amazon.fr/gp/product/B071D7LJ31/)<br/>[https://www.amazon.com/560PCS-Heat-Shrink-Tubing-Eventronic/dp/B072PCQ2LW/](https://www.amazon.com/560PCS-Heat-Shrink-Tubing-Eventronic/dp/B072PCQ2LW/) |
| \-  | 26-22 AWG Spools of Stranded Wire | RECOMMENDED - Good for making connections, extending connectors, building your hose internal wiring, etc. (various colours). Must be stranded wire for flexibility, silicone to resist heat if using heat-shrink tubing. (The 22AWG box from Amazon comes with some heat-shrink tubing and mini zip ties.) | [Silicone Covered Stranded-Core Wire - Adafruit](https://www.adafruit.com/?q=Silicone+Cover+Stranded-Core+Wire+-+25ft+26AWG+&sort=BestMatch)<br/>[22AWG Silicone Hook Up Wire - Amazon](https://a.co/d/dkevHuc) |

## Proton Pack

| Qty | Desc | Use | Link |
|-----|------|-----|------|
| 1   | Arduino Mega 2560 Rev 3 | REQUIRED - You can find clones without headers, but this is what I used. | [From Mouser](https://www.mouser.com/ProductDetail/SparkFun/DEV-11061?qs=WyAARYrbSnb6i8Rlb84YYw%3D%3D)<br/>[From DigiKey](https://www.digikey.com/en/products/detail/arduino/A000067/2639006) |
| 1   | Mega Protoshield | OPTIONAL - If you get a Mega without headers, this is not required. The protoshield goes on top of the Mega and you can solder onto it directly. | [https://www.adafruit.com/product/192](https://www.adafruit.com/product/192) |
| 2   | Pump DC Motor | OPTIONAL - I am using 2 of these for generating smoke from a vape pen. Tubing can be found on the same link.                                                                                                                                                                                                  | [https://www.adafruit.com/product/4699](https://www.adafruit.com/product/4699) |
| 2   | Vape Pens | OPTIONAL - I am using 2 of these for generating smoke with the DC motors mentioned above.                                                                                                                                                                                                  | [https://www.joyetech.com/product/eroll-mac/](https://www.joyetech.com/product/eroll-mac/) |
| 8   | 5mm LED's | OPTIONAL - I am using 3 greens, 2 reds and 3 yellows for the cyclotron switch plate | [https://a.co/d/cxUqcuF](https://a.co/d/cxUqcuF)<br/>[https://amzn.eu/d/1bU37Od](https://amzn.eu/d/1bU37Od) |
| 8   | Cliplites | OPTIONAL - 3 greens, 2 reds and 3 yellows to hold the cyclotron switch plate leds into the cyclotron switch plate. Links to red, green and yellow ones included | [https://www.mouser.fr/ProductDetail/593-2800G](https://www.mouser.fr/ProductDetail/593-2800G) [https://www.mouser.fr/ProductDetail/593-2800R](https://www.mouser.fr/ProductDetail/593-2800R) [https://www.mouser.fr/ProductDetail/593-2800Y](https://www.mouser.fr/ProductDetail/593-2800Y) |
| 1   | NeoPixel Jewel WS2812 5050 RGB | OPTIONAL - For the n-filter light. You can find them anywhere, such as adafruit, mouser etc. | [https://fr.aliexpress.com/item/32822441734.html](https://fr.aliexpress.com/item/32822441734.html) |
| 1   | 35 LED Pixel Ring (96mm OD) | OPTIONAL - For the inner cyclotron. The Arduino software supports other sizes of rings. 35 pixel ring is the default and recommended to avoid altering the software. Although, configuring an alternative ring size is trivial for this particular item. | [Single Ring](https://a.co/d/fAmEV3r) or [Multi-Ring Kit](https://a.co/d/hJlh9vD) |
| 1   | 40 LED Pixel Ring (132mm OD) | OPTIONAL - For the cyclotron lid, which MUST be exactly 40 pixels to fit properly behind the lenses. If you wish to purchase 1 kit for multiple uses, this contains a 112mm ring with 32 LED's which may be a suitable for use with the inner cyclotron (will require a small configuration change to the software). | [Multi-Ring Kit](https://a.co/d/40wMl8T) |
| 1   | ½ Size Proto PCB Board | RECOMMENDED - You can use an Adafruit one or similar from any other manufacturer. I am using a some from GeekPi. | [https://www.adafruit.com/product/1608](https://www.adafruit.com/product/1608)<br/>[Gold Plated PCB Solderable Breadboards](https://a.co/d/8E5rik4) |
| 2   | SPST Switches | OPTIONAL - I just use cheap ones from Amazon. I use these for optional switches such as the cyclotron direction rotation switch, smoke enable switch, etc. | [https://www.amazon.fr/gp/product/B07RQ4ZBQJ/](https://www.amazon.fr/gp/product/B07RQ4ZBQJ/) [https://www.amazon.com/MTS-101-Position-Miniature-Toggle-Switch/dp/B0799LBFNY/](https://www.amazon.com/MTS-101-Position-Miniature-Toggle-Switch/dp/B0799LBFNY/) |
| 1   | Rotary Encoder | OPTIONAL - I use a standard Rotary Encoder with 24 positions. I use this instead of the Haslab one which is 5 positions I believe. I did not test out the Haslab one with my wiring, though I have no reason to believe it would not work the same. This is for the Volume control in the crank generator. | [https://www.adafruit.com/product/377](https://www.adafruit.com/product/377)<br/>[360 Degree Rotary Encoder](https://a.co/d/26nl9OB) |

## Neutrona Wand

| Qty | Desc | Use | Link |
|-----|------|-----|------|
| 1   | Arduino Nano | REQUIRED - I used a 3 pack of clone ones from Elegoo, cheaper and have no headers. | [https://amzn.eu/d/25FUZeS ](https://amzn.eu/d/25FUZeS)<br/>[https://a.co/d/hLWMKby](https://a.co/d/hLWMKby) |
| 1   | ¼ Size Proto PCB board | RECOMMENDED - I use one from Adafruit. Can be any other PCB as you see fit. | [https://www.adafruit.com/product/1608](https://www.adafruit.com/product/1608)<br/>[Gold Plated PCB Solderable Breadboards](https://a.co/d/8E5rik4) |
| 1   | Barmeter 28 Segment Bargraph | OPTIONAL - It is possible to replace the Hasbro 5 LED bargraph with a Barmeter 28 Segment Bargraph. <br><br>Please note the part number: ***BL28Z-3005SA04Y*** | [https://www.aliexpress.us/item/3256803648954593.html](https://www.aliexpress.us/item/3256803648954593.html) |
| 1   | Adafruit 16x8 LED Matrix Driver Backpack - HT16K33 Breakout | OPTIONAL - If you intend to replace the stock Hasbro bargraph with a Barmeter 28 Segment Bargraph, you will require this. | [https://www.adafruit.com/product/1427](https://www.adafruit.com/product/1427) |
| 2   | 2.9 x 9.5mm screw | OPTIONAL - Used to hold the adafruit ¼ pcb onto the 3d printed bracket, and the key lock for the nano bracket. |
| 1   | M3 x 25 allen screw | OPTIONAL - Used to hold the entire bracket setup and wav trigger onto the screw hole in the wand. |
| 5   | 3mm LED Diodes | OPTIONAL - If you have problems with your Hasbro bargraph LEDs, they are just standard 3mm LED diodes. | [https://www.amazon.com/Chanzon-100pcs-colors-Emitting-Assorted/dp/B01AUI4W5U](https://www.amazon.com/Chanzon-100pcs-colors-Emitting-Assorted/dp/B01AUI4W5U)
| 1   | 5mm Flat-Top LED | OPTIONAL - For the wand barrel tip, which has a 160 degree viewing angle and is super bright. | [https://a.co/d/9g8nXPj](https://a.co/d/9g8nXPj) |
