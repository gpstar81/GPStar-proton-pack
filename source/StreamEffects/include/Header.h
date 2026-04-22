/**
 *   GPStar Stream Effects - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2024-2026 Dustin Grau <dustin.grau@gmail.com>
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
 * Assumes 50 LEDs per meter using default lighting: https://a.co/d/dlDyCkz
 */
#define DEVICE_LED_PIN 4
#define DEVICE_MAX_LEDS 500 // Set a hard max for allocating the array of LEDs
uint16_t i_num_leds = 250; // Default is 50 LEDs per meter, with a length of 5 meters (eg. 250)
CRGB device_leds[DEVICE_MAX_LEDS];

/*
 * Define Color Options & Timers
 */
CRGBPalette16 paletteWhite;
CRGBPalette16 paletteProton;
CRGBPalette16 paletteSlime;
CRGBPalette16 paletteStasis;
CRGBPalette16 paletteMeson;
CRGBPalette16 paletteSpectral;
CRGBPalette16 paletteHalloween;
CRGBPalette16 paletteChristmas;
CRGBPalette16 paletteBrass;
CRGBPalette16 cp_StreamPalette; // Current colour palette in use.
static const uint8_t i_palette_count = 9; // Total number of palettes available.
static const uint16_t i_selftest_interval = 2000; // 2 seconds between palette changes.
millisDelay ms_selftest_cycle; // Timer for self-test cycling using an interval.
uint8_t i_selftest_palette = 0; // Current palette index for cycling in self-test.

/*
 * Addressable LED Devices
 */
enum device {
  PRIMARY_LED
};

/*
 * LED colour order type for device
 * Defaults to RGB for the type recommended for the build: https://a.co/d/dlDyCkz
 */
enum LED_COLOR_TYPES : uint8_t {
  LED_RGB = 1,
  LED_GRB = 2,
  LED_GBR = 3
};
LED_COLOR_TYPES LED_COLOR_TYPE = LED_RGB;

/**
 * WebSocketData - Holds all relevant fields received from the WebSocket JSON payload.
 */
struct WebSocketData {
  String mode = "";
  String theme = "";
  String switchState = "";
  String pack = "";
  String safety = "";
  uint8_t wandPower = 5; // Default to max power.
  String wandMode = "";
  String firing = "";
  bool ctsActive = false; // Default to not crossing streams.
  String cable = "";
  String cyclotron = "";
  bool cyclotronLid = true; // Default to lid on.
  String temperature = "";
};
WebSocketData wsData; // Instance of WebSocketData struct.

/*
 * Special States
 */
bool b_firing = false;
