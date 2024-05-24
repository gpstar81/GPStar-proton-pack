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
  setupAudioDevice();

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

  switch_vent.setPushedCallback(&ventSwitched);
  switch_wand.setPushedCallback(&wandSwitched);

  // Rotary encoder on the top of the wand.
  pinModeFast(r_encoderA, INPUT_PULLUP);
  pinModeFast(r_encoderB, INPUT_PULLUP);

  // Setup the bargraph.
  bargraphYearModeUpdate();

  delay(10);

  WIRE.begin();

  byte by_error, by_address;
  uint8_t i_i2c_devices = 0;

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
    pinModeFast(led_bargraph_1, OUTPUT);
    pinModeFast(led_bargraph_2, OUTPUT);
    pinModeFast(led_bargraph_3, OUTPUT);
    pinModeFast(led_bargraph_4, OUTPUT);
    pinModeFast(led_bargraph_5, OUTPUT);
  }

  pinModeFast(led_slo_blo, OUTPUT);

  pinModeFast(led_front_left, OUTPUT); // Front left LED underneath the Clippard valve.
  pinModeFast(led_hat_1, OUTPUT); // Hat light at front of the wand near the barrel tip.
  pinModeFast(led_hat_2, OUTPUT); // Hat light at top of the wand body (gun box).
  pinModeFast(led_barrel_tip, OUTPUT); // LED at the tip of the wand barrel.

  pinMode(led_vent, OUTPUT); // Vent light could be either Digital or PWM based on user setting, so use default functions.
  pinModeFast(led_white, OUTPUT);

  pinMode(vibration, OUTPUT); // Vibration motor is PWM, so fallback to default pinMode just to be safe.

  // Make sure lights are off.
  wandLightsOff();

  // Wand status.
  WAND_STATUS = MODE_OFF;
  WAND_ACTION_STATUS = ACTION_IDLE;

  ms_reset_sound_beep.start(i_sound_timer);

  // We bootup the wand in the classic proton mode.
  FIRING_MODE = PROTON;

  // Select a random GB1/GB2 white LED blink rate for this session.
  i_classic_blink_index = random(0,5);

  // Load any saved settings stored in the EEPROM memory of the GPStar Neutrona Wand.
  if(b_eeprom == true) {
    readEEPROM();
  }

  // Start the button mash check timer.
  ms_bmash.start(0);

  // Sanity check just in case a user forgot to enable CTS while enabling CTS Mix.
  if(b_cross_the_streams_mix == true && b_cross_the_streams != true) {
    b_cross_the_streams = true;
  }

  // Check if we should be in video game mode or not.
  vgModeCheck();

  // Start up some timers for MODE_ORIGINAL.
  ms_slo_blo_blink.start(i_slo_blo_blink_delay);

  // Initialize the fastLED state update timer.
  ms_fast_led.start(i_fast_led_delay);

  // Initialize the timer for initial handshake.
  ms_packsync.start(0);

  if(b_gpstar_benchtest == true) {
    WAND_CONN_STATE = NC_BENCHTEST;

    b_pack_on = true; // Pretend that the pack (not really attached) has been powered on.

    // Stop the pack sync timer since we are no longer syncing to a pack.
    ms_packsync.stop();

    // Check music timer for bench test mode only.
    ms_check_music.start(i_music_check_delay);

    // No pack to do a volume sync with, so reset our master volume manually.
    resetMasterVolume();
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
        digitalWriteFast(led_white, (b_sync_light ? HIGH : LOW)); // Blink an LED.
      }

      checkPack(); // Check for any response from the pack while still waiting.
    break;

    case SYNCHRONIZING:
      // Currently unused.
      checkPack(); // Keep checking for responses from the pack while synchronizing.
    break;

    case PACK_CONNECTED:
      // When connected to a pack, prepare to send a regular handshake to indicate presence.
      if(ms_handshake.justFinished()) {
        wandSerialSend(W_HANDSHAKE); // Remind the pack that a wand is still present.
        ms_handshake.restart(); // Restart the handshake timer.
      }

      updateAudio(); // Update the state of the selected sound board.

      checkPack(); // Get the latest communications from the connected Proton Pack.

      mainLoop(); // Continue on to the main loop.
    break;

    case NC_BENCHTEST:
      updateAudio(); // Update the state of the selected sound board.

      checkMusic(); // Music control is here since pack is not present.

      mainLoop(); // Continue on to the main loop.
    break;
  }
}

void mainLoop() {
  // Get the current state of any input devices (toggles, buttons, and switches).
  switchLoops();
  checkSwitches();
  checkRotaryEncoder();

  if(WAND_ACTION_STATUS != ACTION_FIRING) {
    if(ms_bmash.remaining() < 1) {
      // Clear counter until user begins firing (post any lock-out period).
      i_bmash_count = 0;

      if(b_wand_mash_error == true) {
        // Return the wand to a normal firing state after lock-out from button mashing.
        b_wand_mash_error = false;

        WAND_STATUS = MODE_ON;
        WAND_ACTION_STATUS = ACTION_IDLE;

        postActivation();

        stopEffect(S_SMASH_ERROR_LOOP);
        playEffect(S_SMASH_ERROR_RESTART);

        if(b_extra_pack_sounds == true) {
          wandSerialSend(W_SMASH_ERROR_RESTART);
        }

        /*
        if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
          if(b_extra_pack_sounds == true) {
            wandSerialSend(W_WAND_BOOTUP_SOUND);
          }

          stopEffect(S_WAND_BOOTUP);
          playEffect(S_WAND_BOOTUP);
        }
        */

        bargraphClearAlt();
      }
    }
  }

  switch(WAND_STATUS) {
    case MODE_OFF:
      if(WAND_ACTION_STATUS != ACTION_LED_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU) {
        if(WAND_ACTION_STATUS != ACTION_SETTINGS && b_gpstar_benchtest == true && SYSTEM_MODE == MODE_ORIGINAL && switch_intensify.doubleClick()) {
          // This allows a standalone wand to "flip the ion arm switch" when in MODE_ORIGINAL by double-clicking the Intensify switch while the wand is turned off
          if(b_pack_ion_arm_switch_on == true) {
            b_pack_ion_arm_switch_on = false;
          }
          else {
            b_pack_ion_arm_switch_on = true;
          }
        }

        if(switch_mode.pushed() || b_pack_alarm == true) {
          if(WAND_ACTION_STATUS != ACTION_SETTINGS && b_pack_alarm != true && (b_pack_on != true || b_gpstar_benchtest == true)) {
            playEffect(S_CLICK);

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
            // Only exit the settings menu when on menu #5 in the top menu or the pack ribbon cable alarm is active.
            if(i_wand_menu == 5 && WAND_MENU_LEVEL == MENU_LEVEL_1 && WAND_ACTION_STATUS == ACTION_SETTINGS) {
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
      if(switch_intensify.on() == false) {
        wandSwitchedCount = 0;
        ventSwitchedCount = 0;
      }

      if(WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_LED_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU && (b_pack_on != true || b_gpstar_benchtest == true) && switch_intensify.on() == true && wandSwitchedCount >= 5) {
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
         && (b_pack_on != true || b_gpstar_benchtest == true) && switch_intensify.on() == true && ventSwitchedCount >= 5) {
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
                  digitalWriteFast(led_front_left, LOW);
                }
                else {
                  digitalWriteFast(led_front_left, HIGH);
                }
              }
              else {
                // When the top right wand switch is off, then we make sure the led is off as the Slo-Blo LED will be on or blinking at this point.
                if(switch_wand.on() == false) {
                  digitalWriteFast(led_front_left, LOW);
                }
              }
              break;

            case MODE_SUPER_HERO:
            default:
              if(ms_power_indicator_blink.remaining() < i_ms_power_indicator_blink / 2) {
                digitalWriteFast(led_front_left, LOW);
              }
              else {
                digitalWriteFast(led_front_left, HIGH);
              }
            break;
          }
        }
        else {
          digitalWriteFast(led_front_left, LOW);
        }
      }
    break;

    case MODE_ERROR:
      if(ms_hat_2.remaining() < i_hat_2_delay / 2) {
        digitalWriteFast(led_white, HIGH);

        digitalWriteFast(led_slo_blo, LOW);

        digitalWriteFast(led_hat_2, LOW);
        digitalWriteFast(led_front_left, LOW);
      }
      else {
        digitalWriteFast(led_hat_2, HIGH);
        digitalWriteFast(led_front_left, HIGH);

        digitalWriteFast(led_white, LOW);
        digitalWriteFast(led_slo_blo, HIGH);
      }

      if(ms_hat_2.justFinished()) {
        ms_hat_2.start(i_hat_2_delay);

        if(b_wand_mash_error != true) {
          if(b_extra_pack_sounds == true) {
            wandSerialSend(W_WAND_BEEP_SOUNDS);
          }

          playEffect(S_BEEPS_LOW);
          playEffect(S_BEEPS);
        }
      }

      if(ms_hat_1.justFinished()) {
        if(b_wand_mash_error != true) {

          if(b_extra_pack_sounds == true) {
            wandSerialSend(W_WAND_BEEP_BARGRAPH);
          }

          playEffect(S_BEEPS_BARGRAPH);
        }

        ms_hat_1.start(i_hat_2_delay * 4);
      }

      settingsBlinkingLights();

      if(b_wand_mash_error == true) {
        wandBarrelHeatUp();
      }
    break;

    case MODE_ON:
      if(b_vibration_on == true && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
        vibrationSetting();
      }

      // Hat light 2 blinking when the Proton Pack ribbon cable has been removed.
      if(b_pack_alarm == true) {
        if(ms_hat_2.remaining() < i_hat_2_delay / 2) {
          digitalWriteFast(led_hat_2, LOW);
        }
        else {
          digitalWriteFast(led_hat_2, HIGH);
        }

        if(ms_hat_2.justFinished()) {
          ms_hat_2.start(i_hat_2_delay);
        }
      }
      else {
        if(ms_hat_1.isRunning() != true && ms_hat_2.isRunning() != true && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          // Hat 2 stays solid while the Neutrona Wand is on in Afterlife/Frozen Empire. It will blink when about to overheat and turn off while overheating.
          if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
            digitalWriteFast(led_hat_2, HIGH);
          }
          else {
            digitalWriteFast(led_hat_2, LOW);
          }
        }
      }

      // Top white light.
      if(ms_white_light.justFinished()) {
        ms_white_light.repeat();
        if(digitalReadFast(led_white) == LOW) {
          digitalWriteFast(led_white, HIGH);
        }
        else {
          digitalWriteFast(led_white, LOW);
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
          if(ms_gun_loop_1.justFinished() && switch_vent.on() == false) {
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

  // Handle button press events based on current wand state and menu level (for config/EEPROM purposes).
  checkWandAction();

  if(b_firing == true && WAND_ACTION_STATUS != ACTION_FIRING) {
    modeFireStop();
  }

  // Play the firing pulse effect animation.
  if(ms_firing_pulse.justFinished()) {
    firePulseEffect();
  }

  // Play the firing effect end animation.
  if(ms_firing_effect_end.justFinished()) {
    fireEffectEnd();
  }

  // Play the firing stream end animation.
  if(ms_firing_lights_end.justFinished()) {
    fireStreamEnd(getHueColour(C_BLACK, WAND_BARREL_LED_COUNT));
  }

  // Update the barrel LEDs and restart the timer.
  if(ms_fast_led.justFinished()) {
    FastLED.show();
    ms_fast_led.start(i_fast_led_delay);
  }
}

// Sets the Neutrona Wand to video game mode.
void setVGMode() {
  SYSTEM_MODE = MODE_SUPER_HERO;
  b_cross_the_streams = false;
  b_cross_the_streams_mix = false;
}

// Checks if video game mode should be set.
bool vgModeCheck() {
  if(SYSTEM_MODE == MODE_ORIGINAL || b_cross_the_streams == true || b_cross_the_streams_mix == true) {
    // MODE_ORIGINAL does not support VG modes, so make sure CTS is enabled and firing mode is PROTON.
    if(SYSTEM_MODE == MODE_ORIGINAL && (b_cross_the_streams != true || FIRING_MODE != PROTON)) {
      b_cross_the_streams = true;
      FIRING_MODE = PROTON;
      modeCheck();
    }

    return false;
  }
  else {
    return true;
  }
}

void wandTipOn() {
  switch(WAND_BARREL_LED_COUNT) {
    case LEDS_48:
    {
      // Initialize temporary colour variable to reduce code complexity.
      colours c_temp = C_WHITE;

      if(getSystemYearMode() == SYSTEM_FROZEN_EMPIRE) {
        if(b_wand_mash_error == true || b_pack_alarm == true) {
          if(FIRING_MODE == PROTON && !b_pack_cyclotron_lid_on) {
            // Green goop effect if in Proton Stream mode and pack's cyclotron lid is off.
            c_temp = C_CHARTREUSE;
          }
          else {
            // Set the tip of the Frutto LED array to beige if playing the lockout/cable spark animation.
            c_temp = C_BEIGE;
          }
        }
        else if(b_firing_cross_streams == true && !b_pack_cyclotron_lid_on) {
          // Set the tip of the Frutto LED array to greenish if in Frozen Empire and using CTS mode.
          c_temp = C_CHARTREUSE;
        }
        else if(FIRING_MODE == SLIME) {
          if(getSystemYearMode() == SYSTEM_1989) {
            c_temp = C_PASTEL_PINK;
          }
          else {
            c_temp = C_DARK_GREEN;
          }
        }
      }
      else {
        if(b_wand_mash_error == true || b_pack_alarm == true) {
          // Set the tip of the Frutto LED array to beige if playing the lockout/cable spark animation.
          c_temp = C_BEIGE;
        }
        else if(FIRING_MODE == SLIME) {
          if(getSystemYearMode() == SYSTEM_1989) {
            c_temp = C_PASTEL_PINK;
          }
          else {
            c_temp = C_DARK_GREEN;
          }
        }
      }

      barrel_leds[12] = getHueColour(c_temp, WAND_BARREL_LED_COUNT);

      // Illuminate the wand barrel tip LED.
      if(FIRING_MODE != SLIME) {
        digitalWriteFast(led_barrel_tip, HIGH);
      }
    }
    break;

    case LEDS_5:
    default:
      // Illuminate the wand barrel tip LED.
      if(FIRING_MODE != SLIME) {
        digitalWriteFast(led_barrel_tip, HIGH);
      }
    break;
  }
}

void wandTipOff() {
  switch(WAND_BARREL_LED_COUNT) {
    case LEDS_48:
      // Set the tip of the Frutto LED array to black.
      barrel_leds[12] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);

      // Turn off the wand barrel tip LED.
      digitalWriteFast(led_barrel_tip, LOW);
    break;

    case LEDS_5:
    default:
      // Turn off the wand barrel tip LED.
      digitalWriteFast(led_barrel_tip, LOW);
    break;
  }
}

void wandTipSpark() {
  i_heatup_counter = 0;
  i_heatdown_counter = 100;
  i_bmash_spark_index = 0;
  ms_wand_heatup_fade.start(i_delay_heatup);
}

// Controlled from the Neutrona Wand Menu systems.
void toggleWandModes() {
  stopEffect(S_CLICK);
  playEffect(S_CLICK);

  stopEffect(S_VOICE_CROSS_THE_STREAMS);
  stopEffect(S_VOICE_CROSS_THE_STREAMS_MIX);
  stopEffect(S_VOICE_VIDEO_GAME_MODES);

  // Enable or disable crossing the streams / crossing the streams mix / video game modes.
  if(b_cross_the_streams == true && b_cross_the_streams_mix == true && SYSTEM_MODE != MODE_ORIGINAL) {
    // Turn off crossing the streams mix and switch back to video game mode.
    setVGMode();

    playEffect(S_VOICE_VIDEO_GAME_MODES);

    wandSerialSend(W_VIDEO_GAME_MODE);
  }
  else if(b_cross_the_streams == true && b_cross_the_streams_mix != true) {
    // Keep cross the streams on.
    b_cross_the_streams = true;

    // Turn on cross the streams mix.
    b_cross_the_streams_mix = true;

    playEffect(S_VOICE_CROSS_THE_STREAMS_MIX);

    wandSerialSend(W_CROSS_THE_STREAMS_MIX);
  }
  else {
    // Turn on crossing the streams mode and turn off video game mode.
    b_cross_the_streams = true;
    b_cross_the_streams_mix = false;

    playEffect(S_VOICE_CROSS_THE_STREAMS);

    wandSerialSend(W_CROSS_THE_STREAMS);
  }

  // Reset to proton stream.
  FIRING_MODE = PROTON;
  modeCheck();
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

  // Since the Proton Pack tells the Neutrona Wand when venting is finished, standalone wand needs its own timer.
  if(b_gpstar_benchtest == true) {
    ms_overheating.stop();
  }

  ms_settings_blinking.stop();

  // Turn off hat light 2.
  digitalWriteFast(led_hat_2, LOW);

  WAND_ACTION_STATUS = ACTION_IDLE;

  // Prepare a few things before ramping the bargraph back up from a full ramp down.
  if(b_overheat_bargraph_blink != true) {
    if(BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
      bargraphYearModeUpdate();
    }
    else {
      i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_1984 * 2;
    }
  }

  switch(getNeutronaWandYearMode()) {
    case SYSTEM_1984:
    case SYSTEM_1989:
      stopEffect(S_WAND_BOOTUP_SHORT);
      playEffect(S_WAND_BOOTUP_SHORT);
    break;
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      stopEffect(S_WAND_BOOTUP);
      playEffect(S_WAND_BOOTUP);
      soundIdleLoop(true);
    break;
  }

  if(switch_vent.on() == false && (getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE)) {
    afterLifeRamp1();
  }

  bargraphRampUp();
}

void quickVentFinished() {
  if(b_gpstar_benchtest == true) {
    ms_overheating.stop();
    stopEffect(S_QUICK_VENT_OPEN);
    playEffect(S_QUICK_VENT_CLOSE);

    if(FIRING_MODE == SLIME && WAND_STATUS == MODE_ON && switch_vent.on() == true) {
      playEffect(S_PACK_SLIME_TANK_LOOP, true);
    }
  }

  WAND_ACTION_STATUS = ACTION_IDLE;
}

void startQuickVent() {
  WAND_ACTION_STATUS = ACTION_VENTING;

  // Since the Proton Pack tells the Neutrona Wand when venting is finished, standalone wand needs its own timer.
  if(b_gpstar_benchtest == true) {
    ms_overheating.start(i_ms_overheating >= 4000 ? i_ms_overheating / 2 : 2000);

    stopEffect(S_SLIME_EMPTY);
    stopEffect(S_PACK_SLIME_TANK_LOOP);
    stopEffect(S_QUICK_VENT_CLOSE);
    playEffect(S_QUICK_VENT_OPEN);

    if(FIRING_MODE == SLIME) {
      playEffect(S_SLIME_EMPTY);
    }
  }

  wandSerialSend(W_VENTING);
}

void startVentSequence() {
  if(WAND_ACTION_STATUS == ACTION_FIRING && b_firing == true) {
    modeFireStop();
  }

  // Turn on hat light 2 in 1984/1989 as overheat indicator; turn off in Afterlife/Frozen Empire.
  if(getNeutronaWandYearMode() == SYSTEM_1984 || getNeutronaWandYearMode() == SYSTEM_1989) {
    digitalWriteFast(led_hat_2, HIGH);
  }
  else {
    digitalWriteFast(led_hat_1, LOW);
    digitalWriteFast(led_hat_2, LOW);
  }

  WAND_ACTION_STATUS = ACTION_OVERHEATING;

  // Since the Proton Pack tells the Neutrona Wand when overheating is finished, standalone wand needs its own timer.
  if(b_gpstar_benchtest == true) {
    ms_overheating.start(i_ms_overheating);
  }

  soundBeepLoopStop();
  soundIdleStop();
  soundIdleLoopStop(true);

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
    i_bargraph_status = i_bargraph_segments_5_led; // For Hasbro 5 LED bargraph.

    bargraphFull();

    ms_bargraph.start(d_bargraph_ramp_interval);
  }

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
            ht_bargraph.setLed(bargraphLookupTable(i));
            b_bargraph_status[i] = true;
          }
          else {
            ht_bargraph.clearLed(bargraphLookupTable(i));
            b_bargraph_status[i] = false;
          }
        }

        for(uint8_t i = 24; i < i_bargraph_segments - 1; i++) {
          ht_bargraph.setLed(bargraphLookupTable(i));
          b_bargraph_status[i] = true;
        }

        ht_bargraph.clearLed(bargraphLookupTable(27));
        b_bargraph_status[27] = false;

        ht_bargraph.sendLed(); // Commit the changes.
      }
      else if(b_solid_one == true) {
        for(uint8_t i = 0; i < i_bargraph_segments; i++) {
          if(i < 3) {
            ht_bargraph.setLed(bargraphLookupTable(i));
            b_bargraph_status[i] = true;
          }
          else {
            ht_bargraph.clearLed(bargraphLookupTable(i));
            b_bargraph_status[i] = false;
          }
        }

        ht_bargraph.sendLed(); // Commit the changes.
      }
      else {
        bargraphClearAll();
      }
    }
    else {
      if(b_solid_one == true) {
        digitalWriteFast(bargraphLookupTable(1-1), LOW);
      }
      else {
        digitalWriteFast(bargraphLookupTable(1-1), HIGH);
      }

      digitalWriteFast(bargraphLookupTable(2-1), HIGH);
      digitalWriteFast(bargraphLookupTable(3-1), HIGH);
      digitalWriteFast(bargraphLookupTable(4-1), HIGH);

      if(b_solid_five == true) {
        digitalWriteFast(bargraphLookupTable(5-1), LOW);
      }
      else {
        digitalWriteFast(bargraphLookupTable(5-1), HIGH);
      }
    }
  }
  else {
    switch(i_wand_menu) {
      case 5:
        if(b_28segment_bargraph == true) {
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
                ht_bargraph.setLed(bargraphLookupTable(i));
                b_bargraph_status[i] = true;
              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
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
                ht_bargraph.setLed(bargraphLookupTable(i));
                b_bargraph_status[i] = true;
              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
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
                ht_bargraph.setLed(bargraphLookupTable(i));
                b_bargraph_status[i] = true;
              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
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
                ht_bargraph.setLed(bargraphLookupTable(i));
                b_bargraph_status[i] = true;
              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
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
                ht_bargraph.setLed(bargraphLookupTable(i));
                b_bargraph_status[i] = true;
              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
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
            digitalWriteFast(led_hat_1, LOW);
            digitalWriteFast(led_hat_2, LOW);

            if(WAND_ACTION_STATUS == ACTION_IDLE) {
              // We are going to handle the toggle switch sequence for the MODE_ORIGINAL here.
              if(switch_activate.on() == true && switch_vent.on() == true && switch_wand.on() == true) {
                // Reset the power level back to what it should be.
                i_power_level = i_power_level_prev;

                // Force to power level 2 if it is on power level 1 for MODE_ORIGINAL.
                if(i_power_level < 2) {
                  i_power_level = 2;
                  i_power_level_prev = 2;
                }

                updatePackPowerLevel();

                // Turn on the cyclotron of the Proton Pack.
                modeActivate();
              }
              else {
                // Set the power level to 1 (0 circle). Record the power level so we can restore it when we power everything back up.
                if(i_power_level != 1) {
                  i_power_level_prev = i_power_level;
                  i_power_level = 1;

                  if(b_28segment_bargraph == true) {
                    bargraphPowerCheck2021Alt(false);
                  }

                  updatePackPowerLevel();
                }

                if(switch_vent.on() == true) {
                  // When the bottom right toggle is on, we start flashing the slo-blo light.
                  if(ms_slo_blo_blink.remaining() < i_slo_blo_blink_delay / 2) {
                    digitalWriteFast(led_slo_blo, LOW);
                  }
                  else {
                    digitalWriteFast(led_slo_blo, HIGH);
                  }
                }
                else {
                  // When the bottom right toggle is off, the slo-blo stays on.
                  digitalWriteFast(led_slo_blo, HIGH);
                }

                if(switch_wand.switched() || switch_vent.switched()) {
                  if(switch_vent.switched()) {
                    if(switch_vent.on() == true && switch_wand.on() == false && switch_intensify.on() == false) {
                      if(b_mode_original_toggle_sounds_enabled == true) {
                        if(b_extra_pack_sounds == true) {
                          wandSerialSend(W_BEEPS_ALT);
                        }

                        stopEffect(S_BEEPS_ALT);
                        playEffect(S_BEEPS_ALT);
                      }
                    }
                  }

                  if(switch_vent.on() == true && switch_wand.on() == true) {
                    if(b_mode_original_toggle_sounds_enabled == true) {
                      if(b_extra_pack_sounds == true) {
                        wandSerialSend(W_MODE_ORIGINAL_HEATDOWN_STOP);
                        wandSerialSend(W_MODE_ORIGINAL_HEATUP);
                      }

                      stopEffect(S_WAND_HEATDOWN);
                      stopEffect(S_WAND_HEATUP_ALT);
                      playEffect(S_WAND_HEATUP_ALT);
                    }

                    if(b_28segment_bargraph == true) {
                      bargraphPowerCheck2021Alt(false);
                    }

                    prepBargraphRampUp();
                  }
                  else if(switch_wand.switched() == true && switch_vent.on() == true && switch_wand.on() == false && b_mode_original_toggle_sounds_enabled == true) {
                    if(b_extra_pack_sounds == true) {
                      wandSerialSend(W_MODE_ORIGINAL_HEATUP_STOP);
                      wandSerialSend(W_MODE_ORIGINAL_HEATDOWN);
                    }

                    stopEffect(S_WAND_HEATUP_ALT);
                    stopEffect(S_WAND_HEATDOWN);
                    playEffect(S_WAND_HEATDOWN);
                  }
                  else if(switch_vent.switched() == true && switch_wand.on() == true && b_mode_original_toggle_sounds_enabled == true) {
                    if(b_extra_pack_sounds == true) {
                      wandSerialSend(W_MODE_ORIGINAL_HEATUP_STOP);
                      wandSerialSend(W_MODE_ORIGINAL_HEATDOWN);
                    }

                    stopEffect(S_WAND_HEATUP_ALT);
                    stopEffect(S_WAND_HEATDOWN);
                    playEffect(S_WAND_HEATDOWN);
                  }
                }

                if(switch_vent.on() == true && switch_wand.on() == true) {
                  digitalWriteFast(led_front_left, HIGH); // Turn on the front left LED under the Clippard valve.

                  // Turn on the vent lights.
                  if(b_vent_light_control == true) {
                    analogWrite(led_vent, 220); // Low power, level 1 intensity.
                  }
                  else {
                    digitalWrite(led_vent, LOW);
                  }

                  digitalWriteFast(led_white, LOW);

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

                  digitalWriteFast(led_front_left, LOW); // Turn off the front left LED under the Clippard valve.

                  // Turn off the Neutrona Wand vent lights.
                  digitalWrite(led_vent, HIGH);
                  digitalWriteFast(led_white, HIGH);

                  vibrationOff(); // Turn off vibration, if any.
                }
              }
            }
          }
          else {
            if(WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_LED_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_SETTINGS) {
              vibrationOff(); // Turn off vibration, if any.
              wandLightsOff();
            }
          }
        break;

        case MODE_SUPER_HERO:
        default:
          if(switch_activate.pushed() && WAND_ACTION_STATUS == ACTION_IDLE) {
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
          if(switch_activate.on() == false) {
            b_wand_boot_error_on = false;
            wandOff();
          }
        break;
      }
    break;

    case MODE_ON:
      switch(SYSTEM_MODE) {
        case MODE_ORIGINAL:
          // We shut the pack and wand down if any of the right toggle switches are turned off. Activate switch control is handled in fireControlCheck();
          if(switch_vent.on() == false || switch_wand.on() == false) {
            bargraphYearModeUpdate();
            // If any of the right toggle switches are turned off, we must turn the cyclotron off and shut the Neutrona Wand down to a off idle status.
            WAND_ACTION_STATUS = ACTION_OFF;
            return;
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
    if(switch_vent.on() == true) {
      if(b_vent_light_control == true) {
        // Vent light on, brightness dependent on mode.
        if(WAND_ACTION_STATUS == ACTION_FIRING) {
          analogWrite(led_vent, 0); // 0 = Full Power
        }
        else {
          // Adjust brightness based on the power level.
          switch(i_power_level) {
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
            default:
              analogWrite(led_vent, 220);
            break;
          }
        }
      }
      else {
        digitalWrite(led_vent, LOW);
      }

      soundIdleStart();

      if(switch_wand.on() == true) {
        if(b_beeping != true) {
          // Beep loop.
          soundBeepLoop();
        }
      }
      else {
        soundBeepLoopStop();
      }
    }
    else if(switch_vent.on() == false) {
      // Vent light and top white light off.
      digitalWrite(led_vent, HIGH);

      soundBeepLoopStop();
      soundIdleStop();
    }
  }
}

void wandOff() {
  if(WAND_ACTION_STATUS == ACTION_SETTINGS) {
    // If the wand is shut down while we are in settings mode (can happen if the pack is manually turned off), switch the wand and pack to proton mode.
    switch(FIRING_MODE) {
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

      case PROTON:
      default:
        // Tell the pack we are in proton mode.
        wandSerialSend(W_PROTON_MODE);
      break;
    }
  }

  switch(getNeutronaWandYearMode()) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      stopEffect(S_AFTERLIFE_WAND_IDLE_1);
      stopEffect(S_AFTERLIFE_WAND_RAMP_1);
      stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1);
      stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2);
      stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2_FADE_OUT);
      stopEffect(S_AFTERLIFE_WAND_RAMP_2_FADE_IN);
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      if(b_sound_idle != true) {
        stopEffect(S_WAND_BOOTUP_SHORT);
      }
    break;
  }

  if(WAND_ACTION_STATUS == ACTION_OVERHEATING || b_wand_boot_error_on == true) {
    stopEffect(S_BEEPS_LOW);
    stopEffect(S_BEEPS);
    stopEffect(S_BEEPS_BARGRAPH);
  }

  stopEffect(S_WAND_BOOTUP);
  stopEffect(S_SMASH_ERROR_RESTART);

  b_sound_afterlife_idle_2_fade = true;

  if(WAND_ACTION_STATUS == ACTION_ERROR && b_wand_boot_error_on != true && b_wand_mash_error != true) {
    // We are exiting Wand Boot Error, so change wand state back to off/idle without informing Proton Pack.
    WAND_STATUS = MODE_OFF;
    WAND_ACTION_STATUS = ACTION_IDLE;
  }
  else if(WAND_ACTION_STATUS != ACTION_ERROR && (b_wand_boot_error_on == true || b_wand_mash_error == true)) {
    // We are entering either Wand Boot Error mode or Button Mash Timeout mode, so do nothing.
  }
  else {
    // Full wand shutdown in all other situations.
    wandSerialSend(W_OFF);
    WAND_STATUS = MODE_OFF;
    WAND_ACTION_STATUS = ACTION_IDLE;

    if(b_wand_mash_error == true) {
      stopEffect(S_SMASH_ERROR_LOOP);
      stopEffect(S_SMASH_ERROR_RESTART);
    }

    // Turn off any barrel spark effects and reset the button mash lockout.
    if(b_wand_mash_error == true || b_pack_alarm == true) {
      barrelLightsOff();
      b_wand_mash_error = false;
    }

    if(b_pack_alarm != true) {
      switch(getNeutronaWandYearMode()) {
        case SYSTEM_1984:
        case SYSTEM_1989:
          // Proton Pack plays shutdown sound, but standalone Wand needs to play its own
          if(b_gpstar_benchtest == true && SYSTEM_MODE == MODE_SUPER_HERO && switch_vent.on() == false) {
            stopEffect(S_WAND_HEATDOWN);
            playEffect(S_WAND_HEATDOWN);
          }

          if(SYSTEM_MODE == MODE_SUPER_HERO) {
            if(switch_vent.on() == true) {
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
          if(WAND_ACTION_STATUS != ACTION_ERROR && b_pack_alarm != true) {
            playEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1, false, i_volume_effects - 1);

            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_AFTERLIFE_GUN_RAMP_DOWN_1);
            }
          }

          if(b_extra_pack_sounds == true) {
            wandSerialSend(W_WAND_SHUTDOWN_SOUND);
          }

          stopEffect(S_WAND_SHUTDOWN);
          playEffect(S_WAND_SHUTDOWN);
        break;
      }
    }
  }

  soundBeepLoopStop();
  soundIdleStop();
  soundIdleLoopStop(true);

  vibrationOff();

  // Stop firing if the wand is turned off.
  if(b_firing == true) {
    modeFireStop();
  }

  if(b_extra_pack_sounds == true) {
    wandSerialSend(W_EXTRA_WAND_SOUNDS_STOP);
    wandSerialSend(W_WAND_BEEP_STOP);
  }

  // Turn off any lingering mode switch sounds.
  stopEffect(S_MODE_SWITCH);

  switch(FIRING_MODE) {
    case PROTON:
    default:
      stopEffect(S_FIRE_START_SPARK);
    break;

    case SLIME:
      stopEffect(S_PACK_SLIME_OPEN);
    break;

    case STASIS:
      stopEffect(S_STASIS_OPEN);
    break;

    case MESON:
      stopEffect(S_MESON_OPEN);
    break;
  }

  if(WAND_ACTION_STATUS != ACTION_ERROR && b_wand_mash_error == true) {
    if(b_extra_pack_sounds == true) {
      wandSerialSend(W_WAND_MASH_ERROR_SOUND);
    }

    playEffect(S_WAND_MASH_ERROR);
  }

  // Clear counter until user begins firing again.
  i_bmash_count = 0;

  // Turn off some timers.
  ms_overheating.stop();
  ms_settings_blinking.stop();
  ms_semi_automatic_check.stop();
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

      // Start the timer for the power on indicator option.
      if(b_power_on_indicator == true) {
        ms_power_indicator.start(i_ms_power_indicator);
      }

      wandSwitchedCount = 0;
      ventSwitchedCount = 0;
    break;
    default:
      // Do nothing if we aren't MODE_OFF
    break;
  }
}

// Called from checkSwitches(); Check if we should fire, or if the wand and pack turn off.
void fireControlCheck() {
  // Firing action stuff and shutting cyclotron and the Neutrona Wand off.
  if(WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING && WAND_ACTION_STATUS != ACTION_VENTING && b_pack_alarm != true) {
    // If Activate switch is down, turn wand off.
    if(switch_activate.on() == false) {
      WAND_ACTION_STATUS = ACTION_OFF;
      return;
    }

    if(i_bmash_count >= i_bmash_max) {
      // User has exceeded "normal" firing rate.
      switch(FIRING_MODE) {
        case PROTON:
        default:
          stopEffect(S_FIRING_END_GUN);
        break;
        case SLIME:
          stopEffect(S_SLIME_END);
        break;
        case STASIS:
          stopEffect(S_STASIS_END);
        break;
      }

      wandSerialSend(W_BUTTON_MASHING);
      b_wand_mash_error = true;
      modeError();
      wandTipSpark();

      // Adjust the cool down lockout period based on the power level.
      switch(i_power_level) {
        case 5:
          ms_bmash.start(i_bmash_cool_down + 2000);
        break;

        case 4:
          ms_bmash.start(i_bmash_cool_down + 1500);
        break;

        case 3:
          ms_bmash.start(i_bmash_cool_down + 1000);
        break;

        case 2:
          ms_bmash.start(i_bmash_cool_down + 500);
        break;

        case 1:
        default:
          ms_bmash.start(i_bmash_cool_down);
        break;
      }
    }
    else {
      if(i_slime_tether_count > 0 && ms_semi_automatic_check.remaining() < 1) {
        // Reset the Slime Tether count.
        i_slime_tether_count = 0;
      }

      if(switch_intensify.on() == true && switch_wand.on() == true && switch_vent.on() == true && b_switch_barrel_extended == true) {
        switch(FIRING_MODE) {
          case PROTON:
          case SLIME:
          case SPECTRAL:
          case SPECTRAL_CUSTOM:
          case HOLIDAY:
          default:
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
          break;

          case STASIS:
            // Handle Shock Blast fire start here.
            if(b_firing_semi_automatic != true && ms_semi_automatic_check.remaining() < 1 && WAND_ACTION_STATUS != ACTION_FIRING) {
              // Start rate-of-fire timer.
              ms_semi_automatic_check.start(i_shock_blast_rate);

              modePulseStart();

              b_firing_semi_automatic = true;
            }
          break;

          case MESON:
            // Handle Meson Collider fire start here.
            if(b_firing_semi_automatic != true && ms_semi_automatic_check.remaining() < 1 && WAND_ACTION_STATUS != ACTION_FIRING) {
              // Start rate-of-fire timer.
              ms_semi_automatic_check.start(i_meson_collider_rate);

              modePulseStart();

              b_firing_semi_automatic = true;
            }
          break;
        }
      }

      // When Cross The Streams mode is enabled, video game modes are disabled and the wand menu settings can only be accessed when the Neutrona Wand is powered down.
      if(b_cross_the_streams == true) {
        if(switch_mode.on() == true && switch_wand.on() == true && switch_vent.on() == true && b_switch_barrel_extended == true) {
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
        else if(switch_mode.on() == false) {
          if(b_firing_intensify != true && WAND_ACTION_STATUS == ACTION_FIRING) {
            WAND_ACTION_STATUS = ACTION_IDLE;
          }

          b_firing_alt = false;
        }
      }
      else {
        if(FIRING_MODE == PROTON && WAND_ACTION_STATUS == ACTION_FIRING) {
          if(switch_mode.on() == true) {
            b_firing_alt = true;
          }
        }
        else if(switch_mode.on() == true && switch_wand.on() == true && switch_vent.on() == true && b_switch_barrel_extended == true) {
          switch(FIRING_MODE) {
            case PROTON:
              // Handle Boson Dart fire start here.
              if(b_firing_semi_automatic != true && ms_semi_automatic_check.remaining() < 1) {
                // Start rate-of-fire timer.
                ms_semi_automatic_check.start(i_boson_dart_rate);

                modePulseStart();

                b_firing_semi_automatic = true;
              }
            break;

            case SLIME:
              // Handle Slime Tether fire start here.
              if(b_firing_semi_automatic != true && WAND_ACTION_STATUS != ACTION_FIRING) {
                if(i_slime_tether_count < 1) {
                  // Start the rate-of-fire timer.
                  ms_semi_automatic_check.start(i_slime_tether_rate);

                  modePulseStart();

                  // Increment the Slime Tether counter.
                  i_slime_tether_count++;
                }
                else if (i_slime_tether_count < 2){
                  modePulseStart();

                  // Increment the Slime Tether counter.
                  i_slime_tether_count++;
                }

                b_firing_semi_automatic = true;
              }
            break;

            case STASIS:
            case MESON:
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
            break;

            default:
              // Do nothing.
            break;
          }
        }
      }

      if(switch_intensify.on() != true) {
        switch(FIRING_MODE) {
          case PROTON:
          case SLIME:
          case SPECTRAL:
          case SPECTRAL_CUSTOM:
          case HOLIDAY:
          default:
            if(b_firing == true && b_firing_intensify == true) {
              if(b_firing_alt != true || vgModeCheck() == true) {
                WAND_ACTION_STATUS = ACTION_IDLE;
              }

              b_firing_intensify = false;
            }
          break;

          case STASIS:
          case MESON:
            // Handle resetting semi-auto bool here.
            b_firing_semi_automatic = false;
          break;
        }
      }

      if(switch_mode.on() != true && b_cross_the_streams != true) {
        switch(FIRING_MODE) {
          case PROTON:
          case SLIME:
            // Handle resetting semi-auto bool here.
            b_firing_semi_automatic = false;
          break;

          case STASIS:
          case MESON:
            if(b_firing == true && b_firing_intensify == true) {
              WAND_ACTION_STATUS = ACTION_IDLE;
              b_firing_intensify = false;
            }
          break;

          default:
            // Do nothing.
          break;
        }
      }
    }

    // Quick vent feature. When enabled, clicking Intensify will perform a quick vent, while holding will force the full overheat sequence.
    // Super Hero Mode only, because Mode Original uses different toggle switch combinations which makes this not possible.
    if(b_quick_vent == true && SYSTEM_MODE == MODE_SUPER_HERO && switch_wand.on() == false && switch_vent.on() == true && b_overheat_enabled == true) {
      if(switch_intensify.singleClick()) {
        startQuickVent();
      }
      else if(switch_intensify.longPress()) {
        startVentSequence();
      }
    }
  }
  else if(WAND_ACTION_STATUS == ACTION_OVERHEATING || WAND_ACTION_STATUS == ACTION_VENTING || WAND_ACTION_STATUS == ACTION_SETTINGS || b_pack_alarm == true) {
    // If Activate switch is down, turn wand off.
    if(switch_activate.on() == false) {
      WAND_ACTION_STATUS = ACTION_OFF;
      return;
    }

    if(WAND_ACTION_STATUS == ACTION_IDLE) {
      // Play a little spark effect if the user tries to fire while the ribbon cable is removed.
      if((switch_intensify.pushed() || (b_cross_the_streams == true && switch_mode.pushed())) && !ms_wand_heatup_fade.isRunning() && switch_vent.on() == true && switch_wand.on() == true) {
        if(b_extra_pack_sounds == true) {
          wandSerialSend(W_WAND_MASH_ERROR_SOUND);
        }

        stopEffect(S_WAND_MASH_ERROR);
        playEffect(S_WAND_MASH_ERROR);
        wandTipSpark();
      }
    }
  }
}

// Called from checkSwitches(); Used to enter the settings menu in MODE_SUPER_HERO.
void altWingButtonCheck() {
  if(WAND_ACTION_STATUS != ACTION_FIRING && WAND_ACTION_STATUS != ACTION_OFF && WAND_ACTION_STATUS != ACTION_OVERHEATING && WAND_ACTION_STATUS != ACTION_VENTING && b_pack_alarm != true) {
    if((switch_wand.on() != true || switch_vent.on() != true) && switch_mode.pushed()) {
      // Only exit the settings menu when on menu #5.
      if(i_wand_menu == 5) {
        // Switch between firing mode and settings mode.
        if(WAND_ACTION_STATUS != ACTION_SETTINGS) {
          WAND_ACTION_STATUS = ACTION_SETTINGS;

          ms_settings_blinking.start(i_settings_blinking_delay);

          // Clear the 28 segment bargraph.
          bargraphClearAlt();

          // Tell the pack we are in settings mode.
          wandSerialSend(W_SETTINGS_MODE);
        }
        else {
          modeCheck();
          ms_settings_blinking.stop();
          bargraphClearAlt();

          // If using the 28 segment bargraph with BARGRAPH_ORIGINAL, we need to redraw the segments.
          // BARGRAPH_SUPER_HERO auto ramps and does not need a manual refresh.
          if(b_28segment_bargraph == true && BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
            bargraphPowerCheck2021Alt(true);
          }
        }

        playEffect(S_CLICK);
      }
    }
    else if(WAND_ACTION_STATUS == ACTION_SETTINGS && switch_vent.on() == true && switch_wand.on() == true) {
      // Exit the settings menu if the user turns the wand switch back on.
      modeCheck();
      ms_settings_blinking.stop();
      bargraphClearAlt();

      // If using the 28 segment bargraph with BARGRAPH_ORIGINAL, we need to redraw the segments.
      // BARGRAPH_SUPER_HERO auto ramps and does not need a manual refresh.
      if(b_28segment_bargraph == true && BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
        bargraphPowerCheck2021Alt(true);
      }
    }
  }
}

void modeCheck() {
  if(WAND_CONN_STATE == PACK_CONNECTED) {
    switch(FIRING_MODE) {
      case HOLIDAY:
        // Tell the pack we are in holiday mode.
        wandSerialSend(W_HOLIDAY_MODE);
      break;

      case SPECTRAL:
        // Tell the pack we are in spectral mode.
        wandSerialSend(W_SPECTRAL_MODE);
      break;

      case SPECTRAL_CUSTOM:
        // Tell the pack we are in spectral custom mode.
        wandSerialSend(W_SPECTRAL_CUSTOM_MODE);
      break;

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

      case PROTON:
      default:
        // Tell the pack we are in proton mode.
        wandSerialSend(W_PROTON_MODE);
      break;
    }
  }

  if(WAND_STATUS == MODE_ON) {
    wandHeatUp();
  }

  // Do not exit any menu systems if we are in them.
  if(WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_LED_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU) {
    WAND_ACTION_STATUS = ACTION_IDLE;
  }
  
  if(AUDIO_DEVICE == A_GPSTAR_AUDIO) {
    if(FIRING_MODE == MESON) {
      // Tell GPStar Audio we need short audio mode.
      audio.gpstarShortTrackOverload(false);
    }
    else {
      // Tell GPStar Audio we no longer need short audio.
      audio.gpstarShortTrackOverload(true);
    }
  }

  // Reset the semi-automatic firing timer.
  ms_semi_automatic_check.stop();
}

void modeError() {
  wandOff();

  WAND_STATUS = MODE_ERROR;
  WAND_ACTION_STATUS = ACTION_ERROR;

  if(b_wand_mash_error != true) {
    // This is used for controlling the bargraph beeping while in boot error mode.
    ms_hat_1.start(i_hat_2_delay * 4);
    ms_hat_2.start(i_hat_2_delay);
    ms_settings_blinking.start(i_settings_blinking_delay);

    if(b_extra_pack_sounds == true) {
      wandSerialSend(W_WAND_BEEP_BARGRAPH);
      wandSerialSend(W_WAND_BEEP_SOUNDS);
    }

    playEffect(S_BEEPS_LOW);
    playEffect(S_BEEPS);
    playEffect(S_BEEPS_BARGRAPH);
  }
  else if(b_wand_mash_error == true) {
    playEffect(S_SMASH_ERROR_LOOP, true, i_volume_effects, true, 2500);

    if(b_extra_pack_sounds == true) {
      wandSerialSend(W_SMASH_ERROR_LOOP);
    }
  }
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
      if(switch_activate.switched() == false && b_mode_original_toggle_sounds_enabled == true) {
        if(b_extra_pack_sounds == true) {
          wandSerialSend(W_MODE_ORIGINAL_HEATUP);
        }

        stopEffect(S_WAND_HEATUP_ALT);
        playEffect(S_WAND_HEATUP_ALT);
      }

      wandSerialSend(W_ON);

      postActivation();
    break;

    case MODE_SUPER_HERO:
    default:
      // The wand was started while the top switch was already on, so let's put the wand into startup error mode.
      if(switch_wand.on() == true && b_wand_boot_errors == true) {
        b_wand_boot_error_on = true;
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

  // Stop the heatdown sound if playing.
  stopEffect(S_WAND_HEATDOWN);

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
          if(switch_vent.on() == true) {
            b_all_switch_activation = true; // If vent switch is already on when Activate is flipped, set to true for soundIdleLoop() to use
          }
        break;
      }
    }

    // Turn on slo-blo light.
    digitalWriteFast(led_slo_blo, HIGH);

    // Turn on the Clippard LED.
    digitalWriteFast(led_front_left, HIGH);

    // Top white light.
    ms_white_light.start(d_white_light_interval);
    digitalWriteFast(led_white, LOW);

    // Reset the hat light timers.
    ms_hat_1.stop();
    ms_hat_2.stop();

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

          if(switch_vent.on() == false && b_pack_alarm != true) {
            afterLifeRamp1();
          }
        break;
      }
    }
  }
}

void soundIdleLoop(bool fadeIn) {
  switch(i_power_level) {
    case 1:
    default:
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

  if(b_gpstar_benchtest == true && fadeIn == true) {
    switch(FIRING_MODE) {
      case SLIME:
        playEffect(S_PACK_SLIME_TANK_LOOP, true, 0, true, 900);
      break;
      case STASIS:
        playEffect(S_STASIS_IDLE_LOOP, true, 0, true, 900);
      break;
      case MESON:
        playEffect(S_MESON_IDLE_LOOP, true, 0, true, 900);
      break;
      default:
        // Do nothing.
      break;
    }
  }
}

void soundIdleLoopStop(bool stopAlts) {
  switch(i_power_level) {
    case 1:
    default:
      stopEffect(S_IDLE_LOOP_GUN_1);
    break;

    case 2:
      stopEffect(S_IDLE_LOOP_GUN_1);
      stopEffect(S_IDLE_LOOP_GUN_2);
    break;

    case 3:
      stopEffect(S_IDLE_LOOP_GUN_1);
      stopEffect(S_IDLE_LOOP_GUN_2);
    break;

    case 4:
      stopEffect(S_IDLE_LOOP_GUN_2);
      stopEffect(S_IDLE_LOOP_GUN_5);
    break;

    case 5:
      stopEffect(S_IDLE_LOOP_GUN_2);
      stopEffect(S_IDLE_LOOP_GUN_5);
    break;
  }

  if(stopAlts == true && b_gpstar_benchtest == true) {
    switch(FIRING_MODE) {
      case SLIME:
        stopEffect(S_PACK_SLIME_TANK_LOOP);
      break;
      case STASIS:
        stopEffect(S_STASIS_IDLE_LOOP);
      break;
      case MESON:
        stopEffect(S_MESON_IDLE_LOOP);
      break;
      default:
        // Do nothing.
      break;
    }
  }
}

void soundIdleStart() {
  if(b_sound_idle == false) {
    switch(getNeutronaWandYearMode()) {
      case SYSTEM_1984:
      case SYSTEM_1989:
        if(b_extra_pack_sounds == true && switch_vent.on() && switch_vent.switched()) {
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

        ms_gun_loop_2.start(i_gun_loop_2);

        b_sound_idle = true;

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
        stopEffect(S_WAND_BOOTUP);
        stopEffect(S_WAND_BOOTUP_SHORT);
        stopEffect(S_GB2_WAND_START);
        soundIdleLoopStop(true);

        if(WAND_ACTION_STATUS != ACTION_OFF && WAND_ACTION_STATUS != ACTION_OVERHEATING && b_wand_mash_error != true) {
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
        stopEffect(S_AFTERLIFE_WAND_RAMP_2);
        stopEffect(S_AFTERLIFE_WAND_IDLE_2);

        if(b_extra_pack_sounds == true) {
          wandSerialSend(W_AFTERLIFE_RAMP_LOOP_2_STOP);
        }

        if(b_pack_alarm != true) {
          if(WAND_ACTION_STATUS == ACTION_OVERHEATING) {
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
            ms_gun_loop_1.start(i_gun_loop_1);
            ms_gun_loop_2.stop();
          }
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

      switch(i_power_level) {
        case 1:
        default:
          if(b_next_gen && b_beep_loop == true) {
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_WAND_BEEP_START);
            }

            playEffect(S_AFTERLIFE_BEEP_WAND_S1, true);
          }
          else {
            if(switch_wand.switched() == true) {
              if(b_extra_pack_sounds == true) {
                wandSerialSend(W_WAND_BEEP);
              }

              playEffect(S_AFTERLIFE_BEEP_WAND_S5);
            }
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
            if(switch_wand.switched() == true) {
              if(b_extra_pack_sounds == true) {
                wandSerialSend(W_WAND_BEEP);
              }

              playEffect(S_AFTERLIFE_BEEP_WAND_S5);
            }
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
            if(switch_wand.switched() == true) {
              if(b_extra_pack_sounds == true) {
                wandSerialSend(W_WAND_BEEP);
              }

              playEffect(S_AFTERLIFE_BEEP_WAND_S5);
            }
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
            if(switch_wand.switched() == true) {
              if(b_extra_pack_sounds == true) {
                wandSerialSend(W_WAND_BEEP);
              }

              playEffect(S_AFTERLIFE_BEEP_WAND_S5);
            }
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
            if(switch_wand.switched() == true) {
              if(b_extra_pack_sounds == true) {
                wandSerialSend(W_WAND_BEEP);
              }

              playEffect(S_AFTERLIFE_BEEP_WAND_S5);
            }
          }
        break;
      }

      b_beeping = true;

      ms_reset_sound_beep.stop();
    }
  }
}

void modePulseStart() {
  // Handles all "pulsed" fire modes.
  i_fast_led_delay = FAST_LED_UPDATE_MS;
  barrelLightsOff();

  switch(FIRING_MODE) {
    case PROTON:
      // Boson Dart.
      wandSerialSend(W_BOSON_DART_SOUND);
      playEffect(S_BOSON_DART_FIRE, false, i_volume_effects, false, 0, false);
      ms_firing_pulse.start(0);
    break;

    case SLIME:
      // Slime Tether.
      wandSerialSend(W_SLIME_TETHER_SOUND);
      playEffect(S_SLIME_TETHER_FIRE, false, i_volume_effects, false, 0, false);
      ms_firing_pulse.start(0);
    break;

    case STASIS:
      // Shock Blast.
      wandSerialSend(W_SHOCK_BLAST_SOUND);
      playEffect(S_SHOCK_BLAST_FIRE, false, i_volume_effects, false, 0, false);
      ms_firing_pulse.start(0);
    break;

    case MESON:
      // Meson Collider.
      wandSerialSend(W_MESON_COLLIDER_SOUND);
      playEffect(S_MESON_COLLIDER_FIRE, false, i_volume_effects, false, 0, false);
      ms_firing_pulse.start(0);
    break;

    default:
      // Do nothing.
    break;
  }
}

void modeFireStartSounds() {
  switch(FIRING_MODE) {
    case PROTON:
    default:
      // Some sparks for firing start.
      if(getSystemYearMode() == SYSTEM_1989) {
        playEffect(S_FIRE_START_SPARK, false, i_volume_effects - 10, false, 0, false);
      }
      else if(getSystemYearMode() == SYSTEM_FROZEN_EMPIRE) {
        playEffect(S_FROZEN_EMPIRE_FIRE_START, false, i_volume_effects, false, 0, false);
      }
      else {
        playEffect(S_FIRE_START_SPARK, false, i_volume_effects, false, 0, false);
      }

      switch(i_power_level) {
        case 1 ... 4:
        default:
          if(b_firing_intensify == true) {
            if(getSystemYearMode() == SYSTEM_1989) {
              playEffect(S_GB2_FIRE_START, false, i_volume_effects, false, 0, false);
              playEffect(S_GB2_FIRE_LOOP, true, i_volume_effects, true, 6500, false);
            }
            else if(getSystemYearMode() == SYSTEM_FROZEN_EMPIRE) {
              playEffect(S_GB1_FIRE_LOOP, true, i_volume_effects, true, 500, false);
            }
            else {
              playEffect(S_GB1_FIRE_START, false, i_volume_effects, false, 0, false);
              playEffect(S_GB1_FIRE_LOOP, true, i_volume_effects, true, 1000, false);
            }

            b_sound_firing_intensify_trigger = true;
          }
          else {
            b_sound_firing_intensify_trigger = false;
          }

          if(b_firing_alt == true) {
            if(getSystemYearMode() == SYSTEM_1989) {
              playEffect(S_GB2_FIRE_START, false, i_volume_effects, false, 0, false);
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 1000, false);
            }
            else if(getSystemYearMode() == SYSTEM_FROZEN_EMPIRE) {
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 700, false);
            }
            else {
              playEffect(S_FIRE_START, false, i_volume_effects, false, 0, false);
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 1000, false);
            }

            b_sound_firing_alt_trigger = true;
          }
          else {
            b_sound_firing_alt_trigger = false;
          }
        break;

        case 5:
          switch(getSystemYearMode()) {
            case SYSTEM_1989:
              playEffect(S_GB2_FIRE_START, false, i_volume_effects, false, 0, false);
            break;

            case SYSTEM_1984:
              playEffect(S_GB1_FIRE_START_HIGH_POWER, false, i_volume_effects, false, 0, false);
              playEffect(S_GB1_FIRE_START, false, i_volume_effects, false, 0, false);
            break;

            case SYSTEM_FROZEN_EMPIRE:
              // Do nothing.
            break;

            case SYSTEM_AFTERLIFE:
            default:
            {
              uint8_t i_amplify_tmp = 2;

              if(AUDIO_DEVICE == A_GPSTAR_AUDIO) {
                i_amplify_tmp = 0;
              }

              playEffect(S_AFTERLIFE_FIRE_START, false, i_volume_effects + i_amplify_tmp, false, 0, false);
            }
            break;
          }

          if(b_firing_intensify == true) {
            // Reset some sound triggers.
            b_sound_firing_intensify_trigger = true;
            if(getSystemYearMode() == SYSTEM_FROZEN_EMPIRE) {
              playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, true, 700, false);
            }
            else {
              playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, true, 700, false);
            }
          }
          else {
            b_sound_firing_intensify_trigger = false;
          }

          if(b_firing_alt == true) {
            // Reset some sound triggers.
            b_sound_firing_alt_trigger = true;
            if(getSystemYearMode() == SYSTEM_FROZEN_EMPIRE) {
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 120, false);
            }
            else {
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 700, false);
            }
          }
          else {
            b_sound_firing_alt_trigger = false;
          }
        break;
      }
    break;

    case SLIME:
      stopEffect(S_SLIME_END);
      playEffect(S_SLIME_START, false, i_volume_effects, false, 0, false);
      playEffect(S_SLIME_LOOP, true, i_volume_effects, false, 1500, false);
    break;

    case STASIS:
      stopEffect(S_STASIS_END);
      playEffect(S_STASIS_START, false, i_volume_effects, false, 0, false);
      playEffect(S_STASIS_LOOP, true, i_volume_effects, false, 1000, false);
    break;

    case MESON:
      playEffect(S_MESON_FIRE_PULSE);

      switch(i_power_level) {
        case 5:
          ms_meson_blast.start(i_meson_blast_delay_level_5);
        break;

        case 4:
          ms_meson_blast.start(i_meson_blast_delay_level_4);
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
  }
}

void modeFireStart() {
  i_fast_led_delay = FAST_LED_UPDATE_MS;

  // Tell the Proton Pack that the Neutrona Wand is firing in Intensify mode.
  if(b_firing_intensify == true) {
    wandSerialSend(W_FIRING_INTENSIFY);
  }

  // Tell the Proton Pack that the Neutrona Wand is firing in Alt mode.
  if(b_firing_alt == true) {
    wandSerialSend(W_FIRING_ALT);
  }

  modeFireStartSounds();

  // Tell the pack the wand is firing.
  wandSerialSend(W_FIRING);

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
  digitalWriteFast(led_hat_1, HIGH);

  // This will only overheat when enabled by using the alt firing when in crossing the streams mode.
  bool b_overheat_flag = true;

  if(b_cross_the_streams == true && b_firing_alt != true) {
    b_overheat_flag = false;
  }

  if(b_overheat_flag == true) {
    // If in high power level on the wand, start an overheat timer.
    if(b_overheat_level[i_power_level - 1] == true && b_overheat_enabled == true) {
      ms_overheat_initiate.start(i_ms_overheat_initiate[i_power_level - 1]);
    }
    else if(b_cross_the_streams == true) {
      if(b_firing_alt == true) {
        ms_overheat_initiate.start(i_ms_overheat_initiate[i_power_level - 1]);
      }
    }
  }

  barrelLightsOff();

  if(FIRING_MODE == MESON) {
    ms_firing_stream_effects.start(0);
  }
  else {
    ms_firing_lights.start(0);
  }

  // Stop any bargraph ramps.
  ms_bargraph.stop();

  ms_bargraph_alt.stop();

  // Reset the 28 segment bargraph.
  i_bargraph_status_alt = 0;

  b_bargraph_up = false;

  bargraphRampFiring();

  if(FIRING_MODE == PROTON && b_stream_effects == true) {
    ms_impact.start(random(10,16) * 1000);

    // Standalone wand plays additional SFX from Proton Pack.
    if(b_gpstar_benchtest == true) {
      ms_firing_sound_mix.start(random(7,15) * 1000);
    }
  }
}

void modeFireStopSounds() {
  // Reset some sound triggers.
  b_sound_firing_intensify_trigger = false;
  b_sound_firing_alt_trigger = false;
  b_sound_firing_cross_the_streams = false;
  b_sound_firing_cross_the_streams_mix = false;

  ms_meson_blast.stop();

  if(getSystemYearMode() == SYSTEM_FROZEN_EMPIRE) {
    stopEffect(S_FROZEN_EMPIRE_FIRE_START);
  }

  // Stop all other firing sounds.
  switch(FIRING_MODE) {
    case PROTON:
    default:
      switch(i_power_level) {
        case 1 ... 4:
        default:
          if(getSystemYearMode() == SYSTEM_1989) {
            stopEffect(S_GB2_FIRE_START);
            stopEffect(S_GB2_FIRE_LOOP);
          }
          else {
            stopEffect(S_GB1_FIRE_START);
            stopEffect(S_GB1_FIRE_LOOP);
          }
        break;
        case 5:
          switch(getSystemYearMode()) {
            case SYSTEM_1989:
              stopEffect(S_GB2_FIRE_START);
            break;
            case SYSTEM_1984:
              stopEffect(S_GB1_FIRE_START_HIGH_POWER);
              stopEffect(S_GB1_FIRE_START);
            break;
            case SYSTEM_AFTERLIFE:
            case SYSTEM_FROZEN_EMPIRE:
            default:
              stopEffect(S_AFTERLIFE_FIRE_START);
            break;
          }
        break;
      }

      stopEffect(S_FIRE_START_SPARK);
      stopEffect(S_FIRE_LOOP_IMPACT);
      stopEffect(S_FIRING_LOOP_GB1);
      stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);
    break;

    case SLIME:
      stopEffect(S_SLIME_START);
      stopEffect(S_SLIME_LOOP);
    break;

    case STASIS:
      stopEffect(S_STASIS_START);
      stopEffect(S_STASIS_LOOP);
    break;

    case MESON:
      // Nothing.
    break;
  }

  if(b_wand_mash_error != true) {
    switch(FIRING_MODE) {
      case PROTON:
      default:
        playEffect(S_FIRING_END_GUN, false, i_volume_effects, false, 0, false);
      break;

      case SLIME:
        playEffect(S_SLIME_END, false, i_volume_effects, false, 0, false);
      break;

      case STASIS:
        playEffect(S_STASIS_END, false, i_volume_effects, false, 0, false);
      break;

      case MESON:
        // Nothing.
      break;
    }
  }

  if(b_firing_cross_streams == true) {
    switch(WAND_YEAR_CTS) {
      case CTS_AFTERLIFE:
      case CTS_FROZEN_EMPIRE:
        if(AUDIO_DEVICE != A_GPSTAR_AUDIO) {
          stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
          stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);
        }

        if(b_wand_mash_error != true) {
          playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects, false, 0, false);
        }
      break;

      case CTS_1984:
      case CTS_1989:
        if(AUDIO_DEVICE != A_GPSTAR_AUDIO) {
          stopEffect(S_CROSS_STREAMS_START);
          stopEffect(S_CROSS_STREAMS_END);
        }

        if(b_wand_mash_error != true) {
          playEffect(S_CROSS_STREAMS_END, false, i_volume_effects, false, 0, false);
        }
      break;

      case CTS_DEFAULT:
      default:
        switch(getSystemYearMode()) {
          case SYSTEM_AFTERLIFE:
          case SYSTEM_FROZEN_EMPIRE:
          default:
            if(AUDIO_DEVICE != A_GPSTAR_AUDIO) {
              stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
              stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);
            }

            if(b_wand_mash_error != true) {
              playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects, false, 0, false);
            }
          break;

          case SYSTEM_1984:
          case SYSTEM_1989:
            if(AUDIO_DEVICE != A_GPSTAR_AUDIO) {
              stopEffect(S_CROSS_STREAMS_START);
              stopEffect(S_CROSS_STREAMS_END);
            }

            if(b_wand_mash_error != true) {
              playEffect(S_CROSS_STREAMS_END, false, i_volume_effects, false, 0, false);
            }
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

  ms_bargraph_alt.stop(); // Stop the 1984 28 segment optional bargraph timer.
  b_bargraph_up = false;

  switch(BARGRAPH_MODE) {
    case BARGRAPH_ORIGINAL:
      // Need to restart the regular bargraph timer.
      i_bargraph_status = i_power_level - 1;
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
      i_bargraph_status = i_power_level - 1;
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

  ms_firing_lights.stop();
  ms_impact.stop();
  ms_firing_sound_mix.stop();
  ms_firing_effect_end.start(0);

  if(getNeutronaWandYearMode() == SYSTEM_1984 || getNeutronaWandYearMode() == SYSTEM_1989) {
    digitalWriteFast(led_hat_1, LOW); // Turn off hat light 1 when we stop firing in 1984/1989.
    digitalWriteFast(led_hat_2, LOW); // Make sure we turn off hat light 2 in case it's on as well.
  }
  else {
    digitalWriteFast(led_hat_2, HIGH); // Make sure we turn on hat light 2 in case it's off as well.
  }

  wandTipOff();

  ms_hat_1.stop();

  // Stop overheat beeps.
  stopOverheatBeepWarnings();

  modeFireStopSounds();
}

void modeFiring() {
  // Sound trigger flags.
  if(b_firing_intensify == true && b_sound_firing_intensify_trigger != true) {
    b_sound_firing_intensify_trigger = true;

    if(b_cross_the_streams_mix == true && FIRING_MODE == PROTON) {
      // Tell the Proton Pack that the Neutrona Wand is firing in Intensify mode mix.
      wandSerialSend(W_FIRING_INTENSIFY_MIX);
    }
    else {
      // Tell the Proton Pack that the Neutrona Wand is firing in Intensify mode.
      //wandSerialSend(W_FIRING_INTENSIFY);
    }
  }

  if(b_firing_intensify != true && b_sound_firing_intensify_trigger == true) {
    b_sound_firing_intensify_trigger = false;

    if(b_cross_the_streams_mix == true && FIRING_MODE == PROTON) {
      // Tell the Proton Pack that the Neutrona Wand is no longer firing in Intensify mode mix.
      wandSerialSend(W_FIRING_INTENSIFY_STOPPED_MIX);

      if(i_power_level == 5) {
        // Need to stop and restart this loop to prevent overlaps since the barrel wing button is still held.
        stopEffect(S_FIRING_LOOP_GB1);
        playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, false, 0, false);
      }

      stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);
    }
    else {
      // Tell the Proton Pack that the Neutrona Wand is no longer firing in Intensify mode.
      //wandSerialSend(W_FIRING_INTENSIFY_STOPPED);
    }
  }

  if(b_firing_alt == true && b_sound_firing_alt_trigger != true) {
    b_sound_firing_alt_trigger = true;

    if(b_cross_the_streams_mix == true) {
      playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, false, 0, false);

      // Tell the Proton Pack that the Neutrona Wand is firing in Alt mode mix.
      wandSerialSend(W_FIRING_ALT_MIX);
    }
    else {
      // Tell the Proton Pack that the Neutrona Wand is firing in Alt mode.
      //wandSerialSend(W_FIRING_ALT);
    }
  }

  if(b_firing_alt != true && b_sound_firing_alt_trigger == true) {
    b_sound_firing_alt_trigger = false;

    if(b_cross_the_streams_mix == true) {
      stopEffect(S_FIRING_LOOP_GB1);
      stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);

      // Since Intensify is still held, turn back on its firing loop sounds.
      switch(i_power_level) {
        case 1 ... 4:
        default:
          if(getSystemYearMode() == SYSTEM_1989) {
            playEffect(S_GB2_FIRE_LOOP, true, i_volume_effects, false, 0, false);
          }
          else {
            playEffect(S_GB1_FIRE_LOOP, true, i_volume_effects, false, 0, false);
          }
        break;

        case 5:
          playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, false, 0, false);
        break;
      }

      // Tell the Proton Pack that the Neutrona Wand is no longer firing in Alt mode mix.
      wandSerialSend(W_FIRING_ALT_STOPPED_MIX);
    }
    else {
      // Tell the Proton Pack that the Neutrona Wand is no longer firing in Alt mode.
      //wandSerialSend(W_FIRING_ALT_STOPPED);
    }
  }

  if(b_firing_alt == true && b_firing_intensify == true && b_sound_firing_cross_the_streams != true && b_firing_cross_streams != true) {
    b_firing_cross_streams = true;
    b_sound_firing_cross_the_streams = true;

    switch(WAND_YEAR_CTS) {
      case CTS_AFTERLIFE:
      case CTS_FROZEN_EMPIRE:
        //stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);

        playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START, false, i_volume_effects, false, 0, false);

        if(b_cross_the_streams_mix == true) {
          if(AUDIO_DEVICE != A_GPSTAR_AUDIO) {
            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);
          }

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
        //stopEffect(S_CROSS_STREAMS_END);

        playEffect(S_CROSS_STREAMS_START, false, i_volume_effects, false, 0, false);

        if(b_cross_the_streams_mix == true) {
          if(AUDIO_DEVICE != A_GPSTAR_AUDIO) {
            stopEffect(S_CROSS_STREAMS_END);
          }

          // Tell the Proton Pack that the Neutrona Wand is crossing the streams mix.
          wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_MIX_1984);
        }
        else {
          // Tell the Proton Pack that the Neutrona Wand is crossing the streams.
          wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_1984);
        }
      break;

      case CTS_DEFAULT:
      default:
        switch(getSystemYearMode()) {
          case SYSTEM_AFTERLIFE:
          case SYSTEM_FROZEN_EMPIRE:
          default:
            //stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);

            playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START, false, i_volume_effects, false, 0, false);

            if(b_cross_the_streams_mix == true) {
              if(AUDIO_DEVICE != A_GPSTAR_AUDIO) {
                stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);
              }

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
            //stopEffect(S_CROSS_STREAMS_END);

            playEffect(S_CROSS_STREAMS_START, false, i_volume_effects, false, 0, false);

            if(b_cross_the_streams_mix == true) {
              if(AUDIO_DEVICE != A_GPSTAR_AUDIO) {
                stopEffect(S_CROSS_STREAMS_END);
              }

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

    if(b_stream_effects == true) {
      ms_impact.start(random(10,16) * 1000);
    }

    if(b_cross_the_streams_mix == true) {
      // Mix in some new proton stream sounds for CTS Mix.
      if(i_power_level != i_power_level_max && b_sound_firing_cross_the_streams_mix != true) {
        playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, false, 0, false);
        b_sound_firing_cross_the_streams_mix = true;
      }
      else if(i_power_level == i_power_level_max && b_sound_firing_cross_the_streams_mix != true) {
        playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, false, 0, false);
        b_sound_firing_cross_the_streams_mix = true;
      }

      if(getSystemYearMode() == SYSTEM_1989) {
        stopEffect(S_GB2_FIRE_LOOP);
      }
      else {
        stopEffect(S_GB1_FIRE_LOOP);
      }
    }
    else {
      // Mix in some new proton stream sounds for normal CTS.
      if(i_power_level != i_power_level_max) {
        playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, false, 0, false);
      }
      else {
        playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, false, 0, false);
      }
    }
  }

  if((b_firing_alt != true || b_firing_intensify != true) && b_firing_cross_streams == true && b_cross_the_streams_mix == true) {
    // In CTS Mix mode, you can release either Intensify or the Barrel Wing Button and firing will revert to the mode for the still-held button.
    b_firing_cross_streams = false;
    b_sound_firing_cross_the_streams = false;

    switch(WAND_YEAR_CTS) {
      case CTS_AFTERLIFE:
      case CTS_FROZEN_EMPIRE:
        if(AUDIO_DEVICE != A_GPSTAR_AUDIO) {
          stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
        }
        //stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);

        playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects, false, 0, false);

        wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_STOPPED_MIX_2021);
      break;

      case CTS_1984:
      case CTS_1989:
        if(AUDIO_DEVICE != A_GPSTAR_AUDIO) {
          stopEffect(S_CROSS_STREAMS_START);
        }
        //stopEffect(S_CROSS_STREAMS_END);

        playEffect(S_CROSS_STREAMS_END, false, i_volume_effects, false, 0, false);

        wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_STOPPED_MIX_1984);
      break;

      case CTS_DEFAULT:
      default:
        switch(getSystemYearMode()) {
          case SYSTEM_AFTERLIFE:
          case SYSTEM_FROZEN_EMPIRE:
          default:
            if(AUDIO_DEVICE != A_GPSTAR_AUDIO) {
              stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
            }
            //stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);

            playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects, false, 0, false);

            wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_STOPPED_MIX_2021);
          break;

          case SYSTEM_1984:
          case SYSTEM_1989:
            if(AUDIO_DEVICE != A_GPSTAR_AUDIO) {
              stopEffect(S_CROSS_STREAMS_START);
            }
            //stopEffect(S_CROSS_STREAMS_END);

            playEffect(S_CROSS_STREAMS_END, false, i_volume_effects, false, 0, false);

            wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_STOPPED_MIX_1984);
          break;
        }
      break;
    }

    // Restart the impact sound timer for the standalone wand.
    if(b_stream_effects == true && b_gpstar_benchtest == true) {
      ms_firing_sound_mix.start(random(7,15) * 1000);
    }
  }

  // Overheat timers.
  bool b_overheat_flag = true;

  if(b_cross_the_streams == true && b_firing_alt != true) {
    b_overheat_flag = false;
  }

  if(b_overheat_flag == true) {
    // If the user changes the wand power output while firing, turn off the overheat timer.
    if(b_overheat_level[i_power_level - 1] != true && ms_overheat_initiate.isRunning()) {
      ms_overheat_initiate.stop();

      // Adjust hat light 1 to stay solid.
      digitalWriteFast(led_hat_1, HIGH);

      ms_hat_1.stop();

      // Tell the pack to revert back to regular Cyclotron speeds.
      wandSerialSend(W_CYCLOTRON_NORMAL_SPEED);
    }
    else if(b_overheat_level[i_power_level - 1] == true && ms_overheat_initiate.remaining() == 0 && b_overheat_enabled == true) {
      // If the user changes back to power level that overheats while firing, start up a timer.
      // This currently works only in power levels 1-4. 5 stays locked when firing.
      ms_overheat_initiate.start(i_ms_overheat_initiate[i_power_level - 1]);
    }
  }
  else {
    if(ms_overheat_initiate.isRunning()) {
      ms_overheat_initiate.stop();

      // Tell the pack to revert back to regular Cyclotron speeds.
      wandSerialSend(W_CYCLOTRON_NORMAL_SPEED);
    }
  }

  // Initialize temporary colour variables to reduce code complexity.
  colours c_temp_start = C_WHITE;
  colours c_temp_effect = C_WHITE;

  switch(FIRING_MODE) {
    case PROTON:
    default:
      /*// Shift the stream from red to orange on higher power levels.
      switch(i_power_level) {
        case 1:
        default:
          c_temp_start = C_RED;
          c_temp_effect = C_BLUE;
        break;

        case 2:
          c_temp_start = C_RED2;
          c_temp_effect = C_BLUE;
        break;

        case 3:
          c_temp_start = C_RED3;
          c_temp_effect = C_MID_BLUE;
        break;

        case 4:
          c_temp_start = C_RED4;
          c_temp_effect = C_MID_BLUE;
        break;

        case 5:
          c_temp_start = C_RED5;
          c_temp_effect = C_LIGHT_BLUE;
        break;
      }*/
      if(b_firing_cross_streams == true) {
        if(getSystemYearMode() == SYSTEM_FROZEN_EMPIRE && !b_pack_cyclotron_lid_on) {
          c_temp_start = C_CHARTREUSE;
          c_temp_effect = C_ORANGE;
        }
        else {
          c_temp_start = C_WHITE;
          c_temp_effect = C_YELLOW;
        }
      }
      else {
        // Shift the stream from red to orange on higher power levels.
        switch(i_power_level) {
          case 1:
          default:
            c_temp_start = C_RED;
            c_temp_effect = C_BLUE;
          break;

          case 2:
            c_temp_start = C_RED2;
            c_temp_effect = C_BLUE;
          break;

          case 3:
            c_temp_start = C_RED3;
            c_temp_effect = C_LIGHT_BLUE;
          break;

          case 4:
            c_temp_start = C_RED4;
            c_temp_effect = C_LIGHT_BLUE;
          break;

          case 5:
            c_temp_start = C_RED5;
            c_temp_effect = C_WHITE;
          break;
        }
      }
    break;

    case SLIME:
      if(getSystemYearMode() == SYSTEM_1989) {
        c_temp_start = C_PASTEL_PINK;
        c_temp_effect = C_WHITE;
      }
      else {
        c_temp_start = C_DARK_GREEN;
        c_temp_effect = C_GREEN;
      }
    break;

    case STASIS:
      c_temp_start = C_BLUE;
      c_temp_effect = C_NAVY_BLUE;
    break;

    case MESON:
      c_temp_effect = C_YELLOW;
    break;

    case SPECTRAL:
      c_temp_start = C_RAINBOW;
      c_temp_effect = c_temp_start;
    break;

    case HOLIDAY:
      c_temp_start = C_REDGREEN;
      c_temp_effect = c_temp_start;
    break;

    case SPECTRAL_CUSTOM:
      c_temp_start = C_CUSTOM;

      if(i_spectral_wand_custom_saturation < 254) {
        c_temp_effect = C_BLUE;
      }
      else {
        c_temp_effect = C_WHITE;
      }
    break;
  }

  if(FIRING_MODE != MESON) {
    // Meson does not use "stream start" to make its pulse effect.
    fireStreamStart(getHueColour(c_temp_start, WAND_BARREL_LED_COUNT));
  }

  fireStreamEffect(getHueColour(c_temp_effect, WAND_BARREL_LED_COUNT));

  // Bargraph loop / scroll.
  if(ms_bargraph_firing.justFinished()) {
    bargraphRampFiring();
  }

  // Mix some impact sound every 10-15 seconds while firing.
  if(ms_impact.justFinished() && FIRING_MODE == PROTON && b_firing_cross_streams != true && b_stream_effects == true) {
    playEffect(S_FIRE_LOOP_IMPACT, false, i_volume_effects, false, 0, false);
    ms_impact.start(random(10,16) * 1000);
  }

  // Standalone Neutrona Wand gets additional impact sounds which would normally be played by Proton Pack.
  if(ms_firing_sound_mix.justFinished() && FIRING_MODE == PROTON && b_firing_cross_streams != true && b_stream_effects == true && b_gpstar_benchtest == true) {
    uint8_t i_random = 0;

    switch(i_last_firing_effect_mix) {
      case S_FIRE_SPARKS:
        i_random = random(0,2);
      break;

      case S_FIRE_SPARKS_3:
      case S_FIRE_SPARKS_4:
        i_random = 3;
      break;

      case S_FIRE_SPARKS_5:
        i_random = 2;
      break;

      case S_FIRE_SPARKS_2:
        i_random = 1;
      break;

      default:
        // If no firing effect has played yet.
        i_random = 3;
      break;
    }

    uint16_t i_s_random = random(2,4) * 1000;

    switch (i_random) {
      case 3:
        playEffect(S_FIRE_SPARKS, false, i_volume_effects, false, 0, false);
        i_last_firing_effect_mix = S_FIRE_SPARKS;

        ms_firing_sound_mix.start(i_s_random * 5);
      break;

      case 2:
        playEffect(S_FIRE_SPARKS_4, false, i_volume_effects, false, 0, false);
        i_last_firing_effect_mix = S_FIRE_SPARKS_4;

        ms_firing_sound_mix.start(i_s_random);
      break;

      case 1:
        playEffect(S_FIRE_SPARKS_3, false, i_volume_effects, false, 0, false);
        i_last_firing_effect_mix = S_FIRE_SPARKS_3;

        ms_firing_sound_mix.start(i_s_random);
      break;

      case 0:
        playEffect(S_FIRE_SPARKS_2, false, i_volume_effects, false, 0, false);
        playEffect(S_FIRE_SPARKS_5, false, i_volume_effects, false, 0, false);
        i_last_firing_effect_mix = S_FIRE_SPARKS_5;

        ms_firing_sound_mix.start(1800);
      break;

      default:
        // This will never trigger because i_random will only ever be 0~3.
        playEffect(S_FIRE_SPARKS_2, false, i_volume_effects, false, 0, false);
        i_last_firing_effect_mix = S_FIRE_SPARKS_2;

        ms_firing_sound_mix.start(500);
      break;
    }
  }
}

void wandHeatUp() {
  stopEffect(S_FIRE_START_SPARK);
  stopEffect(S_PACK_SLIME_OPEN);
  stopEffect(S_STASIS_OPEN);
  stopEffect(S_MESON_OPEN);
  stopEffect(S_VENT_DRY);
  stopEffect(S_MODE_SWITCH);

  if(b_gpstar_benchtest == true) {
    stopEffect(S_PACK_SLIME_TANK_LOOP);
    stopEffect(S_STASIS_IDLE_LOOP);
    stopEffect(S_MESON_IDLE_LOOP);
  }

  switch(FIRING_MODE) {
    case PROTON:
    default:
      playEffect(S_FIRE_START_SPARK);
    break;

    case SLIME:
      playEffect(S_PACK_SLIME_OPEN);

      if(b_gpstar_benchtest == true && WAND_STATUS == MODE_ON && switch_vent.on() == true) {
        playEffect(S_PACK_SLIME_TANK_LOOP, true, 0, true, 900);
      }
    break;

    case STASIS:
      playEffect(S_STASIS_OPEN);

      if(b_gpstar_benchtest == true && WAND_STATUS == MODE_ON && switch_vent.on() == true) {
        playEffect(S_STASIS_IDLE_LOOP, true, 0, true, 900);
      }
    break;

    case MESON:
      playEffect(S_MESON_OPEN);

      if(b_gpstar_benchtest == true && WAND_STATUS == MODE_ON && switch_vent.on() == true) {
        playEffect(S_MESON_IDLE_LOOP, true, 0, true, 900);
      }
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

  // Initialize temporary colour variable to reduce code complexity.
  colours c_temp = C_WHITE;

  if(b_wand_mash_error == true || b_pack_alarm == true) {
    // Special spark effect handling for button mash lockout and ribbon cable removal.

    if((i_bmash_spark_index < 1 && i_heatup_counter > 100) || (i_bmash_spark_index > 0 && i_heatup_counter > 75)) {
      wandBarrelHeatDown();
    }
    else if(ms_wand_heatup_fade.justFinished() && ((i_bmash_spark_index < 1 && i_heatup_counter <= 100) || (i_bmash_spark_index > 0 && i_heatup_counter <= 75))) {
      if(getSystemYearMode() == SYSTEM_FROZEN_EMPIRE && FIRING_MODE == PROTON && !b_pack_cyclotron_lid_on) {
        // Green goop effect in FE mode if the cyclotron lid is off.
        c_temp = C_CHARTREUSE;
      }
      else {
        c_temp = C_BEIGE;
      }

      switch(WAND_BARREL_LED_COUNT) {
        case LEDS_48:
          barrel_leds[i_barrel_led] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatup_counter);
          barrel_leds[i_barrel_led - 23] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatup_counter);
          barrel_leds[i_barrel_led - 24] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatup_counter);
          barrel_leds[i_barrel_led - 25] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatup_counter);
          barrel_leds[i_barrel_led + 1] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatup_counter);
        break;

        case LEDS_5:
        default:
          barrel_leds[i_barrel_led] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatup_counter);
        break;
      }

      if((i_bmash_spark_index < 1 && i_heatup_counter == 80) || (i_bmash_spark_index > 0 && i_heatup_counter == 60)) {
        wandTipOn(); // Flash the wand tip at the peak of each spark.
      }

      i_heatup_counter = i_heatup_counter + 5;
      ms_wand_heatup_fade.start(i_delay_heatup);
    }

    return;
  }

  if(i_heatup_counter > 100) {
    wandBarrelHeatDown();
  }
  else if(ms_wand_heatup_fade.justFinished() && i_heatup_counter <= 100) {
    switch(FIRING_MODE) {
      case PROTON:
      default:
        // Do nothing since c_temp is already C_WHITE.
      break;

      case SLIME:
        if(getSystemYearMode() == SYSTEM_1989) {
          c_temp = C_PASTEL_PINK;
        }
        else {
          c_temp = C_GREEN;
        }
      break;

      case STASIS:
        c_temp = C_BLUE;
      break;

      case MESON:
        c_temp = C_YELLOW;
      break;

      case SPECTRAL:
        c_temp = C_RAINBOW;
      break;

      case HOLIDAY:
        c_temp = C_REDGREEN;
      break;

      case SPECTRAL_CUSTOM:
        c_temp = C_CUSTOM;
      break;
    }

    switch(WAND_BARREL_LED_COUNT) {
      case LEDS_48:
        barrel_leds[i_barrel_led] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatup_counter);
        barrel_leds[i_barrel_led - 23] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatup_counter);
        barrel_leds[i_barrel_led - 24] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatup_counter);
        barrel_leds[i_barrel_led - 25] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatup_counter);
        barrel_leds[i_barrel_led + 1] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatup_counter);
      break;

      case LEDS_5:
      default:
        barrel_leds[i_barrel_led] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatup_counter);
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

  // Initialize temporary colour variable to reduce code complexity.
  colours c_temp = C_WHITE;

  if(b_wand_mash_error == true || b_pack_alarm == true) {
    // Special spark effect handling for button mash lockout and ribbon cable removal.

    if(ms_wand_heatup_fade.justFinished() && i_heatdown_counter > 0) {
      if(getSystemYearMode() == SYSTEM_FROZEN_EMPIRE && FIRING_MODE == PROTON && !b_pack_cyclotron_lid_on) {
        // Green goop effect in FE mode if the cyclotron lid is off.
        c_temp = C_CHARTREUSE;
      }
      else {
        c_temp = C_BEIGE;
      }

      switch(WAND_BARREL_LED_COUNT) {
        case LEDS_48:
          barrel_leds[i_barrel_led] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatdown_counter);
          barrel_leds[i_barrel_led - 23] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatdown_counter);
          barrel_leds[i_barrel_led - 24] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatdown_counter);
          barrel_leds[i_barrel_led - 25] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatdown_counter);
          barrel_leds[i_barrel_led + 1] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatdown_counter);
        break;

        case LEDS_5:
        default:
          barrel_leds[i_barrel_led] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatdown_counter);
        break;
      }

      if((i_bmash_spark_index < 1 && i_heatdown_counter == 80) || (i_bmash_spark_index > 0 && i_heatdown_counter == 60)) {
        wandTipOff(); // Flash the wand tip at the peak of each spark.
      }

      i_heatdown_counter = i_heatdown_counter - 5;

      if(i_heatdown_counter == 0 || i_heatdown_counter > 200) {
        i_heatup_counter = 0;
        i_heatdown_counter = 75;
        i_bmash_spark_index++;
      }

      if(i_bmash_spark_index < 3) {
        ms_wand_heatup_fade.start(i_delay_heatup);
      }
      else {
        barrelLightsOff();
      }
    }

    return;
  }

  if(ms_wand_heatup_fade.justFinished() && i_heatdown_counter > 0) {
    switch(FIRING_MODE) {
      case PROTON:
      default:
        // Do nothing since c_temp is already C_WHITE.
      break;

      case SLIME:
        if(getSystemYearMode() == SYSTEM_1989) {
          c_temp = C_PASTEL_PINK;
        }
        else {
          c_temp = C_GREEN;
        }
      break;

      case STASIS:
        c_temp = C_BLUE;
      break;

      case MESON:
        c_temp = C_YELLOW;
      break;

      case SPECTRAL:
        c_temp = C_RAINBOW;
      break;

      case HOLIDAY:
        c_temp = C_REDGREEN;
      break;

      case SPECTRAL_CUSTOM:
        c_temp = C_CUSTOM;
      break;
    }

    switch(WAND_BARREL_LED_COUNT) {
      case LEDS_48:
        barrel_leds[i_barrel_led] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatdown_counter);
        barrel_leds[i_barrel_led - 23] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatdown_counter);
        barrel_leds[i_barrel_led - 24] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatdown_counter);
        barrel_leds[i_barrel_led - 25] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatdown_counter);
        barrel_leds[i_barrel_led + 1] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatdown_counter);
      break;

      case LEDS_5:
      default:
        barrel_leds[i_barrel_led] = getHueColour(c_temp, WAND_BARREL_LED_COUNT, i_heatdown_counter);
      break;
    }

    i_heatdown_counter--;
    ms_wand_heatup_fade.start(i_delay_heatup);
  }

  if(i_heatdown_counter == 0) {
    barrelLightsOff();
  }
}

void barrelLEDTranslation(uint8_t id, colours color) {
  if(WAND_BARREL_LED_COUNT != LEDS_48) {
    barrel_leds[id] = getHueColour(color, WAND_BARREL_LED_COUNT);
    return;
  }
  else {
    switch(id) {
      case 0:
        // Translate to first three rows of LEDs.
        for(uint8_t i = 0; i < 12; i++) {
          barrel_leds[PROGMEM_READU8(frutto_barrel[i])] = getHueColour(color, WAND_BARREL_LED_COUNT);
        }
      break;

      case 1:
        // Translate to rows 4 and 5 of the LED array.
        for(uint8_t i = 12; i < 20; i++) {
          barrel_leds[PROGMEM_READU8(frutto_barrel[i])] = getHueColour(color, WAND_BARREL_LED_COUNT);
        }
      break;

      case 2:
        // Translate to rows 6 and 7 of the LED array.
        for(uint8_t i = 20; i < 28; i++) {
          barrel_leds[PROGMEM_READU8(frutto_barrel[i])] = getHueColour(color, WAND_BARREL_LED_COUNT);
        }
      break;

      case 3:
        // Translate to rows 8 and 9 of the LED array.
        for(uint8_t i = 28; i < 36; i++) {
          barrel_leds[PROGMEM_READU8(frutto_barrel[i])] = getHueColour(color, WAND_BARREL_LED_COUNT);
        }
      break;

      case 4:
        // Translate to the last three rows of LEDs.
        for(uint8_t i = 36; i < 48; i++) {
          barrel_leds[PROGMEM_READU8(frutto_barrel[i])] = getHueColour(color, WAND_BARREL_LED_COUNT);
        }
      break;

      default:
        // Do nothing.
      break;
    }
  }
}

void firePulseEffect() {
  if(i_pulse_step < 1) {
    // Play bargraph animation when pulse sequence begins.
    switch(BARGRAPH_MODE) {
      case BARGRAPH_ORIGINAL:
        // Need to restart the regular bargraph timer.
        i_bargraph_status = i_power_level - 1;
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
      break;

      case BARGRAPH_SUPER_HERO:
      default:
        i_bargraph_status = i_power_level - 1;
        i_bargraph_status_alt = 0;
        bargraphClearAlt();

        i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_1984;

        // We ramp the bargraph back up after finishing firing.
        bargraphRampUp();
      break;
    }
  }

  if(FIRING_MODE == SLIME) {
    ms_firing_stream_effects.start(0); // Start new barrel animation.

    // Draw first pixel.
    if(getSystemYearMode() == SYSTEM_1989) {
      fireStreamEffect(getHueColour(C_WHITE, WAND_BARREL_LED_COUNT));
    }
    else {
      fireStreamEffect(getHueColour(C_GREEN, WAND_BARREL_LED_COUNT));
    }

    ms_firing_effect_end.start(0); // Immediately end animation.
    i_pulse_step = 14; // Immediately go to end of sequence.
  }

  uint8_t i_firing_pulse = d_firing_pulse; // Stores a calculated value based on firing mode.

  switch(i_pulse_step) {
    case 0:
      switch(FIRING_MODE) {
        case PROTON:
          // Boson Dart.
          barrelLEDTranslation(0, C_RED);
        break;

        case STASIS:
          // Shock Blast.
          barrelLEDTranslation(0, C_NAVY_BLUE);
        break;

        case MESON:
          // Meson Collider.
          barrelLEDTranslation(0, C_YELLOW);
        break;

        default:
          // Do nothing.
        break;
      }
    break;

    case 1:
      switch(FIRING_MODE) {
        case PROTON:
          // Boson Dart.
          barrelLEDTranslation(0, C_RED2);
          barrelLEDTranslation(1, C_RED);
        break;

        case STASIS:
          // Shock Blast.
          barrelLEDTranslation(0, C_MID_BLUE);
        break;

        case MESON:
          // Meson Collider.
          barrelLEDTranslation(0, C_ORANGE);
        break;

        default:
          // Do nothing.
        break;
      }
    break;

    case 2:
      switch(FIRING_MODE) {
        case PROTON:
          // Boson Dart.
          barrelLEDTranslation(0, C_WHITE);
          barrelLEDTranslation(1, C_RED2);
          barrelLEDTranslation(2, C_RED);
        break;

        case STASIS:
          // Shock Blast.
          barrelLEDTranslation(0, C_LIGHT_BLUE);
          barrelLEDTranslation(1, C_NAVY_BLUE);
        break;

        case MESON:
          // Meson Collider.
          barrelLEDTranslation(0, C_RED4);
          barrelLEDTranslation(1, C_YELLOW);
        break;

        default:
          // Do nothing.
        break;
      }
    break;

    case 3:
      switch(FIRING_MODE) {
        case PROTON:
          // Boson Dart.
          barrelLEDTranslation(0, C_RED2);
          barrelLEDTranslation(1, C_WHITE);
          barrelLEDTranslation(2, C_RED2);
          barrelLEDTranslation(3, C_RED);
        break;

        case STASIS:
          // Shock Blast.
          barrelLEDTranslation(0, C_BLUE);
          barrelLEDTranslation(1, C_MID_BLUE);
        break;

        case MESON:
          // Meson Collider.
          barrelLEDTranslation(0, C_RED2);
          barrelLEDTranslation(1, C_ORANGE);
        break;

        default:
          // Do nothing.
        break;
      }
    break;

    case 4:
      switch(FIRING_MODE) {
        case PROTON:
          // Boson Dart.
          barrelLEDTranslation(0, C_RED);
          barrelLEDTranslation(1, C_RED2);
          barrelLEDTranslation(2, C_WHITE);
          barrelLEDTranslation(3, C_RED2);
          barrelLEDTranslation(4, C_RED);

          // Bolt has reached barrel tip, so turn on tip light.
          wandTipOn();
        break;

        case STASIS:
          // Shock Blast.
          barrelLEDTranslation(0, C_WHITE);
          barrelLEDTranslation(1, C_LIGHT_BLUE);
          barrelLEDTranslation(2, C_NAVY_BLUE);
        break;

        case MESON:
          // Meson Collider.
          barrelLEDTranslation(0, C_RED);
          barrelLEDTranslation(1, C_RED4);
          barrelLEDTranslation(2, C_YELLOW);
        break;

        default:
          // Do nothing.
        break;
      }
    break;

    case 5:
      switch(FIRING_MODE) {
        case PROTON:
          // Boson Dart.
          barrelLEDTranslation(0, C_BLACK);
          barrelLEDTranslation(1, C_RED);
          barrelLEDTranslation(2, C_RED2);
          barrelLEDTranslation(3, C_WHITE);
          barrelLEDTranslation(4, C_RED2);
        break;

        case STASIS:
          // Shock Blast.
          barrelLEDTranslation(0, C_BLACK);
          barrelLEDTranslation(1, C_BLUE);
          barrelLEDTranslation(2, C_MID_BLUE);
        break;

        case MESON:
          // Meson Collider.
          barrelLEDTranslation(0, C_BLACK);
          barrelLEDTranslation(1, C_RED2);
          barrelLEDTranslation(2, C_ORANGE);
        break;

        default:
          // Do nothing.
        break;
      }
    break;

    case 6:
      switch(FIRING_MODE) {
        case PROTON:
          // Boson Dart.
          barrelLEDTranslation(1, C_BLACK);
          barrelLEDTranslation(2, C_RED);
          barrelLEDTranslation(3, C_RED2);
          barrelLEDTranslation(4, C_WHITE);
        break;

        case STASIS:
          // Shock Blast.
          barrelLEDTranslation(1, C_WHITE);
          barrelLEDTranslation(2, C_LIGHT_BLUE);
          barrelLEDTranslation(3, C_NAVY_BLUE);
        break;

        case MESON:
          // Meson Collider.
          barrelLEDTranslation(1, C_RED);
          barrelLEDTranslation(2, C_RED4);
          barrelLEDTranslation(3, C_YELLOW);
        break;

        default:
          // Do nothing.
        break;
      }
    break;

    case 7:
      switch(FIRING_MODE) {
        case PROTON:
          // Boson Dart.
          barrelLEDTranslation(2, C_BLACK);
          barrelLEDTranslation(3, C_RED);
          barrelLEDTranslation(4, C_RED2);
        break;

        case STASIS:
          // Shock Blast.
          barrelLEDTranslation(1, C_BLACK);
          barrelLEDTranslation(2, C_BLUE);
          barrelLEDTranslation(3, C_MID_BLUE);
        break;

        case MESON:
          // Meson Collider.
          barrelLEDTranslation(1, C_BLACK);
          barrelLEDTranslation(2, C_RED2);
          barrelLEDTranslation(3, C_ORANGE);
        break;

        default:
          // Do nothing.
        break;
      }
    break;

    case 8:
      switch(FIRING_MODE) {
        case PROTON:
          // Boson Dart.
          barrelLEDTranslation(3, C_BLACK);
          barrelLEDTranslation(4, C_RED);
        break;

        case STASIS:
          // Shock Blast.
          barrelLEDTranslation(2, C_WHITE);
          barrelLEDTranslation(3, C_LIGHT_BLUE);
          barrelLEDTranslation(4, C_NAVY_BLUE);

          // Pulse has reached tip, so turn tip light on.
          wandTipOn();
        break;

        case MESON:
          // Meson Collider.
          barrelLEDTranslation(2, C_RED);
          barrelLEDTranslation(3, C_RED4);
          barrelLEDTranslation(4, C_YELLOW);

          // Pulse has reached tip, so turn tip light on.
          wandTipOn();
        break;

        default:
          // Do nothing.
        break;
      }
    break;

    case 9:
      switch(FIRING_MODE) {
        case PROTON:
          // Boson Dart.
          barrelLEDTranslation(4, C_BLACK);
          i_pulse_step = 14; // Immediately end.
        break;

        case STASIS:
          // Shock Blast.
          barrelLEDTranslation(2, C_BLACK);
          barrelLEDTranslation(3, C_BLUE);
          barrelLEDTranslation(4, C_MID_BLUE);
        break;

        case MESON:
          // Meson Collider.
          barrelLEDTranslation(2, C_BLACK);
          barrelLEDTranslation(3, C_RED2);
          barrelLEDTranslation(4, C_ORANGE);
        break;

        default:
          // Do nothing.
        break;
      }
    break;

    case 10:
      switch(FIRING_MODE) {
        case STASIS:
          // Shock Blast.
          barrelLEDTranslation(3, C_WHITE);
          barrelLEDTranslation(4, C_LIGHT_BLUE);
        break;

        case MESON:
          // Meson Collider.
          barrelLEDTranslation(3, C_RED);
          barrelLEDTranslation(4, C_RED4);
        break;

        default:
          // Do nothing.
        break;
      }
    break;

    case 11:
      switch(FIRING_MODE) {
        case STASIS:
          // Shock Blast.
          barrelLEDTranslation(3, C_BLACK);
          barrelLEDTranslation(4, C_BLUE);
        break;

        case MESON:
          // Meson Collider.
          barrelLEDTranslation(3, C_BLACK);
          barrelLEDTranslation(4, C_RED2);
        break;

        default:
          // Do nothing.
        break;
      }
    break;

    case 12:
      switch(FIRING_MODE) {
        case STASIS:
          // Shock Blast.
          barrelLEDTranslation(4, C_WHITE);
        break;

        case MESON:
          // Meson Collider.
          barrelLEDTranslation(4, C_RED);
        break;

        default:
          // Do nothing.
        break;
      }
    break;

    case 13:
      barrelLEDTranslation(4, C_BLACK);
    break;

    default:
      // Do nothing if we somehow end up here.
    break;
  }

  i_pulse_step++;

  if(i_pulse_step < 14) {
    if(FIRING_MODE == PROTON) {
      // Boson Dart is much slower than the others.
      i_firing_pulse *= 2;
    }
    ms_firing_pulse.start(i_firing_pulse);
  }
  else {
    // Animation has concluded, so reset our timer and variable.
    wandTipOff();
    ms_firing_pulse.stop();
    i_pulse_step = 0;
  }
}

void fireStreamEffect(CRGB c_colour) {
  uint8_t i_firing_stream; // Stores a calculated value based on LED count.

  switch(WAND_BARREL_LED_COUNT) {
    case LEDS_48:
      // Frutto Technology - 48 LED + Strobe Tip
      // This effect will "wrap" around the device to appear to push the stream forward.

      i_firing_stream = d_firing_stream / 10;

      if(ms_firing_stream_effects.justFinished()) {
        if(i_barrel_light - 1 < i_num_barrel_leds) {
          switch(FIRING_MODE) {
            case PROTON:
            default:
              if(b_firing_cross_streams == true) {
                if(getSystemYearMode() == SYSTEM_FROZEN_EMPIRE && !b_pack_cyclotron_lid_on) {
                  barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 1])] = getHueColour(C_CHARTREUSE, WAND_BARREL_LED_COUNT);
                  //barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 2])] = c_colour;
                }
                else {
                  barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 1])] = getHueColour(C_WHITE, WAND_BARREL_LED_COUNT);
                  //barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 2])] = c_colour;
                }
              }
              else {
                // Shift the stream from red to orange on higher power levels.
                switch(i_power_level) {
                  case 1:
                  default:
                    barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 1])] = getHueColour(C_RED, WAND_BARREL_LED_COUNT);
                  break;

                  case 2:
                    barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 1])] = getHueColour(C_RED2, WAND_BARREL_LED_COUNT);
                  break;

                  case 3:
                    barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 1])] = getHueColour(C_RED3, WAND_BARREL_LED_COUNT);
                  break;

                  case 4:
                    barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 1])] = getHueColour(C_RED4, WAND_BARREL_LED_COUNT);
                  break;

                  case 5:
                    barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 1])] = getHueColour(C_RED5, WAND_BARREL_LED_COUNT);
                  break;
                }
              }
            break;

            case SLIME:
              if(getSystemYearMode() == SYSTEM_1989) {
                barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 1])] = getHueColour(C_PASTEL_PINK, WAND_BARREL_LED_COUNT);
              }
              else {
                barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 1])] = getHueColour(C_DARK_GREEN, WAND_BARREL_LED_COUNT);
              }
            break;

            case STASIS:
              barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 1])] = getHueColour(C_BLUE, WAND_BARREL_LED_COUNT);
            break;

            case MESON:
            case SPECTRAL:
            case HOLIDAY:
              barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 1])] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
            break;

            case SPECTRAL_CUSTOM:
              barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 1])] = getHueColour(C_CUSTOM, WAND_BARREL_LED_COUNT);
            break;
          }
        }

        if(i_barrel_light == i_num_barrel_leds) {
          i_barrel_light = 0;

          uint8_t i_s_speed = 0;

          switch(FIRING_MODE) {
            case MESON:
              // Do nothing; animation is restarted by checkWandAction();
            break;

            default:
              switch(i_power_level) {
                case 1:
                default:
                  i_s_speed = 5; // 5ms
                break;

                case 2:
                  i_s_speed = 6; // 4ms
                break;

                case 3:
                  i_s_speed = 7; // 3ms
                break;

                case 4:
                  i_s_speed = 8; // 2ms
                break;

                case 5:
                  i_s_speed = 9; // 1ms
                break;
              }
            break;
          }

          if(FIRING_MODE != MESON) {
            ms_firing_stream_effects.start(i_firing_stream - i_s_speed);
          }
        }
        else if(i_barrel_light < i_num_barrel_leds) {
          barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light])] = c_colour;

          switch(FIRING_MODE) {
            case MESON:
              if(i_barrel_light + 1 < i_num_barrel_leds) {
                barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light + 1])] = c_colour;
              }

              if(i_barrel_light + 2 < i_num_barrel_leds) {
                barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light + 2])] = c_colour;
              }

              if(i_barrel_light + 3 < i_num_barrel_leds) {
                barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light + 3])] = c_colour;
              }
            break;

            case PROTON:
            {
              uint8_t i_t_rand = random(0, i_num_barrel_leds / 3); // from 0 to 15

              switch(i_power_level) {
                case 5:
                  i_t_rand = i_t_rand + 10;
                break;

                case 4:
                  i_t_rand = i_t_rand + 8;
                break;

                case 3:
                  i_t_rand = i_t_rand + 6;
                break;

                case 2:
                  i_t_rand = i_t_rand + 3;
                break;

                case 1:
                default:
                i_t_rand = i_t_rand + 2;
                  // Nothing.
                break;
              }

              for(uint8_t i = i_barrel_light + 1; i < i_barrel_light + i_t_rand; i++) {
                if(i < i_num_barrel_leds) {
                  barrel_leds[PROGMEM_READU8(frutto_barrel[i])] = c_colour;
                }
              }
            }
            break;

            default:
            {
              uint8_t i_t_rand_def = random(0, i_num_barrel_leds / 4); // from 0 to 11

              for(uint8_t i = i_barrel_light + 1; i < i_barrel_light + i_t_rand_def; i++) {
                if(i < i_num_barrel_leds) {
                  barrel_leds[PROGMEM_READU8(frutto_barrel[i])] = c_colour;
                }
              }
            }
            break;
          }

          switch(FIRING_MODE) {
            case MESON:
              switch(i_power_level) {
                case 1:
                default:
                  i_fast_led_delay = FAST_LED_UPDATE_MS; // 3ms
                  ms_firing_stream_effects.start((d_firing_stream / 25)); // 4ms
                break;

                case 2:
                  i_fast_led_delay = FAST_LED_UPDATE_MS; // 3ms
                  ms_firing_stream_effects.start((d_firing_stream / 25) - 1); // 3ms
                break;

                case 3:
                  i_fast_led_delay = FAST_LED_UPDATE_MS + 1; // 4ms
                  ms_firing_stream_effects.start((d_firing_stream / 25) - 1); // 3ms
                break;

                case 4:
                  i_fast_led_delay = FAST_LED_UPDATE_MS + 3; // 6ms
                  ms_firing_stream_effects.start((d_firing_stream / 25) - 1); // 3ms
                break;

                case 5:
                  i_fast_led_delay = FAST_LED_UPDATE_MS + 4; // 7ms
                  ms_firing_stream_effects.start((d_firing_stream / 25) - 2); // 2ms
                break;
              }
            break;

            case PROTON:
              switch(i_power_level) {
                case 1:
                default:
                  ms_firing_stream_effects.start((d_firing_stream / 25) + 4); // 8ms
                break;

                case 2:
                  ms_firing_stream_effects.start((d_firing_stream / 25) + 3); // 7ms
                break;

                case 3:
                  ms_firing_stream_effects.start((d_firing_stream / 25) + 2); // 6ms
                break;

                case 4:
                  ms_firing_stream_effects.start((d_firing_stream / 25) + 1); // 5ms
                break;

                case 5:
                  ms_firing_stream_effects.start((d_firing_stream / 25)); // 4ms
                break;
              }
            break;

            case SLIME:
              if(WAND_ACTION_STATUS == ACTION_FIRING) {
                switch(i_power_level) {
                  case 1:
                  default:
                    ms_firing_stream_effects.start((d_firing_stream / 25) + 2); // 6ms
                  break;

                  case 2:
                    ms_firing_stream_effects.start((d_firing_stream / 25) + 1); // 5ms
                  break;

                  case 3:
                    ms_firing_stream_effects.start((d_firing_stream / 25)); // 4ms
                  break;

                  case 4:
                    ms_firing_stream_effects.start((d_firing_stream / 25) - 1); // 3ms
                  break;

                  case 5:
                    ms_firing_stream_effects.start((d_firing_stream / 25) - 2); // 2ms
                  break;
                }
              }
              else {
                // Slime Tether response time is a fixed value.
                ms_firing_stream_effects.start((d_firing_stream / 25) - 3); // 1ms

                // Let Slime Tether turn on the barrel tip.
                if(i_barrel_light + 4 == i_num_barrel_leds) {
                  wandTipOn();
                }
              }
            break;

            default:
              switch(i_power_level) {
                case 1:
                default:
                  ms_firing_stream_effects.start((d_firing_stream / 25) + 2); // 6ms
                break;

                case 2:
                  ms_firing_stream_effects.start((d_firing_stream / 25) + 1); // 5ms
                break;

                case 3:
                  ms_firing_stream_effects.start((d_firing_stream / 25)); // 4ms
                break;

                case 4:
                  ms_firing_stream_effects.start((d_firing_stream / 25) - 1); // 3ms
                break;

                case 5:
                  ms_firing_stream_effects.start((d_firing_stream / 25) - 2); // 2ms
                break;
              }
            break;
          }

          i_barrel_light++;
        }
      }
    break;

    case LEDS_5:
    default:
      i_firing_stream = d_firing_stream;

      if(ms_firing_stream_effects.justFinished()) {
        if(i_barrel_light - 1 < i_num_barrel_leds) {
          switch(FIRING_MODE) {
            case PROTON:
            default:
              if(b_firing_cross_streams == true) {
                if(getSystemYearMode() == SYSTEM_FROZEN_EMPIRE && !b_pack_cyclotron_lid_on) {
                  barrel_leds[i_barrel_light - 1] = getHueColour(C_CHARTREUSE, WAND_BARREL_LED_COUNT);
                }
                else {
                  barrel_leds[i_barrel_light - 1] = getHueColour(C_WHITE, WAND_BARREL_LED_COUNT);
                }
              }
              else {
                // Shift the stream from red to orange on higher power levels.
                switch(i_power_level) {
                  case 1:
                  default:
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
                }
              }
            break;

            case SLIME:
              if(getSystemYearMode() == SYSTEM_1989) {
                barrel_leds[i_barrel_light - 1] = getHueColour(C_PASTEL_PINK, WAND_BARREL_LED_COUNT);
              }
              else {
                barrel_leds[i_barrel_light - 1] = getHueColour(C_DARK_GREEN, WAND_BARREL_LED_COUNT);
              }
            break;

            case STASIS:
              barrel_leds[i_barrel_light - 1] = getHueColour(C_BLUE, WAND_BARREL_LED_COUNT);
            break;

            case MESON:
            case SPECTRAL:
            case HOLIDAY:
              barrel_leds[i_barrel_light - 1] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
            break;

            case SPECTRAL_CUSTOM:
              barrel_leds[i_barrel_light - 1] = getHueColour(C_CUSTOM, WAND_BARREL_LED_COUNT);
            break;
          }
        }

        if(i_barrel_light == i_num_barrel_leds) {
          i_barrel_light = 0;

          switch(FIRING_MODE) {
            case MESON:
              // Do nothing; animation is restarted by checkWandAction();
            break;

            default:
              switch(i_power_level) {
                case 1:
                default:
                  ms_firing_stream_effects.start(i_firing_stream); // 100ms
                break;

                case 2:
                  ms_firing_stream_effects.start(i_firing_stream - 15); // 85ms
                break;

                case 3:
                  ms_firing_stream_effects.start(i_firing_stream - 30); // 70ms
                break;

                case 4:
                  ms_firing_stream_effects.start(i_firing_stream - 45); // 55ms
                break;

                case 5:
                  ms_firing_stream_effects.start(i_firing_stream - 60); // 40ms
                break;
              }
            break;
          }
        }
        else if(i_barrel_light < i_num_barrel_leds) {
          barrel_leds[i_barrel_light] = c_colour;

          switch(FIRING_MODE) {
            default:
              switch(i_power_level) {
                case 1:
                default:
                  ms_firing_stream_effects.start((d_firing_stream / 5) + 10); // 30ms
                break;

                case 2:
                  ms_firing_stream_effects.start((d_firing_stream / 5) + 8); // 28ms
                break;

                case 3:
                  ms_firing_stream_effects.start((d_firing_stream / 5) + 6); // 26ms
                break;

                case 4:
                  ms_firing_stream_effects.start((d_firing_stream / 5) + 5); // 25ms
                break;

                case 5:
                  ms_firing_stream_effects.start((d_firing_stream / 5) + 4); // 24ms
                break;
              }
            break;
          }

          i_barrel_light++;
        }
      }
    break;
  }
}

void barrelLightsOff() {
  ms_firing_pulse.stop();
  ms_firing_lights.stop();
  ms_firing_stream_effects.stop();
  ms_firing_effect_end.stop();
  ms_firing_lights_end.stop();
  ms_wand_heatup_fade.stop();
  i_barrel_light = 0;
  i_pulse_step = 0;
  i_heatup_counter = 0;
  i_heatdown_counter = 100;

  // Turn off the barrel LEDs.
  wandBarrelLightsOff();

  // Turn off the wand barrel tip LED.
  wandTipOff();
}

void fireStreamStart(CRGB c_colour) {
  if(ms_firing_lights.justFinished() && i_barrel_light < i_num_barrel_leds) {
    switch(WAND_BARREL_LED_COUNT) {
      case LEDS_48:
        // Since this arrangement has many more LEDs available, we can make use of extra colour changes
        // to enhance the stream effects. In this case we can darken the lead LED then follow with the
        // primary colour for the stream chosen. Any other colour effects will follow this arrangement.
        barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light])] = c_colour;

        if(i_barrel_light + 2 < i_num_barrel_leds) {
          barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light + 2])] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
        }
      break;

      case LEDS_5:
      default:
        // Just set the current LED to the expected colour.
        barrel_leds[i_barrel_light] = c_colour;
      break;
    }

    switch(WAND_BARREL_LED_COUNT) {
      case LEDS_48:
        // More LEDs means a faster firing rate.
        ms_firing_lights.start(d_firing_stream / 30);
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
          ms_firing_stream_effects.start(d_firing_stream / 25);
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

void fireEffectEnd() {
  // Initialize temporary colour variable to reduce code complexity.
  colours c_temp = C_WHITE;

  if(i_barrel_light < i_num_barrel_leds && ms_firing_stream_effects.isRunning()) {
    switch(FIRING_MODE) {
      case PROTON:
      default:
        if(b_firing_cross_streams == true) {
          if(getSystemYearMode() == SYSTEM_FROZEN_EMPIRE && !b_pack_cyclotron_lid_on) {
            c_temp = C_ORANGE;
          }
          else {
            c_temp = C_YELLOW;
          }
        }
        else {
          // Shift the stream from red to orange on higher power levels.
          switch(i_power_level) {
            case 1:
            default:
              c_temp = C_BLUE;
            break;

            case 2:
              c_temp = C_BLUE;
            break;

            case 3:
              c_temp = C_LIGHT_BLUE;
            break;

            case 4:
              c_temp = C_LIGHT_BLUE;
            break;

            case 5:
              c_temp = C_WHITE;
            break;
          }
        }
        /*switch(i_power_level) {
          case 1:
          default:
            c_temp = C_BLUE;
          break;

          case 2:
            c_temp = C_BLUE;
          break;

          case 3:
            c_temp = C_MID_BLUE;
          break;

          case 4:
            c_temp = C_MID_BLUE;
          break;

          case 5:
            c_temp = C_LIGHT_BLUE;
          break;
        }*/
      break;

      case SLIME:
        if(getSystemYearMode() == SYSTEM_1989) {
          c_temp = C_WHITE;
        }
        else {
          c_temp = C_GREEN;
        }
      break;

      case STASIS:
        c_temp = C_NAVY_BLUE;
      break;

      case MESON:
        c_temp = C_YELLOW;
      break;

      case SPECTRAL:
        c_temp = C_RAINBOW;
      break;

      case HOLIDAY:
        c_temp = C_REDGREEN;
      break;

      case SPECTRAL_CUSTOM:
        if(i_spectral_wand_custom_saturation < 254) {
          c_temp = C_BLUE;
        }
        else {
          c_temp = C_WHITE;
        }
      break;
    }

    fireStreamEffect(getHueColour(c_temp, WAND_BARREL_LED_COUNT));

    if(i_barrel_light < i_num_barrel_leds) {
      ms_firing_effect_end.repeat();
    }
    else {
      // Give a slight delay for the final pixel before clearing it.
      uint8_t i_firing_stream; // Stores a calculated value based on LED count.

      uint8_t i_s_speed = 0; // Stores an additional value used for the 48-LED barrel.

      switch(FIRING_MODE) {
        case MESON:
          switch(i_power_level) {
            case 1:
            default:
              i_s_speed = 8;
            break;

            case 2:
              i_s_speed = 8;
            break;

            case 3:
              i_s_speed = 8;
            break;

            case 4:
              i_s_speed = 8;
            break;

            case 5:
              i_s_speed = 9;
            break;
          }
        break;

        default:
          switch(i_power_level) {
            case 1:
            default:
              i_s_speed = 5;
            break;

            case 2:
              i_s_speed = 6;
            break;

            case 3:
              i_s_speed = 7;
            break;

            case 4:
              i_s_speed = 8;
            break;

            case 5:
              i_s_speed = 9;
            break;
          }
        break;
      }

      switch(WAND_BARREL_LED_COUNT) {
        case LEDS_48:
          // More LEDs means a faster firing rate.
          i_firing_stream = d_firing_stream / 10;
          i_firing_stream = i_firing_stream - i_s_speed;
        break;

        case LEDS_5:
        default:
          // Firing at "normal" speed.
          i_firing_stream = d_firing_stream;

          switch(i_power_level) {
            case 1:
            default:
              // Do nothing.
            break;

            case 2:
              i_firing_stream = i_firing_stream - 15;
            break;

            case 3:
              i_firing_stream = i_firing_stream - 30;
            break;

            case 4:
              i_firing_stream = i_firing_stream - 45;
            break;

            case 5:
              i_firing_stream = i_firing_stream - 60;
            break;
          }
        break;
      }

      ms_firing_effect_end.start(i_firing_stream);
      ms_firing_stream_effects.stop();
    }
  }
  else {
    switch(FIRING_MODE) {
      case PROTON:
      default:
        if(b_firing_cross_streams == true) {
          if(getSystemYearMode() == SYSTEM_FROZEN_EMPIRE && !b_pack_cyclotron_lid_on) {
            c_temp = C_CHARTREUSE;
          }
          else {
            c_temp = C_WHITE;
          }
        }
        else {
          switch(i_power_level) {
            case 1:
            default:
              c_temp = C_RED;
            break;

            case 2:
              c_temp = C_RED2;
            break;

            case 3:
              c_temp = C_RED3;
            break;

            case 4:
              c_temp = C_RED4;
            break;

            case 5:
              c_temp = C_RED5;
            break;
          }
        }
      break;

      case SLIME:
        if(getSystemYearMode() == SYSTEM_1989) {
          c_temp = C_PASTEL_PINK;
        }
        else {
          c_temp = C_DARK_GREEN;
        }
      break;

      case STASIS:
        c_temp = C_BLUE;
      break;

      case MESON:
      case SPECTRAL:
      case HOLIDAY:
        c_temp = C_BLACK;
      break;

      case SPECTRAL_CUSTOM:
        c_temp = C_CUSTOM;
      break;
    }

    switch(WAND_BARREL_LED_COUNT) {
      case LEDS_48:
        // Set the final LED back to whatever color it is without the effect.
        barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light - 1])] = getHueColour(c_temp, WAND_BARREL_LED_COUNT);
      break;

      case LEDS_5:
      default:
        // Set the final LED back to whatever color it is without the effect.
        barrel_leds[i_barrel_light - 1] = getHueColour(c_temp, WAND_BARREL_LED_COUNT);
      break;
    }

    i_barrel_light = 0;
    ms_firing_effect_end.stop();
    ms_firing_lights_end.start(0);
  }
}

void fireStreamEnd(CRGB c_colour) {
  if(i_barrel_light < i_num_barrel_leds) {
    switch(WAND_BARREL_LED_COUNT) {
      case LEDS_48:
        // Set the colour for the mapped LED.
        barrel_leds[PROGMEM_READU8(frutto_barrel[i_barrel_light])] = c_colour;

        // More LEDs means a faster firing rate.
        ms_firing_lights_end.start(d_firing_stream / 25);
      break;

      case LEDS_5:
      default:
        // Set the colour for the specific LED.
        barrel_leds[i_barrel_light] = c_colour;

        // Firing at a "normal" rate
        ms_firing_lights_end.start(d_firing_stream / 5);
      break;
    }

    i_barrel_light++;

    if(i_barrel_light == i_num_barrel_leds) {
      i_barrel_light = 0;

      if(WAND_BARREL_LED_COUNT != LEDS_5) {
        // Turn off wand tip in case it's still on.
        wandTipOff();
      }

      ms_firing_lights_end.stop();

      i_fast_led_delay = FAST_LED_UPDATE_MS;
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
        vibrationOff();
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
    vibrationOff();
  }
}

// This is the Super Hero bargraph firing animation. Ramping up and down from the middle to the top/bottom and back to the middle again.
void bargraphSuperHeroRampFiringAnimation() {
  if(b_28segment_bargraph == true) {
    switch(i_bargraph_status_alt) {
      case 0:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLed(bargraphLookupTable(13));
        ht_bargraph.setLed(bargraphLookupTable(14));

        b_bargraph_status[13] = true;
        b_bargraph_status[14] = true;

        i_bargraph_status_alt++;

        if(b_bargraph_up == false) {
          ht_bargraph.clearLed(bargraphLookupTable(12));
          ht_bargraph.clearLed(bargraphLookupTable(15));

          b_bargraph_status[12] = false;
          b_bargraph_status[15] = false;
        }

        b_bargraph_up = true;

        ht_bargraph.sendLed(); // Commit the changes.
        wandTipOn();
      break;

      case 1:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLed(bargraphLookupTable(12));
        ht_bargraph.setLed(bargraphLookupTable(15));

        b_bargraph_status[12] = true;
        b_bargraph_status[15] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLed(bargraphLookupTable(13));
          ht_bargraph.clearLed(bargraphLookupTable(14));

          b_bargraph_status[13] = false;
          b_bargraph_status[14] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLed(bargraphLookupTable(11));
          ht_bargraph.clearLed(bargraphLookupTable(16));

          b_bargraph_status[11] = false;
          b_bargraph_status[16] = false;

          i_bargraph_status_alt--;
        }

        ht_bargraph.sendLed(); // Commit the changes.
        wandTipOn();
      break;

      case 2:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLed(bargraphLookupTable(11));
        ht_bargraph.setLed(bargraphLookupTable(16));

        b_bargraph_status[11] = true;
        b_bargraph_status[16] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLed(bargraphLookupTable(12));
          ht_bargraph.clearLed(bargraphLookupTable(15));

          b_bargraph_status[12] = false;
          b_bargraph_status[15] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLed(bargraphLookupTable(10));
          ht_bargraph.clearLed(bargraphLookupTable(17));

          b_bargraph_status[10] = false;
          b_bargraph_status[17] = false;

          i_bargraph_status_alt--;
        }

        ht_bargraph.sendLed(); // Commit the changes.
        wandTipOff();
      break;

      case 3:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLed(bargraphLookupTable(10));
        ht_bargraph.setLed(bargraphLookupTable(17));

        b_bargraph_status[10] = true;
        b_bargraph_status[17] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLed(bargraphLookupTable(11));
          ht_bargraph.clearLed(bargraphLookupTable(16));

          b_bargraph_status[11] = false;
          b_bargraph_status[16] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLed(bargraphLookupTable(9));
          ht_bargraph.clearLed(bargraphLookupTable(18));

          b_bargraph_status[9] = false;
          b_bargraph_status[18] = false;

          i_bargraph_status_alt--;
        }

        ht_bargraph.sendLed(); // Commit the changes.
        wandTipOff();
      break;

      case 4:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLed(bargraphLookupTable(9));
        ht_bargraph.setLed(bargraphLookupTable(18));

        b_bargraph_status[9] = true;
        b_bargraph_status[18] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLed(bargraphLookupTable(10));
          ht_bargraph.clearLed(bargraphLookupTable(17));

          b_bargraph_status[10] = false;
          b_bargraph_status[17] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLed(bargraphLookupTable(8));
          ht_bargraph.clearLed(bargraphLookupTable(19));

          b_bargraph_status[8] = false;
          b_bargraph_status[19] = false;

          i_bargraph_status_alt--;
        }

        ht_bargraph.sendLed(); // Commit the changes.
        wandTipOn();
      break;

      case 5:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLed(bargraphLookupTable(8));
        ht_bargraph.setLed(bargraphLookupTable(19));

        b_bargraph_status[8] = true;
        b_bargraph_status[19] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLed(bargraphLookupTable(9));
          ht_bargraph.clearLed(bargraphLookupTable(18));

          b_bargraph_status[9] = false;
          b_bargraph_status[18] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLed(bargraphLookupTable(7));
          ht_bargraph.clearLed(bargraphLookupTable(20));

          b_bargraph_status[7] = false;
          b_bargraph_status[20] = false;

          i_bargraph_status_alt--;
        }

        ht_bargraph.sendLed(); // Commit the changes.
        wandTipOn();
      break;

      case 6:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLed(bargraphLookupTable(7));
        ht_bargraph.setLed(bargraphLookupTable(20));

        b_bargraph_status[7] = true;
        b_bargraph_status[20] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLed(bargraphLookupTable(8));
          ht_bargraph.clearLed(bargraphLookupTable(19));

          b_bargraph_status[8] = false;
          b_bargraph_status[19] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLed(bargraphLookupTable(6));
          ht_bargraph.clearLed(bargraphLookupTable(21));

          b_bargraph_status[6] = false;
          b_bargraph_status[21] = false;

          i_bargraph_status_alt--;
        }

        ht_bargraph.sendLed(); // Commit the changes.
        wandTipOff();
      break;

      case 7:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLed(bargraphLookupTable(6));
        ht_bargraph.setLed(bargraphLookupTable(21));

        b_bargraph_status[6] = true;
        b_bargraph_status[21] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLed(bargraphLookupTable(7));
          ht_bargraph.clearLed(bargraphLookupTable(20));

          b_bargraph_status[7] = false;
          b_bargraph_status[20] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLed(bargraphLookupTable(5));
          ht_bargraph.clearLed(bargraphLookupTable(22));

          b_bargraph_status[5] = false;
          b_bargraph_status[22] = false;

          i_bargraph_status_alt--;
        }

        ht_bargraph.sendLed(); // Commit the changes.
        wandTipOff();
      break;

      case 8:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLed(bargraphLookupTable(5));
        ht_bargraph.setLed(bargraphLookupTable(22));

        b_bargraph_status[5] = true;
        b_bargraph_status[22] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLed(bargraphLookupTable(6));
          ht_bargraph.clearLed(bargraphLookupTable(21));

          b_bargraph_status[6] = false;
          b_bargraph_status[21] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLed(bargraphLookupTable(4));
          ht_bargraph.clearLed(bargraphLookupTable(23));

          b_bargraph_status[4] = false;
          b_bargraph_status[23] = false;

          i_bargraph_status_alt--;
        }

        ht_bargraph.sendLed(); // Commit the changes.
        wandTipOn();
      break;

      case 9:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLed(bargraphLookupTable(4));
        ht_bargraph.setLed(bargraphLookupTable(23));

        b_bargraph_status[4] = true;
        b_bargraph_status[23] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLed(bargraphLookupTable(5));
          ht_bargraph.clearLed(bargraphLookupTable(22));

          b_bargraph_status[5] = false;
          b_bargraph_status[22] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLed(bargraphLookupTable(3));
          ht_bargraph.clearLed(bargraphLookupTable(24));

          b_bargraph_status[3] = false;
          b_bargraph_status[24] = false;

          i_bargraph_status_alt--;
        }

        ht_bargraph.sendLed(); // Commit the changes.
        wandTipOn();
      break;

      case 10:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLed(bargraphLookupTable(3));
        ht_bargraph.setLed(bargraphLookupTable(24));

        b_bargraph_status[3] = true;
        b_bargraph_status[24] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLed(bargraphLookupTable(4));
          ht_bargraph.clearLed(bargraphLookupTable(23));

          b_bargraph_status[4] = false;
          b_bargraph_status[23] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLed(bargraphLookupTable(2));
          ht_bargraph.clearLed(bargraphLookupTable(25));

          b_bargraph_status[2] = false;
          b_bargraph_status[25] = false;

          i_bargraph_status_alt--;
        }

        ht_bargraph.sendLed(); // Commit the changes.
        wandTipOff();
      break;

      case 11:
        vibrationWand(i_vibration_level + 115);

        ht_bargraph.setLed(bargraphLookupTable(2));
        ht_bargraph.setLed(bargraphLookupTable(25));

        b_bargraph_status[2] = false;
        b_bargraph_status[25] = false;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLed(bargraphLookupTable(3));
          ht_bargraph.clearLed(bargraphLookupTable(24));

          b_bargraph_status[3] = false;
          b_bargraph_status[24] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLed(bargraphLookupTable(1));
          ht_bargraph.clearLed(bargraphLookupTable(26));

          b_bargraph_status[1] = false;
          b_bargraph_status[26] = false;

          i_bargraph_status_alt--;
        }

        ht_bargraph.sendLed(); // Commit the changes.
        wandTipOff();
      break;

      case 12:
        vibrationWand(i_vibration_level + 115);

        ht_bargraph.setLed(bargraphLookupTable(1));
        ht_bargraph.setLed(bargraphLookupTable(26));

        b_bargraph_status[1] = true;
        b_bargraph_status[26] = true;

        if(b_bargraph_up == true) {
          ht_bargraph.clearLed(bargraphLookupTable(2));
          ht_bargraph.clearLed(bargraphLookupTable(25));

          b_bargraph_status[2] = false;
          b_bargraph_status[25] = false;

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLed(bargraphLookupTable(0));
          ht_bargraph.clearLed(bargraphLookupTable(27));

          b_bargraph_status[0] = false;
          b_bargraph_status[27] = false;

          i_bargraph_status_alt--;
        }

        ht_bargraph.sendLed(); // Commit the changes.
        wandTipOn();
      break;

      case 13:
        vibrationWand(i_vibration_level + 115);

        ht_bargraph.setLed(bargraphLookupTable(0));
        ht_bargraph.setLed(bargraphLookupTable(27));

        b_bargraph_status[0] = true;
        b_bargraph_status[27] = true;

        ht_bargraph.clearLed(bargraphLookupTable(1));
        ht_bargraph.clearLed(bargraphLookupTable(26));

        b_bargraph_status[1] = false;
        b_bargraph_status[26] = false;

        i_bargraph_status_alt--;

        b_bargraph_up = false;

        ht_bargraph.sendLed(); // Commit the changes.
        wandTipOn();
      break;
    }
  }
  else {
    // Hasbro 5 LED Bargraph.
    switch(i_bargraph_status) {
      case 1:
        vibrationWand(i_vibration_level + 110);

        digitalWriteFast(bargraphLookupTable(1-1), LOW);
        digitalWriteFast(bargraphLookupTable(2-1), HIGH);
        digitalWriteFast(bargraphLookupTable(3-1), HIGH);
        digitalWriteFast(bargraphLookupTable(4-1), HIGH);
        digitalWriteFast(bargraphLookupTable(5-1), LOW);
        i_bargraph_status++;

        wandTipOn();
      break;

      case 2:
        vibrationWand(i_vibration_level + 112);

        digitalWriteFast(bargraphLookupTable(1-1), HIGH);
        digitalWriteFast(bargraphLookupTable(2-1), LOW);
        digitalWriteFast(bargraphLookupTable(3-1), HIGH);
        digitalWriteFast(bargraphLookupTable(4-1), LOW);
        digitalWriteFast(bargraphLookupTable(5-1), HIGH);
        i_bargraph_status++;

        wandTipOff();
      break;

      case 3:
        vibrationWand(i_vibration_level + 115);

        digitalWriteFast(bargraphLookupTable(1-1), HIGH);
        digitalWriteFast(bargraphLookupTable(2-1), HIGH);
        digitalWriteFast(bargraphLookupTable(3-1), LOW);
        digitalWriteFast(bargraphLookupTable(4-1), HIGH);
        digitalWriteFast(bargraphLookupTable(5-1), HIGH);
        i_bargraph_status++;

        wandTipOn();
      break;

      case 4:
        vibrationWand(i_vibration_level + 112);

        digitalWriteFast(bargraphLookupTable(1-1), HIGH);
        digitalWriteFast(bargraphLookupTable(2-1), LOW);
        digitalWriteFast(bargraphLookupTable(3-1), HIGH);
        digitalWriteFast(bargraphLookupTable(4-1), LOW);
        digitalWriteFast(bargraphLookupTable(5-1), HIGH);
        i_bargraph_status++;

        wandTipOff();
      break;

      case 5:
        vibrationWand(i_vibration_level + 110);

        digitalWriteFast(bargraphLookupTable(1-1), LOW);
        digitalWriteFast(bargraphLookupTable(2-1), HIGH);
        digitalWriteFast(bargraphLookupTable(3-1), HIGH);
        digitalWriteFast(bargraphLookupTable(4-1), HIGH);
        digitalWriteFast(bargraphLookupTable(5-1), LOW);
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
      switch(i_power_level) {
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

    switch(i_power_level) {
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
                default:
                  i_bargraph_status_alt = random(18, i_bargraph_segments - 1);
                break;
              }
            }

            if(b_bargraph_status[i] == true) {
              ht_bargraph.clearLed(bargraphLookupTable(i));
              b_bargraph_status[i] = false;

              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
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
              ht_bargraph.setLed(bargraphLookupTable(i));
              b_bargraph_status[i] = true;

              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
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
              ht_bargraph.clearLed(bargraphLookupTable(i));
              b_bargraph_status[i] = false;

              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
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
              ht_bargraph.setLed(bargraphLookupTable(i));
              b_bargraph_status[i] = true;

              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
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
              ht_bargraph.clearLed(bargraphLookupTable(i));
              b_bargraph_status[i] = false;

              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
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
              ht_bargraph.setLed(bargraphLookupTable(i));
              b_bargraph_status[i] = true;

              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
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
              ht_bargraph.clearLed(bargraphLookupTable(i));
              b_bargraph_status[i] = false;

              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
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
              ht_bargraph.setLed(bargraphLookupTable(i));
              b_bargraph_status[i] = true;

              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
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
              ht_bargraph.clearLed(bargraphLookupTable(i));
              b_bargraph_status[i] = false;

              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
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
              ht_bargraph.setLed(bargraphLookupTable(i));
              b_bargraph_status[i] = true;

              break;
            }
          }

          ht_bargraph.sendLed(); // Commit the changes.
        }
      break;
    }

    if(i_bargraph_status_alt > 22) {
      vibrationWand(i_vibration_level + 115);
    }
    else if(i_bargraph_status_alt > 11) {
      vibrationWand(i_vibration_level + 112);
    }
    else {
      vibrationWand(i_vibration_level + 110);
    }
  }
  else {
    // When firing starts, i_bargraph_status resets to 0 in modeFireStart();
    if(i_bargraph_status == 0) {
      // Set our target.
      switch(i_power_level) {
        case 5:
          i_bargraph_status = random(2, i_bargraph_segments_5_led + 1);
        break;

        case 4:
          i_bargraph_status = random(2, i_bargraph_segments_5_led);
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

    switch(i_power_level) {
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

  if(i_bargraph_status > 3) {
    vibrationWand(i_vibration_level + 115);
  }
  else if(i_bargraph_status > 1) {
    vibrationWand(i_vibration_level + 112);
  }
  else {
    vibrationWand(i_vibration_level + 110);
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

      // Strobe the optional tip light on even barrel LED numbers.
      if((i_barrel_light & 0x01) == 0) {
        wandTipOn();
      }
      else {
        wandTipOff();
      }
    break;
  }

  uint8_t i_ramp_interval = d_bargraph_ramp_interval;

  if(b_28segment_bargraph == true) {
    // Switch to a different ramp speed if using the (Optional) 28 segment barmeter bargraph.
    i_ramp_interval = d_bargraph_ramp_interval_alt;
  }

  // If in a power level on the wand that can overheat, change the speed of the bargraph ramp during firing based on time remaining before we overheat.
  if(b_overheat_level[i_power_level - 1] == true && ms_overheat_initiate.isRunning() && b_overheat_enabled == true) {
    if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_level - 1] / 6) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start((i_ramp_interval / 8) + 2); // 7ms per segment
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 5); // 24ms per LED
      }

      cyclotronSpeedUp(6);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_level - 1] / 5) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start((i_ramp_interval / 8) + 4); // 9ms per segment
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 4); // 30ms per LED
      }

      cyclotronSpeedUp(5);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_level - 1] / 4) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start((i_ramp_interval / 4) + 1); // 11ms per segment
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 3); // 40ms per LED
      }

      cyclotronSpeedUp(4);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_level - 1] / 3) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start((i_ramp_interval / 4) + 3); // 13ms per segment
      }
      else {
        ms_bargraph_firing.start((i_ramp_interval / 2) - 10); // 50ms per LED
      }

      cyclotronSpeedUp(3);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_level - 1] / 2) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start((i_ramp_interval / 4) + 5); // 15ms per segment
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 2); // 60ms per LED
      }

      cyclotronSpeedUp(2);
    }
    else {
      if(b_28segment_bargraph == true) {
        switch(i_power_level) {
          case 5:
            ms_bargraph_firing.start((i_ramp_interval / 2) - 5); // 15ms per segment
          break;

          case 4:
            ms_bargraph_firing.start(i_ramp_interval / 2); // 20ms per segment
          break;

          case 3:
            ms_bargraph_firing.start((i_ramp_interval / 2) + 5); // 25ms per segment
          break;

          case 2:
            ms_bargraph_firing.start((i_ramp_interval / 2) + 10); // 30ms per segment
          break;

          case 1:
          default:
            ms_bargraph_firing.start((i_ramp_interval / 2) + 15); // 35ms per segment
          break;
        }
      }
      else {
        if(BARGRAPH_FIRING_ANIMATION == BARGRAPH_ANIMATION_ORIGINAL) {
          switch(i_power_level) {
            case 5:
              ms_bargraph_firing.start(i_ramp_interval / 2); // 60ms per LED
            break;

            case 4:
              ms_bargraph_firing.start((i_ramp_interval / 2) + 30); // 90ms per LED
            break;

            case 3:
              ms_bargraph_firing.start(i_ramp_interval); // 120ms per LED
            break;

            case 2:
              ms_bargraph_firing.start(i_ramp_interval * 2); // 240ms per LED
            break;

            case 1:
            default:
              ms_bargraph_firing.start(i_ramp_interval * 3); // 360ms per LED
            break;
          }
        }
        else {
          ms_bargraph_firing.start(i_ramp_interval / 2); // 60ms per LED
        }
      }

      i_cyclotron_speed_up = 1;
    }
  }
  else {
    if(b_28segment_bargraph == true) {
      switch(i_power_level) {
        case 5:
          ms_bargraph_firing.start((i_ramp_interval / 2) - 7); // 13ms per segment
        break;

        case 4:
          ms_bargraph_firing.start((i_ramp_interval / 2) - 3); // 15ms per segment
        break;

        case 3:
          ms_bargraph_firing.start(i_ramp_interval / 2); // 20ms per segment
        break;

        case 2:
          ms_bargraph_firing.start((i_ramp_interval / 2) + 7); // 25ms per segment
        break;

        case 1:
        default:
          ms_bargraph_firing.start((i_ramp_interval / 2) + 12); // 30ms per segment
        break;
      }
    }
    else {
      if(BARGRAPH_FIRING_ANIMATION == BARGRAPH_ANIMATION_ORIGINAL) {
        switch(i_power_level) {
          case 5:
            ms_bargraph_firing.start(i_ramp_interval / 2); // 60ms per LED
          break;

          case 4:
            ms_bargraph_firing.start((i_ramp_interval / 2) + 30); // 90ms per LED
          break;

          case 3:
            ms_bargraph_firing.start(i_ramp_interval); // 120ms per LED
          break;

          case 2:
            ms_bargraph_firing.start(i_ramp_interval * 2); // 240ms per LED
          break;

          case 1:
          default:
            ms_bargraph_firing.start(i_ramp_interval * 3); // 360ms per LED
          break;
        }
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 2); // 60ms per LED
      }
    }
  }
}

void cyclotronSpeedUp(uint8_t i_switch) {
  if(i_switch != i_cyclotron_speed_up) {
    if(i_switch == 4) {
      // Tell pack to start beeping before we overheat it.
      wandSerialSend(W_BEEP_START);

      // Play overheat alert beeps before we overheat.
      switch(getSystemYearMode()) {
        case SYSTEM_AFTERLIFE:
        case SYSTEM_FROZEN_EMPIRE:
        default:
          playEffect(S_PACK_BEEPS_OVERHEAT, true);
        break;

        case SYSTEM_1984:
        case SYSTEM_1989:
          playEffect(S_BEEP_8, true);
        break;
      }

      ms_hat_1.start(i_hat_1_delay);
    }

    i_cyclotron_speed_up++;

    // Tell the pack to speed up the Cyclotron.
    wandSerialSend(W_CYCLOTRON_INCREASE_SPEED);
  }
}

void stopOverheatBeepWarnings() {
  // Stop overheat beeps.
  switch(getSystemYearMode()) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      stopEffect(S_PACK_BEEPS_OVERHEAT);
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      stopEffect(S_BEEP_8);
    break;
  }
}

void cyclotronSpeedRevert() {
  // Attenuator told us to reset, so stop beeps.
  stopOverheatBeepWarnings();

  i_cyclotron_speed_up = 1;
}

// 2021 mode for optional 28 segment bargraph.
// Checks if we ramp up or down when changing power levels.
// Forces the bargraph to redraw itself to the current power level.
void bargraphPowerCheck2021Alt(bool b_override) {
  if((WAND_ACTION_STATUS != ACTION_FIRING && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) || b_override == true) {
    if(i_power_level != i_power_level_prev || b_override == true) {
      if(i_power_level > i_power_level_prev) {
        b_bargraph_up = true;
      }
      else {
        b_bargraph_up = false;
      }

      switch(i_power_level) {
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
        default:
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

// This function handles returning all bargraph lookup table values.
uint8_t bargraphLookupTable(uint8_t index) {
  if(b_28segment_bargraph) {
    if(b_bargraph_invert) {
      return PROGMEM_READU8(i_bargraph_invert[index]);
    }
    else {
      return PROGMEM_READU8(i_bargraph_normal[index]);
    }
  }
  else {
    if(b_bargraph_invert) {
      return PROGMEM_READU8(i_bargraph_5_led_invert[index]);
    }
    else {
      return PROGMEM_READU8(i_bargraph_5_led_normal[index]);
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

    switch(i_power_level) {
      case 1:
      default:
        for(uint8_t i = 0; i < i_bargraph_segments; i++) {
          if(i <= 4) {
            ht_bargraph.setLed(bargraphLookupTable(i));
            b_bargraph_status[i] = true;
          }
          else {
            ht_bargraph.clearLed(bargraphLookupTable(i));
            b_bargraph_status[i] = false;
          }
        }

        ht_bargraph.sendLed(); // Commit the changes.
        i_bargraph_status_alt = 4;
      break;

      case 2:
        for(uint8_t i = 0; i < i_bargraph_segments; i++) {
          if(i <= 11) {
            ht_bargraph.setLed(bargraphLookupTable(i));
            b_bargraph_status[i] = true;
          }
          else {
            ht_bargraph.clearLed(bargraphLookupTable(i));
            b_bargraph_status[i] = false;
          }
        }

        ht_bargraph.sendLed(); // Commit the changes.
        i_bargraph_status_alt = 11;
      break;

      case 3:
        for(uint8_t i = 0; i < i_bargraph_segments; i++) {
          if(i <= 16) {
            ht_bargraph.setLed(bargraphLookupTable(i));
            b_bargraph_status[i] = true;
          }
          else {
            ht_bargraph.clearLed(bargraphLookupTable(i));
            b_bargraph_status[i] = false;
          }
        }

        ht_bargraph.sendLed(); // Commit the changes.
        i_bargraph_status_alt = 16;
      break;

      case 4:
        for(uint8_t i = 0; i < i_bargraph_segments; i++) {
          if(i <= 22) {
            ht_bargraph.setLed(bargraphLookupTable(i));
            b_bargraph_status[i] = true;
          }
          else {
            ht_bargraph.clearLed(bargraphLookupTable(i));
            b_bargraph_status[i] = false;
          }
        }

        ht_bargraph.sendLed(); // Commit the changes.
        i_bargraph_status_alt = 22;
      break;

      case 5:
        for(uint8_t i = 0; i < i_bargraph_segments; i++) {
          ht_bargraph.setLed(bargraphLookupTable(i));
          b_bargraph_status[i] = true;
        }

        ht_bargraph.sendLed(); // Commit the changes.
        i_bargraph_status_alt = 27;
      break;
    }
  }
  else {
    // Stock haslab bargraph control.
    switch(i_power_level) {
      case 1:
      default:
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
          ht_bargraph.setLedNow(bargraphLookupTable(i_bargraph_status_alt));
          b_bargraph_status[i_bargraph_status_alt] = true;
        }

        switch(i_power_level) {
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
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_level - 1]);
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
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_level - 1]);
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
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_level - 1]);
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
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_level - 1]);
            }
          break;

          case 1:
          default:
            if(i_bargraph_status_alt > 3) {
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
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_level - 1]);
            }
          break;
        }

        if(b_bargraph_up == true) {
          i_bargraph_status_alt++;
        }
      }
      else {
        if(i_bargraph_status_alt < i_bargraph_segments) {
          ht_bargraph.clearLedNow(bargraphLookupTable(i_bargraph_status_alt));
          b_bargraph_status[i_bargraph_status_alt] = false;
        }

        if(i_bargraph_status_alt == 0) {
          b_bargraph_up = true;

          // A little pause when we reach the bottom.
          ms_bargraph_alt.start(i_bargraph_wait / 2);
        }
        else {
          i_bargraph_status_alt--;

          switch(i_power_level) {
            case 5:
              if(BARGRAPH_MODE == BARGRAPH_ORIGINAL && i_bargraph_status_alt < i_bargraph_segments) {
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
            default:
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
    switch(i_power_level) {
      case 1:
      default:
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

// Fully lights up the bargraph.
void bargraphFull() {
  if(b_28segment_bargraph == true) {
    for(uint8_t i = 0; i < i_bargraph_segments; i++) {
      ht_bargraph.setLed(bargraphLookupTable(i));
      b_bargraph_status[i] = true;
    }

    ht_bargraph.sendLed(); // Commit the changes.
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
      5: full: 23 - 27 (5 segments)
      4: 3/4: 17 - 22  (6 segments)
      3: 1/2: 12 - 16  (5 segments)
      2: 1/4: 5 - 11   (7 segments)
      1: none: 0 - 4   (5 segments)
    */

    switch(i_bargraph_status_alt) {
      case 0 ... 27:
        ht_bargraph.setLedNow(bargraphLookupTable(i_bargraph_status_alt));
        b_bargraph_status[i_bargraph_status_alt] = true;

        if(i_bargraph_status_alt > 22) {
          vibrationWand(i_vibration_level + 80);
        }
        else if(i_bargraph_status_alt > 16) {
          vibrationWand(i_vibration_level + 40);
        }
        else if(i_bargraph_status_alt > 11) {
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

            ht_bargraph.clearLedNow(bargraphLookupTable(i_tmp));
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
          if((i_power_level < 5 && BARGRAPH_MODE == BARGRAPH_ORIGINAL) || BARGRAPH_MODE == BARGRAPH_SUPER_HERO) {
            ht_bargraph.clearLedNow(bargraphLookupTable(i_tmp));
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
              switch(i_power_level) {
                case 5:
                  if(i_bargraph_status_alt == 55) {
                    ms_bargraph_alt.stop();
                    ms_bargraph.stop();
                    b_bargraph_up = false;
                    i_bargraph_status_alt = 27;
                    bargraphYearModeUpdate();

                    vibrationWand(i_vibration_level + 25);
                  }
                  else {
                    ms_bargraph.stop();
                    b_bargraph_up = true;
                    i_bargraph_status_alt = 55 - i_bargraph_status_alt;
                    bargraphYearModeUpdate();

                    vibrationWand(i_vibration_level + 25);
                  }
                break;

                case 4:
                  if(i_bargraph_status_alt == 32) {
                    ms_bargraph.stop();
                    b_bargraph_up = false;
                    i_bargraph_status_alt = 22;
                    bargraphYearModeUpdate();

                    vibrationWand(i_vibration_level + 30);
                  }
                  else if(i_bargraph_status_alt > 32) {
                    ms_bargraph.stop();
                    b_bargraph_up = true;
                    i_bargraph_status_alt = 55 - i_bargraph_status_alt;
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
                  else if(i_bargraph_status_alt > 38) {
                    ms_bargraph.stop();
                    b_bargraph_up = true;
                    i_bargraph_status_alt = 55 - i_bargraph_status_alt;
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
                  else if(i_bargraph_status_alt > 43) {
                    ms_bargraph.stop();
                    b_bargraph_up = true;
                    i_bargraph_status_alt = 55 - i_bargraph_status_alt;
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
                default:
                  vibrationWand(i_vibration_level);

                  if(i_bargraph_status_alt == 50) {
                    ms_bargraph.stop();
                    b_bargraph_up = false;
                    i_bargraph_status_alt = 4;
                    bargraphYearModeUpdate();
                  }
                  else if(i_bargraph_status_alt > 50) {
                    ms_bargraph.stop();
                    b_bargraph_up = true;
                    i_bargraph_status_alt = 55 - i_bargraph_status_alt;
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

        if(i_bargraph_status + 1 == i_power_level && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
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

        if(i_bargraph_status - 1 == i_power_level && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
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

        if(i_bargraph_status - 3 == i_power_level && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
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

        if(i_bargraph_status - 5 == i_power_level && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
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

        if(i_bargraph_status - 7 == i_power_level && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
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
    // If bargraph is set to ramp down during ribbon cable error, we need to set a few things.
    soundBeepLoopStop();
    soundIdleStop();
    soundIdleLoopStop(true);

    // Reset some bargraph levels before we ramp the bargraph down.
    i_bargraph_status_alt = i_bargraph_segments; // For 28 segment bargraph
    i_bargraph_status = i_bargraph_segments_5_led; // For Hasbro 5 LED bargraph.

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
    digitalWriteFast(bargraphLookupTable(5-1), LOW);
    b_bargraph_status_5[4] = true;
  }
  else {
    // Off
    digitalWriteFast(bargraphLookupTable(5-1), HIGH);
    b_bargraph_status_5[4] = false;
  }

  if(i_t_level > 3) {
    digitalWriteFast(bargraphLookupTable(4-1), LOW);
    b_bargraph_status_5[3] = true;
  }
  else {
    digitalWriteFast(bargraphLookupTable(4-1), HIGH);
    b_bargraph_status_5[3] = false;
  }

  if(i_t_level > 2) {
    digitalWriteFast(bargraphLookupTable(3-1), LOW);
    b_bargraph_status_5[2] = true;
  }
  else {
    digitalWriteFast(bargraphLookupTable(3-1), HIGH);
    b_bargraph_status_5[2] = false;
  }

  if(i_t_level > 1) {
    digitalWriteFast(bargraphLookupTable(2-1), LOW);
    b_bargraph_status_5[1] = true;
  }
  else {
    digitalWriteFast(bargraphLookupTable(2-1), HIGH);
    b_bargraph_status_5[1] = false;
  }

  if(i_t_level > 0) {
    digitalWriteFast(bargraphLookupTable(1-1), LOW);
    b_bargraph_status_5[0] = true;
  }
  else {
    digitalWriteFast(bargraphLookupTable(1-1), HIGH);
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

  digitalWriteFast(led_slo_blo, LOW);
  digitalWriteFast(led_front_left, LOW); // Turn off the front left LED under the Clippard valve.

  digitalWriteFast(led_hat_1, LOW); // Turn off hat light 1.
  digitalWriteFast(led_hat_2, LOW); // Turn off hat light 2.
  wandTipOff();

  digitalWrite(led_vent, HIGH);
  digitalWriteFast(led_white, HIGH);

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
  digitalWriteFast(led_slo_blo, LOW);
  digitalWrite(led_vent, HIGH);
  digitalWriteFast(led_white, HIGH);
  digitalWriteFast(led_front_left, LOW);

  if(b_power_on_indicator == true) {
    ms_power_indicator.stop();
    ms_power_indicator_blink.stop();
  }
}

void vibrationOff() {
  ms_menu_vibration.stop();
  i_vibration_level_prev = 0;
  b_menu_vibration_active = false;
  analogWrite(vibration, 0);
}

int8_t readRotary() {
  static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

  prev_next_code <<= 2;

  if(digitalReadFast(r_encoderB)) {
    prev_next_code |= 0x02;
  }

  if(digitalReadFast(r_encoderA)) {
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
}

// It is very important that S_1 up to S_60 follow each other in order on the Micro SD Card and sound effects enum.
void overheatVoiceIndicator(uint16_t i_tmp_length) {
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
      if(i_ms_overheat_initiate_level_5 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheat_initiate_level_5 = i_ms_overheat_initiate_level_5 + i_overheat_delay_increment;
        i_ms_overheat_initiate[4] = i_ms_overheat_initiate_level_5;

        overheatVoiceIndicator(i_ms_overheat_initiate_level_5);
      }
    break;

    case 4:
      if(i_ms_overheat_initiate_level_4 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheat_initiate_level_4 = i_ms_overheat_initiate_level_4 + i_overheat_delay_increment;
        i_ms_overheat_initiate[3] = i_ms_overheat_initiate_level_4;

        overheatVoiceIndicator(i_ms_overheat_initiate_level_4);
      }
    break;

    case 3:
      if(i_ms_overheat_initiate_level_3 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheat_initiate_level_3 = i_ms_overheat_initiate_level_3 + i_overheat_delay_increment;
        i_ms_overheat_initiate[2] = i_ms_overheat_initiate_level_3;

        overheatVoiceIndicator(i_ms_overheat_initiate_level_3);
      }
    break;

    case 2:
      if(i_ms_overheat_initiate_level_2 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheat_initiate_level_2 = i_ms_overheat_initiate_level_2 + i_overheat_delay_increment;
        i_ms_overheat_initiate[1] = i_ms_overheat_initiate_level_2;

        overheatVoiceIndicator(i_ms_overheat_initiate_level_2);
      }
    break;

    case 1:
    default:
      if(i_ms_overheat_initiate_level_1 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheat_initiate_level_1 = i_ms_overheat_initiate_level_1 + i_overheat_delay_increment;
        i_ms_overheat_initiate[0] = i_ms_overheat_initiate_level_1;

        overheatVoiceIndicator(i_ms_overheat_initiate_level_1);
      }
    break;
  }
}

void overheatTimerDecrement(uint8_t i_tmp_power_level) {
  switch(i_tmp_power_level) {
    case 5:
      if(i_ms_overheat_initiate_level_5 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheat_initiate_level_5 = i_ms_overheat_initiate_level_5 - i_overheat_delay_increment;
        i_ms_overheat_initiate[4] = i_ms_overheat_initiate_level_5;

        overheatVoiceIndicator(i_ms_overheat_initiate_level_5);
      }
    break;

    case 4:
      if(i_ms_overheat_initiate_level_4 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheat_initiate_level_4 = i_ms_overheat_initiate_level_4 - i_overheat_delay_increment;
        i_ms_overheat_initiate[3] = i_ms_overheat_initiate_level_4;

        overheatVoiceIndicator(i_ms_overheat_initiate_level_4);
      }
    break;

    case 3:
      if(i_ms_overheat_initiate_level_3 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheat_initiate_level_3 = i_ms_overheat_initiate_level_3 - i_overheat_delay_increment;
        i_ms_overheat_initiate[2] = i_ms_overheat_initiate_level_3;

        overheatVoiceIndicator(i_ms_overheat_initiate_level_3);
      }
    break;

    case 2:
      if(i_ms_overheat_initiate_level_2 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheat_initiate_level_2 = i_ms_overheat_initiate_level_2 - i_overheat_delay_increment;
        i_ms_overheat_initiate[1] = i_ms_overheat_initiate_level_2;

        overheatVoiceIndicator(i_ms_overheat_initiate_level_2);
      }
    break;

    case 1:
    default:
      if(i_ms_overheat_initiate_level_1 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheat_initiate_level_1 = i_ms_overheat_initiate_level_1 - i_overheat_delay_increment;
        i_ms_overheat_initiate[0] = i_ms_overheat_initiate_level_1;

        overheatVoiceIndicator(i_ms_overheat_initiate_level_1);
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
          if(WAND_MENU_LEVEL == MENU_LEVEL_3 && i_wand_menu == 5 && switch_intensify.on() == true && switch_mode.on() == false) {
            // Adjust the default bootup system volume.
            wandSerialSend(W_VOLUME_DECREASE_EEPROM);

            // If there is no Pack, we need to adjust the volume manually
            if(b_gpstar_benchtest == true) {
              decreaseVolumeEEPROM();
            }
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 5 && switch_intensify.on() == true && switch_mode.on() == false) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_5);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 4 && switch_intensify.on() == true && switch_mode.on() == false) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_4);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 3 && switch_intensify.on() == true && switch_mode.on() == false) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_3);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 2 && switch_intensify.on() == true && switch_mode.on() == false) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_2);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 1 && switch_intensify.on() == true && switch_mode.on() == false) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_1);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 5 && switch_intensify.on() == false && switch_mode.on() == true) {
            overheatTimerDecrement(5);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 4 && switch_intensify.on() == false && switch_mode.on() == true) {
            overheatTimerDecrement(4);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 3 && switch_intensify.on() == false && switch_mode.on() == true) {
            overheatTimerDecrement(3);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 2 && switch_intensify.on() == false && switch_mode.on() == true) {
            overheatTimerDecrement(2);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 1 && switch_intensify.on() == false && switch_mode.on() == true) {
            overheatTimerDecrement(1);
          }
          else if(i_wand_menu - 1 < 1) {
            switch(WAND_MENU_LEVEL) {
              case MENU_LEVEL_1:
                WAND_MENU_LEVEL = MENU_LEVEL_2;
                i_wand_menu = 5;

                // Turn on some lights to visually indicate which menu we are in.
                digitalWriteFast(led_slo_blo, HIGH); // Level 2

                // Turn off the other lights.
                digitalWrite(led_vent, HIGH); // Level 3
                digitalWriteFast(led_white, HIGH); // Level 4
                digitalWriteFast(led_front_left, LOW); // Level 5

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
                digitalWriteFast(led_slo_blo, HIGH); // Level 2
                digitalWrite(led_vent, LOW); // Level 3

                // Turn off the other lights.
                digitalWriteFast(led_white, HIGH); // Level 4
                digitalWriteFast(led_front_left, LOW); // Level 5

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
                digitalWriteFast(led_slo_blo, HIGH); // Level 2
                digitalWrite(led_vent, LOW); // Level 3
                digitalWriteFast(led_white, LOW); // Level 4

                // Turn off the other lights.
                digitalWriteFast(led_front_left, LOW); // Level 5

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
                digitalWriteFast(led_slo_blo, HIGH); // Level 2
                digitalWrite(led_vent, LOW); // Level 3
                digitalWriteFast(led_white, LOW); // Level 4
                digitalWriteFast(led_front_left, HIGH); // Level 5

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
          if(WAND_MENU_LEVEL == MENU_LEVEL_3 && i_wand_menu == 5 && switch_intensify.on() == true && switch_mode.on() == false) {
            // Adjust the default bootup system volume.
            wandSerialSend(W_VOLUME_INCREASE_EEPROM);

            // If there is no Pack, we need to adjust the volume manually
            if(b_gpstar_benchtest == true) {
              increaseVolumeEEPROM();
            }
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 5 && switch_intensify.on() == true && switch_mode.on() == false) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_5);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 4 && switch_intensify.on() == true && switch_mode.on() == false) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_4);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 3 && switch_intensify.on() == true && switch_mode.on() == false) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_3);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 2 && switch_intensify.on() == true && switch_mode.on() == false) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_2);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 1 && switch_intensify.on() == true && switch_mode.on() == false) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_1);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 5 && switch_intensify.on() == false && switch_mode.on() == true) {
            overheatTimerIncrement(5);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 4 && switch_intensify.on() == false && switch_mode.on() == true) {
            overheatTimerIncrement(4);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 3 && switch_intensify.on() == false && switch_mode.on() == true) {
            overheatTimerIncrement(3);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 2 && switch_intensify.on() == false && switch_mode.on() == true) {
            overheatTimerIncrement(2);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 1 && switch_intensify.on() == false && switch_mode.on() == true) {
            overheatTimerIncrement(1);
          }
          else if(i_wand_menu + 1 > 5) {
            switch(WAND_MENU_LEVEL) {
              case MENU_LEVEL_5:
                WAND_MENU_LEVEL = MENU_LEVEL_4;
                i_wand_menu = 1;

                // Turn on some lights to visually indicate which menu we are in.
                digitalWriteFast(led_slo_blo, HIGH); // Level 2
                digitalWrite(led_vent, LOW); // Level 3
                digitalWriteFast(led_white, LOW); // Level 4

                // Turn off the other lights.
                digitalWriteFast(led_front_left, LOW); // Level 5

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
                digitalWriteFast(led_slo_blo, HIGH); // Level 2
                digitalWrite(led_vent, LOW); // Level 3

                // Turn off the other lights.
                digitalWriteFast(led_white, HIGH); // Level 4
                digitalWriteFast(led_front_left, LOW); // Level 5

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
                digitalWriteFast(led_slo_blo, HIGH); // Level 2

                // Turn off the other lights.
                digitalWrite(led_vent, HIGH); // Level 3
                digitalWriteFast(led_white, HIGH); // Level 4
                digitalWriteFast(led_front_left, LOW); // Level 5

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
                digitalWriteFast(led_slo_blo, LOW); // Level 2
                digitalWrite(led_vent, HIGH); // Level 3
                digitalWriteFast(led_white, HIGH); // Level 4
                digitalWriteFast(led_front_left, LOW); // Level 5

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
          if(WAND_MENU_LEVEL == MENU_LEVEL_1 && i_wand_menu == 4 && switch_intensify.on() == false && switch_mode.on() == true) {
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
          else if(WAND_MENU_LEVEL == MENU_LEVEL_1 && i_wand_menu == 3 && switch_intensify.on() == false && switch_mode.on() == true) {
            // Change colour of the Power Cell Spectral custom colour.
            wandSerialSend(W_SPECTRAL_POWERCELL_CUSTOM_DECREASE);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_1 && i_wand_menu == 2 && switch_intensify.on() == false && switch_mode.on() == true) {
            // Change colour of the Cyclotron Spectral custom colour.
            wandSerialSend(W_SPECTRAL_CYCLOTRON_CUSTOM_DECREASE);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_1 && i_wand_menu == 1 && switch_intensify.on() == false && switch_mode.on() == true) {
            // Change colour of the Inner Cyclotron Spectral custom colour.
            wandSerialSend(W_SPECTRAL_INNER_CYCLOTRON_CUSTOM_DECREASE);
          }
          else if(i_wand_menu - 1 < 1) {
            switch(WAND_MENU_LEVEL) {
              case MENU_LEVEL_1:
                WAND_MENU_LEVEL = MENU_LEVEL_2;
                i_wand_menu = 5;

                // Turn on some lights to visually indicate which menu we are in.
                digitalWriteFast(led_slo_blo, HIGH); // Level 2

                // Turn off the other lights.
                digitalWrite(led_vent, HIGH); // Level 3
                digitalWriteFast(led_white, HIGH); // Level 4
                digitalWriteFast(led_front_left, LOW); // Level 5

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

              // Menu 2 the deepest level.
              case MENU_LEVEL_2:
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
          if(WAND_MENU_LEVEL == MENU_LEVEL_1 && i_wand_menu == 4 && switch_intensify.on() == false && switch_mode.on() == true) {
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
          else if(WAND_MENU_LEVEL == MENU_LEVEL_1 && i_wand_menu == 3 && switch_intensify.on() == false && switch_mode.on() == true) {
            // Change colour of the Power Cell Spectral custom colour.
            wandSerialSend(W_SPECTRAL_POWERCELL_CUSTOM_INCREASE);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_1 && i_wand_menu == 2 && switch_intensify.on() == false && switch_mode.on() == true) {
            // Change colour of the Cyclotron Spectral custom colour.
            wandSerialSend(W_SPECTRAL_CYCLOTRON_CUSTOM_INCREASE);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_1 && i_wand_menu == 1 && switch_intensify.on() == false && switch_mode.on() == true) {
            // Change colour of the Inner Cyclotron Spectral custom colour.
            wandSerialSend(W_SPECTRAL_INNER_CYCLOTRON_CUSTOM_INCREASE);
          }
          else if(i_wand_menu + 1 > 5) {
            switch(WAND_MENU_LEVEL) {
              case MENU_LEVEL_2:
                WAND_MENU_LEVEL = MENU_LEVEL_1;
                i_wand_menu = 1;

                // Turn off the other lights.
                digitalWriteFast(led_slo_blo, LOW); // Level 2
                digitalWrite(led_vent, HIGH); // Level 3
                digitalWriteFast(led_white, HIGH); // Level 4
                digitalWriteFast(led_front_left, LOW); // Level 5

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

      case ACTION_SETTINGS:
        // Counter clockwise.
        if(prev_next_code == 0x0b) {
          if(i_wand_menu == 4 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.on() == true && switch_mode.on() == false) {
            // Tell pack to dim the selected lighting. (Power Cell, Cyclotron or Inner Cyclotron)
            wandSerialSend(W_DIMMING_DECREASE);
          }
          else if(i_wand_menu == 3 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.on() == true && switch_mode.on() == false) {
            // Lower the sound effects volume.
            decreaseVolumeEffects();

            // Tell pack to lower the sound effects volume.
            wandSerialSend(W_VOLUME_SOUND_EFFECTS_DECREASE);
          }
          else if(i_wand_menu == 3 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.on() == false && switch_mode.on() == true && b_playing_music == true) {
            // Decrease the music volume.
            decreaseVolumeMusic();

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
                  digitalWriteFast(led_slo_blo, HIGH);

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
          if(i_wand_menu == 4 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.on() == true && switch_mode.on() == false) {
            // Tell pack to dim the selected lighting. (Power Cell, Cyclotron or Inner Cyclotron)
            wandSerialSend(W_DIMMING_INCREASE);
          }
          else if(i_wand_menu == 3 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.on() == true && switch_mode.on() == false) {
            // Increase sound effects volume.
            increaseVolumeEffects();

            // Tell pack to increase the sound effects volume.
            wandSerialSend(W_VOLUME_SOUND_EFFECTS_INCREASE);
          }
          else if(i_wand_menu == 3 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.on() == false && switch_mode.on() == true && b_playing_music == true) {
            // Increase music volume.
            increaseVolumeMusic();

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
                  digitalWriteFast(led_slo_blo, LOW);

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
        if(((WAND_STATUS == MODE_ON && SYSTEM_MODE != MODE_ORIGINAL) || (WAND_STATUS == MODE_OFF && SYSTEM_MODE == MODE_ORIGINAL))  && switch_intensify.on() == true && switch_vent.on() != true && switch_wand.on() != true) {
            // Counter clockwise.
            if(prev_next_code == 0x0b) {
              // Decrease the master system volume of both the Proton Pack and Neutrona Wand.
              decreaseVolume();
              wandSerialSend(W_VOLUME_DECREASE);
            }
            else if(prev_next_code == 0x07) {
              // Increase the master system volume of both the Proton Pack and Neutrona Wand.
              increaseVolume();
              wandSerialSend(W_VOLUME_INCREASE);
            }
        }
        else if(WAND_ACTION_STATUS != ACTION_OVERHEATING && WAND_ACTION_STATUS != ACTION_VENTING && b_pack_alarm != true) {
          if(WAND_ACTION_STATUS == ACTION_FIRING && i_power_level == i_power_level_max) {
            // Do nothing, we are locked in full power level while firing.
          }
          // Counter clockwise.
          else if(prev_next_code == 0x0b) {
            if((switch_wand.on() == true && switch_vent.on() == true && switch_activate.on() == true) || SYSTEM_MODE == MODE_ORIGINAL) {
              // Check to see the minimal power level depending on which system mode.
              uint8_t i_tmp_power_level_min = i_power_level_min;

              switch(SYSTEM_MODE) {
                case MODE_ORIGINAL:
                  i_tmp_power_level_min = i_power_level_min + 1;
                break;

                case MODE_SUPER_HERO:
                default:
                  i_tmp_power_level_min = i_power_level_min;
                break;
              }

              if(i_power_level - 1 >= i_tmp_power_level_min && WAND_STATUS == MODE_ON) {
                i_power_level_prev = i_power_level;
                i_power_level--;

                if(BARGRAPH_MODE == BARGRAPH_ORIGINAL && b_28segment_bargraph == true) {
                  bargraphPowerCheck2021Alt(false);
                }

                // Forces a redraw of the bargraph if firing while changing the power level in the BARGRAPH_ANIMATION_ORIGINAL.
                if(b_firing == true && b_28segment_bargraph == true && BARGRAPH_FIRING_ANIMATION == BARGRAPH_ANIMATION_ORIGINAL) {
                  bargraphRedraw();
                }

                soundBeepLoopStop();

                switch(getNeutronaWandYearMode()) {
                  case SYSTEM_1984:
                  case SYSTEM_1989:
                    if(switch_vent.on() == true) {
                      soundIdleLoopStop(false);
                      soundIdleLoop(false);
                    }
                  break;

                  case SYSTEM_AFTERLIFE:
                  case SYSTEM_FROZEN_EMPIRE:
                  default:
                    soundIdleLoopStop(false);
                    soundIdleLoop(false);
                  break;
                }

                updatePackPowerLevel();
              }
            }
            else if(vgModeCheck() && switch_wand.on() != true && switch_vent.on() == true && ms_firing_mode_switch.remaining() < 1 && WAND_STATUS == MODE_ON) {
              // Counter clockwise firing mode selection.
              if(FIRING_MODE == PROTON) {
                FIRING_MODE = STASIS;
              }
              else if(FIRING_MODE == STASIS) {
                FIRING_MODE = SLIME;
              }
              else if(FIRING_MODE == SLIME) {
                FIRING_MODE = MESON;
              }
              else if(FIRING_MODE == MESON) {
                // Conditional mode advancement.
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
                  FIRING_MODE = PROTON;
                }
              }
              else if(FIRING_MODE == SPECTRAL) {
                // Conditional mode advancement.
                if(b_holiday_mode_enabled == true) {
                  FIRING_MODE = HOLIDAY;
                }
                else if(b_spectral_custom_mode_enabled == true) {
                  FIRING_MODE = SPECTRAL_CUSTOM;
                }
                else {
                  FIRING_MODE = PROTON;
                }
              }
              else if(FIRING_MODE == HOLIDAY) {
                // Conditional mode advancement.
                if(b_spectral_custom_mode_enabled == true) {
                  FIRING_MODE = SPECTRAL_CUSTOM;
                }
                else {
                  FIRING_MODE = PROTON;
                }
              }
              else {
                FIRING_MODE = PROTON;
              }

              modeCheck();
              ms_firing_mode_switch.start(i_firing_mode_switch_delay);
            }

            // Decrease the music volume if the wand/pack is off. A quick easy way to adjust the music volume on the go.
            if(WAND_STATUS == MODE_OFF && b_playing_music == true && switch_intensify.on() != true) {
              decreaseVolumeMusic();

              // Tell pack to lower music volume.
              wandSerialSend(W_VOLUME_MUSIC_DECREASE);
            }
            else if(WAND_STATUS == MODE_OFF && switch_intensify.on() == true) {
              // Decrease the master volume of the Neutrona Wand only.
              decreaseVolume();
            }
          }

          if(WAND_ACTION_STATUS == ACTION_FIRING && i_power_level == i_power_level_max) {
            // Do nothing, we are locked in full power level while firing.
          }
          // Clockwise.
          else if(prev_next_code == 0x07) {
            if((switch_wand.on() == true && switch_vent.on() == true && switch_activate.on() == true) || SYSTEM_MODE == MODE_ORIGINAL) {
              if(i_power_level + 1 <= i_power_level_max && WAND_STATUS == MODE_ON) {
                if(i_power_level + 1 == i_power_level_max && WAND_ACTION_STATUS == ACTION_FIRING) {
                  // Do nothing, we do not want to go into max power level if firing in a lower power level already.
                }
                else {
                  i_power_level_prev = i_power_level;
                  i_power_level++;

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
                      if(switch_vent.on() == true) {
                        soundIdleLoopStop(false);
                        soundIdleLoop(false);
                      }
                    break;

                    case SYSTEM_AFTERLIFE:
                    case SYSTEM_FROZEN_EMPIRE:
                    default:
                      soundIdleLoopStop(false);
                      soundIdleLoop(false);
                    break;
                  }

                  updatePackPowerLevel();
                }
              }
            }
            else if(vgModeCheck() && switch_wand.on() != true && switch_vent.on() == true && ms_firing_mode_switch.remaining() < 1 && WAND_STATUS == MODE_ON) {
              if(FIRING_MODE == PROTON) {
                // Conditional mode advancement.
                if(b_spectral_custom_mode_enabled == true) {
                  FIRING_MODE = SPECTRAL_CUSTOM;
                }
                else if(b_holiday_mode_enabled == true) {
                  FIRING_MODE = HOLIDAY;
                }
                else if(b_spectral_mode_enabled == true) {
                  FIRING_MODE = SPECTRAL;
                }
                else {
                  FIRING_MODE = MESON;
                }
              }
              else if(FIRING_MODE == SPECTRAL_CUSTOM) {
                // Conditional mode advancement.
                if(b_holiday_mode_enabled == true) {
                  FIRING_MODE = HOLIDAY;
                }
                else if(b_spectral_mode_enabled == true) {
                  FIRING_MODE = SPECTRAL;
                }
                else {
                  FIRING_MODE = MESON;
                }
              }
              else if(FIRING_MODE == HOLIDAY) {
                // Conditional mode advancement.
                if(b_spectral_mode_enabled == true) {
                  FIRING_MODE = SPECTRAL;
                }
                else {
                  FIRING_MODE = MESON;
                }
              }
              else if(FIRING_MODE == SPECTRAL) {
                FIRING_MODE = MESON;
              }
              else if(FIRING_MODE == MESON) {
                FIRING_MODE = SLIME;
              }
              else if(FIRING_MODE == SLIME) {
                FIRING_MODE = STASIS;
              }
              else {
                FIRING_MODE = PROTON;
              }

              modeCheck();
              ms_firing_mode_switch.start(i_firing_mode_switch_delay);
            }

            // Increase the music volume if the wand/pack is off. A quick easy way to adjust the music volume on the go.
            if(WAND_STATUS == MODE_OFF && b_playing_music == true && switch_intensify.on() != true) {
              increaseVolumeMusic();

              // Tell pack to increase music volume.
              wandSerialSend(W_VOLUME_MUSIC_INCREASE);
            }
            else if(WAND_STATUS == MODE_OFF && switch_intensify.on() == true) {
              // Increase the master volume of the Neutrona Wand only.
              increaseVolume();
            }
          }
        }
      break;
    }
  }
}

// Tell the pack which power level the Neutrona Wand is set at.
void updatePackPowerLevel() {
  switch(i_power_level) {
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

    case 1:
    default:
      // Level 1
      wandSerialSend(W_POWER_LEVEL_1);
    break;
  }
}

void vibrationSetting() {
  if(b_vibration_on == true) {
    if(ms_bargraph.isRunning() == false && WAND_ACTION_STATUS != ACTION_FIRING) {
      switch(i_power_level) {
        case 1:
        default:
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
  switch_intensify.poll();
  switch_activate.poll();
  switch_vent.poll();
  switch_wand.poll();
  switch_mode.poll();
  switch_barrel.poll();
}

void ventSwitched(void* n) {
  (void)(n); // Suppress unused variable warning
  ventSwitchedCount++;
}

void wandSwitched(void* n) {
  (void)(n); // Suppress unused variable warning
  wandSwitchedCount++;
}

void wandBarrelLightsOff() {
  for(uint8_t i = 0; i < i_num_barrel_leds; i++) {
    switch(WAND_BARREL_LED_COUNT) {
      case LEDS_48:
        // Turn off the entire Frutto LED array.
        barrel_leds[PROGMEM_READU8(frutto_barrel[i])] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
      break;

      case LEDS_5:
      default:
        // Turn off the entire Hasbro LED array.
        barrel_leds[i] = getHueColour(C_BLACK, WAND_BARREL_LED_COUNT);
      break;
    }
  }
}

// Exit the wand menu system while the wand is off.
void wandExitMenu() {
  i_wand_menu = 5;

  if(b_pack_alarm != true) {
    playEffect(S_CLICK);
  }

  switch(FIRING_MODE) {
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

    case PROTON:
    default:
      // Tell the pack we are in proton mode.
      wandSerialSend(W_PROTON_MODE);
    break;
  }

  WAND_ACTION_STATUS = ACTION_IDLE;

  wandLightsOff();

  // Reset the bargraph in case it was changed.
  bargraphYearModeUpdate();

  // Reset the white LED blink rate setting in case we changed years.
  resetWhiteLEDBlinkRate();

  // In original mode, we need to re-initalise the 28 segment bargraph if some switches are already toggled on.
  if(SYSTEM_MODE == MODE_ORIGINAL) {
    if(switch_vent.on() == true && switch_wand.on() == true) {
      if(b_pack_ion_arm_switch_on == true && b_28segment_bargraph == true && b_mode_original_toggle_sounds_enabled == true) {
        if(b_extra_pack_sounds == true) {
          wandSerialSend(W_MODE_ORIGINAL_HEATUP);
        }

        stopEffect(S_WAND_HEATUP_ALT);
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

  wandSwitchedCount = 0;
  ventSwitchedCount = 0;

  vibrationOff(); // Make sure we stop any menu-related vibration, if any.

  if(b_gpstar_benchtest == true) {
    // Also need to make sure to reset the "ion arm switch" to off if standalone.
    b_pack_ion_arm_switch_on = false;
  }

  i_wand_menu = 5;

  WAND_ACTION_STATUS = ACTION_IDLE;

  wandLightsOff();
  wandBarrelLightsOff();

  // Reset the bargraph in case it was changed.
  bargraphYearModeUpdate();

  // Reset the white LED blink rate setting in case we changed years.
  resetWhiteLEDBlinkRate();

  // Send current preferences to the pack for use by the serial1 device.
  wandSerialSend(W_SEND_PREFERENCES_WAND);
  wandSerialSend(W_SEND_PREFERENCES_SMOKE);
}

// Barrel safety switch is connected to analog pin 7.
bool switchBarrel() {
  if(switch_barrel.on() == true) {
    if(b_switch_barrel_extended == true) {
      wandSerialSend(W_BARREL_RETRACTED);
    }

    b_switch_barrel_extended = false;
  }
  else if(switch_barrel.on() == false) {
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

  ms_gun_loop_1.start(i_gun_loop_1);

  if(b_extra_pack_sounds == true) {
    wandSerialSend(W_AFTERLIFE_GUN_RAMP_1);
  }
}

void resetWhiteLEDBlinkRate() {
  switch(getNeutronaWandYearMode()) {
    case SYSTEM_1984:
    case SYSTEM_1989:
      d_white_light_interval = i_classic_blink_intervals[i_classic_blink_index];
    break;
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      d_white_light_interval = i_afterlife_blink_interval;
    break;
  }
}

// Rebuilds the overheat enable array.
void resetOverheatLevels() {
  b_overheat_level[0] = b_overheat_level_1;
  b_overheat_level[1] = b_overheat_level_2;
  b_overheat_level[2] = b_overheat_level_3;
  b_overheat_level[3] = b_overheat_level_4;
  b_overheat_level[4] = b_overheat_level_5;
}

void checkMenuVibration() {
  if(b_menu_vibration_active == false && ms_menu_vibration.isRunning()) {
    analogWrite(vibration, 150);
    b_menu_vibration_active = true;
  }
  else if(ms_menu_vibration.justFinished() && b_menu_vibration_active == true) {
    vibrationOff();
  }
}

// Included last as the contained logic will control all aspects of the pack using the defined functions above.
#include "Actions.h"
#include "Serial.h"
