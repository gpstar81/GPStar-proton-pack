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

// Reserved for I2C Bus
// SDA -> GPIO21
// SLC -> GPIO22

// Reserved for Serial2
// RX2 -> GPIO16
// TX2 -> GPIO17

#define USE_ESP32_S3

/*
 * Pin for Addressable LEDs
 */
#if defined(USE_ESP32_S3)
  // GPIO21 for Waveshare ESP32-S3 Mini (RGB LED)
  #define BUILT_IN_LED 21
  #define DEVICE_NUM_LEDS 1
  CRGB device_leds[DEVICE_NUM_LEDS];
#else
  // GPIO2 for ESP-WROOM-32 (Blue LED)
  #define BUILT_IN_LED 2
#endif

/*
 * Pins for Devices
 */
#if defined(USE_ESP32_S3)
  // ESP32-S3 Mini
  #define BLOWER_PIN 5 // 30mm blower fan for smoke
  #define SMOKE_PIN 7 // MOSFET for smoke (coil + pump)
  #define TOP_2WHITE 10 // Frutto top panel (2 white LEDs)
  #define TOP_PIXELS 11 // Frutto top panel (12 RGB pixels)
  #define DOOR_CLOSED_PIN 8 // Green Socket (Input Only)
  #define DOOR_OPENED_PIN 9 // Red Socket (Input Only)
#else
  // ESP-WROOM-32
  #define BLOWER_PIN 18 // 30mm blower fan for smoke
  #define SMOKE_PIN 23 // MOSFET for smoke (coil + pump)
  #define TOP_2WHITE 32 // Frutto top panel (2 white LEDs)
  #define TOP_PIXELS 33 // Frutto top panel (12 RGB pixels)
  #define DOOR_CLOSED_PIN 34 // Green Socket (Input Only)
  #define DOOR_OPENED_PIN 35 // Red Socket (Input Only)
#endif

/*
 * Frutto Trap Top (Pixels)
 */
#define NUM_TOP_PIXELS 12
CRGB top_leds[NUM_TOP_PIXELS];

/*
 * Timers for Devices
 */
millisDelay ms_blower;
millisDelay ms_light;
millisDelay ms_smoke;
millisDelay ms_top_leds;

/*
 * Limits for Operation
 */
const uint8_t i_min_power = 0; // Essentially a "low" state (off).
const uint8_t i_max_power = 255; // Essentially a "high" state (on).
const uint16_t i_smoke_duration_min = 1000; // Minimum "sane" time to run smoke (1 second).
const uint16_t i_smoke_duration_max = 10000; // Do not allow smoke to run more than 10 seconds.
const uint16_t i_blower_start_delay = 1500; // Time to delay start of the blower for smoke, allowing built-up (1.5 second).
const uint16_t i_top_leds_delay = 60; // Delay for top LEDs (100ms).

/*
 * Global flag to enable/disable smoke.
 */
bool b_smoke_enabled = true;

/*
 * UI Status Display Type
 */
enum DISPLAY_TYPES : uint8_t {
  STATUS_TEXT = 0,
  STATUS_GRAPHIC = 1,
  STATUS_BOTH = 2
};
enum DISPLAY_TYPES DISPLAY_TYPE;

/*
 * Device States
 */
enum DOOR_STATES : uint8_t {
  DOORS_UNKNOWN = 0,
  DOORS_CLOSED = 1,
  DOORS_OPENED = 2
};
enum DOOR_STATES DOOR_STATE;
enum DOOR_STATES LAST_DOOR_STATE;

/*
 * Smoke Control
 */
bool b_smoke_opened_enabled = false;
bool b_smoke_closed_enabled = false;
uint16_t i_smoke_opened_duration = 2000;
uint16_t i_smoke_closed_duration = 3000;

// Forward declarations.
void debug(String message);
