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

// For pack communication.
struct __attribute__((packed)) DataPacket {
  uint16_t s;
  uint16_t i;
  uint16_t d1; // Reserved for values over 255 (eg. current music track)
  uint8_t d[25]; // Reserved for large data packets (eg. EEPROM configs)
  uint16_t e;
};

struct DataPacket comStruct;
struct DataPacket sendStruct;

// Translates a preferences to user-friendly names.
struct PackPrefs {
  uint8_t defaultSystemModePack;
  uint8_t defaultYearThemePack;
  uint8_t defaultSystemVolume;
  uint8_t cyclotronDirection;
  uint8_t demoLightMode;
  uint8_t protonStreamEffects;
  uint8_t smokeEnabled;
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
  uint8_t overheatDuration5;
  uint8_t overheatContinuous5;
  uint8_t overheatDuration4;
  uint8_t overheatContinuous4;
  uint8_t overheatDuration3;
  uint8_t overheatContinuous3;
  uint8_t overheatDuration2;
  uint8_t overheatContinuous2;
  uint8_t overheatDuration1;
  uint8_t overheatContinuous1;
  uint8_t overheatEnabled5;
  uint8_t overheatStartDelay5;
  uint8_t overheatEnabled4;
  uint8_t overheatStartDelay4;
  uint8_t overheatEnabled3;
  uint8_t overheatStartDelay3;
  uint8_t overheatEnabled2;
  uint8_t overheatStartDelay2;
  uint8_t overheatEnabled1;
  uint8_t overheatStartDelay1;
} smokeConfig;

/*
 * Serial API Communication Handlers
 */

// Sends an API to the Proton Pack
void attenuatorSerialSend(uint16_t i_message, uint16_t i_value = 0) {
  sendStruct.s = A_COM_START;
  sendStruct.i = i_message;
  sendStruct.d1 = i_value;

  // Get the number of elements in the data array
  uint16_t arrayLength = sizeof(sendStruct.d) / sizeof(sendStruct.d[0]);

  // Set each element of the data array to 0
  for (uint16_t i = 0; i < arrayLength; i++) {
    sendStruct.d[i] = 0;
  }

  switch(i_message) {
    case A_SAVE_PREFERENCES_PACK:
      // Convert user-friendly object properties to integer values.
      sendStruct.d[0] = packConfig.defaultSystemModePack;
      sendStruct.d[1] = packConfig.defaultYearThemePack;
      sendStruct.d[2] = packConfig.defaultSystemVolume;
      sendStruct.d[3] = packConfig.protonStreamEffects;
      sendStruct.d[4] = packConfig.smokeEnabled;
      sendStruct.d[5] = packConfig.overheatStrobeNF;
      sendStruct.d[6] = packConfig.overheatLightsOff;
      sendStruct.d[7] = packConfig.overheatSyncToFan;
      sendStruct.d[8] = packConfig.demoLightMode;

      // Cyclotron Lid
      sendStruct.d[9] = packConfig.ledCycLidCount;
      sendStruct.d[10] = packConfig.ledCycLidHue;
      sendStruct.d[11] = packConfig.ledCycLidSat;
      sendStruct.d[12] = packConfig.cyclotronDirection;
      sendStruct.d[13] = packConfig.ledCycLidCenter;
      sendStruct.d[14] = packConfig.ledVGCyclotron;
      sendStruct.d[15] = packConfig.ledCycLidSimRing;

      // Inner Cyclotron
      sendStruct.d[16] = packConfig.ledCycCakeCount;
      sendStruct.d[17] = packConfig.ledCycCakeHue;
      sendStruct.d[18] = packConfig.ledCycCakeSat;
      sendStruct.d[19] = packConfig.ledCycCakeGRB;

      // Power Cell
      sendStruct.d[20] = packConfig.ledPowercellCount;
      sendStruct.d[21] = packConfig.ledPowercellHue;
      sendStruct.d[22] = packConfig.ledPowercellSat;
      sendStruct.d[23] = packConfig.ledVGPowercell;
    break;

    case A_SAVE_PREFERENCES_WAND:
      // Convert user-friendly object properties to integer values.
      sendStruct.d[0] = wandConfig.ledWandCount;
      sendStruct.d[1] = wandConfig.ledWandHue;
      sendStruct.d[2] = wandConfig.ledWandSat;
      sendStruct.d[3] = wandConfig.spectralModeEnabled;
      sendStruct.d[4] = wandConfig.spectralHolidayMode;
      sendStruct.d[5] = wandConfig.overheatEnabled;
      sendStruct.d[6] = wandConfig.defaultFiringMode;
      sendStruct.d[7] = wandConfig.wandSoundsToPack;
      sendStruct.d[8] = wandConfig.quickVenting;
      sendStruct.d[9] = wandConfig.autoVentLight;
      sendStruct.d[10] = wandConfig.wandBeepLoop;
      sendStruct.d[11] = wandConfig.wandBootError;
      sendStruct.d[12] = wandConfig.defaultYearModeWand;
      sendStruct.d[13] = wandConfig.defaultYearModeCTS;
      sendStruct.d[14] = wandConfig.invertWandBargraph;
      sendStruct.d[15] = wandConfig.bargraphOverheatBlink;
      sendStruct.d[16] = wandConfig.bargraphIdleAnimation;
      sendStruct.d[17] = wandConfig.bargraphFireAnimation;
    break;

    case A_SAVE_PREFERENCES_SMOKE:
      // Convert user-friendly object properties to integer values.
      sendStruct.d[0] = smokeConfig.overheatDuration5;
      sendStruct.d[1] = smokeConfig.overheatContinuous5;
      sendStruct.d[2] = smokeConfig.overheatDuration4;
      sendStruct.d[3] = smokeConfig.overheatContinuous4;
      sendStruct.d[4] = smokeConfig.overheatDuration3;
      sendStruct.d[5] = smokeConfig.overheatContinuous3;
      sendStruct.d[6] = smokeConfig.overheatDuration2;
      sendStruct.d[7] = smokeConfig.overheatContinuous2;
      sendStruct.d[8] = smokeConfig.overheatDuration1;
      sendStruct.d[9] = smokeConfig.overheatContinuous1;
      sendStruct.d[10] = smokeConfig.overheatEnabled5;
      sendStruct.d[11] = smokeConfig.overheatStartDelay5;
      sendStruct.d[12] = smokeConfig.overheatEnabled4;
      sendStruct.d[13] = smokeConfig.overheatStartDelay4;
      sendStruct.d[14] = smokeConfig.overheatEnabled3;
      sendStruct.d[15] = smokeConfig.overheatStartDelay3;
      sendStruct.d[16] = smokeConfig.overheatEnabled2;
      sendStruct.d[17] = smokeConfig.overheatStartDelay2;
      sendStruct.d[18] = smokeConfig.overheatEnabled1;
      sendStruct.d[19] = smokeConfig.overheatStartDelay1;
    break;

    default:
      // No-op for all other communications.
    break;
  }

  sendStruct.e = A_COM_END;

  packComs.sendDatum(sendStruct);
}

// Handles an API (and data) sent from the Proton Pack
boolean checkPack() {
  bool b_state_changed = false; // Indicates when a crucial state change occurred.

  // Pack communication to the Attenuator device.
  if(packComs.available()) {
    packComs.rxObj(comStruct);

    if(!packComs.currentPacketID()) {
      if(comStruct.i > 0 && comStruct.s == A_COM_START && comStruct.e == A_COM_END) {
        // Use the passed communication flag to set the proper state for this device.
        switch(comStruct.i) {
          case A_PACK_BOOTUP:
            #if defined(__XTENSA__)
              debug("Pack Bootup");
            #endif
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
              debug("Music Playing: " + String(comStruct.d1));
            #endif

            b_playing_music = true;

            if(comStruct.d1 > 0 && i_music_track_current != comStruct.d1) {
              // Music track changed.
              i_music_track_current = comStruct.d1;
              b_state_changed = true;
            }
          break;

          case A_MUSIC_IS_NOT_PLAYING:
            #if defined(__XTENSA__)
              debug("Music Stopped: " + String(comStruct.d1));
            #endif

            b_playing_music = false;

            if(comStruct.d1 > 0 && i_music_track_current != comStruct.d1) {
              // Music track changed.
              i_music_track_current = comStruct.d1;
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
              debug("Music Track Sync: " + String(comStruct.d1));
            #endif

            if(comStruct.d1 > 0) {
              i_music_track_count = comStruct.d1;
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

            if(comStruct.d[0] > 0) {
              i_spectral_custom = comStruct.d[0];
            }

            if(comStruct.d[1] > 0) {
              i_spectral_custom_saturation = comStruct.d[1];
            }
          break;

          case A_SPECTRAL_COLOUR_DATA:
            #if defined(__XTENSA__)
              debug("Spectral Color Data");
            #endif

            if(comStruct.d[0] > 0) {
              i_spectral_custom = comStruct.d[0];
            }

            if(comStruct.d[1] > 0) {
              i_spectral_custom_saturation = comStruct.d[1];
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
            #endif

            // Convert integer values to user-friendly object properties.
            packConfig.defaultSystemModePack = comStruct.d[0];
            packConfig.defaultYearThemePack = comStruct.d[1];
            packConfig.defaultSystemVolume = comStruct.d[2];
            packConfig.protonStreamEffects = comStruct.d[3];
            packConfig.smokeEnabled = comStruct.d[4];
            packConfig.overheatStrobeNF = comStruct.d[5];
            packConfig.overheatLightsOff = comStruct.d[6];
            packConfig.overheatSyncToFan = comStruct.d[7];
            packConfig.demoLightMode = comStruct.d[8];

            packConfig.ledCycLidCount = comStruct.d[9];
            packConfig.ledCycLidHue = comStruct.d[10];
            packConfig.ledCycLidSat = comStruct.d[11];
            packConfig.cyclotronDirection = comStruct.d[12];
            packConfig.ledCycLidCenter = comStruct.d[13];
            packConfig.ledVGCyclotron = comStruct.d[14];
            packConfig.ledCycLidSimRing = comStruct.d[15];

            packConfig.ledCycCakeCount = comStruct.d[16];
            packConfig.ledCycCakeHue = comStruct.d[17];
            packConfig.ledCycCakeSat = comStruct.d[18];
            packConfig.ledCycCakeGRB = comStruct.d[19];

            packConfig.ledPowercellCount = comStruct.d[20];
            packConfig.ledPowercellHue = comStruct.d[21];
            packConfig.ledPowercellSat = comStruct.d[22];
            packConfig.ledVGPowercell = comStruct.d[23];
          break;

          case A_SEND_PREFERENCES_WAND:
            #if defined(__XTENSA__)
              debug("Wand Preferences Received");
            #endif

            // Convert integer values to user-friendly object properties.
            wandConfig.ledWandCount = comStruct.d[0];
            wandConfig.ledWandHue = comStruct.d[1];
            wandConfig.ledWandSat = comStruct.d[2];
            wandConfig.spectralModeEnabled = comStruct.d[3];
            wandConfig.spectralHolidayMode = comStruct.d[4];
            wandConfig.overheatEnabled = comStruct.d[5];
            wandConfig.defaultFiringMode = comStruct.d[6];
            wandConfig.wandSoundsToPack = comStruct.d[7];
            wandConfig.quickVenting = comStruct.d[8];
            wandConfig.autoVentLight = comStruct.d[9];
            wandConfig.wandBeepLoop = comStruct.d[10];
            wandConfig.wandBootError = comStruct.d[11];
            wandConfig.defaultYearModeWand = comStruct.d[12];
            wandConfig.defaultYearModeCTS = comStruct.d[13];
            wandConfig.invertWandBargraph = comStruct.d[14];
            wandConfig.bargraphOverheatBlink = comStruct.d[15];
            wandConfig.bargraphIdleAnimation = comStruct.d[16];
            wandConfig.bargraphFireAnimation = comStruct.d[17];
          break;

          case A_SEND_PREFERENCES_SMOKE:
            #if defined(__XTENSA__)
              debug("Smoke Preferences Received");
            #endif

            // Convert integer values to user-friendly object properties.
            smokeConfig.overheatDuration5 = comStruct.d[0];
            smokeConfig.overheatContinuous5 = comStruct.d[1];
            smokeConfig.overheatDuration4 = comStruct.d[2];
            smokeConfig.overheatContinuous4 = comStruct.d[3];
            smokeConfig.overheatDuration3 = comStruct.d[4];
            smokeConfig.overheatContinuous3 = comStruct.d[5];
            smokeConfig.overheatDuration2 = comStruct.d[6];
            smokeConfig.overheatContinuous2 = comStruct.d[7];
            smokeConfig.overheatDuration1 = comStruct.d[8];
            smokeConfig.overheatContinuous1 = comStruct.d[9];
            smokeConfig.overheatEnabled5 = comStruct.d[10];
            smokeConfig.overheatStartDelay5 = comStruct.d[11];
            smokeConfig.overheatEnabled4 = comStruct.d[12];
            smokeConfig.overheatStartDelay4 = comStruct.d[13];
            smokeConfig.overheatEnabled3 = comStruct.d[14];
            smokeConfig.overheatStartDelay3 = comStruct.d[15];
            smokeConfig.overheatEnabled2 = comStruct.d[16];
            smokeConfig.overheatStartDelay2 = comStruct.d[17];
            smokeConfig.overheatEnabled1 = comStruct.d[18];
            smokeConfig.overheatStartDelay1 = comStruct.d[19];
          break;

          default:
            // Nothing.
          break;
        }
      }

      comStruct.i = 0;
      comStruct.s = 0;
    }
  }

  return b_state_changed;
}