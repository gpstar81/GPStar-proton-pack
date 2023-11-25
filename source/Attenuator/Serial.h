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

/*
 * Serial API Communication Handlers
 */

// Sends an API to the Proton Pack
void attenuatorSerialSend(uint16_t i_message) {
  sendStruct.i = i_message;
  sendStruct.s = A_COM_START;
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
              debug("Music Playing");
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
              debug("Music Stopped");
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
              debug("Music Track Sync");
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
            if(ARMING_MODE != MODE_SUPERHERO) {
              #if defined(__XTENSA__)
                debug("Super Hero Sequence");
              #endif
              ARMING_MODE = MODE_SUPERHERO;
              b_state_changed = true;
            }
          break;

          case A_MODE_ORIGINAL:
            if(ARMING_MODE != MODE_ORIGINAL) {
              #if defined(__XTENSA__)
                debug("Original Sequence");
              #endif
              ARMING_MODE = MODE_ORIGINAL;
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

/*
          case A_YEAR_FROZEN_EMPIRE:
            if(SYSTEM_YEAR != SYSTEM_FROZEN_EMPIRE) {
              #if defined(__XTENSA__)
                debug("Mode 2024");
              #endif
              SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
              b_state_changed = true;
            }
          break;
*/

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

            if(comStruct.d1 > 0) {
              i_spectral_custom = comStruct.d1;
            }

            if(comStruct.d2 > 0) {
              i_spectral_custom_saturation = comStruct.d2;
            }
          break;

          case A_SPECTRAL_COLOUR_DATA:
            #if defined(__XTENSA__)
              debug("Spectral Color Data");
            #endif
            if(comStruct.d1 > 0) {
              i_spectral_custom = comStruct.d1;
            }

            if(comStruct.d2 > 0) {
              i_spectral_custom_saturation = comStruct.d2;
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