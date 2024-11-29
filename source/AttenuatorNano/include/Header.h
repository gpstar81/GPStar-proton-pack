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
 * Pin for Addressable LEDs.
 */
#define DEVICE_LED_PIN 9
// States there are 2 LEDs: Upper and Lower
#define DEVICE_NUM_LEDS 2
CRGB device_leds[DEVICE_NUM_LEDS];

/*
 * LED Device Ordering - Top, Upper, and Lower
 * Creates a simple byte array of N elements for the ID of each of the 3 LEDs.
 * Due to space constraints, users may have had to install the LEDs in reverse.
 * Therefore, the order of this list may change depending on user preference.
 * This feature will only be available for the ESP32-based controller.
 */
bool b_invert_leds = false; // Denotes whether the order should be reversed.
uint8_t i_device_led[DEVICE_NUM_LEDS] = {0, 1}; // Default Order

/*
 * Delay for fastled to update the addressable LEDs.
 * 0.03 ms to update 1 LED, and this device contains 2.
 * Just setting to 3 which should be sufficient.
 */
millisDelay ms_fast_led;
const uint8_t i_fast_led_delay = 3;

/*
 * LED Animation Options
 */
enum LED_ANIMATION : uint8_t {
  AMBER_PULSE = 0,
  ORANGE_FADE = 1,
  RED_FADE = 2
};
enum LED_ANIMATION RAD_LENS_IDLE;

/*
 * Pins for user feedback (audio/physical)
 *
 * Buzzer Frequencies:
 * buzzOn(440); // A4
 * buzzOn(494); // B4
 * buzzOn(523); // C4
 * buzzOn(587); // D4
 * buzzOn(659); // E4
 * buzzOn(698); // F4
 * buzzOn(784); // G4
 */
#define BUZZER_PIN 10
#define VIBRATION_PIN 11
millisDelay ms_buzzer;
millisDelay ms_vibrate;
bool b_buzzer_on = false; // Denotes when pieze buzzer is active.
bool b_vibrate_on = false; // Denotes when vibration moter is active.
const uint8_t i_min_power = 0; // Essentially a "low" state (off).
const uint8_t i_max_power = 255; // Essentially a "high" state (on).
const uint16_t i_buzzer_max_time = 300; // Longest duration for a standalone "beep".
const uint16_t i_vibrate_min_time = 500; // Minimum runtime for vibration motor.
const uint16_t i_vibrate_max_time = 1500; // Maximum runtime for vibration motor.

/*
 * For the alarm and venting/overheat, set the blink/buzz/vibrate interval.
 */
millisDelay ms_blink_leds;
const uint16_t i_blink_leds = 800;
bool b_blink_blank = false; // Denotes when upper/lower LEDs are mid-blink.

/*
 * Barmeter 28 segment bargraph configuration and timers.
 * Part #: BL28Z-3005SA04Y
 * This will use the following pins for i2c serial communication:
 * Arduino Nano
 *   SDA -> A4
 *   SCL -> A5
 * ESP32
 *   SDA -> GPIO 21
 *   SCL -> GPIO 22
 */
HT16K33 ht_bargraph;
const uint8_t i_bargraph_delay = 12; // Base delay (ms) for bargraph refresh (this should be a value evenly divisible by 2, 3, or 4).
const uint8_t i_bargraph_elements = 28; // Maximum elements for bargraph device; not likely to change but adjustable just in case.
const uint8_t i_bargraph_levels = 5; // Reflects the count of POWER_LEVELS elements (the only dependency on other device behavior).
uint8_t i_bargraph_sim_max = i_bargraph_elements; // Simulated maximum for patterns which may be dependent on other factors.
uint8_t i_bargraph_steps = i_bargraph_elements / 2; // Steps for patterns (1/2 max) which are bilateral/mirrored.
uint8_t i_bargraph_step = 0; // Indicates current step for bilateral/mirrored patterns.
int i_bargraph_element = 0; // Indicates current LED element for adjustment.
bool b_bargraph_present = false; // Denotes that i2c bus found the bargraph device.
millisDelay ms_bargraph; // Timer to control bargraph updates consistently.

// Denotes the speed of the cyclotron (1=Normal) which increases as firing continues.
uint8_t i_speed_multiplier = 1;

// Denotes whether the cyclotron lid is currently on (covered) or off (exposed).
bool b_cyclotron_lid_on = true;

/*
 * Barmeter 28 segment bargraph mapping: allows accessing elements sequentially (0-27)
 * If the pattern appears inverted from what is expected, flip by using the following:
 */
//#define GPSTAR_INVERT_BARGRAPH
#ifdef GPSTAR_INVERT_BARGRAPH
  const uint8_t i_bargraph[28] PROGMEM = {54, 38, 22, 6, 53, 37, 21, 5, 52, 36, 20, 4, 51, 35, 19, 3, 50, 34, 18, 2, 49, 33, 17, 1, 48, 32, 16, 0};
#else
  const uint8_t i_bargraph[28] PROGMEM = {0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4, 20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54};
#endif

/*
 * Year Theme
 */
enum SYSTEM_YEARS { SYSTEM_EMPTY, SYSTEM_TOGGLE_SWITCH, SYSTEM_1984, SYSTEM_1989, SYSTEM_AFTERLIFE, SYSTEM_FROZEN_EMPIRE };
enum SYSTEM_YEARS SYSTEM_YEAR;

/*
 * Wand Firing Modes + Settings
 */
enum POWER_LEVELS { LEVEL_1, LEVEL_2, LEVEL_3, LEVEL_4, LEVEL_5 };
enum POWER_LEVELS POWER_LEVEL;
enum POWER_LEVELS POWER_LEVEL_PREV;
enum STREAM_MODES { PROTON, STASIS, SLIME, MESON, SPECTRAL, HOLIDAY_HALLOWEEN, HOLIDAY_CHRISTMAS, SPECTRAL_CUSTOM, SETTINGS };
enum STREAM_MODES STREAM_MODE;

/*
 * Toggle Switches
 * Will be pulled LOW (down position) when considered "on".
 */
#define LEFT_TOGGLE_PIN 5
#define RIGHT_TOGGLE_PIN 6
ezButton switch_left(LEFT_TOGGLE_PIN, INTERNAL_PULLUP);
ezButton switch_right(RIGHT_TOGGLE_PIN, INTERNAL_PULLUP);
bool b_left_toggle_on = false;
bool b_right_toggle_on = false;
bool b_right_toggle_center_start = false;

/*
 * Debounce Settings
 */
const uint8_t switch_debounce_time = 50;
const uint8_t rotary_debounce_time = 100;

/*
 * Rotary encoder for various uses.
 */
#define r_encoderA 2
#define r_encoderB 3
#define r_button 4
ezButton encoder_center(r_button); // For center-press on encoder dial.
millisDelay ms_rotary_debounce; // Put some timing on the rotary so we do not overload the serial communication buffer.
millisDelay ms_center_double_tap; // Timer for determinine when a double-tap was detected.
millisDelay ms_center_long_press; // Timer for determining when a long press was detected.
bool b_center_pressed = false;
bool b_center_lockout = false;
const uint16_t i_center_double_tap_delay = 300; // When to consider the center dial has a "double tap".
const uint16_t i_center_long_press_delay = 600; // When to consider the center dial has a "long" press.
uint8_t i_press_count = 0;
uint8_t i_rotary_count = 0;
int i_encoder_pos = 0;
int i_val_rotary;
int i_last_val_rotary;

/*
 * Define states for the rotary dial center press.
 */
enum CENTER_STATES { NO_ACTION, SHORT_PRESS, DOUBLE_PRESS, LONG_PRESS };
enum CENTER_STATES CENTER_STATE;
enum MENU_LEVELS { MENU_1, MENU_2 };
enum MENU_LEVELS MENU_LEVEL;

/*
 * Some pack flags which get transmitted to the attenuator depending on the pack status.
 */
bool b_pack_on = false;
bool b_wand_present = false;
bool b_wand_on = false;
bool b_pack_alarm = false;
bool b_firing = false;
bool b_overheating = false;

// Flags relating to the synchronization process.
millisDelay ms_packsync;
const uint16_t i_sync_initial_delay = 750; // Delay to re-try the initial handshake with a proton pack.
const uint16_t i_sync_disconnect_delay = 8000; // Delay before we consider the pack missing.

// Flags for denoting when requested data was received.
bool b_received_prefs_pack = false;
bool b_received_prefs_wand = false;
bool b_received_prefs_smoke = false;
