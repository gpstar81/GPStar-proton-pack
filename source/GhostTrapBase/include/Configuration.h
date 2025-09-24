/**
 *   GPStar Ghost Trap - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Nomake Wan <-redacted->
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
 * Control debug messages for various actions during normal operation.
 * Uncomment the desired line(s) to output messages when and where you
 * expect to see them. Using the console should be reserved for active
 * debugging, while the websocket will help with confirming operations
 * while using the device (post-setup for wireless).
 */
#define DEBUG_WIRELESS_SETUP   // Output debugs related to the WiFi/network setup.
//#define DEBUG_PERFORMANCE      // Send debug messages for CPU/memory to the (USB) console.
#define DEBUG_SEND_TO_CONSOLE  // Send any general messages to the serial (USB) console.
//#define DEBUG_TASK_TO_CONSOLE  // Send any task messages to the serial (USB) console.
#define DEBUG_SEND_TO_WEBSOCKET  // Send any messages to connected WebSocket clients.

/*
 * Force the use of default SSID and password for wireless capabilities.
 * Uncomment and upload to device, then perform a reset of your password
 * to a new and known value. When completed, flash the latest version of
 * the software which has this line commented out.
 */
//#define RESET_AP_SETTINGS

/*
 * Used to reflect the last build date for the binary.
 */
String build_date = "V6_20250924101550";

/*
 * Preferred WiFi Network Defaults
 * Directly provides information for an external WiFi network for the device to join.
 */
String user_wifi_ssid = ""; // Preferred network SSID for external WiFi
String user_wifi_pass = ""; // Preferred network password for external WiFi

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
 * Set to true to enable the onboard amplifier on the WAV Trigger.
 * This is for the WAV Trigger only and does not affect GPStar Audio.
 * If you use the output pins directly on the WAV Trigger board to your speakers, you will need to enable the onboard amp.
 * NOTE: The onboard mono audio amplifier and speaker connector specifications: 2W into 4 Ohms, 1.25W into 8 Ohms
 */
const bool b_onboard_amp_enabled = false;
