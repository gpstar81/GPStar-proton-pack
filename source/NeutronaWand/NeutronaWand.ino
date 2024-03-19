/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
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

/**
 * Please note, due to limitations of the ATMega328P, Arduino Nano builds are no longer supported for the Neutrona Wand.
 * The last supported version is 2.2.0
 * https://github.com/gpstar81/haslab-proton-pack/releases/tag/V2.2.0
 */

#if defined(__AVR_ATmega2560__)
  #define GPSTAR_NEUTRONA_WAND_PCB
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

// 3rd-Party Libraries
#include <EEPROM.h>
#include <millisDelay.h>
#include <FastLED.h>
#include <ezButton.h>
#include <ht16k33.h>
#include <Wire.h>
#include <SerialTransfer.h>

// Local Files
#include "Configuration.h"
#include "MusicSounds.h"
#include "Communication.h"
#include "Header.h"
#include "Colours.h"
#include "Audio.h"
#include "Preferences.h"

void setup() {
  Serial.begin(9600); // Standard serial (USB) console.

  Serial1.begin(9600); // Communication to the Proton Pack.
  wandComs.begin(Serial1, false);

  // Setup the audio device for this controller.
  selectAudioDevice();

  // Change PWM frequency of pin 3 and 11 for the vibration motor, we do not want it high pitched.
  TCCR2B = (TCCR2B & B11111000) | (B00000110); // for PWM frequency of 122.55 Hz

  // Barrel LEDs - NOTE: These are GRB not RGB so note that all CRGB objects will have R/G swapped.
  FastLED.addLeds<NEOPIXEL, BARREL_LED_PIN>(barrel_leds, BARREL_LEDS_MAX);

  // Setup default system settings.
  SYSTEM_MODE = MODE_SUPER_HERO;
  BARGRAPH_MODE = BARGRAPH_ORIGINAL;
  BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_DEFAULT;
  BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_SUPER_HERO;
  BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_DEFAULT;
  VIBRATION_MODE_EEPROM = VIBRATION_DEFAULT;
  WAND_MENU_LEVEL = MENU_LEVEL_1;
  WAND_YEAR_MODE = YEAR_DEFAULT;
  WAND_YEAR_CTS = CTS_DEFAULT;
  SYSTEM_YEAR = SYSTEM_AFTERLIFE;
  WAND_BARREL_LED_COUNT = LEDS_5;

  switch_intensify.setDebounceTime(i_switch_debounce);
  switch_activate.setDebounceTime(i_switch_debounce);
  switch_vent.setDebounceTime(i_switch_debounce);
  switch_wand.setDebounceTime(i_switch_debounce);
  switch_mode.setDebounceTime(i_switch_debounce);
  switch_barrel.setDebounceTime(i_switch_debounce);

  // Rotary encoder on the top of the wand.
  pinMode(r_encoderA, INPUT_PULLUP);
  pinMode(r_encoderB, INPUT_PULLUP);

  // Setup the bargraph.
  bargraphYearModeUpdate();

  delay(10);

  WIRE.begin();

  byte by_error, by_address;
  unsigned int i_i2c_devices = 0;

  // Scan i2c for any devices (28 segment bargraph).
  for(by_address = 1; by_address < 127; by_address++ ) {
    WIRE.beginTransmission(by_address);
    by_error = WIRE.endTransmission();

    if(by_error == 0) {
      i_i2c_devices++;
    }
  }

  if(i_i2c_devices > 0) {
    b_28segment_bargraph = true;
  }
  else {
    b_28segment_bargraph = false;
  }

  if(b_28segment_bargraph == true) {
    ht_bargraph.begin(0x00);
  }
  else {
    // Original 5 LED Hasbro bargraph.
    pinMode(led_bargraph_1, OUTPUT);
    pinMode(led_bargraph_2, OUTPUT);
    pinMode(led_bargraph_3, OUTPUT);
    pinMode(led_bargraph_4, OUTPUT);
    pinMode(led_bargraph_5, OUTPUT);
  }

  setBargraphOrientation();

  pinMode(led_slo_blo, OUTPUT);

  pinMode(led_front_left, OUTPUT); // Front left LED underneath the Clippard valve.
  pinMode(led_hat_1, OUTPUT); // Hat light at front of the wand near the barrel tip.
  pinMode(led_hat_2, OUTPUT); // Hat light at top of the wand body (gun box).
  pinMode(led_barrel_tip, OUTPUT); // LED at the tip of the wand barrel.

  pinMode(led_vent, OUTPUT);
  pinMode(led_white, OUTPUT);

  pinMode(vibration, OUTPUT);

  // Make sure lights are off.
  wandLightsOff();

  // Wand status.
  WAND_STATUS = MODE_OFF;
  WAND_ACTION_STATUS = ACTION_IDLE;

  ms_reset_sound_beep.start(i_sound_timer);

  // We bootup the wand in the classic proton mode.
  FIRING_MODE = PROTON;
  PREV_FIRING_MODE = SETTINGS;

  // Load any saved settings stored in the EEPROM memory of the gpstar Neutrona Wand.
  if(b_eeprom == true) {
    readEEPROM();
  }

  ms_bmash.start(i_bmash_delay);

  // Sanity check just in case a user forgot to enable CTS while enabling CTS Mix.
  if(b_cross_the_streams_mix == true && b_cross_the_streams != true) {
    b_cross_the_streams = true;
  }

  // Check if we should be in video game mode or not.
  vgModeCheck();

  // Start up some timers for MODE_ORIGINAL.
  ms_slo_blo_blink.start(i_slo_blo_blink_delay);

  // Initialize the timer for initial handshake.
  ms_packsync.start(1);
  ms_handshake.stop();

  if(b_gpstar_benchtest == true) {
    WAND_CONN_STATE = NC_BENCHTEST;

    b_pack_on = true; // Pretend that the pack (not really attached) has been powered on.

    // Stop the pack sync timer since we are no longer syncing to a pack.
    ms_packsync.stop();

    // Check music timer for bench test mode only.
    ms_check_music.start(i_music_check_delay);
  }
  else {
    WAND_CONN_STATE = PACK_DISCONNECTED;
  }
}

void loop() {
  switch(WAND_CONN_STATE) {
    case PACK_DISCONNECTED:
      // While waiting for a proton pack, issue a request for synchronization.
      if(ms_packsync.justFinished()) {
        // If not already doing so, explicitly tell the pack a wand is here to sync.
        wandSerialSend(W_SYNC_NOW);
        ms_packsync.start(i_sync_initial_delay); // Prepare for the next sync attempt.
        b_sync_light = !b_sync_light; // Toggle a white LED while attempting to sync.
        digitalWrite(led_white, (b_sync_light ? HIGH : LOW)); // Blink an LED.
      }

      checkPack(); // Check for any response from the pack while still waiting.
    break;

    case SYNCHRONIZING:
      checkPack(); // Keep checking for responses from the pack while synchronizing.
    break;

    case PACK_CONNECTED:
      // When connected to a pack, prepare to send a regular handshake to indicate presence.
      if(ms_handshake.justFinished()) {
        wandSerialSend(W_HANDSHAKE); // Remind the pack that a wand is still present.
        ms_handshake.start(i_heartbeat_delay); // Delay after initial connection.
      }

      mainLoop(); // Continue on to the main loop.
    break;

    case NC_BENCHTEST:
      mainLoop(); // Continue on to the main loop.
    break;
  }
}

void mainLoop() {
  updateAudio(); // Update the state of the audio device.
  checkMusic(); // Music control is here since pack is not present.

  // Get the current state of any input devices (toggles, buttons, and switches).
  switchLoops();
  checkSwitches();
  checkRotaryEncoder();

  if(ms_firing_stop_sound_delay.justFinished()) {
    modeFireStopSounds();
  }

  if(WAND_ACTION_STATUS != ACTION_FIRING) {
    if(ms_bmash.remaining() < 1) {
      // Clear counter until user begins firing (post any lock-out period).
      i_bmash_count = 0;

      if(b_wand_mash_error == true) {
        // Return the wand to a normal firing state after lock-out from button mashing.
        b_wand_mash_error = false;

        WAND_STATUS = MODE_ON;
        WAND_ACTION_STATUS = ACTION_IDLE;

        wandSerialSend(W_ON);
        postActivation();

        if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
          if(b_extra_pack_sounds == true) {
            wandSerialSend(W_WAND_BOOTUP_SOUND);
          }

          stopEffect(S_WAND_BOOTUP);
          playEffect(S_WAND_BOOTUP);
        }

        bargraphClearAlt();

        // Re-enable the hat light on top of the gun box
        digitalWrite(led_hat_2, HIGH);
      }
    }
  }

  // Handle button press events based on current wand state and menu level (for config/EEPROM purposes).
  checkWandAction();

  switch(WAND_STATUS) {
    case MODE_OFF:
      if(WAND_ACTION_STATUS != ACTION_LED_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU) {
        if(FIRING_MODE != SETTINGS && b_gpstar_benchtest == true && SYSTEM_MODE == MODE_ORIGINAL && switch_intensify.getCount() >= 2) {
          // This allows a standalone wand to "flip the ion arm switch" when in MODE_ORIGINAL by double-clicking the Intensify switch while the wand is turned off
          if(b_pack_ion_arm_switch_on == true) {
            b_pack_ion_arm_switch_on = false;
          }
          else {
            b_pack_ion_arm_switch_on = true;
          }

          ms_intensify_doubleclick.stop();
          switch_intensify.resetCount();
        }
        else if(FIRING_MODE != SETTINGS && b_gpstar_benchtest == true && SYSTEM_MODE == MODE_ORIGINAL && switch_intensify.getCount() == 1) {
          // This "times out" the Intensify click after 3 seconds so that a double-click is always required
          if(ms_intensify_doubleclick.justFinished()) {
            switch_intensify.resetCount();
            ms_intensify_doubleclick.stop();
          }
          else if(!ms_intensify_doubleclick.isRunning() && !ms_intensify_doubleclick.justFinished()) {
            ms_intensify_doubleclick.start(i_doubleclick_delay);
          }
        }

        if(switch_mode.isPressed() || b_pack_alarm == true) {
          if(FIRING_MODE != SETTINGS && b_pack_alarm != true && (b_pack_on != true || b_gpstar_benchtest == true)) {
            playEffect(S_CLICK);

            PREV_FIRING_MODE = FIRING_MODE;
            FIRING_MODE = SETTINGS;

            WAND_ACTION_STATUS = ACTION_SETTINGS;
            WAND_MENU_LEVEL = MENU_LEVEL_1;

            i_wand_menu = 5;
            ms_settings_blinking.start(i_settings_blinking_delay);

            ms_bargraph.stop();
            bargraphClearAlt();

            // Make sure some of the wand lights are off.
            wandLightsOffMenuSystem();

            // Tell the pack we are in settings mode.
            wandSerialSend(W_SETTINGS_MODE);
          }
          else {
            // Only exit the settings menu when on menu #5 in the top menu.
            if(i_wand_menu == 5 && WAND_MENU_LEVEL == MENU_LEVEL_1 && FIRING_MODE == SETTINGS) {
              wandExitMenu();
            }
          }
        }
        else if(WAND_ACTION_STATUS == ACTION_SETTINGS && b_pack_on == true) {
          if(b_gpstar_benchtest != true) {
            wandExitMenu();
          }
        }
      }

      // Reset the count of the wand switch
      if(switch_intensify.getState() == HIGH) {
        switch_wand.resetCount();
        switch_vent.resetCount();
      }

      if(WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_LED_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU && (b_pack_on != true || b_gpstar_benchtest == true) && switch_intensify.getState() == LOW && switch_wand.getCount() >= 5) {
        stopEffect(S_BEEPS_BARGRAPH);
        playEffect(S_BEEPS_BARGRAPH);

        stopEffect(S_EEPROM_LED_MENU);
        playEffect(S_EEPROM_LED_MENU);

        wandSerialSend(W_EEPROM_LED_MENU);
        wandSerialSend(W_SPECTRAL_LIGHTS_ON);

        i_wand_menu = 5;

        WAND_ACTION_STATUS = ACTION_LED_EEPROM_MENU;
        WAND_MENU_LEVEL = MENU_LEVEL_1;

        ms_settings_blinking.start(i_settings_blinking_delay);

        wandBarrelSpectralCustomConfigOn();

        // Make sure some of the wand lights are off.
        wandLightsOffMenuSystem();
      }
      else if(WAND_ACTION_STATUS == ACTION_LED_EEPROM_MENU && b_pack_on == true) {
        if(b_gpstar_benchtest != true) {
          wandExitEEPROMMenu();
        }
      }

      if(WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_LED_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU
         && (b_pack_on != true || b_gpstar_benchtest == true) && switch_intensify.getState() == LOW && switch_vent.getCount() >= 5) {
        stopEffect(S_BEEPS_BARGRAPH);
        playEffect(S_BEEPS_BARGRAPH);

        stopEffect(S_EEPROM_CONFIG_MENU);
        playEffect(S_EEPROM_CONFIG_MENU);

        wandSerialSend(W_EEPROM_CONFIG_MENU);

        i_wand_menu = 5;

        WAND_ACTION_STATUS = ACTION_CONFIG_EEPROM_MENU;
        WAND_MENU_LEVEL = MENU_LEVEL_1;

        ms_settings_blinking.start(i_settings_blinking_delay);

        // Make sure some of the wand lights are off.
        wandLightsOffMenuSystem();
      }
      else if(WAND_ACTION_STATUS == ACTION_CONFIG_EEPROM_MENU && b_pack_on == true) {
        if(b_gpstar_benchtest != true) {
          wandExitEEPROMMenu();
        }
      }

      if(b_pack_alarm == true) {
        if(ms_hat_2.justFinished()) {
          ms_hat_2.start(i_hat_2_delay);
        }
      }

      // If the power indicator is enabled. Blink the LED on the Neutrona Wand body next to the clippard valve to indicator the system has battery power.
      if(b_power_on_indicator == true && WAND_ACTION_STATUS == ACTION_IDLE && (b_pack_on != true || b_gpstar_benchtest == true)) {
        if(ms_power_indicator.isRunning() == true && ms_power_indicator.remaining() < 1) {
          if(ms_power_indicator_blink.isRunning() != true || ms_power_indicator_blink.justFinished()) {
            ms_power_indicator_blink.start(i_ms_power_indicator_blink);
          }

          switch(SYSTEM_MODE) {
            case MODE_ORIGINAL:
              if(b_pack_ion_arm_switch_on != true) {
                if(ms_power_indicator_blink.remaining() < i_ms_power_indicator_blink / 2) {
                  analogWrite(led_front_left, 0);
                }
                else {
                  analogWrite(led_front_left, 255);
                }
              }
              else {
                // When the top right wand switch is off, then we make sure the led is off as the Slo-Blo LED will be on or blinking at this point.
                if(switch_wand.getState() == HIGH) {
                  analogWrite(led_front_left, 0);
                }
              }
              break;

            case MODE_SUPER_HERO:
            default:
              if(ms_power_indicator_blink.remaining() < i_ms_power_indicator_blink / 2) {
                analogWrite(led_front_left, 0);
              }
              else {
                analogWrite(led_front_left, 255);
              }
            break;
          }
        }
        else {
          analogWrite(led_front_left, 0);
        }
      }
    break;

    case MODE_ERROR:
      if(ms_hat_2.remaining() < i_hat_2_delay / 2) {
        digitalWrite(led_white, HIGH);

        analogWrite(led_slo_blo, 0);

        digitalWrite(led_hat_2, LOW);
        digitalWrite(led_hat_1, LOW);
        analogWrite(led_front_left, 0);
      }
      else {
        digitalWrite(led_hat_2, HIGH);
        digitalWrite(led_hat_1, HIGH);
        analogWrite(led_front_left, 255);

        digitalWrite(led_white, LOW);
        analogWrite(led_slo_blo, 255);
      }

      if(ms_hat_2.justFinished()) {
        ms_hat_2.start(i_hat_2_delay);

        if(b_extra_pack_sounds == true) {
          // Only play this beep on the Proton Pack if the Neutrona Wand has no audio board.
          wandSerialSend(W_WAND_BEEP_SOUNDS);
        }

        playEffect(S_BEEPS_LOW);
        playEffect(S_BEEPS);
      }

      if(ms_hat_1.justFinished()) {
        if(b_extra_pack_sounds == true) {
          // Only play this beep on the Proton Pack if the Neutrona Wand has no audio board.
          wandSerialSend(W_WAND_BEEP_BARGRAPH);
        }

        playEffect(S_BEEPS_BARGRAPH);

        ms_hat_1.start(i_hat_2_delay * 4);
      }

      settingsBlinkingLights();
    break;

    case MODE_ON:
      if(b_vibration_on == true && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
        vibrationSetting();
      }

      // Hat light 2 blinking when the Proton Pack ribbon cable has been removed.
      if(b_pack_alarm == true) {
        if(ms_hat_2.remaining() < i_hat_2_delay / 2) {
          digitalWrite(led_hat_2, LOW);
        }
        else {
          digitalWrite(led_hat_2, HIGH);
        }

        if(ms_hat_2.justFinished()) {
          ms_hat_2.start(i_hat_2_delay);
        }
      }
      else {
        if(ms_hat_1.isRunning() != true) {
          // Hat 2 stays solid while the Neutrona Wand is on. It will blink when about to overheat.
          digitalWrite(led_hat_2, HIGH);
        }
      }

      // Top white light.
      if(ms_white_light.justFinished()) {
        ms_white_light.start(d_white_light_interval);
        if(digitalRead(led_white) == LOW) {
          digitalWrite(led_white, HIGH);

          // We make the slo-blo light blink during vent mode.
          if(FIRING_MODE == VENTING) {
            analogWrite(led_slo_blo, 255);

            analogWrite(led_front_left, 255);
          }
        }
        else {
          digitalWrite(led_white, LOW);

          // We make the slo-blo light blink during vent mode.
          if(FIRING_MODE == VENTING) {
            analogWrite(led_slo_blo, 0);

            analogWrite(led_front_left, 0);
          }
        }
      }

      if(b_pack_alarm != true) {
        // Ramp the bargraph up then ramp down back to the default power level setting on a fresh start.
        if(ms_bargraph.justFinished()) {
          bargraphRampUp();
        }
        else if(ms_bargraph.isRunning() == false && WAND_ACTION_STATUS != ACTION_FIRING && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          // Bargraph idling loop.
          bargraphPowerCheck();
        }

        if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
          if(ms_gun_loop_1.justFinished() && switch_vent.getState() == HIGH) {
            playEffect(S_AFTERLIFE_WAND_IDLE_1, true, i_volume_effects - 1);
            b_sound_afterlife_idle_2_fade = false;
            ms_gun_loop_1.stop();

            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_AFTERLIFE_GUN_LOOP_1);
            }
          }
        }
      }
      else {
        // This is going to cause the bargraph to ramp down.
        if(ms_bargraph.justFinished()) {
          bargraphRampUp();
        }
      }

      wandBarrelHeatUp();
    break;
  }

  if(b_firing == true && WAND_ACTION_STATUS != ACTION_FIRING) {
    modeFireStop();
  }

  if(ms_firing_lights_end.justFinished()) {
    fireStreamEnd(getHueColour(C_BLACK, WAND_BARREL_LED_COUNT));
  }

  // Update the barrel LEDs.
  if(ms_fast_led.justFinished()) {
    FastLED.show();
    ms_fast_led.stop();
  }
}

// Sets the Neutrona Wand to video game mode.
void setVGMode() {
  b_cross_the_streams = false;
  b_cross_the_streams_mix = false;

  b_vg_mode = true;
}

// Checks if VGA mode should be set.
void vgModeCheck() {
  if(b_cross_the_streams == true || b_cross_the_streams_mix == true) {
    b_vg_mode = false;
  }
  else {
    b_vg_mode = true;
  }
}

void wandTipOn() {
  switch(WAND_BARREL_LED_COUNT) {
    case LEDS_48:
      // Set the tip of the Frutto LED array to white.
      barrel_leds[12] = getHueColour(C_WHITE, WAND_BARREL_LED_COUNT);
    break;

    case LEDS_5:
    default:
      // Illuminate the wand barrel tip LED.
      digitalWrite(led_barrel_tip, HIGH);
    break;
  }
}

void wandTipOff() {
  switch(WAND_BARREL_LED_COUNT) {
    case LEDS_48:
      // Set the tip of the Frutto LED array to black.
      barrel_leds[12] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
    break;

    case LEDS_5:
    default:
      // Turn off the wand barrel tip LED.
      digitalWrite(led_barrel_tip, LOW);
    break;
  }
}

// Controlled from the Neutrona Wand EEPROM Menu system.
void toggleWandModes() {
  stopEffect(S_CLICK);
  playEffect(S_CLICK);

  stopEffect(S_VOICE_CROSS_THE_STREAMS);
  stopEffect(S_VOICE_CROSS_THE_STREAMS_MIX);
  stopEffect(S_VOICE_VIDEO_GAME_MODES);

  // Enable or disable crossing the streams / crossing the streams mix / video game modes.
  if(b_cross_the_streams == true && b_cross_the_streams_mix == true) {
    // Turn off crossing the streams mix and switch back to video game mode.
    b_cross_the_streams = false;
    b_cross_the_streams_mix = false;

    b_vg_mode = true;

    playEffect(S_VOICE_VIDEO_GAME_MODES);

    // Tell the Proton Pack to reset back to the proton stream.
    wandSerialSend(W_PROTON_MODE_REVERT);
  }
  else if(b_cross_the_streams == true && b_cross_the_streams_mix != true) {
    // Keep cross the streams on.
    b_cross_the_streams = true;

    // Turn on cross the streams mix.
    b_cross_the_streams_mix = true;

    b_vg_mode = false;

    playEffect(S_VOICE_CROSS_THE_STREAMS_MIX);

    // Tell the Proton Pack to reset back to the proton stream.
    wandSerialSend(W_RESET_PROTON_STREAM_MIX);
  }
  else {
    // Turn on crossing the streams mode and turn off video game mode.
    b_cross_the_streams = true;
    b_cross_the_streams_mix = false;

    b_vg_mode = false;

    playEffect(S_VOICE_CROSS_THE_STREAMS);

    // Tell the Proton Pack to reset back to the proton stream.
    wandSerialSend(W_RESET_PROTON_STREAM);
  }

  // Reset the previous firing mode to the proton stream.
  PREV_FIRING_MODE = PROTON;
}

// Controlled from the the Wand Sub Menu and Wand EEPROM Menu system.
void toggleOverheating() {
  if(b_overheat_enabled == true) {
    b_overheat_enabled = false;

    // Play the overheating disabled voice.
    stopEffect(S_VOICE_OVERHEAT_DISABLED);
    stopEffect(S_VOICE_OVERHEAT_ENABLED);

    playEffect(S_VOICE_OVERHEAT_DISABLED);

    // Tell the Proton Pack that overheating is disabled.
    wandSerialSend(W_OVERHEATING_DISABLED);
  }
  else {
    b_overheat_enabled = true;

    // Play the overheating enabled voice.
    stopEffect(S_VOICE_OVERHEAT_DISABLED);
    stopEffect(S_VOICE_OVERHEAT_ENABLED);

    playEffect(S_VOICE_OVERHEAT_ENABLED);

    // Tell the Proton Pack that overheating is enabled.
    wandSerialSend(W_OVERHEATING_ENABLED);
  }
}

// Overheating starting is signaled by the Neutrona Wand. However the overheating timing sequence itself it handled on the Proton Pack side.
void overheatingFinished() {
  bargraphClearAlt();

  // Since the Proton Pack tells the Neutrona Wand when overheating is finished, if it is
  // running with no Proton Pack then the Neutrona Wand needs to calculate when to finish.
  if(b_gpstar_benchtest == true) {
    ms_overheating.stop();
  }

  ms_settings_blinking.stop();

  // Turn off hat light 2.
  digitalWrite(led_hat_2, LOW);

  WAND_ACTION_STATUS = ACTION_IDLE;

  stopEffect(S_CLICK);
  stopEffect(S_VENT_DRY);

  // Prepare a few things before ramping the bargraph back up from a full ramp down.
  if(b_overheat_bargraph_blink != true) {
    if(BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
      bargraphYearModeUpdate();
    }
    else {
      i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_1984 * 2;
    }
  }

  playEffect(S_WAND_BOOTUP);

  if(switch_vent.getState() == LOW) {
    soundIdleLoop(true);
  }
  else {
    soundIdleLoop(true);

    if(switch_vent.getState() == HIGH && (getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE)) {
      afterLifeRamp1();
    }
  }

  bargraphRampUp();

  // Tell the pack that we finished overheating.
  //wandSerialSend(W_OVERHEATING_FINISHED);
}

void startVentSequence() {
  ms_overheat_initiate.stop();

  if(WAND_ACTION_STATUS == ACTION_FIRING && b_firing == true) {
    modeFireStop();
  }

  // Turn on hat light 2.
  digitalWrite(led_hat_2, HIGH);

  delay(100);

  WAND_ACTION_STATUS = ACTION_OVERHEATING;

  // Since the Proton Pack tells the Neutrona Wand when overheating is finished, if it is running with no Proton Pack then the Neutrona Wand needs to calculate when to finish.
  if(b_gpstar_benchtest == true) {
    ms_overheating.start(i_ms_overheating);
  }

  soundBeepLoopStop();
  soundIdleStop();
  soundIdleLoopStop();

  b_sound_idle = false; // REMOVE ??
  b_beeping = false;

  // Blinking bargraph option for overheat.
  if(b_overheat_bargraph_blink == true) {
    ms_bargraph.stop();

    bargraphClearAlt();

    if(b_extra_pack_sounds == true) {
      wandSerialSend(W_WAND_BEEP_SOUNDS);
      wandSerialSend(W_WAND_BEEP_BARGRAPH);
    }

    ms_settings_blinking.start(i_settings_blinking_delay);

    playEffect(S_BEEPS_LOW);
    playEffect(S_BEEPS);
    playEffect(S_BEEPS_BARGRAPH);

    ms_blink_sound_timer_1.start(i_blink_sound_timer);
    ms_blink_sound_timer_2.start(i_blink_sound_timer * 4);
  }
  else {
    // Reset some bargraph levels before we ramp the bargraph down.
    i_bargraph_status_alt = i_bargraph_segments; // For 28 segment bargraph
    i_bargraph_status = 5; // For Hasbro 5 LED bargraph.

    bargraphFull();

    ms_bargraph.start(d_bargraph_ramp_interval);
  }

  playEffect(S_VENT_DRY);

  if(b_extra_pack_sounds == true) {
    wandSerialSend(W_WAND_SHUTDOWN_SOUND);
  }

  stopEffect(S_WAND_SHUTDOWN);
  playEffect(S_WAND_SHUTDOWN);

  // Tell the pack we are overheating.
  wandSerialSend(W_OVERHEATING);
}

void settingsBlinkingLights() {
  if(ms_settings_blinking.justFinished()) {
     ms_settings_blinking.start(i_settings_blinking_delay);
  }

  if(ms_settings_blinking.remaining() < i_settings_blinking_delay / 2) {
    bool b_solid_five = false;
    bool b_solid_one = false;

    // Indicator for looping track setting.
    if(b_repeat_track == true && i_wand_menu == 5 && WAND_ACTION_STATUS != ACTION_OVERHEATING && WAND_ACTION_STATUS != ACTION_ERROR && WAND_MENU_LEVEL == MENU_LEVEL_1 && WAND_ACTION_STATUS != ACTION_LED_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU) {
      b_solid_five = true;
    }

    // Indicator for crossing the streams setting.
    if((b_cross_the_streams == true || b_cross_the_streams_mix == true) && i_wand_menu == 5 && WAND_ACTION_STATUS != ACTION_OVERHEATING && WAND_ACTION_STATUS != ACTION_ERROR && WAND_MENU_LEVEL == MENU_LEVEL_2 && WAND_ACTION_STATUS != ACTION_LED_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU) {
      b_solid_five = true;
    }

    if(i_volume_master == i_volume_abs_min && WAND_ACTION_STATUS == ACTION_SETTINGS && WAND_MENU_LEVEL == MENU_LEVEL_1) {
      b_solid_one = true;
    }

    if(b_28segment_bargraph == true) {
      if(b_solid_five == true) {
        for(uint8_t i = 0; i < 24; i++) {
          if(b_solid_one == true && i < 3) {
            ht_bargraph.setLedNow(i_bargraph[i]);
            b_bargraph_status[i] = true;
          }
          else {
            ht_bargraph.clearLedNow(i_bargraph[i]);
            b_bargraph_status[i] = false;
          }
        }

        for(uint8_t i = 24; i < i_bargraph_segments - 1; i++) {
          ht_bargraph.setLedNow(i_bargraph[i]);
          b_bargraph_status[i] = true;
        }

        ht_bargraph.clearLedNow(i_bargraph[27]);
        b_bargraph_status[27] = false;
      }
      else if(b_solid_one == true) {
        for(uint8_t i = 0; i < i_bargraph_segments; i++) {
          if(i < 3) {
            ht_bargraph.setLedNow(i_bargraph[i]);
            b_bargraph_status[i] = true;
          }
          else {
            ht_bargraph.clearLedNow(i_bargraph[i]);
            b_bargraph_status[i] = false;
          }
        }
      }
      else {
        bargraphClearAll();
      }
    }
    else {
      if(b_solid_one == true) {
        digitalWrite(i_bargraph_5_led[1-1], LOW);
      }
      else {
        digitalWrite(i_bargraph_5_led[1-1], HIGH);
      }

      digitalWrite(i_bargraph_5_led[2-1], HIGH);
      digitalWrite(i_bargraph_5_led[3-1], HIGH);
      digitalWrite(i_bargraph_5_led[4-1], HIGH);

      if(b_solid_five == true) {
        digitalWrite(i_bargraph_5_led[5-1], LOW);
      }
      else {
        digitalWrite(i_bargraph_5_led[5-1], HIGH);
      }
    }
  }
  else {
    switch(i_wand_menu) {
      case 5:
        if(b_28segment_bargraph == true) {
          // NOTE: If you draw all 28 segments at once often, you can overflow the serial buffer after around 5 seconds.
          for(uint8_t i = 0; i < i_bargraph_segments; i++) {
            switch(i) {
              case 3:
              case 4:
              case 5:
              case 9:
              case 10:
              case 11:
              case 15:
              case 16:
              case 17:
              case 21:
              case 22:
              case 23:
              case 27:
                // Nothing
              break;

              default:
                ht_bargraph.setLedNow(i_bargraph[i]);
                b_bargraph_status[i] = true;
              break;
            }
          }
        }
        else {
          wandBargraphControl(5);
        }
      break;

      case 4:
        if(b_28segment_bargraph == true) {
          for(uint8_t i = 0; i < 21; i++) {
            switch(i) {
              case 3:
              case 4:
              case 5:
              case 9:
              case 10:
              case 11:
              case 15:
              case 16:
              case 17:
              case 21:
              case 22:
              case 23:
              case 27:
                // Nothing
              break;

              default:
                ht_bargraph.setLedNow(i_bargraph[i]);
                b_bargraph_status[i] = true;
              break;
            }
          }
        }
        else {
          wandBargraphControl(4);
        }
      break;

      case 3:
        if(b_28segment_bargraph == true) {
          for(uint8_t i = 0; i < 16; i++) {
            switch(i) {
              case 3:
              case 4:
              case 5:
              case 9:
              case 10:
              case 11:
              case 15:
              case 16:
              case 17:
              case 21:
              case 22:
              case 23:
              case 27:
                // Nothing
              break;

              default:
                ht_bargraph.setLedNow(i_bargraph[i]);
                b_bargraph_status[i] = true;
              break;
            }
          }
        }
        else {
          wandBargraphControl(3);
        }
      break;

      case 2:
        if(b_28segment_bargraph == true) {
          for(uint8_t i = 0; i < 12; i++) {
            switch(i) {
              case 3:
              case 4:
              case 5:
              case 9:
              case 10:
              case 11:
              case 15:
              case 16:
              case 17:
              case 21:
              case 22:
              case 23:
              case 27:
                // Nothing
              break;

              default:
                ht_bargraph.setLedNow(i_bargraph[i]);
                b_bargraph_status[i] = true;
              break;
            }
          }
        }
        else {
          wandBargraphControl(2);
        }
      break;

      case 1:
        if(b_28segment_bargraph == true) {
          for(uint8_t i = 0; i < 6; i++) {
            switch(i) {
              case 3:
              case 4:
              case 5:
              case 9:
              case 10:
              case 11:
              case 15:
              case 16:
              case 17:
              case 21:
              case 22:
              case 23:
              case 27:
                // Nothing
              break;

              default:
                ht_bargraph.setLedNow(i_bargraph[i]);
                b_bargraph_status[i] = true;
              break;
            }
          }
        }
        else {
          wandBargraphControl(1);
        }
      break;
    }
  }
}

// Change the WAND_STATE here based on switches changing or pressed.
void checkSwitches() {
  if(ms_slo_blo_blink.justFinished()) {
    ms_slo_blo_blink.start(i_slo_blo_blink_delay);
  }

  switchBarrel(); // Determine the state of the barrel safety switch.

  switch(WAND_STATUS) {
    case MODE_OFF:
      switch(SYSTEM_MODE) {
        case MODE_ORIGINAL:
          if(b_pack_ion_arm_switch_on == true) {
            // Keep the hat lights turned off.
            digitalWrite(led_hat_1, LOW);
            digitalWrite(led_hat_2, LOW);

            if(WAND_ACTION_STATUS == ACTION_IDLE) {
              // We are going to handle the toggle switch sequence for the MODE_ORIGINAL here.
              if(switch_activate.getState() == LOW && switch_vent.getState() == LOW && switch_wand.getState() == LOW) {
                // Reset the power mode back to what it should be.
                i_power_mode = i_power_mode_prev;

                // Force to power mode 2 if it is on power mode 1 for MODE_ORIGINAL.
                if(i_power_mode < 2) {
                  i_power_mode = 2;
                  i_power_mode_prev = 2;
                }

                updatePackPowerLevel();

                // Turn on the cyclotron of the Proton Pack.
                modeActivate();
              }
              else {
                // Set the power mode to 1 (0 circle). Record the power mode so we can restore it when we power everything back up.
                if(i_power_mode != 1) {
                  i_power_mode_prev = i_power_mode;
                  i_power_mode = 1;

                  if(b_28segment_bargraph == true) {
                    bargraphPowerCheck2021Alt(false);
                  }

                  updatePackPowerLevel();
                }

                if(switch_vent.getState() == LOW) {
                  // When the bottom right toggle is on, we start flashing the slo-blo light.
                  if(ms_slo_blo_blink.remaining() < i_slo_blo_blink_delay / 2) {
                    analogWrite(led_slo_blo, 0);
                  }
                  else {
                    analogWrite(led_slo_blo, 255);
                  }
                }
                else {
                  // When the bottom right toggle is off, the slo-blo stays on.
                  analogWrite(led_slo_blo, 255);
                }

                if(switch_wand.isPressed() || switch_wand.isReleased() || switch_vent.isPressed() || switch_vent.isReleased()) {
                  if(switch_vent.isPressed() || switch_vent.isReleased()) {
                    if(switch_vent.getState() == LOW) {
                      if(b_mode_original_toggle_sounds_enabled == true) {
                        if(b_extra_pack_sounds == true) {
                          wandSerialSend(W_BEEPS_ALT);
                        }

                        stopEffect(S_BEEPS_ALT);
                        playEffect(S_BEEPS_ALT);
                      }
                    }
                  }

                  if(switch_vent.getState() == LOW && switch_wand.getState() == LOW) {
                    if(b_mode_original_toggle_sounds_enabled == true) {
                      if(b_extra_pack_sounds == true) {
                        wandSerialSend(W_MODE_ORIGINAL_HEATDOWN_STOP);
                        wandSerialSend(W_MODE_ORIGINAL_HEATUP);
                      }

                      stopEffect(S_WAND_HEATDOWN);
                      stopEffect(S_WAND_HEATUP_ALT);
                      stopEffect(S_WAND_HEATUP);
                      playEffect(S_WAND_HEATUP);
                      playEffect(S_WAND_HEATUP_ALT);
                    }

                    if(b_28segment_bargraph == true) {
                      bargraphPowerCheck2021Alt(false);
                    }

                    prepBargraphRampUp();
                  }
                  else if((switch_wand.isPressed() || switch_wand.isReleased()) && switch_vent.getState() == LOW && switch_wand.getState() == HIGH && b_mode_original_toggle_sounds_enabled == true) {
                    if(b_extra_pack_sounds == true) {
                      wandSerialSend(W_MODE_ORIGINAL_HEATUP_STOP);
                      wandSerialSend(W_MODE_ORIGINAL_HEATDOWN);
                    }

                    stopEffect(S_WAND_HEATUP_ALT);
                    stopEffect(S_WAND_HEATUP);
                    playEffect(S_WAND_HEATDOWN);
                  }
                  else if((switch_vent.isPressed() || switch_vent.isReleased()) && switch_wand.getState() == LOW && b_mode_original_toggle_sounds_enabled == true) {
                    if(b_extra_pack_sounds == true) {
                      wandSerialSend(W_MODE_ORIGINAL_HEATUP_STOP);
                      wandSerialSend(W_MODE_ORIGINAL_HEATDOWN);
                    }

                    stopEffect(S_WAND_HEATUP_ALT);
                    stopEffect(S_WAND_HEATUP);
                    playEffect(S_WAND_HEATDOWN);
                  }
                }

                if(switch_vent.getState() == LOW && switch_wand.getState() == LOW) {
                  analogWrite(led_front_left, 255); // Turn on the front left LED under the Clippard valve.

                  // Turn on the vent lights.
                  if(b_vent_light_control == true) {
                    analogWrite(led_vent, 220); // Low power, level 1 intensity.
                  }
                  else {
                    digitalWrite(led_vent, LOW);
                  }

                  digitalWrite(led_white, LOW);

                  if(ms_bargraph.justFinished()) {
                    bargraphRampUp();
                  }
                  else if(ms_bargraph.isRunning() == false && WAND_ACTION_STATUS != ACTION_FIRING && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
                    // Bargraph idling loop.
                    bargraphPowerCheck();
                  }
                }
                else {
                  if(b_28segment_bargraph == true) {
                    bargraphClearAlt();
                  }
                  else {
                    wandBargraphControl(0);
                  }

                  analogWrite(led_front_left, 0); // Turn off the front left LED under the Clippard valve.

                  // Turn off the Neutrona Wand vent lights.
                  digitalWrite(led_vent, HIGH);
                  digitalWrite(led_white, HIGH);
                }
              }
            }
          }
          else {
            if(WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_LED_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_SETTINGS) {
              wandLightsOff();
            }
          }
        break;

        case MODE_SUPER_HERO:
        default:
          if(switch_activate.isPressed() && WAND_ACTION_STATUS == ACTION_IDLE) {
              // Turn wand and pack on.
              WAND_ACTION_STATUS = ACTION_ACTIVATE;
            }
            soundBeepLoopStop();
        break;
      }
    break;

    case MODE_ERROR:
      switch(SYSTEM_MODE) {
        case MODE_ORIGINAL:
          // Nothing.
        break;

        case MODE_SUPER_HERO:
        default:
          if(switch_activate.getState() == HIGH) {
            b_wand_mash_error = false;
            wandOff();
          }
        break;
      }
    break;

    case MODE_ON:
      switch(SYSTEM_MODE) {
        case MODE_ORIGINAL:
          altWingButtonCheck();

          // Do we shut the pack and wand down if any of the right toggle switches are turned off. Activate switch control is handled in fireControlCheck();
          if(switch_vent.getState() == HIGH || switch_wand.getState() == HIGH) {
              bargraphYearModeUpdate();
              // If any of the right toggle switches are turned off, we must turn the cyclotron off and shuts the Neutrona Wand down to a off idle status. MODE_OFF.
              WAND_ACTION_STATUS = ACTION_OFF;
          }
          else {
            // Determine the light status on the wand and any beeps.
            wandLightControlCheck();
          }

          // Check if we should fire, or if the wand and pack turn off.
          fireControlCheck();
        break;

        case MODE_SUPER_HERO:
        default:
          altWingButtonCheck();

          // Determine the light status on the wand and any beeps.
          wandLightControlCheck();

          // Check if we should fire, or if the wand and pack turn off.
          fireControlCheck();
        break;
      }
    break;
  }
}

// Determine the light status on the wand and any beeps.
void wandLightControlCheck() {
  if(WAND_ACTION_STATUS != ACTION_OVERHEATING && b_pack_alarm != true) {
    // Vent light and first stage of the safety system.
    if(switch_vent.getState() == LOW) {
      if(b_vent_light_control == true) {
        // Vent light and top white light on, power dependent on mode.
        if(WAND_ACTION_STATUS == ACTION_FIRING) {
          analogWrite(led_vent, 0); // 0 = Full Power
        }
        else {
          // Adjust brightness based on the power level.
          switch(i_power_mode) {
            case 5:
              analogWrite(led_vent, 100);
            break;
            case 4:
              analogWrite(led_vent, 130);
            break;
            case 3:
              analogWrite(led_vent, 160);
            break;
            case 2:
              analogWrite(led_vent, 190);
            break;
            case 1:
              analogWrite(led_vent, 220);
            break;
          }
        }
      }
      else {
        digitalWrite(led_vent, LOW);
      }

      soundIdleStart();

      if(switch_wand.getState() == LOW) {
        if(b_beeping != true) {
          // Beep loop.
          soundBeepLoop();
        }
      }
      else {
        soundBeepLoopStop();
      }
    }
    else if(switch_vent.getState() == HIGH) {
      // Vent light and top white light off.
      digitalWrite(led_vent, HIGH);

      soundBeepLoopStop();
      soundIdleStop();
    }
  }
}

void wandOff() {
  if(WAND_ACTION_STATUS != ACTION_ERROR && b_wand_mash_error != true) {
    // Tell the pack the wand is turned off.
    wandSerialSend(W_OFF);
  }
  else {
    // Important to turn off looping on these tracks. Otherwise the bargraph beep or other can be used in the settings menu and be stuck in a loop.
    stopEffect(S_BEEPS_LOW);
    stopEffect(S_BEEPS);
    stopEffect(S_BEEPS_BARGRAPH);
  }

  if(FIRING_MODE == SETTINGS) {
    // If the wand is shut down while we are in settings mode (can happen if the pack is manually turned off), switch the wand and pack to proton mode.
    wandSerialSend(W_PROTON_MODE);
    FIRING_MODE = PROTON;
  }

  stopEffect(S_AFTERLIFE_WAND_IDLE_1);
  stopEffect(S_AFTERLIFE_WAND_IDLE_2);
  stopEffect(S_AFTERLIFE_WAND_RAMP_1);
  stopEffect(S_AFTERLIFE_WAND_RAMP_2);
  stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1);
  stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2);
  stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2_FADE_OUT);
  stopEffect(S_AFTERLIFE_WAND_RAMP_2_FADE_IN);

  b_sound_afterlife_idle_2_fade = true;

  if(b_pack_ribbon_cable_on == true) {
    switch(getNeutronaWandYearMode()) {
      case SYSTEM_1984:
      case SYSTEM_1989:
        // Nothing for now.
      break;

      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        if(WAND_ACTION_STATUS != ACTION_ERROR && b_pack_alarm != true) {
          playEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1, false, i_volume_effects - 1);

          if(b_extra_pack_sounds == true) {
            wandSerialSend(W_AFTERLIFE_GUN_RAMP_DOWN_1);
          }
        }
      break;
    }
  }

  soundBeepLoopStop();
  soundIdleStop();
  soundIdleLoopStop();

  if(b_wand_mash_error != true) {
    WAND_STATUS = MODE_OFF;
    WAND_ACTION_STATUS = ACTION_IDLE;
  }

  vibrationOff();

  // Stop firing if the wand is turned off.
  if(b_firing == true) {
    modeFireStop();
  }

  stopEffect(S_WAND_BOOTUP);
  stopEffect(S_WAND_BOOTUP_SHORT);
  stopEffect(S_GB2_WAND_START);

  if(b_extra_pack_sounds == true) {
    wandSerialSend(W_EXTRA_WAND_SOUNDS_STOP);
    wandSerialSend(W_WAND_BEEP_STOP);
  }

  // Turn off any overheating sounds.
  stopEffect(S_CLICK);
  stopEffect(S_VENT_DRY);

  stopEffect(S_FIRE_START_SPARK);
  stopEffect(S_PACK_SLIME_OPEN);
  stopEffect(S_STASIS_START);
  stopEffect(S_MESON_START);

  switch(getNeutronaWandYearMode()) {
    case SYSTEM_1984:
    case SYSTEM_1989:
      if(SYSTEM_MODE == MODE_SUPER_HERO) {
        if(switch_vent.getState() == LOW) {
          if(b_extra_pack_sounds == true) {
            wandSerialSend(W_WAND_SHUTDOWN_SOUND);
          }

          stopEffect(S_WAND_SHUTDOWN);
          playEffect(S_WAND_SHUTDOWN);
        }
      }
      else {
        if(b_extra_pack_sounds == true) {
          wandSerialSend(W_WAND_SHUTDOWN_SOUND);
        }

        stopEffect(S_WAND_SHUTDOWN);
        playEffect(S_WAND_SHUTDOWN);
      }
    break;

    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      if(b_extra_pack_sounds == true) {
        wandSerialSend(W_WAND_SHUTDOWN_SOUND);
      }

      stopEffect(S_WAND_SHUTDOWN);
      playEffect(S_WAND_SHUTDOWN);
    break;
  }

  // Clear counter until user begins firing.
  i_bmash_count = 0;

  barrelLightsOff();

  // Turn off some timers.
  ms_bargraph_firing.stop();
  ms_overheat_initiate.stop();
  ms_overheating.stop();
  ms_settings_blinking.stop();
  ms_hat_1.stop();
  ms_hat_2.stop();

  switch(WAND_STATUS) {
    case MODE_OFF:
      switch(SYSTEM_MODE) {
        case MODE_ORIGINAL:
          // Reset the bargraph speeds.
          if(BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
            bargraphYearModeUpdate();
          }
          else {
            i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_2021;
          }
        break;

        case MODE_SUPER_HERO:
        default:
          // Turn off additional timers.
          ms_bargraph.stop();
          ms_bargraph_alt.stop();

          // Turn off remaining lights.
          wandLightsOff();

          switch(getNeutronaWandYearMode()) {
            case SYSTEM_AFTERLIFE:
            case SYSTEM_FROZEN_EMPIRE:
            default:
              if(BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
                i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_2021;
              }
              else {
                i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_1984;
              }
            break;

            case SYSTEM_1984:
            case SYSTEM_1989:
              i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_1984;
            break;
          }
        break;
      }
    default:
    // Do nothing if we aren't MODE_OFF
    break;
  }

  switch_intensify.resetCount();
  switch_wand.resetCount();
  switch_vent.resetCount();

  // Start the timer for the power on indicator option.
  if(b_power_on_indicator == true) {
    ms_power_indicator.start(i_ms_power_indicator);
  }
}

// Called from checkSwitches(); Check if we should fire, or if the wand and pack turn off.
void fireControlCheck() {
  // Firing action stuff and shutting cyclotron and the neutrona wand off.
  if(WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING && b_pack_alarm != true) {
    if(i_bmash_count >= i_bmash_max) {
      // User has exceeded "normal" firing rate.
      b_wand_mash_error = true;
      modeError();
      ms_bmash.start(i_bmash_cool_down);
    }
    else {
      if(switch_intensify.getState() == LOW && switch_wand.getState() == LOW && switch_vent.getState() == LOW && switch_activate.getState() == LOW && b_pack_on == true && b_switch_barrel_extended == true && b_pack_alarm != true) {
        if(WAND_ACTION_STATUS != ACTION_FIRING) {
          WAND_ACTION_STATUS = ACTION_FIRING;
        }

        if(ms_bmash.remaining() < 1) {
          // Clear counter/timer until user begins firing.
          i_bmash_count = 0;
          ms_bmash.start(i_bmash_delay);
        }

        if(b_firing_intensify != true) {
          // Increase count each time the user presses a firing button.
          i_bmash_count++;
        }

        b_firing_intensify = true;
      }

      // When Cross The Streams mode is enabled, video game modes are disabled and the wand menu settings can only be accessed when the Neutrona Wand is powered down.
      if(b_cross_the_streams == true) {
        if(switch_mode.getState() == LOW && switch_wand.getState() == LOW && switch_vent.getState() == LOW && switch_activate.getState() == LOW && b_pack_on == true && b_switch_barrel_extended == true && b_pack_alarm != true) {
          if(WAND_ACTION_STATUS != ACTION_FIRING) {
            WAND_ACTION_STATUS = ACTION_FIRING;
          }

          if(ms_bmash.remaining() < 1) {
            // Clear counter/timer until user begins firing.
            i_bmash_count = 0;
            ms_bmash.start(i_bmash_delay);
          }

          if(b_firing_alt != true) {
            // Increase count each time the user presses a firing button.
            i_bmash_count++;
          }

          b_firing_alt = true;
        }
        else if(switch_mode.getState() == HIGH) {
          if(b_firing_intensify != true && WAND_ACTION_STATUS == ACTION_FIRING) {
            WAND_ACTION_STATUS = ACTION_IDLE;
          }

          b_firing_alt = false;
        }
      }
      else if(b_vg_mode == true) {
        if(FIRING_MODE == PROTON && WAND_ACTION_STATUS == ACTION_FIRING) {
          if(switch_mode.getState() == LOW) {
            b_firing_alt = true;

            if(ms_bmash.remaining() < 1) {
              // Clear counter/timer until user begins firing.
              i_bmash_count = 0;
              ms_bmash.start(i_bmash_delay);
            }
          }
        }
      }

      if(switch_intensify.getState() == HIGH && b_firing == true && b_firing_intensify == true) {
        if(b_firing_alt != true || b_vg_mode == true) {
          WAND_ACTION_STATUS = ACTION_IDLE;
        }

        b_firing_intensify = false;
      }
    }

    if(switch_activate.getState() == HIGH) {
      WAND_ACTION_STATUS = ACTION_OFF;
    }

    // Quick vent feature. When enabled, press intensify while the top right switch on the pack is flipped down will cause the Proton Pack and Neutrona Wand to manually vent.
    // Super Hero Mode only, because mode original uses different toggle switch combinations which makes this not possible.
    if(b_quick_vent == true && SYSTEM_MODE == MODE_SUPER_HERO) {
      if(switch_intensify.isPressed() && switch_wand.getState() == HIGH && switch_vent.getState() == LOW && switch_activate.getState() == LOW && b_pack_on == true && b_switch_barrel_extended == true && b_pack_alarm != true && b_quick_vent == true && b_overheat_enabled == true) {
        startVentSequence();
      }
    }
  }
  else if(WAND_ACTION_STATUS == ACTION_OVERHEATING || b_pack_alarm == true) {
    if(switch_activate.getState() == HIGH) {
      WAND_ACTION_STATUS = ACTION_OFF;
    }
  }
}

// Called from checkSwitches(); Check if we should fire, or if the wand and pack turn off.
void altWingButtonCheck() {
  // This is for when the Wand Barrel Switch is enabled for video game mode. b_cross_the_streams must not be enabled.
  if(WAND_ACTION_STATUS != ACTION_FIRING && WAND_ACTION_STATUS != ACTION_OFF && WAND_ACTION_STATUS != ACTION_OVERHEATING && b_cross_the_streams != true && b_cross_the_streams_mix != true && b_pack_alarm != true) {
    if(switch_mode.isPressed()) {
      // Only exit the settings menu when on menu #5 and or cycle through modes when the settings menu is on menu #5
      if(i_wand_menu == 5) {
        // Cycle through the firing modes and setting menu.
        if(FIRING_MODE == PROTON) {
          FIRING_MODE = SLIME;
        }
        else if(FIRING_MODE == SLIME) {
          FIRING_MODE = STASIS;
        }
        else if(FIRING_MODE == STASIS) {
          FIRING_MODE = MESON;
        }
        else if(FIRING_MODE == MESON) {
          // Conditional mode advancement
          if(b_spectral_mode_enabled == true) {
            FIRING_MODE = SPECTRAL;
          }
          else if(b_holiday_mode_enabled == true) {
            FIRING_MODE = HOLIDAY;
          }
          else if(b_spectral_custom_mode_enabled == true) {
            FIRING_MODE = SPECTRAL_CUSTOM;
          }
          else {
            FIRING_MODE = VENTING;
          }
        }
        else if(FIRING_MODE == SPECTRAL) {
          // Conditional mode advancement
          if(b_holiday_mode_enabled == true) {
            FIRING_MODE = HOLIDAY;
          }
          else if(b_spectral_custom_mode_enabled == true) {
            FIRING_MODE = SPECTRAL_CUSTOM;
          }
          else {
            FIRING_MODE = VENTING;
          }
        }
        else if(FIRING_MODE == HOLIDAY) {
          if(b_spectral_custom_mode_enabled == true) {
            FIRING_MODE = SPECTRAL_CUSTOM;
          }
          else {
            FIRING_MODE = VENTING;
          }
        }
        else if(FIRING_MODE == SPECTRAL_CUSTOM) {
            FIRING_MODE = VENTING;
        }
        else if(FIRING_MODE == VENTING) {
          FIRING_MODE = SETTINGS;
        }
        else {
          FIRING_MODE = PROTON;

          bargraphClearAlt();

          // If using the 28 segment bargraph with BARGRAPH_ORIGINAL, we need to redraw the segments.
          // BARGRAPH_SUPER_HERO auto ramps and do not need a manual refresh.
          if(b_28segment_bargraph == true && BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
            bargraphPowerCheck2021Alt(true);
          }
        }

        // Make sure the slo-blo light is turned back on, as entering venting mode will make it blink.
        if(FIRING_MODE != VENTING) {
          analogWrite(led_slo_blo, 255);

          // If using the gpstar Neutrona Wand microcontroller the front left LED is wired separately; let's turn it on.
          analogWrite(led_front_left, 255);
        }

        playEffect(S_CLICK);

        switch(FIRING_MODE) {
          case SETTINGS:
            WAND_ACTION_STATUS = ACTION_SETTINGS;
            i_wand_menu = 5;
            ms_settings_blinking.start(i_settings_blinking_delay);

            // Clear the 28 segment bargraph.
            bargraphClearAlt();

            // Tell the pack we are in settings mode.
            wandSerialSend(W_SETTINGS_MODE);
          break;

          case VENTING:
            WAND_ACTION_STATUS = ACTION_IDLE;
            wandHeatUp();

            // The the pack we are in venting mode.
            wandSerialSend(W_VENTING_MODE);
          break;

          case HOLIDAY:
            WAND_ACTION_STATUS = ACTION_IDLE;
            wandHeatUp();

            // Tell the pack we are in holiday mode.
            wandSerialSend(W_HOLIDAY_MODE);
          break;

          case SPECTRAL:
            WAND_ACTION_STATUS = ACTION_IDLE;
            wandHeatUp();

            // Tell the pack we are in spectral mode.
            wandSerialSend(W_SPECTRAL_MODE);
          break;

          case SPECTRAL_CUSTOM:
            WAND_ACTION_STATUS = ACTION_IDLE;
            wandHeatUp();

            // Tell the pack we are in spectral custom mode.
            wandSerialSend(W_SPECTRAL_CUSTOM_MODE);
          break;

          case MESON:
            WAND_ACTION_STATUS = ACTION_IDLE;
            wandHeatUp();

            // Tell the pack we are in meson mode.
            wandSerialSend(W_MESON_MODE);
          break;

          case STASIS:
            WAND_ACTION_STATUS = ACTION_IDLE;
            wandHeatUp();

            // Tell the pack we are in stasis mode.
            wandSerialSend(W_STASIS_MODE);
          break;

          case SLIME:
            WAND_ACTION_STATUS = ACTION_IDLE;
            wandHeatUp();

            // Tell the pack we are in slime mode.
            wandSerialSend(W_SLIME_MODE);
          break;

          case PROTON:
          default:
            WAND_ACTION_STATUS = ACTION_IDLE;
            wandHeatUp();

            // Tell the pack we are in proton mode.
            wandSerialSend(W_PROTON_MODE);
          break;
        }
      }
    }
  }
}

void modeError() {
  wandOff();

  WAND_STATUS = MODE_ERROR;
  WAND_ACTION_STATUS = ACTION_ERROR;

  ms_hat_2.start(i_hat_2_delay);

  // This is used for controlling a bargraph beep in a boot up error.
  ms_hat_1.start(i_hat_2_delay * 4);

  ms_settings_blinking.start(i_settings_blinking_delay);

  if(b_extra_pack_sounds == true) {
    wandSerialSend(W_WAND_BEEP_BARGRAPH);
    wandSerialSend(W_WAND_BEEP_SOUNDS);
  }

  playEffect(S_BEEPS_LOW);
  playEffect(S_BEEPS);
  playEffect(S_BEEPS_BARGRAPH);
}

void modeActivate() {
  b_sound_afterlife_idle_2_fade = true;

  switch(SYSTEM_MODE) {
    case MODE_ORIGINAL:
      b_wand_mash_error = false;
      i_bmash_count = 0;

      WAND_STATUS = MODE_ON;
      WAND_ACTION_STATUS = ACTION_IDLE;

      // If starting up directly from any of the non-toggle-sequence switches, play the wand heatup sound.
      if(switch_activate.isPressed() != true && switch_activate.isReleased() != true && b_mode_original_toggle_sounds_enabled == true) {
        if(b_extra_pack_sounds == true) {
          wandSerialSend(W_MODE_ORIGINAL_HEATUP);
        }

        stopEffect(S_WAND_HEATUP_ALT);
        stopEffect(S_WAND_HEATUP);
        playEffect(S_WAND_HEATUP);
        playEffect(S_WAND_HEATUP_ALT);
      }

      wandSerialSend(W_ON);

      postActivation();
    break;

    case MODE_SUPER_HERO:
    default:
      // The wand was started while the top switch was already on, so let's put the wand into a startup error mode.
      if(switch_wand.getState() == LOW && b_wand_boot_errors == true) {
        b_wand_mash_error = true;
        modeError();
      }
      else {
        WAND_STATUS = MODE_ON;

        // Proper startup. Continue booting up the wand.
        WAND_ACTION_STATUS = ACTION_IDLE;

        // Tell the pack the wand is turned on.
        wandSerialSend(W_ON);

        // Clear counter until user begins firing.
        i_bmash_count = 0;
      }

      b_wand_mash_error = false;

      postActivation(); // Enable lights and bargraph after wand activation.
    break;
  }
}

void postActivation() {
  if(BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
    i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_2021;
  }
  else {
    i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_1984 * 2;
  }

  if(WAND_STATUS != MODE_ERROR) {
    if(b_pack_alarm != true) {
      switch(SYSTEM_MODE) {
        case MODE_ORIGINAL:
          if(ms_bargraph.isRunning() != true) {
            // Ramp up the bargraph if required. Usually if everything is turned on via the top right toggle switch.
            bargraphRampUp();
          }
        break;

        case MODE_SUPER_HERO:
          bargraphRampUp();
          if(switch_vent.getState() == LOW) {
            b_all_switch_activation = true; // If vent switch is already on when Activate is flipped, set to true for soundIdleLoop() to use
          }
        break;
      }
    }

    // Turn on slo-blo light.
    analogWrite(led_slo_blo, 255);

    // Turn on the Clippard LED.
    analogWrite(led_front_left, 255);

    // Top white light.
    ms_white_light.start(d_white_light_interval);
    digitalWrite(led_white, LOW);

    if(b_pack_alarm != true) {
      switch(getNeutronaWandYearMode()) {
        case SYSTEM_1984:
        case SYSTEM_1989:
          stopEffect(S_WAND_BOOTUP_SHORT);
          playEffect(S_WAND_BOOTUP_SHORT);
        break;

        case SYSTEM_AFTERLIFE:
        case SYSTEM_FROZEN_EMPIRE:
        default:
          if(b_gpstar_benchtest == true) {
            stopEffect(S_WAND_BOOTUP);
            playEffect(S_WAND_BOOTUP);
          }

          soundIdleLoop(true);

          if(switch_vent.getState() == HIGH && b_pack_ribbon_cable_on == true) {
            afterLifeRamp1();
          }
        break;
      }
    }
  }
}

void soundIdleLoop(bool fadeIn) {
  switch(i_power_mode) {
    case 1:
      playEffect(S_IDLE_LOOP_GUN_1, true, i_volume_effects, fadeIn, 1000);
    break;

    case 2:
      playEffect(S_IDLE_LOOP_GUN_1, true, i_volume_effects, fadeIn, 1000);
    break;

    case 3:
      playEffect(S_IDLE_LOOP_GUN_2, true, i_volume_effects, fadeIn, 1000);
    break;

    case 4:
      playEffect(S_IDLE_LOOP_GUN_2, true, i_volume_effects, fadeIn, 1000);
    break;

    case 5:
      playEffect(S_IDLE_LOOP_GUN_5, true, i_volume_effects, fadeIn, 1000);
    break;
  }
}

void soundIdleLoopStop() {
  stopEffect(S_IDLE_LOOP_GUN);
  stopEffect(S_IDLE_LOOP_GUN_1);
  stopEffect(S_IDLE_LOOP_GUN_2);
  stopEffect(S_IDLE_LOOP_GUN_3);
  stopEffect(S_IDLE_LOOP_GUN_4);
  stopEffect(S_IDLE_LOOP_GUN_5);
}

void soundIdleStart() {
  if(b_sound_idle == false) {
    switch(getNeutronaWandYearMode()) {
      case SYSTEM_1984:
      case SYSTEM_1989:
        if(b_extra_pack_sounds == true && switch_vent.getState() == LOW && (switch_vent.isPressed() || switch_vent.isReleased())) {
          wandSerialSend(W_WAND_BOOTUP_SOUND);
        }

        if(getNeutronaWandYearMode() == SYSTEM_1989 && b_gpstar_benchtest == true) {
          stopEffect(S_WAND_BOOTUP);
          stopEffect(S_WAND_BOOTUP_SHORT);
          stopEffect(S_GB2_WAND_START);
          playEffect(S_GB2_WAND_START);
        }
        else if(b_all_switch_activation == true) {
          stopEffect(S_WAND_BOOTUP);
          stopEffect(S_WAND_BOOTUP_SHORT);
          playEffect(S_WAND_BOOTUP_SHORT);
        }
        else {
          stopEffect(S_WAND_BOOTUP);
          stopEffect(S_WAND_BOOTUP_SHORT);
          playEffect(S_WAND_BOOTUP);
        }

        soundIdleLoop(true);

        b_sound_idle = true;
      break;

      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        stopEffect(S_AFTERLIFE_WAND_RAMP_1);
        stopEffect(S_AFTERLIFE_WAND_IDLE_2);
        stopEffect(S_AFTERLIFE_WAND_IDLE_1);
        stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1);
        stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2);
        stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2_FADE_OUT);

        if(b_pack_ribbon_cable_on == true) {
          if(b_sound_afterlife_idle_2_fade == true) {
            playEffect(S_AFTERLIFE_WAND_RAMP_2_FADE_IN, false, i_volume_effects - 1);

            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_EXTRA_WAND_SOUNDS_STOP);

              wandSerialSend(W_AFTERLIFE_GUN_RAMP_2_FADE_IN);
            }

            b_sound_afterlife_idle_2_fade = false;
          }
          else {
            playEffect(S_AFTERLIFE_WAND_RAMP_2, false, i_volume_effects - 1);

            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_EXTRA_WAND_SOUNDS_STOP);

              wandSerialSend(W_AFTERLIFE_GUN_RAMP_2);
            }
          }

          ms_gun_loop_2.start(1500);

          b_sound_idle = true;
        }

        ms_gun_loop_1.stop();
      break;
    }
  }

  if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
    if(ms_gun_loop_2.justFinished()) {
      playEffect(S_AFTERLIFE_WAND_IDLE_2, true, i_volume_effects - 1);

      ms_gun_loop_2.stop();

      if(b_extra_pack_sounds == true) {
        wandSerialSend(W_AFTERLIFE_GUN_LOOP_2);
      }
    }
  }

  b_all_switch_activation = false;
}

void soundIdleStop() {
  if(b_sound_idle == true) {
    switch(getNeutronaWandYearMode()) {
      case SYSTEM_1984:
      case SYSTEM_1989:
        if(WAND_ACTION_STATUS != ACTION_OFF && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          if(b_extra_pack_sounds == true) {
            wandSerialSend(W_WAND_SHUTDOWN_SOUND);
          }

          stopEffect(S_WAND_SHUTDOWN);
          playEffect(S_WAND_SHUTDOWN);
        }
      break;

      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        if(b_pack_ribbon_cable_on == true) {
          if(WAND_ACTION_STATUS == ACTION_OVERHEATING || b_pack_alarm == true) {
            //stopEffect(S_WAND_SHUTDOWN);
            //playEffect(S_WAND_SHUTDOWN);

            playEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2_FADE_OUT, false, i_volume_effects - 1);

            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_AFTERLIFE_GUN_RAMP_DOWN_2_FADE_OUT);
            }
          }
          else if(WAND_ACTION_STATUS != ACTION_OFF) {
            playEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2, false, i_volume_effects - 1);

            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_AFTERLIFE_GUN_RAMP_DOWN_2);
            }
          }

          if(WAND_ACTION_STATUS != ACTION_OVERHEATING) {
            ms_gun_loop_1.start(1660);
            ms_gun_loop_2.stop();
          }
        }
      break;
    }
  }

  if(b_sound_idle == true) {
    switch(getNeutronaWandYearMode()) {
      case SYSTEM_1984:
      case SYSTEM_1989:
        stopEffect(S_WAND_BOOTUP);
        stopEffect(S_WAND_BOOTUP_SHORT);
        stopEffect(S_GB2_WAND_START);
        soundIdleLoopStop();
      break;

      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        stopEffect(S_AFTERLIFE_WAND_RAMP_2);
        stopEffect(S_AFTERLIFE_WAND_IDLE_2);

        if(b_extra_pack_sounds == true) {
          wandSerialSend(W_AFTERLIFE_RAMP_LOOP_2_STOP);
        }
      break;
    }
  }

  b_sound_idle = false;
}

void soundBeepLoopStop() {
  if(b_beeping == true) {
    b_beeping = false;

    if(b_extra_pack_sounds == true) {
      wandSerialSend(W_WAND_BEEP_STOP);
    }

    stopEffect(S_AFTERLIFE_BEEP_WAND_S1);
    stopEffect(S_AFTERLIFE_BEEP_WAND_S2);
    stopEffect(S_AFTERLIFE_BEEP_WAND_S3);
    stopEffect(S_AFTERLIFE_BEEP_WAND_S4);
    stopEffect(S_AFTERLIFE_BEEP_WAND_S5);

    ms_reset_sound_beep.stop();
    ms_reset_sound_beep.start(i_sound_timer);
  }
}

void soundBeepLoop() {
  if(ms_reset_sound_beep.justFinished() && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
    if(b_beeping == false) {
      // Quick check to know if effects belong to the next-gen movies (as opposed to the OG 80's themes).
      bool b_next_gen = (getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE);

      switch(i_power_mode) {
        case 1:
          if(b_next_gen && b_beep_loop == true) {
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_WAND_BEEP_START);
            }

            playEffect(S_AFTERLIFE_BEEP_WAND_S1, true);
          }
          else {
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_WAND_BEEP);
            }

            playEffect(S_AFTERLIFE_BEEP_WAND_S1);
          }
        break;

        case 2:
         if(b_next_gen && b_beep_loop == true) {
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_WAND_BEEP_START);
            }

            playEffect(S_AFTERLIFE_BEEP_WAND_S2, true);
          }
          else {
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_WAND_BEEP);
            }

            playEffect(S_AFTERLIFE_BEEP_WAND_S2);
          }
        break;

        case 3:
         if(b_next_gen && b_beep_loop == true) {
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_WAND_BEEP_START);
            }

            playEffect(S_AFTERLIFE_BEEP_WAND_S3, true);
          }
          else {
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_WAND_BEEP);
            }

            playEffect(S_AFTERLIFE_BEEP_WAND_S3);
          }
        break;

        case 4:
         if(b_next_gen && b_beep_loop == true) {
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_WAND_BEEP_START);
            }

            playEffect(S_AFTERLIFE_BEEP_WAND_S4, true);
          }
          else {
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_WAND_BEEP);
            }

            playEffect(S_AFTERLIFE_BEEP_WAND_S4);
          }
        break;

        case 5:
         if(b_next_gen && b_beep_loop == true) {
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_WAND_BEEP_START);
            }

            playEffect(S_AFTERLIFE_BEEP_WAND_S5, true);
          }
          else {
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_WAND_BEEP);
            }

            playEffect(S_AFTERLIFE_BEEP_WAND_S5);
          }
        break;
      }

      b_beeping = true;

      ms_reset_sound_beep.stop();
    }
  }
}

void modeFireStartSounds() {
  ms_firing_start_sound_delay.stop();

  if(FIRING_MODE != MESON) {
    // Some sparks for firing start.
    if(getSystemYearMode() == SYSTEM_1989) {
      playEffect(S_FIRE_START_SPARK, false, i_volume_effects - 10);
    }
    else {
      playEffect(S_FIRE_START_SPARK);
    }
  }

  switch(FIRING_MODE) {
    case PROTON:
    default:
        playEffect(S_FIRE_START);

        switch(i_power_mode) {
          case 1 ... 4:
            if(b_firing_intensify == true) {
              // Reset some sound triggers.
              b_sound_firing_intensify_trigger = true;

              if(getSystemYearMode() == SYSTEM_1989) {
                playEffect(S_GB2_FIRE_START);
                playEffect(S_GB2_FIRE_LOOP, true, i_volume_effects, true, 6500);
              }
              else {
                playEffect(S_GB1_FIRE_START);
                playEffect(S_GB1_FIRE_LOOP, true, i_volume_effects, true, 1000);
              }
            }
            else {
              b_sound_firing_intensify_trigger = false;
            }

            if(b_firing_alt == true) {
              // Reset some sound triggers.
              b_sound_firing_alt_trigger = true;

              if(getSystemYearMode() == SYSTEM_1989) {
                playEffect(S_GB2_FIRE_START);
              }

              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 1000);
            }
            else {
              b_sound_firing_alt_trigger = false;
            }
          break;

          case 5:
            switch(getSystemYearMode()) {
              case SYSTEM_1989:
                playEffect(S_GB2_FIRE_START);
              break;

              case SYSTEM_1984:
                playEffect(S_GB1_FIRE_START_HIGH_POWER, false, i_volume_effects);
                playEffect(S_GB1_FIRE_START);
              break;

              case SYSTEM_AFTERLIFE:
              case SYSTEM_FROZEN_EMPIRE:
              default:
                playEffect(S_AFTERLIFE_FIRE_START, false, i_volume_effects + 2);
              break;
            }

            if(b_firing_intensify == true) {
              // Reset some sound triggers.
              b_sound_firing_intensify_trigger = true;
              playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, true, 700);
            }
            else {
              b_sound_firing_intensify_trigger = false;
            }

            if(b_firing_alt == true) {
              // Reset some sound triggers.
              b_sound_firing_alt_trigger = true;

              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 700);
            }
            else {
              b_sound_firing_alt_trigger = false;
            }

            //playEffect(S_GB1_FIRE_START_HIGH_POWER);
          break;
        }
    break;

    case SLIME:
      stopEffect(S_SLIME_END);
      playEffect(S_SLIME_START);
      playEffect(S_SLIME_LOOP, true);
    break;

    case STASIS:
      stopEffect(S_STASIS_END);
      playEffect(S_STASIS_START);
      playEffect(S_STASIS_LOOP, true);
    break;

    case MESON:
      playEffect(S_MESON_START);

      playEffect(S_MESON_FIRE_PULSE);

      switch(i_power_mode) {
        case 5:
          ms_meson_blast.start(i_meson_blast_delay_level_5);
        break;

        case 4:
          ms_meson_blast.start(i_meson_blast_delay_level_5);
        break;

        case 3:
          ms_meson_blast.start(i_meson_blast_delay_level_3);
        break;

        case 2:
          ms_meson_blast.start(i_meson_blast_delay_level_2);
        break;

        case 1:
        default:
          ms_meson_blast.start(i_meson_blast_delay_level_1);
        break;
      }
    break;

    case VENTING:
    case SETTINGS:
      // Nothing.
    break;
  }
}

void modeFireStart() {
  // Reset some sound triggers.
  b_sound_firing_intensify_trigger = true;
  b_sound_firing_alt_trigger = true;
  b_sound_firing_cross_the_streams_mix = false;
  b_sound_firing_cross_the_streams = false;
  b_firing_cross_streams = false;

  switch(BARGRAPH_FIRING_ANIMATION) {
    case BARGRAPH_ANIMATION_ORIGINAL:
      // Redraw the bargraph to the current power level before doing the MODE_ORIGINAL firing animation.
      bargraphRedraw();

      // Reset the Hasbro bargraph.
      i_bargraph_status = 0;
    break;

    case BARGRAPH_ANIMATION_SUPER_HERO:
    default:
      // Clear the bargraph before we do the animation.
      bargraphClearAlt();

      // Reset the Hasbro bargraph.
      i_bargraph_status = 1;
    break;
  }

  // Turn on hat light 1.
  digitalWrite(led_hat_1, HIGH);

  ms_hat_1.stop();

  // Tell the Proton Pack that the Neutrona Wand is firing in Intensify mode.
  if(b_firing_intensify == true) {
    wandSerialSend(W_FIRING_INTENSIFY);
  }

  // Tell the Proton Pack that the Neutrona Wand is firing in Alt mode.
  if(b_firing_alt == true) {
    wandSerialSend(W_FIRING_ALT);
  }

  // Stop all firing sounds first.
  switch(FIRING_MODE) {
    case PROTON:
    default:
      if(getSystemYearMode() == SYSTEM_1989) {
        stopEffect(S_GB2_FIRE_START);
        stopEffect(S_GB2_FIRE_LOOP);
      }
      else {
        stopEffect(S_GB1_FIRE_START);
        stopEffect(S_GB1_FIRE_LOOP);
      }

      stopEffect(S_GB1_FIRE_START_HIGH_POWER);
      stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);

      stopEffect(S_FIRING_LOOP_GB1);

      stopEffect(S_FIRE_START_SPARK);
      stopEffect(S_FIRING_END_GUN);

      stopEffect(S_FIRE_LOOP_IMPACT);
    break;

    case SLIME:
      stopEffect(S_SLIME_START);
      stopEffect(S_SLIME_LOOP);
      stopEffect(S_SLIME_END);
    break;

    case STASIS:
      stopEffect(S_STASIS_START);
      stopEffect(S_STASIS_LOOP);
      stopEffect(S_STASIS_END);
    break;

    case MESON:
      stopEffect(S_MESON_START);
      stopEffect(S_MESON_END);
    break;

    case VENTING:
    case SETTINGS:
      // Nothing.
    break;
  }

  ms_firing_start_sound_delay.start(i_fire_stop_sound_delay);

  // Tell the pack the wand is firing.
  wandSerialSend(W_FIRING);

  ms_overheat_initiate.stop();

  // This will only overheat when enabled by using the alt firing when in crossing the streams mode.
  bool b_overheat_flag = true;

  if(b_cross_the_streams == true && b_firing_alt != true) {
    b_overheat_flag = false;
  }

  if(b_overheat_flag == true) {
    // If in high power mode on the wand, start an overheat timer.
    if(b_overheat_mode[i_power_mode - 1] == true && b_overheat_enabled == true) {
      ms_overheat_initiate.start(i_ms_overheat_initiate[i_power_mode - 1]);
    }
    else if(b_cross_the_streams == true) {
      if(b_firing_alt == true) {
        ms_overheat_initiate.start(i_ms_overheat_initiate[i_power_mode - 1]);
      }
    }
  }

  barrelLightsOff();

  if(FIRING_MODE == MESON) {
    ms_firing_lights.stop();
    ms_firing_stream_effects.start(1);
  }
  else {
    ms_firing_lights.start(10);
  }

  i_barrel_light = 0;

  // Stop any bargraph ramps.
  ms_bargraph.stop();

  ms_bargraph_alt.stop();

  // Reset the 28 segment bargraph.
  i_bargraph_status_alt = 0;

  b_bargraph_up = false;

  bargraphRampFiring();

  ms_impact.start(random(10,15) * 1000);
}

void modeFireStopSounds() {
  // Reset some sound triggers.
  b_sound_firing_intensify_trigger = false;
  b_sound_firing_alt_trigger = false;
  b_sound_firing_cross_the_streams = false;
  b_sound_firing_cross_the_streams_mix = false;

  ms_firing_stop_sound_delay.stop();

 switch(FIRING_MODE) {
    case PROTON:
    default:
      playEffect(S_FIRING_END_GUN);
    break;

    case SLIME:
      playEffect(S_SLIME_END);
    break;

    case STASIS:
      playEffect(S_STASIS_END);
    break;

    case MESON:
      playEffect(S_MESON_END);
    break;

    case VENTING:
    case SETTINGS:
      // Nothing.
    break;
  }

  if(b_firing_cross_streams == true) {
    switch(WAND_YEAR_CTS) {
      case CTS_AFTERLIFE:
        stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
        stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);

        playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects + 10);
      break;

      case CTS_1984:
      case CTS_1989:
        stopEffect(S_CROSS_STREAMS_START);
        stopEffect(S_CROSS_STREAMS_END);

        playEffect(S_CROSS_STREAMS_END, false, i_volume_effects + 10);
      break;

      case CTS_DEFAULT:
      case CTS_FROZEN_EMPIRE:
      default:
        switch(getSystemYearMode()) {
          case SYSTEM_AFTERLIFE:
          case SYSTEM_FROZEN_EMPIRE:
          default:
            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);

            playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects + 10);
          break;

          case SYSTEM_1984:
          case SYSTEM_1989:
            stopEffect(S_CROSS_STREAMS_START);
            stopEffect(S_CROSS_STREAMS_END);

            playEffect(S_CROSS_STREAMS_END, false, i_volume_effects + 10);
          break;
        }
      break;
    }


    b_firing_cross_streams = false;
  }
}

void modeFireStop() {
  ms_overheat_initiate.stop();

  // Tell the pack the wand stopped firing.
  wandSerialSend(W_FIRING_STOPPED);

  WAND_ACTION_STATUS = ACTION_IDLE;

  b_firing = false;
  b_firing_intensify = false;
  b_firing_alt = false;

  ms_bargraph_firing.stop();

  ms_bargraph_alt.stop(); // Stop the 1984 24 segment optional bargraph timer.
  b_bargraph_up = false;

  switch(BARGRAPH_MODE) {
    case BARGRAPH_ORIGINAL:
      // Need to restart the regular bargraph timer.
      i_bargraph_status = i_power_mode - 1;
      i_bargraph_status_alt = 0;

      switch(BARGRAPH_FIRING_ANIMATION) {
        case BARGRAPH_ANIMATION_ORIGINAL:
          // Reset and redraw all the proper segments for the bargraph.
          bargraphRedraw();

          // Restart the bargraph idling loop.
          bargraphPowerCheck();
        break;

        case BARGRAPH_ANIMATION_SUPER_HERO:
        default:
          bargraphClearAlt();

          i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_2021 / 3;

          bargraphRampUp();
        break;
      }

      if(b_pack_alarm == true) {
        // We are going to ramp the bargraph down if the pack alarm happens while we were firing.
        prepBargraphRampDown();
      }
    break;

    case BARGRAPH_SUPER_HERO:
    default:
      i_bargraph_status = i_power_mode - 1;

      i_bargraph_status_alt = 0;
      bargraphClearAlt();

      i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_1984;

      if(b_pack_alarm == true) {
        // We are going to ramp the bargraph down if the pack alarm happens while we were firing.
        prepBargraphRampDown();
      }
      else {
        // We ramp the bargraph back up after finishing firing.
        bargraphRampUp();
      }
    break;
  }

  ms_firing_stream_effects.stop();
  ms_firing_lights.stop();

  ms_impact.stop();

  i_barrel_light = 0;
  ms_firing_lights_end.start(10);

  // If using optional items on the gpstar Neutrona Wand microcontroller.
  digitalWrite(led_hat_1, LOW); // Turn off hat light 1.
  wandTipOff();

  ms_hat_1.stop();

  // Stop all other firing sounds.
  switch(FIRING_MODE) {
    case PROTON:
    default:
      if(getSystemYearMode() == SYSTEM_1989) {
        stopEffect(S_GB2_FIRE_START);
        stopEffect(S_GB2_FIRE_LOOP);
      }
      else {
        stopEffect(S_GB1_FIRE_START);
        stopEffect(S_GB1_FIRE_LOOP);
      }

      stopEffect(S_FIRING_LOOP_GB1);
      stopEffect(S_GB1_FIRE_START_HIGH_POWER);
      stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);
      stopEffect(S_FIRE_START_SPARK);

      stopEffect(S_FIRE_LOOP_IMPACT);
    break;

    case SLIME:
      stopEffect(S_SLIME_START);
      stopEffect(S_SLIME_LOOP);
      stopEffect(S_SLIME_END);
    break;

    case STASIS:
      stopEffect(S_STASIS_START);
      stopEffect(S_STASIS_LOOP);
      stopEffect(S_STASIS_END);
    break;

    case MESON:
      stopEffect(S_MESON_START);
      stopEffect(S_MESON_END);
    break;

    case VENTING:
    case SETTINGS:
      // Nothing.
    break;
  }

  // A tiny ramp down delay helps with the sounds.
  ms_firing_stop_sound_delay.start(i_fire_stop_sound_delay);
}

void modeFiring() {
  // Sound trigger flags.
  if(b_firing_intensify == true && b_sound_firing_intensify_trigger != true) {
    b_sound_firing_intensify_trigger = true;

    if((b_cross_the_streams_mix == true || b_vg_mode == true) && FIRING_MODE == PROTON) {
      // Tell the Proton Pack that the Neutrona Wand is firing in Intensify mode mix.
      wandSerialSend(W_FIRING_INTENSIFY_MIX);

      switch(i_power_mode) {
        case 1 ... 4:
          if(getSystemYearMode() == SYSTEM_1989) {
            playEffect(S_GB2_FIRE_START);
            playEffect(S_GB2_FIRE_LOOP, true);
          }
          else {
            playEffect(S_GB1_FIRE_START);
            playEffect(S_GB1_FIRE_LOOP, true);
          }
        break;

        case 5:
            playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true);
        break;
      }
    }
    else {
      // Tell the Proton Pack that the Neutrona Wand is firing in Intensify mode.
      wandSerialSend(W_FIRING_INTENSIFY);
    }
  }

  if(b_firing_intensify != true && b_sound_firing_intensify_trigger == true) {
    b_sound_firing_intensify_trigger = false;

    if((b_cross_the_streams_mix == true || b_vg_mode == true) && FIRING_MODE == PROTON) {
      // Tell the Proton Pack that the Neutrona Wand is no longer firing in Intensify mode mix.
      wandSerialSend(W_FIRING_INTENSIFY_STOPPED_MIX);

      switch(i_power_mode) {
        case 1 ... 4:
          if(getSystemYearMode() == SYSTEM_1989) {
            stopEffect(S_GB2_FIRE_LOOP);
            stopEffect(S_GB2_FIRE_START);
          }
          else {
            stopEffect(S_GB1_FIRE_LOOP);
            stopEffect(S_GB1_FIRE_LOOP);
            stopEffect(S_GB1_FIRE_START);
          }
        break;

        case 5:
          stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);
          stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);
        break;
      }
    }
    else {
      // Tell the Proton Pack that the Neutrona Wand is no longer firing in Intensify mode.
      wandSerialSend(W_FIRING_INTENSIFY_STOPPED);
    }
  }

  if(b_firing_alt == true && b_sound_firing_alt_trigger != true) {
    b_sound_firing_alt_trigger = true;

    if(b_cross_the_streams_mix == true || b_vg_mode == true) {
      playEffect(S_FIRING_LOOP_GB1, true);

      // Tell the Proton Pack that the Neutrona Wand is firing in Alt mode mix.
      wandSerialSend(W_FIRING_ALT_MIX);
    }
    else {
      // Tell the Proton Pack that the Neutrona Wand is firing in Alt mode.
      wandSerialSend(W_FIRING_ALT);
    }
  }

  if(b_firing_alt != true && b_sound_firing_alt_trigger == true) {
    b_sound_firing_alt_trigger = false;

    if(b_cross_the_streams_mix == true || b_vg_mode == true) {
      stopEffect(S_FIRING_LOOP_GB1);

      // Tell the Proton Pack that the Neutrona Wand is no longer firing in Alt mode mix.
      wandSerialSend(W_FIRING_ALT_STOPPED_MIX);
    }
    else {
      // Tell the Proton Pack that the Neutrona Wand is no longer firing in Alt mode.
      wandSerialSend(W_FIRING_ALT_STOPPED);
    }
  }

  if(b_firing_alt == true && b_firing_intensify == true && b_sound_firing_cross_the_streams != true && b_firing_cross_streams != true) {
    b_firing_cross_streams = true;
    b_sound_firing_cross_the_streams = true;

    switch(WAND_YEAR_CTS) {
      case CTS_AFTERLIFE:
        stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);
        stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);

        playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START, false, i_volume_effects + 10);

        if(b_cross_the_streams_mix == true) {
          // Tell the Proton Pack that the Neutrona Wand is crossing the streams mix.
          wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_MIX_2021);
        }
        else {
          // Tell the Proton Pack that the Neutrona Wand is crossing the streams.
          wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_2021);
        }
      break;

      case CTS_1984:
      case CTS_1989:
        stopEffect(S_CROSS_STREAMS_END);
        stopEffect(S_CROSS_STREAMS_START);

        playEffect(S_CROSS_STREAMS_START, false, i_volume_effects + 10);

        if(b_cross_the_streams_mix == true) {
          // Tell the Proton Pack that the Neutrona Wand is crossing the streams mix.
          wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_MIX_1984);
        }
        else {
          // Tell the Proton Pack that the Neutrona Wand is crossing the streams.
          wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_1984);
        }
      break;

      case CTS_DEFAULT:
      case CTS_FROZEN_EMPIRE:
      default:
        switch(getSystemYearMode()) {
          case SYSTEM_AFTERLIFE:
          case SYSTEM_FROZEN_EMPIRE:
          default:
            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);
            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);

            playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START, false, i_volume_effects + 10);

            if(b_cross_the_streams_mix == true) {
              // Tell the Proton Pack that the Neutrona Wand is crossing the streams mix.
              wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_MIX_2021);
            }
            else {
              // Tell the Proton Pack that the Neutrona Wand is crossing the streams.
              wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_2021);
            }
          break;

          case SYSTEM_1984:
          case SYSTEM_1989:
            stopEffect(S_CROSS_STREAMS_END);
            stopEffect(S_CROSS_STREAMS_START);

            playEffect(S_CROSS_STREAMS_START, false, i_volume_effects + 10);

            if(b_cross_the_streams_mix == true) {
              // Tell the Proton Pack that the Neutrona Wand is crossing the streams mix.
              wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_MIX_1984);
            }
            else {
              // Tell the Proton Pack that the Neutrona Wand is crossing the streams.
              wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_1984);
            }
          break;
        }
      break;
    }

    playEffect(S_FIRE_START_SPARK);

    if(b_cross_the_streams_mix == true || b_vg_mode == true) {
      playEffect(S_FIRING_LOOP_GB1, true);

      if(i_power_mode != i_power_mode_max && b_sound_firing_cross_the_streams_mix != true) {
        playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true);
        b_sound_firing_cross_the_streams_mix = true;
      }

      stopEffect(S_GB2_FIRE_LOOP);
      stopEffect(S_GB1_FIRE_LOOP);
    }
  }

  if((b_firing_alt != true && b_firing_intensify != true) && b_firing_cross_streams == true && b_cross_the_streams_mix != true) {
    // Can let go of a button and still fires.
    b_firing_cross_streams = false;
    b_sound_firing_cross_the_streams = false;

    switch(WAND_YEAR_CTS) {
      case CTS_AFTERLIFE:
        stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
        stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);

        playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects + 10);

        wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_STOPPED_2021);
      break;

      case CTS_1984:
      case CTS_1989:
        stopEffect(S_CROSS_STREAMS_START);
        stopEffect(S_CROSS_STREAMS_END);

        playEffect(S_CROSS_STREAMS_END, false, i_volume_effects + 10);

        wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_STOPPED_1984);
      break;

      case CTS_DEFAULT:
      case CTS_FROZEN_EMPIRE:
      default:
        switch(getSystemYearMode()) {
          case SYSTEM_AFTERLIFE:
          case SYSTEM_FROZEN_EMPIRE:
          default:
            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);

            playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects + 10);

            wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_STOPPED_2021);
          break;

          case SYSTEM_1984:
          case SYSTEM_1989:
            stopEffect(S_CROSS_STREAMS_START);
            stopEffect(S_CROSS_STREAMS_END);

            playEffect(S_CROSS_STREAMS_END, false, i_volume_effects + 10);

            wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_STOPPED_1984);
          break;
        }
      break;
    }

    stopEffect(S_FIRING_LOOP_GB1);
  }
  else if((b_firing_alt != true || b_firing_intensify != true) && b_firing_cross_streams == true && (b_cross_the_streams_mix == true || b_vg_mode == true)) {
    // Let go of a button and it reverts back to the other firing mode.
    b_firing_cross_streams = false;
    b_sound_firing_cross_the_streams = false;

    switch(WAND_YEAR_CTS) {
      case CTS_AFTERLIFE:
        stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
        stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);

        playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects + 10);

        wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_STOPPED_MIX_2021);
      break;

      case CTS_1984:
      case CTS_1989:
        stopEffect(S_CROSS_STREAMS_START);
        stopEffect(S_CROSS_STREAMS_END);

        playEffect(S_CROSS_STREAMS_END, false, i_volume_effects + 10);

        wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_STOPPED_MIX_1984);
      break;

      case CTS_DEFAULT:
      case CTS_FROZEN_EMPIRE:
      default:
        switch(getSystemYearMode()) {
          case SYSTEM_AFTERLIFE:
          case SYSTEM_FROZEN_EMPIRE:
          default:
            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);

            playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects + 10);

            wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_STOPPED_MIX_2021);
          break;

          case SYSTEM_1984:
          case SYSTEM_1989:
            stopEffect(S_CROSS_STREAMS_START);
            stopEffect(S_CROSS_STREAMS_END);

            playEffect(S_CROSS_STREAMS_END, false, i_volume_effects + 10);

            wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_STOPPED_MIX_1984);
          break;
        }
      break;
    }
  }

  // Overheat timers.
  bool b_overheat_flag = true;

  if(b_cross_the_streams == true && b_firing_alt != true) {
    b_overheat_flag = false;
  }

  if(b_overheat_flag == true) {
    // If the user changes the wand power output while firing, turn off the overheat timer.
    if(b_overheat_mode[i_power_mode - 1] != true && ms_overheat_initiate.isRunning()) {
      ms_overheat_initiate.stop();

      // Adjust hat light 1 to stay solid.
      digitalWrite(led_hat_1, HIGH);

      ms_hat_1.stop();

      // Tell the pack to revert back to regular Cyclotron speeds.
      wandSerialSend(W_CYCLOTRON_NORMAL_SPEED);
    }
    else if(b_overheat_mode[i_power_mode - 1] == true && ms_overheat_initiate.remaining() == 0 && b_overheat_enabled == true) {
      // If the user changes back to power mode that overheats while firing, start up a timer.
      // This currently works only in power levels 1-4. 5 stays locked when firing.
      ms_overheat_initiate.start(i_ms_overheat_initiate[i_power_mode - 1]);
    }
  }
  else {
    if(ms_overheat_initiate.isRunning()) {
      ms_overheat_initiate.stop();

      // Tell the pack to revert back to regular Cyclotron speeds.
      wandSerialSend(W_CYCLOTRON_NORMAL_SPEED);
    }
  }

  switch(FIRING_MODE) {
    case PROTON:
    default:
      // Shift the stream from red to orange on higher power modes.
      switch(i_power_mode) {
        case 1:
        default:
          fireStreamStart(getHueColour(C_RED, WAND_BARREL_LED_COUNT));
          fireStreamEffect(getHueColour(C_BLUE, WAND_BARREL_LED_COUNT));
        break;

        case 2:
          fireStreamStart(getHueColour(C_RED2, WAND_BARREL_LED_COUNT));
          fireStreamEffect(getHueColour(C_BLUE, WAND_BARREL_LED_COUNT));
        break;

        case 3:
          fireStreamStart(getHueColour(C_RED3, WAND_BARREL_LED_COUNT));
          fireStreamEffect(getHueColour(C_LIGHT_BLUE, WAND_BARREL_LED_COUNT));
        break;

        case 4:
          fireStreamStart(getHueColour(C_RED4, WAND_BARREL_LED_COUNT));
          fireStreamEffect(getHueColour(C_LIGHT_BLUE, WAND_BARREL_LED_COUNT));
        break;

        case 5:
          fireStreamStart(getHueColour(C_RED5, WAND_BARREL_LED_COUNT));
          fireStreamEffect(getHueColour(C_WHITE, WAND_BARREL_LED_COUNT));
        break;
      }
    break;

    case SLIME:
      fireStreamStart(getHueColour(C_GREEN, WAND_BARREL_LED_COUNT));
      fireStreamEffect(getHueColour(C_WHITE, WAND_BARREL_LED_COUNT));
    break;

    case STASIS:
      fireStreamStart(getHueColour(C_BLUE, WAND_BARREL_LED_COUNT));
      fireStreamEffect(getHueColour(C_WHITE, WAND_BARREL_LED_COUNT));
    break;

    case MESON:
      // Does not initiate the "stream start" (provides pulse effect)
      fireStreamEffect(getHueColour(C_YELLOW, WAND_BARREL_LED_COUNT));
    break;

    case SPECTRAL:
      fireStreamStart(getHueColour(C_RAINBOW, WAND_BARREL_LED_COUNT));
      fireStreamEffect(getHueColour(C_RAINBOW, WAND_BARREL_LED_COUNT));
    break;

    case HOLIDAY:
      fireStreamStart(getHueColour(C_REDGREEN, WAND_BARREL_LED_COUNT));
      fireStreamEffect(getHueColour(C_REDGREEN, WAND_BARREL_LED_COUNT));
    break;

    case SPECTRAL_CUSTOM:
      fireStreamStart(getHueColour(C_CUSTOM, WAND_BARREL_LED_COUNT));

      if(i_spectral_wand_custom_saturation < 254) {
        fireStreamEffect(getHueColour(C_BLUE, WAND_BARREL_LED_COUNT));
      }
      else {
        fireStreamEffect(getHueColour(C_WHITE, WAND_BARREL_LED_COUNT));
      }
    break;

    case VENTING:
    case SETTINGS:
      // Nothing.
    break;
  }

  // Bargraph loop / scroll.
  if(ms_bargraph_firing.justFinished()) {
    bargraphRampFiring();
  }

  // Mix some impact sound every 10-15 seconds while firing.
  if(ms_impact.justFinished()) {
    playEffect(S_FIRE_LOOP_IMPACT);
    ms_impact.start(15000);
  }
}

void wandHeatUp() {
  stopEffect(S_FIRE_START_SPARK);
  stopEffect(S_PACK_SLIME_OPEN);
  stopEffect(S_STASIS_OPEN);
  stopEffect(S_MESON_OPEN);
  stopEffect(S_VENT_DRY);
  stopEffect(S_VENT_SMOKE);
  stopEffect(S_MODE_SWITCH);

  switch(FIRING_MODE) {
    case PROTON:
    default:
      playEffect(S_FIRE_START_SPARK);
    break;

    case SLIME:
      playEffect(S_PACK_SLIME_OPEN);
    break;

    case STASIS:
      playEffect(S_STASIS_OPEN);
    break;

    case MESON:
      playEffect(S_MESON_OPEN);
    break;

    case VENTING:
      playEffect(S_VENT_DRY);
      playEffect(S_MODE_SWITCH);
    break;

    case SETTINGS:
      // Nothing.
    break;
  }

  wandBarrelPreHeatUp();
}

void wandBarrelPreHeatUp() {
  i_heatup_counter = 0;
  i_heatdown_counter = 100;
  ms_wand_heatup_fade.start(i_delay_heatup);
}

void wandBarrelHeatUp() {
  uint8_t i_barrel_led;

  switch(WAND_BARREL_LED_COUNT) {
    case LEDS_48:
      i_barrel_led = 36;
    break;

    case LEDS_5:
    default:
      i_barrel_led = i_num_barrel_leds - 1;
    break;
  }

  if(i_heatup_counter > 100) {
    wandBarrelHeatDown();
  }
  else if(ms_wand_heatup_fade.justFinished() && i_heatup_counter <= 100) {
    switch(FIRING_MODE) {
      case PROTON:
        barrel_leds[i_barrel_led] = getHueColour(C_WHITE, WAND_BARREL_LED_COUNT, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case SLIME:
        barrel_leds[i_barrel_led] = getHueColour(C_GREEN, WAND_BARREL_LED_COUNT, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case STASIS:
        barrel_leds[i_barrel_led] = getHueColour(C_BLUE, WAND_BARREL_LED_COUNT, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case MESON:
        barrel_leds[i_barrel_led] = getHueColour(C_YELLOW, WAND_BARREL_LED_COUNT, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case SPECTRAL:
        barrel_leds[i_barrel_led] = getHueColour(C_RAINBOW, WAND_BARREL_LED_COUNT, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case HOLIDAY:
        barrel_leds[i_barrel_led] = getHueColour(C_REDGREEN, WAND_BARREL_LED_COUNT, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case SPECTRAL_CUSTOM:
        barrel_leds[i_barrel_led] = getHueColour(C_CUSTOM, WAND_BARREL_LED_COUNT, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case VENTING:
      case SETTINGS:
      default:
        // nothing
      break;
    }

    i_heatup_counter++;
    ms_wand_heatup_fade.start(i_delay_heatup);
  }
}

void wandBarrelHeatDown() {
  uint8_t i_barrel_led;

  switch(WAND_BARREL_LED_COUNT) {
    case LEDS_48:
      i_barrel_led = 36;
    break;

    case LEDS_5:
    default:
      i_barrel_led = i_num_barrel_leds - 1;
    break;
  }

  if(ms_wand_heatup_fade.justFinished() && i_heatdown_counter > 0) {
    switch(FIRING_MODE) {
      case PROTON:
        barrel_leds[i_barrel_led] = getHueColour(C_WHITE, WAND_BARREL_LED_COUNT, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case SLIME:
        barrel_leds[i_barrel_led] = getHueColour(C_GREEN, WAND_BARREL_LED_COUNT, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case STASIS:
        barrel_leds[i_barrel_led] = getHueColour(C_BLUE, WAND_BARREL_LED_COUNT, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case MESON:
        barrel_leds[i_barrel_led] = getHueColour(C_YELLOW, WAND_BARREL_LED_COUNT, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case SPECTRAL:
        barrel_leds[i_barrel_led] = getHueColour(C_RAINBOW, WAND_BARREL_LED_COUNT, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case HOLIDAY:
        barrel_leds[i_barrel_led] = getHueColour(C_REDGREEN, WAND_BARREL_LED_COUNT, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case SPECTRAL_CUSTOM:
        barrel_leds[i_barrel_led] = getHueColour(C_CUSTOM, WAND_BARREL_LED_COUNT, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case VENTING:
      case SETTINGS:
      default:
        // Nothing.
      break;
    }

    i_heatdown_counter--;

    ms_wand_heatup_fade.start(i_delay_heatup);
  }

  if(i_heatdown_counter == 0) {
    barrelLightsOff();
  }
}

void fireStreamEffect(CRGB c_colour) {
  uint8_t i_firing_stream; // Stores a calculated value based on LED count.

  switch(WAND_BARREL_LED_COUNT) {
    case LEDS_48:
      // More LEDs means a faster firing rate.
      i_firing_stream = d_firing_stream / 10;
    break;

    case LEDS_5:
    default:
      // Firing at "normal" speed.
      i_firing_stream = d_firing_stream;
    break;
  }

  switch(WAND_BARREL_LED_COUNT) {
    case LEDS_48:
      // Frutto Technology - 48 LED + Strobe Tip
      // This effect will "wrap" around the device to appear to push the stream forward.
      if(ms_firing_stream_effects.justFinished()) {
        if(i_barrel_light - 1 >= 0 && i_barrel_light - 1 < i_num_barrel_leds) {
          switch(FIRING_MODE) {
            case PROTON:
            default:
              if(b_firing_cross_streams == true) {
                barrel_leds[frutto_barrel[i_barrel_light - 1]] = getHueColour(C_WHITE, WAND_BARREL_LED_COUNT);
                if(i_barrel_light - 2 >= 0 && i_barrel_light - 2 < i_num_barrel_leds) {
                  barrel_leds[frutto_barrel[i_barrel_light - 2]] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
                }
              }
              else {
                // Shift the stream from red to orange on higher power modes.
                switch(i_power_mode) {
                  case 1:
                  default:
                    barrel_leds[frutto_barrel[i_barrel_light - 1]] = getHueColour(C_RED, WAND_BARREL_LED_COUNT);
                  break;

                  case 2:
                    barrel_leds[frutto_barrel[i_barrel_light - 1]] = getHueColour(C_RED2, WAND_BARREL_LED_COUNT);
                  break;

                  case 3:
                    barrel_leds[frutto_barrel[i_barrel_light - 1]] = getHueColour(C_RED3, WAND_BARREL_LED_COUNT);
                  break;

                  case 4:
                    barrel_leds[frutto_barrel[i_barrel_light - 1]] = getHueColour(C_RED4, WAND_BARREL_LED_COUNT);
                  break;

                  case 5:
                    barrel_leds[frutto_barrel[i_barrel_light - 1]] = getHueColour(C_RED5, WAND_BARREL_LED_COUNT);
                  break;
                }
              }
            break;

            case SLIME:
              barrel_leds[frutto_barrel[i_barrel_light - 1]] = getHueColour(C_GREEN, WAND_BARREL_LED_COUNT);
            break;

            case STASIS:
              barrel_leds[frutto_barrel[i_barrel_light - 1]] = getHueColour(C_BLUE, WAND_BARREL_LED_COUNT);
            break;

            case MESON:
              barrel_leds[frutto_barrel[i_barrel_light - 1]] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
            break;

            case SPECTRAL:
              barrel_leds[frutto_barrel[i_barrel_light - 1]] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
            break;

            case HOLIDAY:
              barrel_leds[frutto_barrel[i_barrel_light - 1]] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
            break;

            case SPECTRAL_CUSTOM:
              barrel_leds[frutto_barrel[i_barrel_light - 1]] = getHueColour(C_CUSTOM, WAND_BARREL_LED_COUNT);
            break;

            case VENTING:
            case SETTINGS:
              // Nothing.
            break;
          }

          ms_fast_led.start(i_fast_led_delay);
        }

        if(i_barrel_light == i_num_barrel_leds) {
          i_barrel_light = 0;

          switch(FIRING_MODE) {
            default:
              switch(i_power_mode) {
                case 1:
                  ms_firing_stream_effects.start(i_firing_stream);
                break;

                case 2:
                  ms_firing_stream_effects.start(i_firing_stream - 2);
                break;

                case 3:
                  ms_firing_stream_effects.start(i_firing_stream - 4);
                break;

                case 4:
                  ms_firing_stream_effects.start(i_firing_stream - 6);
                break;

                case 5:
                  ms_firing_stream_effects.start(i_firing_stream - 8);
                break;

                default:
                  ms_firing_stream_effects.start(i_firing_stream);
                break;
              }
            break;
          }
        }
        else if(i_barrel_light < i_num_barrel_leds) {
          barrel_leds[frutto_barrel[i_barrel_light]] = c_colour;
          // if(i_barrel_light + 1 >= 0 && i_barrel_light + 1 < i_num_barrel_leds) {
          //   barrel_leds[frutto_barrel[i_barrel_light + 1]] = c_colour;
          // }

          switch(FIRING_MODE) {
            default:
              switch(i_power_mode) {
                case 1:
                  ms_firing_stream_effects.start((d_firing_stream / 25) + 8);
                break;

                case 2:
                  ms_firing_stream_effects.start((d_firing_stream / 25) + 6);
                break;

                case 3:
                  ms_firing_stream_effects.start((d_firing_stream / 25) + 4);
                break;

                case 4:
                  ms_firing_stream_effects.start((d_firing_stream / 25) + 2);
                break;

                case 5:
                  ms_firing_stream_effects.start((d_firing_stream / 25));
                break;

                default:
                  ms_firing_stream_effects.start((d_firing_stream / 25) + 8);
                break;
              }
            break;
          }

          ms_fast_led.start(i_fast_led_delay);

          i_barrel_light++;
        }
      }
    break;

    case LEDS_5:
    default:
      if(ms_firing_stream_effects.justFinished()) {
        if(i_barrel_light - 1 >= 0 && i_barrel_light - 1 < i_num_barrel_leds) {
          switch(FIRING_MODE) {
            case PROTON:
            default:
              if(b_firing_cross_streams == true) {
                barrel_leds[i_barrel_light - 1] = getHueColour(C_WHITE, WAND_BARREL_LED_COUNT);
              }
              else {
                // Shift the stream from red to orange on higher power modes.
                switch(i_power_mode) {
                  case 1:
                    barrel_leds[i_barrel_light - 1] = getHueColour(C_RED, WAND_BARREL_LED_COUNT);
                  break;

                  case 2:
                    barrel_leds[i_barrel_light - 1] = getHueColour(C_RED2, WAND_BARREL_LED_COUNT);
                  break;

                  case 3:
                    barrel_leds[i_barrel_light - 1] = getHueColour(C_RED3, WAND_BARREL_LED_COUNT);
                  break;

                  case 4:
                    barrel_leds[i_barrel_light - 1] = getHueColour(C_RED4, WAND_BARREL_LED_COUNT);
                  break;

                  case 5:
                    barrel_leds[i_barrel_light - 1] = getHueColour(C_RED5, WAND_BARREL_LED_COUNT);
                  break;

                  default:
                    barrel_leds[i_barrel_light - 1] = getHueColour(C_RED, WAND_BARREL_LED_COUNT);
                  break;
                }
              }
            break;

            case SLIME:
              barrel_leds[i_barrel_light - 1] = getHueColour(C_GREEN, WAND_BARREL_LED_COUNT);
            break;

            case STASIS:
              barrel_leds[i_barrel_light - 1] = getHueColour(C_BLUE, WAND_BARREL_LED_COUNT);
            break;

            case MESON:
              barrel_leds[i_barrel_light - 1] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
            break;

            case SPECTRAL:
              barrel_leds[i_barrel_light - 1] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
            break;

            case HOLIDAY:
              barrel_leds[i_barrel_light - 1] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
            break;

            case SPECTRAL_CUSTOM:
              barrel_leds[i_barrel_light - 1] = getHueColour(C_CUSTOM, WAND_BARREL_LED_COUNT);
            break;

            case VENTING:
            case SETTINGS:
              // Nothing.
            break;
          }

          ms_fast_led.start(i_fast_led_delay);
        }

        if(i_barrel_light == i_num_barrel_leds) {
          i_barrel_light = 0;

          switch(FIRING_MODE) {
            default:
              switch(i_power_mode) {
                case 1:
                  ms_firing_stream_effects.start(i_firing_stream);
                break;

                case 2:
                  ms_firing_stream_effects.start(i_firing_stream - 15);
                break;

                case 3:
                  ms_firing_stream_effects.start(i_firing_stream - 30);
                break;

                case 4:
                  ms_firing_stream_effects.start(i_firing_stream - 45);
                break;

                case 5:
                  ms_firing_stream_effects.start(i_firing_stream - 60);
                break;

                default:
                  ms_firing_stream_effects.start(i_firing_stream);
                break;
              }
            break;
          }
        }
        else if(i_barrel_light < i_num_barrel_leds) {
          barrel_leds[i_barrel_light] = c_colour;

          switch(FIRING_MODE) {
            default:
              switch(i_power_mode) {
                case 1:
                  ms_firing_stream_effects.start((d_firing_stream / 5) + 10);
                break;

                case 2:
                  ms_firing_stream_effects.start((d_firing_stream / 5) + 8);
                break;

                case 3:
                  ms_firing_stream_effects.start((d_firing_stream / 5) + 6);
                break;

                case 4:
                  ms_firing_stream_effects.start((d_firing_stream / 5) + 5);
                break;

                case 5:
                  ms_firing_stream_effects.start((d_firing_stream / 5) + 4);
                break;

                default:
                  ms_firing_stream_effects.start(d_firing_stream / 5);
                break;
              }
            break;
          }

          ms_fast_led.start(i_fast_led_delay);

          i_barrel_light++;
        }
      }
    break;
  }
}

void barrelLightsOff() {
  ms_wand_heatup_fade.stop();
  i_heatup_counter = 0;
  i_heatdown_counter = 100;

  for(uint8_t i = 0; i < i_num_barrel_leds; i++) {
    switch(WAND_BARREL_LED_COUNT) {
      case LEDS_48:
        // Set the tip of the Frutto LED array to white.
        barrel_leds[frutto_barrel[i]] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
      break;

      case LEDS_5:
      default:
        // Illuminate the wand barrel tip LED.
        barrel_leds[i] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
      break;
    }
  }

  // Turn off the wand barrel tip LED.
  wandTipOff();

  ms_fast_led.start(i_fast_led_delay);
}

void fireStreamStart(CRGB c_colour) {
  if(ms_firing_lights.justFinished() && i_barrel_light < i_num_barrel_leds) {
    switch(WAND_BARREL_LED_COUNT) {
      case LEDS_48:
        // Since this arrangement has many more LEDs available, we can make use of extra color changes
        // to enhance the stream effects. In this case we can darken the lead LED then follow with the
        // primary color for the stream chosen. Any other color effects will follow this arrangement.
        barrel_leds[frutto_barrel[i_barrel_light]] = c_colour;
        if(i_barrel_light + 2 >= 0 && i_barrel_light + 2 < i_num_barrel_leds) {
          barrel_leds[frutto_barrel[i_barrel_light + 2]] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
        }
      break;

      case LEDS_5:
      default:
        // Just set the current LED to the expected color.
        barrel_leds[i_barrel_light] = c_colour;
      break;
    }

    ms_fast_led.start(i_fast_led_delay);

    switch(WAND_BARREL_LED_COUNT) {
      case LEDS_48:
        // More LEDs means a faster firing rate.
        ms_firing_lights.start(d_firing_stream / 25);
      break;

      case LEDS_5:
      default:
        // Firing at "normal" speed.
        ms_firing_lights.start(d_firing_stream / 5);
      break;
    }

    i_barrel_light++;

    if(i_barrel_light == i_num_barrel_leds) {
      i_barrel_light = 0;

      ms_firing_lights.stop();

      switch(WAND_BARREL_LED_COUNT) {
        case LEDS_48:
          // More LEDs means a faster firing rate.
          ms_firing_stream_effects.start(d_firing_stream / 10);
        break;

        case LEDS_5:
        default:
          // Firing at "normal" speed.
          ms_firing_stream_effects.start(d_firing_stream);
        break;
      }
    }
  }
}

void fireStreamEnd(CRGB c_colour) {
  if(i_barrel_light < i_num_barrel_leds) {
    ms_fast_led.start(i_fast_led_delay);

    switch(WAND_BARREL_LED_COUNT) {
      case LEDS_48:
        // Set the color for the mapped LED.
        barrel_leds[frutto_barrel[i_barrel_light]] = c_colour;

        // More LEDs means a faster firing rate.
        ms_firing_lights_end.start(d_firing_stream / 25);
      break;

      case LEDS_5:
      default:
        // Set the color for the specific LED.
        barrel_leds[i_barrel_light] = c_colour;

        // Firing at a "normal" rate
        ms_firing_lights_end.start(d_firing_stream / 5);
      break;
    }

    i_barrel_light++;

    if(i_barrel_light == i_num_barrel_leds) {
      i_barrel_light = 0;

      ms_firing_lights_end.stop();
    }
  }
}

void vibrationWand(uint8_t i_level) {
  if(b_vibration_on == true && b_vibration_enabled == true && WAND_ACTION_STATUS != ACTION_OVERHEATING && b_pack_alarm != true) {
    // Vibrate the wand during firing only when enabled. (When enabled by the pack)
    if(b_vibration_firing == true) {
      if(WAND_ACTION_STATUS == ACTION_FIRING) {
        if(i_level != i_vibration_level_prev) {
          i_vibration_level_prev = i_level;
          analogWrite(vibration, i_level);
        }
      }
      else {
        i_vibration_level_prev = 0;
        analogWrite(vibration, 0);
      }
    }
    else {
      // Wand vibrates even when idling, etc. (When enabled by the pack)
      if(i_level != i_vibration_level_prev) {
        i_vibration_level_prev = i_level;
        analogWrite(vibration, i_level);
      }
    }
  }
  else {
    i_vibration_level_prev = 0;
    analogWrite(vibration, 0);
  }
}

// This is the Super Hero bargraph firing animation. Ramping up and down from the middle to the top/bottom and back to the middle again.
void bargraphSuperHeroRampFiringAnimation() {
  if(b_28segment_bargraph == true) {
    switch(i_bargraph_status_alt) {
      case 0:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLedNow(i_bargraph[13]);
        ht_bargraph.setLedNow(i_bargraph[14]);

        b_bargraph_status[13] = true;
        b_bargraph_status[14] = true;

        i_bargraph_status_alt++;

        if(b_bargraph_up == false) {
          ht_bargraph.clearLedNow(i_bargraph[12]);
          ht_bargraph.clearLedNow(i_bargraph[15]);

          b_bargraph_status[12] = false;
          b_bargraph_status[15] = false;
        }

        b_bargraph_up = true;

        wandTipOn();
      break;

      case 1:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLedNow(i_bargraph[12]);
        ht_bargraph.setLedNow(i_bargraph[15]);

        b_bargraph_status[12] = true;
        b_bargraph_status[15] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[13]);
          ht_bargraph.clearLedNow(i_bargraph[14]);

          b_bargraph_status[13] = false;
          b_bargraph_status[14] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[11]);
          ht_bargraph.clearLedNow(i_bargraph[16]);

          b_bargraph_status[11] = false;
          b_bargraph_status[16] = false;

          i_bargraph_status_alt--;
        }

        wandTipOn();
      break;

      case 2:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLedNow(i_bargraph[11]);
        ht_bargraph.setLedNow(i_bargraph[16]);

        b_bargraph_status[11] = true;
        b_bargraph_status[16] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[12]);
          ht_bargraph.clearLedNow(i_bargraph[15]);

          b_bargraph_status[12] = false;
          b_bargraph_status[15] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[10]);
          ht_bargraph.clearLedNow(i_bargraph[17]);

          b_bargraph_status[10] = false;
          b_bargraph_status[17] = false;

          i_bargraph_status_alt--;
        }

        wandTipOff();
      break;

      case 3:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLedNow(i_bargraph[10]);
        ht_bargraph.setLedNow(i_bargraph[17]);

        b_bargraph_status[10] = true;
        b_bargraph_status[17] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[11]);
          ht_bargraph.clearLedNow(i_bargraph[16]);

          b_bargraph_status[11] = false;
          b_bargraph_status[16] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[9]);
          ht_bargraph.clearLedNow(i_bargraph[18]);

          b_bargraph_status[9] = false;
          b_bargraph_status[18] = false;

          i_bargraph_status_alt--;
        }

        wandTipOff();
      break;

      case 4:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLedNow(i_bargraph[9]);
        ht_bargraph.setLedNow(i_bargraph[18]);

        b_bargraph_status[9] = true;
        b_bargraph_status[18] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[10]);
          ht_bargraph.clearLedNow(i_bargraph[17]);

          b_bargraph_status[10] = false;
          b_bargraph_status[17] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[8]);
          ht_bargraph.clearLedNow(i_bargraph[19]);

          b_bargraph_status[8] = false;
          b_bargraph_status[19] = false;

          i_bargraph_status_alt--;
        }

        wandTipOn();
      break;

      case 5:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLedNow(i_bargraph[8]);
        ht_bargraph.setLedNow(i_bargraph[19]);

        b_bargraph_status[8] = true;
        b_bargraph_status[19] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[9]);
          ht_bargraph.clearLedNow(i_bargraph[18]);

          b_bargraph_status[9] = false;
          b_bargraph_status[18] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[7]);
          ht_bargraph.clearLedNow(i_bargraph[20]);

          b_bargraph_status[7] = false;
          b_bargraph_status[20] = false;

          i_bargraph_status_alt--;
        }

        wandTipOn();
      break;

      case 6:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLedNow(i_bargraph[7]);
        ht_bargraph.setLedNow(i_bargraph[20]);

        b_bargraph_status[7] = true;
        b_bargraph_status[20] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[8]);
          ht_bargraph.clearLedNow(i_bargraph[19]);

          b_bargraph_status[8] = false;
          b_bargraph_status[19] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[6]);
          ht_bargraph.clearLedNow(i_bargraph[21]);

          b_bargraph_status[6] = false;
          b_bargraph_status[21] = false;

          i_bargraph_status_alt--;
        }

        wandTipOff();
      break;

      case 7:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLedNow(i_bargraph[6]);
        ht_bargraph.setLedNow(i_bargraph[21]);

        b_bargraph_status[6] = true;
        b_bargraph_status[21] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[7]);
          ht_bargraph.clearLedNow(i_bargraph[20]);

          b_bargraph_status[7] = false;
          b_bargraph_status[20] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[5]);
          ht_bargraph.clearLedNow(i_bargraph[22]);

          b_bargraph_status[5] = false;
          b_bargraph_status[22] = false;

          i_bargraph_status_alt--;
        }

        wandTipOff();
      break;

      case 8:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLedNow(i_bargraph[5]);
        ht_bargraph.setLedNow(i_bargraph[22]);

        b_bargraph_status[5] = true;
        b_bargraph_status[22] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[6]);
          ht_bargraph.clearLedNow(i_bargraph[21]);

          b_bargraph_status[6] = false;
          b_bargraph_status[21] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[4]);
          ht_bargraph.clearLedNow(i_bargraph[23]);

          b_bargraph_status[4] = false;
          b_bargraph_status[23] = false;

          i_bargraph_status_alt--;
        }

        wandTipOn();
      break;

      case 9:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLedNow(i_bargraph[4]);
        ht_bargraph.setLedNow(i_bargraph[23]);

        b_bargraph_status[4] = true;
        b_bargraph_status[23] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[5]);
          ht_bargraph.clearLedNow(i_bargraph[22]);

          b_bargraph_status[5] = false;
          b_bargraph_status[22] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[3]);
          ht_bargraph.clearLedNow(i_bargraph[24]);

          b_bargraph_status[3] = false;
          b_bargraph_status[24] = false;

          i_bargraph_status_alt--;
        }

        wandTipOn();
      break;

      case 10:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLedNow(i_bargraph[3]);
        ht_bargraph.setLedNow(i_bargraph[24]);

        b_bargraph_status[3] = true;
        b_bargraph_status[24] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[4]);
          ht_bargraph.clearLedNow(i_bargraph[23]);

          b_bargraph_status[4] = false;
          b_bargraph_status[23] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[2]);
          ht_bargraph.clearLedNow(i_bargraph[25]);

          b_bargraph_status[2] = false;
          b_bargraph_status[25] = false;

          i_bargraph_status_alt--;
        }

        wandTipOff();
      break;

      case 11:
        vibrationWand(i_vibration_level + 115);

        ht_bargraph.setLedNow(i_bargraph[2]);
        ht_bargraph.setLedNow(i_bargraph[25]);

        b_bargraph_status[2] = false;
        b_bargraph_status[25] = false;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[3]);
          ht_bargraph.clearLedNow(i_bargraph[24]);

          b_bargraph_status[3] = false;
          b_bargraph_status[24] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[1]);
          ht_bargraph.clearLedNow(i_bargraph[26]);

          b_bargraph_status[1] = false;
          b_bargraph_status[26] = false;

          i_bargraph_status_alt--;
        }

        wandTipOff();
      break;

      case 12:
        vibrationWand(i_vibration_level + 115);

        ht_bargraph.setLedNow(i_bargraph[1]);
        ht_bargraph.setLedNow(i_bargraph[26]);

        b_bargraph_status[1] = true;
        b_bargraph_status[26] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[2]);
          ht_bargraph.clearLedNow(i_bargraph[25]);

          b_bargraph_status[2] = false;
          b_bargraph_status[25] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[0]);
          ht_bargraph.clearLedNow(i_bargraph[27]);

          b_bargraph_status[0] = false;
          b_bargraph_status[27] = false;

          i_bargraph_status_alt--;
        }

        wandTipOn();
      break;

      case 13:
        vibrationWand(i_vibration_level + 115);

        ht_bargraph.setLedNow(i_bargraph[0]);
        ht_bargraph.setLedNow(i_bargraph[27]);

        b_bargraph_status[0] = true;
        b_bargraph_status[27] = true;

        ht_bargraph.clearLedNow(i_bargraph[1]);
        ht_bargraph.clearLedNow(i_bargraph[26]);

        b_bargraph_status[1] = false;
        b_bargraph_status[26] = false;

        i_bargraph_status_alt--;

        b_bargraph_up = false;

        wandTipOn();
      break;
    }
  }
  else {
    // Hasbro 5 LED Bargraph.
    switch(i_bargraph_status) {
      case 1:
        vibrationWand(i_vibration_level + 110);

        digitalWrite(i_bargraph_5_led[1-1], LOW);
        digitalWrite(i_bargraph_5_led[2-1], HIGH);
        digitalWrite(i_bargraph_5_led[3-1], HIGH);
        digitalWrite(i_bargraph_5_led[4-1], HIGH);
        digitalWrite(i_bargraph_5_led[5-1], LOW);
        i_bargraph_status++;

        wandTipOn();
      break;

      case 2:
        vibrationWand(i_vibration_level + 112);

        digitalWrite(i_bargraph_5_led[1-1], HIGH);
        digitalWrite(i_bargraph_5_led[2-1], LOW);
        digitalWrite(i_bargraph_5_led[3-1], HIGH);
        digitalWrite(i_bargraph_5_led[4-1], LOW);
        digitalWrite(i_bargraph_5_led[5-1], HIGH);
        i_bargraph_status++;

        wandTipOff();
      break;

      case 3:
        vibrationWand(i_vibration_level + 115);

        digitalWrite(i_bargraph_5_led[1-1], HIGH);
        digitalWrite(i_bargraph_5_led[2-1], HIGH);
        digitalWrite(i_bargraph_5_led[3-1], LOW);
        digitalWrite(i_bargraph_5_led[4-1], HIGH);
        digitalWrite(i_bargraph_5_led[5-1], HIGH);
        i_bargraph_status++;

        wandTipOn();
      break;

      case 4:
        vibrationWand(i_vibration_level + 112);

        digitalWrite(i_bargraph_5_led[1-1], HIGH);
        digitalWrite(i_bargraph_5_led[2-1], LOW);
        digitalWrite(i_bargraph_5_led[3-1], HIGH);
        digitalWrite(i_bargraph_5_led[4-1], LOW);
        digitalWrite(i_bargraph_5_led[5-1], HIGH);
        i_bargraph_status++;

        wandTipOff();
      break;

      case 5:
        vibrationWand(i_vibration_level + 110);

        digitalWrite(i_bargraph_5_led[1-1], LOW);
        digitalWrite(i_bargraph_5_led[2-1], HIGH);
        digitalWrite(i_bargraph_5_led[3-1], HIGH);
        digitalWrite(i_bargraph_5_led[4-1], HIGH);
        digitalWrite(i_bargraph_5_led[5-1], LOW);
        i_bargraph_status = 1;

        wandTipOn();
      break;
    }
  }
}

// This is the Mode Original bargraph firing animation. The top portion fluctuates during firing and becomes more erratic the longer firing continues.
void bargraphModeOriginalRampFiringAnimation() {
  if(b_28segment_bargraph == true) {
    /*
      5: full: 23 - 27  (5 segments)
      4: 3/4: 17 - 22   (6 segments)
      3: 1/2: 12 - 16   (5 segments)
      2: 1/4: 5 - 11    (7 segments)
      1: none: 0 - 4    (5 segments)
    */

    // When firing starts, i_bargraph_status_alt resets to 0 in modeFireStart();
    if(i_bargraph_status_alt == 0) {
      // Set our target.
      switch(i_power_mode) {
        case 5:
          i_bargraph_status_alt = random(18, i_bargraph_segments - 1);
        break;

        case 4:
          i_bargraph_status_alt = random(13, 25);
        break;

        case 3:
          i_bargraph_status_alt = random(9, 19);
        break;

        case 2:
          i_bargraph_status_alt = random(3, 13);
        break;

        case 1:
        default:
          // Not used in MODE_ORIGINAL.
          //i_bargraph_status_alt = random(0, 6);
        break;
      }
    }

    bool b_tmp_down = true;

    for(uint8_t i = 0; i < i_bargraph_segments; i++) {
      if(b_bargraph_status[i] != true && i < i_bargraph_status_alt) {
        b_tmp_down = false;
        break;
      }
    }

    switch(i_power_mode) {
      case 5:
        if(b_tmp_down == true) {
          // Moving down.
          for(uint8_t i = i_bargraph_segments - 1; i >= i_bargraph_status_alt; i--) {
            if(i_bargraph_status_alt == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status_alt = random(6, i_bargraph_segments - 1);
                break;

                case 4:
                  i_bargraph_status_alt = random(9, i_bargraph_segments - 1);
                break;

                case 3:
                  i_bargraph_status_alt = random(12, i_bargraph_segments - 1);
                break;

                case 2:
                  i_bargraph_status_alt = random(15, i_bargraph_segments - 1);
                break;

                case 1:
                  i_bargraph_status_alt = random(18, i_bargraph_segments - 1);
                break;

                default:
                  i_bargraph_status_alt = random(0, i_bargraph_segments - 1);
                break;
              }
            }

            if(b_bargraph_status[i] == true) {
              ht_bargraph.clearLedNow(i_bargraph[i]);
              b_bargraph_status[i] = false;

              break;
            }
          }
        }
        else {
          // Need to move up.
          for(uint8_t i = 0; i <= i_bargraph_status_alt; i++) {
            if(i_bargraph_status_alt == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status_alt = random(8, i_bargraph_segments - 1);
                break;

                case 4:
                  i_bargraph_status_alt = random(12, i_bargraph_segments - 1);
                break;

                case 3:
                  i_bargraph_status_alt = random(14, i_bargraph_segments - 1);
                break;

                case 2:
                  i_bargraph_status_alt = random(16, i_bargraph_segments - 1);
                break;

                case 1:
                  i_bargraph_status_alt = random(18, i_bargraph_segments - 1);
                break;

                default:
                  i_bargraph_status_alt = random(0, i_bargraph_segments - 1);
                break;
              }
            }

            if(b_bargraph_status[i] == false) {
              ht_bargraph.setLedNow(i_bargraph[i]);
              b_bargraph_status[i] = true;

              break;
            }
          }
        }
      break;

      case 4:
        if(b_tmp_down == true) {
          // Moving down.
          for(uint8_t i = 25; i >= i_bargraph_status_alt; i--) {
            if(i_bargraph_status_alt == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status_alt = random(1, 25);
                break;

                case 4:
                  i_bargraph_status_alt = random(4, 25);
                break;

                case 3:
                  i_bargraph_status_alt = random(7, 25);
                break;

                case 2:
                  i_bargraph_status_alt = random(10, 25);
                break;

                case 1:
                  i_bargraph_status_alt = random(13, 25);
                break;

                default:
                  i_bargraph_status_alt = random(0, 25);
                break;
              }
            }

            if(b_bargraph_status[i] == true) {
              ht_bargraph.clearLedNow(i_bargraph[i]);
              b_bargraph_status[i] = false;

              break;
            }
          }
        }
        else {
          // Need to move up.
          for(uint8_t i = 0; i <= i_bargraph_status_alt; i++) {
            if(i_bargraph_status_alt == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status_alt = random(1, 25);
                break;

                case 4:
                  i_bargraph_status_alt = random(4, 25);
                break;

                case 3:
                  i_bargraph_status_alt = random(7, 25);
                break;

                case 2:
                  i_bargraph_status_alt = random(10, 25);
                break;

                case 1:
                  i_bargraph_status_alt = random(13, 25);
                break;

                default:
                  i_bargraph_status_alt = random(0, 25);
                break;
              }
            }

            if(b_bargraph_status[i] == false) {
              ht_bargraph.setLedNow(i_bargraph[i]);
              b_bargraph_status[i] = true;

              break;
            }
          }
        }
      break;

      case 3:
        if(b_tmp_down == true) {
          // Moving down.
          for(uint8_t i = 19; i >= i_bargraph_status_alt; i--) {
            if(i_bargraph_status_alt == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status_alt = random(1, 19);
                break;

                case 4:
                  i_bargraph_status_alt = random(3, 19);
                break;

                case 3:
                  i_bargraph_status_alt = random(5, 19);
                break;

                case 2:
                  i_bargraph_status_alt = random(7, 19);
                break;

                case 1:
                  i_bargraph_status_alt = random(9, 19);
                break;

                default:
                  i_bargraph_status_alt = random(0, 19);
                break;
              }
            }

            if(b_bargraph_status[i] == true) {
              ht_bargraph.clearLedNow(i_bargraph[i]);
              b_bargraph_status[i] = false;

              break;
            }
          }
        }
        else {
          // Need to move up.
          for(uint8_t i = 0; i <= i_bargraph_status_alt; i++) {
            if(i_bargraph_status_alt == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status_alt = random(1, 19);
                break;

                case 4:
                  i_bargraph_status_alt = random(3, 19);
                break;

                case 3:
                  i_bargraph_status_alt = random(5, 19);
                break;

                case 2:
                  i_bargraph_status_alt = random(7, 19);
                break;

                case 1:
                  i_bargraph_status_alt = random(9, 19);
                break;

                default:
                  i_bargraph_status_alt = random(0, 19);
                break;
              }
            }

            if(b_bargraph_status[i] == false) {
              ht_bargraph.setLedNow(i_bargraph[i]);
              b_bargraph_status[i] = true;

              break;
            }
          }
        }
      break;

      case 2:
        if(b_tmp_down == true) {
          // Moving down.
          for(uint8_t i = 13; i >= i_bargraph_status_alt; i--) {
            if(i_bargraph_status_alt == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status_alt = random(1, 13);
                break;

                case 4:
                  i_bargraph_status_alt = random(2, 13);
                break;

                case 3:
                case 2:
                case 1:
                  i_bargraph_status_alt = random(3, 13);
                break;

                default:
                  i_bargraph_status_alt = random(0, 13);
                break;
              }
            }

            if(b_bargraph_status[i] == true) {
              ht_bargraph.clearLedNow(i_bargraph[i]);
              b_bargraph_status[i] = false;

              break;
            }
          }
        }
        else {
          // Need to move up.
          for(uint8_t i = 0; i <= i_bargraph_status_alt; i++) {
            if(i_bargraph_status_alt == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status_alt = random(1, 13);
                break;

                case 4:
                  i_bargraph_status_alt = random(2, 13);
                break;

                case 3:
                case 2:
                case 1:
                  i_bargraph_status_alt = random(3, 13);
                break;

                default:
                  i_bargraph_status_alt = random(0, 13);
                break;
              }
            }

            if(b_bargraph_status[i] == false) {
              ht_bargraph.setLedNow(i_bargraph[i]);
              b_bargraph_status[i] = true;

              break;
            }
          }
        }
      break;

      case 1:
      default:
        if(b_tmp_down == true) {
          // Moving down.
          for(uint8_t i = 7; i >= i_bargraph_status_alt; i--) {
            if(i_bargraph_status_alt == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status_alt = random(0, 10);
                break;

                case 4:
                  i_bargraph_status_alt = random(0, 9);
                break;

                case 3:
                case 2:
                case 1:
                  i_bargraph_status_alt = random(0, 8);
                break;

                default:
                  i_bargraph_status_alt = random(0, 7);
                break;
              }
            }

            if(b_bargraph_status[i] == true) {
              ht_bargraph.clearLedNow(i_bargraph[i]);
              b_bargraph_status[i] = false;

              break;
            }
          }
        }
        else {
          // Need to move up.
          for(uint8_t i = 0; i <= i_bargraph_status_alt; i++) {
            if(i_bargraph_status_alt == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status_alt = random(0, 10);
                break;

                case 4:
                  i_bargraph_status_alt = random(0, 9);
                break;

                case 3:
                case 2:
                case 1:
                  i_bargraph_status_alt = random(0, 8);
                break;

                default:
                  i_bargraph_status_alt = random(0, 7);
                break;
              }
            }

            if(b_bargraph_status[i] == false) {
              ht_bargraph.setLedNow(i_bargraph[i]);
              b_bargraph_status[i] = true;

              break;
            }
          }
        }
      break;
    }
  }
  else {
    // When firing starts, i_bargraph_status resets to 0 in modeFireStart();
    if(i_bargraph_status == 0) {
      // Set our target.
      switch(i_power_mode) {
        case 5:
          i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
        break;

        case 4:
          i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
        break;

        case 3:
          i_bargraph_status = random(1, 4);
        break;

        case 2:
          i_bargraph_status = random(0, 3);
        break;

        case 1:
        default:
          i_bargraph_status = random(0, 2);
        break;
      }
    }

    bool b_tmp_down = true;

    for(uint8_t i = 0; i < i_bargraph_segments_5_led; i++) {
      if(b_bargraph_status_5[i] != true && i <= i_bargraph_status) {
        b_tmp_down = false;
        break;
      }
    }

    switch(i_power_mode) {
      case 5:
        if(b_tmp_down == true) {
          // Moving down.
          for(uint8_t i = i_bargraph_segments_5_led; i >= i_bargraph_status; i--) {
            if(i_bargraph_status == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led + 1);
                break;

                case 4:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led + 1);
                break;

                case 3:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led + 1);
                break;

                case 2:
                  i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
                break;

                case 1:
                  i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
                break;

                default:
                  i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
                break;
              }
            }


            if(b_bargraph_status_5[i-1] == true) {
              wandBargraphControl(i-1);
              break;
            }
          }
        }
        else {
          // Need to move up.
          for(uint8_t i = 0; i <= i_bargraph_status; i++) {
            if(i_bargraph_status == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 4:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 3:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led + 1);
                break;

                case 2:
                  i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
                break;

                case 1:
                  i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
                break;

                default:
                  i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
                break;
              }
            }

            if(b_bargraph_status_5[i] == false) {
              wandBargraphControl(i+1);
              break;
            }
          }
        }
      break;

      case 4:
        if(b_tmp_down == true) {
          // Moving down.
          for(uint8_t i = i_bargraph_segments_5_led; i >= i_bargraph_status; i--) {
            if(i_bargraph_status == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led + 1);
                break;

                case 4:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led + 1);
                break;

                case 3:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led + 1);
                break;

                case 2:
                  i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
                break;

                case 1:
                  i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
                break;

                default:
                  i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
                break;
              }
            }


            if(b_bargraph_status_5[i-1] == true) {
              wandBargraphControl(i-1);
              break;
            }
          }
        }
        else {
          // Need to move up.
          for(uint8_t i = 0; i <= i_bargraph_status; i++) {
            if(i_bargraph_status == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 4:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 3:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led + 1);
                break;

                case 2:
                  i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
                break;

                case 1:
                  i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
                break;

                default:
                  i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
                break;
              }
            }

            if(b_bargraph_status_5[i] == false) {
              wandBargraphControl(i+1);
              break;
            }
          }
        }
      break;

      case 3:
        if(b_tmp_down == true) {
          // Moving down.
          for(uint8_t i = i_bargraph_segments_5_led; i >= i_bargraph_status; i--) {
            if(i_bargraph_status == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led + 1);
                break;

                case 4:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led + 1);
                break;

                case 3:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led + 1);
                break;

                case 2:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led - 1);
                break;

                case 1:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led - 1);
                break;

                default:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led - 1);
                break;
              }
            }


            if(b_bargraph_status_5[i-1] == true) {
              wandBargraphControl(i-1);
              break;
            }
          }
        }
        else {
          // Need to move up.
          for(uint8_t i = 0; i <= i_bargraph_status; i++) {
            if(i_bargraph_status == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 4:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 3:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led + 1);
                break;

                case 2:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led - 1);
                break;

                case 1:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led - 1);
                break;

                default:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led - 1);
                break;
              }
            }

            if(b_bargraph_status_5[i] == false) {
              wandBargraphControl(i+1);
              break;
            }
          }
        }
      break;

      case 2:
        if(b_tmp_down == true) {
          // Moving down.
          for(uint8_t i = i_bargraph_segments_5_led; i >= i_bargraph_status; i--) {
            if(i_bargraph_status == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 4:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 3:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 2:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 1:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led - 2);
                break;

                default:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led - 2);
                break;
              }
            }


            if(b_bargraph_status_5[i-1] == true) {
              wandBargraphControl(i-1);
              break;
            }
          }
        }
        else {
          // Need to move up.
          for(uint8_t i = 0; i <= i_bargraph_status; i++) {
            if(i_bargraph_status == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 4:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 3:
                  i_bargraph_status = random(1, i_bargraph_segments_5_led + 1);
                break;

                case 2:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led - 2);
                break;

                case 1:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led - 2);
                break;

                default:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led - 2);
                break;
              }
            }

            if(b_bargraph_status_5[i] == false) {
              wandBargraphControl(i+1);
              break;
            }
          }
        }
      break;

      case 1:
      default:
        if(b_tmp_down == true) {
          // Moving down.
          for(uint8_t i = i_bargraph_segments_5_led; i >= i_bargraph_status; i--) {
            if(i_bargraph_status == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 4:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 3:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led);
                break;

                case 2:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led - 2);
                break;

                case 1:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led - 2);
                break;

                default:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led - 2);
                break;
              }
            }

            if(b_bargraph_status_5[i-1] == true) {
              wandBargraphControl(i-1);
              break;
            }
          }
        }
        else {
          // Need to move up.
          for(uint8_t i = 0; i <= i_bargraph_status; i++) {
            if(i_bargraph_status == i) {
              switch(i_cyclotron_speed_up) {
                case 5:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 4:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led + 1);
                break;

                case 3:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led);
                break;

                case 2:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led - 2);
                break;

                case 1:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led - 2);
                break;

                default:
                  i_bargraph_status = random(0, i_bargraph_segments_5_led - 2);
                break;
              }
            }

            if(b_bargraph_status_5[i] == false) {
              wandBargraphControl(i+1);
              break;
            }
          }
        }
      break;
    }
  }
}

// Bargraph ramping during firing.
// Optional barrel LED tip strobing is controlled from here to give it a ramp effect if the Proton Pack and Neutrona Wand are going to overheat.
void bargraphRampFiring() {
  switch(BARGRAPH_FIRING_ANIMATION) {
    case BARGRAPH_ANIMATION_SUPER_HERO:
      bargraphSuperHeroRampFiringAnimation();
    break;
    case BARGRAPH_ANIMATION_ORIGINAL:
    default:
      bargraphModeOriginalRampFiringAnimation();

      // Strobe the optional tip light on even barrel lights numbers.
      if((i_barrel_light & 0x01) == 0) {
        wandTipOn();
      }
      else {
        wandTipOff();
      }
    break;
  }

  int i_ramp_interval = d_bargraph_ramp_interval;

  if(b_28segment_bargraph == true) {
    // Switch to a different ramp speed if using the (Optional) 28 segment barmeter bargraph.
    i_ramp_interval = d_bargraph_ramp_interval_alt;
  }

  // If in a power mode on the wand that can overheat, change the speed of the bargraph ramp during firing based on time remaining before we overheat.
  if(b_overheat_mode[i_power_mode - 1] == true && ms_overheat_initiate.isRunning() && b_overheat_enabled == true) {
    if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 6) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start(i_ramp_interval / 15);
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 5);
      }

      cyclotronSpeedUp(6);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 5) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start(i_ramp_interval / 9);
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 4);
      }

      cyclotronSpeedUp(5);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 4) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start(i_ramp_interval / 7);
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 3.5);
      }

      cyclotronSpeedUp(4);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 3) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start(i_ramp_interval / 5);
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 3);
      }

      cyclotronSpeedUp(3);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 2) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start(i_ramp_interval / 3);
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 2.5);
      }

      cyclotronSpeedUp(2);
    }
    else {
      if(b_28segment_bargraph == true) {
        switch(i_power_mode) {
          case 5:
            ms_bargraph_firing.start((i_ramp_interval / 2) - 7); // 13
          break;

          case 4:
            ms_bargraph_firing.start((i_ramp_interval / 2) - 3); // 15
          break;

          case 3:
            ms_bargraph_firing.start(i_ramp_interval / 2); // 20
          break;

          case 2:
            ms_bargraph_firing.start((i_ramp_interval / 2) + 7); // 30
          break;

          case 1:
            ms_bargraph_firing.start((i_ramp_interval / 2) + 12); // 35
          break;
        }
      }
      else {
        if(BARGRAPH_FIRING_ANIMATION == BARGRAPH_ANIMATION_ORIGINAL) {
          switch(i_power_mode) {
            case 5:
              ms_bargraph_firing.start(i_ramp_interval / 2); // 20
            break;

            case 4:
              ms_bargraph_firing.start(i_ramp_interval / 1.5); // 26.6
            break;

            case 3:
              ms_bargraph_firing.start(i_ramp_interval); // 40
            break;

            case 2:
              ms_bargraph_firing.start(i_ramp_interval * 2); // 80
            break;

            case 1:
              ms_bargraph_firing.start(i_ramp_interval * 3); // 120
            break;
          }
        }
        else {
          ms_bargraph_firing.start(i_ramp_interval / 2); // 20
        }
      }

      i_cyclotron_speed_up = 1;
    }
  }
  else {
    if(b_28segment_bargraph == true) {
      switch(i_power_mode) {
        case 5:
          ms_bargraph_firing.start((i_ramp_interval / 2) - 7); // 13
        break;

        case 4:
          ms_bargraph_firing.start((i_ramp_interval / 2) - 3); // 15
        break;

        case 3:
          ms_bargraph_firing.start(i_ramp_interval / 2); // 20
        break;

        case 2:
          ms_bargraph_firing.start((i_ramp_interval / 2) + 7); // 25
        break;

        case 1:
          ms_bargraph_firing.start((i_ramp_interval / 2) + 12); // 30
        break;
      }
    }
    else {
      if(BARGRAPH_FIRING_ANIMATION == BARGRAPH_ANIMATION_ORIGINAL) {
        switch(i_power_mode) {
          case 5:
            ms_bargraph_firing.start(i_ramp_interval / 2); // 20
          break;

          case 4:
            ms_bargraph_firing.start(i_ramp_interval / 1.5); // 26.6
          break;

          case 3:
            ms_bargraph_firing.start(i_ramp_interval); // 40
          break;

          case 2:
            ms_bargraph_firing.start(i_ramp_interval * 2); // 80
          break;

          case 1:
            ms_bargraph_firing.start(i_ramp_interval * 3); // 120
          break;
        }
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 2); // 20
      }
    }
  }
}

void cyclotronSpeedUp(uint8_t i_switch) {
  if(i_switch != i_cyclotron_speed_up) {
    if(i_switch == 4) {
      // Tell pack to start beeping before we overheat it.
      wandSerialSend(W_BEEP_START);

      // Beep the wand 8 times.
      playEffect(S_BEEP_8);

      ms_hat_1.start(i_hat_1_delay);
    }

    i_cyclotron_speed_up++;

    // Tell the pack to speed up the Cyclotron.
    wandSerialSend(W_CYCLOTRON_INCREASE_SPEED);
  }
}

void cyclotronSpeedRevert() {
  // Stop overheat beeps.
  stopEffect(S_BEEP_8);

  i_cyclotron_speed_up = 1;
}

// 2021 mode for optional 28 segment bargraph.
// Checks if we ramp up or down when changing power levels.
// Forces the bargraph to redraw itself to the current power level.
void bargraphPowerCheck2021Alt(bool b_override) {
  if((WAND_ACTION_STATUS != ACTION_FIRING && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) || b_override == true) {
    if(i_power_mode != i_power_mode_prev || b_override == true) {
      if(i_power_mode > i_power_mode_prev) {
        b_bargraph_up = true;
      }
      else {
        b_bargraph_up = false;
      }

      switch(i_power_mode) {
        case 5:
          ms_bargraph_alt.start(i_bargraph_wait / 3);
        break;

        case 4:
          ms_bargraph_alt.start(i_bargraph_wait / 4);
        break;

        case 3:
          ms_bargraph_alt.start(i_bargraph_wait / 5);
        break;

        case 2:
          ms_bargraph_alt.start(i_bargraph_wait / 6);
        break;

        case 1:
          ms_bargraph_alt.start(i_bargraph_wait / 7);
        break;
      }
    }
  }
}

void bargraphClearAll() {
  ht_bargraph.clearAll();

  for(uint8_t i = 0; i < i_bargraph_segments; i++) {
    b_bargraph_status[i] = false;
  }
}

void bargraphClearAlt() {
  if(b_28segment_bargraph == true) {
    bargraphClearAll();

    i_bargraph_status_alt = 0;
  }
}

void setBargraphOrientation() {
  if(b_bargraph_invert != true) {
    // Resets the 28 Segment bargraph orientation.
    for(uint8_t i = 0; i < i_bargraph_segments; i++) {
      i_bargraph[i] = i_bargraph_normal[i];
    }

    for(uint8_t i = 0; i < i_bargraph_segments_5_led; i++) {
      i_bargraph_5_led[i] = i_bargraph_5_led_normal[i];
    }
  }
  else {
    // Resets the 28 Segment bargraph orientation.
    for(uint8_t i = 0; i < i_bargraph_segments; i++) {
      i_bargraph[i] = i_bargraph_invert[i];
    }

    for(uint8_t i = 0; i < i_bargraph_segments_5_led; i++) {
      i_bargraph_5_led[i] = i_bargraph_5_led_invert[i];
    }
  }
}

// Draw the bargraph to the current power level instantly.
void bargraphRedraw() {
  if(b_28segment_bargraph == true) {
    /*
      5: full: 23 - 27  (5 segments)
      4: 3/4: 17 - 22   (6 segments)
      3: 1/2: 12 - 16   (5 segments)
      2: 1/4: 5 - 11    (7 segments)
      1: none: 0 - 4    (5 segments)
    */

    switch(i_power_mode) {
      case 1:
        for(uint8_t i = 0; i < i_bargraph_segments; i++) {
          if(i <= 4) {
            ht_bargraph.setLedNow(i_bargraph[i]);
            b_bargraph_status[i] = true;
          }
          else {
            ht_bargraph.clearLedNow(i_bargraph[i]);
            b_bargraph_status[i] = false;
          }
        }

        i_bargraph_status_alt = 4;
      break;

      case 2:
        for(uint8_t i = 0; i < i_bargraph_segments; i++) {
          if(i <= 11) {
            ht_bargraph.setLedNow(i_bargraph[i]);
            b_bargraph_status[i] = true;
          }
          else {
            ht_bargraph.clearLedNow(i_bargraph[i]);
            b_bargraph_status[i] = false;
          }
        }

        i_bargraph_status_alt = 11;
      break;

      case 3:
        for(uint8_t i = 0; i < i_bargraph_segments; i++) {
          if(i <= 16) {
            ht_bargraph.setLedNow(i_bargraph[i]);
            b_bargraph_status[i] = true;
          }
          else {
            ht_bargraph.clearLedNow(i_bargraph[i]);
            b_bargraph_status[i] = false;
          }
        }

        i_bargraph_status_alt = 16;
      break;

      case 4:
        for(uint8_t i = 0; i < i_bargraph_segments; i++) {
          if(i <= 22) {
            ht_bargraph.setLedNow(i_bargraph[i]);
            b_bargraph_status[i] = true;
          }
          else {
            ht_bargraph.clearLedNow(i_bargraph[i]);
            b_bargraph_status[i] = false;
          }
        }

        i_bargraph_status_alt = 22;
      break;

      case 5:
        for(uint8_t i = 0; i < i_bargraph_segments; i++) {
          ht_bargraph.setLedNow(i_bargraph[i]);
          b_bargraph_status[i] = true;
        }

        i_bargraph_status_alt = 27;
      break;
    }
  }
  else {
    // Stock haslab bargraph control.
    switch(i_power_mode) {
      case 1:
        wandBargraphControl(1);
      break;

      case 2:
        wandBargraphControl(2);
      break;

      case 3:
        wandBargraphControl(3);
      break;

      case 4:
        wandBargraphControl(4);
      break;

      case 5:
        wandBargraphControl(5);
      break;
    }
  }
}

void bargraphPowerCheck() {
  // Control for the 28 segment barmeter bargraph.
  /*
    5: full: 23 - 27  (5 segments)
    4: 3/4: 17 - 22   (6 segments)
    3: 1/2: 12 - 16   (5 segments)
    2: 1/4: 5 - 11    (7 segments)
    1: none: 0 - 4    (5 segments)
  */
  if(b_28segment_bargraph == true) {
    if(ms_bargraph_alt.justFinished()) {
      uint8_t i_bargraph_multiplier[5] = { 7, 6, 5, 4, 3 };

      if(BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
        for(uint8_t i = 0; i <= 4; i++) {
          i_bargraph_multiplier[i] = 10;
        }
      }

      if(b_bargraph_up == true) {
        if(i_bargraph_status_alt < i_bargraph_segments) {
          ht_bargraph.setLedNow(i_bargraph[i_bargraph_status_alt]);
          b_bargraph_status[i_bargraph_status_alt] = true;
        }

        switch(i_power_mode) {
          case 5:
            if(i_bargraph_status_alt > 27) {
              b_bargraph_up = false;

              i_bargraph_status_alt = 27;

              if(BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
                // We stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                // A little pause when we reach the top.
                ms_bargraph_alt.start(i_bargraph_wait / 2);
              }
            }
            else {
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_mode - 1]);
            }
          break;

          case 4:
            if(i_bargraph_status_alt > 21) {
              b_bargraph_up = false;

              if(BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
                // We stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                // A little pause when we reach the top.
                ms_bargraph_alt.start(i_bargraph_wait / 2);
              }
            }
            else {
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_mode - 1]);
            }
          break;

          case 3:
            if(i_bargraph_status_alt > 15) {
              b_bargraph_up = false;
              if(BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
                // We stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                // A little pause when we reach the top.
                ms_bargraph_alt.start(i_bargraph_wait / 2);
              }
            }
            else {
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_mode - 1]);
            }
          break;

          case 2:
            if(i_bargraph_status_alt > 10) {
              b_bargraph_up = false;
              if(BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
                // We stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                // A little pause when we reach the top.
                ms_bargraph_alt.start(i_bargraph_wait / 2);
              }
            }
            else {
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_mode - 1]);
            }
          break;

          case 1:
            if(i_bargraph_status_alt > 4) {
              b_bargraph_up = false;
              if(BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
                // We stop when we reach our target.
                ms_bargraph_alt.stop();

                // Reset and redraw all the proper segments for the bargraph.
                //bargraphRedraw();
              }
              else {
                // A little pause when we reach the top.
                ms_bargraph_alt.start(i_bargraph_wait / 2);
              }
            }
            else {
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_mode - 1]);
            }
          break;
        }

        if(b_bargraph_up == true) {
          i_bargraph_status_alt++;
        }
      }
      else {
        if(i_bargraph_status_alt < i_bargraph_segments) {
          ht_bargraph.clearLedNow(i_bargraph[i_bargraph_status_alt]);
          b_bargraph_status[i_bargraph_status_alt] = false;
        }

        if(i_bargraph_status_alt == 0) {
          i_bargraph_status_alt = 0;
          b_bargraph_up = true;

          // A little pause when we reach the bottom.
          ms_bargraph_alt.start(i_bargraph_wait / 2);
        }
        else {
          i_bargraph_status_alt--;

          switch(i_power_mode) {
            case 5:
              if(BARGRAPH_MODE == BARGRAPH_ORIGINAL && i_bargraph_status_alt < 28) {
                // We stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                ms_bargraph_alt.start(i_bargraph_interval * 3);
              }
            break;

            case 4:
              if(BARGRAPH_MODE == BARGRAPH_ORIGINAL && i_bargraph_status_alt < 23) {
                // We stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                ms_bargraph_alt.start(i_bargraph_interval * 4);
              }
            break;

            case 3:
              if(BARGRAPH_MODE == BARGRAPH_ORIGINAL && i_bargraph_status_alt < 17) {
                // We stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                ms_bargraph_alt.start(i_bargraph_interval * 5);
              }
            break;

            case 2:
              if(BARGRAPH_MODE == BARGRAPH_ORIGINAL && i_bargraph_status_alt < 12) {
                // We stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                ms_bargraph_alt.start(i_bargraph_interval * 6);
              }
            break;

            case 1:
              if(BARGRAPH_MODE == BARGRAPH_ORIGINAL && i_bargraph_status_alt < 5) {
                // We stop when we reach our target.
                ms_bargraph_alt.stop();

                // Reset and redraw all the proper segments for the bargraph.
                //bargraphRedraw();
              }
              else {
                ms_bargraph_alt.start(i_bargraph_interval * 7);
              }
            break;
          }
        }
      }
    }
  }
  else {
    // Stock haslab bargraph control.
    switch(i_power_mode) {
      case 1:
        wandBargraphControl(1);
      break;

      case 2:
        wandBargraphControl(2);
      break;

      case 3:
        wandBargraphControl(3);
      break;

      case 4:
        wandBargraphControl(4);
      break;

      case 5:
        wandBargraphControl(5);
      break;
    }
  }
}

// Fully lights up the bargraphs.
void bargraphFull() {
  if(b_28segment_bargraph == true) {
    for(uint8_t i = 0; i < i_bargraph_segments; i++) {
      ht_bargraph.setLedNow(i_bargraph[i]);
      b_bargraph_status[i] = true;
    }
  }
  else {
    wandBargraphControl(5);
  }
}

void bargraphRampUp() {
  if(i_vibration_level < i_vibration_level_min) {
    i_vibration_level = i_vibration_level_min;
  }

  if(b_28segment_bargraph == true) {
    /*
      5: full: 23 - 27	(5 segments)
      4: 3/4: 17 - 22	(6 segments)
      3: 1/2: 12 - 16	(5 segments)
      2: 1/4: 5 - 11	(7 segments)
      1: none: 0 - 4	(5 segments)
    */

    switch(i_bargraph_status_alt) {
      case 0 ... 27:
        ht_bargraph.setLedNow(i_bargraph[i_bargraph_status_alt]);
        b_bargraph_status[i_bargraph_status_alt] = true;

        if(i_bargraph_status_alt > 22) {
          vibrationWand(i_vibration_level + 80);
        }
        else if(i_bargraph_status_alt > 16) {
          vibrationWand(i_vibration_level + 40);
        }
        else if(i_bargraph_status_alt > 10) {
          vibrationWand(i_vibration_level + 30);
        }
        else if(i_bargraph_status_alt > 4) {
          vibrationWand(i_vibration_level + 20);
        }
        else if(i_bargraph_status_alt > 0) {
          vibrationWand(i_vibration_level + 10);
        }

        i_bargraph_status_alt++;

        if(i_bargraph_status_alt == 28) {
          // A little pause when we reach the top.
          ms_bargraph.start(i_bargraph_wait / 2);

          // Adjust the ramp down speed if necessary.
          if(BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
            i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_2021 / 2;
          }
        }
        else {
          ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
        }
      break;

      case 28 ... 55:
        uint8_t i_tmp = i_bargraph_status_alt - (i_bargraph_segments - 1);
        i_tmp = i_bargraph_segments - i_tmp;

        if(WAND_ACTION_STATUS == ACTION_OVERHEATING || b_pack_alarm == true) {
          vibrationOff();
        }

        if(WAND_ACTION_STATUS == ACTION_OVERHEATING || b_pack_alarm == true) {
            ht_bargraph.clearLedNow(i_bargraph[i_tmp]);
            b_bargraph_status[i_tmp] = false;

            if(i_bargraph_status_alt == 55) {
              ms_bargraph.stop();
              b_bargraph_up = false;
              i_bargraph_status_alt = 0;
            }
            else {
              ms_bargraph.start(d_bargraph_ramp_interval_alt * 2);
              i_bargraph_status_alt++;
            }
        }
        else {
          if((i_power_mode < 5 && BARGRAPH_MODE == BARGRAPH_ORIGINAL) || BARGRAPH_MODE == BARGRAPH_SUPER_HERO) {
            ht_bargraph.clearLedNow(i_bargraph[i_tmp]);
            b_bargraph_status[i_tmp] = false;
          }

          switch(BARGRAPH_MODE) {
            case BARGRAPH_SUPER_HERO:
              // Bargraph has ramped up and down. In 1984/1989 mode we want to start the ramping.
              if(i_bargraph_status_alt == 55) {
                ms_bargraph_alt.start(i_bargraph_interval); // Start the alternate bargraph to ramp up and down continuously.
                ms_bargraph.stop();
                b_bargraph_up = true;
                i_bargraph_status_alt = 0;
                bargraphYearModeUpdate();

                vibrationWand(i_vibration_level);
              }
              else {
                ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
                i_bargraph_status_alt++;
              }

            break;

            case BARGRAPH_ORIGINAL:
              switch(i_power_mode) {
                case 5:
                  // Stop any power check if we are already in level 5.
                  ms_bargraph_alt.stop();

                  ms_bargraph.stop();
                  b_bargraph_up = false;
                  i_bargraph_status_alt = 27;
                  bargraphYearModeUpdate();
                  vibrationWand(i_vibration_level + 25);
                break;

                case 4:
                  if(i_bargraph_status_alt == 32) {
                    ms_bargraph.stop();
                    b_bargraph_up = false;
                    i_bargraph_status_alt = 22;
                    bargraphYearModeUpdate();

                    vibrationWand(i_vibration_level + 30);
                  }
                  else {
                    ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
                    i_bargraph_status_alt++;

                    vibrationWand(i_vibration_level + 12);
                  }
                break;

                case 3:
                  if(i_bargraph_status_alt == 38) {
                    ms_bargraph.stop();
                    b_bargraph_up = false;
                    i_bargraph_status_alt = 16;
                    bargraphYearModeUpdate();

                    vibrationWand(i_vibration_level + 10);
                  }
                  else {
                    ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
                    i_bargraph_status_alt++;

                    vibrationWand(i_vibration_level + 20);
                  }
                break;

                case 2:
                  if(i_bargraph_status_alt == 43) {
                    ms_bargraph.stop();
                    b_bargraph_up = false;
                    i_bargraph_status_alt = 11;
                    bargraphYearModeUpdate();

                    vibrationWand(i_vibration_level + 5);
                  }
                  else {
                    ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
                    i_bargraph_status_alt++;

                    vibrationWand(i_vibration_level + 10);
                  }
                break;

                case 1:
                  vibrationWand(i_vibration_level);

                  if(i_bargraph_status_alt == 50) {
                    ms_bargraph.stop();
                    b_bargraph_up = false;
                    i_bargraph_status_alt = 5;
                    bargraphYearModeUpdate();
                  }
                  else {
                    ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
                    i_bargraph_status_alt++;
                  }
                break;
              }
            break;
          }
        }
      break;
    }
  }
  else {
    uint8_t t_bargraph_ramp_multiplier = 1;

    if(WAND_ACTION_STATUS == ACTION_OVERHEATING || b_pack_alarm == true) {
      t_bargraph_ramp_multiplier = 2;
    }

    switch(i_bargraph_status) {
      case 0:
        vibrationWand(i_vibration_level + 10);

        wandBargraphControl(1);
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 1:
        vibrationWand(i_vibration_level + 20);

        wandBargraphControl(2);
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 2:
        vibrationWand(i_vibration_level + 30);

        wandBargraphControl(3);
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 3:
        vibrationWand(i_vibration_level + 40);

        wandBargraphControl(4);
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 4:
        vibrationWand(i_vibration_level + 80);

        wandBargraphControl(5);

        if(i_bargraph_status + 1 == i_power_mode && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          i_bargraph_status++;
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        }
      break;

      case 5:
        vibrationWand(i_vibration_level + 40);

        wandBargraphControl(4);

        if(i_bargraph_status - 1 == i_power_mode && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          i_bargraph_status++;
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        }
      break;

      case 6:
        vibrationWand(i_vibration_level + 30);

        wandBargraphControl(3);

        if(i_bargraph_status - 3 == i_power_mode && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          i_bargraph_status++;
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        }
      break;

      case 7:
        vibrationWand(i_vibration_level + 20);

        wandBargraphControl(2);

        if(i_bargraph_status - 5 == i_power_mode && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          i_bargraph_status++;
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        }
      break;

      case 8:
        vibrationWand(i_vibration_level + 10);

        wandBargraphControl(1);

        if(i_bargraph_status - 7 == i_power_mode && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
          i_bargraph_status++;
        }
      break;

      case 9:
        wandBargraphControl(0);

        ms_bargraph.stop();
        i_bargraph_status = 0;
      break;
    }
  }
}

void prepBargraphRampDown() {
  if((WAND_STATUS == MODE_ON && WAND_ACTION_STATUS == ACTION_IDLE) || (WAND_STATUS == MODE_OFF && WAND_ACTION_STATUS == ACTION_IDLE && SYSTEM_MODE == MODE_ORIGINAL)) {
    // If bargraph is set to ramp down during overheat, we need to set a few things.
    soundBeepLoopStop();
    soundIdleStop();
    soundIdleLoopStop();

    b_sound_idle = false; // REMOVE ??
    b_beeping = false;

    // Reset some bargraph levels before we ramp the bargraph down.
    i_bargraph_status_alt = 28; // For 28 segment bargraph
    i_bargraph_status = 5; // For Hasbro 5 LED bargraph.

    bargraphFull();

    ms_bargraph.start(d_bargraph_ramp_interval);

    // Prepare to make the bargraph ramp down now.
    bargraphRampUp();
  }
}

void prepBargraphRampUp() {
  if((WAND_STATUS == MODE_ON && WAND_ACTION_STATUS == ACTION_IDLE) || (WAND_STATUS == MODE_OFF && WAND_ACTION_STATUS == ACTION_IDLE && SYSTEM_MODE == MODE_ORIGINAL)) {
    bargraphClearAlt();

    ms_settings_blinking.stop();

    // Prepare a few things before ramping the bargraph back up from a full ramp down.
    if(b_overheat_bargraph_blink != true) {
      if(BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
        bargraphYearModeUpdate();
      }
      else {
        i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_1984 * 2;
      }

      // If using the 28 segment bargraph, in Afterlife, we need to redraw the segments.
      // 1984/1989 years will go in to a auto ramp and do not need a manual refresh.
      if(b_28segment_bargraph == true && BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
        bargraphPowerCheck2021Alt(false);
      }

      updatePackPowerLevel();
      bargraphRampUp();
    }
  }
}

// Return the year mode that the Neturona Wand is supposed to be in. Or if overridden to be in a different year by the user.
SYSTEM_YEARS getNeutronaWandYearMode() {
  switch(WAND_YEAR_MODE) {
    case YEAR_1984:
      return SYSTEM_1984;
    break;

    case YEAR_1989:
      return SYSTEM_1989;
    break;

    case YEAR_AFTERLIFE:
      return SYSTEM_AFTERLIFE;
    break;

    case YEAR_FROZEN_EMPIRE:
      return SYSTEM_FROZEN_EMPIRE;
    break;

    case YEAR_DEFAULT:
    default:
      return SYSTEM_YEAR;
    break;
  }
}

// Returns SYSTEM_YEAR when operating with a Proton Pack, or WAND_YEAR_MODE when in standalone operation
SYSTEM_YEARS getSystemYearMode() {
  if(b_gpstar_benchtest == true) {
    return getNeutronaWandYearMode();
  }
  else {
    return SYSTEM_YEAR;
  }
}

void bargraphYearModeUpdate() {
  // Set the bargraph settings based on data saved in the EEPROM.
  switch(BARGRAPH_MODE_EEPROM) {
    case BARGRAPH_EEPROM_ORIGINAL:
      BARGRAPH_MODE = BARGRAPH_ORIGINAL;
    break;

    case BARGRAPH_EEPROM_SUPER_HERO:
      BARGRAPH_MODE = BARGRAPH_SUPER_HERO;
    break;

    case BARGRAPH_EEPROM_DEFAULT:
    default:
      switch(SYSTEM_MODE) {
        case MODE_ORIGINAL:
          BARGRAPH_MODE = BARGRAPH_ORIGINAL;
        break;

        case MODE_SUPER_HERO:
        default:
          switch(getNeutronaWandYearMode()) {
            case SYSTEM_1984:
            case SYSTEM_1989:
              BARGRAPH_MODE = BARGRAPH_SUPER_HERO;
            break;

            case SYSTEM_AFTERLIFE:
            case SYSTEM_FROZEN_EMPIRE:
            default:
              BARGRAPH_MODE = BARGRAPH_ORIGINAL;
            break;
          }
        break;
      }
    break;
  }

  // Set the bargraph firing animation settings based on data saved in the EEPROM.
  switch(BARGRAPH_EEPROM_FIRING_ANIMATION) {
    case BARGRAPH_EEPROM_ANIMATION_SUPER_HERO:
      BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_SUPER_HERO;
    break;

    case BARGRAPH_EEPROM_ANIMATION_ORIGINAL:
      BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_ORIGINAL;
    break;

    case BARGRAPH_EEPROM_ANIMATION_DEFAULT:
    default:
      switch(SYSTEM_MODE) {
        case MODE_ORIGINAL:
          BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_ORIGINAL;
        break;

        case MODE_SUPER_HERO:
        default:
          BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_SUPER_HERO;
        break;
      }
    break;
  }

  // Set the bargraph speed based on the bargraph animation type.
  switch(BARGRAPH_MODE) {
    case BARGRAPH_ORIGINAL:
      i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_2021;
    break;

    case BARGRAPH_SUPER_HERO:
    default:
      i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_1984;
    break;
  }
}

void wandBargraphControl(uint8_t i_t_level) {
  if(i_t_level > 4) {
    // On
    digitalWrite(i_bargraph_5_led[5-1], LOW);
    b_bargraph_status_5[4] = true;
  }
  else {
    // Off
    digitalWrite(i_bargraph_5_led[5-1], HIGH);
    b_bargraph_status_5[4] = false;
  }

  if(i_t_level > 3) {
    digitalWrite(i_bargraph_5_led[4-1], LOW);
    b_bargraph_status_5[3] = true;
  }
  else {
    digitalWrite(i_bargraph_5_led[4-1], HIGH);
    b_bargraph_status_5[3] = false;
  }

  if(i_t_level > 2) {
    digitalWrite(i_bargraph_5_led[3-1], LOW);
    b_bargraph_status_5[2] = true;
  }
  else {
    digitalWrite(i_bargraph_5_led[3-1], HIGH);
    b_bargraph_status_5[2] = false;
  }

  if(i_t_level > 1) {
    digitalWrite(i_bargraph_5_led[2-1], LOW);
    b_bargraph_status_5[1] = true;
  }
  else {
    digitalWrite(i_bargraph_5_led[2-1], HIGH);
    b_bargraph_status_5[1] = false;
  }

  if(i_t_level > 0) {
    digitalWrite(i_bargraph_5_led[1-1], LOW);
    b_bargraph_status_5[0] = true;
  }
  else {
    digitalWrite(i_bargraph_5_led[1-1], HIGH);
    b_bargraph_status_5[0] = false;
  }
}

void wandLightsOff() {
  if(b_28segment_bargraph == true) {
    bargraphClearAlt();
  }
  else {
    wandBargraphControl(0);
  }

  analogWrite(led_slo_blo, 0);
  analogWrite(led_front_left, 0); // Turn off the front left LED under the Clippard valve.

  digitalWrite(led_hat_1, LOW); // Turn off hat light 1.
  digitalWrite(led_hat_2, LOW); // Turn off hat light 2.
  wandTipOff();

  digitalWrite(led_vent, HIGH);
  digitalWrite(led_white, HIGH);

  i_bargraph_status = 0;
  i_bargraph_status_alt = 0;

  if(b_power_on_indicator == true) {
    if(ms_power_indicator.isRunning() != true) {
      ms_power_indicator.start(i_ms_power_indicator);
    }
  }
}

void wandLightsOffMenuSystem() {
  // Make sure some of the wand lights are off, specifically for the Menu systems.
  analogWrite(led_slo_blo, 0);
  digitalWrite(led_vent, HIGH);
  digitalWrite(led_white, HIGH);
  analogWrite(led_front_left, 0);

  if(b_power_on_indicator == true) {
    ms_power_indicator.stop();
    ms_power_indicator_blink.stop();
  }
}

void vibrationOff() {
  i_vibration_level_prev = 0;
  analogWrite(vibration, 0);
}

int8_t readRotary() {
  static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

  prev_next_code <<= 2;

  if(digitalRead(r_encoderB)) {
    prev_next_code |= 0x02;
  }

  if(digitalRead(r_encoderA)) {
    prev_next_code |= 0x01;
  }

  prev_next_code &= 0x0f;

   // If valid then store as 16 bit data.
   if(rot_enc_table[prev_next_code]) {
      store <<= 4;
      store |= prev_next_code;

      if((store&0xff) == 0x2b) {
        return -1;
      }

      if((store&0xff) == 0x17) {
        return 1;
      }
   }

   return 0;
}

void wandBarrelSpectralCustomConfigOn() {
  for(uint8_t i = 0; i < i_num_barrel_leds; i++) {
    barrel_leds[i] = getHueColour(C_CUSTOM, WAND_BARREL_LED_COUNT);
  }

  ms_fast_led.start(i_fast_led_delay);
}

// It is very important that S_1 up to S_60 follow each other in order on the Micro SD Card and sound effects enum.
void overheatVoiceIndicator(unsigned int i_tmp_length) {
  i_tmp_length = i_tmp_length / i_overheat_delay_increment;

  uint16_t i_tmp_sound = (S_1 - 1) + i_tmp_length;

  stopEffect(i_tmp_sound - 1);
  stopEffect(i_tmp_sound);
  stopEffect(i_tmp_sound + 1);
  playEffect(i_tmp_sound);

  // Tell the Proton Pack to play this sound effect.
  wandSerialSend(W_COM_SOUND_NUMBER, i_tmp_sound);
}

void overheatTimerIncrement(uint8_t i_tmp_power_level) {
  switch(i_tmp_power_level) {
    case 5:
      if(i_ms_overheat_initiate_mode_5 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheat_initiate_mode_5 = i_ms_overheat_initiate_mode_5 + i_overheat_delay_increment;
        i_ms_overheat_initiate[4] = i_ms_overheat_initiate_mode_5;

        overheatVoiceIndicator(i_ms_overheat_initiate_mode_5);
      }
    break;

    case 4:
      if(i_ms_overheat_initiate_mode_4 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheat_initiate_mode_4 = i_ms_overheat_initiate_mode_4 + i_overheat_delay_increment;
        i_ms_overheat_initiate[3] = i_ms_overheat_initiate_mode_4;

        overheatVoiceIndicator(i_ms_overheat_initiate_mode_4);
      }
    break;

    case 3:
      if(i_ms_overheat_initiate_mode_3 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheat_initiate_mode_3 = i_ms_overheat_initiate_mode_3 + i_overheat_delay_increment;
        i_ms_overheat_initiate[2] = i_ms_overheat_initiate_mode_3;

        overheatVoiceIndicator(i_ms_overheat_initiate_mode_3);
      }
    break;

    case 2:
      if(i_ms_overheat_initiate_mode_2 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheat_initiate_mode_2 = i_ms_overheat_initiate_mode_2 + i_overheat_delay_increment;
        i_ms_overheat_initiate[1] = i_ms_overheat_initiate_mode_2;

        overheatVoiceIndicator(i_ms_overheat_initiate_mode_2);
      }
    break;

    case 1:
    default:
      if(i_ms_overheat_initiate_mode_1 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheat_initiate_mode_1 = i_ms_overheat_initiate_mode_1 + i_overheat_delay_increment;
        i_ms_overheat_initiate[0] = i_ms_overheat_initiate_mode_1;

        overheatVoiceIndicator(i_ms_overheat_initiate_mode_1);
      }
    break;
  }
}

void overheatTimerDecrement(uint8_t i_tmp_power_level) {
  switch(i_tmp_power_level) {
    case 5:
      if(i_ms_overheat_initiate_mode_5 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheat_initiate_mode_5 = i_ms_overheat_initiate_mode_5 - i_overheat_delay_increment;
        i_ms_overheat_initiate[4] = i_ms_overheat_initiate_mode_5;

        overheatVoiceIndicator(i_ms_overheat_initiate_mode_5);
      }
    break;

    case 4:
      if(i_ms_overheat_initiate_mode_4 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheat_initiate_mode_4 = i_ms_overheat_initiate_mode_4 - i_overheat_delay_increment;
        i_ms_overheat_initiate[3] = i_ms_overheat_initiate_mode_4;

        overheatVoiceIndicator(i_ms_overheat_initiate_mode_4);
      }
    break;

    case 3:
      if(i_ms_overheat_initiate_mode_3 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheat_initiate_mode_3 = i_ms_overheat_initiate_mode_3 - i_overheat_delay_increment;
        i_ms_overheat_initiate[2] = i_ms_overheat_initiate_mode_3;

        overheatVoiceIndicator(i_ms_overheat_initiate_mode_3);
      }
    break;

    case 2:
      if(i_ms_overheat_initiate_mode_2 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheat_initiate_mode_2 = i_ms_overheat_initiate_mode_2 - i_overheat_delay_increment;
        i_ms_overheat_initiate[1] = i_ms_overheat_initiate_mode_2;

        overheatVoiceIndicator(i_ms_overheat_initiate_mode_2);
      }
    break;

    case 1:
    default:
      if(i_ms_overheat_initiate_mode_1 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheat_initiate_mode_1 = i_ms_overheat_initiate_mode_1 - i_overheat_delay_increment;
        i_ms_overheat_initiate[0] = i_ms_overheat_initiate_mode_1;

        overheatVoiceIndicator(i_ms_overheat_initiate_mode_1);
      }
    break;
  }
}

// Top rotary dial on the wand.
void checkRotaryEncoder() {
  static int8_t c, val;

  if((val = readRotary())) {
    c += val;
    switch(WAND_ACTION_STATUS) {
      case ACTION_CONFIG_EEPROM_MENU:
        // Counter clockwise.
        if(prev_next_code == 0x0b) {
          if(WAND_MENU_LEVEL == MENU_LEVEL_3 && i_wand_menu == 5 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            // Adjust the default bootup system volume.
            wandSerialSend(W_VOLUME_DECREASE_EEPROM);

            // If there is no Pack, we need to adjust the volume manually
            if(b_gpstar_benchtest == true) {
              decreaseVolumeEEPROM();
            }
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 5 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_5);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 4 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_4);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 3 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_3);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 2 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_2);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 1 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_1);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 5 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            overheatTimerDecrement(5);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 4 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            overheatTimerDecrement(4);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 3 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            overheatTimerDecrement(3);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 2 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            overheatTimerDecrement(2);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 1 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            overheatTimerDecrement(1);
          }
          else if(i_wand_menu - 1 < 1) {
            switch(WAND_MENU_LEVEL) {
              case MENU_LEVEL_1:
                WAND_MENU_LEVEL = MENU_LEVEL_2;
                i_wand_menu = 5;

                // Turn on some lights to visually indicate which menu we are in.
                analogWrite(led_slo_blo, 255); // Level 2

                // Turn off the other lights.
                digitalWrite(led_vent, HIGH); // Level 3
                digitalWrite(led_white, HIGH); // Level 4
                analogWrite(led_front_left, 0); // Level 5

                // Play an indication beep to notify we have changed menu levels.
                stopEffect(S_BEEPS);
                playEffect(S_BEEPS);

                stopEffect(S_LEVEL_1);
                stopEffect(S_LEVEL_2);
                stopEffect(S_LEVEL_3);
                stopEffect(S_LEVEL_4);
                stopEffect(S_LEVEL_5);

                playEffect(S_LEVEL_2);

                // Tell the Proton Pack to play some sounds.
                wandSerialSend(W_MENU_LEVEL_2);
              break;

              case MENU_LEVEL_2:
                WAND_MENU_LEVEL = MENU_LEVEL_3;
                i_wand_menu = 5;

                // Turn on some lights to visually indicate which menu we are in.
                analogWrite(led_slo_blo, 255); // Level 2
                digitalWrite(led_vent, LOW); // Level 3

                // Turn off the other lights.
                digitalWrite(led_white, HIGH); // Level 4
                analogWrite(led_front_left, 0); // Level 5

                // Play an indication beep to notify we have changed menu levels.
                stopEffect(S_BEEPS);
                playEffect(S_BEEPS);

                stopEffect(S_LEVEL_1);
                stopEffect(S_LEVEL_2);
                stopEffect(S_LEVEL_3);
                stopEffect(S_LEVEL_4);
                stopEffect(S_LEVEL_5);

                playEffect(S_LEVEL_3);

                // Tell the Proton Pack to play some sounds.
                wandSerialSend(W_MENU_LEVEL_3);
              break;

              case MENU_LEVEL_3:
                WAND_MENU_LEVEL = MENU_LEVEL_4;
                i_wand_menu = 5;

                // Turn on some lights to visually indicate which menu we are in.
                analogWrite(led_slo_blo, 255); // Level 2
                digitalWrite(led_vent, LOW); // Level 3
                digitalWrite(led_white, LOW); // Level 4

                // Turn off the other lights.
                analogWrite(led_front_left, 0); // Level 5

                // Play an indication beep to notify we have changed menu levels.
                stopEffect(S_BEEPS);
                playEffect(S_BEEPS);

                stopEffect(S_LEVEL_1);
                stopEffect(S_LEVEL_2);
                stopEffect(S_LEVEL_3);
                stopEffect(S_LEVEL_4);
                stopEffect(S_LEVEL_5);

                playEffect(S_LEVEL_4);

                // Tell the Proton Pack to play some sounds.
                wandSerialSend(W_MENU_LEVEL_4);
              break;

              case MENU_LEVEL_4:
                WAND_MENU_LEVEL = MENU_LEVEL_5;
                i_wand_menu = 5;

                // Turn on some lights to visually indicate which menu we are in.
                analogWrite(led_slo_blo, 255); // Level 2
                digitalWrite(led_vent, LOW); // Level 3
                digitalWrite(led_white, LOW); // Level 4
                analogWrite(led_front_left, 255); // Level 5

                // Play an indication beep to notify we have changed menu levels.
                stopEffect(S_BEEPS);
                playEffect(S_BEEPS);

                stopEffect(S_LEVEL_1);
                stopEffect(S_LEVEL_2);
                stopEffect(S_LEVEL_3);
                stopEffect(S_LEVEL_4);
                stopEffect(S_LEVEL_5);

                playEffect(S_LEVEL_5);

                // Tell the Proton Pack to play some sounds.
                wandSerialSend(W_MENU_LEVEL_5);
              break;

              // Menu 5 the deepest level.
              case MENU_LEVEL_5:
              default:
                i_wand_menu = 1;
              break;
            }
          }
          else {
            i_wand_menu--;
          }
        }

        // Clockwise.
        if(prev_next_code == 0x07) {
          if(WAND_MENU_LEVEL == MENU_LEVEL_3 && i_wand_menu == 5 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            // Adjust the default bootup system volume.
            wandSerialSend(W_VOLUME_INCREASE_EEPROM);

            // If there is no Pack, we need to adjust the volume manually
            if(b_gpstar_benchtest == true) {
              increaseVolumeEEPROM();
            }
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 5 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_5);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 4 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_4);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 3 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_3);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 2 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_2);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 1 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_1);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 5 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            overheatTimerIncrement(5);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 4 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            overheatTimerIncrement(4);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 3 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            overheatTimerIncrement(3);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 2 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            overheatTimerIncrement(2);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 1 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            overheatTimerIncrement(1);
          }
          else if(i_wand_menu + 1 > 5) {
            switch(WAND_MENU_LEVEL) {
              case MENU_LEVEL_5:
                WAND_MENU_LEVEL = MENU_LEVEL_4;
                i_wand_menu = 1;

                // Turn on some lights to visually indicate which menu we are in.
                analogWrite(led_slo_blo, 255); // Level 2
                digitalWrite(led_vent, LOW); // Level 3
                digitalWrite(led_white, LOW); // Level 4

                // Turn off the other lights.
                analogWrite(led_front_left, 0); // Level 5

                // Play an indication beep to notify we have changed menu levels.
                stopEffect(S_BEEPS);
                playEffect(S_BEEPS);

                stopEffect(S_LEVEL_1);
                stopEffect(S_LEVEL_2);
                stopEffect(S_LEVEL_3);
                stopEffect(S_LEVEL_4);
                stopEffect(S_LEVEL_5);

                playEffect(S_LEVEL_4);

                // Tell the Proton Pack to play some sounds.
                wandSerialSend(W_MENU_LEVEL_4);
              break;

              case MENU_LEVEL_4:
                WAND_MENU_LEVEL = MENU_LEVEL_3;
                i_wand_menu = 1;

                // Turn on some lights to visually indicate which menu we are in.
                analogWrite(led_slo_blo, 255); // Level 2
                digitalWrite(led_vent, LOW); // Level 3

                // Turn off the other lights.
                digitalWrite(led_white, HIGH); // Level 4
                analogWrite(led_front_left, 0); // Level 5

                // Play an indication beep to notify we have changed menu levels.
                stopEffect(S_BEEPS);
                playEffect(S_BEEPS);

                stopEffect(S_LEVEL_1);
                stopEffect(S_LEVEL_2);
                stopEffect(S_LEVEL_3);
                stopEffect(S_LEVEL_4);
                stopEffect(S_LEVEL_5);

                playEffect(S_LEVEL_3);

                // Tell the Proton Pack to play some sounds.
                wandSerialSend(W_MENU_LEVEL_3);
              break;

              case MENU_LEVEL_3:
                WAND_MENU_LEVEL = MENU_LEVEL_2;
                i_wand_menu = 1;

                // Turn on some lights to visually indicate which menu we are in.
                analogWrite(led_slo_blo, 255); // Level 2

                // Turn off the other lights.
                digitalWrite(led_vent, HIGH); // Level 3
                digitalWrite(led_white, HIGH); // Level 4
                analogWrite(led_front_left, 0); // Level 5

                // Play an indication beep to notify we have changed menu levels.
                stopEffect(S_BEEPS);
                playEffect(S_BEEPS);

                stopEffect(S_LEVEL_1);
                stopEffect(S_LEVEL_2);
                stopEffect(S_LEVEL_3);
                stopEffect(S_LEVEL_4);
                stopEffect(S_LEVEL_5);

                playEffect(S_LEVEL_2);

                // Tell the Proton Pack to play some sounds.
                wandSerialSend(W_MENU_LEVEL_2);
              break;

              case MENU_LEVEL_2:
                WAND_MENU_LEVEL = MENU_LEVEL_1;
                i_wand_menu = 1;

                // Turn off the other lights.
                analogWrite(led_slo_blo, 0); // Level 2
                digitalWrite(led_vent, HIGH); // Level 3
                digitalWrite(led_white, HIGH); // Level 4
                analogWrite(led_front_left, 0); // Level 5

                // Play an indication beep to notify we have changed menu levels.
                stopEffect(S_BEEPS);
                playEffect(S_BEEPS);

                stopEffect(S_LEVEL_1);
                stopEffect(S_LEVEL_2);
                stopEffect(S_LEVEL_3);
                stopEffect(S_LEVEL_4);
                stopEffect(S_LEVEL_5);

                playEffect(S_LEVEL_1);

                // Tell the Proton Pack to play some sounds.
                wandSerialSend(W_MENU_LEVEL_1);
              break;

              case MENU_LEVEL_1:
              default:
                // Cannot go any further than menu level 1.
                i_wand_menu = 5;
              break;
            }
          }
          else {
            i_wand_menu++;
          }
        }
      break;

      case ACTION_LED_EEPROM_MENU:
        // Counter clockwise.
        if(prev_next_code == 0x0b) {
          if(i_wand_menu == 4 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            // Change colour of the wand barrel spectral custom colour.
            if(i_spectral_wand_custom_colour > 1 && i_spectral_wand_custom_saturation > 253) {
              i_spectral_wand_custom_colour--;
            }
            else {
              i_spectral_wand_custom_colour = 1;

              if(i_spectral_wand_custom_saturation > 1) {
                i_spectral_wand_custom_saturation--;
              }
              else {
                i_spectral_wand_custom_saturation = 1;
              }
            }

            wandBarrelSpectralCustomConfigOn();
          }
          else if(i_wand_menu == 3 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            // Change colour of the Power Cell Spectral custom colour.
            wandSerialSend(W_SPECTRAL_POWERCELL_CUSTOM_DECREASE);
          }
          else if(i_wand_menu == 2 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            // Change colour of the Cyclotron Spectral custom colour.
            wandSerialSend(W_SPECTRAL_CYCLOTRON_CUSTOM_DECREASE);
          }
          else if(i_wand_menu == 1 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            // Change colour of the Inner Cyclotron Spectral custom colour.
            wandSerialSend(W_SPECTRAL_INNER_CYCLOTRON_CUSTOM_DECREASE);
          }
          else if(i_wand_menu - 1 < 1) {
            i_wand_menu = 1;
          }
          else {
            i_wand_menu--;
          }
        }

        // Clockwise.
        if(prev_next_code == 0x07) {
          if(i_wand_menu == 4 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            // Change colour of the Wand Barrel Spectral custom colour.
            if(i_spectral_wand_custom_saturation < 254) {
              i_spectral_wand_custom_saturation++;

              if(i_spectral_wand_custom_saturation > 253) {
                i_spectral_wand_custom_saturation = 254;
              }
            }
            else if(i_spectral_wand_custom_colour < 253 && i_spectral_wand_custom_saturation > 253) {
              i_spectral_wand_custom_colour++;
            }
            else {
              i_spectral_wand_custom_colour = 254;

              if(i_spectral_wand_custom_saturation < 253) {
                i_spectral_wand_custom_saturation++;
              }
              else {
                i_spectral_wand_custom_saturation = 254;
              }
            }

            wandBarrelSpectralCustomConfigOn();
          }
          else if(i_wand_menu == 3 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            // Change colour of the Power Cell Spectral custom colour.
            wandSerialSend(W_SPECTRAL_POWERCELL_CUSTOM_INCREASE);
          }
          else if(i_wand_menu == 2 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            // Change colour of the Cyclotron Spectral custom colour.
            wandSerialSend(W_SPECTRAL_CYCLOTRON_CUSTOM_INCREASE);
          }
          else if(i_wand_menu == 1 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW) {
            // Change colour of the Inner Cyclotron Spectral custom colour.
            wandSerialSend(W_SPECTRAL_INNER_CYCLOTRON_CUSTOM_INCREASE);
          }
          else if(i_wand_menu + 1 > 5) {
            i_wand_menu = 5;
          }
          else {
            i_wand_menu++;
          }
        }
      break;

      case ACTION_SETTINGS:
        // Counter clockwise.
        if(prev_next_code == 0x0b) {
          if(i_wand_menu == 4 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            // Tell pack to dim the selected lighting. (Power Cell, Cyclotron or Inner Cyclotron)
            wandSerialSend(W_DIMMING_DECREASE);
          }
          else if(i_wand_menu == 3 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            // Lower the sound effects volume.
            decreaseVolumeEffects();

            // Tell pack to lower the sound effects volume.
            wandSerialSend(W_VOLUME_SOUND_EFFECTS_DECREASE);
          }
          else if(i_wand_menu == 3 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW && b_playing_music == true) {
            // Decrease the music volume.
            if(i_volume_music_percentage - VOLUME_MUSIC_MULTIPLIER < 0) {
              i_volume_music_percentage = 0;

              // Provide feedback at minimum volume.
              stopEffect(S_BEEPS_ALT);
              playEffect(S_BEEPS_ALT, false, i_volume_master - 10);
            }
            else {
              i_volume_music_percentage = i_volume_music_percentage - VOLUME_MUSIC_MULTIPLIER;
            }

            i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

            updateMusicVolume();

            // Tell pack to lower the music volume.
            wandSerialSend(W_VOLUME_MUSIC_DECREASE);
          }
          else if(i_wand_menu - 1 < 1) {
            // We are entering the sub menu. Only accessible when the Neutrona Wand is powered down.
            if(WAND_STATUS == MODE_OFF) {
              switch(WAND_MENU_LEVEL) {
                case MENU_LEVEL_1:
                  WAND_MENU_LEVEL = MENU_LEVEL_2;
                  i_wand_menu = 5;

                  // Turn on the slo blow led to indicate we are in the Neutrona Wand sub menu.
                  analogWrite(led_slo_blo, 255);

                  // Play an indication beep to notify we have changed menu levels.
                  stopEffect(S_BEEPS);
                  playEffect(S_BEEPS);

                  stopEffect(S_LEVEL_1);
                  stopEffect(S_LEVEL_2);
                  stopEffect(S_LEVEL_3);
                  stopEffect(S_LEVEL_4);
                  stopEffect(S_LEVEL_5);

                  playEffect(S_LEVEL_2);

                  // Tell the Proton Pack to play some sounds.
                  wandSerialSend(W_MENU_LEVEL_2);
                break;

                case MENU_LEVEL_2:
                default:
                  // Cannot go further than level 2 for this menu.
                  i_wand_menu = 1;
                break;
              }
            }
            else {
              i_wand_menu = 1;
            }
          }
          else {
            i_wand_menu--;
          }
        }

        // Clockwise.
        if(prev_next_code == 0x07) {
          if(i_wand_menu == 4 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            // Tell pack to dim the selected lighting. (Power Cell, Cyclotron or Inner Cyclotron)
            wandSerialSend(W_DIMMING_INCREASE);
          }
          else if(i_wand_menu == 3 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.getState() == LOW && switch_mode.getState() == HIGH) {
            // Increase sound effects volume.
            increaseVolumeEffects();

            // Tell pack to increase the sound effects volume.
            wandSerialSend(W_VOLUME_SOUND_EFFECTS_INCREASE);
          }
          else if(i_wand_menu == 3 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.getState() == HIGH && switch_mode.getState() == LOW && b_playing_music == true) {
            // Increase music volume.
            if(i_volume_music_percentage + VOLUME_MUSIC_MULTIPLIER > 100) {
              i_volume_music_percentage = 100;

              // Provide feedback at maximum volume.
              stopEffect(S_BEEPS_ALT);
              playEffect(S_BEEPS_ALT, false, i_volume_master - 10);
            }
            else {
              i_volume_music_percentage = i_volume_music_percentage + VOLUME_MUSIC_MULTIPLIER;
            }

            i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

            updateMusicVolume();

            // Tell pack to increase music volume.
            wandSerialSend(W_VOLUME_MUSIC_INCREASE);
          }
          else if(i_wand_menu + 1 > 5) {
            // We are leaving changing menu levels. Only accessible when the Neutrona Wand is powered down.
            if(WAND_STATUS == MODE_OFF) {
              switch(WAND_MENU_LEVEL) {
                case MENU_LEVEL_2:
                  WAND_MENU_LEVEL = MENU_LEVEL_1;

                  i_wand_menu = 1;

                  // Turn off the slo blow led to indicate we are no longer in the Neutrona Wand sub menu.
                  analogWrite(led_slo_blo, 0);

                  // Play an indication beep to notify we have changed menu levels.
                  stopEffect(S_BEEPS);
                  playEffect(S_BEEPS);

                  stopEffect(S_LEVEL_1);
                  stopEffect(S_LEVEL_2);
                  stopEffect(S_LEVEL_3);
                  stopEffect(S_LEVEL_4);
                  stopEffect(S_LEVEL_5);

                  playEffect(S_LEVEL_1);

                  // Tell the Proton Pack to play some sounds.
                  wandSerialSend(W_MENU_LEVEL_1);
                break;

                case MENU_LEVEL_1:
                default:
                  // Level 1 is the first menu and nothing above it.
                  i_wand_menu = 5;
                break;
              }
            }
            else {
              i_wand_menu = 5;
            }
          }
          else {
            i_wand_menu++;
          }
        }
      break;

      default:
        if(WAND_ACTION_STATUS != ACTION_OVERHEATING && b_pack_alarm != true) {
          if(WAND_ACTION_STATUS == ACTION_FIRING && i_power_mode == i_power_mode_max) {
            // Do nothing, we are locked in full power mode while firing.
          }
          // Counter clockwise.
          else if(prev_next_code == 0x0b) {
            // Check to see the minimal power mode depending on which system mode.
            uint8_t i_tmp_power_mode_min = i_power_mode_min;

            switch(SYSTEM_MODE) {
              case MODE_ORIGINAL:
                i_tmp_power_mode_min = i_power_mode_min + 1;
              break;

              case MODE_SUPER_HERO:
              default:
                i_tmp_power_mode_min = i_power_mode_min;
              break;
            }

            if(i_power_mode - 1 >= i_tmp_power_mode_min && WAND_STATUS == MODE_ON) {
              i_power_mode_prev = i_power_mode;
              i_power_mode--;

              if(BARGRAPH_MODE == BARGRAPH_ORIGINAL && b_28segment_bargraph == true) {
                bargraphPowerCheck2021Alt(false);
              }

              // Forces a redraw of the bargraph if firing while changing the power level in the BARGRAPH_ANIMATION_ORIGINAL.
              if(b_firing == true && b_28segment_bargraph == true && BARGRAPH_FIRING_ANIMATION == BARGRAPH_ANIMATION_ORIGINAL) {
                bargraphRedraw();
              }

              soundBeepLoopStop();

              switch(getSystemYearMode()) {
                case SYSTEM_1984:
                case SYSTEM_1989:
                  if(switch_vent.getState() == LOW) {
                    soundIdleLoopStop();
                    soundIdleLoop(false);
                  }
                break;

                case SYSTEM_AFTERLIFE:
                case SYSTEM_FROZEN_EMPIRE:
                default:
                    soundIdleLoopStop();
                    soundIdleLoop(false);
                break;
              }

              updatePackPowerLevel();
            }

            // Decrease the music volume if the wand/pack is off. A quick easy way to adjust the music volume on the go.
            if(WAND_STATUS == MODE_OFF && FIRING_MODE != SETTINGS && b_playing_music == true) {
              if(i_volume_music_percentage - VOLUME_MUSIC_MULTIPLIER < 0) {
                i_volume_music_percentage = 0;
              }
              else {
                i_volume_music_percentage = i_volume_music_percentage - VOLUME_MUSIC_MULTIPLIER;
              }

              i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

              updateMusicVolume();

              // Tell pack to lower music volume.
              wandSerialSend(W_VOLUME_MUSIC_DECREASE);
            }
          }

          if(WAND_ACTION_STATUS == ACTION_FIRING && i_power_mode == i_power_mode_max) {
            // Do nothing, we are locked in full power mode while firing.
          }
          // Clockwise.
          else if(prev_next_code == 0x07) {
            if(i_power_mode + 1 <= i_power_mode_max && WAND_STATUS == MODE_ON) {
              if(i_power_mode + 1 == i_power_mode_max && WAND_ACTION_STATUS == ACTION_FIRING) {
                // Do nothing, we do not want to go into max power mode if firing in a lower power mode already.
              }
              else {
                i_power_mode_prev = i_power_mode;
                i_power_mode++;

                if(BARGRAPH_MODE == BARGRAPH_ORIGINAL && b_28segment_bargraph == true) {
                  bargraphPowerCheck2021Alt(false);
                }

                // Forces a redraw of the bargraph if firing while changing the power level if using BARGRAPH_ANIMATION_ORIGINAL.
                if(b_firing == true && b_28segment_bargraph == true && BARGRAPH_FIRING_ANIMATION == BARGRAPH_ANIMATION_ORIGINAL) {
                  bargraphRedraw();
                }

                soundBeepLoopStop();

                switch(getNeutronaWandYearMode()) {
                  case SYSTEM_1984:
                  case SYSTEM_1989:
                    if(switch_vent.getState() == LOW) {
                      soundIdleLoopStop();
                      soundIdleLoop(false);
                    }
                  break;

                  case SYSTEM_AFTERLIFE:
                  case SYSTEM_FROZEN_EMPIRE:
                  default:
                      soundIdleLoopStop();
                      soundIdleLoop(false);
                  break;
                }

                updatePackPowerLevel();
              }
            }

            // Increase the music volume if the wand/pack is off. A quick easy way to adjust the music volume on the go.
            if(WAND_STATUS == MODE_OFF && FIRING_MODE != SETTINGS && b_playing_music == true) {
              if(i_volume_music_percentage + VOLUME_MUSIC_MULTIPLIER > 100) {
                i_volume_music_percentage = 100;
              }
              else {
                i_volume_music_percentage = i_volume_music_percentage + VOLUME_MUSIC_MULTIPLIER;
              }

              i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

              updateMusicVolume();

              // Tell pack to increase music volume.
              wandSerialSend(W_VOLUME_MUSIC_INCREASE);
            }
          }
        }
      break;
    }
  }
}

// Tell the pack which power level the Neutrona Wand is set at.
void updatePackPowerLevel() {
  switch(i_power_mode) {
    case 5:
      // Level 5
      wandSerialSend(W_POWER_LEVEL_5);
    break;

    case 4:
      // Level 4
      wandSerialSend(W_POWER_LEVEL_4);
    break;

    case 3:
      // Level 3
      wandSerialSend(W_POWER_LEVEL_3);
    break;

    case 2:
      // Level 2
      wandSerialSend(W_POWER_LEVEL_2);
    break;

    default:
      // Level 1
      wandSerialSend(W_POWER_LEVEL_1);
    break;
  }
}

void vibrationSetting() {
  if(b_vibration_on == true) {
    if(ms_bargraph.isRunning() == false && WAND_ACTION_STATUS != ACTION_FIRING) {
      switch(i_power_mode) {
        case 1:
          vibrationWand(i_vibration_level);
        break;

        case 2:
          vibrationWand(i_vibration_level + 5);
        break;

        case 3:
          vibrationWand(i_vibration_level + 10);
        break;

        case 4:
          vibrationWand(i_vibration_level + 12);
        break;

        case 5:
          vibrationWand(i_vibration_level + 25);
        break;
      }
    }
  }
  else {
    vibrationOff();
  }
}

void switchLoops() {
  switch_intensify.loop();
  switch_activate.loop();
  switch_vent.loop();
  switch_wand.loop();
  switch_mode.loop();
  switch_barrel.loop();
}

void wandBarrelLightsOff() {
  for(uint8_t i = 0; i < i_num_barrel_leds; i++) {
    switch(WAND_BARREL_LED_COUNT) {
      case LEDS_48:
        // Set the tip of the Frutto LED array to white.
        barrel_leds[frutto_barrel[i]] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
      break;

      case LEDS_5:
      default:
        // Illuminate the wand barrel tip LED.
        barrel_leds[i] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
      break;
    }
  }

  ms_fast_led.start(i_fast_led_delay);
}

// Exit the wand menu system while the wand is off.
void wandExitMenu() {
  FIRING_MODE = PREV_FIRING_MODE;

  i_wand_menu = 5;

  if(b_pack_alarm != true) {
    playEffect(S_CLICK);
  }

  bargraphClearAlt();

  switch(PREV_FIRING_MODE) {
    case MESON:
      // Tell the pack we are in meson mode.
      wandSerialSend(W_MESON_MODE);
    break;

    case STASIS:
      // Tell the pack we are in stasis mode.
      wandSerialSend(W_STASIS_MODE);
    break;

    case SLIME:
      // Tell the pack we are in slime mode.
      wandSerialSend(W_SLIME_MODE);
    break;

    case SPECTRAL:
      // Tell the pack we are in spectral mode.
      wandSerialSend(W_SPECTRAL_MODE);
    break;

    case HOLIDAY:
      // Tell the pack we are in holiday mode.
      wandSerialSend(W_HOLIDAY_MODE);
    break;

    case SPECTRAL_CUSTOM:
      // Tell the pack we are in spectral custom mode.
      wandSerialSend(W_SPECTRAL_CUSTOM_MODE);
    break;

    case VENTING:
      // Tell the pakc we are in venting mode.
      wandSerialSend(W_VENTING_MODE);
    break;

    case PROTON:
    default:
      // Tell the pack we are in proton mode.
      wandSerialSend(W_PROTON_MODE);
    break;
  }

  WAND_ACTION_STATUS = ACTION_IDLE;

  wandLightsOff();

  // In original mode, we need to re-initalise the 28 segment bargraph if some switches are already toggled on.
  if(SYSTEM_MODE == MODE_ORIGINAL) {
    if(switch_vent.getState() == LOW && switch_wand.getState() == LOW) {
      if(b_pack_ion_arm_switch_on == true && b_28segment_bargraph == true && b_mode_original_toggle_sounds_enabled == true) {
        if(b_extra_pack_sounds == true) {
          wandSerialSend(W_MODE_ORIGINAL_HEATUP);
        }

        stopEffect(S_WAND_HEATUP_ALT);
        stopEffect(S_WAND_HEATUP);
        playEffect(S_WAND_HEATUP);
        playEffect(S_WAND_HEATUP_ALT);
      }

      if(b_28segment_bargraph == true) {
        bargraphPowerCheck2021Alt(false);
        prepBargraphRampUp();
      }
    }
  }
}

// Exit the wand menu EEPROM system while the wand is off.
void wandExitEEPROMMenu() {
  playEffect(S_BEEPS_BARGRAPH);
  switch_intensify.resetCount();
  switch_wand.resetCount();
  switch_vent.resetCount();

  if(b_gpstar_benchtest == true) {
    // Also need to make sure to reset the "ion arm switch" to off if standalone
    b_pack_ion_arm_switch_on = false;
  }

  i_wand_menu = 5;

  bargraphClearAlt();

  WAND_ACTION_STATUS = ACTION_IDLE;

  wandLightsOff();
  wandBarrelLightsOff();

  // Send current preferences to the pack for use by the serial1 device.
  wandSerialSend(W_SEND_PREFERENCES_WAND);
  wandSerialSend(W_SEND_PREFERENCES_SMOKE);
}

// Barrel safety switch is connected to analog pin 7.
bool switchBarrel() {
  if(switch_barrel.getState() == LOW) {
    if(b_switch_barrel_extended == true) {
      wandSerialSend(W_BARREL_RETRACTED);
    }

    b_switch_barrel_extended = false;
  }
  else if(switch_barrel.getState() == HIGH) {
    // Play the Afterlife Barrel extension sound effect.
    if((getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) && b_switch_barrel_extended != true) {
      if(b_extra_pack_sounds == true) {
        wandSerialSend(W_AFTERLIFE_WAND_BARREL_EXTEND);
      }

      // Plays the "thwoop" barrel extension sound in Afterlife mode.
      playEffect(S_AFTERLIFE_WAND_BARREL_EXTEND, false, i_volume_effects - 1);
    }

    if(b_switch_barrel_extended != true) {
      wandSerialSend(W_BARREL_EXTENDED);
    }

    b_switch_barrel_extended = true;
  }

  return b_switch_barrel_extended; // Immediate return of state.
}

void stopAfterLifeSounds() {
  stopEffect(S_AFTERLIFE_WAND_RAMP_1);
  stopEffect(S_AFTERLIFE_WAND_IDLE_1);

  stopEffect(S_AFTERLIFE_WAND_RAMP_2);
  stopEffect(S_AFTERLIFE_WAND_IDLE_2);
  stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1);
  stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2);

  stopEffect(S_AFTERLIFE_WAND_RAMP_2_FADE_IN);
  stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2_FADE_OUT);
}

void afterLifeRamp1() {
  stopAfterLifeSounds();

  playEffect(S_AFTERLIFE_WAND_RAMP_1, false, i_volume_effects - 1);
  b_sound_afterlife_idle_2_fade = false;

  ms_gun_loop_1.start(1660);

  if(b_extra_pack_sounds == true) {
    wandSerialSend(W_AFTERLIFE_GUN_RAMP_1);
  }
}

// Rebuilds the overheat enable array.
void resetOverHeatModes() {
  b_overheat_mode[0] = b_overheat_mode_1;
  b_overheat_mode[1] = b_overheat_mode_2;
  b_overheat_mode[2] = b_overheat_mode_3;
  b_overheat_mode[3] = b_overheat_mode_4;
  b_overheat_mode[4] = b_overheat_mode_5;
}

// Included last as the contained logic will control all aspects of the pack using the defined functions above.
#include "Actions.h"
#include "Serial.h"
