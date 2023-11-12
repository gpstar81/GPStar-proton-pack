<h1><img src='images/gpstar_logo.png' width=50 align="left"/>gpstar Proton Pack and Neutrona Wand Operation Manual</h1>


### Standard Features

- The switch underneath the Ion Arm can turn the pack on or off.
- You can run the pack without the Cyclotron Lid attached.
- The volume for the pack and wand can be controlled via the Crank Generator knob.
   - Clockwise to raise volume, counter-clockwise to lower it.
- When the ribbon cable is disconnected it will activate the alarm mode and degrade the Cyclotron spin.
- The pack can run without a wand, though it can only be activated via the switch under the Ion Arm.
- Vibration switch in the Cyclotron switch plate enables or disables the vibration motors for both the pack and wand.
- 1984/Afterlife year switch in the Cyclotron switch plate changes the pack and wand from 1984 to Afterlife modes.
- The Proton Pack vibrates only while firing. This can be changed to vibrate at all times or entirely disabled. Vibration can be enabled or disabled by the vibration toggle switch in the Proton Pack. Vibration settings can be toggled via the wand menu system or the advanced user configuration settings. **\*see below\***
- The Neutrona Wand vibrates only while firing. This can be changed to vibrate at all times or entirely disabled. Vibration can enabled or disabled by the vibration toggle switch in the Proton Pack. Vibrationt settings can be toggled via the wand menu system or the advanced user configuration settings. **\*see below\***
- Single (centered) LED per Cyclotron lens for 1984/1989 modes by default.
	- This can be toggled back to 3 LEDs per Cyclotron lens via the wand menu system.
- The Power Cell, Cyclotron and other lighting on the Proton Pack can have the brightness independently adjusted. **\*see below\***
- 1989 sound effects mode is available on the pack, it can be accessed from the Neutrona Wand menu systems or set with the advanced user configuration settings. **\*see below\***
- Super Hero and Mode Original operation modes.
- Menu system with the ability to customise various settings for both your Proton Pack and Neutrona Wand.

### Optional Features

- Support for switches for Cyclotron direction and to enable or disable smoke effects.
- Support for smoke effects with dedicated fans for the Booster Tube and N-Filter. See [Smoke Effects](SMOKE.md) addendum.
- Additional Lights available for the Cyclotron panel, internal "cake" Cyclotron and Power Cell.
	- Automatically switches to internal "cake" Cyclotron, if implemented (optional).
	- This supports a 35 LED ring light (by default) to be installed in the "cake". Other size LED rings or LED setups are supported in the configuration settings.
- Video game colours supported if using RGB supported LEDs for the Cyclotron Lid, internal "cake" Cyclotron and Power Cell.
- Support for the Attenuator add-on device. See [Attenuator](ATTENUATOR.md) for more information.

## Super Hero Operation Guide
![](images/OperationNeutrona2.jpg)

**Super Hero**: This is the default operation mode.

- Activate toggle on the gun box turns on both the pack and wand.
	- The switch under the Proton Pack's Ion Arm does not need to be engaged for this to work. The switch under the Ion Arm will turn the cyclotron on.
- Right-hand toggles (Bottom and Top) affect the vent light and additional sound effects.
	- The Neutrona Wand will not fire unless these switches are turned on.
	- **Boot-Up Errors**: If you attempt to boot up your Neutrona Wand while the top toggle switch is on, the Neutrona Wand will boot into a error mode. Turn the top toggle switch off and restart your Neutrona Wand to boot normally. **This is the default behaviour**, though this feature can be disabled in Neutrona Wand Config EEPROM Menu. **\*see below\***
- The lever switch near the gunbox acts as a safety switch and it must be pulled to extend the barrel before firing.

**Right-Hand Toggle Switch Behavior**
![](images/OperationNeutrona3.jpg)

- While in 1984/1989 theme modes:
	- Bottom toggle switch turns on the vent light and plays the wand power up sound.
	- Top toggle switch plays a single beep sound.
- While in Afterlife theme mode:
	- Bottom toggle switch turns on the vent light and the wand plays a ramp up sound.
 	- Top toggle switch turns on the looping beeping sounds.

**Top Dial Behavior**
![](images/OperationNeutrona5.jpg)

- The top dial on the top of the gun box changes the power mode of the wand, affecting the intensity of the stream effects, indicated by increasing or decreasing the bargraph lights (just as the stock wand did).
   - Clockwise to raise power/volume, counter-clockwise to lower these values.
- There are 5 wand power levels as based on the original bargraph which has only 5 LEDs available:
	- The wand can be user-configured in the Neutrona Wand code to overheat the pack in any of the power levels. When not set to overheat, you can fire continuously.
	- The pack can also be user configured in the Proton Pack code to adjust the duration of smoke effects (for each power level), which smoke effects are activated (for each power level), use of overheating smoke effects (for each power level), or whether smoke is disabled entirely.
	Default settings: The wand will fire forever on power levels level 1 through 4 and overheat on power level 5.
- The pack will emit smoke effects during longer firing (if enabled).
- For lower power levels, the proton stream is more red. On higher power levels, the stream will appear more yellow.
- As the top dial is turned, any beeping rate will increase or decrease depending on the theme in use.
- This dial will be used to navigate the menu system and adjust volume in certain cases (see "Wand Settings Menu System" section).

## Mode Original Operation Guide
**Mode Original**: This mode reflects original production documentation from 1984 on how the Proton Pack and Neutrona Wand operate together. Other variation differences are the bargraph animations, wand sound effects and power levels.


## Optional Wand Features
![](images/OperationNeutrona7.jpg)

- Support for additional lights such as the wand hat lights and wand barrel end LED strobe. A New Barrel Wing Button can then be added.
- Serial connection and power for a 28-segment bargraph support (to replace stock 5 LED bargraph).

## Firing Modes
**Video Game Mode (Default)**
![](images/OperationNeutrona4.jpg)

- Pressing the Barrel Wing Button while you are already throwing a Proton Stream with the Intensify button will initiate "Cross the Streams".

- The Barrel Wing Button at the end of the Neutrona Wand switches between these available firing modes while the wand is active and not throwing a stream:
	1. Proton Stream (Default)
	1. Slime Blower
	1. Stasis Stream
	1. Meson Collider
	1. <sup>1</sup> `Spectral (Rainbow)`
	1. <sup>2</sup> `Spectral (Holiday)`
	1. <sup>3</sup> `Spectral (Custom)`
	1. Manual Venting Mode (Manual vent your Proton Pack)
	1. Special: Setting 6 is a settings menu system (see "Wand Settings Menu System" section below).

<sup>1</sup> Spectral Mode (Rainbow) is a firing mode which gives a RAINBOW effect. If your Proton Pack has RGB LEDs, they will change to match.

<sup>2</sup> Spectral (Holiday) is a firing mode which gives a RED/GREEN effect. If your Proton Pack has RGB LEDs, it will match these colours.

<sup>3</sup> Spectral (Custom) is a user defined colour mode. You can independently adjust the Neutrona Wand Barrel, Power Cell, Cyclotron and Inner Cyclotron colours directly from the EEPROM LED Menu ystem. The default colours are purple.

`Spectral modes are disabled by default but can be enabled from the EEPROM Menu or flashed directly to the Neutrona Wand board. They take advantage of RGB coloured LED add-ons if installed.`

**Manual Venting Mode**
![](images/OperationNeutrona6.jpg)

- When in manual venting mode, the Slo-Blo LED and the Orange LED on the front of the wand body will blink to indicate that you are in the manual venting mode. Press Intensify on the wand to manually vent your Proton Pack. (All the toggle and safety switches on the wand must be activated).

**Alternate Firing Modes: Cross The Streams (CTS) / Cross The Streams Mix (CTS Mix)**

- The Barrel Wing Button at the end of the wand acts as a alternate fire mode button. When Cross The Streams is enabled, Video Game Modes are disabled and you will only have the Proton Stream.
- The overheat features can only be triggered when holding the alternate fire mode button (Barrel Wing Button) when Cross The Streams mode is enabled.
- Pressing both the Intensify and Barrel Wing Button at the same time enables the "Cross the Streams" (CTS) audio and visual effects. Releasing one of the 2 firing buttons will continue these effects.
	- With Cross The Streams Mix, you need to hold both the Intensify and Barrel Wing Button at the same time. Releasing the Barrel Wing Button will end crossing the streams but continue firing a regular proton stream, and releasing Intensify will end firing completely.

To enable Cross The Streams (CTS) or Cross The Streams Mix (CTS Mix) mode by default, you can set this setting from the Wand Settings Menu System.

## Wand Settings Menu Systems
![](images/OperationNeutrona9.jpg)

### Entering The Menu System
There are 2 ways to enter the Menu System.

- When the Neutrona Wand is in Video Game Mode and powered on, cycle through the various modes with the `Barrel Wing Button` until you reach the Menu System.
- While the Neutrona Wand and Proton Pack are powered down, press the `Barrel Wing Button`.

### Navigation
Use the Top Dial on the top of the wand to navigate up and down through the menu system.
![](images/OperationNeutrona8.jpg)

When you are in the settings menu system the bargraph's 5 LEDs begin flashing (or 5 distinct segments when using the 28-segment bargraph). All other functions of the wand (such as firing) are temporarily disabled while in the menu system. There are 5 different menu settings, which are indicated by bargraph LED segments 1 through 5 (from bottom to top) with menu level 5 being the default upon entering this mode. For example in navigation, when 1 LED (or bargraph segment) is flashing/displayed that means you are on menu 1.

### Actions

Changes are made by pressing either the `Intensify` button on the gun box or the `Barrel Wing Button` at the end of the wand.

## Menu Level 1

| Menu&nbsp;Level&nbsp;1 | Purpose | Intensify Button | Barrel Wing Button |
|----------------|---------|------------------|--------------------|
| <p align="center">5</p> | Music&nbsp;Track&nbsp;Looping | Enable/disable looping of current track<sup>1</sup> | Exit the menu system<sup>2</sup> |
| <p align="center">4</p> | Proton Pack Light Dimming | `(Intensify + Top Dial)` Increases/Decreases the Proton Pack lights brightness. | Toggles between Power Cell, Cyclotron or Inner Cyclotron |
| <p align="center">3</p> | Sound & Music&nbsp;Volume | `(Barrel Wing Button + Top Dial)` Increases/Decreases effects volume | `(Intensify + Top Dial)` Increases/Decreases music volume |
| <p align="center">2</p> | Switch&nbsp;Music&nbsp;Track | Cycle forward in music queue | Cycle backwards in music queue |
| <p align="center">1</p> | Play&nbsp;or&nbsp;Stop&nbsp;Music / System Mute | Start/stop playing of music | Mute the Proton Pack and Neutrona Wand  or revert back to the previous volume<sup>3 |

<sup>1</sup> Note that the LED/segment #5 will stay solid when the single-track loop is enabled while in the top menu system; it will blink on/off when looping is disabled.

<sup>2</sup> When you navigate back to the `Setting 5` while in Video Game Mode and press the Barrel Wing Button, the wand will return the proton stream mode (accompanied by an audio cue to indicate this return to firing readiness). When in CTS mode this will exit the menu and allow the wand to be powered on. Note that music will continue to play (and advance/loop) even when the pack and wand are turned off.

<sup>3</sup> Note that the LED/segment #1 will stay solid when the Proton Pack and Neutrona Wand volume is muted.

## Menu Level 2

`Important: The Neutrona Wand sub-menu settings can only be reached while both the Neutrona Wand and Proton Pack are powered down.`

To access the sub level menu, use the `Top Dial` on the top of the wand. When you reach the sub level menu, the Slo-blo LED will light up to indicate this menu state.
![](images/OperationNeutrona10.jpg)

| Sub&nbsp;Level&nbsp;Menu | Purpose | Intensify Button | Barrel Wing Button |
|----------------|---------|------------------|--------------------|
| <p align="center">5</p> | Firing&nbsp;Mode&nbsp;Selection | Cycle through VG/CTS operation modes<sup>3</sup> | Cycle through VG color modes.<sup>4</sup> |
| <p align="center">4</p> | Smoke&nbsp;Settings&nbsp;/&nbsp;Overheating| Enable/Disable smoke effects. | Enable/Disable overheating. |
| <p align="center">3</p> | Cyclotron&nbsp;LED&nbsp;Control | Change Cyclotron rotation. | Toggle between 1 (centered) or 3 LEDs in the Cyclotron Lid for 1984/1989 modes. |
| <p align="center">2</p> | Vibration Settings | Proton Pack Enable/Disable vibration or vibration during firing. | Neutrona Wand Enable/Disable vibration or vibration during firing. |
| <p align="center">1</p> | Year&nbsp;Mode&nbsp;Selection | Cycle through Afterlife (2021), 1984, or 1989 modes. | Enable or Disable the Proton Stream impact sound effects. |

<sup>3</sup> Switch between Video Game and Cross the Streams (CTS) operation modes. Note that the LED/segment #5 will stay solid when CTS is enabled while in the sub menu system. Remember that CTS Mix requires holding down both intensify and the alternate firing button at the same time.

<sup>4</sup> Enables/disables the video game colour modes for the Proton Pack's Power Cell and Cyclotron lights. This only affects the pack when in Video Game mode, not for CTS modes, and offers the following states:

- Disable all the video game colours (Power Cell remains blue, Cyclotron remains red).
- Enable only the Power Cell LEDs to change colors (requires RGB Power Cell such as that offered by Frutto Technology).
- Enable only the Cyclotron LEDs to change colors (requires RGB replacement such as a Neopixel ring).
- Enable both the Power Cell and Cyclotron to use video game colours.

To exit the menu system, navigate to the top menu in **Setting 5** (Slo-blo LED is no longer illuminated) and press the Barrel Wing Button. A small beep will be heard when transitioning between the top and sub menu levels.

`When music is playing while the pack and wand are turned off, you can use the Top Dial on the top of the Neutrona Wand to easily access the independent music volume control.`

Refer to the [Loading The Audio Files](AUDIO.md) for more information on loading additional music tracks.

## EEPROM Menus
There are two different EEPROM menus that can be accessed to adjust and save configurations onto the Proton Pack and Neutrona Wand EEPROM memory. The EEPROM LED Menu and the EEPROM Configuration Menu.

`The EEPROM memory has a specified life of 100,000 write/erase cycles. You will generally not reach this limit in your life time, but if you did, it can be disabled to bypass it. Refer to the Advanced User Configurations below for more information.`

**EEPROM LED Menu**

The EEPOM LED Menu can be used to configure different LED setups for your Proton Pack. All changes can be saved directly to the Proton Pack EEPROM and will be loaded automatically even if you turn off the power to your devices.

To access the EEPROM LED Menu System, hold the `Intensify Button` down and toggle the right hand `Top Toggle` button 5 times. You will then hear a beep noise and your bargraph will light up to indicate you are in the EEPROM LED Menu System. The Neutrona Wand barrel and all the Proton Pack LEDs will light up to the default Spectral Custom colours when you enter the EEPROM LED Menu.

`The EEPROM LED Menu System can only be reached while both the Neutrona Wand and Proton Pack are powered down.`

![](images/OperationNeutrona11.jpg)

| EEPROM&nbsp;LED<br>Menu&nbsp;Level | Purpose | Intensify&nbsp;Button | Barrel&nbsp;Wing&nbsp;Button |
|----------------|---------|------------------|--------------------|
| <p align="center">5</p> | Save or Clear the EEPROM settings | Clear all the LED settings in the Proton Pack EEPROM.<br><br>***The Proton Pack will instead load the software defined defaults the next time you turn the battery power off.***| Save the current settings to the Proton Pack EEPROM. |
| <p align="center">4</p> | Cyclotron LED Count Toggle / Spectral Custom (Barrel) Colour| Toggle between 40, 20 or 12 LEDs for your Cyclotron. | `(Barrel Wing Button + Top Dial)` Changes the colour hue of the Neutrona Wand barrel for the Spectral Custom mode. |
| <p align="center">3</p> | Power Cell LED Count Toggle / Spectral Custom (Power Cell) Colour | Toggle between 15 or 13 LEDs for your Power Cell. | `(Barrel Wing Button + Top Dial)` Changes the colour hue of the Power Cell for the Spectral Custom mode. |
| <p align="center">2</p> | Inner Cyclotron LED Count Toggle  / Spectral Custom (Cyclotron) Colour | Toggle Between 35, 24, 23 or 12 LEDs for your Inner Cyclotron Cake. | `(Barrel Wing Button + Top Dial)` Changes the colour hue of the Cyclotron for the Spectral Custom mode. |
| <p align="center">1</p> | Inner Cyclotron GRB Toggle / Spectral Custom (Inner Cyclotron) Colour | Toggle your Inner Cyclotron from RGB or GRB LEDs. | `(Barrel Wing Button + Top Dial)` Changes the colour hue of the Inner Cyclotron (Cake) for the Spectral Custom mode. |

**EEPROM Configuration Menu**

The EEPROM Configuration Menu can be used to save certain behaviours of your Neutrona Wand and Proton Pack as the standard settings.

To access the EEPROM Configuration Menu, hold the `Intensify Button` down and toggle the right hand `Bottom Toggle` button 5 times. You will then hear a beep noise and your bargraph will light up to indicate you are in the EEPROM Configuration Menu System.

To access the EEPROM Configuration sub level menu, use the `Top Dial` on the top of the wand. When you reach the sub level menu, the Slo-blo LED will light up to indicate this menu state.

`The EEPROM Configuration Menu System can only be reached while both the Neutrona Wand and Proton Pack are powered down.`

`Please note whatever settings you have temporarily changed in the regular menus and sub menus will be applied automatically and saved if you exit the EEPROM menu by saving the settings.`

![](images/OperationNeutrona12.jpg)

| EEPROM&nbsp;Configuration<br>Menu&nbsp;Level | Purpose | Intensify&nbsp;Button | Barrel&nbsp;Wing&nbsp;Button |
|----------------|---------|------------------|--------------------|
| <p align="center">5</p> | Save or Clear the EEPROM settings | Clear all the configuration only settings in the Proton Pack and Neutrona Wand EEPROM.<br><br>***The Proton Pack and Neutrona Wand will instead load the software defined defaults the next time the battery power is turn back on.***| Save the current settings to the Proton Pack and Neutrona Wand EEPROM. |
| <p align="center">4</p> | Firing&nbsp;Mode&nbsp;Selection / Spectral Modes | Cycle through VG/CTS operation modes to set as your default.<sup>3</sup> | Enable or Disable the Spectral Modes. |
| <p align="center">3</p> | Overheating / Smoke | Enable or Disable overheating. | Enable or Disable smoke. |
| <p align="center">2</p> | Cyclotron&nbsp;LED&nbsp;Control | Cyclotron rotation direction. Clockwise or counter clockwise. | Enable or Disable the Cyclotron Lid Ring Simulation for non ring LED setups in Afterlife (2021) mode. |
| <p align="center">1</p> | Sound Effects | Enable or Disable the proton stream impact sound effects. | Enable or disable extra Neutrona Wand sound effects to be played by the Proton Pack. |

<sup>3</sup> Choose between Video Game and Cross the Streams (CTS) operation modes. Cross the Streams modes is like what you see in the films.

| EEPROM&nbsp;Sub&nbsp;Level&nbsp;Menu | Purpose | Intensify Button | Barrel Wing Button |
|----------------|---------|------------------|--------------------|
| <p align="center">5</p> | Quick&nbsp;Vent / Wand Boot Errors | Enable or Disable the Quick Venting feature. When enabled, you can manually vent your Proton Pack by pressing the Intensify button while the top right toggle switch is switched down. | Enable to disable the Wand boot error mode. When enabled, if you turn on the Neutrona Wand while the top right toggle is up and activated, the Neutrona Wand enters an error mode and remains locked out until the switches are all turned off. |
| <p align="center">4</p> | Auto Vent Light Intensity / Barrel LED Count | Enable or disable the Auto Vent Light Intensity. When enabled, the vent light will adjust the intensity based on the power level of the wand. | Set the barrel led count between 5 (default), 48 and 60. |
| <p align="center">3</p> | Invert Bargraph Adjustment / 28 Segment Bargraph Ramping | Invert or do not invert the bargraph in the Neutrona Wand. | Enable or disable the optional 28 segment bargraph to be always ramping. When enabled, Afterlife mode will have the same bargraph effect as 1984/1989. |
| <p align="center">2</p> | Overheat Lights | Enable or Disable the N-Filter LEDs from strobing during overheating. | Enable or disable the Proton Pack lights to turn off during overheating. When disabled, after ramping down to a slower pace, the lights stay on.
| <p align="center">1</p> | Default Year&nbsp;Mode / Overheat Sync to Fan | Set your default year mode between 1984/1989/Afterlife/Default. When set, the system will ignore the year mode toggle switch during bootup. However the toggle switch can still be used while the Proton Pack is on. If set to Default, the system picks the year mode based on the position of the year mode toggle switch in the Proton Pack. | Enable or Disable the overheat sync to fan. When enabled, the fan connections in the Proton Pack will only activate at the same time as the smoke connections. When disabled, the fan connections activate slightly after the smoke connections which allows some build of smoke before the fans activate. |

## Advanced User Configurations

To modify defaults within the software, please see the [Advanced Configuration](ADVCONFIG.md) guide.

## PDF Operational Guide
(Direct Download link)
[gpstar Operational Guide [pdf]](https://github.com/gpstar81/haslab-proton-pack/raw/main/extras/gpstar-Operational-Guide.pdf)

## Video Demos
Video Demo: [Features and Menu Walkthrough](https://www.youtube.com/watch?v=ePXz99UawLQ) (YouTube, July 2023)
[![Haslab Proton Pack Arduino Powered Demo & Features](https://img.youtube.com/vi/ePXz99UawLQ/maxresdefault.jpg)](https://www.youtube.com/watch?v=ePXz99UawLQ)

Wand Menu Settings Video Demo: [Proton Pack Wand Menu Settings Video Demo](https://www.youtube.com/watch?v=QrevSoQo_3M) (YouTube)
[![Proton Pack Wand Menu Settings Video Demo](https://img.youtube.com/vi/QrevSoQo_3M/maxresdefault.jpg)](https://youtu.be/QrevSoQo_3M)
<br>
<br>
<br>
**Neutrona Wand renders courtesy of Luis Peña*
