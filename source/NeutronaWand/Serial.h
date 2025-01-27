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
  uint8_t rgbVentEnabled;
  uint8_t spectralModesEnabled;
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
  uint8_t numBargraphSegments;
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

struct __attribute__((packed)) WandSyncData {
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
  uint8_t masterMuted;
  uint8_t repeatMusicTrack;
} wandSyncData;

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

        case LEDS_50:
          wandConfig.ledWandCount = 2;
        break;

        case LEDS_2:
          wandConfig.ledWandCount = 3;
        break;
      }

      wandConfig.ledWandHue = i_spectral_wand_custom_colour;
      wandConfig.ledWandSat = i_spectral_wand_custom_saturation;
      wandConfig.spectralModesEnabled = b_spectral_mode_enabled ? 1 : 0;
      wandConfig.overheatEnabled = b_overheat_enabled ? 1 : 0;

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

      wandConfig.wandSoundsToPack = b_extra_pack_sounds ? 1 : 0;
      wandConfig.quickVenting = b_quick_vent ? 1 : 0;
      wandConfig.autoVentLight = b_vent_light_control ? 1 : 0;
      wandConfig.wandBeepLoop = b_beep_loop ? 1 : 0;
      wandConfig.wandBootError = b_wand_boot_errors ? 1 : 0;

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
        case CTS_AFTERLIFE:
          wandConfig.defaultYearModeCTS = 4;
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

      wandConfig.invertWandBargraph = b_bargraph_invert ? 1 : 0;
      wandConfig.bargraphOverheatBlink = b_overheat_bargraph_blink ? 1 : 0;

      switch(BARGRAPH_TYPE_EEPROM) {
        case SEGMENTS_28:
        default:
          wandConfig.numBargraphSegments = 28;
        break;
        case SEGMENTS_30:
          wandConfig.numBargraphSegments = 30;
        break;
      }

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
      smokeConfig.overheatLevel5 = b_overheat_level_5 ? 1 : 0;
      smokeConfig.overheatLevel4 = b_overheat_level_4 ? 1 : 0;
      smokeConfig.overheatLevel3 = b_overheat_level_3 ? 1 : 0;
      smokeConfig.overheatLevel2 = b_overheat_level_2 ? 1 : 0;
      smokeConfig.overheatLevel1 = b_overheat_level_1 ? 1 : 0;

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
              ventLedTopControl(false);
              digitalWriteFast(WAND_STATUS_LED_PIN, LOW);

              // Indicate that a pack is now connected.
              WAND_CONN_STATE = PACK_CONNECTED;
            }
          }
          else if(recvCmd.s == W_COM_START && recvCmd.c == W_SYNC_NOW && recvCmd.d1 == 0 && recvCmd.e == W_COM_END) {
            // We just received our own heartbeat echoed back, so switch to standalone mode.
            WAND_CONN_STATE = NC_BENCHTEST;
            b_gpstar_benchtest = true;
            b_pack_on = true; // Pretend that the pack (not really attached) has been powered on.

            // Turn off the sync indicator LED as it is no longer necessary.
            ventLedTopControl(false);
            digitalWriteFast(WAND_STATUS_LED_PIN, LOW);

            // Reset the audio device now that we are in standalone mode and need music playback.
            setupAudioDevice();

            // Start the music check timer for standalone mode.
            ms_check_music.start(i_music_check_delay);

            // Re-read the EEPROM now that we are in standalone mode to make sure system mode and volume are correct.
            if(b_eeprom) {
              readEEPROM();
            }

            // Sanity check to make sure that a firing mode was set as default.
            if(FIRING_MODE != CTS_MODE && FIRING_MODE != CTS_MIX_MODE) {
              FIRING_MODE = VG_MODE;
              LAST_FIRING_MODE = FIRING_MODE;
            }

            // Check if we should be in video game mode or not.
            vgModeCheck();

            // Reset the bargraph.
            bargraphYearModeUpdate();

            // Stop the pack sync timer since we are no longer syncing to a pack.
            ms_packsync.stop();

            // No pack to do a volume sync with, so reset our master volume manually.
            updateMasterVolume(true);

            // Immediately exit the serial data functions.
            return;
          }
        break;

        case PACKET_DATA:
          wandComs.rxObj(recvData);
          if(recvData.m > 0 && recvData.s == P_COM_START && recvData.e == P_COM_END) {
            debug(F("Recv. Message: "));
            debugln(recvData.m);

            switch(recvData.m) {
              default:
                // Nothing here yet.
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
              i_num_barrel_leds = 5; // Stock count for Haslab equipment.
            break;

            case 1:
              WAND_BARREL_LED_COUNT = LEDS_48;
              i_num_barrel_leds = 48; // Total count is 49, with 1 for the tip.
            break;

            case 2:
              WAND_BARREL_LED_COUNT = LEDS_50;
              i_num_barrel_leds = 48; // Total count is 50, with 2 for the tip.
            break;

            case 3:
              WAND_BARREL_LED_COUNT = LEDS_2;
              i_num_barrel_leds = 2; // Device is tip-only.
            break;
          }

          b_overheat_enabled = (wandConfig.overheatEnabled == 1);
          i_spectral_wand_custom_colour = wandConfig.ledWandHue;
          i_spectral_wand_custom_saturation = wandConfig.ledWandSat;
          b_spectral_mode_enabled = (wandConfig.spectralModesEnabled == 1);
          b_spectral_custom_mode_enabled = b_spectral_mode_enabled;
          b_holiday_mode_enabled = b_spectral_mode_enabled;

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
              VIBRATION_MODE_EEPROM = VIBRATION_ALWAYS;
              VIBRATION_MODE = VIBRATION_MODE_EEPROM;
            break;

            case 2:
              b_vibration_switch_on = true; // Override the Proton Pack vibration toggle switch.
              VIBRATION_MODE_EEPROM = VIBRATION_FIRING_ONLY;
              VIBRATION_MODE = VIBRATION_MODE_EEPROM;
            break;

            case 3:
              VIBRATION_MODE_EEPROM = VIBRATION_NONE;
              VIBRATION_MODE = VIBRATION_MODE_EEPROM;
            break;

            case 4:
            default:
              VIBRATION_MODE_EEPROM = VIBRATION_DEFAULT;
              VIBRATION_MODE = VIBRATION_FIRING_ONLY;
            break;
          }

          b_extra_pack_sounds = (wandConfig.wandSoundsToPack == 1);
          b_quick_vent = (wandConfig.quickVenting == 1);
          b_vent_light_control = (wandConfig.autoVentLight == 1);
          b_beep_loop = (wandConfig.wandBeepLoop == 1);
          b_wand_boot_errors = (wandConfig.wandBootError == 1);

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
            case 4:
              WAND_YEAR_CTS = CTS_AFTERLIFE;
            break;
          }

          b_bargraph_invert = (wandConfig.invertWandBargraph == 1);
          b_overheat_bargraph_blink = (wandConfig.bargraphOverheatBlink == 1);

          switch(wandConfig.numBargraphSegments) {
            case 28:
            default:
              BARGRAPH_TYPE_EEPROM = SEGMENTS_28;
            break;
            case 30:
              BARGRAPH_TYPE_EEPROM = SEGMENTS_30;
            break;
          }

          if(BARGRAPH_TYPE != SEGMENTS_5) {
            // Only change bargraph types if we are not using the stock Hasbro bargraph.
            BARGRAPH_TYPE = BARGRAPH_TYPE_EEPROM;
          }

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
          b_overheat_level_5 = (smokeConfig.overheatLevel5 == 1);
          b_overheat_level_4 = (smokeConfig.overheatLevel4 == 1);
          b_overheat_level_3 = (smokeConfig.overheatLevel3 == 1);
          b_overheat_level_2 = (smokeConfig.overheatLevel2 == 1);
          b_overheat_level_1 = (smokeConfig.overheatLevel1 == 1);

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
          wandComs.rxObj(wandSyncData);
          debugln(F("Recv. Sync Payload"));

          // Write the received data to runtime variables.
          // This will not save to the EEPROM!
          switch(wandSyncData.systemMode) {
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
          changeIonArmSwitchState(wandSyncData.ionArmSwitch == 2);

          // Update the System Year setting.
          switch(wandSyncData.systemYear) {
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
          switch(wandSyncData.packOn) {
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
          i_power_level = wandSyncData.powerLevel;
          i_power_level_prev = i_power_level;

          // Set our firing mode.
          switch(wandSyncData.streamMode) {
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

              if(AUDIO_DEVICE == A_GPSTAR_AUDIO || AUDIO_DEVICE == A_GPSTAR_AUDIO_ADV) {
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
              STREAM_MODE = HOLIDAY_HALLOWEEN;
              setVGMode();
            break;
            case 7:
              STREAM_MODE = HOLIDAY_CHRISTMAS;
              setVGMode();
            break;
            case 8:
              STREAM_MODE = SPECTRAL_CUSTOM;
              setVGMode();
            break;
          }

          // Set up master vibration switch if not configured to override it.
          if(VIBRATION_MODE_EEPROM == VIBRATION_DEFAULT) {
            b_vibration_switch_on = wandSyncData.vibrationEnabled == 2;
          }

          // Update cyclotron lid status and music loop status.
          b_pack_cyclotron_lid_on = wandSyncData.cyclotronLidState == 2;
          b_repeat_track = wandSyncData.repeatMusicTrack == 2;

          // Set the percentage volume.
          i_volume_master_percentage = wandSyncData.masterVolume;
          i_volume_effects_percentage = wandSyncData.effectsVolume;

          // Set the decibel volume.
          i_volume_master = MINIMUM_VOLUME - ((MINIMUM_VOLUME - i_volume_abs_max) * i_volume_master_percentage / 100);
          i_volume_effects = i_volume_abs_min - (i_volume_abs_min * i_volume_effects_percentage / 100);
          i_volume_music = i_volume_abs_min - (i_volume_abs_min * i_volume_music_percentage / 100);

          // Update volume levels.
          i_volume_revert = i_volume_master;
          updateMasterVolume();

          switch(wandSyncData.masterMuted) {
            case 1:
            default:
              // Do nothing; we already have our volumes set correctly.
            break;
            case 2:
              // Remember the current master volume level.
              i_volume_revert = i_volume_master;

              // The pack is telling us to be silent.
              i_volume_master = i_volume_abs_min;
              updateMasterVolume();
            break;
          }
        break;
      }
    }
  }
}

bool handlePackCommand(uint8_t i_command, uint16_t i_value) {
  // This function returns true only when the synchronization process is completed.
  (void)(i_value); // Suppress unused variable warning.

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

      if(i_value == 1) {
        // Pack is currently performing a POST sequence, so set that variable to delay our control loop.
        b_pack_post_finish = false;
      }

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

    case P_POST_FINISH:
      // Pack has completed the Power On Self Test sequence.
      b_pack_post_finish = true;
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

    case P_MASTER_AUDIO_SILENT_MODE:
      // Remember the current master volume level.
      i_volume_revert = i_volume_master;

      // The pack is telling us to be silent.
      i_volume_master = i_volume_abs_min;
      updateMasterVolume();
    break;

    case P_MASTER_AUDIO_NORMAL:
      // The pack is telling us to revert the volume to normal.
      i_volume_master = i_volume_revert;
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

        if(WAND_STATUS == MODE_ON && WAND_ACTION_STATUS != ACTION_OVERHEATING && b_pack_on) {
          soundIdleLoop(true);

          if(getNeutronaWandYearMode() == SYSTEM_AFTERLIFE || getNeutronaWandYearMode() == SYSTEM_FROZEN_EMPIRE) {
            stopEffect(S_WAND_BOOTUP);
            playEffect(S_WAND_BOOTUP);

            if(switch_vent.on() == false) {
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

    case P_VOLUME_INCREASE:
      // Increase overall volume.
      increaseVolume();
    break;

    case P_VOLUME_DECREASE:
      // Decrease overall volume.
      decreaseVolume();
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
