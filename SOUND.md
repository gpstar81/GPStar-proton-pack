# Sound Output Options

"Listen! You smell something?" - Ray Stantz

Sound is a huge component of this build and the options are relatively open for how best you wish to proceed. For the wand you will be limited by space, but for the pack you can go as big and as bold as you wish. The following options should give you an idea of what is possible with hardware given certain space constraints.

For instructions on loading audio files to a microSD card, please see the [AUDIO Guide](AUDIO.md).

## Audio Controllers

Sounds are output via one of the supported polyphonic WAV audio controllers: Robertsonics WavTrigger or GPStar Audio. The former is a 3rd-party component originally included with all GPStar kits up until early 2024, while the latter is a fully custom solution which is part of the GPStar family of controllers for your cosplay needs.

**Firmware Compatibility**

The compatibility matrix below notes whether a device is compatible with major versions of the GPStar software for the Proton Pack and Neutrona Wand controllers. The version number inside each cell is the minimum firmware version required or strongly recommended for the respective device.

| Controller Device | v1.x | v2.x | v3.x | v4.x | v5.0.x | v5.1.x |
|-------------------|------|------|------|------|------|------|
| Robertsonics WavTrigger  | v134 | v134 | v140 | v140 | v140 | v140 |
| <img src='images/gpstar_logo.png' width=20 align="left"/>GPStar Audio | &mdash; | &mdash; | &mdash; | &mdash; | v100<sup>1</sup> | v102<sup>2</sup> |

<sup>1</sup> All initial orders for GPStar Audio controllers shipped with v100 firmware which delivers all of the basic functionality needed for a Pack+Wand operation.

<sup>2</sup> The GPStar Audio v101 firmware added the Pause/Resume ability which is only accessible via the WiFi Web UI, while v102 focused on stability improvements.

**Firmware Updates**

All firmware files for the audio controllers can be found in the [/extras/sound/ folder](/extras/sound/). Files are grouped by their device name, and a firmware update tool for the GPStar Audio firmware is available for ease of flashing that device.

## Wand Audio

Let's begin with the most restrictive of spaces, the wand. Due to the additional components and wiring to be added, there is little room for options but replacements can be made. The stock speaker is rated at 0.25W @ 16Ω and uses a cone made from plastic--essentially more of a tweeter than a full-range speaker, so it is advised that if you would like better sound you will want a better speaker. Otherwise, you can still use the original Hasbro wand speaker. Below are the listed specifications of the on-board amplifiers depending on which audio board you are using.

---

### <img src='images/gpstar_logo.png' width=30 align="left"/>GPStar Audio on-board amplifier (stereo output) ###
Rated for 2.5W @ 4Ω or 1.5W @ 8Ω per channel. 

### WAV Trigger on-board amplifier (mono output only) ###
Rated for 2W @ 4Ω or 1W @ 8Ω.

---

There has been success in using [this 40mm speaker](https://a.co/d/93sbe2V) rated for 3W @ 4Ω. While this does not fit in the standard speaker location under the heatsink (its size interferes with the retraction of the wand) it can be mounted within the base of the gun box and offers just enough sound to be heard (indoors at least) and can double as a rumble motor due to being able to reproduce the low-end sounds which the stock speaker cannot. This larger speaker can also be fitted into a optional Heatsink located in the STL folder of this repository.

A potential example of how the upgraded 40mm speaker might be installed into the gun box is shown below in a DIY build. Note that in order to fit these components as shown the barrel connector for power was CAREFULLY removed (as it will not be used), and the screw posts for the wand mount were filed down to ensure they do not interfere with fitment.

Other users in the community have reported satisfactory results with the Visaton speakers available through DigiKey which are both of identical specs (300 Hz to 20 kHz) and closer in size to the stock speaker. The Visaton speaker fits in the standard heatsink with little modifications or it can fit in a alternative heatsink which is provided in the STL folder of this repository:

* [K-36-WP 8ohm](https://www.digikey.com/en/products/detail/visaton-gmbh-co-kg/K-36-WP-8-OHM/9842373)
* [K-36-MO 8ohm](https://www.digikey.com/en/products/detail/visaton-gmbh-co-kg/K-36-MO-8-OHM/9842375)

Optional Part Shown: [3D Model for Wand - 40mm Speaker Mount](stl/wand/speaker_mount.stl)

![Wand Audio Example](images/Audio-Wand.jpg)

### For more wand mounting solutions, refer to the [Wand Setup](WAND.md) page ###

## Pack Audio

This is where you can branch out to other options as desired and can really go all-out with your choice of audio. The WAV Trigger in the pack does not use its onboard amplifier by default, only the 3.5mm / ⅛" stereo phono jack which means you can use any amplifier which accepts that type of connection. When you choose your amplifier, that should dictate the choice of speakers to match.

The following components were demonstrated in the walkthrough video by JustinDustin and provides ample sound with a benefit of providing enough low-end rumble that a vibration motor may be considered optional:

- [Drok 15W Class D Stereo Amplifier (8-26V)](https://a.co/d/9VnB8e9)
  - This will operate successfully using the full 12V output offered by most TalentCell batteries
  - Note the output wattage using a 12V supply will draw around 1-1.25 Amps so plan the capacity of your battery accordingly (read: this is why the larger 6000 mAh TalentCell battery is recommended)
  - Increasing the output wattage using a different amplifier will increase the power draw as well as heat output, so choose carefully and plan accordingly for how/where to mount your amplifier
- [Pair of 3" Full Range Bookshelf Speaker (15W @ 8Ω)](https://a.co/d/cMg5Vrt)
  - Note the power rating of 15W which matches the amplifier to be used
  - Take note of your amplifier's recommended ohms rating for speakers
- [8" Auxiliary Stereo Audio Cable w/ 90&deg; Jack](https://a.co/d/3H4zxZw)
  - Optional: Useful in tight locations, depending on where the amp is mounted
- [Ground Loop Isolator](https://a.co/d/faBkok9)
  - Recommended: Prevents unwanted hum and noise from the amplifier due to the LEDs

The placement of your speakers is up to you. Though it is possible to mount a 3" speaker in the location used for the original pack speaker, a mount should be used to help offload some of the added weight from that device. And if the stock vibration motor is not important (as the solution above will provide some physical feedback in that regard), removal of that device opens up a cavity just behind and above the cross-bar of a standard ALICE frame for another speaker. For the latter, it will be necessary to carefully  drill some small holes into the motherboard of the pack for sound output.

Shown below is an example of how a pair of stereo speakers may be mounted inside of the pack. The upper speaker is located where the original speaker was mounted, while the lower speaker occupies the space where the vibration motor casing was secured into the pack. Note the WAV Trigger and amplifier as shown here are mounted in the space normally occupied by the D-cell battery compartment, primarily so the volume control for the amplifier can be accessed from the service door when the pack is reassembled. That portion of the motherboard would need to be partially or fully removed if you wish to use this space as shown.

Optional Part Shown: [3D Model for Pack - Upper Speaker Mount](stl/pack/Speaker_Mount.stl)

![Pack Audio Example](images/Audio-Pack.jpg)

### Note: Amplifier Volume Controls

Most amplifier boards will contain their own dedicated volume control (potentiometer) which will adjust output from that device only. The built-in controls for the pack/wand will control the source audio (sound effects vs. music) and the overall volume output for both of the WAV Trigger boards, which will affect the input level to your choice of amplifier and in turn affect the output relative to that device.

**tl;dr:** You should mount your amplifier in a space where you also have access to its volume control so you can adjust the pack to balance with the wand (or make it the domninant audio source) as necessary.

*Under normal operating conditions setting the recommended Drok 15W amplifier to 1/2 power will be sufficient and capable of outputting over 80dB from the suggested pack speakers. Higher volume may also result in faster battery drain with very little improvement in volume.*
