# Power Requirements

You will need a suitable power supply to run the pack controller, 2 sound boards, the wand controller, and optionally any audio amplifiers and smoke effects. If looking for configuration options for the latter, please view [the SMOKE guide](SMOKE.md) for diagrams and references.

## Voltage

The controller, sound boards, and all LEDs will all require 5VDC which is the standard voltage for USB devices. For this reason many simple battery packs used by the community have been using a USB to JST-XH connector for the stock HasLab equipment.

For most audio amplifier choices used with the pack those devices typically require 9-12V. Meanwhile, the components required for smoke kits may use as little as 3.7V or require up to 12V.

**Note:** You will most likely be using a rechargeable lithium-ion battery or similar to power your equipment. Unlike Alkaline batteries these do not drop in voltage as they discharge, but keep up a continuous voltage until their capacity is nearly depleted--at which time the internal circuitry will simply stop providing output. As such, reading voltage is not a reliable means of estimating battery life remaining.

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

A standard USB battery pack with a modified USB cable to deliver only the +5VDC and - (Ground) for use by all components. YMMV based on the option you choose. See the next 2 options for more recommended solutions.

### Better

The TalentCell battery packs which [only offer a 12VDC output](https://a.co/d/j4m2Kff), plus the use of one or more DC buck converters which are set to deliver 5VDC for common components or less than 5VDC for special components, while 12VDC will be used for components such as standalone audio amplifiers. These batteries come in a variety of mAh ratings, though 3000 or 6000 are common capacities. The downsides to using standalone buck converters is that they can become hot over time and may "burn out" with extremely prolonged use. Be sure to mount these where they will be able to radiate heat and do not cover them with tape or other materials.

### Best

The TalentCell battery packs which offer [both USB connections and 12VDC output](https://a.co/d/8q0VcxT). Similar to the above, this may still require a buck converter to obtain less than 5VDC power, but can easily use a modified USB cable for the core components while supplying a full 12VDC to devices such as an audio amplifier. These batteries come in a variety of mAh ratings, though 3000 or 6000 are common capacities.

## Using Multiple Power Sources

For those who wish to use different power sources for components, eg. a 5V USB power pack for electronics with a 12V battery for audio, should be aware that a common ground connection is necessary to avoid unexpected behavior. Each power source will provide its respective voltage, but that voltage is in reference to a 0V ground state--and with 2 power sources means 2 different reference points. Use of a wire which connects the ground of your power supplies together will remedy this particular issue.

As an addendum to this, smoke kits are expected to be triggered using a relay which means they are already suited to being run from a separate power source. Power for the relay will come from the pack controller, while power to the smoke device could come from a separate battery.