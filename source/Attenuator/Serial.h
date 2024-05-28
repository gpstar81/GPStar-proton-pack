/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Dustin Grau <dustin.grau@gmail.com>
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
 * Pack Communication
 */
#if defined(__XTENSA__)
  // ESP32 - Hardware Serial2 Pins
  #define RXD2 16
  #define TXD2 17
#endif
SerialTransfer packComs;
bool b_sync_start = false; // Denotes whether pack communications have begun (initial: false).

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
  uint8_t ledCycLidCenter;
  uint8_t ledCycLidSimRing;
  uint8_t ledCycCakeCount;
  uint8_t ledCycCakeHue;
  uint8_t ledCycCakeSat;
  uint8_t ledCycCakeGRB;
  uint8_t ledCycCavCount;
  uint8_t ledVGCyclotron;
  uint8_t ledPowercellCount;
  uint8_t ledPowercellHue;
  uint8_t ledPowercellSat;
  uint8_t ledVGPowercell;
} packConfig;

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

// Sends an API to the Proton Pack
void attenuatorSerialSend(uint8_t i_command, uint16_t i_value = 0) {
  uint16_t i_send_size = 0;

  #if defined(__XTENSA__) && defined(DEBUG_SERIAL_COMMS)
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

  #if defined(__XTENSA__) && defined(DEBUG_SERIAL_COMMS)
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
      #if defined(__XTENSA__)
        #if defined(DEBUG_SERIAL_COMMS)
          debug("Saving Pack Preferences");
        #endif

        i_send_size = packComs.txObj(packConfig);
        packComs.sendData(i_send_size, (uint8_t) PACKET_PACK);
      #endif
    break;

    case A_SAVE_PREFERENCES_WAND:
      #if defined(__XTENSA__)
        #if defined(DEBUG_SERIAL_COMMS)
          debug("Saving Wand Preferences");
        #endif

        i_send_size = packComs.txObj(wandConfig);
        packComs.sendData(i_send_size, (uint8_t) PACKET_WAND);
      #endif
    break;

    case A_SAVE_PREFERENCES_SMOKE:
      #if defined(__XTENSA__)
        #if defined(DEBUG_SERIAL_COMMS)
          debug("Saving Smoke Preferences");
        #endif

        i_send_size = packComs.txObj(smokeConfig);
        packComs.sendData(i_send_size, (uint8_t) PACKET_SMOKE);
      #endif
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
    #if defined(__XTENSA__)
      // Advanced debugging message, only enable if absolutely needed!
      // debug("PacketID: " + String(i_packet_id));
    #endif

    if(i_packet_id > 0) {
      // Determine the type of packet which was sent by the serial1 device.
      switch(i_packet_id) {
        case PACKET_COMMAND:
          packComs.rxObj(recvCmd);
          if(recvCmd.c > 0 && recvCmd.s == A_COM_START && recvCmd.e == A_COM_END) {
            #if defined(__XTENSA__) && defined(DEBUG_SERIAL_COMMS)
              debug("Recv. Command: " + String(recvCmd.c));
            #endif
            return handleCommand(recvCmd.c, recvCmd.d1);
          }
          else {
            return false;
          }
        break;

        case PACKET_DATA:
          packComs.rxObj(recvData);
          if(recvData.m > 0 && recvData.s == A_COM_START && recvData.e == A_COM_END) {
            #if defined(__XTENSA__) && defined(DEBUG_SERIAL_COMMS)
              debug("Recv. Message: " + String(recvData.m));
            #endif

            switch(recvData.m) {
              case A_VOLUME_SYNC:
                // Only applies to ESP32 for the web UI.
                #if defined(__XTENSA__)
                  try {
                    i_volume_master_percentage = recvData.d[0];
                    i_volume_effects_percentage = recvData.d[1];
                    i_volume_music_percentage = recvData.d[2];
                  }
                  catch (...) {
                    debug("Error during volume sync");
                  }

                  return true; // Indicates a status change.
                #endif
              break;

              case A_SPECTRAL_CUSTOM_MODE:
                FIRING_MODE = SPECTRAL_CUSTOM;

                // Applies to both Arduino Nano and ESP32.
                if(recvData.d[0] > 0) {
                  i_spectral_custom_colour = recvData.d[0];
                }
                if(recvData.d[1] > 0) {
                  i_spectral_custom_saturation = recvData.d[1];
                }

                return true; // Indicates a status change.
              break;

              case A_SPECTRAL_COLOUR_DATA:
                // Applies to both Arduino Nano and ESP32.
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
          // Only applies to ESP32 for the web UI.
          #if defined(__XTENSA__)
            debug("Pack Preferences Received");
            b_received_prefs_pack = true;
            packComs.rxObj(packConfig);
          #endif
        break;

        case PACKET_WAND:
          // Only applies to ESP32 for the web UI.
          #if defined(__XTENSA__)
            debug("Wand Preferences Received");
            b_received_prefs_wand = true;
            packComs.rxObj(wandConfig);
          #endif
        break;

        case PACKET_SMOKE:
          // Only applies to ESP32 for the web UI.
          #if defined(__XTENSA__)
            debug("Smoke Preferences Received");
            b_received_prefs_smoke = true;
            packComs.rxObj(smokeConfig);
          #endif
        break;
      }
    }
  }

  return false; // Returns false if still here.
}

bool handleCommand(uint8_t i_command, uint16_t i_value) {
  bool b_state_changed = false; // Indicates when a crucial state change occurred.

  switch(i_command) {
    case A_PACK_BOOTUP:
      #if defined(__XTENSA__)
        debug("Pack Bootup");
      #endif

      // No action, yet.
    break;

    case A_SYNC_START:
      #if defined(__XTENSA__)
        debug("Sync Start");
      #endif

      i_speed_multiplier = 1;
      b_sync_start = true;
    break;

    case A_SYNC_END:
      #if defined(__XTENSA__)
        debug("Sync End");
      #endif

      b_wait_for_pack = false;
      b_sync_start = false;
      b_state_changed = true;

      attenuatorSerialSend(A_SYNC_END); // Signal end of sync.
    break;

    case A_WAND_CONNECTED:
      #if defined(__XTENSA__)
        // debug("Wand Connected");
      #endif

      b_wand_present = true;
      b_state_changed = true;
    break;

    case A_WAND_DISCONNECTED:
      #if defined(__XTENSA__)
        // debug("Wand Disconnected");
      #endif

      b_wand_present = false;
      b_state_changed = true;
    break;

    case A_PACK_ON:
      #if defined(__XTENSA__)
        debug("Pack On");
      #endif

      // Pack is on (directly).
      b_pack_on = true;
      b_state_changed = true;

      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_WAND_ON:
      #if defined(__XTENSA__)
        debug("Wand On");
      #endif

      // Pack is on (via wand).
      b_pack_on = true;
      b_wand_on = true;
      b_state_changed = true;

      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_PACK_OFF:
      #if defined(__XTENSA__)
        debug("Pack Off");
      #endif

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
      #if defined(__XTENSA__)
        debug("Wand Off");
      #endif

      // Pack is off (directly or via the wand).
      b_pack_on = false;
      b_wand_on = false;
      b_state_changed = true;

      if(BARGRAPH_STATE != BG_OFF) {
        // If not already off, illuminate fully before ramp down.
        bargraphFull();
      }
      BARGRAPH_PATTERN = BG_RAMP_DOWN;
    break;

    case A_MUSIC_IS_PLAYING:
      #if defined(__XTENSA__)
        debug("Music Playing: " + String(i_value));
      #endif

      b_playing_music = true;

      if(i_value > 0 && i_music_track_current != i_value) {
        // Music track changed.
        i_music_track_current = i_value;
        b_state_changed = true;
      }
    break;

    case A_MUSIC_IS_NOT_PLAYING:
      #if defined(__XTENSA__)
        debug("Music Stopped: " + String(i_value));
      #endif

      b_playing_music = false;

      if(i_value > 0 && i_music_track_current != i_value) {
        // Music track changed.
        i_music_track_current = i_value;
        b_state_changed = true;
      }
    break;

    case A_MUSIC_IS_PAUSED:
      if(!b_music_paused) {
        #if defined(__XTENSA__)
          debug("Music Paused");
        #endif

        b_music_paused = true;
        b_state_changed = true;
      }
    break;

    case A_MUSIC_IS_NOT_PAUSED:
      if(b_music_paused) {
        #if defined(__XTENSA__)
          debug("Music Resumed");
        #endif

        b_music_paused = false;
        b_state_changed = true;
      }
    break;

    case A_MUSIC_TRACK_COUNT_SYNC:
      #if defined(__XTENSA__)
        debug("Music Track Sync: " + String(i_value));
      #endif

      if(i_value > 0) {
        i_music_track_count = i_value;
      }

      #if defined(__XTENSA__)
        debug("Track Count: " + String(i_music_track_count));
      #endif

      if(i_music_track_count > 0) {
        i_music_track_min = i_music_track_offset; // First music track possible (eg. 500)
        i_music_track_max = i_music_track_offset + i_music_track_count - 1; // 500 + N - 1 to be inclusive of the offset value.
      }
    break;

    case A_PACK_CONNECTED:
      // The Proton Pack is connected.
      #if defined(__XTENSA__)
        debug("Pack Connected");
      #endif

      b_state_changed = true;
    break;

    case A_HANDSHAKE:
      #if defined(__XTENSA__)
        // debug("Handshake");
      #endif

      if(b_wait_for_pack && !b_sync_start) {
        b_sync_start = true;
        attenuatorSerialSend(A_SYNC_START);
      }
      else if(!b_sync_start) {
        // The pack is asking us if we are still here. Respond back.
        attenuatorSerialSend(A_HANDSHAKE);
      }
    break;

    case A_MODE_SUPER_HERO:
      if(SYSTEM_MODE != MODE_SUPER_HERO) {
        #if defined(__XTENSA__)
          debug("Super Hero Sequence");
        #endif
        SYSTEM_MODE = MODE_SUPER_HERO;
        b_state_changed = true;
      }
    break;

    case A_MODE_ORIGINAL:
      if(SYSTEM_MODE != MODE_ORIGINAL) {
        #if defined(__XTENSA__)
          debug("Original Sequence");
        #endif
        SYSTEM_MODE = MODE_ORIGINAL;
        b_state_changed = true;
      }
    break;

    case A_MODE_ORIGINAL_RED_SWITCH_ON:
      // The proton pack red switch is on and has power (cyclotron not powered up yet).
      if(RED_SWITCH_MODE != SWITCH_ON) {
        #if defined(__XTENSA__)
          debug("Red Switch On");
        #endif
        RED_SWITCH_MODE = SWITCH_ON;
        b_state_changed = true;
      }
    break;

    case A_MODE_ORIGINAL_RED_SWITCH_OFF:
      // The proton pack red switch is off. This will cause a total system shutdown.
      if(RED_SWITCH_MODE != SWITCH_OFF) {
        #if defined(__XTENSA__)
          debug("Red Switch Off");
        #endif
        RED_SWITCH_MODE = SWITCH_OFF;
        b_state_changed = true;
      }
    break;

    case A_YEAR_1984:
      if(SYSTEM_YEAR != SYSTEM_1984) {
        #if defined(__XTENSA__)
          debug("Mode 1984");
        #endif
        SYSTEM_YEAR = SYSTEM_1984;
        b_state_changed = true;
      }
    break;

    case A_YEAR_1989:
      if(SYSTEM_YEAR != SYSTEM_1989) {
        #if defined(__XTENSA__)
          debug("Mode 1989");
        #endif
        SYSTEM_YEAR = SYSTEM_1989;
        b_state_changed = true;
      }
    break;

    case A_YEAR_AFTERLIFE:
      if(SYSTEM_YEAR != SYSTEM_AFTERLIFE) {
        #if defined(__XTENSA__)
          debug("Mode 2021");
        #endif
        SYSTEM_YEAR = SYSTEM_AFTERLIFE;
        b_state_changed = true;
      }
    break;

    case A_YEAR_FROZEN_EMPIRE:
      if(SYSTEM_YEAR != SYSTEM_FROZEN_EMPIRE) {
        #if defined(__XTENSA__)
          debug("Mode 2024");
        #endif
        SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
        b_state_changed = true;
      }
    break;

    case A_PROTON_MODE:
      #if defined(__XTENSA__)
        debug("Proton");
      #endif
      FIRING_MODE = PROTON;
      b_state_changed = true;
    break;

    case A_SLIME_MODE:
      #if defined(__XTENSA__)
        debug("Slime");
      #endif
      FIRING_MODE = SLIME;
      b_state_changed = true;
    break;

    case A_STASIS_MODE:
      #if defined(__XTENSA__)
        debug("Stasis");
      #endif
      FIRING_MODE = STASIS;
      b_state_changed = true;
    break;

    case A_MESON_MODE:
      #if defined(__XTENSA__)
        debug("Meson");
      #endif
      FIRING_MODE = MESON;
      b_state_changed = true;
    break;

    case A_SPECTRAL_MODE:
      #if defined(__XTENSA__)
        debug("Spectral");
      #endif
      FIRING_MODE = SPECTRAL;
      b_state_changed = true;
    break;

    case A_HOLIDAY_MODE:
      #if defined(__XTENSA__)
        debug("Spectral Holiday");
      #endif
      FIRING_MODE = HOLIDAY;
      b_state_changed = true;
    break;

    case A_SETTINGS_MODE:
      #if defined(__XTENSA__)
        debug("Settings");
      #endif
      FIRING_MODE = SETTINGS;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_1:
      #if defined(__XTENSA__)
        debug("Power Level 1");
      #endif
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_1;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_2:
      #if defined(__XTENSA__)
        debug("Power Level 2");
      #endif
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_2;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_3:
      #if defined(__XTENSA__)
        debug("Power Level 3");
      #endif
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_3;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_4:
      #if defined(__XTENSA__)
        debug("Power Level 4");
      #endif
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_4;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_5:
      #if defined(__XTENSA__)
        debug("Power Level 5");
      #endif
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_5;
      b_state_changed = true;
    break;

    case A_ALARM_ON:
      #if defined(__XTENSA__)
        debug("Alarm On");
      #endif

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
      #if defined(__XTENSA__)
        debug("Alarm Off");
      #endif

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
      #if defined(__XTENSA__)
        debug("Quick Venting");
      #endif

      // Pack is performing quick vent; reset bargraph.
      i_speed_multiplier = 1;
      b_overheating = true;
      b_state_changed = true;

      // Go to the standard power ramp.
      bargraphClear();
      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_VENTING_FINISHED:
      #if defined(__XTENSA__)
        debug("Quick Vent Complete");
      #endif

      // Quick vent process completed.
      b_overheating = false;
      b_state_changed = true;
    break;

    case A_OVERHEATING:
      #if defined(__XTENSA__)
        debug("Overheating");
      #endif

      // Pack is overheating.
      b_overheating = true;
      b_state_changed = true;
      ms_blink_leds.start(i_blink_leds);

      bargraphFull();
      BARGRAPH_PATTERN = BG_RAMP_DOWN;
    break;

    case A_OVERHEATING_FINISHED:
      #if defined(__XTENSA__)
        debug("Vented");
      #endif

      // Venting process completed.
      b_overheating = false;
      b_state_changed = true;
      ms_blink_leds.stop();

      i_speed_multiplier = 1; // Return to normal speed.

      bargraphClear();
      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_FIRING:
      #if defined(__XTENSA__)
        debug("Firing");
      #endif

      b_firing = true; // Implies the wand is powered on.
      b_pack_on = true; // Implies the pack is powered on.
      b_wand_on = true; // Implies the wand is powered on.
      b_state_changed = true;
      ms_blink_leds.start(i_blink_leds / i_speed_multiplier);

      bargraphClear();
      BARGRAPH_PATTERN = BG_OUTER_INNER;
    break;

    case A_FIRING_STOPPED:
      #if defined(__XTENSA__)
        debug("Idle");
      #endif

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
      #if defined(__XTENSA__)
        debug("Cyclotron Lid On...");
      #endif

      b_cyclotron_lid_on = true;
    break;

    case A_CYCLOTRON_LID_OFF:
      #if defined(__XTENSA__)
        debug("Cyclotron Lid Off...");
      #endif

      b_cyclotron_lid_on = false;
    break;

    case A_CYCLOTRON_INCREASE_SPEED:
      #if defined(__XTENSA__)
        debug("Cyclotron Speed Increasing...");
      #endif

      i_speed_multiplier++;
      b_state_changed = true;

      #if defined(__XTENSA__)
        debug(String(i_speed_multiplier));
      #endif
    break;

    case A_CYCLOTRON_NORMAL_SPEED:
      #if defined(__XTENSA__)
        debug("Cyclotron Speed Reset");
      #endif

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
        #if defined(__XTENSA__)
          debug("Wand Barrel Extended");
        #endif

        BARREL_STATE = BARREL_EXTENDED;
        b_state_changed = true;
      }
    break;

    case A_BARREL_RETRACTED:
      if(BARREL_STATE != BARREL_RETRACTED) {
        #if defined(__XTENSA__)
          debug("Wand Barrel Retracted");
        #endif

        BARREL_STATE = BARREL_RETRACTED;
        b_state_changed = true;
      }
    break;

    case A_BATTERY_VOLTAGE_PACK:
      #if defined(__XTENSA__) && defined(DEBUG_SERIAL_COMMS)
        debug("Voltage: " + String(i_value));
      #endif

      // Convert to a value X.NN based on expected 5VDC maximum.
      f_batt_volts = (float) i_value / 100;
      b_state_changed = true;
    break;

    default:
      // No-op for anything else.
    break;
  }

  // Indicates a change which should trigger an update to the websocket.
  return b_state_changed;
}