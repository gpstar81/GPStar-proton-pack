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

/*
 * Pin for Addressable LEDs.
 */
#define ATTENUATOR_LED_PIN 13
#define ATTENUATOR_NUM_LEDS 2
CRGB attenuator_leds[ATTENUATOR_NUM_LEDS];

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
bool b_buzzer_on = false;
bool b_vibrate_on = false;
const unsigned int i_buzz_max = 200; // Longest duration for a standalone "beep".
const unsigned int i_vibrate_max = 1000; // Max runtime for the vibration motor.

/*
 * Delay for fastled to update the addressable LEDs.
 * 0.03 ms to update 1 LED, and this device contains 2.
 * Just setting to 3 which should be sufficient.
 */
const uint8_t i_fast_led_delay = 3;
millisDelay ms_fast_led;

const unsigned int i_blink_leds = 550;
millisDelay ms_blink_leds;

/*
 * Barmeter 28 segment bargraph configuration and timers.
 * Part #: BL28Z-3005SA04Y
 * This will use the following pins for i2c serial communication:
 * SDA -> A4
 * SCL -> A5
 */
HT16K33 ht_bargraph;
const uint8_t i_bargraph_delay = 12; // Base delay (ms) for bargraph refresh (this should be a value evenly divisible by 2, 3, or 4).
const uint8_t i_bargraph_elements = 28; // Maximum elements for bargraph device; not likely to change but adjustable just in case.
const uint8_t i_bargraph_levels = 5; // Reflects the count of POWER_LEVELS elements (the only dependency on other device behavior).
uint8_t i_bargraph_sim_max = i_bargraph_elements; // Simulated maximum for patterns which may be dependent on other factors.
uint8_t i_bargraph_steps = i_bargraph_elements / 2; // Steps for patterns (1/2 max) which are bilateral/mirrored.
uint8_t i_bargraph_step = 0; // Indicates current step for bilateral/mirrored patterns.
bool b_bargraph_present = false; // Denotes that i2c bus found the bargraph device.
int i_bargraph_element = 0; // Indicates current LED element for adjustment.
millisDelay ms_bargraph; // Timer to control bargraph updates consistently.

// Denotes the speed of the cyclotron (1=Normal) which increases as firing continues.
uint8_t i_speed_multiplier = 1;

#define WIRE Wire

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
 * Year Theme
 */
enum YEAR_MODES { YEAR_1984, YEAR_1989, YEAR_2021 };
enum YEAR_MODES YEAR_MODE;

/*
 * Wand Firing Modes + Settings
 */
enum FIRING_MODES { PROTON, SLIME, STASIS, MESON, SPECTRAL, HOLIDAY, SPECTRAL_CUSTOM, VENTING, SETTINGS };
enum FIRING_MODES FIRING_MODE;
enum POWER_LEVELS { LEVEL_1, LEVEL_2, LEVEL_3, LEVEL_4, LEVEL_5 };
enum POWER_LEVELS POWER_LEVEL;
enum POWER_LEVELS POWER_LEVEL_PREV;

/*
 * Toggle Switches
 * Will be pulled LOW (down position) when considered "active".
 */
ezButton switch_left(5);
ezButton switch_right(6);

/*
 * Debounce Settings
 */
const uint8_t switch_debounce_time = 50;
const uint8_t rotary_debounce_time = 80;

/*
 * Rotary encoder for various uses.
 */
#define r_encoderA 2
#define r_encoderB 3
ezButton encoder_center(4); // For center-press on encoder dial.
millisDelay ms_rotary_debounce; // Put some timing on the rotary so we do not overload the serial communication buffer.
millisDelay ms_center_double_tap; // Timer for determinine when a double-tap was detected.
millisDelay ms_center_long_press; // Timer for determining when a long press was detected.
bool b_center_pressed = false;
const unsigned int i_center_double_tap_delay = 300; // When to consider the center dial has a "double tap".
const unsigned int i_center_long_press_delay = 600; // When to consider the center dial has a "long" press.
int i_press_count = 0;
int i_encoder_pos = 0;
int i_val_rotary;
int i_last_val_rotary;
int i_rotary_count = 0;

/*
 * Music Track listing count.
 */
int i_music_track_count = 0;

/*
 * Define states for the rotary dial center press.
 */
enum CENTER_STATES { NO_ACTION, SHORT_PRESS, DOUBLE_PRESS, LONG_PRESS };
enum CENTER_STATES CENTER_STATE;
enum MENU_LEVELS { MENU_1, MENU_2 };
enum MENU_LEVELS MENU_LEVEL;

/*
 * Pack Communication
 */
SerialTransfer packComs;

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
bool b_pack_alarm = false;
bool b_firing = false;
bool b_overheating = false;

/*
 * LED Devices
 */
enum device {
  UPPER_LED,
  LOWER_LED
};