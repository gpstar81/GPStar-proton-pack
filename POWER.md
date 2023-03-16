# Power Requirements

You will need a suitable power supply to run the pack controller (Arduino Mega), 2 WavTrigger boards, the wand (Arduino Nano), and optionally any audio amplifiers and smoke effects.

## Voltage

Most components will require 5VDC which is the standard voltage for USB. For this reason many simple battery packs used by the community have been using a USB to JST-XH connector for the stock HasLab equipment. Though for several reasons these can be limiting in what components can be used, main due to audio amplifiers needing upwards of 9-12VDC to properly operate, while some smoke effects may need as low as 3.7VDC.

## Power Draw

There is also the matter of how long you may wish to run your pack. For those who "troop" or will use theirs for events, you need a long runtime which is a factor of the battery's Amp-Hour rating (typically stated in mAh). Some typical power draws from the devices are as follows, which adds up to ~0.6A for the base components minus your audio amp of choice for the pack.

- Pack: 0.14-0.25 A - Measured using the line-out from the WavTrigger to a standalone amplifier
- Wand: 0.08-0.27 A - Measured using the built-in amplifier on the WavTrigger

## Warning

*"I'm fuzzy on the whole good/bad thing. What do you mean, 'bad'?" -Dr. Venkman*

In no case should you use the Arduino Mega's barrel jack to convert 12VDC into 5VDC for that board, nor use the 5V pin on the controller for running any other devices. The power converter on the microcontroller can become hot and may burn out over time, or you could easily draw more power than the Arduino can sustain, causing the need to replace the entire controller. Your best option is to utilize the 5V pin (and GND) to power any Arduino controllers which should come from a stable regulated 5V power source. The D-cell batteries supplied with the stock equipment may not be sufficient for the long runtimes expected by the solutions offered by these guides.

Also note that any battery pack which uses PD (power delivery) is not recommended, nor is use of USB-C connections as these could potentially supply up to 100W of power and could destroy your equipment. Using standard USB-A connections is recommended as there are dedicated power wires which are easy to access from a modified cable and is typically limited to 2-3 Amps for output.

## Options

### Good

A standard USB battery pack with a modified USB cable to deliver only the +5VDC and - (Ground) for use by all components. YMMV based on the option you choose. See the next 2 options for more recommended solutions.

### Better

The TalentCell battery packs which [only offer a 12VDC output](https://a.co/d/j4m2Kff), plus the use of one or more DC buck converters which are set to deliver 5VDC for common components or less than 5VDC for special components, while 12VDC will be used for components such as standalone audio amplifiers. These batteries come in a variety of mAh ratings, though 3000 or 6000 are common capacities. The downsides to using standalone buck converters is that they can become hot over time and may "burn out" with extremely prolonged use. Be sure to mount these where they will be able to radiate heat and do not cover them with tape or other materials.

### Best

The TalentCell battery packs which offer [both USB connections and 12VDC output](https://a.co/d/8q0VcxT). Similar to the above, this may still require a buck converter to obtain less than 5VDC power, but can easily use a modified USB cable for the core components while supplying a full 12VDC to devices such as an audio amplifier. These batteries come in a variety of mAh ratings, though 3000 or 6000 are common capacities.

## Using Multiple Power Sources

For those who wish to use different power sources for components, eg. a 5V USB power pack for electronics with a 12V battery for audio, should be aware that a common ground connection is necessary to avoid unexpected behavior. Each power source will provide its respective voltage, but that voltage is in reference to a 0V ground state--and with 2 power sources means 2 different reference points. Use of a wire which connects the ground of your power supplies together will remedy this issue.