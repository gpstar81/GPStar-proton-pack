<h1><img src='images/gpstar_logo.png' width=50 align="left"/>gpstar Proton Pack and Neutrona Wand Operation Manual</h1>

# EEPROM Menus

There are two EEPROM menus available which can be accessed to adjust and save configurations onto either the Proton Pack and Neutrona Wand EEPROM memory. These are the **EEPROM LED Menu** and the **EEPROM Configuration Menu**.

**NOTICE:** The EEPROM memory has a specified life of 100,000 write/erase cycles. You will generally not reach this limit in your lifetime, but if you did, it can be disabled to bypass it. Refer to the Advanced User Configurations below for more information.

## EEPROM LED Menu

The EEPOM LED Menu can be used to configure different LED setups for your Proton Pack. All changes can be saved directly to the Proton Pack EEPROM and will be loaded automatically even if you turn off the power to your devices.

To access the EEPROM LED Menu System, hold down the `Intensify Button` and toggle the right hand `Top Toggle` button 5 times (up and down). You will then hear a beep noise and your bargraph will light up to indicate you are in the EEPROM LED Menu System. The Neutrona Wand barrel and all the Proton Pack LEDs will light up to the default Spectral Custom colours when you enter the EEPROM LED Menu.

**Note:** The EEPROM LED Menu System can only be reached while both the Neutrona Wand and Proton Pack are powered down.

![](images/OperationNeutrona11.jpg)

| EEPROM&nbsp;LED<br>Menu&nbsp;Level&nbsp;1 | Purpose | Intensify&nbsp;Button | Barrel&nbsp;Wing&nbsp;Button |
|----------------|---------|------------------|--------------------|
| <p align="center">5</p> | Save or Clear the EEPROM settings | Clear all the LED settings in the Proton Pack EEPROM.<br><br>***The Proton Pack will instead load the software defined defaults the next time you turn the battery power off.***| Save the current settings to the Proton Pack EEPROM. |
| <p align="center">4</p> | Cyclotron LED Count Toggle / Spectral Custom (Barrel) Colour| Toggle between 40, 20 or 12 LEDs for your Cyclotron. | `(Barrel Wing Button + Top Dial)` Changes the colour hue of the Neutrona Wand barrel for the Spectral Custom mode. |
| <p align="center">3</p> | Power Cell LED Count Toggle / Spectral Custom (Power Cell) Colour | Toggle between 15 or 13 LEDs for your Power Cell. | `(Barrel Wing Button + Top Dial)` Changes the colour hue of the Power Cell for the Spectral Custom mode. |
| <p align="center">2</p> | Inner Cyclotron LED Count Toggle  / Spectral Custom (Cyclotron) Colour | Toggle Between 35, 24, 23 or 12 LEDs for your Inner Cyclotron Cake. | `(Barrel Wing Button + Top Dial)` Changes the colour hue of the Cyclotron for the Spectral Custom mode. |
| <p align="center">1</p> | Inner Cyclotron GRB Toggle / Spectral Custom (Inner Cyclotron) Colour | Toggle your Inner Cyclotron from RGB or GRB LEDs. | `(Barrel Wing Button + Top Dial)` Changes the colour hue of the Inner Cyclotron (Cake) for the Spectral Custom mode. |

## EEPROM Configuration Menu

The EEPROM Configuration Menu can be used to save certain behaviours of your Neutrona Wand and Proton Pack as the standard settings.

To access the EEPROM Configuration Menu, hold down the `Intensify Button` and toggle the right hand `Bottom Toggle` button 5 times (up and down). You will then hear a beep noise and your bargraph will light up to indicate you are in the EEPROM Configuration Menu System.

To access the EEPROM Configuration menus, use the `Top Dial` on the top of the wand. When you reach the a new menu level, the Slo-blo LED and other lights on the Neutrona Wand will light up to indicate these menu levels, along with audio cues from the system.

	Menu Level 1 -> No lights
	Menu Level 2 -> Slo-Blo
	Menu Level 3 -> Slo-Blo and vent light
	Menu Level 4 -> Slo-Blo, vent light, and top gun box light
	Menu Level 5 -> Slo-Blo; vent light, top gun box light, and front gun box light

**The EEPROM Configuration Menu System can only be reached while both the Neutrona Wand and Proton Pack are powered down.**

Please note whatever settings you have temporarily changed in the regular menus and sub menus will be applied automatically and saved if you exit the EEPROM menu by saving the settings.

![](images/OperationNeutrona12.jpg)

| EEPROM&nbsp;Configuration<br>Menu&nbsp;Level&nbsp;1 | Purpose | Intensify&nbsp;Button | Barrel&nbsp;Wing&nbsp;Button |
|----------------|---------|------------------|--------------------|
| <p align="center">5</p> | Save or Clear the EEPROM settings | Clear all the configuration only settings in the Proton Pack and Neutrona Wand EEPROM.<br><br>***The Proton Pack and Neutrona Wand will instead load the software defined defaults the next time the battery power is turn back on.***| Save the current settings to the Proton Pack and Neutrona Wand EEPROM. |
| <p align="center">4</p> | Firing&nbsp;Mode&nbsp;Selection / Spectral Modes | Cycle through VG/CTS operation modes to set as your default.<sup>3</sup> | Enable or Disable the Spectral Modes. |
| <p align="center">3</p> | Overheating / Smoke | Enable or Disable overheating. | Enable or Disable smoke. |
| <p align="center">2</p> | Cyclotron&nbsp;LED&nbsp;Control | Cyclotron rotation direction. Clockwise or counter clockwise. | Enable or Disable the Cyclotron Lid Ring Simulation for non ring LED setups in Afterlife (2021) mode. |
| <p align="center">1</p> | Sound Effects | Enable or Disable the proton stream impact sound effects. | Enable or disable extra Neutrona Wand sound effects to be played by the Proton Pack. |

<sup>3</sup> Choose between Video Game and Cross the Streams (CTS) operation modes. Cross the Streams modes is like what you see in the films.

| EEPROM&nbsp;Configuration<br>Menu&nbsp;Level&nbsp;2 | Purpose | Intensify Button | Barrel Wing Button |
|----------------|---------|------------------|--------------------|
| <p align="center">5</p> | Quick&nbsp;Vent / Wand Boot Errors | Enable or Disable the Quick Venting feature. When enabled, you can manually vent your Proton Pack by pressing the Intensify button while the top right toggle switch is switched down. | Enable to disable the Wand boot error mode. When enabled, if you turn on the Neutrona Wand while the top right toggle is up and activated, the Neutrona Wand enters an error mode and remains locked out until the switches are all turned off. |
| <p align="center">4</p> | Auto Vent Light Intensity / Barrel LED Count | Enable or disable the Auto Vent Light Intensity. When enabled, the vent light will adjust the intensity based on the power level of the wand. | Set the barrel led count between 5 (default), 48 and 60. |
| <p align="center">3</p> | Mode Original Toggle Switch Sounds / VGA Colour Mode Toggles | Enable or disable the toggle switch sound effects for the mode original system mode. | Cycle through VG color modes.<sup>5</sup> |
| <p align="center">2</p> | Overheat Lights | Enable or Disable the N-Filter LEDs from strobing during overheating. | Enable or disable the Proton Pack lights to turn off during overheating. When disabled, after ramping down to a slower pace, the lights stay on.
| <p align="center">1</p> | Default Year&nbsp;Mode / Overheat Sync to Fan | Set your default year mode between 1984/1989/Afterlife/Default. When set, the system will ignore the year mode toggle switch during bootup. However the toggle switch can still be used while the Proton Pack is on. If set to Default, the system picks the year mode based on the position of the year mode toggle switch in the Proton Pack. | Enable or Disable the overheat sync to fan. When enabled, the fan connections in the Proton Pack will only activate at the same time as the smoke connections. When disabled, the fan connections activate slightly after the smoke connections which allows some build of smoke before the fans activate. |

<sup>5</sup> Enables/disables the video game colour modes for the Proton Pack's Power Cell and Cyclotron lights. This only affects the pack when in Video Game mode, not for CTS modes, and offers the following states:

- Disable all the video game colours (Power Cell remains blue, Cyclotron remains red).
- Enable only the Power Cell LEDs to change colors (requires RGB Power Cell such as that offered by Frutto Technology).
- Enable only the Cyclotron LEDs to change colors (requires RGB replacement such as a Neopixel ring).
- Enable both the Power Cell and Cyclotron to use video game colours.

| EEPROM&nbsp;Configuration<br>Menu&nbsp;Level&nbsp;3 | Purpose | Intensify Button | Barrel Wing Button |
|----------------|---------|------------------|--------------------|
| <p align="center">5</p> | System Default Volume / Neutrona Wand Year Mode | Hold the `Intensify Button` down while turning the `top dial` to adjust and set the default volume for the system. | Set the year mode your Neutrona Wand operates in. Options are: 1984/1989/Afterlife and System Default. When set to System Default, the Neutrona Wand matches the same year mode as that the Proton Pack is in. |
| <p align="center">4</p> | Invert Bargraph Adjustment / Bargraph Overheat Blinking | Invert or do not invert the bargraph in the Neutrona Wand. | Enable or disable the bagraph to blink on and off during overheat. When disabled, the bargraph will ramp down and turn off during the overheat sequence before ramping back up when overheating has finished. |
| <p align="center">3</p> | Bargraph Animations / Bargraph Firing Animations | Toggle between Super Hero / Bargraph Original / System Default as the default idle animations of the bargraph. `System default chooses the default animations based on the year mode and system mode.` | Toggle between Super Hero / Bargraph Original / System Default as the default firing animations of the bargraph. `System default chooses the default animations based on the year mode and system mode.` |
| <p align="center">2</p> | Demo Light Mode / Cyclotron 3 LED Toggle | Enable or disable the Demo Light mode. When enabled, the Proton Pack and Neutrona Wand will automatically boot up as soon as the system has power. | Toggle between 1 LED or 3 LEDs in each cyclotron lens in 1984/1989 year mode. |
| <p align="center">1</p> | System&nbsp;Mode / CTS Override | Toggle between the two different system operation modes. Super Hero or Mode Original. | Toggle between the different CTS modes as your default CTS mode. 1984/1989, Afterlife or System Default. System default automatically chooses the CTS mode based on the current year mode of your system. |

| EEPROM&nbsp;Configuration<br>Menu&nbsp;Level&nbsp;4 | Purpose | Intensify Button | Barrel Wing Button |
|----------------|---------|------------------|--------------------|
| <p align="center">5</p> | Over Heat Smoke Duration Level 5 / Over Heat Start Timer Level 5 | `(Intensify + Top Dial)` to adjust the smoke vent duration during overheat for power level 5. `Range: 2 to 60 seconds` `Default: 6 seconds` | `(Intensify + Top Dial)` to adjust the over heat start for power level 5. `Range: 2 to 60 seconds` `Default: 12 seconds` |
| <p align="center">4</p> | Over Heat Smoke Duration Level 4 / Over Heat Start Timer Level 4 | `(Intensify + Top Dial)` to adjust the smoke vent duration during overheat for power level 4. `Range: 2 to 60 seconds` `Default: 5 seconds` | `(Intensify + Top Dial)` to adjust the over heat start for power level 4. `Range: 2 to 60 seconds` `Default: 15 seconds` |
| <p align="center">3</p> | Over Heat Smoke Duration Level 3 / Over Heat Start Timer Level 3 | `(Intensify + Top Dial)` to adjust the smoke vent duration during overheat for power level 3. `Range: 2 to 60 seconds` `Default: 4 seconds` | `(Intensify + Top Dial)` to adjust the over heat start for power level 3. `Range: 2 to 60 seconds` `Default: 20 seconds` |
| <p align="center">2</p> | Over Heat Smoke Duration Level 2 / Over Heat Start Timer Level 2 | `(Intensify + Top Dial)` to adjust the smoke vent duration during overheat for power level 2. `Range: 2 to 60 seconds` `Default: 3 seconds` | `(Intensify + Top Dial)` to adjust the over heat start for power level 2. `Range: 2 to 60 seconds` `Default: 30 seconds` |
| <p align="center">1</p> | Over Heat Smoke Duration Level 1 / Over Heat Start Timer Level 1 | `(Intensify + Top Dial)` to adjust the smoke vent duration during overheat for power level 1. `Range: 2 to 60 seconds` `Default: 2 seconds` | `(Intensify + Top Dial)` to adjust the over heat start for power level 1. `Range: 2 to 60 seconds` `Default: 60 seconds` |

### WARNING

**Running your smoke devices for long periods can damage them. Adjust these settings with caution and use at your own risk!**

| EEPROM&nbsp;Configuration<br>Menu&nbsp;Level&nbsp;5 | Purpose | Intensify Button | Barrel Wing Button |
|----------------|---------|------------------|--------------------|
| <p align="center">5</p> | Over Heat Level 5 / Continuous Smoke Level 5 | Enable or disable over heating in power level 5. `Default: enabled` | Enable or disable continuous firing smoke effects in power level 5. These smoke effects will randomly activate for very short bursts while continuously firing for long periods. `Default: enabled` |
| <p align="center">4</p> | Over Heat Level 4 / Continuous Smoke Level 4 | Enable or disable over heating in power level 4. `Default: disabled` | Enable or disable continuous firing smoke effects in power level 4. These smoke effects will randomly activate for very short bursts while continuously firing for long periods. `Default: enabled` |
| <p align="center">3</p> | Over Heat Level 3 / Continuous Smoke Level 3 | Enable or disable over heating in power level 3. `Default: disabled` | Enable or disable continuous firing smoke effects in power level 3. These smoke effects will randomly activate for very short bursts while continuously firing for long periods. `Default: enabled` |
| <p align="center">2</p> | Over Heat Level 2 / Continuous Smoke Level 2 | Enable or disable over heating in power level 2. `Default: disabled` | Enable or disable continuous firing smoke effects in power level 2. These smoke effects will randomly activate for very short bursts while continuously firing for long periods. `Default: enabled` |
| <p align="center">1</p> | Over Heat Level 1 / Continuous Smoke Level 1 | Enable or disable over heating in power level 1. `Default: disabled` | Enable or disable continuous firing smoke effects in power level 1. These smoke effects will randomly activate for very short bursts while continuously firing for long periods. `Default: enabled` |

# Advanced User Configurations

To modify defaults within the software rather than the EEPROM menus, please see the [Advanced Configuration](ADVCONFIG.md) guide.
