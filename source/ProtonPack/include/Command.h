/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2026 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

// Forward function declarations.
void doAttenuatorSync(); // From Serial.h
void notifyWSClients(); // From Webhandler.h

/**
 * Centralized handler for commands, allowing the Pack and Attenuator to both perform the same action.
 * This approach is applying the Command Pattern to decouple the sender from the receiver.
 * In order for this to work, the command value must come from a unique source: API_MESSAGE
 *
 * Inputs:
 *   - i_command: Command identifier (API_MESSAGE enum)
 *   - i_value: Optional value for the command (default 0)
 */
void executeCommand(uint8_t i_command, uint16_t i_value = 0) {
  switch(i_command) {
    case A_SYNC_START:
      // Attenuator has explicitly asked to be synchronized.
      doAttenuatorSync();
    break;

    case A_HANDSHAKE:
      b_attenuator_syncing = false; // No longer attempting to force a sync w/ Attenuator.
      b_attenuator_connected = true; // If we're receiving handshake instead of SYNC_NOW we must be connected.

      if(b_diagnostic) {
        // While in diagnostic mode, play a sound to indicate the wand is connected.
        playEffect(S_BEEPS_ALT);
      }
    break;

    case A_SYNC_END:
      sendDebug(F("Attenuator Synchronized"));
      b_attenuator_syncing = false;
      b_attenuator_connected = true;
      ms_attenuator_check.start(i_attenuator_disconnect_delay);
      #ifdef ESP32
      if(WIFI_USER_MODE == WIFI_DEFAULT) {
        WIFI_USER_MODE = WIFI_DISABLED; // Disable WiFi as the Attenuator handles it.
      }
      #endif
    break;

    case A_TURN_PACK_ON:
      // Pretend the ion arm switch was just turned on.
      gpstarPack.setIonArmSwitch(RED_SWITCH_ON);

      if(gpstarPack.getSystemMode() == MODE_SUPER_HERO && PACK_STATE == MODE_OFF) {
        PACK_ACTION_STATE = ACTION_ACTIVATE;
      }

      // Tell the Neutrona Wand that power to the Proton Pack is on.
      if(b_wand_connected) {
        packSerialSend(P_ION_ARM_SWITCH_ON);
      }

      // Tell the Attenuator or any other device that the power to the Proton Pack is on.
      attenuatorSerialSend(A_ION_ARM_SWITCH_ON);
    break;

    case A_TURN_PACK_OFF:
      // Pretend the ion arm switch was just turned on.
      gpstarPack.setIonArmSwitch(RED_SWITCH_OFF);

      if(PACK_STATE == MODE_ON) {
        PACK_ACTION_STATE = ACTION_OFF;

        //Make sure to tell the wireless that we are not overheating.
        attenuatorSerialSend(A_OVERHEATING_FINISHED);
      }

      // Tell the Neutrona Wand that power to the Proton Pack is off.
      if(b_wand_connected) {
        packSerialSend(P_ION_ARM_SWITCH_OFF);
      }

      // Tell the Attenuator or any other device that the power to the Proton Pack is off.
      attenuatorSerialSend(A_ION_ARM_SWITCH_OFF);
    break;

    case A_WARNING_CANCELLED:
      // Tell wand to reset overheat warning.
      packSerialSend(P_WARNING_CANCELLED);
    break;

    case A_MANUAL_OVERHEAT:
      // Trigger a manual overheat vent.
      if(b_wand_connected) {
        packSerialSend(P_MANUAL_OVERHEAT);
      }
      else if(PACK_STATE == MODE_ON) {
        packOverheatingStart();
      }
    break;

    case A_MANUAL_QUICK_VENT:
      // Trigger a manual quick vent.
      if(b_wand_connected) {
        packSerialSend(P_MANUAL_QUICK_VENT);
      }
      else if(PACK_STATE == MODE_ON) {
        packVentingStart();
      }
    break;

    case A_ALARM_ON:
      // Simulate the ribbon cable being detached.
      if(!b_use_ribbon_cable) {
        b_manual_cable_alarm = true;
        attenuatorSerialSend(A_ALARM_ON, ribbonCableAttached() ? 1 : 0);
      }
    break;

    case A_ALARM_OFF:
      // Simulate the ribbon cable being attached.
      if(!b_use_ribbon_cable) {
        b_manual_cable_alarm = false;
        attenuatorSerialSend(A_ALARM_OFF, ribbonCableAttached() ? 1 : 0);
      }
    break;

    case A_SYSTEM_LOCKOUT:
      // Simulate a lockout as if by repeated button presses on the wand.
      if(b_wand_connected) {
        // Tell the wand to lock us out.
        packSerialSend(P_SYSTEM_LOCKOUT);
      }
      else {
        // No wand connected, so do it ourselves.
        startWandMashLockout(6000);
      }

      switch(gpstarPack.getSystemTheme()) {
        case SYSTEM_FROZEN_EMPIRE:
          // No-op for this theme, as this is handled in startWandMashLockout
        break;
        default:
          // Plays the alarm loop as heard on the wand.
          stopMashErrorSounds();
          playEffect(S_MASH_ERROR_LOOP, true, i_volume_effects, true, 2500);
        break;
      }
    break;

    case A_CANCEL_LOCKOUT:
      // Initiate a restart of the pack after a lockout event has occurred.
      if(b_wand_connected) {
        // Tell the wand to restart us.
        packSerialSend(P_CANCEL_LOCKOUT);
      }
      else {
        // No wand connected, so do it ourselves.
        restartFromWandMash();
      }
    break;

    case A_TOGGLE_SMOKE:
      if(i_value != 0) {
        b_smoke_enabled = i_value == 2;
      }
      else {
        b_smoke_enabled = !b_smoke_enabled;
      }
      attenuatorSerialSend(A_TOGGLE_SMOKE, b_smoke_enabled ? 2 : 1);
    break;

    case A_TOGGLE_VIBRATION:
      if(i_value != 0) {
        b_vibration_switch_on = i_value == 2;
      }
      else {
        b_vibration_switch_on = !b_vibration_switch_on;
      }
      attenuatorSerialSend(A_TOGGLE_VIBRATION, b_vibration_switch_on ? 2 : 1);
    break;

    case A_CYCLOTRON_DIRECTION_TOGGLE:
      if(i_value != 0) {
        b_clockwise = i_value == 2;
      }
      else {
        b_clockwise = !b_clockwise;
      }
      attenuatorSerialSend(A_CYCLOTRON_DIRECTION_TOGGLE, b_clockwise ? 2 : 1);
    break;

    case A_TOGGLE_MUTE:
      toggleMute(i_value);
      attenuatorSerialSend(A_TOGGLE_MUTE, i_volume_master == i_volume_abs_min ? 2 : 1);
      packSerialSend(P_MASTER_AUDIO_STATUS, i_volume_master == i_volume_abs_min ? 2 : 1);
    break;

    case A_VOLUME_DECREASE:
      // Decrease overall pack volume.
      decreaseVolume();
    break;

    case A_VOLUME_INCREASE:
      // Increase overall pack volume.
      increaseVolume();
    break;

    case A_VOLUME_SET:
      // Set master volume to specific percentage from Attenuator.
      setMasterVolumePercentage((uint8_t)i_value);
    break;

    case A_VOLUME_SOUND_EFFECTS_DECREASE:
      // Decrease pack effects volume.
      decreaseVolumeEffects();

      // Tell wand to decrease effects volume.
      packSerialSend(P_VOLUME_SOUND_EFFECTS_DECREASE);
    break;

    case A_VOLUME_SOUND_EFFECTS_INCREASE:
      // Increase pack effects volume.
      increaseVolumeEffects();

      // Tell wand to increase effects volume.
      packSerialSend(P_VOLUME_SOUND_EFFECTS_INCREASE);
    break;

    case A_VOLUME_MUSIC_DECREASE:
      // Decrease pack music volume.
      decreaseVolumeMusic();
    break;

    case A_VOLUME_MUSIC_INCREASE:
      // Increase pack music volume.
      increaseVolumeMusic();
    break;

    case A_MUSIC_START_STOP:
      if(b_playing_music) {
        stopMusic();
      }
      else {
        playMusic();
      }
    break;

    case A_MUSIC_PAUSE_RESUME:
      if(b_playing_music) {
        // If last playing music, either pause or resume.
        if(!b_music_paused) {
          pauseMusic();
        }
        else {
          resumeMusic();
        }
      }
      else {
        // if not playing music, start playing the current track.
        playMusic();
      }
    break;

    case A_MUSIC_NEXT_TRACK:
      musicNextTrack();
    break;

    case A_MUSIC_PREV_TRACK:
      musicPrevTrack();
    break;

    case A_MUSIC_TRACK_LOOP_TOGGLE:
      toggleMusicLoop(i_value);
      attenuatorSerialSend(A_MUSIC_TRACK_LOOP_TOGGLE, b_repeat_track ? 2 : 1);
      packSerialSend(P_MUSIC_LOOP_STATUS, b_repeat_track ? 2 : 1);
    break;

    case A_MUSIC_TRACK_SHUFFLE_TOGGLE:
      toggleMusicShuffle(i_value);
      attenuatorSerialSend(A_MUSIC_TRACK_SHUFFLE_TOGGLE, b_shuffle_tracks ? 2 : 1);
      packSerialSend(P_MUSIC_SHUFFLE_STATUS, b_shuffle_tracks ? 2 : 1);
    break;

    case A_YEAR_1984:
      gpstarPack.setSystemTheme(SYSTEM_1984);
      SYSTEM_THEME_TEMP = gpstarPack.getSystemTheme();
      b_switch_mode_override = true; // Explicit mode set, override mode toggle.
      packSerialSend(P_YEAR_1984);
      attenuatorSerialSend(A_YEAR_1984);
      playEffect(S_VOICE_1984);
      resetRampSpeeds();
      packOffReset();
      sendDebug(F("Theme changed to GB1 (1984)"));
    break;

    case A_YEAR_1989:
      gpstarPack.setSystemTheme(SYSTEM_1989);
      SYSTEM_THEME_TEMP = gpstarPack.getSystemTheme();
      b_switch_mode_override = true; // Explicit mode set, override mode toggle.
      packSerialSend(P_YEAR_1989);
      attenuatorSerialSend(A_YEAR_1989);
      playEffect(S_VOICE_1989);
      resetRampSpeeds();
      packOffReset();
      sendDebug(F("Theme changed to GB2 (1989)"));
    break;

    case A_YEAR_AFTERLIFE:
      gpstarPack.setSystemTheme(SYSTEM_AFTERLIFE);
      SYSTEM_THEME_TEMP = gpstarPack.getSystemTheme();
      b_switch_mode_override = true; // Explicit mode set, override mode toggle.
      packSerialSend(P_YEAR_AFTERLIFE);
      attenuatorSerialSend(A_YEAR_AFTERLIFE);
      playEffect(S_VOICE_AFTERLIFE);
      resetRampSpeeds();
      packOffReset();
      sendDebug(F("Theme changed to Afterlife (2021)"));
    break;

    case A_YEAR_FROZEN_EMPIRE:
      gpstarPack.setSystemTheme(SYSTEM_FROZEN_EMPIRE);
      SYSTEM_THEME_TEMP = gpstarPack.getSystemTheme();
      b_switch_mode_override = true; // Explicit mode set, override mode toggle.
      packSerialSend(P_YEAR_FROZEN_EMPIRE);
      attenuatorSerialSend(A_YEAR_FROZEN_EMPIRE);
      playEffect(S_VOICE_FROZEN_EMPIRE);
      resetRampSpeeds();
      packOffReset();
      sendDebug(F("Theme changed to Frozen Empire (2024)"));
    break;

    case A_SET_STREAM_MODE:
      // Leave if unable to set the intended stream mode.
      if(!gpstarPack.setStreamMode((STREAM_MODES)i_value)){ return; }

      // Propagate the stream mode change to both Attenuator and Wand.
      attenuatorSerialSend(A_SET_STREAM_MODE, gpstarPack.getStreamModeByte());
      packSerialSend(P_SET_STREAM_MODE, gpstarPack.getStreamModeByte());

      // Meson requires a more rapid-fire triggering of sound effects.
      if(gpstarPack.inStreamMode(MESON) && i_audio_version < 109) {
        useShortTrackOverload(true);
      }
      else if(gpstarPack.switchedFromStream(MESON) && i_audio_version < 109) {
        useShortTrackOverload(false);
      }

      // Handle special modes that are outside of normal firing streams.
      if(gpstarPack.inStreamMode(SETTINGS)) {
        playEffect(S_CLICK);
        b_settings = true;
        return;
      }
      else if(b_settings) {
        playEffect(S_CLICK);
        b_settings = false;
      }

      // Reset cyclotron state and idle loop volume if switching out of Slime.
      stopCyclotronSlimeEffects(gpstarPack.getPreviousStreamMode());

      // After a legitimate stream switch stop any special sound effects.
      stopEffect(S_PACK_SLIME_TANK_LOOP);
      stopEffect(S_STASIS_IDLE_LOOP);
      stopEffect(S_MESON_IDLE_LOOP);

      if(PACK_STATE == MODE_ON) {
        if(b_fadeout_idle_sounds) {
          // We have to restart the pack idle just in case.
          switch(gpstarPack.getSystemTheme()) {
            case SYSTEM_1984:
              stopEffect(S_GB1_1984_PACK_LOOP);
              playEffect(S_GB1_1984_PACK_LOOP, true);
            break;
            case SYSTEM_1989:
              stopEffect(S_GB2_PACK_LOOP);
              playEffect(S_GB2_PACK_LOOP, true);
            break;
            case SYSTEM_AFTERLIFE:
            default:
              stopEffect(S_AFTERLIFE_PACK_IDLE_LOOP);

              if(gpstarPack.inStreamMode(SLIME)) {
                playEffect(S_AFTERLIFE_PACK_IDLE_LOOP, true, i_volume_effects - i_slime_idle_level);
              }
              else {
                playEffect(S_AFTERLIFE_PACK_IDLE_LOOP, true);
              }

              packSerialSend(P_REQUEST_BEEP_SYNC);
            break;
            case SYSTEM_FROZEN_EMPIRE:
              stopEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP);

              if(gpstarPack.inStreamMode(SLIME)) {
                playEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, true, i_volume_effects - i_slime_idle_level);
              }
              else {
                playEffect(S_FROZEN_EMPIRE_PACK_IDLE_LOOP, true);
              }

              if(!isBrassPack()) {
                packSerialSend(P_REQUEST_BEEP_SYNC);
              }
            break;
          }

          ms_delay_post.start(i_idle_fadeout_time);
        }

        // Trigger the proper sound effect(s) for the new stream mode.
        switch(gpstarPack.getStreamMode()) {
          case PROTON:
            playEffect(S_FIRE_START_SPARK);
          break;
          case STASIS:
            playEffect(S_STASIS_OPEN);
            playEffect(S_STASIS_IDLE_LOOP, true, i_volume_effects, true, 2000);
          break;
          case SLIME:
            playEffect(S_PACK_SLIME_OPEN);
            playEffect(S_PACK_SLIME_TANK_LOOP, true, i_volume_effects, true, 700);
          break;
          case MESON:
            playEffect(S_MESON_OPEN);
            playEffect(S_MESON_IDLE_LOOP, true, i_volume_effects, true, 1250);
          break;
          case SPECTRAL:
            playEffect(S_FIRE_START_SPARK);
          break;
          case HOLIDAY_HALLOWEEN:
            playEffect(S_HALLOWEEN_MODE_VOICE);
          break;
          case HOLIDAY_CHRISTMAS:
            playEffect(S_CHRISTMAS_MODE_VOICE);
          break;
          case SPECTRAL_CUSTOM:
            playEffect(S_FIRE_START_SPARK);
          break;
          default:
            // For all other modes, do nothing.
          break;
        }

        // Trigger an update to long-running sound volume.
        updateEffectsVolume();
      }

      if(b_cyclotron_colour_toggle) {
        if(gpstarPack.inStreamMode(SLIME)) {
          // Reset the Cyclotron states.
          resetCyclotronState();
          clearCyclotronFades();
        }

        // Reset the Cyclotron LED colours.
        cyclotronColourReset();
      }

      if(b_powercell_colour_toggle && PACK_STATE == MODE_ON) {
        // Reset the Power Cell colours if the Power Cell is running.
        b_powercell_updating = true;
        powercellDraw();
      }

      // Update the Inner Cyclotron LEDs if required.
      cyclotronSwitchLEDUpdate();
    break;

    case A_REQUEST_PREFERENCES_PACK:
      // If requested by the Attenuator, send back all pack EEPROM preferences.
      // This will send a data payload directly from the pack as all data is local.
      attenuatorSendData(A_SEND_PREFERENCES_PACK);
    break;

    case A_REQUEST_PREFERENCES_WAND:
      // If requested by the Attenuator, tell the wand we need its EEPROM preferences.
      // This is merely a command to the wand which tells it to send back a data payload.
      b_received_prefs_wand = false;

      if(b_wand_connected) {
        packSerialSend(P_SEND_PREFERENCES_WAND);
      }
    break;

    case A_REQUEST_PREFERENCES_SMOKE:
      if(b_wand_connected) {
        // If requested by the Attenuator, tell the wand we need its EEPROM preferences.
        // This is merely a command to the wand which tells it to send back a data payload.
        packSerialSend(P_SEND_PREFERENCES_SMOKE);
      }
      else {
        // If a wand is not connected, simply return the smoke settings from the pack.
        attenuatorSendData(A_SEND_PREFERENCES_SMOKE);
      }
    break;

    case A_MUSIC_PLAY_TRACK:
      // Music track number to be played.
      if(i_music_track_count > 0 && i_value >= i_music_track_start) {
        if(b_playing_music) {
          stopMusic(); // Stops current track before change.

          // Only update after the music is stopped.
          i_current_music_track = i_value;

          // Play the appropriate track on pack and wand, and notify the Attenuator.
          playMusic();
        }
        else {
          i_current_music_track = i_value;
        }
      }
    break;

    case A_SAVE_EEPROM_SETTINGS_PACK:
      // Commit changes to the EEPROM in the pack controller
      saveLEDEEPROM();
      saveConfigEEPROM();

      // Offer some feedback to the user
      stopEffect(S_VOICE_EEPROM_SAVE);
      playEffect(S_VOICE_EEPROM_SAVE);
    break;

    case A_SAVE_EEPROM_SETTINGS_WAND:
      // Commit changes to the EEPROM on the wand controller
      packSerialSend(P_SAVE_EEPROM_WAND);

      // Offer some feedback to the user
      stopEffect(S_VOICE_EEPROM_SAVE);
      playEffect(S_VOICE_EEPROM_SAVE);
    break;

    case A_RESET_EEPROM_SETTINGS_PACK:
      // Reset the EEPROM on the pack controller
      clearLEDEEPROM();
      clearConfigEEPROM();

      // Offer some feedback to the user
      stopEffect(S_VOICE_EEPROM_ERASE);
      playEffect(S_VOICE_EEPROM_ERASE);
    break;

    case A_RESET_EEPROM_SETTINGS_WAND:
      // Reset the EEPROM on the wand controller
      packSerialSend(P_RESET_EEPROM_WAND);

      // Offer some feedback to the user
      stopEffect(S_VOICE_EEPROM_ERASE);
      playEffect(S_VOICE_EEPROM_ERASE);
    break;

    default:
      // No-op for anything else.
    break;
  }
}
