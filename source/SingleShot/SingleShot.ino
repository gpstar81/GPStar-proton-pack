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

#if defined(__AVR_ATmega2560__)
  #define GPSTAR_NEUTRONA_DEVICE_PCB
#endif

// Set to 1 to enable built-in debug messages
#define DEBUG 0

// Debug macros
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

// PROGMEM macro
#define PROGMEM_READU32(x) pgm_read_dword_near(&(x))
#define PROGMEM_READU16(x) pgm_read_word_near(&(x))
#define PROGMEM_READU8(x) pgm_read_byte_near(&(x))

// 3rd-Party Libraries
#include <CRC32.h>
#include <digitalWriteFast.h>
#include <EEPROM.h>
#include <millisDelay.h>
#include <FastLED.h>
#include <avdweb_Switch.h>
#include <ht16k33.h>
#include <Wire.h>

// Local Files
#include "Configuration.h"
#include "MusicSounds.h"
#include "Header.h"
#include "Colours.h"
#include "Bargraph.h"
#include "Cyclotron.h"
#include "Audio.h"
#include "Preferences.h"

void setup() {
  Serial.begin(9600); // Standard serial (USB) console.

  // Setup the audio device for this controller.
  setupAudioDevice();

  // Change PWM frequency of pin 3 and 11 for the vibration motor, we do not want it high pitched.
  TCCR2B = (TCCR2B & B11111000) | (B00000110); // for PWM frequency of 122.55 Hz

  // System LEDs
  FastLED.addLeds<NEOPIXEL, SYSTEM_LED_PIN>(system_leds, CYCLOTRON_LED_COUNT + BARREL_LED_COUNT);

  // Setup default system settings.
  VIBRATION_MODE_EEPROM = VIBRATION_NONE; // VIBRATION_ALWAYS
  DEVICE_MENU_LEVEL = MENU_LEVEL_1;
  MENU_OPTION_LEVEL = OPTION_5;
  POWER_LEVEL = LEVEL_1;

  // Set callback events for these toggles, which need to count the activations for EEPROM menu entry.
  switch_vent.setPushedCallback(&ventSwitched); // For the LED EEPROM Menu
  switch_device.setPushedCallback(&deviceSwitched); // For the Config EEPROM Menu

  // Rotary encoder on the top of the device.
  encoder.initialize();

  // Setup the bargraph after a brief delay.
  delay(10);
  setupBargraph();

  // Initialize all non-addressable LEDs
  led_SloBlo.initialize();
  led_Clippard.initialize();
  led_TopWhite.initialize();
  led_Vent.initialize();
  led_Hat1.initialize();
  led_Hat2.initialize();
  led_Tip.initialize();

  pinMode(vibration, OUTPUT); // Vibration motor is PWM, so fallback to default pinMode just to be safe.

  // Make sure lights are off, including the bargraph.
  allLightsOff();

  // Device status.
  DEVICE_STATUS = MODE_OFF;
  DEVICE_ACTION_STATUS = ACTION_IDLE;

  // We bootup the device in the classic proton mode.
  STREAM_MODE = PROTON;

  // Load any saved settings stored in the EEPROM memory of the GPStar Single-Shot Blaster.
  if(b_eeprom) {
    readEEPROM();
  }

  // Start the button mash check timer.
  ms_bmash.start(0);

  // Start up some timers for MODE_ORIGINAL.
  ms_slo_blo_blink.start(i_slo_blo_blink_delay);

  // Initialize the fastLED state update timer.
  ms_fast_led.start(i_fast_led_delay);

  // Check music timer for bench test mode only.
  ms_check_music.start(i_music_check_delay);

  // Reset our master volume manually.
  resetMasterVolume();

  // System Power On Self Test
  systemPOST();
}

void loop() {
  updateAudio(); // Update the state of the selected sound board.

  checkMusic(); // Music control is here since in standalone mode.

  switchLoops(); // Poll for switch/button changes via user inputs.

  mainLoop(); // Continue on to the main loop for taking actions.
}

void systemPOST() {
  uint8_t i_delay = 100;

  // Play a sound to test the audio system.
  playEffect(S_DEVICE_READY);

  // Turn on all bargraph elements and force an update
  bargraph.reset();
  bargraph.full();
  bargraph.commit();

  // These go HIGH to turn on.
  led_SloBlo.turnOn();
  delay(i_delay);
  led_Clippard.turnOn();
  delay(i_delay);
  led_Hat2.turnOn();
  delay(i_delay);

  // These go LOW to turn on.
  led_Vent.turnOn();
  delay(i_delay);
  led_TopWhite.turnOn();
  delay(i_delay);

  led_Tip.turnOn();
  delay(i_delay);

  // Sequentially turn on all LEDs in the cyclotron.
  for(uint8_t i = 0; i < i_num_cyclotron_leds; i++) {
    system_leds[i] = getHueAsRGB(C_RED);
    FastLED.show();
    delay(i_delay);
  }

  // Turn on the front barrel.
  system_leds[i_barrel_led] = getHueAsRGB(C_WHITE);
  FastLED.show();

  delay(i_delay * 8);

  allLightsOff(); // Turn off all lights, including the bargraph.

  // Make sure change to bargraph is immediate.
  bargraph.commit();
}

bool increasePowerLevel() {
  bool b_changed = true;

  switch(POWER_LEVEL){
    case LEVEL_1:
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_2;
    break;
    case LEVEL_2:
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_3;
    break;
    case LEVEL_3:
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_4;
    break;
    case LEVEL_4:
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_5;
    break;
    case LEVEL_5:
      // No change, at highest level.
      b_changed = false;
    break;
  }

  if(b_changed) {
    playEffect(S_BEEPS);
  }

  // Returns true if value was changed.
  return b_changed;
}

bool decreasePowerLevel() {
  bool b_changed = true;

  switch(POWER_LEVEL){
    case LEVEL_1:
      // No change, at lowest level.
      b_changed = false;
    break;
    case LEVEL_2:
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_1;
    break;
    case LEVEL_3:
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_2;
    break;
    case LEVEL_4:
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_3;
    break;
    case LEVEL_5:
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_4;
    break;
  }

  if(b_changed) {
    playEffect(S_BEEPS);
  }

  // Returns true if value was changed.
  return b_changed;
}

// Increasing the menu level means the user is going deeper
bool lowerMenuLevel() {
  bool b_changed = true;

  switch(DEVICE_MENU_LEVEL){
    case MENU_LEVEL_1:
      if(DEVICE_STATUS == MODE_ON && DEVICE_ACTION_STATUS == ACTION_SETTINGS) {
        // Do not advance past level 1 for the settings menu when on.
        b_changed = false;
      }
      else {
        DEVICE_MENU_LEVEL = MENU_LEVEL_2;
        led_SloBlo.turnOn(); // Level 2
        led_Vent.turnOff(); // Level 3
        led_TopWhite.turnOff(); // Level 4
        led_Clippard.turnOff(); // Level 5

        stopEffect(S_BEEPS);
        playEffect(S_BEEPS);

        stopEffect(S_VOICE_LEVEL_1);
        playEffect(S_VOICE_LEVEL_2);
        stopEffect(S_VOICE_LEVEL_3);
        stopEffect(S_VOICE_LEVEL_4);
        stopEffect(S_VOICE_LEVEL_5);
        }
    break;
    case MENU_LEVEL_2:
      if(DEVICE_STATUS == MODE_OFF && DEVICE_ACTION_STATUS == ACTION_SETTINGS) {
        // Do not advance past level 2 for the settings menu when off.
        b_changed = false;
      }
      else {
        DEVICE_MENU_LEVEL = MENU_LEVEL_3;
        led_SloBlo.turnOn(); // Level 2
        led_Vent.turnOn(); // Level 3
        led_TopWhite.turnOff(); // Level 4
        led_Clippard.turnOff(); // Level 5

        stopEffect(S_BEEPS);
        playEffect(S_BEEPS);

        stopEffect(S_VOICE_LEVEL_1);
        stopEffect(S_VOICE_LEVEL_2);
        playEffect(S_VOICE_LEVEL_3);
        stopEffect(S_VOICE_LEVEL_4);
        stopEffect(S_VOICE_LEVEL_5);
      }
     break;
    case MENU_LEVEL_3:
      DEVICE_MENU_LEVEL = MENU_LEVEL_4;
      led_SloBlo.turnOn(); // Level 2
      led_Vent.turnOn(); // Level 3
      led_TopWhite.turnOn(); // Level 4
      led_Clippard.turnOff(); // Level 5

      stopEffect(S_BEEPS);
      playEffect(S_BEEPS);

      stopEffect(S_VOICE_LEVEL_1);
      stopEffect(S_VOICE_LEVEL_2);
      stopEffect(S_VOICE_LEVEL_3);
      playEffect(S_VOICE_LEVEL_4);
      stopEffect(S_VOICE_LEVEL_5);
    break;
    case MENU_LEVEL_4:
      DEVICE_MENU_LEVEL = MENU_LEVEL_5;
      led_SloBlo.turnOn(); // Level 2
      led_Vent.turnOn(); // Level 3
      led_TopWhite.turnOn(); // Level 4
      led_Clippard.turnOn(); // Level 5

      stopEffect(S_BEEPS);
      playEffect(S_BEEPS);

      stopEffect(S_VOICE_LEVEL_1);
      stopEffect(S_VOICE_LEVEL_2);
      stopEffect(S_VOICE_LEVEL_3);
      stopEffect(S_VOICE_LEVEL_4);
      playEffect(S_VOICE_LEVEL_5);
    break;
    case MENU_LEVEL_5:
      b_changed = false;
    break;
  }

  // Returns true if value was changed.
  return b_changed;
}

bool raiseMenuLevel() {
  bool b_changed = true;

  switch(DEVICE_MENU_LEVEL){
    case MENU_LEVEL_1:
      // Menu level 1 is actually the top, so make sure all lights are off;
      b_changed = false;
    break;
    case MENU_LEVEL_2:
      DEVICE_MENU_LEVEL = MENU_LEVEL_1;
      led_SloBlo.turnOff(); // Level 2
      led_Vent.turnOff(); // Level 3
      led_TopWhite.turnOff(); // Level 4
      led_Clippard.turnOff(); // Level 5

      stopEffect(S_BEEPS);
      playEffect(S_BEEPS);

      playEffect(S_VOICE_LEVEL_1);
      stopEffect(S_VOICE_LEVEL_2);
      stopEffect(S_VOICE_LEVEL_3);
      stopEffect(S_VOICE_LEVEL_4);
      stopEffect(S_VOICE_LEVEL_5);
    break;
    case MENU_LEVEL_3:
      DEVICE_MENU_LEVEL = MENU_LEVEL_2;
      led_SloBlo.turnOn(); // Level 2
      led_Vent.turnOff(); // Level 3
      led_TopWhite.turnOff(); // Level 4
      led_Clippard.turnOff(); // Level 5

      stopEffect(S_BEEPS);
      playEffect(S_BEEPS);

      stopEffect(S_VOICE_LEVEL_1);
      playEffect(S_VOICE_LEVEL_2);
      stopEffect(S_VOICE_LEVEL_3);
      stopEffect(S_VOICE_LEVEL_4);
      stopEffect(S_VOICE_LEVEL_5);
    break;
    case MENU_LEVEL_4:
      DEVICE_MENU_LEVEL = MENU_LEVEL_3;
      led_SloBlo.turnOn(); // Level 2
      led_Vent.turnOn(); // Level 3
      led_TopWhite.turnOff(); // Level 4
      led_Clippard.turnOff(); // Level 5

      stopEffect(S_BEEPS);
      playEffect(S_BEEPS);

      stopEffect(S_VOICE_LEVEL_1);
      stopEffect(S_VOICE_LEVEL_2);
      playEffect(S_VOICE_LEVEL_3);
      stopEffect(S_VOICE_LEVEL_4);
      stopEffect(S_VOICE_LEVEL_5);
    break;
    case MENU_LEVEL_5:
      DEVICE_MENU_LEVEL = MENU_LEVEL_4;
      led_SloBlo.turnOn(); // Level 2
      led_Vent.turnOn(); // Level 3
      led_TopWhite.turnOn(); // Level 4
      led_Clippard.turnOff(); // Level 5

      stopEffect(S_BEEPS);
      playEffect(S_BEEPS);

      stopEffect(S_VOICE_LEVEL_1);
      stopEffect(S_VOICE_LEVEL_2);
      stopEffect(S_VOICE_LEVEL_3);
      playEffect(S_VOICE_LEVEL_4);
      stopEffect(S_VOICE_LEVEL_5);
    break;
  }

  // Returns true if value was changed.
  return b_changed;
}

bool decreaseOptionLevel() {
  bool b_changed = true;

  switch(MENU_OPTION_LEVEL){
    case OPTION_1:
      MENU_OPTION_LEVEL = OPTION_2;
    break;
    case OPTION_2:
      MENU_OPTION_LEVEL = OPTION_3;
    break;
    case OPTION_3:
      MENU_OPTION_LEVEL = OPTION_4;
    break;
    case OPTION_4:
      MENU_OPTION_LEVEL = OPTION_5;
    break;
    case OPTION_5:
      if(raiseMenuLevel()) {
        MENU_OPTION_LEVEL = OPTION_1;
      }
      else {
        b_changed = false;
      }
    break;
  }

  // Returns true if value was changed.
  return b_changed;
}

bool increaseOptionLevel() {
  bool b_changed = true;

  switch(MENU_OPTION_LEVEL){
    case OPTION_1:
      if(lowerMenuLevel()) {
        MENU_OPTION_LEVEL = OPTION_5;
      }
      else {
        b_changed = false;
      }
    break;
    case OPTION_2:
      MENU_OPTION_LEVEL = OPTION_1;
    break;
    case OPTION_3:
      MENU_OPTION_LEVEL = OPTION_2;
    break;
    case OPTION_4:
      MENU_OPTION_LEVEL = OPTION_3;
    break;
    case OPTION_5:
      MENU_OPTION_LEVEL = OPTION_4;
    break;
  }

  // Returns true if value was changed.
  return b_changed;
}

void mainLoop() {
  // Get the current state of any input devices (toggles, buttons, and switches).
  checkRotaryEncoder();
  checkMenuVibration();

  // Handle button press events based on current device state and menu level (for config/EEPROM purposes).
  checkDeviceAction();

  // Update bargraph with latest state and pattern changes.
  if(ms_firing_pulse.isRunning()) {
    // Increase the speed for updates while this timer is still running.
    bargraphUpdate(POWER_LEVEL - 1);
  }
  else {
    // Otherwise run with the standard timing.
    bargraphUpdate();
  }

  // Keep the cyclotron spinning as necessary.
  checkCyclotron();

  // Perform updates/actions based on timer events.
  checkGeneralTimers();
}

void checkGeneralTimers() {
  // Play the firing pulse effect animation if timer completed.
  if(ms_firing_pulse.justFinished()) {
    firePulseEffect(); // Single shot animation.
  }

  // Update the timer for the slo-blo blink.
  if(ms_slo_blo_blink.justFinished()) {
    ms_slo_blo_blink.start(i_slo_blo_blink_delay);
  }

  // Update all addressable LEDs and restart the timer.
  if(ms_fast_led.justFinished()) {
    FastLED.show();
    ms_fast_led.start(i_fast_led_delay);
  }
}

void checkCyclotron() {
  if(DEVICE_STATUS == MODE_ON) {
    if(!ms_cyclotron.isRunning()) {
      // Start the cyclotron animation with consideration for timing from the power level.
      ms_cyclotron.start(getCyclotronDelay());
    }

    switch(POWER_LEVEL) {
      case LEVEL_1:
      default:
        updateCyclotron(C_RED);
      break;
      case LEVEL_2:
        updateCyclotron(C_RED2);
      break;
      case LEVEL_3:
        updateCyclotron(C_RED3);
      break;
      case LEVEL_4:
        updateCyclotron(C_RED4);
      break;
      case LEVEL_5:
        updateCyclotron(C_RED5);
      break;
    }
  }
}

// Determine the light status on the device and any beeps.
void deviceLightControlCheck() {
  // Vent light and first stage of the safety system.
  if(switch_vent.on()) {
    if(b_vent_light_control) {
      // Vent light on, brightness dependent on mode.
      if(DEVICE_ACTION_STATUS == ACTION_FIRING || (ms_semi_automatic_firing.isRunning() && !ms_semi_automatic_firing.justFinished())) {
        led_Vent.dim(0); // 0 = Full Power
      }
      else {
        // Adjust brightness based on the power level.
        switch(POWER_LEVEL) {
          case LEVEL_1:
          default:
            led_Vent.dim(220);
          break;
          case LEVEL_2:
            led_Vent.dim(190);
          break;
          case LEVEL_3:
            led_Vent.dim(160);
          break;
          case LEVEL_4:
            led_Vent.dim(130);
          break;
          case LEVEL_5:
            led_Vent.dim(100);
          break;
        }
      }
    }
    else {
      led_Vent.turnOn();
    }
  }
  else if(!switch_vent.on()) {
    // Vent light and top white light off.
    led_Vent.turnOff();
  }
}

void deviceOff() {
  if(b_device_boot_error_on) {
    stopEffect(S_BEEPS_LOW);
    stopEffect(S_BEEPS);
    stopEffect(S_BEEPS);
  }

  stopEffect(S_BOOTUP);
  //stopEffect(S_SMASH_ERROR_RESTART);

  if(DEVICE_ACTION_STATUS == ACTION_ERROR && !b_device_boot_error_on && !b_device_mash_error) {
    // We are exiting Device Boot Error, so change device state back to off/idle.
    DEVICE_STATUS = MODE_OFF;
    DEVICE_ACTION_STATUS = ACTION_IDLE;
  }
  else if(DEVICE_ACTION_STATUS != ACTION_ERROR && (b_device_boot_error_on || b_device_mash_error)) {
    // We are entering either Device Boot Error mode or Button Mash Timeout mode, so do nothing.
  }
  else {
    // Full device shutdown in all other situations.
    DEVICE_STATUS = MODE_OFF;
    DEVICE_ACTION_STATUS = ACTION_IDLE;

    if(b_device_mash_error) {
      // stopEffect(S_SMASH_ERROR_LOOP);
      // stopEffect(S_SMASH_ERROR_RESTART);
    }

    // Turn off any barrel spark effects and reset the button mash lockout.
    if(b_device_mash_error) {
      barrelLightsOff();
      b_device_mash_error = false;
    }

    stopEffect(S_SHUTDOWN);
    playEffect(S_SHUTDOWN);
  }

  soundIdleLoopStop();

  vibrationOff();

  // Stop firing if the device is turned off.
  if(b_firing) {
    modeFireStop();
  }

  if(DEVICE_ACTION_STATUS != ACTION_ERROR && b_device_mash_error) {
    // playEffect(S_DEVICE_MASH_ERROR);
  }

  // Clear counter until user begins firing again.
  i_bmash_count = 0;

  // Turn off some timers.
  ms_cyclotron.stop();
  ms_settings_blinking.stop();
  ms_semi_automatic_check.stop();
  ms_semi_automatic_firing.stop();
  ms_hat_1.stop();
  ms_hat_2.stop();

  switch(DEVICE_STATUS) {
    case MODE_OFF:
      // Turn off all device lights.
      allLightsOff();

      // Start the timer for the power on indicator option.
      if(b_power_on_indicator) {
        ms_power_indicator.start(i_ms_power_indicator);
      }

      deviceSwitchedCount = 0;
      ventSwitchedCount = 0;
    break;
    default:
      // Do nothing if we aren't MODE_OFF
    break;
  }
}

// Check if we should fire, or if the device was turned off.
void fireControlCheck() {
  // Firing action stuff and shutting cyclotron and the Single-Shot Blaster off.
  if(DEVICE_ACTION_STATUS != ACTION_SETTINGS) {
    // If Activate switch is down, turn device off.
    if(!switch_activate.on()) {
      DEVICE_ACTION_STATUS = ACTION_OFF;
      return;
    }

    if(i_bmash_count >= i_bmash_max) {
      // User has exceeded "normal" firing rate.
      switch(STREAM_MODE) {
        case PROTON:
        default:
          stopEffect(S_FIRE_BLAST);
        break;
      }

      b_device_mash_error = true;
      modeError();
      //deviceTipSpark();

      // Adjust the cool down lockout period based on the power level.
      switch(POWER_LEVEL) {
        case LEVEL_1:
        default:
          ms_bmash.start(i_bmash_cool_down);
        break;
        case LEVEL_2:
          ms_bmash.start(i_bmash_cool_down + 500);
        break;
        case LEVEL_3:
          ms_bmash.start(i_bmash_cool_down + 1000);
        break;
        case LEVEL_4:
          ms_bmash.start(i_bmash_cool_down + 1500);
        break;
        case LEVEL_5:
          ms_bmash.start(i_bmash_cool_down + 2000);
        break;
      }
    }
    else {
      if(switch_intensify.on() && switch_device.on() && switch_vent.on()) {
        switch(STREAM_MODE) {
          case PROTON:
          default:
            if(DEVICE_ACTION_STATUS != ACTION_FIRING) {
              DEVICE_ACTION_STATUS = ACTION_FIRING;
            }

            if(ms_bmash.remaining() < 1) {
              // Clear counter/timer until user begins firing.
              i_bmash_count = 0;
              ms_bmash.start(i_bmash_delay);
            }

            if(!b_firing_intensify) {
              // Increase count each time the user presses a firing button.
              i_bmash_count++;
            }

            b_firing_intensify = true;
          break;
        }
      }

      if(STREAM_MODE == PROTON && DEVICE_ACTION_STATUS == ACTION_FIRING) {
        if(switch_grip.on()) {
          b_firing_alt = true;
        }
      }
      else if(switch_grip.on() && switch_device.on() && switch_vent.on()) {
        switch(STREAM_MODE) {
          case PROTON:
            // Handle Primary Blast fire start here.
            if(!b_firing_semi_automatic && ms_semi_automatic_check.remaining() < 1) {
              // Start rate-of-fire timer.
              ms_semi_automatic_check.start(i_single_shot_rate);

              modePulseStart();

              b_firing_semi_automatic = true;
            }
          break;

          default:
            // Do nothing.
          break;
        }
      }

      if(!switch_intensify.on()) {
        switch(STREAM_MODE) {
          case PROTON:
          default:
            if(b_firing && b_firing_intensify) {
              if(!b_firing_alt) {
                DEVICE_ACTION_STATUS = ACTION_IDLE;
              }

              b_firing_intensify = false;
            }
          break;
        }
      }

      if(!switch_grip.on()) {
        switch(STREAM_MODE) {
          case PROTON:
            // Handle resetting semi-auto bool here.
            b_firing_semi_automatic = false;
          break;

          default:
            // Do nothing.
          break;
        }
      }
    }
  }
  else if(DEVICE_ACTION_STATUS == ACTION_SETTINGS) {
    // If Activate switch is down, turn device off.
    if(!switch_activate.on()) {
      DEVICE_ACTION_STATUS = ACTION_OFF;
      return;
    }
  }
}

void modeError() {
  deviceOff();

  DEVICE_STATUS = MODE_ERROR;
  DEVICE_ACTION_STATUS = ACTION_ERROR;

  if(!b_device_mash_error) {
    // This is used for controlling the bargraph beeping while in boot error mode.
    ms_hat_1.start(i_hat_2_delay * 4);
    ms_hat_2.start(i_hat_2_delay);
    ms_settings_blinking.start(i_settings_blinking_delay);

    playEffect(S_BEEPS_LOW);
    playEffect(S_BEEPS);
    playEffect(S_BEEPS);
  }
  else if(b_device_mash_error) {
    // playEffect(S_SMASH_ERROR_LOOP, true, i_volume_effects, true, 2500);
  }
}

void modeActivate() {
  // The device was started while the top switch was already on, so let's put the device into startup error mode.
  if(switch_device.on() && b_device_boot_errors) {
    b_device_boot_error_on = true;
    modeError();
  }
  else {
    // Device is officially activated and on.
    DEVICE_STATUS = MODE_ON;

    // Proper startup. Continue booting up the device.
    DEVICE_ACTION_STATUS = ACTION_IDLE;

    // Clear counter until user begins firing.
    i_bmash_count = 0;
  }

  b_device_mash_error = false;

  postActivation(); // Enable lights and bargraph after device activation.
}

void postActivation() {
  if(DEVICE_STATUS != MODE_ERROR) {
    // Turn on slo-blo light.
    led_SloBlo.turnOn();

    // Turn on the Clippard LED.
    led_Clippard.turnOn();

    // Top white light.
    ms_white_light.start(i_top_blink_interval);
    led_TopWhite.turnOn();

    // Reset the hat light timers.
    ms_hat_1.stop();
    ms_hat_2.stop();

    stopEffect(S_BOOTUP);
    playEffect(S_BOOTUP);

    soundIdleLoop(true);

    if(bargraph.STATE == BG_OFF) {
      bargraph.reset(); // Enable bargraph for use (resets variables and turns it on).
      bargraph.PATTERN = BG_POWER_RAMP; // Bargraph idling loop.
    }
  }
}

void soundIdleLoop(bool fadeIn) {
  switch(POWER_LEVEL) {
    case LEVEL_1:
    default:
      playEffect(S_IDLE_LOOP, true, i_volume_effects, fadeIn, 5000);
    break;
    case LEVEL_2:
      playEffect(S_IDLE_LOOP, true, i_volume_effects, fadeIn, 5000);
    break;
    case LEVEL_3:
      playEffect(S_IDLE_LOOP, true, i_volume_effects, fadeIn, 5000);
    break;
    case LEVEL_4:
      playEffect(S_IDLE_LOOP, true, i_volume_effects, fadeIn, 5000);
    break;
    case LEVEL_5:
      playEffect(S_IDLE_LOOP, true, i_volume_effects, fadeIn, 5000);
    break;
  }
}

void soundIdleLoopStop() {
  switch(POWER_LEVEL) {
    case LEVEL_1:
    default:
      stopEffect(S_IDLE_LOOP);
    break;
    case LEVEL_2:
      stopEffect(S_IDLE_LOOP);
    break;
    case LEVEL_3:
      stopEffect(S_IDLE_LOOP);
    break;
    case LEVEL_4:
      stopEffect(S_IDLE_LOOP);
    break;
    case LEVEL_5:
      stopEffect(S_IDLE_LOOP);
    break;
  }
}

void modePulseStart() {
  // Handles all "pulsed" fire modes.
  i_fast_led_delay = FAST_LED_UPDATE_MS;
  barrelLightsOff();

  playEffect(S_FIRE_BLAST, false, i_volume_effects, false, 0, false);

  ms_firing_pulse.start(i_firing_pulse);
  ms_semi_automatic_firing.start(350);
}

void modeFireStart() {
  i_fast_led_delay = FAST_LED_UPDATE_MS;

  //modeFireStartSounds();

  // Just in case a semi-auto was fired before we started firing a stream, stop its timer.
  ms_semi_automatic_firing.stop();

  // Turn on hat light 1.
  led_Hat1.turnOn();

  barrelLightsOff();
}

void modeFireStopSounds() {
  // Reset some sound triggers.
  b_sound_firing_intensify_trigger = false;
  b_sound_firing_alt_trigger = false;

  ms_single_blast.stop();
}

void modeFireStop() {
  DEVICE_ACTION_STATUS = ACTION_IDLE;

  b_firing = false;
  b_firing_intensify = false;
  b_firing_alt = false;

  led_Hat2.turnOn(); // Make sure we turn on hat light 2 in case it's off as well.

  led_Tip.turnOff();

  ms_hat_1.stop();

  modeFireStopSounds();
}

void modeFiring() {
  // Sound trigger flags.
  if(b_firing_intensify && !b_sound_firing_intensify_trigger) {
    b_sound_firing_intensify_trigger = true;
  }

  if(!b_firing_intensify && b_sound_firing_intensify_trigger) {
    b_sound_firing_intensify_trigger = false;
  }

  if(b_firing_alt && !b_sound_firing_alt_trigger) {
    b_sound_firing_alt_trigger = true;
  }

  if(!b_firing_alt && b_sound_firing_alt_trigger) {
    b_sound_firing_alt_trigger = false;
  }
}

void firePulseEffect() {
  if(i_pulse_step == 0) {
    // Force clear and reset of bargraph state.
    bargraph.clear();
    bargraph.reset();
    bargraph.commit();

    // Change bargraph animation when pulse begins.
    bargraph.PATTERN = BG_OUTER_INNER;
  }

  // Primary Blast.
  switch(i_pulse_step) {
    case 0:
      system_leds[i_barrel_led] = getHueAsRGB(C_RED);
    break;
    case 1:
      system_leds[i_barrel_led] = getHueAsRGB(C_RED3);
    break;
    case 2:
      system_leds[i_barrel_led] = getHueAsRGB(C_RED5);
    break;
    case 3:
      system_leds[i_barrel_led] = getHueAsRGB(C_WHITE);
    break;
    case 4:
      system_leds[i_barrel_led] = getHueAsRGB(C_BLACK);
    break;
    case 5:
      system_leds[i_barrel_led] = getHueAsRGB(C_WHITE);
    break;
    case 6:
      system_leds[i_barrel_led] = getHueAsRGB(C_BLACK);
    break;
    case 7:
      system_leds[i_barrel_led] = getHueAsRGB(C_WHITE);
    break;
    case 8:
      system_leds[i_barrel_led] = getHueAsRGB(C_RED4);
    break;
    case 9:
      system_leds[i_barrel_led] = getHueAsRGB(C_RED2);
    break;
    case 10:
      system_leds[i_barrel_led] = getHueAsRGB(C_RED);
      // Bolt has reached barrel tip, so turn on tip light.
      led_Tip.turnOn();
    break;
    case 11:
      system_leds[i_barrel_led] = getHueAsRGB(C_BLACK);
    break;
    default:
      // Do nothing if we somehow end up here.
    break;
  }

  i_pulse_step++;

  if(i_pulse_step < i_pulse_step_max) {
    ms_firing_pulse.start(i_firing_pulse);
  }
  else {
    // Animation has concluded, so reset our timer and variable.
    led_Tip.turnOff();
    ms_firing_pulse.stop();
    i_pulse_step = 0;

    // Clear the bargraph and return to ramping.
    bargraph.clear();
    bargraph.PATTERN = BG_POWER_RAMP;
  }
}

void barrelLightsOff() {
  ms_firing_pulse.stop();

  i_pulse_step = 0;

  // Turn off the barrel LED.
  system_leds[i_barrel_led] = getHueAsRGB(C_BLACK);

  // Turn off the device barrel tip LED.
  led_Tip.turnOff();
}

void allLightsOff() {
  bargraph.off();

  // Turn off all non-addressable LEDs.
  led_Clippard.turnOff(); // Turn off the front left LED under the Clippard valve.
  led_Hat1.turnOff(); // Turn off hat light 1 (not used, but just make sure).
  led_Hat2.turnOff(); // Turn off hat light 2.
  led_SloBlo.turnOff();
  led_Tip.turnOff(); // Not used normally, but make sure it's off.
  led_TopWhite.turnOff();
  led_Vent.turnOff();

  // Clear all addressable LEDs by filling the array with black.
  fill_solid(system_leds, CYCLOTRON_LED_COUNT + BARREL_LED_COUNT, CRGB::Black);

  if(b_power_on_indicator && !ms_power_indicator.isRunning()) {
    ms_power_indicator.start(i_ms_power_indicator);
  }
}

void allMenuLightsOff() {
  // Make sure some of the device lights are off, specifically for the Menu systems.
  // LEDs are in the order by which they indicate the menu levels above level 1.
  led_SloBlo.turnOff(); // Level 2
  led_Vent.turnOff(); // Level 3
  led_TopWhite.turnOff(); // Level 4
  led_Clippard.turnOff(); // Level 5

  if(b_power_on_indicator) {
    ms_power_indicator.stop();
    ms_power_indicator_blink.stop();
  }
}

void vibrationDevice(uint8_t i_level) {
  if(b_vibration_enabled && i_level > 0) {
    // Vibrate the device during firing only when enabled.
    if(b_vibration_firing) {
      if(DEVICE_ACTION_STATUS == ACTION_FIRING || (ms_semi_automatic_firing.isRunning() && !ms_semi_automatic_firing.justFinished())) {
        if(ms_semi_automatic_firing.isRunning()) {
          analogWrite(vibration, 180);
        }
        else if(i_level != i_vibration_level_prev) {
          i_vibration_level_prev = i_level;
          analogWrite(vibration, i_level);
        }
      }
      else {
        vibrationOff();
      }
    }
    else {
      // Device vibrates even when idling, etc.
      if(i_level != i_vibration_level_prev) {
        i_vibration_level_prev = i_level;
        analogWrite(vibration, i_level);
      }
    }
  }
  else {
    vibrationOff();
  }
}

void vibrationSetting() {
  if(DEVICE_ACTION_STATUS != ACTION_FIRING) {
    switch(POWER_LEVEL) {
      case LEVEL_1:
      default:
        vibrationDevice(i_vibration_level);
      break;

      case LEVEL_2:
        vibrationDevice(i_vibration_level + 5);
      break;

      case LEVEL_3:
        vibrationDevice(i_vibration_level + 10);
      break;

      case LEVEL_4:
        vibrationDevice(i_vibration_level + 12);
      break;

      case LEVEL_5:
        vibrationDevice(i_vibration_level + 25);
      break;
    }
  }
}

void checkMenuVibration() {
  if(ms_menu_vibration.justFinished()) {
    vibrationOff();
  }
  else if(ms_menu_vibration.isRunning()) {
    analogWrite(vibration, 150);
  }
}

void vibrationOff() {
  ms_menu_vibration.stop();
  i_vibration_level_prev = 0;
  analogWrite(vibration, 0);
}

void switchLoops() {
  switch_intensify.poll();
  switch_activate.poll();
  switch_vent.poll();
  switch_device.poll();
  switch_grip.poll();
}

void ventSwitched(void* n) {
  (void)(n); // Suppress unused variable warning
  ventSwitchedCount++;
}

void deviceSwitched(void* n) {
  (void)(n); // Suppress unused variable warning
  deviceSwitchedCount++;
}

// Enter the device menu system.
void deviceEnterMenu() {
  debug("deviceEnterMenu|");
  debugln(DEVICE_ACTION_STATUS);

  // Enter a menu at level 1, at option #5
  DEVICE_MENU_LEVEL = MENU_LEVEL_1;
  MENU_OPTION_LEVEL = OPTION_5;

  playEffect(S_CLICK);

  allLightsOff();
  allMenuLightsOff();

  bargraph.showBars(MENU_OPTION_LEVEL);
}

// Exit the device menu system while the device is off.
void deviceExitMenu() {
  debug("deviceExitMenu|");
  debugln(DEVICE_ACTION_STATUS);

  // Reset the menu level/option to default
  DEVICE_MENU_LEVEL = MENU_LEVEL_1;
  MENU_OPTION_LEVEL = OPTION_5;

  playEffect(S_CLICK);

  DEVICE_ACTION_STATUS = ACTION_IDLE;

  allLightsOff();

  if(DEVICE_STATUS == MODE_ON && bargraph.STATE == BG_OFF) {
    bargraph.reset(); // Enable bargraph for use (resets variables and turns it on).
    bargraph.PATTERN = BG_POWER_RAMP; // Bargraph idling loop.
  }
}

// Exit the device menu EEPROM system while the device is off.
void deviceExitEEPROMMenu() {
  debug("deviceExitEEPROMMenu|");
  debugln(DEVICE_ACTION_STATUS);

  // Reset the menu level/option to default
  DEVICE_MENU_LEVEL = MENU_LEVEL_1;
  MENU_OPTION_LEVEL = OPTION_5;

  playEffect(S_BEEPS);

  DEVICE_ACTION_STATUS = ACTION_IDLE;

  allLightsOff();

  deviceSwitchedCount = 0;
  ventSwitchedCount = 0;

  vibrationOff(); // Make sure we stop any menu-related vibration, if any.
}

// Included last as the contained logic will control all aspects of the device using the defined functions above.
#include "Actions.h"

// Checks the top rotary dial on the device.
void checkRotaryEncoder() {
  encoder.check(); // Update the latest state of the device resulting from any user input.
  checkEncoderAction(); // Take action specifically from interaction by the user.
}