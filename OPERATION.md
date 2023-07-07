# Operating Manual
					
Video Demo: [Features and Menu Walkthrough](https://www.youtube.com/watch?v=ePXz99UawLQ) (YouTube)
[![Haslab Proton Pack Arduino Powered Demo & Features](https://img.youtube.com/vi/ePXz99UawLQ/maxresdefault.jpg)](https://www.youtube.com/watch?v=ePXz99UawLQ)

## Proton Pack		

### Standard Pack Features

- The switch underneath the ion arm can turn the pack on or off.
- You can run the pack without the cyclotron lid on.
- The volume for the pack and wand can be controlled via the Crank Generator knob.
   - Clockwise to raise volume, counter-clockwise to lower it.
- When the ribbon cable is disconnected it will activate the alarm mode and degraded the cyclotron spin.
- The pack can run without a wand, though it can only be activated via the switch under the Ion Arm.
- Vibration switch in the cyclotron switch plate enables or disables the vibration motors for both the pack and wand.
- 1984/2021 (Afterlife) year switch in the cyclotron switch plate changes the pack and wand from 1984 (OG) to 2021 (Afterlife) modes.
- The Proton Pack vibrates only while firing. This can be changed to vibrate at all times or entirely disabled. Vibration can be enabled or disabled by the vibration toggle switch in the Proton Pack. Vibration settings can be toggled via the wand menu system or the advanced user configuration settings. **\*see below\***
- Single (centered) LED per cyclotron lens for 1984/1989 modes by default.
	- This can be toggled back to 3 LED's per cyclotron lens via the wand menu system.


### Optional Pack Features

- 1989 sound effects mode available on the pack, it can be accessed from the Wand sub menu system or set with the advanced user configuration settings. **\*see below\***
- Support for switches for cyclotron direction and to enable or disable smoke effects.
- Smoke effects with dedicated fans for the booster tube and N-filter.
- Additional Lights available for the cyclotron panel, internal "cake" cyclotron and Power Cell.
	- Automatically switches to internal "cake" cyclotron, if implemented (optional).
	- This supports a 35 LED ring light (by default) to be installed in the "cake". Other size LED rings or LED setups are supported in the configuration settings.
- Video game colours supported if using RGB supported LEDs for the Cyclotron Lid, internal "cake" cyclotron and Power Cell.

## Neutrona Wand				

### Standard Wand Features

- Activate toggle on the gun box turns on both the pack and wand.
	- The switch under the pack's ion arm does not need to be engaged for this to work.
- Right-hand toggles affect the vent light and additional sound effects depending on mode.
	- Regardless of mode, the wand will not fire unless these switches are turned on.
	- Additionally, the lever switch near the gunbox must be pulled to extend the barrel before firing.
	- **Boot-Up Errors**: If you attempt to boot up your Neutrona Wand while the top-right switch (beep switch) is on, the Neutrona Wand will boot into a error mode. Turn this top-right switch off and restart your Neutrona Wand to boot normally. **This is the default behaviour**, though this feature can be disabled in the advanced user configuration settings by setting **b&#95;wand&#95;boot&#95;errors** to false. **\*see below\***
- The Neutrona Wand vibrates only while firing. This can be changed to vibrate at all times or entirely disabled. Vibration can enabled or disabled by the vibration toggle switch in the Proton Pack. Vibration settings can be toggled via the wand menu system or the advanced user configuration settings. **\*see below\***

**Toggle Switch Behavior**

- While in 1984/1989 modes:
	- Bottom right switch turns on the vent light and plays the wand power up sound.
	- Top right switch plays a single beep sound.
- While in Afterlife mode:
	- Bottom right switch turns on the vent light and the wand plays a ramp up sound.
 	- Top right switch turns on the looping beeping sounds.

**Top Dial Behavior**

- The rotary dial on the top of the gun box changes the power mode of the wand, affecting the intensity of the stream effects, indicated by increasing or decreasing the bargraph lights (just as the stock wand did).
   - Clockwise to raise power/volume, counter-clockwise to lower these values.
- There are 5 wand power modes as based on the original bargraph which has only 5 LED's available:
	- The wand can be user-configured in the Neutrona Wand code to overheat the pack in any of the power modes. When not set to overheat, you can fire continiously.
	- The pack can also be user configured in the Proton Pack Code to adjust the duration of smoke effects (for each power mode), which smoke effects are activated (for each power mode), use of overheating smoke effects (for each power mode), or whether smoke disabled entirely.
	Default settings: The wand will fire forever on modes 1 through 4 and overheat on mode 5.
- The pack will emit smoke effects during longer firing (if enabled).
- For lower power modes, the proton stream is more red. On higher modes, the stream will appear more yellow.
- As the dial is turned, any beeping rate will increase or decrease depending on the theme in use.
- This dial will be used to navigate the menu system and adjust volume in certain cases (see "Wand Settings Menu System" section).

### Optional Wand Features

- Support for additional lights such as the wand hat lights and wand barrel end strobe.
- Serial connection and power for a 28-segment bargraph support (to replace stock 5 LED bargraph).

## Runtime Modes

The following features require the use of both the pack and wand replacement controllers (Arduino-based) which implements a menu system to access special features which affects overall operation. Details for the menu system will be covered below, but it's important to describe the various operating modes first.

**Video Game Mode (Default)**

- The Barrel Wing (orange) button at the end of the wand switches between these available firing modes while the wand is active:
	1. Proton Stream (Default)
	1. Slime Gun
	1. Stasis Beam
	1. Meson Collider
	1. Manual Venting Mode (Manual vent your Proton Pack)
	1. Special: Setting 6 is a settings menu system (see "Top Level Menu" section below).

**Manual Venting Mode**

- When in manual venting mode, the slo-blo LED and the orange LED on the front of the wand body will blink to indicate that you are in the manual venting mode. Press Intensify on the wand to manually vent your Proton Pack. (All the safety switches on the wand must be activated).

**Alternate Firing Mode: Cross The Streams (CTS) / Cross The Streams Mix (CTS Mix)**

- The Barrel Wing (orange) button at the end of the wand acts as a alternate fire mode only--it does not switch stream modes.
- The overheat features can only be triggered when enabled by the alternate CTS firing mode.
- Pressing both the Intensify and Barrel Wing button (at the end of the wand) at the same time enables the "Cross the Streams" (CTS) audio and visual effects. Releasing one of the 2 firing buttons will continue these effects.
	- During Cross The Streams Mix, you need to hold both the intensify and alternate firing button at the same time. Releasing one or the other will end crossing the streams but continue firing a regular proton stream. The Proton Pack and Wand can still overheat if enabled while crossing the streams in the power level you are at.

To enable a Cross The Streams (CTS) mode and alternate firing by default, please see the [Advanced Configuration](ADVCONFIG.md) guide, otherwise these can be accessed from the Wand settings menu system.

## Wand Settings Menu System

Wand Menu Settings Video Demo: [Proton Pack Wand Menu Settings Video Demo](https://www.youtube.com/watch?v=QrevSoQo_3M) (YouTube)
[![Proton Pack Wand Menu Settings Video Demo](https://img.youtube.com/vi/QrevSoQo_3M/maxresdefault.jpg)](https://youtu.be/QrevSoQo_3M)

### Navigation

**Use the rotary knob on the top of the wand to navigate up and down through the menu system.**

When you are in the settings menu system the bargraph's 5 LED's begin flashing (or 5 distinct segments when using the 28-segment bargraph). All other functions of the wand (such as firing) are temporarily disabled while in the menu system. There are 5 different menu settings, which are indicated by the bargraph LED's/segments 1 through 5 (from bottom to top) with menu level 5 being the default upon entering this mode. For example in navigation, when 1 LED (or bargraph segment) is flashing that means you are on menu 1.

### Actions

Changes are made by pressing either the "Intensify" button on the gun box or the "Barrel Wing" button (orange mode button) at the end of the wand. As the latter button may be altered by some optional upgrades, 

## Top Level Menu

| Top&nbsp;Menu&nbsp;Level | Purpose | Intensify Button | Barrel Wing Button |
|----------------|---------|------------------|--------------------|
| 5 | Music&nbsp;Track&nbsp;Looping | Enable/disable looping of current track<sup>1</sup> | Exit the menu system<sup>2</sup> |
| 4 | Sound&nbsp;FX&nbsp;Volume | Increases volume | Decreases volume |
| 3 | Music&nbsp;Volume | Increases volume | Decreases volume |
| 2 | Switch&nbsp;Music&nbsp;Track | Cycle forward in music queue | Cycle backwards in music queue |
| 1 | Play&nbsp;or&nbsp;Stop&nbsp;Music / System Mute | Start/stop playing of music | Mute the Proton Pack and Neutrona Wand  or revert back to the previous volume<sup>3 |

<sup>1</sup> Note that the LED/segment #5 will stay solid when the single-track loop is enabled while in the top menu system; it will blink on/off when looping is disabled.

<sup>2</sup> When you navigate back to the **Setting 5** while in Video Game Mode and press the Barrel Wing Button, the wand will return the proton stream mode (accompanied by an audio cue to indicate this return to firing readiness). When in CTS mode this will exit the menu and allow the wand to be powered on. Note that music will continue to play (and advance/loop) even when the pack and wand are turned off. 

<sup>3</sup> Note that the LED/segment #1 will stay solid when the Proton Pack and Neutrona Wand volume is muted and turned off.

## Sub Level Menu

**Important: The Neutrona Wand sub-menu settings can only be reached while both the Neutrona Wand and Proton Pack are powered down**

To access the sub level menu, use the rotary dial on the top of the wand. When you reach the sub level menu, the Slo-blo LED will light up to indicate this menu state.

| Sub&nbsp;Menu&nbsp;Level | Purpose | Intensify Button | Barrel Wing Button |
|----------------|---------|------------------|--------------------|
| 5 | Firing&nbsp;Mode&nbsp;Selection | Cycle through available modes<sup>3</sup> | Enable/Disable video game colour modes when video game mode is enabled. (Proton Pack only) |
| 4 | Smoke&nbsp;Settings&nbsp;/&nbsp;Overheating| Enable/Disable smoke effects | Enable/Disable overheating |
| 3 | Cyclotron&nbsp;LED&nbsp;Control | Change cyclotron rotation | Toggle between 1 (centered) or 3 LED's in cyclotron lid for 1984/1989 modes |
| 2 | Vibration Settings | Proton Pack Enable/Disable vibration or vibration during firing. | Neutrona Wand Enable/Disable vibration or vibration during firing. |
| 1 | Year&nbsp;Mode&nbsp;Selection | Cycle through Afterlife (2021), 1984, or 1989 modes | - No Action - |

<sup>3</sup> Note that the LED/segment #5 will stay solid when CTS is enabled while in the sub menu system. Remember that CTS Mix requires holding down both intensify and the alternate firing button at the same time.

To exit the menu system, navigate to the top menu in **Setting 5** (Slo-blo LED is no longer illuminated) and press the Barrel Wing button button.

**When music is playing while the pack and wand are turned off, you can use the Rotary knob on the top of the wand to easily access the independent music volume control.**

Refer the [WavTrigger section](WAVTRIGGER.md) for more information on loading additional music tracks.

## Advanced User Configurations

To modify defaults within the software, please see the [Advanced Configuration](ADVCONFIG.md) guide.