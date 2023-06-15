/**
 *   gpstar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
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
 *  PowerCell and Cyclotron Lid LEDs + optional n_filter NeoPixel.
 *  7 additional (32 in total in stock form) for a NeoPixel jewel that you can put into the n-filter (optional). 
 *  This jewel chains off cyclotron lens #4 in the lid (top left lens).
 */
#define PACK_NUM_LEDS i_pack_num_leds + 7
#define VENT_LIGHT_START i_pack_num_leds

/*
 * Proton pack powercell and cyclotron lid led pin.
*/
#define PACK_LED_PIN 53
CRGB pack_leds[PACK_NUM_LEDS];

/*
 * Inner Cyclotron LEDs. (optional).
 * Uses pin 13.
 */
#define CYCLOTRON_LED_PIN 13
CRGB cyclotron_leds[CYCLOTRON_NUM_LEDS];

/*
 * Delay for fastled to update the addressable LEDs. 
 * We have up to 88 addressable LEDs if using NeoPixel jewels in the inner cyclotron and n-filter.
 * 0.03 ms to update 1 LED. So 3 ms should be ok. Lets bump it up to 6 just in case.
 */
const uint8_t i_fast_led_delay = 6;
millisDelay ms_fast_led;

/*
 * Powercell LEDs control.
 */
const uint8_t i_powercell_delay_1984 = 75;
const uint8_t i_powercell_delay_2021 = 40;
unsigned int i_powercell_delay = i_powercell_delay_2021;
int i_powercell_led = 0;
millisDelay ms_powercell;

/* 
 *  Cyclotron Inner Switch Panel LEDs control. (optional)
 */
const uint8_t i_cyclotron_switch_led_delay_base = 150;
unsigned int i_cyclotron_switch_led_delay = i_cyclotron_switch_led_delay_base;
uint8_t i_cyclotron_sw_led = 0;
uint8_t i_cyclotron_switch_led_mulitplier = 1;
millisDelay ms_cyclotron_switch_led;

/* 
 *  State of the pack.
 */
enum PACK_STATE { MODE_OFF, MODE_ON };
enum PACK_STATE PACK_STATUS;

/*
 * Pack action state.
 */
enum PACK_ACTION_STATE { ACTION_IDLE, ACTION_OFF, ACTION_ACTIVATE };
enum PACK_ACTION_STATE PACK_ACTION_STATUS;

/*
 * Cyclotron lid LEDs control and lid detection.
 */
const uint8_t cyclotron_led_start = 13; // First LED in the cyclotron.
uint8_t i_led_cyclotron = cyclotron_led_start; // Current cyclotron LED that we are lighting up.
const unsigned int i_2021_ramp_delay = 300;
const unsigned int i_2021_ramp_length = 6000;
const unsigned int i_1984_ramp_length = 3000;
const unsigned int i_2021_ramp_down_length = 10500;
const unsigned int i_1984_ramp_down_length = 2500;
unsigned int i_current_ramp_speed = i_2021_ramp_delay;
uint8_t i_cyclotron_multiplier = 1;
bool b_2021_ramp_up = true;
bool b_2021_ramp_up_start = true;
bool b_2021_ramp_down_start = false;
bool b_2021_ramp_down = false;
bool b_reset_start_led = true;
bool b_1984_led_start = true;
rampInt r_2021_ramp;
millisDelay ms_cyclotron;
bool b_cyclotron_lid_on = true;
rampInt ms_cyclotron_fade_out_led_1;
rampInt ms_cyclotron_fade_out_led_2; 
rampInt ms_cyclotron_fade_out_led_3;
rampInt ms_cyclotron_fade_out_led_4;
rampInt ms_cyclotron_fade_out_led_5;
rampInt ms_cyclotron_fade_out_led_6;
rampInt ms_cyclotron_fade_out_led_7;
rampInt ms_cyclotron_fade_out_led_8;
rampInt ms_cyclotron_fade_out_led_9;
rampInt ms_cyclotron_fade_out_led_10;
rampInt ms_cyclotron_fade_out_led_11;
rampInt ms_cyclotron_fade_out_led_12;
rampInt ms_cyclotron_fade_out_led_13;
rampInt ms_cyclotron_fade_out_led_14;
rampInt ms_cyclotron_fade_out_led_15;
rampInt ms_cyclotron_fade_out_led_16;
rampInt ms_cyclotron_fade_out_led_17;
rampInt ms_cyclotron_fade_out_led_18;
rampInt ms_cyclotron_fade_out_led_19;
rampInt ms_cyclotron_fade_out_led_20;
rampInt ms_cyclotron_fade_out_led_21;
rampInt ms_cyclotron_fade_out_led_22;
rampInt ms_cyclotron_fade_out_led_23;
rampInt ms_cyclotron_fade_out_led_24;
rampInt ms_cyclotron_fade_out_led_25;
rampInt ms_cyclotron_fade_out_led_26;
rampInt ms_cyclotron_fade_out_led_27;
rampInt ms_cyclotron_fade_out_led_28;
rampInt ms_cyclotron_fade_out_led_29;
rampInt ms_cyclotron_fade_out_led_30;
rampInt ms_cyclotron_fade_out_led_31;
rampInt ms_cyclotron_fade_out_led_32;
rampInt ms_cyclotron_fade_out_led_33;
rampInt ms_cyclotron_fade_out_led_34;
rampInt ms_cyclotron_fade_out_led_35;
rampInt ms_cyclotron_fade_out_led_36;
rampInt ms_cyclotron_fade_out_led_37;
rampInt ms_cyclotron_fade_out_led_38;
rampInt ms_cyclotron_fade_out_led_39;
rampInt ms_cyclotron_fade_out_led_40;
rampInt ms_cyclotron_fade_in_led_1;
rampInt ms_cyclotron_fade_in_led_2;
rampInt ms_cyclotron_fade_in_led_3;
rampInt ms_cyclotron_fade_in_led_4;
rampInt ms_cyclotron_fade_in_led_5;
rampInt ms_cyclotron_fade_in_led_6;
rampInt ms_cyclotron_fade_in_led_7;
rampInt ms_cyclotron_fade_in_led_8;
rampInt ms_cyclotron_fade_in_led_9;
rampInt ms_cyclotron_fade_in_led_10;
rampInt ms_cyclotron_fade_in_led_11;
rampInt ms_cyclotron_fade_in_led_12;
rampInt ms_cyclotron_fade_in_led_13;
rampInt ms_cyclotron_fade_in_led_14;
rampInt ms_cyclotron_fade_in_led_15;
rampInt ms_cyclotron_fade_in_led_16;
rampInt ms_cyclotron_fade_in_led_17;
rampInt ms_cyclotron_fade_in_led_18;
rampInt ms_cyclotron_fade_in_led_19;
rampInt ms_cyclotron_fade_in_led_20;
rampInt ms_cyclotron_fade_in_led_21;
rampInt ms_cyclotron_fade_in_led_22;
rampInt ms_cyclotron_fade_in_led_23;
rampInt ms_cyclotron_fade_in_led_24;
rampInt ms_cyclotron_fade_in_led_25;
rampInt ms_cyclotron_fade_in_led_26;
rampInt ms_cyclotron_fade_in_led_27;
rampInt ms_cyclotron_fade_in_led_28;
rampInt ms_cyclotron_fade_in_led_29;
rampInt ms_cyclotron_fade_in_led_30;
rampInt ms_cyclotron_fade_in_led_31;
rampInt ms_cyclotron_fade_in_led_32;
rampInt ms_cyclotron_fade_in_led_33;
rampInt ms_cyclotron_fade_in_led_34;
rampInt ms_cyclotron_fade_in_led_35;
rampInt ms_cyclotron_fade_in_led_36;
rampInt ms_cyclotron_fade_in_led_37;
rampInt ms_cyclotron_fade_in_led_38;
rampInt ms_cyclotron_fade_in_led_39;
rampInt ms_cyclotron_fade_in_led_40;
bool i_cyclotron_led_on_status[40] = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
rampInt ms_cyclotron_led_fade_out[40] = { ms_cyclotron_fade_out_led_1, ms_cyclotron_fade_out_led_2, ms_cyclotron_fade_out_led_3, ms_cyclotron_fade_out_led_4, ms_cyclotron_fade_out_led_5, ms_cyclotron_fade_out_led_6, ms_cyclotron_fade_out_led_7, ms_cyclotron_fade_out_led_8, ms_cyclotron_fade_out_led_9, ms_cyclotron_fade_out_led_10, ms_cyclotron_fade_out_led_11, ms_cyclotron_fade_out_led_12, ms_cyclotron_fade_out_led_13, ms_cyclotron_fade_out_led_14, ms_cyclotron_fade_out_led_15, ms_cyclotron_fade_out_led_16, ms_cyclotron_fade_out_led_17, ms_cyclotron_fade_out_led_18, ms_cyclotron_fade_out_led_19, ms_cyclotron_fade_out_led_20, ms_cyclotron_fade_out_led_21, ms_cyclotron_fade_out_led_22, ms_cyclotron_fade_out_led_23, ms_cyclotron_fade_out_led_24, ms_cyclotron_fade_out_led_25, ms_cyclotron_fade_out_led_26, ms_cyclotron_fade_out_led_27, ms_cyclotron_fade_out_led_28, ms_cyclotron_fade_out_led_29, ms_cyclotron_fade_out_led_30, ms_cyclotron_fade_out_led_31, ms_cyclotron_fade_out_led_32, ms_cyclotron_fade_out_led_33, ms_cyclotron_fade_out_led_34, ms_cyclotron_fade_out_led_35, ms_cyclotron_fade_out_led_36, ms_cyclotron_fade_out_led_37, ms_cyclotron_fade_out_led_38, ms_cyclotron_fade_out_led_39, ms_cyclotron_fade_out_led_40 };
rampInt ms_cyclotron_led_fade_in[40] = { ms_cyclotron_fade_in_led_1, ms_cyclotron_fade_in_led_2, ms_cyclotron_fade_in_led_3, ms_cyclotron_fade_in_led_4, ms_cyclotron_fade_in_led_5, ms_cyclotron_fade_in_led_6, ms_cyclotron_fade_in_led_7, ms_cyclotron_fade_in_led_8, ms_cyclotron_fade_in_led_9, ms_cyclotron_fade_in_led_10, ms_cyclotron_fade_in_led_11, ms_cyclotron_fade_in_led_12, ms_cyclotron_fade_in_led_13, ms_cyclotron_fade_in_led_14, ms_cyclotron_fade_in_led_15, ms_cyclotron_fade_in_led_16, ms_cyclotron_fade_in_led_17, ms_cyclotron_fade_in_led_18, ms_cyclotron_fade_in_led_19, ms_cyclotron_fade_in_led_20, ms_cyclotron_fade_in_led_21, ms_cyclotron_fade_in_led_22, ms_cyclotron_fade_in_led_23, ms_cyclotron_fade_in_led_24, ms_cyclotron_fade_in_led_25, ms_cyclotron_fade_in_led_26, ms_cyclotron_fade_in_led_27, ms_cyclotron_fade_in_led_28, ms_cyclotron_fade_in_led_29, ms_cyclotron_fade_in_led_30, ms_cyclotron_fade_in_led_31, ms_cyclotron_fade_in_led_32, ms_cyclotron_fade_in_led_33, ms_cyclotron_fade_in_led_34, ms_cyclotron_fade_in_led_35, ms_cyclotron_fade_in_led_36, ms_cyclotron_fade_in_led_37, ms_cyclotron_fade_in_led_38, ms_cyclotron_fade_in_led_39, ms_cyclotron_fade_in_led_40 };
uint8_t i_cyclotron_led_value[40] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int i_1984_counter = 0;

/*
 * Inner cyclotron NeoPixel ring ramp control.
 * This is for the 35 LED NeoPixel ring option.
 */
millisDelay ms_cyclotron_ring;
rampInt r_inner_ramp;
const unsigned int i_inner_delay = i_2021_inner_delay;
const unsigned int i_inner_ramp_delay = 300;
int i_led_cyclotron_ring = 0;
bool b_inner_ramp_up = true;
bool b_inner_ramp_down = false;
unsigned int i_inner_current_ramp_speed = i_inner_ramp_delay;

/*
 * Cyclotron Switch Plate LEDs
 */
const uint8_t cyclotron_sw_plate_led_r1 = 4;
const uint8_t cyclotron_sw_plate_led_r2 = 5;
const uint8_t cyclotron_sw_plate_led_y1 = 6;
const uint8_t cyclotron_sw_plate_led_y2 = 7;
const uint8_t cyclotron_sw_plate_led_g1 = 8;
const uint8_t cyclotron_sw_plate_led_g2 = 9;
const uint8_t cyclotron_switch_led_green = 10; // 1984/2021 mode switch led.
const uint8_t cyclotron_switch_led_yellow = 11; // Vibration on/off switch led.
millisDelay ms_cyclotron_switch_plate_leds;
const unsigned int i_cyclotron_switch_plate_leds_delay = 1000;

/* 
 * Alarm. Used during overheating and or ribbon cable removal.
 */
const unsigned int i_alarm_delay = 500;
bool b_alarm = false;
millisDelay ms_alarm;

/* 
 *  Switches
 */
ezButton switch_alarm(23); // Ribbon cable removal switch
ezButton switch_mode(25); // 1984 / 2021 mode toggle switch
ezButton switch_vibration(27); // Vibration toggle switch
ezButton switch_cyclotron_direction(29); // Newly added switch for controlling the direction of the cyclotron lights. Not required. Defaults to clockwise.
ezButton switch_power(31); // Red power switch under the ion arm.
ezButton switch_smoke(37); // Switch to enable smoke effects. Not required. Defaults to off/disabled.

/* 
 *  Wav trigger
 */
wavTrigger w_trig;
unsigned int i_music_count = 0;
unsigned int i_current_music_track = 0;
const int i_music_track_start = 500; // Music tracks start on file named 500_ and higher.
bool b_playing_music = false;
bool b_repeat_track = false;

/* 
 *  Volume (0 = loudest, -70 = quietest)
 */
uint8_t i_volume_percentage = STARTUP_VOLUME_EFFECTS; // Sound effects
uint8_t i_volume_master_percentage = STARTUP_VOLUME; // Master overall volume
uint8_t i_volume_music_percentage = STARTUP_VOLUME_MUSIC; // Music volume

unsigned int i_volume = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_percentage / 100); // Sound effects
unsigned int i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100); // Master overall volume
unsigned int i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100); // Music volume

millisDelay ms_volume_check; // Put some timing on the master volume gain to not overload the wav trigger serial communication.

/*
 * Vibration motor settings
 */
const uint8_t vibration = 45;
int i_vibration_level = 0;
int i_vibration_level_prev = 0;
bool b_vibration = false;

/*
 * Smoke
 */
const uint8_t smoke_pin = 39;

/*
 * Smoke for a second smoke machine or motor. I use this in the booster tube.
 */
const uint8_t smoke_booster_pin = 35;

/*
 * Fan for the smoke booster tube. 
 * This will go off at the same time the smoke booster pin.
 * It is not needed but it was requested by some people, who may want to use the smoke booster for another purpose.
*/
const uint8_t fan_booster_pin = 50;

/*
 * Fan for n-filter smoke
 */
const uint8_t fan_pin = 33;
millisDelay ms_fan_stop_timer;
const unsigned int i_fan_stop_timer = 7000;

/* 
 * Overheating and smoke timers for smoke_pin. 
 */
millisDelay ms_overheating;
const unsigned int i_overheating_delay = 4000;
bool b_overheating = false;
millisDelay ms_smoke_timer;
millisDelay ms_smoke_on;
const unsigned long int i_smoke_timer[5] = { i_smoke_timer_mode_1, i_smoke_timer_mode_2, i_smoke_timer_mode_3, i_smoke_timer_mode_4, i_smoke_timer_mode_5 };
const unsigned long int i_smoke_on_time[5] = { i_smoke_on_time_mode_1, i_smoke_on_time_mode_2, i_smoke_on_time_mode_3, i_smoke_on_time_mode_4, i_smoke_on_time_mode_5 };
const bool b_smoke_continuous_mode[5] = { b_smoke_continuous_mode_1, b_smoke_continuous_mode_2, b_smoke_continuous_mode_3, b_smoke_continuous_mode_4, b_smoke_continuous_mode_5 };
const bool b_smoke_overheat_mode[5] = { b_smoke_overheat_mode_1, b_smoke_overheat_mode_2, b_smoke_overheat_mode_3, b_smoke_overheat_mode_4, b_smoke_overheat_mode_5 };

/*
 * N-Filter LED (White) (Optional)
 * Use a White LED with a Forward voltage of 3.0-3.2 with up to a 20ma current draw.
*/
const uint8_t i_nfilter_led_pin = 46;

/*
 * Vent light timers and delay for over heating.
 */
millisDelay ms_vent_light_on;
millisDelay ms_vent_light_off;
const uint8_t i_vent_light_delay = 50;
bool b_vent_sounds; // A flag for playing smoke and vent sounds.
bool b_vent_light_on = false; // To know if the light is on or off.

/* 
 *  Wand communication
 */
enum FIRING_MODES { PROTON, SLIME, STASIS, MESON, SETTINGS };
enum FIRING_MODES FIRING_MODE;
bool b_wand_firing = false;
bool b_firing_alt = false;
bool b_firing_intensify = false;
bool b_firing_cross_streams = false;
bool b_sound_firing_intensify_trigger = false;
bool b_sound_firing_alt_trigger = false;
bool b_wand_connected = false;
bool b_wand_on = false;
millisDelay ms_wand_handshake;
const unsigned int i_wand_handshake_delay = 3000;
millisDelay ms_wand_handshake_checking;
uint8_t i_wand_power_level = 1; // Power level of the wand.
const uint8_t i_wand_power_level_max = 5; // Max power level of the wand.

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
 * Firing timers
 */
millisDelay ms_firing_length_timer;
const unsigned int i_firing_timer_length = 15000; // 15 seconds. Used by ms_firing_length_timer to determine which tail_end sound effects to play.

/* 
 * Rotary encoder for volume control 
 */
#define encoder_pin_a 2
#define encoder_pin_b 3
int i_encoder_pos = 0;
int i_val_rotary;
int i_last_val_rotary;

/*
 * Misc.
 */
unsigned int i_mode_year = 2021; // 1984, 1989 or 2021
unsigned int i_mode_year_tmp = 2021; // Controlled by the Neutrona wand.
bool b_switch_mode_override = false; // Year mode override flag controlled by the Neutrona wand. This resets when you flip the mode year toggle switch on the pack.
bool b_pack_on = false;
bool b_pack_shutting_down = false;

/*
 * If you are compiling this for a Arduino Mega and the error message brings you here, go to the bottom of the Configuration.h file for more information.
*/
#ifdef GPSTAR_PROTON_PACK_PCB
  ezButton switch_cyclotron_lid(43); // Second cyclotron ground pin (brown) that we detect if the lid is removed or not.
#else
  ezButton switch_cyclotron_lid(51); // Second cyclotron ground pin (brown) that we detect if the lid is removed or not.
#endif