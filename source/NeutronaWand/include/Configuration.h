/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
 * Used to reflect the last build date for the binary.
 */
String build_date = "V6_20250624022601";

/*
 * -------------****** CUSTOM USER CONFIGURABLE SETTINGS ******-------------
 * Change the variables below to alter the behaviour of your Neutrona Wand.
 */

/*
 * You can set the default master startup volume for your wand here.
 * This gets overridden if you connect your wand to the pack.
 * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
 */
const uint8_t STARTUP_VOLUME = 100;

/*
 * You can set the default music volume for your wand here.
 * This gets overridden if you connect your wand to the pack.
 * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
 */
const uint8_t STARTUP_VOLUME_MUSIC = 100;

/*
 * You can set the default sound effects volume for your wand here.
 * This gets overridden if you connect your wand to the pack.
 * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
 */
const uint8_t STARTUP_VOLUME_EFFECTS = 100;

/*
 * Minimum volume that the Neutrona Wand can achieve.
 * Values must be from 0 to -70. 0 = the loudest and -70 = the quietest.
 * Volume changes are based on percentages.
 * If your pack is overpowering the wand at lower volumes, you can either increase the minimum value in the wand,
 * or decrease the minimum value for the pack.
 */
const int8_t MINIMUM_VOLUME = -35;

/*
 * Percentage increments of main volume change.
 */
const uint8_t VOLUME_MULTIPLIER = 5;

/*
 * Percentage increments of the music volume change..
 */
const uint8_t VOLUME_MUSIC_MULTIPLIER = 5;

/*
 * Percentage increments of the sound effects volume change.
 */
const uint8_t VOLUME_EFFECTS_MULTIPLIER = 5;

/*
 * Set to false to disable the onboard amplifier on the WAV Trigger.
 * Turning off the onboard amp draws less power.
 * If using the AUX cable jack, the amp can be disabled to save power.
 * If you use the output pins directly on the WAV Trigger board to your speakers, you will need to enable the onboard amp.
 * NOTE: The onboard mono audio amplifier and speaker connector specifications: 2W into 4 Ohms, 1.25W into 8 Ohms
 */
const bool b_onboard_amp_enabled = true;

/*
 * When set to true, the bargraph will invert the sequence.
 */
bool b_bargraph_invert = false;

/*
 * Enables the optional addressable RGB vent/top light board.
 */
bool b_rgb_vent_light = false;

/*
 * Enables special brightness controls during idle and firing modes if set to true.
 */
bool b_vent_light_control = true;

/*
 * When set to true, you can manually quick vent your Proton Pack and Neutrona Wand by
 * pressing the intensify button if the top right toggle switch is switched off.
 */
bool b_quick_vent = true;

/*
 * When set to true, the LED at the front of the Neutrona Wand body next to the Clippard valve will start blinking after 1 minute of inactivity while the Neutrona Wand and Proton Pack are powered off to indicate battery power is still feeding the system.
*/
bool b_power_on_indicator = true;

/*
 * When set to true, allows selection of special firing modes after the standard video game modes.
 * Nano builds of the wand do not get a full effect of the colours due to memory limitations.
 * gpstar Neutrona Wand boards will the full effect from the wand.
 * The Proton Pack needs RGB coloured LEDs to see the effects on the pack side.
 * Refer to the operational manual for more information regarding these special firing modes.
 * These settings can be controlled from the EEPROM menus.
 * Spectral modes are only supported by the gpstar Neutrona Wand board.
 */
bool b_spectral_mode_enabled = false;
bool b_holiday_mode_enabled = false;
bool b_spectral_custom_mode_enabled = false;

/*
 * The CHSV colour value for the Spectral custom mode.
 * This can be adjusted in the EEPROM LED menu. Any EEPROM settings will overwrite these values.
 * The Proton Pack custom spectral colours are stored on the Proton Pack EEPROM. So it is possible to mix and match different wands colours to different pack settings.
 * Value range: 1 <--> 254
 * Spectral modes are only supported by the gpstar Neutrona Wand board.
 */
uint8_t i_spectral_wand_custom_colour = 200;

/*
 * The CHSV saturation range for the Spectral custom mode.
 * This can be adjusted in the EEPROM LED menu. Any EEPROM settings will overwrite these values.
 * The Proton Pack custom spectral colours are stored on the Proton Pack EEPROM. The Neutrona Wand custom spectral colours are stored on the Neutrona Wand. So it is possible to mix and match different wands colours to different pack settings.
 * Value range: 1 <--> 254
 */
uint8_t i_spectral_wand_custom_saturation = 254;

/*
 * Set to true to enable overheating. Overheat settings are defined below.
 * Set to false to disable overheating. This will override any overheat settings below.
 * This can be controlled from the Neutrona Wand submenu system.
 */
bool b_overheat_enabled = true;

/*
 * Set to true to have your Neutrona Wand boot up with errors when the top right switch (beep switch) is on while you are turning on your wand.
 * When set to false, this will be ignored.
 */
bool b_wand_boot_errors = true;

/*
 * Set to true to have your bargraph blink on/off when the Neutrona Wand and Proton Pack overheat.
 * When false, the bargraph will ramp down instead.
 */
bool b_overheat_bargraph_blink = false;

/*
 * When set to false, the Neutrona Wand beeping will not loop in Afterlife mode.
*/
bool b_beep_loop = true;

/*
 * Which power levels do you want to be able to overheat.
 * Set to true to allow the wand and pack to overheat in that power level.
 * Set to false to disable overheating in that power level. You will be able to continuously fire instead.
 */
bool b_overheat_level_1 = false;
bool b_overheat_level_2 = false;
bool b_overheat_level_3 = false;
bool b_overheat_level_4 = false;
bool b_overheat_level_5 = true;

/*
 * Time in milliseconds for when overheating will initiate if enabled for that power level.
 * Overheat only happens if enabled for that power level (see above).
 * Example: 60000 = (60 seconds)
 * Max is 60000 to keep within the uint16 integer type.
 */
uint16_t i_ms_overheat_initiate_level_1 = 60000;
uint16_t i_ms_overheat_initiate_level_2 = 50000;
uint16_t i_ms_overheat_initiate_level_3 = 40000;
uint16_t i_ms_overheat_initiate_level_4 = 35000;
uint16_t i_ms_overheat_initiate_level_5 = 30000;

/*
 * When set to true, various impact and other stream effects will overlap and mix randomly into the Proton Stream for an added experience.
 */
bool b_stream_effects = true;

/*
 * When set to true, the Neutrona Wand will tell the Proton Pack to play the Neutrona Wand sound effects.
 * Perfect if you want louder Neutrona Wand sounds or if your Neutrona Wand does not have a audio board.
 * The default setting is true.
 */
bool b_extra_pack_sounds = true;

/*
 * Set to false to ignore reading data from the EEPROM.
 */
const bool b_eeprom = true;

/*
 * When set to true, the Neutrona Wand will function without a Proton Pack connected.
 * This prevents the Neutrona Wand from sending and reading serial data. Set to false return communication back to the Proton Pack.
 */
bool b_gpstar_benchtest = false;