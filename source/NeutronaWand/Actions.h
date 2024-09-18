/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
            if(WAND_ACTION_STATUS != ACTION_OVERHEATING && WAND_ACTION_STATUS != ACTION_VENTING && b_pack_alarm != true) {
              // When ready to fire the hat light LED at the barrel tip lights up in Afterlife mode.
              if(b_switch_barrel_extended == true && switch_vent.on() == true && switch_wand.on() == true) {
                digitalWriteFast(BARREL_HAT_LED_PIN, HIGH);
              }
              else {
                digitalWriteFast(BARREL_HAT_LED_PIN, LOW);
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
      if(b_pack_on == true && b_pack_alarm == false) {
        if(STREAM_MODE == MESON) {
          if(ms_meson_blast.justFinished()) {
            playEffect(S_MESON_FIRE_PULSE, false, i_volume_effects, false, 0, false);
            wandSerialSend(W_MESON_FIRE_PULSE);

            if(WAND_BARREL_LED_COUNT == LEDS_48) {
              // Reset the barrel before starting a new pulse.
              barrelLightsOff();
            }

            ms_firing_stream_effects.start(0); // Start new barrel animation.

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
          }
        }

        if(b_firing == false) {
          b_firing = true;
          modeFireStart();
        }

        if(ms_hat_1.isRunning()) {
          if(ms_hat_1.remaining() < i_hat_1_delay / 2) {
            digitalWriteFast(TOP_HAT_LED_PIN, HIGH);
          }
          else {
            digitalWriteFast(TOP_HAT_LED_PIN, LOW);
          }

          if(ms_hat_1.justFinished()) {
            ms_hat_1.start(i_hat_1_delay);
          }
        }

        // Overheating check, start vent sequence if expected for power level and timer delay is completed.
        if(ms_overheat_initiate.justFinished()) {
          startVentSequence();
        }
        else {
          modeFiring(); // Tell the pack whether firing has started/stopped.

          // Stop firing if any of the main switches are turned off or the barrel is retracted.
          if(switch_vent.on() == false || switch_wand.on() == false || b_switch_barrel_extended != true) {
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
          if(b_extra_pack_sounds == true) {
            wandSerialSend(W_WAND_BEEP_SOUNDS);
          }

          ms_blink_sound_timer_1.start(i_blink_sound_timer);

          playEffect(S_BEEPS_LOW, false, i_volume_effects, false, 0, false);
          playEffect(S_BEEPS, false, i_volume_effects, false, 0, false);
        }

        if(ms_blink_sound_timer_2.justFinished()) {
          if(b_extra_pack_sounds == true) {
            wandSerialSend(W_WAND_BEEP_BARGRAPH);
          }

          playEffect(S_BEEPS_BARGRAPH, false, i_volume_effects, false, 0, false);

          ms_blink_sound_timer_2.start(i_blink_sound_timer * 4);
        }
      }
      else {
        // Prepare to make the bargraph ramp down.
        if(ms_bargraph.justFinished()) {
          bargraphRampUp();
        }
      }

      if(ms_overheating.justFinished()) {
        overheatingFinished();
      }
    break;

    case ACTION_VENTING:
      // Since the Proton Pack tells the Neutrona Wand when venting is finished, standalone wand needs its own timer.
      if(ms_overheating.justFinished()) {
        quickVentFinished();
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
        // Level 1 Intensify: Clear the Proton Pack EEPROM settings and exit.
        // Level 1 Barrel Wing Button: Save the current settings to the Proton Pack EEPROM and exit.
        // Level 2 Intensify: Video Game Neutrona Wand lights toggle.
        case 5:
          // Tell the Proton Pack to clear the EEPROM settings and exit.
          if(switch_intensify.pushed()) {
            switch(WAND_MENU_LEVEL) {
              case MENU_LEVEL_2:
                // Save this space for the video game Neutrona Wand lights.
              break;

              case MENU_LEVEL_1:
              default:
                // Tell pack to clear the EEPROM and exit.
                wandSerialSend(W_CLEAR_LED_EEPROM_SETTINGS);
                wandSerialSend(W_SPECTRAL_LIGHTS_OFF);

                stopEffect(S_VOICE_EEPROM_ERASE);
                playEffect(S_VOICE_EEPROM_ERASE);

                clearLEDEEPROM();

                wandExitEEPROMMenu();
              break;
            }
          }
          else if(switch_mode.pushed()) {
            switch(WAND_MENU_LEVEL) {
              case MENU_LEVEL_2:
              break;

              case MENU_LEVEL_1:
              default:
                // Tell the Proton Pack to save the current settings to the EEPROM and exit.
                wandSerialSend(W_SAVE_LED_EEPROM_SETTINGS);
                wandSerialSend(W_SPECTRAL_LIGHTS_OFF);

                stopEffect(S_VOICE_EEPROM_SAVE);
                playEffect(S_VOICE_EEPROM_SAVE);

                saveLEDEEPROM();

                wandExitEEPROMMenu();
              break;
            }
          }
        break;

        // Level 1 Intensify: Cycle through the different Neutrona Wand barrel LED counts.
        // Level 1 Barrel Wing Button: Adjust the Neutrona Wand barrel colour hue. <- Controlled by checkRotaryEncoder()
        // Level 2 Intensify: Toggle between 28-segment and 30-segment bargraph LEDs.
        case 4:
          if(switch_intensify.pushed()) {
            switch(WAND_MENU_LEVEL) {
              case MENU_LEVEL_2:
                if(BARGRAPH_TYPE_EEPROM != SEGMENTS_30) {
                  // Switch to 30-segment bargraph.
                  BARGRAPH_TYPE_EEPROM = SEGMENTS_30;

                  stopEffect(S_BARGRAPH_28_SEGMENTS);
                  stopEffect(S_BARGRAPH_30_SEGMENTS);

                  playEffect(S_BARGRAPH_30_SEGMENTS);

                  wandSerialSend(W_BARGRAPH_30_SEGMENTS);
                }
                else {
                  // Switch to 28-segment bargraph.
                  BARGRAPH_TYPE_EEPROM = SEGMENTS_28;

                  stopEffect(S_BARGRAPH_28_SEGMENTS);
                  stopEffect(S_BARGRAPH_30_SEGMENTS);

                  playEffect(S_BARGRAPH_28_SEGMENTS);

                  wandSerialSend(W_BARGRAPH_28_SEGMENTS);
                }

                if(BARGRAPH_TYPE != SEGMENTS_5) {
                  // Only toggle between segment types if not on a stock Hasbro bargraph.
                  BARGRAPH_TYPE = BARGRAPH_TYPE_EEPROM;
                }
              break;

              case MENU_LEVEL_1:
              default:
                switch(i_num_barrel_leds) {
                  case 5:
                  default:
                    wandBarrelLightsOff();
                    wandTipOff();

                    WAND_BARREL_LED_COUNT = LEDS_48;
                    i_num_barrel_leds = 48;

                    wandBarrelSpectralCustomConfigOn();

                    stopEffect(S_VOICE_BARREL_LED_48);
                    stopEffect(S_VOICE_BARREL_LED_5);

                    playEffect(S_VOICE_BARREL_LED_48);

                    wandSerialSend(W_BARREL_LEDS_48);
                  break;

                  case 48:
                    wandBarrelLightsOff();
                    wandTipOff();

                    WAND_BARREL_LED_COUNT = LEDS_5;
                    i_num_barrel_leds = 5;

                    wandBarrelSpectralCustomConfigOn();

                    stopEffect(S_VOICE_BARREL_LED_5);
                    stopEffect(S_VOICE_BARREL_LED_48);

                    playEffect(S_VOICE_BARREL_LED_5);

                    wandSerialSend(W_BARREL_LEDS_5);
                  break;
                }
              break;
            }
          }
        break;

        // Level 1 Intensify: Cycle through the different Power Cell LED counts.
        // Level 1 Barrel Wing Button: Adjust the Power Cell colour hue. <- Controlled by checkRotaryEncoder()
        // Level 2 Intensify: Toggle inverting of Power Cell LED direction (required for 1984 Power Cell).
        case 3:
          if(switch_intensify.pushed()) {
            switch(WAND_MENU_LEVEL) {
              case MENU_LEVEL_2:
                wandSerialSend(W_TOGGLE_POWERCELL_DIRECTION);
              break;

              case MENU_LEVEL_1:
              default:
                wandSerialSend(W_TOGGLE_POWERCELL_LEDS);
              break;
            }
          }
        break;

        // Level 1 Intensify: Cycle through the different Cyclotron LED counts.
        // Level 1 Barrel Wing Button: Adjust the Cyclotron colour hue. <- Controlled by checkRotaryEncoder()
        // Level 2 Intensify: Enable or disable the Inner Cyclotron LED Panel.
        case 2:
          if(switch_intensify.pushed()) {
            switch(WAND_MENU_LEVEL) {
              case MENU_LEVEL_2:
                wandSerialSend(W_TOGGLE_INNER_CYCLOTRON_PANEL);
              break;

              case MENU_LEVEL_1:
              default:
                wandSerialSend(W_TOGGLE_CYCLOTRON_LEDS);
              break;
            }
          }
        break;


        // Level 1 Intensify: Cycle through the different inner Cyclotron LED counts.
        // Level 1 Barrel Wing Button: Adjust the Inner Cyclotron colour hue. <- Controlled by checkRotaryEncoder()
        // Level 2 Intensify: Enable or disable GRB mode for the inner Cyclotron LEDs.
        case 1:
          if(switch_intensify.pushed()) {
            switch(WAND_MENU_LEVEL) {
              case MENU_LEVEL_2:
                wandSerialSend(W_TOGGLE_RGB_INNER_CYCLOTRON_LEDS);
              break;

              case MENU_LEVEL_1:
              default:
                wandSerialSend(W_TOGGLE_INNER_CYCLOTRON_LEDS);
              break;
            }
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
        // Menu Level 3: Barrel Wing Button: Set Neutrona Wand year mode (84/89/AL/FE/Match Proton Pack).
        // Menu Level 4: Intensify + top dial: Adjust overheat smoke duration by 1 second : Power Level 5
        // Menu Level 4: Barrel Wing Button + top dial: Adjust overheat start timer by 1 second : Power Level 5
        // Menu Level 5: Intensify: Enable/Disable overheat in power level #5
        // Menu Level 5: Barrel Wing Button: Enable/Disable continuous smoke in power level #5
        case 5:
          // Tell the Neutrona Wand to clear the EEPROM settings and exit.
          if(switch_intensify.pushed()) {
            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              // Tell the Proton Pack to clear its current configuration from the EEPROM.
              wandSerialSend(W_CLEAR_CONFIG_EEPROM_SETTINGS);

              stopEffect(S_VOICE_EEPROM_ERASE);
              playEffect(S_VOICE_EEPROM_ERASE);

              // Clear wand EEPROM.
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
              // Adjustment is handled in checkRotaryEncoder()
              stopEffect(S_VOICE_DEFAULT_SYSTEM_VOLUME_ADJUSTMENT);
              playEffect(S_VOICE_DEFAULT_SYSTEM_VOLUME_ADJUSTMENT);

              wandSerialSend(W_SOUND_DEFAULT_SYSTEM_VOLUME_ADJUSTMENT);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Overheat smoke duration level 5.
              // Adjustment is handled in checkRotaryEncoder()
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);
              playEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);

              wandSerialSend(W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_5);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              if(b_overheat_level_5 == true) {
                b_overheat_level_5 = false;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_5_DISABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_5_ENABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_5_DISABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_5_DISABLED);
              }
              else {
                b_overheat_level_5 = true;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_5_ENABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_5_DISABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_5_ENABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_5_ENABLED);
              }

              resetOverheatLevels();
            }
          }
          else if(switch_mode.pushed()) {
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
                  // 1984 -> 1989
                  WAND_YEAR_MODE = YEAR_1989;

                  stopEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1989);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1984);

                  playEffect(S_VOICE_NEUTRONA_WAND_1989);

                  wandSerialSend(W_NEUTRONA_WAND_1989_MODE);
                break;

                case YEAR_1989:
                  // 1989 -> Afterlife
                  WAND_YEAR_MODE = YEAR_AFTERLIFE;

                  stopEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1989);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1984);

                  playEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);

                  wandSerialSend(W_NEUTRONA_WAND_AFTERLIFE_MODE);
                break;

                case YEAR_AFTERLIFE:
                  // Afterlife -> Frozen Empire
                  WAND_YEAR_MODE = YEAR_FROZEN_EMPIRE;

                  stopEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1989);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1984);

                  playEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);

                  wandSerialSend(W_NEUTRONA_WAND_FROZEN_EMPIRE_MODE);
                break;

                case YEAR_FROZEN_EMPIRE:
                  // Frozen Empire -> Default (Toggle)
                  WAND_YEAR_MODE = YEAR_DEFAULT;

                  stopEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1989);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1984);

                  playEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);

                  wandSerialSend(W_NEUTRONA_WAND_DEFAULT_MODE);
                break;

                case YEAR_DEFAULT:
                default:
                  // Default (Toggle) -> 1984
                  WAND_YEAR_MODE = YEAR_1984;

                  stopEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1989);
                  stopEffect(S_VOICE_NEUTRONA_WAND_1984);

                  playEffect(S_VOICE_NEUTRONA_WAND_1984);

                  wandSerialSend(W_NEUTRONA_WAND_1984_MODE);
                break;
              }
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Handled in checkRotaryEncoder()
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
        // Menu Level 2: Intensify: Enable pack vibration, enable pack vibration while firing only, disable pack vibration, reset to defaults. *Note that the pack vibration switch will toggle both pack and wand vibration on or off*
        // Menu Level 2: Barrel Wing Button: Enable wand vibration, enable wand vibration while firing only, disable wand vibration, reset to defaults.
        // Menu Level 3: Intensify: Invert Bargraph
        // Menu Level 3: Barrel Wing Button: Toggle Bargraph Overheat Blinking enabled/disabled
        // Menu Level 4: Intensify + top dial: Adjust overheat smoke duration by 1 second : Power Level 4
        // Menu Level 4: Barrel Wing Button + top dial: Adjust overheat start timer by 1 second : Power Level 4
        // Menu Level 5: Intensify: Enable/Disable overheat in power level #4
        // Menu Level 5: Barrel Wing Button: Enable/Disable continuous smoke in power level #4
        case 4:
          if(switch_intensify.pushed()) {
            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              toggleWandModes();
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
              wandSerialSend(W_VIBRATION_CYCLE_TOGGLE_EEPROM);
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
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Overheat smoke duration level 4.
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);
              playEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);

              // Handled in checkRotaryEncoder()
              wandSerialSend(W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_4);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              if(b_overheat_level_4 == true) {
                b_overheat_level_4 = false;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_4_DISABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_4_ENABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_4_DISABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_4_DISABLED);
              }
              else {
                b_overheat_level_4 = true;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_4_ENABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_4_DISABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_4_ENABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_4_ENABLED);
              }

              resetOverheatLevels();
            }
          }

          if(switch_mode.pushed()) {
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
              stopEffect(S_BEEPS_ALT);

              playEffect(S_BEEPS_ALT);

              switch(VIBRATION_MODE_EEPROM) {
                case VIBRATION_DEFAULT:
                default:
                  VIBRATION_MODE_EEPROM = VIBRATION_ALWAYS;
                  VIBRATION_MODE = VIBRATION_MODE_EEPROM;
                  b_vibration_switch_on = true; // Override the Proton Pack vibration toggle switch.

                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DEFAULT);

                  playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);

                  wandSerialSend(W_VIBRATION_ENABLED);

                  ms_menu_vibration.start(250); // Confirmation buzz for 250ms.
                break;
                case VIBRATION_ALWAYS:
                  VIBRATION_MODE_EEPROM = VIBRATION_FIRING_ONLY;
                  VIBRATION_MODE = VIBRATION_MODE_EEPROM;
                  b_vibration_switch_on = true; // Override the Proton Pack vibration toggle switch.

                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DEFAULT);

                  playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);

                  wandSerialSend(W_VIBRATION_FIRING_ENABLED);

                  ms_menu_vibration.start(250); // Confirmation buzz for 250ms.
                break;
                case VIBRATION_FIRING_ONLY:
                  VIBRATION_MODE_EEPROM = VIBRATION_NONE;
                  VIBRATION_MODE = VIBRATION_MODE_EEPROM;

                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DEFAULT);

                  playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

                  wandSerialSend(W_VIBRATION_DISABLED);
                break;
                case VIBRATION_NONE:
                  VIBRATION_MODE_EEPROM = VIBRATION_DEFAULT;
                  VIBRATION_MODE = VIBRATION_FIRING_ONLY;

                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DEFAULT);

                  playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DEFAULT);

                  wandSerialSend(W_VIBRATION_DEFAULT);

                  ms_menu_vibration.start(250); // Confirmation buzz for 250ms.
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
              // Handled in checkRotaryEncoder()
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
        // Menu Level 2: Intensify: Enable/Disable Wand beeping in Afterlife / Frozen Empire modes.
        // Menu Level 2: Barrel Wing Button: Cycle through VG colour modes to disable them. (see operational guide for more details on this).
        // Menu Level 3: Intensify: Bargraph Idle Animation Toggle setting: Super Hero / Bargraph Original / System Default
        // Menu Level 3: Barrel Wing Button: Bargraph Firing Animation Toggle setting: Super Hero / Bargraph Original / System Default
        // Menu Level 4: Intensify + top dial: Adjust overheat smoke duration by 1 second : Power Level 3
        // Menu Level 4: Barrel Wing Button + top dial: Adjust overheat start timer by 1 second : Power Level 3
        // Menu Level 5: Intensify: Enable/Disable overheat in power level #3
        // Menu Level 5: Barrel Wing Button: Enable/Disable continuous smoke in power level #3
        case 3:
          if(switch_intensify.pushed()) {
            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              toggleOverheating();
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
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
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Overheat smoke duration level .
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);
              stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);
              playEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);

              // Handled in checkRotaryEncoder()
              wandSerialSend(W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_3);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              if(b_overheat_level_3 == true) {
                b_overheat_level_3 = false;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_3_DISABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_3_ENABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_3_DISABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_3_DISABLED);
              }
              else {
                b_overheat_level_3 = true;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_3_ENABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_3_DISABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_3_ENABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_3_ENABLED);
              }

              resetOverheatLevels();
            }
          }

          if(switch_mode.pushed()) {
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
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Handled in checkRotaryEncoder()
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
        // Menu Level 4: Intensify + top dial: Adjust overheat smoke duration by 1 second : Power Level 2
        // Menu Level 4: Barrel Wing Button + top dial: Adjust overheat start timer by 1 second : Power Level 2
        // Menu Level 5: Intensify: Enable/Disable overheat in power level #2
        // Menu Level 5: Barrel Wing Button: Enable/Disable continuous smoke in power level #2
        case 2:
          if(switch_intensify.pushed()) {
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

              // Handled in checkRotaryEncoder()
              wandSerialSend(W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_2);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              if(b_overheat_level_2 == true) {
                b_overheat_level_2 = false;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_2_DISABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_2_ENABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_2_DISABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_2_DISABLED);
              }
              else {
                b_overheat_level_2 = true;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_2_ENABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_2_DISABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_2_ENABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_2_ENABLED);
              }

              resetOverheatLevels();
            }
          }

          // Barrel Wing Button: Enable/Disable Ring Simulation in the Cyclotron LEDs in Afterlife (2021) mode.
          if(switch_mode.pushed()) {
            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              wandSerialSend(W_CYCLOTRON_SIMULATE_RING_TOGGLE);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
              // Sub menu.
              wandSerialSend(W_OVERHEAT_LIGHTS_OFF_TOGGLE);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_3) {
              // Tell the Proton Pack to toggle the Single LED or 3 LEDs for 1984/1989 modes.
              wandSerialSend(W_CYCLOTRON_LED_TOGGLE); // Move this to the LED menu in the future.
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Handled in checkRotaryEncoder()
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

        // Menu Level 1: Intensify: Enable or disable extra Neutrona Wand Sounds.
        // Menu Level 1: Barrel Wing Button: Enable or disable Proton Stream Impact Effects.
        // Menu Level 2: Intensify: 1984 / 1989 / Afterlife / Frozen Empire / Default (Proton Pack toggle switch) year mode selection.
        // Menu Level 2: Barrel Wing Button: Overheat sync to fan.
        // Menu Level 3: Intensify: Toggle between Super Hero and Original Mode.
        // Menu Level 3: Barrel Wing Button: Toggle CTS between: 1984 / 1989 / Afterlife / Frozen Empire CTS | Default CTS (Based on the year you are in)
        // Menu Level 4: Intensify + top dial: Adjust overheat smoke duration by 1 second : Power Level 1
        // Menu Level 4: Barrel Wing Button + top dial: Adjust overheat start timer by 1 second : Power Level 1
        // Menu Level 5: Intensify: Enable/Disable overheat in power level #1
        // Menu Level 5: Barrel Wing Button: Enable/Disable continuous smoke in power level #1
        case 1:
          if(switch_intensify.pushed()) {
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
              // Sub menu.
              wandSerialSend(W_YEAR_MODES_CYCLE_EEPROM);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_3) {
              // Toggle between Super Hero and Mode Original.
              wandSerialSend(W_MODE_TOGGLE);

              // If there is no Pack, we need to cycle modes manually.
              if(b_gpstar_benchtest == true) {
                if(SYSTEM_MODE == MODE_SUPER_HERO) {
                  SYSTEM_MODE = MODE_ORIGINAL;

                  stopEffect(S_VOICE_MODE_ORIGINAL);
                  stopEffect(S_VOICE_MODE_SUPER_HERO);
                  playEffect(S_VOICE_MODE_ORIGINAL);
                }
                else {
                  SYSTEM_MODE = MODE_SUPER_HERO;

                  stopEffect(S_VOICE_MODE_SUPER_HERO);
                  stopEffect(S_VOICE_MODE_ORIGINAL);
                  playEffect(S_VOICE_MODE_SUPER_HERO);
                }

                vgModeCheck();
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

              // Handled in checkRotaryEncoder()
              wandSerialSend(W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_1);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_5) {
              if(b_overheat_level_1 == true) {
                b_overheat_level_1 = false;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_1_DISABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_1_ENABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_1_DISABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_1_DISABLED);
              }
              else {
                b_overheat_level_1 = true;

                stopEffect(S_VOICE_OVERHEAT_LEVEL_1_ENABLED);
                stopEffect(S_VOICE_OVERHEAT_LEVEL_1_DISABLED);
                playEffect(S_VOICE_OVERHEAT_LEVEL_1_ENABLED);

                wandSerialSend(W_OVERHEAT_LEVEL_1_ENABLED);
              }

              resetOverheatLevels();
            }
          }

          if(switch_mode.pushed()) {
            if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
              // Tell the Proton Pack to toggle the Proton Stream impact effects.
              wandSerialSend(W_PROTON_STREAM_IMPACT_TOGGLE);

              // Standalone Neutrona Wand has to change this setting on its own.
              if(b_gpstar_benchtest == true) {
                if(b_stream_effects == true) {
                  b_stream_effects = false;

                  stopEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
                  stopEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);
                  playEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);
                }
                else {
                  b_stream_effects = true;

                  stopEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
                  stopEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);
                  playEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
                }
              }
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
              wandSerialSend(W_OVERHEAT_SYNC_TO_FAN_TOGGLE);
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_3) {
              switch(WAND_YEAR_CTS) {
                case CTS_1984:
                  WAND_YEAR_CTS = CTS_AFTERLIFE;

                  stopEffect(S_VOICE_CTS_1984);
                  stopEffect(S_VOICE_CTS_AFTERLIFE);
                  stopEffect(S_VOICE_CTS_DEFAULT);

                  playEffect(S_VOICE_CTS_AFTERLIFE);

                  wandSerialSend(W_CTS_AFTERLIFE);
                break;

                case CTS_AFTERLIFE:
                  WAND_YEAR_CTS = CTS_DEFAULT;

                  stopEffect(S_VOICE_CTS_1984);
                  stopEffect(S_VOICE_CTS_AFTERLIFE);
                  stopEffect(S_VOICE_CTS_DEFAULT);

                  playEffect(S_VOICE_CTS_DEFAULT);

                  wandSerialSend(W_CTS_DEFAULT);
                break;

                case CTS_DEFAULT:
                default:
                  WAND_YEAR_CTS = CTS_1984;

                  stopEffect(S_VOICE_CTS_1984);
                  stopEffect(S_VOICE_CTS_AFTERLIFE);
                  stopEffect(S_VOICE_CTS_DEFAULT);

                  playEffect(S_VOICE_CTS_1984);

                  wandSerialSend(W_CTS_1984);
                break;
              }
            }
            else if(WAND_MENU_LEVEL == MENU_LEVEL_4) {
              // Handled in checkRotaryEncoder()
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
        // Menu Level 1: (Barrel Wing Button) -> Exit menu. <--handled by altWingButtonCheck() if wand is on, or mainLoop() if wand is off
        // Menu Level 2: (Intensify) -> Enable or disable crossing the streams / video game modes.
        // Menu Level 2: (Barrel Wing Button) -> Enable/Disable Video Game Colour Modes for the Proton Pack LEDs (when video game mode is selected).
        case 5:
        // Music track loop setting.
        if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
          if(switch_intensify.pushed()) {
            toggleMusicLoop();

            // Tell pack to loop the music track.
            wandSerialSend(W_MUSIC_TRACK_LOOP_TOGGLE);
          }
        }
        else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
          if(switch_intensify.pushed()) {
            toggleWandModes();
          }

          // Enable/Disable Video Game Colour Modes for the Proton Pack LEDs.
          if(switch_mode.pushed()) {
            if(FIRING_MODE == VG_MODE) {
              // Tell the Proton Pack to cycle through the Video Game Colour toggles.
              wandSerialSend(W_VIDEO_GAME_MODE_COLOUR_TOGGLE);
            }
          }
        }
        break;

        // Menu Level 1: (Intensify + Top dial) -> Adjust the LED dimming of the Power Cell, Cyclotron and Inner Cyclotron.
        // Menu Level 1: (Barrel Wing Button) -> Cycle through which dimming mode to adjust in the Proton Pack. Power Cell, Cyclotron, Inner Cyclotron.
        // Menu Level 2: (Intensify) -> Enable or disable overheating.
        // Menu Level 2: (Barrel Wing Button) -> Enable or disable smoke for the Proton Pack.
        case 4:
          // Adjust the Proton Pack / Neutrona Wand sound effects volume.
          if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
            // Cycle through the dimming modes in the Proton Pack. (Power Cell, Cyclotron and Inner Cyclotron). Actual control of the dimming is handled in checkRotaryEncoder().
            if(switch_mode.pushed()) {
              // Tell the Proton Pack to change to the next dimming mode.
              wandSerialSend(W_DIMMING_TOGGLE);
            }
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
            // Enable or disable overheating.
            if(switch_intensify.pushed()) {
              toggleOverheating();
            }

            // Enable or disable smoke for the Proton Pack.
            if(switch_mode.pushed()) {
              // Tell the Proton Pack to toggle the smoke on or off.
              wandSerialSend(W_SMOKE_TOGGLE);
            }
          }
        break;

        // Menu Level 1: (Intensify + Top dial) -> Adjust Proton Pack / Neutrona Wand sound effects.
        // Menu Level 1: (Barrel Wing Button + top dial) Adjust Proton Pack / Neutrona Wand music volume.
        // Menu Level 2: (Intensify) -> Toggle Cyclotron rotation direction.
        // Menu Level 2: (Barrel Wing Button) -> Toggle the Proton Pack Single LED or 3 LEDs for 1984/1989 modes.
        case 3:
          // Top menu code is handled in checkRotaryEncoder()
          // Sub menu. Adjust Cyclotron settings.
          if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
            if(switch_intensify.pushed()) {
              // Tell the Proton Pack to change the Cyclotron rotation direction.
              wandSerialSend(W_CYCLOTRON_DIRECTION_TOGGLE);
            }

            if(switch_mode.pushed()) {
              // Tell the Proton Pack to toggle the Single LED or 3 LEDs for 1984/1989 modes.
              wandSerialSend(W_CYCLOTRON_LED_TOGGLE);
            }
          }
        break;

        // Menu Level 1: (Intensify) -> Go to next music track.
        // Menu Level 1: (Barrel Wing Button) -> Go to previous music track.
        // Menu Level 2: (Intensify) -> Enable pack vibration, enable pack vibration while firing only, disable pack vibration. *Note that the pack vibration switch will toggle both pack and wand vibration on or off*
        // Menu Level 2: (Barrel Wing Button) -> Enable wand vibration, enable wand vibration while firing only, disable wand vibration.
        case 2:
          // Change music tracks.
          if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
            if(switch_intensify.pushed()) {
              if(b_gpstar_benchtest == true) {
                musicNextTrack();
              }
              else {
                // Tell the pack to play the next track.
                wandSerialSend(W_MUSIC_NEXT_TRACK);
              }
            }

            if(switch_mode.pushed()) {
              if(b_gpstar_benchtest == true) {
                musicPrevTrack();
              }
              else {
                // Tell the pack to play the previous track.
                wandSerialSend(W_MUSIC_PREV_TRACK);
              }
            }
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
            // Enable or disable vibration for the pack or during firing only.
            if(switch_intensify.pushed()) {
              wandSerialSend(W_VIBRATION_CYCLE_TOGGLE);
            }

            // Enable or disable vibration or firing vibration only for the wand.
            if(switch_mode.pushed()) {
              stopEffect(S_BEEPS_ALT);
              playEffect(S_BEEPS_ALT);

              switch(VIBRATION_MODE) {
                case VIBRATION_ALWAYS:
                  VIBRATION_MODE = VIBRATION_FIRING_ONLY;
                  b_vibration_switch_on = true; // Override the Proton Pack vibration toggle switch.

                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

                  playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);

                  wandSerialSend(W_VIBRATION_FIRING_ENABLED);

                  ms_menu_vibration.start(250); // Confirmation buzz for 250ms.
                break;
                case VIBRATION_FIRING_ONLY:
                default:
                  VIBRATION_MODE = VIBRATION_NONE;

                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

                  playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

                  wandSerialSend(W_VIBRATION_DISABLED);
                break;
                case VIBRATION_NONE:
                  VIBRATION_MODE = VIBRATION_ALWAYS;
                  b_vibration_switch_on = true; // Override the Proton Pack vibration toggle switch.

                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
                  stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

                  playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);

                  wandSerialSend(W_VIBRATION_ENABLED);

                  ms_menu_vibration.start(250); // Confirmation buzz for 250ms.
                break;
              }
            }
          }
        break;

        // Menu Level 1: (Intensify) -> Play music or stop music.
        // Menu Level 1: (Barrel Wing Button) -> Mute the Proton Pack and Neutrona Wand.
        // Menu Level 2: (Intensify) -> Switch between 1984/1989/Afterlife/Frozen Empire mode.
        // Menu Level 2: (Barrel Wing Button) -> Enable or disable Proton Stream impact effects.
        case 1:
          // Play or stop the current music track.
          if(WAND_MENU_LEVEL == MENU_LEVEL_1) {
            if(switch_intensify.pushed()) {
              if(b_playing_music == true) {
                stopMusic();
              }
              else {
                // Tell the pack to start or stop its music.
                wandSerialSend(W_MUSIC_TOGGLE);

                if(b_gpstar_benchtest == true) {
                  playMusic();
                }
              }
            }

            // Silence the Proton Pack and Neutrona Wand or revert back to previously-selected volume.
            if(switch_mode.pushed()) {
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

              resetMasterVolume();
            }
          }
          else if(WAND_MENU_LEVEL == MENU_LEVEL_2) {
            // Switch between 1984/1989/Afterlife/Frozen Empire mode.
            if(switch_intensify.pushed()) {
              // Tell the Proton Pack to cycle through year modes.
              wandSerialSend(W_YEAR_MODES_CYCLE);

              // There is no pack connected; let's change the years.
              if(b_gpstar_benchtest == true) {
                stopEffect(S_BEEPS_BARGRAPH);
                playEffect(S_BEEPS_BARGRAPH);

                switch(getNeutronaWandYearMode()) {
                  case SYSTEM_1984:
                    // 1984 -> 1989
                    WAND_YEAR_MODE = YEAR_1989;

                    stopEffect(S_VOICE_FROZEN_EMPIRE);
                    stopEffect(S_VOICE_AFTERLIFE);
                    stopEffect(S_VOICE_1989);
                    stopEffect(S_VOICE_1984);

                    playEffect(S_VOICE_1989);
                  break;

                  case SYSTEM_1989:
                    // 1989 -> Afterlife
                    WAND_YEAR_MODE = YEAR_AFTERLIFE;

                    stopEffect(S_VOICE_FROZEN_EMPIRE);
                    stopEffect(S_VOICE_AFTERLIFE);
                    stopEffect(S_VOICE_1989);
                    stopEffect(S_VOICE_1984);

                    playEffect(S_VOICE_AFTERLIFE);
                  break;

                  case SYSTEM_AFTERLIFE:
                  default:
                    // Afterlife -> Frozen Empire
                    WAND_YEAR_MODE = YEAR_FROZEN_EMPIRE;

                    stopEffect(S_VOICE_FROZEN_EMPIRE);
                    stopEffect(S_VOICE_AFTERLIFE);
                    stopEffect(S_VOICE_1989);
                    stopEffect(S_VOICE_1984);

                    playEffect(S_VOICE_FROZEN_EMPIRE);
                  break;

                  case SYSTEM_FROZEN_EMPIRE:
                    // Frozen Empire -> 1984
                    WAND_YEAR_MODE = YEAR_1984;

                    stopEffect(S_VOICE_FROZEN_EMPIRE);
                    stopEffect(S_VOICE_AFTERLIFE);
                    stopEffect(S_VOICE_1989);
                    stopEffect(S_VOICE_1984);

                    playEffect(S_VOICE_1984);
                  break;
                }
              }
            }

            if(switch_mode.pushed()) {
              // Tell the Proton Pack to toggle the Proton Stream Impact Effects.
              wandSerialSend(W_PROTON_STREAM_IMPACT_TOGGLE);

              // Standalone Neutrona Wand has to change this setting on its own.
              if(b_gpstar_benchtest == true) {
                if(b_stream_effects == true) {
                  b_stream_effects = false;

                  stopEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
                  stopEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);
                  playEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);
                }
                else {
                  b_stream_effects = true;

                  stopEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
                  stopEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);
                  playEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
                }
              }
            }
          }
        break;
      }
    break;
  }
}
