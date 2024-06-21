/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

#pragma once

/*
 * -------------****** CUSTOM USER CONFIGURABLE SETTINGS ******-------------
 * Change the variables below to alter the behaviour of your Proton Pack.
 * All the default settings below are setup for a stock HasLab Proton Pack lighting kit.
 */

/*
 * Cyclotron Lid LEDs.
 * For the stock HasLab LEDs, there are 12 LEDs in the cyclotron lid.
 * Use uint8_t i_cyclotron_leds = 12.
 *
 * For a 40 LED NeoPixel ring, if you align your ring so that the first LED is the middle, then use uint8_t i_cyclotron_leds = 40.
 * Adjust as necessary depending on how you align your NeoPixel ring.
 * You can use any LED setup with up to 40 LEDs. If you change them out to individual NeoPixels or NeoPixel Rings, adjust your settings accordingly.
 *
 * Any settings saved in the EEPROM menu will overwrite these settings.
 */
uint8_t i_cyclotron_leds = 12;

/*
 * Power Cell LEDs
 * The number of Power Cell LEDs. Stock HasLab has 13.
 * If you are installing a Frutto Technology Power Cell which has 15 LEDs, then change this to 15.
 * Note that you may need to adjust the i_powercell_delay_1984 and i_powercell_delay_2021 to a lower number to increase the Power Cell update speed.
 * Any settings saved in the EEPROM menu will overwrite these settings.
 */
uint8_t i_powercell_leds = 13;

/*
 * Cyclotron Video Game Colour Toggle
 * If you are using Cyclotron Lid LEDs and Inner Cyclotron LEDs with RGB support, such as the Frutto Technology Cyclotron LEDs or NeoPixel Rings etc.
 * You can toggle if you want it to change colours to match the Video Game Modes or stay the default red at all times.
 * Note that this has no effect on the stock HasLab Cyclotron Lid LEDs, which are red only.
 * The default setting is true, which makes the Cyclotron Lid and Inner Cyclotron change colours to match the Video Game Modes.
 * This can be toggled in the Neutrona Wand sub menu system.
 */
bool b_cyclotron_colour_toggle = true;

/*
 * Power Cell Video Game Colour Toggle
 * If you are using Power Cell LEDs with RGB support, such as the Frutto Technology Power Cells,
 * You can toggle if you want it to change colours to match the Video Game Modes or stay the default blue at all times.
 * Note that this has no effect on the stock HasLab Power Cell LEDs, which are blue only.
 * The default setting is true, which makes the Power Cell change colours to match the Video Game Modes.
 * This can be toggled in the Neutrona Wand sub menu system.
 */
bool b_powercell_colour_toggle = true;

/*
 * Cyclotron Lid LED delays.
 * Time in milliseconds between when a LED changes.
 * 1000 = 1 second.
 * i_1984_delay does not need to be changed at all, unless you want to make the delay shorter or quicker.
 *
 * CYCLOTRON_DELAY_2021_12_LED is for the stock Haslab 12 LED setup.
 * CYCLOTRON_DELAY_2021_20_LED is for the Frutto Technology 20 LED setup.
 * CYCLOTRON_DELAY_2021_40_LED is for a 40 LED NeoPixel ring.
 */
const uint16_t i_1984_delay = 1050;
#define CYCLOTRON_DELAY_2021_12_LED 15 // For 12 LEDs.
#define CYCLOTRON_DELAY_2021_20_LED 10 // For 20 LEDs.
#define CYCLOTRON_DELAY_2021_36_LED 5 // For 36 LEDs.
#define CYCLOTRON_DELAY_2021_40_LED 7 // For 40 LEDs.

/*
 * This is the middle LED aligned in each lens window. (0 is the first LED). Adjust these setting if you use different LED setups and installations.
 * Put the sequence in order from lowest to highest in a clockwise direction. (Top right lens as Cyclotron lens #1 and work your way clockwise)
 *
 * i_1984_cyclotron_12_leds is for the stock Haslab 12 LED setup.
 * i_1984_cyclotron_20_leds is for the Frutto Technology 20 LED setup.
 * i_1984_cyclotron_36_leds is for the Frutto Technology Max 36 LED setup.
 * i_1984_cyclotron_40_leds is for a 40 LED NeoPixel ring.
 */
const uint8_t i_1984_cyclotron_12_leds_cw[4] PROGMEM = { 1, 4, 7, 10 };
const uint8_t i_1984_cyclotron_12_leds_ccw[4] PROGMEM = { 1, 10, 7, 4 };
const uint8_t i_1984_cyclotron_20_leds_cw[4] PROGMEM = { 2, 7, 12, 17 };
const uint8_t i_1984_cyclotron_20_leds_ccw[4] PROGMEM = { 2, 17, 12, 7 };
const uint8_t i_1984_cyclotron_36_leds_cw[4] PROGMEM = { 4, 13, 22, 31 };
const uint8_t i_1984_cyclotron_36_leds_ccw[4] PROGMEM = { 4, 31, 22, 13 };
const uint8_t i_1984_cyclotron_40_leds_cw[4] PROGMEM = { 0, 10, 18, 28 };
const uint8_t i_1984_cyclotron_40_leds_ccw[4] PROGMEM = { 0, 28, 18, 10 };

/*
 * Afterlife and Frozen Empire only.
 * When set to true, using LEDs that are not a ring will simulate a ring rotation for the Cyclotron LEDs in the lid.
 * For example, for the 12, 20 or 36 LED options, extra LEDs will be simulated to provide a delay/spinning effect.
 * The 40 LED ring option is unaffected as it is a true ring.
 * This setting will be overridden by the EEPROM settings.
 */
bool b_cyclotron_simulate_ring = true;

/*
 * Power Cell LED delay in milliseconds.
 * 1000 = 1 second.
 * The lower the number the faster the Power Cell lights cycle.
 * For the stock HasLab Power Cell with 13 leds, 1984 at 75 and 2021 at 40 is a good setting.
 * For a Power Cell with 15 LEDS such as from Frutto Technology, 1984 at 60, then for Afterlife and Frozen Empire at 34 is a good setting.
 * If you add more Power Cell LEDs, it is suggested to lower the values a little bit.
 * Any settings saved in the EEPROM menu will overwrite these settings.
 */
uint8_t i_powercell_delay_1984 = 75;
uint8_t i_powercell_delay_2021 = 40;

/*
 * (OPTIONAL) Inner Cyclotron (cake) NeoPixel ring
 * If you are not using any, then this can be left alone.
 * Leave at least one in place even if you are not using this optional item.
 * You can use up to 35 LEDs.
 * 24 -> For a 24 LED NeoPixel Ring
 * 35 -> For a 35 LED NeoPixel Ring. (Recommended ring size)
 */
uint8_t i_inner_cyclotron_cake_num_leds = 35;

/*
 * (OPTIONAL) Inner Cyclotron (cavity) effects
 * If you are not using any, then this can be left alone (Default: 0).
 * You can use up to 20 LEDs (eg. addressable fairy lights as recommended device)
 */
uint8_t i_inner_cyclotron_cavity_num_leds = 0;

/*
 * If you use GRB (green/red/blue) instead of RGB (red/green/blue) addressable LEDs for your Inner Cyclotron LEDs, then set to true.
 * Likewise for the cyclotron cavity LEDs if those are GBR instead of RGB set the flag true to adjust the colour order.
 * Any settings, if saved in the EEPROM, will overwrite these settings.
 */
bool b_grb_cyclotron_cake = false; // Default is false (assumed to be RGB)
bool b_gbr_cyclotron_cavity = true; // Default is true (set false for RGB)

/*
 * The CHSV colour value for the Spectral Custom mode.
 * This can be adjusted in the EEPROM LED menu. Any EEPROM settings will overwrite these values.
 * The Proton Pack custom spectral colours are stored on the Proton Pack EEPROM. The Neutrona Wand custom spectral colours are stored on the Neutrona Wand.
 * So it is possible to mix and match different wands' colours to different pack settings.
 * Value range: 1 <--> 254
 */
uint8_t i_spectral_powercell_custom_colour = 200;
uint8_t i_spectral_cyclotron_custom_colour = 200;
uint8_t i_spectral_cyclotron_inner_custom_colour = 200;

/*
 * The CHSV saturation range for the Spectral Custom mode.
 * This can be adjusted in the EEPROM LED menu. Any EEPROM settings will overwrite these values.
 * The Proton Pack custom spectral colours are stored on the Proton Pack EEPROM. The Neutrona Wand custom spectral colours are stored on the Neutrona Wand. So it is possible to mix and match different wands' colours to different pack settings.
 * Value range: 1 <--> 254
 */
uint8_t i_spectral_powercell_custom_saturation = 254;
uint8_t i_spectral_cyclotron_custom_saturation = 254;
uint8_t i_spectral_cyclotron_inner_custom_saturation = 254;

/*
 * You can set the default brightness of your Power Cell, Cyclotron, Inner Cyclotron, or Inner Cyclotron Switch Panel LEDs.
 * Values are in percentages %.
 * 0 = off.
 * 100 = Maximum brightness.
 * This can be adjusted from the Neutrona Wand menu system.
 */
uint8_t i_powercell_brightness = 100;
uint8_t i_cyclotron_brightness = 100;
uint8_t i_cyclotron_inner_brightness = 100;
uint8_t i_cyclotron_panel_brightness = 100;

/*
 * When set to true, the Proton Pack will turn on automatically when it receives power.
 * If you want your Proton Pack to be silent, change your STARTUP_VOLUME to be 0 and or unplug the power to your amplifier.
 */
bool b_demo_light_mode = false;

/*
 * You can set the default master startup volume for your pack here.
 * When a Neutrona Wand is connected, it will sync to these settings.
 * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
 */
const uint8_t STARTUP_VOLUME = 100;

/*
 * You can set the default music volume for your pack here.
 * When a Neutrona Wand is connected, it will sync to these settings.
 * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
 */
const uint8_t STARTUP_VOLUME_MUSIC = 100;

/*
 * You can set the default sound effects volume for your pack here.
 * When a Neutrona Wand is connected, it will sync to these settings.
 * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
 */
const uint8_t STARTUP_VOLUME_EFFECTS = 100;

/*
 * Minimum volume that the pack can achieve.
 * Values must be from 0 to -70. 0 = the loudest and -70 = the quietest (no audible sound).
 * Volume changes are based on percentages which are converted to the appropriate decibel value.
 * If your pack is overpowering the wand at lower volumes, you can either increase the minimum value in the wand,
 * or decrease the minimum value for the pack. By default the pack will be nearly silent at 0% volume, but not off.
 */
const int8_t MINIMUM_VOLUME = -60;

/*
 * Percentage increments of main volume change.
 */
const uint8_t VOLUME_MULTIPLIER = 5;

/*
 * Percentage increments of the music volume change.
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
 * Default settings for a 35 NeoPixel ring are 5 for 2021 mode and 9 for 1984/1989 mode.
 * If you are using a ring with less than 35 NeoPixels, you may need to slightly raise these numbers.
 */
uint8_t i_2021_inner_delay = 5;
uint8_t i_1984_inner_delay = 9;

/*
 * Cyclotron direction
 * Set to true to have your Cyclotron spin clockwise. (default)
 * This can be controlled by an optional switch on pin 29 and also from the Neutrona Wand sub menu system.
 * Set to false to be counter clockwise.
 * This can be overridden by whatever value is stored in the EEPROM.
 */
bool b_clockwise = true;

/*
 * When set to true, when vibration is enabled from the Proton Pack vibration toggle switch, the Proton Pack will always vibrate.
 * Note that turning off the vibration toggle switch will disable all individual vibration settings.
 * This can be enabled or disabled from the Neutrona Wand sub menu system.
 */
bool b_vibration_enabled = true;

/*
 * When set to true, when b_vibration_enabled is also set to true, the Proton Pack will only vibrate while the Neutrona Wand is firing.
 * Note that turning off the vibration toggle switch will disable all individual vibration settings.
 * This can be enabled or disabled from the Neutrona Wand sub menu system.
 */
bool b_vibration_firing = true;

/*
 * When set to true, 1984/1989 mode LEDs will fade in and out.
 */
const bool b_fade_cyclotron_led = true;

/*
 * When set to true, 1984/1989 will utilise the middle single LED only in each cyclotron lens.
 * When set to false, 3 LEDs from each cyclotron lens will light up instead for 1984/1989 mode.
 * Useful feature for Proton Packs that utiltise 3 LEDs per Cyclotron lens, such as the HasLab Proton Pack.
 * This can also be toggled from the Neutrona Wand sub menu system.
 */
bool b_cyclotron_single_led = true;

/*
 * When fading is enabled for 1984 mode Cyclotron Lid lights, control the delay of the fading.
 */
const uint16_t i_1984_fade_out_delay = 1110;
const uint16_t i_1984_fade_in_delay = 210;

/*
 * Set to true to enable the onboard amplifier on the WAV Trigger.
 * This is for the WAV Trigger only and does not affect GPStar Audio.
 * If you use the output pins directly on the WAV Trigger board to your speakers, you will need to enable the onboard amp.
 * NOTE: The onboard mono audio amplifier and speaker connector specifications: 2W into 4 Ohms, 1.25W into 8 Ohms
 */
const bool b_onboard_amp_enabled = false;

/*
 * If you want the optional N-Filter NeoPixel jewel to strobe during overheat venting.
 * If false, the light will stay solid during overheat venting.
 * This does not affect the LED-W optional light nor does it affect the jewel during continuous fire venting which always strobes.
 * LED-W always stays solid during any venting sequences.
 */
bool b_overheat_strobe = true;

/*
 * When the pack is overheating, the Cyclotron and Power Cell lights will ramp off when set to true.
 * When set to false, the Power Cell and Cyclotron lights ramp at a slow speed during overheating.
 */
bool b_overheat_lights_off = true;

/*
 * When set to true, The N-Filter smoke pin will only operate at the same time the N-Filter fan pin.
 * If you have a smoke/fan kit for the N-Filter that operates the smoke and fan at the same time, and you are connected to the smoke N-Filter pin on the pack board, then you MAY want to set this to true.
 * Alternatively, you can connect to the N-Filter fan pin instead to recreate the same effect.
 * When set to false (default), smoke in the N-Filter will pump earlier than the fan to fill up the N-Filter with some smoke.
 * If you have a smoke kit where the smoke and fan are independently connected to the Proton Pack board, setting to false is preferred.
 */
bool b_overheat_sync_to_fan = false;

/*
 * Enable or disable overall smoke settings.
 * This can be toggled with a switch on PIN 37. This can also be controlled from the Neutrona Wand sub menu system.
 * This can be overridden by whatever value is stored in the EEPROM.
 */
bool b_smoke_enabled = true;

/*
 * Enable or disable the inner cyclotron LED panel board.
 * When enabled, this becomes the first in the chain from the Inner Cyclotron JST-XH connector from the Proton Pack.
 */
bool b_inner_cyclotron_led_panel = false;

/*
 * ****************** ADVANCED USER CONFIGURABLE SMOKE SETTINGS BELOW ************************
 * The default settings work very well. Changing these can produce strange timing effects.
 */

/*
 * Enable or disable smoke during continuous firing.
 * Control which of the 4 pins go high during continuous firing smoke effects.
 * This can be overridden if b_smoke_enabled is set to false.
 */
bool b_smoke_1_continuous_firing = true;
bool b_smoke_2_continuous_firing = true;
bool b_fan_continuous_firing = true;
bool b_fan_booster_continuous_firing = true;

/*
 * Enable or disable smoke in individual wand power levels for continuous firing smoke.
 * Example: if b_smoke_continuous_level_1 is true, smoke will happen in continuous firing in wand power level 1. If false, no smoke in mode 1.
 * This is overridden if b_smoke_enabled or can be by the continuous_firing settings above when they are set to false.
 */
bool b_smoke_continuous_level_1 = true;
bool b_smoke_continuous_level_2 = true;
bool b_smoke_continuous_level_3 = true;
bool b_smoke_continuous_level_4 = true;
bool b_smoke_continuous_level_5 = true;

/*
 * How long (in milliseconds) until the smoke pins (+ fan) are activated during continuous firing in each firing power level (not overheating venting).
 * Example: 30,000 milliseconds (30 seconds)
 */
const uint32_t i_smoke_timer_level_1 = 30000;
const uint32_t i_smoke_timer_level_2 = 15000;
const uint32_t i_smoke_timer_level_3 = 10000;
const uint32_t i_smoke_timer_level_4 = 7500;
const uint32_t i_smoke_timer_level_5 = 6000;

/*
 * How long you want your smoke pins (+ fan) to stay on while firing for each firing power level. (not overheating venting)
 * When the pins are high (controlled by the i_smoke_timer above), then smoke will be generated if you have smoke machines wired up.
 * Default is 3000 milliseconds (3 seconds).
 * This does not affect smoke during overheat.
 * This only affects how long your smoke stays on after it has been triggered in continuous firing.
 */
const uint32_t i_smoke_on_time_level_1 = 3000;
const uint32_t i_smoke_on_time_level_2 = 3000;
const uint32_t i_smoke_on_time_level_3 = 3500;
const uint32_t i_smoke_on_time_level_4 = 3500;
const uint32_t i_smoke_on_time_level_5 = 4000;

/*
 * Enable or disable smoke during overheat sequences.
 * Control which of the 3 pins that go 5V high during overheat.
 * This can be overridden if b_smoke_enabled is set to false.
 */
bool b_smoke_1_overheat = true;
bool b_smoke_2_overheat = true;
bool b_fan_overheat = true;
bool b_fan_booster_overheat = true;

/*
 * Enable or disable overheat smoke in different wand power levels.
 * Example: If b_smoke_overheat_level_1 is false, then no smoke will be generated during overheat in wand power level 1, if overheat is enabled for that power level in the wand code.
 * This is overridden if b_smoke_enabled or can be by the b_overheat settings above when they are set to false.
 */
const bool b_smoke_overheat_level_1 = true;
const bool b_smoke_overheat_level_2 = true;
const bool b_smoke_overheat_level_3 = true;
const bool b_smoke_overheat_level_4 = true;
const bool b_smoke_overheat_level_5 = true;

/*
 * This is the length in duration of the overheat sequence when the fan (and smoke if synced to fan) stays on for.
 * This can be adjusted in 1 second increments via the wand menu system.
 * Default setting is for overheat to only happen in power level 5. However this can be adjused on the Neutrona Wand to enable overheating in any power level.
 * It is recommended not to go below 2000 milliseconds.
 */
uint16_t i_ms_overheating_length_1 = 2000; // Time in milliseconds (2 seconds) for the overheating to last when the fans (and smoke when synced to fan) turns on. Power Level 1.
uint16_t i_ms_overheating_length_2 = 3000; // Time in milliseconds (3 seconds) for the overheating to last when the fans (and smoke when synced to fan) turns on. Power Level 2.
uint16_t i_ms_overheating_length_3 = 4000; // Time in milliseconds (4 seconds) for the overheating to last when the fans (and smoke when synced to fan) turns on. Power Level 3.
uint16_t i_ms_overheating_length_4 = 5000; // Time in milliseconds (5 seconds) for the overheating to last when the fans (and smoke when synced to fan) turns on. Power Level 4.
uint16_t i_ms_overheating_length_5 = 6000; // Time in milliseconds (6 seconds) for the overheating to last when the fans (and smoke when synced to fan) turns on. Power Level 5.

/*
 * Set this to true if you want to know if your wand and pack are communicating.
 * If the wand and pack have a serial connection, you will hear a beeping sound.
 * Set to false to turn off the sound.
 */
const bool b_diagnostic = false;

/*
 * Set to false to disable the Proton Pack Ribbon Alarm switch.
 */
bool b_use_ribbon_cable = true;

/*
 * Set to false to ignore reading data from the EEPROM.
 */
const bool b_eeprom = true;

/*
 * Changing the colour space with a CHSV Object affects the brightness slightly for non RGB pixels such as the ones used in the HasLab Cyclotron Lid.
 * When using 12 LEDs for the Cyclotron Lid, the system will default it to always red.
 * Setting this to true will override it and allow CHSV colours to be applied to Cyclotron Lids with 12 LEDs.
 * Note that a NeoPixel Jewel will use the CHSV colour space which can make the default HasLab Cyclotron LEDs flicker when the jewel N-Filter vent-light strobes.
 */
const bool b_cyclotron_haslab_chsv_colour_change = false;

/*
 *****
 ***** INFORMATION FOR DIY builds of the gpstar Proton Packs using an Arduino Mega ********
 *****

 * If you are compiling the code to upload to an Arduino Mega with the original gpstar home built instructions, you will want to disable GPSTAR_PROTON_PACK_PCB.
 * example: //#define GPSTAR_PROTON_PACK_PCB
 * This is a legacy flag, for people who originally put the Cyclotron Lid detection on pin 51 and not pin 43. If your Cyclotron Lid detection is on pin 51, then comment/disable this define.
 * If your home built gpstar Proton Pack was built with pin 43 for the Cyclotron Lid detection, then you can leave this enabled.
 *
 * If you are compiling the code to upload to the gpstar Proton Pack microcontroller, or latest gpstar home built instructions, then enable and uncomment it (default).
 * example: #define GPSTAR_PROTON_PACK_PCB
 * In general, leave this enabled by default as very few people did the pin 51 setup.
 */
#define GPSTAR_PROTON_PACK_PCB