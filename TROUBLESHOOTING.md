# Troubleshooting

If you are experiencing issues during the installation process there are a few ways to get back on track. Use the guides below to help narrow down where you are having an issue and try the suggested remedies. Should you need additional assistance, reach out to the [gpstar Technologies Facebook group](https://www.facebook.com/groups/gpstartechnologies) and either post a question or message one of the group administrators.

Regardless of the device, whether pack or wand, there are no built-in LED's on the PCB's for diagnostics which can be consulted for status. Though there will be certain devices which should be available once connected which can help narrow down a root cause of the issue. For audio-related issues please refer to the image below which highlights the location of the test button (blue circle) and red status LED (green circle) on the WavTrigger device.

![](images/WavTrigger_Debug.jpg)

## Proton Pack

Since all connections to the Proton Pack utilize standard JST-XH connections to the available devices, please re-check any connections for incorrect seating and polarity before continuing. If these are in place then at a minimum you should be able to turn on the pack using the red switch under the ion arm which should cause the Powercell lights to activate. This will be the primary means of validating that the pack controller is enabled.

### Pack will not power on (no lights or sounds)

Confirm that the battery pack is sufficiently charged, switched to the on state, and USB-to-JST connection is in place.

### Pack has lights but does not play audio

Press the test button on the WavTrigger to play the first audio file on the SD card and observe the status LED state.

If the red LED illuminates, this indicates the WavTrigger is receiving power and can play the audio files.

* Re-check the 3.5mm audio cable is connected to the audio amplifier, and that the audio amplifier has power, is connected to at least 1 speaker, and volume has been turned up to at least 30%.
* If possible, connect the 3.5mm audio cable to a known-good audio source such as a smartphone, Alexa device, or similar to confirm that the amplifier is capable of receiving a signal.

If the red LED does not illuminate, this could mean a connection or data problem.

* Reformat the microSD card as FAT32 using the provided utility and re-copy the audio files to the microSD card.
* Re-check the connections to the WavTrigger, taking note of the power and Rx/Tx connections. Note that is it normal to see some empty pins/sockets on the connecting wire.


## Neutrona Wand

The wand assumes successful operation of the pack, so please only proceed once that has been established.

### Wand will not power on (no lights or sounds)

The wand is expects to communicate with the pack and will only power on when that connection is successfully established. If no lights or audio are observed when using the Activate switch, please consider the following options.

### Wand has lights but does not play audio

Troubleshooting will be similar to the process used with the pack, though there will not be an external amplifier or audio cable in use. With the wand attached to the pack, press the test button on the WavTrigger to play the first audio file on the SD card and observe the status LED state.

If the red LED illuminates, this indicates the WavTrigger is receiving power and can play the audio files.

* Re-check the connection to the speaker. Polarity should be observed for the best audio quality, though a reversed connection should still support some form of audio playback.
* Re-flash the latest software to both the pack and wand controllers. These devices must be in agreement for the communication layer to operate correctly.
* Re-check the Tx/Rx connections to the pack. If these are reversed it will prevent proper communication. Attempt to switch the wires at the pack-side connector which use screw terminals, and retry the Activate switch.
 
If the red LED does not illuminate, this could mean a connection or data problem.

* Reformat the microSD card as FAT32 using the provided utility and re-copy the audio files to the microSD card.
* Re-check the connections to the WavTrigger, taking note of the power and Rx/Tx connections. Note that is it normal to see some empty pins/sockets on the connecting wire.
* Re-check the power connection to the wand. If you have a digital multimeter available, set the device to the DC power measurement and check that the 5V-OUT connection is supplying the expected voltage. Be careful to not short out the pins and mind the polarity.
receiving