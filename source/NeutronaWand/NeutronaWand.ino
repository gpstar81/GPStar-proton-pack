/**
 *   gpstar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gmail.com>
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

#if defined(__AVR_ATmega2560__)
  #define GPSTAR_NEUTRONA_WAND_PCB
#endif

/*
  ***** IMPORTANT *****
  * You need to open and edit the wavTrigger.h file and make sure you un comment out the proper serial port.
  * This can be found near the top of the wavTrigger.h file after opening it.
  * All other serial ports will need to be disabled and commented out in the wavTrigger.h file.

  * If you are compiling the code to upload to a Arduino Nano:
  * IMPORTANT: Do not forget to unplug the TX1/RX1 cables from Serial1 while you are uploading code to your Nano.
  You want to use: #define __WT_USE_ALTSOFTSERIAL__

  * If you are compiling the code to upload to the gpstar Neutrona Wand micro controller:
  You want to use: #define __WT_USE_SERIAL3__
*/
#include <wavTrigger.h>

#include <millisDelay.h>
#include <FastLED.h>
#include <ezButton.h>

#ifdef GPSTAR_NEUTRONA_WAND_PCB
  #include <ht16k33.h>
  #include <Wire.h>
#endif

/*
  ***** IMPORTANT *****
  * For Arduino Nano builds, you need to open Packet.h located in your Arduino/Libraries/SerialTransfer folder and on line #34 and change the max packet size to 0x9B:
  * When building for your Mega, you can switch it back to 0xFE

  * Before:
  const uint8_t MAX_PACKET_SIZE = 0xFE; // Maximum allowed payload bytes per packet

  * After:
  const uint8_t MAX_PACKET_SIZE = 0x9B; // Maximum allowed payload bytes per packet
*/
#include <SerialTransfer.h>

#include "Configuration.h"
#include "MusicSounds.h"
#include "Communication.h"
#include "Header.h"

#ifndef GPSTAR_NEUTRONA_WAND_PCB
  #include <AltSoftSerial.h>
#endif

void setup() {
  Serial.begin(9600);

  // Enable Serial1 if compiling for the gpstar Neutrona Wand micro controller.
  #ifdef HAVE_HWSERIAL1
    #ifdef GPSTAR_NEUTRONA_WAND_PCB
      Serial1.begin(9600);
      wandComs.begin(Serial1);
    #endif
  #else
    wandComs.begin(Serial);
  #endif

  // Change PWM frequency of pin 3 and 11 for the vibration motor, we do not want it high pitched.
  TCCR2B = (TCCR2B & B11111000) | (B00000110); // for PWM frequency of 122.55 Hz

  setupWavTrigger();

  // Barrel LEDs
  FastLED.addLeds<NEOPIXEL, BARREL_LED_PIN>(barrel_leds, BARREL_NUM_LEDS);

  switch_wand.setDebounceTime(switch_debounce_time);
  switch_intensify.setDebounceTime(switch_debounce_time);
  switch_activate.setDebounceTime(switch_debounce_time);
  switch_vent.setDebounceTime(switch_debounce_time);

  #ifndef GPSTAR_NEUTRONA_WAND_PCB
    b_28segment_bargraph = false; // Force Arduino Nano builds to not use the 28 segment bargraph.
  #endif

  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    pinMode(switch_mode, INPUT_PULLUP);
    pinMode(switch_barrel, INPUT_PULLUP);
  #endif

  // Rotary encoder on the top of the wand.
  pinMode(r_encoderA, INPUT_PULLUP);
  pinMode(r_encoderB, INPUT_PULLUP);

  bargraphYearModeUpdate();

  // Setup the bargraph.
  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    delay(10);

    WIRE.begin();

    byte by_error, by_address;
    unsigned int i_i2c_devices = 0;
    
    // Scan i2c for any devices (28 segment bargraph).
    for(by_address = 1; by_address < 127; by_address++ ) {
      WIRE.beginTransmission(by_address);
      by_error = WIRE.endTransmission();

      if(by_error == 0) {
        i_i2c_devices++;
      }
    }

    if(i_i2c_devices > 0) {
      b_28segment_bargraph = true;
    }
    else {
      b_28segment_bargraph = false;
    }
          
    if(b_28segment_bargraph == true) {
      ht_bargraph.begin(0x00);
    }
    else {
      pinMode(led_bargraph_1, OUTPUT);
      pinMode(led_bargraph_2, OUTPUT);
      pinMode(led_bargraph_3, OUTPUT);
      pinMode(led_bargraph_4, OUTPUT);
      pinMode(led_bargraph_5, OUTPUT);
    }
  #else
    // Original Hasbro bargraph.
    pinMode(led_bargraph_1, OUTPUT);
    pinMode(led_bargraph_2, OUTPUT);
    pinMode(led_bargraph_3, OUTPUT);
    pinMode(led_bargraph_4, OUTPUT);
    pinMode(led_bargraph_5, OUTPUT);
  #endif

  pinMode(led_slo_blo, OUTPUT);

  // Extra optional items if using them with the gpstar Neutrona Wand micro controller.
  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    pinMode(led_front_left, OUTPUT); // Front left LED. When using the gpstar Neutrona Wand micro controller, it is wired to it's own pin. When using a Arduino Nano, it is linked with led_slo_blo.
    pinMode(led_hat_1, OUTPUT); // Hat light at front of the wand near the barrel tip.
    pinMode(led_hat_2, OUTPUT); // Hat light at top of the wand body.
    pinMode(led_barrel_tip, OUTPUT); // LED at the tip of the wand barrel.
  #endif

  pinMode(led_vent, OUTPUT);
  pinMode(led_white, OUTPUT);

  pinMode(vibration, OUTPUT);

  // Make sure lights are off.
  wandLightsOff();

  // Wand status.
  WAND_STATUS = MODE_OFF;
  WAND_ACTION_STATUS = ACTION_IDLE;

  ms_reset_sound_beep.start(i_sound_timer);

  // Setup the mode switch debounce.
  ms_switch_mode_debounce.start(1);

  i_wand_menu = 5;

  // We bootup the wand in the classic proton mode.
  FIRING_MODE = PROTON;
  PREV_FIRING_MODE = SETTINGS;

  // Check music timer.
  ms_check_music.start(i_music_check_delay);

  if(b_no_pack == true || b_debug == true) {
    b_wait_for_pack = false;
    b_pack_on = true;
  }
}

void loop() {
  if(b_wait_for_pack == true) {
    if(b_volume_sync_wait != true) {
      // Handshake with the pack. Telling the pack that we are here.
      wandSerialSend(W_HANDSHAKE);
    }

    // Synchronise some settings with the pack.
    checkPack();

    delay(10);
  }
  else {
    mainLoop();
  }
}

void mainLoop() {
  w_trig.update();

  checkMusic();
  checkPack();
  switchLoops();
  checkRotary();
  checkSwitches();

  if(ms_firing_stop_sound_delay.justFinished()) {
    modeFireStopSounds();
  }

  switch(WAND_ACTION_STATUS) {
    case ACTION_IDLE:
      if(WAND_STATUS == MODE_ON) {
        switch(year_mode) {
          case 1984:
          case 1989:
            // Do nothing.
          break;

          case 2021:
            #ifdef GPSTAR_NEUTRONA_WAND_PCB
              if(WAND_ACTION_STATUS != ACTION_OVERHEATING && b_pack_alarm != true) {
                // Ready to fire, the hat light LED at the barrel tip lights up in Afterlife mode.
                if(switchBarrel() != true && switch_vent.getState() == LOW && switch_wand.getState() == LOW) {
                  digitalWrite(led_hat_1, HIGH);
                }
                else {
                  digitalWrite(led_hat_1, LOW);
                }
              }
            #endif
          break;
        }
      }
    break;

    case ACTION_OFF:
      wandOff();
    break;

    case ACTION_FIRING:
      if(FIRING_MODE == VENTING) {
        // If we are in venting mode, lets trigger a vent sequence.
        startVentSequence();
      }
      else if(b_pack_on == true && b_pack_alarm == false) {
        if(ms_firing_start_sound_delay.justFinished()) {
          modeFireStartSounds();
        }

        if(b_firing == false) {
          b_firing = true;
          modeFireStart();
        }

        if(ms_hat_1.isRunning()) {
          if(ms_hat_1.remaining() < i_hat_1_delay / 2) {
            #ifdef GPSTAR_NEUTRONA_WAND_PCB
              digitalWrite(led_hat_1, LOW);
              digitalWrite(led_hat_2, HIGH);
            #endif
          }
          else {
            #ifdef GPSTAR_NEUTRONA_WAND_PCB
              digitalWrite(led_hat_1, HIGH);
              digitalWrite(led_hat_2, LOW);
            #endif
          }

          if(ms_hat_1.justFinished()) {
            ms_hat_1.start(i_hat_1_delay);
          }
        }

        // Overheating.
        if(ms_overheat_initiate.justFinished() && b_overheat_mode[i_power_mode - 1] == true && b_overheat_enabled == true) {
          startVentSequence();
        }
        else {
          modeFiring();

          // Stop firing if any of the main switches are turned off or the barrel is retracted.
          if(switch_vent.getState() == HIGH || switch_wand.getState() == HIGH || switchBarrel() == true) {
            modeFireStop();
          }
        }
      }
      else if(b_pack_alarm == true && b_firing == true) {
        modeFireStop();
      }
    break;

    case ACTION_OVERHEATING:
      if(b_overheat_bargraph_blink == true) {
        settingsBlinkingLights();
      }
      else {
        // Prepare to make the bargraph ramp if set to during overheat.
        if(ms_bargraph.justFinished()) {
          bargraphRampUp();
        }
      }

      if(ms_overheating.justFinished()) {
        bargraphClearAlt();

        ms_overheating.stop();
        ms_settings_blinking.stop();

        // Turn off hat light 2.
        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_hat_2, LOW);
        #endif

        WAND_ACTION_STATUS = ACTION_IDLE;

        stopEffect(S_CLICK);
        stopEffect(S_VENT_DRY);

        // Prepare a few things before ramping the bargraph back up from a full ramp down.
        if(b_overheat_bargraph_blink != true) {
          playEffect(S_BOOTUP);

          if(year_mode == 2021) {
            bargraphYearModeUpdate();
          }
          else {
            i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_1984 * 2;
          }

          if(switch_vent.getState() == LOW) {
            soundIdleLoop(true);
          }
          else {
            switch(year_mode) {
              case 1984:
              case 1989:
                // Do nothing.
              break;

              case 2021:
                soundIdleLoop(true);

                playEffect(S_AFTERLIFE_GUN_RAMP_1, false, i_volume - 10);
                ms_gun_loop_1.start(2000);
              break;
            }
          }
        }

        bargraphRampUp();

        // Tell the pack that we finished overheating.
        wandSerialSend(W_OVERHEATING_FINISHED);
      }
    break;

    case ACTION_ERROR:
      // nothing.
    break;

    case ACTION_ACTIVATE:
      modeActivate();
    break;

    case ACTION_SETTINGS:
      settingsBlinkingLights();

      switch(i_wand_menu) {
        // Top menu: Music track loop setting.
        // Sub menu: Enable or disable crossing the streams / video game modes.
        // Sub menu: (Mode switch) -> Enable/Disable Video Game Colour Modes for the Proton Pack LEDs. (when video game mode is selected).
        case 5:
        // Music track loop setting.
        if(b_wand_menu_sub != true) {
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            if(b_repeat_track == false) {
              // Loop the track.
              b_repeat_track = true;
              w_trig.trackLoop(i_current_music_track, 1);
            }
            else {
              b_repeat_track = false;
              w_trig.trackLoop(i_current_music_track, 0);
            }

            // Tell pack to loop the music track.
            wandSerialSend(W_MUSIC_TRACK_LOOP_TOGGLE);
          }
        }
        else {
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            // Enable or disable crossing the streams / crossing the streams mix / video game modes.
            if(b_cross_the_streams == true && b_cross_the_streams_mix == true) {
              // Turn off crossing the streams mode and switch back to video game mode.
              b_cross_the_streams = false;
              b_cross_the_streams_mix = false;
              
              stopEffect(S_CLICK);

              playEffect(S_CLICK);

              stopEffect(S_VOICE_CROSS_THE_STREAMS);
              stopEffect(S_VOICE_CROSS_THE_STREAMS_MIX);
              stopEffect(S_VOICE_VIDEO_GAME_MODES);

              playEffect(S_VOICE_VIDEO_GAME_MODES);

              // Tell the proton pack to reset back to the proton stream.
              wandSerialSend(W_PROTON_MODE_REVERT);
            }
            else if(b_cross_the_streams == true && b_cross_the_streams_mix != true) {
              // Keep cross the streams on.
              b_cross_the_streams = true;

              // Turn on cross the streams mix.
              b_cross_the_streams_mix = true;

              stopEffect(S_CLICK);

              playEffect(S_CLICK);

              stopEffect(S_VOICE_VIDEO_GAME_MODES);
              stopEffect(S_VOICE_CROSS_THE_STREAMS);
              stopEffect(S_VOICE_CROSS_THE_STREAMS_MIX);

              playEffect(S_VOICE_CROSS_THE_STREAMS_MIX);

              // Tell the proton pack to reset back to the proton stream.
              wandSerialSend(W_RESET_PROTON_STREAM_MIX);
            }
            else {
              // Turn on crossing the streams mode and turn off video game mode.
              b_cross_the_streams = true;
              b_cross_the_streams_mix = false;

              stopEffect(S_CLICK);

              playEffect(S_CLICK);

              stopEffect(S_VOICE_VIDEO_GAME_MODES);
              stopEffect(S_VOICE_CROSS_THE_STREAMS_MIX);
              stopEffect(S_VOICE_CROSS_THE_STREAMS);

              playEffect(S_VOICE_CROSS_THE_STREAMS);

              // Tell the proton pack to reset back to the proton stream.
              wandSerialSend(W_RESET_PROTON_STREAM);
            }

            // Reset the previous firing mode to the proton stream.
            PREV_FIRING_MODE = PROTON;
          }

          // Enable/Disable Video Game Colour Modes for the Proton Pack LEDs.
          if(switchMode() == true) {
            if(b_cross_the_streams != true && b_cross_the_streams_mix != true) {
              // Tell the Proton Pack to cycle through the Video Game Colour toggles.
              wandSerialSend(W_VIDEO_GAME_MODE_COLOUR_TOGGLE);
            }
          }
        }
        break;

        // Top menu: (Intensify + Top dial) Adjust the LED dimming of the Power Cell, Cyclotron and Inner Cyclotron.
        // Top menu: (Mode Switch) Cycle through which dimming mode to adjust in the Proton Pack. Power Cell, Cyclotron, Inner Cyclotron.
        // Sub menu: Enable or disable smoke for the Proton Pack.
        // Sub menu: (Mode switch) -> Enable or disable overheating.
        case 4:
          // Adjust the Proton Pack / Neutrona wand sound effects volume.
          if(b_wand_menu_sub != true) {
            // Cycle through the dimming modes in the Proton Pack. (Power Cell, Cyclotron and Inner Cyclotron). Actualy control of the dimming is handled in checkRotary().
            if(switchMode() == true) {
              // Tell the Proton Pack to change to the next dimming mode.
              wandSerialSend(W_DIMMING_TOGGLE);
            }
          }
          else {
            // Enable or disable smoke for the Proton Pack.
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              // Tell the Proton Pack to toggle the smoke on or off.
              wandSerialSend(W_SMOKE_TOGGLE);
            }

            // Enable or disable overheating.
            if(switchMode() == true) {
              if(b_overheat_enabled == true) {
                b_overheat_enabled = false;

                // Play the overheating disabled voice.
                stopEffect(S_VOICE_OVERHEAT_DISABLED);
                stopEffect(S_VOICE_OVERHEAT_ENABLED);

                playEffect(S_VOICE_OVERHEAT_DISABLED);

                // Tell the Proton Pack that overheating is disabled.
                wandSerialSend(W_OVERHEATING_DISABLED);
              }
              else {
                b_overheat_enabled = true;

                // Play the overheating enabled voice.
                stopEffect(S_VOICE_OVERHEAT_DISABLED);
                stopEffect(S_VOICE_OVERHEAT_ENABLED);

                playEffect(S_VOICE_OVERHEAT_ENABLED);

                // Tell the Proton Pack that overheating is enabled.
                wandSerialSend(W_OVERHEATING_ENABLED);
              }
            }
          }
        break;

        // Top menu: (Intensify + top dial) Adjust Proton Pack / Neutrona wand sound effects. (Mode switch + top dial) Adjust Proton Pack / Neutrona Wand music volume.
        // Top menu: (Intensify + top dial) Adjust Proton Pack / Neutrona wand sound effects. (Mode switch + top dial) Adjust Proton Pack / Neutrona Wand music volume.
        // Sub menu: Toggle cyclotron rotation direction.
        // Sub menu: (Mode switch) -> Toggle the Proton Pack Single LED or 3 LEDs for 1984/1989 modes.
        case 3:
          // Top menu code is handled in checkRotary();
          // Sub menu. Adjust cyclotron settings.
          if(b_wand_menu_sub == true) {
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              // Tell the Proton Pack to change the cyclotron rotation direction.
              wandSerialSend(W_CYCLOTRON_DIRECTION_TOGGLE);
            }

            if(switchMode() == true) {
              // Tell the Proton Pack to toggle the Single LED or 3 LEDs for 1984/1989 modes.
              wandSerialSend(W_CYCLOTRON_LED_TOGGLE);
            }
          }
        break;

        // Top menu: Change music tracks.
        // Sub menu: Enable pack vibration, enable pack vibration while firing only, disable pack vibration. *Note that the pack vibration switch will toggle both pack and wand vibiration on or off*
        // Sub menu: (Mode switch) -> Enable wand vibration, enable wand vibration while firing only, disable wand vibration.
        case 2:
          // Change music tracks.
          if(b_wand_menu_sub != true) {
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              if(i_current_music_track + 1 > i_music_track_start + (i_music_count - 1)) {
                if(b_playing_music == true) {
                  // Go to the first track to play it.
                  stopMusic();
                  i_current_music_track = i_music_track_start;
                  playMusic();
                }
                else {
                  i_current_music_track = i_music_track_start;
                }
              }
              else {
                // Stop the old track and play the new track if music is currently playing.
                if(b_playing_music == true) {
                  stopMusic();
                  i_current_music_track++;
                  playMusic();
                }
                else {
                  i_current_music_track++;
                }
              }

              // Tell the pack which music track to change to.
              wandSerialSend(i_current_music_track);
            }

            if(switchMode() == true) {
              if(i_current_music_track - 1 < i_music_track_start) {
                if(b_playing_music == true) {
                  // Go to the last track to play it.
                  stopMusic();
                  i_current_music_track = i_music_track_start + (i_music_count -1);
                  playMusic();
                }
                else {
                  i_current_music_track = i_music_track_start + (i_music_count -1);
                }
              }
              else {
                // Stop the old track and play the new track if music is currently playing.
                if(b_playing_music == true) {
                  stopMusic();
                  i_current_music_track--;
                  playMusic();
                }
                else {
                  i_current_music_track--;
                }
              }

              // Tell the pack which music track to change to.
              wandSerialSend(i_current_music_track);
            }
          }
          else {
            // Enable or disable vibration for the Pack or during firing only.
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              stopEffect(S_BEEPS_ALT);

              playEffect(S_BEEPS_ALT);

              wandSerialSend(W_VIBRATION_CYCLE_TOGGLE);
            }

            // Enable or disable vibration or firing vibration only for the wand.
            if(switchMode() == true) {
              stopEffect(S_BEEPS_ALT);

              playEffect(S_BEEPS_ALT);

              if(b_vibration_on != true) {
                b_vibration_on = true;
                b_vibration_enabled = true; // Override the Proton Pack vibration toggle switch.

                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

                playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);

                wandSerialSend(W_VIBRATION_ENABLED);

                analogWrite(vibration, 150);
                delay(250);
                analogWrite(vibration,0);
              }
              else if(b_vibration_on == true && b_vibration_firing != true) {
                b_vibration_firing = true;
                b_vibration_enabled = true; // Override the Proton Pack vibration toggle switch.

                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

                playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);

                wandSerialSend(W_VIBRATION_FIRING_ENABLED);

                analogWrite(vibration, 150);
                delay(250);
                analogWrite(vibration,0);
              }
              else {
                b_vibration_on = false;
                b_vibration_firing = false;

                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_FIRING_ENABLED);
                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_ENABLED);
                stopEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

                playEffect(S_VOICE_NEUTRONA_WAND_VIBRATION_DISABLED);

                wandSerialSend(W_VIBRATION_DISABLED);
              }
            }
          }
        break;

        // Top menu: Play music or stop music.
        // Top menu: (Mode Switch). Mute the Proton Pack and Neutrona Wand.
        // Sub menu: (Intensify) -> Switch between 1984/1989/Afterlife mode.
        // Sub Menu: (Mode Switch) -> Enable or disable Proton Stream impact effects.
        case 1:
          // Play or stop the current music track.
          if(b_wand_menu_sub != true) {
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              if(b_playing_music == true) {
                // Stop music
                b_playing_music = false;

                // Tell the pack to stop music.
                wandSerialSend(W_MUSIC_STOP);

                stopMusic();
              }
              else {
                if(i_music_count > 0 && i_current_music_track >= i_music_track_start) {
                  // Start music.
                  b_playing_music = true;

                  // Tell the pack to play music.
                  wandSerialSend(W_MUSIC_START);

                  playMusic();
                }
              }
            }

            // Silent the Proton Pack or Neutrona Wand or revert back.
            if(switchMode() == true) {
              if(i_volume_master == i_volume_abs_min) {
                wandSerialSend(W_VOLUME_REVERT);

                i_volume_master = i_volume_revert;
              }
              else {
                i_volume_revert = i_volume_master;

                // Set the master volume to silent.
                i_volume_master = i_volume_abs_min;

                wandSerialSend(W_SILENT_MODE);
              }

              w_trig.masterGain(i_volume_master); // Reset the master gain.
            }
          }
          else {
            // Switch between 1984/1989/Afterlife mode.
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              // Tell the Proton Pack to cycle through year modes.
              wandSerialSend(W_YEAR_MODES_CYCLE);

              stopEffect(S_BEEPS_BARGRAPH);

              playEffect(S_BEEPS_BARGRAPH);

              // There is no pack connected, lets change the years.
              if(b_no_pack == true) {
                if(year_mode == 1984) {
                  year_mode = 1989;

                  stopEffect(S_VOICE_AFTERLIFE);
                  stopEffect(S_VOICE_1984);
                  stopEffect(S_VOICE_1989);

                  playEffect(S_VOICE_1989);
                }
                else if(year_mode == 1989) {
                  year_mode = 2021;

                  stopEffect(S_VOICE_AFTERLIFE);
                  stopEffect(S_VOICE_1984);
                  stopEffect(S_VOICE_1989);

                  playEffect(S_VOICE_AFTERLIFE);
                }
                else if(year_mode == 2021) {
                  year_mode = 1984;

                  stopEffect(S_VOICE_AFTERLIFE);
                  stopEffect(S_VOICE_1989);
                  stopEffect(S_VOICE_1984);

                  playEffect(S_VOICE_1984);
                }
              }
            }

            if(switchMode() == true) {
              // Tell the Proton Pack to toggle the Proton Stream impact effects.
              wandSerialSend(W_PROTON_STREAM_IMPACT_TOGGLE);
            }
          }
        break;
      }
    break;
  }

  switch(WAND_STATUS) {
    case MODE_OFF:
      if(switchMode() == true || b_pack_alarm == true) {
        if(FIRING_MODE != SETTINGS && b_pack_alarm != true && (b_pack_on != true || b_no_pack == true)) {
          playEffect(S_CLICK);

          PREV_FIRING_MODE = FIRING_MODE;
          FIRING_MODE = SETTINGS;

          WAND_ACTION_STATUS = ACTION_SETTINGS;
          i_wand_menu = 5;
          ms_settings_blinking.start(i_settings_blinking_delay);

          // Tell the pack we are in settings mode.
          wandSerialSend(W_SETTINGS_MODE);
        }
        else {
          // Only exit the settings menu when on menu #5 in the top menu.
          if(i_wand_menu == 5 && b_wand_menu_sub != true && FIRING_MODE == SETTINGS) {
            wandExitMenu();
          }
        }
      }
      else if(WAND_ACTION_STATUS == ACTION_SETTINGS && b_pack_on == true) {
        if(b_no_pack != true) {
          wandExitMenu();
        }
      }

      if(b_pack_alarm == true) {
        if(ms_hat_2.justFinished()) {
          ms_hat_2.start(i_hat_2_delay);
        }
      }
    break;

    case MODE_ERROR:
      if(ms_hat_2.remaining() < i_hat_2_delay / 2) {
        digitalWrite(led_white, HIGH);

        analogWrite(led_slo_blo, 0);

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_hat_2, LOW);
          digitalWrite(led_hat_1, LOW);
          analogWrite(led_front_left, 0);
        #endif
      }
      else {
        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_hat_2, HIGH);
          digitalWrite(led_hat_1, HIGH);
          analogWrite(led_front_left, 255);
        #endif

        digitalWrite(led_white, LOW);
        analogWrite(led_slo_blo, 255);
      }

      if(ms_hat_2.justFinished()) {
        ms_hat_2.start(i_hat_2_delay);

        playEffect(S_BEEPS_LOW);
        playEffect(S_BEEPS);
      }

      if(ms_hat_1.justFinished()) {
        playEffect(S_BEEPS_BARGRAPH);

        ms_hat_1.start(i_hat_2_delay * 4);
      }

      settingsBlinkingLights();
    break;

    case MODE_ON:
      if(b_vibration_on == true && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
        vibrationSetting();
      }

      // Hat light 2 blinking when the Pack ribbon cable has been removed.
      if(b_pack_alarm == true) {
        if(ms_hat_2.remaining() < i_hat_2_delay / 2) {
          #ifdef GPSTAR_NEUTRONA_WAND_PCB
            digitalWrite(led_hat_2, LOW);
          #endif
        }
        else {
          #ifdef GPSTAR_NEUTRONA_WAND_PCB
            digitalWrite(led_hat_2, HIGH);
          #endif
        }

        if(ms_hat_2.justFinished()) {
          ms_hat_2.start(i_hat_2_delay);
        }
      }
      else {
        if(ms_hat_1.isRunning() != true) {
          #ifdef GPSTAR_NEUTRONA_WAND_PCB
            // Hat 2 stays solid while the Neutrona Wand is on. It will blink though when about to overheat.
            digitalWrite(led_hat_2, HIGH);
          #endif
        }
      }

      // Top white light.
      if(ms_white_light.justFinished()) {
        ms_white_light.start(d_white_light_interval);
        if(digitalRead(led_white) == LOW) {
          digitalWrite(led_white, HIGH);

          // We make the slo-blo light blink during vent mode.
          if(FIRING_MODE == VENTING) {
            analogWrite(led_slo_blo, 255);

            // If using the gpstar neutrona wand micro controller, the front left LED is wired separately, lets turn it on.
            #ifdef GPSTAR_NEUTRONA_WAND_PCB
              analogWrite(led_front_left, 255);
            #endif
          }
        }
        else {
          digitalWrite(led_white, LOW);

          // We make the slo-blo light blink during vent mode.
          if(FIRING_MODE == VENTING) {
            analogWrite(led_slo_blo, 0);

            // If using the gpstar neutrona wand micro controller, the front left LED is wired separately, lets turn it on.
            #ifdef GPSTAR_NEUTRONA_WAND_PCB
              analogWrite(led_front_left, 0);
            #endif
          }
        }
      }

      if(b_pack_alarm != true) {
        // Ramp the bargraph up ramp down back to the default power level setting on a fresh start.
        if(ms_bargraph.justFinished()) {
          bargraphRampUp();
        }
        else if(ms_bargraph.isRunning() == false && WAND_ACTION_STATUS != ACTION_FIRING && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          // Bargraph idling loop.
          bargraphPowerCheck();
        }

        if(year_mode == 2021) {
          if(ms_gun_loop_1.justFinished()) {
            playEffect(S_AFTERLIFE_GUN_LOOP_1, true, i_volume - 10);
            ms_gun_loop_1.stop();
          }
        }
      }
      else {
        // This is going to cause the bargraph to ramp down.
        if(ms_bargraph.justFinished()) {
          bargraphRampUp();
        }
      }

      wandBarrelHeatUp();
    break;
  }

  if(b_firing == true && WAND_ACTION_STATUS != ACTION_FIRING) {
    modeFireStop();
  }

  if(ms_firing_lights_end.justFinished()) {
    fireStreamEnd(0,0,0);
  }

  // Update the barrel LEDs.
  if(ms_fast_led.justFinished()) {
    FastLED.show();
    ms_fast_led.stop();
  }
}

void startVentSequence() {
  ms_overheat_initiate.stop();

  if(WAND_ACTION_STATUS == ACTION_FIRING && b_firing == true) {
    modeFireStop();
  }

  // Turn on hat light 2.
  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    digitalWrite(led_hat_2, HIGH);
  #endif

  delay(100);

  WAND_ACTION_STATUS = ACTION_OVERHEATING;

  // Play overheating sounds.
  ms_overheating.start(i_ms_overheating);

  // Blinking bargraph option for overheat.
  if(b_overheat_bargraph_blink == true) {
    ms_settings_blinking.start(i_settings_blinking_delay);
  }
  else {
    // If bargraph is set to ramp down during overheat, we need to set a few things.
    soundBeepLoopStop();
    soundIdleStop();
    soundIdleLoopStop();

    b_sound_idle = false; // REMOVE ??
    b_beeping = false;

    // Reset some bargraph levels before we ramp the bargraph down.
    i_bargraph_status_alt = 28; // For 28 segment bargraph
    i_bargraph_status = 5; // For Hasbro 5 LED bargraph.

    bargraphFull();

    ms_bargraph.start(d_bargraph_ramp_interval);
  }

  playEffect(S_VENT_DRY);
  playEffect(S_CLICK);

  // Tell the pack we are overheating.
  wandSerialSend(W_OVERHEATING);
}

void checkMusic() {
  if(ms_check_music.justFinished() && ms_music_next_track.isRunning() != true) {
    ms_check_music.start(i_music_check_delay);

    // Loop through all the tracks if the music is not set to repeat a track.
    if(b_playing_music == true && b_repeat_track == false) {
      if(!w_trig.isTrackPlaying(i_current_music_track)) {
        ms_check_music.stop();

        stopMusic();

        // Tell the pack to stop playing music.
        wandSerialSend(W_MUSIC_STOP);

        if(i_current_music_track + 1 > i_music_track_start + i_music_count - 1) {
          i_current_music_track = i_music_track_start;
        }
        else {
          i_current_music_track++;
        }

        // Tell the pack which music track to change to.
        wandSerialSend(i_current_music_track);

        ms_music_next_track.start(i_music_next_track_delay);
      }
    }
  }

  if(ms_music_next_track.justFinished()) {
    ms_music_next_track.stop();

    playMusic();

    // Tell the pack to play music.
    wandSerialSend(W_MUSIC_START);

    ms_check_music.start(i_music_check_delay);
  }
}

void settingsBlinkingLights() {
  if(ms_settings_blinking.justFinished()) {
     ms_settings_blinking.start(i_settings_blinking_delay);
  }

  if(ms_settings_blinking.remaining() < i_settings_blinking_delay / 2) {
    bool b_solid_five = false;
    bool b_solid_one = false;

    // Indicator for looping track setting.
    if(b_repeat_track == true && i_wand_menu == 5 && WAND_ACTION_STATUS != ACTION_OVERHEATING && WAND_ACTION_STATUS != ACTION_ERROR && b_wand_menu_sub != true) {
      b_solid_five = true;
    }

    // Indicator for crossing the streams setting.
    if(b_cross_the_streams == true && i_wand_menu == 5 && WAND_ACTION_STATUS != ACTION_OVERHEATING && WAND_ACTION_STATUS != ACTION_ERROR && b_wand_menu_sub == true) {
      b_solid_five = true;
    }

    if(i_volume_master == i_volume_abs_min && WAND_ACTION_STATUS == ACTION_SETTINGS && b_wand_menu_sub != true) {
      b_solid_one = true;
    }

    #ifdef GPSTAR_NEUTRONA_WAND_PCB
      if(b_28segment_bargraph == true) {
        if(b_solid_five == true) {
          for(uint8_t i = 0; i < 16; i++) {
            if(b_solid_one == true && i < 2) {
              ht_bargraph.setLedNow(i_bargraph[i]);
            }
            else {
              ht_bargraph.clearLedNow(i_bargraph[i]);
            }
          }

          for(uint8_t i = 16; i < 18; i++) {
            ht_bargraph.setLedNow(i_bargraph[i]);
          }
        }
        else if(b_solid_one == true) {
          for(uint8_t i = 0; i < 18; i++) {
            if(i < 2) {
              ht_bargraph.setLedNow(i_bargraph[i]);
            }
            else {
              ht_bargraph.clearLedNow(i_bargraph[i]);
            }
          }
        }
        else {
          ht_bargraph.clearAll();
        }
      }
      else {
        if(b_solid_one == true) {
          digitalWrite(led_bargraph_1, LOW);
        }
        else {
          digitalWrite(led_bargraph_1, HIGH);
        }

        digitalWrite(led_bargraph_2, HIGH);
        digitalWrite(led_bargraph_3, HIGH);
        digitalWrite(led_bargraph_4, HIGH);

        if(b_solid_five == true) {
          digitalWrite(led_bargraph_5, LOW);
        }
        else {
          digitalWrite(led_bargraph_5, HIGH);
        }
      }
    #else
      if(b_solid_one == true) {
        digitalWrite(led_bargraph_1, LOW);
      }
      else {
        digitalWrite(led_bargraph_1, HIGH);
      }

      digitalWrite(led_bargraph_2, HIGH);
      digitalWrite(led_bargraph_3, HIGH);
      digitalWrite(led_bargraph_4, HIGH);

      if(b_solid_five == true) {
        digitalWrite(led_bargraph_5, LOW);
      }
      else {
        digitalWrite(led_bargraph_5, HIGH);
      }
    #endif
  }
  else {
    switch(i_wand_menu) {
      case 5:
        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          if(b_28segment_bargraph == true) {
            // 18 for the 5 level menu system.
            uint8_t i_leds = 18;

            if(WAND_ACTION_STATUS == ACTION_OVERHEATING || WAND_ACTION_STATUS == ACTION_ERROR) {
              // All the segments.
              i_leds = 28;
            }

            // NOTE: If you draw all 28 segments at once often, you can overflow the serial buffer after around 5 seconds.
            for(uint8_t i = 0; i < i_leds; i++) {
              if(WAND_ACTION_STATUS == ACTION_OVERHEATING || WAND_ACTION_STATUS == ACTION_ERROR) {
                switch(i) {
                  case 3:
                  case 4:
                  case 5:
                  case 9:
                  case 10:
                  case 11:
                  case 15:
                  case 16:
                  case 17:
                  case 21:
                  case 22:
                  case 23:
                  case 27:
                    // Nothing
                  break;

                  default:
                    ht_bargraph.setLedNow(i_bargraph[i]);
                  break;
                }
              }
              else {
                switch(i) {
                  case 2:
                  case 3:
                  case 6:
                  case 7:
                  case 10:
                  case 11:
                  case 14:
                  case 15:
                    // Nothing
                  break;

                  default:
                    ht_bargraph.setLedNow(i_bargraph[i]);
                  break;
                }
              }
            }
          }
          else {
            digitalWrite(led_bargraph_1, LOW);
            digitalWrite(led_bargraph_2, LOW);
            digitalWrite(led_bargraph_3, LOW);
            digitalWrite(led_bargraph_4, LOW);
            digitalWrite(led_bargraph_5, LOW);
          }
        #else
            digitalWrite(led_bargraph_1, LOW);
            digitalWrite(led_bargraph_2, LOW);
            digitalWrite(led_bargraph_3, LOW);
            digitalWrite(led_bargraph_4, LOW);
            digitalWrite(led_bargraph_5, LOW);
        #endif
      break;

      case 4:
        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          if(b_28segment_bargraph == true) {
            for(uint8_t i = 0; i < 14; i++) {
              if(WAND_ACTION_STATUS == ACTION_OVERHEATING || WAND_ACTION_STATUS == ACTION_ERROR) {
                switch(i) {
                  case 3:
                  case 4:
                  case 5:
                  case 9:
                  case 10:
                  case 11:
                  case 15:
                  case 16:
                  case 17:
                  case 21:
                  case 22:
                  case 23:
                  case 27:
                    // Nothing
                  break;

                  default:
                    ht_bargraph.setLedNow(i_bargraph[i]);
                  break;
                }
              }
              else {
                switch(i) {
                  case 2:
                  case 3:
                  case 6:
                  case 7:
                  case 10:
                  case 11:
                    // Nothing
                  break;

                  default:
                    ht_bargraph.setLedNow(i_bargraph[i]);
                  break;
                }
              }
            }
          }
          else {
            digitalWrite(led_bargraph_1, LOW);
            digitalWrite(led_bargraph_2, LOW);
            digitalWrite(led_bargraph_3, LOW);
            digitalWrite(led_bargraph_4, LOW);
            digitalWrite(led_bargraph_5, HIGH);
          }
        #else
          digitalWrite(led_bargraph_1, LOW);
          digitalWrite(led_bargraph_2, LOW);
          digitalWrite(led_bargraph_3, LOW);
          digitalWrite(led_bargraph_4, LOW);
          digitalWrite(led_bargraph_5, HIGH);
        #endif
      break;

      case 3:
        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          if(b_28segment_bargraph == true) {
            for(uint8_t i = 0; i < 10; i++) {
              if(WAND_ACTION_STATUS == ACTION_OVERHEATING || WAND_ACTION_STATUS == ACTION_ERROR) {
                switch(i) {
                  case 3:
                  case 4:
                  case 5:
                  case 9:
                  case 10:
                  case 11:
                  case 15:
                  case 16:
                  case 17:
                  case 21:
                  case 22:
                  case 23:
                  case 27:
                    // Nothing
                  break;

                  default:
                    ht_bargraph.setLedNow(i_bargraph[i]);
                  break;
                }
              }
              else {
                switch(i) {
                  case 2:
                  case 3:
                  case 6:
                  case 7:
                    // Nothing
                  break;

                  default:
                    ht_bargraph.setLedNow(i_bargraph[i]);
                  break;
                }
              }
            }
            }
            else {
              digitalWrite(led_bargraph_1, LOW);
              digitalWrite(led_bargraph_2, LOW);
              digitalWrite(led_bargraph_3, LOW);
              digitalWrite(led_bargraph_4, HIGH);
              digitalWrite(led_bargraph_5, HIGH);
            }
        #else
          digitalWrite(led_bargraph_1, LOW);
          digitalWrite(led_bargraph_2, LOW);
          digitalWrite(led_bargraph_3, LOW);
          digitalWrite(led_bargraph_4, HIGH);
          digitalWrite(led_bargraph_5, HIGH);
        #endif
      break;

      case 2:
        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          if(b_28segment_bargraph == true) {
            for(uint8_t i = 0; i < 6; i++) {
              if(WAND_ACTION_STATUS == ACTION_OVERHEATING || WAND_ACTION_STATUS == ACTION_ERROR) {
                switch(i) {
                  case 3:
                  case 4:
                  case 5:
                  case 9:
                  case 10:
                  case 11:
                  case 15:
                  case 16:
                  case 17:
                  case 21:
                  case 22:
                  case 23:
                  case 27:
                    // Nothing
                  break;

                  default:
                    ht_bargraph.setLedNow(i_bargraph[i]);
                  break;
                }
              }
              else {
                switch(i) {
                  case 2:
                  case 3:
                    // Nothing
                  break;

                  default:
                    ht_bargraph.setLedNow(i_bargraph[i]);
                  break;
                }
              }
            }
          }
          else {
            digitalWrite(led_bargraph_1, LOW);
            digitalWrite(led_bargraph_2, LOW);
            digitalWrite(led_bargraph_3, HIGH);
            digitalWrite(led_bargraph_4, HIGH);
            digitalWrite(led_bargraph_5, HIGH);
          }
        #else
          digitalWrite(led_bargraph_1, LOW);
          digitalWrite(led_bargraph_2, LOW);
          digitalWrite(led_bargraph_3, HIGH);
          digitalWrite(led_bargraph_4, HIGH);
          digitalWrite(led_bargraph_5, HIGH);
        #endif
      break;

      case 1:
        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          if(b_28segment_bargraph == true) {
            for(uint8_t i = 0; i < 2; i++) {
                switch(i) {
                  case 3:
                  case 4:
                  case 5:
                  case 9:
                  case 10:
                  case 11:
                  case 15:
                  case 16:
                  case 17:
                  case 21:
                  case 22:
                  case 23:
                  case 27:
                    // Nothing
                  break;

                  default:
                    ht_bargraph.setLedNow(i_bargraph[i]);
                  break;
                }
            }
          }
          else {
            digitalWrite(led_bargraph_1, LOW);
            digitalWrite(led_bargraph_2, HIGH);
            digitalWrite(led_bargraph_3, HIGH);
            digitalWrite(led_bargraph_4, HIGH);
            digitalWrite(led_bargraph_5, HIGH);
          }
        #else
          digitalWrite(led_bargraph_1, LOW);
          digitalWrite(led_bargraph_2, HIGH);
          digitalWrite(led_bargraph_3, HIGH);
          digitalWrite(led_bargraph_4, HIGH);
          digitalWrite(led_bargraph_5, HIGH);
        #endif
      break;
    }
  }
}

// Change the WAND_STATE here based on switches changing or pressed.
void checkSwitches() {
  if(b_debug == true) {
    #ifndef GPSTAR_NEUTRONA_WAND_PCB
      Serial.print(F("A6 -> "));
      Serial.println(analogRead(switch_mode));

      Serial.print(F("\n"));

      Serial.print(F("A7 -> "));
      Serial.println(analogRead(switch_barrel));
    #endif
  }

  if(ms_intensify_timer.justFinished()) {
    ms_intensify_timer.stop();
  }

  switch(WAND_STATUS) {
    case MODE_OFF:
     if(switch_activate.isPressed() && WAND_ACTION_STATUS == ACTION_IDLE) {
        // Turn wand and pack on.
        WAND_ACTION_STATUS = ACTION_ACTIVATE;
      }

      soundBeepLoopStop();
    break;

    case MODE_ERROR:
      if(switch_activate.getState() == HIGH) {
        wandOff();
      }
    break;

    case MODE_ON:
      // This is for when the mode switch is enabled for video game mode. b_cross_the_streams must not be enabled.
      if(WAND_ACTION_STATUS != ACTION_FIRING && WAND_ACTION_STATUS != ACTION_OFF && WAND_ACTION_STATUS != ACTION_OVERHEATING && b_cross_the_streams != true && b_pack_alarm != true) {
        if(switchMode() == true) {
          // Only exit the settings menu when on menu #5 and or cycle through modes when the settings menu is on menu #5
          if(i_wand_menu == 5) {
            // Cycle through the firing modes and setting menu.
            if(FIRING_MODE == PROTON) {
              FIRING_MODE = SLIME;
            }
            else if(FIRING_MODE == SLIME) {
              FIRING_MODE = STASIS;
            }
            else if(FIRING_MODE == STASIS) {
              FIRING_MODE = MESON;
            }
            else if(FIRING_MODE == MESON) {
              FIRING_MODE = VENTING;
            }
            else if(FIRING_MODE == VENTING) {
              FIRING_MODE = SETTINGS;
            }
            else {
              FIRING_MODE = PROTON;

              bargraphClearAlt();

              // If using the 28 segment bargraph, in Afterlife, we need to redraw the segments.
              // 1984/1989 years will go in to a auto ramp and do not need a manual refresh.
              #ifdef GPSTAR_NEUTRONA_WAND_PCB
                if(year_mode == 2021 && b_28segment_bargraph == true) {
                  bargraphPowerCheck2021Alt(true);
                }
              #endif
            }

            // Make sure the slo-blo light is turned back on, as entering venting mode will make it blink.
            if(FIRING_MODE != VENTING) {
              analogWrite(led_slo_blo, 255);

              // If using the gpstar neutrona wand micro controller, the front left LED is wired separately, lets turn it on.
              #ifdef GPSTAR_NEUTRONA_WAND_PCB
                analogWrite(led_front_left, 255);
              #endif
            }

            playEffect(S_CLICK);

            switch(FIRING_MODE) {
              case SETTINGS:
                WAND_ACTION_STATUS = ACTION_SETTINGS;
                i_wand_menu = 5;
                ms_settings_blinking.start(i_settings_blinking_delay);

                // Clear the bargraph.
                bargraphClearAlt();

                // Tell the pack we are in settings mode.
                wandSerialSend(W_SETTINGS_MODE);
              break;

              case VENTING:
                WAND_ACTION_STATUS = ACTION_IDLE;
                wandHeatUp();

                // The the pack we are in venting mode.
                wandSerialSend(W_VENTING_MODE);
              break;

              case MESON:
                WAND_ACTION_STATUS = ACTION_IDLE;
                wandHeatUp();

                // Tell the pack we are in meson mode.
                wandSerialSend(W_MESON_MODE);
              break;

              case STASIS:
                WAND_ACTION_STATUS = ACTION_IDLE;
                wandHeatUp();

                // Tell the pack we are in stasis mode.
                wandSerialSend(W_STASIS_MODE);
              break;

              case SLIME:
                WAND_ACTION_STATUS = ACTION_IDLE;
                wandHeatUp();

                // Tell the pack we are in slime mode.
                wandSerialSend(W_SLIME_MODE);
              break;

              case PROTON:
                WAND_ACTION_STATUS = ACTION_IDLE;
                wandHeatUp();

                // Tell the pack we are in proton mode.
                wandSerialSend(W_PROTON_MODE);
              break;
            }
          }
        }
      }

      if(WAND_ACTION_STATUS != ACTION_OVERHEATING && b_pack_alarm != true) {
        // Vent light and first stage of the safety system.
        if(switch_vent.getState() == LOW) {
          // Vent light and top white light on.
          digitalWrite(led_vent, LOW);

          soundIdleStart();

          if(switch_wand.getState() == LOW) {
            if(b_beeping != true) {
              // Beep loop.
              soundBeepLoop();
            }
          }
          else {
            soundBeepLoopStop();
          }
        }
        else if(switch_vent.getState() == HIGH) {
          // Vent light and top white light off.
          digitalWrite(led_vent, HIGH);

          soundBeepLoopStop();
          soundIdleStop();
        }
      }

      if(WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING && b_pack_alarm != true) {
        if(switch_intensify.getState() == LOW && ms_intensify_timer.isRunning() != true && switch_wand.getState() == LOW && switch_vent.getState() == LOW && switch_activate.getState() == LOW && b_pack_on == true && switchBarrel() != true && b_pack_alarm != true) {
          if(WAND_ACTION_STATUS != ACTION_FIRING) {
            WAND_ACTION_STATUS = ACTION_FIRING;
          }

          b_firing_intensify = true;
        }

        // When the mode switch is changed to a alternate firing button. Video game modes are disabled and the wand menu settings can only be accessed when the Neutrona wand is powered down.
        if(b_cross_the_streams == true) {
          if(switchMode() == true && switch_wand.getState() == LOW && switch_vent.getState() == LOW && switch_activate.getState() == LOW && b_pack_on == true && switchBarrel() != true && b_pack_alarm != true) {
            if(WAND_ACTION_STATUS != ACTION_FIRING) {
              WAND_ACTION_STATUS = ACTION_FIRING;
            }

            b_firing_alt = true;
          }
          else if(switchMode() != true && ms_switch_mode_debounce.remaining() < 1) {
            if(b_firing_intensify != true && WAND_ACTION_STATUS == ACTION_FIRING) {
              WAND_ACTION_STATUS = ACTION_IDLE;
            }

            b_firing_alt = false;
          }
        }

        if(switch_intensify.getState() == HIGH && b_firing == true && b_firing_intensify == true) {
          if(b_firing_alt != true) {
            WAND_ACTION_STATUS = ACTION_IDLE;
          }

          b_firing_intensify = false;
        }

        if(switch_activate.getState() == HIGH) {
          WAND_ACTION_STATUS = ACTION_OFF;
        }
      }
      else if(WAND_ACTION_STATUS == ACTION_OVERHEATING || b_pack_alarm == true) {
        if(switch_activate.getState() == HIGH) {
          WAND_ACTION_STATUS = ACTION_OFF;
        }
      }
    break;
  }
}

void wandOff() {
  if(WAND_ACTION_STATUS != ACTION_ERROR) {
    // Tell the pack the wand is turned off.
    wandSerialSend(W_OFF);
  }
  else {
    // Important to turn off looping on these tracks. Otherwise the bargraph beep or other can be used in the settings menu and be stuck in a loop.
    stopEffect(S_BEEPS_LOW);
    stopEffect(S_BEEPS);
    stopEffect(S_BEEPS_BARGRAPH);
  }

  if(FIRING_MODE == SETTINGS) {
    // If the wand is shut down while we are in settings mode (can happen if the pack is manually turned off), switch the wand and pack to proton mode.
    wandSerialSend(W_PROTON_MODE);
    FIRING_MODE = PROTON;
  }

  WAND_STATUS = MODE_OFF;
  WAND_ACTION_STATUS = ACTION_IDLE;

  soundBeepLoopStop();
  soundIdleStop();
  soundIdleLoopStop();

  vibrationOff();

  // Stop firing if the wand is turned off.
  if(b_firing == true) {
    modeFireStop();
  }

  stopEffect(S_AFTERLIFE_GUN_LOOP_1);
  stopEffect(S_AFTERLIFE_GUN_LOOP_2);

  stopEffect(S_AFTERLIFE_GUN_RAMP_1);
  stopEffect(S_AFTERLIFE_GUN_RAMP_2);
  stopEffect(S_AFTERLIFE_GUN_RAMP_DOWN_1);
  stopEffect(S_AFTERLIFE_GUN_RAMP_DOWN_2);
  stopEffect(S_BOOTUP);

  // Turn off any overheating sounds.
  stopEffect(S_CLICK);
  stopEffect(S_VENT_DRY);

  stopEffect(S_FIRE_START_SPARK);
  stopEffect(S_PACK_SLIME_OPEN);
  stopEffect(S_STASIS_START);
  stopEffect(S_MESON_START);

  playEffect(S_WAND_SHUTDOWN);
  playEffect(S_AFTERLIFE_GUN_RAMP_DOWN_1);

  // Turn off some timers.
  ms_bargraph.stop();
  ms_bargraph_alt.stop();
  ms_bargraph_firing.stop();
  ms_overheat_initiate.stop();
  ms_overheating.stop();
  ms_settings_blinking.stop();
  ms_hat_1.stop();
  ms_hat_2.stop();

  // Turn off remaining lights.
  wandLightsOff();
  barrelLightsOff();

  switch(year_mode) {
    case 2021:
      i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_2021;
    break;

    case 1984:
    case 1989:
      i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_1984;
    break;
  }
}

void modeActivate() {
  // The wand was started while the top switch was already on. Lets put the wand into a startup error mode.
  if(switch_wand.getState() == LOW && b_wand_boot_errors == true) {
    ms_hat_2.start(i_hat_2_delay);

    // This is used for controlling a bargraph beep in a boot up error.
    ms_hat_1.start(i_hat_2_delay * 4);

    WAND_STATUS = MODE_ERROR;
    WAND_ACTION_STATUS = ACTION_ERROR;

    ms_settings_blinking.start(i_settings_blinking_delay);

    playEffect(S_BEEPS_LOW);

    playEffect(S_BEEPS);

    playEffect(S_BEEPS_BARGRAPH);
  }
  else {
    WAND_STATUS = MODE_ON;

    // Proper startup. Continue booting up the wand.
    WAND_ACTION_STATUS = ACTION_IDLE;

    // Tell the pack the wand is turned on.
    wandSerialSend(W_ON);
  }

  // Ramp up the bargraph.
  switch(year_mode) {
    case 2021:
      i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_2021;
    break;

    case 1984:
    case 1989:
      i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_1984 * 2;
    break;
  }

  if(WAND_STATUS != MODE_ERROR) {
    if(b_pack_alarm != true) {
      bargraphRampUp();
    }

    // Turn on slo-blo light (and front left LED if using a Ardunio Nano).
    analogWrite(led_slo_blo, 255);

    // If using the gpstar neutrona wand micro controller, the front left LED is wired separately, lets turn it on.
    #ifdef GPSTAR_NEUTRONA_WAND_PCB
      analogWrite(led_front_left, 255);
    #endif

    // Top white light.
    ms_white_light.start(d_white_light_interval);
    digitalWrite(led_white, LOW);

    if(b_pack_alarm != true) {
      switch(year_mode) {
        case 1984:
        case 1989:
          playEffect(S_CLICK);
        break;

        default:
          soundIdleLoop(true);

          playEffect(S_AFTERLIFE_GUN_RAMP_1, false, i_volume - 10);
          ms_gun_loop_1.start(2000);
        break;
      }
    }
  }
}

void soundIdleLoop(bool fadeIn) {
  switch(i_power_mode) {
    case 1:
      playEffect(S_IDLE_LOOP_GUN_1, true, i_volume, fadeIn, 1000);
    break;

    case 2:
      playEffect(S_IDLE_LOOP_GUN_1, true, i_volume, fadeIn, 1000);
    break;

    case 3:
      playEffect(S_IDLE_LOOP_GUN_2, true, i_volume, fadeIn, 1000);
    break;

    case 4:
      playEffect(S_IDLE_LOOP_GUN_2, true, i_volume, fadeIn, 1000);
    break;

    case 5:
      playEffect(S_IDLE_LOOP_GUN_5, true, i_volume, fadeIn, 1000);
    break;
  }
}

void soundIdleLoopStop() {
  stopEffect(S_IDLE_LOOP_GUN);
  stopEffect(S_IDLE_LOOP_GUN_1);
  stopEffect(S_IDLE_LOOP_GUN_2);
  stopEffect(S_IDLE_LOOP_GUN_3);
  stopEffect(S_IDLE_LOOP_GUN_4);
  stopEffect(S_IDLE_LOOP_GUN_5);
}

void soundIdleStart() {
  if(b_sound_idle == false) {
    switch(year_mode) {
      case 1984:
      case 1989:
        playEffect(S_BOOTUP);

        soundIdleLoop(true);
      break;

      default:
        stopEffect(S_AFTERLIFE_GUN_RAMP_1);
        stopEffect(S_AFTERLIFE_GUN_LOOP_2);
        stopEffect(S_AFTERLIFE_GUN_RAMP_DOWN_1);
        stopEffect(S_AFTERLIFE_GUN_RAMP_DOWN_2);

        playEffect(S_AFTERLIFE_GUN_RAMP_2, false, i_volume - 10);

        ms_gun_loop_1.stop();
        ms_gun_loop_2.start(1500);
      break;
    }
  }

  if(b_sound_idle == false) {
    b_sound_idle = true;
  }

  if(year_mode == 2021) {
    if(ms_gun_loop_2.justFinished()) {
      playEffect(S_AFTERLIFE_GUN_LOOP_2, true, i_volume - 10);

      ms_gun_loop_2.stop();
    }
  }
}

void soundIdleStop() {
  if(b_sound_idle == true) {
    switch(year_mode) {
      case 1984:
      case 1989:
        playEffect(S_WAND_SHUTDOWN);
      break;

      default:
        if(WAND_ACTION_STATUS == ACTION_OVERHEATING) {
          playEffect(S_WAND_SHUTDOWN);
        }

        playEffect(S_AFTERLIFE_GUN_RAMP_DOWN_2, false, i_volume - 8);

        if(WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          ms_gun_loop_1.start(1700);
          ms_gun_loop_2.stop();
        }
      break;
    }
  }

  if(b_sound_idle == true) {
    switch(year_mode) {
      case 1984:
      case 1989:
        stopEffect(S_BOOTUP);
        soundIdleLoopStop();
      break;

      case 2021:
        stopEffect(S_AFTERLIFE_GUN_RAMP_2);
        stopEffect(S_AFTERLIFE_GUN_LOOP_2);
      break;
    }
  }

  b_sound_idle = false;
}

void soundBeepLoopStop() {
  if(b_beeping == true) {
    b_beeping = false;

    stopEffect(S_AFTERLIFE_BEEP_WAND);
    stopEffect(S_AFTERLIFE_BEEP_WAND_S1);
    stopEffect(S_AFTERLIFE_BEEP_WAND_S2);
    stopEffect(S_AFTERLIFE_BEEP_WAND_S3);
    stopEffect(S_AFTERLIFE_BEEP_WAND_S4);
    stopEffect(S_AFTERLIFE_BEEP_WAND_S5);

    ms_reset_sound_beep.stop();
    ms_reset_sound_beep.start(i_sound_timer);
  }
}
void soundBeepLoop() {
  if(ms_reset_sound_beep.justFinished() && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
    if(b_beeping == false) {
      switch(i_power_mode) {
        case 1:
          if(year_mode == 2021) {
            playEffect(S_AFTERLIFE_BEEP_WAND_S1, true);
          }
          else {
            playEffect(S_AFTERLIFE_BEEP_WAND_S1);
          }
        break;

        case 2:
         if(year_mode == 2021) {
            playEffect(S_AFTERLIFE_BEEP_WAND_S2, true);
          }
          else {
            playEffect(S_AFTERLIFE_BEEP_WAND_S2);
          }
        break;

        case 3:
         if(year_mode == 2021) {
            playEffect(S_AFTERLIFE_BEEP_WAND_S3, true);
          }
          else {
            playEffect(S_AFTERLIFE_BEEP_WAND_S3);
          }
        break;

        case 4:
         if(year_mode == 2021) {
            playEffect(S_AFTERLIFE_BEEP_WAND_S4, true);
          }
          else {
            playEffect(S_AFTERLIFE_BEEP_WAND_S4);
          }
        break;

        case 5:
         if(year_mode == 2021) {
            playEffect(S_AFTERLIFE_BEEP_WAND_S5, true);
          }
          else {
            playEffect(S_AFTERLIFE_BEEP_WAND_S5);
          }
        break;
      }

      b_beeping = true;

      ms_reset_sound_beep.stop();
    }
  }
}

void modeFireStartSounds() {
  ms_firing_start_sound_delay.stop();

  // Some sparks for firing start.
  if(year_mode == 1989) {
    playEffect(S_FIRE_START_SPARK, false, i_volume - 10);
  }
  else {
    playEffect(S_FIRE_START_SPARK);
  }

  switch(FIRING_MODE) {
    case PROTON:
        switch(i_power_mode) {
          case 1 ... 4:
            if(b_firing_intensify == true) {
              // Reset some sound triggers.
              b_sound_firing_intensify_trigger = true;

              if(year_mode == 1989) {
                playEffect(S_GB2_FIRE_START);
                playEffect(S_GB2_FIRE_LOOP, true);
              }
              else {
                playEffect(S_GB1_FIRE_START);
                playEffect(S_GB1_FIRE_LOOP, true);
              }
            }
            else {
              b_sound_firing_intensify_trigger = false;
            }

            if(b_firing_alt == true) {
              // Reset some sound triggers.
              b_sound_firing_alt_trigger = true;

              playEffect(S_FIRE_START);
              playEffect(S_FIRING_LOOP_GB1, true);
            }
            else {
              b_sound_firing_alt_trigger = false;
            }
          break;

          case 5:
            if(b_firing_intensify == true) {
              // Reset some sound triggers.
              b_sound_firing_intensify_trigger = true;
              playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true);
            }
            else {
              b_sound_firing_intensify_trigger = false;
            }

            if(b_firing_alt == true) {
              // Reset some sound triggers.
              b_sound_firing_alt_trigger = true;

              playEffect(S_FIRING_LOOP_GB1, true);
            }
            else {
              b_sound_firing_alt_trigger = false;
            }

            playEffect(S_GB1_FIRE_START_HIGH_POWER);
          break;
        }
    break;

    case SLIME:
      playEffect(S_SLIME_START);
      playEffect(S_SLIME_LOOP, true);
    break;

    case STASIS:
      playEffect(S_STASIS_START);
      playEffect(S_STASIS_LOOP, true);
    break;

    case MESON:
      playEffect(S_MESON_START);
      playEffect(S_MESON_LOOP, true);
    break;

    case VENTING:
    case SETTINGS:
      // Nothing.
    break;
  }
}

void modeFireStart() {
  // Reset some sound triggers.
  b_sound_firing_intensify_trigger = true;
  b_sound_firing_alt_trigger = true;
  b_sound_firing_cross_the_streams = false;
  b_firing_cross_streams = false;

  bargraphClearAlt();

  // Turn on hat light 1.
  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    digitalWrite(led_hat_1, HIGH);
  #endif

  ms_hat_1.stop();

  if(ms_intensify_timer.isRunning() != true) {
    ms_intensify_timer.start(i_intensify_delay);
  }

  // Tell the Proton Pack that the Neutrona wand is firing in Intensify mode.
  if(b_firing_intensify == true) {
    wandSerialSend(W_FIRING_INTENSIFY);
  }

  // Tell the Proton Pack that the Neutrona wand is firing in Alt mode.
  if(b_firing_alt == true) {
    wandSerialSend(W_FIRING_ALT);
  }

  // Stop all firing sounds first.
  switch(FIRING_MODE) {
    case PROTON:
      if(year_mode == 1989) {
        stopEffect(S_GB2_FIRE_START);
        stopEffect(S_GB2_FIRE_LOOP);
      }
      else {
        stopEffect(S_GB1_FIRE_START);
        stopEffect(S_GB1_FIRE_LOOP);
      }

      stopEffect(S_GB1_FIRE_START_HIGH_POWER);
      stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);

      stopEffect(S_FIRING_LOOP_GB1);

      stopEffect(S_FIRE_START_SPARK);
      stopEffect(S_FIRING_END_GUN);

      #ifdef GPSTAR_NEUTRONA_WAND_PCB
        stopEffect(S_FIRE_LOOP_IMPACT);
      #endif
    break;

    case SLIME:
      stopEffect(S_SLIME_START);
      stopEffect(S_SLIME_LOOP);
      stopEffect(S_SLIME_END);
    break;

    case STASIS:
      stopEffect(S_STASIS_START);
      stopEffect(S_STASIS_LOOP);
      stopEffect(S_STASIS_END);
    break;

    case MESON:
      stopEffect(S_MESON_START);
      stopEffect(S_MESON_LOOP);
      stopEffect(S_MESON_END);
    break;

    case VENTING:
    case SETTINGS:
      // Nothing.
    break;
  }

  ms_firing_start_sound_delay.start(i_fire_stop_sound_delay);

  // Tell the pack the wand is firing.
  wandSerialSend(W_FIRING);

  ms_overheat_initiate.stop();

  // This will only overheat when enabled by using the alt firing when in crossing the streams mode.
  bool b_overheat_flag = true;

  if(b_cross_the_streams == true && b_firing_alt != true) {
    b_overheat_flag = false;
  }

  if(b_overheat_flag == true) {
    // If in high power mode on the wand, start a overheat timer.
    if(b_overheat_mode[i_power_mode - 1] == true && b_overheat_enabled == true) {
      ms_overheat_initiate.start(i_ms_overheat_initiate[i_power_mode - 1]);
    }
    else if(b_cross_the_streams == true) {
      if(b_firing_alt == true) {
        ms_overheat_initiate.start(i_ms_overheat_initiate[i_power_mode - 1]);
      }
    }
  }

  barrelLightsOff();

  ms_firing_lights.start(10);
  i_barrel_light = 0;

  // Stop any bargraph ramps.
  ms_bargraph.stop();
  ms_bargraph_alt.stop();
  b_bargraph_up = false;
  i_bargraph_status = 1;
  i_bargraph_status_alt = 0;
  bargraphRampFiring();

  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    ms_impact.start(random(10,15) * 1000);
  #endif
}

void modeFireStopSounds() {
  // Reset some sound triggers.
  b_sound_firing_intensify_trigger = false;
  b_sound_firing_alt_trigger = false;
  b_sound_firing_cross_the_streams = false;

  ms_firing_stop_sound_delay.stop();

 switch(FIRING_MODE) {
    case PROTON:
      playEffect(S_FIRING_END_GUN);
    break;

    case SLIME:
      playEffect(S_SLIME_END);
    break;

    case STASIS:
      playEffect(S_STASIS_END);
    break;

    case MESON:
      playEffect(S_MESON_END);
    break;

    case VENTING:
    case SETTINGS:
      // Nothing.
    break;
  }

  if(b_firing_cross_streams == true) {
    switch(year_mode) {
      case 2021:
        playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume + 10);
      break;

      case 1984:
      case 1989:
        playEffect(S_CROSS_STREAMS_END, false, i_volume + 10);
      break;
    }

    b_firing_cross_streams = false;
  }
}

void modeFireStop() {
  ms_overheat_initiate.stop();

  // Tell the pack the wand stopped firing.
  wandSerialSend(W_FIRING_STOPPED);

  WAND_ACTION_STATUS = ACTION_IDLE;

  b_firing = false;
  b_firing_intensify = false;
  b_firing_alt = false;

  ms_bargraph_firing.stop();
  ms_bargraph_alt.stop(); // Stop the 1984 24 segment optional bargraph timer just in case.
  b_bargraph_up = false;

  i_bargraph_status = i_power_mode - 1;
  i_bargraph_status_alt = 0;
  bargraphClearAlt();

  switch(year_mode) {
    case 2021:
      i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_2021 / 3;
    break;

    case 1984:
    case 1989:
      i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_1984;
    break;
  }

  if(b_pack_alarm == true) {
    // We are going to ramp the bargraph down if the pack alarm happens while we were firing.
    prepBargraphRampDown();
  }
  else {
    // We ramp the bargraph back up after finishing firing.
    bargraphRampUp();
  }

  ms_firing_stream_blue.stop();
  ms_firing_lights.stop();

  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    ms_impact.stop();
  #endif

  i_barrel_light = 0;
  ms_firing_lights_end.start(10);

  // If using optional items on the gpstar Neutrona Wand micro controller.
  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    digitalWrite(led_hat_1, LOW); // Turn off hat light 1.
    digitalWrite(led_barrel_tip, LOW); // Turn off hat the wand barrel tip LED.
  #endif

  ms_hat_1.stop();

  // Stop all other firing sounds.
  switch(FIRING_MODE) {
    case PROTON:
      if(year_mode == 1989) {
        stopEffect(S_GB2_FIRE_START);
        stopEffect(S_GB2_FIRE_LOOP);
      }
      else {
        stopEffect(S_GB1_FIRE_START);
        stopEffect(S_GB1_FIRE_LOOP);
      }

      stopEffect(S_FIRING_LOOP_GB1);
      stopEffect(S_GB1_FIRE_START_HIGH_POWER);
      stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);
      stopEffect(S_FIRE_START_SPARK);

      #ifdef GPSTAR_NEUTRONA_WAND_PCB
        stopEffect(S_FIRE_LOOP_IMPACT);
      #endif
    break;

    case SLIME:
      stopEffect(S_SLIME_START);
      stopEffect(S_SLIME_LOOP);
      stopEffect(S_SLIME_END);
    break;

    case STASIS:
      stopEffect(S_STASIS_START);
      stopEffect(S_STASIS_LOOP);
      stopEffect(S_STASIS_END);
    break;

    case MESON:
      stopEffect(S_MESON_START);
      stopEffect(S_MESON_LOOP);
      stopEffect(S_MESON_END);
    break;

    case VENTING:
    case SETTINGS:
      // Nothing.
    break;
  }

  // A tiny ramp down delay, helps with the sounds.
  ms_firing_stop_sound_delay.start(i_fire_stop_sound_delay);
}

void modeFiring() {
  // Sound trigger flags.
  if(b_firing_intensify == true && b_sound_firing_intensify_trigger != true) {
    b_sound_firing_intensify_trigger = true;

    if(b_cross_the_streams_mix == true) {
      // Tell the Proton Pack that the Neutrona wand is firing in Intensify mode mix.
      wandSerialSend(W_FIRING_INTENSIFY_MIX);

      switch(i_power_mode) {
        case 1 ... 4:
          if(year_mode == 1989) {
            playEffect(S_GB2_FIRE_START);
            playEffect(S_GB2_FIRE_LOOP, true);
          }
          else {
            playEffect(S_GB1_FIRE_START);
            playEffect(S_GB1_FIRE_LOOP, true);
          }
        break;

        case 5:
            playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true);
        break;
      }
    }
    else {
      // Tell the Proton Pack that the Neutrona wand is firing in Intensify mode.
      wandSerialSend(W_FIRING_INTENSIFY);
    }
  }

  if(b_firing_intensify != true && b_sound_firing_intensify_trigger == true) {
    b_sound_firing_intensify_trigger = false;

    if(b_cross_the_streams_mix == true) {
      // Tell the Proton Pack that the Neutrona wand is no longer firing in Intensify mode mix.
      wandSerialSend(W_FIRING_INTENSIFY_STOPPED_MIX);

      switch(i_power_mode) {
        case 1 ... 4:
          if(year_mode == 1989) {
            stopEffect(S_GB2_FIRE_LOOP);
            stopEffect(S_GB2_FIRE_START);
          }
          else {
            stopEffect(S_GB1_FIRE_LOOP);
            stopEffect(S_GB1_FIRE_LOOP);
            stopEffect(S_GB1_FIRE_START);
          }
        break;

        case 5:
          stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);
          stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);
        break;
      }
    }
    else {
      // Tell the Proton Pack that the Neutrona wand is no longer firing in Intensify mode.
      wandSerialSend(W_FIRING_INTENSIFY_STOPPED);
    }
  }

  if(b_firing_alt == true && b_sound_firing_alt_trigger != true) {
    b_sound_firing_alt_trigger = true;

    if(b_cross_the_streams_mix == true) {
      playEffect(S_FIRING_LOOP_GB1, true);

      // Tell the Proton Pack that the Neutrona wand is firing in Alt mode mix.
      wandSerialSend(W_FIRING_ALT_MIX);
    }
    else {
      // Tell the Proton Pack that the Neutrona wand is firing in Alt mode.
      wandSerialSend(W_FIRING_ALT);
    }
  }

  if(b_firing_alt != true && b_sound_firing_alt_trigger == true) {
    b_sound_firing_alt_trigger = false;

    if(b_cross_the_streams_mix == true) {
      stopEffect(S_FIRING_LOOP_GB1);

      // Tell the Proton Pack that the Neutrona wand is no longer firing in Alt mode mix.
      wandSerialSend(W_FIRING_ALT_STOPPED_MIX);
    }
    else {
      // Tell the Proton Pack that the Neutrona wand is no longer firing in Alt mode.
      wandSerialSend(W_FIRING_ALT_STOPPED);
    }
  }

  if(b_firing_alt == true && b_firing_intensify == true && b_sound_firing_cross_the_streams != true && b_firing_cross_streams != true) {
    b_firing_cross_streams = true;
    b_sound_firing_cross_the_streams = true;

    switch(year_mode) {
      case 2021:
        playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START, false, i_volume + 10);
      break;

      case 1984:
      case 1989:
        playEffect(S_CROSS_STREAMS_START, false, i_volume + 10);
      break;
    }

    playEffect(S_FIRE_START_SPARK);

    if(b_cross_the_streams_mix != true) {
      // Tell the Proton Pack that the Neutrona wand is crossing the streams mix.
      wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_MIX);

      playEffect(S_FIRING_LOOP_GB1, true);

      if(i_power_mode != i_power_mode_max) {
        playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true);
      }

      stopEffect(S_GB2_FIRE_LOOP);
      stopEffect(S_GB1_FIRE_LOOP);
    }
    else {
      // Tell the Proton Pack that the Neutrona wand is crossing the streams.
      wandSerialSend(W_FIRING_CROSSING_THE_STREAMS);
    }
  }

  if((b_firing_alt != true && b_firing_intensify != true) && b_firing_cross_streams == true && b_cross_the_streams_mix != true) {
    // Can let go of a button and still fires.
    // Tell the Proton Pack that the Neutrona wand is no longer crossing the streams.
    wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_STOPPED);

    b_firing_cross_streams = false;
    b_sound_firing_cross_the_streams = false;

    switch(year_mode) {
      case 2021:
        playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume + 10);
      break;

      case 1984:
      case 1989:
        playEffect(S_CROSS_STREAMS_END, false, i_volume + 10);
      break;
    }

    stopEffect(S_FIRING_LOOP_GB1);
  }
  else if((b_firing_alt != true || b_firing_intensify != true) && b_firing_cross_streams == true && b_cross_the_streams_mix == true) {
    // Let go of a button and it reverts back to the other firing mode.
    // Tell the Proton Pack that the Neutrona wand is no longer crossing the streams.
    wandSerialSend(W_FIRING_CROSSING_THE_STREAMS_STOPPED_MIX);

    b_firing_cross_streams = false;
    b_sound_firing_cross_the_streams = false;

    switch(year_mode) {
      case 2021:
        playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume + 10);
      break;

      case 1984:
      case 1989:
        playEffect(S_CROSS_STREAMS_END, false, i_volume + 10);
      break;
    }
  }

  // Overheat timers.
  bool b_overheat_flag = true;

  if(b_cross_the_streams == true && b_firing_alt != true) {
    b_overheat_flag = false;
  }

  if(b_overheat_flag == true) {
    // If the user changes the wand power output while firing, turn off the overheat timer.
    if(b_overheat_mode[i_power_mode - 1] != true && ms_overheat_initiate.isRunning()) {
      ms_overheat_initiate.stop();

      // Adjust hat light 1 to stay solid.
      #ifdef GPSTAR_NEUTRONA_WAND_PCB
        digitalWrite(led_hat_1, HIGH);
      #endif

      ms_hat_1.stop();

      // Tell the pack to revert back to regular cyclotron speeds.
      wandSerialSend(W_CYCLOTRON_NORMAL_SPEED);
    }
    else if(b_overheat_mode[i_power_mode - 1] == true && ms_overheat_initiate.remaining() == 0 && b_overheat_enabled == true) {
      // If the user changes back to power mode that overheats while firing, start up a timer.
      // This currently works only in power levels 1-4. 5 stays locked when firing.
      ms_overheat_initiate.start(i_ms_overheat_initiate[i_power_mode - 1]);
    }
  }
  else {
    if(ms_overheat_initiate.isRunning()) {
      ms_overheat_initiate.stop();

      // Tell the pack to revert back to regular cyclotron speeds.
      wandSerialSend(W_CYCLOTRON_NORMAL_SPEED);
    }
  }

  switch(FIRING_MODE) {
    case PROTON:
      // Make the stream more slightly more red on higher power modes.
      switch(i_power_mode) {
        case 1:
          fireStreamStart(255, 20, 0);
        break;

        case 2:
          fireStreamStart(255, 30, 0);
        break;

        case 3:
          fireStreamStart(255, 40, 0);
        break;

        case 4:
          fireStreamStart(255, 60, 0);
        break;

        case 5:
          fireStreamStart(255, 70, 0);
        break;

        default:
          fireStreamStart(255, 20, 0);
        break;
      }

      fireStream(0, 0, 255);
    break;

    case SLIME:
       fireStreamStart(0, 255, 45);
       fireStream(20, 200, 45);
    break;

    case STASIS:
       fireStreamStart(0, 45, 100);
       fireStream(0, 100, 255);
    break;

    case MESON:
       fireStreamStart(200, 200, 20);
       fireStream(190, 20, 70);
    break;

    case VENTING:
    case SETTINGS:
      // Nothing.
    break;
  }

  // Bargraph loop / scroll.
  if(ms_bargraph_firing.justFinished()) {
    bargraphRampFiring();
  }

  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    // Mix some impact sound every 10-15 seconds while firing.
    if(ms_impact.justFinished()) {
      playEffect(S_FIRE_LOOP_IMPACT);
      ms_impact.start(15000);
    }
  #endif
}

void wandHeatUp() {
  stopEffect(S_FIRE_START_SPARK);
  stopEffect(S_PACK_SLIME_OPEN);
  stopEffect(S_STASIS_OPEN);
  stopEffect(S_MESON_OPEN);
  stopEffect(S_VENT_DRY);
  stopEffect(S_VENT_SMOKE);
  stopEffect(S_MODE_SWITCH);

  switch(FIRING_MODE) {
    case PROTON:
      playEffect(S_FIRE_START_SPARK);
    break;

    case SLIME:
      playEffect(S_PACK_SLIME_OPEN);
    break;

    case STASIS:
      playEffect(S_STASIS_OPEN);
    break;

    case MESON:
      playEffect(S_MESON_OPEN);
    break;

    case VENTING:
      playEffect(S_VENT_DRY);
      playEffect(S_MODE_SWITCH);
    break;

    case SETTINGS:
      // Nothing.
    break;
  }

  wandBarrelPreHeatUp();
}

void wandBarrelPreHeatUp() {
  i_heatup_counter = 0;
  i_heatdown_counter = 100;
  ms_wand_heatup_fade.start(i_delay_heatup);
}

void wandBarrelHeatUp() {
  if(i_heatup_counter > 100) {
    wandBarrelHeatDown();
  }
  else if(ms_wand_heatup_fade.justFinished() && i_heatup_counter <= 100) {
    switch(FIRING_MODE) {
      case PROTON:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(i_heatup_counter, i_heatup_counter, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case SLIME:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(i_heatup_counter, 0, 0);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case STASIS:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(0, 0, i_heatup_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case MESON:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(i_heatup_counter, i_heatup_counter, 0);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case VENTING:
      case SETTINGS:
        // nothing
      break;
    }

    i_heatup_counter++;
    ms_wand_heatup_fade.start(i_delay_heatup);
  }
}

void wandBarrelHeatDown() {
  if(ms_wand_heatup_fade.justFinished() && i_heatdown_counter > 0) {
    switch(FIRING_MODE) {
      case PROTON:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(i_heatdown_counter, i_heatdown_counter, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case SLIME:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(i_heatdown_counter, 0, 0);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case STASIS:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(0, 0, i_heatdown_counter);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case MESON:
        barrel_leds[BARREL_NUM_LEDS - 1] = CRGB(i_heatdown_counter, i_heatdown_counter, 0);
        ms_fast_led.start(i_fast_led_delay);
      break;

      case VENTING:
      case SETTINGS:
        // Nothing.
      break;
    }

    i_heatdown_counter--;

    ms_wand_heatup_fade.start(i_delay_heatup);
  }

  if(i_heatdown_counter == 0) {
    barrelLightsOff();
  }
}

void fireStream(uint8_t r, uint8_t g, uint8_t b) {
  if(ms_firing_stream_blue.justFinished()) {
    if(i_barrel_light - 1 > -1 && i_barrel_light - 1 < BARREL_NUM_LEDS) {
      switch(FIRING_MODE) {
        case PROTON:
          if(b_firing_cross_streams == true) {
            barrel_leds[i_barrel_light - 1] = CRGB(255, 255, 255);
          }
          else {
            // Make the stream more slightly more red on higher power modes.
            switch(i_power_mode) {
              case 1:
                barrel_leds[i_barrel_light - 1] = CRGB(10, 255, 0);
              break;

              case 2:
                barrel_leds[i_barrel_light - 1] = CRGB(20, 255, 0);
              break;

              case 3:
                barrel_leds[i_barrel_light - 1] = CRGB(30, 255, 0);
              break;

              case 4:
                barrel_leds[i_barrel_light - 1] = CRGB(40, 255, 0);
              break;

              case 5:
                barrel_leds[i_barrel_light - 1] = CRGB(50, 255, 0);
              break;

              default:
                barrel_leds[i_barrel_light - 1] = CRGB(10, 255, 0);
              break;
            }
          }
        break;

        case SLIME:
          barrel_leds[i_barrel_light - 1] = CRGB(120, 20, 45);
        break;

        case STASIS:
          barrel_leds[i_barrel_light - 1] = CRGB(15, 50, 155);
        break;

        case MESON:
          barrel_leds[i_barrel_light - 1] = CRGB(200, 200, 15);
        break;

        case VENTING:
        case SETTINGS:
          // Nothing.
        break;
      }

      ms_fast_led.start(i_fast_led_delay);
    }

    if(i_barrel_light == BARREL_NUM_LEDS) {
      i_barrel_light = 0;

      ms_firing_stream_blue.start(d_firing_stream / 2);
    }
    else if(i_barrel_light < BARREL_NUM_LEDS) {
      barrel_leds[i_barrel_light] = CRGB(g,r,b);

      ms_fast_led.start(i_fast_led_delay);

      ms_firing_stream_blue.start(d_firing_lights);

      i_barrel_light++;
    }
  }
}

void barrelLightsOff() {
  ms_wand_heatup_fade.stop();
  i_heatup_counter = 0;
  i_heatdown_counter = 100;

  for(uint8_t i = 0; i < BARREL_NUM_LEDS; i++) {
    barrel_leds[i] = CRGB(0,0,0);
  }

  // Turn off the wand barrel tip LED.
  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    digitalWrite(led_barrel_tip, LOW);
  #endif

  ms_fast_led.start(i_fast_led_delay);
}

void fireStreamStart(uint8_t r, uint8_t g, uint8_t b) {
  if(ms_firing_lights.justFinished() && i_barrel_light < BARREL_NUM_LEDS) {
    barrel_leds[i_barrel_light] = CRGB(g,r,b);
    ms_fast_led.start(i_fast_led_delay);

    ms_firing_lights.start(d_firing_lights);

    i_barrel_light++;

    if(i_barrel_light == BARREL_NUM_LEDS) {
      i_barrel_light = 0;

      ms_firing_lights.stop();
      ms_firing_stream_blue.start(d_firing_stream);
    }
  }
}

void fireStreamEnd(uint8_t r, uint8_t g, uint8_t b) {
  if(i_barrel_light < BARREL_NUM_LEDS) {
    barrel_leds[i_barrel_light] = CRGB(g,r,b);
    ms_fast_led.start(i_fast_led_delay);

    ms_firing_lights_end.start(d_firing_lights);

    i_barrel_light++;

    if(i_barrel_light == BARREL_NUM_LEDS) {
      i_barrel_light = 0;

      ms_firing_lights_end.stop();
    }
  }
}

void vibrationWand(uint8_t i_level) {
  if(b_vibration_on == true && b_vibration_enabled == true && WAND_ACTION_STATUS != ACTION_OVERHEATING && b_pack_alarm != true) {
    // Only vibrate the wand during firing only when enabled. (When enabled by the pack)
    if(b_vibration_firing == true) {
      if(WAND_ACTION_STATUS == ACTION_FIRING) {
        if(i_level != i_vibration_level_prev) {
          i_vibration_level_prev = i_level;
          analogWrite(vibration, i_level);
        }
      }
      else {
        i_vibration_level_prev = 0;
        analogWrite(vibration, 0);
      }
    }
    else {
      // Wand vibrates, even when idling, etc. (When enabled by the pack)
      if(i_level != i_vibration_level_prev) {
        i_vibration_level_prev = i_level;
        analogWrite(vibration, i_level);
      }
    }
  }
  else {
    i_vibration_level_prev = 0;
    analogWrite(vibration, 0);
  }
}

// Bargraph ramping during firing.
// Optional barrel LED tip strobing is controlled from here to give it a ramp effect if the Proton Pack and Neutrona Wand are going to overheat.
void bargraphRampFiring() {
  // (Optional) 28 Segment barmeter bargraph.
  if(b_28segment_bargraph == true) {
    #ifdef GPSTAR_NEUTRONA_WAND_PCB
    // Start ramping up and down from the middle to the top/bottom and back to the middle again.
    switch(i_bargraph_status_alt) {
      case 0:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLedNow(i_bargraph[13]);
        ht_bargraph.setLedNow(i_bargraph[14]);

        i_bargraph_status_alt++;

        if(b_bargraph_up == false) {
          ht_bargraph.clearLedNow(i_bargraph[12]);
          ht_bargraph.clearLedNow(i_bargraph[15]);
        }

        b_bargraph_up = true;

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, HIGH);
        #endif
      break;

      case 1:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLedNow(i_bargraph[12]);
        ht_bargraph.setLedNow(i_bargraph[15]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[13]);
          ht_bargraph.clearLedNow(i_bargraph[14]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[11]);
          ht_bargraph.clearLedNow(i_bargraph[16]);

          i_bargraph_status_alt--;
        }

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, HIGH);
        #endif
      break;

      case 2:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLedNow(i_bargraph[11]);
        ht_bargraph.setLedNow(i_bargraph[16]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[12]);
          ht_bargraph.clearLedNow(i_bargraph[15]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[10]);
          ht_bargraph.clearLedNow(i_bargraph[17]);

          i_bargraph_status_alt--;
        }

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, LOW);
        #endif
      break;

      case 3:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLedNow(i_bargraph[10]);
        ht_bargraph.setLedNow(i_bargraph[17]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[11]);
          ht_bargraph.clearLedNow(i_bargraph[16]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[9]);
          ht_bargraph.clearLedNow(i_bargraph[18]);

          i_bargraph_status_alt--;
        }

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, LOW);
        #endif
      break;

      case 4:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLedNow(i_bargraph[9]);
        ht_bargraph.setLedNow(i_bargraph[18]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[10]);
          ht_bargraph.clearLedNow(i_bargraph[17]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[8]);
          ht_bargraph.clearLedNow(i_bargraph[19]);

          i_bargraph_status_alt--;
        }

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, HIGH);
        #endif
      break;

      case 5:
        vibrationWand(i_vibration_level + 110);

        ht_bargraph.setLedNow(i_bargraph[8]);
        ht_bargraph.setLedNow(i_bargraph[19]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[9]);
          ht_bargraph.clearLedNow(i_bargraph[18]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[7]);
          ht_bargraph.clearLedNow(i_bargraph[20]);

          i_bargraph_status_alt--;
        }

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, HIGH);
        #endif
      break;

      case 6:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLedNow(i_bargraph[7]);
        ht_bargraph.setLedNow(i_bargraph[20]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[8]);
          ht_bargraph.clearLedNow(i_bargraph[19]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[6]);
          ht_bargraph.clearLedNow(i_bargraph[21]);

          i_bargraph_status_alt--;
        }

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, LOW);
        #endif
      break;

      case 7:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLedNow(i_bargraph[6]);
        ht_bargraph.setLedNow(i_bargraph[21]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[7]);
          ht_bargraph.clearLedNow(i_bargraph[20]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[5]);
          ht_bargraph.clearLedNow(i_bargraph[22]);

          i_bargraph_status_alt--;
        }

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, LOW);
        #endif
      break;

      case 8:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLedNow(i_bargraph[5]);
        ht_bargraph.setLedNow(i_bargraph[22]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[6]);
          ht_bargraph.clearLedNow(i_bargraph[21]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[4]);
          ht_bargraph.clearLedNow(i_bargraph[23]);

          i_bargraph_status_alt--;
        }

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, HIGH);
        #endif
      break;

      case 9:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLedNow(i_bargraph[4]);
        ht_bargraph.setLedNow(i_bargraph[23]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[5]);
          ht_bargraph.clearLedNow(i_bargraph[22]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[3]);
          ht_bargraph.clearLedNow(i_bargraph[24]);

          i_bargraph_status_alt--;
        }

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, HIGH);
        #endif
      break;

      case 10:
        vibrationWand(i_vibration_level + 112);

        ht_bargraph.setLedNow(i_bargraph[3]);
        ht_bargraph.setLedNow(i_bargraph[24]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[4]);
          ht_bargraph.clearLedNow(i_bargraph[23]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[2]);
          ht_bargraph.clearLedNow(i_bargraph[25]);

          i_bargraph_status_alt--;
        }

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, LOW);
        #endif
      break;

      case 11:
        vibrationWand(i_vibration_level + 115);

        ht_bargraph.setLedNow(i_bargraph[2]);
        ht_bargraph.setLedNow(i_bargraph[25]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[3]);
          ht_bargraph.clearLedNow(i_bargraph[24]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[1]);
          ht_bargraph.clearLedNow(i_bargraph[26]);

          i_bargraph_status_alt--;
        }

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, LOW);
        #endif
      break;

      case 12:
        vibrationWand(i_vibration_level + 115);

        ht_bargraph.setLedNow(i_bargraph[1]);
        ht_bargraph.setLedNow(i_bargraph[26]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[2]);
          ht_bargraph.clearLedNow(i_bargraph[25]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[0]);
          ht_bargraph.clearLedNow(i_bargraph[27]);

          i_bargraph_status_alt--;
        }

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, HIGH);
        #endif
      break;

      case 13:
        vibrationWand(i_vibration_level + 115);

        ht_bargraph.setLedNow(i_bargraph[0]);
        ht_bargraph.setLedNow(i_bargraph[27]);

        ht_bargraph.clearLedNow(i_bargraph[1]);
        ht_bargraph.clearLedNow(i_bargraph[26]);

        i_bargraph_status_alt--;

        b_bargraph_up = false;

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, HIGH);
        #endif
      break;
    }
    #endif
  }
  else {
    // Hasbro bargraph.
    switch(i_bargraph_status) {
      case 1:
        vibrationWand(i_vibration_level + 110);

        digitalWrite(led_bargraph_1, LOW);
        digitalWrite(led_bargraph_2, HIGH);
        digitalWrite(led_bargraph_3, HIGH);
        digitalWrite(led_bargraph_4, HIGH);
        digitalWrite(led_bargraph_5, LOW);
        i_bargraph_status++;

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, HIGH);
        #endif
      break;

      case 2:
        vibrationWand(i_vibration_level + 112);

        digitalWrite(led_bargraph_1, HIGH);
        digitalWrite(led_bargraph_2, LOW);
        digitalWrite(led_bargraph_3, HIGH);
        digitalWrite(led_bargraph_4, LOW);
        digitalWrite(led_bargraph_5, HIGH);
        i_bargraph_status++;

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, LOW);
        #endif
      break;

      case 3:
        vibrationWand(i_vibration_level + 115);

        digitalWrite(led_bargraph_1, HIGH);
        digitalWrite(led_bargraph_2, HIGH);
        digitalWrite(led_bargraph_3, LOW);
        digitalWrite(led_bargraph_4, HIGH);
        digitalWrite(led_bargraph_5, HIGH);
        i_bargraph_status++;

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, HIGH);
        #endif
      break;

      case 4:
        vibrationWand(i_vibration_level + 112);

        digitalWrite(led_bargraph_1, HIGH);
        digitalWrite(led_bargraph_2, LOW);
        digitalWrite(led_bargraph_3, HIGH);
        digitalWrite(led_bargraph_4, LOW);
        digitalWrite(led_bargraph_5, HIGH);
        i_bargraph_status++;

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, LOW);
        #endif
      break;

      case 5:
        vibrationWand(i_vibration_level + 110);

        digitalWrite(led_bargraph_1, LOW);
        digitalWrite(led_bargraph_2, HIGH);
        digitalWrite(led_bargraph_3, HIGH);
        digitalWrite(led_bargraph_4, HIGH);
        digitalWrite(led_bargraph_5, LOW);
        i_bargraph_status = 1;

        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          digitalWrite(led_barrel_tip, HIGH);
        #endif
      break;
    }
  }

  int i_ramp_interval = d_bargraph_ramp_interval;

  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    if(b_28segment_bargraph == true) {
      // Switch to a different ramp speed if using the (Optional) 28 segment barmeter bargraph.
      i_ramp_interval = d_bargraph_ramp_interval_alt;
    }
  #endif

  // If in a power mode on the wand that can overheat, change the speed of the bargraph ramp during firing based on time remaining before we overheat.
  if(b_overheat_mode[i_power_mode - 1] == true && ms_overheat_initiate.isRunning() && b_overheat_enabled == true) {
    /*
    #ifdef GPSTAR_NEUTRONA_WAND_PCB
      for(uint8_t i = 0; i < 13; i++) {
        switch(i) {
          case 12:
            if(ms_overheat_initiate.remaining() < 3000) {
              b_overheat_indicators[i] = true;
            }
            else {
              b_overheat_indicators[i] = false;
            }
          break;

          case 11:
            if(ms_overheat_initiate.remaining() < 3750) {
              b_overheat_indicators[i] = true;
            }
            else {
              b_overheat_indicators[i] = false;
            }
          break;

          case 10:
            if(ms_overheat_initiate.remaining() < 4500) {
              b_overheat_indicators[i] = true;
            }
            else {
              b_overheat_indicators[i] = false;
            }
          break;

          case 9:
            if(ms_overheat_initiate.remaining() < 5250) {
              b_overheat_indicators[i] = true;
            }
            else {
              b_overheat_indicators[i] = false;
            }
          break;

          case 8:
            if(ms_overheat_initiate.remaining() < 6000) {
              b_overheat_indicators[i] = true;
            }
            else {
              b_overheat_indicators[i] = false;
            }
          break;

          case 7:
            if(ms_overheat_initiate.remaining() < 6750) {
              b_overheat_indicators[i] = true;
            }
            else {
              b_overheat_indicators[i] = false;
            }
          break;

          case 6:
            if(ms_overheat_initiate.remaining() < 7500) {
              b_overheat_indicators[i] = true;
            }
            else {
              b_overheat_indicators[i] = false;
            }
          break;

          case 5:
            if(ms_overheat_initiate.remaining() < 6750) {
              b_overheat_indicators[i] = true;
            }
            else {
              b_overheat_indicators[i] = false;
            }
          break;

          case 4:
            if(ms_overheat_initiate.remaining() < 8250) {
              b_overheat_indicators[i] = true;
            }
            else {
              b_overheat_indicators[i] = false;
            }
          break;

          case 3:
            if(ms_overheat_initiate.remaining() < 6750) {
              b_overheat_indicators[i] = true;
            }
            else {
              b_overheat_indicators[i] = false;
            }
          break;

          case 2:
            if(ms_overheat_initiate.remaining() < 9000) {
              b_overheat_indicators[i] = true;
            }
            else {
              b_overheat_indicators[i] = false;
            }
          break;

          case 1:
            if(ms_overheat_initiate.remaining() < 9750) {
              b_overheat_indicators[i] = true;
            }
            else {
              b_overheat_indicators[i] = false;
            }
          break;

          case 0:
            if(ms_overheat_initiate.remaining() < 1050) {
              b_overheat_indicators[i] = true;
            }
            else {
              b_overheat_indicators[i] = false;
            }
          break;

          default:
            b_overheat_indicators[i] = false;
          break;
        }
      }

      wandSerialSend(W_VGA_OVERHEAT_LIGHTS);
    #endif
    */

    if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 6) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start(i_ramp_interval / i_ramp_interval);
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 5);
      }

      cyclotronSpeedUp(6);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 5) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start(i_ramp_interval / 9);
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 4);
      }

      cyclotronSpeedUp(5);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 4) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start(i_ramp_interval / 7);
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 3.5);
      }

      cyclotronSpeedUp(4);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 3) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start(i_ramp_interval / 5);
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 3);
      }

      cyclotronSpeedUp(3);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 2) {
      if(b_28segment_bargraph == true) {
        ms_bargraph_firing.start(i_ramp_interval / 3);
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 2.5);
      }

      cyclotronSpeedUp(2);
    }
    else {
      if(b_28segment_bargraph == true) {
        #ifdef GPSTAR_NEUTRONA_WAND_PCB
          switch(i_power_mode) {
            case 5:
              ms_bargraph_firing.start((i_ramp_interval / 2) - 7); // 13
            break;

            case 4:
              ms_bargraph_firing.start((i_ramp_interval / 2) - 3); // 15
            break;

            case 3:
              ms_bargraph_firing.start(i_ramp_interval / 2); // 20
            break;

            case 2:
              ms_bargraph_firing.start((i_ramp_interval / 2) + 7); // 30
            break;

            case 1:
              ms_bargraph_firing.start((i_ramp_interval / 2) + 12); // 35
            break;
          }
        #endif
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 2);
      }
      i_cyclotron_speed_up = 1;
    }
  }
  else {
    if(b_28segment_bargraph == true) {
      #ifdef GPSTAR_NEUTRONA_WAND_PCB
        switch(i_power_mode) {
          case 5:
            ms_bargraph_firing.start((i_ramp_interval / 2) - 7); // 13
          break;

          case 4:
            ms_bargraph_firing.start((i_ramp_interval / 2) - 3); // 15
          break;

          case 3:
            ms_bargraph_firing.start(i_ramp_interval / 2); // 20
          break;

          case 2:
            ms_bargraph_firing.start((i_ramp_interval / 2) + 7); // 25
          break;

          case 1:
            ms_bargraph_firing.start((i_ramp_interval / 2) + 12); // 30
          break;
        }
      #endif
    }
    else {
      ms_bargraph_firing.start(i_ramp_interval / 2);
    }
  }
}

void cyclotronSpeedUp(uint8_t i_switch) {
  if(i_switch != i_cyclotron_speed_up) {
    if(i_switch == 4) {
      // Tell pack to start beeping before we overheat it.
      wandSerialSend(W_BEEP_START);

      // Beep the wand 8 times.
      playEffect(S_BEEP_8);

      ms_hat_1.start(i_hat_1_delay);
    }

    i_cyclotron_speed_up++;

    // Tell the pack to speed up the cyclotron.
    wandSerialSend(W_CYCLOTRON_INCREASE_SPEED);
  }
}

// 2021 mode for optional 28 segment bargraph.
// Checks if we ramp up or down when changing power levels.
// Forces the bargraph to redraw itself to the current power level.
void bargraphPowerCheck2021Alt(bool b_override) {
  if((WAND_ACTION_STATUS != ACTION_FIRING && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) || b_override == true) {
    if(i_power_mode != i_power_mode_prev || b_override == true) {
      if(i_power_mode > i_power_mode_prev) {
        b_bargraph_up = true;
      }
      else {
        b_bargraph_up = false;
      }
      switch(i_power_mode) {
        case 5:
          ms_bargraph_alt.start(i_bargraph_wait / 3);
        break;

        case 4:
          ms_bargraph_alt.start(i_bargraph_wait / 4);
        break;

        case 3:
          ms_bargraph_alt.start(i_bargraph_wait / 5);
        break;

        case 2:
          ms_bargraph_alt.start(i_bargraph_wait / 6);
        break;

        case 1:
          ms_bargraph_alt.start(i_bargraph_wait / 7);
        break;
      }
    }
  }
}

void bargraphClearAlt() {
  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    if(b_28segment_bargraph == true) {
      ht_bargraph.clearAll();

      i_bargraph_status_alt = 0;
    }
  #endif
}

void bargraphPowerCheck() {
  // Control for the 28 segment barmeter bargraph.
  if(b_28segment_bargraph == true) {
    #ifdef GPSTAR_NEUTRONA_WAND_PCB
      if(ms_bargraph_alt.justFinished()) {
        uint8_t i_bargraph_multiplier[5] = { 7, 6, 5, 4, 3 };

        if(year_mode == 2021) {
          for(uint8_t i = 0; i <= 4; i++) {
            i_bargraph_multiplier[i] = 10;
          }
        }

        if(b_bargraph_up == true) {
          ht_bargraph.setLedNow(i_bargraph[i_bargraph_status_alt]);

          switch(i_power_mode) {
            case 5:
              if(i_bargraph_status_alt > 26) {
                b_bargraph_up = false;

                i_bargraph_status_alt = 27;

                if(year_mode == 2021) {
                  // In 2021 mode, we stop when we reach our target.
                  ms_bargraph_alt.stop();
                }
                else {
                // A little pause when we reach the top.
                  ms_bargraph_alt.start(i_bargraph_wait / 2);
                }
              }
              else {
                ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_mode - 1]);
              }
            break;

            case 4:
              if(i_bargraph_status_alt > 21) {
                b_bargraph_up = false;

                if(year_mode == 2021) {
                  // In 2021 mode, we stop when we reach our target.
                  ms_bargraph_alt.stop();
                }
                else {
                  // A little pause when we reach the top.
                  ms_bargraph_alt.start(i_bargraph_wait / 2);
                }
              }
              else {
                ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_mode - 1]);
              }
            break;

            case 3:
              if(i_bargraph_status_alt > 16) {
                b_bargraph_up = false;
                if(year_mode == 2021) {
                  // In 2021 mode, we stop when we reach our target.
                  ms_bargraph_alt.stop();
                }
                else {
                  // A little pause when we reach the top.
                  ms_bargraph_alt.start(i_bargraph_wait / 2);
                }
              }
              else {
                ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_mode - 1]);
              }
            break;

            case 2:
              if(i_bargraph_status_alt > 10) {
                b_bargraph_up = false;
                if(year_mode == 2021) {
                  // In 2021 mode, we stop when we reach our target.
                  ms_bargraph_alt.stop();
                }
                else {
                  // A little pause when we reach the top.
                  ms_bargraph_alt.start(i_bargraph_wait / 2);
                }
              }
              else {
                ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_mode - 1]);
              }
            break;

            case 1:
              if(i_bargraph_status_alt > 4) {
                b_bargraph_up = false;
                if(year_mode == 2021) {
                  // In 2021 mode, we stop when we reach our target.
                  ms_bargraph_alt.stop();
                }
                else {
                  // A little pause when we reach the top.
                  ms_bargraph_alt.start(i_bargraph_wait / 2);
                }
              }
              else {
                ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[i_power_mode - 1]);
              }
            break;
          }

          if(b_bargraph_up == true) {
            i_bargraph_status_alt++;
          }
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[i_bargraph_status_alt]);

          if(i_bargraph_status_alt == 0) {
            i_bargraph_status_alt = 0;
            b_bargraph_up = true;
            // A little pause when we reach the bottom.
            ms_bargraph_alt.start(i_bargraph_wait / 2);
          }
          else {
            i_bargraph_status_alt--;

            switch(i_power_mode) {
              case 5:
                if(year_mode == 2021 && i_bargraph_status_alt < 27) {
                  // In 2021 mode, we stop when we reach our target.
                  ms_bargraph_alt.stop();
                }
                else {
                  ms_bargraph_alt.start(i_bargraph_interval * 3);
                }
              break;

              case 4:
                if(year_mode == 2021 && i_bargraph_status_alt < 22) {
                  // In 2021 mode, we stop when we reach our target.
                  ms_bargraph_alt.stop();
                }
                else {
                  ms_bargraph_alt.start(i_bargraph_interval * 4);
                }
              break;

              case 3:
                if(year_mode == 2021 && i_bargraph_status_alt < 17) {
                  // In 2021 mode, we stop when we reach our target.
                  ms_bargraph_alt.stop();
                }
                else {
                  ms_bargraph_alt.start(i_bargraph_interval * 5);
                }
              break;

              case 2:
                if(year_mode == 2021 && i_bargraph_status_alt < 11) {
                  // In 2021 mode, we stop when we reach our target.
                  ms_bargraph_alt.stop();
                }
                else {
                  ms_bargraph_alt.start(i_bargraph_interval * 6);
                }
              break;

              case 1:
                if(year_mode == 2021 && i_bargraph_status_alt < 5) {
                  // In 2021 mode, we stop when we reach our target.
                  ms_bargraph_alt.stop();
                }
                else {
                  ms_bargraph_alt.start(i_bargraph_interval * 7);
                }
              break;
            }
          }
        }
      }
    #endif
  }
  else {
    // Stock haslab bargraph control.
    switch(i_power_mode) {
      case 1:
        digitalWrite(led_bargraph_1, LOW);
        digitalWrite(led_bargraph_2, HIGH);
        digitalWrite(led_bargraph_3, HIGH);
        digitalWrite(led_bargraph_4, HIGH);
        digitalWrite(led_bargraph_5, HIGH);
      break;

      case 2:
        digitalWrite(led_bargraph_1, LOW);
        digitalWrite(led_bargraph_2, LOW);
        digitalWrite(led_bargraph_3, HIGH);
        digitalWrite(led_bargraph_4, HIGH);
        digitalWrite(led_bargraph_5, HIGH);
      break;

      case 3:
        digitalWrite(led_bargraph_1, LOW);
        digitalWrite(led_bargraph_2, LOW);
        digitalWrite(led_bargraph_3, LOW);
        digitalWrite(led_bargraph_4, HIGH);
        digitalWrite(led_bargraph_5, HIGH);
      break;

      case 4:
        digitalWrite(led_bargraph_1, LOW);
        digitalWrite(led_bargraph_2, LOW);
        digitalWrite(led_bargraph_3, LOW);
        digitalWrite(led_bargraph_4, LOW);
        digitalWrite(led_bargraph_5, HIGH);
      break;

      case 5:
        digitalWrite(led_bargraph_1, LOW);
        digitalWrite(led_bargraph_2, LOW);
        digitalWrite(led_bargraph_3, LOW);
        digitalWrite(led_bargraph_4, LOW);
        digitalWrite(led_bargraph_5, LOW);
      break;
    }
  }
}

// Fully lights up the bargraphs.
void bargraphFull() {
  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    if(b_28segment_bargraph == true) {
      for(uint8_t i = 0; i < 28; i++) {
        ht_bargraph.setLedNow(i_bargraph[i]);
      }
    }
    else {
      digitalWrite(led_bargraph_1, LOW);
      digitalWrite(led_bargraph_2, LOW);
      digitalWrite(led_bargraph_3, LOW);
      digitalWrite(led_bargraph_4, LOW);
      digitalWrite(led_bargraph_5, LOW);
    }
  #else
    digitalWrite(led_bargraph_1, LOW);
    digitalWrite(led_bargraph_2, LOW);
    digitalWrite(led_bargraph_3, LOW);
    digitalWrite(led_bargraph_4, LOW);
    digitalWrite(led_bargraph_5, LOW);
  #endif
}

void bargraphRampUp() {
  if(i_vibration_level < i_vibration_level_min) {
    i_vibration_level = i_vibration_level_min;
  }

  if(b_28segment_bargraph == true) {
    #ifdef GPSTAR_NEUTRONA_WAND_PCB
      switch(i_bargraph_status_alt) {
        case 0 ... 27:
          ht_bargraph.setLedNow(i_bargraph[i_bargraph_status_alt]);

          if(i_bargraph_status_alt > 22) {
            vibrationWand(i_vibration_level + 80);
          }
          else if(i_bargraph_status_alt > 16) {
            vibrationWand(i_vibration_level + 40);
          }
          else if(i_bargraph_status_alt > 10) {
            vibrationWand(i_vibration_level + 30);
          }
          else if(i_bargraph_status_alt > 4) {
            vibrationWand(i_vibration_level + 20);
          }
          else if(i_bargraph_status_alt > 0) {
            vibrationWand(i_vibration_level + 10);
          }

          i_bargraph_status_alt++;

          if(i_bargraph_status_alt == 28) {
            // A little pause when we reach the top.
            ms_bargraph.start(i_bargraph_wait / 2);

            // Adjust the ramp down speed if necessary.
            switch(year_mode) {
              case 2021:
                i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_2021 / 2;
              break;

              case 1984:
              case 1989:
                // No changes.
              break;
            }
          }
          else {
            ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
          }
        break;

        case 28 ... 56:
          uint8_t i_tmp = i_bargraph_status_alt - 27;
          i_tmp = 28 - i_tmp;

          if(WAND_ACTION_STATUS == ACTION_OVERHEATING || b_pack_alarm == true) {
            vibrationOff();
          }

          if(WAND_ACTION_STATUS == ACTION_OVERHEATING || b_pack_alarm == true) {
            if(i_bargraph_status_alt == 56) {
              ms_bargraph.stop();
              b_bargraph_up = false;
              i_bargraph_status_alt = 0;
            }
            else {
              ht_bargraph.clearLedNow(i_bargraph[i_tmp]);

              ms_bargraph.start(d_bargraph_ramp_interval_alt * 2);
              i_bargraph_status_alt++;
            }
          }
          else {
            if((i_power_mode < 5 && year_mode == 2021) || year_mode == 1984 || year_mode == 1989) {
              ht_bargraph.clearLedNow(i_bargraph[i_tmp]);
            }

            switch(year_mode) {
              case 1984:
              case 1989:
                // Bargraph has ramped up and down. In 1984 mode we want to start the ramping.
                if(i_bargraph_status_alt == 54) {
                  ms_bargraph_alt.start(i_bargraph_interval); // Start the alternate bargraph to ramp up and down continiuously.
                  ms_bargraph.stop();
                  b_bargraph_up = true;
                  i_bargraph_status_alt = 0;
                  bargraphYearModeUpdate();

                  vibrationWand(i_vibration_level);
                }
                else {
                  ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
                  i_bargraph_status_alt++;
                }

              break;

              case 2021:
                switch(i_power_mode) {
                  case 5:
                    // Stop any power check in 2021 if we are already in level 5.
                    ms_bargraph_alt.stop();

                    ms_bargraph.stop();
                    b_bargraph_up = false;
                    i_bargraph_status_alt = 27;
                    bargraphYearModeUpdate();
                    vibrationWand(i_vibration_level + 25);
                  break;

                  case 4:
                    if(i_bargraph_status_alt == 31) {
                      ms_bargraph.stop();
                      b_bargraph_up = false;
                      i_bargraph_status_alt = 23;
                      bargraphYearModeUpdate();

                      vibrationWand(i_vibration_level + 30);
                    }
                    else {
                      ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
                      i_bargraph_status_alt++;

                      vibrationWand(i_vibration_level + 12);
                    }
                  break;

                  case 3:
                    if(i_bargraph_status_alt == 37) {
                      ms_bargraph.stop();
                      b_bargraph_up = false;
                      i_bargraph_status_alt = 17;
                      bargraphYearModeUpdate();

                      vibrationWand(i_vibration_level + 10);
                    }
                    else {
                      ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
                      i_bargraph_status_alt++;

                      vibrationWand(i_vibration_level + 20);
                    }
                  break;

                  case 2:
                    if(i_bargraph_status_alt == 43) {
                      ms_bargraph.stop();
                      b_bargraph_up = false;
                      i_bargraph_status_alt = 11;
                      bargraphYearModeUpdate();

                      vibrationWand(i_vibration_level + 5);
                    }
                    else {
                      ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
                      i_bargraph_status_alt++;

                      vibrationWand(i_vibration_level + 10);
                    }
                  break;

                  case 1:
                    vibrationWand(i_vibration_level);

                    if(i_bargraph_status_alt == 49) {
                      ms_bargraph.stop();
                      b_bargraph_up = false;
                      i_bargraph_status_alt = 5;

                      bargraphYearModeUpdate();
                    }
                    else {
                      ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
                      i_bargraph_status_alt++;
                    }
                  break;
                }
              break;
            }
          }

        break;
      }
    #endif
  }
  else {
    uint8_t t_bargraph_ramp_multiplier = 1;

    if(WAND_ACTION_STATUS == ACTION_OVERHEATING || b_pack_alarm == true) {
      t_bargraph_ramp_multiplier = 2;
    }

    switch(i_bargraph_status) {
      case 0:
        vibrationWand(i_vibration_level + 10);

        digitalWrite(led_bargraph_1, LOW);
        digitalWrite(led_bargraph_2, HIGH);
        digitalWrite(led_bargraph_3, HIGH);
        digitalWrite(led_bargraph_4, HIGH);
        digitalWrite(led_bargraph_5, HIGH);
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 1:
        vibrationWand(i_vibration_level + 20);

        digitalWrite(led_bargraph_2, LOW);
        digitalWrite(led_bargraph_3, HIGH);
        digitalWrite(led_bargraph_4, HIGH);
        digitalWrite(led_bargraph_5, HIGH);
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 2:
        vibrationWand(i_vibration_level + 30);

        digitalWrite(led_bargraph_3, LOW);
        digitalWrite(led_bargraph_4, HIGH);
        digitalWrite(led_bargraph_5, HIGH);
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 3:
        vibrationWand(i_vibration_level + 40);

        digitalWrite(led_bargraph_4, LOW);
        digitalWrite(led_bargraph_5, HIGH);
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 4:
        vibrationWand(i_vibration_level + 80);

        digitalWrite(led_bargraph_5, LOW);

        if(i_bargraph_status + 1 == i_power_mode && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          i_bargraph_status++;
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        }
      break;

      case 5:
        vibrationWand(i_vibration_level + 40);

        digitalWrite(led_bargraph_5, HIGH);

        if(i_bargraph_status - 1 == i_power_mode && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          i_bargraph_status++;
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        }
      break;

      case 6:
        vibrationWand(i_vibration_level + 30);

        digitalWrite(led_bargraph_4, HIGH);

        if(i_bargraph_status - 3 == i_power_mode && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          i_bargraph_status++;
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        }
      break;

      case 7:
        vibrationWand(i_vibration_level + 20);

        digitalWrite(led_bargraph_3, HIGH);

        if(i_bargraph_status - 5 == i_power_mode && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          i_bargraph_status++;
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        }
      break;

      case 8:
        vibrationWand(i_vibration_level + 10);

        digitalWrite(led_bargraph_2, HIGH);

        if(i_bargraph_status - 7 == i_power_mode && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
          i_bargraph_status++;
        }
      break;

      case 9:
        digitalWrite(led_bargraph_1, HIGH);
        ms_bargraph.stop();
        i_bargraph_status = 0;
      break;
    }
  }
}

void prepBargraphRampDown() {
  if(WAND_STATUS == MODE_ON && WAND_ACTION_STATUS == ACTION_IDLE) {
    // If bargraph is set to ramp down during overheat, we need to set a few things.
    soundBeepLoopStop();
    soundIdleStop();
    soundIdleLoopStop();

    b_sound_idle = false; // REMOVE ??
    b_beeping = false;

    // Reset some bargraph levels before we ramp the bargraph down.
    i_bargraph_status_alt = 28; // For 28 segment bargraph
    i_bargraph_status = 5; // For Hasbro 5 LED bargraph.

    bargraphFull();

    ms_bargraph.start(d_bargraph_ramp_interval);

    // Prepare to make the bargraph ramp down now.
    bargraphRampUp();
  }
}

void prepBargraphRampUp() {
  if(WAND_STATUS == MODE_ON && WAND_ACTION_STATUS == ACTION_IDLE) {
    bargraphClearAlt();

    ms_settings_blinking.stop();

    // Prepare a few things before ramping the bargraph back up from a full ramp down.
    if(b_overheat_bargraph_blink != true) {
      playEffect(S_BOOTUP);

      if(year_mode == 2021) {
        bargraphYearModeUpdate();
      }
      else {
        i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_1984 * 2;
      }

      // If using the 28 segment bargraph, in Afterlife, we need to redraw the segments.
      // 1984/1989 years will go in to a auto ramp and do not need a manual refresh.
      if(year_mode == 2021 && b_28segment_bargraph == true) {
        bargraphPowerCheck2021Alt(false);
      }

      updatePackPowerLevel();
      bargraphRampUp();

      if(switch_vent.getState() == LOW) {
        soundIdleLoop(true);
      }
      else {
        switch(year_mode) {
          case 1984:
          case 1989:
            // Do nothing.
          break;

          case 2021:
            soundIdleLoop(true);

            playEffect(S_AFTERLIFE_GUN_RAMP_1, false, i_volume - 10);
            ms_gun_loop_1.start(2000);
          break;
        }
      }
    }
  }
}

void bargraphYearModeUpdate() {
  switch(year_mode) {
    case 2021:
      i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_2021;
    break;

    case 1984:
    case 1989:
      i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_1984;
    break;
  }
}

void wandLightsOff() {
  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    if(b_28segment_bargraph == true) {
      bargraphClearAlt();
    }
    else {
      digitalWrite(led_bargraph_1, HIGH);
      digitalWrite(led_bargraph_2, HIGH);
      digitalWrite(led_bargraph_3, HIGH);
      digitalWrite(led_bargraph_4, HIGH);
      digitalWrite(led_bargraph_5, HIGH);
    }
  #else
      digitalWrite(led_bargraph_1, HIGH);
      digitalWrite(led_bargraph_2, HIGH);
      digitalWrite(led_bargraph_3, HIGH);
      digitalWrite(led_bargraph_4, HIGH);
      digitalWrite(led_bargraph_5, HIGH);
  #endif

  analogWrite(led_slo_blo, 0);

  // If using the gpstar Neutrona Wand micro controller.
  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    analogWrite(led_front_left, 0); // The front left LED is wired separately, lets turn it off.

    digitalWrite(led_hat_1, LOW); // Turn off hat light 1.
    digitalWrite(led_hat_2, LOW); // Turn off hat light 2.
    digitalWrite(led_barrel_tip, LOW); // Turn off the wand barrel tip LED.
  #endif

  digitalWrite(led_vent, HIGH);
  digitalWrite(led_white, HIGH);

  i_bargraph_status = 0;
  i_bargraph_status_alt = 0;
}

void vibrationOff() {
  i_vibration_level_prev = 0;
  analogWrite(vibration, 0);
}

void adjustVolumeEffectsGain() {
  // Reset the gain on all sound effect tracks.
  for(unsigned int i=0; i <= i_last_effects_track; i++) {
    w_trig.trackGain(i, i_volume);
  }
}

void increaseVolumeEffects() {
  if(i_volume_percentage + VOLUME_EFFECTS_MULTIPLIER > 100) {
    i_volume_percentage = 100;
  }
  else {
    i_volume_percentage = i_volume_percentage + VOLUME_EFFECTS_MULTIPLIER;
  }

  i_volume = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_percentage / 100);

  adjustVolumeEffectsGain();
}

void decreaseVolumeEffects() {
  if(i_volume_percentage - VOLUME_EFFECTS_MULTIPLIER < 0) {
    i_volume_percentage = 0;
  }
  else {
    i_volume_percentage = i_volume_percentage - VOLUME_EFFECTS_MULTIPLIER;
  }

  i_volume = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_percentage / 100);

  adjustVolumeEffectsGain();
}

void increaseVolume() {
  if(i_volume_master == i_volume_abs_min && MINIMUM_VOLUME > i_volume_master) {
    i_volume_master = MINIMUM_VOLUME;
  }

  if(i_volume_master_percentage + VOLUME_MULTIPLIER > 100) {
    i_volume_master_percentage = 100;
  }
  else {
    i_volume_master_percentage = i_volume_master_percentage + VOLUME_MULTIPLIER;
  }

  i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
  i_volume_revert = i_volume_master;

  w_trig.masterGain(i_volume_master);
}

void decreaseVolume() {
  if(i_volume_master == i_volume_abs_min) {
    // Can not go any lower.
  }
  else {
    if(i_volume_master_percentage - VOLUME_MULTIPLIER < 0) {
      i_volume_master_percentage = 0;
    }
    else {
      i_volume_master_percentage = i_volume_master_percentage - VOLUME_MULTIPLIER;
    }

    i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
    i_volume_revert = i_volume_master;

    w_trig.masterGain(i_volume_master);
  }
}

int8_t readRotary() {
  static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

  prev_next_code <<= 2;

  if(digitalRead(r_encoderB)) {
    prev_next_code |= 0x02;
  }

  if(digitalRead(r_encoderA)) {
    prev_next_code |= 0x01;
  }

  prev_next_code &= 0x0f;

   // If valid then store as 16 bit data.
   if(rot_enc_table[prev_next_code]) {
      store <<= 4;
      store |= prev_next_code;

      if((store&0xff) == 0x2b) {
        return -1;
      }

      if((store&0xff) == 0x17) {
        return 1;
      }
   }

   return 0;
}

// Top rotary dial on the wand.
void checkRotary() {
  static int8_t c,val;            

  if((val = readRotary())) {
    c += val;
    switch(WAND_ACTION_STATUS) {
      case ACTION_SETTINGS:
        // Counter clockwise.
        if(prev_next_code == 0x0b) {
          if(i_wand_menu == 4 && b_wand_menu_sub != true && switch_intensify.getState() == LOW && switchMode() != true) {
            // Tell pack to dim the selected lighting. (Power Cell, Cyclotron or Inner Cyclotron)
            wandSerialSend(W_DIMMING_DECREASE);

            stopEffect(S_BEEPS);
            playEffect(S_BEEPS);                    
          }
          else if(i_wand_menu == 3 && b_wand_menu_sub != true && switch_intensify.getState() == LOW && switchMode() != true) {
            // Lower the sound effects volume.
            decreaseVolumeEffects();

            // Tell pack to lower the sound effects volume.
            wandSerialSend(W_VOLUME_SOUND_EFFECTS_DECREASE);
          }
          else if(i_wand_menu == 3 && b_wand_menu_sub != true && switch_intensify.getState() == HIGH && analogRead(switch_mode) < i_switch_mode_value && b_playing_music == true) {
            // Decrease the music volume.
            if(i_volume_music_percentage - VOLUME_MUSIC_MULTIPLIER < 0) {
              i_volume_music_percentage = 0;
            }
            else {
              i_volume_music_percentage = i_volume_music_percentage - VOLUME_MUSIC_MULTIPLIER;
            }

            i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

            w_trig.trackGain(i_current_music_track, i_volume_music);

            // Tell pack to lower the music volume.
            wandSerialSend(W_VOLUME_MUSIC_DECREASE);
          }
          else if(i_wand_menu - 1 < 1) {
            // We are entering the sub menu. Only accessible when the Neutrona Wand is powered down.
            if(WAND_STATUS == MODE_OFF) {
              if(b_wand_menu_sub != true) {
                b_wand_menu_sub = true;
                i_wand_menu = 5;

                // Turn on the slo blow led to indicate we are in the Neutrona Wand sub menu.
                analogWrite(led_slo_blo, 255);

                // Play a indication beep to notify we have changed to the sub menu.
                stopEffect(S_BEEPS);
                playEffect(S_BEEPS);

                // Tell the Proton Pack to play a beep during a sub menu to menu level change.
                wandSerialSend(W_MENU_LEVEL_CHANGE);
              }
              else {
                i_wand_menu = 1;
              }
            }
            else {
              i_wand_menu = 1;
            }
          }
          else {
            i_wand_menu--;
          }
        }

        // Clockwise.
        if(prev_next_code == 0x07) {
          if(i_wand_menu == 4 && b_wand_menu_sub != true && switch_intensify.getState() == LOW && switchMode() != true) {
            // Tell pack to dim the selected lighting. (Power Cell, Cyclotron or Inner Cyclotron)
            wandSerialSend(W_DIMMING_INCREASE);

            stopEffect(S_BEEPS);
            playEffect(S_BEEPS);                
          }
          else if(i_wand_menu == 3 && b_wand_menu_sub != true && switch_intensify.getState() == LOW && switchMode() != true) {
            // Increase sound effects volume.
            increaseVolumeEffects();

            // Tell pack to increase the sound effects volume.
            wandSerialSend(W_VOLUME_SOUND_EFFECTS_INCREASE);
          }
          else if(i_wand_menu == 3 && b_wand_menu_sub != true && switch_intensify.getState() == HIGH && analogRead(switch_mode) < i_switch_mode_value && b_playing_music == true) {
            // Increase music volume.
            if(i_volume_music_percentage + VOLUME_MUSIC_MULTIPLIER > 100) {
              i_volume_music_percentage = 100;
            }
            else {
              i_volume_music_percentage = i_volume_music_percentage + VOLUME_MUSIC_MULTIPLIER;
            }

            i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

            w_trig.trackGain(i_current_music_track, i_volume_music);

            // Tell pack to increase music volume.
            wandSerialSend(W_VOLUME_MUSIC_INCREASE);
          }     
          else if(i_wand_menu + 1 > 5) {
            // We are leaving the sub menu. Only accessible when the Neutrona Wand is powered down.
            if(WAND_STATUS == MODE_OFF) {
              if(b_wand_menu_sub == true) {
                b_wand_menu_sub = false;
                i_wand_menu = 1;

                // Turn off the slo blow led to indicate we are no longer in the Neutrona Wand sub menu.
                analogWrite(led_slo_blo, 0);

                // Play a indication beep to notify we have left the sub menu.
                stopEffect(S_BEEPS);
                playEffect(S_BEEPS);

                // Tell the Proton Pack to play a beep during a submenu to menu level change.
                wandSerialSend(W_MENU_LEVEL_CHANGE);
              }
              else {
                i_wand_menu = 5;
              }
            }
            else {
              i_wand_menu = 5;
            }
          }
          else {
            i_wand_menu++;
          }
        }
      break;

      default:
        if(WAND_ACTION_STATUS != ACTION_OVERHEATING && b_pack_alarm != true) {
          if(WAND_ACTION_STATUS == ACTION_FIRING && i_power_mode == i_power_mode_max) {
            // Do nothing, we are locked in full power mode while firing.
          }
          // Counter clockwise.
          else if(prev_next_code == 0x0b) {
            if(i_power_mode - 1 >= i_power_mode_min && WAND_STATUS == MODE_ON) {
              i_power_mode_prev = i_power_mode;
              i_power_mode--;

              #ifdef GPSTAR_NEUTRONA_WAND_PCB
                if(year_mode == 2021 && b_28segment_bargraph == true) {
                  bargraphPowerCheck2021Alt(false);
                }
              #endif

              soundBeepLoopStop();

              switch(year_mode) {
                case 1984:
                case 1989:
                  if(switch_vent.getState() == LOW) {
                    soundIdleLoopStop();
                    soundIdleLoop(false);
                  }
                break;

                default:
                    soundIdleLoopStop();
                    soundIdleLoop(false);
                break;
              }

              updatePackPowerLevel();
            }

            // Decrease the music volume if the wand/pack is off. A quick easy way to adjust the music volume on the go.
            if(WAND_STATUS == MODE_OFF && FIRING_MODE != SETTINGS && b_playing_music == true) {
              if(i_volume_music_percentage - VOLUME_MUSIC_MULTIPLIER < 0) {
                i_volume_music_percentage = 0;
              }
              else {
                i_volume_music_percentage = i_volume_music_percentage - VOLUME_MUSIC_MULTIPLIER;
              }

              i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

              w_trig.trackGain(i_current_music_track, i_volume_music);

              // Tell pack to lower music volume.
              wandSerialSend(W_VOLUME_MUSIC_DECREASE);
            }
          }

          if(WAND_ACTION_STATUS == ACTION_FIRING && i_power_mode == i_power_mode_max) {
            // Do nothing, we are locked in full power mode while firing.
          }
          // Clockwise.
          else if(prev_next_code == 0x07) {
            if(i_power_mode + 1 <= i_power_mode_max && WAND_STATUS == MODE_ON) {
              if(i_power_mode + 1 == i_power_mode_max && WAND_ACTION_STATUS == ACTION_FIRING) {
                // Do nothing, we not want to go into max power mode if firing in a lower power mode already.
              }
              else {
                i_power_mode_prev = i_power_mode;
                i_power_mode++;

                #ifdef GPSTAR_NEUTRONA_WAND_PCB
                  if(year_mode == 2021 && b_28segment_bargraph == true) {
                    bargraphPowerCheck2021Alt(false);
                  }
                #endif

                soundBeepLoopStop();

                switch(year_mode) {
                  case 1984:
                  case 1989:
                    if(switch_vent.getState() == LOW) {
                      soundIdleLoopStop();
                      soundIdleLoop(false);
                    }
                  break;

                  default:
                      soundIdleLoopStop();
                      soundIdleLoop(false);
                  break;
                }

                updatePackPowerLevel();
              }
            }

            // Increase the music volume if the wand/pack is off. A quick easy way to adjust the music volume on the go.
            if(WAND_STATUS == MODE_OFF && FIRING_MODE != SETTINGS && b_playing_music == true) {
              if(i_volume_music_percentage + VOLUME_MUSIC_MULTIPLIER > 100) {
                i_volume_music_percentage = 100;
              }
              else {
                i_volume_music_percentage = i_volume_music_percentage + VOLUME_MUSIC_MULTIPLIER;
              }

              i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

              w_trig.trackGain(i_current_music_track, i_volume_music);

              // Tell pack to increase music volume.
              wandSerialSend(W_VOLUME_MUSIC_INCREASE);
            }
          }
        }
      break;
    }
  }
}

// Tell the pack which power level the wand is at.
void updatePackPowerLevel() {
  switch(i_power_mode) {
    case 5:
      // Level 5
      wandSerialSend(W_POWER_LEVEL_5);
    break;

    case 4:
      // Level 4
      wandSerialSend(W_POWER_LEVEL_4);
    break;

    case 3:
      // Level 3
      wandSerialSend(W_POWER_LEVEL_3);
    break;

    case 2:
      // Level 2
      wandSerialSend(W_POWER_LEVEL_2);
    break;

    default:
      // Level 1
      wandSerialSend(W_POWER_LEVEL_1);
    break;
  }
}

void vibrationSetting() {
  if(b_vibration_on == true) {
    if(ms_bargraph.isRunning() == false && WAND_ACTION_STATUS != ACTION_FIRING) {
      switch(i_power_mode) {
        case 1:
          vibrationWand(i_vibration_level);
        break;

        case 2:
          vibrationWand(i_vibration_level + 5);
        break;

        case 3:
          vibrationWand(i_vibration_level + 10);
        break;

        case 4:
          vibrationWand(i_vibration_level + 12);
        break;

        case 5:
          vibrationWand(i_vibration_level + 25);
        break;
      }
    }
  }
  else {
    vibrationOff();
  }
}

void switchLoops() {
  switch_wand.loop();
  switch_intensify.loop();
  switch_activate.loop();
  switch_vent.loop();
}

void wandBarrelLightsOff() {
  for(uint8_t i = 0; i < BARREL_NUM_LEDS; i++) {
    barrel_leds[i] = CRGB(0,0,0);
  }

  ms_fast_led.start(i_fast_led_delay);
}

// Exit the wand menu system while the wand is off.
void wandExitMenu() {
  FIRING_MODE = PREV_FIRING_MODE;

  i_wand_menu = 5;

  if(b_pack_alarm != true) {
    playEffect(S_CLICK);
  }

  bargraphClearAlt();

  switch(PREV_FIRING_MODE) {
    case MESON:
      // Tell the pack we are in meson mode.
      wandSerialSend(W_MESON_MODE);
    break;

    case STASIS:
      // Tell the pack we are in stasis mode.
      wandSerialSend(W_STASIS_MODE);
    break;

    case SLIME:
      // Tell the pack we are in slime mode.
      wandSerialSend(W_SLIME_MODE);
    break;

    case PROTON:
      // Tell the pack we are in proton mode.
      wandSerialSend(W_PROTON_MODE);
    break;

    case VENTING:
      // Tell the pakc we are in venting mode.
      wandSerialSend(W_VENTING_MODE);
    break;

    default:
      // Tell the pack we are in proton mode.
      wandSerialSend(W_PROTON_MODE);
    break;
  }

  WAND_ACTION_STATUS = ACTION_IDLE;

  wandLightsOff();
}

// Mode switch is connected to analog input.
// PCB builds is pulled high.
// Nano builds is pulled low.
bool switchMode() {
  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    if(analogRead(switch_mode) < i_switch_mode_value && ms_switch_mode_debounce.remaining() < 1) {
      if(WAND_ACTION_STATUS == ACTION_SETTINGS) {
        ms_switch_mode_debounce.start(a_switch_debounce_time * 2);
      }
      else {
        ms_switch_mode_debounce.start(a_switch_debounce_time);
      }

      return true;
    }
    else {
      return false;
    }
  #else
    if(analogRead(switch_mode) > i_switch_mode_value && ms_switch_mode_debounce.remaining() < 1) {
      if(WAND_ACTION_STATUS == ACTION_SETTINGS) {
        ms_switch_mode_debounce.start(a_switch_debounce_time * 2);
      }
      else {
        ms_switch_mode_debounce.start(a_switch_debounce_time);
      }

      return true;
    }
    else {
      return false;
    }
  #endif
}

// Barrel safety switch is connected to analog input.
// PCB builds is pulled high.
// Nano builds is pulled low.
bool switchBarrel() {
  #ifdef GPSTAR_NEUTRONA_WAND_PCB
    if(analogRead(switch_barrel) < i_switch_barrel_value) {
      return true;
    }
    else {
      return false;
    }
  #else
    if(analogRead(switch_barrel) > i_switch_barrel_value) {
      return true;
    }
    else {
      return false;
    }
  #endif
}

// Pack communication to the wand.
void checkPack() {
    if(wandComs.available()) {
      wandComs.rxObj(comStruct);

      if(!wandComs.currentPacketID()) {
        if(comStruct.i > 0 && comStruct.s == P_COM_START && comStruct.e == P_COM_END) {

        if(b_volume_sync_wait == true) {
          switch(VOLUME_SYNC_WAIT) {
            case EFFECTS:
              i_volume_percentage = comStruct.i;
              i_volume = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_percentage / 100);

              adjustVolumeEffectsGain();
              VOLUME_SYNC_WAIT = MASTER;
            break;

            case MASTER:
              i_volume_master_percentage = comStruct.i;
              i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);

              i_volume_revert = i_volume_master;

              w_trig.masterGain(i_volume_master);

              VOLUME_SYNC_WAIT = MUSIC;
            break;

            case MUSIC:
              i_volume_music_percentage = comStruct.i;
              i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

              VOLUME_SYNC_WAIT = SILENT;
            break;

            case SILENT:
              if(comStruct.i == P_MASTER_AUDIO_SILENT_MODE) {
                i_volume_revert = i_volume_master;

                // The pack is telling us to be silent.
                i_volume_master = i_volume_abs_min;
                w_trig.masterGain(i_volume_master);
              }

              // Exit the sync mode.
              b_volume_sync_wait = false;
              b_wait_for_pack = false;

              VOLUME_SYNC_WAIT = EFFECTS;
            break;
          }
        }
        else {
          switch(comStruct.i) {
            case P_ON:
              // Pack is on.
              b_pack_on = true;
            break;

            case P_OFF:
              if(b_pack_on == true) {
                // Turn wand off.
                if(WAND_STATUS != MODE_OFF) {
                  if(WAND_STATUS == MODE_ERROR) {
                    wandOff();
                  }
                  else {
                    WAND_ACTION_STATUS = ACTION_OFF;
                  }
                }
              }

              // Pack is off.
              b_pack_on = false;
            break;

            case P_ALARM_ON:
              // Alarm is on.
              b_pack_alarm = true;

              if(WAND_STATUS != MODE_ERROR) {
                #ifdef GPSTAR_NEUTRONA_WAND_PCB
                  if(WAND_STATUS == MODE_ON) {
                    digitalWrite(led_hat_2, HIGH); // Turn on hat light 2.
                  }
                #endif

                ms_hat_2.start(i_hat_2_delay); // Start the hat light 2 blinking timer.

                if(WAND_STATUS == MODE_ON && FIRING_MODE == SETTINGS) {
                  // If the wand is in settings mode while the alarm is activated, exit the settings mode.
                  wandSerialSend(W_PROTON_MODE);
                  FIRING_MODE = PROTON;
                  WAND_ACTION_STATUS = ACTION_IDLE;
                }

                if(WAND_STATUS == MODE_ON) {
                  prepBargraphRampDown();
                }
              }
            break;

            case P_ALARM_OFF:
              // Alarm is off.
              b_pack_alarm = false;

              if(WAND_STATUS != MODE_ERROR) {
                #ifdef GPSTAR_NEUTRONA_WAND_PCB
                  digitalWrite(led_hat_2, LOW); // Turn off hat light 2.
                #endif

                ms_hat_2.stop();

                if(WAND_STATUS == MODE_ON) {
                  prepBargraphRampUp();
                }
              }
            break;

            case P_YEAR_1984:
              // 1984 mode.
              year_mode = 1984;
              bargraphYearModeUpdate();
            break;

            case P_YEAR_1989:
              // 1984 mode.
              year_mode = 1989;
              bargraphYearModeUpdate();
            break;

            case P_YEAR_AFTERLIFE:
              // 2021 mode.
              year_mode = 2021;
              bargraphYearModeUpdate();
            break;

            case P_VOLUME_INCREASE:
              // Increase overall volume.
              increaseVolume();
            break;

            case P_VOLUME_DECREASE:
              // Decrease overall volume.
              decreaseVolume();
            break;

            case P_HANDSHAKE:
              // The pack is asking us if we are still here. Respond back.
              wandSerialSend(W_HANDSHAKE);
            break;

            case P_MUSIC_REPEAT:
              // Repeat music track.
              b_repeat_track = true;
            break;

            case P_MUSIC_NO_REPEAT:
              // Do not repeat the music track.
              b_repeat_track = false;
            break;

            case P_VOLUME_SYNC_MODE:
              // Put the wand into volume sync mode.
              b_volume_sync_wait = true;
              VOLUME_SYNC_WAIT = EFFECTS;
            break;

            case P_VIBRATION_ENABLED:
              // Vibration enabled (from Proton Pack vibration toggle switch).
              b_vibration_enabled = true;

              // Only play the voice if we are not doing a Proton Pack / Neutrona Wand synchronisation.
              if(b_wait_for_pack != true) {
                stopEffect(S_BEEPS_ALT);

                playEffect(S_BEEPS_ALT);

                stopEffect(S_VOICE_VIBRATION_ENABLED);
                stopEffect(S_VOICE_VIBRATION_DISABLED);

                playEffect(S_VOICE_VIBRATION_ENABLED);
              }
            break;

            case P_VIBRATION_DISABLED:
              // Vibration disabled (from Proton Pack vibration toggle switch).
              b_vibration_enabled = false;

              // Only play the voice if we are not doing a Proton Pack / Neutrona Wand synchronisation.
              if(b_wait_for_pack != true) {
                stopEffect(S_BEEPS_ALT);

                playEffect(S_BEEPS_ALT);

                stopEffect(S_VOICE_VIBRATION_DISABLED);
                stopEffect(S_VOICE_VIBRATION_ENABLED);

                playEffect(S_VOICE_VIBRATION_DISABLED);
              }

              vibrationOff();
            break;

            case P_PACK_VIBRATION_ENABLED:
              // Proton Pack Vibration enabled.
              stopEffect(S_BEEPS_ALT);

              playEffect(S_BEEPS_ALT);

              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);

              playEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
            break;

            case P_PACK_VIBRATION_DISABLED:
              // Proton Pack Vibration disabled.
              stopEffect(S_BEEPS_ALT);

              playEffect(S_BEEPS_ALT);

              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);

              playEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);
            break;

            case P_PACK_VIBRATION_FIRING_ENABLED:
              // Proton Pack Vibration firing enabled.
              stopEffect(S_BEEPS_ALT);

              playEffect(S_BEEPS_ALT);

              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_ENABLED);
              stopEffect(S_VOICE_PROTON_PACK_VIBRATION_DISABLED);

              playEffect(S_VOICE_PROTON_PACK_VIBRATION_FIRING_ENABLED);
            break;

            case P_MODE_AFTERLIFE:
              // Play 2021 voice.
              stopEffect(S_VOICE_AFTERLIFE);
              stopEffect(S_VOICE_1989);
              stopEffect(S_VOICE_1984);

              playEffect(S_VOICE_AFTERLIFE);
            break;

            case P_MODE_1989:
              // Play 1989 voice.
              stopEffect(S_VOICE_AFTERLIFE);
              stopEffect(S_VOICE_1989);
              stopEffect(S_VOICE_1984);

              playEffect(S_VOICE_1989);
            break;

            case P_MODE_1984:
              // Play 1984 voice.
              stopEffect(S_VOICE_AFTERLIFE);
              stopEffect(S_VOICE_1989);
              stopEffect(S_VOICE_1984);

              playEffect(S_VOICE_1984);
            break;

            case P_SMOKE_DISABLED:
              // Play smoke disabled voice.
              stopEffect(S_VOICE_SMOKE_DISABLED);
              stopEffect(S_VOICE_SMOKE_ENABLED);

              playEffect(S_VOICE_SMOKE_DISABLED);
            break;

            case P_SMOKE_ENABLED:
              // Play smoke enabled voice.
              stopEffect(S_VOICE_SMOKE_ENABLED);
              stopEffect(S_VOICE_SMOKE_DISABLED);

              playEffect(S_VOICE_SMOKE_ENABLED);
            break;

            case P_CYCLOTRON_COUNTER_CLOCKWISE:
              // Play cyclotron counter clockwise voice.
              stopEffect(S_VOICE_CYCLOTRON_CLOCKWISE);
              stopEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);

              playEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);
            break;

            case P_CYCLOTRON_CLOCKWISE:
              // Play cyclotron clockwise voice.
              stopEffect(S_VOICE_CYCLOTRON_CLOCKWISE);
              stopEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);

              playEffect(S_VOICE_CYCLOTRON_CLOCKWISE);
            break;

            case P_CYCLOTRON_SINGLE_LED:
              // Play Single LED voice.
              stopEffect(S_VOICE_THREE_LED);
              stopEffect(S_VOICE_SINGLE_LED);

              playEffect(S_VOICE_SINGLE_LED);
            break;

            case P_CYCLOTRON_THREE_LED:
              // Play 3 LED voice.
              stopEffect(S_VOICE_THREE_LED);
              stopEffect(S_VOICE_SINGLE_LED);

              playEffect(S_VOICE_THREE_LED);
            break;

            case P_VIDEO_GAME_MODE_COLOURS_DISABLED:
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_DISABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED);

              playEffect(S_VOICE_VIDEO_GAME_COLOURS_DISABLED);
            break;

            case P_VIDEO_GAME_MODE_POWER_CELL_ENABLED:
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_DISABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED);

              playEffect(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED);
            break;

            case P_VIDEO_GAME_MODE_CYCLOTRON_ENABLED:
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_DISABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED);

              playEffect(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED);
            break;

            case P_VIDEO_GAME_MODE_COLOURS_ENABLED:
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_DISABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_POWERCELL_ENABLED);
              stopEffect(S_VOICE_VIDEO_GAME_COLOURS_CYCLOTRON_ENABLED);

              playEffect(S_VOICE_VIDEO_GAME_COLOURS_ENABLED);
            break;

            case P_MUSIC_STOP:
              // Stop music
              stopMusic();
            break;

            case P_POWERCELL_DIMMING:
              stopEffect(S_VOICE_POWERCELL_BRIGHTNESS);
              stopEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
              stopEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);

              playEffect(S_VOICE_POWERCELL_BRIGHTNESS);
            break;

            case P_CYCLOTRON_DIMMING:
              stopEffect(S_VOICE_POWERCELL_BRIGHTNESS);
              stopEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
              stopEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);

              playEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
            break;
            
            case P_INNER_CYCLOTRON_DIMMING:
              stopEffect(S_VOICE_POWERCELL_BRIGHTNESS);
              stopEffect(S_VOICE_CYCLOTRON_BRIGHTNESS);
              stopEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);

              playEffect(S_VOICE_CYCLOTRON_INNER_BRIGHTNESS);
            break;

            case P_PROTON_STREAM_IMPACT_ENABLED:
              stopEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
              stopEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);

              playEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
            break;

            case P_PROTON_STREAM_IMPACT_DISABLED:
              stopEffect(S_VOICE_PROTON_MIX_EFFECTS_ENABLED);
              stopEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);

              playEffect(S_VOICE_PROTON_MIX_EFFECTS_DISABLED);
            break;

            default:
              // Music track number to be played.
              if(comStruct.i >= i_music_track_start) {
                if(b_playing_music == true) {
                  stopMusic();
                  i_current_music_track = comStruct.i;
                  playMusic();
                }
                else {
                  i_current_music_track = comStruct.i;
                }
              }
            break;
          }
        }

        comStruct.i = 0;
        comStruct.s = 0;
      }
    }
  }
}

void wandSerialSend(int i_message) {
  sendStruct.i = i_message;
  sendStruct.s = W_COM_START;
  sendStruct.e = W_COM_END;

  wandComs.sendDatum(sendStruct);
}

// Helper method to play a sound effect using certain defaults.
void playEffect(int i_track_id, bool b_track_loop = false, int8_t i_track_volume = i_volume, bool b_fade_in = false, unsigned int i_fade_time = 0) {
  if(i_track_volume < i_volume_abs_min) {
    i_track_volume = i_volume_abs_min;
  }
  if(i_track_volume > 10) {
    i_track_volume = i_volume_abs_max;
  }

  if(b_fade_in == true) {
    w_trig.trackGain(i_track_id, i_volume_abs_min);
    w_trig.trackPlayPoly(i_track_id, true);
    w_trig.trackFade(i_track_id, i_track_volume, i_fade_time, 0);
  }
  else {
    w_trig.trackGain(i_track_id, i_track_volume);
    w_trig.trackPlayPoly(i_track_id, true);
  }

  if(b_track_loop == true) {
    w_trig.trackLoop(i_track_id, 1);
  }
}

void stopEffect(int i_track_id) {
  w_trig.trackStop(i_track_id);
}

// Helper method to play a music track using certain defaults.
void playMusic(int i_music_id = i_current_music_track, bool b_music_loop = b_repeat_track, int8_t i_music_volume = i_volume_music) {
  if(i_music_volume < i_volume_abs_min) {
    i_music_volume = i_volume_abs_min;
  }
  if(i_music_volume > 10) {
    i_music_volume = i_volume_abs_max;
  }

  w_trig.trackGain(i_music_id, i_music_volume);
  w_trig.trackPlayPoly(i_music_id, true);

  if(b_music_loop == true) {
    w_trig.trackLoop(i_music_id, 1);
  }

  w_trig.update();
}

void stopMusic(int i_music_id = i_current_music_track) {
  w_trig.trackStop(i_music_id);

  w_trig.update();
}

void setupWavTrigger() {
  // If the controller is powering the WAV Trigger, we should wait for the WAV trigger to finish reset before trying to send commands.
  delay(1000);

  // WAV Trigger's startup at 57600
  w_trig.start();

  delay(10);

  // Send a stop-all command and reset the sample-rate offset, in case we have
  //  reset while the WAV Trigger was already playing.
  w_trig.stopAllTracks();
  w_trig.samplerateOffset(0); // Reset our sample rate offset
  w_trig.masterGain(i_volume_master); // Reset the master gain db. 0db is default. Range is -70 to 0.
  w_trig.setAmpPwr(b_onboard_amp_enabled); // Turn on the onboard amp.

  // Enable track reporting from the WAV Trigger
  w_trig.setReporting(true);

  // Allow time for the WAV Trigger to respond with the version string and number of tracks.
  delay(350);

  unsigned int w_num_tracks = w_trig.getNumTracks();

  // Build the music track count.
  i_music_count = w_num_tracks - i_last_effects_track;

  if(i_music_count > 0) {
    i_current_music_track = i_music_track_start; // Set the first track of music as file 500_
  }
}
