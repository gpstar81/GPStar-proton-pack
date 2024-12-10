/**
 *   GPStar Stream Effects - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2024 Dustin Grau <dustin.grau@gmail.com>
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
 * As an alternative to the standard ESP32 dev board is the Waveshare ESP32-S3 Mini:
 * https://www.waveshare.com/wiki/ESP32-S3-Zero
 */

/*
 * Pin for Addressable LEDs
 * 50 LEDs per Meter: https://a.co/d/dlDyCkz
 */
#define DEVICE_LED_PIN 4
#define DEVICE_NUM_LEDS 250
CRGB device_leds[DEVICE_NUM_LEDS];

/*
 * Define Color Palettes
 */
CRGBPalette16 paletteProton;
CRGBPalette16 paletteSlime;
CRGBPalette16 paletteStasis;
CRGBPalette16 paletteMeson;
CRGBPalette16 paletteSpectral;
CRGBPalette16 paletteHalloween;
CRGBPalette16 paletteChristmas;
CRGBPalette16 paletteWhite;

/*
 * Addressable LED Devices
 */
enum device {
  PRIMARY_LED
};

/*
 * Timer and delay for LED animation sequence
 */
CRGBPalette16 cp_StreamPalette; // Current color palette
millisDelay ms_anim_change;
const uint16_t i_animation_time = 10; // How often to update the position of LEDs
const uint8_t i_animation_step = 6; // Base rate for stepping through positions
uint8_t i_min_brightness = 0;   // Minimum brightness
uint8_t i_max_brightness = 255; // Maximum brightness

/*
 * Wand Firing Modes + Settings
 */
enum POWER_LEVELS { LEVEL_1, LEVEL_2, LEVEL_3, LEVEL_4, LEVEL_5 };
enum POWER_LEVELS POWER_LEVEL;
enum STREAM_MODES { PROTON, STASIS, SLIME, MESON, SPECTRAL, HOLIDAY_HALLOWEEN, HOLIDAY_CHRISTMAS, SPECTRAL_CUSTOM, SETTINGS };
enum STREAM_MODES STREAM_MODE;
bool b_firing = false;
uint8_t i_power = 1;

// Forward declarations.
void debug(String message);