# Operating Manual
					
Video Demo: [Haslab Proton Pack Arduino Powered Demo & Features](https://www.youtube.com/watch?v=nhGX1oJBNHk) (YouTube)
[![Haslab Proton Pack Arduino Powered Demo & Features](https://img.youtube.com/vi/nhGX1oJBNHk/maxresdefault.jpg)](https://youtu.be/nhGX1oJBNHk)

## Proton Pack		

### Features

- The switch underneath the ion arm can turn the pack on or off.
- You can run the pack without the cyclotron lid on.
	- Automatically switches to internal "cake" cyclotron, if implemented (optional).
- Alarm ribbon cable will activate the alarm mode and degraded cyclotron spin.
- The pack can run without a wand.
- Vibration switch in the cyclotron switch plate enables or disables the vibration motors for both the pack and wand.
- 1984/2021 year switch in the cyclotron switch plate changes the pack and wand from 1984 (OG) to 2021 (Afterlife) modes.
- Single LED per cyclotron lens for 1984/1989 modes. This can be toggled back to 3 LED's per cyclotron lens via the wand menu system.
- Optional 1989 sound effects mode on the pack with advanced user configuration setting **\*see below\***
- Optional switches and lights available for smoke effects, cyclotron panel, etc.

## Neutrona Wand				

### Key Features

- Activate toggle on the gun box turns on both the pack and wand (the switch under the pack's ion arm does not need to be engaged for this to work).
- Right-hand toggles affect the vent light and additional sound effects.
	- Note that for both 2021 and 1984 modes, the wand will not fire unless these switches are turned on.
	- Additionally, the lever switch near the gunbox must be pulled to extend the barrel before firing.

**Video Game Mode (Default)**

- The orange button at the end of the wand switches between the available firing modes:
	1. Proton Stream (Default)
	1. Slime Gun
	1. Stasis Beam
	1. Meson Collider
	1. Special: Setting 5 is a settings menu system (see section below).

**Crossing The Streams / Crossing The Streams Mix / Alternate Firing (Optional)**

- The orange button at the end of the wand acts as a alternate fire mode.
- The overheat features can only be triggered when enabled by the alternate firing mode.
- Pressing both the Intensify and orange button at the end of the wand at the same time enables the Crossing the Streams. Releasing one of the 2 firing buttons will continue crossing the streams. During Crossing The Streams Mix, you need to hold both the intensify and alternate firing button at the same time. Release one or the other will end crossing the streams but continue firing a regular proton stream. The Proton Pack and Wand can still overheat if enabled while crossing the streams in the power level you are at.

To enable the Crossing The Streams and alternate firing by default, you need to set **bool b&#95;cross&#95;the&#95;streams** to true in the Advanced User Configuration settings at the top of the Neutrona wand code. To further enable Crossing The Streams Mix, you need to set **bool b&#95;cross&#95;the&#95;streams&#95;** to true (see Advanced User Configuration settings below). Otherwise you can toggle between Crossing The Streams, Crossing The Streams Mix and Video Game modes via the Neutrona Wand Sub Menu Settings (see below). **Important: The Neutrona Wand sub menu settings can only be reached while the Neutrona Wand is powered down**

**Toggle Switch Behavior**

- While in 1984 mode:
	- Bottom right switch turns on the vent light and plays the wand power up sound.
	- Top right switch plays a single beep sound.
- While in 2021 mode:
	- Bottom right switch turns on the vent light and the wand plays a ramp up sound.
 	- Top right switch turns on the looping beeping sounds.

**Top Dial Behavior**

- The rotary dial on the top of the gun box changes the power mode of the wand, affecting the intensity of the stream effects, indicated by increasing or decreasing the bargraph lights (just as the stock wand did). There are 5 wand power modes as based on the original bargraph which has only 5 LED's available:
	- The wand can be user configured in the Neutrona Wand code to overheat the pack in any of the power modes. When not set to overheat, you can fire continiously. 
	- The pack can also be user configured in the Proton Pack Code to adjust the duration of smoke effects (for each power mode), which smoke pins are activated (for each power mode), overheating smoke effects (for each power mode) or smoke disabled entirely.

	Default settings: The wand will fire forever on modes 1 through 4 and overheat on mode 5.
- The pack will emit smoke effects during longer firing (if enabled).
- Lower power modes, the proton stream is more red. On higher modes, the stream will appear more yellow.
- As the dial is turned, any beeping rate will increase or decrease depending on the theme in use.
- This dial will be used to navigate the menu system and adjust volume in certain cases (see next section).

## Wand Settings Menu System
Wand Menu Settings Video Demo: [Proton Pack Wand Menu Settings Video Demo](https://www.youtube.com/watch?v=QrevSoQo_3M) (YouTube)
[![Proton Pack Wand Menu Settings Video Demo](https://img.youtube.com/vi/QrevSoQo_3M/maxresdefault.jpg)](https://youtu.be/QrevSoQo_3M)

When you are in the settings menu system [Mode 5] all the bargraph leds begin flashing. All other functions of the wand (such as firing) are disabled while in the menu system. There are 5 different menu settings, which are indicated by the bargraph LED's 1 through 5 (from bottom to top) with menu level 5 being the default upon entering this mode. For example in navigation, when 1 LED is flashing that means you are on menu 1. **Use the rotary knob on the top of the wand to navigate up and down through the menu system!**

**Top Level Menu**

| Top Menu Level | Purpose | Description |
|------------|---------|-------------|
| 5 | Music&nbsp;Track&nbsp;Looping | Press **Intensify** to set the current track to loop forever; press again to turn off the loop feature. (Note that the LED #5 will stay solid when this single-track loop is enabled while in the top menu system, it will blink on/off when looping is disabled). |
| 4 | Effects-only&nbsp;Volume | Press **Intensify** to increase volume, press the orange **Mode** switch to decrease volume. |
| 3 | Music-only&nbsp;Volume | Press **Intensify** to increase volume, press the orange **Mode** switch to decrease volume. |
| 2 | Switch&nbsp;Music&nbsp;Track | Press **Intensify** to cycle forward. Press the orange **Mode** switch to cycle backward. |
| 1 | Play&nbsp;or&nbsp;Stop&nbsp;Music | Press **Intensify** to play music. Press it again to stop the music. |

**Sub Level Menu (Only accessible while the Neutrona Wand is powered down)**

**To access the sub level menu, use the rotary dial on the top of the wand. When you reach the sub level menu, the slo-blo LED will light up to indicate this menu**

| Sub Menu Level | Purpose | Description |
|------------|---------|-------------|
| 5 | Cross The Streams / Cross The Streams Mix / Video Game Mode | Press **Intensify** to enable the cross the streams feature or revert back to video game modes. (Note that the LED #5 will stay solid when cross the streams is enabled while in the sub menu system). Cross The Streams Mix requires holding down both intensify and the alternate firing button at the same time.
| 4 | Smoke Settings / Overheating| Press **Intensify** to enable or disable smoke. Press the **Orange Mode Switch** to enable or disable overheating |
| 3 | Cyclotron Rotation Direction / Cyclotron LED Setting Toggle | Press **Intensify** button to change the direction of the cyclotron rotation. Press the **Orange Mode Switch** to toggle between 1 single LED for the cyclotron lens or 3 LEDs during 1984/1989 modes for the cyclotron lid. |
| 2 | Vibration / Vibration Firing Mode | Press **Intensify** to enable vibration or disable. Press the **Orange Mode Switch** to enable vibration during firing only. Press again to disable. |
| 1 | Year Mode | Press **Intensify** cycle through 1984, 1989 or 2021 modes. |

To exit the menu system, navigate to the top menu in **Setting 5** and press the orange **Mode** switch button, the wand will now go to the Proton Stream mode (this will be accompanied by an additional audio cue that you've returned to the firing modes). Music will continue to play (and advance/loop) even when the pack and wand are turned off. 

**When music is playing while the pack and wand are turned off, you can use the Rotary knob on the top of the wand to easily access independent music volume control.**

**When the wand and pack are turned off, you can still access the Wand Settings Menu System by pressing the orange mode switch button at the end of the wand.**

Refer the [WavTrigger section](WAVTRIGGER.md) for more information on loading additional music tracks.

## Advanced User Configurations
It is possible to change some of the default behaviour for both the Proton Pack and Neutrona Wand by changing some values in the Configuration.h file. The Proton Pack and Neutrona Wand each have there own Configuration.h file that can be found in their respective folders.

**Proton Pack**

|Variable| Description| Default Setting |
|--------|------------|-----------------|
| const int i&#95;pack&#95;num&#95;leds | PowerCell and Cyclotron Lid LED count. 25 LEDs in the stock Haslab kit. 13 in the Powercell and 12 in the Cyclotron lid. If you install a 40 LED NeoPixel ring in your cyclotron lid, then this should be set to 53. The default setting is 25. **Note: The cyclotron lid can support up to 40 (rgbw) addressable LEDs such as NeoPixels (ws2812b). They will change colour accordingly if you switch to video game modes firing modes.**| 25 |
| const int i&#95;1984&#95;cyclotron&#95;leds[4] | This is the middle led aligned in each cyclotron lens. Put the sequence in order from lowest to highest in a clockwise direction. The top right lens is the cyclotron lens #1 and work your way clockwise. The default setting is { 2, 5, 8, 11 } which is for the stock Haslab LEDs. If you installed a 40 LED NeoPixel ring in your cyclotron lid, then the settings of { 1, 11, 19, 29 } is recommended depending on how you aligned the ring. | { 2, 5, 8, 11 } |
| const int i&#95;1984&#95;delay | This controls the delay in milliseconds when a LED changes on the cyclotron lid in 1984 mode. The default setting is 1050 which is 1.05 seconds. | 1050 |
| const int i&#95;2021&#95;delay | This controls the delay in milliseconds when a LED changes on the cyclotron lid in 2021 mode. The default setting is 15. If you use a 40 LED NeoPixel ring in the cyclotron lid, changing this to 10 is a good setting.| 15 |
| #define CYCLOTRON&#95;NUM&#95;LEDS | This setting controls how many optional LEDs are in the Inner (cake) Cyclotron. Default setting of 35 is for a 35 pixel NeoPixel ring. If you change this for a smaller NeoPixel ring, you may need to alter the i&#95;2021&#95;inner&#95;delay and i&#95;1984&#95;inner&#95;delay to lower values to slow down the rotation speed for a smaller ring. | 35 |
| const int i&#95;2021&#95;inner&#95;delay | This setting controls the optional NeoPixel ring speed for the inner cyclotron in 2021 mode. The default setting is 5 which is set for a 35 pixel NeoPixel ring. Decrease this number to make the ring spin faster, or increase to spin slower. | 5 |
| const int i&#95;1984&#95;inner&#95;delay | The same as above, but for 1984 mode. Default setting is 9 for a 35 pixel NeoPixel ring. | 9 |
| const int STARTUP&#95;VOLUME | The default startup master volume of the proton pack. This is a percentage value of between 100% and 0%. 100 = highest, 0 = lowest. When a Neutrona Wand is connected, it will sync to this value. | 100 |
| const int STARTUP&#95;VOLUME&#95;MUSIC | The default music volume of the Proton Pack. This is a percentage value of between 100% and 0%. 100 = highest, 0 = lowest. When a Neutrona Wand is connected, it will sync to this value. | 100 |
| const int STARTUP&#95;VOLUME&#95;EFFECTS | The default sound effects volume of the Proton Pack. This is a percentage value of between 100% and 0%. 100 = highest, 0 = lowest. When a Neutrona Wand is connected, it will sync to this value. | 100 |
| const int MINIMUM&#95;VOLUME | You can set the minimum lowest volume the proton pack can go. 0 = loudest, and -70 = quietest. | -50 |
| const int VOLUME&#95;MULTIPLIER | Percentage increments of master volume change. | 2 |
| const int VOLUME&#95;MUSIC&#95;MULTIPLIER | Percentage increments of the music volume change. | 5 |
| const int VOLUME&#95;EFFECTS&#95;MULTIPLIER | Percentage increments of the sound effects volume change. | 5 |
| const bool b&#95;onboard&#95;amp&#95;enabled | Set to true to enable the onboard amplifier of the Wav Trigger. Turning off the onboard amp draws less power. If using the AUX cable jack of the Wav Trigger, the amp can be disabled to save power. If you are using the output pins directly on the Wav Trigger board to your speakers, you will need to enable the amp. **The onboard mono audio amplifier and speaker connector specifications: 2w into 4 Ohms, 1.25W into 8 Ohms**| false |
| bool b&#95;clockwise | Set to true for the cyclotron lights to spin clockwise. Set to false to be counter clockwise. This can be controlled by an optional switch on pin 29. **Note: This can be toggled from the Neutrona Wand sub menu system.** | true |
| const bool b&#95;vibration&#95;enabled | Enable or disable vibration control for the Proton Pack. When set to false, there will be no vibration enabled for the Proton Pack, and it will ignore the toggle switch on pin 27. **Note: This can be toggled from the Neutrona Wand sub menu system.**| true |
| bool b&#95;vibration&#95;firing | When set to true, when vibration is enabled, the Proton Pack will only vibrate while the Neutrona wand is firing. **Note: This can be toggled from the Neutrona Wand sub menu system.**| false |
| const bool b&#95;overheat&#95;strobe | If you want the optional n-filter NeoPixel jewel to strobe during overheat, set to true. When false, the light stays solid white during overheat. | false |
| const bool b&#95;overheat&#95;lights&#95;off | When the pack is overheating, the powercell will ramp down and turn off. The cyclotron also turns off. When set to false, the powercell and cyclotron stays on during overheat but ramps and spins very slowly. | true |
| const bool b&#95;fade&#95;cyclotron&#95;led | When set to false, the LEDs on the cyclotron lid in 1984 mode will not fade. | true |
| bool b&#95;cyclotron&#95;single&#95;led | When set to true, the LEDs on the cyclotron lid will light up the middle LED only per cyclotron lens during 1984 and 1989 mode. When set to false, 3 LED's per lens will be displayed instead. **Note: This can be toggled from the Neutrona Wand sub menu system.** | true |
| const int i&#95;1984&#95;fade&#95;out&#95;delay | Time in milliseconds for the cyclotron lid LED to fade out when enabled. | 210 |
| const int i&#95;1984&#95;fade&#95;in&#95;delay | Time in milliseconds for the cyclotron lid LED to fade in when enabled. | 210 |
| bool b&#95;smoke&#95;enabled | Enable or disable smoke settings. This can be toggled with a switch on PIN 37. **Note: This can be toggled from the Neutrona Wand sub menu system.** | true |
| const bool b&#95;smoke&#95;1&#95;continuous&#95;firing | Set to true to enable smoke 1 during continuous firing events. **This can be overridden if b&#95;smoke&#95;enabled is set to false** | true | 
| const bool b&#95;smoke&#95;2&#95;continuous&#95;firing | Set to true to enable smoke 2 during continuous firing events. **This can be overridden if b&#95;smoke&#95;enabled is set to false** | true |
| const bool b&#95;fan&#95;continuous&#95;firing | Set to true to enable the fan during continuous firing events. **This can be overridden if b&#95;smoke&#95;enabled is set to false** | true |
| const bool b&#95;smoke&#95;continuous&#95;mode&#95;1 | Set to true to enable smoke events in **wand power mode 1** during continuous firing events. *This can be overridden if b&#95;smoke&#95;enabled is set to false or if all 3 of the smoke&#95;1, smoke&#95;2 and fan&#95;continuous are set to false* | true |
| const bool b&#95;smoke&#95;continuous&#95;mode&#95;2 | Set to true to enable smoke events in **wand power mode 2** during continuous firing events. *This can be overridden if b&#95;smoke&#95;enabled is set to false or if all 3 of the smoke&#95;1, smoke&#95;2 and fan&#95;continuous are set to false* | true |
| const bool b&#95;smoke&#95;continuous&#95;mode&#95;3 | Set to true to enable smoke events in **wand power mode 3** during continuous firing events. *This can be overridden if b&#95;smoke&#95;enabled is set to false or if all 3 of the smoke&#95;1, smoke&#95;2 and fan&#95;continuous are set to false* | true |
| const bool b&#95;smoke&#95;continuous&#95;mode&#95;4 | Set to true to enable smoke events in **wand power mode 4** during continuous firing events. *This can be overridden if b&#95;smoke&#95;enabled is set to false or if all 3 of the smoke&#95;1, smoke&#95;2 and fan&#95;continuous are set to false* | true |
| const bool b&#95;smoke&#95;continuous&#95;mode&#95;5 | Set to true to enable smoke events in **wand power mode 5** during continuous firing events. *This can be overridden if b&#95;smoke&#95;enabled is set to false or if all 3 of the smoke&#95;1, smoke&#95;2 and fan&#95;continuous are set to false* | true |
| const unsigned long int i&#95;smoke&#95;timer&#95;mode&#95;1 | How long in milliseconds until the smoke pins & fan pin are activated during a continious firing in **wand power mode 1**. *Example: 30000 is 30 seconds* | 30000 |
| const unsigned long int i&#95;smoke&#95;timer&#95;mode&#95;2 | How long in milliseconds until the smoke pins & fan pin are activated during a continious firing in **wand power mode 2**. *Example: 15000 is 15 seconds* | 15000 |
| const unsigned long int i&#95;smoke&#95;timer&#95;mode&#95;3 | How long in milliseconds until the smoke pins & fan pin are activated during a continious firing in **wand power mode 3**. *Example: 10000 is 10 seconds* | 10000 |
| const unsigned long int i&#95;smoke&#95;timer&#95;mode&#95;4 | How long in milliseconds until the smoke pins & fan pin are activated during a continious firing in **wand power mode 4**. *Example: 7500 is 7.5 seconds* | 7500 |
| const unsigned long int i&#95;smoke&#95;timer&#95;mode&#95;5 | How long in milliseconds until the smoke pins & fan pin are activated during a continious firing in **wand power mode 5**. *Example: 6000 is 6 seconds* | 6000 |
| const unsigned long int i&#95;smoke&#95;on&#95;time&#95;mode&#95;1 | How long do you want your smoke pins and fan pin to stay on while continuous firing in **wand power mode 1** after they have been activated. *Example: 3000 is 3 seconds* | 3000 |
| const unsigned long int i&#95;smoke&#95;on&#95;time&#95;mode&#95;2 | How long do you want your smoke pins and fan pin to stay on while continuous firing in **wand power mode 2** after they have been activated. *Example: 3000 is 3 seconds* | 3000 |
| const unsigned long int i&#95;smoke&#95;on&#95;time&#95;mode&#95;3 | How long do you want your smoke pins and fan pin to stay on while continuous firing in **wand power mode 3** after they have been activated. *Example: 3500 is 3.5 seconds* | 3500 |
| const unsigned long int i&#95;smoke&#95;on&#95;time&#95;mode&#95;4 | How long do you want your smoke pins and fan pin to stay on while continuous firing in **wand power mode 4** after they have been activated. *Example: 3500 is 3.5 seconds* | 3500 |
| const unsigned long int i&#95;smoke&#95;on&#95;time&#95;mode&#95;5 | How long do you want your smoke pins and fan pin to stay on while continuous firing in **wand power mode 3** after they have been activated. *Example: 3500 is 4 seconds* | 4000 |
| const bool b&#95;smoke&#95;1&#95;overheat | Set to true to enable smoke 1 pin during the overheat sequence. *This can be overridden if b&#95;smoke&#95;enabled is false* | true |
| const bool b&#95;smoke&#95;2&#95;overheat | Set to true to enable smoke 2 pin during the overheat seqeuence. *This can be overridden if b&#95;smoke&#95;enabled is false* | true |
| const bool b&#95;fan&#95;overheat | Set to true to enable the fan pin during the overheat seqeuence. *This can be overridden if b&#95;smoke&#95;enabled is false* | true |
| const bool b&#95;smoke&#95;overheat&#95;mode&#95;1 | Set to true to enable smoke events during the overheat sequence in **wand power mode 1**. *This can be overridden if b&#95;smoke&#95;enabled is false or if all 3 of the smoke&#95;1, smoke&#95;2 and fan overheat are set to false*| true |
| const bool b&#95;smoke&#95;overheat&#95;mode&#95;2 | Set to true to enable smoke events during the overheat sequence in **wand power mode 2**. *This can be overridden if b&#95;smoke&#95;enabled is false or if all 3 of the smoke&#95;1, smoke&#95;2 and fan overheat are set to false*| true |
| const bool b&#95;smoke&#95;overheat&#95;mode&#95;3 | Set to true to enable smoke events during the overheat sequence in **wand power mode 3**. *This can be overridden if b&#95;smoke&#95;enabled is false or if all 3 of the smoke&#95;1, smoke&#95;2 and fan overheat are set to false*| true |
| const bool b&#95;smoke&#95;overheat&#95;mode&#95;4 | Set to true to enable smoke events during the overheat sequence in **wand power mode 4**. *This can be overridden if b&#95;smoke&#95;enabled is false or if all 3 of the smoke&#95;1, smoke&#95;2 and fan overheat are set to false*| true |
| const bool b&#95;smoke&#95;overheat&#95;mode&#95;5 | Set to true to enable smoke events during the overheat sequence in **wand power mode 5**. *This can be overridden if b&#95;smoke&#95;enabled is false or if all 3 of the smoke&#95;1, smoke&#95;2 and fan overheat are set to false*| true |
| const bool b&#95;diagnostic | Set to true if you want to know if your wand and pack are communicating. If the wand and pack have a serial connection to each other, you will hear a constant beeping sound. | false |

**Neutrona Wand**

|Variable| Description| Default Setting |
|--------|------------|-----------------|
| const int STARTUP&#95;VOLUME | The default master volume of your wand. This will be overriden if you connect your Neutrona Wand to the Proton Pack. This is a percentage value of between 100% and 0%. 100 = highest, 0 = lowest. | 100 |
| const int STARTUP&#95;VOLUME&#95;MUSIC | The default music volume of the Neutrona Wand. This will be overriden if you connect your Neutrona Wand to the Proton Pack. This is a percentage value of between 100% and 0%. 100 = highest, 0 = lowest. | 100 |
| const int STARTUP&#95;VOLUME&#95;EFFECTS | The default sound effects volume of the Neutrona Wand. This will be overriden if you connect your Neutrona Wand to the Proton Pack. This is a percentage value of between 100% and 0%. 100 = highest, 0 = lowest. | 100 |
| const int MINIMUM&#95;VOLUME | You can set the minimum lowest volume the Neutona Wand can go. 0 = loudest, and -70 = quietest. If your Proton Pack is overpowering your Neutrona Wand sounds at lower volume levels, increase this number. | -35 |
| const int VOLUME&#95;MULTIPLIER | Percentage increments of main volume change. | 2 |
| const int VOLUME&#95;MUSIC&#95;MULTIPLIER | Percentage increments of the music volume change. | 5 |
| const int VOLUME&#95;EFFECTS&#95;MULTIPLIER | Percentage increments of the sound effects volume change. | 5 |
| const bool b&#95;onboard&#95;amp&#95;enabled | Set to true to enable the onboard amplifier of the Wav Trigger. Turning off the onboard amp draws less power. If using the AUX cable jack of the Wav Trigger, the amp can be disabled to save power. If you are using the output pins directly on the Wav Trigger board to your speakers, you will need to enable the amp. **The onboard mono audio amplifier and speaker connector specifications: 2w into 4 Ohms, 1.25W into 8 Ohms** | true |
| const bool b&#95;cross&#95;the&#95;streams |  When set to true, the mode switch button to change firing modes changes to a alternate firing button. Pressing this button together at the same time as the Intensify button does a cross the streams firing. **The video game firing modes will be disabled when you enable this.** **This can be toggled from the Neutrona Wand sub menu system.** | false |
| const bool b&#95;bargraph&#95;alt | Set to true if you are replacing the stock Hasbro bargraph with a Barmeter 28 segment bargraph. Part #: BL28Z-3005SA04Y | false |
| const bool b&#95;vibration&#95;enabled | Enable or disable vibration control for the Neutrona wand. When set to false, there will be no vibration enabled for the Neutrona wand, and it will ignore the toggle switch settings from the Proton Pack. **Note: This can be toggled from the Neutrona Wand sub menu system.** | true |
| bool b&#95;vibration&#95;firing | When set to true, when vibration is enabled from the Proton Pack side, the Neutrona will only vibrate during firing. Setting b&#95;vibration&#95;enabled to false will override this. **Note: This can be toggled from the Neutrona Wand sub menu system.** | false |
| const bool b&#95;no&#95;pack | Set this to true if you want to use your Neutrona Wand without a Proton Pack connected. Otherwise when false, the Neutrona Wand will wait until it is connected to a Proton Pack before it can activate. | false |
| bool b&#95;overheat&#95;enabled | Set to true if you want the Neutrona Wand and Proton Pack to overheat (Overheat settings below still need to be enabled for the necessary power levels. When set to false, it will override and overheating settings in each individual power mode. **Note: This can be toggled from the Neutrona Wand sub menu system.** | true |
| const bool b&#95;overheat&#95;bargraph&#95;blink | Set to true if you want the Neutrona Wand bargraph to blink/flash during overheat. When false, the bargraph ramps down and then ramps back up when overheating has ended.  | false |
| const bool b&#95;overheat&#95;mode&#95;1 | Set to true if you want the Neutrona Wand and Proton Pack to overheat in **wand power mode 1** | false |
| const bool b&#95;overheat&#95;mode&#95;2 | Set to true if you want the Neutrona Wand and Proton Pack to overheat in **wand power mode 2** | false |
| const bool b&#95;overheat&#95;mode&#95;3 | Set to true if you want the Neutrona Wand and Proton Pack to overheat in **wand power mode 3** | false |
| const bool b&#95;overheat&#95;mode&#95;4 | Set to true if you want the Neutrona Wand and Proton Pack to overheat in **wand power mode 4** | false |
| const bool b&#95;overheat&#95;mode&#95;5 | Set to true if you want the Neutrona Wand and Proton Pack to overheat in **wand power mode 5** | true |
| const unsigned long int i&#95;ms&#95;overheat&#95;initiate&#95;mode&#95;1 | Time in milliseconds for when the overheat sequence will initiate for the Neutrona Wand and Proton Pack if enabled for **wand power mode 1**. *Example: 60000 is 60 seconds* | 60000 |
| const unsigned long int i&#95;ms&#95;overheat&#95;initiate&#95;mode&#95;2 | Time in milliseconds for when the overheat sequence will initiate for the Neutrona Wand and Proton Pack if enabled for **wand power mode 2**. *Example: 30000 is 30 seconds* | 30000 |
| const unsigned long int i&#95;ms&#95;overheat&#95;initiate&#95;mode&#95;3 | Time in milliseconds for when the overheat sequence will initiate for the Neutrona Wand and Proton Pack if enabled for **wand power mode 3**. *Example: 20000 is 20 seconds* | 20000 |
| const unsigned long int i&#95;ms&#95;overheat&#95;initiate&#95;mode&#95;4 | Time in milliseconds for when the overheat sequence will initiate for the Neutrona Wand and Proton Pack if enabled for **wand power mode 4**. *Example: 15000 is 15 seconds* | 15000 |
| const unsigned long int i&#95;ms&#95;overheat&#95;initiate&#95;mode&#95;5 | Time in milliseconds for when the overheat sequence will initiate for the Neutrona Wand and Proton Pack if enabled for **wand power mode 5**. *Example: 12000 is 12 seconds* | 12000 |
| const bool b_debug | Debug testing. Set to true to debug the A6 and A7 analog pin readings of the Neutrona Wand. This uses the USB port and the tx/rx pins need to be disconnected from the Proton Pack so serial information can be sent back to the Arduino IDE. *The wand will respond slower in this mode as it is streaming constant serial data back to the debugger.* | false |