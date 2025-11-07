/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
 * Pin for Addressable LEDs
 */
#define BUILT_IN_LED 2
#define DEVICE_LED_PIN 23
#define DEVICE_NUM_LEDS 3 // States there are 3 LEDs: Top, Upper, and Lower
CRGB device_leds[DEVICE_NUM_LEDS];

/*
 * LED Device Ordering - Top, Upper, and Lower
 * Creates a simple byte array of N elements for the ID of each of the 3 LEDs.
 * Due to space constraints, users may have had to install the LEDs in reverse.
 * Therefore, the order of this list may change depending on user preference.
 * This feature will only be available for the ESP32-based controller.
 */
bool b_invert_leds = false; // Denotes whether the order should be reversed.
bool b_grb_leds = false; // Denotes whether to use GRB ordering for LEDs.
uint8_t i_device_led[DEVICE_NUM_LEDS] = {0, 1, 2}; // Default Order

/*
 * LED Animation Options
 */
enum LED_ANIMATION : uint8_t {
  AMBER_PULSE = 0,
  ORANGE_FADE = 1,
  RED_FADE = 2
};
enum LED_ANIMATION RAD_LENS_IDLE = AMBER_PULSE;

/*
 * Flag to indicate serial comms have been established after bootup.
 */
bool b_comms_open = false;

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
 * Manage the colour and blink pattern for the top LED.
 */
millisDelay ms_top_blink; // Allows the top LED to blink for a menu state.
const uint16_t i_top_blink_delay = 800; // Duration for blink pattern.
uint8_t i_top_led_colour; // Remember the last colour for the top LED.
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
#define BUZZER_PIN 18
#define VIBRATION_PIN 19
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
uint8_t i_cyclotron_multiplier = 1;

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
 * System Mode
 */
enum SYSTEM_MODES { MODE_SUPER_HERO, MODE_ORIGINAL };
enum SYSTEM_MODES SYSTEM_MODE = MODE_SUPER_HERO;
enum RED_SWITCH_MODES { SWITCH_ON, SWITCH_OFF };
enum RED_SWITCH_MODES RED_SWITCH_MODE = SWITCH_OFF;

/*
 * Year Theme
 */
enum SYSTEM_YEARS { SYSTEM_EMPTY, SYSTEM_TOGGLE_SWITCH, SYSTEM_1984, SYSTEM_1989, SYSTEM_AFTERLIFE, SYSTEM_FROZEN_EMPIRE };
enum SYSTEM_YEARS SYSTEM_YEAR = SYSTEM_AFTERLIFE;

/*
 * Wand Firing Modes + Settings
 */
enum BARREL_STATES { BARREL_RETRACTED, BARREL_EXTENDED };
enum BARREL_STATES BARREL_STATE;
enum POWER_LEVELS { LEVEL_1, LEVEL_2, LEVEL_3, LEVEL_4, LEVEL_5 };
enum POWER_LEVELS POWER_LEVEL = LEVEL_5;
enum POWER_LEVELS POWER_LEVEL_PREV = LEVEL_5;
enum STREAM_MODES { UNSET_STREAM, PROTON, STASIS, SLIME, MESON, SPECTRAL, HOLIDAY_HALLOWEEN, HOLIDAY_CHRISTMAS, SPECTRAL_CUSTOM, SETTINGS };
enum STREAM_MODES STREAM_MODE = PROTON;
enum STREAM_MODE_FLAGS : uint8_t { FLAG_NONE = 0, FLAG_VG = 1, FLAG_SPECTRAL = 2, FLAG_SPECTRAL_CUSTOM = 4, FLAG_HOLIDAY_HALLOWEEN = 8, FLAG_HOLIDAY_CHRISTMAS = 16 };
uint8_t STREAM_MODE_FLAG = FLAG_VG; // By default, only enable the three VG modes.
millisDelay ms_streamchange; // Debounce for change of stream via dial.
uint16_t i_stream_change_delay = 500; // Delay between stream mode changes.

/*
 * Toggle Switches
 * Will be pulled LOW (down position) when considered "on".
 */
#define LEFT_TOGGLE_PIN 34
#define RIGHT_TOGGLE_PIN 35
ezButton switch_left(LEFT_TOGGLE_PIN, EXTERNAL_PULLUP);
ezButton switch_right(RIGHT_TOGGLE_PIN, EXTERNAL_PULLUP);
// Provide a known default at startup for switches.
bool b_left_toggle_on = false;
bool b_right_toggle_on = false;
bool b_right_toggle_center_start = false;

/*
 * Debounce Settings
 */
const uint8_t switch_debounce_time = 50;

/*
 * Rotary encoder for various uses.
 */
#define ROTARY_ENCODER_A 33
#define ROTARY_ENCODER_B 32
#define ROTARY_SWITCH 4
ezButton encoder_center(ROTARY_SWITCH); // For center-press on encoder dial.
enum ENCODER_STATES : int8_t { ENCODER_IDLE = 0, ENCODER_CW = 1, ENCODER_CCW = -1 };

/*
 * Simple class for the rotary encoder events.
 */
struct Encoder {
  const static uint8_t PinA = ROTARY_ENCODER_A;
  const static uint8_t PinB = ROTARY_ENCODER_B;

  private:
    uint8_t PrevNextCode = 0;
    uint16_t CodeStore = 0;
    int8_t i_last_val = 0; // Use a small integer as value range is 1 to -1 depending on direction.
    bool b_direction_inverted = false; // Invert the direction of rotation to match user expectation.

    int8_t read() {
      const static int8_t RotEncTable[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

      PrevNextCode <<= 2;

      if(digitalRead(PinB)) {
        PrevNextCode |= 0x02;
      }

      if(digitalRead(PinA)) {
        PrevNextCode |= 0x01;
      }

      PrevNextCode &= 0x0f;

      // If valid then CodeStore as 16 bit data.
      if(RotEncTable[PrevNextCode]) {
        CodeStore <<= 4;
        CodeStore |= PrevNextCode;

        if((CodeStore & 0xff) == 0x2b) {
          return -1;
        }

        if((CodeStore & 0xff) == 0x17) {
          return 1;
        }
      }

      return 0;
    }

  public:
    enum ENCODER_STATES STATE;

    // Consume the current transient STATE and clear it so the caller receives the event once.
    ENCODER_STATES consumeState() {
      ENCODER_STATES s = STATE;
      STATE = ENCODER_IDLE;
      return s;
    }

    void initialize(bool inverted = false) {
      // Rotary encoder on the top of the device.
      pinMode(PinA, INPUT_PULLUP);
      pinMode(PinB, INPUT_PULLUP);
      STATE = ENCODER_IDLE;
      b_direction_inverted = inverted;
    }

    // Runtime getter for dial direction (false = default, true = inverted).
    bool isRotationInverted() { return b_direction_inverted; }

    // Runtime setter to invert direction.
    void setRotationInverted(bool invert) { b_direction_inverted = invert; }

    void check() {
      // Read the current encoder value, noting state when adjusted.
      int8_t i_new_val = read();

      // Default to idle which ensures STATE is always assigned.
      STATE = ENCODER_IDLE;

      // Change state only if there was a recognized change.
      if(i_last_val != i_new_val) {
        i_last_val = i_new_val; // Update stored last value so next call can detect changes.

        // Map terminal PrevNextCode to CW/CCW, invert if requested.
        if(PrevNextCode == 0x07) {
          STATE = b_direction_inverted ? ENCODER_CCW : ENCODER_CW;
        } else if(PrevNextCode == 0x0b) {
          STATE = b_direction_inverted ? ENCODER_CW : ENCODER_CCW;
        }
      }
    }
} encoder;

/*
 * Rotary dial switch.
 */
millisDelay ms_center_double_tap; // Timer for determinine when a double-tap was detected.
millisDelay ms_center_long_press; // Timer for determining when a long press was detected.
bool b_center_pressed = false;
bool b_center_lockout = false;
const uint16_t i_center_double_tap_delay = 300; // When to consider the center dial has a "double tap".
const uint16_t i_center_long_press_delay = 600; // When to consider the center dial has a "long" press.
uint8_t i_press_count = 0;
uint8_t i_rotary_count = 0;

/*
 * Define states for the rotary dial center press or rotation.
 */
enum CENTER_STATES { NO_ACTION, SHORT_PRESS, DOUBLE_PRESS, LONG_PRESS };
enum CENTER_STATES CENTER_STATE;
enum MENU_LEVELS { MENU_1, MENU_2, MENU_STREAM };
enum MENU_LEVELS MENU_LEVEL = MENU_1;

/*
 * Music Track Info and Playback States
 */
const uint16_t i_music_track_offset = 500; // Music tracks always start at index 500.
uint16_t i_music_track_count = 0; // Count of tracks as returned by the pack.
uint16_t i_current_music_track = 0;
uint16_t i_music_track_min = 0; // Min value for music track index (0 = unset).
uint16_t i_music_track_max = 0; // Max value for music track index (0 = unset).
uint16_t i_pack_audio_version = 0; // Type/Version of Proton Pack audio board (0 = no audio).
uint16_t i_wand_audio_version = 0; // Type/Version of Neutrona Wand audio board (0 = no audio).
uint8_t i_volume_master_percentage = 100; // Master overall volume
uint8_t i_volume_effects_percentage = 100; // Sound effects
uint8_t i_volume_music_percentage = 100; // Music volume
bool b_master_muted = false;
bool b_playing_music = false;
bool b_music_paused = false;
bool b_repeat_track = false;
String s_track_listing = "";

/*
 * Some pack flags which get transmitted to the attenuator depending on the pack status.
 */
bool b_esp32_pack = false; // Used by the A_SYNC_START for immediate identification.
bool b_pack_on = false;
bool b_pack_shutting_down = false;
bool b_wand_connected = false;
bool b_wand_on = false;
bool b_pack_alarm = false;
bool b_wand_firing = false;
bool b_overheating = false;
bool b_smoke_enabled = false;
bool b_vibration_switch_on = false;
bool b_clockwise = false;

// Flags relating to the synchronization process.
millisDelay ms_packsync;
const uint16_t i_sync_initial_delay = 750; // Delay to re-try the initial handshake with a proton pack.
const uint16_t i_sync_disconnect_delay = 8000; // Delay before we consider the pack missing.

// Flags for denoting when requested data was received.
bool b_received_prefs_pack = false;
bool b_received_prefs_wand = false;
bool b_received_prefs_smoke = false;

// Pack Battery (V) and Wand Power (A) Values
float f_batt_volts = 0.0;
float f_wand_amps = 0.0;

// Pack Temperature Values
float f_temperature_c = 0;
float f_temperature_f = 0;
