/**
 *   GPStar BeltGizmo - Ghostbusters Props, Mods, and Kits.
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
 */
#define BUILT_IN_LED 21 // GPIO21 for Waveshare ESP32-S3 Mini (RGB LED)
#define DEVICE_LED_PIN 4
#define DEVICE_MAX_LEDS 11 // 10 "nixie" tubes + 1 "E" bulb
uint8_t i_num_leds = 8; // Default to 7 nixie tubes + 1 "E" bulb
CRGB device_leds[DEVICE_MAX_LEDS];

/*
 * Define Color Options & Timers
 */
#define ANIMATION_DURATION_MS 800  // Time for a full end-to-end animation
millisDelay ms_anim_change;
const uint16_t i_animation_time = 400;
const uint8_t i_animation_step = 4;
uint16_t i_animation_duration = ANIMATION_DURATION_MS / i_num_leds;
bool b_invert_animation = true; // false = Right to Left, true = Left to Right
static const uint8_t i_colour_count = 4; // Total number of colour available.
static const uint16_t i_selftest_interval = 2000; // 2 seconds between colour changes.
millisDelay ms_selftest_cycle; // Timer for self-test cycling using an interval.
uint8_t i_selftest_colour = 0; // Current colour index for cycling in self-test.
uint8_t i_stream_colour; // Current colour index for the stream type.

/*
 * Addressable LED Devices
 */
enum device {
  PRIMARY_LED
};

/*
 * LED colour order type for device
 * Defaults to GBR for the type recommended for the build: https://a.co/d/ia74QSm
 */
enum LED_COLOR_TYPES : uint8_t {
  LED_RGB = 1,
  LED_GRB = 2,
  LED_GBR = 3
};
LED_COLOR_TYPES LED_COLOR_TYPE = LED_GBR;

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
