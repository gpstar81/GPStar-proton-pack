/**
 *   gpstar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
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
 * Wand state. 
 */
enum WAND_STATE { MODE_OFF, MODE_ON };
enum WAND_STATE WAND_STATUS;

/*
 * Various wand action states.
 */
enum WAND_ACTION_STATE { ACTION_IDLE, ACTION_OFF, ACTION_ACTIVATE, ACTION_FIRING, ACTION_OVERHEATING, ACTION_SETTINGS };
enum WAND_ACTION_STATE WAND_ACTION_STATUS;

/* 
 *  Barrel LEDs. There are 5 LEDs. 0 = Base, 4 = tip. These are addressable with a single pin and are RGB.
 */
#define BARREL_LED_PIN 10
#define BARREL_NUM_LEDS 5
CRGB barrel_leds[BARREL_NUM_LEDS];

/*
 * Delay for fastled to update the addressable LEDs. 
 * We have up to 5 addressable LEDs in the wand barrel.
 * 0.03 ms to update 1 LED. So 0.15 ms should be ok? Lets bump it up to 3 just in case.
 */
const int i_fast_led_delay = 3;
millisDelay ms_fast_led;

/* 
 *  Wav trigger
 */
wavTrigger w_trig;
uint8_t i_music_count = 0;
uint8_t i_current_music_track = 0;
const uint8_t i_music_track_start = 100; // Music tracks start on file named 100_ and higher.

/*
 *  Volume (0 = loudest, -70 = quietest)
*/
int i_volume_percentage = STARTUP_VOLUME_EFFECTS; // Sound effects
int i_volume_master_percentage = STARTUP_VOLUME; // Master overall volume
int i_volume_music_percentage = STARTUP_VOLUME_MUSIC; // Music volume
int i_volume = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_percentage / 100); // Sound effects
int i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100); // Master overall volume
int i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100); // Music volume

/* 
 * Rotary encoder on the top of the wand. Changes the wand power level and controls the wand settings menu.
 * Also controls independent music volume while the pack/wand is off and if music is playing.
 */
#define r_encoderA 6
#define r_encoderB 7
static uint8_t prev_next_code = 0;
static uint16_t store = 0;

/* 
 *  Vibration
 */
const uint8_t vibration = 11;
int i_vibration_level = 55;
int i_vibration_level_prev = 0;
bool b_vibration_on = false;

/* 
 *  Various Switches on the wand.
 */
ezButton switch_wand(A0); // Contols the beeping. Top right switch on the wand.
ezButton switch_intensify(2);
ezButton switch_activate(3);
ezButton switch_vent(4); // Turns on the vent light.
const int switch_mode = A6; // Changes firing modes or to reach the settings menu.
const int switch_barrel = A7; // Barrel extension/open switch.

/*
 * Some switch settings.
 */
const uint8_t switch_debounce_time = 50;
const uint8_t a_switch_debounce_time = 250;
millisDelay ms_switch_mode_debounce;
millisDelay ms_intensify_timer;
const int i_intensify_delay = 400;
const int i_switch_mode_value = 200;
const int i_switch_barrel_value = 100;

/*
 * Wand lights
 */
const uint8_t led_white = 12; // Blinking white light beside the vent on top of the wand.
const uint8_t led_vent = 13; // Vent light
const uint8_t led_bargraph_1 = A1;
const uint8_t led_bargraph_2 = A2;
const uint8_t led_bargraph_3 = A3;
const uint8_t led_bargraph_4 = A4;
const uint8_t led_bargraph_5 = A5;

/* 
 *  Idling timers
 */
millisDelay ms_gun_loop_1;
millisDelay ms_gun_loop_2;
millisDelay ms_white_light;
const int d_white_light_interval = 150;

/* 
 *  Overheat timers
 */
millisDelay ms_overheat_initiate;
millisDelay ms_overheating;
const int i_ms_overheating = 6500; // Overheating for 6.5 seconds.
const bool b_overheat_mode[5] = { b_overheat_mode_1, b_overheat_mode_2, b_overheat_mode_3, b_overheat_mode_4, b_overheat_mode_5 };
const long int i_ms_overheat_initiate[5] = { i_ms_overheat_initiate_mode_1, i_ms_overheat_initiate_mode_2, i_ms_overheat_initiate_mode_3, i_ms_overheat_initiate_mode_4, i_ms_overheat_initiate_mode_5 };

/* 
 *  Stock Hasbro Bargraph timers
 */
millisDelay ms_bargraph;
millisDelay ms_bargraph_firing;
const int d_bargraph_ramp_interval = 120;
unsigned int i_bargraph_status = 0;

/*
 * (Optional) Barmeter 28 segment bargraph configuration and timers.
 * Part #: BL28Z-3005SA04Y
*/
HT16K33 ht_bargraph;
const int i_bargraph_interval = 4;
const int i_bargraph_wait = 180;
millisDelay ms_bargraph_alt;
bool b_bargraph_up = false;
unsigned int i_bargraph_status_alt = 0;
const int d_bargraph_ramp_interval_alt = 40;
const int i_bargraph_multiplier_ramp_1984 = 3;
const int i_bargraph_multiplier_ramp_2021 = 16;
int i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_2021;

/*
 * (Optional) Barmeter 28 segment bargraph mapping.
 * Part #: BL28Z-3005SA04Y
*/
const uint8_t i_bargraph[28] = {0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4, 20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54};

/* 
 *  A timer for controlling the wand beep. in 2021 mode.
 */
millisDelay ms_reset_sound_beep;
const int i_sound_timer = 50;

/* 
 *  Wand tip heatup timers (when changing firing modes).
 */
millisDelay ms_wand_heatup_fade;
const int i_delay_heatup = 10;
int i_heatup_counter = 0;
int i_heatdown_counter = 100;

/* 
 *  Firing timers.
 */
millisDelay ms_firing_lights;
millisDelay ms_firing_lights_end;
millisDelay ms_firing_stream_blue;
millisDelay ms_firing_stream_orange;
millisDelay ms_impact; // Mix some impact sounds while firing.
millisDelay ms_firing_start_sound_delay;
millisDelay ms_firing_stop_sound_delay;
const int d_firing_lights = 20; // 20 milliseconds. Timer for adjusting the firing stream colours.
const int d_firing_stream = 100; // 100 milliseconds. Used by the firing timers to adjust stream colours.
int i_barrel_light = 0; // using this to keep track which LED in the barrel is currently lighting up.
const int i_fire_start_sound_delay = 50; // Delay for starting firing sounds.
const int i_fire_stop_sound_delay = 100; // Delay for stopping fire sounds.

/* 
 *  Wand power mode. Controlled by the rotary encoder on the top of the wand.
 *  You can enable or disable overheating for each mode individually in the user adjustable values at the top of this file.
 */
const int i_power_mode_max = 5;
const int i_power_mode_min = 1;
int i_power_mode = 1;
int i_power_mode_prev = 1;

/* 
 *  Wand / Pack communication
 */
int rx_byte = 0;

/*
 * Some pack flags which get transmitted to the wand depending on the pack status.
 */
bool b_pack_on = false;
bool b_pack_alarm = false;
bool b_wait_for_pack = true;
bool b_volume_sync_wait = false;
int i_cyclotron_speed_up = 1; // For telling the pack to speed up or slow down the cyclotron lights.

/*
 * Volume sync status with the pack.
 */
enum VOLUME_SYNC { EFFECTS, MASTER, MUSIC };
enum VOLUME_SYNC VOLUME_SYNC_WAIT;

/* 
 *  Wand menu & music
 */
int i_wand_menu = 5;
bool b_wand_menu_sub = false;
const int i_settings_blinking_delay = 350;
bool b_playing_music = false;
bool b_repeat_track = false;
const int i_music_check_delay = 2000;
const int i_music_next_track_delay = 2000;
millisDelay ms_settings_blinking;
millisDelay ms_check_music;
millisDelay ms_music_next_track;

/* 
 *  Wand firing modes + settings
 *  Proton, Slime, Stasis, Meson, Settings
 */
enum FIRING_MODES { PROTON, SLIME, STASIS, MESON, SETTINGS };
enum FIRING_MODES FIRING_MODE;
enum FIRING_MODES PREV_FIRING_MODE;

/*
 * Misc wand settings and flags.
 */
int year_mode = 2021;
bool b_firing = false;
bool b_firing_intensify = false;
bool b_firing_alt = false;
bool b_firing_cross_streams = false;
bool b_sound_firing_intensify_trigger = false;
bool b_sound_firing_alt_trigger = false;
bool b_sound_firing_cross_the_streams = false;

bool b_sound_idle = false;
bool b_beeping = false;