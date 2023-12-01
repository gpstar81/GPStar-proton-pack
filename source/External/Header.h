/**
 *   GPStar External - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

#define USE_DEBUGS

/*
 * Pin for built-in LED
 */
#define BUILT_IN_LED 2

/*
 * Pin for Addressable LEDs
 */
#define DEVICE_LED_PIN 23
#define DEVICE_NUM_LEDS 1
CRGB device_leds[DEVICE_NUM_LEDS];

/*
 * Pins for RGB LEDs
 */
#define LED_R_PIN 5
#define LED_G_PIN 18
#define LED_B_PIN 19

/*
 * Addressable LED Devices
 */
enum device {
  PRIMARY_LED
};

/*
 * Delay for fastled to update the addressable LEDs
 */
millisDelay ms_fast_led;
const uint8_t i_fast_led_delay = 3;

/*
 * Delay for LED blinking.
 */
millisDelay ms_blink;
const uint8_t i_blink_delay = 200;
bool b_blink = true;

/*
 * Current firing states and wand/firing mode
 */
enum FIRING_MODES { PROTON, SLIME, STASIS, MESON, SPECTRAL, HOLIDAY, SPECTRAL_CUSTOM, VENTING, SETTINGS };
enum FIRING_MODES FIRING_MODE;
bool b_firing = false;
uint8_t i_power = 0;