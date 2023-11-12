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

/*
  Please note, due to limitations of the ATMega328P, Arduino Nano builds are no longer supported for the Neutrona Wand.
  The last supported version is 2.2.0
  https://github.com/gpstar81/haslab-proton-pack/releases/tag/V2.2.0
*/

#if defined(__AVR_ATmega2560__)
  #define GPSTAR_NEUTRONA_WAND_PCB
#endif

// 3rd-Party Libraries
#include <millisDelay.h>
#include <FastLED.h>
#include <ezButton.h>
#include <EEPROM.h>
#include <ht16k33.h>
#include <Wire.h>

#include <SerialTransfer.h>

/*
  ***** IMPORTANT *****
  * You no longer need to edit and configure wavTrigger.h anymore.
  * Please make sure your WAV Trigger devices are running firmware version 1.40 or higher.
  * You can download the latest directly from the gpstar github repository or from the Robertsonics website.
  https://github.com/gpstar81/haslab-proton-pack/tree/main/extras

  * Information on how to update your WAV Trigger devices can be found on the gpstar github repository.
  https://github.com/gpstar81/haslab-proton-pack/blob/main/WAVTRIGGER.md
*/
#include "wavTrigger.h"

// Local Files
#include "Configuration.h"
#include "MusicSounds.h"
#include "Communication.h"
#include "Header.h"
#include "Colours.h"

void setup() {
  Serial.begin(9600);

  // Enable communication to the Proton Pack.
  Serial1.begin(9600);
  wandComs.begin(Serial1, false);

  // Change PWM frequency of pin 3 and 11 for the vibration motor, we do not want it high pitched.
  TCCR2B = (TCCR2B & B11111000) | (B00000110); // for PWM frequency of 122.55 Hz

  // Setup the WAV Trigger.
  setupWavTrigger();

  // Barrel LEDs - NOTE: These are GRB not RGB so note that all CRGB objects will have R/G swapped.
  FastLED.addLeds<NEOPIXEL, BARREL_LED_PIN>(barrel_leds, BARREL_LEDS_MAX);

  // Setup default system settings.
  SYSTEM_MODE = MODE_SUPER_HERO;
  BARGRAPH_MODE = BARGRAPH_ORIGINAL;
  BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_DEFAULT;
  BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_SUPER_HERO;
  BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_DEFAULT;
  WAND_MENU_LEVEL = MENU_LEVEL_1;
  WAND_YEAR_MODE = YEAR_DEFAULT;
  WAND_YEAR_CTS = CTS_DEFAULT;
  SYSTEM_YEAR = SYSTEM_AFTERLIFE;

  switch_wand.setDebounceTime(switch_debounce_time);
  switch_intensify.setDebounceTime(switch_debounce_time);
  switch_activate.setDebounceTime(switch_debounce_time);
  switch_vent.setDebounceTime(switch_debounce_time);

  pinMode(switch_mode, INPUT_PULLUP);
  pinMode(switch_barrel, INPUT_PULLUP);

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

  // Extra optional items if using them with the gpstar Neutrona Wand microcontroller.
  pinMode(led_front_left, OUTPUT); // Front left LED. When using the gpstar Neutrona Wand microcontroller, it is wired to its own pin. When using an Arduino Nano, it is linked with led_slo_blo.
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

  // Setup the mode switch and barrel switch debounce.
  ms_switch_mode_debounce.start(1);
  ms_switch_barrel_debounce.start(1);

  // We bootup the wand in the classic proton mode.
  FIRING_MODE = PROTON;
  PREV_FIRING_MODE = SETTINGS;

  // Load any saved settings stored in the EEPROM memory of the gpstar Neutrona Wand.
  if(b_eeprom == true) {
    readEEPROM();
  }

  ms_bmash.start(i_bmash_delay);

  ms_firing_debounce.start(i_firing_debounce);

  // Sanity check just in case a user forgot to enable CTS while enabling CTS Mix.
  if(b_cross_the_streams_mix == true && b_cross_the_streams != true) {
    b_cross_the_streams = true;
  }

  // Check if we should be in video game mode or not.
  vgModeCheck();

  // Start up some timers for MODE_ORIGINAL.
  ms_slo_blo_blink.start(i_slo_blo_blink_delay);

  if(b_gpstar_benchtest == true) {
    b_no_pack = true;
    b_wait_for_pack = false;
    b_pack_on = true;
  }
}

void loop() {
  if(b_wait_for_pack == true) {
    if(b_volume_sync_wait != true) {
      // Handshake with the pack telling the pack that we are here.
      wandSerialSend(W_HANDSHAKE);
    }

    // Synchronise some settings with the pack.
    checkPack();

    delay(10);
  }
  else {
    mainLoop();
  }
}


void mainLoop() {
  w_trig.update();

  checkPack();
  switchLoops();
  checkRotary();
  checkSwitches();

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

        if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE) {
          playEffect(S_BOOTUP);
        }

        bargraphClearAlt();

        // Re-enable the hat light on top of the gun box
        digitalWrite(led_hat_2, HIGH);
      }
    }
  }

  switch(WAND_ACTION_STATUS) {
    case ACTION_IDLE:
    default:
      if(WAND_STATUS == MODE_ON) {
        switch(getNeutronaWandYearMode()) {
          case SYSTEM_1984:
          case SYSTEM_1989:
            // Do nothing.
          break;

          case SYSTEM_AFTERLIFE:
          case SYSTEM_FROZEN_EMPIRE:
          default:
            if(WAND_ACTION_STATUS != ACTION_OVERHEATING && b_pack_alarm != true) {
              // When ready to fire the hat light LED at the barrel tip lights up in Afterlife mode.
              if(b_switch_barrel_extended == true && switch_vent.getState() == LOW && switch_wand.getState() == LOW) {
                digitalWrite(led_hat_1, HIGH);
              }
              else {
                digitalWrite(led_hat_1, LOW);
              }
            }
          break;
        }
      }
    break;

    case ACTION_OFF:
      b_wand_mash_error = false;
      wandOff();
    break;

    case ACTION_FIRING:
      if(FIRING_MODE == VENTING) {
        // If we are in venting mode, let's trigger a vent sequence.
        startVentSequence();
      }
      else if(b_pack_on == true && b_pack_alarm == false) {
        if(ms_firing_start_sound_delay.justFinished()) {
          modeFireStartSounds();
        }

        if(b_firing == false) {
          b_firing = true;
          modeFireStart();
        }

        if(ms_hat_1.isRunning()) {
          if(ms_hat_1.remaining() < i_hat_1_delay / 2) {
            digitalWrite(led_hat_1, LOW);
            digitalWrite(led_hat_2, HIGH);
          }
          else {
            digitalWrite(led_hat_1, HIGH);
            digitalWrite(led_hat_2, LOW);
          }

          if(ms_hat_1.justFinished()) {
            ms_hat_1.start(i_hat_1_delay);
          }
        }

        // Overheating.
        if(ms_overheat_initiate.justFinished() && b_overheat_mode[i_power_mode - 1] == true && b_overheat_enabled == true) {
          startVentSequence();
        }
        else {
          modeFiring();

          // Stop firing if any of the main switches are turned off or the barrel is retracted.
          if(switch_vent.getState() == HIGH || switch_wand.getState() == HIGH || b_switch_barrel_extended != true) {
            modeFireStop();
          }
        }
      }
      else if(b_pack_alarm == true && b_firing == true) {
        modeFireStop();
      }
    break;

    case ACTION_OVERHEATING:
      if(b_overheat_bargraph_blink == true) {
        settingsBlinkingLights();

        if(ms_blink_sound_timer_1.justFinished()) {
          ms_blink_sound_timer_1.start(i_blink_sound_timer);

          playEffect(S_BEEPS_LOW);
          playEffect(S_BEEPS);
        }

        if(ms_blink_sound_timer_2.justFinished()) {
          playEffect(S_BEEPS_BARGRAPH);

          ms_blink_sound_timer_2.start(i_blink_sound_timer * 4);
        }
      }
      else {
        // Prepare to make the bargraph ramp down.
        if(ms_bargraph.justFinished()) {
          bargraphRampUp();
        }
      }

      if(b_no_pack == true) {
        // Since the pack tells the wand when overheating is finished. If the wand is running with no pack, then the wand needs to calculate when to finish.
        if(ms_overheating.justFinished()) {
          overHeatingFinished();
        }
      }

    break;

    case ACTION_ERROR:
      // nothing.
    break;

    case ACTION_ACTIVATE:
      modeActivate();
    break;

    case ACTION_EEPROM_MENU:
      settingsBlinkingLights();

      switch(i_wand_menu) {
        // Intensify: Clear the Proton Pack EEPROM settings and exit.
        // Barrel Wing Button: Save the current settings to the Proton Pack EEPROM and exit.
        case 5:
          // Tell the Proton Pack to clear the EEPROM settings and exit.
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            // Tell pack to clear the EEPROM and exit.
            wandSerialSend(W_CLEAR_EEPROM_SETTINGS);
            wandSerialSend(W_SPECTRAL_LIGHTS_OFF);

            stopEffect(S_VOICE_EEPROM_ERASE);
            playEffect(S_VOICE_EEPROM_ERASE);

            clearLEDEEPROM();

            wandExitEEPROMMenu();
          }
          else if(switchMode() == true) {
            // Tell the Proton Pack to save the current settings to the EEPROM and exit.
            wandSerialSend(W_SAVE_EEPROM_SETTINGS);
            wandSerialSend(W_SPECTRAL_LIGHTS_OFF);

            stopEffect(S_VOICE_EEPROM_SAVE);
            playEffect(S_VOICE_EEPROM_SAVE);

            saveLEDEEPROM();

            wandExitEEPROMMenu();
          }
        break;

        // Intensify: Cycle through the different Cyclotron LED counts.
        // Barrel Wing Button: Adjust the Neutrona Wand barrel colour hue. <- Controlled by checkRotary();
        case 4:
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            wandSerialSend(W_TOGGLE_CYCLOTRON_LEDS);
          }
        break;

        // Intensify: Cycle through the different Power Cell LED counts.
        // Barrel Wing Button: Adjust the Power Cell colour hue. <- Controlled by checkRotary();
        case 3:
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            wandSerialSend(W_TOGGLE_POWERCELL_LEDS);
          }
        break;

        // Intensify: Cycle through the different inner Cyclotron LED counts.
        // Barrel Wing Button: Adjust the Cyclotron colour hue. <- Controlled by checkRotary();
        case 2:
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            wandSerialSend(W_TOGGLE_INNER_CYCLOTRON_LEDS);
          }
        break;

        // Intensify: Enable or disable GRB mode for the inner Cyclotron LEDs.
        // Barrel Wing Button: Adjust the Inner Cyclotron colour hue. <- Controlled by checkRotary();
        case 1:
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            wandSerialSend(W_TOGGLE_RGB_INNER_CYCLOTRON_LEDS);
          }
        break;
      }
    break;

    case ACTION_CONFIG_EEPROM_MENU:
      /*
        Menu Level 3 - 5: Intensify: Default main system volume + top dial | Alt button: Toggle (?? UNUSED ??)
        Menu Level 3 - 4: Intensify: Invert Bargraph | Alt button: Toggle Bargraph Overheat Blinking enabled/disabled
        Menu Level 3 - 3: Intensify: Bargraph Animation Toggle setting: Super Hero / Bargraph Original / System Default | Alt button: Bargraph Firing Animation Toggle setting: Super Hero / Bargraph Original / System Default
        Menu Level 3 - 2: Intensify: Demo Light Mode Enabled | Alt button: Toggle between 1 or 3 LEDs for the Cyclotron (1984/1989 mode)
        Menu Level 3 - 1: Intensify: Toggle between Super Hero and Original Mode. | Alt button: Toggle (?? UNUSED ??)

        EEPROM Menu Level 4: (duration ranges from 1 second to 60 seconds)
        Menu 5: Intensify: Increase overheat duration by 1 second : Power Mode 5 | Alt Wing Button: Decrease overheat duration by 1 second : Power Mode 5
        Menu 4: Intensify: Increase overheat duration by 1 second : Power Mode 4 | Alt Wing Button: Decrease overheat duration by 1 second : Power Mode 4
        Menu 3: Intensify: Increase overheat duration by 1 second : Power Mode 3 | Alt Wing Button: Decrease overheat duration by 1 second : Power Mode 3
        Menu 2: Intensify: Increase overheat duration by 1 second : Power Mode 2 | Alt Wing Button: Decrease overheat duration by 1 second : Power Mode 2
        Menu 1: Intensify: Increase overheat duration by 1 second : Power Mode 1 | Alt Wing Button: Decrease overheat duration by 1 second : Power Mode 1

        EEPROM Menu Level 5:
        Menu 5: Intensify: Enable/Disable overheat in power mode #5 | Alt Wing Button: Enable/Disable continuous smoke in power mode #5
        Menu 4: Intensify: Enable/Disable overheat in power mode #4 | Alt Wing Button: Enable/Disable continuous smoke in power mode #4
        Menu 3: Intensify: Enable/Disable overheat in power mode #3 | Alt Wing Button: Enable/Disable continuous smoke in power mode #3
        Menu 2: Intensify: Enable/Disable overheat in power mode #2 | Alt Wing Button: Enable/Disable continuous smoke in power mode #2
        Menu 1: Intensify: Enable/Disable overheat in power mode #1 | Alt Wing Button: Enable/Disable continuous smoke in power mode #1
      */
      settingsBlinkingLights();

      switch(i_wand_menu) {
        // Menu Level 1: Intensify: Clear the Neutrona Wand EEPROM settings and exit.
        // Menu Level 1: Barrel Wing Button: Save the current settings to the Neutrona Wand EEPROM and exit.
        // Menu Level 2: Intensify: Quick Vent.
        // Menu Level 2: Barrel Wing Button: Wand Boot Errors.
        // Menu Level 3: Intensify + top dial: Default main system volume.
        // Menu Level 3: Barrel Wing Button: Set Neutrona Wand to 1984/1989 Mode | Set Neutrona Wand to 2021 Mode | Default (Matches the Proton Pack)
        // Menu Level 4: Intensify + top dial: Adjust overheat smoke duration by 1 second : Power Mode 5
        // Menu Level 4: Barrel Wing Button + top dial: Adjust overheat start timer by 1 second : Power Mode 5
        // Menu Level 5: Intensify: Enable/Disable overheat in power mode #5
        // Menu Level 5: Barrel Wing Button: Enable/Disable continuous smoke in power mode #5
        case 5:
          // Tell the Neutrona Wand to clear the EEPROM settings and exit.
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              // Tell the Proton Pack to clear its current configuration from the EEPROM.
              // Proton Stream Impact Effects / 3 LED mode in 1984/1989
              wandSerialSend(W_CLEAR_CONFIG_EEPROM_SETTINGS);

              stopEffect(S_VOICE_EEPROM_ERASE);
              playEffect(S_VOICE_EEPROM_ERASE);

              // Clear wand EEPROM. (CTS/VGA, Overheating)
              clearEEPROM();

              wandExitEEPROMMenu();
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
              if(b_quick_vent == true) {
                b_quick_vent = false;

                stopEffect(S_VOICE_QUICK_VENT_DISABLED);
                stopEffect(S_VOICE_QUICK_VENT_ENABLED);
                playEffect(S_VOICE_QUICK_VENT_DISABLED);

                wandSerialSend(W_QUICK_VENT_DISABLED);
              }
              else {
                b_quick_vent = true;

                stopEffect(S_VOICE_QUICK_VENT_DISABLED);
                stopEffect(S_VOICE_QUICK_VENT_ENABLED);
                playEffect(S_VOICE_QUICK_VENT_ENABLED);

                wandSerialSend(W_QUICK_VENT_ENABLED);
              }
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_3) {
              // Main system volume adjustment.
              // Adjustment is handled in checkRotary();
              stopEffect(S_VOICE_DEFAULT_SYSTEM_VOLUME_ADJUSTMENT);
              playEffect(S_VOICE_DEFAULT_SYSTEM_VOLUME_ADJUSTMENT);

              wandSerialSend(W_SOUND_DEFAULT_SYSTEM_VOLUME_ADJUSTMENT);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Overheat smoke duration level 5.
              // Adjustment is handled in checkRotary();
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);
              playEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);

              wandSerialSend(W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_5);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              if(b_overheat_mode_5 == true) {
                b_overheat_mode_5 = false;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_5_DISABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_5_ENABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_5_DISABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_5_DISABLED);
              }
              else {
                b_overheat_mode_5 = true;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_5_ENABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_5_DISABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_5_ENABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_5_ENABLED);
              }
            }
          }
          else if(switchMode() == true) {
            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              // Tell the Proton Pack to save its current configuration to the EEPROM.
              wandSerialSend(W_SAVE_CONFIG_EEPROM_SETTINGS);

              stopEffect(S_VOICE_EEPROM_SAVE);
              playEffect(S_VOICE_EEPROM_SAVE);

              // Save wand EEPROM. (CTS/VGA, Overheating)
              saveEEPROM();

              wandExitEEPROMMenu();
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
              if(b_wand_boot_errors == true) {
                b_wand_boot_errors = false;

                stopEffect(S_VOICE_BOOTUP_ERRORS_DISABLED);
                stopEffect(S_VOICE_BOOTUP_ERRORS_ENABLED);
                playEffect(S_VOICE_BOOTUP_ERRORS_DISABLED);

                wandSerialSend(W_BOOTUP_ERRORS_DISABLED);
              }
              else {
                b_wand_boot_errors = true;

                stopEffect(S_VOICE_BOOTUP_ERRORS_ENABLED);
                stopEffect(S_VOICE_BOOTUP_ERRORS_DISABLED);
                playEffect(S_VOICE_BOOTUP_ERRORS_ENABLED);

                wandSerialSend(W_BOOTUP_ERRORS_ENABLED);
              }
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_3) {
              switch(WAND_YEAR_MODE) {
                case YEAR_1984:
                  WAND_YEAR_MODE = YEAR_1989;

                  stopEffect(S_VOICE_NEUTRONA_WAND_1984);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1989);
                  stopEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);

                  playEffect(W_NEUTRONA_WAND_1989_MODE);
                break;

                case YEAR_1989:
                  WAND_YEAR_MODE = YEAR_AFTERLIFE;

                  stopEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1984);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1989);

                  playEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);

                  wandSerialSend(W_NEUTRONA_WAND_AFTERLIFE_MODE);
                break;

                case YEAR_AFTERLIFE:
                  WAND_YEAR_MODE = YEAR_DEFAULT;

                  stopEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1984);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1989);

                  playEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);

                  wandSerialSend(W_NEUTRONA_WAND_DEFAULT_MODE);
                break;

                case YEAR_DEFAULT:
                case YEAR_FROZEN_EMPIRE:
                default:
                  WAND_YEAR_MODE = YEAR_1984;

                  stopEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1984);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1989);

                  playEffect(S_VOICE_NEUTRONA_WAND_1984);

                  wandSerialSend(W_NEUTRONA_WAND_1984_MODE);
                break;
              }
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Handled in checkRotary();
              // The time it takes to overheat in power level 5.
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_5);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_4);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_3);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_2);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_1);
              playEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_5);

              wandSerialSend(W_SOUND_OVERHEAT_START_TIMER_LEVEL_5);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              wandSerialSend(W_CONTINUOUS_SMOKE_TOGGLE_5);
            }
          }
        break;

        // Menu Level 1: Intensify: Cycle through the modes (Video Game, Cross The Streams, Cross The Streams Mix)
        // Menu Level 1: Barrel Wing Button: Enable Spectral and Holiday modes.
        // Menu Level 2: Intensify: Vent Light Auto Intensity.
        // Menu Level 2: Barrel Wing Button: 5 / 48 / 60 barrel LEDs.
        // Menu Level 3: Intensify: Invert Bargraph
        // Menu Level 3: Barrel Wing Button: Toggle Bargraph Overheat Blinking enabled/disabled
        // Menu Level 4: Intensify + top dial: Adjust overheat smoke duration by 1 second : Power Mode 4
        // Menu Level 4: Barrel Wing Button + top dial: Adjust overheat start timer by 1 second : Power Mode 4
        // Menu Level 5: Intensify: Enable/Disable overheat in power mode #4
        // Menu Level 5: Barrel Wing Button: Enable/Disable continuous smoke in power mode #4
        case 4:
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              toggleWandModes();
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
              if(b_vent_light_control == true) {
                b_vent_light_control= false;

                stopEffect(S_VOICE_VENT_LIGHT_INTENSITY_DISABLED);
                stopEffect(S_VOICE_VENT_LIGHT_INTENSITY_ENABLED);
                playEffect(S_VOICE_VENT_LIGHT_INTENSITY_DISABLED);

                wandSerialSend(W_VENT_LIGHT_INTENSITY_DISABLED);
              }
              else {
                b_vent_light_control = true;

                stopEffect(S_VOICE_VENT_LIGHT_INTENSITY_ENABLED);
                stopEffect(S_VOICE_VENT_LIGHT_INTENSITY_DISABLED);
                playEffect(S_VOICE_VENT_LIGHT_INTENSITY_ENABLED);

                wandSerialSend(W_VENT_LIGHT_INTENSITY_ENABLED);
              }
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_3) {
              if(b_bargraph_invert == true) {
                b_bargraph_invert = false;

                stopEffect(S_VOICE_BARGRAPH_INVERTED);
                stopEffect(S_VOICE_BARGRAPH_NOT_INVERTED);
                playEffect(S_VOICE_BARGRAPH_NOT_INVERTED);

                wandSerialSend(W_BARGRAPH_NOT_INVERTED);
              }
              else {
                b_bargraph_invert = true;

                stopEffect(S_VOICE_BARGRAPH_INVERTED);
                stopEffect(S_VOICE_BARGRAPH_NOT_INVERTED);
                playEffect(S_VOICE_BARGRAPH_INVERTED);

                wandSerialSend(W_BARGRAPH_INVERTED);
              }

              setBargraphOrientation();
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Overheat smoke duration level 4.
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);
              playEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);

              // Handled in checkRotary();
              wandSerialSend(W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_4);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              if(b_overheat_mode_4 == true) {
                b_overheat_mode_4 = false;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_4_DISABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_4_ENABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_4_DISABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_4_DISABLED);
              }
              else {
                b_overheat_mode_4 = true;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_4_ENABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_4_DISABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_4_ENABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_4_ENABLED);
              }
            }
          }

          if(switchMode() == true) {
            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              if(b_spectral_mode_enabled == false || b_holiday_mode_enabled == false || b_spectral_custom_mode_enabled == false) {
                // Enable the spectral modes.
                b_spectral_mode_enabled = true;
                b_holiday_mode_enabled = true;
                b_spectral_custom_mode_enabled = true;

                stopEffect(S_VOICE_SPECTRAL_MODES_DISABLED);
                stopEffect(S_VOICE_SPECTRAL_MODES_ENABLED);
                playEffect(S_VOICE_SPECTRAL_MODES_ENABLED);

                wandSerialSend(W_SPECTRAL_MODES_ENABLED);
              }
              else {
                // Disable the spectral modes.
                b_spectral_mode_enabled = false;
                b_holiday_mode_enabled = false;
                b_spectral_custom_mode_enabled = false;

                stopEffect(S_VOICE_SPECTRAL_MODES_DISABLED);
                stopEffect(S_VOICE_SPECTRAL_MODES_ENABLED);
                playEffect(S_VOICE_SPECTRAL_MODES_DISABLED);

                wandSerialSend(W_SPECTRAL_MODES_DISABLED);
              }
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
              switch(i_num_barrel_leds) {
                case 5:
                default:
                  i_num_barrel_leds = 48;

                  stopEffect(S_VOICE_BARREL_LED_5);
                  stopEffect(S_VOICE_BARREL_LED_48);
                  stopEffect(S_VOICE_BARREL_LED_60);
                  playEffect(S_VOICE_BARREL_LED_48);

                  wandSerialSend(W_BARREL_LEDS_48);
                break;

                // 48 LED wand barrel board coming soon.
                case 48:
                  i_num_barrel_leds = 5;

                  stopEffect(S_VOICE_BARREL_LED_5);
                  stopEffect(S_VOICE_BARREL_LED_48);
                  stopEffect(S_VOICE_BARREL_LED_60);
                  playEffect(S_VOICE_BARREL_LED_5);

                  wandSerialSend(W_BARREL_LEDS_5);
                  /*
                  i_num_barrel_leds = 60;

                  stopEffect(S_VOICE_BARREL_LED_5);
                  stopEffect(S_VOICE_BARREL_LED_48);
                  stopEffect(S_VOICE_BARREL_LED_60);
                  playEffect(S_VOICE_BARREL_LED_60);

                  wandSerialSend(W_BARREL_LEDS_60);
                  */
                break;

                /*
                // The 60 LED flexi-pcb may come one day in the future.
                case 60:
                default:
                  i_num_barrel_leds = 5;

                  stopEffect(S_VOICE_BARREL_LED_5);
                  stopEffect(S_VOICE_BARREL_LED_48);
                  stopEffect(S_VOICE_BARREL_LED_60);
                  playEffect(S_VOICE_BARREL_LED_5);

                  wandSerialSend(W_BARREL_LEDS_5);
                break;
                */
              }
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_3) {
              // Toggle Bargraph Overheat Blinking enabled/disabled
              if(b_overheat_bargraph_blink == true) {
                b_overheat_bargraph_blink = false;

                stopEffect(S_VOICE_BARGRAPH_OVERHEAT_BLINK_DISABLED);
                stopEffect(S_VOICE_BARGRAPH_OVERHEAT_BLINK_ENABLED);
                playEffect(S_VOICE_BARGRAPH_OVERHEAT_BLINK_DISABLED);

                wandSerialSend(W_BARGRAPH_OVERHEAT_BLINK_DISABLED);
              }
              else {
                b_overheat_bargraph_blink = true;

                stopEffect(S_VOICE_BARGRAPH_OVERHEAT_BLINK_DISABLED);
                stopEffect(S_VOICE_BARGRAPH_OVERHEAT_BLINK_ENABLED);
                playEffect(S_VOICE_BARGRAPH_OVERHEAT_BLINK_ENABLED);

                wandSerialSend(W_BARGRAPH_OVERHEAT_BLINK_ENABLED);
              }
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Handled in checkRotary();
              // The time it takes to overheat in power level 4.
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_5);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_4);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_3);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_2);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_1);
              playEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_4);

              wandSerialSend(W_SOUND_OVERHEAT_START_TIMER_LEVEL_4);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              wandSerialSend(W_CONTINUOUS_SMOKE_TOGGLE_4);
            }
          }
        break;

        // Menu Level 1: Intensify: Enable or Disable overheating settings.
        // Menu Level 1: Barrel Wing Button: Enable or disable smoke.
        // Menu Level 2: Intensify: Enable/Disable MODE_ORIGINAL toggle switch sound effects.
        // Menu Level 2: Barrel Wing Button: Cycle through VG color modes to disable them. (see operational guide for more details on this).
        // Menu Level 3: Intensify: Bargraph Animation Toggle setting: Super Hero / Bargraph Original / System Default
        // Menu Level 3: Barrel Wing Button: Bargraph Firing Animation Toggle setting: Super Hero / Bargraph Original / System Default
        // Menu Level 4: Intensify + top dial: Adjust overheat smoke duration by 1 second : Power Mode 3
        // Menu Level 4: Barrel Wing Button + top dial: Adjust overheat start timer by 1 second : Power Mode 3
        // Menu Level 5: Intensify: Enable/Disable overheat in power mode #3
        // Menu Level 5: Barrel Wing Button: Enable/Disable continuous smoke in power mode #3
        case 3:
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              toggleOverHeating();
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
              // Enable/Disable MODE_ORIGINAL toggle switch sound effects. b_mode_original_toggle_sounds_enabled
              if(b_mode_original_toggle_sounds_enabled == true) {
                b_mode_original_toggle_sounds_enabled = false;

                stopEffect(S_VOICE_MODE_ORIGINAL_TOGGLE_SOUNDS_DISABLED);
                stopEffect(S_VOICE_MODE_ORIGINAL_TOGGLE_SOUNDS_ENABLED);
                playEffect(S_VOICE_MODE_ORIGINAL_TOGGLE_SOUNDS_DISABLED);

                wandSerialSend(W_MODE_ORIGINAL_TOGGLE_SOUNDS_DISABLED);
              }
              else {
                b_mode_original_toggle_sounds_enabled = true;

                stopEffect(S_VOICE_MODE_ORIGINAL_TOGGLE_SOUNDS_DISABLED);
                stopEffect(S_VOICE_MODE_ORIGINAL_TOGGLE_SOUNDS_ENABLED);
                playEffect(S_VOICE_MODE_ORIGINAL_TOGGLE_SOUNDS_ENABLED);

                wandSerialSend(W_MODE_ORIGINAL_TOGGLE_SOUNDS_ENABLED);
              }
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_3) {
              switch(BARGRAPH_MODE_EEPROM) {
                case BARGRAPH_EEPROM_ORIGINAL:
                  BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_SUPER_HERO;

                  stopEffect(S_VOICE_DEFAULT_BARGRAPH);
                  stopEffect(S_VOICE_SUPER_HERO_BARGRAPH);
                  stopEffect(S_VOICE_MODE_ORIGINAL_BARGRAPH);
                  playEffect(S_VOICE_SUPER_HERO_BARGRAPH);

                  wandSerialSend(W_SUPER_HERO_BARGRAPH);
                break;

                case BARGRAPH_EEPROM_SUPER_HERO:
                  BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_DEFAULT;

                  stopEffect(S_VOICE_DEFAULT_BARGRAPH);
                  stopEffect(S_VOICE_MODE_ORIGINAL_BARGRAPH);
                  stopEffect(S_VOICE_SUPER_HERO_BARGRAPH);
                  playEffect(S_VOICE_DEFAULT_BARGRAPH);

                  wandSerialSend(W_DEFAULT_BARGRAPH);
                break;

                case BARGRAPH_EEPROM_DEFAULT:
                default:
                  BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_ORIGINAL;

                  stopEffect(S_VOICE_DEFAULT_BARGRAPH);
                  stopEffect(S_VOICE_MODE_ORIGINAL_BARGRAPH);
                  stopEffect(S_VOICE_SUPER_HERO_BARGRAPH);
                  playEffect(S_VOICE_MODE_ORIGINAL_BARGRAPH);

                  wandSerialSend(W_MODE_ORIGINAL_BARGRAPH);
                break;
              }

              // Reset the bargraph.
              bargraphYearModeUpdate();
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Overheat smoke duration level .
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);
              playEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);

              // Handled in checkRotary();
              wandSerialSend(W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_3);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              if(b_overheat_mode_3 == true) {
                b_overheat_mode_3 = false;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_3_DISABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_3_ENABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_3_DISABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_3_DISABLED);
              }
              else {
                b_overheat_mode_3 = true;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_3_ENABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_3_DISABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_3_ENABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_3_ENABLED);
              }
            }
          }

          if(switchMode() == true) {
            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              // Enable or disable smoke.
              wandSerialSend(W_SMOKE_TOGGLE);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
              // Enable or disable video game colours for the Power Cell, Cyclotron etc.
              wandSerialSend(W_VIDEO_GAME_MODE_COLOUR_TOGGLE);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_3) {
              switch(BARGRAPH_EEPROM_FIRING_ANIMATION) {
                case BARGRAPH_EEPROM_ORIGINAL:
                  BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_SUPER_HERO;

                  stopEffect(S_VOICE_SUPER_HERO_FIRING_ANIMATIONS_BARGRAPH);
                  stopEffect(S_VOICE_DEFAULT_FIRING_ANIMATIONS_BARGRAPH);
                  stopEffect(S_VOICE_MODE_ORIGINAL_FIRING_ANIMATIONS_BARGRAPH);
                  playEffect(S_VOICE_SUPER_HERO_FIRING_ANIMATIONS_BARGRAPH);

                  wandSerialSend(W_SUPER_HERO_FIRING_ANIMATIONS_BARGRAPH);
                break;

                case BARGRAPH_EEPROM_SUPER_HERO:
                  BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_DEFAULT;

                  stopEffect(S_VOICE_DEFAULT_FIRING_ANIMATIONS_BARGRAPH);
                  stopEffect(S_VOICE_MODE_ORIGINAL_FIRING_ANIMATIONS_BARGRAPH);
                  stopEffect(S_VOICE_SUPER_HERO_FIRING_ANIMATIONS_BARGRAPH);
                  playEffect(S_VOICE_DEFAULT_FIRING_ANIMATIONS_BARGRAPH);

                  wandSerialSend(W_DEFAULT_FIRING_ANIMATIONS_BARGRAPH);
                break;

                case BARGRAPH_EEPROM_DEFAULT:
                default:
                  BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_ORIGINAL;

                  stopEffect(S_VOICE_DEFAULT_FIRING_ANIMATIONS_BARGRAPH);
                  stopEffect(S_VOICE_MODE_ORIGINAL_FIRING_ANIMATIONS_BARGRAPH);
                  stopEffect(S_VOICE_SUPER_HERO_FIRING_ANIMATIONS_BARGRAPH);
                  playEffect(S_VOICE_MODE_ORIGINAL_FIRING_ANIMATIONS_BARGRAPH);

                  wandSerialSend(W_MODE_ORIGINAL_FIRING_ANIMATIONS_BARGRAPH);
                break;
              }

              // Reset the bargraph.
              bargraphYearModeUpdate();
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Handled in checkRotary();
              // The time it takes to overheat in power level 3.
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_5);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_4);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_3);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_2);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_1);
              playEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_3);

              wandSerialSend(W_SOUND_OVERHEAT_START_TIMER_LEVEL_3);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              wandSerialSend(W_CONTINUOUS_SMOKE_TOGGLE_3);
            }
          }
        break;

        // Menu Level 1: Intensify: Change the Cyclotron direction.
        // Menu Level 1: Barrel Wing Button: Enable the simulation of a ring for the Cyclotron lid.
        // Menu Level 2: Intensify: Overheat strobe.
        // Menu Level 2: Barrel Wing Button: Overheat lights off.
        // Menu Level 3: Intensify: Demo Light Mode Enabled
        // Menu Level 3: Barrel Wing Button: Toggle between 1 or 3 LEDs for the Cyclotron (1984/1989 mode)
        // Menu Level 4: Intensify + top dial: Adjust overheat smoke duration by 1 second : Power Mode 2
        // Menu Level 4: Barrel Wing Button + top dial: Adjust overheat start timer by 1 second : Power Mode 2
        // Menu Level 5: Intensify: Enable/Disable overheat in power mode #2
        // Menu Level 5: Barrel Wing Button: Enable/Disable continuous smoke in power mode #2
        case 2:
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              // Tell the Proton Pack to change the Cyclotron rotation direction.
              wandSerialSend(W_CYCLOTRON_DIRECTION_TOGGLE);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
              // Sub menu.
              wandSerialSend(W_OVERHEAT_STROBE_TOGGLE);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_3) {
              wandSerialSend(W_DEMO_LIGHT_MODE_TOGGLE);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Overheat smoke duration level 2.
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);
              playEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);

              // Handled in checkRotary();
              wandSerialSend(W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_2);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              if(b_overheat_mode_2 == true) {
                b_overheat_mode_2 = false;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_2_DISABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_2_ENABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_2_DISABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_2_DISABLED);
              }
              else {
                b_overheat_mode_2 = true;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_2_ENABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_2_DISABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_2_ENABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_2_ENABLED);
              }
            }
          }

          // Barrel Wing Button: Enable/Disable Ring Simulation in the Cyclotron LEDs in Afterlife (2021) mode.
          if(switchMode() == true) {
            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              wandSerialSend(W_CYCLOTRON_SIMULATE_RING_TOGGLE);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
              // Sub menu.
              wandSerialSend(W_OVERHEAT_LIGHTS_OFF_TOGGLE);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_3) {
              // Tell the Proton Pack to toggle the Single LED or 3 LEDs for 1984/1989 modes.
              wandSerialSend(W_CYCLOTRON_LED_TOGGLE);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Handled in checkRotary();
              // The time it takes to overheat in power level 2.
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_5);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_4);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_3);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_2);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_1);
              playEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_2);

              wandSerialSend(W_SOUND_OVERHEAT_START_TIMER_LEVEL_2);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              wandSerialSend(W_CONTINUOUS_SMOKE_TOGGLE_2);
            }
          }
        break;

        // Menu Level 1: Intensify: Enable or disable Proton Stream Impact Effects.
        // Menu Level 1: Barrel Wing Button: Enable or disable extra Neutrona Wand Sounds.
        // Menu Level 2: Intensify: 1984 / 1989 / Afterlife / Default (Proton Pack toggle switch) year mode selection.
        // Menu Level 2: Barrel Wing Button: Overheat sync to fan.
        // Menu Level 3: Intensify: Toggle between Super Hero and Original Mode.
        // Menu Level 3: Barrel Wing Button: Toggle CTS between: 1984/1989 CTS | 2021 CTS | Default CTS (Based on the year you are in)
        // Menu Level 4: Intensify + top dial: Adjust overheat smoke duration by 1 second : Power Mode 1
        // Menu Level 4: Barrel Wing Button + top dial: Adjust overheat start timer by 1 second : Power Mode 1
        // Menu Level 5: Intensify: Enable/Disable overheat in power mode #1
        // Menu Level 5: Barrel Wing Button: Enable/Disable continuous smoke in power mode #1
        case 1:
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              // Tell the Proton Pack to toggle the Proton Stream impact effects.
              wandSerialSend(W_PROTON_STREAM_IMPACT_TOGGLE);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
              // Sub menu.
              wandSerialSend(W_YEAR_MODES_CYCLE_EEPROM);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_3) {
              switch(WAND_YEAR_CTS) {
                case CTS_1984:
                case CTS_1989:
                  WAND_YEAR_CTS = CTS_AFTERLIFE;

                  stopEffect(S_VOICE_CTS_AFTERLIFE);
                  stopEffect(S_VOICE_CTS_1984);
                  //stopEffect(S_VOICE_CTS_1989);
                  //stopEffect(S_VOICE_CTS_FROZEN_EMPIRE);
                  stopEffect(S_VOICE_CTS_DEFAULT);

                  playEffect(S_VOICE_CTS_AFTERLIFE);

                  wandSerialSend(W_CTS_AFTERLIFE);
                break;

                case CTS_AFTERLIFE:
                case CTS_FROZEN_EMPIRE:
                  WAND_YEAR_CTS = CTS_DEFAULT;

                  stopEffect(S_VOICE_CTS_DEFAULT);
                  stopEffect(S_VOICE_CTS_AFTERLIFE);
                  stopEffect(S_VOICE_CTS_1984);
                  //stopEffect(S_VOICE_CTS_1989);
                  //stopEffect(S_VOICE_CTS_FROZEN_EMPIRE);

                  playEffect(S_VOICE_CTS_DEFAULT);

                  wandSerialSend(W_CTS_DEFAULT);
                break;

                case CTS_DEFAULT:
                default:
                  WAND_YEAR_CTS = CTS_1984;

                  stopEffect(S_VOICE_CTS_1984);
                  stopEffect(S_VOICE_CTS_DEFAULT);
                  stopEffect(S_VOICE_CTS_AFTERLIFE);
                  //stopEffect(S_VOICE_CTS_1989);
                  //stopEffect(S_VOICE_CTS_FROZEN_EMPIRE);

                  playEffect(S_VOICE_CTS_1984);

                  wandSerialSend(W_CTS_1984);
                break;
              }

            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Overheat smoke duration level 1.
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);
              playEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);

              // Handled in checkRotary();
              wandSerialSend(W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_1);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              if(b_overheat_mode_1 == true) {
                b_overheat_mode_1 = false;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_1_DISABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_1_ENABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_1_DISABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_1_DISABLED);
              }
              else {
                b_overheat_mode_1 = true;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_1_ENABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_1_DISABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_1_ENABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_1_ENABLED);
              }
            }
          }

          if(switchMode() == true) {
            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              if(b_extra_pack_sounds == true) {
                b_extra_pack_sounds = false;

                playEffect(S_VOICE_NEUTRONA_WAND_SOUNDS_DISABLED);

                wandSerialSend(W_VOICE_NEUTRONA_WAND_SOUNDS_DISABLED);
              }
              else {
                b_extra_pack_sounds = true;

                playEffect(S_VOICE_NEUTRONA_WAND_SOUNDS_ENABLED);

                wandSerialSend(W_VOICE_NEUTRONA_WAND_SOUNDS_ENABLED);
              }
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
              wandSerialSend(W_OVERHEAT_SYNC_TO_FAN_TOGGLE);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_3) {
              // Toggle between Super Hero and Mode Original.
              wandSerialSend(W_MODE_TOGGLE);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Handled in checkRotary();
              // The time it takes to overheat in power level 1.
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_5);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_4);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_3);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_2);
              stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_1);
              playEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_1);

              wandSerialSend(W_SOUND_OVERHEAT_START_TIMER_LEVEL_1);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              wandSerialSend(W_CONTINUOUS_SMOKE_TOGGLE_1);
            }
          }
        break;
      }
    break;

    case ACTION_SETTINGS:
      settingsBlinkingLights();

      switch(i_wand_menu) {
        // Menu Level 1: Music track loop setting.
        // Menu Level 2: Enable or disable crossing the streams / video game modes.
        // Menu Level 2: (Barrel Wing Button) -> Enable/Disable Video Game Colour Modes for the Proton Pack LEDs (when video game mode is selected).
        case 5:
        // Music track loop setting.
        if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            if(i_music_count > 0) {
              if(b_repeat_track == false) {
                // Loop the track.
                b_repeat_track = true;
                w_trig.trackLoop(i_current_music_track, 1);
              }
              else {
                b_repeat_track = false;
                w_trig.trackLoop(i_current_music_track, 0);
              }
            }

            // Tell pack to loop the music track.
            wandSerialSend(W_MUSIC_TRACK_LOOP_TOGGLE);
          }
        }
        else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            toggleWandModes();
          }

          // Enable/Disable Video Game Colour Modes for the Proton Pack LEDs.
          if(switchMode() == true) {
            if(b_cross_the_streams != true && b_cross_the_streams_mix != true) {
              // Tell the Proton Pack to cycle through the Video Game Colour toggles.
              wandSerialSend(W_VIDEO_GAME_MODE_COLOUR_TOGGLE);
            }
          }
        }
        break;

        // Menu Level 1: (Intensify + Top dial) Adjust the LED dimming of the Power Cell, Cyclotron and Inner Cyclotron.
        // Menu Level 1: (Barrel Wing Button) Cycle through which dimming mode to adjust in the Proton Pack. Power Cell, Cyclotron, Inner Cyclotron.
        // Menu Level 2: Enable or disable smoke for the Proton Pack.
        // Menu Level 2: (Barrel Wing Button) -> Enable or disable overheating.
        case 4:
          // Adjust the Proton Pack / Neutrona Wand sound effects volume.
          if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
            // Cycle through the dimming modes in the Proton Pack. (Power Cell, Cyclotron and Inner Cyclotron). Actual control of the dimming is handled in checkRotary().
            if(switchMode() == true) {
              // Tell the Proton Pack to change to the next dimming mode.
              wandSerialSend(W_DIMMING_TOGGLE);
            }
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
            // Enable or disable smoke for the Proton Pack.
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              // Tell the Proton Pack to toggle the smoke on or off.
              wandSerialSend(W_SMOKE_TOGGLE);
            }

            // Enable or disable overheating.
            if(switchMode() == true) {
              toggleOverHeating();
            }
          }
        break;

        // Menu Level 1: (Intensify + top dial) Adjust Proton Pack / Neutrona Wand sound effects. (Barrel Wing Button + top dial) Adjust Proton Pack / Neutrona Wand music volume.
        // Menu Level 1: Toggle Cyclotron rotation direction.
        // Menu Level 2: (Barrel Wing Button) -> Toggle the Proton Pack Single LED or 3 LEDs for 1984/1989 modes.
        case 3:
          // Top menu code is handled in checkRotary();
          // Sub menu. Adjust Cyclotron settings.
          if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              // Tell the Proton Pack to change the Cyclotron rotation direction.
              wandSerialSend(W_CYCLOTRON_DIRECTION_TOGGLE);
            }

            if(switchMode() == true) {
              // Tell the Proton Pack to toggle the Single LED or 3 LEDs for 1984/1989 modes.
              wandSerialSend(W_CYCLOTRON_LED_TOGGLE);
            }
          }
        break;

        // Menu Level 1: Change music tracks.
        // Menu Level 2: Enable pack vibration, enable pack vibration while firing only, disable pack vibration. *Note that the pack vibration switch will toggle both pack and wand vibiration on or off*
        // Menu Level 2: (Barrel Wing Button) -> Enable wand vibration, enable wand vibration while firing only, disable wand vibration.
        case 2:
          // Change music tracks.
          if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              if(b_playing_music == true) {
                stopMusic();
              }

              // Tell the pack to play the next track.
              wandSerialSend(W_MUSIC_NEXT_TRACK);
            }

            if(switchMode() == true) {
              if(b_playing_music == true) {
                stopMusic();
              }

              // Tell the pack to play the next track.
              wandSerialSend(W_MUSIC_PREV_TRACK);
            }
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
            // Enable or disable vibration for the pack or during firing only.
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              stopEffect(S_BEEPS_ALT);
              playEffect(S_BEEPS_ALT);

              wandSerialSend(W_VIBRATION_CYCLE_TOGGLE);
            }

            // Enable or disable vibration or firing vibration only for the wand.
            if(switchMode() == true) {
              stopEffect(S_BEEPS_ALT);
              playEffect(S_BEEPS_ALT);

              if(b_vibration_on != true) {
                b_vibration_on = true;
                b_vibration_enabled = true; // Override the Proton Pack vibration toggle switch.

                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

                playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);

                wandSerialSend(W_VIBRATION_ENABLED);

                analogWrite(vibration, 150);
                delay(250);
                analogWrite(vibration,0);
              }
              else if(b_vibration_on == true && b_vibration_firing != true) {
                b_vibration_firing = true;
                b_vibration_enabled = true; // Override the Proton Pack vibration toggle switch.

                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

                playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);

                wandSerialSend(W_VIBRATION_FIRING_ENABLED);

                analogWrite(vibration, 150);
                delay(250);
                analogWrite(vibration,0);
              }
              else {
                b_vibration_on = false;
                b_vibration_firing = false;

                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

                playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

                wandSerialSend(W_VIBRATION_DISABLED);
              }
            }
          }
        break;

        // Menu Level 1: Play music or stop music.
        // Menu Level 1: (Barrel Wing Button). Mute the Proton Pack and Neutrona Wand.
        // Menu Level 2: (Intensify) -> Switch between 1984/1989/Afterlife mode.
        // Menu Level 2: (Barrel Wing Button) -> Enable or disable Proton Stream impact effects.
        case 1:
          // Play or stop the current music track.
          if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              if(b_playing_music == true) {
                // Tell the pack to stop music.
                wandSerialSend(W_MUSIC_STOP);
              }
              else {
                // Tell the pack to play music.
                wandSerialSend(W_MUSIC_START);
              }
            }

            // Silence the Proton Pack or Neutrona Wand or revert back.
            if(switchMode() == true) {
              if(i_volume_master == i_volume_abs_min) {
                wandSerialSend(W_VOLUME_REVERT);

                i_volume_master = i_volume_revert;
              }
              else {
                i_volume_revert = i_volume_master;

                // Set the master volume to silent.
                i_volume_master = i_volume_abs_min;

                wandSerialSend(W_SILENT_MODE);
              }

              w_trig.masterGain(i_volume_master); // Reset the master gain.
            }
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
            // Switch between 1984/1989/Afterlife mode.
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              // Tell the Proton Pack to cycle through year modes.
              wandSerialSend(W_YEAR_MODES_CYCLE);

              stopEffect(S_BEEPS_BARGRAPH);
              playEffect(S_BEEPS_BARGRAPH);

              // There is no pack connected; let's change the years.
              if(b_no_pack == true) {
                switch(SYSTEM_YEAR) {
                  case SYSTEM_1984:
                    SYSTEM_YEAR = SYSTEM_1989;

                    stopEffect(S_VOICE_FROZEN_EMPIRE);
                    stopEffect(S_VOICE_AFTERLIFE);
                    stopEffect(S_VOICE_1984);
                    stopEffect(S_VOICE_1989);

                    playEffect(S_VOICE_1989);

                    bargraphYearModeUpdate();
                  break;

                  case SYSTEM_1989:
                    SYSTEM_YEAR == SYSTEM_AFTERLIFE;

                    stopEffect(S_VOICE_FROZEN_EMPIRE);
                    stopEffect(S_VOICE_AFTERLIFE);
                    stopEffect(S_VOICE_1984);
                    stopEffect(S_VOICE_1989);

                    playEffect(S_VOICE_AFTERLIFE);

                    bargraphYearModeUpdate();
                  break;

                  case SYSTEM_AFTERLIFE:
                  case SYSTEM_FROZEN_EMPIRE:
                  default:
                    SYSTEM_YEAR = SYSTEM_1984;

                    stopEffect(S_VOICE_FROZEN_EMPIRE);
                    stopEffect(S_VOICE_AFTERLIFE);
                    stopEffect(S_VOICE_1989);
                    stopEffect(S_VOICE_1984);

                    playEffect(S_VOICE_1984);

                    bargraphYearModeUpdate();
                  break;
                }
              }
            }

            if(switchMode() == true) {
              // Tell the Proton Pack to toggle the Proton Stream Impact Effects.
              wandSerialSend(W_PROTON_STREAM_IMPACT_TOGGLE);
            }
          }
        break;
      }
    break;
  }

  switch(WAND_STATUS) {
    case MODE_OFF:
      if(WAND_ACTION_STATUS != ACTION_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU) {
        if(switchMode() == true || b_pack_alarm == true) {
          if(FIRING_MODE != SETTINGS && b_pack_alarm != true && (b_pack_on != true || b_no_pack == true)) {
            playEffect(S_CLICK);

            PREV_FIRING_MODE = FIRING_MODE;
            FIRING_MODE = SETTINGS;

            WAND_ACTION_STATUS = ACTION_SETTINGS;
            WAND_MENU_LEVEL == MENU_LEVEL_1;

            i_wand_menu = 5;
            ms_settings_blinking.start(i_settings_blinking_delay);

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
          if(b_no_pack != true) {
            wandExitMenu();
          }
        }
      }

      // Reset the count of the wand switch
      if(switch_intensify.getState() == HIGH) {
        switch_wand.resetCount();
        switch_vent.resetCount();
      }

      if(WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU && (b_pack_on != true || b_no_pack == true) && switch_intensify.getState() == LOW && switch_wand.getCount() >= 5) {
        stopEffect(S_BEEPS_BARGRAPH);
        playEffect(S_BEEPS_BARGRAPH);

        stopEffect(S_EEPROM_LED_MENU);
        playEffect(S_EEPROM_LED_MENU);

        wandSerialSend(W_EEPROM_LED_MENU);
        wandSerialSend(W_SPECTRAL_LIGHTS_ON);

        i_wand_menu = 5;

        WAND_ACTION_STATUS = ACTION_EEPROM_MENU;
        WAND_MENU_LEVEL == MENU_LEVEL_1;

        ms_settings_blinking.start(i_settings_blinking_delay);

        wandBarrelSpectralCustomConfigOn();

        // Make sure some of the wand lights are off.
        wandLightsOffMenuSystem();
      }
      else if(WAND_ACTION_STATUS == ACTION_EEPROM_MENU && b_pack_on == true) {
        if(b_no_pack != true) {
          wandExitEEPROMMenu();
        }
      }

      if(WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU && (b_pack_on != true || b_no_pack == true) && switch_intensify.getState() == LOW && switch_vent.getCount() >= 5) {
        stopEffect(S_BEEPS_BARGRAPH);
        playEffect(S_BEEPS_BARGRAPH);

        stopEffect(S_EEPROM_CONFIG_MENU);
        playEffect(S_EEPROM_CONFIG_MENU);

        wandSerialSend(W_EEPROM_CONFIG_MENU);

        i_wand_menu = 5;

        WAND_ACTION_STATUS = ACTION_CONFIG_EEPROM_MENU;
        WAND_MENU_LEVEL == MENU_LEVEL_1;

        ms_settings_blinking.start(i_settings_blinking_delay);

        // Make sure some of the wand lights are off.
        wandLightsOffMenuSystem();
      }
      else if(WAND_ACTION_STATUS == ACTION_CONFIG_EEPROM_MENU && b_pack_on == true) {
        if(b_no_pack != true) {
          wandExitEEPROMMenu();
        }
      }

      if(b_pack_alarm == true) {
        if(ms_hat_2.justFinished()) {
          ms_hat_2.start(i_hat_2_delay);
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

        playEffect(S_BEEPS_LOW);
        playEffect(S_BEEPS);
      }

      if(ms_hat_1.justFinished()) {
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
    fireStreamEnd(getHueAsGRB(C_BLACK));
  }

  // Check the Barrel Wing Button button status.
  switchModePressedReset();

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
void toggleOverHeating() {
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
void overHeatingFinished() {
  bargraphClearAlt();

  // Since the Proton Pack tells the Neutrona Wand when overheating is finished. If the Neutrona Wand is running with no pack, then it will use it's own timer to calculate when to finish.
  if(b_no_pack == true) {
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

  playEffect(S_BOOTUP);

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

  // Since the Proton Pack tells the Neutrona Wand when overheating is finished. If it is running with no Proton Pack, then the Neutrona Wand needs to calculate when to finish.
  if(b_no_pack == true) {
    ms_overheating.start(i_ms_overheating);
  }

  soundBeepLoopStop();
  soundIdleStop();
  soundIdleLoopStop();

  b_sound_idle = false; // REMOVE ??
  b_beeping = false;

  // Blinking bargraph option for overheat.
  if(b_overheat_bargraph_blink == true) {
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
  playEffect(S_CLICK);

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
    if(b_repeat_track == true && i_wand_menu == 5 && WAND_ACTION_STATUS != ACTION_OVERHEATING && WAND_ACTION_STATUS != ACTION_ERROR && WAND_MENU_LEVEL == MENU_LEVEL_1 && WAND_ACTION_STATUS != ACTION_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU) {
      b_solid_five = true;
    }

    // Indicator for crossing the streams setting.
    if((b_cross_the_streams == true || b_cross_the_streams_mix == true) && i_wand_menu == 5 && WAND_ACTION_STATUS != ACTION_OVERHEATING && WAND_ACTION_STATUS != ACTION_ERROR && WAND_MENU_LEVEL == MENU_LEVEL_2 && WAND_ACTION_STATUS != ACTION_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU) {
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
  if(ms_intensify_timer.justFinished()) {
    ms_intensify_timer.stop();
  }

  if(ms_switch_mode_firing.justFinished()) {
    ms_switch_mode_firing.stop();
  }

  if(ms_slo_blo_blink.justFinished()) {
    ms_slo_blo_blink.start(i_slo_blo_blink_delay);
  }

  switchBarrel();

  switch(WAND_STATUS) {
    case MODE_OFF:
      switch(SYSTEM_MODE) {
        case MODE_ORIGINAL:
          if(b_pack_ion_arm_switch_on == true) {
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
                        stopEffect(S_BEEPS_ALT);
                        stopEffect(S_BEEP_VARIATION);
                        playEffect(S_BEEPS_ALT);
                      }
                    }
                  }

                  if(switch_vent.getState() == LOW && switch_wand.getState() == LOW) {
                    if(b_mode_original_toggle_sounds_enabled == true) {
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
                    stopEffect(S_WAND_HEATUP_ALT);
                    stopEffect(S_WAND_HEATUP);
                    playEffect(S_WAND_HEATDOWN);
                  }
                  else if((switch_vent.isPressed() || switch_vent.isReleased()) && switch_wand.getState() == LOW && b_mode_original_toggle_sounds_enabled == true) {
                    stopEffect(S_WAND_HEATUP_ALT);
                    stopEffect(S_WAND_HEATUP);
                    playEffect(S_WAND_HEATDOWN);
                  }
                }

                if(switch_vent.getState() == LOW && switch_wand.getState() == LOW) {
                  analogWrite(led_front_left, 255); // The front right orange LED, turn it on.

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

                  analogWrite(led_front_left, 0); // The front right orange LED, turn it off.

                  // Turn off the Neutrona Wand vent lights.
                  digitalWrite(led_vent, HIGH);
                  digitalWrite(led_white, HIGH);
                }
              }
            }
          }
          else {
            if(WAND_ACTION_STATUS != ACTION_CONFIG_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_EEPROM_MENU && WAND_ACTION_STATUS != ACTION_SETTINGS) {
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

  stopEffect(S_BOOTUP);

  if(b_extra_pack_sounds == true) {
    wandSerialSend(W_EXTRA_WAND_SOUNDS_STOP);
  }

  // Turn off any overheating sounds.
  stopEffect(S_CLICK);
  stopEffect(S_VENT_DRY);

  stopEffect(S_FIRE_START_SPARK);
  stopEffect(S_PACK_SLIME_OPEN);
  stopEffect(S_STASIS_START);
  stopEffect(S_MESON_START);

  stopEffect(S_WAND_SHUTDOWN);
  playEffect(S_WAND_SHUTDOWN);

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
                i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_2021;
              }
              else {
                i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_1984;
              }
            break;

            case SYSTEM_1984:
            case SYSTEM_1989:
              i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_1984;
            break;
          }
        break;
      }
  }

  switch_wand.resetCount();
  switch_vent.resetCount();
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
      if(switch_intensify.getState() == LOW && ms_firing_debounce.remaining() < 1 && ms_intensify_timer.isRunning() != true && switch_wand.getState() == LOW && switch_vent.getState() == LOW && switch_activate.getState() == LOW && b_pack_on == true && b_switch_barrel_extended == true && b_pack_alarm != true) {
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

          ms_firing_debounce.start(i_firing_debounce);
        }

        b_firing_intensify = true;
      }

      // When Cross The Streams mode is enabled, video game modes are disabled and the wand menu settings can only be accessed when the Neutrona Wand is powered down.
      if(b_cross_the_streams == true) {
        if(switchMode() == true && switch_wand.getState() == LOW && ms_firing_debounce.remaining() < 1 && ms_switch_mode_firing.isRunning() != true && switch_vent.getState() == LOW && switch_activate.getState() == LOW && b_pack_on == true && b_switch_barrel_extended == true && b_pack_alarm != true) {
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

            ms_firing_debounce.start(i_firing_debounce);
          }

          b_firing_alt = true;
        }
        else if(b_switch_mode_pressed != true) {
          if(b_firing_intensify != true && WAND_ACTION_STATUS == ACTION_FIRING) {
            WAND_ACTION_STATUS = ACTION_IDLE;
          }

          if(b_firing_alt == true) {
            ms_firing_debounce.start(i_firing_debounce);
          }

          b_firing_alt = false;
        }
      }
      else if(b_vg_mode == true) {
        if(FIRING_MODE == PROTON && WAND_ACTION_STATUS == ACTION_FIRING) {
          if(switchMode() == true) {
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

        if(b_firing_intensify == true) {
          ms_firing_debounce.start(i_firing_debounce);
        }

        b_firing_intensify = false;
      }
    }

    if(switch_activate.getState() == HIGH) {
      WAND_ACTION_STATUS = ACTION_OFF;
    }

    // Quick vent feature. When enabled, press intensify while the top right switch on the pack is flipped down will cause the Proton Pack and Neutrona Wand to manually vent.
    if(b_quick_vent == true) {
      if(switch_intensify.getState() == LOW && ms_firing_debounce.remaining() < 1 && ms_intensify_timer.isRunning() != true && switch_wand.getState() == HIGH && switch_vent.getState() == LOW && switch_activate.getState() == LOW && b_pack_on == true && b_switch_barrel_extended == true && b_pack_alarm != true && b_quick_vent == true && b_overheat_enabled == true) {
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
    if(switchMode() == true) {
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

      // If starting up directly from any of the none toggle sequence switches, play the wand heatup sound.
      if(switch_activate.isPressed() != true && switch_activate.isReleased() != true && b_mode_original_toggle_sounds_enabled == true) {
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
        break;
      }
    }

    // Turn on slo-blo light (and front left LED if using a Ardunio Nano).
    analogWrite(led_slo_blo, 255);

    // If using the gpstar Neutrona Wand microcontroller the front left LED is wired separately; let's turn it on.
    analogWrite(led_front_left, 255);

    // Top white light.
    ms_white_light.start(d_white_light_interval);
    digitalWrite(led_white, LOW);

    if(b_pack_alarm != true) {
      switch(getNeutronaWandYearMode()) {
        case SYSTEM_1984:
        case SYSTEM_1989:
          playEffect(S_CLICK);
        break;

        case SYSTEM_AFTERLIFE:
        case SYSTEM_FROZEN_EMPIRE:
        default:
          if(b_no_pack == true) {
            playEffect(S_BOOTUP);
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
        playEffect(S_BOOTUP);

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
}

void soundIdleStop() {
  if(b_sound_idle == true) {
    switch(getNeutronaWandYearMode()) {
      case SYSTEM_1984:
      case SYSTEM_1989:
        if(WAND_ACTION_STATUS != ACTION_OFF) {
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
        stopEffect(S_BOOTUP);
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

    stopEffect(S_AFTERLIFE_BEEP_WAND);
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
      switch(i_power_mode) {
        case 1:
          if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
            playEffect(S_AFTERLIFE_BEEP_WAND_S1, true);
          }
          else {
            playEffect(S_AFTERLIFE_BEEP_WAND_S1);
          }
        break;

        case 2:
         if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
            playEffect(S_AFTERLIFE_BEEP_WAND_S2, true);
          }
          else {
            playEffect(S_AFTERLIFE_BEEP_WAND_S2);
          }
        break;

        case 3:
         if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
            playEffect(S_AFTERLIFE_BEEP_WAND_S3, true);
          }
          else {
            playEffect(S_AFTERLIFE_BEEP_WAND_S3);
          }
        break;

        case 4:
         if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
            playEffect(S_AFTERLIFE_BEEP_WAND_S4, true);
          }
          else {
            playEffect(S_AFTERLIFE_BEEP_WAND_S4);
          }
        break;

        case 5:
         if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
            playEffect(S_AFTERLIFE_BEEP_WAND_S5, true);
          }
          else {
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

  // Some sparks for firing start.
  if(SYSTEM_YEAR == SYSTEM_1989) {
    playEffect(S_FIRE_START_SPARK, false, i_volume_effects - 10);
  }
  else {
    playEffect(S_FIRE_START_SPARK);
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

              if(SYSTEM_YEAR == SYSTEM_1989) {
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

              if(SYSTEM_YEAR == SYSTEM_1989) {
                playEffect(S_GB2_FIRE_START);
              }

              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 1000);
            }
            else {
              b_sound_firing_alt_trigger = false;
            }
          break;

          case 5:
            switch(SYSTEM_YEAR) {
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
      playEffect(S_SLIME_START);
      playEffect(S_SLIME_LOOP, true);
    break;

    case STASIS:
      playEffect(S_STASIS_START);
      playEffect(S_STASIS_LOOP, true);
    break;

    case MESON:
      playEffect(S_MESON_START);
      playEffect(S_MESON_LOOP, true);
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

  if(ms_intensify_timer.isRunning() != true) {
    ms_intensify_timer.start(i_intensify_delay);
  }

  if(ms_switch_mode_firing.isRunning() != true) {
    ms_switch_mode_firing.start(i_intensify_delay);
  }

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
      if(SYSTEM_YEAR == SYSTEM_1989) {
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
      stopEffect(S_MESON_LOOP);
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

  ms_firing_lights.start(10);
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
        switch(SYSTEM_YEAR) {
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

  ms_firing_stream_blue.stop();
  ms_firing_lights.stop();

  ms_impact.stop();

  i_barrel_light = 0;
  ms_firing_lights_end.start(10);

  // If using optional items on the gpstar Neutrona Wand microcontroller.
  digitalWrite(led_hat_1, LOW); // Turn off hat light 1.
  digitalWrite(led_barrel_tip, LOW); // Turn off the wand barrel tip LED.

  ms_hat_1.stop();

  // Stop all other firing sounds.
  switch(FIRING_MODE) {
    case PROTON:
    default:
      if(SYSTEM_YEAR == SYSTEM_1989) {
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
      stopEffect(S_MESON_LOOP);
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
          if(SYSTEM_YEAR == SYSTEM_1989) {
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
          if(SYSTEM_YEAR == SYSTEM_1989) {
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
        switch(SYSTEM_YEAR) {
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
        switch(SYSTEM_YEAR) {
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
        switch(SYSTEM_YEAR) {
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
          fireStreamStart(getHueAsGRB(C_RED));
        break;

        case 2:
          fireStreamStart(getHueAsGRB(C_RED2));
        break;

        case 3:
          fireStreamStart(getHueAsGRB(C_RED3));
        break;

        case 4:
          fireStreamStart(getHueAsGRB(C_RED4));
        break;

        case 5:
          fireStreamStart(getHueAsGRB(C_RED5));
        break;

        default:
          fireStreamStart(getHueAsGRB(C_RED));
        break;
      }

      fireStream(getHueAsGRB(C_BLUE));
    break;

    case SLIME:
      fireStreamStart(getHueAsGRB(C_GREEN));
      fireStream(getHueAsGRB(C_WHITE));
    break;

    case STASIS:
      fireStreamStart(getHueAsGRB(C_BLUE));
      fireStream(getHueAsGRB(C_WHITE));
    break;

    case MESON:
      fireStreamStart(getHueAsGRB(C_YELLOW));
      fireStream(getHueAsGRB(C_RED));
    break;

    case SPECTRAL:
      fireStreamStart(getHueAsGRB(C_RAINBOW));
      fireStream(getHueAsGRB(C_RAINBOW));
    break;

    case HOLIDAY:
      fireStreamStart(getHueAsGRB(C_REDGREEN));
      fireStream(getHueAsGRB(C_REDGREEN));
    break;

    case SPECTRAL_CUSTOM:
      fireStreamStart(getHueAsGRB(C_CUSTOM));

      if(i_spectral_wand_saturation_custom < 254) {
        fireStream(getHueAsGRB(C_BLUE));
      }
      else {
        fireStream(getHueAsGRB(C_WHITE));
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
  if(i_heatup_counter > 100) {
    wandBarrelHeatDown();
  }
  else if(ms_wand_heatup_fade.justFinished() && i_heatup_counter <= 100) {
    switch(FIRING_MODE) {
      case PROTON:
        barrel_leds[i_num_barrel_leds - 1] = getHueAsGRB(C_WHITE, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case SLIME:
        barrel_leds[i_num_barrel_leds - 1] = getHueAsGRB(C_GREEN, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case STASIS:
        barrel_leds[i_num_barrel_leds - 1] = getHueAsGRB(C_BLUE, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case MESON:
        barrel_leds[i_num_barrel_leds - 1] = getHueAsGRB(C_ORANGE, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case SPECTRAL:
        barrel_leds[i_num_barrel_leds - 1] = getHueAsGRB(C_RAINBOW, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case HOLIDAY:
        barrel_leds[i_num_barrel_leds - 1] = getHueAsGRB(C_REDGREEN, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case SPECTRAL_CUSTOM:
        barrel_leds[i_num_barrel_leds - 1] = getHueAsGRB(C_CUSTOM, i_heatup_counter);
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
  if(ms_wand_heatup_fade.justFinished() && i_heatdown_counter > 0) {
    switch(FIRING_MODE) {
      case PROTON:
        barrel_leds[i_num_barrel_leds - 1] = getHueAsGRB(C_WHITE, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case SLIME:
        barrel_leds[i_num_barrel_leds - 1] = getHueAsGRB(C_GREEN, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case STASIS:
        barrel_leds[i_num_barrel_leds - 1] = getHueAsGRB(C_BLUE, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case MESON:
        barrel_leds[i_num_barrel_leds - 1] = getHueAsGRB(C_ORANGE, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case SPECTRAL:
        barrel_leds[i_num_barrel_leds - 1] = getHueAsGRB(C_RAINBOW, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case HOLIDAY:
        barrel_leds[i_num_barrel_leds - 1] = getHueAsGRB(C_REDGREEN, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case SPECTRAL_CUSTOM:
        barrel_leds[i_num_barrel_leds - 1] = getHueAsGRB(C_CUSTOM, i_heatdown_counter);
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

void fireStream(CRGB c_colour) {
  if(ms_firing_stream_blue.justFinished()) {
    if(i_barrel_light - 1 > -1 && i_barrel_light - 1 < i_num_barrel_leds) {
      switch(FIRING_MODE) {
        case PROTON:
        default:
          if(b_firing_cross_streams == true) {
            barrel_leds[i_barrel_light - 1] = getHueAsGRB(C_WHITE);
          }
          else {
            // Shift the stream from red to orange on higher power modes.
            switch(i_power_mode) {
              case 1:
                barrel_leds[i_barrel_light - 1] = getHueAsGRB(C_RED);
              break;

              case 2:
                barrel_leds[i_barrel_light - 1] = getHueAsGRB(C_RED2);
              break;

              case 3:
                barrel_leds[i_barrel_light - 1] = getHueAsGRB(C_RED3);
              break;

              case 4:
                barrel_leds[i_barrel_light - 1] = getHueAsGRB(C_RED4);
              break;

              case 5:
                barrel_leds[i_barrel_light - 1] = getHueAsGRB(C_RED5);
              break;

              default:
                barrel_leds[i_barrel_light - 1] = getHueAsGRB(C_RED);
              break;
            }
          }
        break;

        case SLIME:
          barrel_leds[i_barrel_light - 1] = getHueAsGRB(C_GREEN);
        break;

        case STASIS:
          barrel_leds[i_barrel_light - 1] = getHueAsGRB(C_BLUE);
        break;

        case MESON:
          barrel_leds[i_barrel_light - 1] = getHueAsGRB(C_ORANGE);
        break;

        case SPECTRAL:
          barrel_leds[i_barrel_light - 1] = getHueAsGRB(C_BLACK);
        break;

        case HOLIDAY:
          barrel_leds[i_barrel_light - 1] = getHueAsGRB(C_BLACK);
        break;

        case SPECTRAL_CUSTOM:
          barrel_leds[i_barrel_light - 1] = getHueAsGRB(C_CUSTOM);
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
              ms_firing_stream_blue.start(d_firing_stream);
            break;

            case 2:
              ms_firing_stream_blue.start(d_firing_stream - 15);
            break;

            case 3:
              ms_firing_stream_blue.start(d_firing_stream - 30);
            break;

            case 4:
              ms_firing_stream_blue.start(d_firing_stream - 45);
            break;

            case 5:
              ms_firing_stream_blue.start(d_firing_stream - 60);
            break;

            default:
              ms_firing_stream_blue.start(d_firing_stream);
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
              ms_firing_stream_blue.start(d_firing_lights + 10);
            break;

            case 2:
              ms_firing_stream_blue.start(d_firing_lights + 8);
            break;

            case 3:
              ms_firing_stream_blue.start(d_firing_lights + 6);
            break;

            case 4:
              ms_firing_stream_blue.start(d_firing_lights + 5);
            break;

            case 5:
              ms_firing_stream_blue.start(d_firing_lights + 4);
            break;

            default:
              ms_firing_stream_blue.start(d_firing_lights);
            break;
          }
        break;
      }

      ms_fast_led.start(i_fast_led_delay);

      i_barrel_light++;
    }
  }
}

void barrelLightsOff() {
  ms_wand_heatup_fade.stop();
  i_heatup_counter = 0;
  i_heatdown_counter = 100;

  for(uint8_t i = 0; i < i_num_barrel_leds; i++) {
    barrel_leds[i] = getHueAsGRB(C_BLACK);
  }

  // Turn off the wand barrel tip LED.
  digitalWrite(led_barrel_tip, LOW);

  ms_fast_led.start(i_fast_led_delay);
}

void fireStreamStart(CRGB c_colour) {
  if(ms_firing_lights.justFinished() && i_barrel_light < i_num_barrel_leds) {
    barrel_leds[i_barrel_light] = c_colour;

    ms_fast_led.start(i_fast_led_delay);

    ms_firing_lights.start(d_firing_lights);

    i_barrel_light++;

    if(i_barrel_light == i_num_barrel_leds) {
      i_barrel_light = 0;

      ms_firing_lights.stop();
      ms_firing_stream_blue.start(d_firing_stream);
    }
  }
}

void fireStreamEnd(CRGB c_colour) {
  if(i_barrel_light < i_num_barrel_leds) {
    barrel_leds[i_barrel_light] = c_colour;

    ms_fast_led.start(i_fast_led_delay);

    ms_firing_lights_end.start(d_firing_lights);

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

        digitalWrite(led_barrel_tip, HIGH);
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

        digitalWrite(led_barrel_tip, HIGH);
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

        digitalWrite(led_barrel_tip, LOW);
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

        digitalWrite(led_barrel_tip, LOW);
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

        digitalWrite(led_barrel_tip, HIGH);
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

        digitalWrite(led_barrel_tip, HIGH);
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

        digitalWrite(led_barrel_tip, LOW);
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

        digitalWrite(led_barrel_tip, LOW);
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

        digitalWrite(led_barrel_tip, HIGH);
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

        digitalWrite(led_barrel_tip, HIGH);
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

        digitalWrite(led_barrel_tip, LOW);
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

        digitalWrite(led_barrel_tip, LOW);
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

        digitalWrite(led_barrel_tip, HIGH);
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

        digitalWrite(led_barrel_tip, HIGH);
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

        digitalWrite(led_barrel_tip, HIGH);
      break;

      case 2:
        vibrationWand(i_vibration_level + 112);

        digitalWrite(i_bargraph_5_led[1-1], HIGH);
        digitalWrite(i_bargraph_5_led[2-1], LOW);
        digitalWrite(i_bargraph_5_led[3-1], HIGH);
        digitalWrite(i_bargraph_5_led[4-1], LOW);
        digitalWrite(i_bargraph_5_led[5-1], HIGH);
        i_bargraph_status++;

        digitalWrite(led_barrel_tip, LOW);
      break;

      case 3:
        vibrationWand(i_vibration_level + 115);

        digitalWrite(i_bargraph_5_led[1-1], HIGH);
        digitalWrite(i_bargraph_5_led[2-1], HIGH);
        digitalWrite(i_bargraph_5_led[3-1], LOW);
        digitalWrite(i_bargraph_5_led[4-1], HIGH);
        digitalWrite(i_bargraph_5_led[5-1], HIGH);
        i_bargraph_status++;

        digitalWrite(led_barrel_tip, HIGH);
      break;

      case 4:
        vibrationWand(i_vibration_level + 112);

        digitalWrite(i_bargraph_5_led[1-1], HIGH);
        digitalWrite(i_bargraph_5_led[2-1], LOW);
        digitalWrite(i_bargraph_5_led[3-1], HIGH);
        digitalWrite(i_bargraph_5_led[4-1], LOW);
        digitalWrite(i_bargraph_5_led[5-1], HIGH);
        i_bargraph_status++;

        digitalWrite(led_barrel_tip, LOW);
      break;

      case 5:
        vibrationWand(i_vibration_level + 110);

        digitalWrite(i_bargraph_5_led[1-1], LOW);
        digitalWrite(i_bargraph_5_led[2-1], HIGH);
        digitalWrite(i_bargraph_5_led[3-1], HIGH);
        digitalWrite(i_bargraph_5_led[4-1], HIGH);
        digitalWrite(i_bargraph_5_led[5-1], LOW);
        i_bargraph_status = 1;

        digitalWrite(led_barrel_tip, HIGH);
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
        if(i_bargraph_status_alt >= 0 && i_bargraph_status_alt < i_bargraph_segments) {
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
        if(i_bargraph_status_alt >= 0 && i_bargraph_status_alt < i_bargraph_segments) {
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
  if(WAND_STATUS == MODE_ON && WAND_ACTION_STATUS == ACTION_IDLE || (WAND_STATUS == MODE_OFF && WAND_ACTION_STATUS == ACTION_IDLE && SYSTEM_MODE == MODE_ORIGINAL)) {
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

// Return the year mode that the Neturona Wand is suppose to be in. Or if overridden to be in a different year by the user.
unsigned int getNeutronaWandYearMode() {
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

    case YEAR_DEFAULT:
    case YEAR_FROZEN_EMPIRE:
    default:
      return SYSTEM_YEAR;
    break;
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
  analogWrite(led_front_left, 0); // The front right orange LED.

  digitalWrite(led_hat_1, LOW); // Turn off hat light 1.
  digitalWrite(led_hat_2, LOW); // Turn off hat light 2.
  digitalWrite(led_barrel_tip, LOW); // Turn off the wand barrel tip LED.

  digitalWrite(led_vent, HIGH);
  digitalWrite(led_white, HIGH);

  i_bargraph_status = 0;
  i_bargraph_status_alt = 0;
}

void wandLightsOffMenuSystem() {
  // Make sure some of the wand lights are off, specifically for the Menu systems.
  analogWrite(led_slo_blo, 0);
  digitalWrite(led_vent, HIGH);
  digitalWrite(led_white, HIGH);
  analogWrite(led_front_left, 0);
}

void vibrationOff() {
  i_vibration_level_prev = 0;
  analogWrite(vibration, 0);
}

void adjustVolumeEffectsGain() {
  // Since adjusting only happens while in the menu mode, only certain effects need to be adjusted on the fly.
  w_trig.trackGain(S_BEEPS, i_volume_effects);
  w_trig.trackGain(S_BEEPS_ALT, i_volume_effects);
  w_trig.trackGain(S_BEEPS_LOW, i_volume_effects);
  w_trig.trackGain(S_BEEPS_BARGRAPH, i_volume_effects);

  w_trig.trackGain(S_AFTERLIFE_WAND_IDLE_1, i_volume_effects - 1); // Special volume in use.
  w_trig.trackGain(S_AFTERLIFE_WAND_IDLE_2, i_volume_effects - 1); // Special volume in use.
  w_trig.trackGain(S_AFTERLIFE_WAND_RAMP_1, i_volume_effects - 1); // Special volume in use.
  w_trig.trackGain(S_AFTERLIFE_WAND_RAMP_2, i_volume_effects - 1); // Special volume in use.
  w_trig.trackGain(S_AFTERLIFE_WAND_RAMP_2_FADE_IN, i_volume_effects - 1); // Special volume in use.
  w_trig.trackGain(S_AFTERLIFE_WAND_RAMP_DOWN_1, i_volume_effects - 1); // Special volume in use.
  w_trig.trackGain(S_AFTERLIFE_WAND_RAMP_DOWN_2, i_volume_effects - 1); // Special volume in use.
  w_trig.trackGain(S_AFTERLIFE_WAND_RAMP_DOWN_2_FADE_OUT, i_volume_effects - 1); // Special volume in use.

  w_trig.trackGain(S_AFTERLIFE_BEEP_WAND_S1, i_volume_effects);
  w_trig.trackGain(S_AFTERLIFE_BEEP_WAND_S2, i_volume_effects);
  w_trig.trackGain(S_AFTERLIFE_BEEP_WAND_S3, i_volume_effects);
  w_trig.trackGain(S_AFTERLIFE_BEEP_WAND_S4, i_volume_effects);
  w_trig.trackGain(S_AFTERLIFE_BEEP_WAND_S5, i_volume_effects);

  w_trig.trackGain(S_IDLE_LOOP_GUN, i_volume_effects);
  w_trig.trackGain(S_IDLE_LOOP_GUN_1, i_volume_effects);
  w_trig.trackGain(S_IDLE_LOOP_GUN_2, i_volume_effects);
  w_trig.trackGain(S_IDLE_LOOP_GUN_3, i_volume_effects);
  w_trig.trackGain(S_IDLE_LOOP_GUN_4, i_volume_effects);
  w_trig.trackGain(S_IDLE_LOOP_GUN_5, i_volume_effects);
}

void increaseVolumeEffects() {
  if(i_volume_percentage + VOLUME_EFFECTS_MULTIPLIER > 100) {
    i_volume_percentage = 100;

    // Provide feedback at maximum volume.
    stopEffect(S_BEEPS_ALT);
    playEffect(S_BEEPS_ALT, false, i_volume_master - 10);
  }
  else {
    i_volume_percentage = i_volume_percentage + VOLUME_EFFECTS_MULTIPLIER;
  }

  i_volume_effects = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_percentage / 100);

  adjustVolumeEffectsGain();
}

void decreaseVolumeEffects() {
  if(i_volume_percentage - VOLUME_EFFECTS_MULTIPLIER < 0) {
    i_volume_percentage = 0;

    // Provide feedback at minimum volume.
    stopEffect(S_BEEPS_ALT);
    playEffect(S_BEEPS_ALT, false, i_volume_master - 10);
  }
  else {
    i_volume_percentage = i_volume_percentage - VOLUME_EFFECTS_MULTIPLIER;
  }

  i_volume_effects = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_percentage / 100);

  adjustVolumeEffectsGain();
}

void increaseVolume() {
  if(i_volume_master == i_volume_abs_min && MINIMUM_VOLUME > i_volume_master) {
    i_volume_master = MINIMUM_VOLUME;
  }

  if(i_volume_master_percentage + VOLUME_MULTIPLIER > 100) {
    i_volume_master_percentage = 100;

    // Provide feedback at maximum volume.
    stopEffect(S_BEEPS_ALT);
    playEffect(S_BEEPS_ALT, false, i_volume_master - 10);
  }
  else {
    i_volume_master_percentage = i_volume_master_percentage + VOLUME_MULTIPLIER;
  }

  i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
  i_volume_revert = i_volume_master;

  w_trig.masterGain(i_volume_master);
}

void decreaseVolume() {
  if(i_volume_master == i_volume_abs_min) {
    // Can not go any lower.
  }
  else {
    if(i_volume_master_percentage - VOLUME_MULTIPLIER < 0) {
      i_volume_master_percentage = 0;
    }
    else {
      i_volume_master_percentage = i_volume_master_percentage - VOLUME_MULTIPLIER;
    }

    i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
    i_volume_revert = i_volume_master;

    w_trig.masterGain(i_volume_master);
  }
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
    barrel_leds[i] = getHueAsGRB(C_CUSTOM);
  }

  ms_fast_led.start(i_fast_led_delay);
}

// It is very important that S_1 up to S_60 follow each other in order on the Micro SD Card and sound effects enum.
void overheatVoiceIndicator(unsigned int i_tmp_length) {
  i_tmp_length = i_tmp_length / i_overheat_delay_increment;

  unsigned int i_tmp_sound = (S_1 - 1) + i_tmp_length;

  stopEffect(i_tmp_sound - 1);
  stopEffect(i_tmp_sound);
  stopEffect(i_tmp_sound + 1);
  playEffect(i_tmp_sound);

  // Tell the Proton Pack to play this sound effect.
  wandSerialSend(i_tmp_sound, true);
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
void checkRotary() {
  static int8_t c,val;

  if((val = readRotary())) {
    c += val;
    switch(WAND_ACTION_STATUS) {
      case ACTION_CONFIG_EEPROM_MENU:
        // Counter clockwise.
        if(prev_next_code == 0x0b) {
          if(WAND_MENU_LEVEL == MENU_LEVEL_3 && i_wand_menu == 5 && switch_intensify.getState() == LOW && digitalRead(switch_mode) == HIGH) {
            // Adjust the default bootup system volume.
            wandSerialSend(W_VOLUME_DECREASE_EEPROM);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 5 && switch_intensify.getState() == LOW && digitalRead(switch_mode) == HIGH) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_5);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 4 && switch_intensify.getState() == LOW && digitalRead(switch_mode) == HIGH) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_4);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 3 && switch_intensify.getState() == LOW && digitalRead(switch_mode) == HIGH) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_3);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 2 && switch_intensify.getState() == LOW && digitalRead(switch_mode) == HIGH) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_2);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 1 && switch_intensify.getState() == LOW && digitalRead(switch_mode) == HIGH) {
            wandSerialSend(W_OVERHEAT_DECREASE_LEVEL_1);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 5 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
            overheatTimerDecrement(5);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 4 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
            overheatTimerDecrement(4);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 3 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
            overheatTimerDecrement(3);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 2 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
            overheatTimerDecrement(2);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 1 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
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

                // Play an indication beep to notify we have change menu levels.
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

                // Play an indication beep to notify we have change menu levels.
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

                // Play an indication beep to notify we have change menu levels.
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

                // Play an indication beep to notify we have change menu levels.
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
          if(WAND_MENU_LEVEL == MENU_LEVEL_3 && i_wand_menu == 5 && switch_intensify.getState() == LOW && digitalRead(switch_mode) == HIGH) {
            // Adjust the default bootup system volume.
            wandSerialSend(W_VOLUME_INCREASE_EEPROM);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 5 && switch_intensify.getState() == LOW && digitalRead(switch_mode) == HIGH) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_5);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 4 && switch_intensify.getState() == LOW && digitalRead(switch_mode) == HIGH) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_4);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 3 && switch_intensify.getState() == LOW && digitalRead(switch_mode) == HIGH) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_3);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 2 && switch_intensify.getState() == LOW && digitalRead(switch_mode) == HIGH) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_2);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 1 && switch_intensify.getState() == LOW && digitalRead(switch_mode) == HIGH) {
            wandSerialSend(W_OVERHEAT_INCREASE_LEVEL_1);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 5 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
            overheatTimerIncrement(5);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 4 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
            overheatTimerIncrement(4);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 3 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
            overheatTimerIncrement(3);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 2 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
            overheatTimerIncrement(2);
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_4 && i_wand_menu == 1 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
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

                // Play an indication beep to notify we have change menu levels.
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

                // Play an indication beep to notify we have change menu levels.
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

                // Play an indication beep to notify we have change menu levels.
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

                // Play an indication beep to notify we have change menu levels.
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
                // Can not go any further than menu level 1.
                i_wand_menu = 5;
              break;
            }
          }
          else {
            i_wand_menu++;
          }
        }
      break;

      case ACTION_EEPROM_MENU:
        // Counter clockwise.
        if(prev_next_code == 0x0b) {
          if(i_wand_menu == 4 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
            // Change colour of the wand barrel spectral custom colour.
            if(i_spectral_wand_custom > 1 && i_spectral_wand_saturation_custom > 253) {
              i_spectral_wand_custom--;
            }
            else {
              i_spectral_wand_custom = 1;

              if(i_spectral_wand_saturation_custom > 1) {
                i_spectral_wand_saturation_custom--;
              }
              else {
                i_spectral_wand_saturation_custom = 1;
              }
            }

            wandBarrelSpectralCustomConfigOn();
          }
          else if(i_wand_menu == 3 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
            // Change colour of the Power Cell Spectral custom colour.
            wandSerialSend(W_SPECTRAL_POWERCELL_CUSTOM_DECREASE);
          }
          else if(i_wand_menu == 2 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
            // Change colour of the Cyclotron Spectral custom colour.
            wandSerialSend(W_SPECTRAL_CYCLOTRON_CUSTOM_DECREASE);
          }
          else if(i_wand_menu == 1 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
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
          if(i_wand_menu == 4 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
            // Change colour of the Wand Barrel Spectral custom colour.
            if(i_spectral_wand_saturation_custom < 254) {
              i_spectral_wand_saturation_custom++;

              if(i_spectral_wand_saturation_custom > 253) {
                i_spectral_wand_saturation_custom = 254;
              }
            }
            else if(i_spectral_wand_custom < 253 && i_spectral_wand_saturation_custom > 253) {
              i_spectral_wand_custom++;
            }
            else {
              i_spectral_wand_custom = 254;

              if(i_spectral_wand_saturation_custom < 253) {
                i_spectral_wand_saturation_custom++;
              }
              else {
                i_spectral_wand_saturation_custom = 254;
              }
            }

            wandBarrelSpectralCustomConfigOn();
          }
          else if(i_wand_menu == 3 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
            // Change colour of the Power Cell Spectral custom colour.
            wandSerialSend(W_SPECTRAL_POWERCELL_CUSTOM_INCREASE);
          }
          else if(i_wand_menu == 2 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
            // Change colour of the Cyclotron Spectral custom colour.
            wandSerialSend(W_SPECTRAL_CYCLOTRON_CUSTOM_INCREASE);
          }
          else if(i_wand_menu == 1 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW) {
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
          if(i_wand_menu == 4 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.getState() == LOW && switchMode() != true) {
            // Tell pack to dim the selected lighting. (Power Cell, Cyclotron or Inner Cyclotron)
            wandSerialSend(W_DIMMING_DECREASE);
          }
          else if(i_wand_menu == 3 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.getState() == LOW && switchMode() != true) {
            // Lower the sound effects volume.
            decreaseVolumeEffects();

            // Tell pack to lower the sound effects volume.
            wandSerialSend(W_VOLUME_SOUND_EFFECTS_DECREASE);
          }
          else if(i_wand_menu == 3 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW && b_playing_music == true) {
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

            if(i_music_count > 0) {
              w_trig.trackGain(i_current_music_track, i_volume_music);
            }

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

                  // Play an indication beep to notify we have change menu levels.
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
                  // Can not go further than level 2 for this menu.
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
          if(i_wand_menu == 4 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.getState() == LOW && switchMode() != true) {
            // Tell pack to dim the selected lighting. (Power Cell, Cyclotron or Inner Cyclotron)
            wandSerialSend(W_DIMMING_INCREASE);
          }
          else if(i_wand_menu == 3 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.getState() == LOW && switchMode() != true) {
            // Increase sound effects volume.
            increaseVolumeEffects();

            // Tell pack to increase the sound effects volume.
            wandSerialSend(W_VOLUME_SOUND_EFFECTS_INCREASE);
          }
          else if(i_wand_menu == 3 && WAND_MENU_LEVEL == MENU_LEVEL_1 && switch_intensify.getState() == HIGH && digitalRead(switch_mode) == LOW && b_playing_music == true) {
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

            if(i_music_count > 0) {
              w_trig.trackGain(i_current_music_track, i_volume_music);
            }

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

                  // Play an indication beep to notify we have change menu levels.
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

              // Forces a redraw of the bargraph if firing while changing the power level in the BARGRAPH_ORIGINAL.
              if(b_firing == true && b_28segment_bargraph == true && BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
                bargraphRedraw();
              }

              soundBeepLoopStop();

              switch(SYSTEM_YEAR) {
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

              if(i_music_count > 0) {
                w_trig.trackGain(i_current_music_track, i_volume_music);
              }

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

                // Forces a redraw of the bargraph if firing while changing the power level if using BARGRAPH_ORIGINAL.
                if(b_firing == true && b_28segment_bargraph == true && BARGRAPH_MODE == BARGRAPH_ORIGINAL) {
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

              if(i_music_count > 0) {
                w_trig.trackGain(i_current_music_track, i_volume_music);
              }

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
  switch_wand.loop();
  switch_intensify.loop();
  switch_activate.loop();
  switch_vent.loop();
}

void wandBarrelLightsOff() {
  for(uint8_t i = 0; i < i_num_barrel_leds; i++) {
    barrel_leds[i] = getHueAsGRB(C_BLACK);
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

    case PROTON:
      // Tell the pack we are in proton mode.
      wandSerialSend(W_PROTON_MODE);
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
  switch_wand.resetCount();
  switch_vent.resetCount();

  i_wand_menu = 5;

  bargraphClearAlt();

  WAND_ACTION_STATUS = ACTION_IDLE;

  wandLightsOff();
  wandBarrelLightsOff();
}

// Barrel Wing Button is connected to analog pin 6.
// PCB builds is pulled high as digital input.
// At some point, switch this to a ezButton.
bool switchMode() {
  if(digitalRead(switch_mode) == LOW && ms_switch_mode_debounce.remaining() < 1 && b_switch_mode_pressed != true) {
    ms_switch_mode_debounce.start(switch_debounce_time * 5);

    b_switch_mode_pressed = true;

    return true;
  }
  else {
    return false;
  }
}

// Check if the Barrel Wing Button is being held down or not.
// At some point, switch this to a ezButton.
void switchModePressedReset() {
  if(digitalRead(switch_mode) == HIGH && b_switch_mode_pressed == true && ms_switch_mode_debounce.remaining() < 1) {
    b_switch_mode_pressed = false;
  }
}

// Barrel safety switch is connected to analog pin 7.
// PCB builds is pulled high as digital input.
// Maybe switch it to a ezButton later??
void switchBarrel() {
  if(digitalRead(switch_barrel) == LOW && ms_switch_barrel_debounce.remaining() < 1) {
    ms_switch_barrel_debounce.start(switch_debounce_time * 5);

    if(b_switch_barrel_extended == true) {
      wandSerialSend(W_BARREL_RETRACTED);
    }

    b_switch_barrel_extended = false;
  }
  else if(digitalRead(switch_barrel) == HIGH && ms_switch_barrel_debounce.remaining() < 1) {
    // Play the Afterlife Barrel extension sound effect.
    if((getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) && b_switch_barrel_extended != true) {
      // Plays the "thwoop" barrel extension sound in Afterlife mode.
      playEffect(S_AFTERLIFE_WAND_BARREL_EXTEND, false, i_volume_effects - 1);
    }

    if(b_switch_barrel_extended != true) {
      wandSerialSend(W_BARREL_EXTENDED);
    }

    b_switch_barrel_extended = true;
  }
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

// Pack communication to the wand.
void checkPack() {
  if(wandComs.available() && b_no_pack != true) {
    wandComs.rxObj(comStruct);

    if(!wandComs.currentPacketID()) {
      if(comStruct.i > 0 && comStruct.s == P_COM_START && comStruct.e == P_COM_END) {

        if(b_volume_sync_wait == true) {
          switch(VOLUME_SYNC_WAIT) {
            case EFFECTS:
              i_volume_percentage = comStruct.i;
              i_volume_effects = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_percentage / 100);

              adjustVolumeEffectsGain();
              VOLUME_SYNC_WAIT = MASTER;
            break;

            case MASTER:
              i_volume_master_percentage = comStruct.i;
              i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);

              i_volume_revert = i_volume_master;

              w_trig.masterGain(i_volume_master);

              VOLUME_SYNC_WAIT = MUSIC;
            break;

            case MUSIC:
              i_volume_music_percentage = comStruct.i;
              i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

              VOLUME_SYNC_WAIT = SILENT;
            break;

            case SILENT:
              if(comStruct.i == P_MASTER_AUDIO_SILENT_MODE) {
                i_volume_revert = i_volume_master;

                // The pack is telling us to be silent.
                i_volume_master = i_volume_abs_min;
                w_trig.masterGain(i_volume_master);
              }

              // Exit the sync mode.
              b_volume_sync_wait = false;
              b_wait_for_pack = false;

              VOLUME_SYNC_WAIT = EFFECTS;
            break;
          }
        }
        else {
          switch(comStruct.i) {
            case P_ON:
              // Pack is on.
              b_pack_on = true;
            break;

            case P_OFF:
              if(b_pack_on == true) {
                // Turn wand off.
                if(WAND_STATUS != MODE_OFF) {
                  if(WAND_STATUS == MODE_ERROR) {
                    b_wand_mash_error = false;
                    wandOff();
                  }
                  else {
                    b_wand_mash_error = false;
                    WAND_ACTION_STATUS = ACTION_OFF;
                  }
                }
              }

              // Pack is off.
              b_pack_on = false;
            break;

            case P_SYNC_START:
              b_sync = true;
            break;

            case P_SYNC_END:
              b_sync = false;

              switchBarrel();

              // Tell the pack the status of the Neutrona Wand barrel. We only need to tell if its extended. Otherwise the switchBarrel() will tell it if it's retracted during bootup.
              if(b_switch_barrel_extended == true) {
                wandSerialSend(W_BARREL_EXTENDED);
              }
            break;

            case P_SOUND_SUPER_HERO:
              stopEffect(S_VOICE_MODE_SUPER_HERO);
              stopEffect(S_VOICE_MODE_ORIGINAL);
              playEffect(S_VOICE_MODE_SUPER_HERO);
            break;

            case P_SOUND_MODE_ORIGINAL:
              stopEffect(S_VOICE_MODE_ORIGINAL);
              stopEffect(S_VOICE_MODE_SUPER_HERO);
              playEffect(S_VOICE_MODE_ORIGINAL);
            break;

            case P_MODE_SUPER_HERO:
              SYSTEM_MODE = MODE_SUPER_HERO;
            break;

            case P_MODE_ORIGINAL:
              SYSTEM_MODE = MODE_ORIGINAL;
            break;

            case P_OVERHEATING_FINISHED:
              if(WAND_STATUS != MODE_OFF) {
                overHeatingFinished();
              }
            break;

            case P_MODE_ORIGINAL_RED_SWITCH_ON:
              b_pack_ion_arm_switch_on = true;

              // Prep the bargraph for MODE_ORIGINAL. This only preps it when the pack switch is turned on and the wand is still off but all the toggle switches are on for the bargraph to settle at the off position. (0 circle).
              if(WAND_ACTION_STATUS == ACTION_IDLE) {
                switch(WAND_STATUS) {
                  case MODE_OFF:
                    switch(SYSTEM_MODE) {
                      case MODE_ORIGINAL:
                        if(switch_vent.getState() == LOW && switch_wand.getState() == LOW && switch_activate.getState() == HIGH) {
                          if(b_mode_original_toggle_sounds_enabled == true) {
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
                      break;

                      default:
                        // Do nothing.
                      break;
                    }
                  break;
                }
              }
            break;

            case P_MODE_ORIGINAL_RED_SWITCH_OFF:
              b_pack_ion_arm_switch_on = false;

              switch(SYSTEM_MODE) {
                case MODE_ORIGINAL:
                  if(switch_vent.getState() == LOW && switch_wand.getState() == LOW && b_mode_original_toggle_sounds_enabled == true) {
                    stopEffect(S_WAND_HEATDOWN);
                    stopEffect(S_WAND_HEATUP_ALT);
                    stopEffect(S_WAND_HEATUP);
                    playEffect(S_WAND_HEATDOWN);
                  }
                break;

                default:
                  // Do nothing.
                break;
              }
            break;

            case P_MASTER_AUDIO_SILENT_MODE:
              i_volume_revert = i_volume_master;

              // The pack is telling us to be silent.
              i_volume_master = i_volume_abs_min;
              w_trig.masterGain(i_volume_master);
            break;

            case P_MANUAL_OVERHEAT:
              if(WAND_STATUS == MODE_ON && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
                if(b_pack_on == true && b_pack_alarm != true && b_overheat_enabled == true) {
                  startVentSequence();
                }
              }
            break;

            case P_MASTER_AUDIO_NORMAL:
              // The pack is telling us to revert the audio to normal.
              i_volume_master = i_volume_revert;
              w_trig.masterGain(i_volume_master);
            break;

            case P_RIBBON_CABLE_ON:
              b_pack_ribbon_cable_on = true;

              if(WAND_STATUS == MODE_ON && b_pack_alarm != true && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
                soundIdleLoop(true);

                if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
                  stopEffect(S_BOOTUP);
                  playEffect(S_BOOTUP);

                  if(switch_vent.getState() == HIGH) {
                    afterLifeRamp1();
                  }
                  else {
                    stopAfterLifeSounds();
                  }
                }
              }
            break;

            case P_RIBBON_CABLE_OFF:
              b_pack_ribbon_cable_on = false;

              if(WAND_STATUS == MODE_ON && b_pack_alarm == true && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
                switch(getNeutronaWandYearMode()) {
                  case SYSTEM_1984:
                  case SYSTEM_1989:
                    // Nothing for now.
                  break;

                  case SYSTEM_AFTERLIFE:
                  case SYSTEM_FROZEN_EMPIRE:
                  default:
                      stopEffect(S_WAND_SHUTDOWN);
                      playEffect(S_WAND_SHUTDOWN);

                    if(switch_vent.getState() == HIGH) {
                      stopAfterLifeSounds();
                      playEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1, false, i_volume_effects - 1);

                      if(b_extra_pack_sounds == true) {
                        wandSerialSend(W_EXTRA_WAND_SOUNDS_STOP);
                        wandSerialSend(W_AFTERLIFE_GUN_RAMP_DOWN_1);
                      }
                    }
                  break;
                }
              }
            break;

            case P_ALARM_ON:
              // Alarm is on.
              b_pack_alarm = true;

              if(WAND_STATUS != MODE_ERROR) {
                if(WAND_STATUS == MODE_ON) {
                  digitalWrite(led_hat_2, HIGH); // Turn on hat light 2.
                }

                ms_hat_2.start(i_hat_2_delay); // Start the hat light 2 blinking timer.

                if(WAND_STATUS == MODE_ON && FIRING_MODE == SETTINGS) {
                  // If the wand is in settings mode while the alarm is activated, exit the settings mode.
                  wandSerialSend(W_PROTON_MODE);
                  FIRING_MODE = PROTON;
                  WAND_ACTION_STATUS = ACTION_IDLE;
                }

                if(WAND_STATUS == MODE_ON) {
                  prepBargraphRampDown();
                }
              }
            break;

            case P_ALARM_OFF:
              if(WAND_STATUS != MODE_ERROR) {
                digitalWrite(led_hat_2, LOW); // Turn off hat light 2.

                ms_hat_2.stop();

                if(WAND_STATUS == MODE_ON) {
                  switch(SYSTEM_MODE) {
                    case MODE_ORIGINAL:

                      if(switch_vent.getState() == LOW && switch_wand.getState() == LOW && switch_activate.getState() == LOW && b_pack_alarm == true) {
                        b_pack_alarm = false;

                        prepBargraphRampUp();
                      }
                    break;

                    case MODE_SUPER_HERO:
                    default:
                      b_pack_alarm = false;

                      prepBargraphRampUp();
                    break;
                  }
                }
              }

              // Alarm is off.
              b_pack_alarm = false;
            break;

            case P_WARNING_CANCELLED:
              // Pack is telling wand to cancel any overheat warnings.
              // First, stop the timers which trigger the overheat.
              ms_overheat_initiate.stop();
              ms_overheating.stop();
              ms_hat_1.stop();
              ms_hat_2.stop();

              if(b_firing == true) {
                // Keep both lights on if still firing.
                digitalWrite(led_hat_1, HIGH);
                digitalWrite(led_hat_2, HIGH);
              }

              // Next, reset the cyclotron speed on all devices.
              wandSerialSend(W_CYCLOTRON_NORMAL_SPEED);
              cyclotronSpeedRevert();
            break;

            case P_YEAR_1984:
              // 1984 mode.
              SYSTEM_YEAR = SYSTEM_1984;
              bargraphYearModeUpdate();
            break;

            case P_YEAR_1989:
              // 1984 mode.
              SYSTEM_YEAR = SYSTEM_1989;
              bargraphYearModeUpdate();
            break;

            case P_YEAR_AFTERLIFE:
              // 2021 mode.
              SYSTEM_YEAR = SYSTEM_AFTERLIFE;
              bargraphYearModeUpdate();
            break;

            case P_YEAR_FROZEN_EMPIRE:
              SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
              bargraphYearModeUpdate();
            break;

            case P_VOLUME_SOUND_EFFECTS_INCREASE:
              // Increase effects volume.
              increaseVolumeEffects();
            break;

            case P_VOLUME_SOUND_EFFECTS_DECREASE:
              // Decrease effects volume.
              decreaseVolumeEffects();
            break;

            case P_VOLUME_INCREASE:
              // Increase overall volume.
              increaseVolume();
            break;

            case P_VOLUME_DECREASE:
              // Decrease overall volume.
              decreaseVolume();
            break;

            case P_HANDSHAKE:
              // The pack is asking us if we are still here. Respond back.
              wandSerialSend(W_HANDSHAKE);
            break;

            case P_MUSIC_REPEAT:
              // Repeat music track.
              b_repeat_track = true;
            break;

            case P_MUSIC_NO_REPEAT:
              // Do not repeat the music track.
              b_repeat_track = false;
            break;

            case P_VOLUME_SYNC_MODE:
              // Put the wand into volume sync mode.
              b_volume_sync_wait = true;
              VOLUME_SYNC_WAIT = EFFECTS;
            break;

            case P_VIBRATION_ENABLED:
              // Vibration enabled (from Proton Pack vibration toggle switch).
              b_vibration_enabled = true;

              // Only play the voice if we are not doing a Proton Pack / Neutrona Wand synchronisation.
              if(b_wait_for_pack != true) {
                stopEffect(S_BEEPS_ALT);

                playEffect(S_BEEPS_ALT);

                stopEffect(S_VOICE_VIBRATION_ENABLED);
                stopEffect(S_VOICE_VIBRATION_DISABLED);

                playEffect(S_VOICE_VIBRATION_ENABLED);
              }
            break;

            case P_VIBRATION_DISABLED:
              // Vibration disabled (from Proton Pack vibration toggle switch).
              b_vibration_enabled = false;

              // Only play the voice if we are not doing a Proton Pack / Neutrona Wand synchronisation.
              if(b_wait_for_pack != true) {
                stopEffect(S_BEEPS_ALT);

                playEffect(S_BEEPS_ALT);

                stopEffect(S_VOICE_VIBRATION_DISABLED);
                stopEffect(S_VOICE_VIBRATION_ENABLED);

                playEffect(S_VOICE_VIBRATION_DISABLED);
              }

              vibrationOff();
            break;

            case P_PACK_VIBRATION_ENABLED:
              // Proton Pack Vibration enabled.
              stopEffect(S_BEEPS_ALT);

              playEffect(S_BEEPS_ALT);

              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);

              playEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
            break;

            case P_PACK_VIBRATION_DISABLED:
              // Proton Pack Vibration disabled.
              stopEffect(S_BEEPS_ALT);

              playEffect(S_BEEPS_ALT);

              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);

              playEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);
            break;

            case P_PACK_VIBRATION_FIRING_ENABLED:
              // Proton Pack Vibration firing enabled.
              stopEffect(S_BEEPS_ALT);

              playEffect(S_BEEPS_ALT);

              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);

              playEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
            break;

            case P_MODE_FROZEN_EMPIRE:
              // Play Frozen Empire voice.
              stopEffect(S_VOICE_FROZEN_EMPIRE);
              stopEffect(S_VOICE_AFTERLIFE);
              stopEffect(S_VOICE_1989);
              stopEffect(S_VOICE_1984);

              playEffect(S_VOICE_FROZEN_EMPIRE);
            break;

            case P_MODE_AFTERLIFE:
              // Play Afterlife voice.
              stopEffect(S_VOICE_FROZEN_EMPIRE);
              stopEffect(S_VOICE_AFTERLIFE);
              stopEffect(S_VOICE_1989);
              stopEffect(S_VOICE_1984);

              playEffect(S_VOICE_AFTERLIFE);
            break;

            case P_MODE_1989:
              // Play 1989 voice.
              stopEffect(S_VOICE_FROZEN_EMPIRE);
              stopEffect(S_VOICE_AFTERLIFE);
              stopEffect(S_VOICE_1989);
              stopEffect(S_VOICE_1984);

              playEffect(S_VOICE_1989);
            break;

            case P_MODE_1984:
              // Play 1984 voice.
              stopEffect(S_VOICE_FROZEN_EMPIRE);
              stopEffect(S_VOICE_AFTERLIFE);
              stopEffect(S_VOICE_1989);
              stopEffect(S_VOICE_1984);

              playEffect(S_VOICE_1984);
            break;

            case P_SMOKE_DISABLED:
              // Play smoke disabled voice.
              stopEffect(S_VOICE_SMOKE_DISABLED);
              stopEffect(S_VOICE_SMOKE_ENABLED);

              playEffect(S_VOICE_SMOKE_DISABLED);
            break;

            case P_SMOKE_ENABLED:
              // Play smoke enabled voice.
              stopEffect(S_VOICE_SMOKE_ENABLED);
              stopEffect(S_VOICE_SMOKE_DISABLED);

              playEffect(S_VOICE_SMOKE_ENABLED);
            break;

            case P_CYCLOTRON_COUNTER_CLOCKWISE:
              // Play Cyclotron counter clockwise voice.
              stopEffect(S_VOICE_CYCLOTRON_CLOCKWISE);
              stopEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);

              playEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);
            break;

            case P_CYCLOTRON_CLOCKWISE:
              // Play Cyclotron clockwise voice.
              stopEffect(S_VOICE_CYCLOTRON_CLOCKWISE);
              stopEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);

              playEffect(S_VOICE_CYCLOTRON_CLOCKWISE);
            break;

            case P_CYCLOTRON_SINGLE_LED:
              // Play Single LED voice.
              stopEffect(S_VOICE_THREE_LED);
              stopEffect(S_VOICE_SINGLE_LED);

              playEffect(S_VOICE_SINGLE_LED);
            break;

            case P_CYCLOTRON_THREE_LED:
              // Play 3 LED voice.
              stopEffect(S_VOICE_THREE_LED);
              stopEffect(S_VOICE_SINGLE_LED);

              playEffect(S_VOICE_THREE_LED);
            break;

            case P_VIDEO_GAME_MODE_COLOURS_DISABLED:
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_DISABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED);

              playEffect(S_VOICE_VIDEO_GAME_COLOURS_DISABLED);
            break;

            case P_VIDEO_GAME_MODE_POWER_CELL_ENABLED:
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_DISABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED);

              playEffect(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED);
            break;

            case P_VIDEO_GAME_MODE_CYCLOTRON_ENABLED:
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_DISABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED);

              playEffect(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED);
            break;

            case P_VIDEO_GAME_MODE_COLOURS_ENABLED:
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_DISABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED);

              playEffect(S_VOICE_VIDEO_GAME_COLOURS_ENABLED);
            break;

            case P_MUSIC_STOP:
              b_playing_music = false;

              // Stop music
              stopMusic();
            break;

            case P_DIMMING:
              stopEffect(S_BEEPS);
              playEffect(S_BEEPS);
            break;

            case P_CONTINUOUS_SMOKE_5_ENABLED:
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_5_ENABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_5_DISABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_5_ENABLED);
            break;

            case P_CONTINUOUS_SMOKE_4_ENABLED:
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_4_ENABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_4_DISABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_4_ENABLED);
            break;

            case P_CONTINUOUS_SMOKE_3_ENABLED:
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_3_ENABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_3_DISABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_3_ENABLED);
            break;

            case P_CONTINUOUS_SMOKE_2_ENABLED:
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_2_ENABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_2_DISABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_2_ENABLED);
            break;

            case P_CONTINUOUS_SMOKE_1_ENABLED:
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_1_ENABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_1_DISABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_1_ENABLED);
            break;

            case P_CONTINUOUS_SMOKE_5_DISABLED:
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_5_DISABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_5_ENABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_5_DISABLED);
            break;

            case P_CONTINUOUS_SMOKE_4_DISABLED:
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_4_DISABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_4_ENABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_4_DISABLED);
            break;

            case P_CONTINUOUS_SMOKE_3_DISABLED:
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_3_DISABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_3_ENABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_3_DISABLED);
            break;

            case P_CONTINUOUS_SMOKE_2_DISABLED:
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_2_DISABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_2_ENABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_2_DISABLED);
            break;

            case P_CONTINUOUS_SMOKE_1_DISABLED:
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_1_DISABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_1_ENABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_1_DISABLED);
            break;

            case P_OVERHEAT_STROBE_DISABLED:
              stopEffect(S_VOICE_OVERHEAT_STROBE_DISABLED);
              stopEffect(S_VOICE_OVERHEAT_STROBE_ENABLED);

              playEffect(S_VOICE_OVERHEAT_STROBE_DISABLED);
            break;

            case P_OVERHEAT_STROBE_ENABLED:
              stopEffect(S_VOICE_OVERHEAT_STROBE_ENABLED);
              stopEffect(S_VOICE_OVERHEAT_STROBE_DISABLED);

              playEffect(S_VOICE_OVERHEAT_STROBE_ENABLED);
            break;

            case P_OVERHEAT_LIGHTS_OFF_DISABLED:
              stopEffect(S_VOICE_OVERHEAT_LIGHTS_OFF_DISABLED);
              stopEffect(S_VOICE_OVERHEAT_LIGHTS_OFF_ENABLED);

              playEffect(S_VOICE_OVERHEAT_LIGHTS_OFF_DISABLED);
            break;

            case P_OVERHEAT_LIGHTS_OFF_ENABLED:
              stopEffect(S_VOICE_OVERHEAT_LIGHTS_OFF_ENABLED);
              stopEffect(S_VOICE_OVERHEAT_LIGHTS_OFF_DISABLED);

              playEffect(S_VOICE_OVERHEAT_LIGHTS_OFF_ENABLED);
            break;

            case P_OVERHEAT_SYNC_FAN_DISABLED:
              stopEffect(S_VOICE_OVERHEAT_FAN_SYNC_DISABLED);
              stopEffect(S_VOICE_OVERHEAT_FAN_SYNC_ENABLED);

              playEffect(S_VOICE_OVERHEAT_FAN_SYNC_DISABLED);
            break;

            case P_OVERHEAT_SYNC_FAN_ENABLED:
              stopEffect(S_VOICE_OVERHEAT_FAN_SYNC_ENABLED);
              stopEffect(S_VOICE_OVERHEAT_FAN_SYNC_DISABLED);

              playEffect(S_VOICE_OVERHEAT_FAN_SYNC_ENABLED);
            break;

            case P_POWERCELL_DIMMING:
              stopEffect(S_VOICE_POWERCELL_BRIGHTNESS);
              stopEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
              stopEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);

              playEffect(S_VOICE_POWERCELL_BRIGHTNESS);
            break;

            case P_CYCLOTRON_DIMMING:
              stopEffect(S_VOICE_POWERCELL_BRIGHTNESS);
              stopEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
              stopEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);

              playEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
            break;

            case P_INNER_CYCLOTRON_DIMMING:
              stopEffect(S_VOICE_POWERCELL_BRIGHTNESS);
              stopEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
              stopEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);

              playEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);
            break;

            case P_PROTON_STREAM_IMPACT_ENABLED:
              stopEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
              stopEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);

              playEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
            break;

            case P_PROTON_STREAM_IMPACT_DISABLED:
              stopEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
              stopEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);

              playEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);
            break;

            case P_CYCLOTRON_SIMULATE_RING_DISABLED:
              stopEffect(S_VOICE_CYCLOTRON_SIMULATE_RING_DISABLED);
              stopEffect(S_VOICE_CYCLOTRON_SIMULATE_RING_ENABLED);

              playEffect(S_VOICE_CYCLOTRON_SIMULATE_RING_DISABLED);
            break;

            case P_CYCLOTRON_SIMULATE_RING_ENABLED:
              stopEffect(S_VOICE_CYCLOTRON_SIMULATE_RING_DISABLED);
              stopEffect(S_VOICE_CYCLOTRON_SIMULATE_RING_ENABLED);

              playEffect(S_VOICE_CYCLOTRON_SIMULATE_RING_ENABLED);
            break;

            case P_DEMO_LIGHT_MODE_ENABLED:
              stopEffect(S_VOICE_DEMO_LIGHT_MODE_ENABLED);
              stopEffect(S_VOICE_DEMO_LIGHT_MODE_DISABLED);

              playEffect(S_VOICE_DEMO_LIGHT_MODE_ENABLED);
            break;

            case P_DEMO_LIGHT_MODE_DISABLED:
              stopEffect(S_VOICE_DEMO_LIGHT_MODE_DISABLED);
              stopEffect(S_VOICE_DEMO_LIGHT_MODE_ENABLED);

              playEffect(S_VOICE_DEMO_LIGHT_MODE_DISABLED);
            break;

            case P_PROTON_MODE:
              FIRING_MODE = PROTON;
              PREV_FIRING_MODE = SETTINGS;
            break;

            case P_SLIME_MODE:
              FIRING_MODE = SLIME;
              PREV_FIRING_MODE = PROTON;
            break;

            case P_STASIS_MODE:
              FIRING_MODE = STASIS;
              PREV_FIRING_MODE = SLIME;

              setVGMode();
            break;

            case P_MESON_MODE:
              FIRING_MODE = MESON;
              PREV_FIRING_MODE = STASIS;

              setVGMode();
            break;

            case P_SPECTRAL_CUSTOM_MODE:
              FIRING_MODE = SPECTRAL_CUSTOM;
              PREV_FIRING_MODE = PROTON;
              setVGMode();
            break;

            case P_SPECTRAL_MODE:
              FIRING_MODE = SPECTRAL;
              PREV_FIRING_MODE = PROTON;

              setVGMode();
            break;

            case P_HOLIDAY_MODE:
              FIRING_MODE = HOLIDAY;
              PREV_FIRING_MODE = PROTON;

              setVGMode();
            break;

            case P_VENTING_MODE:
              FIRING_MODE = VENTING;
              PREV_FIRING_MODE = MESON;

              setVGMode();
            break;

            case P_SETTINGS_MODE:
              FIRING_MODE = SETTINGS;
              PREV_FIRING_MODE = VENTING;

              setVGMode();
            break;

            case P_POWER_LEVEL_1:
              i_power_mode = 1;
              i_power_mode_prev = 1;
            break;

            case P_POWER_LEVEL_2:
              i_power_mode = 2;
              i_power_mode_prev = 2;
            break;

            case P_POWER_LEVEL_3:
              i_power_mode = 3;
              i_power_mode_prev = 3;
            break;

            case P_POWER_LEVEL_4:
              i_power_mode = 4;
              i_power_mode_prev = 4;
            break;

            case P_POWER_LEVEL_5:
              i_power_mode = 5;
              i_power_mode_prev = 5;
            break;

            case P_RGB_INNER_CYCLOTRON_LEDS:
              stopEffect(S_VOICE_RGB_INNER_CYCLOTRON);
              stopEffect(S_VOICE_GRB_INNER_CYCLOTRON);

              playEffect(S_VOICE_RGB_INNER_CYCLOTRON);
            break;

            case P_GRB_INNER_CYCLOTRON_LEDS:
              stopEffect(S_VOICE_GRB_INNER_CYCLOTRON);
              stopEffect(S_VOICE_RGB_INNER_CYCLOTRON);

              playEffect(S_VOICE_GRB_INNER_CYCLOTRON);
            break;

            case P_CYCLOTRON_LEDS_40:
              stopEffect(S_VOICE_CYCLOTRON_40);
              stopEffect(S_VOICE_CYCLOTRON_20);
              stopEffect(S_VOICE_CYCLOTRON_12);

              playEffect(S_VOICE_CYCLOTRON_40);
            break;

            case P_CYCLOTRON_LEDS_20:
              stopEffect(S_VOICE_CYCLOTRON_40);
              stopEffect(S_VOICE_CYCLOTRON_20);
              stopEffect(S_VOICE_CYCLOTRON_12);

              playEffect(S_VOICE_CYCLOTRON_20);
            break;

            case P_CYCLOTRON_LEDS_12:
              stopEffect(S_VOICE_CYCLOTRON_40);
              stopEffect(S_VOICE_CYCLOTRON_20);
              stopEffect(S_VOICE_CYCLOTRON_12);

              playEffect(S_VOICE_CYCLOTRON_12);
            break;

            case P_POWERCELL_LEDS_15:
              stopEffect(S_VOICE_POWERCELL_15);
              stopEffect(S_VOICE_POWERCELL_13);

              playEffect(S_VOICE_POWERCELL_15);
            break;

            case P_POWERCELL_LEDS_13:
              stopEffect(S_VOICE_POWERCELL_15);
              stopEffect(S_VOICE_POWERCELL_13);

              playEffect(S_VOICE_POWERCELL_13);
            break;

            case P_INNER_CYCLOTRON_LEDS_23:
              stopEffect(S_VOICE_INNER_CYCLOTRON_35);
              stopEffect(S_VOICE_INNER_CYCLOTRON_24);
              stopEffect(S_VOICE_INNER_CYCLOTRON_23);
              stopEffect(S_VOICE_INNER_CYCLOTRON_12);

              playEffect(S_VOICE_INNER_CYCLOTRON_23);
            break;

            case P_INNER_CYCLOTRON_LEDS_24:
              stopEffect(S_VOICE_INNER_CYCLOTRON_35);
              stopEffect(S_VOICE_INNER_CYCLOTRON_24);
              stopEffect(S_VOICE_INNER_CYCLOTRON_23);
              stopEffect(S_VOICE_INNER_CYCLOTRON_12);

              playEffect(S_VOICE_INNER_CYCLOTRON_24);
            break;

            case P_INNER_CYCLOTRON_LEDS_35:
              stopEffect(S_VOICE_INNER_CYCLOTRON_35);
              stopEffect(S_VOICE_INNER_CYCLOTRON_24);
              stopEffect(S_VOICE_INNER_CYCLOTRON_23);
              stopEffect(S_VOICE_INNER_CYCLOTRON_12);

              playEffect(S_VOICE_INNER_CYCLOTRON_35);
            break;

            case P_INNER_CYCLOTRON_LEDS_12:
              stopEffect(S_VOICE_INNER_CYCLOTRON_35);
              stopEffect(S_VOICE_INNER_CYCLOTRON_24);
              stopEffect(S_VOICE_INNER_CYCLOTRON_23);
              stopEffect(S_VOICE_INNER_CYCLOTRON_12);

              playEffect(S_VOICE_INNER_CYCLOTRON_12);
            break;

            case P_YEAR_MODE_DEFAULT:
              stopEffect(S_VOICE_YEAR_MODE_DEFAULT);
              stopEffect(S_VOICE_FROZEN_EMPIRE);
              stopEffect(S_VOICE_AFTERLIFE);
              stopEffect(S_VOICE_1984);
              stopEffect(S_VOICE_1989);

              playEffect(S_VOICE_YEAR_MODE_DEFAULT);
            break;

            case P_MUSIC_START:
              if(b_playing_music == true) {
                stopMusic();
              }

              playMusic();
            break;

            case P_MUSIC_PAUSE:
              pauseMusic();
            break;

            case P_MUSIC_RESUME: {
              resumeMusic();
            }

            default:
              // Music track number to be played.
              i_current_music_track = comStruct.i;
            break;
          }
        }

        comStruct.i = 0;
        comStruct.s = 0;
      }
    }
  }
}

void wandSerialSend(int i_message, bool b_sound) {
  if(b_no_pack != true) {
    sendStruct.i = i_message;
    sendStruct.s = W_COM_START;

    if(b_sound == true) {
      // Tell the Proton Pack to play the sound # i_message.
      sendStruct.e = W_COM_SOUND_NUMBER;
    }
    else {
      sendStruct.e = W_COM_END;
    }

    wandComs.sendDatum(sendStruct);
  }
}

void clearLEDEEPROM() {
  // Clear out the EEPROM data for the configuration settings only.
  unsigned int i_eepromLEDAddress = EEPROM.length() / 2;

  for(unsigned int i = 0 ; i < sizeof(objLEDEEPROM); i++) {
    EEPROM.put(i_eepromLEDAddress, 0);

    i_eepromLEDAddress++;
  }

  updateCRCEEPROM();
}

void saveLEDEEPROM() {
  unsigned int i_eepromLEDAddress = EEPROM.length() / 2;

  // For now we are just saving the Spectral Custom colour.
  objLEDEEPROM obj_eeprom = {
    i_spectral_wand_custom,
    i_spectral_wand_saturation_custom,
  };

  // Save to the EEPROM.
  EEPROM.put(i_eepromLEDAddress, obj_eeprom);

  updateCRCEEPROM();
}

void clearEEPROM() {
  // Clear out the EEPROM only in the memory addresses used for our EEPROM data object.
  for(unsigned int i = 0 ; i < sizeof(objEEPROM); i++) {
    EEPROM.put(i, 0);
  }

  updateCRCEEPROM();
}

void saveEEPROM() {
  // (Video Game Modes) + Cross The Streams / Cross The Streams Mix / Overheating
  uint8_t i_cross_the_streams = 1;
  uint8_t i_cross_the_streams_mix = 1;
  uint8_t i_overheating = 1;
  uint8_t i_neutrona_wand_sounds = 1;
  uint8_t i_spectral = 1;
  uint8_t i_holiday = 1;

  uint8_t i_quick_vent = 1;
  uint8_t i_wand_boot_errors = 1;
  uint8_t i_vent_light_auto_intensity = 1;
  uint8_t i_invert_bargraph = 1;
  uint8_t i_bargraph_mode = 1;
  uint8_t i_bargraph_firing_animation = 1;

  if(b_cross_the_streams == true) {
    i_cross_the_streams = 2;
  }

  if(b_cross_the_streams_mix == true) {
    i_cross_the_streams_mix = 2;
  }

  if(b_overheat_enabled == true) {
    i_overheating = 2;
  }

  if(b_extra_pack_sounds == true) {
    i_neutrona_wand_sounds = 2;
  }

  if(b_spectral_mode_enabled == true || b_holiday_mode_enabled == true) {
    i_spectral = 2;
    i_holiday = 2;
  }

  if(b_quick_vent == true) {
    i_quick_vent = 2;
  }

  if(b_wand_boot_errors == true) {
    i_wand_boot_errors = 2;
  }

  if(b_vent_light_control == true) {
    i_vent_light_auto_intensity = 2;
  }

  if(b_bargraph_invert == true) {
    i_invert_bargraph = 2;
  }

  switch(BARGRAPH_MODE_EEPROM) {
    case BARGRAPH_EEPROM_SUPER_HERO:
      // 2 = BARGRAPH_EEPROM_SUPER_HERO
      i_bargraph_mode = 2;
    break;

    case BARGRAPH_EEPROM_ORIGINAL:
      // 3 = BARGRAPH_EEPROM_ORIGINAL
      i_bargraph_mode = 3;
    break;

    case BARGRAPH_EEPROM_DEFAULT:
    default:
      // The system chooses which bargraph mode when set to this. This is the default setting.
      // 1 = BARGRAPH_EEPROM_DEFAULT
      i_bargraph_mode = 1;
    break;
  }

  switch(BARGRAPH_EEPROM_FIRING_ANIMATION) {
    case BARGRAPH_EEPROM_ANIMATION_SUPER_HERO:
      // 2 = BARGRAPH_EEPROM_ANIMATION_SUPER_HERO
      i_bargraph_firing_animation = 2;
    break;

    case BARGRAPH_EEPROM_ANIMATION_ORIGINAL:
      // 3 = BARGRAPH_EEPROM_ANIMATION_ORIGINAL
      i_bargraph_firing_animation = 3;
    break;

    case BARGRAPH_EEPROM_ANIMATION_DEFAULT:
    default:
      // The system chooses which bargraph firing animation when set to this. This is the default setting.
      // 1 = BARGRAPH_EEPROM_ANIMATION_DEFAULT
      i_bargraph_firing_animation = 1;
    break;
  }

  // Write the data to the EEPROM if any of the values have changed.
  objEEPROM obj_eeprom = {
    i_cross_the_streams,
    i_cross_the_streams_mix,
    i_overheating,
    i_neutrona_wand_sounds,
    i_spectral,
    i_holiday,
    i_quick_vent,
    i_wand_boot_errors,
    i_vent_light_auto_intensity,
    i_num_barrel_leds,
    i_invert_bargraph,
    i_bargraph_mode,
    i_bargraph_firing_animation
  };

  // Save and update our object in the EEPROM.
  EEPROM.put(i_eepromAddress, obj_eeprom);

  updateCRCEEPROM();
}

// Update the CRC in the EEPROM.
void updateCRCEEPROM() {
  EEPROM.put(EEPROM.length() - sizeof(l_crc_size), eepromCRC());
}

unsigned long eepromCRC(void) {
  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  unsigned long crc = l_crc_size;

  for(unsigned int index = 0; index < EEPROM.length() - sizeof(crc); ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }

  return crc;
}

void readEEPROM() {
  // Get the stored CRC from the EEPROM.
  unsigned long l_crc_check;
  EEPROM.get(EEPROM.length() - sizeof(l_crc_size), l_crc_check);

  // Check if the calculated CRC matches the stored CRC value in the EEPROM.
  if(eepromCRC() == l_crc_check) {
    // Read our object from the EEPROM.
    objEEPROM obj_eeprom;
    EEPROM.get(i_eepromAddress, obj_eeprom);

    if(obj_eeprom.cross_the_streams > 0 && obj_eeprom.cross_the_streams != 255) {
      if(obj_eeprom.cross_the_streams > 1) {
        b_cross_the_streams = true;
      }
      else {
        b_cross_the_streams = false;
      }
    }

    if(obj_eeprom.cross_the_streams_mix > 0 && obj_eeprom.cross_the_streams_mix != 255) {
      if(obj_eeprom.cross_the_streams_mix > 1) {
        b_cross_the_streams_mix = true;
      }
      else {
        b_cross_the_streams_mix = false;
      }
    }

    if(obj_eeprom.overheating > 0 && obj_eeprom.overheating != 255) {
      if(obj_eeprom.overheating > 1) {
        b_overheat_enabled = true;
      }
      else {
        b_overheat_enabled = false;
      }
    }

    if(obj_eeprom.neutrona_wand_sounds > 0 && obj_eeprom.neutrona_wand_sounds != 255) {
      if(obj_eeprom.neutrona_wand_sounds > 1) {
        b_extra_pack_sounds = true;
      }
      else {
        b_extra_pack_sounds = false;
      }
    }

    if(obj_eeprom.spectral_mode > 0 && obj_eeprom.spectral_mode != 255) {
      if(obj_eeprom.spectral_mode > 1) {
        b_spectral_mode_enabled = true;
        b_spectral_custom_mode_enabled = true;
      }
      else {
        b_spectral_mode_enabled = false;
        b_spectral_custom_mode_enabled = false;
      }
    }

    if(obj_eeprom.holiday_mode > 0 && obj_eeprom.holiday_mode != 255) {
      if(obj_eeprom.holiday_mode > 1) {
        b_holiday_mode_enabled = true;
      }
      else {
        b_holiday_mode_enabled = false;
      }
    }

    if(obj_eeprom.quick_vent > 0 && obj_eeprom.quick_vent != 255) {
      if(obj_eeprom.quick_vent > 1) {
        b_quick_vent = true;
      }
      else {
        b_quick_vent = false;
      }
    }

    if(obj_eeprom.wand_boot_errors > 0 && obj_eeprom.wand_boot_errors != 255) {
      if(obj_eeprom.wand_boot_errors > 1) {
        b_wand_boot_errors = true;
      }
      else {
        b_wand_boot_errors = false;
      }
    }

    if(obj_eeprom.vent_light_auto_intensity > 0 && obj_eeprom.vent_light_auto_intensity != 255) {
      if(obj_eeprom.vent_light_auto_intensity > 1) {
        b_vent_light_control = true;
      }
      else {
        b_vent_light_control = false;
      }
    }

    if(obj_eeprom.num_barrel_leds > 0 && obj_eeprom.num_barrel_leds != 255) {
      //i_num_barrel_leds = obj_eeprom.num_barrel_leds; // Keep it disabled for now until new barrel leds are ready.
    }

    if(obj_eeprom.invert_bargraph > 0 && obj_eeprom.invert_bargraph != 255) {
      if(obj_eeprom.invert_bargraph > 1) {
        b_bargraph_invert = true;
      }
      else {
        b_bargraph_invert = false;
      }

      setBargraphOrientation();
    }

    if(obj_eeprom.bargraph_mode > 0 && obj_eeprom.bargraph_mode != 255) {
      switch(obj_eeprom.bargraph_mode) {
        case 3:
          BARGRAPH_MODE = BARGRAPH_ORIGINAL;
          BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_ORIGINAL;
        break;

        case 2:
          BARGRAPH_MODE = BARGRAPH_SUPER_HERO;
          BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_SUPER_HERO;
        break;

        case 1:
        default:
          BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_DEFAULT;
        break;
      }
    }

    if(obj_eeprom.bargraph_mode > 0 && obj_eeprom.bargraph_mode != 255) {
      switch(obj_eeprom.bargraph_mode) {
        case 3:
          BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_ORIGINAL;
          BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_ORIGINAL;
        break;

        case 2:
          BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_SUPER_HERO;
          BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_SUPER_HERO;
        break;

        case 1:
        default:
          BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_DEFAULT;
        break;
      }
    }

    // Update the bargraph settings again after loading EEPROM setting data for it.
    bargraphYearModeUpdate();

    // Read our LED object from the EEPROM.
    objLEDEEPROM obj_led_eeprom;
    unsigned int i_eepromLEDAddress = EEPROM.length() / 2;

    EEPROM.get(i_eepromLEDAddress, obj_led_eeprom);
    if(obj_led_eeprom.barrel_spectral_custom > 0 && obj_led_eeprom.barrel_spectral_custom != 255) {
      i_spectral_wand_custom = obj_led_eeprom.barrel_spectral_custom;
    }

    if(obj_led_eeprom.barrel_spectral_saturation_custom > 0 && obj_led_eeprom.barrel_spectral_saturation_custom != 255) {
      i_spectral_wand_saturation_custom = obj_led_eeprom.barrel_spectral_saturation_custom;
    }
  }
}

// Helper method to play a sound effect using certain defaults.
void playEffect(int i_track_id, bool b_track_loop, int8_t i_track_volume, bool b_fade_in, unsigned int i_fade_time) {
  if(i_track_volume < i_volume_abs_min) {
    i_track_volume = i_volume_abs_min;
  }
  if(i_track_volume > 10) {
    i_track_volume = i_volume_abs_max;
  }

  if(b_sync != true) {
    if(b_fade_in == true) {
      w_trig.trackGain(i_track_id, i_volume_abs_min);
      w_trig.trackPlayPoly(i_track_id, true);
      w_trig.trackFade(i_track_id, i_track_volume, i_fade_time, 0);
    }
    else {
      w_trig.trackGain(i_track_id, i_track_volume);
      w_trig.trackPlayPoly(i_track_id, true);
    }

    if(b_track_loop == true) {
      w_trig.trackLoop(i_track_id, 1);
    }
    else {
      w_trig.trackLoop(i_track_id, 0);
    }
  }
}

void stopEffect(int i_track_id) {
  w_trig.trackStop(i_track_id);
}

// Helper method to play a music track using certain defaults.
void playMusic() {
  if(i_music_count > 0 && i_current_music_track >= i_music_track_start) {
    b_playing_music = true;

    // Loop the music track.
    if(b_repeat_track == true) {
      w_trig.trackLoop(i_current_music_track, 1);
    }
    else {
      w_trig.trackLoop(i_current_music_track, 0);
    }

    w_trig.trackGain(i_current_music_track, i_volume_music);
    w_trig.trackPlayPoly(i_current_music_track, true);

    w_trig.update();
  }
}

void stopMusic() {
  b_playing_music = false;

  if(i_music_count > 0) {
    w_trig.trackStop(i_current_music_track);
  }

  w_trig.update();
}

void pauseMusic() {
  if(i_music_count > 0 && i_current_music_track >= i_music_track_start) {
    w_trig.trackPause(i_current_music_track);
  }

  w_trig.update();
}

void resumeMusic() {
  if(i_music_count > 0 && i_current_music_track >= i_music_track_start) {
    w_trig.trackResume(i_current_music_track);
  }

  w_trig.update();
}

void setupWavTrigger() {
  // If the controller is powering the WAV Trigger, we should wait for the WAV Trigger to finish reset before trying to send commands.
  delay(1000);

  // WAV Trigger's startup at 57600
  w_trig.start();

  delay(10);

  // Stop all tracks.
  w_trig.stopAllTracks();

  // Reset the sample-rate offset, in case we have
  w_trig.samplerateOffset(0); // Reset our sample rate offset

  w_trig.masterGain(i_volume_master); // Reset the master gain db. 0db is default. Range is -70 to 0.
  w_trig.setAmpPwr(b_onboard_amp_enabled); // Turn on the onboard amp.

  // Enable track reporting from the WAV Trigger
  w_trig.setReporting(false);

  // Allow time for the WAV Trigger to respond with the version string and number of tracks.
  delay(350);

  unsigned int w_num_tracks = w_trig.getNumTracks();

  // Build the music track count.
  i_music_count = w_num_tracks - i_last_effects_track;

  if(i_music_count > 0) {
    i_current_music_track = i_music_track_start; // Set the first track of music as file 500_
  }
}