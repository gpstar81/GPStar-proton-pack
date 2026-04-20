/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2026 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

#include "BuildInfo.h"

/*
 * Control debug messages for various actions during normal operation.
 * Uncomment the desired line(s) to output messages when and where you
 * expect to see them. Using the console should be reserved for active
 * debugging, while the websocket will help with confirming operations
 * while using the device (post-setup for wireless).
 *
 * For console output, must first set GPSTAR_DEBUG 1 in main.cpp to enable debug macros.
 */
//#define DEBUG_WIRELESS_SETUP    // Output debugs related to the WiFi/network setup.
//#define DEBUG_PERFORMANCE       // Send debug messages for CPU/memory to the (USB) console.
//#define DEBUG_SERIAL_COMMS      // Output debugs related to the serial communications.
//#define DEBUG_SEND_TO_CONSOLE   // Send any general messages to the serial (USB) console.
//#define DEBUG_TASK_TO_CONSOLE   // Send any task messages to the serial (USB) console.
//#define DEBUG_SEND_TO_WEBSOCKET // Send any messages to connected WebSocket clients.
//#define DEBUG_SEND_TO_EVENTS    // Send any messages to the server-side events stream.

/*
 * Force the use of default SSID and password for wireless capabilities.
 * Uncomment and upload to device, then perform a reset of your password
 * to a new and known value. When completed, flash the latest version of
 * the software which has this line commented out.
 */
//#define RESET_AP_SETTINGS

/*
 * Invert logic for toggle switches
 */
bool b_left_toggle_inverted = false;
bool b_right_toggle_inverted = false;

/*
 * Enable Physical Feedback Effects (Sound + Vibration)
 */
bool b_enable_buzzer = true; // Enable/disable all buzzing via the local piezo buzzer
bool b_enable_vibration = true; // Enable/disable all effects via the vibration motor
bool b_overheat_feedback = true; // Enable/disable buzzing/vibration on pack overheat
bool b_firing_feedback = false; // Enable/disable vibration when throwing a stream

/*
 * Enable Visual Feedback Effects (Bargraph + Device LEDs, UI Animations)
 */
bool b_enable_bargraph = true; // Enable/disable bargraph display
bool b_enable_device_leds = true; // Enable/disable device LEDs display
bool b_enable_ui_animations = true; // Enable/disable UI animation effects
bool b_invert_leds = false; // Denotes whether the order should be reversed.
bool b_grb_leds = false; // Denotes whether to use GRB ordering for LEDs.

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
