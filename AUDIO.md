## Loading The Audio Files

This repository (for somewhat obvious reasons) does not provide music tracks for your project other than a single track which was graciously approved by the independent artist for inclusion. You will need to prepare any music tracks for use by exporting MP3 or AAC audio files as 44.1kHz 16-bit WAV files. This can be done easily using a program such as [Audacity](https://www.audacityteam.org/) to open the music file and export to WAV format. See the instructions below for how to name your files for loading onto the SD cards used by the WAV Trigger boards.

## Format your SD Card to FAT32
`Please consider high quality SD Cards. We recommend Sandisk Extreme A1/A2 U3 V30 32GB Micro SD Cards.`

Your SD cards **MUST** be formatted as FAT32 using 32KB block size per the WAV Trigger manual. Use a full formatting option for the SD cards not a quick formatting for best results.

## SD Cards with capacities larger than 32GB in FAT32.

### MAC and Linux
Mac and Linux users can format SD Cards larger than 32GB as FAT32 normally with their standard disk utility software.
![Mac and Linux Formatting](images/macfat32.jpg)


### WINDOWS
SD Cards with capacities of **32GB OR LESS** can be formatted normally in the Windows Explorer.
![Mac and Linux Formatting](images/fat32windows.jpg)

**SD Cards with capacities larger than 32GB** for Windows users will need to be formatted a different way. We recommend the free tool Fat32Format by Tom Thornhill. It can be found in the `extras` folder of this repository or in the direct download link below

(Web Site)
[Fat 32 Format GUI version](http://ridgecrop.co.uk/index.htm?guiformat.htm)

(Direct Download link)
[fat32format (exe)](https://github.com/gpstar81/haslab-proton-pack/raw/main/extras/guiformat.exe)

![fat32formatter](images/fat32.jpg)

`Thank you to Tom Thornhill for allowing us to distribute his FAT32 formatter for Windows users. If you find his software useful, you can donate directly to him by clicking on the button below`

[![Tom Thornhill donate](images/donate.gif)](https://www.paypal.com/webapps/shoppingcart?flowlogging_id=f8611276523bf&mfid=1690302003463_f8611276523bf#/checkout/openButton)

## Loading Files to the SD Cards

Load [all sound files from the "sounds" folder](sounds) (and music, optionally) onto both SD cards. The file names must be identical and match between the SD cards so that any synchronized effects are both played by both Proton Pack and Neutrona Wand at the same time. Although only some of the same tracks are played on the wand and pack while firing, some tracks play the same like the firing tail end, video game firing sounds, etc. For the proton stream, the wand plays a different sound vs the pack, so they overlap and mix in person for a nice effect.

Sound effects are prefixed with numbers 001-499 while music files will be prefixed as 500 and above. `DO NOT LEAVE GAPS IN PREFIXES WHEN NUMBERING FILES!` The only exception is between the last sound effect and first music track.

To add more music, copy stereo wav tracks (16-bit 44khz stereo wav files) onto the SD card of the pack and wand. File naming of the music tracks start at 500. So for example, `500_somesong.wav`, `501_somesong.wav`, `502_somesong.wav`, etc. **Do not leave any gaps in the number sequencing!** (For example of what NOT to do, `500_somesong.wav` followed by `502_somesong.wav` leaving off `501_somesong.wav` on the SD card.) The most and only important part in the naming is the XXX_ prefix for all tracks.

`Please remove any metadata that may be in the music wav files that you add. This can cause problems with loading and playing back your music tracks`

`The below photo is a example of the prefixing and music sequencing`

![audio example](images/audioexample.jpg)

## Troubleshooting

[Manual for the WAV Trigger [pdf]](https://github.com/gpstar81/haslab-proton-pack/raw/main/extras/WT_UserGuide_20230602.pdf)

If you have problems, here are a few things to check:

1. Make sure your WAV files are named properly and are located in the root directory of the microSD card. When you first power on the WAV Trigger, it will locate and index the files and then blink 3 times to indicate that it found at least one WAV file. One long blink means that it either couldn’t read the card or it didn’t find any WAV files.

1. If there’s no activity on the status LED whatsoever when you power up the WAV Trigger, check the “Load/Run” switch next to the power connector and make sure it’s in the “Run” position.

1. Once you get the 3 blinks, pressing the on-board button will play the lowest numbered track on the microSD card. Confirm this by observing the status LED turning on to indicate a track is playing. You should hear the track through the stereo jack. If the audio is not working, try plugging in a set of headphones into the 3.5mm audio port and test again. If you’re using the on-board audio amp and speaker connection, you won’t hear anything unless the onboard amp is enabled using the init file or via software.

1. Low quality SD Cards sometimes provide issues such as sounds not playing. Please consider using high quality SD Cards. We recommend the Sandisk Extreme A1/A2 U3 V30 32GB Micro SD Cards.

1. If you’ve gotten this far and tracks won’t play, then it’s most likely that your WAV files are not the correct format. The WAV Trigger will only play WAV files formatted as 16-bit, stereo, 44.1kHz, and there can be no meta-data (non-audio data) in the file before the audio data. It seems that many Mac audio applications insert a meta-data chunk prior to the audio, and this will prevent the track from playing. In such cases, simply importing the file and re-exporting with Audacity will likely fix the problem.

1. Please remove any metadata contained in the music wav files you add, as it can cause issues loading the file for playback.

