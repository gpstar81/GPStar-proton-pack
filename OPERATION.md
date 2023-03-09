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
- Optional switches and lights available for smoke effects, cyclotron panel, etc.

## Neutrona Wand				

### Key Features

- Activate toggle on the gun box turns on both the pack and wand (the switch under the pack's ion arm does not need to be engaged for this to work).
- Right-hand toggles affect the vent light and additional sound effects.
	- Note that for both 2021 and 1984 modes, the wand will not fire unless these switches are turned on.
	- Additionally, the lever switch near the gunbox must be pulled to extend the barrel before firing.
- The orange button at the end of the wand switches between the available firing modes:
	1. Proton Stream (Default)
	1. Slime Gun
	1. Statis Beam
	1. Meson Collider
	1. Special: Setting 5 is a settings menu system (see section below).

**Toggle Switch Behavior**

- While in 1984 mode:
	- Bottom right switch turns on the vent light and plays the wand power up sound.
	- Top right switch plays a single beep sound.
- While in 2021 mode:
	- Bottom right switch turns on the vent light and the wand plays a ramp up sound.
 	- Top right switch turns on the looping beeping sounds.

**Top Dial Behavior**

- The rotary dial on the top of the gun box changes the power mode of the wand, affecting the intensity of the stream effects, indicated by increasing or decreasing the bargraph lights (just as the stock wand did). There are 5 wand power modes as based on the original bargraph which has only 5 LED's available:
	- The wand can be user configured in the Neutrona Wand code to overheat the pack in any of the power modes. When not set to overheat, you can fire continiously. 
	- The pack can also be user configured in the Proton Pack Code to adjust the duration of smoke effects (for each power mode), which smoke pins are activated (for each power mode), overheating smoke effects (for each power mode) or smoke disabled entirely.

	Default settings: The wand will fire forever on modes 1 through 4 and overheat on mode 5.
- The pack will emit smoke effects during longer firing (if enabled).
- Lower power modes, the proton stream is more red. On higher modes, the stream will appear more yellow.
- As the dial is turned, any beeping rate will increase or decrease depending on the theme in use.
- This dial will be used to navigate the menu system and adjust volume in certain cases (see next section).

## Wand Settings Menu System

When you are in the settings menu system [Mode 5] all the bargraph leds begin flashing. All other functions of the wand (such as firing) are disabled while in the menu system. There are 5 different menu settings, which are indicated by the bargraph LED's 1 through 5 (from bottom to top) with menu level 5 being the default upon entering this mode. For example in navigation, when 1 LED is flashing that means you are on menu 1. **Use the rotary knob on the top of the wand to navigate up and down through the menu system!**

| Menu Level | Purpose | Description |
|------------|---------|-------------|
| 5 | Music&nbsp;Track&nbsp;Looping | Press **Intensify** to set the current track to loop forever; press again to turn off the loop feature. (Note that the LED #5 will stay solid when this single-track loop is enabled, it will blink on/off when looping is disabled). |
| 4 | Effects-only&nbsp;Volume | Press **Intensify** to increase volume, press the orange **Mode** switch to decrease volume. |
| 3 | Music-only&nbsp;Volume | Press **Intensify** to increase volume, press the orange **Mode** switch to decrease volume. |
| 2 | Switch&nbsp;Music&nbsp;Track | Press **Intensify** to cycle forward. Press the orange **Mode** switch to cycle backward. |
| 1 | Play&nbsp;or&nbsp;Stop&nbsp;Music | Press **Intensify** to play music. Press it again to stop the music. |

To exit the menu system, navigate to **Menu 5** and press the orange **Mode** switch button, the wand will now go to the Proton Stream mode (this will be accompanied by an additional audio cue that you've returned to the firing modes). Music will continue to play (and advance/loop) even when the pack and wand are turned off. When music is playing while the pack and wand are turned off, you can use the Rotary knob on the top of the wand to easily access independent music volume control.

Refer the [WavTrigger section](WAVTRIGGER.md) for more information on loading additional music tracks.
