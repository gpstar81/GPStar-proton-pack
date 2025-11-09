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

#ifdef ESP32
/*
 * Used to reflect the last build date for the binary.
 */
String build_date = "V6_20251109070855";

/*
 * Control debug messages for various actions during normal operation.
 * Uncomment the desired line(s) to output messages when and where you
 * expect to see them. Using the console should be reserved for active
 * debugging, while the websocket will help with confirming operations
 * while using the device (post-setup for wireless).
 */
//#define DEBUG_WIRELESS_SETUP     // Output debugs related to the WiFi/network setup.
//#define DEBUG_SEND_TO_CONSOLE    // Send any general messages to the serial (USB) console.
//#define DEBUG_SEND_TO_WEBSOCKET  // Send any messages to connected WebSocket clients.
//#define DEBUG_TELEMETRY_DATA     // Output debugs related to the motion sensors.

/*
 * Force the use of default SSID and password for wireless capabilities.
 * Uncomment and upload to device, then perform a reset of your password
 * to a new and known value. When completed, flash the latest version of
 * the software which has this line commented out.
 */
//#define RESET_AP_SETTINGS

/*
 * Enable the use of the onboard sensors for telemetry tracking.
 * Leave this defined to enable the magnetometer and gyroscope.
 * Only available on the ESP32 builds.
 */
#define MOTION_SENSORS

/*
 * Enable the use of motion offsets (bias compensation) for sensors.
 * Only available on the ESP32 builds.
 */
#define MOTION_OFFSETS
#endif

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
const uint8_t STARTUP_VOLUME = 100;

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
 * Enables the optional addressable RGB vent/top light board.
 */
#ifdef ESP32
  const bool b_rgb_vent_light = true; // Only RGB for ESP32 builds, user settings are ignored.
#else
  bool b_rgb_vent_light = false; // Assumes stock LED for ATMega, overridden with EEPROM.
#endif

/*
 * When set to true, the LED at the front of the Single-Shot Blaster body next to the Clippard valve will
 * start blinking after 1 minute of inactivity to indicate battery power is still feeding the system.
 */
bool b_power_on_indicator = true;

/*
 * Set to false to ignore reading data from the EEPROM.
 */
const bool b_eeprom = true;

/*
 * Vibration modes for the device.
 */
enum VIBRATION_MODES : uint8_t {
  VIBRATION_EMPTY = 0,
  VIBRATION_NONE = 1,
  VIBRATION_FIRING_ONLY = 2,
  VIBRATION_ALWAYS = 4  
};

/*
 * Data structure object for device customizations which are saved into NVS/EEPROM.
 * WARNING: Do not reorder fields without changing field names or types, as this
 * would cause size validation to pass but load data into incorrect fields.
 */
struct __attribute__((packed)) UserDeviceConfig {
  bool deviceBootErrorBeep = true; // Enables the error beeps when the device is started with the top right switch on.
  bool invertBlasterBargraph = false; // When set to true, the bargraph will invert all animation sequences.
  bool ventLightAutoIntensity = true; // Enables special brightness controls during idle and firing modes.
  bool ventLightRGB = b_rgb_vent_light; // Enables the addressable RGB vent/top light board.
  bool gpstarAudioLed = false; // When set to true, the LED on the GPStar Audio stay on while the system is running.
  uint8_t defaultSystemVolume = STARTUP_VOLUME; // Sets the default system volume percentage (0-100).
  VIBRATION_MODES deviceVibration = VIBRATION_FIRING_ONLY; // Sets the vibration mode (default: only when firing).
} blasterConfig;
