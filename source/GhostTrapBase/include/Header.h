/**
 *   GPStar Ghost Trap - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Nomake Wan <nomake_wan@yahoo.co.jp>
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

/*
 * Pins for Devices
 */
#define ENCODER_A 4 // Pin which connects to the "A" side of the rotary encoder
#define ENCODER_B 5 // Pin which connects to the "B" side of the rotary encoder
#define TOP_ROD_SWITCH_PIN 7 // Pin which connects to the top rod detection switch
#define BOTTOM_ROD_SWITCH_PIN 17 // Pin which connects to the bottom rod detection switch
#define PEDAL_LED_PIN 18 // Pin which connects to the pedal LED
#define PEDAL_SWITCH_PIN 42 // Pin which connects to the pedal switch
#define SERVICE_SWITCH_PIN 3 // Pin which connects to the hidden switch in the trap handle
#define G_INT1_PIN 12 // Pin which connects to the G_INT1 pin on the IMU
#define G_INT2_PIN 21 // Pin which connects to the G_INT2 pin on the IMU
#define NORUMBLE_TOGGLE_PIN 13 // Pin which detects when the DPST switch is in the no-rumble position
#define RUMBLE_TOGGLE_PIN 14 // Pin which detects when the DPST switch is in the rumble position
#define SDA_PIN 47 // SDA pin for I2C communications with the IMU
#define SCL_PIN 48 // SCL pin for I2C communications with the IMU
#define GHOST_SELECTOR_W_PIN 40 // Pin from the "W" wire of the ghost selector knob
#define GHOST_SELECTOR_R_PIN 39 // Pin from the "R" wire of the ghost selector knob
#define GHOST_SELECTOR_B_PIN 38 // Pin from the "B" wire of the ghost selector knob
#define RED_LED_PIN 41 // Pin to control the red LED on the top of the trap base
#define VIBRATION_PIN 2 // Pin to trigger vibration motor
#define UPDI_PIN 1 // Pin to program trap cartridge via UPDI

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
enum DISPLAY_TYPES DISPLAY_TYPE = STATUS_GRAPHIC;

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
