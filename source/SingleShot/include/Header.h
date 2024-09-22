/**
 *   GPStar Single-Shot Blaster
 *   Copyright (C) 2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

// Used to scan the i2c bus and to locate the 28-segment bargraph.
#define WIRE Wire

/*
 * Delay for fastled to update the addressable LEDs.
 * 0.03 ms to update 1 LED. So 1.47 ms should be okay? Let's bump it up to 3 just in case.
 */
#define FAST_LED_UPDATE_MS 3
uint8_t i_fast_led_delay = FAST_LED_UPDATE_MS; // Default delay via standard definition
millisDelay ms_fast_led; // Timer for all updates to addressable LEDs across the device

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
enum DEVICE_ACTION_STATE { ACTION_IDLE, ACTION_OFF, ACTION_ACTIVATE, ACTION_FIRING, ACTION_ERROR, ACTION_SETTINGS, ACTION_CONFIG_EEPROM_MENU };
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
enum POWER_LEVELS POWER_LEVEL;
enum POWER_LEVELS POWER_LEVEL_PREV;

/*
 * Addressable LEDs
 * The device contains 2 GPStar 7-LED jewel arrays: 1 for the barrel, 1 for the cyclotron.
 * The barrel will be the first in the addressable LED chain, while the cyclotron is last.
 * LED #1 is the "top" (near the DIN pin) while #7 is the dead center of the jewel itself.
 */
#define SYSTEM_LED_PIN 10
#define CYCLOTRON_LED_COUNT 7 // GPStar 7-LED Jewel
#define BARREL_LED_COUNT 7 // GPStar 7-LED Jewel
CRGB system_leds[CYCLOTRON_LED_COUNT + BARREL_LED_COUNT];
const uint8_t i_barrel_led = 6; // This will be the index of the light (#7), not the count
const uint8_t i_num_barrel_leds = CYCLOTRON_LED_COUNT; // This will be the number of barrel LEDs
const uint8_t i_num_cyclotron_leds = CYCLOTRON_LED_COUNT; // This will be the number of cyclotron LEDs
const uint8_t i_cyclotron_led_start = i_num_barrel_leds; // The first element (index) for the cyclotron.

/*
 * Non-addressable LEDs
 * Uses a common object to define and set expected properties for all LEDs
 */
struct StandaloneLED {
  uint8_t Pin; // Pin Assignment
  uint8_t On;  // State for "on"
  uint8_t Off; // State for "off"

  // Function to initialize the LED
  void initialize() {
    pinModeFast(Pin, OUTPUT);
    digitalWriteFast(Pin, Off);
  }

  // Function to dim the LED
  void dim(uint8_t brightness) {
    analogWrite(Pin, brightness);
  }

  // Function to get LED state/value
  uint8_t getState() {
    return digitalReadFast(Pin);
  }

  // Function to turn on the LED
  void turnOn() {
    digitalWriteFast(Pin, On);
  }

  // Function to turn off the LED
  void turnOff() {
    digitalWriteFast(Pin, Off);
  }
};
// Create instances and initialize LEDs with their pin and respective values for on/off.
StandaloneLED led_SloBlo = {8, HIGH, LOW};
StandaloneLED led_Clippard = {9, HIGH, LOW};
StandaloneLED led_TopWhite = {12, LOW, HIGH};
StandaloneLED led_Vent = {13, LOW, HIGH};
StandaloneLED led_Hat1 = {22, HIGH, LOW};
StandaloneLED led_Hat2 = {23, HIGH, LOW};
StandaloneLED led_Tip = {24, HIGH, LOW};

/*
 * Rotary encoder on the top of the device.
 * Changes the device power level and controls the device settings menu.
 * Also controls independent music volume while the device is off and if music is playing.
 */
#define r_encoderA 6
#define r_encoderB 7
enum ENCODER_STATES { ENCODER_IDLE = 0, ENCODER_CW = 1, ENCODER_CCW = -1 };
struct Encoder {
  const static uint8_t PinA = r_encoderA;
  const static uint8_t PinB = r_encoderB;

  private:
    uint8_t PrevNextCode = 0;
    uint16_t CodeStore = 0;

    int8_t read() {
      static int8_t RotEncTable[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

      PrevNextCode <<= 2;

      if(digitalReadFast(r_encoderB)) {
        PrevNextCode |= 0x02;
      }

      if(digitalReadFast(r_encoderA)) {
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

    void initialize() {
      // Rotary encoder on the top of the device.
      pinModeFast(PinA, INPUT_PULLUP);
      pinModeFast(PinB, INPUT_PULLUP);
      STATE = ENCODER_IDLE;
    }

    void check() {
      static int8_t i_last_val; // Always stored to know if change occurred.

      // Read the current encoder value, noting state when adjusted.
      if(i_last_val != read()) {
        // Clockwise.
        if(PrevNextCode == 0x07) {
          STATE = ENCODER_CW;
        }

        // Counter-clockwise.
        if(PrevNextCode == 0x0b) {
          STATE = ENCODER_CCW;
        }
      }
      else {
        STATE = ENCODER_IDLE;
      }
    }

} encoder;

/*
 * Vibration
 *
 * These are references for the EEPROM menu. Empty is a zero value, not used in the EEPROM.
 */
enum VIBRATION_MODES { VIBRATION_EMPTY, VIBRATION_ALWAYS, VIBRATION_FIRING_ONLY, VIBRATION_NONE };
enum VIBRATION_MODES VIBRATION_MODE_EEPROM;
enum VIBRATION_MODES VIBRATION_MODE;
const uint8_t vibration = 11;
const uint8_t i_vibration_level_min = 65;
uint8_t i_vibration_level = i_vibration_level_min;
uint8_t i_vibration_level_prev = 0;
millisDelay ms_menu_vibration; // Timer to do non-blocking confirmation buzzing in the vibration menu.

/*
 * Various toggles and buttons on the device.
 * Uses the Switch class which provides debounce control and detects state.
 */
Switch switch_intensify(2); // Considered a primary firing button, though for this device will be an alt-fire.
Switch switch_activate(3); // Considered the primary power toggle on the right of the gun box.
Switch switch_device(A0); // Top right switch on the device. Enables device for firing.
Switch switch_vent(4); // Bottom right switch on the device. Turns on the vent light.
Switch switch_grip(A6); // Hand-grip button to be the primary fire and used in settings menus.
uint8_t ventSwitchedCount = 0; // Used for detection of LED EEPROM menu access
uint8_t deviceSwitchedCount = 0; // Used for detection of Config EEPROM menu access

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
millisDelay ms_hat_1;
millisDelay ms_hat_2;
const uint8_t i_hat_1_delay = 100;
const uint16_t i_hat_2_delay = 400;

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
 * Device Menu
 */
enum DEVICE_MENU_LEVELS {
  MENU_LEVEL_1 = 1,
  MENU_LEVEL_2 = 2,
  MENU_LEVEL_3 = 3,
  MENU_LEVEL_4 = 4,
  MENU_LEVEL_5 = 5 };
enum DEVICE_MENU_LEVELS DEVICE_MENU_LEVEL;
enum MENU_OPTION_LEVELS {
  OPTION_1 = 1,
  OPTION_2 = 2,
  OPTION_3 = 3,
  OPTION_4 = 4,
  OPTION_5 = 5
};
enum MENU_OPTION_LEVELS MENU_OPTION_LEVEL;
uint8_t i_device_menu = 5;
const uint16_t i_settings_blinking_delay = 350;
millisDelay ms_settings_blinking;

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
millisDelay ms_power_indicator_blink;
const uint32_t i_ms_power_indicator = 60000; // 1 Minute -> 60000
const uint16_t i_ms_power_indicator_blink = 1000;

/*
 * Function prototypes.
 */
void checkDeviceAction();
void ventSwitched(void* n = nullptr);
void deviceSwitched(void* n = nullptr);
