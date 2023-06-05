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
#include <ht16k33.h>
#include "Configuration.h"
#include "MusicSounds.h"
//#include "SerialDefs.h"
#include "Header.h"

/*
  ***** IMPORTANT *****
  * If you are compiling the code to upload to a Arduino Nano. You want to use NeutronaWandNano.h and comment out NeutronaWandPCB.h
  * If you are compiling the code to upload to the gpstar Neutrona Wand micro controller, you want to use NeutronaWandPCB.h and comment out NeutronaWandNano.h
*/
//#include "NeutronaWandNano.h"
#include "NeutronaWandPCB.h"

void setup() {  
  Serial.begin(9600);
  Serial1.begin(9600);
  
  // Change PWM frequency of pin 3 and 11 for the vibration motor, we do not want it high pitched.
  TCCR2B = (TCCR2B & B11111000) | (B00000110); // for PWM frequency of 122.55 Hz
  
  setupWavTrigger();
  
  // Barrel LEDs
  FastLED.addLeds<NEOPIXEL, BARREL_LED_PIN>(barrel_leds, BARREL_NUM_LEDS);
  
  switch_wand.setDebounceTime(switch_debounce_time);
  switch_intensify.setDebounceTime(switch_debounce_time);
  switch_activate.setDebounceTime(switch_debounce_time);
  switch_vent.setDebounceTime(switch_debounce_time);
  
  if(b_pcb == true) {
    pinMode(switch_mode, INPUT_PULLUP);
    pinMode(switch_barrel, INPUT_PULLUP);
  }

  // Rotary encoder on the top of the wand.
  pinMode(r_encoderA, INPUT_PULLUP);
  pinMode(r_encoderB, INPUT_PULLUP);
  
  bargraphYearModeUpdate();

  // Setup the bargraph.
  if(b_bargraph_alt == true) {
    // 28 Segment optional bargraph.
    ht_bargraph.begin(0x00);
  }
  else {
    // Original Hasbro bargraph.
    pinMode(led_bargraph_1, OUTPUT);
    pinMode(led_bargraph_2, OUTPUT);
    pinMode(led_bargraph_3, OUTPUT);
    pinMode(led_bargraph_4, OUTPUT);
    pinMode(led_bargraph_5, OUTPUT);
  }
  
  pinMode(led_slo_blo, OUTPUT);

  // Extra optional items if using them with the gpstar Neutrona Wand micro controller.
  if(b_pcb == true) {
    pinMode(led_front_left, OUTPUT); // Front left LED. When using the gpstar Neutrona Wand micro controller, it is wired to it's own pin. When using a Arduino Nano, it is linked with led_slo_blo.
    pinMode(led_hat_1, OUTPUT); // Hat light at front of the wand near the barrel tip.
    pinMode(led_hat_2, OUTPUT); // Hat light at top of the wand body.
    pinMode(led_barrel_tip, OUTPUT); // LED at the tip of the wand barrel.
  }

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
      Serial1.write(14);
    }

    // Synchronise some settings with the pack.
    checkPack();
    
    delay(200);
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
      // Do nothing.
    break;

    case ACTION_OFF:
      wandOff();
    break;
    
    case ACTION_FIRING:
      if(ms_firing_start_sound_delay.justFinished()) {
        modeFireStartSounds();
      }
    
      if(b_pack_on == true && b_pack_alarm == false) {
        if(b_firing == false) {
          b_firing = true;
          modeFireStart();
        }

        if(b_pcb == true) {
          if(ms_hat_1.isRunning()) {
            if(ms_hat_1.remaining() < i_hat_1_delay / 2) {
              digitalWrite(led_hat_1, LOW);
              digitalWrite(led_hat_2, HIGH);
            }
            else {
              digitalWrite(led_hat_1, HIGH);
              digitalWrite(led_hat_2, LOW);
            }

            if(ms_hat_1.justFinished()) {
              ms_hat_1.start(i_hat_1_delay);
            }
          }
        }

        // Overheating.
        if(ms_overheat_initiate.justFinished() && b_overheat_mode[i_power_mode - 1] == true && b_overheat_enabled == true) {
          ms_overheat_initiate.stop();

          modeFireStop();

          // Turn on hat light 2.
          if(b_pcb == true) {
            digitalWrite(led_hat_2, HIGH);
          }

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

            b_sound_idle == false;
            b_beeping = false;

            // Reset some bargraph levels before we ramp the bargraph down.
            i_bargraph_status_alt = 28; // For 28 segment bargraph
            i_bargraph_status = 5; // For Hasbro 5 LED bargraph.

            if(b_bargraph_alt == true) {
              for(int i = 0; i < 28; i++) {
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

            ms_bargraph.start(d_bargraph_ramp_interval);
          }

          w_trig.trackPlayPoly(S_VENT_DRY);
          w_trig.trackPlayPoly(S_CLICK);

          // Tell the pack we are overheating.
          Serial1.write(10);
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
        if(b_pcb == true) {
          digitalWrite(led_hat_2, LOW);
        }

        WAND_ACTION_STATUS = ACTION_IDLE;

        w_trig.trackStop(S_CLICK);
        w_trig.trackStop(S_VENT_DRY);
        
        // Prepare a few things before ramping the bargraph back up from a full ramp down.
        if(b_overheat_bargraph_blink != true) {
          w_trig.trackPlayPoly(S_BOOTUP, true);
          
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
                // Do nothing.
              break;

              case 2021:
                soundIdleLoop(true);

                w_trig.trackGain(S_AFTERLIFE_GUN_RAMP_1, i_volume);
                w_trig.trackPlayPoly(S_AFTERLIFE_GUN_RAMP_1, true); // Start track
                ms_gun_loop_1.start(2000);
              break;
            }
          }
        }

        bargraphRampUp();
        
        // Tell the pack that we finished overheating.
        Serial1.write(11);
      }
    break;
    
    case ACTION_ACTIVATE:
      modeActivate();
    break;

    case ACTION_SETTINGS:
      settingsBlinkingLights();
      
      switch(i_wand_menu) {
        /*
         * Top menu: Music track loop setting.
         * Sub menu: Enable or disable crossing the streams / video game modes.
        */
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
            Serial1.write(93);
          }
        }
        else {
          if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
            ms_intensify_timer.start(i_intensify_delay / 2);

            // Enable or disable crossing the streams / video game modes.
            if(b_cross_the_streams == true) {
              // Turn off crossing the streams mode and switch back to video game mode.
              b_cross_the_streams = false;
              w_trig.trackStop(S_CLICK);    
              w_trig.trackGain(S_CLICK, i_volume);
              w_trig.trackPlayPoly(S_CLICK);

              w_trig.trackStop(S_VOICE_CROSS_THE_STREAMS);    
              w_trig.trackStop(S_VOICE_VIDEO_GAME_MODES);    
              w_trig.trackGain(S_VOICE_VIDEO_GAME_MODES, i_volume);
              w_trig.trackPlayPoly(S_VOICE_VIDEO_GAME_MODES);

              // Tell the proton pack to reset back to the proton stream.
              Serial1.write(34);
            }
            else {
              // Turn on crossing the streams mode and turn off video game mode.
              b_cross_the_streams = true;

              w_trig.trackStop(S_CLICK);    
              w_trig.trackGain(S_CLICK, i_volume);
              w_trig.trackPlayPoly(S_CLICK);

              w_trig.trackStop(S_VOICE_VIDEO_GAME_MODES);    
              w_trig.trackStop(S_VOICE_CROSS_THE_STREAMS);    
              w_trig.trackGain(S_VOICE_CROSS_THE_STREAMS, i_volume);
              w_trig.trackPlayPoly(S_VOICE_CROSS_THE_STREAMS);

              // Tell the proton pack to reset back to the proton stream.
              Serial1.write(28);
            }

            // Reset the previous firing mode to the proton stream.
            PREV_FIRING_MODE = PROTON;
          }
        }
        break;

        /*
         * Top menu: Adjust the Proton Pack / Neutrona wand sound effects volume.
         * Sub menu: Enable or disable smoke for the Proton Pack.
         * Sub menu: (Mode switch) -> Enable or disable overheating.
        */
        case 4:
          // Adjust the Proton Pack / Neutrona wand sound effects volume.
          if(b_wand_menu_sub != true) {
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);
              
              increaseVolumeEffects();
              
              // Tell pack to increase the sound effects volume.
              Serial1.write(92);
            }

            if(switchMode() == true && ms_switch_mode_debounce.justFinished()) {
              decreaseVolumeEffects();
              
              // Tell pack to lower the sound effects volume.
              Serial1.write(91);
              
              ms_switch_mode_debounce.start(a_switch_debounce_time * 2);
            }
          }
          else {
            // Enable or disable smoke for the Proton Pack.
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              // Tell the Proton Pack to toggle the smoke on or off.
              Serial1.write(33);
            }

            // Enable or disable overheating.
            if(switchMode() == true && ms_switch_mode_debounce.justFinished()) {              
              if(b_overheat_enabled == true) {
                b_overheat_enabled = false;
                
                // Play the overheating disabled voice.
                w_trig.trackStop(S_VOICE_OVERHEAT_DISABLED);    
                w_trig.trackStop(S_VOICE_OVERHEAT_ENABLED);    
                w_trig.trackGain(S_VOICE_OVERHEAT_DISABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_OVERHEAT_DISABLED);

                // Tell the Proton Pack that overheating is disabled.
                Serial1.write(37);
              }
              else {
                b_overheat_enabled = true;

                // Play the overheating enabled voice.
                w_trig.trackStop(S_VOICE_OVERHEAT_DISABLED);    
                w_trig.trackStop(S_VOICE_OVERHEAT_ENABLED);    
                w_trig.trackGain(S_VOICE_OVERHEAT_ENABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_OVERHEAT_ENABLED);

                // Tell the Proton Pack that overheating is enabled.
                Serial1.write(38);
              }
              
              ms_switch_mode_debounce.start(a_switch_debounce_time * 2);
            }
          }
        break;

        /*
         * Top menu: Adjust Proton Pack / Neutrona wand music volume.
         * Sub menu: Toggle cyclotron rotation direction.
         * Sub menu: (Mode switch) -> Toggle the Proton Pack Single LED or 3 LEDs for 1984/1989 modes.
        */
        case 3:
          // Adjust Proton Pack / Neutrona wand music volume..
          if(b_wand_menu_sub != true) {
            if(b_playing_music == true) {
              if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
                ms_intensify_timer.start(i_intensify_delay);
                
                if(i_volume_music_percentage + VOLUME_MUSIC_MULTIPLIER > 100) {
                  i_volume_music_percentage = 100;
                }
                else {
                  i_volume_music_percentage = i_volume_music_percentage + VOLUME_MUSIC_MULTIPLIER;
                }

                i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

                w_trig.trackGain(i_current_music_track, i_volume_music);
                
                // Tell pack to increase music volume.
                Serial1.write(90);
              }
    
              if(switchMode() == true && ms_switch_mode_debounce.justFinished()) {                
                if(i_volume_music_percentage - VOLUME_MUSIC_MULTIPLIER < 0) {
                  i_volume_music_percentage = 0;
                }
                else {
                  i_volume_music_percentage = i_volume_music_percentage - VOLUME_MUSIC_MULTIPLIER;
                }

                i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

                w_trig.trackGain(i_current_music_track, i_volume_music);
                
                // Tell pack to lower music volume.
                Serial1.write(89);
                
                ms_switch_mode_debounce.start(a_switch_debounce_time * 2);
              }  
            }
          }
          else {
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              // Tell the Proton Pack to change the cyclotron rotation direction.
              Serial1.write(35);
            }

            if(switchMode() == true && ms_switch_mode_debounce.justFinished()) {              
              ms_switch_mode_debounce.start(a_switch_debounce_time * 2);

              // Tell the Proton Pack to toggle the Single LED or 3 LEDs for 1984/1989 modes.
              Serial1.write(36);
            }
          }
        break;

        /*
         * Top menu: Change music tracks.
         * Sub menu: Enable or disable vibration (Proton Pack and Neutrona wand)
         * Sub menu: (Mode switch) -> Enable or disable vibration for firing events only.
        */
        case 2:       
          // Change music tracks.
          if(b_wand_menu_sub != true) {             
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);
              
              if(i_current_music_track + 1 > i_music_track_start + i_music_count - 1) {
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
              Serial1.write(i_current_music_track);
            }

            if(switchMode() == true && ms_switch_mode_debounce.justFinished()) {                     
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
              Serial1.write(i_current_music_track);
              
              ms_switch_mode_debounce.start(a_switch_debounce_time * 2);
            }   
          }
          else {
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              w_trig.trackStop(S_BEEPS_ALT);    
              w_trig.trackGain(S_BEEPS_ALT, i_volume);
              w_trig.trackPlayPoly(S_BEEPS_ALT);

              // Enable or disable vibration
              if(b_vibration_on == true) {
                b_vibration_on = false;

                w_trig.trackStop(S_VOICE_VIBRATION_DISABLED);    
                w_trig.trackStop(S_VOICE_VIBRATION_ENABLED);    
                w_trig.trackGain(S_VOICE_VIBRATION_DISABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_VIBRATION_DISABLED);

                // Tell the proton pack to disable vibration.
                Serial1.write(29);
              }
              else {
                b_vibration_on = true;

                w_trig.trackStop(S_VOICE_VIBRATION_ENABLED);    
                w_trig.trackStop(S_VOICE_VIBRATION_DISABLED);    
                w_trig.trackGain(S_VOICE_VIBRATION_ENABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_VIBRATION_ENABLED);

                // Tell the Proton pack to enable vibration.
                Serial1.write(30);

                analogWrite(vibration, 150);
                delay(250);
                analogWrite(vibration,0);
              }
            }

            // Enable or disable vibration for firing events only.
            if(switchMode() == true && ms_switch_mode_debounce.justFinished()) {              
              ms_switch_mode_debounce.start(a_switch_debounce_time * 2);

              w_trig.trackStop(S_BEEPS_ALT);    
              w_trig.trackGain(S_BEEPS_ALT, i_volume);
              w_trig.trackPlayPoly(S_BEEPS_ALT);

              // Enable or disable vibration for firing only events.
              if(b_vibration_firing == true) {
                b_vibration_firing = false;

                w_trig.trackStop(S_VOICE_VIBRATION_FIRING_DISABLED);    
                w_trig.trackStop(S_VOICE_VIBRATION_FIRING_ENABLED);    
                w_trig.trackGain(S_VOICE_VIBRATION_FIRING_DISABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_VIBRATION_FIRING_DISABLED);

                // Tell the proton pack to disable vibration during firing only option.
                Serial1.write(31);
              }
              else {
                b_vibration_firing = true;

                w_trig.trackStop(S_VOICE_VIBRATION_FIRING_ENABLED);    
                w_trig.trackStop(S_VOICE_VIBRATION_FIRING_DISABLED);    
                w_trig.trackGain(S_VOICE_VIBRATION_FIRING_ENABLED, i_volume);
                w_trig.trackPlayPoly(S_VOICE_VIBRATION_FIRING_ENABLED);

                // Tell the Proton pack to enable vibration during firing only.
                Serial1.write(32);

                analogWrite(vibration, 150);
                delay(250);
                analogWrite(vibration,0);
              }
            }            
          }  
        break;

        /*
         * Top menu: Play music or stop music.
         * Sub menu: (Intensify) -> Switch between 1984/1989/2021 mode.
        */
        case 1:
          // Play or stop the current music track.
          if(b_wand_menu_sub != true) {          
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);

              if(b_playing_music == true) {
                // Stop music
                b_playing_music = false;

                // Tell the pack to stop music.
                Serial1.write(98);
                
                stopMusic();             
              }
              else {
                if(i_music_count > 0 && i_current_music_track > 99) {
                  // Start music.
                  b_playing_music = true;

                  // Tell the pack to play music.
                  Serial1.write(99);
                  
                  playMusic();
                }
              }
            }
          }
          else {
            // Switch between 1984/1989/2021 mode.
            if(switch_intensify.isPressed() && ms_intensify_timer.isRunning() != true) {
              ms_intensify_timer.start(i_intensify_delay);
              
              // Tell the Proton Pack to cycle through year modes.
              Serial1.write(27);

              w_trig.trackStop(S_BEEPS_BARGRAPH);    
              w_trig.trackGain(S_BEEPS_BARGRAPH, i_volume);
              w_trig.trackPlayPoly(S_BEEPS_BARGRAPH);

              // There is no pack connected, lets change the years.
              if(b_no_pack == true) {
                if(year_mode == 1984) {
                  year_mode = 2021;
                  w_trig.trackStop(S_VOICE_AFTERLIFE);    
                  w_trig.trackStop(S_VOICE_1984);    
                  w_trig.trackGain(S_VOICE_AFTERLIFE, i_volume);
                  w_trig.trackPlayPoly(S_VOICE_AFTERLIFE);

                }
                else if(year_mode == 2021) {
                  year_mode = 1984;

                  w_trig.trackStop(S_VOICE_AFTERLIFE);    
                  w_trig.trackStop(S_VOICE_1984);    
                  w_trig.trackGain(S_VOICE_1984, i_volume);
                  w_trig.trackPlayPoly(S_VOICE_1984);
                }
              }
            }
          }
        break;
      }
    break;
  }
  
  switch(WAND_STATUS) {
    case MODE_OFF:          
      if(switchMode() == true && ms_switch_mode_debounce.justFinished()) {     
        if(FIRING_MODE != SETTINGS) {
          w_trig.trackPlayPoly(S_CLICK);

          PREV_FIRING_MODE = FIRING_MODE;
          FIRING_MODE = SETTINGS;
          
          WAND_ACTION_STATUS = ACTION_SETTINGS;
          i_wand_menu = 5;
          ms_settings_blinking.start(i_settings_blinking_delay);
    
          // Tell the pack we are in settings mode.
          Serial1.write(9);
        }
        else {
          // Only exit the settings menu when on menu #5 in the top menu.
          if(i_wand_menu == 5 && b_wand_menu_sub != true) {
            FIRING_MODE = PREV_FIRING_MODE;
            w_trig.trackPlayPoly(S_CLICK);
            
            bargraphClearAlt();

            switch(PREV_FIRING_MODE) {
              case MESON:
                // Tell the pack we are in meson mode.
                Serial1.write(8);
              break;
  
              case STASIS:
                // Tell the pack we are in stasis mode.
                Serial1.write(7);
              break;
  
              case SLIME:  
                // Tell the pack we are in slime mode.
                Serial1.write(6);
              break;
  
              case PROTON: 
                // Tell the pack we are in proton mode.
                Serial1.write(5);
              break;
  
              default:
                // Tell the pack we are in proton mode.
                Serial1.write(5);
              break;
            }
            
            WAND_ACTION_STATUS = ACTION_IDLE;
  
            wandLightsOff();
          }
        }
      
        ms_switch_mode_debounce.start(a_switch_debounce_time);
      }
    break;

    case MODE_ON:
      if(b_vibration_on == true && WAND_ACTION_STATUS != ACTION_SETTINGS) {
        vibrationSetting();
      }

      // Hat light 2 blinking when the Pack ribbon cable has been removed.
      if(b_pcb == true) {
        if(b_pack_alarm == true) {
          if(ms_hat_2.remaining() < i_hat_2_delay / 2) {
            digitalWrite(led_hat_2, LOW);
          }
          else {
            digitalWrite(led_hat_2, HIGH);
          }

          if(ms_hat_2.justFinished()) {
            ms_hat_2.start(i_hat_2_delay);
          }
        }
        else {
          if(ms_hat_1.isRunning() != true) {
            // Hat 2 stays solid while the Neutrona Wand is on. It will blink though when about to overheat.
            digitalWrite(led_hat_2, HIGH);
          }
        }
      }

      // Top white light.
      if(ms_white_light.justFinished()) {
        ms_white_light.start(d_white_light_interval);
        if(digitalRead(led_white) == LOW) {
          digitalWrite(led_white, HIGH);
        }
        else {
          digitalWrite(led_white, LOW);
        }
      }

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
          w_trig.trackPlayPoly(S_AFTERLIFE_GUN_LOOP_1, true);
          w_trig.trackLoop(S_AFTERLIFE_GUN_LOOP_1, 1);
          ms_gun_loop_1.stop();
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

void checkMusic() { 
  if(ms_check_music.justFinished() && ms_music_next_track.isRunning() != true) {
    ms_check_music.start(i_music_check_delay);
        
    // Loop through all the tracks if the music is not set to repeat a track.
    if(b_playing_music == true && b_repeat_track == false) {      
      if(!w_trig.isTrackPlaying(i_current_music_track)) {
        ms_check_music.stop();
                
        stopMusic();

        // Tell the pack to stop playing music.
        Serial1.write(98);
            
        if(i_current_music_track + 1 > i_music_track_start + i_music_count - 1) {
          i_current_music_track = i_music_track_start;
        }
        else {
          i_current_music_track++;          
        }

        // Tell the pack which music track to change to.
        Serial1.write(i_current_music_track);
    
        ms_music_next_track.start(i_music_next_track_delay);
      }
    }
  }

  if(ms_music_next_track.justFinished()) {
    ms_music_next_track.stop();
        
    playMusic(); 

    // Tell the pack to play music.
    Serial1.write(99);
    
    ms_check_music.start(i_music_check_delay); 
  }
} 

void stopMusic() {
  w_trig.trackStop(i_current_music_track);
  w_trig.update();
}

void playMusic() {  
  w_trig.trackGain(i_current_music_track, i_volume_music);
  w_trig.trackPlayPoly(i_current_music_track, true);
  
  if(b_repeat_track == true) {
    w_trig.trackLoop(i_current_music_track, 1);
  }
  else {
    w_trig.trackLoop(i_current_music_track, 0);
  }

  w_trig.update();
}

void settingsBlinkingLights() {  
  if(ms_settings_blinking.justFinished()) {
     ms_settings_blinking.start(i_settings_blinking_delay);
  }

  if(ms_settings_blinking.remaining() < i_settings_blinking_delay / 2) {
    bool b_solid_five = false;

    // Indicator for looping track setting.
    if(b_repeat_track == true && i_wand_menu == 5 && WAND_ACTION_STATUS != ACTION_OVERHEATING && b_wand_menu_sub != true) {
      b_solid_five = true;
    }

    // Indicator for crossing the streams setting.
    if(b_cross_the_streams == true && i_wand_menu == 5 && WAND_ACTION_STATUS != ACTION_OVERHEATING && b_wand_menu_sub == true) {
      b_solid_five = true;
    }

    if(b_bargraph_alt == true) {
      if(b_solid_five == true) {
        for(int i = 0; i < 4; i++) {
          ht_bargraph.clearLedNow(i_bargraph[i]);
        }

        for(int i = 4; i < 5; i++) {
          ht_bargraph.setLedNow(i_bargraph[i]);
        }
      }
      else {
        ht_bargraph.clearAll();
      }
    }
    else {
      digitalWrite(led_bargraph_1, HIGH);
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
  }
  else {
    switch(i_wand_menu) {
      case 5:
        if(b_bargraph_alt == true) {
          int i_leds = 5;

          if(WAND_ACTION_STATUS == ACTION_OVERHEATING) {
            i_leds = 28;
          }

          for(int i = 0; i < i_leds; i++) {
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
      break;

      case 4:
        if(b_bargraph_alt == true) {
          for(int i = 0; i < 4; i++) {
            ht_bargraph.setLedNow(i_bargraph[i]);
          }
        }
        else {
          digitalWrite(led_bargraph_1, LOW);
          digitalWrite(led_bargraph_2, LOW);
          digitalWrite(led_bargraph_3, LOW);
          digitalWrite(led_bargraph_4, LOW);
          digitalWrite(led_bargraph_5, HIGH);
        }
      break;

      case 3:
        if(b_bargraph_alt == true) {
          for(int i = 0; i < 3; i++) {
            ht_bargraph.setLedNow(i_bargraph[i]);
          }
        }
        else {      
          digitalWrite(led_bargraph_1, LOW);
          digitalWrite(led_bargraph_2, LOW);
          digitalWrite(led_bargraph_3, LOW);
          digitalWrite(led_bargraph_4, HIGH);
          digitalWrite(led_bargraph_5, HIGH);
        }
      break;

      case 2:
        if(b_bargraph_alt == true) {
          for(int i = 0; i < 2; i++) {
            ht_bargraph.setLedNow(i_bargraph[i]);
          }
        }
        else {   
          digitalWrite(led_bargraph_1, LOW);
          digitalWrite(led_bargraph_2, LOW);
          digitalWrite(led_bargraph_3, HIGH);
          digitalWrite(led_bargraph_4, HIGH);
          digitalWrite(led_bargraph_5, HIGH);
        }
      break;

      case 1:
        if(b_bargraph_alt == true) {
          for(int i = 0; i < 1; i++) {
            ht_bargraph.setLedNow(i_bargraph[i]);
          }
        }
        else {  
          digitalWrite(led_bargraph_1, LOW);
          digitalWrite(led_bargraph_2, HIGH);
          digitalWrite(led_bargraph_3, HIGH);
          digitalWrite(led_bargraph_4, HIGH);
          digitalWrite(led_bargraph_5, HIGH);
        }
      break;
    }
  }
}

// Change the WAND_STATE here based on switches changing or pressed.
void checkSwitches() {
  if(b_debug == true) {
    if(b_pcb != true) {
      Serial.print(F("A6 -> "));
      Serial.println(analogRead(switch_mode));  

      Serial.print(F("\n"));
      
      Serial.print(F("A7 -> "));
      Serial.println(analogRead(switch_barrel));
    }
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

    case MODE_ON:
      // This is for when the mode switch is enabled for video game mode. b_cross_the_streams must not be true.
      if(WAND_ACTION_STATUS != ACTION_FIRING && WAND_ACTION_STATUS != ACTION_OFF && WAND_ACTION_STATUS != ACTION_OVERHEATING && b_cross_the_streams != true) {
          if(switchMode() == true && ms_switch_mode_debounce.justFinished()) {
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
              FIRING_MODE = SETTINGS;
            }
            else {
              FIRING_MODE = PROTON;

              bargraphClearAlt();
            }
   
            w_trig.trackPlayPoly(S_CLICK);
  
            switch(FIRING_MODE) {
              case SETTINGS:
                WAND_ACTION_STATUS = ACTION_SETTINGS;
                i_wand_menu = 5;
                ms_settings_blinking.start(i_settings_blinking_delay);
  
                // Tell the pack we are in settings mode.
                Serial1.write(9);
              break;
  
              case MESON:
                WAND_ACTION_STATUS = ACTION_IDLE;
                wandHeatUp();
  
                // Tell the pack we are in meson mode.
                Serial1.write(8);
              break;
  
              case STASIS:
                WAND_ACTION_STATUS = ACTION_IDLE;
                wandHeatUp();
  
                // Tell the pack we are in stasis mode.
                Serial1.write(7);
              break;
  
              case SLIME:
                WAND_ACTION_STATUS = ACTION_IDLE;
                wandHeatUp();
  
                // Tell the pack we are in slime mode.
                Serial1.write(6);
              break;
  
              case PROTON:
                WAND_ACTION_STATUS = ACTION_IDLE;
                wandHeatUp();
  
                // Tell the pack we are in proton mode.
                Serial1.write(5);
              break;
            }
          }
          
          ms_switch_mode_debounce.start(a_switch_debounce_time);
        }
      }

      if(WAND_ACTION_STATUS != ACTION_OVERHEATING) {
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

      if(WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
        if(switch_intensify.getState() == LOW && ms_intensify_timer.isRunning() != true && switch_wand.getState() == LOW && switch_vent.getState() == LOW && switch_activate.getState() == LOW && b_pack_on == true && switchBarrel() != true) {          
          if(WAND_ACTION_STATUS != ACTION_FIRING) {
            WAND_ACTION_STATUS = ACTION_FIRING;
          }
        
          b_firing_intensify = true;
        }

        // When the mode switch is changed to a alternate firing button. Video game modes are disabled and the wand menu settings can only be accessed when the Neutrona wand is powered down.
        if(b_cross_the_streams == true) {          
          if(switchMode() == true && ms_switch_mode_debounce.justFinished() && switch_wand.getState() == LOW && switch_vent.getState() == LOW && switch_activate.getState() == LOW && b_pack_on == true && switchBarrel() != true) {
            if(WAND_ACTION_STATUS != ACTION_FIRING) {
              WAND_ACTION_STATUS = ACTION_FIRING;
            }

            b_firing_alt = true;

            ms_switch_mode_debounce.start(a_switch_debounce_time);
          }
          else if(switchMode() != true && ms_switch_mode_debounce.justFinished()) {
            if(b_firing_intensify != true && WAND_ACTION_STATUS == ACTION_FIRING) {
              WAND_ACTION_STATUS = ACTION_IDLE;
            }

            b_firing_alt = false;

            ms_switch_mode_debounce.start(a_switch_debounce_time);
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
      else if(WAND_ACTION_STATUS == ACTION_OVERHEATING) {
        if(switch_activate.getState() == HIGH) {
          WAND_ACTION_STATUS = ACTION_OFF;
        }
      }
    break;
  }
}

void wandOff() {
  // Tell the pack the wand is turned off.
  Serial1.write(2);

  if(FIRING_MODE == SETTINGS) {
    // If the wand is shut down while we are in settings mode (can happen if the pack is manually turned off), switch the wand and pack to proton mode.
    Serial1.write(5);
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

  w_trig.trackStop(S_AFTERLIFE_GUN_LOOP_1);
  w_trig.trackStop(S_AFTERLIFE_GUN_LOOP_2);
  
  w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_1);
  w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_2);
  w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_DOWN_1);
  w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_DOWN_2);
  w_trig.trackStop(S_BOOTUP);

  // Turn off any overheating sounds.
  w_trig.trackStop(S_CLICK);
  w_trig.trackStop(S_VENT_DRY);

  w_trig.trackStop(S_FIRE_START_SPARK);
  w_trig.trackStop(S_PACK_SLIME_OPEN);
  w_trig.trackStop(S_STASIS_START);
  w_trig.trackStop(S_MESON_START);

  w_trig.trackPlayPoly(S_WAND_SHUTDOWN);
  w_trig.trackPlayPoly(S_AFTERLIFE_GUN_RAMP_DOWN_1, true);
  
  // Turn off some timers.
  ms_bargraph.stop();
  ms_bargraph_alt.stop();
  ms_bargraph_firing.stop();
  ms_overheat_initiate.stop();
  ms_overheating.stop();
  ms_settings_blinking.stop();
  ms_hat_1.stop();

  // Turn off remaining lights.
  wandLightsOff();
  barrelLightsOff();

  switch(year_mode) {
    case 2021:
      i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_2021;
    break;

    case 1984:
      i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_1984;
    break;
  }
}

void modeActivate() {
  // Tell the pack the wand is turned on.
  Serial1.write(1);
  
  WAND_STATUS = MODE_ON;
  WAND_ACTION_STATUS = ACTION_IDLE;
  
  // Ramp up the bargraph.
  switch(year_mode) {
    case 2021:
      i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_2021;
    break;

    case 1984:
      i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_1984 * 2;
    break;
  }

  bargraphRampUp();
  
  // Turn on slo-blo light (and front left LED if using a Ardunio Nano).
  analogWrite(led_slo_blo, 255);
  
  // If using the gpstar neutrona wand micro controller, the front left LED is wired separately, lets turn it on.
  if(b_pcb == true) {
    analogWrite(led_front_left, 255);
  }

  // Top white light.
  ms_white_light.start(d_white_light_interval);
  digitalWrite(led_white, LOW);

  switch(year_mode) {
    case 1984:
      w_trig.trackPlayPoly(S_CLICK, true);
    break;

    default:
      soundIdleLoop(true);

      w_trig.trackGain(S_AFTERLIFE_GUN_RAMP_1, i_volume);
      w_trig.trackPlayPoly(S_AFTERLIFE_GUN_RAMP_1, true); // Start track
      ms_gun_loop_1.start(2000);
    break;
  }
}

void soundIdleLoop(bool fade) {      
  switch(i_power_mode) {
    case 1:
      if(fade == true) {
        w_trig.trackGain(S_IDLE_LOOP_GUN_1, i_volume - 20);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_1, true);
        w_trig.trackFade(S_IDLE_LOOP_GUN_1, i_volume, 1000, 0);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_1, 1);
      }
      else {
        w_trig.trackGain(S_IDLE_LOOP_GUN_1, i_volume);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_1, true);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_1, 1);
      }
     break;

     case 2:
      if(fade == true) {
        w_trig.trackGain(S_IDLE_LOOP_GUN_1, i_volume - 20);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_1, true);
        w_trig.trackFade(S_IDLE_LOOP_GUN_1, i_volume, 1000, 0);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_1, 1);
      }
      else {
        w_trig.trackGain(S_IDLE_LOOP_GUN_1, i_volume);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_1, true);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_1, 1);
      }
     break;

     case 3:
      if(fade == true) {
        w_trig.trackGain(S_IDLE_LOOP_GUN_2, i_volume - 20);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_2, true);
        w_trig.trackFade(S_IDLE_LOOP_GUN_2, i_volume, 1000, 0);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_2, 1);
      }
      else {
        w_trig.trackGain(S_IDLE_LOOP_GUN_2, i_volume);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_2, true);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_2, 1);
      }
     break;

     case 4:
      if(fade == true) {
        w_trig.trackGain(S_IDLE_LOOP_GUN_2, i_volume - 20);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_2, true);
        w_trig.trackFade(S_IDLE_LOOP_GUN_2, i_volume, 1000, 0);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_2, 1);
      }
      else {
        w_trig.trackGain(S_IDLE_LOOP_GUN_2, i_volume);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_2, true);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_2, 1);
      }
     break;

     case 5:
      if(fade == true) {
        w_trig.trackGain(S_IDLE_LOOP_GUN_5, i_volume - 20);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_5, true);
        w_trig.trackFade(S_IDLE_LOOP_GUN_5, i_volume, 1000, 0);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_5, 1);
      }
      else {
        w_trig.trackGain(S_IDLE_LOOP_GUN_5, i_volume);
        w_trig.trackPlayPoly(S_IDLE_LOOP_GUN_5, true);
        w_trig.trackLoop(S_IDLE_LOOP_GUN_5, 1);
      }
     break;
    }
}

void soundIdleLoopStop() {
  w_trig.trackStop(S_IDLE_LOOP_GUN);
  w_trig.trackStop(S_IDLE_LOOP_GUN_1);
  w_trig.trackStop(S_IDLE_LOOP_GUN_2);
  w_trig.trackStop(S_IDLE_LOOP_GUN_3);
  w_trig.trackStop(S_IDLE_LOOP_GUN_4);
  w_trig.trackStop(S_IDLE_LOOP_GUN_5);
}

void soundIdleStart() {
  if(b_sound_idle == false) {      
    switch(year_mode) {
      case 1984:
        w_trig.trackPlayPoly(S_BOOTUP, true);

        soundIdleLoop(true);        
      break;
  
      default:
        w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_1);
        w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_DOWN_1);
        w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_DOWN_2);
        w_trig.trackStop(S_AFTERLIFE_GUN_LOOP_1);
        
        w_trig.trackPlayPoly(S_AFTERLIFE_GUN_RAMP_2, true); // Start track

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
      w_trig.trackPlayPoly(S_AFTERLIFE_GUN_LOOP_2, true);
      w_trig.trackLoop(S_AFTERLIFE_GUN_LOOP_2, 1);
      
      ms_gun_loop_2.stop();
    }
  }
}

void soundIdleStop() {
  if(b_sound_idle == true) {
    switch(year_mode) {
      case 1984:
        w_trig.trackPlayPoly(S_WAND_SHUTDOWN, true);
      break;

      default:
        if(WAND_ACTION_STATUS == ACTION_OVERHEATING) {
          w_trig.trackPlayPoly(S_WAND_SHUTDOWN, true);
        }

        w_trig.trackPlayPoly(S_AFTERLIFE_GUN_RAMP_DOWN_2, true);

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
        w_trig.trackStop(S_BOOTUP);
        soundIdleLoopStop();
      break;

      case 2021:
        w_trig.trackStop(S_AFTERLIFE_GUN_RAMP_2);
        w_trig.trackStop(S_AFTERLIFE_GUN_LOOP_2);
      break;
    }
  }

  b_sound_idle = false;
}

void soundBeepLoopStop() {
  if(b_beeping == true) {
    b_beeping = false;
    
    w_trig.trackStop(S_AFTERLIFE_BEEP_WAND);
    w_trig.trackStop(S_AFTERLIFE_BEEP_WAND_S1);
    w_trig.trackStop(S_AFTERLIFE_BEEP_WAND_S2);
    w_trig.trackStop(S_AFTERLIFE_BEEP_WAND_S3);
    w_trig.trackStop(S_AFTERLIFE_BEEP_WAND_S4);
    w_trig.trackStop(S_AFTERLIFE_BEEP_WAND_S5);
    
    ms_reset_sound_beep.stop();
    ms_reset_sound_beep.start(i_sound_timer);
  }
}
void soundBeepLoop() {  
  if(ms_reset_sound_beep.justFinished() && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
    if(b_beeping == false) {
      switch(i_power_mode) {
        case 1:
          w_trig.trackPlayPoly(S_AFTERLIFE_BEEP_WAND_S1, true);
          
          if(year_mode != 1984) {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S1, 1);
          }
          else {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S1, 0);
          }
         break;
  
         case 2:
          w_trig.trackPlayPoly(S_AFTERLIFE_BEEP_WAND_S2, true);
          
          if(year_mode != 1984) {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S2, 1);
          }
          else {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S2, 0);
          }
         break;
  
         case 3:
          w_trig.trackPlayPoly(S_AFTERLIFE_BEEP_WAND_S3, true);
          
          if(year_mode != 1984) {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S3, 1);
          }
          else {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S3, 0);
          }
         break;
  
         case 4:
          w_trig.trackPlayPoly(S_AFTERLIFE_BEEP_WAND_S4, true);
          
          if(year_mode != 1984) {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S4, 1);
          }
          else {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S4, 0);
          }
         break;
  
         case 5:
          w_trig.trackPlayPoly(S_AFTERLIFE_BEEP_WAND_S5, true);
          
          if(year_mode != 1984) {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S5, 1);
          }
          else {
            w_trig.trackLoop(S_AFTERLIFE_BEEP_WAND_S5, 0);
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
  w_trig.trackPlayPoly(S_FIRE_START_SPARK);
  
  switch(FIRING_MODE) {
    case PROTON:
        if(b_firing_intensify == true) {
          // Reset some sound triggers.
          b_sound_firing_intensify_trigger = true;
          w_trig.trackPlayPoly(S_FIRE_LOOP_GUN, true);
          w_trig.trackLoop(S_FIRE_LOOP_GUN, 1);
        }
        else {
          b_sound_firing_intensify_trigger = false;
        }

        if(b_firing_alt == true) {
          // Reset some sound triggers.
          b_sound_firing_alt_trigger = true;

          w_trig.trackPlayPoly(S_FIRING_LOOP_GB1, true);
          w_trig.trackLoop(S_FIRING_LOOP_GB1, 1);          
        }
        else {
          b_sound_firing_alt_trigger = false;
        }

        w_trig.trackPlayPoly(S_FIRE_START);        
    break;

    case SLIME:
      w_trig.trackPlayPoly(S_SLIME_START);
      
      w_trig.trackPlayPoly(S_SLIME_LOOP, true);
      w_trig.trackLoop(S_SLIME_LOOP, 1);
    break;

    case STASIS:
      w_trig.trackPlayPoly(S_STASIS_START);
      
      w_trig.trackPlayPoly(S_STASIS_LOOP, true);
      w_trig.trackLoop(S_STASIS_LOOP, 1);
    break;

    case MESON:
      w_trig.trackPlayPoly(S_MESON_START);
      
      w_trig.trackPlayPoly(S_MESON_LOOP, true);
      w_trig.trackLoop(S_MESON_LOOP, 1);
    break;

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
  if(b_pcb == true) {
    digitalWrite(led_hat_1, HIGH);
  }
  ms_hat_1.stop();

  if(ms_intensify_timer.isRunning() != true) {
    ms_intensify_timer.start(i_intensify_delay);
  }
  
  // Tell the Proton Pack that the Neutrona wand is firing in Intensify mode.
  if(b_firing_intensify == true) {
    Serial1.write(21);
  }

  // Tell the Proton Pack that the Neutrona wand is firing in Alt mode.
  if(b_firing_alt == true) {
    Serial1.write(23);
  }

  // Stop all firing sounds first.
  switch(FIRING_MODE) {
    case PROTON:
      w_trig.trackStop(S_FIRE_LOOP);
      w_trig.trackStop(S_FIRE_LOOP_GUN);
      w_trig.trackStop(S_FIRING_LOOP_GB1);

      w_trig.trackStop(S_FIRE_START);
      w_trig.trackStop(S_FIRE_START_SPARK);
      w_trig.trackStop(S_FIRING_END_GUN);
      w_trig.trackStop(S_FIRE_LOOP_IMPACT);
    break;

    case SLIME:
      w_trig.trackStop(S_SLIME_START);
      w_trig.trackStop(S_SLIME_LOOP);
      w_trig.trackStop(S_SLIME_END);
    break;

    case STASIS:
      w_trig.trackStop(S_STASIS_START);
      w_trig.trackStop(S_STASIS_LOOP);
      w_trig.trackStop(S_STASIS_END);
    break;

    case MESON:
      w_trig.trackStop(S_MESON_START);
      w_trig.trackStop(S_MESON_LOOP);
      w_trig.trackStop(S_MESON_END);
    break;

    case SETTINGS:
      // Nothing.
    break;
  }

  ms_firing_start_sound_delay.start(i_fire_stop_sound_delay);

  // Tell the pack the wand is firing.
  Serial1.write(3);

  ms_overheat_initiate.stop();

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

  ms_impact.start(random(10,15) * 1000);
}

void modeFireStopSounds() {
  // Reset some sound triggers.
  b_sound_firing_intensify_trigger = false;
  b_sound_firing_alt_trigger = false;
  b_sound_firing_cross_the_streams = false;

  ms_firing_stop_sound_delay.stop();

 switch(FIRING_MODE) {
    case PROTON:
      w_trig.trackPlayPoly(S_FIRING_END_GUN, true);
    break;

    case SLIME:
      w_trig.trackPlayPoly(S_SLIME_END, true);
    break;

    case STASIS:
      w_trig.trackPlayPoly(S_STASIS_END, true);
    break;

    case MESON:
      w_trig.trackPlayPoly(S_MESON_END, true);
    break;

    case SETTINGS:
      // Nothing.
    break;
  }

  if(b_firing_cross_streams == true) {
    w_trig.trackPlayPoly(S_CROSS_STREAMS_END, true);

    b_firing_cross_streams = false;
  }
}

void modeFireStop() {
  ms_overheat_initiate.stop();
  
  // Tell the pack the wand stopped firing.
  Serial1.write(4);
  
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
      i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_1984;
    break;
  }
  
  bargraphRampUp();
  
  ms_firing_stream_blue.stop();
  ms_firing_lights.stop();
  ms_impact.stop();

  i_barrel_light = 0;
  ms_firing_lights_end.start(10);

  // If using optional items on the gpstar Neutrona Wand micro controller.
  if(b_pcb == true) {
    digitalWrite(led_hat_1, LOW); // Turn off hat light 1.
    digitalWrite(led_barrel_tip, LOW); // Turn off hat the wand barrel tip LED.
  }
  
  ms_hat_1.stop();

  // Stop all other firing sounds.
  switch(FIRING_MODE) {
    case PROTON:
      w_trig.trackStop(S_FIRE_LOOP);
      w_trig.trackStop(S_FIRE_LOOP_GUN);
      w_trig.trackStop(S_FIRING_LOOP_GB1);
      //w_trig.trackStop(S_FIRING_END_GUN);
      w_trig.trackStop(S_FIRE_START);
      w_trig.trackStop(S_FIRE_START_SPARK);
      w_trig.trackStop(S_FIRE_LOOP_IMPACT);
    break;

    case SLIME:
      w_trig.trackStop(S_SLIME_START);
      w_trig.trackStop(S_SLIME_LOOP);
      w_trig.trackStop(S_SLIME_END);
    break;

    case STASIS:
      w_trig.trackStop(S_STASIS_START);
      w_trig.trackStop(S_STASIS_LOOP);
      w_trig.trackStop(S_STASIS_END);
    break;

    case MESON:
      w_trig.trackStop(S_MESON_START);
      w_trig.trackStop(S_MESON_LOOP);
      w_trig.trackStop(S_MESON_END);
    break;

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
    // Tell the Proton Pack that the Neutrona wand is firing in Intensify mode.
    Serial1.write(21);

    b_sound_firing_intensify_trigger = true;
    w_trig.trackPlayPoly(S_FIRE_LOOP_GUN, true);
    w_trig.trackLoop(S_FIRE_LOOP_GUN, 1);
  }

  if(b_firing_intensify != true && b_sound_firing_intensify_trigger == true) {
    // Tell the Proton Pack that the Neutrona wand is no longer firing in Intensify mode.
    Serial1.write(22);

    b_sound_firing_intensify_trigger = false;
    w_trig.trackStop(S_FIRE_LOOP_GUN);
  }

  if(b_firing_alt == true && b_sound_firing_alt_trigger != true) {
    // Tell the Proton Pack that the Neutrona wand is firing in Alt mode.
    Serial1.write(23);

    b_sound_firing_alt_trigger = true;
    w_trig.trackPlayPoly(S_FIRING_LOOP_GB1, true);
    w_trig.trackLoop(S_FIRING_LOOP_GB1, 1);
  }

  if(b_firing_alt != true && b_sound_firing_alt_trigger == true) {
    // Tell the Proton Pack that the Neutrona wand is firing in Alt mode.
    Serial1.write(24);

    b_sound_firing_alt_trigger = false;
    w_trig.trackStop(S_FIRING_LOOP_GB1);
  }

  if(b_firing_alt == true && b_firing_intensify == true && b_sound_firing_cross_the_streams != true && b_firing_cross_streams != true) {
    // Tell the Proton Pack that the Neutrona wand is crossing the streams.
    Serial1.write(25);

    b_firing_cross_streams = true;
    b_sound_firing_cross_the_streams = true;
    w_trig.trackPlayPoly(S_CROSS_STREAMS_START, true);
    w_trig.trackPlayPoly(S_FIRE_START_SPARK);
    w_trig.trackPlayPoly(S_FIRE_LOOP, true);
    w_trig.trackLoop(S_FIRE_LOOP, 1);
  }

  if((b_firing_alt != true || b_firing_intensify != true) && b_firing_cross_streams == true) {
    // Tell the Proton Pack that the Neutrona wand is no longer crossing the streams.
    Serial1.write(26);

    b_firing_cross_streams = false;
    b_sound_firing_cross_the_streams = false;
    w_trig.trackPlayPoly(S_CROSS_STREAMS_END, true);
    w_trig.trackStop(S_FIRE_LOOP);
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
      if(b_pcb == true) {
        digitalWrite(led_hat_1, HIGH);
      }
      ms_hat_1.stop();

      // Tell the pack to revert back to regular cyclotron speeds.
      Serial1.write(12);
    }
    else if(b_overheat_mode[i_power_mode - 1] == true && ms_overheat_initiate.remaining() == 0 && b_overheat_enabled == true) {
      // If the user changes back to power mode that overheats while firing, start up a timer.
      ms_overheat_initiate.start(i_ms_overheat_initiate[i_power_mode - 1]);
    }
  }
  else {
    if(ms_overheat_initiate.isRunning()) {
      ms_overheat_initiate.stop();
      
      // Tell the pack to revert back to regular cyclotron speeds.
      Serial1.write(12);
    }
  }

  /*
   * CRGB 
   * R = green
   * G = red
   * B = blue
   * 
   * yellow = 255, 255, 0
   * mid-yellow = 150,255,0
   * orange = 40, 255, 0
   * dark orange = 20, 255, 0
   */
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

    case SETTINGS:
      // Nothing.
    break;
  }

  // Bargraph loop / scroll.
  if(ms_bargraph_firing.justFinished()) {
    bargraphRampFiring();
  }

  // Mix some impact sound every 10-15 seconds while firing.
  if(ms_impact.justFinished()) {
    w_trig.trackPlayPoly(S_FIRE_LOOP_IMPACT);
    ms_impact.start(15000);
  }
}

void wandHeatUp() {
  w_trig.trackStop(S_FIRE_START_SPARK);
  w_trig.trackStop(S_PACK_SLIME_OPEN);
  w_trig.trackStop(S_STASIS_OPEN);
  w_trig.trackStop(S_MESON_OPEN);

  switch(FIRING_MODE) {
    case PROTON:
      w_trig.trackPlayPoly(S_FIRE_START_SPARK);
    break;

    case SLIME:
      w_trig.trackPlayPoly(S_PACK_SLIME_OPEN);
    break;

    case STASIS:
      w_trig.trackPlayPoly(S_STASIS_OPEN);
    break;

    case MESON:
      w_trig.trackPlayPoly(S_MESON_OPEN);
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

void fireStream(int r, int g, int b) {
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
  
  for(int i = 0; i < BARREL_NUM_LEDS; i++) {
    barrel_leds[i] = CRGB(0,0,0);
  }

  // Turn off the wand barrel tip LED.
  if(b_pcb == true) {
    digitalWrite(led_barrel_tip, LOW);
  }

  ms_fast_led.start(i_fast_led_delay);
}

void fireStreamStart(int r, int g, int b) {
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

void fireStreamEnd(int r, int g, int b) {
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

void vibrationWand(int i_level) {
  if(b_vibration_on == true && b_vibration_enabled == true) {
    // Only vibrate the wand during firing only when enabled. (When enabled by the pack)
    if(b_vibration_firing == true) {
      if(WAND_ACTION_STATUS == ACTION_FIRING) {
        if(i_level != i_vibration_level_prev) {
          i_vibration_level_prev = i_level;
          analogWrite(vibration, i_level);
        }
      }
      else {
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
    analogWrite(vibration, 0);
  }
}

/*
 * Bargraph ramping during firing.
 * Optional barrel LED tip strobing is controlled from here to give it a ramp effect if the Proton Pack and Neutrona Wand are going to overheat.
*/
void bargraphRampFiring() {
  // (Optional) 28 Segment barmeter bargraph.
  if(b_bargraph_alt == true) {    
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

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, HIGH);
        }
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

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, HIGH);
        }
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

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, LOW);
        }
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

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, LOW);
        }        
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

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, HIGH);
        }
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

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, HIGH);
        }
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

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, LOW);
        }        
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

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, LOW);
        }        
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

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, HIGH);
        }        
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

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, HIGH);
        }        
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

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, LOW);
        }        
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

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, LOW);
        }        
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

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, HIGH);
        }        
      break;

      case 13:
        vibrationWand(i_vibration_level + 115);

        ht_bargraph.setLedNow(i_bargraph[0]);
        ht_bargraph.setLedNow(i_bargraph[27]);

        ht_bargraph.clearLedNow(i_bargraph[1]);
        ht_bargraph.clearLedNow(i_bargraph[26]);

        i_bargraph_status_alt--;

        b_bargraph_up = false;

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, HIGH);
        }        
      break;    
    }
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

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, HIGH);
        }        
      break;

      case 2:
        vibrationWand(i_vibration_level + 112);  
        
        digitalWrite(led_bargraph_1, HIGH);
        digitalWrite(led_bargraph_2, LOW);
        digitalWrite(led_bargraph_3, HIGH);
        digitalWrite(led_bargraph_4, LOW);
        digitalWrite(led_bargraph_5, HIGH);
        i_bargraph_status++;

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, LOW);
        }        
      break;

      case 3:
        vibrationWand(i_vibration_level + 115);
        
        digitalWrite(led_bargraph_1, HIGH);
        digitalWrite(led_bargraph_2, HIGH);
        digitalWrite(led_bargraph_3, LOW);
        digitalWrite(led_bargraph_4, HIGH);
        digitalWrite(led_bargraph_5, HIGH);
        i_bargraph_status++;

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, HIGH);
        }        
      break;

      case 4:
        vibrationWand(i_vibration_level + 112);

        digitalWrite(led_bargraph_1, HIGH);
        digitalWrite(led_bargraph_2, LOW);
        digitalWrite(led_bargraph_3, HIGH);
        digitalWrite(led_bargraph_4, LOW);
        digitalWrite(led_bargraph_5, HIGH);
        i_bargraph_status++;

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, LOW);
        }        
      break;

      case 5:
        vibrationWand(i_vibration_level + 110);
        
        digitalWrite(led_bargraph_1, LOW);
        digitalWrite(led_bargraph_2, HIGH);
        digitalWrite(led_bargraph_3, HIGH);
        digitalWrite(led_bargraph_4, HIGH);
        digitalWrite(led_bargraph_5, LOW);
        i_bargraph_status = 1;

        if(b_pcb == true) {
          digitalWrite(led_barrel_tip, HIGH);
        }        
      break;
    }
  }

  int i_ramp_interval = d_bargraph_ramp_interval;

  if(b_bargraph_alt == true) {
    // Switch to a different ramp speed if using the (Optional) 28 segment barmeter bargraph.
    i_ramp_interval = d_bargraph_ramp_interval_alt;
  }

  // If in a power mode on the wand that can overheat, change the speed of the bargraph ramp during firing based on time remaining before we overheat.
  if(b_overheat_mode[i_power_mode - 1] == true && ms_overheat_initiate.isRunning() && b_overheat_enabled == true) {
    if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 6) {
      if(b_bargraph_alt == true) {
        ms_bargraph_firing.start(i_ramp_interval / i_ramp_interval);
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 5);
      }
      
      cyclotronSpeedUp(6);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 5) {
      if(b_bargraph_alt == true) {
        ms_bargraph_firing.start(i_ramp_interval / 9);
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 4);
      }

      cyclotronSpeedUp(5);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 4) {
      if(b_bargraph_alt == true) {
        ms_bargraph_firing.start(i_ramp_interval / 7);
      }
      else {
        ms_bargraph_firing.start(i_ramp_interval / 3.5);
      }

      cyclotronSpeedUp(4);    
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 3) {
      if(b_bargraph_alt == true) {
        ms_bargraph_firing.start(i_ramp_interval / 5);
      }
      else {      
        ms_bargraph_firing.start(i_ramp_interval / 3);
      }

      cyclotronSpeedUp(3);
    }
    else if(ms_overheat_initiate.remaining() < i_ms_overheat_initiate[i_power_mode - 1] / 2) {
      if(b_bargraph_alt == true) {
        ms_bargraph_firing.start(i_ramp_interval / 3);
      }
      else {      
        ms_bargraph_firing.start(i_ramp_interval / 2.5);
      }

      cyclotronSpeedUp(2);
    }
    else {
      ms_bargraph_firing.start(i_ramp_interval / 2);
      i_cyclotron_speed_up = 1;
    }
  }
  else {  
    ms_bargraph_firing.start(i_ramp_interval / 2);
  }
}

void cyclotronSpeedUp(int i_switch) {
  if(i_switch != i_cyclotron_speed_up) {
    if(i_switch == 4) {
      // Tell pack to start beeping before we overheat it.
      Serial1.write(15);

      // Beep the wand 8 times.
      w_trig.trackPlayPoly(S_BEEP_8);

      ms_hat_1.start(i_hat_1_delay);      
    }
    
    i_cyclotron_speed_up++;
    
    // Tell the pack to speed up the cyclotron.
    Serial1.write(13);
  }  
}

/*
 * 2021 mode for optional 28 segment bargraph. Checks if we ramp up or down when changing power levels.
*/
void bargraphPowerCheck2021Alt() {
  if(WAND_ACTION_STATUS != ACTION_FIRING && WAND_ACTION_STATUS != ACTION_SETTINGS && WAND_ACTION_STATUS != ACTION_OVERHEATING) {
    if(i_power_mode != i_power_mode_prev) {
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
  if(b_bargraph_alt == true) {
    ht_bargraph.clearAll();
    /*
    for(int i = 0; i < 28; i++) {
      ht_bargraph.clearLedNow(i_bargraph[i]);
    }
    */
    i_bargraph_status_alt = 0;
  }
}

void bargraphPowerCheck() {
  // Control for the 28 segment barmeter bargraph.
  if(b_bargraph_alt == true) {
    if(ms_bargraph_alt.justFinished()) {
      int i_bargraph_multiplier[5] = { 7, 6, 5, 4, 3 };
      
      if(year_mode == 2021) {
        for(int i = 0; i <= 4; i++) {
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

void bargraphRampUp() { 
  if(b_bargraph_alt == true) {
    switch(i_bargraph_status_alt) {
      case 0 ... 27:
        ht_bargraph.setLedNow(i_bargraph[i_bargraph_status_alt]);

        if(i_bargraph_status > 22) {
          vibrationWand(i_vibration_level + 80);
        }
        else if(i_bargraph_status > 16) {
          vibrationWand(i_vibration_level + 40);
        }
        else if(i_bargraph_status > 10) {
          vibrationWand(i_vibration_level + 30);
        }
        else if(i_bargraph_status > 4) {
          vibrationWand(i_vibration_level + 20);
        }
        else if(i_bargraph_status > 0) {
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
              // No changes.
            break;
          }
        }
        else {
          ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
        }
      break;

      case 28 ... 56:
        int i_tmp = i_bargraph_status_alt - 27;
        i_tmp = 28 - i_tmp;

        if(WAND_ACTION_STATUS == ACTION_OVERHEATING) {
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
          if((i_power_mode < 5 && year_mode == 2021) || year_mode == 1984) {
            ht_bargraph.clearLedNow(i_bargraph[i_tmp]);
          }

          switch(year_mode) {
            case 1984:
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
  }
  else {
    int t_bargraph_ramp_multiplier = 1;

    if(WAND_ACTION_STATUS == ACTION_OVERHEATING) {
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

void bargraphYearModeUpdate() {
  switch(year_mode) {
    case 2021:
      i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_2021;
    break;

    case 1984:
      i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_1984;
    break;
  }
}

void wandLightsOff() {
  if(b_bargraph_alt == true) {
    bargraphClearAlt();
  }
  else {
    digitalWrite(led_bargraph_1, HIGH);
    digitalWrite(led_bargraph_2, HIGH);
    digitalWrite(led_bargraph_3, HIGH);
    digitalWrite(led_bargraph_4, HIGH);
    digitalWrite(led_bargraph_5, HIGH);
  }

  analogWrite(led_slo_blo, 0);

  // If using the gpstar Neutrona Wand micro controller.
  if(b_pcb == true) {
    analogWrite(led_front_left, 0); // The front left LED is wired separately, lets turn it off.

    digitalWrite(led_hat_1, LOW); // Turn off hat light 1.
    digitalWrite(led_hat_2, LOW); // Turn off hat light 2.
    digitalWrite(led_barrel_tip, LOW); // Turn off the wand barrel tip LED.
  }

  digitalWrite(led_vent, HIGH);
  digitalWrite(led_white, HIGH);

  i_bargraph_status = 0;
  i_bargraph_status_alt = 0;
}

void vibrationOff() {
  analogWrite(vibration, 0);
}

void adjustVolumeEffectsGain() {
  /*
   * Reset the gain on all sound effect tracks.
   */
  for(int i=0; i <= i_last_effects_track; i++) {
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
  if(i_volume_master_percentage + VOLUME_MULTIPLIER > 100) {
    i_volume_master_percentage = 100;
  }
  else {
    i_volume_master_percentage = i_volume_master_percentage + VOLUME_MULTIPLIER;
  }

  i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
  
  w_trig.masterGain(i_volume_master);
}

void decreaseVolume() {
  if(i_volume_master_percentage - VOLUME_MULTIPLIER < 0) {
    i_volume_master_percentage = 0;
  }
  else {
    i_volume_master_percentage = i_volume_master_percentage - VOLUME_MULTIPLIER;
  }

  i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
  
  w_trig.masterGain(i_volume_master);
}

/*
 * Top rotary dial on the wand.
 */
void checkRotary() {
  static int8_t c,val;

  if((val = readRotary())) {
    c += val;

    switch(WAND_ACTION_STATUS) {
      case ACTION_SETTINGS:
        // Counter clockwise.
        if(prev_next_code == 0x0b) {
          if(i_wand_menu - 1 < 1) {
            
            // We are entering the sub menu. Only accessible when the Neutrona Wand is powered down.
            if(WAND_STATUS == MODE_OFF) {
              if(b_wand_menu_sub != true) {
                b_wand_menu_sub = true;
                i_wand_menu = 5;

                // Turn on the slo blow led to indicate we are in the Neutrona Wand sub menu.
                analogWrite(led_slo_blo, 255);
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
          if(i_wand_menu + 1 > 5) {
            // We are leaving the sub menu. Only accessible when the Neutrona Wand is powered down.
            if(WAND_STATUS == MODE_OFF) {
              if(b_wand_menu_sub == true) {
                b_wand_menu_sub = false;
                i_wand_menu = 1;

                // Turn off the slo blow led to indicate we are no longer in the Neutrona Wand sub menu.
                analogWrite(led_slo_blo, 0);
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
        if(WAND_ACTION_STATUS != ACTION_OVERHEATING) {
          // Counter clockwise.
          if(prev_next_code == 0x0b) {
            if(i_power_mode - 1 >= i_power_mode_min && WAND_STATUS == MODE_ON) {
              i_power_mode_prev = i_power_mode;
              i_power_mode--;

              if(year_mode == 2021 && b_bargraph_alt == true) {
                bargraphPowerCheck2021Alt();
              }

              soundBeepLoopStop();
      
              switch(year_mode) {
                case 1984:
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
              Serial1.write(96);
            }
          }
          
          // Clockwise.
          if(prev_next_code == 0x07) {
            if(i_power_mode + 1 <= i_power_mode_max && WAND_STATUS == MODE_ON) {
              i_power_mode_prev = i_power_mode;
              i_power_mode++;
              
              if(year_mode == 2021 && b_bargraph_alt == true) {
                bargraphPowerCheck2021Alt();
              }

              soundBeepLoopStop();
      
              switch(year_mode) {
                case 1984:
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
              Serial1.write(97);
            }
          }
        }
      break;
    }    
  }
}

/*
 * Tell the pack which power level the wand is at.
 */
void updatePackPowerLevel() {
  switch(i_power_mode) {
    case 5:
      // Level 5
      Serial1.write(20);
    break;

    case 4:
      // Level 4
      Serial1.write(19);
    break;

    case 3:
      // Level 3
      Serial1.write(18);
    break;

    case 2:
      // Level 2
      Serial1.write(17);
    break;

    default:
      // Level 1
      Serial1.write(16);
    break;
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
  for(int i = 0; i < BARREL_NUM_LEDS; i++) {
    barrel_leds[i] = CRGB(0,0,0);
  }
  
  ms_fast_led.start(i_fast_led_delay);
}

/*
Mode switch is connected to analog input.
PCB builds is pulled high.
Nano builds is pulled low.
*/
bool switchMode() {
  if(b_pcb == true) {
    if(analogRead(switch_mode) < i_switch_mode_value) {
      return true;
    }
    else {
      return false;
    }
  }
  else {
    if(analogRead(switch_mode) > i_switch_mode_value) {
      return true;
    }
    else {
      return false;
    }
  }
}

/*
Barrel safety switch is connected to analog input.
PCB builds is pulled high.
Nano builds is pulled low.
*/
bool switchBarrel() {
  if(b_pcb == true) {
    if(analogRead(switch_barrel) < i_switch_barrel_value) {
      return true;
    }
    else {
      return false;
    }
  }
  else {
    if(analogRead(switch_barrel) > i_switch_barrel_value) {
      return true;
    }
    else {
      return false;
    }
  }
}

/*
 * Pack commuication to the wand.
 */
void checkPack() {
  if(Serial1.available() > 0) {
    rx_byte = Serial1.read();

    if(b_volume_sync_wait == true) {
        switch(VOLUME_SYNC_WAIT) {
          case EFFECTS:
            i_volume_percentage = rx_byte;
            i_volume = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_percentage / 100);

            adjustVolumeEffectsGain();
            VOLUME_SYNC_WAIT = MASTER;
          break;

          case MASTER:
            i_volume_master_percentage = rx_byte;
            i_volume_master = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);

            w_trig.masterGain(i_volume_master);
            VOLUME_SYNC_WAIT = MUSIC;
          break;

          case MUSIC:
            i_volume_music_percentage = rx_byte;
            i_volume_music = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_music_percentage / 100);

            b_volume_sync_wait = false;
            b_wait_for_pack = false;
            VOLUME_SYNC_WAIT = EFFECTS;
          break;
        }      
    }
    else {
      switch(rx_byte) {      
        case 1:
          // Pack is on.
          b_pack_on = true;
        break;
  
        case 2:
          if(b_pack_on == true) {
            // Turn wand off.
            if(WAND_STATUS != MODE_OFF) {
              WAND_ACTION_STATUS = ACTION_OFF;
            }
          }
          
          // Pack is off.
          b_pack_on = false;
        break;
  
        case 3:
          // Alarm is on.
          b_pack_alarm = true;

          if(b_pcb == true) {
            digitalWrite(led_hat_2, HIGH); // Turn on hat light 2.
            ms_hat_2.start(i_hat_2_delay); // Start the hat light 2 blinking timer.
          }
        break;
  
        case 4:
          // Alarm is off.
          b_pack_alarm = false;

          if(b_pcb == true) {
            digitalWrite(led_hat_2, LOW); // Turn off hat light 2.
          }
          ms_hat_2.stop();
        break;
  
        case 5:
          // Vibration on.
          b_vibration_on = true;
        break;
  
        case 6:
          // Vibration off.
          b_vibration_on = false;
          vibrationOff();
        break;
  
        case 7:
          // 1984 mode.
          year_mode = 1984;
          bargraphYearModeUpdate();
        break;
  
        case 8:
          // 2021 mode.
          year_mode = 2021;
          bargraphYearModeUpdate();
        break;
  
        case 9:
          // Increase overall volume.
          increaseVolume();
        break;
  
        case 10:
          // Decrease overall volume.
          decreaseVolume();
        break;
  
        case 11:
          // The pack is asking us if we are still here. Respond back.
          Serial1.write(14);
        break;
  
        case 12:
          // Repeat music track.
          b_repeat_track = true;
        break;
  
        case 13:
          // Repeat music track.
          b_repeat_track = false;
        break;

        /*
         * Not used.
         */
        case 14:

        break;
  
        case 15:
          // Put the wand into volume sync mode.
          b_volume_sync_wait = true;
          VOLUME_SYNC_WAIT = EFFECTS;
        break;
        
        case 16:
          // Vibration firing on.
          b_vibration_firing = true;
          vibrationOff();
        break;
  
        case 17:
          // Vibration firing off
          b_vibration_firing = false;
        break;

        case 18:
          // Play 2021 voice.
          w_trig.trackStop(S_VOICE_AFTERLIFE);  
          w_trig.trackStop(S_VOICE_1989);
          w_trig.trackStop(S_VOICE_1984);    
          w_trig.trackGain(S_VOICE_AFTERLIFE, i_volume);
          w_trig.trackPlayPoly(S_VOICE_AFTERLIFE);
        break;

        case 19:
          // Play 1989 voice.
          w_trig.trackStop(S_VOICE_AFTERLIFE);  
          w_trig.trackStop(S_VOICE_1989);
          w_trig.trackStop(S_VOICE_1984);    
          w_trig.trackGain(S_VOICE_1989, i_volume);
          w_trig.trackPlayPoly(S_VOICE_1989);
        break;

        case 20:
          // Play 1984 voice.
          w_trig.trackStop(S_VOICE_AFTERLIFE);  
          w_trig.trackStop(S_VOICE_1989);
          w_trig.trackStop(S_VOICE_1984);    
          w_trig.trackGain(S_VOICE_1984, i_volume);
          w_trig.trackPlayPoly(S_VOICE_1984);
        break;

        case 21:
          // Play smoke disabled voice.
          w_trig.trackStop(S_VOICE_SMOKE_DISABLED);
          w_trig.trackStop(S_VOICE_SMOKE_ENABLED);    
          w_trig.trackGain(S_VOICE_SMOKE_DISABLED, i_volume);
          w_trig.trackPlayPoly(S_VOICE_SMOKE_DISABLED);
        break;

        case 22:
          // Play smoke enabled voice.
          w_trig.trackStop(S_VOICE_SMOKE_ENABLED);
          w_trig.trackStop(S_VOICE_SMOKE_DISABLED);    
          w_trig.trackGain(S_VOICE_SMOKE_ENABLED, i_volume);
          w_trig.trackPlayPoly(S_VOICE_SMOKE_ENABLED);
        break;

        case 23:
          // Play cyclotron counter clockwise voice.
          w_trig.trackStop(S_VOICE_CYCLOTRON_CLOCKWISE);
          w_trig.trackStop(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);    
          w_trig.trackGain(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE, i_volume);
          w_trig.trackPlayPoly(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);   
        break;

        case 24:
          // Play cyclotron clockwise voice.
          w_trig.trackStop(S_VOICE_CYCLOTRON_CLOCKWISE);
          w_trig.trackStop(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);    
          w_trig.trackGain(S_VOICE_CYCLOTRON_CLOCKWISE, i_volume);
          w_trig.trackPlayPoly(S_VOICE_CYCLOTRON_CLOCKWISE);   
        break;

        case 25:
          // Play Single LED voice.
          w_trig.trackStop(S_VOICE_THREE_LED);
          w_trig.trackStop(S_VOICE_SINGLE_LED);    
          w_trig.trackGain(S_VOICE_SINGLE_LED, i_volume);
          w_trig.trackPlayPoly(S_VOICE_SINGLE_LED);
        break;

        case 26:
          // Play 3 LED voice.
          w_trig.trackStop(S_VOICE_THREE_LED);
          w_trig.trackStop(S_VOICE_SINGLE_LED);    
          w_trig.trackGain(S_VOICE_THREE_LED, i_volume);
          w_trig.trackPlayPoly(S_VOICE_THREE_LED);
        break;

        case 99:
          // Stop music
          stopMusic();
        break;
        
        default:
          // Music track number to be played.
          if(rx_byte > 99) {
            if(b_playing_music == true) {
              stopMusic();
              i_current_music_track = rx_byte;
              playMusic();
            }
            else {
              i_current_music_track = rx_byte;
            }
          }
        break;
      }
    }
  }
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
  
  char w_trig_version[VERSION_STRING_LEN]; // Firmware version.
  int w_num_tracks = w_trig.getNumTracks();

  w_trig.getVersion(w_trig_version, VERSION_STRING_LEN);

  if(b_debug == true) {
    Serial.println(w_trig_version);
  }

  // Build the music track count.
  i_music_count = w_num_tracks - i_last_effects_track;

  if(i_music_count > 0) {
    i_current_music_track = i_music_track_start; // Set the first track of music as file 100_
  }
}