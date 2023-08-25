/**
 *   gpstar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
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
 * Pin for Addressable LED's.
*/
#define ATTENUATOR_LED_PIN 12
#define ATTENUATOR_NUM_LEDS 2
CRGB attenuator_leds[ATTENUATOR_NUM_LEDS];

/*
 * Delay for fastled to update the addressable LEDs. 
 */
const uint8_t i_fast_led_delay = 6;
millisDelay ms_fast_led;

/*
 * Barmeter 28 segment bargraph configuration and timers.
 * Part #: BL28Z-3005SA04Y
*/
HT16K33 ht_bargraph;
  
// Used to scan the i2c bus and to locate the 28 segment bargraph.
#define WIRE Wire

/* 
 *  State of the pack.
 */
enum PACK_STATE { MODE_OFF, MODE_ON };
enum PACK_STATE PACK_STATUS;

/*
 * Pack action state.
 */
enum PACK_ACTION_STATE { ACTION_IDLE, ACTION_OFF, ACTION_ACTIVATE };
enum PACK_ACTION_STATE PACK_ACTION_STATUS;

/* 
 *  Switches
 */
ezButton switch_left(23);
ezButton switch_right(25);

/*
 * Switch Settings.
 */
const uint8_t switch_debounce_time = 50;

/* 
 *  Wand Firing Modes + Settings
 */
enum FIRING_MODES { PROTON, SLIME, STASIS, MESON, SPECTRAL, HOLIDAY, SPECTRAL_CUSTOM, VENTING, SETTINGS };
enum FIRING_MODES FIRING_MODE;

/* 
 * Rotary encoder for pattern selection
 */
#define r_encoderA 2
#define r_encoderB 3

/*
 * LED Devices.
 */
enum device {
  UPPER,
  LOWER
};
