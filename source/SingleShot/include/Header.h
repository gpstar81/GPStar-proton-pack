/**
 *   GPStar Single-Shot Blaster
 *   Copyright (C) 2024-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
 * Motion Devices.
 */
#ifdef ESP32
  #define GYRO_INT1_PIN 1
  #define GYRO_INT2_PIN 2
  #define MAG_INT_PIN 43
  #define MAG_RDY_PIN 44
#endif

/*
 * Addressable LEDs
 * The device contains 2 GPStar 7-LED jewel arrays: 1 for the barrel, 1 for the cyclotron.
 * The barrel will be the first in the addressable LED chain, while the cyclotron is last.
 * LED #1 is the "top" (near the DIN pin) while #7 is the dead center of the jewel itself.
 */
#ifdef ESP32
  // For the i2c Bus
  #define I2C_SDA 15
  #define I2C_SCL 16
  #define IMU_SCL 47
  #define IMU_SDA 48

  #define SYSTEM_LED_PIN 41
  #define TOP_LED_PIN 42 // RGB Vent light only for ESP32.
#else
  #define SYSTEM_LED_PIN 10
  #define TOP_LED_PIN 12
#endif

#define CYCLOTRON_LED_COUNT 7 // GPStar 7-LED Jewel
#define BARREL_LED_COUNT 7 // GPStar 7-LED Jewel
CRGB system_leds[CYCLOTRON_LED_COUNT + BARREL_LED_COUNT];
const uint8_t i_barrel_led = 6; // This will be the index of the light (#7), not the count
const uint8_t i_num_barrel_leds = CYCLOTRON_LED_COUNT; // This will be the number of barrel LEDs
const uint8_t i_num_cyclotron_leds = CYCLOTRON_LED_COUNT; // This will be the number of cyclotron LEDs
const uint8_t i_cyclotron_led_start = i_num_barrel_leds; // The first element (index) for the cyclotron.

/*
 * RGB vent lights.
 */
#define VENT_LEDS_MAX 2 // The maximum number of LEDs for the vent lights. Main vent + top Cliplite.
CRGB vent_leds[VENT_LEDS_MAX]; // FastLED object array for the RGB top/vent LEDs.
const uint16_t i_vent_light_update_interval = 150; // FastLED update interval specifically for the top/vent LEDs.
bool b_vent_lights_changed = false; // Check for whether there was actually a change to prevent superfluous calls to showLeds().

/*
 * Non-addressable LEDs
 * Uses a common object to define and set expected properties for all LEDs
 */
#ifdef ESP32
  #define SLO_BLO_LED_PIN 12 // SLO-BLO LED. (Red LED)
  #define CLIPPARD_LED_PIN 3 // LED underneath the Clippard valve. (Orange or White LED)
  #define BARREL_LED_PIN 41 // Data pin for the addressable LEDs in the barrel.
  #define BARREL_HAT_LED_PIN 10 // Hat light at front of the blaster near the barrel tip. (Orange LED)
  #define TOP_HAT_LED_PIN 9 // Hat light at top of the blaster body near vent. (Orange or White LED)
  #define BARREL_TIP_LED_PIN 46 // White LED at tip of the blaster barrel. (White LED)
  #define WAND_STATUS_LED_PIN 38 // V1.4 GPStar Neutrona Wand onboard LED pin.
#else
  #define SLO_BLO_LED_PIN 8 // SLO-BLO LED. (Red LED)
  #define CLIPPARD_LED_PIN 9 // LED underneath the Clippard valve. (Orange or White LED)
  #define TOP_LED_PIN 12 // Blinking white light beside the vent on top of the blaster.
  #define VENT_LED_PIN 13 // Vent light (either stock or RGB LED).
  #define BARREL_HAT_LED_PIN 22 // Hat[1] light at front of the blaster near the barrel tip. (Orange LED)
  #define TOP_HAT_LED_PIN 23 // Hat[2] light at top of the blaster body near vent. (Orange or White LED)
  #define BARREL_TIP_LED_PIN 24 // White LED at tip of the blaster barrel. (White LED)
  #define WAND_STATUS_LED_PIN 38 // V1.4 GPStar Neutrona Wand onboard LED pin.
#endif

/*
 * Device state.
 * Typically either on or off, but could be in an intermediate error state.
 */
enum DEVICE_STATE { MODE_OFF, MODE_ON, MODE_ERROR };
enum DEVICE_STATE DEVICE_STATUS;

/*
 * Various device action states.
 * Indicates a specific mode of operation as initiated by the user.
 */
enum DEVICE_ACTION_STATE { ACTION_IDLE, ACTION_OFF, ACTION_ACTIVATE, ACTION_FIRING, ACTION_ERROR, ACTION_SETTINGS };
enum DEVICE_ACTION_STATE DEVICE_ACTION_STATUS;

/*
 * Device Stream Modes + Settings
 * Stream = Type of particle stream to be thrown by the device
 */
enum STREAM_MODES { PROTON };
enum STREAM_MODES STREAM_MODE;
enum POWER_LEVELS {
  LEVEL_1 = 1,
  LEVEL_2 = 2,
  LEVEL_3 = 3,
  LEVEL_4 = 4,
  LEVEL_5 = 5
};
enum POWER_LEVELS POWER_LEVEL = LEVEL_5;
enum POWER_LEVELS POWER_LEVEL_PREV = POWER_LEVEL;

struct StandaloneLED {
  uint8_t Pin; // Pin Assignment
  uint8_t On;  // State for "on"
  uint8_t Off; // State for "off"

  // Function to initialize the LED
  void initialize() {
    pinMode(Pin, OUTPUT);
    digitalWrite(Pin, Off);
  }

  // Function to dim the LED
  void dim(uint8_t brightness) {
    analogWrite(Pin, brightness);
  }

  // Function to get LED state/value
  uint8_t getState() {
    return digitalRead(Pin);
  }

  // Function to turn on the LED
  void turnOn() {
    digitalWrite(Pin, On);
  }

  // Function to turn off the LED
  void turnOff() {
    digitalWrite(Pin, Off);
  }
};

// Create instances and initialize LEDs with their pin and respective values for on/off.
StandaloneLED led_Status = {WAND_STATUS_LED_PIN, HIGH, LOW};
StandaloneLED led_SloBlo = {SLO_BLO_LED_PIN, HIGH, LOW};
StandaloneLED led_Clippard = {CLIPPARD_LED_PIN, HIGH, LOW};
#ifndef ESP32
  StandaloneLED led_TopWhite = {TOP_LED_PIN, LOW, HIGH};
  StandaloneLED led_Vent = {VENT_LED_PIN, LOW, HIGH};
#endif
StandaloneLED led_Hat1 = {BARREL_HAT_LED_PIN, HIGH, LOW};
StandaloneLED led_Hat2 = {TOP_HAT_LED_PIN, HIGH, LOW};
StandaloneLED led_Tip = {BARREL_TIP_LED_PIN, HIGH, LOW};

/*
 * Rotary encoder on the top of the device.
 * Changes the device power level and controls the device settings menu.
 * Also controls independent music volume while the device is off and if music is playing.
 */
#ifdef ESP32
  #define ROTARY_ENCODER_A 4
  #define ROTARY_ENCODER_B 5
#else
  #define ROTARY_ENCODER_A 6
  #define ROTARY_ENCODER_B 7
#endif
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
 * Vibration
 *
 * These are references for the EEPROM menu. Empty is a zero value, not used in the EEPROM.
 */
#ifdef ESP32
  #define VIBRATION_PIN 18 // Pin for the vibration motor.
#else
  #define VIBRATION_PIN 11 // Pin for the vibration motor.
#endif
const uint8_t i_vibration_level_min = 15; // Minimum vibration level is 6%.
uint8_t i_vibration_level_current = 0; // Set the current value to 0 (off) on first start.
millisDelay ms_menu_vibration; // Timer to do non-blocking confirmation buzzing in the vibration menu.

/*
 * Various toggles and buttons on the device.
 * Uses the Switch class which provides debounce control and detects state.
 */
#ifdef ESP32
  #define INTENSIFY_SWITCH_PIN 39
  #define ACTIVATE_SWITCH_PIN 40
  #define VENT_SWITCH_PIN 45
  #define DEVICE_SWITCH_PIN 8
  #define GRIP_SWITCH_PIN 11
#else
  #define INTENSIFY_SWITCH_PIN 2
  #define ACTIVATE_SWITCH_PIN 3
  #define VENT_SWITCH_PIN 4
  #define DEVICE_SWITCH_PIN A0
  #define GRIP_SWITCH_PIN A6
#endif
Switch switch_intensify(INTENSIFY_SWITCH_PIN); // Considered a primary firing button, though for this device will be an alt-fire.
Switch switch_activate(ACTIVATE_SWITCH_PIN); // Considered the primary power toggle on the right of the gun box.
Switch switch_device(DEVICE_SWITCH_PIN); // Top right switch on the device. Enables device for firing.
Switch switch_vent(VENT_SWITCH_PIN); // Bottom right switch on the device. Turns on the vent light.
Switch switch_grip(GRIP_SWITCH_PIN); // Hand-grip button to be the primary fire and used in settings menus.

/*
 * Control for the primary blast sound effects.
 */
millisDelay ms_single_blast;
const uint16_t i_single_blast_delay_level_5 = 240;
const uint16_t i_single_blast_delay_level_4 = 260;
const uint16_t i_single_blast_delay_level_3 = 280;
const uint16_t i_single_blast_delay_level_2 = 300;
const uint16_t i_single_blast_delay_level_1 = 320;

/*
 * Idling timers
 */
millisDelay ms_white_light;
const uint16_t i_top_blink_interval = 146; // Blinking interval (ms)

/*
 * For blinking the slo-blo light when the cyclotron is not on.
 */
millisDelay ms_slo_blo_blink;
const uint16_t i_slo_blo_blink_delay = 500;

/*
 * Timers for the optional hat lights.
 * Also used for vent lights during error modes.
 */
millisDelay ms_warning_blink;
millisDelay ms_error_blink;
const uint16_t i_warning_blink_delay = 100;
const uint16_t i_error_blink_delay = 400;
const uint16_t i_bargraph_beep_delay = 1600;

/*
 * Firing timers.
 */
millisDelay ms_firing_pulse;
millisDelay ms_semi_automatic_check; // Timer used to set the rate of fire for the semi-automatic firing modes.
millisDelay ms_semi_automatic_firing; // Timer used to handle firing effect duration for the semi-automatic firing modes.
const uint16_t i_single_shot_rate = 2000; // Single shot firing rate, locking out actions after each blast.
const uint8_t i_firing_pulse = 40; // Used to drive semi-automatic firing stream effect timers.
const uint8_t i_pulse_step_max = 12; // Total number of steps per pulse animation.
uint8_t i_pulse_step = 0; // Used to keep track of which pulse animation step we are on.

/*
 * Vent LED brightness settings.
 * Non-addressable LEDs have logarithmic brightness, so we use a lookup table to make this roughly linear.
 * This is because addressable LEDs use a roughly linear brightness curve already.
 */
const uint8_t ledLookupTable[256] PROGMEM = { 0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,6,6,6,7,7,7,8,8,8,9,9,9,10,10,11,11,11,12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,19,20,20,21,21,22,23,23,24,24,25,26,26,27,28,28,29,30,30,31,32,32,33,34,35,35,36,37,38,38,39,40,41,42,42,43,44,45,46,47,47,48,49,50,51,52,53,54,55,56,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,73,74,75,76,77,78,79,80,81,82,84,85,86,87,88,89,91,92,93,94,95,97,98,99,100,102,103,104,105,107,108,109,111,112,113,115,116,117,119,120,121,123,124,126,127,128,130,131,133,134,136,137,139,140,142,143,145,146,148,149,151,152,154,155,157,158,160,162,163,165,166,168,170,171,173,175,176,178,180,181,183,185,186,188,190,192,193,195,197,199,200,202,204,206,207,209,211,213,215,217,218,220,222,224,226,228,230,232,233,235,237,239,241,243,245,247,249,251,253,255 };
const uint8_t i_vent_led_power_1 = 102;
const uint8_t i_vent_led_power_2 = 128;
const uint8_t i_vent_led_power_3 = 153;
const uint8_t i_vent_led_power_4 = 178;
const uint8_t i_vent_led_power_5 = 204;

/*
 * Device Menu
 */
enum DEVICE_MENU_LEVELS {
  MENU_LEVEL_1 = 1,
  MENU_LEVEL_2 = 2,
  MENU_LEVEL_3 = 3,
  MENU_LEVEL_4 = 4,
  MENU_LEVEL_5 = 5 };
enum DEVICE_MENU_LEVELS DEVICE_MENU_LEVEL = MENU_LEVEL_1;
enum MENU_OPTION_LEVELS {
  OPTION_1 = 1,
  OPTION_2 = 2,
  OPTION_3 = 3,
  OPTION_4 = 4,
  OPTION_5 = 5
};
enum MENU_OPTION_LEVELS MENU_OPTION_LEVEL = OPTION_5;
uint8_t i_device_menu = 5;
const uint16_t i_settings_blink_delay = 400;
millisDelay ms_settings_blink;

/*
 * Misc device settings and flags.
 */
bool b_firing = false; // Check for general firing state.
bool b_firing_intensify = false; // Check for Intensify button activity.
bool b_firing_alt = false; // Check for grip button firing activity.
bool b_firing_semi_automatic = false; // Check for semi-automatic firing modes.
bool b_sound_firing_intensify_trigger = false;
bool b_sound_firing_alt_trigger = false;
bool b_device_boot_error_on = false;

/*
 * A timer to turn on some Single-Shot Blaster lights when the system is shut down after some inactivity, as a reminder you left your power on to the system.
 */
millisDelay ms_power_indicator;
const uint32_t i_ms_power_indicator = 60000; // 1 minute -> 60000 milliseconds
const uint16_t i_ms_power_indicator_blink = 500;

/**
 * Infrared (IR) signal for the Ghost Trap or other devices (GPStar II Only).
 */
#ifdef ESP32
  #define CARRIER_KHZ 38 // Defines the standard IR carrier frequency in kHz.

  // Defines an IR command as captured from the PKE device at full power.
  const uint16_t ir_GhostInTrap[] = {
    1770, 1200, 600, 600, 600, 600, 580, 1200, 600, 600,
    580, 1200, 600, 1200, 580, 600, 580, 1200, 600
  };
#endif

/*
 * Function prototypes.
 */
void checkDeviceAction();
