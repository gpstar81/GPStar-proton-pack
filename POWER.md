# Power Requirements

You will need a suitable power supply to run the pack controller, 2 sound boards, the wand controller, and optionally any audio amplifiers and smoke effects. If looking for configuration options for the latter, please view [the SMOKE guide](SMOKE.md) for diagrams and references.

## Voltage

The controller, sound boards, and all LEDs will all require 5VDC which is the standard voltage for USB devices. For this reason many simple battery packs used by the community have been using a USB to JST-XH connector for the stock HasLab equipment.

For most audio amplifier choices used with the pack those devices typically require 9-12V. Meanwhile, the components required for smoke kits may use as little as 3.7V or require up to 12V.

**Note:** You will most likely be using a rechargeable lithium-ion battery or similar to power your equipment. Unlike Alkaline batteries these do not drop in voltage as they discharge, but keep up a continuous voltage until their capacity is nearly depleted--at which time the internal circuitry will simply stop providing output. As such, reading voltage is not a reliable means of estimating battery life remaining.

**2025 Update** - The new [GPStar Amplifier II](https://gpstartechnologies.com/products/gpstar-amplifier-ii) and [Power Hub II](https://gpstartechnologies.com/products/gpstar-power-hub-ii) devices now allow your system to be 24V compatible. These devices can take 12V or 24V source voltage as input while still providing 5V distribution for accessories.

## Power Draw

In addition to voltage we need to consider the amperage required for running components. You will need a power source which can deliver at least 0.6A to run the most basic kit, while additional upgrades such as the 48-LED barrel will require a minimum of 0.8A to run effectively. The 12V batteries offered by Talentcell are a popular choice as they provide up to 2A from the USB port and 3A from the 12V barrel jack.

Next is the matter of how long you may wish to run your pack. For those who "troop" or will use their equipment for events, you need a long runtime which is a factor of the battery's Amp-Hour rating (typically stated in milli-Amp-hour or mAh). To estimate how long your battery will last you would need to know how many Amps are required to run your kit at a maximum, then divide the mAh rating by Amps. For example, a 3000mAh battery is equivalent to 3Ah, which at a constant 0.8A draw would run for approximately 3.75hrs.

**June 2024 Update** - Since the advent of this kit several components have been added as optional equipment, and some as alternatives to original parts. When using the pack and wand controllers with GPStar Audio boards, an ESP32 and Attenuator kit, all possible wand lighting upgrades, and the GPStar Amplifier the total power draw is around 0.35A at idle and around 0.93A while firing all effects. If adding a single smoke kit for the N-Filter during overheat/venting the maximum power draw is closer to 2.92A which is approaching the maximum rating for the Talentcell batteries.

## Warnings

*"I'm fuzzy on the whole good/bad thing. What do you mean, 'bad'?" -Dr. Venkman*

- For purposes of a DIY approach, in no case should you use the Arduino Mega's barrel jack to convert 12VDC into 5VDC for that board, nor use the 5V pin on the controller for running any other devices. The power converter on the microcontroller can become hot and may burn out over time, or you could easily draw more power than the Arduino can sustain, causing the need to replace the entire controller. Your best option is to utilize the 5V pin (and GND) to power any Arduino controllers which should come from a stable regulated 5V power source. The D-cell batteries supplied with the stock equipment may not be sufficient for the long runtimes expected by the solutions offered by these guides--furthermore, Alkaline batteries will exhibit a voltage drop as they drain which can fall below the minimum necessary to run the Arduino controllers.

- Any battery pack which uses PD (power delivery) is not recommended, nor is use of USB-C connections as these could potentially supply up to 100W of power and could destroy your equipment. Using standard USB-A connections is recommended as there are dedicated power wires which are easy to access from a modified cable and is typically limited to 2-3 Amps for output.

- Putting too high a load on a rechargeable battery is not good long-term for the battery or the components. The battery will be limited to a maximum output in terms of both voltage and camperage. In some cases this can present as degraded behavior for the component (eg. smoke air pumps run slower) or it could affect the controllers causing stuttering or other unexpected behavior. You are encouraged to put high-draw devices on their own separate battery or otherwise limit smoke effects to 1 device per battery if sharing with the pack/wand electronics.

- When using the new GPStar Amplifier in your Proton Pack, do not run more than 1 smoke device (vape coil + air pump) from any of the provided 5V JST-XH connections. The built-in voltage regulator can only provide a maximum of 3A, and when running all of the pack/wand components plus a single 5V smoke device can easily approach that maximum. Similarly, if using the GPStar Power Hub this also uses the same voltage regulator and should not be used to drive more than 2 smoke device.

## Options

### Good

A standard USB battery pack with a USB-to-JST cable to deliver only the +5VDC and Ground (-) for use by all components. YMMV based on the option you choose, but it MUST support at least 2A of power for the core electronics, exclusive of any smoke effects. See the next 2 options for more recommended solutions.

### Better

A TalentCell<sup>1</sup> battery pack which [only offers a 12VDC output](https://a.co/d/j4m2Kff) or [both USB connections and 12VDC output](https://a.co/d/8q0VcxT). For models which support USB, use of a USB-to-JST cable can provide power to the core electronics while the 12VDC barrel jack will be used for the audio amplifier in the pack.

### Best

Either of the TalentCell<sup>1</sup> battery packs just mentioned in conjunction with the [GPStar Amplifier](https://gpstartechnologies.com/products/gpstar-amplifier). The latter device connects easily using a barrel jack, such as the splitter provided with the TalentCell battery for simultaneous charging and use. The amplifier contains a built-in power regulator and multiple 5VDC JST-XH ports for connecting your GPStar Pack Controller, sending power to the Neutrona Wand, and power for the Attenuator or wireless adapter.

<sup>1</sup> TalentCell batteries come in a variety of amp-hour ratings, though 3000mAh or 6000mAh are common capacities.

### Ultimate

With the introduction of the [GPStar Amplifier II](https://gpstartechnologies.com/products/gpstar-amplifier-ii) and [Power Hub II](https://gpstartechnologies.com/products/gpstar-power-hub-ii) devices your Proton Pack can be adapted to run on a 24V input voltage! This will require a suitable battery such as the [TalentCell 24V 82.88Wh Battery, model PB240A1](https://talentcell.com/lithium-ion-battery/24v/pb240a1.html).

## Using Multiple Power Sources

For those who wish to make use of smoke effects, the need for multiple batteries has become more apparent and recommended. Every addition and upgrade to your pack and wand comes at a cost of power, and as of 2024 the number of LEDs in use may require up to 1.2A just for the base system functionality. While most DIY and community smoke kits may require 1.5-2.5A each, which easily pushes the stated limit (3A) of a single TalentCell battery.

Please follow our diagrams in the [Smoke Guide](SMOKE.md) for more information about how to make use of multiple batteries. With the addition of the [GPStar Power Hub](https://gpstartechnologies.com/products/gpstar-power-hub) this is much easier as you can use a single device with a standalone battery to operate up to 2 smoke kits simultaneously. Per our recommendations, smoke kits can and should be triggered by use of a relay which also makes this scenario possible, as it provides isolation between the power source to the primary electronics from the high-draw heating elements used in most smoke kits.

## Charging Switch

Some users have made use of what's called the "DPDT Mod" which uses any standard "Dual-Pole, Dual-Throw" [toggle](https://a.co/d/gVtXMck) or [switch](https://a.co/d/3EYqrDI) to act as a master kill switch to the electronics, but allowing a Talentcell battery to be charged. Assuming you use both the 5V from the USB port and 12V from the barrel jack on a standard Talentcell battery (regardless of mAh capacity), the basic installation would look like this:

- A [5.5mm x 2.1mm (or 5521) female port](https://a.co/d/hxna1qj) is installed on the Proton Pack as a charging port
- All ground connections are tied together: from the USB, Barrel Jack, and Charging Port
- Each positive (+) connection is attached to a post on the toggle or switch as follows:
	- The 12V+ and 5V+ to the Proton Pack are connected to the upper posts of the toggle/switch
	- The Barrel Jack's 12V+ and USB's 5V+ are connected to the center posts of the toggle/switch
	- ONLY the 12V+ from the charging port is connected to the end OPPOSITE the Proton Pack connections

For a visual aid see the ASCII drawing below for reference. The 12V is intended to go to your audio amplifier, while the 5V goes to the Proton Pack PCB. **If using the GPStar Audio Amp** which can supply 5V to your pack/wand PCB's you can simply use the 12V connection only and ignore all 5V connections shown/mentioned.

**Note:** In order to use the Talentcell or charge it, the battery itself must be in the ON position. If you wish to conserve power once charged, turn off the battery via its built-in switch if you intend to store or not use your equipment for a long period.

```
Proton Pack
  -------------- Negative from Common Ground
  |
 Amp   PCB ----- Negative from Common Ground
  |+    |+    
===========
|12V+  5V+|
| |     | |
|12V+  5V+| == Common Center to Battery (Jack/USB)    
| |       |
|12V+  NC*|  *No Connection for 5V+ side (USB)
===========
  |+
Charging Port -- Negative from Common Ground
```
