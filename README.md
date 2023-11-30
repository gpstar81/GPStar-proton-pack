<h1><img src='images/gpstar_logo.png' width=50 align="left"/>GPStar Proton Pack and Neutrona Wand</h1>

<h3>Microcontroller Replacements for the HasLab Plasma Series Proton Pack and Neutrona Wand</h3>

This repository outlines the full replacement of the stock microcontrollers which run the Proton Pack and Neutrona Wand while retaining the use and function of all existing controls and lighting in these HasLab products.

### Summary

A fully integrated Proton Pack and Neutrona Wand, packed with features and add-ons.

- Polyphonic sound
  - Allows simultaneous playback of multiple audio tracks which can be layered together to create a rich experience with smoothly layered sound effects and buttery smooth transitions.
  - While blasting away with the Proton stream you can still hear all the layers of sound effects that came before...the Pack effects, each Wand toggle effect, Music etc.
- Wand / Pack communication system
   - By implementing both the pack and wand controllers with an upgraded hose connection, both devices work seamlessly together. Interactions with the wand have an immediate effect on the pack, and vice-versa.
   - This functionality includes sound effects, alarm triggers, Cyclotron interaction, venting/overheat sequences, and proton stream effects (eg. video game colours and crossing the streams).

## Table of Contents

*"Ray, pretend for a moment that I don't know anything about metallurgy, engineering, or physics, and just tell me what the hell is going on." -Dr. Venkman*

That's alright, this will help you study. Below you will find the various guides to build out your new electronic brains, and maybe find some cool new features to implement as part of the addendums.

---

<img src='images/gpstar_logo.png' width=50 align="left"/><h3>GPStar Proton Pack and Neutrona Wand PCB Guide</h3>This guide is intended for completing the connections and installation for the GPStar Proton Pack and Neutrona Wand PCB boards.

1. [Pack PCB](PACK_PCB.md)

1. [Wand PCB](WAND_PCB.md)

1. [Installation Video & Guide](INSTALL_GUIDE.md)

1. [Loading The Audio Files](AUDIO.md)

1. [Flashing Updates](FLASHING.md)

1. [Troubleshooting](TROUBLESHOOTING.md)

---

### Serial Expansion Controller

These are devices which utilize the available expansion port on the GPStar Proton Pack controller. Both devices are based on the same software which drives the Attenuator device, but can also be used without inputs or other devices to provide a WiFi capability to your Proton Pack. Please read the guides for more specific details.

Note: As of November 2023 these are DIY devices! Parts for the Wireless Adapter can be ordered 100% from Amazon and can be assembled without soldering. However, the Attenuator will require some minor soldering to make use of all features available.

1. [Wireless Adapter](WIRELESS.md) - Scaled-Down Attenuator

1. [Pack Attenuator](ATTENUATOR.md) - Full Device Implementation

---

### Addendums

These apply to the normal operation and usage of the equipment, as well as add-on items which may be integrated with the GPStar controllers.

1. [Operation Manual](OPERATION.md)

1. [Advanced Configuration](ADVCONFIG.md)

1. [Audio Output](SOUND.md)

1. [Smoke Effects](SMOKE.md) (Optional add-ons)

1. [Cyclotron Lights Upgrades](CYCLOTRON.md) (Optional add-ons)

1. [N-Filter Vent Light](NFILTER.md) (Optional add-on)

1. [Power Cell Lights Upgrade](POWERCELL.md) (Optional add-ons)

1. [Neutrona Wand 28 Segment Bar Graph Upgrade](BARGRAPH.md) (Optional add-on)

1. [Neutrona Wand Hat Lights](HATS.md) (Optional add-on)

1. [Neutrona Wand High Intensity Vent Light](VENTLIGHT.md) (Optional add-on)

1. [Frequently Asked Questions](FAQ.md)

---

### DIY Arduino Build Guides

Considered the "legacy" guide to building your own solution using the Arduino platform.
This is an advanced build-out which requires opening all devices and performing a significant amount of soldering. **Proceed with caution, regardless of your skillset or abilities.**

1. [Bill of Materials](BOM.md)

1. [Power Requirements](POWER.md)

1. [Compiling and Uploading the code](ARDUINO_FLASHING.md)

1. [WAV Trigger Configuration](WAVTRIGGER.md)

1. [Loading The Audio Files](AUDIO.md)

1. [Pack Setup](PACK.md)

1. [Wand Setup - DEPRECATED](WAND.md)

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

* Michael Rajotte (aka. "GPStar Technologies") - Director of Operations, Software Architect, Product Designer, 3D Modelling, EU & Worldwide Distribution
* Jonathan Gunn (aka. "Jonogunn") - Video Instruction Guides, PCB Design, Install Testing, North America Distribution
* Dustin Grau (aka. "JustinDustin") - Prototyping and Testing, Documentation, 3D Modelling, Software Support
* Angie Rutto (aka. "Frutto Technology") - Maker and purveyor of fine add-on products, UK Distribution

### Special Thanks

* Emre Sahare (GBFans: One Time) - For the original prop mode "Mode Original" operation deciphering.

Thank you to our supporters on [GBFans.com](https://www.gbfans.com/forum) and the [Facebook group](https://www.facebook.com/groups/gpstartechnologies).

For those who are curious you can view a condensed [history of this project](HISTORY.md).

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
