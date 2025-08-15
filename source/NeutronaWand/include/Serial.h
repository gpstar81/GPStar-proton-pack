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

/*
 * Proton Pack communication.
 */
#ifdef ESP32
  // ESP32 allows defining custom objects, so create one using UART1.
  #ifndef PACK_RX_PIN
    #define PACK_RX_PIN 21
  #endif
  #ifndef PACK_TX_PIN
    #define PACK_TX_PIN 14
  #endif

  HardwareSerial PackSerial(1);
#else
  // ATMEGA 2560 has hardcoded serial UART objects, so use aliases instead.
  #define PackSerial Serial1
#endif
SerialTransfer packComs;

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
  uint8_t effectsVolume;
  uint8_t masterMuted;
  uint8_t musicStatus;
  uint8_t repeatMusicTrack;
} wandSyncData;

/*
 * Serial API Communication Handlers
 */

 // Helper function to check if a command is excluded from WebSocket notifications.
bool isExcludedCommand(uint8_t i_command) {
  return i_command == W_HANDSHAKE ||
         i_command == W_SYNC_NOW ||
         i_command == W_COM_SOUND_NUMBER ||
         i_command == W_SEND_PREFERENCES_WAND ||
         i_command == W_SEND_PREFERENCES_SMOKE;
}

// Outgoing commands to the pack.
void wandSerialSend(uint8_t i_command, uint16_t i_value) {
  uint16_t i_send_size = 0;

#ifdef ESP32
  // Send latest status to the WebSocket (ESP32 only), skipping this action on certain commands.
  // We make a special case for a disconnected pack, or one in benchtest mode, so that the WebSocket gets updates.
  if (WAND_CONN_STATE == PACK_DISCONNECTED || WAND_CONN_STATE == NC_BENCHTEST || !isExcludedCommand(i_command)) {
    notifyWSClients();
  }
#endif

  // Leave when a pack is not intended to be connected.
  if(b_gpstar_benchtest) {
    return;
  }

  // debug(F("Command to Pack: "));
  // debugln(i_command);

  sendCmd.s = W_COM_START;
  sendCmd.c = i_command;
  sendCmd.d1 = i_value;
  sendCmd.e = W_COM_END;

  if(WAND_CONN_STATE == PACK_CONNECTED) {
    // Once connected, each send of data should restart the timer.
    ms_handshake.restart();
  }

  i_send_size = packComs.txObj(sendCmd);
  packComs.sendData(i_send_size, (uint8_t) PACKET_COMMAND);
}
// Override function to handle calls with a single parameter.
void wandSerialSend(uint8_t i_command) {
  wandSerialSend(i_command, 0);
}

// Outgoing payloads to the pack.
void wandSerialSendData(uint8_t i_message) {
  uint16_t i_send_size = 0;

  // Leave when a pack is not intended to be connected.
  if(b_gpstar_benchtest) {
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
      wandConfig.rgbVentEnabled = b_rgb_vent_light ? 1 : 0;
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

      i_send_size = packComs.txObj(wandConfig);
      packComs.sendData(i_send_size, (uint8_t) PACKET_WAND);
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

      i_send_size = packComs.txObj(smokeConfig);
      packComs.sendData(i_send_size, (uint8_t) PACKET_SMOKE);
    break;

    default:
      // No-op for all other actions.
    break;
  }
}

// Forward function declarations.
bool handlePackCommand(uint8_t i_command, uint16_t i_value);

// Perform update of the pack preferences based on the current configuration object.
void handleWandPrefsUpdate() {
  sendDebug(F("Saving Pack Preferences"));

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
  b_rgb_vent_light = (wandConfig.rgbVentEnabled == 1);
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

  if(BARGRAPH_TYPE == SEGMENTS_28 || BARGRAPH_TYPE == SEGMENTS_30) {
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
}

// Pack communication to the wand.
void checkPack() {
  // Leave when a pack is not intended to be connected.
  if(b_gpstar_benchtest) {
    return;
  }

  if(packComs.available() > 0) {
    uint8_t i_packet_id = packComs.currentPacketID();
    // debug(F("PacketID: "));
    // debugln(i_packet_id);

    if(i_packet_id > 0) {
      // Determine the type of packet which was sent by the Pack.
      switch(i_packet_id) {
        case PACKET_COMMAND:
          packComs.rxObj(recvCmd);
          if(recvCmd.c > 0 && recvCmd.s == P_COM_START && recvCmd.e == P_COM_END) {
            debug(F("Recv. Command: "));
            debugln(recvCmd.c);
            if(handlePackCommand(recvCmd.c, recvCmd.d1)) {
              // Begin timer for future keepalive handshakes from the wand.
              ms_handshake.start(i_heartbeat_delay);

              // Turn off the sync indicator LED as the sync is completed.
              ventTopLightControl(false);
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

            // Reset music status variables just in case they were previously set by a pack.
            b_playing_music = false;
            b_music_paused = false;

            // Turn off the sync indicator LED as it is no longer necessary.
            ventTopLightControl(false);
            digitalWriteFast(WAND_STATUS_LED_PIN, LOW);

            // Reset the audio device now that we are in standalone mode and need music playback.
            setupAudioDevice();

            // Start the music check timer for standalone mode.
            ms_check_music.start(i_music_check_delay);

            // Re-read the EEPROM now that we are in standalone mode to make sure system mode and volume are correct.
            if(b_eeprom) {
              readEEPROM();
            }

            // Reset our master volume according to the new EEPROM values.
            updateMasterVolume(true);

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

            // Immediately exit the serial data functions.
            return;
          }
        break;

        case PACKET_DATA:
          packComs.rxObj(recvData);
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
          packComs.rxObj(wandConfig);
          debugln(F("Recv. Wand Config"));

          // Writes new preferences back to runtime variables.
          // This action does not save changes to the EEPROM!
          handleWandPrefsUpdate();
        break;

        case PACKET_SMOKE:
          packComs.rxObj(smokeConfig);
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
          packComs.rxObj(wandSyncData);
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

          // Update cyclotron lid status.
          b_pack_cyclotron_lid_on = wandSyncData.cyclotronLidState == 2;

          // Update music status.
          b_repeat_track = wandSyncData.repeatMusicTrack == 2;
          switch(wandSyncData.musicStatus) {
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

          // Set the percentage volume.
          i_volume_effects_percentage = wandSyncData.effectsVolume;

          // Set the decibel volume.
          i_volume_effects = i_volume_abs_min - (i_volume_abs_min * i_volume_effects_percentage / 100);
          updateEffectsVolume();

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
      if(switchBarrel()) {
        wandSerialSend(W_BARREL_EXTENDED);
      }

      return true;
    break;

    case P_POST_FINISH:
      // Pack has completed the Power On Self Test sequence.
      b_pack_post_finish = true;
    break;

    case P_SEND_PREFERENCES_WAND:
      // The pack wants the latest wand preferences.
      wandSerialSendData(W_SEND_PREFERENCES_WAND);
    break;

    case P_SEND_PREFERENCES_SMOKE:
      // The pack wants the latest smoke preferences.
      wandSerialSendData(W_SEND_PREFERENCES_SMOKE);
    break;

    default:
      // Fall through to the generic command handler.
      executeCommand(i_command, i_value);
    break;
  }

  return false; // Default return value.
}
