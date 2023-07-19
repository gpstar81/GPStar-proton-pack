/**
 *   gpstar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */
 
/*
 * -------------****** CUSTOM USER CONFIGURABLE SETTINGS ******-------------
 * Change the variables below to alter the behaviour of your Proton Pack.
 * All the default settings below are setup for a stock Haslab Proton Pack lighting kit.
*/
 
 /* 
 *  Cyclotron Lid LEDs.
 *  For the stock Haslab LEDs, there are 12 LED's in the cyclotron lid.
 *  Use const int i_cyclotron_leds = 12; and const i_1984_cyclotron_leds 1, 4, 7, 10.
 *
 *  For a 40 LED NeoPixel ring, if you align your ring so that the first led is the middle, then use const int i_cyclotron_leds = 40 and const i_1984_cyclotron_leds 0, 10, 18, 28.
 *  Adjust as neccesary depending on how you align your NeoPixel ring.
 *  You can use any LED setup with up to 40 LEDs. If you change them out to individual NeoPixels or NeoPixel Rings, adjust your settings accordingly.  
 *
 *  i_1984_cyclotron_leds is the middle led aligned in each lens window. (0 is the first LED). Adjust this setting if you use different LED setups and installations.
 *  Put the sequence in order from lowest to highest in a clockwise direction. (Top right lens as cyclotron lens #1 and work your way clockwise)
 *
*/ 
// For stock Haslab LEDs
const uint8_t i_cyclotron_leds = 12;
const uint8_t i_1984_cyclotron_leds[4] = { 1, 4, 7, 10 };

// For a 40 LED NeoPixel ring.
//const uint8_t i_cyclotron_leds = 40;
//const uint8_t i_1984_cyclotron_leds[4] = { 0, 10, 18, 28 };

/*
 * Power Cell LEDs
 * The number of Power Cell leds. Stock Haslab has 13. 
 * If you are installing a Frutto Technology PowerCell which has 15 LEDs, then change this to 15.
 * Note that you may need to adjust the i_powercell_delay_1984 and i_powercell_delay_2021 to a lower number to increase the Power Cell update speed.
*/
const uint8_t i_powercell_leds = 15;

/*
 * Cyclotron Video Game Colour Toggle
 * If you are using Cyclotron Lid LEDs and Inner Cyclotron LEDs with RGB support, such as the Frutto Technology Cyclotron LEDs or NeoPixel Rings etc.
 * You can toggle if you want it to change colours to match the Video Game Modes or stay the default Red at all times.
 * Note that this has no affect on the stock Haslab Cyclotron Lid LEDs, which are red only.
 * The default setting is true, which makes the Cyclotron Lid and Inner Cyclotron change colours to match the Video Game Modes.
 * This can be toggled in the Neutrona wand sub menu system.
*/
bool b_cyclotron_colour_toggle = true;

/*
 * Power Cell Video Game Colour Toggle
 * If you are using a Power Cell LEDs with RGB support, such as the Frutto Technology Power Cells, 
 * You can toggle if you want it to change colours to match the Video Game Modes or stay the default Blue at all times.
 * Note that this has no affect on the stock Haslab Power Cell LEDs, which are blue only.
 * The default setting is true, which makes the Power Cell change colours to match the Video Game Modes.
 * This can be toggled in the Neutrona wand sub menu system.
*/
bool b_powercell_colour_toggle = true;

/*
 * Cyclotron Lid LED delays.
 * Time in milliseconds between when a LED changes.
 * 1000 = 1 second.
 * For a 40 LED NeoPixel ring, 10 for i_2021_delay is good.
 * For stock Haslab LEDs, 15 for i_2021_delay is good.
 * i_1984_delay does not need to be changed at all, unless you want to make the delay shorter or quicker.
*/
const unsigned int i_1984_delay = 1050;
const unsigned int i_2021_delay = 15; // 15 for stock Haslab LEDs. Change to 10 for a 40 LED NeoPixel ring.

/*
 * Power Cell delays in milliseconds.
 * 1000 = 1 second.
 * The lower the number the faster the powercell lights cycle.
 * For the stock Haslab powercell with 13 leds, 1984 at 75 and 2021 at 40 is a good setting.
 * For a Power Cell with 15 LEDS such as from Frutto Technology, 1984 at 60 and 2021 at 34 is a good setting.
 * If you add more powercell leds, it is suggested to lower the values a little bit.
*/
const uint8_t i_powercell_delay_1984 = 60;
const uint8_t i_powercell_delay_2021 = 34;

/*
 * (OPTIONAL) Inner Cyclotron (cake) NeoPixel ring
 * If you are not using any, then this can be left alone.
 * Leave at least one in place, even if you are not using this optional item.
 * You can use up to 35 LEDs.
 * 24 -> For a 24 LED NeoPixel Ring
 * 35 -> For a 35 LED NeoPixel Ring. (Recommended ring size)
*/
//#define CYCLOTRON_NUM_LEDS 24
#define CYCLOTRON_NUM_LEDS 35

/*
 * If you use GRB (green/red/blue) instead of RGB (red/green/blue) addressable LEDs for your inner cyclotron leds, then set to true.
 * Default is false.
*/
bool b_grb_cyclotron = false;

/*
 * You can set the default brightness of your Power Cell, Cyclotron or Inner Cyclotron LEDs.
 * Values are in percentages %.
 * 0 = off.
 * 100 = Maximum brightness.
 * This can be adjusted from the Neutrona wand menu system.
*/
uint8_t i_powercell_brightness = 100;
uint8_t i_cyclotron_brightness = 100;
uint8_t i_cyclotron_inner_brightness = 100;

/*
 * You can set the default master startup volume for your pack here.
 * When a Neutrona wand is connected, it will sync to these settings.
 * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
*/
const uint8_t STARTUP_VOLUME = 100;

/*
 * You can set the default music volume for your pack here.
 * When a Neutrona wand is connected, it will sync to these settings.
 * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
*/
const uint8_t STARTUP_VOLUME_MUSIC = 100;

/*
 * You can set the default sound effects volume for your pack here.
 * When a Neutrona wand is connected, it will sync to these settings.
 * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
 */
const uint8_t STARTUP_VOLUME_EFFECTS = 100;

/*
 * Minimum volume that the pack can achieve. 
 * Values must be from 0 to -70. 0 = the loudest and -70 = the quietest.
 * Volume changes are based on percentages. 
 * If your pack is overpowering the wand at lower volumes, you can either increase the minimum value in the wand,
 * or decrease the minimum value for the pack.
*/
const int MINIMUM_VOLUME = -50;

/*
 * Percentage increments of main volume change.
*/
const uint8_t VOLUME_MULTIPLIER = 2;

/*
 * Percentage increments of the music volume change..
*/
const uint8_t VOLUME_MUSIC_MULTIPLIER = 5;

/*
 * Percentage increments of the sound effects volume change.
*/
const uint8_t VOLUME_EFFECTS_MULTIPLIER = 5;

/*
  * When set to true, various impact and other stream effects will overlap and mix randomly into the Proton Stream for an added experience.
*/
bool b_stream_effects = true;

/*
 * Inner Cyclotron NeoPixel ring speed.
 * The lower the number, the faster it will spin.
 * Default settings for a 35 NeoPixel ring is: 5 for 2021 mode and 9 for 1984/1989 mode.
 * If you are using a ring with less than 35 NeoPixels, you may need to slightly raise these numbers.
*/
const uint8_t i_2021_inner_delay = 5;
const uint8_t i_1984_inner_delay = 9;

/* 
 * Cyclotron direction 
 * Set to true to have your Cyclotron spin clockwise. (default)
 * This can be controlled by an optional switch on pin 29 and also from the Neutrona wand sub menu system.
 * Set to false to be counter clockwise.
*/
bool b_clockwise = true;

/*
 * When set to true, when vibration is enabled from the Proton Pack vibration toggle switch, the Proton Pack will only vibrate always vibrate.
 * Note that vibration is controlled by a the vibration toggle switch and can disable all vibration settings.
 * This can be enabled or disabled from the Neutrona wand sub menu system.
*/
bool b_vibration = true;

/*
 * When set to true, when b_vibration is also set to true, the Proton Pack will only vibrate while the Neutrona wand is firing.
 * Note that vibration is controlled by a the vibration toggle switch in the Proton Pack and it can disable all vibration settings.
 * This can be enabled or disabled from the Neutrona wand sub menu system.
*/
bool b_vibration_firing = true;

/*
 * When set to false, 1984/1989 mode LED's will fade in or out.
*/
const bool b_fade_cyclotron_led = true;

/*
 * When set to true, 1984/1989 will utilise the middle single LED only in each cyclotron lens.
 * When set to false, 3 LEDs from each cyclotron lens will light up instead for 1984/1989 mode.
 * Useful feature for proton packs who utiltise 3 LED's per cyclotron lens, such as the Haslab Proton Pack.
 * This can also be toggled from the Neutrona wand sub menu system.
*/
bool b_cyclotron_single_led = true;

/*
 * When fading is enabled for 1984 mode cyclotron lid lights, control the delay of the fading.
*/
const unsigned int i_1984_fade_out_delay = 210;
const unsigned int i_1984_fade_in_delay = 210;

/*
 * Set to true to enable the onboard amplifer on the wav trigger. 
 * Turning off the onboard amp draws less power. 
 * If using the Stereo AUX cable jack, the amp can be disabled to save power.
 * If you use the output pins directly on the wav trigger board to your speakers, you will need to enable the onboard amp.
 * NOTE: The On-board mono audio amplifier and speaker connector specifications: 2W into 4 Ohms, 1.25W into 8 Ohms
*/
const bool b_onboard_amp_enabled = false;

/*
 * If you want the optional n-filter NeoPixel jewel to strobe during overheat, set to true.
 * If false, the light stay solid white during overheat.
*/
const bool b_overheat_strobe = false;

/*
 * When the pack is overheating, the cyclotron and powercell lights will turn off when set to true.
 * When set to false, the powercell and cyclotron lights ramp at a slow speed during overheating.
*/
bool b_overheat_lights_off = true;

/*
 * When set to true, The n-filter smoke pin will only operate at the same time the n-filter fan pin.
 * If you have a smoke/fan kit for the n-filter that operates the smoke and fan at the same time, and you are connected to the smoke n-filter pin on the pack board, then you MAY want want to set this to true.
 * Alternatively, you can connect to the n-filter fan pin instead to recreate the same effect.
 * When set to false (default), smoke in the n-filter will pump earlier than the fan to fill up the n-filter with some smoke.
 * If you have a smoke kit where the smoke and fan are independently connected to the Proton Pack board, setting to false is preferred.
*/
bool b_overheat_sync_to_fan = false;

/*
 * Enable or disable overall smoke settings.
 * This can be toggled with a switch on PIN 37. This can also be controlled from
the Neutrona wand sub menu system.
 */
bool b_smoke_enabled = true;

/*
 * ****************** ADVANCED USER CONFIGURABLE SMOKE SETTINGS BELOW ************************
 * The default settings work very well. Changing them can produce strange timing effect.
 */
 
/*
 * Enable or disable smoke during continuous firing.
 * Control which of the 3 pins that go high during continuous firing smoke effects.
 * This can be overriden if b_smoke_enabled is set to false.
*/
const bool b_smoke_1_continuous_firing = true;
const bool b_smoke_2_continuous_firing = true;
const bool b_fan_continuous_firing = true;

/*
 * Enable or disable smoke in individual wand power modes for continuous firing smoke.
 * Example: if b_smoke_continuous_mode_1 is true, smoke will happen in continuous firing in wand power mode 1. If false, no smoke in mode 1.
 * This is overridden if b_smoke_enabled or can be by the continuous_firing settings above when they are set to false.
*/
const bool b_smoke_continuous_mode_1 = true;
const bool b_smoke_continuous_mode_2 = true;
const bool b_smoke_continuous_mode_3 = true;
const bool b_smoke_continuous_mode_4 = true;
const bool b_smoke_continuous_mode_5 = true;

/*
 * How long (in milliseconds) until the smoke pins (+ fan) are activated during continuous firing in each firing power mode. (not overheating venting)
 * Example: 30,000 milliseconds (30 seconds)
*/
const unsigned long int i_smoke_timer_mode_1 = 30000;
const unsigned long int i_smoke_timer_mode_2 = 15000;
const unsigned long int i_smoke_timer_mode_3 = 10000;
const unsigned long int i_smoke_timer_mode_4 = 7500;
const unsigned long int i_smoke_timer_mode_5 = 6000;

/*
 * How long do you want your smoke pins (+ fan) to stay on while firing for each firing power mode. (not overheating venting)
 * When the pins are high (controlled by the i_smoke_timer above), then smoke will be generated if you have smoke machines etc wired up.
 * Default is 3000 milliseconds (3 seconds). 
 * This does not affect smoke during overheat. 
 * This only affects how long your smoke stays on after it has been triggered in continuous firing.
*/
const unsigned long int i_smoke_on_time_mode_1 = 3000;
const unsigned long int i_smoke_on_time_mode_2 = 3000;
const unsigned long int i_smoke_on_time_mode_3 = 3500;
const unsigned long int i_smoke_on_time_mode_4 = 3500;
const unsigned long int i_smoke_on_time_mode_5 = 4000;

/*
 * Enable or disable smoke during overheat sequences.
 * Control which of the 3 pins that go 5V high during overheat.
 * This can be overridden if b_smoke_enabled is set to false.
*/
const bool b_smoke_1_overheat = true;
const bool b_smoke_2_overheat = true;
const bool b_fan_overheat = true;

/*
 * Enable or disable overheat smoke in different wand power modes.
 * Example: If b_smoke_overheat_mode_1 is false, then no smoke will be generated during overheat in wand power mode 1, if overheat is enabled for that power mode in the wand code.
 * This is overridden if b_smoke_enabled or can be by the b_overheat settings above when they are set to false.
*/
const bool b_smoke_overheat_mode_1 = true;
const bool b_smoke_overheat_mode_2 = true;
const bool b_smoke_overheat_mode_3 = true;
const bool b_smoke_overheat_mode_4 = true;
const bool b_smoke_overheat_mode_5 = true;

/*
 * Set this to true if you want to know if your wand and pack are communicating.
 * If the wand and pack have a serial connection, you will hear a beeping sound.
 * Set to false to turn off the sound.
*/
const bool b_diagnostic = false;

/*
  *****
  ***** INFORMATION FOR HOME BUILT gpstar Proton Packs using an Arduino Mega ********
  *****
  
  * If you are compiling the code to upload to an Arduino Mega with the original gpstar home built instructions. You want to use disabled GPSTAR_PROTON_PACK_PCB.
  * example: //#define GPSTAR_PROTON_PACK_PCB 
  * This is a legacy flag, for people who originally put the cyclotron lid detection on pin 51 and not pin 43. If your cyclotron lid detection is on pin 51, then comment/disable this define.
  * If your home built gpstar Proton Pack was built with pin 43 for the cyclotron lid detection, then you can leave this enabled.

  * If you are compiling the code to upload to the gpstar Proton Pack micro controller, or latest gpstar home built instructions, then enable and uncomment it (default).
  * example: #define GPSTAR_PROTON_PACK_PCB
  * In general, leave this enabled by default as very few people did the pin 51 setup.
*/
#define GPSTAR_PROTON_PACK_PCB