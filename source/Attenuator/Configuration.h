/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                         & Dustin Grau <dustin.grau@gmail.com>
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
 * Debugging options for ESP32 only.
 * Control debug messages for various actions during normal operation.
 * Uncomment the desired line(s) to output messages when and where you
 * expect to see them. Using the console should be reserved for active
 * debugging, while the websocket will help with confirming operations
 * while using the device (post-setup for wireless).
 */
//#define DEBUG_WIRELESS_SETUP   // Output debugs related to the WiFi/network setup.
//#define DEBUG_SERIAL_COMMS     // Output debugs related to the serial communications.
//#define DEBUG_SEND_TO_CONSOLE  // Send any messages to the serial (USB) console.
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
String build_date = "20240918145440";

/*
 * Preferred WiFi Network Defaults
 * When a network name/password is not specified via the web UI, these
 * values may be used to provide defaults for joining a known network.
 * Set these manually to have your device connect automatically to a
 * known wireless network without needing to access the private WiFi.
 * Note: Applies only to usage with the ESP32 not the Arduino Nano.
 */
String user_wifi_ssid = ""; // Preferred network SSID for external WiFi
String user_wifi_pass = ""; // Preferred network password for external WiFi

/*
 * Enable Physical Feedback Effects (Sound + Vibration)
 */
bool b_enable_buzzer = true; // Enable/disable all buzzing via the local piezo buzzer
bool b_enable_vibration = true; // Enable/disable all effects via the vibration motor
bool b_overheat_feedback = true; // Enable/disable buzzing/vibration on pack overheat
bool b_firing_feedback = false; // Enable/disable vibration when throwing a stream

/*
 * Wait for pack communication or operate without pack integration.
 */
bool b_wait_for_pack = true;

/*
 * Custom values from pack EEPROM.
 *
 * Override as desired if b_wait_for_pack is false.
 */
uint8_t i_spectral_custom_colour = 0;
uint8_t i_spectral_custom_saturation = 254;
