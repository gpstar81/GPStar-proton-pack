/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
 * Function prototypes.
 */
void updateSystemModeYear();

// For command signals (2 byte ID, 2 byte optional data).
struct __attribute__((packed)) CommandPacket {
  uint16_t c;
  uint16_t d1; // Reserved for values over 255 (eg. current music track)
};

// For data communication (2 byte ID, 24 byte data payload).
struct __attribute__((packed)) MessagePacket {
  uint16_t m;
  uint8_t d[23]; // Reserved for large data packets (eg. EEPROM configs)
};

struct CommandPacket recvCmdW;
struct CommandPacket sendCmdW;
struct CommandPacket sendCmdS;
struct CommandPacket recvCmdS;

struct MessagePacket recvDataW;
struct MessagePacket sendDataW;
struct MessagePacket sendDataS;
struct MessagePacket recvDataS;

struct WandPrefs {
  uint8_t ledWandCount;
  uint8_t ledWandHue;
  uint8_t ledWandSat;
  uint8_t spectralModeEnabled;
  uint8_t spectralHolidayMode;
  uint8_t overheatEnabled;
  uint8_t defaultFiringModeCTS;
  uint8_t defaultFiringModeMix;
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
} wandConfig;

// Adjusts which year mode the Proton Pack and Neutrona Wand are in, when switched by the Neutrona Wand.
void toggleYearModes() {
  // Toggle between the year modes.
  stopEffect(S_BEEPS_BARGRAPH);
  playEffect(S_BEEPS_BARGRAPH);

  // Cycles through year modes: 1984 -> 1989 -> Afterlife -> Frozen Empire

  switch(SYSTEM_YEAR_TEMP) {
    case SYSTEM_1984:
      SYSTEM_YEAR_TEMP = SYSTEM_1989;

      stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1989);
      stopEffect(S_VOICE_1984);

      playEffect(S_VOICE_1989);

      // Tell the wand to play the 1989 sound effects.
      packSerialSend(P_MODE_1989);
    break;

    case SYSTEM_1989:
      SYSTEM_YEAR_TEMP = SYSTEM_AFTERLIFE;

      stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1989);
      stopEffect(S_VOICE_1984);

      playEffect(S_VOICE_AFTERLIFE);

      // Tell the wand to play the Afterlife sound effects.
      packSerialSend(P_MODE_AFTERLIFE);
    break;

    case SYSTEM_AFTERLIFE:
      SYSTEM_YEAR_TEMP = SYSTEM_FROZEN_EMPIRE;

      stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1989);
      stopEffect(S_VOICE_1984);

      playEffect(S_VOICE_FROZEN_EMPIRE);

      // Tell the wand to play the Frozen Empire sound effects.
      packSerialSend(P_MODE_FROZEN_EMPIRE);
    break;

    case SYSTEM_FROZEN_EMPIRE:
      SYSTEM_YEAR_TEMP = SYSTEM_1984;

      stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1989);
      stopEffect(S_VOICE_1984);

      playEffect(S_VOICE_1984);

      // Tell the wand to play the 1984 sound effects.
      packSerialSend(P_MODE_1984);
    break;

    default:
      // Nothing.
    break;
  }
}

// Handle telling connected devices the proper mode/year in use.
void updateSystemModeYear() {
  switch(SYSTEM_MODE) {
    case MODE_ORIGINAL:
      packSerialSend(P_MODE_ORIGINAL);
      serial1Send(A_MODE_ORIGINAL);
    break;

    case MODE_SUPER_HERO:
    default:
      packSerialSend(P_MODE_SUPER_HERO);
      serial1Send(A_MODE_ORIGINAL);
    break;
  }

  switch(SYSTEM_YEAR) {
    case SYSTEM_1984:
      packSerialSend(P_YEAR_1984);
      serial1Send(A_YEAR_1984);
    break;
    case SYSTEM_1989:
      packSerialSend(P_YEAR_1989);
      serial1Send(A_YEAR_1989);
    break;
    case SYSTEM_AFTERLIFE:
    default:
      packSerialSend(P_YEAR_AFTERLIFE);
      serial1Send(A_YEAR_AFTERLIFE);
    break;
    case SYSTEM_FROZEN_EMPIRE:
      packSerialSend(P_YEAR_FROZEN_EMPIRE);
      serial1Send(A_YEAR_FROZEN_EMPIRE);
    break;
  }
}

// Outgoing commands to the Serial1 device
void serial1Send(uint16_t i_command, uint16_t i_value) {
  uint16_t i_send_size = 0;

  sendCmdS.c = i_command;
  sendCmdS.d1 = i_value;

  i_send_size = serial1Coms.txObj(sendCmdS, i_send_size);
  serial1Coms.sendData(i_send_size, 1);
}
// Override function to handle calls with a single parameter.
void serial1Send(uint16_t i_command) {
  serial1Send(i_command, 0);
}

// Outgoing payloads to the Serial1 device
void serial1SendData(uint16_t i_message) {
  uint16_t i_send_size = 0;

  sendDataS.m = i_message;

  // Set all elements of the data array to 0
  memset(sendDataW.d, 0, sizeof(sendDataW.d));

  // Provide additional data with certain messages.
  switch(i_message) {
    case A_SPECTRAL_CUSTOM_MODE:
    case A_SPECTRAL_COLOUR_DATA:
      sendDataS.d[0] = i_spectral_cyclotron_custom_colour;
      sendDataS.d[1] = i_spectral_cyclotron_custom_saturation;
    break;

    case A_VOLUME_SYNC:
      // Send the current volume levels.
      sendDataS.d[0] = i_volume_master_percentage;
      sendDataS.d[1] = i_volume_effects_percentage;
      sendDataS.d[2] = i_volume_music_percentage;
    break;

    case A_SEND_PREFERENCES_PACK:
      // Sends values from current runtime variables as values in an int array.
      // Any ENUM or boolean types will simply translate as numeric values.
      sendDataS.d[0] = SYSTEM_MODE;
      sendDataS.d[1] = SYSTEM_YEAR;
      sendDataS.d[2] = i_volume_master_percentage;
      sendDataS.d[3] = b_stream_effects;
      sendDataS.d[4] = b_overheat_strobe;
      sendDataS.d[5] = b_overheat_lights_off;
      sendDataS.d[6] = b_overheat_sync_to_fan;
      sendDataS.d[7] = b_demo_light_mode;

      // Cyclotron Lid
      sendDataS.d[8] = i_cyclotron_leds;
      sendDataS.d[9] = i_spectral_cyclotron_custom_colour;
      sendDataS.d[10] = i_spectral_cyclotron_custom_saturation;
      sendDataS.d[11] = b_clockwise; // Cyclotron Direction
      sendDataS.d[12] = b_cyclotron_single_led;
      sendDataS.d[13] = b_cyclotron_colour_toggle;
      sendDataS.d[14] = b_cyclotron_simulate_ring;

      // Inner Cyclotron
      sendDataS.d[15] = i_inner_cyclotron_num_leds;
      sendDataS.d[16] = i_spectral_cyclotron_inner_custom_colour;
      sendDataS.d[17] = i_spectral_cyclotron_inner_custom_saturation;
      sendDataS.d[18] = b_grb_cyclotron;

      // Power Cell
      sendDataS.d[19] = i_powercell_leds;
      sendDataS.d[20] = i_spectral_powercell_custom_colour;
      sendDataS.d[21] = i_spectral_powercell_custom_saturation;
      sendDataS.d[22] = b_powercell_colour_toggle;
    break;

    case A_SEND_PREFERENCES_WAND:
      // Sends values from current runtime variables as values in an int array.
      // Any ENUM or boolean types will simply translate as numeric values.
      sendDataS.d[0] = wandConfig.ledWandCount;
      sendDataS.d[1] = wandConfig.ledWandHue;
      sendDataS.d[2] = wandConfig.ledWandSat;
      sendDataS.d[3] = wandConfig.spectralModeEnabled;
      sendDataS.d[4] = wandConfig.spectralHolidayMode;
      sendDataS.d[5] = wandConfig.overheatEnabled;
      sendDataS.d[6] = wandConfig.defaultFiringModeCTS;
      sendDataS.d[7] = wandConfig.defaultFiringModeMix;
      sendDataS.d[8] = wandConfig.wandSoundsToPack;
      sendDataS.d[9] = wandConfig.quickVenting;
      sendDataS.d[10] = wandConfig.autoVentLight;
      sendDataS.d[11] = wandConfig.wandBeepLoop;
      sendDataS.d[12] = wandConfig.wandBootError;
      sendDataS.d[13] = wandConfig.defaultYearModeWand;
      sendDataS.d[14] = wandConfig.defaultYearModeCTS;
      sendDataS.d[15] = wandConfig.invertWandBargraph;
      sendDataS.d[16] = wandConfig.bargraphOverheatBlink;
      sendDataS.d[17] = wandConfig.bargraphIdleAnimation;
      sendDataS.d[18] = wandConfig.bargraphFireAnimation;
    break;

    case A_SEND_PREFERENCES_SMOKE:
      // Sends values from current runtime variables as values in an int array.
      // Duration (in seconds) an overheat event persists once activated.
      sendDataS.d[0] = i_ms_overheating_length_5 / 1000;
      sendDataS.d[1] = i_ms_overheating_length_4 / 1000;
      sendDataS.d[2] = i_ms_overheating_length_3 / 1000;
      sendDataS.d[3] = i_ms_overheating_length_2 / 1000;
      sendDataS.d[4] = i_ms_overheating_length_1 / 1000;

      // Determines whether smoke effects while firing is enabled by power level.
      sendDataS.d[5] = b_smoke_continuous_mode_5;
      sendDataS.d[6] = b_smoke_continuous_mode_4;
      sendDataS.d[7] = b_smoke_continuous_mode_3;
      sendDataS.d[8] = b_smoke_continuous_mode_2;
      sendDataS.d[9] = b_smoke_continuous_mode_1;

      // Determines whether overheating is enabled for a power level.
      sendDataS.d[10] = wandConfig.overheatLevel5;
      sendDataS.d[11] = wandConfig.overheatLevel4;
      sendDataS.d[12] = wandConfig.overheatLevel3;
      sendDataS.d[13] = wandConfig.overheatLevel2;
      sendDataS.d[14] = wandConfig.overheatLevel1;

      // Time (seconds) before an overheat event is triggered by level.
      sendDataS.d[15] = wandConfig.overheatDelay5;
      sendDataS.d[16] = wandConfig.overheatDelay4;
      sendDataS.d[17] = wandConfig.overheatDelay3;
      sendDataS.d[18] = wandConfig.overheatDelay2;
      sendDataS.d[19] = wandConfig.overheatDelay1;

      // Enable or disable smoke effects overall.
      sendDataS.d[20] = b_smoke_enabled;
    break;

    default:
      // No-op for all other communications.
    break;
  }

  i_send_size = serial1Coms.txObj(sendDataS, i_send_size);
  serial1Coms.sendData(i_send_size, 2);
}

// Outgoing commands to the wand
void packSerialSend(uint16_t i_command, uint16_t i_value) {
  uint16_t i_send_size = 0;

  sendCmdW.c = i_command;
  sendCmdW.d1 = i_value;

  i_send_size = serial1Coms.txObj(sendCmdW, i_send_size);
  serial1Coms.sendData(i_send_size, 1);
}
// Override function to handle calls with a single parameter.
void packSerialSend(uint16_t i_command) {
  packSerialSend(i_command, 0);
}

// Outgoing payloads to the wand
void packSerialSendData(uint16_t i_message) {
  uint16_t i_send_size = 0;

  sendDataW.m = i_message;

  // Set all elements of the data array to 0
  memset(sendDataW.d, 0, sizeof(sendDataW.d));

  // Provide additional data with certain messages.
  switch(i_message) {
    case P_VOLUME_SYNC:
      // Send the current volume levels.
      sendDataW.d[0] = i_volume_master_percentage;
      sendDataW.d[1] = i_volume_effects_percentage;
      sendDataW.d[2] = i_volume_music_percentage;
    break;

    case P_SAVE_PREFERENCES_WAND:
      // Sends values from current runtime variables as values in an int array.
      // Any ENUM or boolean types will simply translate as numeric values.
      sendDataW.d[0] = wandConfig.ledWandCount;
      sendDataW.d[1] = wandConfig.ledWandHue;
      sendDataW.d[2] = wandConfig.ledWandSat;
      sendDataW.d[3] = wandConfig.spectralModeEnabled;
      sendDataW.d[4] = wandConfig.spectralHolidayMode;
      sendDataW.d[5] = wandConfig.overheatEnabled;
      sendDataW.d[6] = wandConfig.defaultFiringModeCTS;
      sendDataW.d[7] = wandConfig.defaultFiringModeMix;
      sendDataW.d[8] = wandConfig.wandSoundsToPack;
      sendDataW.d[9] = wandConfig.quickVenting;
      sendDataW.d[10] = wandConfig.autoVentLight;
      sendDataW.d[11] = wandConfig.wandBeepLoop;
      sendDataW.d[12] = wandConfig.wandBootError;
      sendDataW.d[13] = wandConfig.defaultYearModeWand;
      sendDataW.d[14] = wandConfig.defaultYearModeCTS;
      sendDataW.d[16] = wandConfig.invertWandBargraph;
      sendDataW.d[16] = wandConfig.bargraphOverheatBlink;
      sendDataW.d[17] = wandConfig.bargraphIdleAnimation;
      sendDataW.d[18] = wandConfig.bargraphFireAnimation;
    break;

    case P_SAVE_PREFERENCES_SMOKE:
      // Sends values from current runtime variables as values in an int array.
      sendDataW.d[0] = wandConfig.overheatLevel5;
      sendDataW.d[1] = wandConfig.overheatLevel4;
      sendDataW.d[2] = wandConfig.overheatLevel3;
      sendDataW.d[3] = wandConfig.overheatLevel2;
      sendDataW.d[4] = wandConfig.overheatLevel1;
      sendDataW.d[5] = wandConfig.overheatDelay5;
      sendDataW.d[6] = wandConfig.overheatDelay4;
      sendDataW.d[7] = wandConfig.overheatDelay3;
      sendDataW.d[8] = wandConfig.overheatDelay2;
      sendDataW.d[9] = wandConfig.overheatDelay1;
    break;

    default:
      // No-op for all other communications.
    break;
  }

  i_send_size = serial1Coms.txObj(sendDataW, i_send_size);
  serial1Coms.sendData(i_send_size, 2);
}

// Incoming messages from the extra Serial 1 port.
void checkSerial1() {
  if(serial1Coms.available() > 0) {
    uint8_t i_packet_id = serial1Coms.currentPacketID();
    Serial.println("Serial PacketID: " + String(i_packet_id));

    if(i_packet_id > 0) {
      switch(i_packet_id) {
        case 1:
          serial1Coms.rxObj(recvCmdS);
          Serial.println("Recv. Serial Command: " + String(recvCmdS.c));
        break;
        case 2:
          serial1Coms.rxObj(recvDataS);
          Serial.println("Recv. Serial Message: " + String(recvDataS.m));
        break;
      }

      // Perform checks based on whether a serial1 device is connected or not.
      if(b_serial1_connected == true) {
        // Handle simple commands.
        if(i_packet_id == 1) {
          switch(recvCmdS.c) {
            case A_HANDSHAKE:
              // The Attenuator is still here.
              ms_serial1_handshake.start(i_serial1_handshake_delay);
              ms_serial1_handshake_checking.start(i_serial1_handshake_delay / 2);
            break;

            case A_TURN_PACK_ON:
              // Turn the pack on.
              if(PACK_STATE != MODE_ON) {
                PACK_ACTION_STATE = ACTION_ACTIVATE;
              }
            break;

            case A_TURN_PACK_OFF:
              // Turn the pack off.
              if(PACK_STATE != MODE_OFF) {
                PACK_ACTION_STATE = ACTION_OFF;
              }
            break;

            case A_WARNING_CANCELLED:
              // Tell wand to reset overheat warning.
              packSerialSend(P_WARNING_CANCELLED);
            break;

            case A_MANUAL_OVERHEAT:
              // Trigger a manual overheat vent.
              packSerialSend(P_MANUAL_OVERHEAT);
            break;

            case A_TOGGLE_MUTE:
              if(i_volume_master == i_volume_abs_min) {
                i_volume_master = i_volume_revert;

                w_trig.masterGain(i_volume_master); // Reset the master gain.

                packSerialSend(P_MASTER_AUDIO_NORMAL);
              }
              else {
                i_volume_revert = i_volume_master;

                // Set the master volume to silent.
                i_volume_master = i_volume_abs_min;

                w_trig.masterGain(i_volume_master); // Reset the master gain.

                packSerialSend(P_MASTER_AUDIO_SILENT_MODE);
              }
            break;

            case A_VOLUME_DECREASE:
              // Decrease overall pack volume.
              decreaseVolume();

              // Tell wand to decrease volume.
              packSerialSend(P_VOLUME_DECREASE);
            break;

            case A_VOLUME_INCREASE:
              // Increase overall pack volume.
              increaseVolume();

              // Tell wand to increase volume.
              packSerialSend(P_VOLUME_INCREASE);
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

            case A_MUSIC_START_STOP:
              if(b_playing_music == true) {
                stopMusic();
              }
              else {
                if(i_music_count > 0 && i_current_music_track >= i_music_track_start) {
                  // Play the appropriate track on pack and wand, and notify the serial1 device.
                  playMusic();
                }
              }
            break;

            case A_MUSIC_PAUSE_RESUME:
              if(b_playing_music == true) {
                if(b_music_paused != true) {
                  pauseMusic();
                }
                else {
                  if(i_music_count > 0 && i_current_music_track >= i_music_track_start) {
                    resumeMusic();
                  }
                }
              }
            break;

            case A_MUSIC_NEXT_TRACK:
              musicNextTrack();
            break;

            case A_MUSIC_PREV_TRACK:
              musicPrevTrack();
            break;

            case A_REQUEST_PREFERENCES_PACK:
              // If requested by the serial device, send back all pack EEPROM preferences.
              serial1SendData(A_SEND_PREFERENCES_PACK);
            break;

            case A_REQUEST_PREFERENCES_WAND:
              // If requested by the serial device, tell the wand we need EEPROM preferences.
              serial1SendData(P_SEND_PREFERENCES_WAND);
            break;

            case A_REQUEST_PREFERENCES_SMOKE:
              // If requested by the serial device, tell the wand we need EEPROM preferences.
              serial1SendData(P_SEND_PREFERENCES_SMOKE);
            break;

            case A_SYNC_START:
              //Serial.println("Serial1 Sync Start");
            break;

            case A_MUSIC_PLAY_TRACK:
              // Music track number to be played.
              if(i_music_count > 0 && recvCmdS.d1 >= i_music_track_start) {
                if(b_playing_music == true) {
                  stopMusic(); // Stops current track before change.

                  // Only update after the music is stopped.
                  i_current_music_track = recvCmdS.d1;

                  // Play the appropriate track on pack and wand, and notify the serial1 device.
                  playMusic();
                }
                else {
                  i_current_music_track = recvCmdS.d1;

                  // Just tell the wand which track was requested for play.
                  packSerialSend(P_MUSIC_PLAY_TRACK, i_current_music_track);
                }
              }
            break;

            case A_SAVE_EEPROM_SETTINGS_PACK:
              // Commit changes to the EEPROM in the pack controller
              saveLedEEPROM();
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

            case A_SYNC_END:
              //Serial.println("Serial1 Sync End");
            break;

            default:
              // No-op for anything else.
            break;
          }
        }

        // Handle data payloads.
        if(i_packet_id == 2) {
          switch(recvDataS.m) {
            case A_SAVE_PREFERENCES_PACK:
              // Writes new preferences back to runtime variables.
              // This action does not save changes to the EEPROM!
              switch(recvDataS.d[0]) {
                case 0:
                default:
                  SYSTEM_MODE = MODE_SUPER_HERO;
                break;
                case 1:
                  SYSTEM_MODE = MODE_ORIGINAL;
                break;
              }
              switch(recvDataS.d[1]) {
                case 1:
                default:
                  SYSTEM_YEAR = SYSTEM_TOGGLE_SWITCH;
                break;
                case 2:
                  SYSTEM_YEAR = SYSTEM_1984;
                break;
                case 3:
                  SYSTEM_YEAR = SYSTEM_1989;
                break;
                case 4:
                  SYSTEM_YEAR = SYSTEM_AFTERLIFE;
                break;
                case 5:
                  SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
                break;
              }
              i_volume_master_percentage = recvDataS.d[2];
              b_stream_effects = recvDataS.d[3];
              b_overheat_strobe = recvDataS.d[4];
              b_overheat_lights_off = recvDataS.d[5];
              b_overheat_sync_to_fan = recvDataS.d[6];
              b_demo_light_mode = recvDataS.d[7];

              // Cyclotron Lid
              i_cyclotron_leds = recvDataS.d[8];
              i_spectral_cyclotron_custom_colour = recvDataS.d[9];
              i_spectral_cyclotron_custom_saturation = recvDataS.d[10];
              b_clockwise = recvDataS.d[11]; // Cyclotron Direction
              b_cyclotron_single_led = recvDataS.d[12];
              b_cyclotron_colour_toggle = recvDataS.d[13];
              b_cyclotron_simulate_ring = recvDataS.d[14];

              // Inner Cyclotron
              i_inner_cyclotron_num_leds = recvDataS.d[15];
              i_spectral_cyclotron_inner_custom_colour = recvDataS.d[16];
              i_spectral_cyclotron_inner_custom_saturation = recvDataS.d[17];
              b_grb_cyclotron = recvDataS.d[18];

              // Power Cell
              i_powercell_leds = recvDataS.d[19];
              i_spectral_powercell_custom_colour = recvDataS.d[20];
              i_spectral_powercell_custom_saturation = recvDataS.d[21];
              b_powercell_colour_toggle = recvDataS.d[22];

              // Push changes to connected devices and reset related variables
              SYSTEM_YEAR_TEMP = SYSTEM_YEAR;
              SYSTEM_EEPROM_YEAR = SYSTEM_YEAR;
              b_switch_mode_override = true;

              // Update system values and reset as needed.
              updateSystemModeYear();
              updateProtonPackLEDCounts();
              resetContinuousSmoke();
              resetCyclotronLEDs();
              resetRampSpeeds();

              // Offer some feedback to the user
              stopEffect(S_VENT_DRY);
              playEffect(S_VENT_DRY);
            break;

            case A_SAVE_PREFERENCES_WAND:
              // Send latest preferences from serial1 web UI back to wand
              wandConfig.ledWandCount = recvDataS.d[0];
              wandConfig.ledWandHue = recvDataS.d[1];
              wandConfig.ledWandSat = recvDataS.d[2];
              wandConfig.spectralModeEnabled = recvDataS.d[3];
              wandConfig.spectralHolidayMode = recvDataS.d[4];
              wandConfig.overheatEnabled = recvDataS.d[5];
              wandConfig.defaultFiringModeCTS = recvDataS.d[6];
              wandConfig.defaultFiringModeMix = recvDataS.d[7];
              wandConfig.wandSoundsToPack = recvDataS.d[8];
              wandConfig.quickVenting = recvDataS.d[9];
              wandConfig.autoVentLight = recvDataS.d[10];
              wandConfig.wandBeepLoop = recvDataS.d[11];
              wandConfig.wandBootError = recvDataS.d[12];
              wandConfig.defaultYearModeWand = recvDataS.d[13];
              wandConfig.defaultYearModeCTS = recvDataS.d[14];
              wandConfig.invertWandBargraph = recvDataS.d[15];
              wandConfig.bargraphOverheatBlink = recvDataS.d[16];
              wandConfig.bargraphIdleAnimation = recvDataS.d[17];
              wandConfig.bargraphFireAnimation = recvDataS.d[18];

              // This will pass select values from the wandConfig object
              packSerialSendData(P_SAVE_PREFERENCES_WAND);

              // Offer some feedback to the user
              stopEffect(S_VENT_DRY);
              playEffect(S_VENT_DRY);
            break;

            case A_SAVE_PREFERENCES_SMOKE:
              // Save local and remote (wand) smoke timing settings
              i_ms_overheating_length_5 = recvDataS.d[0] * 1000;
              i_ms_overheating_length_4 = recvDataS.d[1] * 1000;
              i_ms_overheating_length_3 = recvDataS.d[2] * 1000;
              i_ms_overheating_length_2 = recvDataS.d[3] * 1000;
              i_ms_overheating_length_1 = recvDataS.d[4] * 1000;

              b_smoke_continuous_mode_5 = recvDataS.d[5];
              b_smoke_continuous_mode_4 = recvDataS.d[6];
              b_smoke_continuous_mode_3 = recvDataS.d[7];
              b_smoke_continuous_mode_2 = recvDataS.d[8];
              b_smoke_continuous_mode_1 = recvDataS.d[9];

              wandConfig.overheatLevel5 = recvDataS.d[10];
              wandConfig.overheatLevel4 = recvDataS.d[11];
              wandConfig.overheatLevel3 = recvDataS.d[12];
              wandConfig.overheatLevel2 = recvDataS.d[13];
              wandConfig.overheatLevel1 = recvDataS.d[14];

              wandConfig.overheatDelay5 = recvDataS.d[15];
              wandConfig.overheatDelay4 = recvDataS.d[16];
              wandConfig.overheatDelay3 = recvDataS.d[17];
              wandConfig.overheatDelay2 = recvDataS.d[18];
              wandConfig.overheatDelay1 = recvDataS.d[19];

              b_smoke_enabled = recvDataS.d[20];
              resetContinuousSmoke();

              // This will pass select values from the wandConfig object
              packSerialSendData(P_SAVE_PREFERENCES_SMOKE);

              // Offer some feedback to the user
              stopEffect(S_VENT_SMOKE);
              playEffect(S_VENT_SMOKE);
            break;

            default:
              // No-op for anything else.
            break;
          }
        }
      }
      else {
        // Check if the Attenuator is telling us it is here after connecting it to the pack.
        // Then synchronise some settings between the pack and the Attenuator.
        if(recvCmdS.c == A_SYNC_START && b_serial_1_syncing != true) {
          b_serial_1_syncing = true;

          serial1Send(A_SYNC_START);

          // Tell the Attenuator that the pack is here.
          serial1Send(A_PACK_CONNECTED);

          // Make sure this is called before the A_YEAR is sent over to the Attenuator/Wireless.
          switch(SYSTEM_MODE) {
            case MODE_ORIGINAL:
              serial1Send(A_MODE_ORIGINAL);
            break;

            case MODE_SUPER_HERO:
            default:
              serial1Send(A_MODE_SUPER_HERO);
            break;
          }

          switch(SYSTEM_YEAR) {
            case SYSTEM_1984:
              serial1Send(A_YEAR_1984);
            break;
            case SYSTEM_1989:
              serial1Send(A_YEAR_1989);
            break;
            case SYSTEM_AFTERLIFE:
            default:
              serial1Send(A_YEAR_AFTERLIFE);
            break;
            case SYSTEM_FROZEN_EMPIRE:
              serial1Send(A_YEAR_FROZEN_EMPIRE);
            break;
          }

          // Ribbon cable alarm.
          if(b_alarm == true) {
            serial1Send(A_ALARM_ON);
          }
          else {
            serial1Send(A_ALARM_OFF);
          }

          // Pack status
          if(PACK_STATE != MODE_OFF) {
            serial1Send(A_PACK_ON);
          }
          else {
            serial1Send(A_PACK_OFF);
          }

          // Send the current power level.
          switch(i_wand_power_level) {
            case 5:
              serial1Send(A_POWER_LEVEL_5);
            break;

            case 4:
              serial1Send(A_POWER_LEVEL_4);
            break;

            case 3:
              serial1Send(A_POWER_LEVEL_3);
            break;

            case 2:
              serial1Send(A_POWER_LEVEL_2);
            break;

            case 1:
            default:
              serial1Send(A_POWER_LEVEL_1);
            break;
          }

          // Synchronise the firing modes.
          switch(FIRING_MODE) {
            case SLIME:
              serial1Send(A_SLIME_MODE);
            break;

            case STASIS:
              serial1Send(A_STASIS_MODE);
            break;

            case MESON:
              serial1Send(A_MESON_MODE);
            break;

            case SPECTRAL:
              serial1Send(A_SPECTRAL_MODE);
            break;

            case HOLIDAY:
              serial1Send(A_HOLIDAY_MODE);
            break;

            case SPECTRAL_CUSTOM:
              serial1SendData(A_SPECTRAL_CUSTOM_MODE);
            break;

            case VENTING:
              serial1Send(A_VENTING_MODE);
            break;

            case PROTON:
            case SETTINGS:
            default:
              serial1Send(A_PROTON_MODE);
            break;
          }

          serial1SendData(A_SPECTRAL_COLOUR_DATA);

          if(switch_power.getState() == LOW) {
            // Tell the Attenuator or any other device that the power to the Proton Pack is on.
            serial1Send(A_MODE_ORIGINAL_RED_SWITCH_ON);
          }
          else {
            // Tell the Attenuator or any other device that the power to the Proton Pack is off.
            serial1Send(A_MODE_ORIGINAL_RED_SWITCH_OFF);
          }

          // This sends over the music status and the current music track.
          if(b_playing_music == true) {
            serial1Send(A_MUSIC_IS_PLAYING, i_current_music_track);
          }
          else {
            serial1Send(A_MUSIC_IS_NOT_PLAYING, i_current_music_track);
          }

          if(b_music_paused == true) {
            serial1Send(A_MUSIC_IS_PAUSED);
          }
          else {
            serial1Send(A_MUSIC_IS_NOT_PAUSED);
          }

          serial1Send(A_MUSIC_TRACK_COUNT_SYNC, i_music_count);

          b_serial1_connected = true; // Device is officially connected.
          b_serial_1_syncing = false; // Sync process has been completed.

          ms_serial1_handshake.start(i_serial1_handshake_delay);
          ms_serial1_handshake_checking.start(i_serial1_handshake_delay / 2);

          serial1Send(A_SYNC_END);
        }
      }
    }
  }
}

// Incoming messages from the wand.
void checkWand() {
  if(packComs.available() > 0) {
    uint8_t i_packet_id = packComs.currentPacketID();
    Serial.println("Wand PacketID: " + String(i_packet_id));

    if(i_packet_id > 0) {
      switch(i_packet_id) {
        case 1:
          packComs.rxObj(recvCmdW);
          Serial.println("Recv. Wand Command: " + String(recvCmdW.c));
        break;
        case 2:
          packComs.rxObj(recvDataW);
          Serial.println("Recv. Wand Message: " + String(recvDataW.m));
        break;
      }

      // Perform checks based on whether a wand is connected or not.
      if(b_wand_connected == true) {
        // Handle simple commands.
        switch(recvCmdW.c) {
          case W_ON:
            // The wand has been turned on.
            b_wand_on = true;

            // Turn the pack on.
            if(PACK_STATE != MODE_ON) {
              PACK_ACTION_STATE = ACTION_ACTIVATE;
              serial1Send(A_PACK_ON);
            }

            serial1Send(A_WAND_ON);
          break;

          case W_OFF:
            // The wand has been turned off.
            b_wand_on = false;

            // Turn the pack off.
            if(PACK_STATE != MODE_OFF) {
              PACK_ACTION_STATE = ACTION_OFF;
              serial1Send(A_PACK_OFF);
            }

            serial1Send(A_WAND_OFF);
          break;

          case W_BARREL_EXTENDED:
            // Unused at the moment.
            //b_neutrona_wand_barrel_extended = true;

            // Tell the attenuator or any other device on Serial 1 that the Neutrona Wand barrel is extended.
            serial1Send(A_BARREL_EXTENDED);
          break;

          case W_BARREL_RETRACTED:
            // Unused at the moment.
            //b_neutrona_wand_barrel_extended = false;

            // Tell the attenuator or any other device on Serial 1 that the Neutrona Wand barrel is retracted.
            serial1Send(A_BARREL_RETRACTED);
          break;

          case W_BARGRAPH_OVERHEAT_BLINK_ENABLED:
            stopEffect(S_VOICE_BARGRAPH_OVERHEAT_BLINK_ENABLED);
            stopEffect(S_VOICE_BARGRAPH_OVERHEAT_BLINK_DISABLED);

            playEffect(S_VOICE_BARGRAPH_OVERHEAT_BLINK_ENABLED);
          break;

          case W_BARGRAPH_OVERHEAT_BLINK_DISABLED:
            stopEffect(S_VOICE_BARGRAPH_OVERHEAT_BLINK_DISABLED);
            stopEffect(S_VOICE_BARGRAPH_OVERHEAT_BLINK_ENABLED);

            playEffect(S_VOICE_BARGRAPH_OVERHEAT_BLINK_DISABLED);
          break;

          case W_MODE_BEEP_LOOP_ENABLED:
            stopEffect(S_VOICE_NEUTRONA_WAND_BEEPING_DISABLED);
            stopEffect(S_VOICE_NEUTRONA_WAND_BEEPING_ENABLED);

            playEffect(S_VOICE_NEUTRONA_WAND_BEEPING_ENABLED);
          break;

          case W_MODE_BEEP_LOOP_DISABLED:
            stopEffect(S_VOICE_NEUTRONA_WAND_BEEPING_DISABLED);
            stopEffect(S_VOICE_NEUTRONA_WAND_BEEPING_ENABLED);

            playEffect(S_VOICE_NEUTRONA_WAND_BEEPING_DISABLED);
          break;

          case W_CYCLOTRON_SIMULATE_RING_TOGGLE:
            if(b_cyclotron_simulate_ring == true) {
              stopEffect(S_VOICE_CYCLOTRON_SIMULATE_RING_DISABLED);
              stopEffect(S_VOICE_CYCLOTRON_SIMULATE_RING_ENABLED);
              playEffect(S_VOICE_CYCLOTRON_SIMULATE_RING_DISABLED);

              b_cyclotron_simulate_ring = false;

              packSerialSend(P_CYCLOTRON_SIMULATE_RING_DISABLED);
            }
            else {
              stopEffect(S_VOICE_CYCLOTRON_SIMULATE_RING_DISABLED);
              stopEffect(S_VOICE_CYCLOTRON_SIMULATE_RING_ENABLED);
              playEffect(S_VOICE_CYCLOTRON_SIMULATE_RING_ENABLED);

              b_cyclotron_simulate_ring = true;

              packSerialSend(P_CYCLOTRON_SIMULATE_RING_ENABLED);
            }
          break;

          case W_VOICE_NEUTRONA_WAND_SOUNDS_ENABLED:
            stopEffect(S_VOICE_NEUTRONA_WAND_SOUNDS_ENABLED);
            playEffect(S_VOICE_NEUTRONA_WAND_SOUNDS_ENABLED);
          break;

          case W_VOICE_NEUTRONA_WAND_SOUNDS_DISABLED:
            stopEffect(S_VOICE_NEUTRONA_WAND_SOUNDS_DISABLED);
            playEffect(S_VOICE_NEUTRONA_WAND_SOUNDS_DISABLED);
          break;

          case W_AFTERLIFE_RAMP_LOOP_2_STOP:
            stopEffect(S_AFTERLIFE_WAND_IDLE_2);
          break;

          case W_AFTERLIFE_GUN_RAMP_1:
            stopEffect(S_AFTERLIFE_WAND_RAMP_1);
            playEffect(S_AFTERLIFE_WAND_RAMP_1, false, i_volume_effects - 10);
          break;

          case W_AFTERLIFE_GUN_RAMP_2:
            stopEffect(S_AFTERLIFE_WAND_RAMP_2);
            playEffect(S_AFTERLIFE_WAND_RAMP_2, false, i_volume_effects - 10);
          break;

          case W_AFTERLIFE_GUN_RAMP_2_FADE_IN:
            stopEffect(S_AFTERLIFE_WAND_RAMP_2_FADE_IN);
            playEffect(S_AFTERLIFE_WAND_RAMP_2_FADE_IN, false, i_volume_effects - 10);
          break;

          case W_AFTERLIFE_GUN_LOOP_1:
            stopEffect(S_AFTERLIFE_WAND_IDLE_1);
            playEffect(S_AFTERLIFE_WAND_IDLE_1, true, i_volume_effects - 10);
          break;

          case W_AFTERLIFE_GUN_LOOP_2:
            stopEffect(S_AFTERLIFE_WAND_IDLE_2);
            playEffect(S_AFTERLIFE_WAND_IDLE_2, true, i_volume_effects - 10);
          break;

          case W_AFTERLIFE_GUN_RAMP_DOWN_2:
            stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2);
            playEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2, false, i_volume_effects - 10);
          break;

          case W_AFTERLIFE_GUN_RAMP_DOWN_2_FADE_OUT:
            stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2_FADE_OUT);
            playEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2_FADE_OUT, false, i_volume_effects - 10);
          break;

          case W_AFTERLIFE_GUN_RAMP_DOWN_1:
            stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1);
            playEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1, false, i_volume_effects - 10);
          break;

          case W_EXTRA_WAND_SOUNDS_STOP:
            wandExtraSoundsStop();
          break;

          case W_FIRING:
            // Wand is firing.
            wandFiring();
          break;

          case W_FIRING_STOPPED:
            // Wand just stopped firing.
            //firingDebouncing();
            wandStoppedFiring();
            cyclotronSpeedRevert();
          break;

          case W_PROTON_MODE:
            // Proton mode
            FIRING_MODE = PROTON;
            playEffect(S_CLICK);

            stopEffect(S_PACK_SLIME_TANK_LOOP);
            stopEffect(S_STASIS_IDLE_LOOP);
            stopEffect(S_MESON_IDLE_LOOP);

            if(PACK_STATE == MODE_ON && b_wand_on == true) {
              playEffect(S_FIRE_START_SPARK);
            }

            if(b_cyclotron_colour_toggle == true) {
              // Reset the Cyclotron LED colours.
              cyclotronColourReset();
            }

            if(b_powercell_colour_toggle == true) {
              // Reset the Power Cell colours.
              b_powercell_updating = true;
              powercellDraw();
            }

            serial1Send(A_PROTON_MODE);
          break;

          case W_SLIME_MODE:
            // Slime mode
            FIRING_MODE = SLIME;
            playEffect(S_CLICK);

            stopEffect(S_PACK_SLIME_TANK_LOOP);
            stopEffect(S_STASIS_IDLE_LOOP);
            stopEffect(S_MESON_IDLE_LOOP);

            if(PACK_STATE == MODE_ON && b_wand_on == true) {
              playEffect(S_PACK_SLIME_OPEN);
              playEffect(S_PACK_SLIME_TANK_LOOP, true, 0, true, 900);
            }

            if(b_cyclotron_colour_toggle == true) {
              // Reset the Cyclotron LED colours.
              cyclotronColourReset();
            }

            if(b_powercell_colour_toggle == true) {
              // Reset the Power Cell colours.
              b_powercell_updating = true;

              powercellDraw();
            }

            serial1Send(A_SLIME_MODE);
          break;

          case W_STASIS_MODE:
            // Stasis mode
            FIRING_MODE = STASIS;
            playEffect(S_CLICK);

            stopEffect(S_PACK_SLIME_TANK_LOOP);
            stopEffect(S_STASIS_IDLE_LOOP);
            stopEffect(S_MESON_IDLE_LOOP);

            if(PACK_STATE == MODE_ON && b_wand_on == true) {
              playEffect(S_STASIS_OPEN);
              playEffect(S_STASIS_IDLE_LOOP, true, 0, true, 900);
            }

            if(b_cyclotron_colour_toggle == true) {
              // Reset the Cyclotron LED colours.
              cyclotronColourReset();
            }

            if(b_powercell_colour_toggle == true) {
              // Reset the Power Cell colours.
              b_powercell_updating = true;
              powercellDraw();
            }

            serial1Send(A_STASIS_MODE);
          break;

          case W_MESON_MODE:
            // Meson mode
            FIRING_MODE = MESON;
            playEffect(S_CLICK);

            stopEffect(S_PACK_SLIME_TANK_LOOP);
            stopEffect(S_STASIS_IDLE_LOOP);
            stopEffect(S_MESON_IDLE_LOOP);

            if(PACK_STATE == MODE_ON && b_wand_on == true) {
              playEffect(S_MESON_OPEN);
              playEffect(S_MESON_IDLE_LOOP, true, 0, true, 900);
            }

            if(b_cyclotron_colour_toggle == true) {
              // Reset the Cyclotron LED colours.
              cyclotronColourReset();
            }

            if(b_powercell_colour_toggle == true) {
              // Reset the Power Cell colours.
              b_powercell_updating = true;
              powercellDraw();
            }

            serial1Send(A_MESON_MODE);
          break;

          case W_SPECTRAL_MODE:
            // Proton mode
            FIRING_MODE = SPECTRAL;
            playEffect(S_CLICK);

            stopEffect(S_PACK_SLIME_TANK_LOOP);
            stopEffect(S_STASIS_IDLE_LOOP);
            stopEffect(S_MESON_IDLE_LOOP);

            if(PACK_STATE == MODE_ON && b_wand_on == true) {
              playEffect(S_FIRE_START_SPARK);
            }

            if(b_cyclotron_colour_toggle == true) {
              // Reset the Cyclotron LED colours.
              cyclotronColourReset();
            }

            if(b_powercell_colour_toggle == true) {
              // Reset the Power Cell colours.
              b_powercell_updating = true;
              powercellDraw();
            }

            serial1Send(A_SPECTRAL_MODE);
          break;

          case W_HOLIDAY_MODE:
            // Proton mode
            FIRING_MODE = HOLIDAY;
            playEffect(S_CLICK);

            stopEffect(S_PACK_SLIME_TANK_LOOP);
            stopEffect(S_STASIS_IDLE_LOOP);
            stopEffect(S_MESON_IDLE_LOOP);

            if(PACK_STATE == MODE_ON && b_wand_on == true) {
              stopEffect(S_PACK_SLIME_TANK_LOOP);
              playEffect(S_FIRE_START_SPARK);
            }

            if(b_cyclotron_colour_toggle == true) {
              // Reset the Cyclotron LED colours.
              cyclotronColourReset();
            }

            if(b_powercell_colour_toggle == true) {
              // Reset the Power Cell colours.
              b_powercell_updating = true;
              powercellDraw();
            }

            serial1Send(A_HOLIDAY_MODE);
          break;

          case W_SPECTRAL_CUSTOM_MODE:
            // Proton mode
            FIRING_MODE = SPECTRAL_CUSTOM;
            playEffect(S_CLICK);

            stopEffect(S_PACK_SLIME_TANK_LOOP);
            stopEffect(S_STASIS_IDLE_LOOP);
            stopEffect(S_MESON_IDLE_LOOP);

            if(PACK_STATE == MODE_ON && b_wand_on == true) {
              stopEffect(S_PACK_SLIME_TANK_LOOP);
              playEffect(S_FIRE_START_SPARK);
            }

            if(b_cyclotron_colour_toggle == true) {
              // Reset the Cyclotron LED colours.
              cyclotronColourReset();
            }

            if(b_powercell_colour_toggle == true) {
              // Reset the Power Cell colours.
              b_powercell_updating = true;
              powercellDraw();
            }

            serial1SendData(A_SPECTRAL_CUSTOM_MODE);
          break;

          case W_VENTING_MODE:
            // Settings mode
            FIRING_MODE = VENTING;
            playEffect(S_CLICK);

            stopEffect(S_PACK_SLIME_TANK_LOOP);
            stopEffect(S_STASIS_IDLE_LOOP);
            stopEffect(S_MESON_IDLE_LOOP);

            if(PACK_STATE == MODE_ON && b_wand_on == true) {
              stopEffect(S_PACK_SLIME_TANK_LOOP);
              playEffect(S_VENT_DRY);
              playEffect(S_MODE_SWITCH);
            }

            if(b_cyclotron_colour_toggle == true) {
              // Reset the Cyclotron LED colours.
              cyclotronColourReset();
            }

            if(b_powercell_colour_toggle == true) {
              // Reset the Power Cell colours.
              b_powercell_updating = true;
              powercellDraw();
            }

            serial1Send(A_VENTING_MODE);
          break;

          case W_SETTINGS_MODE:
            // Settings mode
            FIRING_MODE = SETTINGS;
            playEffect(S_CLICK);

            stopEffect(S_PACK_SLIME_TANK_LOOP);
            stopEffect(S_STASIS_IDLE_LOOP);
            stopEffect(S_MESON_IDLE_LOOP);

            if(b_cyclotron_colour_toggle == true) {
              // Reset the Cyclotron LED colours.
              cyclotronColourReset();
            }

            if(b_powercell_colour_toggle == true) {
              // Reset the Power Cell colours.
              b_powercell_updating = true;
              powercellDraw();
            }

            serial1Send(A_SETTINGS_MODE);
          break;

          case W_OVERHEATING:
            // Overheating
            stopEffect(S_BEEP_8);

            playEffect(S_VENT_SLOW);

            b_overheating = true;

            // Start timer for a second smoke sound.
            ms_overheating.start(i_overheating_delay);

            // Reset some vent light timers.
            ms_vent_light_off.stop();
            ms_vent_light_on.stop();
            //ms_fan_stop_timer.stop();
            ms_vent_light_off.start(i_vent_light_delay);
            //ms_fan_stop_timer.start(i_fan_stop_timer);

            // Reset the Inner Cyclotron speed.
            if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
              i_inner_current_ramp_speed = i_inner_ramp_delay;
            }

            serial1Send(A_OVERHEATING);
          break;

          // No longer used.
          case W_OVERHEATING_FINISHED:
            // Overheating finished
            packOverHeatingFinished();

            serial1Send(A_OVERHEATING_FINISHED);
          break;

          case W_CYCLOTRON_NORMAL_SPEED:
            // Reset Cyclotron speed.
            cyclotronSpeedRevert();

            // Indicate normalcy to serial device.
            serial1Send(A_CYCLOTRON_NORMAL_SPEED);
          break;

          case W_CYCLOTRON_INCREASE_SPEED:
            // Speed up Cyclotron.
            cyclotronSpeedIncrease();

            // Indicate speed-up to serial device.
            serial1Send(A_CYCLOTRON_INCREASE_SPEED);
          break;

          case W_HANDSHAKE:
            // The wand is still here.
            ms_wand_handshake.start(i_wand_handshake_delay);
            ms_wand_handshake_checking.start(i_wand_handshake_delay / 2);
            b_wand_connected = true;
          break;

          case W_BEEP_START:
            // Play 8 overheat beeps before we overheat.
            playEffect(S_BEEP_8);
          break;

          case W_POWER_LEVEL_1:
            // Wand power level 1
            i_wand_power_level = 1;

            // Reset the smoke timer if the wand is firing.
            if(b_wand_firing == true) {
              if(ms_smoke_timer.isRunning() == true) {
                ms_smoke_timer.start(i_smoke_timer[i_wand_power_level - 1]);
              }
            }

            serial1Send(A_POWER_LEVEL_1);
          break;

          case W_POWER_LEVEL_2:
            // Wand power level 2
            i_wand_power_level = 2;

            // Reset the smoke timer if the wand is firing.
            if(b_wand_firing == true) {
              if(ms_smoke_timer.isRunning() == true) {
                ms_smoke_timer.start(i_smoke_timer[i_wand_power_level - 1]);
              }
            }

            serial1Send(A_POWER_LEVEL_2);
          break;

          case W_POWER_LEVEL_3:
            // Wand power level 3
            i_wand_power_level = 3;

            // Reset the smoke timer if the wand is firing.
            if(b_wand_firing == true) {
              if(ms_smoke_timer.isRunning() == true) {
                ms_smoke_timer.start(i_smoke_timer[i_wand_power_level - 1]);
              }
            }

            serial1Send(A_POWER_LEVEL_3);
          break;

          case W_POWER_LEVEL_4:
            // Wand power level 4
            i_wand_power_level = 4;

            // Reset the smoke timer if the wand is firing.
            if(b_wand_firing == true) {
              if(ms_smoke_timer.isRunning() == true) {
                ms_smoke_timer.start(i_smoke_timer[i_wand_power_level - 1]);
              }
            }

            serial1Send(A_POWER_LEVEL_4);
          break;

          case W_POWER_LEVEL_5:
            // Wand power level 5
            i_wand_power_level = 5;

            // Reset the smoke timer if the wand is firing.
            if(b_wand_firing == true) {
              if(ms_smoke_timer.isRunning() == true) {
                ms_smoke_timer.start(i_smoke_timer[i_wand_power_level - 1]);
              }
            }

            serial1Send(A_POWER_LEVEL_5);
          break;

          case W_OVERHEAT_INCREASE_LEVEL_1:
            overheatIncrement(1);
          break;

          case W_OVERHEAT_INCREASE_LEVEL_2:
            overheatIncrement(2);
          break;

          case W_OVERHEAT_INCREASE_LEVEL_3:
            overheatIncrement(3);
          break;

          case W_OVERHEAT_INCREASE_LEVEL_4:
            overheatIncrement(4);
          break;

          case W_OVERHEAT_INCREASE_LEVEL_5:
            overheatIncrement(5);
          break;

          case W_OVERHEAT_DECREASE_LEVEL_1:
            overheatDecrement(1);
          break;

          case W_OVERHEAT_DECREASE_LEVEL_2:
            overheatDecrement(2);
          break;

          case W_OVERHEAT_DECREASE_LEVEL_3:
            overheatDecrement(3);
          break;

          case W_OVERHEAT_DECREASE_LEVEL_4:
            overheatDecrement(4);
          break;

          case W_OVERHEAT_DECREASE_LEVEL_5:
            overheatDecrement(5);
          break;

          case W_FIRING_INTENSIFY:
            // Wand firing in intensify mode.
            b_firing_intensify = true;

            if(b_wand_firing == true && b_sound_firing_intensify_trigger != true) {
              b_sound_firing_intensify_trigger = true;
            }
          break;

          case W_FIRING_INTENSIFY_MIX:
            // Wand firing in intensify mode.
            b_firing_intensify = true;

            if(b_wand_firing == true && b_sound_firing_intensify_trigger != true) {
              b_sound_firing_intensify_trigger = true;

              switch(i_wand_power_level) {
                case 1 ... 4:
                  if(SYSTEM_YEAR == SYSTEM_1989) {
                    playEffect(S_GB2_FIRE_LOOP);
                    playEffect(S_GB2_FIRE_START);
                  }
                  else {
                    playEffect(S_GB1_FIRE_LOOP, true);
                    playEffect(S_GB1_FIRE_START);
                  }
                break;

                case 5:
                  playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true);
                break;
              }
            }

          break;

          case W_FIRING_INTENSIFY_STOPPED:
            // Wand no longer firing in intensify mode.
            b_firing_intensify = false;
            b_sound_firing_intensify_trigger = false;
          break;

          case W_FIRING_INTENSIFY_STOPPED_MIX:
            // Wand no longer firing in intensify mode.
            if(STATUS_CTS == CTS_NOT_FIRING && b_firing_intensify == true) {
              switch(i_wand_power_level) {
                case 1 ... 4:
                  if(SYSTEM_YEAR == SYSTEM_1989) {
                    stopEffect(S_GB2_FIRE_LOOP);
                    stopEffect(S_GB2_FIRE_START);
                  }
                  else {
                    stopEffect(S_GB1_FIRE_LOOP);
                    stopEffect(S_GB1_FIRE_START);
                  }
                break;

                case 5:
                  stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);
                break;
              }
            }

            b_firing_intensify = false;
            b_sound_firing_intensify_trigger = false;
          break;

          case W_FIRING_ALT:
            // Wand firing in alt mode.
            b_firing_alt = true;

            if(b_wand_firing == true && b_sound_firing_alt_trigger != true) {
              b_sound_firing_alt_trigger = true;
            }
          break;

          case W_FIRING_ALT_MIX:
            // Wand firing in alt mode.
            b_firing_alt = true;

            if(b_wand_firing == true && b_sound_firing_alt_trigger != true) {
              b_sound_firing_alt_trigger = true;

              playEffect(S_FIRING_LOOP_GB1, true);
            }
          break;

          case W_FIRING_ALT_STOPPED:
            // Wand no longer firing in alt mode.
            b_firing_alt = false;
            b_sound_firing_alt_trigger = false;
          break;

          case W_FIRING_ALT_STOPPED_MIX:
            // Wand no longer firing in alt mode mix.
            if(b_firing_alt == true) {
              stopEffect(S_FIRING_LOOP_GB1);
            }

            b_firing_alt = false;
            b_sound_firing_alt_trigger = false;
          break;

          case W_FIRING_CROSSING_THE_STREAMS_1984:
            // Wand is crossing the streams.
            STATUS_CTS = CTS_FIRING_1984;

            stopEffect(S_CROSS_STREAMS_END);
            stopEffect(S_CROSS_STREAMS_START);
            playEffect(S_FIRE_SPARKS);

            playEffect(S_CROSS_STREAMS_START, false, i_volume_effects + 10);

            playEffect(S_FIRE_START_SPARK, false, i_volume_effects + 10);
          break;

          case W_FIRING_CROSSING_THE_STREAMS_2021:
            // Wand is crossing the streams.
            STATUS_CTS = CTS_FIRING_2021;

            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);
            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
            playEffect(S_FIRE_SPARKS);

            playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START, false, i_volume_effects + 10);

            playEffect(S_FIRE_START_SPARK, false, i_volume_effects + 10);
          break;

          case W_FIRING_CROSSING_THE_STREAMS_MIX_1984:
            // Wand is crossing the streams.
            STATUS_CTS = CTS_FIRING_1984;

            stopEffect(S_CROSS_STREAMS_END);
            stopEffect(S_CROSS_STREAMS_START);

            playEffect(S_CROSS_STREAMS_START, false, i_volume_effects + 10);

            playEffect(S_FIRE_START_SPARK);
            playEffect(S_FIRING_LOOP_GB1, true);

            if(i_wand_power_level != i_wand_power_level_max) {
              playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true);
            }

            if(SYSTEM_YEAR == SYSTEM_1989) {
              stopEffect(S_GB2_FIRE_LOOP);
            }
            else {
              stopEffect(S_GB1_FIRE_LOOP);
            }
          break;

          case W_FIRING_CROSSING_THE_STREAMS_MIX_2021:
            // Wand is crossing the streams.
            STATUS_CTS = CTS_FIRING_2021;

            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);
            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);

            playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START, false, i_volume_effects + 10);

            playEffect(S_FIRE_START_SPARK);
            playEffect(S_FIRING_LOOP_GB1, true);

            if(i_wand_power_level != i_wand_power_level_max) {
              playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true);
            }

            if(SYSTEM_YEAR == SYSTEM_1989) {
              stopEffect(S_GB2_FIRE_LOOP);
            }
            else {
              stopEffect(S_GB1_FIRE_LOOP);
            }
          break;

          case W_FIRING_CROSSING_THE_STREAMS_STOPPED_1984:
            // The wand is no longer crossing the streams.
            STATUS_CTS = CTS_NOT_FIRING;

            stopEffect(S_CROSS_STREAMS_START);
            stopEffect(S_CROSS_STREAMS_END);

            playEffect(S_CROSS_STREAMS_END, false, i_volume_effects + 10);

            stopEffect(S_FIRING_LOOP_GB1);
          break;

          case W_FIRING_CROSSING_THE_STREAMS_STOPPED_2021:
            // The wand is no longer crossing the streams.
            STATUS_CTS = CTS_NOT_FIRING;

            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);

            playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects + 10);

            stopEffect(S_FIRING_LOOP_GB1);
          break;

          case W_FIRING_CROSSING_THE_STREAMS_STOPPED_MIX_1984:
            // The wand is no longer crossing the streams.
            STATUS_CTS = CTS_NOT_FIRING;

            stopEffect(S_CROSS_STREAMS_START);
            stopEffect(S_CROSS_STREAMS_END);

            playEffect(S_CROSS_STREAMS_END, false, i_volume_effects + 10);
          break;

          case W_FIRING_CROSSING_THE_STREAMS_STOPPED_MIX_2021:
            // The wand is no longer crossing the streams.
            STATUS_CTS = CTS_NOT_FIRING;

            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
            stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);

            playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects + 10);
          break;

          case W_YEAR_MODES_CYCLE:
            toggleYearModes();

            // Turn on the year mode override flag. This resets when you flip the year mode toggle switch on the pack.
            b_switch_mode_override = true;
          break;

          case W_RESET_PROTON_STREAM:
            // Revert back to proton mode. Usually because we are switching from crossing the streams to video game mode or vice versa.
            FIRING_MODE = PROTON;

            stopEffect(S_CLICK);

            playEffect(S_CLICK);

            stopEffect(S_VOICE_VIDEO_GAME_MODES);
            stopEffect(S_VOICE_CROSS_THE_STREAMS_MIX);
            stopEffect(S_VOICE_CROSS_THE_STREAMS);

            playEffect(S_VOICE_CROSS_THE_STREAMS);
          break;

          case W_RESET_PROTON_STREAM_MIX:
            // Revert back to proton mode. Usually because we are switching from crossing the streams to video game mode or vice versa.
            FIRING_MODE = PROTON;

            stopEffect(S_CLICK);
            playEffect(S_CLICK);

            stopEffect(S_VOICE_VIDEO_GAME_MODES);
            stopEffect(S_VOICE_CROSS_THE_STREAMS);
            stopEffect(S_VOICE_CROSS_THE_STREAMS_MIX);

            playEffect(S_VOICE_CROSS_THE_STREAMS_MIX);
          break;

          case W_SPECTRAL_MODES_ENABLED:
            stopEffect(S_VOICE_SPECTRAL_MODES_DISABLED);
            stopEffect(S_VOICE_SPECTRAL_MODES_ENABLED);
            playEffect(S_VOICE_SPECTRAL_MODES_ENABLED);
          break;

          case W_SPECTRAL_MODES_DISABLED:
            stopEffect(S_VOICE_SPECTRAL_MODES_DISABLED);
            stopEffect(S_VOICE_SPECTRAL_MODES_ENABLED);
            playEffect(S_VOICE_SPECTRAL_MODES_DISABLED);
          break;

          case W_VIBRATION_DISABLED:
            // Neutrona Wand vibration disabled.
            stopEffect(S_BEEPS_ALT);

            playEffect(S_BEEPS_ALT);

            stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
            stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
            stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

            playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);
          break;

          case W_VIBRATION_ENABLED:
            // Neutrona Wand vibration enabled.
            stopEffect(S_BEEPS_ALT);

            playEffect(S_BEEPS_ALT);

            stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
            stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
            stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

            playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
          break;

          case W_VIBRATION_FIRING_ENABLED:
            // Neutrona Wand vibration during firing only enabled.
            stopEffect(S_BEEPS_ALT);

            playEffect(S_BEEPS_ALT);

            stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
            stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
            stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

            playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
          break;

          case W_VIBRATION_CYCLE_TOGGLE:
            stopEffect(S_BEEPS_ALT);

            playEffect(S_BEEPS_ALT);

            if(b_vibration == false) {
              b_vibration = true;
              b_vibration_enabled = true; // Override the Proton Pack vibration toggle switch.

              // Proton Pack vibration enabled.
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);

              playEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);

              packSerialSend(P_PACK_VIBRATION_ENABLED);

              analogWrite(vibration, 150);
              delay(250);
              analogWrite(vibration,0);
            }
            else if(b_vibration == true && b_vibration_firing != true) {
              b_vibration_firing = true;
              b_vibration_enabled = true; // Override the Proton Pack vibration toggle switch.

              // Proton Pack vibration firing enabled.
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);

              playEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);

              packSerialSend(P_PACK_VIBRATION_FIRING_ENABLED);

              analogWrite(vibration, 150);
              delay(250);
              analogWrite(vibration,0);
            }
            else {
              b_vibration_firing = false;
              b_vibration = false;

              // Proton Pack vibration disabled.
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);

              playEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);

              packSerialSend(P_PACK_VIBRATION_DISABLED);
            }
          break;

          case W_SMOKE_TOGGLE:
            if(b_smoke_enabled == true) {
              b_smoke_enabled = false;

              stopEffect(S_VENT_DRY);

              playEffect(S_VENT_DRY);

              stopEffect(S_VOICE_SMOKE_DISABLED);
              stopEffect(S_VOICE_SMOKE_ENABLED);

              playEffect(S_VOICE_SMOKE_DISABLED);

              // Tell the wand to play the smoke disabled voice.
              packSerialSend(P_SMOKE_DISABLED);
            }
            else {
              b_smoke_enabled = true;

              stopEffect(S_VENT_SMOKE);

              playEffect(S_VENT_SMOKE);

              stopEffect(S_VOICE_SMOKE_ENABLED);
              stopEffect(S_VOICE_SMOKE_DISABLED);

              playEffect(S_VOICE_SMOKE_ENABLED);

              // Tell the wand to play the smoke enabled voice.
              packSerialSend(P_SMOKE_ENABLED);
            }
          break;

          case W_PROTON_MODE_REVERT:
            // Revert back to proton mode. Usually because we are switching from crossing the streams to video game mode or vice versa.
            FIRING_MODE = PROTON;

            stopEffect(S_CLICK);

            playEffect(S_CLICK);

            stopEffect(S_VOICE_CROSS_THE_STREAMS);
            stopEffect(S_VOICE_CROSS_THE_STREAMS_MIX);
            stopEffect(S_VOICE_VIDEO_GAME_MODES);

            playEffect(S_VOICE_VIDEO_GAME_MODES);
          break;

          case W_CYCLOTRON_DIRECTION_TOGGLE:
            // Toggle the Cyclotron direction.
            if(b_clockwise == true) {
              b_clockwise = false;

              stopEffect(S_BEEPS_ALT);

              playEffect(S_BEEPS_ALT);

              stopEffect(S_VOICE_CYCLOTRON_CLOCKWISE);
              stopEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);

              playEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);

              // Tell the wand to play the same sound.
              packSerialSend(P_CYCLOTRON_COUNTER_CLOCKWISE);
            }
            else {
              b_clockwise = true;

              stopEffect(S_BEEPS);

              playEffect(S_BEEPS);

              stopEffect(S_VOICE_CYCLOTRON_CLOCKWISE);
              stopEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);

              playEffect(S_VOICE_CYCLOTRON_CLOCKWISE);

              // Tell the wand to play the same sound.
              packSerialSend(P_CYCLOTRON_CLOCKWISE);
            }
          break;

          case W_CYCLOTRON_LED_TOGGLE:
            // Toggle single LED or 3 LEDs per Cyclotron lens in 1984/1989 modes.
            if(b_cyclotron_single_led == true) {
              b_cyclotron_single_led = false;

              stopEffect(S_VOICE_THREE_LED);
              stopEffect(S_VOICE_SINGLE_LED);

              playEffect(S_VOICE_THREE_LED);

              // Tell the wand to play the same sound.
              packSerialSend(P_CYCLOTRON_THREE_LED);
            }
            else {
              b_cyclotron_single_led = true;

              // Play Single LED voice.
              stopEffect(S_VOICE_THREE_LED);
              stopEffect(S_VOICE_SINGLE_LED);

              playEffect(S_VOICE_SINGLE_LED);

              // Tell the wand to play the same sound.
              packSerialSend(P_CYCLOTRON_SINGLE_LED);
            }
          break;

          case W_VIDEO_GAME_MODE_COLOUR_TOGGLE:
            stopEffect(S_VOICE_VIDEO_GAME_COLOURS_DISABLED);
            stopEffect(S_VOICE_VIDEO_GAME_COLOURS_ENABLED);
            stopEffect(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED);
            stopEffect(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED);

            // Toggle through the various Video Game Colour Modes for the Proton Pack LEDs (if supported).
            if(b_cyclotron_colour_toggle == true && b_powercell_colour_toggle == true) {
              // Disabled, both Cyclotron and Power Cell video game colours.
              b_cyclotron_colour_toggle = false;
              b_powercell_colour_toggle = false;

              playEffect(S_VOICE_VIDEO_GAME_COLOURS_DISABLED);

              // Tell the wand to play the same sound.
              packSerialSend(P_VIDEO_GAME_MODE_COLOURS_DISABLED);
            }
            else if(b_cyclotron_colour_toggle != true && b_powercell_colour_toggle != true) {
              // Power Cell only.
              b_cyclotron_colour_toggle = false;
              b_powercell_colour_toggle = true;

              playEffect(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED);

              // Tell the wand to play the same sound.
              packSerialSend(P_VIDEO_GAME_MODE_POWER_CELL_ENABLED);
            }
            else if(b_cyclotron_colour_toggle != true && b_powercell_colour_toggle == true) {
              // Cyclotron only.
              b_cyclotron_colour_toggle = true;
              b_powercell_colour_toggle = false;

              playEffect(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED);

              // Tell the wand to play the same sound.
              packSerialSend(P_VIDEO_GAME_MODE_CYCLOTRON_ENABLED);
            }
            else {
              // Enabled, both Cyclotron and Power Cell video game colours.
              b_cyclotron_colour_toggle = true;
              b_powercell_colour_toggle = true;

              playEffect(S_VOICE_VIDEO_GAME_COLOURS_ENABLED);

              // Tell the wand to play the same sound.
              packSerialSend(P_VIDEO_GAME_MODE_COLOURS_ENABLED);
            }
          break;

          case W_OVERHEATING_DISABLED:
            // Play the overheating disabled voice.
            stopEffect(S_VOICE_OVERHEAT_DISABLED);
            stopEffect(S_VOICE_OVERHEAT_ENABLED);

            playEffect(S_VOICE_OVERHEAT_DISABLED);
          break;

          case W_OVERHEATING_ENABLED:
            // Play the overheating enabled voice.
            stopEffect(S_VOICE_OVERHEAT_DISABLED);
            stopEffect(S_VOICE_OVERHEAT_ENABLED);

            playEffect(S_VOICE_OVERHEAT_ENABLED);
          break;

          case W_MENU_LEVEL_1:
            // Play a beep and other sounds when changing menu levels.
            stopEffect(S_BEEPS);
            playEffect(S_BEEPS);

            stopEffect(S_LEVEL_1);
            stopEffect(S_LEVEL_2);
            stopEffect(S_LEVEL_3);
            stopEffect(S_LEVEL_4);
            stopEffect(S_LEVEL_5);

            playEffect(S_LEVEL_1);
          break;

          case W_MENU_LEVEL_2:
            // Play a beep and other sounds when changing menu levels.
            stopEffect(S_BEEPS);
            playEffect(S_BEEPS);

            stopEffect(S_LEVEL_1);
            stopEffect(S_LEVEL_2);
            stopEffect(S_LEVEL_3);
            stopEffect(S_LEVEL_4);
            stopEffect(S_LEVEL_5);

            playEffect(S_LEVEL_2);
          break;

          case W_MENU_LEVEL_3:
            // Play a beep and other sounds when changing menu levels.
            stopEffect(S_BEEPS);
            playEffect(S_BEEPS);

            stopEffect(S_LEVEL_1);
            stopEffect(S_LEVEL_2);
            stopEffect(S_LEVEL_3);
            stopEffect(S_LEVEL_4);
            stopEffect(S_LEVEL_5);

            playEffect(S_LEVEL_3);
          break;

          case W_MENU_LEVEL_4:
            // Play a beep and other sounds when changing menu levels.
            stopEffect(S_BEEPS);
            playEffect(S_BEEPS);

            stopEffect(S_LEVEL_1);
            stopEffect(S_LEVEL_2);
            stopEffect(S_LEVEL_3);
            stopEffect(S_LEVEL_4);
            stopEffect(S_LEVEL_5);

            playEffect(S_LEVEL_4);
          break;

          case W_MENU_LEVEL_5:
            // Play a beep and other sounds when changing menu levels.
            stopEffect(S_BEEPS);
            playEffect(S_BEEPS);

            stopEffect(S_LEVEL_1);
            stopEffect(S_LEVEL_2);
            stopEffect(S_LEVEL_3);
            stopEffect(S_LEVEL_4);
            stopEffect(S_LEVEL_5);

            playEffect(S_LEVEL_5);
          break;

          case W_VOLUME_MUSIC_DECREASE:
            // Lower music volume.
            if(b_playing_music == true) {
              if(i_volume_music_percentage - VOLUME_MUSIC_MULTIPLIER < 0) {
                i_volume_music_percentage = 0;

                // Provide feedback at minimum volume.
                stopEffect(S_BEEPS_ALT);
                playEffect(S_BEEPS_ALT, false, i_volume_master - 10);
              }
              else {
                i_volume_music_percentage = i_volume_music_percentage - VOLUME_MUSIC_MULTIPLIER;
              }

              i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

              w_trig.trackGain(i_current_music_track, i_volume_music);
              serial1SendData(A_VOLUME_SYNC);
            }
          break;

          case W_VOLUME_MUSIC_INCREASE:
            // Increase music volume.
            if(b_playing_music == true) {
              if(i_volume_music_percentage + VOLUME_MUSIC_MULTIPLIER > 100) {
                i_volume_music_percentage = 100;

                // Provide feedback at maximum volume.
                stopEffect(S_BEEPS_ALT);
                playEffect(S_BEEPS_ALT, false, i_volume_master - 10);
              }
              else {
                i_volume_music_percentage = i_volume_music_percentage + VOLUME_MUSIC_MULTIPLIER;
              }

              i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

              w_trig.trackGain(i_current_music_track, i_volume_music);
              serial1SendData(A_VOLUME_SYNC);
            }
          break;

          case W_VOLUME_SOUND_EFFECTS_DECREASE:
            // Lower the sound effects volume.
            decreaseVolumeEffects();
          break;

          case W_VOLUME_SOUND_EFFECTS_INCREASE:
            // Increase the sound effects volume.
            increaseVolumeEffects();
          break;

          case W_MUSIC_TRACK_LOOP_TOGGLE:
            // Loop the music track.
            if(b_repeat_track == false) {
              b_repeat_track = true;
              w_trig.trackLoop(i_current_music_track, 1);
            }
            else {
              b_repeat_track = false;
              w_trig.trackLoop(i_current_music_track, 0);
            }
          break;

          case W_SILENT_MODE:
            // Remember the current master volume level.
            i_volume_revert = i_volume_master;

            // Set the master volume to silent.
            i_volume_master = i_volume_abs_min;

            w_trig.masterGain(i_volume_master); // Reset the master gain.
          break;

          case W_VOLUME_REVERT:
            // Restore the master volume to previous level.
            i_volume_master = i_volume_revert;

            w_trig.masterGain(i_volume_master); // Reset the master gain.
          break;

          case W_VOLUME_DECREASE:
            // Lower overall pack volume.
            decreaseVolume();
          break;

          case W_VOLUME_INCREASE:
            // Increase overall pack volume.
            increaseVolume();
          break;

          case W_MUSIC_STOP:
            // Stop music.
            b_playing_music = false;
            stopMusic();
          break;

          case W_MUSIC_START:
            // Play the appropriate track on pack and wand, and notify the serial1 device.
            playMusic();
          break;

          case W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_5:
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);

            playEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);
          break;

          case W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_4:
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);

            playEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);
          break;

          case W_VOLUME_DECREASE_EEPROM:
            // Decrease the overall default pack volume which is saved into the EEPROM.
            decreaseVolumeEEPROM();

            // Tell wand to decrease volume.
            packSerialSend(P_VOLUME_DECREASE);
          break;

          case W_VOLUME_INCREASE_EEPROM:
            // Increase the overall default pack volume which is saved into the EEPROM.
            increaseVolumeEEPROM();

            // Tell wand to increase volume.
            packSerialSend(P_VOLUME_INCREASE);
          break;

          case W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_3:
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);

            playEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);
          break;

          case W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_2:
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);

            playEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);
          break;

          case W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_1:
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_5);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_4);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_3);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_2);
            stopEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);

            playEffect(S_VOICE_OVERHEAT_SMOKE_DURATION_LEVEL_1);
          break;

          case W_SOUND_OVERHEAT_START_TIMER_LEVEL_5:
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_5);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_4);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_3);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_2);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_1);

            playEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_5);
          break;

          case W_SOUND_OVERHEAT_START_TIMER_LEVEL_4:
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_5);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_4);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_3);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_2);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_1);

            playEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_4);
          break;

          case W_SOUND_OVERHEAT_START_TIMER_LEVEL_3:
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_5);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_4);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_3);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_2);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_1);

            playEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_3);
          break;

          case W_SOUND_OVERHEAT_START_TIMER_LEVEL_2:
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_5);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_4);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_3);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_2);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_1);

            playEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_2);
          break;

          case W_SOUND_OVERHEAT_START_TIMER_LEVEL_1:
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_5);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_4);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_3);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_2);
            stopEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_1);

            playEffect(S_VOICE_OVERHEAT_START_TIMER_LEVEL_1);
          break;

          case W_SOUND_DEFAULT_SYSTEM_VOLUME_ADJUSTMENT:
            stopEffect(S_VOICE_DEFAULT_SYSTEM_VOLUME_ADJUSTMENT);

            playEffect(S_VOICE_DEFAULT_SYSTEM_VOLUME_ADJUSTMENT);
          break;

          case W_PROTON_STREAM_IMPACT_TOGGLE:
            if(b_stream_effects == true) {
              b_stream_effects = false;

              stopEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
              stopEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);
              playEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);

              packSerialSend(P_PROTON_STREAM_IMPACT_DISABLED);
            }
            else {
              b_stream_effects = true;

              stopEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
              stopEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);
              playEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);

              packSerialSend(P_PROTON_STREAM_IMPACT_ENABLED);
            }
          break;

          case W_CTS_1984:
            stopEffect(S_VOICE_CTS_1984);
            stopEffect(S_VOICE_CTS_DEFAULT);
            stopEffect(S_VOICE_CTS_AFTERLIFE);
            stopEffect(S_VOICE_CTS_1989);
            stopEffect(S_VOICE_CTS_FROZEN_EMPIRE);

            playEffect(S_VOICE_CTS_1984);
          break;

          case W_CTS_1989:
            stopEffect(S_VOICE_CTS_1989);
            stopEffect(S_VOICE_CTS_1984);
            stopEffect(S_VOICE_CTS_DEFAULT);
            stopEffect(S_VOICE_CTS_AFTERLIFE);
            stopEffect(S_VOICE_CTS_FROZEN_EMPIRE);

            playEffect(S_VOICE_CTS_1989);
          break;

          case W_CTS_AFTERLIFE:
            stopEffect(S_VOICE_CTS_AFTERLIFE);
            stopEffect(S_VOICE_CTS_1984);
            stopEffect(S_VOICE_CTS_1989);
            stopEffect(S_VOICE_CTS_FROZEN_EMPIRE);
            stopEffect(S_VOICE_CTS_DEFAULT);

            playEffect(S_VOICE_CTS_AFTERLIFE);
          break;

          case W_CTS_FROZEN_EMPIRE:
            stopEffect(S_VOICE_CTS_FROZEN_EMPIRE);
            stopEffect(S_VOICE_CTS_DEFAULT);
            stopEffect(S_VOICE_CTS_AFTERLIFE);
            stopEffect(S_VOICE_CTS_1984);
            stopEffect(S_VOICE_CTS_1989);

            playEffect(S_VOICE_CTS_FROZEN_EMPIRE);
          break;

          case W_CTS_DEFAULT:
            stopEffect(S_VOICE_CTS_DEFAULT);
            stopEffect(S_VOICE_CTS_AFTERLIFE);
            stopEffect(S_VOICE_CTS_1984);
            stopEffect(S_VOICE_CTS_1989);
            stopEffect(S_VOICE_CTS_FROZEN_EMPIRE);

            playEffect(S_VOICE_CTS_DEFAULT);
          break;

          case W_MODE_TOGGLE:
            switch(SYSTEM_MODE) {
              case MODE_ORIGINAL:
                SYSTEM_MODE = MODE_SUPER_HERO;

                stopEffect(S_VOICE_MODE_SUPER_HERO);
                stopEffect(S_VOICE_MODE_ORIGINAL);
                playEffect(S_VOICE_MODE_SUPER_HERO);

                packSerialSend(P_SOUND_SUPER_HERO);
                packSerialSend(P_MODE_SUPER_HERO);
                serial1Send(A_MODE_SUPER_HERO);
              break;

              case MODE_SUPER_HERO:
              default:
                SYSTEM_MODE = MODE_ORIGINAL;

                stopEffect(S_VOICE_MODE_ORIGINAL);
                stopEffect(S_VOICE_MODE_SUPER_HERO);
                playEffect(S_VOICE_MODE_ORIGINAL);

                packSerialSend(P_SOUND_MODE_ORIGINAL);
                packSerialSend(P_MODE_ORIGINAL);
                serial1Send(A_MODE_ORIGINAL);
              break;
            }
          break;

          case W_SPECTRAL_LIGHTS_ON:
            spectralLightsOn();
          break;

          case W_SPECTRAL_LIGHTS_OFF:
            spectralLightsOff();
          break;

          case W_SPECTRAL_INNER_CYCLOTRON_CUSTOM_DECREASE:
            if(i_spectral_cyclotron_inner_custom_colour > 1 && i_spectral_cyclotron_inner_custom_saturation > 253) {
              i_spectral_cyclotron_inner_custom_colour--;
            }
            else {
              i_spectral_cyclotron_inner_custom_colour = 1;

              if(i_spectral_cyclotron_inner_custom_saturation > 1) {
                i_spectral_cyclotron_inner_custom_saturation--;
              }
              else {
                i_spectral_cyclotron_inner_custom_saturation = 1;
              }
            }

            spectralLightsOn();
          break;

          case W_SPECTRAL_CYCLOTRON_CUSTOM_DECREASE:
            if(i_spectral_cyclotron_custom_colour > 1 && i_spectral_cyclotron_custom_saturation > 253) {
              i_spectral_cyclotron_custom_colour--;
            }
            else {
              i_spectral_cyclotron_custom_colour = 1;

              if(i_spectral_cyclotron_custom_saturation > 1) {
                i_spectral_cyclotron_custom_saturation--;
              }
              else {
                i_spectral_cyclotron_custom_saturation = 1;
              }
            }

            spectralLightsOn();
          break;

          case W_SPECTRAL_POWERCELL_CUSTOM_DECREASE:
            if(i_spectral_powercell_custom_colour > 1 && i_spectral_powercell_custom_saturation > 253) {
              i_spectral_powercell_custom_colour--;
            }
            else {
              i_spectral_powercell_custom_colour = 1;

              if(i_spectral_powercell_custom_saturation > 1) {
                i_spectral_powercell_custom_saturation--;
              }
              else {
                i_spectral_powercell_custom_saturation = 1;
              }
            }

            spectralLightsOn();
          break;

          case W_SPECTRAL_POWERCELL_CUSTOM_INCREASE:
            if(i_spectral_powercell_custom_saturation < 254) {
              i_spectral_powercell_custom_saturation++;

              if(i_spectral_powercell_custom_saturation > 253) {
                i_spectral_powercell_custom_saturation = 254;
              }
            }
            else if(i_spectral_powercell_custom_colour < 253) {
              i_spectral_powercell_custom_colour++;
            }
            else {
              i_spectral_powercell_custom_colour = 254;
            }

            spectralLightsOn();
          break;

          case W_SPECTRAL_CYCLOTRON_CUSTOM_INCREASE:
            if(i_spectral_cyclotron_custom_saturation < 254) {
              i_spectral_cyclotron_custom_saturation++;

              if(i_spectral_cyclotron_custom_saturation > 253) {
                i_spectral_cyclotron_custom_saturation = 254;
              }
            }
            else if(i_spectral_cyclotron_custom_colour < 253) {
              i_spectral_cyclotron_custom_colour++;
            }
            else {
              i_spectral_cyclotron_custom_colour = 254;

              if(i_spectral_cyclotron_custom_saturation < 253) {
                i_spectral_cyclotron_custom_saturation++;
              }
              else {
                i_spectral_cyclotron_custom_saturation = 254;
              }
            }

            spectralLightsOn();
          break;

          case W_SPECTRAL_INNER_CYCLOTRON_CUSTOM_INCREASE:
            if(i_spectral_cyclotron_inner_custom_saturation < 254) {
              i_spectral_cyclotron_inner_custom_saturation++;

              if(i_spectral_cyclotron_inner_custom_saturation > 253) {
                i_spectral_cyclotron_inner_custom_saturation = 254;
              }
            }
            else if(i_spectral_cyclotron_inner_custom_colour < 253) {
              i_spectral_cyclotron_inner_custom_colour++;
            }
            else {
              i_spectral_cyclotron_inner_custom_colour = 254;

              if(i_spectral_cyclotron_inner_custom_saturation < 253) {
                i_spectral_cyclotron_inner_custom_saturation++;
              }
              else {
                i_spectral_cyclotron_inner_custom_saturation = 254;
              }
            }

            spectralLightsOn();
          break;

          case W_DIMMING_TOGGLE:
            switch(pack_dim_toggle) {
              case DIM_CYCLOTRON:
                pack_dim_toggle = DIM_INNER_CYCLOTRON;

                stopEffect(S_VOICE_POWERCELL_BRIGHTNESS);
                stopEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
                stopEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);

                playEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);

                packSerialSend(P_INNER_CYCLOTRON_DIMMING);
              break;

              case DIM_INNER_CYCLOTRON:
                pack_dim_toggle = DIM_POWERCELL;

                stopEffect(S_VOICE_POWERCELL_BRIGHTNESS);
                stopEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
                stopEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);

                playEffect(S_VOICE_POWERCELL_BRIGHTNESS);

                packSerialSend(P_POWERCELL_DIMMING);
              break;

              case DIM_POWERCELL:
              default:
                pack_dim_toggle = DIM_CYCLOTRON;

                stopEffect(S_VOICE_POWERCELL_BRIGHTNESS);
                stopEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
                stopEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);

                playEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);

                packSerialSend(P_CYCLOTRON_DIMMING);
              break;
            }
          break;

          case W_DIMMING_INCREASE:
            switch(pack_dim_toggle) {
              case DIM_CYCLOTRON:
                if(i_cyclotron_brightness < 100) {
                  if(i_cyclotron_brightness + 10 > 100) {
                    i_cyclotron_brightness = 100;
                  }
                  else {
                    i_cyclotron_brightness = i_cyclotron_brightness + 10;
                  }

                  resetCyclotronState();

                  packSerialSend(P_DIMMING);

                  stopEffect(S_BEEPS);
                  playEffect(S_BEEPS);
                }
                else {
                  // Already at 100%, indicate as such.
                  stopEffect(S_BEEPS_ALT);
                  playEffect(S_BEEPS_ALT);
                }
              break;

              case DIM_INNER_CYCLOTRON:
                if(i_cyclotron_inner_brightness < 100) {
                  if(i_cyclotron_inner_brightness + 10 > 100) {
                    i_cyclotron_inner_brightness = 100;
                  }
                  else {
                    i_cyclotron_inner_brightness = i_cyclotron_inner_brightness + 10;
                  }

                  stopEffect(S_BEEPS);
                  playEffect(S_BEEPS);
                }
                else {
                  // Already at 100%, indicate as such.
                  stopEffect(S_BEEPS_ALT);
                  playEffect(S_BEEPS_ALT);
                }
              break;

              case DIM_POWERCELL:
              default:
                if(i_powercell_brightness < 100) {
                  if(i_powercell_brightness + 10 > 100) {
                    i_powercell_brightness = 100;
                  }
                  else {
                    i_powercell_brightness = i_powercell_brightness + 10;
                  }

                  // Reset the Power Cell.
                  powercellDraw();

                  stopEffect(S_BEEPS);
                  playEffect(S_BEEPS);
                }
                else {
                  // Already at 100%, indicate as such.
                  stopEffect(S_BEEPS_ALT);
                  playEffect(S_BEEPS_ALT);
                }
              break;
            }
          break;

          case W_DIMMING_DECREASE:
            switch(pack_dim_toggle) {
              case DIM_CYCLOTRON:
                if(i_cyclotron_brightness > 0) {
                  if(i_cyclotron_brightness - 10 < 0) {
                    i_cyclotron_brightness = 0;
                  }
                  else {
                    i_cyclotron_brightness = i_cyclotron_brightness - 10;
                  }

                  resetCyclotronState();

                  packSerialSend(P_DIMMING);

                  stopEffect(S_BEEPS);
                  playEffect(S_BEEPS);
                }
                else {
                  // Already at 0%, indicate as such.
                  stopEffect(S_BEEPS_ALT);
                  playEffect(S_BEEPS_ALT);
                }
              break;

              case DIM_INNER_CYCLOTRON:
                if(i_cyclotron_inner_brightness > 0) {
                  if(i_cyclotron_inner_brightness - 10 < 0) {
                    i_cyclotron_inner_brightness = 0;
                  }
                  else {
                    i_cyclotron_inner_brightness = i_cyclotron_inner_brightness - 10;
                  }

                  stopEffect(S_BEEPS);
                  playEffect(S_BEEPS);
                }
                else {
                  // Already at 0%, indicate as such.
                  stopEffect(S_BEEPS_ALT);
                  playEffect(S_BEEPS_ALT);
                }
              break;

              case DIM_POWERCELL:
              default:
                if(i_powercell_brightness > 0) {
                  if(i_powercell_brightness - 10 < 0) {
                    i_powercell_brightness = 0;
                  }
                  else {
                    i_powercell_brightness = i_powercell_brightness - 10;
                  }

                  // Reset the Power Cell.
                  powercellDraw();

                  stopEffect(S_BEEPS);
                  playEffect(S_BEEPS);
                }
                else {
                  // Already at 0%, indicate as such.
                  stopEffect(S_BEEPS_ALT);
                  playEffect(S_BEEPS_ALT);
                }
              break;
            }
          break;

          case W_CLEAR_CONFIG_EEPROM_SETTINGS:
            stopEffect(S_VOICE_EEPROM_ERASE);
            playEffect(S_VOICE_EEPROM_ERASE);

            clearConfigEEPROM();
          break;

          case W_SAVE_CONFIG_EEPROM_SETTINGS:
            stopEffect(S_VOICE_EEPROM_SAVE);
            playEffect(S_VOICE_EEPROM_SAVE);

            saveConfigEEPROM();
          break;

          case W_CLEAR_LED_EEPROM_SETTINGS:
            clearLedEEPROM();

            stopEffect(S_VOICE_EEPROM_ERASE);
            playEffect(S_VOICE_EEPROM_ERASE);
          break;

          case W_SAVE_LED_EEPROM_SETTINGS:
            saveLedEEPROM();

            stopEffect(S_VOICE_EEPROM_SAVE);
            playEffect(S_VOICE_EEPROM_SAVE);
          break;

          case W_TOGGLE_INNER_CYCLOTRON_LEDS:
            stopEffect(S_VOICE_INNER_CYCLOTRON_35);
            stopEffect(S_VOICE_INNER_CYCLOTRON_24);
            stopEffect(S_VOICE_INNER_CYCLOTRON_23);
            stopEffect(S_VOICE_INNER_CYCLOTRON_12);

            switch(i_inner_cyclotron_num_leds) {
              case 12:
                // Switch to 23 LEDs.
                i_inner_cyclotron_num_leds = 23;
                i_2021_inner_delay = 8;
                i_1984_inner_delay = 12;

                playEffect(S_VOICE_INNER_CYCLOTRON_23);
                packSerialSend(P_INNER_CYCLOTRON_LEDS_23);
              break;

              case 23:
                // Switch to 24 LEDs.
                i_inner_cyclotron_num_leds = 24;
                i_2021_inner_delay = 8;
                i_1984_inner_delay = 12;

                playEffect(S_VOICE_INNER_CYCLOTRON_24);
                packSerialSend(P_INNER_CYCLOTRON_LEDS_24);
              break;

              case 24:
              default:
                // Switch to 35 LEDs.
                i_inner_cyclotron_num_leds = 35;
                i_2021_inner_delay = 5;
                i_1984_inner_delay = 9;

                playEffect(S_VOICE_INNER_CYCLOTRON_35);
                packSerialSend(P_INNER_CYCLOTRON_LEDS_35);
              break;

              case 35:
                // Switch to 12 LEDs.
                i_inner_cyclotron_num_leds = 12;
                i_2021_inner_delay = 12;
                i_1984_inner_delay = 15;

                playEffect(S_VOICE_INNER_CYCLOTRON_12);
                packSerialSend(P_INNER_CYCLOTRON_LEDS_12);
              break;
            }

            updateProtonPackLEDCounts();

            spectralLightsOff();
            spectralLightsOn();
          break;

          case W_TOGGLE_POWERCELL_LEDS:
            stopEffect(S_VOICE_POWERCELL_15);
            stopEffect(S_VOICE_POWERCELL_13);

            switch(i_powercell_leds) {
                case HASLAB_POWERCELL_LED_COUNT:
                // Switch to 15 Power Cell LEDs.
                i_powercell_leds = FRUTTO_POWERCELL_LED_COUNT;
                i_powercell_delay_1984 = 60;
                i_powercell_delay_2021 = 34;

                playEffect(S_VOICE_POWERCELL_15);
                packSerialSend(P_POWERCELL_LEDS_15);
              break;

              case FRUTTO_POWERCELL_LED_COUNT:
                default:
                // Switch to 13 Power Cell LEDs.
                i_powercell_leds = HASLAB_POWERCELL_LED_COUNT;
                i_powercell_delay_1984 = 75;
                i_powercell_delay_2021 = 40;

                playEffect(S_VOICE_POWERCELL_13);
                packSerialSend(P_POWERCELL_LEDS_13);
              break;
            }

            updateProtonPackLEDCounts();

            spectralLightsOff();
            spectralLightsOn();
          break;

          case W_TOGGLE_CYCLOTRON_LEDS:
            stopEffect(S_VOICE_CYCLOTRON_40);
            stopEffect(S_VOICE_CYCLOTRON_20);
            stopEffect(S_VOICE_CYCLOTRON_12);

            switch(i_cyclotron_leds) {
              case OUTER_CYCLOTRON_LED_MAX:
                // Switch to 20 LEDs. Frutto Technology.
                i_cyclotron_leds = FRUTTO_CYCLOTRON_LED_COUNT;

                resetCyclotronState();

                playEffect(S_VOICE_CYCLOTRON_20);
                packSerialSend(P_CYCLOTRON_LEDS_20);
              break;

              case FRUTTO_CYCLOTRON_LED_COUNT:
              default:
                // Switch to 12 LEDs. Default HasLab.
                i_cyclotron_leds = HASLAB_CYCLOTRON_LED_COUNT;

                resetCyclotronState();

                playEffect(S_VOICE_CYCLOTRON_12);
                packSerialSend(P_CYCLOTRON_LEDS_12);
              break;

              case HASLAB_CYCLOTRON_LED_COUNT:
                // Switch to 40 LEDs.
                i_cyclotron_leds = OUTER_CYCLOTRON_LED_MAX;

                resetCyclotronState();

                playEffect(S_VOICE_CYCLOTRON_40);
                packSerialSend(P_CYCLOTRON_LEDS_40);
              break;
            }

            updateProtonPackLEDCounts();

            spectralLightsOff();
            spectralLightsOn();
          break;

          case W_TOGGLE_RGB_INNER_CYCLOTRON_LEDS:
            stopEffect(S_VOICE_RGB_INNER_CYCLOTRON);
            stopEffect(S_VOICE_GRB_INNER_CYCLOTRON);

            if(b_grb_cyclotron == true) {
              b_grb_cyclotron = false;
              playEffect(S_VOICE_RGB_INNER_CYCLOTRON);

              packSerialSend(P_RGB_INNER_CYCLOTRON_LEDS);
            }
            else {
              b_grb_cyclotron = true;
              playEffect(S_VOICE_GRB_INNER_CYCLOTRON);

              packSerialSend(P_GRB_INNER_CYCLOTRON_LEDS);
            }

            if(b_spectral_lights_on == true) {
              spectralLightsOn();
            }
          break;

          case W_EEPROM_LED_MENU:
            stopEffect(S_BEEPS_BARGRAPH);
            playEffect(S_BEEPS_BARGRAPH);

            stopEffect(S_EEPROM_LED_MENU);
            playEffect(S_EEPROM_LED_MENU);
          break;

          case W_EEPROM_CONFIG_MENU:
            stopEffect(S_BEEPS_BARGRAPH);
            playEffect(S_BEEPS_BARGRAPH);

            stopEffect(S_EEPROM_CONFIG_MENU);
            playEffect(S_EEPROM_CONFIG_MENU);
          break;

          case W_QUICK_VENT_DISABLED:
            stopEffect(S_VOICE_QUICK_VENT_ENABLED);
            stopEffect(S_VOICE_QUICK_VENT_DISABLED);

            playEffect(S_VOICE_QUICK_VENT_DISABLED);
          break;

          case W_QUICK_VENT_ENABLED:
            stopEffect(S_VOICE_QUICK_VENT_ENABLED);
            stopEffect(S_VOICE_QUICK_VENT_DISABLED);

            playEffect(S_VOICE_QUICK_VENT_ENABLED);
          break;

          case W_BOOTUP_ERRORS_DISABLED:
            stopEffect(S_VOICE_BOOTUP_ERRORS_DISABLED);
            stopEffect(S_VOICE_BOOTUP_ERRORS_ENABLED);

            playEffect(S_VOICE_BOOTUP_ERRORS_DISABLED);
          break;

          case W_BOOTUP_ERRORS_ENABLED:
            stopEffect(S_VOICE_BOOTUP_ERRORS_ENABLED);
            stopEffect(S_VOICE_BOOTUP_ERRORS_DISABLED);

            playEffect(S_VOICE_BOOTUP_ERRORS_ENABLED);
          break;

          case W_VENT_LIGHT_INTENSITY_ENABLED:
            stopEffect(S_VOICE_VENT_LIGHT_INTENSITY_DISABLED);
            stopEffect(S_VOICE_VENT_LIGHT_INTENSITY_ENABLED);

            playEffect(S_VOICE_VENT_LIGHT_INTENSITY_ENABLED);
          break;

          case W_VENT_LIGHT_INTENSITY_DISABLED:
            stopEffect(S_VOICE_VENT_LIGHT_INTENSITY_DISABLED);
            stopEffect(S_VOICE_VENT_LIGHT_INTENSITY_ENABLED);

            playEffect(S_VOICE_VENT_LIGHT_INTENSITY_DISABLED);
          break;

          case W_DEMO_LIGHT_MODE_TOGGLE:
            if(b_demo_light_mode == true) {
              b_demo_light_mode = false;

              stopEffect(S_VOICE_DEMO_LIGHT_MODE_DISABLED);
              stopEffect(S_VOICE_DEMO_LIGHT_MODE_ENABLED);

              playEffect(S_VOICE_DEMO_LIGHT_MODE_DISABLED);

              packSerialSend(P_DEMO_LIGHT_MODE_DISABLED);
            }
            else {
              b_demo_light_mode = true;

              stopEffect(S_VOICE_DEMO_LIGHT_MODE_ENABLED);
              stopEffect(S_VOICE_DEMO_LIGHT_MODE_DISABLED);

              playEffect(S_VOICE_DEMO_LIGHT_MODE_ENABLED);

              packSerialSend(P_DEMO_LIGHT_MODE_ENABLED);
            }
          break;

          case W_OVERHEAT_LEVEL_5_ENABLED:
            stopEffect(S_VOICE_OVERHEAT_LEVEL_5_ENABLED);
            stopEffect(S_VOICE_OVERHEAT_LEVEL_5_DISABLED);
            playEffect(S_VOICE_OVERHEAT_LEVEL_5_ENABLED);
          break;

          case W_OVERHEAT_LEVEL_4_ENABLED:
            stopEffect(S_VOICE_OVERHEAT_LEVEL_4_ENABLED);
            stopEffect(S_VOICE_OVERHEAT_LEVEL_4_DISABLED);
            playEffect(S_VOICE_OVERHEAT_LEVEL_4_ENABLED);
          break;

          case W_OVERHEAT_LEVEL_3_ENABLED:
            stopEffect(S_VOICE_OVERHEAT_LEVEL_3_ENABLED);
            stopEffect(S_VOICE_OVERHEAT_LEVEL_3_DISABLED);
            playEffect(S_VOICE_OVERHEAT_LEVEL_3_ENABLED);
          break;

          case W_OVERHEAT_LEVEL_2_ENABLED:
            stopEffect(S_VOICE_OVERHEAT_LEVEL_2_ENABLED);
            stopEffect(S_VOICE_OVERHEAT_LEVEL_2_DISABLED);
            playEffect(S_VOICE_OVERHEAT_LEVEL_2_ENABLED);
          break;

          case W_OVERHEAT_LEVEL_1_ENABLED:
            stopEffect(S_VOICE_OVERHEAT_LEVEL_1_ENABLED);
            stopEffect(S_VOICE_OVERHEAT_LEVEL_1_DISABLED);
            playEffect(S_VOICE_OVERHEAT_LEVEL_1_ENABLED);
          break;

          case W_OVERHEAT_LEVEL_5_DISABLED:
            stopEffect(S_VOICE_OVERHEAT_LEVEL_5_DISABLED);
            stopEffect(S_VOICE_OVERHEAT_LEVEL_5_ENABLED);
            playEffect(S_VOICE_OVERHEAT_LEVEL_5_DISABLED);
          break;

          case W_OVERHEAT_LEVEL_4_DISABLED:
            stopEffect(S_VOICE_OVERHEAT_LEVEL_4_DISABLED);
            stopEffect(S_VOICE_OVERHEAT_LEVEL_4_ENABLED);
            playEffect(S_VOICE_OVERHEAT_LEVEL_4_DISABLED);
          break;

          case W_OVERHEAT_LEVEL_3_DISABLED:
            stopEffect(S_VOICE_OVERHEAT_LEVEL_3_DISABLED);
            stopEffect(S_VOICE_OVERHEAT_LEVEL_3_ENABLED);
            playEffect(S_VOICE_OVERHEAT_LEVEL_3_DISABLED);
          break;

          case W_OVERHEAT_LEVEL_2_DISABLED:
            stopEffect(S_VOICE_OVERHEAT_LEVEL_2_DISABLED);
            stopEffect(S_VOICE_OVERHEAT_LEVEL_2_ENABLED);
            playEffect(S_VOICE_OVERHEAT_LEVEL_2_DISABLED);
          break;

          case W_OVERHEAT_LEVEL_1_DISABLED:
            stopEffect(S_VOICE_OVERHEAT_LEVEL_1_DISABLED);
            stopEffect(S_VOICE_OVERHEAT_LEVEL_1_ENABLED);
            playEffect(S_VOICE_OVERHEAT_LEVEL_1_DISABLED);
          break;

          case W_CONTINUOUS_SMOKE_TOGGLE_5:
            if(b_smoke_continuous_mode_5 == true) {
              b_smoke_continuous_mode_5 = false;

              stopEffect(S_VOICE_CONTINUOUS_SMOKE_5_DISABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_5_ENABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_5_DISABLED);

              packSerialSend(P_CONTINUOUS_SMOKE_5_DISABLED);
            }
            else {
              b_smoke_continuous_mode_5 = true;

              stopEffect(S_VOICE_CONTINUOUS_SMOKE_5_ENABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_5_DISABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_5_ENABLED);

              packSerialSend(P_CONTINUOUS_SMOKE_5_ENABLED);
            }

            resetContinuousSmoke();
          break;

          case W_CONTINUOUS_SMOKE_TOGGLE_4:
            if(b_smoke_continuous_mode_4 == true) {
              b_smoke_continuous_mode_4 = false;

              stopEffect(S_VOICE_CONTINUOUS_SMOKE_4_DISABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_4_ENABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_4_DISABLED);

              packSerialSend(P_CONTINUOUS_SMOKE_4_DISABLED);
            }
            else {
              b_smoke_continuous_mode_4 = true;

              stopEffect(S_VOICE_CONTINUOUS_SMOKE_4_ENABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_4_DISABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_4_ENABLED);

              packSerialSend(P_CONTINUOUS_SMOKE_4_ENABLED);
            }

            resetContinuousSmoke();
          break;

          case W_CONTINUOUS_SMOKE_TOGGLE_3:
            if(b_smoke_continuous_mode_3 == true) {
              b_smoke_continuous_mode_3 = false;

              stopEffect(S_VOICE_CONTINUOUS_SMOKE_3_DISABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_3_ENABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_3_DISABLED);

              packSerialSend(P_CONTINUOUS_SMOKE_3_DISABLED);
            }
            else {
              b_smoke_continuous_mode_3 = true;

              stopEffect(S_VOICE_CONTINUOUS_SMOKE_3_ENABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_3_DISABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_3_ENABLED);

              packSerialSend(P_CONTINUOUS_SMOKE_3_ENABLED);
            }

            resetContinuousSmoke();
          break;

          case W_CONTINUOUS_SMOKE_TOGGLE_2:
            if(b_smoke_continuous_mode_2 == true) {
              b_smoke_continuous_mode_2 = false;

              stopEffect(S_VOICE_CONTINUOUS_SMOKE_2_DISABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_2_ENABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_2_DISABLED);

              packSerialSend(P_CONTINUOUS_SMOKE_2_DISABLED);
            }
            else {
              b_smoke_continuous_mode_2 = true;

              stopEffect(S_VOICE_CONTINUOUS_SMOKE_2_ENABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_2_DISABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_2_ENABLED);

              packSerialSend(P_CONTINUOUS_SMOKE_2_ENABLED);
            }

            resetContinuousSmoke();
          break;

          case W_CONTINUOUS_SMOKE_TOGGLE_1:
            if(b_smoke_continuous_mode_1 == true) {
              b_smoke_continuous_mode_1 = false;
              b_smoke_continuous_mode[0] = false;
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_1_DISABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_1_ENABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_1_DISABLED);

              packSerialSend(P_CONTINUOUS_SMOKE_1_DISABLED);
            }
            else {
              b_smoke_continuous_mode_1 = true;

              stopEffect(S_VOICE_CONTINUOUS_SMOKE_1_ENABLED);
              stopEffect(S_VOICE_CONTINUOUS_SMOKE_1_DISABLED);
              playEffect(S_VOICE_CONTINUOUS_SMOKE_1_ENABLED);

              packSerialSend(P_CONTINUOUS_SMOKE_1_ENABLED);
            }

            resetContinuousSmoke();
          break;

          case W_BARREL_LEDS_48:
            stopEffect(S_VOICE_BARREL_LED_5);
            stopEffect(S_VOICE_BARREL_LED_48);
            stopEffect(S_VOICE_BARREL_LED_60);

            playEffect(S_VOICE_BARREL_LED_48);
          break;

          case W_BARREL_LEDS_60:
            stopEffect(S_VOICE_BARREL_LED_5);
            stopEffect(S_VOICE_BARREL_LED_48);
            stopEffect(S_VOICE_BARREL_LED_60);

            playEffect(S_VOICE_BARREL_LED_60);
          break;

          case W_BARREL_LEDS_5:
            stopEffect(S_VOICE_BARREL_LED_5);
            stopEffect(S_VOICE_BARREL_LED_48);
            stopEffect(S_VOICE_BARREL_LED_60);

            playEffect(S_VOICE_BARREL_LED_5);
          break;

          case W_BARGRAPH_INVERTED:
            stopEffect(S_VOICE_BARGRAPH_INVERTED);
            stopEffect(S_VOICE_BARGRAPH_NOT_INVERTED);

            playEffect(S_VOICE_BARGRAPH_INVERTED);
          break;

          case W_BARGRAPH_NOT_INVERTED:
            stopEffect(S_VOICE_BARGRAPH_NOT_INVERTED);
            stopEffect(S_VOICE_BARGRAPH_INVERTED);

            playEffect(S_VOICE_BARGRAPH_NOT_INVERTED);
          break;

          case W_DEFAULT_BARGRAPH:
            stopEffect(S_VOICE_DEFAULT_BARGRAPH);
            stopEffect(S_VOICE_MODE_ORIGINAL_BARGRAPH);
            stopEffect(S_VOICE_SUPER_HERO_BARGRAPH);

            playEffect(S_VOICE_DEFAULT_BARGRAPH);
          break;

          case W_MODE_ORIGINAL_BARGRAPH:
            stopEffect(S_VOICE_DEFAULT_BARGRAPH);
            stopEffect(S_VOICE_MODE_ORIGINAL_BARGRAPH);
            stopEffect(S_VOICE_SUPER_HERO_BARGRAPH);

            playEffect(S_VOICE_MODE_ORIGINAL_BARGRAPH);
          break;

          case W_SUPER_HERO_BARGRAPH:
            stopEffect(S_VOICE_DEFAULT_BARGRAPH);
            stopEffect(S_VOICE_SUPER_HERO_BARGRAPH);
            stopEffect(S_VOICE_MODE_ORIGINAL_BARGRAPH);

            playEffect(S_VOICE_SUPER_HERO_BARGRAPH);
          break;

          case W_SUPER_HERO_FIRING_ANIMATIONS_BARGRAPH:
            stopEffect(S_VOICE_SUPER_HERO_FIRING_ANIMATIONS_BARGRAPH);
            stopEffect(S_VOICE_DEFAULT_FIRING_ANIMATIONS_BARGRAPH);
            stopEffect(S_VOICE_MODE_ORIGINAL_FIRING_ANIMATIONS_BARGRAPH);

            playEffect(S_VOICE_SUPER_HERO_FIRING_ANIMATIONS_BARGRAPH);
          break;

          case W_MODE_ORIGINAL_FIRING_ANIMATIONS_BARGRAPH:
            stopEffect(S_VOICE_DEFAULT_FIRING_ANIMATIONS_BARGRAPH);
            stopEffect(S_VOICE_MODE_ORIGINAL_FIRING_ANIMATIONS_BARGRAPH);
            stopEffect(S_VOICE_SUPER_HERO_FIRING_ANIMATIONS_BARGRAPH);

            playEffect(S_VOICE_MODE_ORIGINAL_FIRING_ANIMATIONS_BARGRAPH);
          break;

          case W_DEFAULT_FIRING_ANIMATIONS_BARGRAPH:
            stopEffect(S_VOICE_DEFAULT_FIRING_ANIMATIONS_BARGRAPH);
            stopEffect(S_VOICE_MODE_ORIGINAL_FIRING_ANIMATIONS_BARGRAPH);
            stopEffect(S_VOICE_SUPER_HERO_FIRING_ANIMATIONS_BARGRAPH);

            playEffect(S_VOICE_DEFAULT_FIRING_ANIMATIONS_BARGRAPH);
          break;

          case W_NEUTRONA_WAND_1984_MODE:
            stopEffect(S_VOICE_NEUTRONA_WAND_1984);
            stopEffect(S_VOICE_NEUTRONA_WAND_1989);
            stopEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
            stopEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
            stopEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);

            playEffect(S_VOICE_NEUTRONA_WAND_1984);
          break;

          case W_NEUTRONA_WAND_1989_MODE:
            stopEffect(S_VOICE_NEUTRONA_WAND_1984);
            stopEffect(S_VOICE_NEUTRONA_WAND_1989);
            stopEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
            stopEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
            stopEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);

            playEffect(S_VOICE_NEUTRONA_WAND_1989);
          break;

          case W_NEUTRONA_WAND_AFTERLIFE_MODE:
            stopEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
            stopEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
            stopEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);
            stopEffect(S_VOICE_NEUTRONA_WAND_1984);
            stopEffect(S_VOICE_NEUTRONA_WAND_1989);

            playEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);
          break;

          case W_NEUTRONA_WAND_FROZEN_EMPIRE_MODE:
            stopEffect(S_VOICE_NEUTRONA_WAND_1984);
            stopEffect(S_VOICE_NEUTRONA_WAND_1989);
            stopEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
            stopEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
            stopEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);

            playEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
          break;

          case W_NEUTRONA_WAND_DEFAULT_MODE:
            stopEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
            stopEffect(S_VOICE_NEUTRONA_WAND_FROZEN_EMPIRE);
            stopEffect(S_VOICE_NEUTRONA_WAND_AFTERLIFE);
            stopEffect(S_VOICE_NEUTRONA_WAND_1984);
            stopEffect(S_VOICE_NEUTRONA_WAND_1989);

            playEffect(S_VOICE_NEUTRONA_WAND_DEFAULT_MODE);
          break;

          case W_OVERHEAT_STROBE_TOGGLE:
            if(b_overheat_strobe == true) {
              b_overheat_strobe = false;

              stopEffect(S_VOICE_OVERHEAT_STROBE_DISABLED);
              stopEffect(S_VOICE_OVERHEAT_STROBE_ENABLED);
              playEffect(S_VOICE_OVERHEAT_STROBE_DISABLED);

              packSerialSend(P_OVERHEAT_STROBE_DISABLED);
            }
            else {
              b_overheat_strobe = true;

              stopEffect(S_VOICE_OVERHEAT_STROBE_ENABLED);
              stopEffect(S_VOICE_OVERHEAT_STROBE_DISABLED);
              playEffect(S_VOICE_OVERHEAT_STROBE_ENABLED);

              packSerialSend(P_OVERHEAT_STROBE_ENABLED);
            }
          break;

          case W_OVERHEAT_LIGHTS_OFF_TOGGLE:
            if(b_overheat_lights_off == true) {
              b_overheat_lights_off = false;

              stopEffect(S_VOICE_OVERHEAT_LIGHTS_OFF_DISABLED);
              stopEffect(S_VOICE_OVERHEAT_LIGHTS_OFF_ENABLED);
              playEffect(S_VOICE_OVERHEAT_LIGHTS_OFF_DISABLED);

              packSerialSend(P_OVERHEAT_LIGHTS_OFF_DISABLED);
            }
            else {
              b_overheat_lights_off = true;

              stopEffect(S_VOICE_OVERHEAT_LIGHTS_OFF_ENABLED);
              stopEffect(S_VOICE_OVERHEAT_LIGHTS_OFF_DISABLED);
              playEffect(S_VOICE_OVERHEAT_LIGHTS_OFF_ENABLED);

              packSerialSend(P_OVERHEAT_LIGHTS_OFF_ENABLED);
            }
          break;

          case W_OVERHEAT_SYNC_TO_FAN_TOGGLE:
            if(b_overheat_sync_to_fan == true) {
              b_overheat_sync_to_fan = false;

              stopEffect(S_VOICE_OVERHEAT_FAN_SYNC_DISABLED);
              stopEffect(S_VOICE_OVERHEAT_FAN_SYNC_ENABLED);
              playEffect(S_VOICE_OVERHEAT_FAN_SYNC_DISABLED);

              packSerialSend(P_OVERHEAT_SYNC_FAN_DISABLED);
            }
            else {
              b_overheat_sync_to_fan = true;

              stopEffect(S_VOICE_OVERHEAT_FAN_SYNC_ENABLED);
              stopEffect(S_VOICE_OVERHEAT_FAN_SYNC_DISABLED);
              playEffect(S_VOICE_OVERHEAT_FAN_SYNC_ENABLED);

              packSerialSend(P_OVERHEAT_SYNC_FAN_ENABLED);
            }
          break;

          case W_YEAR_MODES_CYCLE_EEPROM:
            if(b_switch_mode_override == true) {
              if(SYSTEM_YEAR_TEMP == SYSTEM_AFTERLIFE) {
                b_switch_mode_override = false;

                stopEffect(S_VOICE_YEAR_MODE_DEFAULT);
                stopEffect(S_VOICE_FROZEN_EMPIRE);
                stopEffect(S_VOICE_AFTERLIFE);
                stopEffect(S_VOICE_1984);
                stopEffect(S_VOICE_1989);
                playEffect(S_VOICE_YEAR_MODE_DEFAULT);

                packSerialSend(P_YEAR_MODE_DEFAULT);

                // 1 = toggle switch, 2 = 1984, 3 = 1989, 4 = Afterlife, 5 = Frozen Empire.
                SYSTEM_EEPROM_YEAR = SYSTEM_TOGGLE_SWITCH;
              }
              else {
                toggleYearModes();
              }
            }
            else {
              toggleYearModes();

              // Turn on the year mode override flag. This resets when you flip the year mode toggle switch on the pack.
              b_switch_mode_override = true;
            }

            if(b_switch_mode_override == true) {
              switch(SYSTEM_YEAR_TEMP) {
                case SYSTEM_1984:
                  SYSTEM_EEPROM_YEAR = SYSTEM_1984;
                break;

                case SYSTEM_1989:
                  SYSTEM_EEPROM_YEAR = SYSTEM_1989;
                break;

                case SYSTEM_FROZEN_EMPIRE:
                  SYSTEM_EEPROM_YEAR = SYSTEM_FROZEN_EMPIRE;
                break;

                case SYSTEM_AFTERLIFE:
                default:
                  SYSTEM_EEPROM_YEAR = SYSTEM_AFTERLIFE;
                break;
              }
            }
          break;

          case W_MUSIC_NEXT_TRACK:
            musicNextTrack();
          break;

          case W_MUSIC_PREV_TRACK:
            musicPrevTrack();
          break;

          case W_MUSIC_PLAY_TRACK:
            // Music track number to be played.
            if(i_music_count > 0 && recvCmdW.d1 >= i_music_track_start) {
              if(b_playing_music == true) {
                stopMusic(); // Stops current track before change.

                // Only update after the music is stopped.
                i_current_music_track = recvCmdW.d1;

                // Play the appropriate track on pack and wand, and notify the serial1 device.
                playMusic();
              }
              else {
                i_current_music_track = recvCmdW.d1;
              }
            }
          break;

          case W_COM_SOUND_NUMBER:
            if(recvCmdW.d1 > 0 && recvCmdW.d1 < i_music_track_start) {
              // The Neutrona Wand is telling us to play a sound effect only (S_1 through S_60).
              stopEffect(recvCmdW.d1 + 1);

              if(recvCmdW.d1 - 1 > 0) {
                stopEffect(recvCmdW.d1 - 1);
              }

              stopEffect(recvCmdW.d1);
              playEffect(recvCmdW.d1);
            }
          break;

          default:
            // No-op for all other actions.
          break;
        }

        // Handle data payloads.
        switch(recvDataW.m) {
          case W_SEND_PREFERENCES_WAND:
            // Preferences are received from the wand.
            wandConfig.ledWandCount = recvDataW.d[0];
            wandConfig.ledWandHue = recvDataW.d[1];
            wandConfig.ledWandSat = recvDataW.d[2];
            wandConfig.spectralModeEnabled = recvDataW.d[3];
            wandConfig.spectralHolidayMode = recvDataW.d[4];
            wandConfig.overheatEnabled = recvDataW.d[5];
            wandConfig.defaultFiringModeCTS = recvDataW.d[6];
            wandConfig.defaultFiringModeMix = recvDataW.d[7];
            wandConfig.wandSoundsToPack = recvDataW.d[8];
            wandConfig.quickVenting = recvDataW.d[9];
            wandConfig.autoVentLight = recvDataW.d[10];
            wandConfig.wandBeepLoop = recvDataW.d[11];
            wandConfig.wandBootError = recvDataW.d[12];
            wandConfig.defaultYearModeWand = recvDataW.d[13];
            wandConfig.defaultYearModeCTS = recvDataW.d[14];
            wandConfig.invertWandBargraph = recvDataW.d[15];
            wandConfig.bargraphOverheatBlink = recvDataW.d[16];
            wandConfig.bargraphIdleAnimation = recvDataW.d[17];
            wandConfig.bargraphFireAnimation = recvDataW.d[18];

            // Send the EEPROM preferences just returned by the wand.
            serial1SendData(A_SEND_PREFERENCES_WAND);
          break;

          case W_SEND_PREFERENCES_SMOKE:
            // Preferences are received from the wand.
            wandConfig.overheatLevel5 = recvDataW.d[0];
            wandConfig.overheatLevel4 = recvDataW.d[1];
            wandConfig.overheatLevel3 = recvDataW.d[2];
            wandConfig.overheatLevel2 = recvDataW.d[3];
            wandConfig.overheatLevel1 = recvDataW.d[4];
            wandConfig.overheatDelay5 = recvDataW.d[5];
            wandConfig.overheatDelay4 = recvDataW.d[6];
            wandConfig.overheatDelay3 = recvDataW.d[7];
            wandConfig.overheatDelay2 = recvDataW.d[8];
            wandConfig.overheatDelay1 = recvDataW.d[9];

            // Send the EEPROM preferences just returned by the wand.
            // This data will combine with the pack's smoke settings.
            serial1SendData(A_SEND_PREFERENCES_SMOKE);
          break;

          default:
            // No-op for all other actions.
          break;
        }
      }
      else {
        // Check if the wand is telling us it is here after connecting it to the pack.
        // Then synchronise some basic/current settings between the pack and the wand.
        switch(recvCmdW.c) {
          case W_HANDSHAKE:
            Serial.println("Got Initial Wand Handshake");

            // Turn on a single Power Cell LED to indicate that the wand sync process has begun.
            // This LED will be turned off automatically on the next iteration of the main loop.
            pack_leds[0] = getHueAsRGB(POWERCELL, C_WHITE); // White works with any LED choice.
            FastLED.show(); // Force update of color state.

            // Begin the synchronization process which tells the wand the pack got the handshake.
            
            packSerialSend(P_SYNC_START);
            Serial.println("Sending Sync Start");
          /*
            if(b_overheating == true) {
              packOverHeatingFinished();
            }

            // Make sure this is called before the P_YEAR is sent over to the Neutrona Wand.
            switch(SYSTEM_MODE) {
              case MODE_ORIGINAL:
                packSerialSend(P_MODE_ORIGINAL);
              break;

              case MODE_SUPER_HERO:
              default:
                packSerialSend(P_MODE_SUPER_HERO);
              break;
            }

            if(switch_power.getState() == LOW) {
              // Tell the Neutrona Wand that power to the Proton Pack is on.
              packSerialSend(P_MODE_ORIGINAL_RED_SWITCH_ON);
            }
            else {
              // Tell the Neutrona Wand that power to the Proton Pack is off.
              packSerialSend(P_MODE_ORIGINAL_RED_SWITCH_OFF);
            }

            switch(SYSTEM_YEAR) {
              case SYSTEM_1984:
                packSerialSend(P_YEAR_1984);
              break;
              case SYSTEM_1989:
                packSerialSend(P_YEAR_1989);
              break;
              case SYSTEM_AFTERLIFE:
                packSerialSend(P_YEAR_AFTERLIFE);
              break;
              case SYSTEM_FROZEN_EMPIRE:
                packSerialSend(P_YEAR_FROZEN_EMPIRE);
              break;
            }

            // Stop any music. Mainly for when flashing while connected to a computer with a running wand.
            //packSerialSend(P_MUSIC_STOP);

            // Sync the current music track.
            // If music is already playing on a pack while a wand is reconnected, the wand will start playing music when the current track ends.
            packSerialSend(P_MUSIC_PLAY_TRACK, i_current_music_track);

            if(b_repeat_track == true) {
              packSerialSend(P_MUSIC_REPEAT);
            }
            else {
              packSerialSend(P_MUSIC_NO_REPEAT);
            }

            // Vibration enabled or disabled from the Proton Pack toggle switch
            if(b_vibration_enabled == true) {
              packSerialSend(P_VIBRATION_ENABLED);
            }
            else {
              packSerialSend(P_VIBRATION_DISABLED);
            }

            // Ribbon cable alarm.
            if(b_alarm == true) {
              packSerialSend(P_ALARM_ON);
            }
            else {
              packSerialSend(P_ALARM_OFF);
            }

            // Pack status
            if(PACK_STATE != MODE_OFF) {
              packSerialSend(P_ON);
            }
            else {
              packSerialSend(P_OFF);
            }

            // Reset the wand power levels.
            switch(i_wand_power_level) {
              case 5:
                packSerialSend(P_POWER_LEVEL_5);
              break;

              case 4:
                packSerialSend(P_POWER_LEVEL_4);
              break;

              case 3:
                packSerialSend(P_POWER_LEVEL_3);
              break;

              case 2:
                packSerialSend(P_POWER_LEVEL_2);
              break;

              case 1:
              default:
                packSerialSend(P_POWER_LEVEL_1);
              break;
            }

            // Synchronise the firing modes.
            switch(FIRING_MODE) {
              case SLIME:
                packSerialSend(P_SLIME_MODE);
              break;

              case STASIS:
                packSerialSend(P_STASIS_MODE);
              break;

              case MESON:
                packSerialSend(P_MESON_MODE);
              break;

              case SPECTRAL:
                packSerialSend(P_SPECTRAL_MODE);
              break;

              case HOLIDAY:
                packSerialSend(P_HOLIDAY_MODE);
              break;

              case SPECTRAL_CUSTOM:
                packSerialSend(P_SPECTRAL_CUSTOM_MODE);
              break;

              case VENTING:
                packSerialSend(P_VENTING_MODE);
              break;

              case PROTON:
              case SETTINGS:
              default:
                packSerialSend(P_PROTON_MODE);

                FIRING_MODE = PROTON;

                if(b_pack_on != true && b_pack_shutting_down != true) {
                  if(b_cyclotron_colour_toggle == true) {
                    // Reset the Cyclotron LED colours.
                    cyclotronColourReset();
                  }

                  if(b_powercell_colour_toggle == true) {
                    // Reset the Power Cell colours.
                    b_powercell_updating = true;
                    powercellDraw();
                  }
                }
              break;
            }

            // Tell the wand the status of the Proton Pack ribbon cable.
            if(switch_alarm.getState() == LOW) {
              // Ribbon cable is on.
              packSerialSend(P_RIBBON_CABLE_ON);
            }
            else {
              packSerialSend(P_RIBBON_CABLE_OFF);
            }

            // Synchronise the volume settings.
            packSerialSendData(P_VOLUME_SYNC);

            if(i_volume_master == i_volume_abs_min) {
              // Telling the wand to be silent if required.
              packSerialSend(P_MASTER_AUDIO_SILENT_MODE);
            }
            else {
              packSerialSend(P_MASTER_AUDIO_NORMAL);
            }
          */

            Serial.println("Sending Sync End");
            packSerialSend(P_SYNC_END);
          break;

          case W_SYNCHRONIZED:
            Serial.println("Wand Synchronized");
            b_wand_connected = true; // Indicate completion of wand sync process.
          break;

          default:
            // No-op for all other actions.
          break;
        }
      }
    }
  }
}
