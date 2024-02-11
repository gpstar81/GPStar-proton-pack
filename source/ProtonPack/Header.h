/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
 * The HasLab Power Cell has 13 LEDs.
 */
#define HASLAB_POWERCELL_LED_COUNT 13

/*
 * The Frutto Power Cell has 15 LEDs.
 */
#define FRUTTO_POWERCELL_LED_COUNT 15

/*
 * The HasLab Cyclotron Lid has 12 LEDs.
 */
#define HASLAB_CYCLOTRON_LED_COUNT 12

/*
 * The Frutto Cyclotron Lid has 20 LEDs.
 */
#define FRUTTO_CYCLOTRON_LED_COUNT 20

/*
 * Set the number of steps for the Inner Cyclotron (cake).
 */
#define INNER_CYCLOTRON_CAKE_LED_MAX 35

/*
 * Set the number of steps for the Outer Cyclotron (lid).
 */
#define OUTER_CYCLOTRON_LED_MAX 40

/*
 * The gpstar N-Filter expects 7 LEDs.
 */
#define JEWEL_NFILTER_LED_COUNT 7

 /*
 * Total number of LEDs in the standard Proton Pack configuration.
 * Power Cell and Cyclotron Lid LEDs + optional N-Filter NeoPixel.
 * 25 LEDs in the stock HasLab kit. 13 in the Power Cell and 12 in the Cyclotron lid.
 * 7 additional (32 in total) for a NeoPixel jewel that you can put into the N-Filter (optional).
 * This jewel chains off Cyclotron lens #4 in the lid (top left lens).
 * Max amount of LEDs allowed: 15 for the Power Cell and 40 for the Cyclotron lid.
 */
const uint8_t i_max_pack_leds = FRUTTO_POWERCELL_LED_COUNT + OUTER_CYCLOTRON_LED_MAX;
const uint8_t i_nfilter_jewel_leds = JEWEL_NFILTER_LED_COUNT;
const uint8_t i_max_inner_cyclotron_leds = INNER_CYCLOTRON_CAKE_LED_MAX;

/*
 * Updated count of all the LEDs plus the N-Filter jewel.
 * This gets updated by the system if the wand changes the led count in the EEPROM menu system.
 */
uint8_t i_pack_num_leds = i_powercell_leds + i_cyclotron_leds + i_nfilter_jewel_leds;

/*
 * Which LED the N-Filter jewel LEDs start.
 * This gets updated by the system if the wand changes the LED count in the EEPROM menu system.
 */
uint8_t i_vent_light_start = i_powercell_leds + i_cyclotron_leds;

// The cyclotron delay in 2021 mode. This is reset by the system during bootup based on settings in the Configuration.h
unsigned int i_2021_delay = 15; // 15 for stock HasLab LEDs. Change to 10 for the Frutto Technology Cyclotron or 7 for a 40 LED NeoPixel ring.

// The middle centre LED.
uint8_t i_1984_cyclotron_leds[4] = { 1, 4, 7, 10 };

/*
 * Proton Pack Power Cell and Cyclotron lid LED pin.
 */
#define PACK_LED_PIN 53
CRGB pack_leds[i_max_pack_leds + i_nfilter_jewel_leds];

/*
 * Inner Cyclotron LEDs (optional).
 * Max number of LEDs supported = 35.
 * Maximum allowed LEDs for the Inner Cyclotron Cake is 35.
 * Uses pin 13.
 */
#define CYCLOTRON_LED_PIN 13
CRGB cyclotron_leds[i_max_inner_cyclotron_leds];

/*
 * Delay for fastled to update the addressable LEDs.
 * We have up to 90 addressable LEDs if using NeoPixel jewels in the Inner Cyclotron and N-Filter.
 * 0.03 ms to update 1 LED. So 3 ms should be okay. Let's bump it up to 6 just in case.
 */
const uint8_t i_fast_led_delay = 6;
const uint16_t i_fast_led_bounce_delay = 200;
millisDelay ms_fast_led;
millisDelay ms_fast_led_bounce;

/*
 * Power Cell LEDs control.
 */
unsigned int i_powercell_delay = i_powercell_delay_2021;
int i_powercell_led = 0;
millisDelay ms_powercell;
bool b_powercell_updating = false;
uint8_t i_powercell_multiplier = 1;

/*
 *  Cyclotron Inner Switch Panel LEDs control (optional).
 */
const uint8_t i_cyclotron_switch_led_delay_base = 150;
unsigned int i_cyclotron_switch_led_delay = i_cyclotron_switch_led_delay_base;
uint8_t i_cyclotron_sw_led = 0;
uint8_t i_cyclotron_switch_led_mulitplier = 1;
millisDelay ms_cyclotron_switch_led;

/*
 * State of the pack.
 */
enum PACK_STATES { MODE_OFF, MODE_ON };
enum PACK_STATES PACK_STATE;

/*
 * Pack action state.
 */
enum PACK_ACTION_STATES { ACTION_IDLE, ACTION_OFF, ACTION_ACTIVATE };
enum PACK_ACTION_STATES PACK_ACTION_STATE;

/*
 * Cyclotron lid LEDs control and lid detection.
 */
uint8_t i_cyclotron_led_start = i_powercell_leds; // First LED in the Cyclotron.
uint8_t i_led_cyclotron = i_cyclotron_led_start; // Current Cyclotron LED that we are lighting up.
const unsigned int i_2021_ramp_delay = 300;
const unsigned int i_2021_ramp_length = 6000;
const unsigned int i_1984_ramp_length = 3000;
const unsigned int i_2021_ramp_down_length = 10500;
const unsigned int i_1984_ramp_down_length = 2500;
unsigned int i_current_ramp_speed = i_2021_ramp_delay;
uint8_t i_cyclotron_multiplier = 1;
millisDelay ms_cyclotron_auto_speed_timer; // A timer that is active while firing only. Used to speed up the Cyclotron by small increments based on the power modes in Afterlife (2021) only.
const unsigned int i_cyclotron_auto_speed_timer_length = 4500;
bool b_2021_ramp_up = true;
bool b_2021_ramp_up_start = true;
bool b_2021_ramp_down_start = false;
bool b_2021_ramp_down = false;
bool b_reset_start_led = true;
bool b_1984_led_start = true;
rampInt r_2021_ramp;
millisDelay ms_cyclotron;
bool b_cyclotron_lid_on = true;
int i_1984_counter = 0;
bool i_cyclotron_led_on_status[OUTER_CYCLOTRON_LED_MAX] = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
rampInt ms_cyclotron_led_fade_out[OUTER_CYCLOTRON_LED_MAX] = {};
rampInt ms_cyclotron_led_fade_in[OUTER_CYCLOTRON_LED_MAX] = {};
uint8_t i_cyclotron_led_value[OUTER_CYCLOTRON_LED_MAX] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// For the Afterlife Cyclotron matrix pattern, map a location on a circle of 40 positions to a target LED (where 0 is the top-right lens).
const uint8_t i_cyclotron_12led_matrix[OUTER_CYCLOTRON_LED_MAX] = { 1, 2, 3, 0, 0, 0, 0, 0, 0, 0, 4, 5, 6, 0, 0, 0, 0, 0, 0, 0, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 0, 0, 0, 0, 0, 0, 0 };
const uint8_t i_cyclotron_20led_matrix[OUTER_CYCLOTRON_LED_MAX] = { 1, 2, 3, 4, 5, 0, 0, 0, 0, 0, 6, 7, 8, 9, 10, 0, 0, 0, 0, 0, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 16, 17, 18, 19, 20, 0, 0, 0, 0, 0 };
const uint8_t i_cyclotron_40led_matrix[OUTER_CYCLOTRON_LED_MAX] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40 };

/*
 * Inner Cyclotron NeoPixel ring ramp control.
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
 * Alarm. Used during overheating and/or ribbon cable removal.
 */
const unsigned int i_alarm_delay = 500;
bool b_alarm = false;
millisDelay ms_alarm;

/*
 * Switches
 */
ezButton switch_alarm(23); // Ribbon cable removal switch
ezButton switch_mode(25); // 1984 / 2021 mode toggle switch
ezButton switch_vibration(27); // Vibration toggle switch
ezButton switch_cyclotron_direction(29); // Newly added switch for controlling the direction of the Cyclotron lights. Not required. Defaults to clockwise.
ezButton switch_power(31); // Red power switch under the Ion Arm.
ezButton switch_smoke(37); // Switch to enable smoke effects. Not required. Defaults to off/disabled.
//bool b_neutrona_wand_barrel_extended = false; // Unused at the moment.

/*
 * WAV Trigger
 */
wavTrigger w_trig;
uint16_t i_music_count = 0;
uint16_t i_current_music_track = 0;
const int i_music_track_start = 500; // Music tracks start on file named 500_ and higher.
const int8_t i_volume_abs_min = -70; // System (absolute) minimum volume possible.
const int8_t i_volume_abs_max = 10; // System (absolute) maximum volume possible.
bool b_playing_music = false;
bool b_music_paused = false;
bool b_repeat_track = false;

/*
 * Music control and checking.
 */
const unsigned int i_music_check_delay = 2000;
const unsigned int i_music_next_track_delay = 2000;
millisDelay ms_check_music;
millisDelay ms_music_next_track;
millisDelay ms_music_status_check;

/*
 *  Volume (0 = loudest, -70 = quietest)
 */
uint8_t i_volume_master_percentage = STARTUP_VOLUME; // Master overall volume
uint8_t i_volume_effects_percentage = STARTUP_VOLUME_EFFECTS; // Sound effects
uint8_t i_volume_music_percentage = STARTUP_VOLUME_MUSIC; // Music volume

int8_t i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100); // Master overall volume
int8_t i_volume_master_eeprom = i_volume_master; // Master overall volume that is saved into the eeprom menu and loaded during bootup.
int8_t i_volume_effects = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_effects_percentage / 100); // Sound effects
int8_t i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100); // Music volume
int8_t i_volume_revert = i_volume_master;

millisDelay ms_volume_check; // Put some timing on the master volume gain to not overload the WAV Trigger serial communication.

/*
 * Vibration motor settings
 *
 * Vibration default is based on the toggle switch position. These are references for the EEPROM menu. Empty is a zero value, not used in the EEPROM.
 */ 
enum VIBRATION_MODES_EEPROM { VIBRATION_EMPTY, VIBRATION_ALWAYS, VIBRATION_FIRING_ONLY, VIBRATION_NONE, VIBRATION_DEFAULT };
enum VIBRATION_MODES_EEPROM VIBRATION_MODE_EEPROM;
const uint8_t vibration = 45;
int i_vibration_level = 0;
int i_vibration_level_prev = 0;
const int i_vibration_idle_level_2021 = 60;
const int i_vibration_idle_level_1984 = 35;
const int i_vibration_lowest_level = 15;

/*
 * Enable or disable vibration control for the Proton Pack.
 * Vibration is toggled on and off the toggle switch in the Proton Pack.
 */
bool b_vibration_enabled = true;

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
 * It is not needed but it was requested by some people who may want to use the smoke booster for another purpose.
 */
const uint8_t fan_booster_pin = 38;

/*
 * Fan for N-Filter smoke
 */
const uint8_t fan_pin = 33;

/*
 * Control for the Meson Shock Blast sound effects.
*/
millisDelay ms_meson_blast;
const unsigned int i_meson_blast_delay_level_5 = 140;
const unsigned int i_meson_blast_delay_level_4 = 160;
const unsigned int i_meson_blast_delay_level_3 = 180;
const unsigned int i_meson_blast_delay_level_2 = 200;
const unsigned int i_meson_blast_delay_level_1 = 220;

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
bool b_smoke_continuous_mode[5] = { b_smoke_continuous_mode_1, b_smoke_continuous_mode_2, b_smoke_continuous_mode_3, b_smoke_continuous_mode_4, b_smoke_continuous_mode_5 };
const bool b_smoke_overheat_mode[5] = { b_smoke_overheat_mode_1, b_smoke_overheat_mode_2, b_smoke_overheat_mode_3, b_smoke_overheat_mode_4, b_smoke_overheat_mode_5 };
millisDelay ms_overheating_length; // The total length of the when the fans turn on (or smoke if smoke synced to fan)
const unsigned int i_overheat_delay_increment = 1000; // Used to increment the overheat delays by 1000 milliseconds.
const unsigned int i_overheat_delay_max = 60000; // The max length a overheat can be.

/*
 * N-Filter LED (White) (Optional)
 * Use a White LED with a Forward voltage of 3.0-3.2 with up to a 20ma current draw.
 */
const uint8_t i_nfilter_led_pin = 46;

/*
 * Vent light timers and delay for overheating.
 */
millisDelay ms_vent_light_on;
millisDelay ms_vent_light_off;
const uint8_t i_vent_light_delay = 50;
bool b_vent_sounds; // A flag for playing smoke and vent sounds.
bool b_vent_light_on = false; // To know if the light is on or off.

/*
 * Wand Firing Modes + Settings
 */
enum FIRING_MODES { PROTON, SLIME, STASIS, MESON, SPECTRAL, HOLIDAY, SPECTRAL_CUSTOM, VENTING, SETTINGS };
enum FIRING_MODES FIRING_MODE;

/*
 * System modes.
 * Super Hero: A idealised system based on the close up of the Super Hero Proton Pack and Neutrona Wand in the 1984 Rooftop closeup scene and what is shown in Afterlife. (Different toggle switch sequences for turning on the pack and wand)
 * Original: Based on the original operational manual during production of GB1. (Wand toggle switches must be on before the cyclotron can turn on from the Wand only.)
 * Super Hero will be the default system mode.
 */
enum SYSTEM_MODES { MODE_SUPER_HERO, MODE_ORIGINAL };
enum SYSTEM_MODES SYSTEM_MODE;

/*
 * Cross The Streams Status
 */
enum STATUS_CROSS_THE_STREAMS { CTS_FIRING_1984, CTS_FIRING_2021, CTS_NOT_FIRING };
enum STATUS_CROSS_THE_STREAMS STATUS_CTS;

/*
 * Wand Status
 */
bool b_wand_firing = false;
bool b_firing_alt = false;
bool b_firing_intensify = false;
bool b_sound_firing_intensify_trigger = false;
bool b_sound_firing_alt_trigger = false;
bool b_wand_connected = false;
bool b_wand_syncing = false;
bool b_wand_on = false;
const uint8_t i_wand_power_level_max = 5; // Max power level of the wand.
uint8_t i_wand_power_level = 1; // Power level of the wand.
millisDelay ms_wand_check; // Timer used to determine whether the wand has been disconnected.
const unsigned int i_wand_disconnect_delay = 8000; // Time until the pack considers a wand as disconnected.

/*
 * Serial1 Status
 */
bool b_serial1_connected = false;
bool b_serial1_syncing = false;
millisDelay ms_serial1_handshake;
const unsigned int i_serial1_handshake_delay = 4000;
millisDelay ms_serial1_handshake_checking;

/*
 * Define Serial Communication Buffers
 */
SerialTransfer serial1Coms;
SerialTransfer packComs;

/*
 * Firing timers
 */
millisDelay ms_firing_length_timer;
const unsigned int i_firing_timer_length = 15000; // 15 seconds. Used by ms_firing_length_timer to determine which tail_end sound effects to play.
millisDelay ms_firing_sound_mix; // Used to play misc sound effects during firing.
int i_last_firing_effect_mix = 0;
millisDelay ms_idle_fire_fade; // Used for fading the Afterlife idling sound with firing

/*
 * Rotary encoder for volume control
 */
#define encoder_pin_a 2
#define encoder_pin_b 3
int i_encoder_pos = 0;
int i_val_rotary;
int i_last_val_rotary;

/*
 * LED Dimming / Brightness Control.
 */
enum pack_led_dim_control {
  DIM_POWERCELL,
  DIM_CYCLOTRON,
  DIM_INNER_CYCLOTRON
};

uint8_t pack_dim_toggle = DIM_POWERCELL;

/*
 * LED Devices.
 */
enum device {
  POWERCELL,
  CYCLOTRON_OUTER,
  CYCLOTRON_INNER,
  VENT_LIGHT
};

/*
 * The year the Proton Pack operates in.
 * SYSTEM_EMPTY is just a empty place holder. We need this as we write this data to the EEPROM.
 */
enum SYSTEM_YEARS { SYSTEM_EMPTY, SYSTEM_TOGGLE_SWITCH, SYSTEM_1984, SYSTEM_1989, SYSTEM_AFTERLIFE, SYSTEM_FROZEN_EMPIRE };
enum SYSTEM_YEARS SYSTEM_YEAR;
enum SYSTEM_YEARS SYSTEM_YEAR_TEMP;
enum SYSTEM_YEARS SYSTEM_EEPROM_YEAR;

/*
 * Misc.
 */
bool b_switch_mode_override = false; // Year mode override flag controlled by the Neutrona Wand. This resets when you flip the mode year toggle switch on the pack.
bool b_pack_on = false;
bool b_pack_shutting_down = false;
bool b_spectral_lights_on = false;
bool b_fade_out = false;
millisDelay ms_fadeout;

/*
 * Voltage reference.
 */
uint16_t i_batt_volts; // Current voltage value (Vcc) using internal bandgap reference.
const unsigned int i_ms_battcheck_delay = 5000; // Time between battery voltage checks.
millisDelay ms_battcheck; // Timer for checking battery voltage on a regular interval.

/*
 * Function prototypes.
 */
void packSerialSend(uint8_t i_command, uint16_t i_value);
void packSerialSend(uint8_t i_command);
void packSerialSendData(uint8_t i_message);
void serial1Send(uint8_t i_command, uint16_t i_value);
void serial1Send(uint8_t i_command);
void serial1SendData(uint8_t i_message);
void checkSerial1();
void checkWand();
void playEffect(int i_track_id, bool b_track_loop = false, int8_t i_track_volume = i_volume_effects, bool b_fade_in = false, unsigned int i_fade_time = 0);
void stopEffect(int i_track_id);
void playMusic();
void stopMusic();
void adjustGainEffect(int i_track_id, int8_t i_track_volume = i_volume_effects, bool b_fade = false, unsigned int i_fade_time = 0);
void powercellDraw(uint8_t i_start = 0);

/*
 * If you are compiling this for an Arduino Mega and the error message brings you here, go to the bottom of the Configuration.h file for more information.
 */
#ifdef GPSTAR_PROTON_PACK_PCB
  ezButton switch_cyclotron_lid(43); // Second Cyclotron ground pin (brown) that we detect if the lid is removed or not.
#else
  ezButton switch_cyclotron_lid(51); // Second Cyclotron ground pin (brown) that we detect if the lid is removed or not.
#endif