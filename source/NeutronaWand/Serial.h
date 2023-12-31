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

// For pack communication.
struct __attribute__((packed)) MessagePacket {
  uint16_t s;
  uint16_t i;
  uint16_t d1; // Reserved for values over 255 (eg. current music track)
  uint16_t e;
};

struct MessagePacket comStruct;
struct MessagePacket sendStruct;

// Pack communication from the wand.
void wandSerialSend(uint16_t i_message, uint16_t i_value = 0) {
  if(b_no_pack != true) {
    sendStruct.s = W_COM_START;
    sendStruct.i = i_message;
    sendStruct.d1 = i_value;
    sendStruct.e = W_COM_END;

    wandComs.sendDatum(sendStruct);
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

            case P_PACK_BOOTUP:
              // Nothing for now.
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
                      break;

                      default:
                        // Do nothing.
                      break;
                    }
                  break;

                  default:
                  // Do nothing if we aren't MODE_OFF
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

                  // Start the power on indicator timer if enabled.
                  if(b_power_on_indicator == true) {
                    ms_power_indicator.start(i_ms_power_indicator);
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

            case P_YEAR_FROZEN_EMPIRE:
              // Play Frozen Empire voice.
              stopEffect(S_VOICE_FROZEN_EMPIRE);
              stopEffect(S_VOICE_AFTERLIFE);
              stopEffect(S_VOICE_1989);
              stopEffect(S_VOICE_1984);

              playEffect(S_VOICE_FROZEN_EMPIRE);

              SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
              bargraphYearModeUpdate();
            break;

            case P_YEAR_AFTERLIFE:
              // Play Afterlife voice.
              stopEffect(S_VOICE_FROZEN_EMPIRE);
              stopEffect(S_VOICE_AFTERLIFE);
              stopEffect(S_VOICE_1989);
              stopEffect(S_VOICE_1984);

              playEffect(S_VOICE_AFTERLIFE);

              SYSTEM_YEAR = SYSTEM_AFTERLIFE;
              bargraphYearModeUpdate();
            break;

            case P_YEAR_1989:
              // Play 1989 voice.
              stopEffect(S_VOICE_FROZEN_EMPIRE);
              stopEffect(S_VOICE_AFTERLIFE);
              stopEffect(S_VOICE_1989);
              stopEffect(S_VOICE_1984);

              playEffect(S_VOICE_1989);

              SYSTEM_YEAR = SYSTEM_1989;
              bargraphYearModeUpdate();
            break;

            case P_YEAR_1984:
              // Play 1984 voice.
              stopEffect(S_VOICE_FROZEN_EMPIRE);
              stopEffect(S_VOICE_AFTERLIFE);
              stopEffect(S_VOICE_1989);
              stopEffect(S_VOICE_1984);

              playEffect(S_VOICE_1984);

              SYSTEM_YEAR = SYSTEM_1984;
              bargraphYearModeUpdate();
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

            case P_MUSIC_RESUME:
              resumeMusic();
            break;

            case P_MUSIC_PLAY_TRACK:
              // Music track number to be played.
              i_current_music_track = comStruct.d1;
            break;

            default:
              // No-op for anything else.
            break;
          }
        }

        comStruct.i = 0;
        comStruct.s = 0;
      }
    }
  }
}