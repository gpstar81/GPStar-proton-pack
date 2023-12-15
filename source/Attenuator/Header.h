/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
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

#pragma once

/*
 * Pin for Addressable LEDs.
 */
#if defined(__XTENSA__)
  // ESP32
  #define DEVICE_LED_PIN 23
  #define BUILT_IN_LED 2
#else
  // Nano
  #define DEVICE_LED_PIN 9
#endif
#define DEVICE_NUM_LEDS 3
CRGB device_leds[DEVICE_NUM_LEDS];

/*
 * LED Devices
 */
enum device {
  TOP_LED,
  UPPER_LED,
  LOWER_LED
};

/*
 * Delay for fastled to update the addressable LEDs.
 * 0.03 ms to update 1 LED, and this device contains 2.
 * Just setting to 3 which should be sufficient.
 */
millisDelay ms_fast_led;
const uint8_t i_fast_led_delay = 3;

/*
 * Manage the color and blink pattern for the top LED.
 */
millisDelay ms_top_blink; // Allows the top LED to blink for a menu state.
const unsigned int i_top_blink_delay = 800; // Duration for blink pattern.
uint8_t i_top_led_color; // Remember the last color for the top LED.
uint8_t i_top_led_brightness = 128; // Max brightness for this LED.
bool b_top_led_off = false; // Denotes when top LED is mid-blink.

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
#if defined(__XTENSA__)
  // ESP32
  #define BUZZER_PIN 18
  #define VIBRATION_PIN 19
  #define PWM_CHANNEL 0
#else
  // Nano
  #define BUZZER_PIN 10
  #define VIBRATION_PIN 11
#endif
millisDelay ms_buzzer;
millisDelay ms_vibrate;
bool b_buzzer_on = false;
bool b_vibrate_on = false;
const uint8_t i_min_power = 0;
const uint8_t i_max_power = 255;
const unsigned int i_buzzer_max_time = 300; // Longest duration for a standalone "beep".
const unsigned int i_vibrate_min_time = 500; // Minimum runtime for vibration motor.
const unsigned int i_vibrate_max_time = 1500; // Maximum runtime for vibration motor.

/*
 * For the alarm and venting/overheat, set the blink/buzz/vibrate interval.
 */
millisDelay ms_blink_leds;
const unsigned int i_blink_leds = 600;
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

// Define Wire object for the i2c bus.
#define WIRE Wire

// Denotes the speed of the cyclotron (1=Normal) which increases as firing continues.
uint8_t i_speed_multiplier = 1;

/*
 * Barmeter 28 segment bargraph mapping: allows accessing elements sequentially (0-27)
 * If the pattern appears inverted from what is expected, flip by using the following:
 */
//#define GPSTAR_INVERT_BARGRAPH
#ifdef GPSTAR_INVERT_BARGRAPH
  const uint8_t i_bargraph[28] = {54, 38, 22, 6, 53, 37, 21, 5, 52, 36, 20, 4, 51, 35, 19, 3, 50, 34, 18, 2, 49, 33, 17, 1, 48, 32, 16, 0};
#else
  const uint8_t i_bargraph[28] = {0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4, 20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54};
#endif

/*
 * System Mode
 */
enum ARMING_MODES { MODE_SUPERHERO, MODE_ORIGINAL };
enum ARMING_MODES ARMING_MODE;
enum RED_SWITCH_MODES { SWITCH_ON, SWITCH_OFF };
enum RED_SWITCH_MODES RED_SWITCH_MODE;

/*
 * Year Theme
 */
enum SYSTEM_YEARS { SYSTEM_1984, SYSTEM_1989, SYSTEM_AFTERLIFE, SYSTEM_FROZEN_EMPIRE };
enum SYSTEM_YEARS SYSTEM_YEAR;

/*
 * Wand Firing Modes + Settings
 */
enum BARREL_STATES { BARREL_RETRACTED, BARREL_EXTENDED };
enum BARREL_STATES BARREL_STATE;
enum FIRING_MODES { PROTON, SLIME, STASIS, MESON, SPECTRAL, HOLIDAY, SPECTRAL_CUSTOM, VENTING, SETTINGS };
enum FIRING_MODES FIRING_MODE;
enum POWER_LEVELS { LEVEL_1, LEVEL_2, LEVEL_3, LEVEL_4, LEVEL_5 };
enum POWER_LEVELS POWER_LEVEL;
enum POWER_LEVELS POWER_LEVEL_PREV;

/*
 * Toggle Switches
 * Will be pulled LOW (down position) when considered "on".
 */
#if defined(__XTENSA__)
  // ESP32
  #define LEFT_TOGGLE_PIN 34
  #define RIGHT_TOGGLE_PIN 35
  ezButton switch_left(LEFT_TOGGLE_PIN, INPUT);
  ezButton switch_right(RIGHT_TOGGLE_PIN, INPUT);
#else
  // Nano
  #define LEFT_TOGGLE_PIN 5
  #define RIGHT_TOGGLE_PIN 6
  ezButton switch_left(LEFT_TOGGLE_PIN, INPUT_PULLUP);
  ezButton switch_right(RIGHT_TOGGLE_PIN, INPUT_PULLUP);
#endif
bool b_left_toggle_on = false;
bool b_right_toggle_on = false;

/*
 * Debounce Settings
 */
const uint8_t switch_debounce_time = 50;
const uint8_t rotary_debounce_time = 100;

/*
 * Rotary encoder for various uses.
 */
#if defined(__XTENSA__)
  // ESP32
  #define r_encoderA 32
  #define r_encoderB 33
  #define r_button 4
#else
  // Nano
  #define r_encoderA 2
  #define r_encoderB 3
  #define r_button 4
#endif
ezButton encoder_center(r_button); // For center-press on encoder dial.
millisDelay ms_rotary_debounce; // Put some timing on the rotary so we do not overload the serial communication buffer.
millisDelay ms_center_double_tap; // Timer for determinine when a double-tap was detected.
millisDelay ms_center_long_press; // Timer for determining when a long press was detected.
bool b_center_pressed = false;
const unsigned int i_center_double_tap_delay = 300; // When to consider the center dial has a "double tap".
const unsigned int i_center_long_press_delay = 600; // When to consider the center dial has a "long" press.
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
* Music Track Info and Playback States
*/
const unsigned int i_music_track_offset = 500; // Music tracks always start at index 500.
unsigned int i_music_track_count = 0; // Count of tracks as returned by the pack.
unsigned int i_music_track_current = 0;
unsigned int i_music_track_min = 0; // Min value for music track index (0 = unset).
unsigned int i_music_track_max = 0; // Max value for music track index (0 = unset).
bool b_playing_music = false;
bool b_music_paused = false;

/*
 * Pack Communication
 */
#if defined(__XTENSA__)
  // ESP32 - Hardware Serial2 Pins
  #define RXD2 16
  #define TXD2 17
#endif
SerialTransfer packComs;
bool b_a_sync_start = false; // Denotes pack communications have begun.

struct __attribute__((packed)) STRUCT {
  uint16_t s;
  uint16_t i;
  uint16_t d1; // Data 1
  uint16_t d2; // Data 2
  uint16_t e;
} comStruct;

struct __attribute__((packed)) STRUCTSEND {
  uint16_t s;
  uint16_t i;
  uint16_t d1; // Data 1
  uint16_t d2; // Data 2
  uint16_t e;
} sendStruct;

/*
 * Some pack flags which get transmitted to the attenuator depending on the pack status.
 */
bool b_pack_on = false;
bool b_wand_on = false;
bool b_pack_alarm = false;
bool b_firing = false;
bool b_overheating = false;

// Forward declarations.
void attenuatorSerialSend(uint16_t i_message);
void debug(String message);