# Smoke Configuration

Below are some examples of how to implement smoke into your Proton Pack.

There are 3 pins on the Mega configured to go HIGH during smoke effect phases. Smoke1 -> Pin 39, Smoke2 -> Pin 35 and Fan 1 - Pin 33. With a simple transistor setup, you can activate pumps, external smoke machine devices, etc.

**WARNING: DO NOT DRAW MORE THAN 40ma FROM A PIN. Use a transistor setup if you need more power.**

## Booster Tube Example
For this example, I used the following parts:

* 1 - Air Pump and Vacuum DC Motor - 4.5 V -> https://www.adafruit.com/product/4699
* 1 - Vape Pen -> https://www.joyetech.com/product/eroll-mac/
* Silicone Tubing for Air Pumps - https://www.adafruit.com/product/4661
* 1 - 1n4001 diode -> https://www.adafruit.com/product/755
* 1 - 330 ohm resistor -> Refer to parts list
* 1 - PN2222 NPN Transitor
* 100% Pure Vegetable Glycerine for the vape pen to generate smoke

I will be using SMOKE2, which is Pin #39 on the Mega. In the pack schematic diagrams, refer to SMOKE2 setup on the breadboard for the connection guide in the PACK instructions for more detail instructions. A quick rundown on what happens is, Pin #39 on the Mega goes high during smoke events, which makes the transistor open and the connected DC pump will then have 5V power which begins to operate. I removed the original battery compartment in the HasLab motherboard, so I have quick access to the vape pen from the battery door on the motherboard when it needs recharging and refilling with more vegetable glycerine.

Here are some photos the basic smoke setup. 

![Booster Smoke Setup](images/SmokeBooster5.jpg)

![Booster Tube](images/SmokeBooster1.jpg)

![Booster Tube 2](images/SmokeBooster2.jpg)

![Booster Setup](images/SmokeBooster3.jpg)

![Booster Setup](images/SmokeBooster4.jpg)


## N-Filter Example

...