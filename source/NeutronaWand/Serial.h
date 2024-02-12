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

// Types of packets to be sent.
enum PACKET_TYPE : uint8_t {
  PACKET_UNKNOWN = 0,
  PACKET_COMMAND = 1,
  PACKET_DATA = 2,
  PACKET_PACK = 3,
  PACKET_WAND = 4,
  PACKET_SMOKE = 5
};

// For command signals (1 byte ID, 2 byte optional data).
struct __attribute__((packed)) CommandPacket {
  uint8_t c;
  uint16_t d1; // Reserved for values over 255 (eg. current music track)
};

struct CommandPacket sendCmd;
struct CommandPacket recvCmd;

// For generic data communication (1 byte ID, 4 byte array).
struct __attribute__((packed)) MessagePacket {
  uint8_t m;
  uint8_t d[3]; // Reserved for multiple, arbitrary byte values.
};

struct MessagePacket sendData;
struct MessagePacket recvData;

struct __attribute__((packed)) WandPrefs {
  uint8_t ledWandCount;
  uint8_t ledWandHue;
  uint8_t ledWandSat;
  uint8_t spectralModeEnabled;
  uint8_t spectralHolidayMode;
  uint8_t overheatEnabled;
  uint8_t defaultFiringMode;
  uint8_t wandVibration;
  uint8_t wandSoundsToPack;
  uint8_t quickVenting;
  uint8_t autoVentLight;
  uint8_t wandBeepLoop;
  uint8_t wandBootError;
  uint8_t defaultYearModeWand;
  uint8_t defaultYearModeCTS;
  uint8_t invertWandBargraph;
  uint8_t bargraphOverheatBlink;
  uint8_t bargraphIdleAnimation;
  uint8_t bargraphFireAnimation;
} wandConfig;

struct __attribute__((packed)) SmokePrefs {
  // Pack
  uint8_t smokeEnabled;
  uint8_t overheatContinuous5;
  uint8_t overheatContinuous4;
  uint8_t overheatContinuous3;
  uint8_t overheatContinuous2;
  uint8_t overheatContinuous1;
  uint8_t overheatDuration5;
  uint8_t overheatDuration4;
  uint8_t overheatDuration3;
  uint8_t overheatDuration2;
  uint8_t overheatDuration1;
  // Wand
  uint8_t overheatLevel5;
  uint8_t overheatLevel4;
  uint8_t overheatLevel3;
  uint8_t overheatLevel2;
  uint8_t overheatLevel1;
  uint8_t overheatDelay5;
  uint8_t overheatDelay4;
  uint8_t overheatDelay3;
  uint8_t overheatDelay2;
  uint8_t overheatDelay1;
} smokeConfig;

/*
 * Serial API Communication Handlers
 */

// Outgoing commands to the pack.
void wandSerialSend(uint8_t i_command, uint16_t i_value) {
  uint16_t i_send_size = 0;

  // Only sends when pack is present.
  if(b_gpstar_benchtest != true) {
    debugln("Command to Pack: " + String(i_command));

    sendCmd.c = i_command;
    sendCmd.d1 = i_value;

    ms_handshake.restart(); // Restart heartbeat timer.

    i_send_size = wandComs.txObj(sendCmd);
    wandComs.sendData(i_send_size, (uint8_t) PACKET_COMMAND);
  }
}
// Override function to handle calls with a single parameter.
void wandSerialSend(uint8_t i_command) {
  wandSerialSend(i_command, 0);
}

// Outgoing payloads to the pack.
void wandSerialSendData(uint8_t i_message) {
  uint16_t i_send_size = 0;

  // Only sends when pack is present.
  if(b_gpstar_benchtest != true) {
    debugln("Data to Pack: " + String(i_message));

    sendData.m = i_message;

    // Set all elements of the data array to 0
    memset(sendData.d, 0, sizeof(sendData.d));

    switch(i_message) {
      case W_SEND_PREFERENCES_WAND:
        // Boolean types will simply translate as 1/0, ENUMs should be converted.
        switch(WAND_BARREL_LED_COUNT) {
          case LEDS_5:
          default:
            wandConfig.ledWandCount = 0;
          break;
          case LEDS_29:
            wandConfig.ledWandCount = 1;
          break;
          case LEDS_48:
            wandConfig.ledWandCount = 2;
          break;
        }

        wandConfig.ledWandHue = i_spectral_wand_custom_colour;
        wandConfig.ledWandSat = i_spectral_wand_custom_saturation;
        wandConfig.spectralModeEnabled = b_spectral_mode_enabled;
        wandConfig.spectralHolidayMode = b_holiday_mode_enabled;
        wandConfig.overheatEnabled = b_overheat_enabled;

        if(b_cross_the_streams_mix) {
          // More significant, implies b_cross_the_streams.
          wandConfig.defaultFiringMode = 3;
        }
        else if(b_cross_the_streams) {
          // Implies that b_cross_the_streams_mix was false.
          wandConfig.defaultFiringMode = 2;
        }
        else {
          // Use VG modes as default.
          wandConfig.defaultFiringMode = 1;
        }

        wandConfig.wandSoundsToPack = b_extra_pack_sounds;
        wandConfig.quickVenting = b_quick_vent;
        wandConfig.autoVentLight = b_vent_light_control;
        wandConfig.wandBeepLoop = b_beep_loop;
        wandConfig.wandBootError = b_wand_boot_errors;

        switch(WAND_YEAR_MODE) {
          case YEAR_DEFAULT:
          default:
            wandConfig.defaultYearModeWand = 1;
          break;
          case YEAR_1984:
            wandConfig.defaultYearModeWand = 2;
          break;
          case YEAR_1989:
            wandConfig.defaultYearModeWand = 3;
          break;
          case YEAR_AFTERLIFE:
            wandConfig.defaultYearModeWand = 4;
          break;
          case YEAR_FROZEN_EMPIRE:
            wandConfig.defaultYearModeWand = 5;
          break;
        }

        switch(WAND_YEAR_CTS) {
          case CTS_DEFAULT:
          default:
            wandConfig.defaultYearModeCTS = 1;
          break;
          case CTS_1984:
            wandConfig.defaultYearModeCTS = 2;
          break;
          case CTS_1989:
            wandConfig.defaultYearModeCTS = 3;
          break;
          case CTS_AFTERLIFE:
            wandConfig.defaultYearModeCTS = 4;
          break;
          case CTS_FROZEN_EMPIRE:
            wandConfig.defaultYearModeCTS = 5;
          break;
        }

        switch(VIBRATION_MODE_EEPROM) {
          case VIBRATION_ALWAYS:
            wandConfig.wandVibration = 1;
          break;
          case VIBRATION_FIRING_ONLY:
            wandConfig.wandVibration = 2;
          break;
          case VIBRATION_NONE:
            wandConfig.wandVibration = 3;
          break;
          case VIBRATION_DEFAULT:
          default:
            wandConfig.wandVibration = 4;
          break;
        }

        wandConfig.invertWandBargraph = b_bargraph_invert;
        wandConfig.bargraphOverheatBlink = b_overheat_bargraph_blink;

        switch(BARGRAPH_MODE_EEPROM) {
          case BARGRAPH_EEPROM_DEFAULT:
          default:
            wandConfig.bargraphIdleAnimation = 1;
          break;
          case BARGRAPH_EEPROM_SUPER_HERO:
            wandConfig.bargraphIdleAnimation = 2;
          break;
          case BARGRAPH_EEPROM_ORIGINAL:
            wandConfig.bargraphIdleAnimation = 3;
          break;
        }

        switch(BARGRAPH_EEPROM_FIRING_ANIMATION) {
          case BARGRAPH_EEPROM_ANIMATION_DEFAULT:
          default:
            wandConfig.bargraphFireAnimation = 1;
          break;
          case BARGRAPH_EEPROM_ANIMATION_SUPER_HERO:
            wandConfig.bargraphFireAnimation = 2;
          break;
          case BARGRAPH_EEPROM_ANIMATION_ORIGINAL:
            wandConfig.bargraphFireAnimation = 3;
          break;
        }

        i_send_size = wandComs.txObj(wandConfig);
        wandComs.sendData(i_send_size, (uint8_t) PACKET_WAND);
      break;

      case W_SEND_PREFERENCES_SMOKE:
        // Determines whether overheating is enabled for a power level.
        smokeConfig.overheatLevel5 = b_overheat_mode_5;
        smokeConfig.overheatLevel4 = b_overheat_mode_4;
        smokeConfig.overheatLevel3 = b_overheat_mode_3;
        smokeConfig.overheatLevel2 = b_overheat_mode_2;
        smokeConfig.overheatLevel1 = b_overheat_mode_1;

        // Time (seconds) before an overheat event takes place by level.
        smokeConfig.overheatDelay5 = i_ms_overheat_initiate_mode_5 / 1000;
        smokeConfig.overheatDelay4 = i_ms_overheat_initiate_mode_4 / 1000;
        smokeConfig.overheatDelay3 = i_ms_overheat_initiate_mode_3 / 1000;
        smokeConfig.overheatDelay2 = i_ms_overheat_initiate_mode_2 / 1000;
        smokeConfig.overheatDelay1 = i_ms_overheat_initiate_mode_1 / 1000;

        i_send_size = wandComs.txObj(smokeConfig);
        wandComs.sendData(i_send_size, (uint8_t) PACKET_SMOKE);
      break;

      default:
        // No-op for all other actions.
      break;
    }
  }
}

// Forward function declaration.
void handlePackCommand(uint8_t i_command, uint16_t i_value);

// Pack communication to the wand.
void checkPack() {
  // Only checks when pack is present.
  if(b_gpstar_benchtest != true && wandComs.available() > 0) {
    uint8_t i_packet_id = wandComs.currentPacketID();
    // debugln("PacketID: " + String(i_packet_id));

    if(i_packet_id > 0) {
      // Determine the type of packet which was sent by the serial1 device.
      switch(i_packet_id) {
        case PACKET_COMMAND:
          wandComs.rxObj(recvCmd);
          debugln("Recv. Command: " + String(recvCmd.c));
          handlePackCommand(recvCmd.c, recvCmd.d1);
        break;

        case PACKET_DATA:
          wandComs.rxObj(recvData);
          debugln("Recv. Message: " + String(recvData.m));

          switch(recvData.m) {
            case P_VOLUME_SYNC:
              // Set the percentage volume.
              i_volume_master_percentage = recvData.d[0];
              i_volume_effects_percentage = recvData.d[1];
              i_volume_music_percentage = recvData.d[2];

              // Set the decibel volume.
              i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
              i_volume_effects = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_effects_percentage / 100);
              i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

              // Update volume levels.
              i_volume_revert = i_volume_master;
              w_trig.masterGain(i_volume_master);
              adjustVolumeEffectsGain();
            break;
          }
        break;

        case PACKET_WAND:
          wandComs.rxObj(wandConfig);
          debugln("Recv. Wand Config");

          // Writes new preferences back to runtime variables.
          // This action does not save changes to the EEPROM!
          switch(wandConfig.ledWandCount) {
            case 0:
            default:
              i_num_barrel_leds = 5;
              WAND_BARREL_LED_COUNT = LEDS_5;
            break;
            case 1:
              i_num_barrel_leds = 29;
              WAND_BARREL_LED_COUNT = LEDS_29;
            break;
            case 2:
              i_num_barrel_leds = 48;
              WAND_BARREL_LED_COUNT = LEDS_48;
            break;
          }

          b_overheat_enabled = wandConfig.overheatEnabled;
          i_spectral_wand_custom_colour = wandConfig.ledWandHue;
          i_spectral_wand_custom_saturation = wandConfig.ledWandSat;
          b_holiday_mode_enabled = wandConfig.spectralHolidayMode;
          b_spectral_mode_enabled = wandConfig.spectralModeEnabled;

          // Spectral custom, is linked to spectral mode overall, just like in the Neutrona Wand EEPROM menu system.
          b_spectral_custom_mode_enabled = wandConfig.spectralModeEnabled;

          switch(wandConfig.defaultFiringMode) {
            case 3:
              // Default: CTS Mix
              b_cross_the_streams_mix = true;
              b_cross_the_streams = true;
              b_vg_mode = false;

              // Force into Proton mode.
              wandSerialSend(W_PROTON_MODE);
              FIRING_MODE = PROTON;
              PREV_FIRING_MODE = PROTON;
            break;
            case 2:
              // Default: Cross the Streams
              b_cross_the_streams_mix = false;
              b_cross_the_streams = true;
              b_vg_mode = false;

              // Force into Proton mode.
              wandSerialSend(W_PROTON_MODE);
              FIRING_MODE = PROTON;
              PREV_FIRING_MODE = PROTON;
            break;
            default:
              // Default: Video Game
              b_cross_the_streams_mix = false;
              b_cross_the_streams = false;
              b_vg_mode = true;
            break;
          }

          switch(wandConfig.wandVibration) {
            case 1:
              b_vibration_enabled = true;
              b_vibration_on = true;
              b_vibration_firing = false;
              VIBRATION_MODE_EEPROM = VIBRATION_ALWAYS;
            break;
            case 2:
              b_vibration_enabled = true;
              b_vibration_on = true;
              b_vibration_firing = true;
              VIBRATION_MODE_EEPROM = VIBRATION_FIRING_ONLY;
            break;
            case 3:
              b_vibration_enabled = false;
              b_vibration_firing = false;
              b_vibration_on = false;
              VIBRATION_MODE_EEPROM = VIBRATION_NONE;
            break;
            case 4:
            default:
              // Readings are taken from the vibration toggle switch from the Proton pack or configuration setting in stand alone mode.
              VIBRATION_MODE_EEPROM = VIBRATION_DEFAULT;
            break;
          }

          b_extra_pack_sounds = wandConfig.wandSoundsToPack;
          b_quick_vent = wandConfig.quickVenting;
          b_vent_light_control = wandConfig.autoVentLight;
          b_beep_loop = wandConfig.wandBeepLoop;
          b_wand_boot_errors = wandConfig.wandBootError;

          switch(wandConfig.defaultYearModeWand) {
            case 1:
            default:
              WAND_YEAR_MODE = YEAR_DEFAULT;
            break;
            case 2:
              WAND_YEAR_MODE = YEAR_1984;
            break;
            case 3:
              WAND_YEAR_MODE = YEAR_1989;
            break;
            case 4:
              WAND_YEAR_MODE = YEAR_AFTERLIFE;
            break;
            case 5:
              WAND_YEAR_MODE = YEAR_FROZEN_EMPIRE;
            break;
          }

          switch(wandConfig.defaultYearModeCTS) {
            case 1:
            default:
              WAND_YEAR_CTS = CTS_DEFAULT;
            break;
            case 2:
              WAND_YEAR_CTS = CTS_1984;
            break;
            case 3:
              WAND_YEAR_CTS = CTS_1989;
            break;
            case 4:
              WAND_YEAR_CTS = CTS_AFTERLIFE;
            break;
            case 5:
              WAND_YEAR_CTS = CTS_FROZEN_EMPIRE;
            break;
          }

          b_bargraph_invert = wandConfig.invertWandBargraph;
          b_overheat_bargraph_blink = wandConfig.bargraphOverheatBlink;

          switch(wandConfig.bargraphIdleAnimation) {
            case 1:
            default:
              BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_DEFAULT;
            break;
            case 2:
              BARGRAPH_MODE = BARGRAPH_SUPER_HERO;
              BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_SUPER_HERO;
            break;
            case 3:
              BARGRAPH_MODE = BARGRAPH_ORIGINAL;
              BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_ORIGINAL;
            break;
          }

          switch(wandConfig.bargraphFireAnimation) {
            case 1:
            default:
              BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_DEFAULT;
            break;
            case 2:
              BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_SUPER_HERO;
              BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_SUPER_HERO;
            break;
            case 3:
              BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_ORIGINAL;
              BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_ORIGINAL;
            break;
          }

          // Update and reset wand components.
          setBargraphOrientation();
          bargraphYearModeUpdate();
          resetOverHeatModes();
        break;

        case PACKET_SMOKE:
          wandComs.rxObj(smokeConfig);
          debugln("Recv. Smoke Config");

          // Writes new preferences back to runtime variables.
          // This action does not save changes to the EEPROM!
          b_overheat_mode_5 = smokeConfig.overheatLevel5;
          b_overheat_mode_4 = smokeConfig.overheatLevel4;
          b_overheat_mode_3 = smokeConfig.overheatLevel3;
          b_overheat_mode_2 = smokeConfig.overheatLevel2;
          b_overheat_mode_1 = smokeConfig.overheatLevel1;
          i_ms_overheat_initiate_mode_5 = smokeConfig.overheatDelay5;
          i_ms_overheat_initiate_mode_4 = smokeConfig.overheatDelay4;
          i_ms_overheat_initiate_mode_3 = smokeConfig.overheatDelay3;
          i_ms_overheat_initiate_mode_2 = smokeConfig.overheatDelay2;
          i_ms_overheat_initiate_mode_1 = smokeConfig.overheatDelay1;

          // Update and reset wand components.
          resetOverHeatModes();
        break;
      }
    }
  }
}

void handlePackCommand(uint8_t i_command, uint16_t i_value) {
  switch(i_command) {
    case P_HANDSHAKE:
      // The pack is asking us if we are still here so respond accordingly.
      if(b_wait_for_pack) {
        // If still waiting for the pack, trigger an immediate synchronization.
        wandSerialSend(W_SYNC_NOW);
      }
      else {
        // The wand already synchronized with the pack, so respond as such.
        wandSerialSend(W_SYNCHRONIZED);
      }
    break;

    case P_SYNC_START:
      debugln("Pack Sync Start");
      b_synchronizing = true; // Sync process has begun, set a semaphore to avoid another sync attempt.
    break;

    case P_SYNC_END:
      debugln("Pack Sync End");
      b_synchronizing = false; // Sync process has completed so remove the semaphore.
      b_wait_for_pack = false; // Initial handshake is complete, no longer waiting on the pack.

      digitalWrite(led_white, HIGH); // Turn off the sync indicator LED. The wand is now connected.

      switchBarrel(); // Determine the state of the barrel safety switch.

      // Tell the pack the status of the Neutrona Wand barrel. We only need to tell if its extended.
      // Otherwise the switchBarrel() will tell it if it's retracted during bootup.
      if(b_switch_barrel_extended == true) {
        wandSerialSend(W_BARREL_EXTENDED);
      }

      // Acknowledgement that the wand is now synchronized.
      wandSerialSend(W_SYNCHRONIZED);
    break;

    case P_PACK_BOOTUP:
      // Does nothing at the moment.
    break;

    case P_ON:
      // Pack is on.
      b_pack_on = true;
    break;

    case P_OFF:
      // Pack is off.
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

    case P_SEND_PREFERENCES_WAND:
      // The pack wants the latest wand preferences.
      wandSerialSendData(W_SEND_PREFERENCES_WAND);
    break;

    case P_SEND_PREFERENCES_SMOKE:
      // The pack wants the latest smoke preferences.
      wandSerialSendData(W_SEND_PREFERENCES_SMOKE);
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
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_MODE_ORIGINAL_HEATUP_STOP);
              wandSerialSend(W_MODE_ORIGINAL_HEATDOWN);
            }

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

    case P_MANUAL_OVERHEAT:
      if(WAND_STATUS == MODE_ON && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
        if(b_pack_on == true && b_pack_alarm != true && b_overheat_enabled == true) {
          switch(getNeutronaWandYearMode()) {
              case SYSTEM_1984:
              case SYSTEM_1989:
                if(b_extra_pack_sounds == true) {
                  wandSerialSend(W_EXTRA_WAND_SOUNDS_STOP);
                  wandSerialSend(W_WAND_SHUTDOWN_SOUND);
                }
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
                    wandSerialSend(W_WAND_SHUTDOWN_SOUND);
                    wandSerialSend(W_AFTERLIFE_GUN_RAMP_DOWN_1);
                  }
                }
              break;
            }

          startVentSequence();
        }
      }
    break;

    case P_MASTER_AUDIO_SILENT_MODE:
      // Remember the current master volume level.
      i_volume_revert = i_volume_master;

      // The pack is telling us to be silent.
      i_volume_master = i_volume_abs_min;
      w_trig.masterGain(i_volume_master);
    break;

    case P_MASTER_AUDIO_NORMAL:
      // The pack is telling us to revert the volume to normal.
      i_volume_master = i_volume_revert;
      w_trig.masterGain(i_volume_master);
    break;

    case P_RIBBON_CABLE_ON:
      b_pack_ribbon_cable_on = true;
    break;

    case P_RIBBON_CABLE_OFF:
      b_pack_ribbon_cable_on = false;
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

      if(WAND_STATUS == MODE_ON && b_pack_ribbon_cable_on != true && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
        switch(getNeutronaWandYearMode()) {
          case SYSTEM_1984:
          case SYSTEM_1989:
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_EXTRA_WAND_SOUNDS_STOP);
              wandSerialSend(W_WAND_SHUTDOWN_SOUND);
            }
          break;

          case SYSTEM_AFTERLIFE:
          case SYSTEM_FROZEN_EMPIRE:
          default:
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_EXTRA_WAND_SOUNDS_STOP);
              wandSerialSend(W_AFTERLIFE_GUN_RAMP_DOWN_1);

              wandSerialSend(W_WAND_SHUTDOWN_SOUND);
            }

            stopEffect(S_WAND_SHUTDOWN);
            playEffect(S_WAND_SHUTDOWN);

            if(switch_vent.getState() == HIGH) {
              stopAfterLifeSounds();
            }

            playEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1, false, i_volume_effects - 1);
          break;
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

      if(WAND_STATUS == MODE_ON && b_pack_ribbon_cable_on == true && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
        soundIdleLoop(true);

        if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
          stopEffect(S_WAND_BOOTUP);
          playEffect(S_WAND_BOOTUP);

          if(switch_vent.getState() == HIGH) {
            afterLifeRamp1();
          }
        }
      }
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

    case P_MUSIC_REPEAT:
      // Repeat music track.
      b_repeat_track = true;
    break;

    case P_MUSIC_NO_REPEAT:
      // Do not repeat the music track.
      b_repeat_track = false;
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
      stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DEFAULT);

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

      playEffect(S_VOICE_PROTON_PACK_VIBRATION_DEFAULT);
    break;

    case P_YEAR_1984:
      // Indicates system (pack) year is 1984 mode
      SYSTEM_YEAR = SYSTEM_1984;
      bargraphYearModeUpdate();
    break;

    case P_YEAR_1989:
      // Indicates system (pack) year is 1984 mode
      SYSTEM_YEAR = SYSTEM_1989;
      bargraphYearModeUpdate();
    break;

    case P_YEAR_AFTERLIFE:
      // Indicates system (pack) year is Afterlife mode
      SYSTEM_YEAR = SYSTEM_AFTERLIFE;
      bargraphYearModeUpdate();
    break;

    case P_YEAR_FROZEN_EMPIRE:
      // Indicates system (pack) year is Frozen Empire mode
      SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
      bargraphYearModeUpdate();
    break;

    case P_MODE_FROZEN_EMPIRE:
      // Play only the Frozen Empire voice
      stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1989);
      stopEffect(S_VOICE_1984);

      playEffect(S_VOICE_FROZEN_EMPIRE);
    break;

    case P_MODE_AFTERLIFE:
      // Play only the Afterlife voice
      stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1989);
      stopEffect(S_VOICE_1984);

      playEffect(S_VOICE_AFTERLIFE);
    break;

    case P_MODE_1989:
      // Play only the 1989 voice
      stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1989);
      stopEffect(S_VOICE_1984);

      playEffect(S_VOICE_1989);
    break;

    case P_MODE_1984:
      // Play only the 1984 voice
      stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1989);
      stopEffect(S_VOICE_1984);

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

      setVGMode();
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

    case P_SPECTRAL_CUSTOM_MODE:
      FIRING_MODE = SPECTRAL_CUSTOM;
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

    case P_MUSIC_STOP:
      // Stop music for current track.
      stopMusic();
    break;

    case P_MUSIC_START:
      if(b_playing_music == true) {
        // Stop playing current track.
        stopMusic();
      }

      if(i_music_count > 0 && i_value >= i_music_track_start) {
        // Update the music track number to be played.
        i_current_music_track = i_value;
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
      if(i_music_count > 0 && i_value >= i_music_track_start) {
        // Update the music track number to be played.
        i_current_music_track = i_value;
      }
    break;

    case P_SAVE_EEPROM_WAND:
      // Commit changes to the EEPROM in the wand controller
      saveLedEEPROM();
      saveConfigEEPROM();
      stopEffect(S_VOICE_EEPROM_SAVE);
      playEffect(S_VOICE_EEPROM_SAVE);
    break;

    default:
      // No-op for anything else.
    break;
  }
}
