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

#pragma once

// Forward function declaration.
void wandSerialSend(uint8_t i_command, uint16_t i_value); // From Serial.h
void notifyWSClients(); // From Webhandler.h

/**
 * Centralized handler for commands, allowing the Pack and Wand to both perform the same action.
 * This approach is applying the Command Pattern to decouple the sender from the receiver.
 * In order for this to work, the command value must come from a unique source: PACK_MESSAGE
 *
 * Inputs:
 *   - i_command: Command identifier (PACK_MESSAGE enum)
 *   - i_value: Optional value for the command (default 0)
 */
void executeCommand(uint8_t i_command, uint16_t i_value = 0) {
  switch(i_command) {
    case P_ON:
      // Pack is on.
      b_pack_on = true;
    break;

    case P_OFF:
      // Pack is off.
      if(b_pack_on) {
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

    case P_SOUND_SUPER_HERO:
      stopEffect(S_VOICE_MODE_SUPER_HERO);
      stopEffect(S_VOICE_MODE_ORIGINAL);
      playEffect(S_VOICE_MODE_SUPER_HERO);
    break;

    case P_SOUND_MODE_ORIGINAL:
      stopEffect(S_VOICE_MODE_SUPER_HERO);
      stopEffect(S_VOICE_MODE_ORIGINAL);
      playEffect(S_VOICE_MODE_ORIGINAL);
    break;

    case P_MODE_SUPER_HERO:
      SYSTEM_MODE = MODE_SUPER_HERO;
      vgModeCheck(); // Re-check VG/CTS mode.
      updateStreamFlags(); // Update the stream flags.
      wandSerialSend(W_STREAM_FLAGS, STREAM_MODE_FLAG); // Send the updated flags upstream.
    break;

    case P_MODE_ORIGINAL:
      SYSTEM_MODE = MODE_ORIGINAL;
      vgModeCheck(); // Assert CTS mode.
      updateStreamFlags(); // Update the stream flags.
      wandSerialSend(W_STREAM_FLAGS, STREAM_MODE_FLAG); // Send the updated flags upstream.
    break;

    case P_OVERHEATING_FINISHED:
      if(WAND_STATUS != MODE_OFF) {
        overheatingFinished();
      }
    break;

    case P_VENTING_FINISHED:
      if(WAND_STATUS != MODE_OFF) {
        quickVentFinished();
      }
    break;

    case P_INNER_CYCLOTRON_PANEL_DISABLED:
      stopEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_STATIC_COLORS);
      stopEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_DYNAMIC_COLORS);
      stopEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_DISABLED);
      playEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_DISABLED);
    break;

    case P_INNER_CYCLOTRON_PANEL_STATIC:
      stopEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_STATIC_COLORS);
      stopEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_DYNAMIC_COLORS);
      stopEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_DISABLED);
      playEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_STATIC_COLORS);
    break;

    case P_INNER_CYCLOTRON_PANEL_DYNAMIC:
      stopEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_STATIC_COLORS);
      stopEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_DYNAMIC_COLORS);
      stopEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_DISABLED);
      playEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_DYNAMIC_COLORS);
    break;

    case P_ION_ARM_SWITCH_ON:
      changeIonArmSwitchState(true);
    break;

    case P_ION_ARM_SWITCH_OFF:
      changeIonArmSwitchState(false);
    break;

    case P_CYCLOTRON_LID_ON:
      b_pack_cyclotron_lid_on = true;
    break;

    case P_CYCLOTRON_LID_OFF:
      b_pack_cyclotron_lid_on = false;
    break;

    case P_MANUAL_OVERHEAT:
      if(WAND_STATUS == MODE_ON && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
        if(b_pack_on && !b_pack_alarm && b_overheat_enabled) {
          switch(getNeutronaWandYearMode()) {
            case SYSTEM_1984:
            case SYSTEM_1989:
              if(b_extra_pack_sounds) {
                wandSerialSend(W_EXTRA_WAND_SOUNDS_STOP);
              }
            break;

            case SYSTEM_AFTERLIFE:
            case SYSTEM_FROZEN_EMPIRE:
            default:
              if(!b_sound_idle) {
                stopAfterlifeSounds();
                playEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1);

                if(b_extra_pack_sounds) {
                  wandSerialSend(W_EXTRA_WAND_SOUNDS_STOP);
                  wandSerialSend(W_AFTERLIFE_GUN_RAMP_DOWN_1);
                }
              }
            break;
          }

          startVentSequence();
        }
      }
      else if(WAND_STATUS == MODE_OFF) {
        wandSerialSend(W_OVERHEATING);
      }
    break;

    case P_MUSIC_STATUS:
      // Received music status update, so set playing music variables accordingly.
      switch(i_value) {
        case 1:
        default:
          // Music stopped.
          b_playing_music = false;
          b_music_paused = false;
        break;
        case 2:
          // Music started.
          b_playing_music = true;
          b_music_paused = false;
        break;
        case 3:
          // Music resumed.
          b_playing_music = true;
          b_music_paused = false;
        break;
        case 4:
          // Music paused.
          b_playing_music = true;
          b_music_paused = true;
        break;
      }

      // If we are fully off we must also make sure to start/stop the power reminder.
      if(b_playing_music && !b_music_paused) {
        setPowerOnReminder(false);
      }
      else if(WAND_STATUS == MODE_OFF && WAND_ACTION_STATUS == ACTION_IDLE && !b_pack_on) {
        setPowerOnReminder(true);
      }
    break;

    case P_MUSIC_LOOP_STATUS:
      // The pack is telling us if the current music track is looped or not.
      b_repeat_track = i_value == 2;
    break;

    case P_MASTER_AUDIO_STATUS:
      switch(i_value) {
        case 1:
        default:
          // The pack is telling us to revert the volume to normal.
          i_volume_master = i_volume_revert;
        break;

        case 2:
          // Remember the current master volume level.
          i_volume_revert = i_volume_master;

          // The pack is telling us to be silent.
          i_volume_master = i_volume_abs_min;
        break;
      }

      updateMasterVolume();
    break;

    case P_ALARM_ON:
      // Alarm is on.
      b_pack_alarm = true;

      if(WAND_STATUS != MODE_ERROR && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
        if(WAND_STATUS == MODE_ON) {
          if(b_extra_pack_sounds) {
            wandSerialSend(W_WAND_SHUTDOWN_SOUND);
            wandSerialSend(W_EXTRA_WAND_SOUNDS_STOP);
          }

          stopEffect(S_WAND_SHUTDOWN);
          playEffect(S_WAND_SHUTDOWN);

          switch(getNeutronaWandYearMode()) {
            case SYSTEM_1984:
            case SYSTEM_1989:
              // Do nothing.
            break;

            case SYSTEM_AFTERLIFE:
            case SYSTEM_FROZEN_EMPIRE:
            default:
              if(!b_sound_idle) {
                stopAfterlifeSounds();
                playEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1);

                if(b_extra_pack_sounds) {
                  wandSerialSend(W_AFTERLIFE_GUN_RAMP_DOWN_1);
                }
              }
            break;
          }

          if(!b_firing) {
            // This is handled by modeFireStop() if firing when ribbon cable is removed.
            prepBargraphRampDown();
          }

          if(WAND_ACTION_STATUS == ACTION_SETTINGS) {
            // If the wand is in settings mode while the alarm is activated, exit the settings mode.
            switch(STREAM_MODE) {
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

              case HOLIDAY_HALLOWEEN:
                // Tell the pack we are in Halloween mode.
                wandSerialSend(W_HALLOWEEN_MODE);
              break;

              case HOLIDAY_CHRISTMAS:
                // Tell the pack we are in Christmas mode.
                wandSerialSend(W_CHRISTMAS_MODE);
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
          }
        }

        ms_error_blink.start(i_error_blink_delay); // Start the error blink timer.
      }
    break;

    case P_ALARM_OFF:
      if(WAND_STATUS != MODE_ERROR && b_pack_alarm) {
        resetHatLights(); // Reset the hat light states.

        if(WAND_STATUS == MODE_ON) {
          switch(SYSTEM_MODE) {
            case MODE_ORIGINAL:
              if(switch_vent.on() && switch_wand.on() && switch_activate.on()) {
                prepBargraphRampUp();
              }
            break;

            case MODE_SUPER_HERO:
            default:
              prepBargraphRampUp();
            break;
          }
        }

        if(WAND_STATUS == MODE_ON && WAND_ACTION_STATUS != ACTION_OVERHEATING && b_pack_on) {
          soundIdleLoop(true);

          if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
            stopEffect(S_WAND_BOOTUP);
            playEffect(S_WAND_BOOTUP);

            if(!switch_vent.on()) {
              afterlifeRampSound1();
            }
          }
        }
      }

      // Alarm is off.
      b_pack_alarm = false;
    break;

    case P_WARNING_CANCELLED:
      // Pack is telling wand to cancel any overheat warnings.
      // First, stop the timer which triggers the overheat.
      ms_overheat_initiate.stop();

      // Then reset the hat light states.
      resetHatLights();

      // Next, reset the cyclotron speed on all devices.
      wandSerialSend(W_CYCLOTRON_NORMAL_SPEED);
      cyclotronSpeedRevert();
    break;

    case P_VOLUME_SOUND_EFFECTS_INCREASE:
      // Increase effects volume.
      increaseVolumeEffects();
    break;

    case P_VOLUME_SOUND_EFFECTS_DECREASE:
      // Decrease effects volume.
      decreaseVolumeEffects();
    break;

    case P_VIBRATION_ENABLED:
      // Vibration enabled (from Proton Pack vibration toggle switch).
      b_vibration_switch_on = true;

      stopEffect(S_BEEPS_ALT);
      playEffect(S_BEEPS_ALT);

      stopEffect(S_VOICE_VIBRATION_ENABLED);
      stopEffect(S_VOICE_VIBRATION_DISABLED);
      playEffect(S_VOICE_VIBRATION_ENABLED);
    break;

    case P_VIBRATION_DISABLED:
      // Vibration disabled (from Proton Pack vibration toggle switch).
      b_vibration_switch_on = false;

      stopEffect(S_BEEPS_ALT);
      playEffect(S_BEEPS_ALT);

      stopEffect(S_VOICE_VIBRATION_DISABLED);
      stopEffect(S_VOICE_VIBRATION_ENABLED);
      playEffect(S_VOICE_VIBRATION_DISABLED);

      vibrationOff();
    break;

    case P_PACK_VIBRATION_ENABLED:
      // Proton Pack Vibration enabled.
      stopEffect(S_BEEPS_ALT);
      playEffect(S_BEEPS_ALT);

      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DEFAULT);
      stopEffect(S_VOICE_MOTORIZED_CYCLOTRON_ENABLED);
      playEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
    break;

    case P_PACK_VIBRATION_DISABLED:
      // Proton Pack Vibration disabled.
      stopEffect(S_BEEPS_ALT);
      playEffect(S_BEEPS_ALT);

      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DEFAULT);
      stopEffect(S_VOICE_MOTORIZED_CYCLOTRON_ENABLED);
      playEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);
    break;

    case P_PACK_VIBRATION_FIRING_ENABLED:
      // Proton Pack Vibration firing enabled.
      stopEffect(S_BEEPS_ALT);
      playEffect(S_BEEPS_ALT);

      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DEFAULT);
      stopEffect(S_VOICE_MOTORIZED_CYCLOTRON_ENABLED);
      playEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
    break;

    case P_PACK_VIBRATION_DEFAULT:
      // Proton Pack Vibration EEPROM reset to default.
      stopEffect(S_BEEPS_ALT);
      playEffect(S_BEEPS_ALT);

      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DEFAULT);
      stopEffect(S_VOICE_MOTORIZED_CYCLOTRON_ENABLED);
      playEffect(S_VOICE_PROTON_PACK_VIBRATION_DEFAULT);
    break;

    case P_PACK_MOTORIZED_CYCLOTRON_ENABLED:
      // Proton Pack Vibration EEPROM reset to default.
      stopEffect(S_BEEPS_ALT);
      playEffect(S_BEEPS_ALT);

      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DEFAULT);
      stopEffect(S_VOICE_MOTORIZED_CYCLOTRON_ENABLED);
      playEffect(S_VOICE_MOTORIZED_CYCLOTRON_ENABLED);
    break;

    case P_YEAR_1984:
      // Indicates system (pack) year is 1984 mode
      SYSTEM_YEAR = SYSTEM_1984;
      bargraphYearModeUpdate();
      resetWhiteLEDBlinkRate();
    break;

    case P_YEAR_1989:
      // Indicates system (pack) year is 1984 mode
      SYSTEM_YEAR = SYSTEM_1989;
      bargraphYearModeUpdate();
      resetWhiteLEDBlinkRate();
    break;

    case P_YEAR_AFTERLIFE:
      // Indicates system (pack) year is Afterlife mode
      SYSTEM_YEAR = SYSTEM_AFTERLIFE;
      bargraphYearModeUpdate();
      resetWhiteLEDBlinkRate();
    break;

    case P_YEAR_FROZEN_EMPIRE:
      // Indicates system (pack) year is Frozen Empire mode
      SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
      bargraphYearModeUpdate();
      resetWhiteLEDBlinkRate();
    break;

    case P_MODE_FROZEN_EMPIRE:
      // Play only the Frozen Empire voice
      stopEffect(S_BEEPS_BARGRAPH);
      stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1989);
      stopEffect(S_VOICE_1984);

      playEffect(S_BEEPS_BARGRAPH);
      playEffect(S_VOICE_FROZEN_EMPIRE);
    break;

    case P_MODE_AFTERLIFE:
      // Play only the Afterlife voice
      stopEffect(S_BEEPS_BARGRAPH);
      stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1989);
      stopEffect(S_VOICE_1984);

      playEffect(S_BEEPS_BARGRAPH);
      playEffect(S_VOICE_AFTERLIFE);
    break;

    case P_MODE_1989:
      // Play only the 1989 voice
      stopEffect(S_BEEPS_BARGRAPH);
      stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1989);
      stopEffect(S_VOICE_1984);

      playEffect(S_BEEPS_BARGRAPH);
      playEffect(S_VOICE_1989);
    break;

    case P_MODE_1984:
      // Play only the 1984 voice
      stopEffect(S_BEEPS_BARGRAPH);
      stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1989);
      stopEffect(S_VOICE_1984);

      playEffect(S_BEEPS_BARGRAPH);
      playEffect(S_VOICE_1984);
    break;

    case P_YEAR_MODE_DEFAULT:
      // Play only the default year voice
      stopEffect(S_VOICE_YEAR_MODE_DEFAULT);
      stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1984);
      stopEffect(S_VOICE_1989);

      playEffect(S_VOICE_YEAR_MODE_DEFAULT);
    break;

    case P_SET_STREAM_MODE:
      if(vgModeCheck()) {
        // Only change our stream mode if VG mode is actually enabled.
        switch(i_value) {
          case 1:
          default:
            STREAM_MODE = PROTON;
          break;
          case 2:
            STREAM_MODE = STASIS;
          break;
          case 3:
            STREAM_MODE = SLIME;
          break;
          case 4:
            STREAM_MODE = MESON;
          break;
          case 5:
            STREAM_MODE = SPECTRAL;
          break;
          case 6:
            STREAM_MODE = HOLIDAY_HALLOWEEN;
          break;
          case 7:
            STREAM_MODE = HOLIDAY_CHRISTMAS;
          break;
          case 8:
            STREAM_MODE = SPECTRAL_CUSTOM;
          break;
        }

        // Apply the change immediately.
        streamModeCheck();
      }
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

    case P_POWERCELL_NOT_INVERTED:
      stopEffect(S_VOICE_POWERCELL_NOT_INVERTED);
      stopEffect(S_VOICE_POWERCELL_INVERTED);

      playEffect(S_VOICE_POWERCELL_NOT_INVERTED);
    break;

    case P_POWERCELL_INVERTED:
      stopEffect(S_VOICE_POWERCELL_INVERTED);
      stopEffect(S_VOICE_POWERCELL_NOT_INVERTED);

      playEffect(S_VOICE_POWERCELL_INVERTED);
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
      stopEffect(S_VOICE_INNER_CYCLOTRON_PANEL_BRIGHTNESS);

      playEffect(S_VOICE_POWERCELL_BRIGHTNESS);
    break;

    case P_CYCLOTRON_DIMMING:
      stopEffect(S_VOICE_POWERCELL_BRIGHTNESS);
      stopEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
      stopEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);
      stopEffect(S_VOICE_INNER_CYCLOTRON_PANEL_BRIGHTNESS);

      playEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
    break;

    case P_INNER_CYCLOTRON_DIMMING:
      stopEffect(S_VOICE_POWERCELL_BRIGHTNESS);
      stopEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
      stopEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);
      stopEffect(S_VOICE_INNER_CYCLOTRON_PANEL_BRIGHTNESS);

      playEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);
    break;

    case P_CYCLOTRON_PANEL_DIMMING:
      stopEffect(S_VOICE_POWERCELL_BRIGHTNESS);
      stopEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
      stopEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);
      stopEffect(S_VOICE_INNER_CYCLOTRON_PANEL_BRIGHTNESS);

      playEffect(S_VOICE_INNER_CYCLOTRON_PANEL_BRIGHTNESS);
    break;

    case P_PROTON_STREAM_IMPACT_ENABLED:
      // Enables additional Proton Stream sparking sounds.
      b_stream_effects = true;

      stopEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
      stopEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);

      playEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
    break;

    case P_PROTON_STREAM_IMPACT_DISABLED:
      // Disables additional Proton Stream sparking sounds.
      b_stream_effects = false;

      stopEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
      stopEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);

      playEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);
    break;

    case P_CYCLOTRON_FADING_DISABLED:
      stopEffect(S_VOICE_CYCLOTRON_FADING_DISABLED);
      stopEffect(S_VOICE_CYCLOTRON_FADING_ENABLED);

      playEffect(S_VOICE_CYCLOTRON_FADING_DISABLED);
    break;

    case P_CYCLOTRON_FADING_ENABLED:
      stopEffect(S_VOICE_CYCLOTRON_FADING_DISABLED);
      stopEffect(S_VOICE_CYCLOTRON_FADING_ENABLED);

      playEffect(S_VOICE_CYCLOTRON_FADING_ENABLED);
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
      stopEffect(S_VOICE_CYCLOTRON_36);
      stopEffect(S_VOICE_CYCLOTRON_20);
      stopEffect(S_VOICE_CYCLOTRON_12);

      playEffect(S_VOICE_CYCLOTRON_40);
    break;

    case P_CYCLOTRON_LEDS_36:
      stopEffect(S_VOICE_CYCLOTRON_40);
      stopEffect(S_VOICE_CYCLOTRON_36);
      stopEffect(S_VOICE_CYCLOTRON_20);
      stopEffect(S_VOICE_CYCLOTRON_12);

      playEffect(S_VOICE_CYCLOTRON_36);
    break;

    case P_CYCLOTRON_LEDS_20:
      stopEffect(S_VOICE_CYCLOTRON_40);
      stopEffect(S_VOICE_CYCLOTRON_36);
      stopEffect(S_VOICE_CYCLOTRON_20);
      stopEffect(S_VOICE_CYCLOTRON_12);

      playEffect(S_VOICE_CYCLOTRON_20);
    break;

    case P_CYCLOTRON_LEDS_12:
      stopEffect(S_VOICE_CYCLOTRON_40);
      stopEffect(S_VOICE_CYCLOTRON_36);
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
      stopEffect(S_VOICE_INNER_CYCLOTRON_36);
      stopEffect(S_VOICE_INNER_CYCLOTRON_35);
      stopEffect(S_VOICE_INNER_CYCLOTRON_26);
      stopEffect(S_VOICE_INNER_CYCLOTRON_24);
      stopEffect(S_VOICE_INNER_CYCLOTRON_23);
      stopEffect(S_VOICE_INNER_CYCLOTRON_12);

      playEffect(S_VOICE_INNER_CYCLOTRON_23);
    break;

    case P_INNER_CYCLOTRON_LEDS_24:
      stopEffect(S_VOICE_INNER_CYCLOTRON_36);
      stopEffect(S_VOICE_INNER_CYCLOTRON_35);
      stopEffect(S_VOICE_INNER_CYCLOTRON_26);
      stopEffect(S_VOICE_INNER_CYCLOTRON_24);
      stopEffect(S_VOICE_INNER_CYCLOTRON_23);
      stopEffect(S_VOICE_INNER_CYCLOTRON_12);

      playEffect(S_VOICE_INNER_CYCLOTRON_24);
    break;

    case P_INNER_CYCLOTRON_LEDS_26:
      stopEffect(S_VOICE_INNER_CYCLOTRON_36);
      stopEffect(S_VOICE_INNER_CYCLOTRON_35);
      stopEffect(S_VOICE_INNER_CYCLOTRON_26);
      stopEffect(S_VOICE_INNER_CYCLOTRON_24);
      stopEffect(S_VOICE_INNER_CYCLOTRON_23);
      stopEffect(S_VOICE_INNER_CYCLOTRON_12);

      playEffect(S_VOICE_INNER_CYCLOTRON_26);
    break;

    case P_INNER_CYCLOTRON_LEDS_35:
      stopEffect(S_VOICE_INNER_CYCLOTRON_36);
      stopEffect(S_VOICE_INNER_CYCLOTRON_35);
      stopEffect(S_VOICE_INNER_CYCLOTRON_26);
      stopEffect(S_VOICE_INNER_CYCLOTRON_24);
      stopEffect(S_VOICE_INNER_CYCLOTRON_23);
      stopEffect(S_VOICE_INNER_CYCLOTRON_12);

      playEffect(S_VOICE_INNER_CYCLOTRON_35);
    break;

    case P_INNER_CYCLOTRON_LEDS_36:
      stopEffect(S_VOICE_INNER_CYCLOTRON_36);
      stopEffect(S_VOICE_INNER_CYCLOTRON_35);
      stopEffect(S_VOICE_INNER_CYCLOTRON_26);
      stopEffect(S_VOICE_INNER_CYCLOTRON_24);
      stopEffect(S_VOICE_INNER_CYCLOTRON_23);
      stopEffect(S_VOICE_INNER_CYCLOTRON_12);

      playEffect(S_VOICE_INNER_CYCLOTRON_36);
    break;

    case P_INNER_CYCLOTRON_LEDS_12:
      stopEffect(S_VOICE_INNER_CYCLOTRON_36);
      stopEffect(S_VOICE_INNER_CYCLOTRON_35);
      stopEffect(S_VOICE_INNER_CYCLOTRON_26);
      stopEffect(S_VOICE_INNER_CYCLOTRON_24);
      stopEffect(S_VOICE_INNER_CYCLOTRON_23);
      stopEffect(S_VOICE_INNER_CYCLOTRON_12);

      playEffect(S_VOICE_INNER_CYCLOTRON_12);
    break;

    case P_PACK_GPSTAR_AUDIO_LED_DISABLED:
      stopEffect(S_VOICE_PROTON_PACK_GPSTAR_AUDIO_LED_DISABLED);
      stopEffect(S_VOICE_PROTON_PACK_GPSTAR_AUDIO_LED_ENABLED);
      playEffect(S_VOICE_PROTON_PACK_GPSTAR_AUDIO_LED_DISABLED);
    break;

    case P_PACK_GPSTAR_AUDIO_LED_ENABLED:
      stopEffect(S_VOICE_PROTON_PACK_GPSTAR_AUDIO_LED_DISABLED);
      stopEffect(S_VOICE_PROTON_PACK_GPSTAR_AUDIO_LED_ENABLED);
      playEffect(S_VOICE_PROTON_PACK_GPSTAR_AUDIO_LED_ENABLED);
    break;

    case P_QUICK_BOOTUP_ENABLED:
      stopEffect(S_VOICE_QUICK_BOOTUP_ENABLED);
      stopEffect(S_VOICE_QUICK_BOOTUP_DISABLED);
      playEffect(S_VOICE_QUICK_BOOTUP_ENABLED);
    break;

    case P_QUICK_BOOTUP_DISABLED:
      stopEffect(S_VOICE_QUICK_BOOTUP_ENABLED);
      stopEffect(S_VOICE_QUICK_BOOTUP_DISABLED);
      playEffect(S_VOICE_QUICK_BOOTUP_DISABLED);
    break;

    case P_TURN_WAND_ON:
      if(WAND_STATUS == MODE_OFF && SYSTEM_MODE == MODE_SUPER_HERO) {
        if(switch_activate.on() && WAND_ACTION_STATUS == ACTION_IDLE) {
          // Turn wand and pack on.
          WAND_ACTION_STATUS = ACTION_ACTIVATE;
        }
      }
    break;

    case P_SAVE_EEPROM_WAND:
      // Commit changes to the EEPROM in the wand controller
      saveLEDEEPROM();
      saveConfigEEPROM();
      stopEffect(S_VOICE_EEPROM_SAVE);
      playEffect(S_VOICE_EEPROM_SAVE);
    break;

    default:
      // No-op for anything else.
    break;
  }
}
