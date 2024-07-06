#include "Header.h"
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
  PACKET_SMOKE = 5,
  PACKET_SYNC = 6
};

// For command signals (1 byte ID, 2 byte optional data).
struct __attribute__((packed)) CommandPacket {
  uint8_t s;
  uint8_t c;
  uint16_t d1; // Reserved for values over 255 (eg. current music track)
  uint8_t e;
};

struct CommandPacket sendCmd;
struct CommandPacket recvCmd;

// For generic data communication (1 byte ID, 4 byte array).
struct __attribute__((packed)) MessagePacket {
  uint8_t s;
  uint8_t m;
  uint8_t d[3]; // Reserved for multiple, arbitrary byte values.
  uint8_t e;
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

struct __attribute__((packed)) SyncData {
  uint8_t systemMode;
  uint8_t ionArmSwitch;
  uint8_t cyclotronLidState;
  uint8_t systemYear;
  uint8_t packOn;
  uint8_t powerLevel;
  uint8_t streamMode;
  uint8_t vibrationEnabled;
  uint8_t masterVolume;
  uint8_t effectsVolume;
  uint8_t musicVolume;
  uint8_t masterMuted;
  uint16_t currentMusicTrack;
  uint8_t repeatMusicTrack;
} packSync;

/*
 * Serial API Communication Handlers
 */

// Outgoing commands to the pack.
void wandSerialSend(uint8_t i_command, uint16_t i_value) {
  uint16_t i_send_size = 0;

  // Leave when a pack is not intended to be connected.
  if(b_gpstar_benchtest == true) {
    return;
  }

  debug(F("Command to Pack: "));
  debugln(i_command);

  sendCmd.s = W_COM_START;
  sendCmd.c = i_command;
  sendCmd.d1 = i_value;
  sendCmd.e = W_COM_END;

  if(WAND_CONN_STATE == PACK_CONNECTED) {
    // Once connected, each send of data should restart the timer.
    ms_handshake.restart();
  }

  i_send_size = wandComs.txObj(sendCmd);
  wandComs.sendData(i_send_size, (uint8_t) PACKET_COMMAND);
}
// Override function to handle calls with a single parameter.
void wandSerialSend(uint8_t i_command) {
  wandSerialSend(i_command, 0);
}

// Outgoing payloads to the pack.
void wandSerialSendData(uint8_t i_message) {
  uint16_t i_send_size = 0;

  // Leave when a pack is not intended to be connected.
  if(b_gpstar_benchtest == true) {
    return;
  }

  debug(F("Data to Pack: "));
  debugln(i_message);

  sendData.s = W_COM_START;
  sendData.m = i_message;
  sendData.e = W_COM_END;

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
        case LEDS_48:
          wandConfig.ledWandCount = 1;
        break;
      }

      wandConfig.ledWandHue = i_spectral_wand_custom_colour;
      wandConfig.ledWandSat = i_spectral_wand_custom_saturation;
      wandConfig.spectralModeEnabled = b_spectral_mode_enabled;
      wandConfig.spectralHolidayMode = b_holiday_mode_enabled;
      wandConfig.overheatEnabled = b_overheat_enabled;

      switch(FIRING_MODE) {
        case VG_MODE:
        default:
          wandConfig.defaultFiringMode = 1;
        break;
        case CTS_MODE:
          wandConfig.defaultFiringMode = 2;
        break;
        case CTS_MIX_MODE:
          wandConfig.defaultFiringMode = 3;
        break;
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
      smokeConfig.overheatLevel5 = b_overheat_level_5;
      smokeConfig.overheatLevel4 = b_overheat_level_4;
      smokeConfig.overheatLevel3 = b_overheat_level_3;
      smokeConfig.overheatLevel2 = b_overheat_level_2;
      smokeConfig.overheatLevel1 = b_overheat_level_1;

      // Time (seconds) before an overheat event takes place by level.
      smokeConfig.overheatDelay5 = (uint8_t)(i_ms_overheat_initiate_level_5 / 1000);
      smokeConfig.overheatDelay4 = (uint8_t)(i_ms_overheat_initiate_level_4 / 1000);
      smokeConfig.overheatDelay3 = (uint8_t)(i_ms_overheat_initiate_level_3 / 1000);
      smokeConfig.overheatDelay2 = (uint8_t)(i_ms_overheat_initiate_level_2 / 1000);
      smokeConfig.overheatDelay1 = (uint8_t)(i_ms_overheat_initiate_level_1 / 1000);

      i_send_size = wandComs.txObj(smokeConfig);
      wandComs.sendData(i_send_size, (uint8_t) PACKET_SMOKE);
    break;

    default:
      // No-op for all other actions.
    break;
  }
}

// Forward function declaration.
bool handlePackCommand(uint8_t i_command, uint16_t i_value);

// Pack communication to the wand.
void checkPack() {
  // Leave when a pack is not intended to be connected.
  if(b_gpstar_benchtest == true) {
    return;
  }

  if(wandComs.available() > 0) {
    uint8_t i_packet_id = wandComs.currentPacketID();
    // debug(F("PacketID: "));
    // debugln(i_packet_id);

    if(i_packet_id > 0) {
      // Determine the type of packet which was sent by the serial1 device.
      switch(i_packet_id) {
        case PACKET_COMMAND:
          wandComs.rxObj(recvCmd);
          if(recvCmd.c > 0 && recvCmd.s == P_COM_START && recvCmd.e == P_COM_END) {
            debug(F("Recv. Command: "));
            debugln(recvCmd.c);
            if(handlePackCommand(recvCmd.c, recvCmd.d1)) {
              // Begin timer for future keepalive handshakes from the wand.
              ms_handshake.start(i_heartbeat_delay);

              // Turn off the sync indicator LED as the sync is completed.
              digitalWriteFast(led_white, HIGH);

              // Indicate that a pack is now connected.
              WAND_CONN_STATE = PACK_CONNECTED;
            }
          }
        break;

        case PACKET_DATA:
          wandComs.rxObj(recvData);
          if(recvData.m > 0 && recvData.s == P_COM_START && recvData.e == P_COM_END) {
            debug(F("Recv. Message: "));
            debugln(recvData.m);

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
                resetMasterVolume();
                updateEffectsVolume();
              break;
            }
          }
        break;

        case PACKET_WAND:
          wandComs.rxObj(wandConfig);
          debugln(F("Recv. Wand Config"));

          // Writes new preferences back to runtime variables.
          // This action does not save changes to the EEPROM!
          // Entering the EEPROM menu afterwards and saving settings will.
          switch(wandConfig.ledWandCount) {
            case 0:
            default:
              WAND_BARREL_LED_COUNT = LEDS_5;
              i_num_barrel_leds = 5;
            break;
            case 1:
              WAND_BARREL_LED_COUNT = LEDS_48;
              i_num_barrel_leds = 48;
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
            case 1:
            default:
              // Default: Video Game
              FIRING_MODE = VG_MODE;
              setVGMode();
              wandSerialSend(W_VIDEO_GAME_MODE);
            break;

            case 2:
              // Cross the Streams (CTS)
              FIRING_MODE = CTS_MODE;

              // Force into Proton mode.
              STREAM_MODE = PROTON;
              wandSerialSend(W_PROTON_MODE);
              wandSerialSend(W_CROSS_THE_STREAMS);
            break;

            case 3:
              // CTS Mix
              FIRING_MODE = CTS_MIX_MODE;

              // Force into Proton mode.
              STREAM_MODE = PROTON;
              wandSerialSend(W_PROTON_MODE);
              wandSerialSend(W_CROSS_THE_STREAMS_MIX);
            break;
          }

          LAST_FIRING_MODE = FIRING_MODE;

          switch(wandConfig.wandVibration) {
            case 1:
              b_vibration_switch_on = true; // Override the Proton Pack vibration toggle switch.
              b_vibration_firing = false; // Disable the "only vibrate while firing" feature.
              b_vibration_enabled = true; // Enable wand vibration.
              VIBRATION_MODE_EEPROM = VIBRATION_ALWAYS;
            break;

            case 2:
              b_vibration_switch_on = true; // Override the Proton Pack vibration toggle switch.
              b_vibration_firing = true; // Enable the "only vibrate while firing" feature.
              b_vibration_enabled = true; // Enable wand vibration.
              VIBRATION_MODE_EEPROM = VIBRATION_FIRING_ONLY;
            break;

            case 3:
              b_vibration_firing = false; // Disable the "only vibrate while firing" feature.
              b_vibration_enabled = false; // Disable wand vibration.
              VIBRATION_MODE_EEPROM = VIBRATION_NONE;
            break;

            case 4:
            default:
              b_vibration_firing = true; // Enable the "only vibrate while firing" feature.
              b_vibration_enabled = true; // Enable wand vibration.
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
          bargraphYearModeUpdate();
          resetOverheatLevels();
          resetWhiteLEDBlinkRate();
        break;

        case PACKET_SMOKE:
          wandComs.rxObj(smokeConfig);
          debugln(F("Recv. Smoke Config"));

          // Writes new preferences back to runtime variables.
          // This action does not save changes to the EEPROM!
          b_overheat_level_5 = smokeConfig.overheatLevel5;
          b_overheat_level_4 = smokeConfig.overheatLevel4;
          b_overheat_level_3 = smokeConfig.overheatLevel3;
          b_overheat_level_2 = smokeConfig.overheatLevel2;
          b_overheat_level_1 = smokeConfig.overheatLevel1;

          // Values are sent as seconds, must convert to milliseconds.
          i_ms_overheat_initiate_level_5 = smokeConfig.overheatDelay5 * 1000;
          i_ms_overheat_initiate_level_4 = smokeConfig.overheatDelay4 * 1000;
          i_ms_overheat_initiate_level_3 = smokeConfig.overheatDelay3 * 1000;
          i_ms_overheat_initiate_level_2 = smokeConfig.overheatDelay2 * 1000;
          i_ms_overheat_initiate_level_1 = smokeConfig.overheatDelay1 * 1000;

          // Update and reset wand components.
          resetOverheatLevels();
        break;

        case PACKET_SYNC:
          wandComs.rxObj(packSync);
          debugln(F("Recv. Sync Payload"));

          // Write the received data to runtime variables.
          // This will not save to the EEPROM!
          switch(packSync.systemMode) {
            case 1:
            default:
              SYSTEM_MODE = MODE_SUPER_HERO;
            break;
            case 2:
              SYSTEM_MODE = MODE_ORIGINAL;
            break;
          }

          vgModeCheck(); // Re-check VG/CTS mode.

          // Set whether the switch under the ion arm is on or off.
          switch(packSync.ionArmSwitch) {
            case 1:
            default:
              b_pack_ion_arm_switch_on = false;

              // If the ion arm switch is turned off in MODE_ORIGINAL, start the power indicator timer.
              if(SYSTEM_MODE == MODE_ORIGINAL && b_power_on_indicator) {
                ms_power_indicator.start(i_ms_power_indicator);
              }
            break;
            case 2:
              b_pack_ion_arm_switch_on = true;

              // If the ion arm switch is on in MODE_ORIGINAL, we do not need a power indicator.
              if(SYSTEM_MODE == MODE_ORIGINAL && b_power_on_indicator) {
                ms_power_indicator.stop();
                ms_power_indicator_blink.stop();
              }
            break;
          }

          switch(packSync.cyclotronLidState) {
            case 1:
              b_pack_cyclotron_lid_on = false;
            break;

            case 2:
            default:
              b_pack_cyclotron_lid_on = true;
            break;
          }

          // Update the System Year setting.
          switch(packSync.systemYear) {
            case 1:
              SYSTEM_YEAR = SYSTEM_1984;
            break;
            case 2:
              SYSTEM_YEAR = SYSTEM_1989;
            break;
            case 3:
            default:
              SYSTEM_YEAR = SYSTEM_AFTERLIFE;
            break;
            case 4:
              SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
            break;
          }

          // Reset the bargraph now that we have our SYSTEM_MODE and SYSTEM_YEAR set.
          bargraphYearModeUpdate();

          // Reset the white LED blink rate in case we changed wand year.
          resetWhiteLEDBlinkRate();

          // Set whether the Proton Pack is currently on or off.
          switch(packSync.packOn) {
            case 1:
            default:
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

              b_pack_on = false;
            break;
            case 2:
              // Pack is on.
              b_pack_on = true;
            break;
          }

          // Set our starting power level.
          i_power_level = packSync.powerLevel;
          i_power_level_prev = i_power_level;

          // Set our firing mode.
          switch(packSync.streamMode) {
            case 1:
            default:
              STREAM_MODE = PROTON;
            break;
            case 2:
              STREAM_MODE = SLIME;
              setVGMode();
            break;
            case 3:
              STREAM_MODE = STASIS;
              setVGMode();
            break;
            case 4:
              STREAM_MODE = MESON;

              if(AUDIO_DEVICE == A_GPSTAR_AUDIO) {
                // Tell GPStar Audio we need short audio mode.
                audio.gpstarShortTrackOverload(false);
              }

              setVGMode();
            break;
            case 5:
              STREAM_MODE = SPECTRAL;
              setVGMode();
            break;
            case 6:
              STREAM_MODE = HOLIDAY;
              setVGMode();
            break;
            case 7:
              STREAM_MODE = SPECTRAL_CUSTOM;
              setVGMode();
            break;
          }

          // Set up master vibration switch if not configured to override it.
          if(VIBRATION_MODE_EEPROM == VIBRATION_DEFAULT) {
            switch(packSync.vibrationEnabled) {
              case 1:
                b_vibration_switch_on = false;
              break;
              case 2:
              default:
                b_vibration_switch_on = true;
              break;
            }
          }

          // Set the percentage volume.
          i_volume_master_percentage = packSync.masterVolume;
          i_volume_effects_percentage = packSync.effectsVolume;
          i_volume_music_percentage = packSync.musicVolume;

          // Set the decibel volume.
          i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
          i_volume_effects = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_effects_percentage / 100);
          i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

          // Update volume levels.
          i_volume_revert = i_volume_master;
          resetMasterVolume();
          updateEffectsVolume();

          switch(packSync.masterMuted) {
            case 1:
            default:
              // Do nothing; we already have our volumes set correctly.
            break;
            case 2:
              // Remember the current master volume level.
              i_volume_revert = i_volume_master;

              // The pack is telling us to be silent.
              i_volume_master = i_volume_abs_min;
              resetMasterVolume();
            break;
          }

          // Reset current music track.
          if(i_music_count > 0 && packSync.currentMusicTrack >= i_music_track_start) {
            i_current_music_track = packSync.currentMusicTrack;
          }

          switch(packSync.repeatMusicTrack) {
            case 1:
            default:
              b_repeat_track = false;
            break;
            case 2:
              b_repeat_track = true;
            break;
          }
        break;
      }
    }
  }
}

bool handlePackCommand(uint8_t i_command, uint16_t i_value) {
  // This function returns true only when the synchronization process is completed.

  switch(i_command) {
    case P_HANDSHAKE:
      // The pack is asking us if we are still here so respond accordingly.
      if(WAND_CONN_STATE != PACK_CONNECTED) {
        // If still waiting for the pack, trigger an immediate synchronization.
        wandSerialSend(W_SYNC_NOW);
      }
      else {
        // The wand had already synchronized with the pack, so respond with handshake.
        wandSerialSend(W_HANDSHAKE);
      }
    break;

    case P_SYNC_START:
      debugln(F("Pack Sync Start"));

      stopEffect(S_WAND_SYNC);
      playEffect(S_WAND_SYNC);

      // Stop regular sync attempts while communicating with the pack.
      ms_packsync.stop();
    break;

    case P_SYNC_END:
      debugln(F("Pack Sync End"));

      // Acknowledgement that the wand is now synchronized.
      wandSerialSend(W_SYNCHRONIZED);

      // Tell the pack the status of the Neutrona Wand barrel. We only need to tell if its extended.
      // Otherwise the switchBarrel() will tell it if it's retracted during bootup.
      if(switchBarrel() == true) {
        wandSerialSend(W_BARREL_EXTENDED);
      }

      return true;
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
      vgModeCheck(); // Re-check VG/CTS mode.
    break;

    case P_MODE_ORIGINAL:
      SYSTEM_MODE = MODE_ORIGINAL;
      vgModeCheck(); // Assert CTS mode.
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

    case P_TOGGLE_INNER_CYCLOTRON_PANEL_ENABLED:
      stopEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_ENABLED);
      stopEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_DISABLED);
      playEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_ENABLED);
    break;

    case P_TOGGLE_INNER_CYCLOTRON_PANEL_DISABLED:
      stopEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_DISABLED);
      stopEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_ENABLED);
      playEffect(S_VOICE_INNER_CYCLOTRON_LED_PANEL_DISABLED);
    break;

    case P_MODE_ORIGINAL_RED_SWITCH_ON:
      b_pack_ion_arm_switch_on = true;

      // Prep the bargraph for MODE_ORIGINAL. This only preps it when the pack switch is turned on and the wand is still off but all the toggle switches are on for the bargraph to settle at the off position. (0 circle).
      if(WAND_ACTION_STATUS == ACTION_IDLE) {
        switch(WAND_STATUS) {
          case MODE_OFF:
            switch(SYSTEM_MODE) {
              case MODE_ORIGINAL:
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

                // Stop the power on indicator timer if enabled.
                if(b_power_on_indicator) {
                  ms_power_indicator.stop();
                  ms_power_indicator_blink.stop();
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
          if(switch_vent.on() == true && switch_wand.on() == true && b_mode_original_toggle_sounds_enabled == true) {
            if(b_extra_pack_sounds == true) {
              wandSerialSend(W_MODE_ORIGINAL_HEATUP_STOP);
              wandSerialSend(W_MODE_ORIGINAL_HEATDOWN);
            }

            stopEffect(S_WAND_HEATDOWN);
            stopEffect(S_WAND_HEATUP_ALT);
            playEffect(S_WAND_HEATDOWN);
          }

          // Turn off any vibration and all lights.
          vibrationOff();
          wandLightsOff();
        break;

        default:
          // Do nothing.
        break;
      }
    break;

    case P_CYCLOTRON_LID_ON:
      b_pack_cyclotron_lid_on = true;
    break;

    case P_CYCLOTRON_LID_OFF:
      b_pack_cyclotron_lid_on = false;
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

                if(switch_vent.on() == false) {
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
      resetMasterVolume();
    break;

    case P_MASTER_AUDIO_NORMAL:
      // The pack is telling us to revert the volume to normal.
      i_volume_master = i_volume_revert;
      resetMasterVolume();
    break;

    case P_RIBBON_CABLE_ON:
      // Currently unused.
      //b_pack_ribbon_cable_on = true;
    break;

    case P_RIBBON_CABLE_OFF:
      // Currently unused.
      //b_pack_ribbon_cable_on = false;
    break;

    case P_ALARM_ON:
      // Alarm is on.
      b_pack_alarm = true;

      if(WAND_STATUS != MODE_ERROR) {
        if(WAND_STATUS == MODE_ON) {
          digitalWriteFast(led_hat_2, HIGH); // Turn on hat light 2.
          prepBargraphRampDown();

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
          }
        }

        ms_hat_2.start(i_hat_2_delay); // Start the hat light 2 blinking timer.
      }

      if(WAND_STATUS == MODE_ON && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
        switch(getNeutronaWandYearMode()) {
          case SYSTEM_1984:
          case SYSTEM_1989:
            // Do nothing.
          break;

          case SYSTEM_AFTERLIFE:
          case SYSTEM_FROZEN_EMPIRE:
          default:
            if(switch_vent.on() == false) {
              stopAfterLifeSounds();
            }

            playEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1, false, i_volume_effects - 1);
          break;
        }

        if(b_extra_pack_sounds == true) {
          wandSerialSend(W_EXTRA_WAND_SOUNDS_STOP);
          wandSerialSend(W_WAND_SHUTDOWN_SOUND);
        }

        stopEffect(S_WAND_SHUTDOWN);
        playEffect(S_WAND_SHUTDOWN);
      }
    break;

    case P_ALARM_OFF:
      if(WAND_STATUS != MODE_ERROR) {
        digitalWriteFast(led_hat_2, LOW); // Turn off hat light 2.

        ms_hat_2.stop();

        if(WAND_STATUS == MODE_ON) {
          switch(SYSTEM_MODE) {
            case MODE_ORIGINAL:
              if(switch_vent.on() == true && switch_wand.on() == true && switch_activate.on() == true) {
                prepBargraphRampUp();
              }
            break;

            case MODE_SUPER_HERO:
            default:
              prepBargraphRampUp();
            break;
          }
        }
      }

      // Alarm is off.
      b_pack_alarm = false;

      if(WAND_STATUS == MODE_ON && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
        soundIdleLoop(true);

        if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
          stopEffect(S_WAND_BOOTUP);
          playEffect(S_WAND_BOOTUP);

          if(switch_vent.on() == false) {
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
        // Keep hat light 1 on if still firing.
        digitalWriteFast(led_hat_1, HIGH);
      }

      // Revert hat light 2 to its normal non-overheat status.
      if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
        digitalWriteFast(led_hat_2, HIGH);
      }
      else {
        digitalWriteFast(led_hat_2, LOW);
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

    case P_VOLUME_MUSIC_INCREASE:
      // Increase music volume.
      increaseVolumeMusic();
    break;

    case P_VOLUME_MUSIC_DECREASE:
      // Decrease music volume.
      decreaseVolumeMusic();
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

    case P_POWER_LEVEL_1:
      i_power_level = 1;
      i_power_level_prev = 1;
    break;

    case P_POWER_LEVEL_2:
      i_power_level = 2;
      i_power_level_prev = 2;
    break;

    case P_POWER_LEVEL_3:
      i_power_level = 3;
      i_power_level_prev = 3;
    break;

    case P_POWER_LEVEL_4:
      i_power_level = 4;
      i_power_level_prev = 4;
    break;

    case P_POWER_LEVEL_5:
      i_power_level = 5;
      i_power_level_prev = 5;
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

      // The pack is playing music, so we need to make sure the wand knows this, even if the wand does not have any music or a audio device.
      if(b_gpstar_benchtest != true) {
        b_playing_music = true;
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
      saveLEDEEPROM();
      saveConfigEEPROM();
      stopEffect(S_VOICE_EEPROM_SAVE);
      playEffect(S_VOICE_EEPROM_SAVE);
    break;

    default:
      // No-op for anything else.
    break;
  }

  return false; // Default return value.
}
