# Operating Manual
					
Video Demo: [Haslab Proton Pack Arduino Powered Demo & Features](https://www.youtube.com/watch?v=nhGX1oJBNHk) (YouTube)

## Proton Pack		

### Features

- The switch underneath the ion arm can turn the pack on or off.
- You can run the pack without the cyclotron lid on.
	- Automatically switches to internal "cake" cyclotron, if implemented (optional).
- Alarm ribbon cable will activate the alarm mode and degraded cyclotron spin.
- The pack can run without a wand.
- Vibration switch in the cyclotron switch plate enables or disables the vibration motors for both the pack and wand.
- 1984/2021 year switch in the cyclotron switch plate changes the pack and wand from 1984 (OG) to 2021 (Afterlife) modes.

## Neutrona Wand				

### Features

- Activate toggle on the gun box turns on both the pack and wand.
- Right-hand toggles affect the vent light and additional sound effects (see below).
	- Note that for both 2021 and 1984 modes, the wand will not fire unless these switches are turned on.
	- Additionally, the lever switch near the gunbox must be pulled to extend the barrel before firing.
- The orange button at the end of the wand switches between the available firing modes:
	1. Proton Stream (Default)
	1. Slime Gun
	1. Statis Beam
	1. Meson Collider
	1. Special: Setting 5 is a settings menu system (see below).

**Toggle Behavior**

- While in 1984 mode:
	- Bottom right switch turns on the vent light and plays the wand power up sound.
	- Top right switch plays a single beep sound.
- While in 2021 mode:
	- Bottom right switch turns on the vent light and the wand plays a ramp up sound.
 	- Top right switch turns on the looping beeping sounds.

### Wand Settings Menu System							
When you are in the settings menu system, the bargraph leds begin flashing. All other functions of the wand are disabled while in the menu system. There are 5 different menu settings, which are indicated by the bargraph LED's 1 through 5. For example, when 1 LED is flashing that means you are on menu 1. Use the rotary knob on the top of the wand to navigate the menu system!

| Menu Level | Purpose | Description |
| ----------- | ----------- | ----------- |
| 5 | Music track loop settings | Press Intensify to set the current track to loop forever. Press again to turn off the loop feature. (Note that the LED #5 will stay solid when loop is enabled, it will blink on/off when looping is disabled). |
| 4 | Overall system volume | Press Intensify to increase volume, press the orange mode switch to decrease volume. |
| 3 | Adjust independent music volume | Press Intensify to increase volume, press the orange mode switch to decrease volume. |
| 2 | Switches music track | Press Intensify to cycle forward. Press the orange mode switch to cycle backwards. |
| 1 | Plays or stops the current music track | Press Intensify to play music. Press it again to stop the music. |

To exit the menu system, goto Menu 5 and press the Orange mode switch button, the wand will now go to the Proton stream mode. The music will play even when the pack and wand are turned off. When music is playing while the pack and wand are turned off, you can use the Rotary knob on the top of the wand to easily access independent music volume control.

See the [WavTrigger section](WAVTRIGGER.md) for more information on loading additional music tracks.

## Wand Power Modes

There are 5 wand power modes, indicated by the bargraph leds. Use the rotary knob on top of the wand to change power modes. The wand will fire forever on modes 1 through 4. On mode 5, the wand and pack will overheat after a certain amount of time of firing.