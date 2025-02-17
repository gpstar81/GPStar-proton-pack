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
 * Pack Communication
 */
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

  sendCmd.s = A_COM_START;
  sendCmd.c = i_command;
  sendCmd.d1 = i_value;
  sendCmd.e = A_COM_END;

  i_send_size = packComs.txObj(sendCmd);
  packComs.sendData(i_send_size, (uint8_t) PACKET_COMMAND);
}

// Forward function declaration.
bool handleCommand(uint8_t i_command, uint16_t i_value);

// Handles an API (and data) sent from the Proton Pack
bool checkPack() {
  // Pack communication to the Attenuator device.
  if(packComs.available() > 0) {
    uint8_t i_packet_id = packComs.currentPacketID();

    if(i_packet_id > 0) {
      if(ms_packsync.isRunning() && !b_wait_for_pack) {
        // If the timer is still running and Pack is connected, consider any request as proof of life.
        ms_packsync.restart();
      }
      else if(!ms_packsync.isRunning()) {
        // If the timer isn't running but we have serial data, switch to full operation.
        ms_packsync.start(0);
      }

      // Determine the type of packet which was sent by the serial1 device.
      switch(i_packet_id) {
        case PACKET_COMMAND:
          packComs.rxObj(recvCmd);
          if(recvCmd.c > 0 && recvCmd.s == P_COM_START && recvCmd.e == P_COM_END) {
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
            switch(recvData.m) {
              case A_SPECTRAL_CUSTOM_MODE:
                STREAM_MODE = SPECTRAL_CUSTOM;

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

        case PACKET_SYNC:
          // Used to sync the pack to the Attenuator.
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

    case A_SYNC_END:
      b_wait_for_pack = false;
      b_state_changed = true;
      ms_packsync.start(i_sync_disconnect_delay);

      attenuatorSerialSend(A_SYNC_END); // Signal end of sync.
    break;

    case A_WAND_CONNECTED:
      b_wand_present = true;
      b_state_changed = true;
    break;

    case A_WAND_DISCONNECTED:
      b_wand_present = false;
      b_state_changed = true;
    break;

    case A_PACK_ON:
      // Pack is on (directly).
      b_pack_on = true;
      b_state_changed = true;

      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_WAND_ON:
      // Pack is on (via wand).
      b_pack_on = true;
      b_wand_on = true;
      b_state_changed = true;

      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_PACK_OFF:
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

    case A_ION_ARM_SWITCH_ON:
      // The proton pack red switch is on and has power (cyclotron not powered up yet).
    break;

    case A_ION_ARM_SWITCH_OFF:
      // The proton pack red switch is off. This will cause a total system shutdown.
    break;

    case A_YEAR_1984:
      if(SYSTEM_YEAR != SYSTEM_1984) {
        SYSTEM_YEAR = SYSTEM_1984;
        b_state_changed = true;
      }
    break;

    case A_YEAR_1989:
      if(SYSTEM_YEAR != SYSTEM_1989) {
        SYSTEM_YEAR = SYSTEM_1989;
        b_state_changed = true;
      }
    break;

    case A_YEAR_AFTERLIFE:
      if(SYSTEM_YEAR != SYSTEM_AFTERLIFE) {
        SYSTEM_YEAR = SYSTEM_AFTERLIFE;
        b_state_changed = true;
      }
    break;

    case A_YEAR_FROZEN_EMPIRE:
      if(SYSTEM_YEAR != SYSTEM_FROZEN_EMPIRE) {
        SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
        b_state_changed = true;
      }
    break;

    case A_PROTON_MODE:
      STREAM_MODE = PROTON;
      b_state_changed = true;
    break;

    case A_STASIS_MODE:
      STREAM_MODE = STASIS;
      b_state_changed = true;
    break;

    case A_SLIME_MODE:
      STREAM_MODE = SLIME;
      b_state_changed = true;
    break;

    case A_MESON_MODE:
      STREAM_MODE = MESON;
      b_state_changed = true;
    break;

    case A_SPECTRAL_MODE:
      STREAM_MODE = SPECTRAL;
      b_state_changed = true;
    break;

    case A_HALLOWEEN_MODE:
      STREAM_MODE = HOLIDAY_HALLOWEEN;
      b_state_changed = true;
    break;

    case A_CHRISTMAS_MODE:
      STREAM_MODE = HOLIDAY_CHRISTMAS;
      b_state_changed = true;
    break;

    case A_SETTINGS_MODE:
      STREAM_MODE = SETTINGS;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_1:
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_1;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_2:
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_2;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_3:
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_3;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_4:
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_4;
      b_state_changed = true;
    break;

    case A_POWER_LEVEL_5:
      POWER_LEVEL_PREV = POWER_LEVEL;
      POWER_LEVEL = LEVEL_5;
      b_state_changed = true;
    break;

    case A_ALARM_ON:
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
      // Pack is performing quick vent; reset bargraph.
      i_speed_multiplier = 1;
      b_overheating = true;
      b_state_changed = true;

      // Go to the standard power ramp.
      bargraphClear();
      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_VENTING_FINISHED:
      // Quick vent process completed.
      b_overheating = false;
      b_state_changed = true;
    break;

    case A_OVERHEATING:
      // Pack is overheating.
      b_overheating = true;
      b_state_changed = true;
      ms_blink_leds.start(i_blink_leds);

      bargraphFull();
      BARGRAPH_PATTERN = BG_RAMP_DOWN;
    break;

    case A_OVERHEATING_FINISHED:
      // Venting process completed.
      b_overheating = false;
      b_state_changed = true;
      ms_blink_leds.stop();

      i_speed_multiplier = 1; // Return to normal speed.

      bargraphClear();
      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_FIRING:
      b_firing = true; // Implies the wand is powered on.
      b_pack_on = true; // Implies the pack is powered on.
      b_wand_on = true; // Implies the wand is powered on.
      b_state_changed = true;
      ms_blink_leds.start(i_blink_leds / i_speed_multiplier);

      bargraphClear();
      BARGRAPH_PATTERN = BG_OUTER_INNER;
    break;

    case A_FIRING_STOPPED:
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

    case A_CYCLOTRON_INCREASE_SPEED:
      i_speed_multiplier++;
      b_state_changed = true;
    break;

    case A_CYCLOTRON_NORMAL_SPEED:
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

    default:
      // No-op for anything else.
    break;
  }

  // Indicates a change which should trigger an update to the websocket.
  return b_state_changed;
}
