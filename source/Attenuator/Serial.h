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
bool b_a_sync_start = false; // Denotes pack communications have begun.

// For command signals (2 byte ID, 2 byte optional data).
struct __attribute__((packed)) CommandPacket {
  uint16_t i;
  uint16_t d1; // Reserved for values over 255 (eg. current music track)
};

// For pack communication (2 byte ID, 2 byte optional data, 24 byte data payload).
struct __attribute__((packed)) DataPacket {
  uint16_t i;
  uint16_t d1; // Reserved for values over 255 (eg. current music track)
  uint8_t d[23]; // Reserved for large data packets (eg. EEPROM configs)
};

struct DataPacket recvCmd;
struct DataPacket sendCmd;

struct DataPacket recvData;
struct DataPacket sendData;

// Translates a preferences to user-friendly names.
struct PackPrefs {
  uint8_t defaultSystemModePack;
  uint8_t defaultYearThemePack;
  uint8_t defaultSystemVolume;
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
  uint8_t ledVGCyclotron;
  uint8_t ledPowercellCount;
  uint8_t ledPowercellHue;
  uint8_t ledPowercellSat;
  uint8_t ledVGPowercell;
} packConfig;

struct WandPrefs {
  uint8_t ledWandCount;
  uint8_t ledWandHue;
  uint8_t ledWandSat;
  uint8_t spectralModeEnabled;
  uint8_t spectralHolidayMode;
  uint8_t overheatEnabled;
  uint8_t defaultFiringMode;
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

struct SmokePrefs {
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

// Sends an API to the Proton Pack
void attenuatorSerialCommand(uint16_t i_message, uint16_t i_value = 0) {
  sendCmd.i = i_message;
  sendCmd.d1 = i_value;
  packComs.sendDatum(sendCmd);
}
// Override function to handle calls with a single parameter.
void attenuatorSerialCommand(uint16_t i_message) {
  attenuatorSerialCommand(i_message, 0);
}

// Sends an API to the Proton Pack
void attenuatorSerialSend(uint16_t i_message, uint16_t i_value = 0) {
  sendData.i = i_message;
  sendData.d1 = i_value;

  // Set all elements of the data array to 0
  memset(sendData.d, 0, sizeof(sendData.d));

  switch(i_message) {
    case A_SAVE_PREFERENCES_PACK:
      #if defined(__XTENSA__)
        debug("Saving Pack Preferences");

        try {
          // Convert user-friendly object properties to integer values.
          sendData.d[0] = packConfig.defaultSystemModePack;
          sendData.d[1] = packConfig.defaultYearThemePack;
          sendData.d[2] = packConfig.defaultSystemVolume;
          sendData.d[3] = packConfig.protonStreamEffects;
          sendData.d[4] = packConfig.overheatStrobeNF;
          sendData.d[5] = packConfig.overheatLightsOff;
          sendData.d[6] = packConfig.overheatSyncToFan;
          sendData.d[7] = packConfig.demoLightMode;

          // Cyclotron Lid
          sendData.d[8] = packConfig.ledCycLidCount;
          sendData.d[9] = packConfig.ledCycLidHue;
          sendData.d[10] = packConfig.ledCycLidSat;
          sendData.d[11] = packConfig.cyclotronDirection;
          sendData.d[12] = packConfig.ledCycLidCenter;
          sendData.d[13] = packConfig.ledVGCyclotron;
          sendData.d[14] = packConfig.ledCycLidSimRing;

          // Inner Cyclotron
          sendData.d[15] = packConfig.ledCycCakeCount;
          sendData.d[16] = packConfig.ledCycCakeHue;
          sendData.d[17] = packConfig.ledCycCakeSat;
          sendData.d[18] = packConfig.ledCycCakeGRB;

          // Power Cell
          sendData.d[19] = packConfig.ledPowercellCount;
          sendData.d[20] = packConfig.ledPowercellHue;
          sendData.d[21] = packConfig.ledPowercellSat;
          sendData.d[22] = packConfig.ledVGPowercell;
        }
        catch (...) {
          debug("Error while saving pack preferences");
        }
      #endif
    break;

    case A_SAVE_PREFERENCES_WAND:
      #if defined(__XTENSA__)
        debug("Saving Wand Preferences");

        try {
          // Convert user-friendly object properties to integer values.
          sendData.d[0] = wandConfig.ledWandCount;
          sendData.d[1] = wandConfig.ledWandHue;
          sendData.d[2] = wandConfig.ledWandSat;
          sendData.d[3] = wandConfig.spectralModeEnabled;
          sendData.d[4] = wandConfig.spectralHolidayMode;
          sendData.d[5] = wandConfig.overheatEnabled;
          switch(wandConfig.defaultFiringMode) {
            case 3:
              sendData.d[6] = 1; // CTS
              sendData.d[7] = 1; // CTS Mix
            break;
            case 2:
              sendData.d[6] = 1; // CTS
              sendData.d[7] = 0; // CTS Mix
            break;
            case 1:
            default:
              sendData.d[6] = 0; // CTS
              sendData.d[7] = 0; // CTS Mix
            break;
          }
          sendData.d[8] = wandConfig.wandSoundsToPack;
          sendData.d[9] = wandConfig.quickVenting;
          sendData.d[10] = wandConfig.autoVentLight;
          sendData.d[11] = wandConfig.wandBeepLoop;
          sendData.d[12] = wandConfig.wandBootError;
          sendData.d[13] = wandConfig.defaultYearModeWand;
          sendData.d[14] = wandConfig.defaultYearModeCTS;
          sendData.d[15] = wandConfig.invertWandBargraph;
          sendData.d[16] = wandConfig.bargraphOverheatBlink;
          sendData.d[17] = wandConfig.bargraphIdleAnimation;
          sendData.d[18] = wandConfig.bargraphFireAnimation;
        }
        catch (...) {
          debug("Error while saving wand preferences");
        }
      #endif
    break;

    case A_SAVE_PREFERENCES_SMOKE:
      #if defined(__XTENSA__)
        debug("Saving Smoke Preferences");

        try {
          // Convert user-friendly object properties to integer values.
          sendData.d[0] = smokeConfig.overheatDuration5;
          sendData.d[1] = smokeConfig.overheatDuration4;
          sendData.d[2] = smokeConfig.overheatDuration3;
          sendData.d[3] = smokeConfig.overheatDuration2;
          sendData.d[4] = smokeConfig.overheatDuration1;

          sendData.d[5] = smokeConfig.overheatContinuous5;
          sendData.d[6] = smokeConfig.overheatContinuous4;
          sendData.d[7] = smokeConfig.overheatContinuous3;
          sendData.d[8] = smokeConfig.overheatContinuous2;
          sendData.d[9] = smokeConfig.overheatContinuous1;

          sendData.d[10] = smokeConfig.overheatLevel5;
          sendData.d[11] = smokeConfig.overheatLevel4;
          sendData.d[12] = smokeConfig.overheatLevel3;
          sendData.d[13] = smokeConfig.overheatLevel2;
          sendData.d[14] = smokeConfig.overheatLevel1;

          sendData.d[15] = smokeConfig.overheatDelay5;
          sendData.d[16] = smokeConfig.overheatDelay4;
          sendData.d[17] = smokeConfig.overheatDelay3;
          sendData.d[18] = smokeConfig.overheatDelay2;
          sendData.d[19] = smokeConfig.overheatDelay1;

          sendData.d[20] = smokeConfig.smokeEnabled;
        }
        catch (...) {
          debug("Error while saving smoke preferences");
        }
      #endif
    break;

    default:
      // No-op for all other communications.
    break;
  }

  packComs.sendDatum(sendData);
}

// Handles an API (and data) sent from the Proton Pack
boolean checkPack() {
  bool b_state_changed = false; // Indicates when a crucial state change occurred.

  // Pack communication to the Attenuator device.
  if(packComs.available()) {
    //packComs.rxObj(recvCmd);
    packComs.rxObj(recvData);

    if(!packComs.currentPacketID()) {
Serial.println("Command: " + String(recvCmd.i));
Serial.println("Message: " + String(recvData.i));

      // Use the passed communication flag to set the proper state for this device.
      switch(recvData.i) {
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
          b_a_sync_start = true;
        break;

        case A_SYNC_END:
          #if defined(__XTENSA__)
            debug("Sync End");
          #endif

          b_wait_for_pack = false;
          b_a_sync_start = false;
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
            debug("Music Playing: " + String(recvData.d1));
          #endif

          b_playing_music = true;

          if(recvData.d1 > 0 && i_music_track_current != recvData.d1) {
            // Music track changed.
            i_music_track_current = recvData.d1;
            b_state_changed = true;
          }
        break;

        case A_MUSIC_IS_NOT_PLAYING:
          #if defined(__XTENSA__)
            debug("Music Stopped: " + String(recvData.d1));
          #endif

          b_playing_music = false;

          if(recvData.d1 > 0 && i_music_track_current != recvData.d1) {
            // Music track changed.
            i_music_track_current = recvData.d1;
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
            debug("Music Track Sync: " + String(recvData.d1));
          #endif

          if(recvData.d1 > 0) {
            i_music_track_count = recvData.d1;
          }

          #if defined(__XTENSA__)
            debug("Track Count: " + String(i_music_track_count));
          #endif

          if(i_music_track_count > 0) {
            i_music_track_min = i_music_track_offset; // First music track possible (eg. 500)
            i_music_track_max = i_music_track_offset + i_music_track_count - 1; // 500 + N - 1 to be inclusive of the offset value.
          }
        break;

        case A_VOLUME_SYNC:
          #if defined(__XTENSA__)
            debug("Volume Sync");

            try {
              i_volume_master_percentage = recvData.d[0];
              i_volume_effects_percentage = recvData.d[1];
              i_volume_music_percentage = recvData.d[2];
            }
            catch (...) {
              debug("Error during volume sync");
            }

            b_state_changed = true;
          #endif
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

          if(b_wait_for_pack && !b_a_sync_start) {
            b_a_sync_start = true;
            attenuatorSerialSend(A_SYNC_START);
          }
          else if(b_a_sync_start != true) {
            // The pack is asking us if we are still here. Respond back.
            attenuatorSerialSend(A_HANDSHAKE);
          }
        break;

        case A_MODE_SUPER_HERO:
          if(SYSTEM_MODE != MODE_SUPERHERO) {
            #if defined(__XTENSA__)
              debug("Super Hero Sequence");
            #endif
            SYSTEM_MODE = MODE_SUPERHERO;
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

        case A_SPECTRAL_CUSTOM_MODE:
          #if defined(__XTENSA__)
            debug("Spectral Custom");
          #endif
          FIRING_MODE = SPECTRAL_CUSTOM;
          b_state_changed = true;

          if(recvData.d[0] > 0) {
            i_spectral_custom = recvData.d[0];
          }

          if(recvData.d[1] > 0) {
            i_spectral_custom_saturation = recvData.d[1];
          }
        break;

        case A_SPECTRAL_COLOUR_DATA:
          #if defined(__XTENSA__)
            debug("Spectral Color Data");
          #endif

          if(recvData.d[0] > 0) {
            i_spectral_custom = recvData.d[0];
          }

          if(recvData.d[1] > 0) {
            i_spectral_custom_saturation = recvData.d[1];
          }
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

        case A_VENTING_MODE:
          #if defined(__XTENSA__)
            debug("Venting");
          #endif
          FIRING_MODE = VENTING;
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

        case A_SEND_PREFERENCES_PACK:
          #if defined(__XTENSA__)
            debug("Pack Preferences Received");

            try {
              // Convert integer values to user-friendly object properties.
              packConfig.defaultSystemModePack = recvData.d[0];
              packConfig.defaultYearThemePack = recvData.d[1];
              packConfig.defaultSystemVolume = recvData.d[2];
              packConfig.protonStreamEffects = recvData.d[3];
              packConfig.overheatStrobeNF = recvData.d[4];
              packConfig.overheatLightsOff = recvData.d[5];
              packConfig.overheatSyncToFan = recvData.d[6];
              packConfig.demoLightMode = recvData.d[7];

              packConfig.ledCycLidCount = recvData.d[8];
              packConfig.ledCycLidHue = recvData.d[9];
              packConfig.ledCycLidSat = recvData.d[10];
              packConfig.cyclotronDirection = recvData.d[11];
              packConfig.ledCycLidCenter = recvData.d[12];
              packConfig.ledVGCyclotron = recvData.d[13];
              packConfig.ledCycLidSimRing = recvData.d[14];

              packConfig.ledCycCakeCount = recvData.d[15];
              packConfig.ledCycCakeHue = recvData.d[16];
              packConfig.ledCycCakeSat = recvData.d[17];
              packConfig.ledCycCakeGRB = recvData.d[18];

              packConfig.ledPowercellCount = recvData.d[19];
              packConfig.ledPowercellHue = recvData.d[20];
              packConfig.ledPowercellSat = recvData.d[21];
              packConfig.ledVGPowercell = recvData.d[22];
            }
            catch (...) {
              debug("Error while receiving pack preferences");
            }
          #endif
        break;

        case A_SEND_PREFERENCES_WAND:
          #if defined(__XTENSA__)
            debug("Wand Preferences Received");

            try{
              // Convert integer values to user-friendly object properties.
              wandConfig.ledWandCount = recvData.d[0];
              wandConfig.ledWandHue = recvData.d[1];
              wandConfig.ledWandSat = recvData.d[2];
              wandConfig.spectralModeEnabled = recvData.d[3];
              wandConfig.spectralHolidayMode = recvData.d[4];
              wandConfig.overheatEnabled = recvData.d[5];
              if(recvData.d[6] == 1 && recvData.d[7] == 1) {
                wandConfig.defaultFiringMode = 3; // CTS Mix
              }
              else if(recvData.d[6] == 1 && recvData.d[7] == 0) {
                wandConfig.defaultFiringMode = 2; // CTS
              }
              else {
                wandConfig.defaultFiringMode = 1; // VG
              }
              wandConfig.wandSoundsToPack = recvData.d[8];
              wandConfig.quickVenting = recvData.d[9];
              wandConfig.autoVentLight = recvData.d[10];
              wandConfig.wandBeepLoop = recvData.d[11];
              wandConfig.wandBootError = recvData.d[12];
              wandConfig.defaultYearModeWand = recvData.d[13];
              wandConfig.defaultYearModeCTS = recvData.d[14];
              wandConfig.invertWandBargraph = recvData.d[15];
              wandConfig.bargraphOverheatBlink = recvData.d[16];
              wandConfig.bargraphIdleAnimation = recvData.d[17];
              wandConfig.bargraphFireAnimation = recvData.d[18];
            }
            catch (...) {
              debug("Error while receiving wand preferences");
            }
          #endif
        break;

        case A_SEND_PREFERENCES_SMOKE:
          #if defined(__XTENSA__)
            debug("Smoke Preferences Received");

            try {
              // Convert integer values to user-friendly object properties.
              smokeConfig.overheatDuration5 = recvData.d[0];
              smokeConfig.overheatDuration4 = recvData.d[1];
              smokeConfig.overheatDuration3 = recvData.d[2];
              smokeConfig.overheatDuration2 = recvData.d[3];
              smokeConfig.overheatDuration1 = recvData.d[4];

              smokeConfig.overheatContinuous5 = recvData.d[5];
              smokeConfig.overheatContinuous4 = recvData.d[6];
              smokeConfig.overheatContinuous3 = recvData.d[7];
              smokeConfig.overheatContinuous2 = recvData.d[8];
              smokeConfig.overheatContinuous1 = recvData.d[9];

              smokeConfig.overheatLevel5 = recvData.d[10];
              smokeConfig.overheatLevel4 = recvData.d[11];
              smokeConfig.overheatLevel3 = recvData.d[12];
              smokeConfig.overheatLevel2 = recvData.d[13];
              smokeConfig.overheatLevel1 = recvData.d[14];

              smokeConfig.overheatDelay5 = recvData.d[15];
              smokeConfig.overheatDelay4 = recvData.d[16];
              smokeConfig.overheatDelay3 = recvData.d[17];
              smokeConfig.overheatDelay2 = recvData.d[18];
              smokeConfig.overheatDelay1 = recvData.d[19];

              smokeConfig.smokeEnabled = recvData.d[20];
            }
            catch (...) {
              debug("Error while receiving smoke preferences");
            }
          #endif
        break;

        case A_BATTERY_VOLTAGE_PACK:
          #if defined(__XTENSA__)
            debug("Voltage: " + String(recvData.d1));
          #endif

          // Convert to a value X.NN based on expected 5VDC maximum.
          f_batt_volts = (float) recvData.d1 / 100;
          b_state_changed = true;
        break;

        default:
          // No-op for anything else.
        break;
      }
    }
  }

  // Indicates a change which should trigger an update to the websocket.
  return b_state_changed;
}