/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
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
      WIFI_MODE = WIFI_DISABLED;
      #endif
    break;

    case A_TURN_PACK_ON:
      // Pretend the ion arm switch was just turned on.
      if(SYSTEM_MODE == MODE_SUPER_HERO) {
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
      else if(b_pack_on) {
        packOverheatingStart();
      }
    break;

    case A_SYSTEM_LOCKOUT:
      // Simulate a lockout as if by repeated button presses on the wand.
      startWandMashLockout(6000);

      switch(SYSTEM_YEAR) {
        case SYSTEM_FROZEN_EMPIRE:
          // No-op for this theme, as this is handled in startWandMashLockout
        break;
        default:
          // Plays the alarm loop as heard on the wand.
          stopMashErrorSounds();
          playEffect(S_SMASH_ERROR_LOOP, true, i_volume_effects, true, 2500);
        break;
      }
    break;

    case A_CANCEL_LOCKOUT:
      // Initiate a restart of the pack after a lockout event has occurred.
      restartFromWandMash();
    break;

    case A_TOGGLE_SMOKE:
      b_smoke_enabled = !b_smoke_enabled;
      attenuatorSerialSend(A_TOGGLE_SMOKE, b_smoke_enabled ? 2 : 1);
    break;

    case A_TOGGLE_VIBRATION:
      b_vibration_switch_on = !b_vibration_switch_on;
      attenuatorSerialSend(A_TOGGLE_VIBRATION, b_vibration_switch_on ? 2 : 1);
    break;

    case A_CYCLOTRON_DIRECTION_TOGGLE:
      b_clockwise = !b_clockwise;
      attenuatorSerialSend(A_CYCLOTRON_DIRECTION_TOGGLE, b_clockwise ? 2 : 1);
    break;

    case A_TOGGLE_MUTE:
      if(i_volume_master == i_volume_abs_min) {
        i_volume_master = i_volume_revert;

        attenuatorSerialSend(A_TOGGLE_MUTE, 1);
        packSerialSend(P_MASTER_AUDIO_STATUS, 1);
      }
      else {
        i_volume_revert = i_volume_master;

        // Set the master volume to minimum.
        i_volume_master = i_volume_abs_min;

        attenuatorSerialSend(A_TOGGLE_MUTE, 2);
        packSerialSend(P_MASTER_AUDIO_STATUS, 2);
      }

      updateMasterVolume();
    break;

    case A_VOLUME_DECREASE:
      // Decrease overall pack volume.
      decreaseVolume();
    break;

    case A_VOLUME_INCREASE:
      // Increase overall pack volume.
      increaseVolume();
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
      toggleMusicLoop();
      attenuatorSerialSend(A_MUSIC_TRACK_LOOP_TOGGLE, b_repeat_track ? 2 : 1);
      packSerialSend(P_MUSIC_LOOP_STATUS, b_repeat_track ? 2 : 1);
    break;

    case A_YEAR_1984:
      SYSTEM_YEAR = SYSTEM_1984;
      SYSTEM_YEAR_TEMP = SYSTEM_YEAR;
      b_switch_mode_override = true; // Explicit mode set, override mode toggle.
      packSerialSend(P_YEAR_1984);
      attenuatorSerialSend(A_YEAR_1984);
      playEffect(S_VOICE_1984);
      resetRampSpeeds();
      packOffReset();
      debugln("Theme changed to GB1 (1984)");
    break;

    case A_YEAR_1989:
      SYSTEM_YEAR = SYSTEM_1989;
      SYSTEM_YEAR_TEMP = SYSTEM_YEAR;
      b_switch_mode_override = true; // Explicit mode set, override mode toggle.
      packSerialSend(P_YEAR_1989);
      attenuatorSerialSend(A_YEAR_1989);
      playEffect(S_VOICE_1989);
      resetRampSpeeds();
      packOffReset();
      debugln("Theme changed to GB2 (1989)");
    break;

    case A_YEAR_AFTERLIFE:
      SYSTEM_YEAR = SYSTEM_AFTERLIFE;
      SYSTEM_YEAR_TEMP = SYSTEM_YEAR;
      b_switch_mode_override = true; // Explicit mode set, override mode toggle.
      packSerialSend(P_YEAR_AFTERLIFE);
      attenuatorSerialSend(A_YEAR_AFTERLIFE);
      playEffect(S_VOICE_AFTERLIFE);
      resetRampSpeeds();
      packOffReset();
      debugln("Theme changed to Afterlife (2021)");
    break;

    case A_YEAR_FROZEN_EMPIRE:
      SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
      SYSTEM_YEAR_TEMP = SYSTEM_YEAR;
      b_switch_mode_override = true; // Explicit mode set, override mode toggle.
      packSerialSend(P_YEAR_FROZEN_EMPIRE);
      attenuatorSerialSend(A_YEAR_FROZEN_EMPIRE);
      playEffect(S_VOICE_FROZEN_EMPIRE);
      resetRampSpeeds();
      packOffReset();
      debugln("Theme changed to Frozen Empire (2024)");
    break;

    case A_PROTON_MODE:
      if((AUDIO_DEVICE == A_GPSTAR_AUDIO || AUDIO_DEVICE == A_GPSTAR_AUDIO_ADV) && STREAM_MODE == MESON) {
        // Tell GPStar Audio we no longer need short audio.
        audio.gpstarShortTrackOverload(true);
      }

      // Returning from Slime mode, so we need to reset the Cyclotron again.
      if(usingSlimeCyclotron()) {
        resetCyclotronState();
        clearCyclotronFades();

        if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE)) {
          adjustGainEffect(S_AFTERLIFE_PACK_STARTUP, i_volume_effects, true, 100);
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects, true, 100);
        }
      }

      if(PACK_STATE == MODE_ON && STREAM_MODE != PROTON) {
        stopEffect(S_PACK_SLIME_TANK_LOOP);
        stopEffect(S_STASIS_IDLE_LOOP);
        stopEffect(S_MESON_IDLE_LOOP);

        playEffect(S_FIRE_START_SPARK);
      }

      // Proton mode.
      STREAM_MODE = PROTON;

      if(b_settings) {
        playEffect(S_CLICK);
        b_settings = false;
      }

      if(b_cyclotron_colour_toggle) {
        // Reset the Cyclotron LED colours.
        cyclotronColourReset();
      }

      if(b_powercell_colour_toggle && b_pack_on) {
        // Reset the Power Cell colours if the Power Cell is running.
        b_powercell_updating = true;
        powercellDraw();
      }

      // Update the Inner Cyclotron LEDs if required.
      cyclotronSwitchLEDUpdate();

      packSerialSend(P_SET_STREAM_MODE, 1);
      attenuatorSerialSend(A_PROTON_MODE);
    break;

    case A_STASIS_MODE:
      if((AUDIO_DEVICE == A_GPSTAR_AUDIO || AUDIO_DEVICE == A_GPSTAR_AUDIO_ADV) && STREAM_MODE == MESON) {
        // Tell GPStar Audio we no longer need short audio.
        audio.gpstarShortTrackOverload(true);
      }

      // Returning from Slime mode, so we need to reset the Cyclotron again.
      if(usingSlimeCyclotron()) {
        resetCyclotronState();
        clearCyclotronFades();

        if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE)) {
          adjustGainEffect(S_AFTERLIFE_PACK_STARTUP, i_volume_effects, true, 100);
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects, true, 100);
        }
      }

      if(PACK_STATE == MODE_ON && STREAM_MODE != STASIS) {
        stopEffect(S_PACK_SLIME_TANK_LOOP);
        stopEffect(S_STASIS_IDLE_LOOP);
        stopEffect(S_MESON_IDLE_LOOP);

        playEffect(S_STASIS_OPEN);
        playEffect(S_STASIS_IDLE_LOOP, true, i_volume_effects, true, 2000);
      }

      // Stasis mode.
      STREAM_MODE = STASIS;

      if(b_settings) {
        playEffect(S_CLICK);
        b_settings = false;
      }

      if(b_cyclotron_colour_toggle) {
        // Reset the Cyclotron LED colours.
        cyclotronColourReset();
      }

      if(b_powercell_colour_toggle && b_pack_on) {
        // Reset the Power Cell colours if the Power Cell is running.
        b_powercell_updating = true;
        powercellDraw();
      }

      // Update the Inner Cyclotron LEDs if required.
      cyclotronSwitchLEDUpdate();

      packSerialSend(P_SET_STREAM_MODE, 2);
      attenuatorSerialSend(A_STASIS_MODE);
    break;

    case A_SLIME_MODE:
      if((AUDIO_DEVICE == A_GPSTAR_AUDIO || AUDIO_DEVICE == A_GPSTAR_AUDIO_ADV) && STREAM_MODE == MESON) {
        // Tell GPStar Audio we no longer need short audio.
        audio.gpstarShortTrackOverload(true);
      }

      if(PACK_STATE == MODE_ON && STREAM_MODE != SLIME) {
        stopEffect(S_PACK_SLIME_TANK_LOOP);
        stopEffect(S_STASIS_IDLE_LOOP);
        stopEffect(S_MESON_IDLE_LOOP);

        playEffect(S_PACK_SLIME_OPEN);
        playEffect(S_PACK_SLIME_TANK_LOOP, true, i_volume_effects, true, 700);

        if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE)) {
          adjustGainEffect(S_AFTERLIFE_PACK_STARTUP, i_volume_effects - 30, true, 100);
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - 40, true, 100);
        }
      }

      // Slime mode.
      STREAM_MODE = SLIME;

      if(b_settings) {
        playEffect(S_CLICK);
        b_settings = false;
      }

      if(b_cyclotron_colour_toggle) {
        // Reset the Cyclotron states.
        resetCyclotronState();
        clearCyclotronFades();

        // Reset the Cyclotron LED colours.
        cyclotronColourReset();
      }

      if(b_powercell_colour_toggle && b_pack_on) {
        // Reset the Power Cell colours if the Power Cell is running.
        b_powercell_updating = true;
        powercellDraw();
      }

      // Update the Inner Cyclotron LEDs if required.
      cyclotronSwitchLEDUpdate();

      packSerialSend(P_SET_STREAM_MODE, 3);
      attenuatorSerialSend(A_SLIME_MODE);
    break;

    case A_MESON_MODE:
      // Returning from Slime mode, so we need to reset the Cyclotron again.
      if(usingSlimeCyclotron()) {
        resetCyclotronState();
        clearCyclotronFades();

        if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE)) {
          adjustGainEffect(S_AFTERLIFE_PACK_STARTUP, i_volume_effects, true, 100);
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects, true, 100);
        }
      }

      if(PACK_STATE == MODE_ON && STREAM_MODE != MESON) {
        stopEffect(S_PACK_SLIME_TANK_LOOP);
        stopEffect(S_STASIS_IDLE_LOOP);
        stopEffect(S_MESON_IDLE_LOOP);

        playEffect(S_MESON_OPEN);
        playEffect(S_MESON_IDLE_LOOP, true, i_volume_effects, true, 1250);
      }

      // Meson mode.
      STREAM_MODE = MESON;

      if(b_settings) {
        playEffect(S_CLICK);
        b_settings = false;
      }

      if(AUDIO_DEVICE == A_GPSTAR_AUDIO || AUDIO_DEVICE == A_GPSTAR_AUDIO_ADV) {
        // Tell GPStar Audio we need short audio mode.
        audio.gpstarShortTrackOverload(false);
      }

      if(b_cyclotron_colour_toggle) {
        // Reset the Cyclotron LED colours.
        cyclotronColourReset();
      }

      if(b_powercell_colour_toggle && b_pack_on) {
        // Reset the Power Cell colours if the Power Cell is running.
        b_powercell_updating = true;
        powercellDraw();
      }

      // Update the Inner Cyclotron LEDs if required.
      cyclotronSwitchLEDUpdate();

      packSerialSend(P_SET_STREAM_MODE, 4);
      attenuatorSerialSend(A_MESON_MODE);
    break;

    case A_SPECTRAL_MODE:
      if((AUDIO_DEVICE == A_GPSTAR_AUDIO || AUDIO_DEVICE == A_GPSTAR_AUDIO_ADV) && STREAM_MODE == MESON) {
        // Tell GPStar Audio we no longer need short audio.
        audio.gpstarShortTrackOverload(true);
      }

      // Returning from Slime mode, so we need to reset the Cyclotron again.
      if(usingSlimeCyclotron()) {
        resetCyclotronState();
        clearCyclotronFades();

        if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE)) {
          adjustGainEffect(S_AFTERLIFE_PACK_STARTUP, i_volume_effects, true, 100);
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects, true, 100);
        }
      }

      if(PACK_STATE == MODE_ON && STREAM_MODE != SPECTRAL) {
        stopEffect(S_PACK_SLIME_TANK_LOOP);
        stopEffect(S_STASIS_IDLE_LOOP);
        stopEffect(S_MESON_IDLE_LOOP);

        playEffect(S_FIRE_START_SPARK);
      }

      // Spectral mode.
      STREAM_MODE = SPECTRAL;

      if(b_settings) {
        playEffect(S_CLICK);
        b_settings = false;
      }

      if(b_cyclotron_colour_toggle) {
        // Reset the Cyclotron LED colours.
        cyclotronColourReset();
      }

      if(b_powercell_colour_toggle && b_pack_on) {
        // Reset the Power Cell colours if the Power Cell is running.
        b_powercell_updating = true;
        powercellDraw();
      }

      // Update the Inner Cyclotron LEDs if required.
      cyclotronSwitchLEDUpdate();

      packSerialSend(P_SET_STREAM_MODE, 5);
      attenuatorSerialSend(A_SPECTRAL_MODE);
    break;

    case A_HALLOWEEN_MODE:
      if((AUDIO_DEVICE == A_GPSTAR_AUDIO || AUDIO_DEVICE == A_GPSTAR_AUDIO_ADV) && STREAM_MODE == MESON) {
        // Tell GPStar Audio we no longer need short audio.
        audio.gpstarShortTrackOverload(true);
      }

      // Returning from Slime mode, so we need to reset the Cyclotron again.
      if(usingSlimeCyclotron()) {
        resetCyclotronState();
        clearCyclotronFades();

        if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE)) {
          adjustGainEffect(S_AFTERLIFE_PACK_STARTUP, i_volume_effects, true, 100);
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects, true, 100);
        }
      }

      if(PACK_STATE == MODE_ON && (STREAM_MODE != HOLIDAY_HALLOWEEN || STREAM_MODE != HOLIDAY_CHRISTMAS)) {
        stopEffect(S_PACK_SLIME_TANK_LOOP);
        stopEffect(S_STASIS_IDLE_LOOP);
        stopEffect(S_MESON_IDLE_LOOP);

        playEffect(S_HALLOWEEN_MODE_VOICE);
      }

      // Set appropriate holiday mode.
      STREAM_MODE = HOLIDAY_HALLOWEEN;

      if(b_settings) {
        playEffect(S_CLICK);
        b_settings = false;
      }

      if(b_cyclotron_colour_toggle) {
        // Reset the Cyclotron LED colours.
        cyclotronColourReset();
      }

      if(b_powercell_colour_toggle && b_pack_on) {
        // Reset the Power Cell colours if the Power Cell is running.
        b_powercell_updating = true;
        powercellDraw();
      }

      // Update the Inner Cyclotron LEDs if required.
      cyclotronSwitchLEDUpdate();

      packSerialSend(P_SET_STREAM_MODE, 6);
      attenuatorSerialSend(A_HALLOWEEN_MODE);
    break;

    case A_CHRISTMAS_MODE:
      if((AUDIO_DEVICE == A_GPSTAR_AUDIO || AUDIO_DEVICE == A_GPSTAR_AUDIO_ADV) && STREAM_MODE == MESON) {
        // Tell GPStar Audio we no longer need short audio.
        audio.gpstarShortTrackOverload(true);
      }

      // Returning from Slime mode, so we need to reset the Cyclotron again.
      if(usingSlimeCyclotron()) {
        resetCyclotronState();
        clearCyclotronFades();

        if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE)) {
          adjustGainEffect(S_AFTERLIFE_PACK_STARTUP, i_volume_effects, true, 100);
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects, true, 100);
        }
      }

      if(PACK_STATE == MODE_ON && (STREAM_MODE != HOLIDAY_HALLOWEEN || STREAM_MODE != HOLIDAY_CHRISTMAS)) {
        stopEffect(S_PACK_SLIME_TANK_LOOP);
        stopEffect(S_STASIS_IDLE_LOOP);
        stopEffect(S_MESON_IDLE_LOOP);

        playEffect(S_CHRISTMAS_MODE_VOICE);
      }

      // Set appropriate holiday mode.
      STREAM_MODE = HOLIDAY_CHRISTMAS;

      if(b_settings) {
        playEffect(S_CLICK);
        b_settings = false;
      }

      if(b_cyclotron_colour_toggle) {
        // Reset the Cyclotron LED colours.
        cyclotronColourReset();
      }

      if(b_powercell_colour_toggle && b_pack_on) {
        // Reset the Power Cell colours if the Power Cell is running.
        b_powercell_updating = true;
        powercellDraw();
      }

      // Update the Inner Cyclotron LEDs if required.
      cyclotronSwitchLEDUpdate();

      packSerialSend(P_SET_STREAM_MODE, 7);
      attenuatorSerialSend(A_CHRISTMAS_MODE);
    break;

    case A_SPECTRAL_CUSTOM_MODE:
      if((AUDIO_DEVICE == A_GPSTAR_AUDIO || AUDIO_DEVICE == A_GPSTAR_AUDIO_ADV) && STREAM_MODE == MESON) {
        // Tell GPStar Audio we no longer need short audio.
        audio.gpstarShortTrackOverload(true);
      }

      // Returning from Slime mode, so we need to reset the Cyclotron again.
      if(usingSlimeCyclotron()) {
        resetCyclotronState();
        clearCyclotronFades();

        if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE)) {
          adjustGainEffect(S_AFTERLIFE_PACK_STARTUP, i_volume_effects, true, 100);
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects, true, 100);
        }
      }

      if(PACK_STATE == MODE_ON && STREAM_MODE != SPECTRAL_CUSTOM) {
        stopEffect(S_PACK_SLIME_TANK_LOOP);
        stopEffect(S_STASIS_IDLE_LOOP);
        stopEffect(S_MESON_IDLE_LOOP);

        playEffect(S_FIRE_START_SPARK);
      }

      // Custom spectral mode.
      STREAM_MODE = SPECTRAL_CUSTOM;

      if(b_settings) {
        playEffect(S_CLICK);
        b_settings = false;
      }

      if(b_cyclotron_colour_toggle) {
        // Reset the Cyclotron LED colours.
        cyclotronColourReset();
      }

      if(b_powercell_colour_toggle && b_pack_on) {
        // Reset the Power Cell colours if the Power Cell is running.
        b_powercell_updating = true;
        powercellDraw();
      }

      // Update the Inner Cyclotron LEDs if required.
      cyclotronSwitchLEDUpdate();

      packSerialSend(P_SET_STREAM_MODE, 8);
      attenuatorSendData(A_SPECTRAL_CUSTOM_MODE);
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

    default:
      // No-op for anything else.
    break;
  }
}
