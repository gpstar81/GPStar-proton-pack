/**
 *   GPStar Single-Shot Blaster
 *   Copyright (C) 2024-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Dustin Grau <dustin.grau@gmail.com>
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
 * Change the variables below to alter the behaviour of your Single-Shot Blaster.
 */

/*
 * You can set the default master startup volume for your device here.
  * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
 */
const uint8_t STARTUP_VOLUME = 80;

/*
 * You can set the default music volume for your device here.
 * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
 */
const uint8_t STARTUP_VOLUME_MUSIC = 100;

/*
 * You can set the default sound effects volume for your device here.
 * Values are in % of the volume.
 * 0 = quietest
 * 100 = loudest
 */
const uint8_t STARTUP_VOLUME_EFFECTS = 100;

/*
 * Minimum volume that the Single-Shot Blaster can achieve.
 * Values must be from 0 to -70. 0 = the loudest and -70 = the quietest.
 * Volume changes are based on percentages.
 */
const int8_t MINIMUM_VOLUME = -70;

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
 * When set to true, the LED at the front of the Single-Shot Blaster body next to the Clippard valve will
 * start blinking after 1 minute of inactivity to indicate battery power is still feeding the system.
 */
bool b_power_on_indicator = true;

/*
 * Set to true to have your Single-Shot Blaster boot up with errors when the top right switch (beep switch) is on while you are turning on your device.
 * When set to false, this will be ignored.
 */
bool b_device_boot_errors = true;

/*
 * Set to false to disable the onboard amplifier on the WAV Trigger.
 * Turning off the onboard amp draws less power.
 * If using the AUX cable jack, the amp can be disabled to save power.
 * If you use the output pins directly on the WAV Trigger board to your speakers, you will need to enable the onboard amp.
 * NOTE: The onboard mono audio amplifier and speaker connector specifications: 2W into 4 Ohms, 1.25W into 8 Ohms
 */
const bool b_onboard_amp_enabled = true;

/*
 * Set to false to ignore reading data from the EEPROM.
 */
const bool b_eeprom = true;
