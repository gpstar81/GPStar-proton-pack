<h1><span class="logo-icon"></span> GPStar System Guides</h1>

# Introduction

These are the most important guides which will get you familiar with the major components and how to update them with the latest capabilities offered by software and sound effects. This includes guides to aid you in completing the connections and installation for the GPStar Proton Pack and Neutrona Wand PCB boards.

## Installation

1. [Pack PCB](PACK_PCB.md)

1. [Wand PCB](WAND_PCB.md)

1. [Pack-Wand Hose](HOSE.md)

1. [GPStar Audio PCB](GPSTAR_AUDIO_PCB.md)

1. [Installation Videos](INSTALL_VIDEOS_JONOGUNN.md)

1. [Troubleshooting](TROUBLESHOOTING.md)

A full installation guide is available as a PDF here:
[GPStar Proton Pack Kit Installation Manual](https://github.com/gpstar81/GPStar-proton-pack/blob/main/extras/gpstar-Haslab-Proton-Pack-Kit-Installation-Manual-V4.pdf?raw=1)

## Maintenance

These guides will help you update the software and audio files as new releases become available.

1. [Flashing Firmware Updates](FLASHING.md)

1. [Loading The Audio Files](AUDIO_FILES.md)

## By-Component Guides

This section is presented in a way to offer a more complete view of what components are available (by major equipment areas), whether those components are Required<sup>R</sup> or Optional<sup>O</sup>, if choices are offerred at a specific component level. Where possible, each option will link to a related guide for more details and/or installation instructions.

**Proton Pack**

| **Component**          | **Option w/ Guide** |
|------------------------|---------------------------------------------|
| Power Source<sup>R</sup>    | [TalentCell 3000mAh or 6000mAh](POWER.md) |
| Controller<sup>R</sup>      | [GPStar Pack Controller](PACK_PCB.md) or [Arduino 2560](DIY_PACK.md) |
| Software<sup>R</sup>        | [Flashing the Latest Firmware](FLASHING.md) |
| Audio Board<sup>R</sup>     | [GPStar Audio](GPSTAR_AUDIO_PCB.md) or [Robertsonics WAV Trigger](WAVTRIGGER.md) |
| Speakers/Amp<sup>R</sup>    | [Audio Output Guide](SOUND.md) |
| Sound Effects<sup>R</sup>   | [Loading The Audio Files](AUDIO_FILES.md) |
| Cyclotron Lid<sup>O</sup>   | [12/20/36-LED options](CYCLOTRON_LID.md) |
| Inner Cyclotron<sup>O</sup> | [Cake and Panel Lights](CYCLOTRON_INNER.md)
| N-Filter<sup>O</sup>        | [7-LED Vent Light](NFILTER.md) |
| Power Cell<sup>O</sup>       | [13 and 15-LED options](POWERCELL.md) |
| Smoke Effects<sup>O</sup>   | [Various Solutions Supported](SMOKE.md) |
| Cyclotron Bypass<sup>R* & O</sup> | [Bypass board for non Haslab Proton Packs](CYCLOTRON_BYPASS.md) |

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
| Sound Effects<sup>O</sup> | [Loading The Audio Files](AUDIO_FILES.md) |
| Hat Lights<sup>O</sup>    | [Kit option by Jonogunn](HATS.md) |
| Bargraph<sup>O</sup>      | [28-Segment or 30-Segment](BARGRAPH.md) |
| Vent Light<sup>O</sup>    | [GPStar RGB Vent Lights or High Intensity option by Frutto Technology](VENTLIGHT.md) |
| Wand Barrel<sup>O</sup>   | 2, 5, 49 and 50 LED options (Guide TBD) |

**Installation, Operation, and Troubleshooting**

| **Task**         | **Guide** |
|------------------|---------------------------------------------|
| Initial Assembly | [Installation Videos](INSTALL_VIDEOS_JONOGUNN.md) & [Installation Guide (PDF)](https://github.com/gpstar81/GPStar-proton-pack/blob/main/extras/gpstar-Haslab-Proton-Pack-Kit-Installation-Manual-V4.pdf?raw=1) |
| System Operation | [Equipment Operation Manual](OPERATION_USAGE.md) |
| Configuration    | [EEPROM LED/Config Menus](OPERATION_EEPROM.md) |
| I Need Help!     | [Troubleshooting](TROUBLESHOOTING.md) and [Frequently Asked Questions](FAQ.md) |

**Software Development (Advanced Topics)**

| **Task**         | **Guide** |
|------------------|---------------------------------------------|
| Compile Software | [VSCode + PlatformIO](VSCODE.md)
| Modify Software  | [Advanced Software Configuration](ADVCONFIG.md) |
| Upload Software  | [Pack/Wand Flashing](COMPILING_FLASHING.md) or [Attenuator/Wireless Flashing](ATTENUATOR_FLASHING.md) |

**Special/Standalone Devices**

| **Devices**         | **Guide** |
|------------------|---------------------------------------------|
| Single-Shot Blaster | [Build and Operation Guide](SINGLESHOT.md) |
| Belt Gizmo | *Build Guide TBD* |
| Stream Effects | *Build Guide TBD* |
