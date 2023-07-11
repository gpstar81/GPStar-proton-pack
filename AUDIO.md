## Loading The Audio Files

This repository (for somewhat obvious reasons) does not provide music tracks for your project other than a single track which was graciously approved by the independent artist for inclusion. You will need to prepare any music tracks for use by exporting MP3 or AAC audio files as 44.1kHz 16-bit WAV files. This can be done easily using a program such as [Audacity](https://www.audacityteam.org/) to open the music file and export to WAV format. See the instructions below for how to name your files for loading onto the SD cards used by the WavTrigger boards.

## Loading Files to the SD Cards

`NOTE: Please consider high quality SD Cards. We recommend Sandisk Extreme A2 U3 V30 Micro SD Cards.`

Your SD cards **must** be formatted as FAT32 using 32KB block size per the WavTrigger manual. Use a full formatting option for the SD cards not a quick formatting for best results.

Load [all sound files from the "sounds" folder](sounds) (and music, optionally) onto both SD cards. The file names must be identical and match between the SD cards so that any synchronized effects are both played by both WavTrigger devices at the same time. Although only some of the same tracks are played on the wand and pack while firing, some tracks play the same like the firing tail end, video game firing sounds, etc. For the proton stream, the wand plays a different sound vs the pack, so they overlap and mix in person for a nice effect.

Sound effects are prefixed with numbers 001-499 while music files will be prefixed as 500 and above. `DO NOT LEAVE GAPS IN PREFIXES WHEN NUMBERING FILES!` The only exception is between the last sound effect and first music track; when the pack and wand boot up, they ask the WavTrigger to count all the sound files on the SD cards, then I do some simple math by subtracting a known number of sound effects from the total number of loaded sound files, then it knows how many music tracks have been added and plays them accordingly.

To add more music, copy stereo wav tracks (44khz stereo wav files) onto the SD card of the pack and wand. File naming of the music tracks start at 500. So for example, `500_somesong.wav`, `501_somesong.wav`, `502_somesong.wav`, etc. **Do not leave any gaps in the number sequencing!** (For example of what NOT to do, `500_somesong.wav` followed by `502_somesong.wav` leaving off `501_somesong.wav` on the SD card.) The most and only important part in the naming is the XXX_ prefix for all tracks.

## Troubleshooting

From the Robertsonics [manual for the WavTrigger](https://static1.squarespace.com/static/62ab6e0d1f3ea036834d4a0b/t/63c331cab98b7f0d1d5fe04b/1673736656985/WT_UserGuide_20230114.pdf):

If you have problems, here are a few things to check:

1. Make sure your WAV files are named properly and are located in the root directory of the microSD card. When you first power on the WAV Trigger, it will locate and index the files and then blink 3 times to indicate that it found at least one WAV file. One long blink means that it either couldn’t read the card or it didn’t find any WAV files.

1. If there’s no activity on the status LED whatsoever when you power up the WAV Trigger, check the “Load/Run” switch next to the power connector and make sure it’s in the “Run” position.

1. Once you get the 3 blinks, pressing the on-board button will play the lowest numbered track on the microSD card. Confirm this by observing the status LED turning on to indicate a track is playing. You should hear the track through the stereo jack. If you’re using the on-board audio amp and speaker connection, you won’t hear anything unless you have enabled the amp using the init file.

1. Low quality SD Cards sometimes provide issues such as sounds not playing. Please consider using high quality SD Cards. We recommend the Sandisk Extreme A2 U3 V30 Micro SD Cards.

1. If you’ve gotten this far and tracks won’t play, then it’s most likely that your WAV files are not the correct format. The WAV Trigger will only play WAV files formatted as 16-bit, stereo, 44.1kHz, and there can be no meta-data (non-audio data) in the file before the audio data. It seems that many Mac audio applications insert a meta-data chunk prior to the audio, and this will prevent the track from playing. In such cases, simply importing the file and re-exporting with Audacity will likely fix the problem.

1. Please consider using a supply voltage of no more than 9V, preferably less. Playing loud audio with the on-board amp for extended periods can draw a lot of current and the lower the input voltage, the less warm the 5V regulator will get. This is especially important if you’re encasing the WAV Trigger in a small project box. I personally use 6V wall mount adaptors. Alternatively, you can power the WAV Trigger with regulated 5V (5Vin) and bypass the on-board regulator completely.
