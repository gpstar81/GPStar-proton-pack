/**
 *   gpstar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
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

/* 
 * Wand state. 
 */
enum WAND_STATE { MODE_OFF, MODE_ON, MODE_ERROR };
enum WAND_STATE WAND_STATUS;

/*
 * Various wand action states.
 */
enum WAND_ACTION_STATE { ACTION_IDLE, ACTION_OFF, ACTION_ACTIVATE, ACTION_FIRING, ACTION_OVERHEATING, ACTION_SETTINGS, ACTION_ERROR, ACTION_EEPROM_MENU, ACTION_CONFIG_EEPROM_MENU };
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
const uint8_t i_fast_led_delay = 3;
millisDelay ms_fast_led;

/* 
 *  Wav trigger
 */
wavTrigger w_trig;
unsigned int i_music_count = 0;
unsigned int i_current_music_track = 0;
const int i_music_track_start = 500; // Music tracks start on file named 500_ and higher.
const int8_t i_volume_abs_min = -70; // System (absolute) minimum volume possible.
const int8_t i_volume_abs_max = 10; // System (absolute) maximum volume possible.

/*
 *  Volume (0 = loudest, -70 = quietest)
*/
uint8_t i_volume_percentage = STARTUP_VOLUME_EFFECTS; // Sound effects
uint8_t i_volume_master_percentage = STARTUP_VOLUME; // Master overall volume
uint8_t i_volume_music_percentage = STARTUP_VOLUME_MUSIC; // Music volume
int8_t i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100); // Master overall volume
int8_t i_volume_effects = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_percentage / 100); // Sound effects
int8_t i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100); // Music volume
int8_t i_volume_revert = i_volume_master;
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
const uint8_t i_vibration_level_min = 65;
uint8_t i_vibration_level = i_vibration_level_min;
uint8_t i_vibration_level_prev = 0;

/*
 * Enable or disable vibration control for the Neutrona wand.
 * When set to false, there will be no vibration enabled for the Neutrona wand. 
 * This is toggled by the proton pack vibration toggle switch.
*/
bool b_vibration_enabled = true;

/* 
 *  Various Switches on the wand.
 */
ezButton switch_wand(A0); // Contols the beeping. Top right switch on the wand.
ezButton switch_intensify(2);
ezButton switch_activate(3);
ezButton switch_vent(4); // Turns on the vent light.
const int switch_mode = A6; // Changes firing modes or to reach the settings menu.
const int switch_barrel = A7; // Barrel extension/open switch.
bool b_switch_mode_pressed = false;
bool b_switch_barrel_extended = false;

/*
 * Some switch settings.
 */
const uint8_t switch_debounce_time = 50;
millisDelay ms_switch_mode_debounce;
millisDelay ms_intensify_timer;
const unsigned int i_intensify_delay = 400;
const uint8_t i_switch_mode_value = 200;
const uint8_t i_switch_barrel_value = 100;

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
const uint8_t d_white_light_interval = 250;

/* 
 *  Overheat timers
 */
millisDelay ms_overheat_initiate;
millisDelay ms_overheating;
const unsigned int i_ms_overheating = 6500; // Overheating for 6.5 seconds.
const bool b_overheat_mode[5] = { b_overheat_mode_1, b_overheat_mode_2, b_overheat_mode_3, b_overheat_mode_4, b_overheat_mode_5 };
const unsigned long int i_ms_overheat_initiate[5] = { i_ms_overheat_initiate_mode_1, i_ms_overheat_initiate_mode_2, i_ms_overheat_initiate_mode_3, i_ms_overheat_initiate_mode_4, i_ms_overheat_initiate_mode_5 };

/* 
 *  Stock Hasbro Bargraph timers
 */
millisDelay ms_bargraph;
millisDelay ms_bargraph_firing;
const uint8_t d_bargraph_ramp_interval = 120;
uint8_t i_bargraph_status = 0;

/*
 * (Optional) Barmeter 28 segment bargraph configuration and timers.
 * Part #: BL28Z-3005SA04Y
  * Only supported by the gpstar Neutrona Wand micro controller.
*/
#ifdef GPSTAR_NEUTRONA_WAND_PCB
  HT16K33 ht_bargraph;
  
  // Used to scan the i2c bus and to locate the 28 segment bargraph.
  #define WIRE Wire
#endif

/*
 * Set to true if you are replacing the stock Hasbro bargraph with a Barmeter 28 segment bargraph.
 * Set to false if you are using the stock Hasbro bargraph.
 * Part #: BL28Z-3005SA04Y
 * Only compatible with the gpstar Neutrona Wand board, and not a Arduino Nano.
*/
bool b_28segment_bargraph = false;

#ifdef GPSTAR_NEUTRONA_WAND_PCB
  const uint8_t i_bargraph_interval = 4;
  const uint8_t i_bargraph_wait = 180;
  bool b_bargraph_up = false;
  millisDelay ms_bargraph_alt;
  uint8_t i_bargraph_status_alt = 0;
  const uint8_t d_bargraph_ramp_interval_alt = 40;
  const uint8_t i_bargraph_multiplier_ramp_1984 = 3;
  const uint8_t i_bargraph_multiplier_ramp_2021 = 16;
  unsigned int i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_2021;
#endif

/*
 * (Optional) Barmeter 28 segment bargraph mapping.
 * Part #: BL28Z-3005SA04Y
 * Only supported by the gpstar Neutrona Wand micro controller.
*/
#ifdef GPSTAR_NEUTRONA_WAND_PCB
  #ifdef GPSTAR_INVERT_BARGRAPH
    const uint8_t i_bargraph[28] = {54, 38, 22, 6, 53, 37, 21, 5, 52, 36, 20, 4, 51, 35, 19, 3, 50, 34, 18, 2, 49, 33, 17, 1, 48, 32, 16, 0};
  #else
    const uint8_t i_bargraph[28] = {0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4, 20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54};
  #endif
#endif

/*
 * (Optional) Support for the Frutto Technology Video Game Accessories
*/
/*
#ifdef GPSTAR_NEUTRONA_WAND_PCB
  bool b_overheat_indicators[13] = {false, false, false, false, false, false, false, false, false, false, false, false, false};
#endif
*/

#ifdef GPSTAR_NEUTRONA_WAND_PCB
  /*
  * EEPROM
  */
  unsigned int i_eepromAddress = 0; // The address in the EEPROM to start reading from.
  unsigned long l_crc_size = ~0L; // The 4 last bytes are reserved for storing the CRC.

  /*
  * EEPROM data structure object that is saved into the EEPROM memory of the Neutrona Wand.
  */
  struct objEEPROM {
    uint8_t cross_the_streams;
    uint8_t cross_the_streams_mix;
    uint8_t overheating;
    uint8_t neutrona_wand_sounds;
    uint8_t spectral_mode;
    uint8_t holiday_mode;
  };

  /*
  * EEPROM Another data structure object that is saved into the EEPROM memory.
  */
  struct objLEDEEPROM {
    uint8_t barrel_spectral_custom;
    uint8_t barrel_spectral_saturation_custom;
  };  
#endif

/*
 * Timers for the optional hat lights.
 * Also used for vent lights during error modes.
*/
millisDelay ms_hat_1;
millisDelay ms_hat_2;
const uint8_t i_hat_1_delay = 100;
const unsigned int i_hat_2_delay = 400;

/* 
 *  A timer for controlling the wand beep. in 2021 mode.
 */
millisDelay ms_reset_sound_beep;
const uint8_t i_sound_timer = 50;

/* 
 *  Wand tip heatup timers (when changing firing modes).
 */
millisDelay ms_wand_heatup_fade;
const uint8_t i_delay_heatup = 10;
uint8_t i_heatup_counter = 0;
uint8_t i_heatdown_counter = 100;

/* 
 *  Wand Firing Modes + Settings
 */
enum FIRING_MODES { PROTON, SLIME, STASIS, MESON, SPECTRAL, HOLIDAY, SPECTRAL_CUSTOM, VENTING, SETTINGS };
enum FIRING_MODES FIRING_MODE;
enum FIRING_MODES PREV_FIRING_MODE;

/* 
 *  Firing timers.
 */
millisDelay ms_firing_lights;
millisDelay ms_firing_lights_end;
millisDelay ms_firing_stream_blue;
millisDelay ms_firing_stream_orange;

#ifdef GPSTAR_NEUTRONA_WAND_PCB
  millisDelay ms_impact; // Mix some impact sounds while firing.
#endif

millisDelay ms_firing_start_sound_delay;
millisDelay ms_firing_stop_sound_delay;
const uint8_t d_firing_lights = 20; // 20 milliseconds. Timer for adjusting the firing stream colours.
const uint8_t d_firing_stream = 100; // 100 milliseconds. Used by the firing timers to adjust stream colours.
uint8_t i_barrel_light = 0; // using this to keep track which LED in the barrel is currently lighting up.
const uint8_t i_fire_start_sound_delay = 50; // Delay for starting firing sounds.
const uint8_t i_fire_stop_sound_delay = 100; // Delay for stopping fire sounds.

/* 
 *  Wand power mode. Controlled by the rotary encoder on the top of the wand.
 *  You can enable or disable overheating for each mode individually in the user adjustable values at the top of this file.
 */
const uint8_t i_power_mode_max = 5;
const uint8_t i_power_mode_min = 1;
uint8_t i_power_mode = 1;
uint8_t i_power_mode_prev = 1;

/* 
 *  Wand / Pack communication
 */
SerialTransfer wandComs;

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
 * Some pack flags which get transmitted to the wand depending on the pack status.
 */
bool b_pack_on = false;
bool b_pack_alarm = false;
bool b_wait_for_pack = true;
bool b_volume_sync_wait = false;
bool b_sync = false;
uint8_t i_cyclotron_speed_up = 1; // For telling the pack to speed up or slow down the cyclotron lights.

/*
 * Volume sync status with the pack.
 */
enum VOLUME_SYNC { EFFECTS, MASTER, MUSIC, SILENT };
enum VOLUME_SYNC VOLUME_SYNC_WAIT;

/* 
 *  Wand menu & music
 */
uint8_t i_wand_menu = 5;
bool b_wand_menu_sub = false;
const unsigned int i_settings_blinking_delay = 350;
bool b_playing_music = false;
bool b_repeat_track = false;
millisDelay ms_settings_blinking;

/*
 * Misc wand settings and flags.
 */
unsigned int year_mode = 2021;
bool b_firing = false;
bool b_firing_intensify = false;
bool b_firing_alt = false;
bool b_firing_cross_streams = false;
bool b_sound_firing_intensify_trigger = false;
bool b_sound_firing_alt_trigger = false;
bool b_sound_firing_cross_the_streams = false;
bool b_sound_idle = false;
bool b_beeping = false;
bool b_sound_afterlife_idle_2_fade = true;
bool b_pack_ribbon_cable_on = true;

/*
 * Set to true to have your bargraph blink on/off when the Neutrona Wand and Proton Pack overheat.
 * When false, the bargraph will ramp down instead.
 * Removing this feature eventually....
*/
bool b_overheat_bargraph_blink = false;

/*
 * Function prototypes.
*/
void playEffect(int i_track_id, bool b_track_loop = false, int8_t i_track_volume = i_volume_effects, bool b_fade_in = false, unsigned int i_fade_time = 0);
void stopEffect(int i_track_id);
void stopMusic();
void playMusic();

#ifdef GPSTAR_NEUTRONA_WAND_PCB
  const uint8_t led_slo_blo = 8;
  const uint8_t led_front_left = 9;
  const uint8_t led_hat_1 = 22; // Hat light at front of the wand near the barrel tip. (Red LED)
  const uint8_t led_hat_2 = 23; // Hat light at top of the wand body. (Red LED)
  const uint8_t led_barrel_tip = 24; // White led at tip of the wand barrel. (White LED).
#else
  const uint8_t led_slo_blo = 5; // There are 2 LED's attached to this pin when using a Arduino Nano. The slo-blo LED and the orange light on the front of the wand body (front_lef).
#endif