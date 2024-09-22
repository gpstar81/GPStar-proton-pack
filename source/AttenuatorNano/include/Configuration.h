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

//#define DEBUG_SEND_TO_CONSOLE  // Send any messages to the serial (USB) console.

/*
 * Enable Physical Feedback Effects (Sound + Vibration)
 */
const bool b_enable_buzzer = true; // Enable/disable all buzzing via the local piezo buzzer
const bool b_enable_vibration = true; // Enable/disable all effects via the vibration motor
const bool b_overheat_feedback = true; // Enable/disable buzzing/vibration on pack overheat
const bool b_firing_feedback = false; // Enable/disable vibration when throwing a stream

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
