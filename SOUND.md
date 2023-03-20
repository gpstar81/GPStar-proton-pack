# Audio Output Options

"Listen! You smell something?" - Ray Stantz

Sound is a huge component of this build and the options are relatively open for how best you wish to proceed. For the wand you will be limited by space, but for the pack you can go as big and as bold as you wish. The following options should give you an idea of what is possible with hardware given certain space constraints.

## Wand Audio

Let's begin with the most restrictive of spaces, the Wand. Due to the additional components and wiring to be added, there is little room for options but replacements can be made. The stock speaker is rated at 0.25W @ 16Ω and uses a cone made from plastic--essentially more of a tweeter than a full-range speaker. By default, the WavTrigger to be used in the wand will enable its on-board mono amplifier which is rated for 2W @ 4Ω or 1W @ 8Ω, so it is advised that if you would like better sound you will want a better speaker. Otherwise, you can still use the original Hasbro wand speaker successfully with the on-board amplifier.

There has been success in using [this 40mm speaker](https://a.co/d/93sbe2V) rated for 3W @ 4Ω. While this does not fit in the standard speaker location under the heatsink (its size interferes with the retraction of the wand) it can be mounted within the base of the gun box and offers just enough sound to be heard (indoors at least) and can double as a rumble motor due to being able to reproduce the low-end sounds which the stock speaker cannot. YMMV with any other choice of speaker, though there are many options for 40mm speakers on Amazon.

A potential example of how the WavTrigger and an upgraded 40mm speaker might be installed into the gun box is shown below. Note that in order to fit these components as shown the barrel connector for power was CAREFULLY removed (as it will not be used), and the screw posts for the wand mount were filed down to ensure they do not interfere with fitment behind the WavTrigger once mounted on the gun box.

Optional Part Shown: [3D Model for Wand - 40mm Speaker Mount](stl/wand/speaker_mount.stl)

![Wand Audio Example](images/Audio-Wand.jpg)

## Pack Audio

This is where you can branch out to other options as desired and can really go all-out with your choice of audio. The WavTrigger in the pack does not use its onboard amplifier by default, only the 3.5mm / ⅛" stereo phono jack which means you can use any amplifier which accepts that type of connection. When you choose your amplifier, that should dictate the choice of speakers to match.

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

The placement of your speakers is up to you, though it is possible to mount a 3" speaker in the location used for the original pack speaker, though a mount should be used to help offload some of the added weight from that device. And if the stock vibration motor is not important (as the solution above will provide some physical feedback in that regard), removal of that device opens up a cavity just behind and above the cross-bar of a standard ALICE frame for another speaker. For the latter, it will be necessary to carefully  drill some small holes into the motherboard of the pack for sound output.

Shown below is an example of how a pair of stereo speakers may be mounted inside of the pack. The upper speaker is located where the original speaker was mounted, while the lower speaker occupies the space where the vibration motor casing was secured into the pack. Note the WavTrigger and amplifier as shown here are mounted in the space normally occupied by the D-cell battery compartment, primarily so the volume control for the amplifier can be accessed from the service door when the pack is reassembled. That portion of the motherboard would need to be partially or fully removed if you wish to use this space as shown.

Optional Part Shown: [3D Model for Pack - Upper Speaker Mount](stl/pack/Speaker_Mount.stl)

![Pack Audio Example](images/Audio-Pack.jpg)

### Note: Amplifier Volume Controls

Most amplifier boards will contain their own dedicated volume control (potentiometer) which will adjust output from that device only. The built-in controls for the pack/wand will control the source audio (sound effects vs. music) and the overall volume output for both of the WavTrigger boards, which will affect the input level to your choice of amplifier and in turn affect the output relative to that device.

**tl;dr:** You should mount your amplifier in a space where you also have access to its volume control so you can adjust the pack to balance with the wand (or make it the domninant audio source) as necessary.
