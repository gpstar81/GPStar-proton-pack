<h1><img src='images/gpstar_logo.png' width=50 align="left"/>GPStar Proton Pack and Neutrona Wand</h1>

### Summary

A fully integrated Proton Pack and Neutrona Wand, packed with features and add-ons. Compatible with the HasLab Plasma Series Proton Pack and Neutrona Wand

- Polyphonic sound
  - Allows simultaneous playback of multiple audio tracks which can be layered together to create a rich experience with smoothly layered sound effects and buttery smooth transitions.
  - While blasting away with the Proton stream you can still hear all the layers of sound effects that came before...the Pack effects, each Wand toggle effect, Music etc.
- Wand / Pack communication system
   - By implementing both the pack and wand controllers with an upgraded hose connection, both devices work seamlessly together. Interactions with the wand have an immediate effect on the pack, and vice-versa.
   - This functionality includes sound effects, alarm triggers, Cyclotron interaction, venting/overheat sequences, and proton stream effects (eg. video game colours and crossing the streams).

## Feature Reel Demonstration ##

Just see for yourself what this kit can do, and you'll be ready to believe us!
[![GPStar Proton Pack and Neutrona Wand feature reel demonstration](https://img.youtube.com/vi/lDD9TEip7_s/maxresdefault.jpg)](https://www.youtube.com/watch?v=lDD9TEip7_s)

## Table of Contents

*"Ray, pretend for a moment that I don't know anything about metallurgy, engineering, or physics, and just tell me what the hell is going on." -Dr. Venkman*

That's alright, this will help you study. Below you will find the various guides to build out your new electronic brains, and maybe find some cool new features to implement as part of the addendums.

---

<img src='images/gpstar_logo.png' width=50 align="left"/><h3>GPStar Proton Pack and Neutrona Wand PCB Guide</h3>These are the most important guides which will get you familiar with the major components and how to update them with the latest capabilities offered by software and sound effects. This includes guides to aid you in completing the connections and installation for the GPStar Proton Pack and Neutrona Wand PCB boards.

1. [Pack PCB](PACK_PCB.md)

1. [Wand PCB](WAND_PCB.md)

1. [Flashing Firmware Updates](FLASHING.md)

1. [GPStar Audio PCB](GPSTAR_AUDIO_PCB.md)

1. [Loading The Audio Files](AUDIO.md)

1. [Installation Video & Guide](INSTALL_GUIDE.md)

1. [Troubleshooting](TROUBLESHOOTING.md)

For more information about optional upgrades please see the "Component Guides" section below.

---

### Serial1 Expansion Controller

This is a device which utilize the available expansion port on the GPStar Proton Pack controller. At present the only supported uses are as an "Pack Attenuator" (using an ESP32 or Arduino Nano) or a "Wireless Adapter" (using an ESP32). Both devices use the same software which normally drives the Attenuator device, though in the case of the wireless adapter can be used without switches or other peripherals to only provide a WiFi capability to your Proton Pack. Please read the following guides for more specific details.

1. [Pack Attenuator](ATTENUATOR.md) - Full Device Implementation (w/ Switches and Inputs)

1. [Wireless Adapter](WIRELESS.md) - Scaled-Down Attenuator (No Physical User Inputs)

---

### By-Component Guides

This section is presented in a way to offer a more complete view of what components are available (by major equipment areas), whether those components are Required<sup>R</sup> or Optional<sup>O</sup>, if choices are offerred at a specific component level. Where possible, each option will link to a related guide for more details and/or installation instructions.

**Proton Pack**

| **Component**          | **Option w/ Guide** |
|------------------------|---------------------------------------------|
| Power Source<sup>R</sup>    | [TalentCell 3000mAh or 6000mAh](POWER.md) |
| Controller<sup>R</sup>      | [GPStar Pack Controller](PACK_PCB.md) or [Arduino 2560](PACK.md) |
| Software<sup>R</sup>        | [Flashing the Latest Firmware](FLASHING.md) |
| Audio Board<sup>R</sup>     | [GPStar Audio](GPSTAR_AUDIO_PCB.md) or [Robertsonics WAV Trigger](WAVTRIGGER.md) |
| Speakers/Amp<sup>R</sup>    | [Audio Output Guide](SOUND.md) |
| Sound Effects<sup>R</sup>   | [Loading The Audio Files](AUDIO.md) |
| Cyclotron Lid<sup>O</sup>   | [12/20/36-LED options](CYCLOTRON_LID.md) |
| Inner Cyclotron<sup>O</sup> | [Cake and Panel Lights](CYCLOTRON_INNER.md)
| N-Filter<sup>O</sup>        | [7-LED Vent Light](NFILTER.md) |
| Powercell<sup>O</sup>       | [13 and 15-LED options](POWERCELL.md) |
| Smoke Effects<sup>O</sup>   | [Various Solutions Supported](SMOKE.md) |

**Connectivity**

| **Component**   | **Option w/ Guide** |
|-----------------|---------------------------------------------|
| Wired<sup>R</sup>    | [Pack-Wand Hose Connections](HOSE.md)|
| Wireless<sup>O</sup> | [Pack Attenuator](ATTENUATOR.md) or [Wireless Adapter](WIRELESS.md) |

**Neutrona Wand**

| **Component**        | **Option w/ Guide** |
|----------------------|---------------------------------------------|
| Power Source<sup>R</sup>  | Dedicated Wand Power Source (Guide TBD) |
| Controller<sup>R</sup>    | [GPStar Wand Controller](WAND_PCB.md) |
| Software<sup>R</sup>      | [Flashing the Latest Firmware](FLASHING.md) |
| Audio Board<sup>R</sup>   | [GPStar Audio](GPSTAR_AUDIO_PCB.md) or [Robertsonics WAV Trigger](WAVTRIGGER.md) |
| Speakers<sup>R</sup>      | [Audio Output Guide](SOUND.md) |
| Sound Effects<sup>O</sup> | [Loading The Audio Files](AUDIO.md) |
| Hat Lights<sup>O</sup>    | [Kit option by Jonogunn](HATS.md) |
| Bargraph<sup>O</sup>      | [28-Segment or 30-Segment](BARGRAPH.md) |
| Vent Light<sup>O</sup>    | [High Intensity option by Frutto Technology](VENTLIGHT.md) |
| Wand Barrel<sup>O</sup>   | 2, 5, 49 and 50 LED options (Guide TBD) |

**Installation, Operation, and Troubleshooting**

| **Task**         | **Guide** |
|------------------|---------------------------------------------|
| Initial Assembly | [Installation - Video & Guide](INSTALL_GUIDE.md) |
| System Operation | [Equipment Operation Manual](OPERATION.md) |
| Configuration    | [EEPROM LED/Config Menus](OPERATION_EEPROM.md) |
| I Need Help!     | [Troubleshooting](TROUBLESHOOTING.md) and [Frequently Asked Questions](FAQ.md) |

**Software Development (Advanced Topics)**

| **Task**         | **Guide** |
|------------------|---------------------------------------------|
| Modify Software  | [Advanced Software Configuration](ADVCONFIG.md) |
| Upload Software  | [Pack/Wand Flashing](COMPILING_FLASHING.md) or [Attenuator/Wireless Flashing](ATTENUATOR_FLASHING.md) |

**Special/Standalone Devices**

| **Devices**         | **Guide** |
|------------------|---------------------------------------------|
| Single-Shot Blaster | [Build and Operation Guide](SINGLESHOT.md) |

---

### DIY Arduino Build Guides

Considered the "legacy" guide to building your own solution using the Arduino platform. *Please note that the original Neutrona Wand guide used an Arduino Nano which is no longer capable of providing the storage space required by the full range of features supported and that hardware has been deprecated. In order to continue with these guides, please note that a replacement board has been identified for the Neutrona Wand and MUST be used with the latest software release.*

This is an advanced build-out which requires opening all devices and performing a significant amount of soldering. **Proceed with caution, regardless of your skillset or abilities.**

1. [Bill of Materials](DIY_BOM.md)

1. [Power Requirements](POWER.md)

1. [Compiling and Uploading the Code](DIY_FLASHING.md)

1. [GPStar Audio PCB](GPSTAR_AUDIO_PCB.md) or [WAV Trigger Configuration](WAVTRIGGER.md)

1. [Loading The Audio Files](AUDIO.md)

1. [Audio Output Guide](SOUND.md)

1. [DIY Pack Setup - Arduino Mega 2560 R3](DIY_PACK.md)

1. [DIY Wand Setup - Mega 2560 Pro Mini](DIY_WAND.md)

1. [Hose Connections](HOSE.md)

## Walkthrough Videos ##

*"You know, it just occurred to me that we really haven't had a successful test of this equipment." -Ray Stantz*

Except that we have! Here is a [Walkthrough Video Contributed by JustinDustin](https://www.youtube.com/watch?v=mnfljGd5-uU) (YouTube, March 2023) showcasing several of the stock features in a converted pack/wand combination. Several optional features were implemented as part of this build using the Arduino platform.
[![Haslab Proton Pack Arduino Powered Walkthrough Video](https://img.youtube.com/vi/mnfljGd5-uU/maxresdefault.jpg)](https://www.youtube.com/watch?v=mnfljGd5-uU)

Additionally, this video covers several new updates in the months since, using the new GPStar controllers, as the [Optional Features and Menu Walkthrough](https://www.youtube.com/watch?v=ePXz99UawLQ) (YouTube, July 2023).
[![Optional Features and Menu Walkthrough Video](https://img.youtube.com/vi/ePXz99UawLQ/maxresdefault.jpg)](https://www.youtube.com/watch?v=mnfljGd5-uU)

## Viewing MD Files

GitHub should automatically display these files with basic formatting, though to view these MD (MarkDown) files linked above with full formatting including color-coding, you can use a program such as [MarkdownPad](http://markdownpad.com/) (Windows) or [MacDown](https://macdown.uranusjr.com/) (MacOS). Alternatively, you may copy the contents into [an online viewer](https://markdownlivepreview.com/).

### "Yes, have some"...music!

Special thanks to Michael Klodzinksi for graciously allowing us to include his version of [Savin' The Day](https://www.youtube.com/watch?v=shJslMSAxE0) as a bundled music file to demo your awesome Proton Pack mods! Check out his other works at [michaelk.net](https://michaelk.net).

## Sound files

The sound effects files are combination of self made files for this project to ones from within the Ghostbusters community, etc. Apologies if it is forgotten to acknowledge where some of the files originate from.

## Licensing

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

## Acknowledgements

This project is made possible by Ghostbusters community makers and volunteers spanning 4 countries over 2 continents.

* Michael Rajotte (aka. "GPStar Technologies") - Director of Operations, Software Architect, Product Designer, PCB Design, 3D Modelling, EU & Worldwide Distribution
* Jonathan Gunn (aka. "Jonogunn") - Video Instruction Guides, Install Testing.
* Dustin Grau (aka. "JustinDustin") - Prototyping and Testing, Documentation, 3D Modelling, Software Support
* Angie Rutto (aka. "Frutto Technology") - Maker and purveyor of fine add-on products, UK Distribution

### Special Thanks

* Emre Sahare (GBFans: One Time) - For the original prop mode "Mode Original" operation deciphering.
* Alexander Hibbs (@BeaulieuDesigns87) - For his equipment schematic as used in the WiFi web UI.
* Nomake Wan - For continued Software Support and Feature Development

Thank you to our supporters on [GBFans.com](https://www.gbfans.com/forum) and the [Facebook group](https://www.facebook.com/groups/gpstartechnologies).

For those who are curious you can view a condensed [history of how this project began](HISTORY.md).

## Disclaimer

<sub>
This community-driven project is independent and not affiliated with, endorsed by, or sponsored by Hasbro Inc., Ghost Corps, or Sony Pictures. Hasbro Inc. does not endorse or support this project, and any views or opinions expressed within the project are those of the individual contributors and not necessarily those of Hasbro Inc.
</sub>
<br/>
<br/>
<sub>
Participants in this project should be aware that it is entirely separate from any official activities or initiatives of Hasbro Inc.. Any use of Hasbro Inc.'s name or its products within this project is purely for informative purposes and does not imply any form of partnership, endorsement, or association with Hasbro Inc.
</sub>
<br/>
<br/>
<sub>
Individuals involved in this project are responsible for their own actions, and Hasbro Inc. bears no responsibility for the content, decisions, or outcomes related to this community-driven effort.
</sub>
<br/>
<br/>
<sub>
By participating in this project, individuals acknowledge that it is an independent initiative and that Hasbro Inc. is not responsible for the project's development, management, or outcomes.
</sub>
<br/>
<br/>
<sub>
This disclaimer is subject to change, and individuals are encouraged to check for updates regularly.
</sub>
