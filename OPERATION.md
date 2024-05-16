<p align="center" style="background-color:#333;">
  <img src="images/gpstar_logo_white.png"/>
</p>

<br/>

<center><h1><img height="30" src="images/gpstar_logo.png"/> GPStar Proton Pack &amp; Neutrona Wand Operation Manual</h1></center>

<br/>

![](images/OperationNeutrona1.jpg)
<sup>Neutrona Wand renders courtesy of Luis Peña</sup>

<div class="page-break"></div>
<div class="page-break"></div>

# Operational Guide - Table of Contents

- [Standard Features](#standard-features)
- [Optional Features](#optional-features)
- [Toggle Operation Modes](#toggle-operation-modes)
- [System Modes](#system-modes)
- [Neutrona Wand Menu System](#neutrona-wand-menu-system)
- [Additional Resources](#additional-resources)

<div class="page-break"></div>

# Standard Features

- The switch underneath the Ion Arm can turn the Proton Pack on or off.
- You can run the pack without the Cyclotron Lid attached.
- The volume for the pack and wand can be controlled via the Crank Generator knob.
   - Clockwise to raise volume, counter-clockwise to lower it.
- When the ribbon cable is disconnected it will activate the alarm mode and degrade the Cyclotron spin.
- The Proton Pack can run without a wand, though it can only be activated via the switch under the Ion Arm.
- Vibration switch in the Cyclotron switch plate enables or disables the vibration motors for both the Proton Pack and Neutrona Wand.
- 1984/Afterlife year switch in the Cyclotron switch plate changes the Proton Pack and Neutrona Wand from 1984 to Afterlife modes.
- The Proton Pack vibrates only while firing. This can be changed to vibrate at all times or entirely disabled. Vibration can be enabled or disabled by the vibration toggle switch in the Proton Pack. Vibration settings can be toggled via the Neutrona Wand menu system or the advanced user configuration settings.
- The Neutrona Wand vibrates only while firing. This can be changed to vibrate at all times or entirely disabled. Vibration can enabled or disabled by the vibration toggle switch in the Proton Pack. Vibration settings can be toggled via the wand menu system or the advanced user configuration settings.
- Single (centered) LED per Cyclotron lens for 1984/1989 modes by default.
	- This can be toggled back to 3 LEDs per Cyclotron lens via the Neturona Wand menu system.
- The Power Cell, Cyclotron and other lighting on the Proton Pack can have the brightness independently adjusted.
- 1989 and Frozen Empire sound effect modes are available; they can be accessed from the Neutrona Wand menus or set with the advanced user configuration settings.
- Super Hero and Mode Original system operation modes.
- Menu system with the ability to customise various settings for both your Proton Pack and Neutrona Wand.
- Up to 5 different power levels as indicated on the Neutrona Wand bargraph.
	- The Neutrona Wand can be configured within the menu system to adjust the overheat settings. When not set to overheat, you can fire continuously.
	- The Proton Pack can also be configured within the Neutrona Wand menu system to adjust the duration of smoke effects (for each power level), which smoke effects are activated (for each power level), use of overheating smoke effects (for each power level), or whether smoke is enabled or disabled entirely (for each power level).
	Default settings: The Neutrona Wand will fire forever on power levels level 1 through 4 and overheat on power level 5.
- The Proton Pack will emit smoke effects during longer firing (Default setting: enabled). This can be configured from the Neutrona Wand menu system.

### System Power Readiness Indicator
- When battery power is first applied on the Proton Pack, the lights will flicker and a sound will play to indicate that the system now has power and is ready for use.

### System Power Inactivity Indicator
- The LED at the front of the Neutrona Wand body next to the Clippard valve will start to blink after 1 minute of inactivity while the Neutrona Wand and Proton Pack are powered down to indicate the system is still receiving power from the battery.

<div class="page-break"></div>

# Optional Features

- Support for changing Cyclotron direction and to enable/disable smoke effects.
- Support for smoke effects with dedicated fans for the Booster Tube and N-Filter.
	- See the [Smoke Effects](SMOKE.md) addendum for more information.
- Additional Lights available for the Cyclotron panel, internal "cake" Cyclotron, and Power Cell.
	- Automatically switches to internal "cake" Cyclotron, if implemented (optional).
	- This supports a 35-LED RGB ring light (by default) to be installed in the "cake". Other size LED rings or LED setups are supported in the configuration settings.
- Video game colours supported if using RGB-supported LEDs for the Cyclotron Lid, internal "cake" Cyclotron and Power Cell.
- Support for the Attenuator add-on device. See the [Attenuator](ATTENUATOR.md) addendum for more information.
- Support for additional lights such as the wand hat lights and wand barrel end LED strobe.
	- When adding the Hat1 light a New Barrel Wing Button can also be added.
- Serial connection and power for a 28-segment bargraph support (to replace the stock 5-LED bargraph).

![](images/OperationNeutrona7.jpg)
<sup>Location of optional hardware for Neutrona Wand, requires advanced modifications.</sup>

<div class="page-break"></div>

## Top Dial Behaviour

![](images/OperationNeutrona5.jpg)

- The primary purpose of the top dial on the top of the gun box is to change the power level of the wand, affecting the intensity of the stream effects (indicated by increasing or decreasing the bargraph lights just as the stock wand did).
   - Rotate clockwise to raise power, and counter-clockwise to lower power.
- There are 5 wand power levels in Super Hero mode and 4 in Mode Original.
- For lower power levels, the proton stream is more red. On higher power levels, the stream will appear more yellow.
- When Video Game Mode is active and the Neutrona Wand is **ON** and the top toggle switch is **DOWN**, rotating the top dial will switch between weapon modes. For a description of these modes please see the Video Game Mode section. 📝 `Super Hero mode only`
- This dial will also be used to navigate the menu systems (see "Neutrona Wand Menu System" section).

### Quick Audio Volume Adjustment

- **Main Volume: Super Hero Mode**
 	- While the Neutrona Wand is **ON** and the top and bottom toggle switches are **DOWN**, hold the `Intensify` button while turning the top dial to quickly adjust the Proton Pack and Neutrona Wand master volume.
 	- While the Neutrona Wand is **OFF** you can independently adjust the master volume of the Neutrona Wand by holding the `Intensify` button down and turning the dial. 

- **Main Volume: Mode Original**
	- While the Proton Pack cyclotron is **OFF** and the Neutrona Wand top and bottom toggle switches are **DOWN**, hold the `Intensify` button while turning the top dial to quickly adjust the Proton Pack and Neutrona Wand master volume. 
 	- While the Proton Pack cyclotron is **OFF** and if either the Neutrona Wand top or bottom toggle switch is **UP**, you can independently adjust the master volume of the Neutrona Wand by holding the `Intensify` button down and turning the dial. 

- **Music: Super Hero and Mode Original**
	- When music is playing while the Proton Pack and Neutrona Wand are turned off, you can use the Top Dial on the top of the Neutrona Wand to easily adjust the music volume.

Refer to the [Loading The Audio Files](AUDIO.md) for more information on loading additional music tracks.

<div class="page-break"></div>

# Toggle Operation Modes

There are two modes available which replicate movie or prop-accurate behaviors of the available toggle switches. These operational modes can be altered via an [EEPROM menu](OPERATION_EEPROM.md) and will affect the startup sequence for your Proton Pack and Neutrona Wand.

![](images/OperationNeutrona2.jpg)

<div class="page-break"></div>

## Super Hero Mode Operation Guide

**Super Hero**: This is the default operation mode based primarily on the 2021 film.

### Activate Toggle

- Activate toggle on the gun box turns on both the Proton Pack and Neutrona Wand.
	- The switch under the Proton Pack's Ion Arm does not need to be engaged for this to work. The switch under the Ion Arm will turn the cyclotron on.
- The green lever near the gunbox acts as a safety and must be pulled to extend the barrel before firing.
- While firing, the default bargraph animations slide/split the bargraph in half and scroll up and down simultaneously.
	- **Button Mash Lockout:** If you press the firing button(s) too rapidly the system will malfunction and lock you out for a period of time proportional to the current power level.

### Right-Hand Toggle Switch Behaviour
![](images/OperationNeutrona3.jpg)

- Right-hand toggles (Bottom and Top) affect the vent light and additional sound effects.
	- The Neutrona Wand will not fire unless these switches are both turned on.
	- **Boot-Up Errors**: If you attempt to boot up your Neutrona Wand while the top toggle switch is on, the Neutrona Wand will boot into a error mode. Turn the top toggle switch off and restart your Neutrona Wand to boot normally. **This is the default behaviour**, though this feature can be disabled in the Neutrona Wand EEPROM Configuration Menu.
	- **Quick Vent**: If the Neutrona Wand is **ON** and the Top toggle switch is **DOWN**, pressing `Intensify` will perform a quick vent. Holding down `Intensify` will perform a full overheat sequence. **This is the default behaviour**, though this feature can be disabled in the Neutrona Wand EEPROM Configuration Menu.
- While in 1984/1989 theme modes:
	- Bottom toggle switch turns on the vent light and plays the wand power up sound.
	- Top toggle switch plays a single beep sound.
- While in Afterlife and Frozen Empire theme mode:
	- Bottom toggle switch turns on the vent light and the wand plays a ramp up sound.
 	- Top toggle switch turns on the looping beeping sounds.

<div class="page-break"></div>

## Mode Original (Prop) Operation Guide

**Mode Original**: This mode reflects original production documentation from 1984 on how the Proton Pack and Neutrona Wand operate together. Other variation differences are the bargraph animations, Neturona Wand sound effects and power levels. Note that the firing modes from the 2009 video game are unavailable in this mode as they are not accurate to these production notes.

- There are 4 power levels in Mode Original. When the Neutrona Wand is in standby mode (see toggle switch behaviours below), the bargraph settles on the first level.
- The switch under the Ion Arm on the Proton Pack must be switched on. This gives power to the entire system. The slo-blo light on the Neutrona Wand will turn solid red to indicate power to the system.
	- If you are using a standalone Neutrona Wand without a Proton Pack, double-clicking the `Intensify` button while the Neutrona Wand is turned off will imitate flipping the switch under the Ion Arm.
- While firing, the default bargraph animations slide the bargraph up and down small increments to indicate the instability and power of the proton stream.
	- **Button Mash Lockout:** If you press the firing button(s) too rapidly the system will malfunction and lock you out for a period of time proportional to the current power level.

### Right-Hand Toggle Switch Behaviours
![](images/OperationNeutrona13.jpg)

- The bottom toggle will put the Neutrona Wand into standby mode with an audible beep, and the slo-blo LED will begin to blink.
- If the bottom toggle is on the top toggle turns on the bargraph, the vent light, and Clippard LED and the heatup sound will play. The bargraph will ramp up and back down to the lowest level setting.

### Activate Toggle

- Activate toggle on the gun box turns on the cyclotron of the Proton Pack. To activate the cyclotron, both right hand toggle switches on the Neutrona Wand must be activated.
	- The Cyclotron will not turn on unless both toggle switches on the Neutrona Wand are activated and the Ion Arm switch on the Pack is in the ON position.
- The green lever near the gunbox acts as a safety and must be pulled to extend the barrel before firing.

<div class="page-break"></div>

# System Modes

## Video Game Mode (Default)<br>📝 `Super Hero mode only` 

![](images/OperationNeutrona8.jpg)

- While the Neutrona Wand is **ON** and the top toggle switch is **DOWN**, rotating the top dial will select from the following weapon modes (listed in counter-clockwise order):
	1. [Proton Stream (Default)](https://ghostbusters.fandom.com/wiki/Proton_Pack/Realistic_Version)
	1. [Dark Matter Generator](https://ghostbusters.fandom.com/wiki/Dark_Matter_Generator_(realistic_version))
	1. [Plasm Distribution System](https://ghostbusters.fandom.com/wiki/Plasm_Distribution_System_(realistic_version))
	1. [Composite Particle System](https://ghostbusters.fandom.com/wiki/Composite_Particle_System_(realistic_version))
	1. <sup>1</sup> `Spectral (Rainbow)`
	1. <sup>2</sup> `Spectral (Holiday)`
	1. <sup>3</sup> `Spectral (Custom)`

<sup>1</sup> Spectral Mode (Rainbow) is a Proton Stream which gives a RAINBOW effect. If your Proton Pack has RGB LEDs, they will change to match.

<sup>2</sup> Spectral (Holiday) is a Proton Stream which gives a RED/GREEN effect. If your Proton Pack has RGB LEDs, it will match these colours.

<sup>3</sup> Spectral (Custom) is a Proton Stream with a user-defined colour mode. You can independently adjust the Neutrona Wand Barrel, Power Cell, Cyclotron and Inner Cyclotron colours directly from the EEPROM LED Menu system. The default colours are purple.

📝 **Note:** Spectral modes are disabled by default but can be enabled from the EEPROM Menu or flashed directly to the Neutrona Wand board. They take advantage of RGB coloured LED add-ons if installed.

### Proton Stream
- Holding down `Intensify` will throw your normal Proton Stream.
	- Pressing the `Barrel Wing Button` while you are already throwing a Proton Stream will initiate "Cross the Streams".
- Pressing the `Barrel Wing Button` will fire a Boson Dart.
### Dark Matter Generator
- Pressing `Intensify` will fire a Shock Blast.
- Holding down the `Barrel Wing Button` will throw a Stasis Stream.
### Plasm Distribution System
- Holding down `Intensify` will activate the Slime Blower.
- Pressing the `Barrel Wing Button` will fire a Slime Tether.
### Composite Particle System
- Pressing `Intensify` will fire the Meson Collider.
- Pressing the `Barrel Wing Button` will fire an Overload Pulse. Holding down the `Barrel Wing Button` allows for rapid-fire.
	- The firing rate is dependent on the Neutrona Wand's current power level.

## Cross The Streams (CTS) / Cross The Streams Mix (CTS Mix)

- The `Barrel Wing Button` at the end of the wand acts as a alternate firing button. When Cross The Streams is enabled, Video Game Modes are disabled and you will only have the Proton Stream on both `Intensify` and the `Barrel Wing Button`.
- The overheat features can only be triggered when holding the `Barrel Wing Button` when Cross The Streams mode is enabled.
- Pressing both the `Intensify` and `Barrel Wing Button` at the same time enables the "Cross the Streams" (CTS) audio and visual effects. Releasing either of the firing buttons will continue these effects as long as one is still held, and will end once both buttons are released.
	- With Cross The Streams Mix, you need to hold both the `Intensify` and `Barrel Wing Button` at the same time to remain in Cross The Streams. Releasing one of the firing buttons will revert to the normal Proton Stream for that button.

Cross The Streams (CTS) is the default firing mode for Mode Original. Cross The Streams (CTS) or Cross The Streams Mix (CTS Mix) modes can be selected from the Neutrona Wand Menu System.

<div class="page-break"></div>

# Neutrona Wand Menu System

![](images/OperationNeutrona9.jpg)

## Entering The Menu System

There are 2 ways to enter the Menu System.

- With the Activate switch **UP** and any right-hand switch **DOWN**, press the `Barrel Wing Button` to enter the Neutrona Wand Menu System. <br>📝 `Super Hero mode only`

- While the Neutrona Wand and Proton Pack are powered down, press the `Barrel Wing Button`.

<div class="page-break"></div>

## Navigation

Use the Top Dial on the top of the wand to navigate up and down through the menu system.

![](images/OperationNeutrona8.jpg)

When you are in the settings menu system the bargraph's 5 LEDs begin flashing (or 5 distinct segments when using the 28-segment bargraph). All other functions of the wand (such as firing) are temporarily disabled while in the menu system. There are 5 different menu settings, which are indicated by bargraph LED segments 1 through 5 (from bottom to top) with menu level 5 being the default upon entering this mode. For example in navigation, when 1 LED (or bargraph segment) is flashing/displayed that means you are on `Option 1`.

## Actions

Changes are made by pressing either the `Intensify` button on the gun box or the `Barrel Wing Button` at the end of the wand.

<div class="page-break"></div>

### Menu Level 1

| Option | Purpose | Intensify Button | Barrel Wing Button |
|:------:|---------|------------------|--------------------|
| 5| Music&nbsp;Track&nbsp;Looping | Enable/disable looping of current track<sup>1</sup> | Exit the menu system<sup>2</sup> |
| 4 | Proton&nbsp;Pack&nbsp;Light&nbsp;Dimming | `(Intensify + Top Dial)` Increases/Decreases the Proton Pack device LED brightness | Toggles between Power Cell, Cyclotron, or Inner Cyclotron devices |
| 3 | Music & SFX Volume | `(Intensify + Top Dial)` Increases/Decreases SFX Volume | `(Barrel Wing Button + Top Dial)` Increases/Decreases Music Volume |
| 2 | Switch&nbsp;Music&nbsp;Track | Cycle forward in music queue | Cycle backwards in music queue |
| 1 | Play&nbsp;or&nbsp;Stop&nbsp;Music / System&nbsp;Mute | Start/stop music playback | Mute the Proton Pack and Neutrona Wand  or revert back to the previous volume<sup>3 |

<sup>1</sup> Note that LED/segment #5 will stay solid when the single-track loop is enabled while in the top menu system; it will blink on/off when looping is disabled.

<sup>2</sup> When you navigate back to `Option 5` and press the `Barrel Wing Button`, the wand will return to the previously-selected firing mode (if menu was entered while wand was on) or an idle state (if menu was entered while wand was off). Note that music will continue to play (and advance/loop) even when the pack and wand are turned off.

<sup>3</sup> Note that LED/segment #1 will stay solid when the Proton Pack and Neutrona Wand volume is muted.

<div class="page-break"></div>

### Menu Level 2

📝 **Note:** The Neutrona Wand Menu Level 2 can only be reached while the Neutrona Wand and Proton Pack are both turned off.

To access Menu Level 2, use the `Top Dial` on the top of the wand to move down and past `Option 1` on Menu Level 1. When you reach the the next menu level, an audio cue will play and the Slo-Blo LED will light up to indicate this menu state.

![](images/OperationNeutrona10.jpg)

| Option | Purpose | Intensify Button | Barrel Wing Button |
|:------:|---------|------------------|--------------------|
| 5 | Firing&nbsp;Mode&nbsp;Selection | Cycle through VG/CTS operation modes<sup>4</sup> | Cycle through VG colour modes <sup>5</sup> |
| 4 | Overheating / Smoke | Enable/Disable Overheating Ability | Enable/Disable Smoke Effects |
| 3 | Cyclotron&nbsp;LED&nbsp;Control | Change Cyclotron Rotation | Toggle between 1 (centered) or 3 LEDs in the Cyclotron Lid for 1984/1989 modes |
| 2 | Vibration&nbsp;Settings | Enable/Disable Proton Pack vibration or vibration during firing | Enable/Disable Neutrona Wand vibration or vibration during firing |
| 1 | Year&nbsp;Mode&nbsp;Selection | Cycle through 1984, 1989, Afterlife, and Frozen Empire modes | Enable/Disable the Proton Stream impact sound effects |

<sup>4</sup> Switch between Video Game and Cross the Streams (CTS) operation modes. Note that LED/segment #5 will stay solid when CTS or CTS Mix are enabled while in Menu Level 2. Note also that Video Game mode is only available in Super Hero mode.

<sup>5</sup> Enables/disables the video game colour modes for the Proton Pack's Power Cell and Cyclotron lights. This only affects the pack when in Video Game mode, not for CTS modes, and offers the following states:

- Disable all the video game colours (Power Cell remains blue, Cyclotron remains red).
- Enable only the Power Cell LEDs to change colours (requires RGB Power Cell such as that offered by Frutto Technology).
- Enable only the Cyclotron LEDs to change colours (requires RGB replacement such as that offered by Frutto Technology).
- Enable both the Power Cell and Cyclotron to use video game colours.

To exit the menu system, navigate to `Menu Level 1, Option 5` (Slo-blo LED is no longer illuminated) and press the `Barrel Wing Button`. A voice callout will be heard when transitioning between the top and sub menu levels.

<div class="page-break"></div>

# Additional Resources

## EEPROM Menu Guide

Please view the [supplemental operation guide for EEPROM menus](OPERATION_EEPROM.md) which can be used to alter hardware configurations and to set user preferences which will persist across battery power-down events.

## PDF Operational Guide

(Direct Download link)
[GPStar Operational Guide [pdf]](extras/gpstar-Operational-Guide.pdf)

<div class="page-break"></div>

## Video Demos

Video Demo: [Features and Menu Walkthrough](https://www.youtube.com/watch?v=ePXz99UawLQ) (YouTube, July 2023)
[![Haslab Proton Pack Arduino Powered Demo & Features](https://img.youtube.com/vi/ePXz99UawLQ/maxresdefault.jpg)](https://www.youtube.com/watch?v=ePXz99UawLQ)

Neutrona Wand Menu System Video Demo: [Proton Pack Wand Menu Settings Video Demo](https://www.youtube.com/watch?v=QrevSoQo_3M) (YouTube)
[![Proton Pack Wand Menu Settings Video Demo](https://img.youtube.com/vi/QrevSoQo_3M/maxresdefault.jpg)](https://www.youtube.com/watch?v=QrevSoQo_3M)

Video Game Firing Modes: [Ghostbusters: The Video Game (2009) Firing Modes](https://www.youtube.com/watch?v=5CMfQdIIx04) (YouTube)
[![Ghostbusters: The Video Game (2009) Firing Modes](https://img.youtube.com/vi/5CMfQdIIx04/maxresdefault.jpg)](https://www.youtube.com/watch?v=5CMfQdIIx04)
