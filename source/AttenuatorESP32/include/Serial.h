/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                         & Dustin Grau <dustin.grau@gmail.com>
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
 * Pack Communication - Hardware Serial2 Pins
 */
#define RXD2 16
#define TXD2 17
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

struct __attribute__((packed)) PackPrefs {
  uint8_t defaultSystemModePack;
  uint8_t defaultYearThemePack;
  uint8_t currentYearThemePack;
  uint8_t defaultSystemVolume;
  uint8_t packVibration;
  uint8_t ribbonCableAlarm;
  uint8_t cyclotronDirection;
  uint8_t demoLightMode;
  uint8_t protonStreamEffects;
  uint8_t overheatStrobeNF;
  uint8_t overheatSyncToFan;
  uint8_t overheatLightsOff;
  uint8_t ledCycLidCount;
  uint8_t ledCycLidHue;
  uint8_t ledCycLidSat;
  uint8_t ledCycLidLum;
  uint8_t ledCycLidCenter;
  uint8_t ledCycLidFade;
  uint8_t ledCycLidSimRing;
  uint8_t ledCycInnerPanel;
  uint8_t ledCycPanLum;
  uint8_t ledCycCakeCount;
  uint8_t ledCycCakeHue;
  uint8_t ledCycCakeSat;
  uint8_t ledCycCakeLum;
  uint8_t ledCycCakeGRB;
  uint8_t ledCycCavCount;
  uint8_t ledCycCavType;
  uint8_t ledVGCyclotron;
  uint8_t ledPowercellCount;
  uint8_t ledInvertPowercell;
  uint8_t ledPowercellHue;
  uint8_t ledPowercellSat;
  uint8_t ledPowercellLum;
  uint8_t ledVGPowercell;
} packConfig;

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

struct __attribute__((packed)) AttenuatorSyncData {
  uint8_t systemMode;
  uint8_t ionArmSwitch;
  uint8_t cyclotronLidState;
  uint8_t systemYear;
  uint8_t packOn;
  uint8_t powerLevel;
  uint8_t streamMode;
  uint8_t wandPresent;
  uint8_t barrelExtended;
  uint8_t wandFiring;
  uint8_t overheatingNow;
  uint8_t speedMultiplier;
  uint8_t spectralColour;
  uint8_t spectralSaturation;
  uint8_t masterMuted;
  uint8_t masterVolume;
  uint8_t effectsVolume;
  uint8_t musicVolume;
  uint8_t musicPlaying;
  uint8_t musicPaused;
  uint8_t trackLooped;
  uint16_t currentTrack;
  uint16_t musicCount;
  uint16_t packVoltage;
} attenuatorSyncData;

/*
 * Serial API Communication Handlers
 */

// Sends an API to the Proton Pack
void attenuatorSerialSend(uint8_t i_command, uint16_t i_value = 0) {
  uint16_t i_send_size = 0;

  #if defined(DEBUG_SERIAL_COMMS)
    // Can only debug communications when using the ESP32.
    debug("Send Command: " + String(i_command));
  #endif

  sendCmd.s = A_COM_START;
  sendCmd.c = i_command;
  sendCmd.d1 = i_value;
  sendCmd.e = A_COM_END;

  i_send_size = packComs.txObj(sendCmd);
  packComs.sendData(i_send_size, (uint8_t) PACKET_COMMAND);
}

// Sends an API to the Proton Pack
void attenuatorSerialSendData(uint8_t i_message) {
  uint16_t i_send_size = 0;

  #if defined(DEBUG_SERIAL_COMMS)
    // Can only debug communications when using the ESP32.
    debug("Send Data: " + String(i_message));
  #endif

  sendData.s = A_COM_START;
  sendData.m = i_message;
  sendData.s = A_COM_END;

  // Set all elements of the data array to 0
  memset(sendData.d, 0, sizeof(sendData.d));

  switch(i_message) {
    case A_SAVE_PREFERENCES_PACK:
      #if defined(DEBUG_SERIAL_COMMS)
        debug("Saving Pack Preferences");
      #endif

      i_send_size = packComs.txObj(packConfig);
      packComs.sendData(i_send_size, (uint8_t) PACKET_PACK);
    break;

    case A_SAVE_PREFERENCES_WAND:
      #if defined(DEBUG_SERIAL_COMMS)
        debug("Saving Wand Preferences");
      #endif

      i_send_size = packComs.txObj(wandConfig);
      packComs.sendData(i_send_size, (uint8_t) PACKET_WAND);
    break;

    case A_SAVE_PREFERENCES_SMOKE:
      #if defined(DEBUG_SERIAL_COMMS)
        debug("Saving Smoke Preferences");
      #endif

      i_send_size = packComs.txObj(smokeConfig);
      packComs.sendData(i_send_size, (uint8_t) PACKET_SMOKE);
    break;

    default:
      // No-op for all other communications.
    break;
  }
}

// Forward function declaration.
bool handleCommand(uint8_t i_command, uint16_t i_value);

// Handles an API (and data) sent from the Proton Pack
bool checkPack() {
  // Pack communication to the Attenuator device.
  if(packComs.available() > 0) {
    uint8_t i_packet_id = packComs.currentPacketID();
    #if defined(DEBUG_SERIAL_COMMS)
      // Advanced debugging message, only enable if absolutely needed!
      // debug("PacketID: " + String(i_packet_id));
    #endif

    if(i_packet_id > 0) {
      if(ms_packsync.isRunning() && !b_wait_for_pack) {
        // If the timer is still running and Pack is connected, consider any request as proof of life.
        ms_packsync.restart();
      }

      // Determine the type of packet which was sent by the serial1 device.
      switch(i_packet_id) {
        case PACKET_COMMAND:
          packComs.rxObj(recvCmd);
          if(recvCmd.c > 0 && recvCmd.s == P_COM_START && recvCmd.e == P_COM_END) {
            #if defined(DEBUG_SERIAL_COMMS)
              debug("Recv. Command: " + String(recvCmd.c));
            #endif
            return handleCommand(recvCmd.c, recvCmd.d1);
          }
          else {
            return false;
          }
        break;

        case PACKET_DATA:
          if(b_wait_for_pack) {
            // Can't proceed if the Pack isn't connected; prevents phantom actions from occurring.
            return false;
          }

          packComs.rxObj(recvData);
          if(recvData.m > 0 && recvData.s == P_COM_START && recvData.e == P_COM_END) {
            #if defined(DEBUG_SERIAL_COMMS)
              debug("Recv. Message: " + String(recvData.m));
            #endif

            switch(recvData.m) {
              case A_VOLUME_SYNC:
                try {
                  i_volume_master_percentage = recvData.d[0];
                  i_volume_effects_percentage = recvData.d[1];
                  i_volume_music_percentage = recvData.d[2];
                }
                catch (...) {
                  debug("Error during volume sync");
                }

                return true; // Indicates a status change.
              break;

              case A_SPECTRAL_CUSTOM_MODE:
                STREAM_MODE = SPECTRAL_CUSTOM;

                if(recvData.d[0] > 0) {
                  i_spectral_custom_colour = recvData.d[0];
                }
                if(recvData.d[1] > 0) {
                  i_spectral_custom_saturation = recvData.d[1];
                }

                return true; // Indicates a status change.
              break;

              case A_SPECTRAL_COLOUR_DATA:
                if(recvData.d[0] > 0) {
                  i_spectral_custom_colour = recvData.d[0];
                }
                if(recvData.d[1] > 0) {
                  i_spectral_custom_saturation = recvData.d[1];
                }
              break;
            }
          }
        break;

        case PACKET_PACK:
          if(b_wait_for_pack) {
            // Can't proceed if the Pack isn't connected; prevents phantom actions from occurring.
            return false;
          }

          // Only applies to ESP32 for the web UI.
          debug("Pack Preferences Received");

          b_received_prefs_pack = true;
          packComs.rxObj(packConfig);
        break;

        case PACKET_WAND:
          if(b_wait_for_pack) {
            // Can't proceed if the Pack isn't connected; prevents phantom actions from occurring.
            return false;
          }

          // Only applies to ESP32 for the web UI.
          debug("Wand Preferences Received");

          b_received_prefs_wand = true;
          packComs.rxObj(wandConfig);
        break;

        case PACKET_SMOKE:
          if(b_wait_for_pack) {
            // Can't proceed if the Pack isn't connected; prevents phantom actions from occurring.
            return false;
          }

          debug("Smoke Preferences Received");

          b_received_prefs_smoke = true;
          packComs.rxObj(smokeConfig);
        break;

        case PACKET_SYNC:
          // Used to sync the Attenuator to the pack.
          debug("Pack Sync Packet Received");

          packComs.rxObj(attenuatorSyncData);

          // Sync all required variables.
          switch(attenuatorSyncData.systemYear) {
            case 1:
              SYSTEM_YEAR = SYSTEM_1984;
            break;
            case 2:
              SYSTEM_YEAR = SYSTEM_1989;
            break;
            case 3:
              SYSTEM_YEAR = SYSTEM_AFTERLIFE;
            default:
            break;
            case 4:
              SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
            break;
          }

          switch(attenuatorSyncData.streamMode) {
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

          POWER_LEVEL_PREV = POWER_LEVEL;
          switch(attenuatorSyncData.powerLevel) {
            case 1:
            default:
              POWER_LEVEL = LEVEL_1;
            break;
            case 2:
              POWER_LEVEL = LEVEL_2;
            break;
            case 3:
              POWER_LEVEL = LEVEL_3;
            break;
            case 4:
              POWER_LEVEL = LEVEL_4;
            break;
            case 5:
              POWER_LEVEL = LEVEL_5;
            break;
          }

          // Common actions to all hardware.
          b_pack_on = attenuatorSyncData.packOn == 1;
          b_firing = attenuatorSyncData.wandFiring == 1;
          b_overheating = attenuatorSyncData.overheatingNow == 1;
          i_speed_multiplier = attenuatorSyncData.speedMultiplier;
          i_spectral_custom_colour = attenuatorSyncData.spectralColour;
          i_spectral_custom_saturation = attenuatorSyncData.spectralSaturation;

          // Specific to the ESP32 and Web UI
          SYSTEM_MODE = attenuatorSyncData.systemMode == 1 ? MODE_SUPER_HERO : MODE_ORIGINAL;
          RED_SWITCH_MODE = attenuatorSyncData.ionArmSwitch == 2 ? SWITCH_ON : SWITCH_OFF;
          BARREL_STATE = attenuatorSyncData.barrelExtended == 1 ? BARREL_EXTENDED : BARREL_RETRACTED;
          b_wand_present = attenuatorSyncData.wandPresent == 1;
          b_cyclotron_lid_on = attenuatorSyncData.cyclotronLidState == 1;
          f_batt_volts = (float) attenuatorSyncData.packVoltage / 100;
          i_volume_master_percentage = attenuatorSyncData.masterVolume;
          i_volume_effects_percentage = attenuatorSyncData.effectsVolume;
          i_volume_music_percentage = attenuatorSyncData.musicVolume;
          i_music_track_current = attenuatorSyncData.currentTrack;
          i_music_track_count = attenuatorSyncData.musicCount;
          b_repeat_track = attenuatorSyncData.trackLooped == 2;
          b_playing_music = attenuatorSyncData.musicPlaying == 1;
          b_music_paused = attenuatorSyncData.musicPaused == 1;
          b_master_muted = attenuatorSyncData.masterMuted == 2;

          if(i_music_track_count > 0) {
            i_music_track_min = i_music_track_offset; // First music track possible (eg. 500)
            i_music_track_max = i_music_track_offset + i_music_track_count - 1; // 500 + N - 1 to be inclusive of the offset value.
          }

          return true; // Indicates a status change.
        break;
      }
    }
  }

  return false; // Returns false if still here.
}

bool handleCommand(uint8_t i_command, uint16_t i_value) {
  bool b_state_changed = false; // Indicates when a crucial state change occurred.

  switch(i_command) {
    case A_HANDSHAKE:
      if(!b_wait_for_pack) {
        // The pack is asking us if we are still here. Respond back.
        attenuatorSerialSend(A_HANDSHAKE);
      }
      else {
        // Who the heck is this pack!? Demand a sync!
        attenuatorSerialSend(A_SYNC_START);
      }
    break;

    case A_SYNC_START:
      debug("Sync Start");
    break;

    case A_SYNC_END:
      debug("Sync End");

      b_wait_for_pack = false;
      b_state_changed = true;
      ms_packsync.start(i_sync_disconnect_delay);

      attenuatorSerialSend(A_SYNC_END); // Signal end of sync.
    break;

    case A_WAND_CONNECTED:
      // debug("Wand Connected");

      b_wand_present = true;
      b_state_changed = true;
    break;

    case A_WAND_DISCONNECTED:
      // debug("Wand Disconnected");

      b_wand_present = false;
      b_state_changed = true;
    break;

    case A_PACK_ON:
      debug("Pack On");

      // Pack is on (directly).
      b_pack_on = true;
      b_state_changed = true;

      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_WAND_ON:
      debug("Wand On");

      // Pack is on (via wand).
      b_wand_on = true;
      b_state_changed = true;

      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_PACK_OFF:
      debug("Pack Off");

      // Pack is off (directly or via the wand).
      b_pack_on = false;
      b_state_changed = true;

      if(BARGRAPH_STATE != BG_OFF) {
        // If not already off, illuminate fully before ramp down.
        bargraphFull();
      }
      BARGRAPH_PATTERN = BG_RAMP_DOWN;
    break;

    case A_WAND_OFF:
      debug("Wand Off");

      // Pack is off (directly or via the wand).
      b_wand_on = false;
      b_state_changed = true;

      if(BARGRAPH_STATE != BG_OFF) {
        // If not already off, illuminate fully before ramp down.
        bargraphFull();
      }
      BARGRAPH_PATTERN = BG_RAMP_DOWN;
    break;

    case A_TOGGLE_MUTE:
      debug("Received mute value: " + String(i_value));
      b_master_muted = i_value == 2;
    break;

    case A_MUSIC_TRACK_LOOP_TOGGLE:
      debug("Received loop value: " + String(i_value));
      b_repeat_track = i_value == 2;
    break;

    case A_MUSIC_IS_PLAYING:
      debug("Music Playing: " + String(i_value));

      b_playing_music = true;
      b_music_paused = false;

      if(i_value > 0 && i_music_track_current != i_value) {
        // Music track changed.
        i_music_track_current = i_value;
        b_state_changed = true;
      }
    break;

    case A_MUSIC_IS_NOT_PLAYING:
      debug("Music Stopped: " + String(i_value));

      b_playing_music = false;
      b_music_paused = false;

      if(i_value > 0 && i_music_track_current != i_value) {
        // Music track changed.
        i_music_track_current = i_value;
        b_state_changed = true;
      }
    break;

    case A_MUSIC_IS_PAUSED:
      if(!b_music_paused) {
        debug("Music Paused");

        b_music_paused = true;
        b_state_changed = true;
      }
    break;

    case A_MUSIC_IS_NOT_PAUSED:
      if(b_music_paused) {
        debug("Music Resumed");

        b_music_paused = false;
        b_state_changed = true;
      }
    break;

    case A_MUSIC_TRACK_COUNT_SYNC:
      debug("Music Track Sync: " + String(i_value));

      if(i_value > 0) {
        i_music_track_count = i_value;
      }

      debug("Track Count: " + String(i_music_track_count));

      if(i_music_track_count > 0) {
        i_music_track_min = i_music_track_offset; // First music track possible (eg. 500)
        i_music_track_max = i_music_track_offset + i_music_track_count - 1; // 500 + N - 1 to be inclusive of the offset value.
      }
    break;

    case A_MODE_SUPER_HERO:
      if(SYSTEM_MODE != MODE_SUPER_HERO) {
        debug("Super Hero Sequence");

        SYSTEM_MODE = MODE_SUPER_HERO;
        b_state_changed = true;
      }
    break;

    case A_MODE_ORIGINAL:
      if(SYSTEM_MODE != MODE_ORIGINAL) {
        debug("Original Sequence");

        SYSTEM_MODE = MODE_ORIGINAL;
        b_state_changed = true;
      }
    break;

    case A_ION_ARM_SWITCH_ON:
      // The proton pack red switch is on and has power (cyclotron not powered up yet).
      if(RED_SWITCH_MODE != SWITCH_ON) {
        debug("Red Switch On");

        RED_SWITCH_MODE = SWITCH_ON;
        b_state_changed = true;
      }
    break;

    case A_ION_ARM_SWITCH_OFF:
      // The proton pack red switch is off. This will cause a total system shutdown.
      if(RED_SWITCH_MODE != SWITCH_OFF) {
        debug("Red Switch Off");

        RED_SWITCH_MODE = SWITCH_OFF;
        b_state_changed = true;
      }
    break;

    case A_YEAR_1984:
      if(SYSTEM_YEAR != SYSTEM_1984) {
        debug("Mode 1984");

        SYSTEM_YEAR = SYSTEM_1984;
        b_state_changed = true;
      }
    break;

    case A_YEAR_1989:
      if(SYSTEM_YEAR != SYSTEM_1989) {
        debug("Mode 1989");

        SYSTEM_YEAR = SYSTEM_1989;
        b_state_changed = true;
      }
    break;

    case A_YEAR_AFTERLIFE:
      if(SYSTEM_YEAR != SYSTEM_AFTERLIFE) {
        debug("Mode 2021");

        SYSTEM_YEAR = SYSTEM_AFTERLIFE;
        b_state_changed = true;
      }
    break;

    case A_YEAR_FROZEN_EMPIRE:
      if(SYSTEM_YEAR != SYSTEM_FROZEN_EMPIRE) {
        debug("Mode 2024");

        SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
        b_state_changed = true;
      }
    break;

    case A_PROTON_MODE:
      debug("Proton");

      STREAM_MODE = PROTON;
      b_state_changed = true;
    break;

    case A_STASIS_MODE:
      debug("Stasis");

      STREAM_MODE = STASIS;
      b_state_changed = true;
    break;

    case A_SLIME_MODE:
      debug("Slime");

      STREAM_MODE = SLIME;
      b_state_changed = true;
    break;

    case A_MESON_MODE:
      debug("Meson");

      STREAM_MODE = MESON;
      b_state_changed = true;
    break;

    case A_SPECTRAL_MODE:
      debug("Spectral");

      STREAM_MODE = SPECTRAL;
      b_state_changed = true;
    break;

    case A_HALLOWEEN_MODE:
      debug("Holiday: Halloween");

      STREAM_MODE = HOLIDAY_HALLOWEEN;
      b_state_changed = true;
    break;

    case A_CHRISTMAS_MODE:
      debug("Holiday: Christmas");

      STREAM_MODE = HOLIDAY_CHRISTMAS;
      b_state_changed = true;
    break;

    case A_SETTINGS_MODE:
      debug("Settings");

      STREAM_MODE = SETTINGS;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_1:
      debug("Power Level 1");

      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_1;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_2:
      debug("Power Level 2");

      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_2;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_3:
      debug("Power Level 3");

      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_3;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_4:
      debug("Power Level 4");

      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_4;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_5:
      debug("Power Level 5");

      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_5;
      b_state_changed = true;
    break;

    case A_ALARM_ON:
      debug("Alarm On");

      // Alarm is on.
      b_firing = false;
      b_pack_alarm = true;
      b_state_changed = true;

      bargraphFull();
      BARGRAPH_PATTERN = BG_RAMP_DOWN;

      if(b_pack_on) {
        ms_blink_leds.start(i_blink_leds);
      }
    break;

    case A_ALARM_OFF:
      debug("Alarm Off");

      // Alarm is off.
      b_pack_alarm = false;
      b_state_changed = true;

      if(b_pack_on) {
        ms_blink_leds.stop();

        bargraphClear();
        BARGRAPH_PATTERN = BG_POWER_RAMP;
      }
    break;

    case A_VENTING:
      debug("Quick Venting");

      // Pack is performing quick vent; reset bargraph.
      i_speed_multiplier = 1;
      b_overheating = true;
      b_state_changed = true;

      // Go to the standard power ramp.
      bargraphClear();
      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_VENTING_FINISHED:
      debug("Quick Vent Complete");

      // Quick vent process completed.
      b_overheating = false;
      b_state_changed = true;
    break;

    case A_OVERHEATING:
      debug("Overheating");

      // Pack is overheating.
      b_overheating = true;
      b_state_changed = true;
      ms_blink_leds.start(i_blink_leds);

      bargraphFull();
      BARGRAPH_PATTERN = BG_RAMP_DOWN;
    break;

    case A_OVERHEATING_FINISHED:
      debug("Vented");

      // Venting process completed.
      b_overheating = false;
      b_state_changed = true;
      ms_blink_leds.stop();

      i_speed_multiplier = 1; // Return to normal speed.

      bargraphClear();
      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_FIRING:
      debug("Firing");

      b_firing = true;
      b_state_changed = true;
      ms_blink_leds.start(i_blink_leds / i_speed_multiplier);

      bargraphClear();
      BARGRAPH_PATTERN = BG_OUTER_INNER;
    break;

    case A_FIRING_STOPPED:
      debug("Idle");

      b_firing = false;
      b_state_changed = true;
      ms_blink_leds.stop();

      if(!b_overheating) {
        i_speed_multiplier = 1; // Return to normal speed.
      }

      if(b_pack_alarm) {
        // Ramp down if the pack alarm happens while firing.
        bargraphFull();
        BARGRAPH_PATTERN = BG_RAMP_DOWN;
      }
      else {
        // We ramp the bargraph back up after finishing firing.
        bargraphClear();
        BARGRAPH_PATTERN = BG_POWER_RAMP;
      }
    break;

    case A_CYCLOTRON_LID_ON:
      debug("Cyclotron Lid On...");

      b_cyclotron_lid_on = true;
    break;

    case A_CYCLOTRON_LID_OFF:
      debug("Cyclotron Lid Off...");

      b_cyclotron_lid_on = false;
    break;

    case A_CYCLOTRON_INCREASE_SPEED:
      debug("Cyclotron Speed Increasing...");

      i_speed_multiplier++;
      b_state_changed = true;

      debug(String(i_speed_multiplier));
    break;

    case A_CYCLOTRON_NORMAL_SPEED:
      debug("Cyclotron Speed Reset");

      i_speed_multiplier = 1;
      b_state_changed = true;

      if(b_firing) {
        // Use the "normal" pattern if still firing.
        bargraphClear();
        BARGRAPH_PATTERN = BG_OUTER_INNER;
      }
      else {
        // Otherwise go to the standard power ramp.
        bargraphClear();
        BARGRAPH_PATTERN = BG_POWER_RAMP;
      }
    break;

    case A_BARREL_EXTENDED:
      if(BARREL_STATE != BARREL_EXTENDED) {
        debug("Wand Barrel Extended");

        BARREL_STATE = BARREL_EXTENDED;
        b_state_changed = true;
      }
    break;

    case A_BARREL_RETRACTED:
      if(BARREL_STATE != BARREL_RETRACTED) {
        debug("Wand Barrel Retracted");

        BARREL_STATE = BARREL_RETRACTED;
        b_state_changed = true;
      }
    break;

    case A_BATTERY_VOLTAGE_PACK:
      #if defined(DEBUG_SERIAL_COMMS)
        // This will be called a lot, so we put it behind the debug option.
        debug("Pack Voltage (x100): " + String(i_value));
      #endif

      // Convert to a value X.NN based on expected 5VDC maximum.
      f_batt_volts = (float) i_value / 100;
      b_state_changed = true;
    break;

    case A_WAND_POWER_AMPS:
      #if defined(DEBUG_SERIAL_COMMS)
        // This will be called a lot, so we put it behind the debug option.
        debug("Wand Current (x100): " + String(i_value));
      #endif

      // Convert to a value X.NN based on expected 1Amp maximum.
      f_wand_amps = (float) i_value / 100;
      b_state_changed = true;
    break;

    default:
      // No-op for anything else.
    break;
  }

  // Indicates a change which should trigger an update to the websocket.
  return b_state_changed;
}
