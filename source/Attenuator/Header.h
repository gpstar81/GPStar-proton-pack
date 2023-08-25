/**
 *   gpstar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gmail.com>
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
 */
const uint8_t i_fast_led_delay = 3;
millisDelay ms_fast_led;

/*
 * Barmeter 28 segment bargraph configuration and timers.
 * Part #: BL28Z-3005SA04Y
 * This will use the following pins for i2c serial communication:
 * SDA -> A4
 * SCL -> A5
 */
// HT16K33 ht_bargraph;
const uint8_t i_bargraph_interval = 4;
const uint8_t i_bargraph_wait = 180;
bool b_28segment_bargraph = false;
bool b_bargraph_up = false;
millisDelay ms_bargraph;
uint8_t i_bargraph_status = 0;
const uint8_t d_bargraph_ramp_interval = 40;
const uint8_t i_bargraph_multiplier_ramp_1984 = 3;
const uint8_t i_bargraph_multiplier_ramp_2021 = 16;
unsigned int i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_2021;

// Used to scan the i2c bus and to locate the 28 segment bargraph.
// #define WIRE Wire

/*
 * Barmeter 28 segment bargraph mapping.
 */
#ifdef GPSTAR_INVERT_BARGRAPH
  const uint8_t i_bargraph[28] = {54, 38, 22, 6, 53, 37, 21, 5, 52, 36, 20, 4, 51, 35, 19, 3, 50, 34, 18, 2, 49, 33, 17, 1, 48, 32, 16, 0};
#else
  const uint8_t i_bargraph[28] = {0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4, 20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54};
#endif

/* 
 *  Switches
 */
ezButton switch_left(3);
ezButton switch_right(4);

/*
 * Switch Settings.
 */
const uint8_t switch_debounce_time = 50;

/* 
 * Rotary encoder for various uses.
 */
#define r_encoderA 6
#define r_encoderB 7

/* 
 *  Pack Communication
 */
SerialTransfer packComs;

struct __attribute__((packed)) STRUCT {
  int s;
  int i;
  int e;
} comStruct;

struct __attribute__((packed)) STRUCTSEND {
  int s;
  int i;
  int e;
} sendStruct;

/*
 * LED Devices.
 */
enum device {
  UPPER,
  LOWER
};
