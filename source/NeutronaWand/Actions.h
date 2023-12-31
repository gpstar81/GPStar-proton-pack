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

#pragma once

void checkWandAction() {
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
        if(FIRING_MODE == MESON) {
          if(ms_meson_blast.justFinished()) {
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
          }
        }

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
        // Since the Proton Pack tells the Neutrona Wand when overheating is finished, if it is running with no Proton Pack then the Neutrona Wand needs to calculate when to finish.
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

    case ACTION_LED_EEPROM_MENU:
      settingsBlinkingLights();

      switch(i_wand_menu) {
        // Intensify: Clear the Proton Pack EEPROM settings and exit.
        // Barrel Wing Button: Save the current settings to the Proton Pack EEPROM and exit.
        case 5:
          // Tell the Proton Pack to clear the EEPROM settings and exit.
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            // Tell pack to clear the EEPROM and exit.
            wandSerialSend(W_CLEAR_LED_EEPROM_SETTINGS);
            wandSerialSend(W_SPECTRAL_LIGHTS_OFF);

            stopEffect(S_VOICE_EEPROM_ERASE);
            playEffect(S_VOICE_EEPROM_ERASE);

            clearLEDEEPROM();

            wandExitEEPROMMenu();
          }
          else if(switchMode() == true) {
            // Tell the Proton Pack to save the current settings to the EEPROM and exit.
            wandSerialSend(W_SAVE_LED_EEPROM_SETTINGS);
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
              clearConfigEEPROM();

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

              resetOverHeatModes();
            }
          }
          else if(switchMode() == true) {
            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              // Tell the Proton Pack to save its current configuration to the EEPROM.
              wandSerialSend(W_SAVE_CONFIG_EEPROM_SETTINGS);

              stopEffect(S_VOICE_EEPROM_SAVE);
              playEffect(S_VOICE_EEPROM_SAVE);

              // Save wand EEPROM. (CTS/VGA, Overheating)
              saveConfigEEPROM();

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

                  playEffect(S_VOICE_NEUTRONA_WAND_1989);

                  wandSerialSend(W_NEUTRONA_WAND_1989_MODE);
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

              resetOverHeatModes();
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
              switch(WAND_BARREL_LED_COUNT) {
                case LEDS_5:
                default:
                  /*
                  i_num_barrel_leds = 48;
                  WAND_BARREL_LED_COUNT = LEDS_48;

                  stopEffect(S_VOICE_BARREL_LED_5);
                  stopEffect(S_VOICE_BARREL_LED_48);
                  stopEffect(S_VOICE_BARREL_LED_60);
                  playEffect(S_VOICE_BARREL_LED_48);

                  wandSerialSend(W_BARREL_LEDS_48);
                  */
                  stopEffect(S_VOICE_BARREL_LED_5);
                  playEffect(S_VOICE_BARREL_LED_5);

                  wandSerialSend(W_BARREL_LEDS_5);
                break;

                // 48 LED wand barrel board coming soon.
                case LEDS_48:
                  /*
                  i_num_barrel_leds = 5;
                  WAND_BARREL_LED_COUNT = LEDS_5;

                  stopEffect(S_VOICE_BARREL_LED_5);
                  stopEffect(S_VOICE_BARREL_LED_48);
                  stopEffect(S_VOICE_BARREL_LED_60);
                  playEffect(S_VOICE_BARREL_LED_5);

                  wandSerialSend(W_BARREL_LEDS_5);
                  */

                  /*
                  i_num_barrel_leds = 60;
                  WAND_BARREL_LED_COUNT = LEDS_60;

                  stopEffect(S_VOICE_BARREL_LED_5);
                  stopEffect(S_VOICE_BARREL_LED_48);
                  stopEffect(S_VOICE_BARREL_LED_60);
                  playEffect(S_VOICE_BARREL_LED_60);

                  wandSerialSend(W_BARREL_LEDS_60);
                  */
                break;

                // The 60 LED flexi-pcb may come one day in the future.
                case LEDS_60:
                /*
                  i_num_barrel_leds = 5;
                  WAND_BARREL_LED_COUNT = LEDS_5;

                  stopEffect(S_VOICE_BARREL_LED_5);
                  stopEffect(S_VOICE_BARREL_LED_48);
                  stopEffect(S_VOICE_BARREL_LED_60);
                  playEffect(S_VOICE_BARREL_LED_5);

                  wandSerialSend(W_BARREL_LEDS_5);
                  */
                break;
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
              if(b_beep_loop == true) {
                b_beep_loop = false;

                stopEffect(S_VOICE_NEUTRONA_WAND_BEEPING_DISABLED);
                stopEffect(S_VOICE_NEUTRONA_WAND_BEEPING_ENABLED);
                playEffect(S_VOICE_NEUTRONA_WAND_BEEPING_DISABLED);

                wandSerialSend(W_MODE_BEEP_LOOP_DISABLED);
              }
              else {
                b_beep_loop = true;

                stopEffect(S_VOICE_NEUTRONA_WAND_BEEPING_DISABLED);
                stopEffect(S_VOICE_NEUTRONA_WAND_BEEPING_ENABLED);
                playEffect(S_VOICE_NEUTRONA_WAND_BEEPING_ENABLED);

                wandSerialSend(W_MODE_BEEP_LOOP_ENABLED);
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

              resetOverHeatModes();
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

              resetOverHeatModes();
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
              // Toggle between Super Hero and Mode Original.
              wandSerialSend(W_MODE_TOGGLE);
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

              resetOverHeatModes();
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
        // Menu Level 1: (Intensify) -> Music track loop setting.
        // Menu Level 2: (Intensify) -> Enable or disable crossing the streams / video game modes.
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

        // Menu Level 1: (Intensify + Top dial) -> Adjust the LED dimming of the Power Cell, Cyclotron and Inner Cyclotron.
        // Menu Level 1: (Barrel Wing Button) -> Cycle through which dimming mode to adjust in the Proton Pack. Power Cell, Cyclotron, Inner Cyclotron.
        // Menu Level 2: (Intensify) -> Enable or disable smoke for the Proton Pack.
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

        // Menu Level 1: (Intensify + Top dial) -> Adjust Proton Pack / Neutrona Wand sound effects. (Barrel Wing Button + top dial) Adjust Proton Pack / Neutrona Wand music volume.
        // Menu Level 1: (Intensify) -> Toggle Cyclotron rotation direction.
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

        // Menu Level 1: (Intensify) -> Go to next music track.
        // Menu Level 1: (Barrel Wing Button) -> Go to previous music track.
        // Menu Level 2: (Intensify) -> Enable pack vibration, enable pack vibration while firing only, disable pack vibration. *Note that the pack vibration switch will toggle both pack and wand vibiration on or off*
        // Menu Level 2: (Barrel Wing Button) -> Enable wand vibration, enable wand vibration while firing only, disable wand vibration.
        case 2:
          // Change music tracks.
          if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              if(b_no_pack == true) {
                musicNextTrack();
              }
              else {
                if(b_playing_music == true) {
                  stopMusic();
                }

                // Tell the pack to play the next track.
                wandSerialSend(W_MUSIC_NEXT_TRACK);
              }
            }

            if(switchMode() == true) {
              if(b_no_pack == true) {
                musicPrevTrack();
              }
              else {
                if(b_playing_music == true) {
                  stopMusic();
                }

                // Tell the pack to play the previous track.
                wandSerialSend(W_MUSIC_PREV_TRACK);
              }
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

        // Menu Level 1: (Intensify) -> Play music or stop music.
        // Menu Level 1: (Barrel Wing Button) -> Mute the Proton Pack and Neutrona Wand.
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

                if(b_no_pack == true) {
                  stopMusic();
                }
              }
              else {
                // Tell the pack to play music.
                wandSerialSend(W_MUSIC_START);

                if(b_no_pack == true) {
                  playMusic();
                }
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
                switch(getNeutronaWandYearMode()) {
                  case SYSTEM_1984:
                    WAND_YEAR_MODE = YEAR_1989;

                    stopEffect(S_VOICE_FROZEN_EMPIRE);
                    stopEffect(S_VOICE_AFTERLIFE);
                    stopEffect(S_VOICE_1984);
                    stopEffect(S_VOICE_1989);

                    playEffect(S_VOICE_1989);

                    bargraphYearModeUpdate();
                  break;

                  case SYSTEM_1989:
                    WAND_YEAR_MODE = YEAR_AFTERLIFE;

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
                    WAND_YEAR_MODE = YEAR_1984;

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
}