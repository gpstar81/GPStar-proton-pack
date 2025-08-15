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

// Forward function declaration.
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

    case A_TOGGLE_MUTE:
      if(i_volume_master == i_volume_abs_min) {
        i_volume_master = i_volume_revert;

        packSerialSend(P_MASTER_AUDIO_NORMAL);
        attenuatorSerialSend(A_TOGGLE_MUTE, 1);
      }
      else {
        i_volume_revert = i_volume_master;

        // Set the master volume to minimum.
        i_volume_master = i_volume_abs_min;

        packSerialSend(P_MASTER_AUDIO_SILENT_MODE);
        attenuatorSerialSend(A_TOGGLE_MUTE, 2);
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
        if(i_music_track_count > 0 && i_current_music_track >= i_music_track_start) {
          // Play the appropriate track on pack and wand, and notify the Attenuator.
          playMusic();
        }
      }
    break;

    case A_MUSIC_PAUSE_RESUME:
      if(b_playing_music) {
        if(!b_music_paused) {
          pauseMusic();
        }
        else {
          resumeMusic();
        }
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
    break;

    case A_REQUEST_PREFERENCES_PACK:
      // If requested by the serial device, send back all pack EEPROM preferences.
      // This will send a data payload directly from the pack as all data is local.
      attenuatorSendData(A_SEND_PREFERENCES_PACK);
    break;

    case A_REQUEST_PREFERENCES_WAND:
      // If requested by the serial device, tell the wand we need its EEPROM preferences.
      // This is merely a command to the wand which tells it to send back a data payload.
      if(b_wand_connected) {
        packSerialSend(P_SEND_PREFERENCES_WAND);
      }
    break;

    case A_REQUEST_PREFERENCES_SMOKE:
      if(b_wand_connected) {
        // If requested by the serial device, tell the wand we need its EEPROM preferences.
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
