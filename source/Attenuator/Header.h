/**
 *   gpstar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
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
 * Pin for Addressable LED's.
 */
#define ATTENUATOR_LED_PIN 10
#define ATTENUATOR_NUM_LEDS 2
CRGB attenuator_leds[ATTENUATOR_NUM_LEDS];

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
bool b_28segment_bargraph = false;
bool b_bargraph_up = false;
uint8_t i_bargraph_status = 0;
const uint8_t i_bargraph_interval = 4;
const uint8_t i_bargraph_wait = 180;
const uint8_t d_bargraph_ramp_interval = 40;
millisDelay ms_bargraph_alt;
millisDelay ms_bargraph;
millisDelay ms_bargraph_firing;
uint8_t i_bargraph_status_alt = 0;
const uint8_t d_bargraph_ramp_interval_alt = 40;
const uint8_t i_bargraph_multiplier_ramp_1984 = 3;
const uint8_t i_bargraph_multiplier_ramp_2021 = 16;
unsigned int i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_2021;

millisDelay ms_settings_blinking;
const unsigned int i_settings_blinking_delay = 350;
uint8_t i_speed_multiplier = 1;

#define WIRE Wire

/*
 * Barmeter 28 segment bargraph mapping: allows accessing elements sequentially (0-27)
 * If the pattern appears inverted from what is expected, flip by using the following:
 *   #define GPSTAR_INVERT_BARGRAPH
 */
#ifdef GPSTAR_INVERT_BARGRAPH
  const uint8_t i_bargraph[28] = {54, 38, 22, 6, 53, 37, 21, 5, 52, 36, 20, 4, 51, 35, 19, 3, 50, 34, 18, 2, 49, 33, 17, 1, 48, 32, 16, 0};
#else
  const uint8_t i_bargraph[28] = {0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4, 20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54};
#endif

/*
 * Year Theme
 */
unsigned int i_mode_year = 2021; // 1984, 1989, or 2021

/* 
 *  Wand Firing Modes + Settings
 */
enum FIRING_MODES { PROTON, SLIME, STASIS, MESON, SPECTRAL, HOLIDAY, SPECTRAL_CUSTOM, VENTING, SETTINGS };
enum FIRING_MODES FIRING_MODE;
enum POWER_LEVELS { LEVEL_0, LEVEL_1, LEVEL_2, LEVEL_3, LEVEL_4, LEVEL_5 };
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
int i_encoder_pos = 0;
int i_val_rotary;
int i_last_val_rotary;

/* 
 * Pack Communication
 */
SerialTransfer packComs;

struct __attribute__((packed)) STRUCT {
  int s;
  int i;
  int d1;
  int d2;
  int e;
} comStruct;

struct __attribute__((packed)) STRUCTSEND {
  int s;
  int i;
  int d1;
  int d2;
  int e;
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
