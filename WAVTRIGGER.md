# WavTrigger Configuration

The first thing to take care of is updating the firmware on the WavTrigger boards. Sometimes they ship with firmware version 1.28 and 1.34 is required for one feature used during music playback. It is possible to use the WavTriggers without updating the firmware, the only issue you will have is during music playback: when a track ends, the next track will not automatically load and play itself.

The WavTrigger firmware and software updater and instructions [can be found on the Robertsonics website](https://www.robertsonics.com/wav-trigger). To update the firmware, an [FTDI Basic (5V version)](https://www.sparkfun.com/products/9716) from Sparkfun should be used to interface with the WavTrigger.

[WAV Trigger Firmware v1.34](https://www.robertsonics.com/s/WAVTrig_134_20200324.zip)

[WAV Trigger Flasher Utility v2.00 Windows Executable (zip)](https://www.robertsonics.com/s/WTFlasher_20230108_v200.zip)

[WAV Trigger Flasher Utility v2.00 OS X (10.11 and above) (dmg)](https://www.robertsonics.com/s/WTFlasher_20230108_v200.dmg)


![FTDI Board](images/FTDI.png)

Only 4 connections between the boards will be required: TX/RX and 5V/Ground.
![USB and Wire Connections](images/WavTriggerFTDI.jpg)

**Firmware Summary Steps**

1. Solder wires directly or use a row of pin headers to the holes along the bottom right edge of the WavTrigger (as viewed with the barrel connector oriented at the top left).
1. Solder the small pad marked "5V" just below the sockets for SPKR +/-. This will allow the WavTrigger to operate from the 5V supplied by USB and later the battery for your Proton Pack.
1. There is a small switch near the power connect (barrel) to on the WavTrigger, set it to the “load” position.
1. Connect the following wires from the FTDI to WavTrigger:
	- RX on the FTDI to the TX of the WavTrigger.
	- TX from the FTDI to the RX of the WavTrigger.
	- 5V from the FTDI to 5Vin.
	- GND from one side to the other.
1. Plug in a mini-USB cable from computer to the FTDI board.
1. Follow the instructions outlined in the manual for upgrading the firmware using the WTFlasher utility.

**IMPORTANT: Make sure after flashing the firmware to set the small switch on the Wav Trigger back to the RUN position.**

Note the "5V" pad just below the SPKR connection holes.
![Board Closeup](images/WavTrigger.jpg)

![Connection Overview](images/WavTrigger5V.jpg)

## Arduino Connections

Once the firmware is updated to 1.34 and you have confirmed this using the WavTrigger setup utility, there will be connections made to the respective Arduino boards. Similar to the previous step the RX and TX will go to the respective TX and RX ports on the Arduino boards. See the setup instructions for the Pack and Wand as necessary. The 5Vin and GND will be connected to the power to each of the pack and wand, respectively. DO NOT use the 5V port on either Arduino to drive this device--you will need to split your power connection to each device to provide sufficient current.

NOTE: See below for more information. I used standard breadboard friendly header jumper cables to make the connections from the FTDI basic to the connectors I added to the WavTrigger. Also note that the WavTriggers have both a AUX output and/or 2 pins for audio output. You can use either or to connect to your speakers or amps.

**IMPORTANT: You will need to solder the 5V pad on the WavTrigger to enable 5V power from the device. After you do this, NEVER USE the BARREL CONNECTOR for power. We will be using 5V to power the Wav Triggers for both in the Pack and the Wand.**

## Loading Audio Files

Please see the [Audio Files](AUDIO.md) guide.