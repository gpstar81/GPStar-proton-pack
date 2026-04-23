/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2026 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
 * Proton Pack Communication.
 */
#define RXD2 16
#define TXD2 17
HardwareSerial PackSerial(2); // Associate PackSerial with UART2
SerialTransfer packComs;

// Forward declarations
void sendDebug(const String& message); // from main.cpp

// Declare external reference to WirelessManager pointer (allocated in main.cpp after NVS init)
extern WirelessManager* wirelessMgr;

// Command and Message Data Packets
struct CommandPacket sendCmd;
struct CommandPacket recvCmd;
struct MessagePacket sendData;
struct MessagePacket recvData;

/*
 * Serial API Communication Handlers
 */

// Sends an API to the Proton Pack
void attenuatorSerialSend(uint8_t i_command, uint16_t i_value = 0) {
  uint16_t i_send_size = 0;

  #if defined(DEBUG_SERIAL_COMMS)
    // Can only debug communications when using the ESP32.
    sendDebug(String(F("Send Command: ")) + String(i_command));
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
    sendDebug(String(F("Send Data: ")) + String(i_message));
  #endif

  sendData.s = A_COM_START;
  sendData.m = i_message;
  sendData.e = A_COM_END;

  // Set all elements of the data array to 0
  memset(sendData.d, 0, sizeof(sendData.d));

  switch(i_message) {
    case A_SAVE_PREFERENCES_PACK:
      #if defined(DEBUG_SERIAL_COMMS)
        sendDebug(F("Saving Pack Preferences"));
      #endif

      i_send_size = packComs.txObj(packConfig);
      packComs.sendData(i_send_size, (uint8_t) PACKET_PACK);
    break;

    case A_SAVE_PREFERENCES_WAND:
      #if defined(DEBUG_SERIAL_COMMS)
        sendDebug(F("Saving Wand Preferences"));
      #endif

      i_send_size = packComs.txObj(wandConfig);
      packComs.sendData(i_send_size, (uint8_t) PACKET_WAND);
    break;

    case A_SAVE_PREFERENCES_SMOKE:
      #if defined(DEBUG_SERIAL_COMMS)
        sendDebug(F("Saving Smoke Preferences"));
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
  // Attenuator communication from the Proton Pack.
  if(packComs.available() > 0) {
    uint8_t i_packet_id = packComs.currentPacketID();
    #if defined(DEBUG_SERIAL_COMMS)
      // Advanced debugging message, only enable if absolutely needed!
      // sendDebug(String(F("PacketID: ")) + String(i_packet_id));
    #endif

    if(i_packet_id > 0) {
      if(ms_packsync.isRunning() && !b_wait_for_pack) {
        // If the timer is still running and Pack is connected, consider any request as proof of life.
        ms_packsync.restart();
      }

      // Determine the type of packet which was sent by the Proton Pack.
      switch(i_packet_id) {
        case PACKET_COMMAND:
          packComs.rxObj(recvCmd);
          if(recvCmd.c > 0 && recvCmd.s == P_COM_START && recvCmd.e == P_COM_END) {
            #if defined(DEBUG_SERIAL_COMMS)
              sendDebug(String(F("Recv. Command: ")) + String(recvCmd.c));
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
              sendDebug(String(F("Recv. Message: ")) + String(recvData.m));
            #endif

            switch(recvData.m) {
              case A_VOLUME_SYNC:
                try {
                  i_volume_master_percentage = recvData.d[0];
                  i_volume_effects_percentage = recvData.d[1];
                  i_volume_music_percentage = recvData.d[2];
                }
                catch (...) {
                  sendDebug(F("Error during volume sync"));
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
          #if defined(DEBUG_SERIAL_COMMS)
            sendDebug(F("Pack Preferences Received"));
          #endif

          b_received_prefs_pack = true;
          packComs.rxObj(packConfig);
        break;

        case PACKET_WAND:
          if(b_wait_for_pack) {
            // Can't proceed if the Pack isn't connected; prevents phantom actions from occurring.
            return false;
          }

          // Only applies to ESP32 for the web UI.
          #if defined(DEBUG_SERIAL_COMMS)
            sendDebug(F("Wand Preferences Received"));
          #endif

          b_received_prefs_wand = true;
          packComs.rxObj(wandConfig);
        break;

        case PACKET_SMOKE:
          if(b_wait_for_pack) {
            // Can't proceed if the Pack isn't connected; prevents phantom actions from occurring.
            return false;
          }

          // Only applies to ESP32 for the web UI.
          #if defined(DEBUG_SERIAL_COMMS)
            sendDebug(F("Smoke Preferences Received"));
          #endif

          b_received_prefs_smoke = true;
          packComs.rxObj(smokeConfig);
        break;

        case PACKET_SYNC:
          // Used to sync the Attenuator to the pack.
          #if defined(DEBUG_SERIAL_COMMS)
            sendDebug(F("Pack Sync Packet Received"));
          #endif

          // Check if the received packet size matches the expected struct size
          uint16_t expectedSize = sizeof(attenuatorSyncData);
          uint16_t receivedSize = packComs.bytesRead;
          bool fullPacketReceived = (receivedSize == expectedSize);

          packComs.rxObj(attenuatorSyncData);

          // Import sync data into DeviceState using centralized method
          gpstarSystem.importData(attenuatorSyncData);

          // Set non-DeviceState variables (Attenuator-specific state)
          b_pack_on = attenuatorSyncData.packOn;
          b_wand_firing = attenuatorSyncData.wandFiring;
          b_overheating = attenuatorSyncData.overheatingNow;
          i_cyclotron_multiplier = attenuatorSyncData.speedMultiplier;
          i_spectral_custom_colour = attenuatorSyncData.spectralColour;
          i_spectral_custom_saturation = attenuatorSyncData.spectralSaturation;

          // Specific to the ESP32 and Web UI
          b_wand_connected = attenuatorSyncData.wandPresent;
          b_cyclotron_lid_on = attenuatorSyncData.cyclotronLidState;
          b_clockwise = attenuatorSyncData.cyclotronClockwise;
          b_smoke_enabled = attenuatorSyncData.smokeOn;
          b_vibration_switch_on = attenuatorSyncData.vibrationOn;
          f_batt_volts = (attenuatorSyncData.packVoltage > 0) ? ((float)attenuatorSyncData.packVoltage / 100.0) : 0.0;
          i_pack_audio_version = attenuatorSyncData.packAudioVersion;
          i_wand_audio_version = attenuatorSyncData.wandAudioVersion;
          i_volume_master_percentage = attenuatorSyncData.masterVolume;
          i_volume_effects_percentage = attenuatorSyncData.effectsVolume;
          i_volume_music_percentage = attenuatorSyncData.musicVolume;
          i_current_music_track = attenuatorSyncData.currentTrack;
          i_music_track_count = attenuatorSyncData.musicCount;
          b_repeat_track = attenuatorSyncData.trackLooped;
          b_shuffle_tracks = attenuatorSyncData.shuffleTracks;
          b_playing_music = attenuatorSyncData.musicPlaying;
          b_music_paused = attenuatorSyncData.musicPaused;
          b_master_muted = attenuatorSyncData.masterMuted;

          // Only trust audioCorrupt/audioOutdated flags if the full packet was received
          // Older firmware sends a smaller struct, so these fields would contain garbage
          if (fullPacketReceived) {
            b_microsd_corrupt = attenuatorSyncData.audioCorrupt;
            b_microsd_outdated = attenuatorSyncData.audioOutdated;
          } else {
            // Ignore these flags from older firmware to prevent false positives
            b_microsd_corrupt = false;
            b_microsd_outdated = false;
          }

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
      sendDebug(F("Sync Start"));

      // Indicates whether we are talking to a GPStar Pack II.
      b_esp32_pack = (i_value == 1);
    break;

    case A_SYNC_END:
      sendDebug(F("Sync End"));

      b_wait_for_pack = false;
      b_state_changed = true;
      ms_packsync.start(i_sync_disconnect_delay);

      attenuatorSerialSend(A_SYNC_END); // Signal end of sync.
    break;

    case A_RESET_WIFI_PASSWORD:
      // Pack told us to reset our Wifi password, so do that.
      wirelessMgr->resetWifiPassword();
    break;

    case A_WAND_CONNECTED:
      sendDebug(F("Wand Connected"));

      #if defined(DEBUG_SERIAL_COMMS)
      if (!b_wand_connected) {
        sendDebug(F("Wand Connected")); // Indicate recently (re-)connected.
      }
      #endif

      b_wand_connected = true;
      b_state_changed = true;

      if (!b_received_prefs_wand) {
        attenuatorSerialSend(A_REQUEST_PREFERENCES_WAND); // Request current wand prefs.
      }
    break;

    case A_WAND_DISCONNECTED:
      #if defined(DEBUG_SERIAL_COMMS)
      if (b_wand_connected) {
        sendDebug(F("Wand Disconnected")); // Indicate recently disconnected.
      }
      #endif

      b_wand_connected = false;
      b_state_changed = true;

      b_received_prefs_wand = false; // Clear flag to force request on reconnect.
    break;

    case A_PACK_ON:
      sendDebug(F("Pack On"));

      // Pack is on (directly).
      b_pack_on = true;
      b_pack_shutting_down = false;
      b_state_changed = true;

      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_WAND_ON:
      sendDebug(F("Wand On"));

      // Pack is on (via wand).
      b_wand_on = true;
      b_state_changed = true;

      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_PACK_OFF:
      sendDebug(F("Pack Off"));

      // Pack is off (directly or via the wand).
      b_pack_on = false;
      b_pack_shutting_down = (i_value == 1);
      b_state_changed = true;

      if(BARGRAPH_STATE != BG_OFF) {
        // If not already off, illuminate fully before ramp down.
        bargraphFull();
      }
      BARGRAPH_PATTERN = BG_RAMP_DOWN;
    break;

    case A_WAND_OFF:
      sendDebug(F("Wand Off"));

      // Pack is off (directly or via the wand).
      b_wand_on = false;
      b_state_changed = true;

      if(BARGRAPH_STATE != BG_OFF) {
        // If not already off, illuminate fully before ramp down.
        bargraphFull();
      }
      BARGRAPH_PATTERN = BG_RAMP_DOWN;
    break;

    case A_TOGGLE_SMOKE:
      sendDebug(String(F("Received smoke value: ")) + String(i_value));
      b_smoke_enabled = i_value == 2;
    break;

    case A_TOGGLE_VIBRATION:
      sendDebug(String(F("Received vibration value: ")) + String(i_value));
      b_vibration_switch_on = i_value == 2;
    break;

    case A_CYCLOTRON_DIRECTION_TOGGLE:
      sendDebug(String(F("Received cyclotron direction value: ")) + String(i_value));
      b_clockwise = i_value == 2;
    break;

    case A_TOGGLE_MUTE:
      sendDebug(String(F("Received mute value: ")) + String(i_value));
      b_master_muted = i_value == 2;
    break;

    case A_MUSIC_TRACK_LOOP_TOGGLE:
      sendDebug(String(F("Received loop value: ")) + String(i_value));
      b_repeat_track = i_value == 2;
    break;

    case A_MUSIC_TRACK_SHUFFLE_TOGGLE:
      sendDebug(String(F("Received shuffle value: ")) + String(i_value));
      b_shuffle_tracks = i_value == 2;
    break;

    case A_MUSIC_IS_PLAYING:
      sendDebug(String(F("Music Playing: ")) + String(i_value));

      b_playing_music = true;
      b_music_paused = false;

      if(i_value > 0 && i_current_music_track != i_value) {
        // Music track changed.
        i_current_music_track = i_value;
        b_state_changed = true;
      }
    break;

    case A_MUSIC_IS_NOT_PLAYING:
      sendDebug(String(F("Music Stopped: ")) + String(i_value));

      b_playing_music = false;
      b_music_paused = false;

      if(i_value > 0 && i_current_music_track != i_value) {
        // Music track changed.
        i_current_music_track = i_value;
        b_state_changed = true;
      }
    break;

    case A_MUSIC_IS_PAUSED:
      if(!b_music_paused) {
        sendDebug(F("Music Paused"));

        b_music_paused = true;
        b_state_changed = true;
      }
    break;

    case A_MUSIC_IS_NOT_PAUSED:
      if(b_music_paused) {
        sendDebug(F("Music Resumed"));

        b_music_paused = false;
        b_state_changed = true;
      }
    break;

    case A_MUSIC_TRACK_COUNT_SYNC:
      sendDebug(String(F("Music Track Sync: ")) + String(i_value));

      if(i_value > 0) {
        i_music_track_count = i_value;
      }

      sendDebug(String(F("Track Count: ")) + String(i_music_track_count));

      if(i_music_track_count > 0) {
        i_music_track_min = i_music_track_offset; // First music track possible (eg. 500)
        i_music_track_max = i_music_track_offset + i_music_track_count - 1; // 500 + N - 1 to be inclusive of the offset value.
      }
    break;

    case A_MODE_SUPER_HERO:
      if(gpstarSystem.getSystemMode() != MODE_SUPER_HERO) {
        sendDebug(F("Super Hero Sequence"));
        b_state_changed = gpstarSystem.setSystemMode(MODE_SUPER_HERO);
      }
    break;

    case A_MODE_ORIGINAL:
      if(gpstarSystem.getSystemMode() != MODE_ORIGINAL) {
        sendDebug(F("Original Sequence"));
        b_state_changed = gpstarSystem.setSystemMode(MODE_ORIGINAL);
      }
    break;

    case A_ION_ARM_SWITCH_ON:
      // The proton pack red switch is on and has power (cyclotron not powered up yet).
      if(gpstarSystem.getIonArmSwitch() != RED_SWITCH_ON) {
        sendDebug(F("Red Switch On"));

        // Performs toggle only when not already on.
        b_state_changed = gpstarSystem.setIonArmSwitch(RED_SWITCH_ON);
      }
    break;

    case A_ION_ARM_SWITCH_OFF:
      // The proton pack red switch is off. This will cause a total system shutdown.
      if(gpstarSystem.getIonArmSwitch() != RED_SWITCH_OFF) {
        sendDebug(F("Red Switch Off"));

        // Performs toggle only when not already off.
        b_state_changed = gpstarSystem.setIonArmSwitch(RED_SWITCH_OFF);
      }
    break;

    case A_YEAR_1984:
      if(gpstarSystem.getSystemTheme() != SYSTEM_1984) {
        sendDebug(F("Mode 1984"));

        gpstarSystem.setSystemTheme(SYSTEM_1984);
        b_state_changed = true;
      }
    break;

    case A_YEAR_1989:
      if(gpstarSystem.getSystemTheme() != SYSTEM_1989) {
        sendDebug(F("Mode 1989"));

        gpstarSystem.setSystemTheme(SYSTEM_1989);
        b_state_changed = true;
      }
    break;

    case A_YEAR_AFTERLIFE:
      if(gpstarSystem.getSystemTheme() != SYSTEM_AFTERLIFE) {
        sendDebug(F("Mode 2021"));

        gpstarSystem.setSystemTheme(SYSTEM_AFTERLIFE);
        b_state_changed = true;
      }
    break;

    case A_YEAR_FROZEN_EMPIRE:
      if(gpstarSystem.getSystemTheme() != SYSTEM_FROZEN_EMPIRE) {
        sendDebug(F("Mode 2024"));

        gpstarSystem.setSystemTheme(SYSTEM_FROZEN_EMPIRE);
        b_state_changed = true;
      }
    break;

    case A_SET_STREAM_MODE:
      sendDebug(String(F("Pack changed to ")) + String(i_value));

      gpstarSystem.setStreamMode((STREAM_MODES)i_value);
      b_state_changed = true;
    break;

    case A_SET_FIRING_MODE:
      // Set the variable for firing mode (for web UI).
      switch(i_value) {
        case FLAG_VG_MODE:
        default:
          gpstarSystem.setFiringModeVG();
        break;

        case FLAG_CTS_MODE:
          gpstarSystem.setFiringModeCTS();
        break;

        case FLAG_CTS_MIX_MODE:
          gpstarSystem.setFiringModeCTSMix();
        break;
      }
    break;

    case A_POWER_LEVEL_1:
      sendDebug(F("Power Level 1"));
      b_state_changed = gpstarSystem.setPowerLevel(LEVEL_1);
    break;

    case A_POWER_LEVEL_2:
      sendDebug(F("Power Level 2"));
      b_state_changed = gpstarSystem.setPowerLevel(LEVEL_2);
    break;

    case A_POWER_LEVEL_3:
      sendDebug(F("Power Level 3"));
      b_state_changed = gpstarSystem.setPowerLevel(LEVEL_3);
    break;

    case A_POWER_LEVEL_4:
      sendDebug(F("Power Level 4"));
      b_state_changed = gpstarSystem.setPowerLevel(LEVEL_4);
    break;

    case A_POWER_LEVEL_5:
      sendDebug(F("Power Level 5"));
      b_state_changed = gpstarSystem.setPowerLevel(LEVEL_5);
    break;

    case A_ALARM_ON:
      sendDebug(F("Alarm On"));

      // Set ribbon cable status.
      b_ribbon_cable_attached = i_value == 1;

      // Alarm is on.
      b_wand_firing = false;
      b_pack_alarm = true;
      b_state_changed = true;

      bargraphFull();
      BARGRAPH_PATTERN = BG_RAMP_DOWN;

      if(b_pack_on) {
        ms_blink_leds.start(i_blink_leds);
      }
    break;

    case A_ALARM_OFF:
      sendDebug(F("Alarm Off"));

      // Set ribbon cable status.
      b_ribbon_cable_attached = i_value == 1;

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
      sendDebug(F("Quick Venting"));

      // Pack is performing quick vent; reset bargraph.
      i_cyclotron_multiplier = 1;
      b_overheating = true;
      b_state_changed = true;

      // Go to the standard power ramp.
      bargraphClear();
      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_VENTING_FINISHED:
      sendDebug(F("Quick Vent Complete"));

      // Quick vent process completed.
      b_overheating = false;
      b_state_changed = true;
    break;

    case A_OVERHEATING:
      sendDebug(F("Overheating"));

      // Pack is overheating.
      b_overheating = true;
      b_state_changed = true;
      ms_blink_leds.start(i_blink_leds);

      bargraphFull();
      BARGRAPH_PATTERN = BG_RAMP_DOWN;
    break;

    case A_OVERHEATING_FINISHED:
      sendDebug(F("Vented"));

      // Venting process completed.
      b_overheating = false;
      b_state_changed = true;
      ms_blink_leds.stop();

      i_cyclotron_multiplier = 1; // Return to normal speed.

      bargraphClear();
      BARGRAPH_PATTERN = BG_POWER_RAMP;
    break;

    case A_SYSTEM_LOCKOUT:
      sendDebug(F("Button Mash Lockout"));

      // Button mash lockout has been triggered.
      b_wand_mash_lockout = true;
      b_state_changed = true;

      if(b_pack_on) {
        bargraphFull();
        BARGRAPH_PATTERN = BG_RAMP_DOWN;

        ms_blink_leds.start(i_blink_leds);
      }
    break;

    case A_CANCEL_LOCKOUT:
      sendDebug(F("Mash Lockout Cancelled"));

      // Button mash lockout has been cancelled.
      b_wand_mash_lockout = false;
      b_state_changed = true;

      if(b_pack_on) {
        ms_blink_leds.stop();

        bargraphClear();
        BARGRAPH_PATTERN = BG_POWER_RAMP;
      }
    break;

    case A_FIRING:
      sendDebug(F("Firing"));

      b_wand_firing = true;
      b_state_changed = true;
      ms_blink_leds.start(int(i_blink_leds / i_cyclotron_multiplier));

      bargraphClear();
      BARGRAPH_PATTERN = BG_OUTER_INNER;
    break;

    case A_FIRING_STOPPED:
      sendDebug(F("Idle"));

      b_wand_firing = false;
      b_wand_firing_cts = false;
      b_state_changed = true;
      ms_blink_leds.stop();

      if(!b_overheating) {
        i_cyclotron_multiplier = 1; // Return to normal speed.
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

    case A_FIRING_CTS:
      sendDebug(F("Firing CTS"));

      if(b_wand_firing) {
        b_wand_firing_cts = true;
        b_state_changed = true;
      }
    break;

    case A_FIRING_CTS_STOPPED:
      sendDebug(F("Stopped CTS"));

      if(b_wand_firing || b_wand_firing_cts) {
        b_wand_firing_cts = false;
        b_state_changed = true;
      }
    break;

    case A_CYCLOTRON_LID_ON:
      sendDebug(F("Cyclotron Lid On..."));

      b_cyclotron_lid_on = true;
      b_state_changed = true;
    break;

    case A_CYCLOTRON_LID_OFF:
      sendDebug(F("Cyclotron Lid Off..."));

      b_cyclotron_lid_on = false;
      b_state_changed = true;
    break;

    case A_CYCLOTRON_INCREASE_SPEED:
      sendDebug(F("Cyclotron Speed Increasing..."));

      i_cyclotron_multiplier++;
      b_state_changed = true;

      sendDebug(String(i_cyclotron_multiplier));
    break;

    case A_CYCLOTRON_NORMAL_SPEED:
      sendDebug(F("Cyclotron Speed Reset"));

      i_cyclotron_multiplier = 1;
      b_state_changed = true;

      if(b_wand_firing) {
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
      if(gpstarSystem.getBarrelState() != BARREL_EXTENDED) {
        sendDebug(F("Wand Barrel Extended"));
        b_state_changed = gpstarSystem.setBarrelState(BARREL_EXTENDED);
      }
    break;

    case A_BARREL_RETRACTED:
      if(gpstarSystem.getBarrelState() != BARREL_RETRACTED) {
        sendDebug(F("Wand Barrel Retracted"));
        b_state_changed = gpstarSystem.setBarrelState(BARREL_RETRACTED);
      }
    break;

    case A_STREAM_FLAGS:
      sendDebug(F("Received new Stream Mode Flags"));

      gpstarSystem.setStreamModeOpts((uint8_t)i_value);
    break;

    case A_BATTERY_VOLTAGE_PACK:
      #if defined(DEBUG_SERIAL_COMMS)
        // This will be called a lot, so comment in only when needed.
        //sendDebug(String(F("Pack Voltage (x100): ")) + String(i_value));
      #endif

      // Convert to a value X.NN based on expected 5VDC maximum.
      f_batt_volts = (i_value > 0) ? ((float)i_value / 100.0) : 0.0;
      b_state_changed = true;
    break;

    case A_TEMPERATURE_PACK:
      #if defined(DEBUG_SERIAL_COMMS)
        // This will be called a lot, so comment in only when needed.
        //sendDebug(String(F("Pack Temperature (x100): ")) + String(i_value));
      #endif

      // Convert back to float with 2 decimal places
      f_temperature_c = (float)i_value / 100.0; // First convert to Celsius
      f_temperature_f = (f_temperature_c * 1.8) + 32; // Convert Celsius to Fahrenheit
      b_state_changed = true;
    break;

    case A_WAND_POWER_AMPS:
      #if defined(DEBUG_SERIAL_COMMS)
        // This will be called a lot, so we put it behind the debug option.
        sendDebug(String(F("Wand Current (x100): ")) + String(i_value));
      #endif

      // Convert to a value X.NN based on expected 1Amp maximum.
      f_wand_amps = (float)i_value / 100.0;
      b_state_changed = true;
    break;

    case A_WAND_AUDIO_VERSION:
      i_wand_audio_version = i_value;
      sendDebug(String(F("Wand Audio Version: ")) + String(i_value));
    break;

    default:
      // No-op for anything else.
    break;
  }

  // Indicates a change which should trigger an update to the websocket.
  return b_state_changed;
}
