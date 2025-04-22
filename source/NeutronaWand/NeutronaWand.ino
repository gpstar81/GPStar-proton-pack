/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
 * Please note, due to limitations of the ATMega328P, Arduino Nano builds are no
 * longer supported for the Neutrona Wand. Upgrading to the GPStar controller or
 * a Mega 2560 Pro Mini is the only viable solution for continued support.
 * The last supported version for the Arduino Nano is 2.2.0
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
#include "System.h"
#include "Actions.h"
#include "Serial.h"

void setup() {
  Serial.begin(9600); // Standard serial (USB) console.

  Serial1.begin(9600); // Communication to the Proton Pack.
  wandComs.begin(Serial1, false);

  // Setup the audio device for this controller.
  setupAudioDevice();

  // Change PWM frequency of pin 11 for the vibration motor, we do not want it high pitched.
  TCCR1B = (TCCR1B & B11111000) | B00000100; // for PWM frequency of 122.55 Hz

  // Barrel LEDs - NOTE: These are GRB not RGB so note that all CRGB objects will have R/G swapped.
  FastLED.addLeds<NEOPIXEL, BARREL_LED_PIN>(barrel_leds, BARREL_LEDS_MAX);

  // RGB Vent Light.
  FastLED.addLeds<NEOPIXEL, TOP_LED_PIN>(vent_leds, VENT_LEDS_MAX);
  vent_leds[0] = getHueAsRGB(C_WHITE); // Set vent light array to white for initial reset.
  vent_leds[1] = getHueAsRGB(C_WHITE); // Set top light array to white for initial reset.
  ms_vent_light.start(i_vent_light_update_interval); // Setup a timer for updating the vent light.

  // Setup default system settings.
  SYSTEM_MODE = MODE_SUPER_HERO;
  BARGRAPH_MODE = BARGRAPH_ORIGINAL;
  BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_DEFAULT;
  BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_SUPER_HERO;
  BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_DEFAULT;
  VIBRATION_MODE_EEPROM = VIBRATION_DEFAULT;
  VIBRATION_MODE = VIBRATION_FIRING_ONLY;
  WAND_MENU_LEVEL = MENU_LEVEL_1;
  WAND_YEAR_MODE = YEAR_DEFAULT;
  WAND_YEAR_CTS = CTS_DEFAULT;
  SYSTEM_YEAR = SYSTEM_AFTERLIFE;
  WAND_BARREL_LED_COUNT = LEDS_5;

  switch_vent.setPushedCallback(&ventSwitched);
  switch_wand.setPushedCallback(&wandSwitched);

  // Rotary encoder on the top of the wand.
  pinModeFast(ROTARY_ENCODER_A, INPUT_PULLUP);
  pinModeFast(ROTARY_ENCODER_B, INPUT_PULLUP);

  Wire.begin();
  Wire.setClock(400000UL); // Sets the i2c bus to 400kHz

  byte by_error, by_address;
  uint8_t i_i2c_devices = 0;

  // Scan i2c for any devices (28 segment bargraph).
  for(by_address = 1; by_address < 127; by_address++ ) {
    Wire.beginTransmission(by_address);
    by_error = Wire.endTransmission();

    if(by_error == 0) {
      i_i2c_devices++;
    }
  }

  if(i_i2c_devices > 0) {
    // Set to 28-segment, though this will be overridden by EEPROM.
    BARGRAPH_TYPE = SEGMENTS_28;
    ht_bargraph.begin(0x00);
  }
  else {
    // Original 5 LED Hasbro bargraph.
    BARGRAPH_TYPE = SEGMENTS_5;

    pinModeFast(BARGRAPH_LED_1_PIN, OUTPUT);
    pinModeFast(BARGRAPH_LED_2_PIN, OUTPUT);
    pinModeFast(BARGRAPH_LED_3_PIN, OUTPUT);
    pinModeFast(BARGRAPH_LED_4_PIN, OUTPUT);
    pinModeFast(BARGRAPH_LED_5_PIN, OUTPUT);
  }

  pinModeFast(SLO_BLO_LED_PIN, OUTPUT); // SLO-BLO LED under the toggle switches.
  pinModeFast(CLIPPARD_LED_PIN, OUTPUT); // Front left LED underneath the Clippard valve.
  pinModeFast(BARREL_HAT_LED_PIN, OUTPUT); // Hat light at front of the wand near the barrel tip.
  pinModeFast(TOP_HAT_LED_PIN, OUTPUT); // Hat light at top of the wand body (gun box).
  pinModeFast(BARREL_TIP_LED_PIN, OUTPUT); // LED at the tip of the wand barrel.

  pinMode(VENT_LED_PIN, OUTPUT); // Vent light could be either Digital or PWM based on user setting, so use default functions.
  pinMode(TOP_LED_PIN, OUTPUT); // Blinking top light could be either addressable or non-addressable based on user setting, so use default functions.
  pinMode(VIBRATION_PIN, OUTPUT); // Vibration motor is PWM, so fallback to default pinMode just to be safe.

  // Status indicator LED on the v1.4 GPStar Neutrona Wand Board.
  pinModeFast(WAND_STATUS_LED_PIN, OUTPUT);

  // Wand status.
  WAND_STATUS = MODE_OFF;
  WAND_ACTION_STATUS = ACTION_IDLE;

  ms_reset_sound_beep.start(0);

  // We bootup the wand in the classic proton mode.
  STREAM_MODE = PROTON;

  // Select a random GB1/GB2 white LED blink rate for this session.
  i_classic_blink_index = random(0,5);

  // Load any saved settings stored in the EEPROM memory of the GPStar Neutrona Wand.
  if(b_eeprom) {
    readEEPROM();
  }

  // Make sure lights are off.
  wandLightsOff();

  // Start the button mash check timer.
  ms_bmash.start(0);

  // Sanity check to make sure that a firing mode was set as default.
  if(FIRING_MODE != CTS_MODE && FIRING_MODE != CTS_MIX_MODE) {
    FIRING_MODE = VG_MODE;
    LAST_FIRING_MODE = FIRING_MODE;
  }

  // Check if we should be in video game mode or not.
  vgModeCheck();

  // Setup the bargraph.
  bargraphYearModeUpdate();

  // Start up some timers for MODE_ORIGINAL.
  ms_slo_blo_blink.start(i_slo_blo_blink_delay);

  // Initialize the fastLED state update timer.
  ms_fast_led.start(i_fast_led_delay);

  // Initialize the timer for initial handshake.
  ms_packsync.start(0);

  if(b_gpstar_benchtest) {
    WAND_CONN_STATE = NC_BENCHTEST;

    b_pack_on = true; // Pretend that the pack (not really attached) has been powered on.

    // Stop the pack sync timer since we are no longer syncing to a pack.
    ms_packsync.stop();

    // Check music timer for bench test mode only.
    ms_check_music.start(i_music_check_delay);

    // No pack to do a volume sync with, so reset our master volume manually.
    updateMasterVolume(true);
  }
  else {
    WAND_CONN_STATE = PACK_DISCONNECTED;
  }
}

void mainLoop() {
  // Get the current state of any input devices (toggles, buttons, and switches).
  switchLoops();
  checkSwitches();
  checkRotaryEncoder();
  checkMenuVibration();

  if(WAND_ACTION_STATUS != ACTION_FIRING) {
    if(ms_bmash.remaining() < 1) {
      // Clear counter until user begins firing (post any lock-out period).
      i_bmash_count = 0;

      if(b_wand_mash_error) {
        // Return the wand to a normal firing state after lock-out from button mashing.
        WAND_STATUS = MODE_ON;
        WAND_ACTION_STATUS = ACTION_IDLE;

        postActivation();

        b_wand_mash_error = false;

        stopEffect(S_WAND_STASIS_IDLE_LOOP);
        stopEffect(S_SMASH_ERROR_LOOP);
        playEffect(S_SMASH_ERROR_RESTART);

        wandSerialSend(W_MASH_ERROR_RESTART);

        bargraphClearAlt();
      }
    }
  }

  switch(WAND_STATUS) {
    case MODE_OFF:
      if(WAND_ACTION_STATUS != ACTION_LED_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU) {
        if(WAND_ACTION_STATUS != ACTION_SETTINGS && b_gpstar_benchtest && SYSTEM_MODE == MODE_ORIGINAL && switch_intensify.doubleClick()) {
          // This allows a standalone wand to "flip the ion arm switch" when in MODE_ORIGINAL by double-clicking the Intensify switch while the wand is turned off
          changeIonArmSwitchState(!b_pack_ion_arm_switch_on);
        }

        if(switch_mode.pushed() || b_pack_alarm) {
          if(WAND_ACTION_STATUS != ACTION_SETTINGS && !b_pack_alarm && (!b_pack_on || b_gpstar_benchtest)) {
            playEffect(S_CLICK);

            WAND_ACTION_STATUS = ACTION_SETTINGS;
            WAND_MENU_LEVEL = MENU_LEVEL_1;

            i_wand_menu = 5;
            ms_settings_blink.start(i_settings_blink_delay);

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
        else if(WAND_ACTION_STATUS == ACTION_SETTINGS && b_pack_on) {
          if(!b_gpstar_benchtest) {
            wandExitMenu();
          }
        }
      }

      // Reset the count of the wand switch
      if(!switch_intensify.on()) {
        wandSwitchedCount = 0;
        ventSwitchedCount = 0;
      }

      if(WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_LED_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU && (!b_pack_on || b_gpstar_benchtest) && switch_intensify.on() && wandSwitchedCount >= 5) {
        stopEffect(S_BEEPS_BARGRAPH);
        playEffect(S_BEEPS_BARGRAPH);

        stopEffect(S_EEPROM_LED_MENU);
        playEffect(S_EEPROM_LED_MENU);

        wandSerialSend(W_EEPROM_LED_MENU);
        wandSerialSend(W_SPECTRAL_LIGHTS_ON);

        i_wand_menu = 5;

        WAND_ACTION_STATUS = ACTION_LED_EEPROM_MENU;
        WAND_MENU_LEVEL = MENU_LEVEL_1;

        ms_settings_blink.start(i_settings_blink_delay);

        wandBarrelSpectralCustomConfigOn();

        // Make sure some of the wand lights are off.
        wandLightsOffMenuSystem();
      }
      else if(WAND_ACTION_STATUS == ACTION_LED_EEPROM_MENU && b_pack_on) {
        if(!b_gpstar_benchtest) {
          wandExitEEPROMMenu();
        }
      }

      if(WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_LED_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU
        && (!b_pack_on || b_gpstar_benchtest) && switch_intensify.on() && ventSwitchedCount >= 5) {
        stopEffect(S_BEEPS_BARGRAPH);
        playEffect(S_BEEPS_BARGRAPH);

        stopEffect(S_EEPROM_CONFIG_MENU);
        playEffect(S_EEPROM_CONFIG_MENU);

        wandSerialSend(W_EEPROM_CONFIG_MENU);

        i_wand_menu = 5;

        WAND_ACTION_STATUS = ACTION_CONFIG_EEPROM_MENU;
        WAND_MENU_LEVEL = MENU_LEVEL_1;

        ms_settings_blink.start(i_settings_blink_delay);

        // Make sure some of the wand lights are off.
        wandLightsOffMenuSystem();
      }
      else if(WAND_ACTION_STATUS == ACTION_CONFIG_EEPROM_MENU && b_pack_on) {
        if(!b_gpstar_benchtest) {
          wandExitEEPROMMenu();
        }
      }

      // Check to see if we should be blinking the power-on reminder LED.
      checkPowerOnReminder();
    break;

    case MODE_ERROR:
      if(ms_error_blink.justFinished()) {
        ms_error_blink.start(i_error_blink_delay);

        if(!b_wand_mash_error) {
          if(b_extra_pack_sounds) {
            wandSerialSend(W_WAND_BEEP_SOUNDS);
          }

          playEffect(S_BEEPS_LOW, false, i_volume_effects, false, 0, false);
          playEffect(S_BEEPS, false, i_volume_effects, false, 0, false);
        }
      }

      if(ms_warning_blink.justFinished()) {
        if(!b_wand_mash_error) {

          if(b_extra_pack_sounds) {
            wandSerialSend(W_WAND_BEEP_BARGRAPH);
          }

          playEffect(S_BEEPS_BARGRAPH, false, i_volume_effects, false, 0, false);
        }

        ms_warning_blink.repeat();
      }

      settingsBlinkingLights();

      if(b_wand_mash_error) {
        wandBarrelHeatUp();
      }
    break;

    case MODE_ON:
      // Hat light 2 blinking when the Proton Pack ribbon cable has been removed.
      if(b_pack_alarm) {
        if(ms_error_blink.justFinished()) {
          ms_error_blink.start(i_error_blink_delay);
        }

        // This is going to cause the bargraph to ramp down.
        if(ms_bargraph.justFinished()) {
          bargraphRampUp();
        }
      }
      else {
        // Ramp the bargraph up then ramp down back to the default power level setting on a fresh start.
        if(ms_bargraph.justFinished()) {
          bargraphRampUp();
        }
        else if(!ms_bargraph.isRunning() && WAND_ACTION_STATUS != ACTION_FIRING && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          // Bargraph idling loop.
          bargraphPowerCheck();
        }

        if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
          if(ms_gun_loop_1.justFinished() && !switch_vent.on()) {
            if(b_extra_pack_sounds) {
              wandSerialSend(W_AFTERLIFE_GUN_LOOP_1);
            }

            if(AUDIO_DEVICE != A_GPSTAR_AUDIO_ADV) {
              playEffect(S_AFTERLIFE_WAND_IDLE_1, true);
            }
          }
        }
      }

      // Top white light.
      if(ms_white_light.justFinished()) {
        vent_leds[1] ? ventTopLightControl(false) : ventTopLightControl(true);

        ms_white_light.repeat();
      }

      wandBarrelHeatUp();
      vibrationSetting();
    break;
  }

  // Handle hat light status changes.
  hatLightControl();

  // Handle button press events based on current wand state and menu level (for config/EEPROM purposes).
  checkWandAction();

  if(b_firing && WAND_ACTION_STATUS != ACTION_FIRING) {
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
    //FastLED.show();
    FastLED[0].showLeds(255);
    ms_fast_led.start(i_fast_led_delay);
  }

  // Update the vent/top LEDs and restart the timer.
  if(ms_vent_light.justFinished()) {
    // Only send an update if we actually made a change.
    if(b_vent_lights_changed) {
      if(b_rgb_vent_light || WAND_CONN_STATE == PACK_DISCONNECTED) {
        // Only commit an update if the addressable LED panel is installed or if the Neutrona Wand can not make a connection to the Proton Pack.
        FastLED[1].showLeds(255);
      }

      b_vent_lights_changed = false;
    }

    ms_vent_light.repeat();
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
        vent_leds[1] ? ventTopLightControl(false) : ventTopLightControl(true); // Blink the top LED.
        digitalWriteFast(WAND_STATUS_LED_PIN, (digitalReadFast(WAND_STATUS_LED_PIN) == LOW) ? HIGH : LOW); // Blink the onboard LED on the Neutrona Wand board.
      }

      checkPack(); // Check for any response from the pack while still waiting.
    break;

    case PACK_CONNECTED:
      // When connected to a pack, prepare to send a regular handshake to indicate presence.
      if(ms_handshake.justFinished()) {
        wandSerialSend(W_HANDSHAKE); // Remind the pack that a wand is still present.
        ms_handshake.restart(); // Restart the handshake timer.
      }

      updateAudio(); // Update the state of the selected sound board.

      checkPack(); // Get the latest communications from the connected Proton Pack.

      if(b_pack_post_finish) {
        mainLoop(); // Continue on to the main loop.
      }
    break;

    case NC_BENCHTEST:
      updateAudio(); // Update the state of the selected sound board.

      checkMusic(); // Music control is here since pack is not present.

      mainLoop(); // Continue on to the main loop.
    break;
  }
}