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

// 3rd-Party Libraries
#include <EEPROM.h>
#include <millisDelay.h>
#include <FastLED.h>
#include <ezButton.h>
#include <Ramp.h>
#include <SerialTransfer.h>

/*
  ***** IMPORTANT *****
  * Please make sure your WAV Trigger devices are running firmware version 1.40 or higher.
  * You can download the latest directly from the GPStar github repository or from the Robertsonics website.
  https://github.com/gpstar81/haslab-proton-pack/tree/main/extras

  * Information on how to update your WAV Trigger devices can be found on the GPStar github repository.
  https://github.com/gpstar81/haslab-proton-pack/blob/main/WAVTRIGGER.md
*/
#include "wavTrigger.h"

// Local Files
#include "Configuration.h"
#include "MusicSounds.h"
#include "Communication.h"
#include "Header.h"
#include "Colours.h"

void setup() {
  Serial.begin(9600); // Standard serial console.

  Serial1.begin(9600); // Add-on serial communication.
  Serial2.begin(9600); // Communication to the Neutrona Wand.

  // Connect the serial ports.
  serial1Coms.begin(Serial1, false);
  packComs.begin(Serial2, false);

  // Setup the WAV Trigger.
  setupWavTrigger();

  // Rotary encoder for volume control.
  pinMode(encoder_pin_a, INPUT_PULLUP);
  pinMode(encoder_pin_b, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoder_pin_a), readEncoder, CHANGE);

  // Configure the various switches on the pack.
  switch_cyclotron_lid.setDebounceTime(50);
  switch_alarm.setDebounceTime(50);
  switch_mode.setDebounceTime(50);
  switch_vibration.setDebounceTime(50);
  switch_cyclotron_direction.setDebounceTime(50);
  switch_smoke.setDebounceTime(50);

  // Adjust the PWM frequency of the vibration motor.
  TCCR5B = (TCCR5B & B11111000) | (B00000100);  // for PWM frequency of 122.55 Hz

  // Vibration motor
  pinMode(vibration, OUTPUT);

  // Smoke motor for the N-Filter.
  pinMode(smoke_pin, OUTPUT);

  // Fan pin for the N-Filter smoke.
  pinMode(fan_pin, OUTPUT);

  // Second smoke motor (booster tube)
  pinMode(smoke_booster_pin, OUTPUT);

  // A fan pin that goes off at the same time as the booster tube smoke pin.
  pinMode(fan_booster_pin, OUTPUT);

  // Another optional N-Filter LED.
  pinMode(i_nfilter_led_pin, OUTPUT);

  // Power Cell and Cyclotron Lid.
  FastLED.addLeds<NEOPIXEL, PACK_LED_PIN>(pack_leds, i_max_pack_leds + i_nfilter_jewel_leds);

  // Inner Cyclotron LEDs.
  FastLED.addLeds<NEOPIXEL, CYCLOTRON_LED_PIN>(cyclotron_leds, CYCLOTRON_NUM_LEDS);
  //FastLED.setMaxPowerInVoltsAndMilliamps(5, 800);  // Limit draw to 800mA at 5v of power. Enabling this can cause some flickering of the LEDs.

  // Cyclotron Switch Panel LEDs
  pinMode(cyclotron_sw_plate_led_r1, OUTPUT);
  pinMode(cyclotron_sw_plate_led_r2, OUTPUT);
  pinMode(cyclotron_sw_plate_led_y1, OUTPUT);
  pinMode(cyclotron_sw_plate_led_y2, OUTPUT);
  pinMode(cyclotron_sw_plate_led_g1, OUTPUT);
  pinMode(cyclotron_sw_plate_led_g2, OUTPUT);
  pinMode(cyclotron_switch_led_green, OUTPUT);
  pinMode(cyclotron_switch_led_yellow, OUTPUT);

  // Misc configuration before startup.
  resetCyclotronLeds();

  // Bootup the pack into Proton mode, the same as the wand.
  FIRING_MODE = PROTON;

  // Set the CTS to not firing.
  STATUS_CTS = CTS_NOT_FIRING;
  SYSTEM_YEAR = SYSTEM_AFTERLIFE;
  SYSTEM_YEAR_TEMP = SYSTEM_AFTERLIFE;
  SYSTEM_EEPROM_YEAR = SYSTEM_TOGGLE_SWITCH;

  switch(SYSTEM_YEAR) {
    case SYSTEM_1984:
    case SYSTEM_1989:
      i_current_ramp_speed = i_1984_delay * 1.3;
      i_inner_current_ramp_speed = i_inner_ramp_delay;
    break;

    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      i_current_ramp_speed = i_2021_ramp_delay;
      i_inner_current_ramp_speed = i_inner_ramp_delay;
    break;
  }

  // Start some timers
  ms_cyclotron.start(i_current_ramp_speed);
  ms_cyclotron_ring.start(i_inner_current_ramp_speed);
  ms_cyclotron_switch_plate_leds.start(i_cyclotron_switch_plate_leds_delay);
  ms_wand_handshake.start(1);

  ms_serial1_handshake.start(int(i_serial1_handshake_delay / 2));
  ms_fast_led.start(i_fast_led_delay);

  // Configure the vibration state.
  if(switch_vibration.getState() == LOW) {
    b_vibration_enabled = true;
  }
  else {
    b_vibration_enabled = false;
  }

  // Configure the year mode.
  if(switch_mode.getState() == LOW) {
    SYSTEM_YEAR = SYSTEM_1984;
  }
  else {
    SYSTEM_YEAR = SYSTEM_AFTERLIFE;
  }

  // Check some LED brightness settings for various LEDs.
  // The datatype used should avoid checks for negative values.
  if(i_powercell_brightness > 100) {
    i_powercell_brightness = 100;
  }

  if(i_cyclotron_brightness > 100) {
    i_cyclotron_brightness = 100;
  }

  if(i_cyclotron_inner_brightness > 100) {
    i_cyclotron_inner_brightness = 100;
  }

  // Tell the Attenuator the pack is here.
  serial1Send(A_PACK_BOOTUP);

  // Tell the wand the pack is here.
  packSerialSend(P_PACK_BOOTUP);

  // Check music timer.
  ms_check_music.start(i_music_check_delay);

  // Default mode is MODE_SUPER_HERO.
  SYSTEM_MODE = MODE_SUPER_HERO;

  // Load any saved settings stored in the EEPROM memory of the Proton Pack.
  if(b_eeprom == true) {
    readEEPROM();
  }

  if(SYSTEM_MODE == MODE_SUPER_HERO) {
    packSerialSend(P_MODE_SUPER_HERO);
    serial1Send(A_MODE_SUPER_HERO);

    // Auto start the pack if it is in demo light mode.
    if(b_demo_light_mode == true) {
      // Turn the pack on.
      PACK_ACTION_STATE = ACTION_ACTIVATE;
    }
  }
  else {
    packSerialSend(P_MODE_ORIGINAL);
    serial1Send(A_MODE_ORIGINAL);
  }

  // Reset the master volume. Important to keep this as we startup the system at the lowest volume. Then the EEPROM reads any settings if required, then we reset the volume.
  w_trig.masterGain(i_volume_master);
}

void loop() {
  w_trig.update();

  wandHandShake();
  checkWand();

  serial1HandShake();
  checkSerial1();

  checkMusic();
  checkRibbonCableSwitch();
  cyclotronSwitchPlateLEDs();

  switch_cyclotron_lid.loop();
  switch_alarm.loop();
  switch_cyclotron_direction.loop();
  switch_mode.loop();
  switch_vibration.loop();
  switch_power.loop();
  switch_smoke.loop();

  checkSwitches();
  checkRotaryEncoder();

  switch (PACK_STATE) {
    case MODE_OFF:
      if(b_pack_on == true) {
        b_2021_ramp_up = false;
        b_2021_ramp_up_start = false;
        b_inner_ramp_up = false;
        b_fade_out = true;

        reset2021RampDown();

        b_pack_shutting_down = true;

        ms_fadeout.start(1);
      }

      if(b_2021_ramp_down == true && b_overheating == false && b_alarm == false) {
        // If we enter the LED EEPROM menu while the pack is ramping off, stop it right away.
        if(b_spectral_lights_on == true) {
          packOffReset();
          spectralLightsOn();
        }
        else {
          cyclotronSwitchLEDLoop();
          powercellLoop();
          cyclotronControl();
        }
      }
      else {
        if(b_spectral_lights_on != true) {
          if(ms_fadeout.justFinished()) {
            if(fadeOutLights() == true) {
              ms_fadeout.start(50);
            }
            else {
              ms_fadeout.stop();
              b_fade_out = false;
            }
          }

          packOffReset();
        }
      }

      if(b_pack_on == true) {
        switch(SYSTEM_MODE) {
          case MODE_ORIGINAL:
            if(switch_power.getState() == HIGH) {
             // Tell the Neutrona Wand that power to the Proton Pack is off.
              packSerialSend(P_MODE_ORIGINAL_RED_SWITCH_OFF);

              // Tell the Attenuator or any other device that the power to the Proton Pack is off.
              serial1Send(A_MODE_ORIGINAL_RED_SWITCH_OFF);
            }
          break;

          case MODE_SUPER_HERO:
          default:
            // Do nothing.
          break;
        }

        // Tell the wand the pack is off, so shut down the wand if it happens to still be on.
        packSerialSend(P_OFF);
        serial1Send(A_PACK_OFF);
      }

      b_pack_on = false;
    break;

    case MODE_ON:
      if(b_spectral_lights_on == true) {
        spectralLightsOff();
      }

      if(b_pack_shutting_down == true) {
        b_pack_shutting_down = false;
      }

      if(b_pack_on == false) {
        // Tell the wand the pack is on.
        packSerialSend(P_ON);
        serial1Send(A_PACK_ON);
      }

      b_pack_on = true;
      b_fade_out = false;
      ms_fadeout.stop();

      if(b_2021_ramp_down == true) {
        b_2021_ramp_down = false;
        b_2021_ramp_down_start = false;
        b_inner_ramp_down = false;

        reset2021RampUp();
      }

      if(switch_alarm.getState() == LOW && b_overheating == false) {
        if(b_alarm == true) {
          if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
            // Reset the LEDs before resetting the alarm flag.
            resetCyclotronLeds();
            ms_cyclotron.start(0);
          }
          else {
            ms_cyclotron.start(i_current_ramp_speed);
          }

          ms_cyclotron_ring.start(i_inner_current_ramp_speed);

          ventLight(false);
          ventLightLEDW(false);

          b_alarm = false;

          reset2021RampUp();

          packStartup();
        }
      }

      checkCyclotronAutoSpeed();

      // Play a little bit of smoke and N-Filter vent lights while firing and other misc sound effects.
      if(b_wand_firing == true) {
        // If firing in Meson Blast, control the pulse repeat effect.
        if(FIRING_MODE == MESON) {
          if(ms_meson_blast.justFinished()) {
            playEffect(S_MESON_FIRE_PULSE);

            switch(i_wand_power_level) {
              case 5:
                ms_meson_blast.start(i_meson_blast_delay_level_5);
              break;

              case 4:
                ms_meson_blast.start(i_meson_blast_delay_level_5);
              break;

              case 3:
                ms_meson_blast.start(i_meson_blast_delay_level_3);
              break;

              case 2:
                ms_meson_blast.start(i_meson_blast_delay_level_2);
              break;

              case 1:
              default:
                ms_meson_blast.start(i_meson_blast_delay_level_1);
              break;
            }
          }
        }

        // Mix some impact sound effects.
        if(ms_firing_sound_mix.justFinished() && FIRING_MODE == PROTON && b_stream_effects == true) {
          uint8_t i_random = 0;

          switch(i_last_firing_effect_mix) {
            case S_FIRE_SPARKS:
              i_random = random(0,2);
            break;

            case S_FIRE_SPARKS_3:
            case S_FIRE_SPARKS_4:
              i_random = 3;
            break;

            case S_FIRE_SPARKS_5:
              i_random = 2;
            break;

            case S_FIRE_SPARKS_2:
              i_random = 1;
            break;

            default:
              i_random = 3;
            break;
          }

          unsigned int i_s_random = random(2,4) * 1000;

          switch (i_random) {
            case 3:
              playEffect(S_FIRE_SPARKS, false, i_volume_effects + 5);
              i_last_firing_effect_mix = S_FIRE_SPARKS;

              ms_firing_sound_mix.start(i_s_random * 10);
            break;

            case 2:
              playEffect(S_FIRE_SPARKS_4, false, i_volume_effects + 5);
              i_last_firing_effect_mix = S_FIRE_SPARKS_4;

              ms_firing_sound_mix.start(i_s_random);
            break;

            case 1:
              playEffect(S_FIRE_SPARKS_3, false, i_volume_effects + 5);
              i_last_firing_effect_mix = S_FIRE_SPARKS_3;

              ms_firing_sound_mix.start(i_s_random);
            break;

            case 0:
              playEffect(S_FIRE_SPARKS_2, false, i_volume_effects + 5);
              playEffect(S_FIRE_SPARKS_5, false, i_volume_effects + 5);
              i_last_firing_effect_mix = S_FIRE_SPARKS_5;

              ms_firing_sound_mix.start(1800);
            break;

            default:
              playEffect(S_FIRE_SPARKS_2, false, i_volume_effects + 5);
              i_last_firing_effect_mix = S_FIRE_SPARKS_2;

              ms_firing_sound_mix.start(500);
            break;
          }
        }

        if(ms_smoke_on.justFinished()) {
          ms_smoke_on.stop();
          ms_smoke_timer.start(i_smoke_timer[i_wand_power_level - 1]);
          b_vent_sounds = true;
        }

        if(ms_smoke_timer.justFinished()) {
          if(ms_smoke_on.isRunning() != true) {
            ms_smoke_on.start(i_smoke_on_time[i_wand_power_level - 1]);
          }
        }

        if(ms_smoke_on.isRunning() == true) {
          // Turn on some smoke and play some vent sounds if smoke is enabled.
          if(b_smoke_enabled == true) {
            // Turn on some smoke.
            smokeControl(true);

            // Play some sounds with the smoke and vent lighting.
            if(b_vent_sounds == true) {
              playVentSounds();

              b_vent_sounds = false;
            }

            fanControl(true);
            fanBooster(true);
          }

          // We are strobing the N-Filter jewel.
          if(ms_vent_light_off.justFinished()) {
            ms_vent_light_off.stop();
            ms_vent_light_on.start(i_vent_light_delay);

            ventLight(true);
          }
          else if(ms_vent_light_on.justFinished()) {
            ms_vent_light_on.stop();
            ms_vent_light_off.start(i_vent_light_delay);

            ventLight(false);
          }

          // The LED-W will not strobe during this venting.
          ventLightLEDW(true);
        }
        else {
          smokeControl(false);
          ventLight(false);
          ventLightLEDW(false);
          fanControl(false);
          fanBooster(false);
        }
      }

      cyclotronSwitchLEDLoop();

      if(b_overheating == true && b_overheat_lights_off == true) {
        powercellRampDown();
      }
      else {
        powercellLoop();
      }

      cyclotronControl();
    break;
   }

  switch(PACK_ACTION_STATE) {
    case ACTION_IDLE:
      // Do nothing.
    break;

    case ACTION_OFF:
      packShutdown();
    break;

    case ACTION_ACTIVATE:
      packStartup();
    break;
  }

  // Update the LEDs
  if(ms_fast_led.justFinished()) {
    FastLED.show();

    ms_fast_led.start(i_fast_led_delay);

    if(b_powercell_updating == true) {
      b_powercell_updating = false;
    }
  }
}

bool fadeOutLights() {
  bool b_return = false;

  if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
    uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, FIRING_MODE, b_cyclotron_colour_toggle);

    // We override the colour changes when using stock HasLab Cyclotron LEDs.
    // Changing the colour space with a CHSV Object affects the brightness slightly for non RGB pixels.
    if(i_cyclotron_leds == HASLAB_CYCLOTRON_LED_COUNT && b_cyclotron_haslab_chsv_colour_change != true) {
      i_colour_scheme = C_HASLAB;
    }

    uint8_t i_cyclotron_leds_total = i_pack_num_leds - i_nfilter_jewel_leds - cyclotron_led_start;

    if(b_cyclotron_simulate_ring == true) {
      i_cyclotron_leds_total = OUTER_CYCLOTRON_LED_MAX;
    }

    for(int i = 0; i < i_cyclotron_leds_total; i++) {
      int i_curr_brightness = i_cyclotron_led_value[i] - 10;

      if(i_curr_brightness < 0) {
        i_curr_brightness = 0;
      }

      i_cyclotron_led_value[i] = i_curr_brightness;

      if(i_curr_brightness > 0) {

        b_return = true;

        if(b_cyclotron_simulate_ring == true) {
          switch(i_cyclotron_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
              // For 40-element LED ring.
              pack_leds[i_cyclotron_40led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
            break;

            case FRUTTO_CYCLOTRON_LED_COUNT:
              // For Frutto Technology 20 LEDs.
              pack_leds[i_cyclotron_20led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
            default:
              // For stock HasLab LEDs.
              pack_leds[i_cyclotron_12led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
            break;
          }
        }
        else {
          pack_leds[i + cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
        }
      }
      else {
        if(b_cyclotron_simulate_ring == true) {
          switch(i_cyclotron_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
              // For 40-element LED ring.
              pack_leds[i_cyclotron_40led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, C_BLACK);
            break;

            case FRUTTO_CYCLOTRON_LED_COUNT:
              // For Frutto Technology 20 LEDs.
              pack_leds[i_cyclotron_20led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, C_BLACK);
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
            default:
              // For stock HasLab LEDs.
              pack_leds[i_cyclotron_12led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, C_BLACK);
            break;
          }
        }
        else {
          pack_leds[i + cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, C_BLACK);
        }
      }
    }
  }

  return b_return;
}

void checkMusic() {
  if(ms_check_music.justFinished() && ms_music_next_track.isRunning() != true) {
    ms_check_music.start(i_music_check_delay);
    w_trig.trackPlayingStatus(i_current_music_track);

    // Loop through all the tracks if the music is not set to repeat a track.
    if(b_playing_music == true && b_repeat_track == false && b_music_paused != true) {
      if(w_trig.currentMusicTrackStatus(i_current_music_track) != true && ms_music_status_check.justFinished() && w_trig.trackCounterReset() != true) {
        ms_check_music.stop();
        ms_music_status_check.stop();

        stopMusic();

        // Switch to the next track.
        if(i_current_music_track + 1 > i_music_track_start + i_music_count - 1) {
          i_current_music_track = i_music_track_start;
        }
        else {
          i_current_music_track++;
        }

        // Start timer to prepare to play music again.
        ms_music_next_track.start(i_music_next_track_delay);
      }
      else {
        if(ms_music_status_check.justFinished()) {
          ms_music_status_check.start(i_music_check_delay * 4);
        }
      }
    }
  }

  // Start playing music again.
  if(ms_music_next_track.justFinished()) {
    ms_music_next_track.stop();
    ms_check_music.start(i_music_check_delay);

    // Play the appropriate track on pack and wand, and notify the serial1 device.
    playMusic();
  }
}

void checkRibbonCableSwitch() {
  if(switch_alarm.isPressed() || switch_alarm.isReleased()) {
    if(switch_alarm.getState() == LOW) {
      // Ribbon cable is on.
      packSerialSend(P_RIBBON_CABLE_ON);
    }
    else {
      packSerialSend(P_RIBBON_CABLE_OFF);
    }
  }
}

void playVentSounds() {
  playEffect(S_VENT_SMOKE);
  playEffect(S_SPARKS_LOOP);
}

void packStartup() {
  PACK_STATE = MODE_ON;
  PACK_ACTION_STATE = ACTION_IDLE;

  if(b_alarm == true) {
    if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
      ms_cyclotron.start(0);
      ms_alarm.start(0);
    }

    packAlarm();

    // Tell the wand the pack alarm is off.
    packSerialSend(P_ALARM_ON);
    serial1Send(A_ALARM_ON);
  }
  else {
    // Tell the wand the pack alarm is off.
    packSerialSend(P_ALARM_OFF);

    // Tell any add-on devices that the alarm is off.
    serial1Send(A_ALARM_OFF);

    stopEffect(S_PACK_RIBBON_ALARM_1);
    stopEffect(S_ALARM_LOOP);
    stopEffect(S_RIBBON_CABLE_START);
    stopEffect(S_PACK_SHUTDOWN_AFTERLIFE); // This is a long track which may still be playing.

    switch(SYSTEM_YEAR) {
      case SYSTEM_1984:
        playEffect(S_BOOTUP);
        playEffect(S_IDLE_LOOP, true, i_volume_effects, true, 2000);
      break;

      case SYSTEM_1989:
        playEffect(S_GB2_PACK_START);
        playEffect(S_GB2_PACK_LOOP, true, i_volume_effects, true, 3000);
      break;

      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        playEffect(S_AFTERLIFE_PACK_STARTUP);
        playEffect(S_AFTERLIFE_PACK_IDLE_LOOP, true, i_volume_effects, true, 18000);
        ms_idle_fire_fade.start(18000);
      break;
    }

    switch(FIRING_MODE) {
      case SLIME:
        playEffect(S_PACK_SLIME_TANK_LOOP, true, 0, true, 900);
      break;

      case STASIS:
        playEffect(S_STASIS_IDLE_LOOP, true, 0, true, 900);
      break;

      case MESON:
        playEffect(S_MESON_IDLE_LOOP, true, 0, true, 900);
      break;

      default:
        // Do nothing.
      break;
    }
  }
}

void packShutdown() {
  PACK_STATE = MODE_OFF;
  PACK_ACTION_STATE = ACTION_IDLE;

  // Stop the firing if the pack is doing it.
  wandStoppedFiring();

  switch(SYSTEM_YEAR) {
    case SYSTEM_1984:
    case SYSTEM_1989:
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
      stopEffect(S_PACK_RIBBON_ALARM_1);
      stopEffect(S_ALARM_LOOP);
      stopEffect(S_RIBBON_CABLE_START);
    break;

    // Not used.
    default:
      stopEffect(S_PACK_BEEPING);
    break;
  }

  wandExtraSoundsStop();

  stopEffect(S_BEEP_8);
  stopEffect(S_SHUTDOWN);
  stopEffect(S_STEAM_LOOP);

  stopEffect(S_PACK_SLIME_TANK_LOOP);
  stopEffect(S_STASIS_IDLE_LOOP);
  stopEffect(S_MESON_IDLE_LOOP);

  if(SYSTEM_YEAR == SYSTEM_1989) {
    stopEffect(S_GB2_PACK_START);
    stopEffect(S_GB2_PACK_LOOP);
    stopEffect(S_GB2_PACK_OFF);
  }

  if(SYSTEM_YEAR == SYSTEM_1984) {
    stopEffect(S_PACK_SHUTDOWN);
    stopEffect(S_IDLE_LOOP);
    stopEffect(S_BOOTUP);
  }

  if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
    stopEffect(S_PACK_SHUTDOWN_AFTERLIFE);
    stopEffect(S_AFTERLIFE_PACK_STARTUP);
    stopEffect(S_AFTERLIFE_PACK_IDLE_LOOP);
  }

  if(b_alarm != true) {
    switch(SYSTEM_YEAR) {
      case SYSTEM_1984:
        playEffect(S_SHUTDOWN);
        playEffect(S_PACK_SHUTDOWN);
      break;

      case SYSTEM_1989:
        playEffect(S_SHUTDOWN);
        playEffect(S_GB2_PACK_OFF);
      break;

      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        playEffect(S_PACK_SHUTDOWN_AFTERLIFE);
      break;
    }
  }
  else {
    playEffect(S_SHUTDOWN);
  }

  // Turn off the vent lights if they were on.
  ventLight(false);
  ventLightLEDW(false);
  ms_vent_light_off.stop();
  ms_vent_light_on.stop();

  // Turn off any smoke.
  smokeControl(false);
  ms_smoke_timer.stop();
  ms_smoke_on.stop();

  // Turn off the fans.
  fanControl(false);
  fanBooster(false);

  // Turn off the Cyclotron auto speed timer.
  ms_cyclotron_auto_speed_timer.stop();

  // Reset vent sounds flag.
  b_vent_sounds = true;
}

void packOffReset() {
  powercellOff();
  cyclotronSwitchLEDOff();

  // Reset the Power Cell timer.
  ms_powercell.stop();
  ms_powercell.start(i_powercell_delay);

  // Reset the Cyclotron LED switch timer.
  ms_cyclotron_switch_led.stop();
  ms_cyclotron_switch_led.start(i_cyclotron_switch_led_delay);

  // Need to reset the Cyclotron timers.
  ms_cyclotron.start(i_2021_delay);
  ms_cyclotron_ring.start(i_inner_ramp_delay);

  ms_overheating_length.stop();
  b_overheating = false;
  b_2021_ramp_down = false;
  b_2021_ramp_down_start = false;
  b_reset_start_led = true; // reset the start LED of the Cyclotron.
  b_inner_ramp_down = false;

  resetCyclotronLeds();
  reset2021RampUp();

  // Update Cyclotron LED timer delay and optional Cyclotron LED switch plate LED timer delays.
  switch(SYSTEM_YEAR) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      i_powercell_delay = i_powercell_delay_2021;
      i_cyclotron_switch_led_delay = i_cyclotron_switch_led_delay_base;
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      i_powercell_delay = i_powercell_delay_1984;
      i_cyclotron_switch_led_delay = i_cyclotron_switch_led_delay_base * 4;
    break;
  }

  // Vibration motor off.
  vibrationPack(0);
  i_vibration_level = 0;

  if(b_pack_shutting_down == true) {
    b_pack_shutting_down = false;

    clearCyclotronFades();
  }

  // Tell the wand and any add-on devices that the alarm is off.
  if(b_alarm == true) {
    b_alarm = false;
    // Tell the wand that the alarm is off.
    packSerialSend(P_ALARM_OFF);

    // Tell any add-on devices that the alarm is off.
    serial1Send(A_ALARM_OFF);
  }
}

void checkSwitches() {
  // Cyclotron direction toggle switch.
  if(switch_cyclotron_direction.isPressed() || switch_cyclotron_direction.isReleased()) {
    if(b_clockwise == true) {
      b_clockwise = false;

      stopEffect(S_BEEPS_ALT);

      playEffect(S_BEEPS_ALT);

      stopEffect(S_VOICE_CYCLOTRON_CLOCKWISE);
      stopEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);

      playEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);

      // Tell wand to play Cyclotron counter clockwise voice.
      packSerialSend(P_CYCLOTRON_COUNTER_CLOCKWISE);
    }
    else {
      b_clockwise = true;

      stopEffect(S_BEEPS);

      playEffect(S_BEEPS);

      stopEffect(S_VOICE_CYCLOTRON_CLOCKWISE);
      stopEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);

      playEffect(S_VOICE_CYCLOTRON_CLOCKWISE);

      // Tell wand to play Cyclotron clockwise voice.
      packSerialSend(P_CYCLOTRON_CLOCKWISE);
    }
  }

  // Smoke
  if(switch_smoke.isPressed() || switch_smoke.isReleased()) {
    if(b_smoke_enabled == true) {
      b_smoke_enabled = false;

      stopEffect(S_VENT_DRY);

      playEffect(S_VENT_DRY);

      stopEffect(S_VOICE_SMOKE_DISABLED);
      stopEffect(S_VOICE_SMOKE_ENABLED);

      playEffect(S_VOICE_SMOKE_DISABLED);

      // Tell wand to play smoke disabled voice.
      packSerialSend(P_SMOKE_DISABLED);
    }
    else {
      b_smoke_enabled = true;

      stopEffect(S_VENT_SMOKE);

      playEffect(S_VENT_SMOKE);

      stopEffect(S_VOICE_SMOKE_ENABLED);
      stopEffect(S_VOICE_SMOKE_DISABLED);

      playEffect(S_VOICE_SMOKE_ENABLED);

      // Tell wand to play smoke enabled voice.
      packSerialSend(P_SMOKE_ENABLED);
    }
  }

  // Vibration toggle switch.
  if(switch_vibration.isPressed() || switch_vibration.isReleased()) {
      stopEffect(S_BEEPS_ALT);

      playEffect(S_BEEPS_ALT);

      if(switch_vibration.getState() == LOW) {
        if(b_vibration_enabled == false) {
          // Tell the wand to enable vibration.
          packSerialSend(P_VIBRATION_ENABLED);

          b_vibration_enabled = true;

          stopEffect(S_VOICE_VIBRATION_ENABLED);
          stopEffect(S_VOICE_VIBRATION_DISABLED);

          playEffect(S_VOICE_VIBRATION_ENABLED);
        }
      }
      else {
        if(b_vibration_enabled == true) {
          // Tell the wand to disable vibration.
          packSerialSend(P_VIBRATION_DISABLED);

          b_vibration_enabled = false;

          stopEffect(S_VOICE_VIBRATION_DISABLED);
          stopEffect(S_VOICE_VIBRATION_ENABLED);

          playEffect(S_VOICE_VIBRATION_DISABLED);
        }
      }
  }

  // Play a sound when the year mode switch is pressed or released.
  if(switch_mode.isPressed() || switch_mode.isReleased()) {
    stopEffect(S_BEEPS_BARGRAPH);
    playEffect(S_BEEPS_BARGRAPH);

    // Turn off the year mode override flag controlled by the Neutrona Wand.
    b_switch_mode_override = false;
  }

  switch(PACK_STATE) {
    case MODE_OFF:
      switch(SYSTEM_MODE) {
        case MODE_ORIGINAL:
          if(switch_power.isPressed() || switch_power.isReleased()) {
            if(switch_power.getState() == LOW) {
              // Tell the Neutrona Wand that power to the Proton Pack is on.
              packSerialSend(P_MODE_ORIGINAL_RED_SWITCH_ON);

              // Tell the Attenuator or any other device that the power to the Proton Pack is on.
              serial1Send(A_MODE_ORIGINAL_RED_SWITCH_ON);
            }
            else {
              // Tell the Neutrona Wand that power to the Proton Pack is off.
              packSerialSend(P_MODE_ORIGINAL_RED_SWITCH_OFF);

              // Tell the Attenuator or any other device that the power to the Proton Pack is off.
              serial1Send(A_MODE_ORIGINAL_RED_SWITCH_OFF);
            }
          }
        break;

        case MODE_SUPER_HERO:
        default:
          if(switch_power.isPressed() || switch_power.isReleased()) {
            // Turn the pack on.
            PACK_ACTION_STATE = ACTION_ACTIVATE;
          }
        break;
      }

      // Year mode. Best to adjust it only when the pack is off.
      if(b_2021_ramp_down != true && b_pack_on == false) {
        // If switching manually by the pack toggle switch.
        if(b_switch_mode_override != true) {
          if(switch_mode.getState() == LOW) {
            if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
              // Tell the wand to switch to 1984 mode.
              packSerialSend(P_YEAR_1984);
            }

            SYSTEM_YEAR = SYSTEM_1984;
            SYSTEM_YEAR_TEMP = SYSTEM_1984;

            serial1Send(A_YEAR_1984);
          }
          else {
            if(SYSTEM_YEAR == SYSTEM_1984) {
              // Tell the wand to switch to Afterlife mode.
              packSerialSend(P_YEAR_AFTERLIFE);
            }

            SYSTEM_YEAR = SYSTEM_AFTERLIFE;
            SYSTEM_YEAR_TEMP = SYSTEM_AFTERLIFE;

            serial1Send(A_YEAR_AFTERLIFE);
          }
        }
        else {
          // If the Neutrona Wand sub menu setting told the Proton Pack to change years.
          switch(SYSTEM_YEAR_TEMP) {
            case SYSTEM_1984:
              if(SYSTEM_YEAR != SYSTEM_YEAR_TEMP) {
                // Tell the wand to switch to 1984 mode.
                packSerialSend(P_YEAR_1984);
              }

              SYSTEM_YEAR = SYSTEM_1984;
              SYSTEM_YEAR_TEMP = SYSTEM_1984;

              serial1Send(A_YEAR_1984);
            break;

            case SYSTEM_1989:
              if(SYSTEM_YEAR != SYSTEM_YEAR_TEMP) {
                // Tell the wand to switch to 1989 mode.
                packSerialSend(P_YEAR_1989);
              }

              SYSTEM_YEAR = SYSTEM_1989;
              SYSTEM_YEAR_TEMP = SYSTEM_1989;

              serial1Send(A_YEAR_1989);
            break;

            /*
            case SYSTEM_FROZEN_EMPIRE:
            default:
              if(SYSTEM_YEAR != SYSTEM_YEAR_TEMP) {
                // Tell the wand to switch to Frozen Empire mode.
                packSerialSend(P_YEAR_FROZEN_EMPIRE);
              }

              SYSTEM_YEAR = SYSTEM_YEAR_FROZEN_EMPIRE;
              SYSTEM_YEAR_TEMP = SYSTEM_YEAR_FROZEN_EMPIRE;

              serial1Send(A_YEAR_FROZEN_EMPIRE);
            break;
            */

            case SYSTEM_AFTERLIFE:
            default:
              if(SYSTEM_YEAR != SYSTEM_YEAR_TEMP) {
                // Tell the wand to switch to Afterlife mode.
                packSerialSend(P_YEAR_AFTERLIFE);
              }

              SYSTEM_YEAR = SYSTEM_AFTERLIFE;
              SYSTEM_YEAR_TEMP = SYSTEM_AFTERLIFE;

              serial1Send(A_YEAR_AFTERLIFE);
            break;
          }
        }

        // Reset the ramp speeds.
        switch(SYSTEM_YEAR) {
          case SYSTEM_1984:
          case SYSTEM_1989:
              // Reset the ramp speeds.
              i_current_ramp_speed = i_1984_delay * 1.3;
              i_inner_current_ramp_speed = i_inner_ramp_delay;
          break;

          case SYSTEM_AFTERLIFE:
          case SYSTEM_FROZEN_EMPIRE:
          default:
            // Reset the ramp speeds.
            i_current_ramp_speed = i_2021_ramp_delay;
            i_inner_current_ramp_speed = i_inner_ramp_delay;
          break;
        }
      }
    break;

    case MODE_ON:
      if(switch_power.isReleased() || switch_power.isPressed()) {
        // Turn the pack off.
        PACK_ACTION_STATE = ACTION_OFF;
      }
    break;
  }
}

void cyclotronSwitchLEDOff() {
  digitalWrite(cyclotron_sw_plate_led_r1, LOW);
  digitalWrite(cyclotron_sw_plate_led_r2, LOW);

  digitalWrite(cyclotron_sw_plate_led_y1, LOW);
  digitalWrite(cyclotron_sw_plate_led_y2, LOW);

  digitalWrite(cyclotron_sw_plate_led_g1, LOW);
  digitalWrite(cyclotron_sw_plate_led_g2, LOW);

  i_cyclotron_sw_led = 0;
}

void cyclotronSwitchLEDLoop() {
  if(ms_cyclotron_switch_led.justFinished()) {
    if(b_cyclotron_lid_on != true) {
      if(b_alarm == true) {
        if(i_cyclotron_sw_led > 0) {
          digitalWrite(cyclotron_sw_plate_led_r1, HIGH);
          digitalWrite(cyclotron_sw_plate_led_r2, HIGH);

          digitalWrite(cyclotron_sw_plate_led_y1, HIGH);
          digitalWrite(cyclotron_sw_plate_led_y2, HIGH);

          digitalWrite(cyclotron_sw_plate_led_g1, HIGH);
          digitalWrite(cyclotron_sw_plate_led_g2, HIGH);

          i_cyclotron_sw_led = 0;
        }
        else {
          digitalWrite(cyclotron_sw_plate_led_r1, LOW);
          digitalWrite(cyclotron_sw_plate_led_r2, LOW);

          digitalWrite(cyclotron_sw_plate_led_y1, LOW);
          digitalWrite(cyclotron_sw_plate_led_y2, LOW);

          digitalWrite(cyclotron_sw_plate_led_g1, LOW);
          digitalWrite(cyclotron_sw_plate_led_g2, LOW);

          i_cyclotron_sw_led++;
        }
      }
      else {
        switch(i_cyclotron_sw_led) {
          case 0:
            digitalWrite(cyclotron_sw_plate_led_r1, LOW);
            digitalWrite(cyclotron_sw_plate_led_r2, LOW);

            digitalWrite(cyclotron_sw_plate_led_y1, LOW);
            digitalWrite(cyclotron_sw_plate_led_y2, LOW);

            digitalWrite(cyclotron_sw_plate_led_g1, LOW);
            digitalWrite(cyclotron_sw_plate_led_g2, LOW);

            i_cyclotron_sw_led++;
          break;

          case 1:
            digitalWrite(cyclotron_sw_plate_led_r1, LOW);
            digitalWrite(cyclotron_sw_plate_led_r2, LOW);

            digitalWrite(cyclotron_sw_plate_led_y1, LOW);
            digitalWrite(cyclotron_sw_plate_led_y2, LOW);

            digitalWrite(cyclotron_sw_plate_led_g1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_g2, HIGH);

            i_cyclotron_sw_led++;
          break;

          case 2:
            digitalWrite(cyclotron_sw_plate_led_r1, LOW);
            digitalWrite(cyclotron_sw_plate_led_r2, LOW);

            digitalWrite(cyclotron_sw_plate_led_y1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_y2, HIGH);

            digitalWrite(cyclotron_sw_plate_led_g1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_g2, HIGH);

            i_cyclotron_sw_led++;
          break;

          case 3:
            digitalWrite(cyclotron_sw_plate_led_r1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_r2, HIGH);

            digitalWrite(cyclotron_sw_plate_led_y1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_y2, HIGH);

            digitalWrite(cyclotron_sw_plate_led_g1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_g2, HIGH);

            i_cyclotron_sw_led++;
          break;

          case 4:
            digitalWrite(cyclotron_sw_plate_led_r1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_r2, HIGH);

            digitalWrite(cyclotron_sw_plate_led_y1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_y2, HIGH);

            digitalWrite(cyclotron_sw_plate_led_g1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_g2, HIGH);

            i_cyclotron_sw_led++;
          break;

          case 5:
            digitalWrite(cyclotron_sw_plate_led_r1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_r2, HIGH);

            digitalWrite(cyclotron_sw_plate_led_y1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_y2, HIGH);

            digitalWrite(cyclotron_sw_plate_led_g1, LOW);
            digitalWrite(cyclotron_sw_plate_led_g2, LOW);

            i_cyclotron_sw_led++;
          break;

          case 6:
            digitalWrite(cyclotron_sw_plate_led_r1, HIGH);
            digitalWrite(cyclotron_sw_plate_led_r2, HIGH);

            digitalWrite(cyclotron_sw_plate_led_y1, LOW);
            digitalWrite(cyclotron_sw_plate_led_y2, LOW);

            digitalWrite(cyclotron_sw_plate_led_g1, LOW);
            digitalWrite(cyclotron_sw_plate_led_g2, LOW);

            i_cyclotron_sw_led++;
          break;

          case 7:
            digitalWrite(cyclotron_sw_plate_led_r1, LOW);
            digitalWrite(cyclotron_sw_plate_led_r2, LOW);

            digitalWrite(cyclotron_sw_plate_led_y1, LOW);
            digitalWrite(cyclotron_sw_plate_led_y2, LOW);

            digitalWrite(cyclotron_sw_plate_led_g1, LOW);
            digitalWrite(cyclotron_sw_plate_led_g2, LOW);

            i_cyclotron_sw_led = 0;
          break;
        }
      }
    }
    else {
      // No need to have the Inner Cyclotron switch plate LEDs on when the lid is on.
     cyclotronSwitchLEDOff();
    }

    // Setup the delays again.
    int i_cyc_led_delay = i_cyclotron_switch_led_delay / i_cyclotron_switch_led_mulitplier;

    switch(SYSTEM_YEAR) {
      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        if(b_2021_ramp_up == true) {
          i_cyc_led_delay = i_cyclotron_switch_led_delay + (i_2021_ramp_delay - r_2021_ramp.update());
        }
        else if(b_2021_ramp_down == true) {
          i_cyc_led_delay = i_cyclotron_switch_led_delay + r_2021_ramp.update();
        }
      break;

      case SYSTEM_1984:
      case SYSTEM_1989:
        if(b_2021_ramp_up == true) {
          i_cyc_led_delay = i_cyclotron_switch_led_delay + (r_2021_ramp.update() - i_1984_delay);
        }
        else if(b_2021_ramp_down == true) {
          i_cyc_led_delay = i_cyclotron_switch_led_delay / 6 + r_2021_ramp.update();
        }
      break;
    }

    if(b_alarm == true) {
      i_cyc_led_delay = i_cyclotron_switch_led_delay * 2;
    }

    ms_cyclotron_switch_led.start(i_cyc_led_delay);
  }
}

void powercellRampDown() {
 if(ms_powercell.justFinished()) {
    int i_extra_delay = 0;

    // Power Cell
    if(i_powercell_led < 0) {
      // Do Nothing.
    }
    else {
      pack_leds[i_powercell_led] = getHueAsRGB(POWERCELL, C_BLACK);

      i_powercell_led--;
    }

    // Setup the delays again.
    int i_pc_delay = i_powercell_delay;

    switch(SYSTEM_YEAR) {
      case SYSTEM_1984:
      case SYSTEM_1989:
        if(b_2021_ramp_up == true) {
          i_pc_delay = i_powercell_delay + (r_2021_ramp.update() - i_1984_delay);
        }
        else if(b_2021_ramp_down == true) {
          i_pc_delay = i_powercell_delay + (r_2021_ramp.update() - i_1984_delay);
        }
      break;

      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        if(b_2021_ramp_up == true) {
          i_pc_delay = i_powercell_delay + r_2021_ramp.update();
        }
        else if(b_2021_ramp_down == true) {
          i_pc_delay = i_powercell_delay + r_2021_ramp.update();
        }
      break;
    }

    if(b_alarm == true) {
      i_pc_delay = i_powercell_delay * 3;
    }

    ms_powercell.start(i_pc_delay + i_extra_delay);
  }
}

void powercellLoop() {
  if(ms_powercell.justFinished()) {
    int i_extra_delay = 0;

    // Power Cell
    if(i_powercell_led >= i_powercell_leds) {
      powercellOff();

      i_powercell_led = 0;
    }
    else {
      if(b_powercell_updating != true) {
        powercellDraw(i_powercell_led); // Update starting at a specific LED.

        // Add a small delay to pause the Power Cell when all Power Cell LEDs are lit up, to match the 2021 pack.
        if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) && b_alarm != true && i_powercell_led == cyclotron_led_start - 1) {
          i_extra_delay = 250;
        }

        i_powercell_led++;
      }
    }

    // Setup the delays again.
    int i_pc_delay = i_powercell_delay;

    switch(SYSTEM_YEAR) {
      case SYSTEM_1984:
      case SYSTEM_1989:
        if(b_2021_ramp_up == true) {
          i_pc_delay = i_powercell_delay + (r_2021_ramp.update() - i_1984_delay);
        }
        else if(b_2021_ramp_down == true) {
          i_pc_delay = i_powercell_delay + (r_2021_ramp.update() - i_1984_delay);
        }
      break;

      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        if(b_2021_ramp_up == true) {
          i_pc_delay = i_powercell_delay + r_2021_ramp.update();
        }
        else if(b_2021_ramp_down == true) {
          i_pc_delay = i_powercell_delay + r_2021_ramp.update();
        }
      break;
    }

    if(b_alarm == true) {
      i_pc_delay = i_powercell_delay * 5;
    }

    // Speed up the Power Cell when the cyclotron speeds up before a overheat.
    unsigned int i_multiplier = 0;

    if(i_powercell_multiplier > 1) {
      switch(i_powercell_multiplier) {
        case 2:
          if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
            i_multiplier = 5;
          }
          else {
            i_multiplier = 10;
          }
        break;

        case 3:
          if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
            i_multiplier = 10;
          }
          else {
            i_multiplier = 20;
          }
        break;

        case 4:
          if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
            i_multiplier = 15;
          }
          else {
            i_multiplier = 30;
          }
        break;

        case 5:
          if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
            i_multiplier = 25;
          }
          else {
            i_multiplier = 40;
          }
        break;

        case 6:
          if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
            i_multiplier = 30;
          }
          else {
            i_multiplier = 50;
          }
        break;
      }
    }

    ms_powercell.start((i_pc_delay + i_extra_delay) - i_multiplier);
  }
}

void powercellOn() {
  i_powercell_led = cyclotron_led_start - 1;

  powercellDraw();
}

void powercellOff() {
  for(int i = 0; i <= cyclotron_led_start - 1; i++) {
    pack_leds[i] = getHueAsRGB(POWERCELL, C_BLACK);
  }

  i_powercell_led = 0;
}

void spectralLightsOff() {
  b_spectral_lights_on = false;

  for(int i = 0; i <= i_max_pack_leds - 1; i++) {
    pack_leds[i] = getHueAsRGB(POWERCELL, C_BLACK);
  }

  for(int i = 0; i < i_max_inner_cyclotron_leds; i++) {
    if(b_grb_cyclotron == true) {
      cyclotron_leds[i] = getHueAsGRB(CYCLOTRON_INNER, C_BLACK);
    }
    else {
      cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
    }
  }
}

void spectralLightsOn() {
  b_spectral_lights_on = true;

  uint8_t i_colour_scheme = getDeviceColour(POWERCELL, SPECTRAL_CUSTOM, true);
  for(int i = 0; i <= cyclotron_led_start - 1; i++) {
    pack_leds[i] = getHueAsRGB(POWERCELL, i_colour_scheme);
  }

  uint8_t i_max = i_pack_num_leds - i_nfilter_jewel_leds - cyclotron_led_start;
  i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, SPECTRAL_CUSTOM, true);
  for(int i = 0; i < i_max; i++) {
    pack_leds[i + cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme);
  }

  i_colour_scheme = getDeviceColour(CYCLOTRON_INNER, SPECTRAL_CUSTOM, true);
  for(int i = 0; i < i_inner_cyclotron_num_leds; i++) {
    if(b_grb_cyclotron == true) {
      cyclotron_leds[i] = getHueAsGRB(CYCLOTRON_INNER, i_colour_scheme);
    }
    else {
      cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_INNER, i_colour_scheme);
    }
  }

  serial1Send(A_SPECTRAL_COLOUR_DATA);
}

void powercellDraw(uint8_t i_start) {
  uint8_t i_brightness = getBrightness(i_powercell_brightness); // Calculate desired brightness.
  uint8_t i_colour_scheme = getDeviceColour(POWERCELL, FIRING_MODE, b_powercell_colour_toggle);

  // Sets the colour for each Power Cell LED, subject to colour toggle setting.
  for(uint8_t i = i_start; i <= i_powercell_led; i++) {
    if(i_powercell_led < i_powercell_leds) {
      // Note: Always assumed to be RGB for built-in or Frutto LEDs.
      pack_leds[i] = getHueAsRGB(POWERCELL, i_colour_scheme, i_brightness);
    }
  }
}

// Reset the Cyclotron LED colours.
void cyclotronColourReset() {
  uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, FIRING_MODE, b_cyclotron_colour_toggle);

  // We override the colour changes when using stock HasLab Cyclotron LEDs, returning full white.
  // Changing the colour space with a CHSV Object affects the brightness slightly for non RGB pixels.
  if(i_cyclotron_leds == HASLAB_CYCLOTRON_LED_COUNT && b_cyclotron_haslab_chsv_colour_change != true) {
    i_colour_scheme = C_HASLAB;
  }

  // Accounts for a total # of LEDs minus the N-Filter jewel and whatever precedes the Cyclotron.
  uint8_t i_max = i_pack_num_leds - i_nfilter_jewel_leds - cyclotron_led_start;
  for(int i = 0; i < i_max; i++) {
    if(i_cyclotron_led_on_status[i] == true) {
      // Note: Always assumed to be RGB for built-in or Frutto LEDs.
      // Sets 0-index <i> plus the position of the first Cyclotron LED.
      pack_leds[i + cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_cyclotron_led_value[i]);
    }
  }
}

void cyclotronControl() {
  // Only reset the starting LED when the pack is first started up.
  if(b_reset_start_led == true) {
    b_reset_start_led = false;
    if(b_clockwise == false) {
      if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
        i_led_cyclotron = cyclotron_led_start + 2; // Start on LED #2 in counter-clockwise mode in 2021 mode.
      }
      else {
        i_1984_counter = 1;
        i_led_cyclotron = cyclotron_led_start + i_1984_cyclotron_leds[i_1984_counter];
      }
    }
    else {
      if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
        i_1984_counter = 3;
        i_led_cyclotron = cyclotron_led_start + i_1984_cyclotron_leds[i_1984_counter];

      }
      else {
        i_led_cyclotron = cyclotron_led_start;
      }
    }
  }

  if(switch_alarm.getState() == HIGH && PACK_STATE != MODE_OFF && b_2021_ramp_down_start != true && b_overheating == false) {
    if(b_alarm == false) {
      stopEffect(S_BEEP_8);

      b_2021_ramp_up = false;
      b_inner_ramp_up = false;
      b_alarm = true;

      if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
        resetCyclotronLeds();
        ms_cyclotron.start(0);
        ms_alarm.start(0);
      }
      else {
        ms_alarm.start(i_alarm_delay);
      }

      packAlarm();

      // Tell the wand the pack alarm is on.
      packSerialSend(P_ALARM_ON);

      // Tell any add-on devices that the alarm is on.
      serial1Send(A_ALARM_ON);
    }

    // Ribbon cable has been removed.
    cyclotronNoCable();
  }
  else if(b_overheating == true) {
    if(b_alarm == false) {
      stopEffect(S_BEEP_8);

      b_2021_ramp_up = false;
      b_inner_ramp_up = false;

      if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
        resetCyclotronLeds();
        clearCyclotronFades();

        ms_cyclotron.start(0);
        ms_alarm.start(0);
      }
      else {
        ms_alarm.start(i_alarm_delay);
      }

      if(b_overheat_lights_off == true) {
        powercellOn();
      }

      b_alarm = true;

      packAlarm();
    }

    cyclotronOverHeating();
  }
  else {
    if(b_2021_ramp_up_start == true) {
      b_2021_ramp_up_start = false;

      if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
        r_2021_ramp.go(i_current_ramp_speed); // Reset the ramp.
        r_2021_ramp.go(i_1984_delay, i_1984_ramp_length, CIRCULAR_OUT);

        r_inner_ramp.go(i_inner_current_ramp_speed); // Inner Cyclotron ramp reset.
        r_inner_ramp.go(i_1984_inner_delay, i_1984_ramp_length, CIRCULAR_OUT);
      }
      else {
        r_2021_ramp.go(i_current_ramp_speed); // Reset the ramp.
        r_2021_ramp.go(i_2021_delay, i_2021_ramp_length, QUARTIC_OUT);
        r_inner_ramp.go(i_inner_current_ramp_speed);
        r_inner_ramp.go(i_2021_inner_delay, i_2021_ramp_length, QUARTIC_OUT);
      }
    }
    else if(b_2021_ramp_down_start == true) {
      b_2021_ramp_down_start = false;

      r_2021_ramp.go(i_current_ramp_speed); // Reset the ramp.
      r_inner_ramp.go(i_inner_current_ramp_speed); // Reset the Inner Cyclotron ramp.

      if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
        r_2021_ramp.go(i_1984_delay * 1.3, i_1984_ramp_down_length, CIRCULAR_IN);

        r_inner_ramp.go(i_inner_ramp_delay, i_1984_ramp_down_length, CIRCULAR_IN);
      }
      else {
        r_2021_ramp.go(i_2021_ramp_delay, i_2021_ramp_down_length, QUARTIC_IN);
        r_inner_ramp.go(i_inner_ramp_delay, i_2021_ramp_down_length, QUARTIC_IN);
      }
    }

    if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
      cyclotron1984(i_current_ramp_speed);
      innerCyclotronRing(i_inner_current_ramp_speed);
    }
    else {
      cyclotron2021(i_current_ramp_speed);
      innerCyclotronRing(i_inner_current_ramp_speed);
    }
  }

  cyclotronFade();
}

void cyclotronFade() {
  uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, FIRING_MODE, b_cyclotron_colour_toggle);

  // We override the colour changes when using stock HasLab Cyclotron LEDs.
  // Changing the colour space with a CHSV Object affects the brightness slightly for non RGB pixels.
  if(i_cyclotron_leds == HASLAB_CYCLOTRON_LED_COUNT && b_cyclotron_haslab_chsv_colour_change != true) {
    i_colour_scheme = C_HASLAB;
  }

  uint8_t i_cyclotron_leds_total = i_pack_num_leds - i_nfilter_jewel_leds - cyclotron_led_start;

  if(b_cyclotron_simulate_ring == true) {
    i_cyclotron_leds_total = OUTER_CYCLOTRON_LED_MAX;
  }

  switch (SYSTEM_YEAR) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      for(int i = 0; i < i_cyclotron_leds_total; i++) {
        if(ms_cyclotron_led_fade_in[i].isRunning()) {
          i_cyclotron_led_on_status[i] = true;

          int i_curr_brightness = ms_cyclotron_led_fade_in[i].update();
          i_cyclotron_led_value[i] = i_curr_brightness;

          if(b_cyclotron_simulate_ring == true) {
            switch(i_cyclotron_leds) {
              case OUTER_CYCLOTRON_LED_MAX:
                // For 40-element LED ring.
                pack_leds[i_cyclotron_40led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
              break;

              case FRUTTO_CYCLOTRON_LED_COUNT:
                // For Frutto Technology 20 LEDs.
                pack_leds[i_cyclotron_20led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
              break;

              case HASLAB_CYCLOTRON_LED_COUNT:
              default:
                  pack_leds[i_cyclotron_12led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
              break;
            }
          }
          else {
            pack_leds[i + cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
          }
        }

        int i_new_brightness = getBrightness(i_cyclotron_brightness);
        if(ms_cyclotron_led_fade_in[i].isFinished() && i_cyclotron_led_value[i] > (i_new_brightness - 1) && i_cyclotron_led_on_status[i] == true) {
          i_cyclotron_led_value[i] = i_new_brightness;
          i_cyclotron_led_on_status[i] = false;

          ms_cyclotron_led_fade_out[i].go(i_new_brightness);

          switch(i_cyclotron_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
            case FRUTTO_CYCLOTRON_LED_COUNT:
              ms_cyclotron_led_fade_out[i].go(0, i_current_ramp_speed * 3, CIRCULAR_OUT);
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
            default:
              ms_cyclotron_led_fade_out[i].go(0, i_current_ramp_speed * 2, CIRCULAR_OUT);
            break;
          }

          if(b_cyclotron_simulate_ring == true) {
            switch(i_cyclotron_leds) {
              case OUTER_CYCLOTRON_LED_MAX:
                // For 40-element LED ring.
                pack_leds[i_cyclotron_40led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_new_brightness);
              break;

              case FRUTTO_CYCLOTRON_LED_COUNT:
                // For Frutto Technology 20 LEDs.
                pack_leds[i_cyclotron_20led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_new_brightness);
              break;

              case HASLAB_CYCLOTRON_LED_COUNT:
              default:
                // For stock HasLab LEDs.
                pack_leds[i_cyclotron_12led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_new_brightness);
              break;
            }
          }
          else {
            pack_leds[i + cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_new_brightness);
          }
        }

        if(ms_cyclotron_led_fade_out[i].isRunning() && i_cyclotron_led_on_status[i] == false) {
          int i_curr_brightness = ms_cyclotron_led_fade_out[i].update();
          i_cyclotron_led_value[i] = i_curr_brightness;

          if(b_cyclotron_simulate_ring == true) {
            switch(i_cyclotron_leds) {
              case OUTER_CYCLOTRON_LED_MAX:
                // For 40-element LED ring.
                pack_leds[i_cyclotron_40led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
              break;

              case FRUTTO_CYCLOTRON_LED_COUNT:
                // For Frutto Technology 20 LEDs.
                pack_leds[i_cyclotron_20led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
              break;

              case HASLAB_CYCLOTRON_LED_COUNT:
              default:
                // For stock HasLab LEDs.
                pack_leds[i_cyclotron_12led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
              break;
            }
          }
          else {
            pack_leds[i + cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
          }
        }

        if(ms_cyclotron_led_fade_out[i].isFinished() && i_cyclotron_led_on_status[i] == false) {
          i_cyclotron_led_value[i] = 0;
          i_cyclotron_led_on_status[i] = true;

          if(b_cyclotron_simulate_ring == true) {
            switch(i_cyclotron_leds) {
              case OUTER_CYCLOTRON_LED_MAX:
                // For 40-element LED ring.
                pack_leds[i_cyclotron_40led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
              break;

              case FRUTTO_CYCLOTRON_LED_COUNT:
                // For Frutto Technology 20 LEDs.
                pack_leds[i_cyclotron_20led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
              break;

              case HASLAB_CYCLOTRON_LED_COUNT:
              default:
                // For stock HasLab LEDs.
                pack_leds[i_cyclotron_12led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
              break;
            }
          }
          else {
            pack_leds[i + cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
          }
        }
      }
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      if(b_fade_cyclotron_led == true) {
        for(int i = 0; i < i_pack_num_leds - i_nfilter_jewel_leds - cyclotron_led_start; i++) {
          if(ms_cyclotron_led_fade_in[i].isRunning()) {
            i_cyclotron_led_on_status[i] = true;
            int i_curr_brightness = ms_cyclotron_led_fade_in[i].update();

            pack_leds[i + cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);

            i_cyclotron_led_value[i] = i_curr_brightness;
          }

          int i_new_brightness = getBrightness(i_cyclotron_brightness);
          if(ms_cyclotron_led_fade_in[i].isFinished() && i_cyclotron_led_value[i] > (i_new_brightness - 1) && i_cyclotron_led_on_status[i] == true) {
            pack_leds[i + cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_new_brightness);

            i_cyclotron_led_value[i] = i_new_brightness;
          }

          if(ms_cyclotron_led_fade_out[i].isRunning()) {
            int i_curr_brightness = ms_cyclotron_led_fade_out[i].update();

            if(i_curr_brightness < 30) {
              ms_cyclotron_led_fade_out[i].go(0);

              pack_leds[i + cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);

              i_cyclotron_led_value[i] = 0;
              i_cyclotron_led_on_status[i] = true;
            }
            else {
              pack_leds[i + cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);

              i_cyclotron_led_value[i] = i_curr_brightness;
              i_cyclotron_led_on_status[i] = false;
            }
          }

          if(ms_cyclotron_led_fade_out[i].isFinished() && i_cyclotron_led_on_status[i] == false) {
            pack_leds[i + cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);

            i_cyclotron_led_value[i] = 0;
            i_cyclotron_led_on_status[i] = true;
          }
        }
      }
    break;
  }
}

void cyclotron2021(int cDelay) {
  uint8_t i_brightness = getBrightness(i_cyclotron_brightness); // Calculate desired brightness.

  if(ms_cyclotron.justFinished()) {
    uint8_t i_cyclotron_matrix_led = 0;

    switch(i_cyclotron_leds) {
      case OUTER_CYCLOTRON_LED_MAX:
         i_cyclotron_matrix_led = i_cyclotron_40led_matrix[i_led_cyclotron - cyclotron_led_start];
      break;

      case FRUTTO_CYCLOTRON_LED_COUNT:
        i_cyclotron_matrix_led = i_cyclotron_20led_matrix[i_led_cyclotron - cyclotron_led_start];
      break;

      case HASLAB_CYCLOTRON_LED_COUNT:
      default:
        i_cyclotron_matrix_led = i_cyclotron_12led_matrix[i_led_cyclotron - cyclotron_led_start];
      break;
    }

    if(b_2021_ramp_up == true) {
      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_up = false;

        i_current_ramp_speed = cDelay;

        if(b_cyclotron_simulate_ring == true) {
          switch(i_cyclotron_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
              ms_cyclotron.start(i_current_ramp_speed);
            break;

            case FRUTTO_CYCLOTRON_LED_COUNT:
              if(i_cyclotron_matrix_led > 0) {
                ms_cyclotron.start(i_current_ramp_speed);
              }
              else {
                ms_cyclotron.start(i_current_ramp_speed - i_2021_delay);
              }
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
            default:
              if(i_cyclotron_matrix_led > 0) {
                ms_cyclotron.start(i_current_ramp_speed);
              }
              else {
                ms_cyclotron.start(i_current_ramp_speed - i_2021_delay);
              }
            break;
          }
        }
        else {
          ms_cyclotron.start(i_current_ramp_speed);
        }

        i_vibration_level = i_vibration_idle_level_2021;
      }
      else {
        i_current_ramp_speed = r_2021_ramp.update();

        if(b_cyclotron_simulate_ring == true) {
          switch(i_cyclotron_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
              ms_cyclotron.start(i_current_ramp_speed);
            break;

            case FRUTTO_CYCLOTRON_LED_COUNT:
              if(i_cyclotron_matrix_led > 0) {
                ms_cyclotron.start(i_current_ramp_speed);
              }
              else {
                ms_cyclotron.start(i_current_ramp_speed - i_2021_delay);
              }
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
            default:
              if(i_cyclotron_matrix_led > 0) {
                ms_cyclotron.start(i_current_ramp_speed);
              }
              else {
                ms_cyclotron.start(i_current_ramp_speed - i_2021_delay);
              }
            break;
          }
        }
        else {
          ms_cyclotron.start(i_current_ramp_speed);
        }

        i_vibration_level = i_vibration_level + 1;

        if(i_vibration_level < 30) {
          i_vibration_level = 30;
        }

        if(i_vibration_level > i_vibration_idle_level_2021) {
          i_vibration_level = i_vibration_idle_level_2021;
        }
      }
    }
    else if(b_2021_ramp_down == true) {
      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_down = false;
      }
      else {
        i_current_ramp_speed = r_2021_ramp.update();

        if(b_cyclotron_simulate_ring == true) {
          switch(i_cyclotron_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
              ms_cyclotron.start(i_current_ramp_speed);
            break;

            case FRUTTO_CYCLOTRON_LED_COUNT:
              if(i_cyclotron_matrix_led > 0) {
                ms_cyclotron.start(i_current_ramp_speed);
              }
              else {
                ms_cyclotron.start(i_current_ramp_speed - i_2021_delay);
              }
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
            default:
              if(i_cyclotron_matrix_led > 0) {
                ms_cyclotron.start(i_current_ramp_speed);
              }
              else {
                ms_cyclotron.start(i_current_ramp_speed - i_2021_delay);
              }
            break;
          }
        }
        else {
          ms_cyclotron.start(i_current_ramp_speed);
        }

        if(i_current_ramp_speed > 40 && i_vibration_level > i_vibration_lowest_level + 20) {
          i_vibration_level = i_vibration_level - 1;
        }
        else if(i_current_ramp_speed > 100 && i_vibration_level > i_vibration_lowest_level) {
          i_vibration_level = i_vibration_level - 1;
        }

        if(i_vibration_level < i_vibration_lowest_level) {
          i_vibration_level = i_vibration_lowest_level;
        }
      }
    }
    else {
      i_current_ramp_speed = cDelay;

      int t_cDelay = cDelay;

      switch(i_cyclotron_leds) {
        case OUTER_CYCLOTRON_LED_MAX:
        case FRUTTO_CYCLOTRON_LED_COUNT:
        case HASLAB_CYCLOTRON_LED_COUNT:
        default:
          if(i_cyclotron_multiplier > 1) {
            t_cDelay = t_cDelay - i_cyclotron_multiplier;

            if(t_cDelay < 1) {
              t_cDelay = 1;
            }
          }
        break;
      }

      if(b_cyclotron_simulate_ring == true) {
        switch(i_cyclotron_leds) {
          case OUTER_CYCLOTRON_LED_MAX:
            ms_cyclotron.start(t_cDelay);
          break;

          case FRUTTO_CYCLOTRON_LED_COUNT:
            if(i_cyclotron_matrix_led > 0) {
              ms_cyclotron.start(t_cDelay);
            }
            else if(i_current_ramp_speed > i_2021_delay) {
              ms_cyclotron.start(t_cDelay - i_2021_delay);
            }
            else {
              ms_cyclotron.start(t_cDelay - t_cDelay);
            }
          break;

          case HASLAB_CYCLOTRON_LED_COUNT:
          default:
            if(i_cyclotron_matrix_led > 0) {
              ms_cyclotron.start(t_cDelay);
            }
            else if(i_current_ramp_speed > i_2021_delay) {
              // This will simulate the fake LEDs during overheat and ribbon cable alarms.
              ms_cyclotron.start(t_cDelay - i_2021_delay);
            }
            else {
              ms_cyclotron.start(t_cDelay - t_cDelay);
            }
          break;
        }
      }
      else {
        ms_cyclotron.start(t_cDelay);
      }
    }

    if(b_wand_firing != true && b_overheating != true && b_alarm != true) {
      vibrationPack(i_vibration_level);
    }

    switch(i_cyclotron_leds) {
      case OUTER_CYCLOTRON_LED_MAX:
      case FRUTTO_CYCLOTRON_LED_COUNT:
        if(i_cyclotron_multiplier > 1) {
          cDelay = cDelay - i_cyclotron_multiplier;
        }
        else {
          cDelay = cDelay / i_cyclotron_multiplier;

          cDelay = cDelay * 3;
        }
      break;

      case HASLAB_CYCLOTRON_LED_COUNT:
      default:
        if(i_cyclotron_multiplier > 1) {
          cDelay = cDelay - i_cyclotron_multiplier;
        }
        else {
          cDelay = cDelay / i_cyclotron_multiplier;

          cDelay = cDelay * 2;
        }
      break;
    }

    if(cDelay < 1) {
      cDelay = 1;
    }

    if(b_clockwise == true) {
      if((i_cyclotron_led_value[i_led_cyclotron - cyclotron_led_start] == 0 && b_cyclotron_simulate_ring != true) || (i_cyclotron_led_value[i_led_cyclotron - cyclotron_led_start] == 0 && b_cyclotron_simulate_ring == true && i_cyclotron_matrix_led > 0)) {
        ms_cyclotron_led_fade_in[i_led_cyclotron - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[i_led_cyclotron - cyclotron_led_start].go(i_brightness, cDelay, CIRCULAR_IN);
      }

      i_led_cyclotron++;

      if(b_cyclotron_simulate_ring == true) {
        if(i_led_cyclotron > (i_powercell_leds + OUTER_CYCLOTRON_LED_MAX + i_nfilter_jewel_leds) - i_nfilter_jewel_leds - 1) {
          i_led_cyclotron = cyclotron_led_start;
        }
        else if(i_current_ramp_speed <= i_2021_delay) {
          switch(i_cyclotron_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
              // Do nothing.
            break;

            case FRUTTO_CYCLOTRON_LED_COUNT:
              i_cyclotron_matrix_led = i_cyclotron_20led_matrix[i_led_cyclotron - cyclotron_led_start];

              if(i_cyclotron_matrix_led == 0) {
                for(uint8_t i = i_led_cyclotron; i < OUTER_CYCLOTRON_LED_MAX; i++) {
                  if(i_cyclotron_20led_matrix[i - cyclotron_led_start] > 0) {
                    i_led_cyclotron = i;
                    break;
                  }
                }
              }
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
            default:
              i_cyclotron_matrix_led = i_cyclotron_12led_matrix[i_led_cyclotron - cyclotron_led_start];

              if(i_cyclotron_matrix_led == 0) {
                for(uint8_t i = i_led_cyclotron; i < OUTER_CYCLOTRON_LED_MAX; i++) {
                  if(i_cyclotron_12led_matrix[i - cyclotron_led_start] > 0) {
                    i_led_cyclotron = i;
                    break;
                  }
                }
              }
            break;
          }
        }
      }
      else {
        if(i_led_cyclotron > i_pack_num_leds - i_nfilter_jewel_leds - 1) {
          i_led_cyclotron = cyclotron_led_start;
        }
      }
    }
    else {
      if((i_cyclotron_led_value[i_led_cyclotron - cyclotron_led_start] == 0 && b_cyclotron_simulate_ring != true) || (i_cyclotron_led_value[i_led_cyclotron - cyclotron_led_start] == 0 && b_cyclotron_simulate_ring == true && i_cyclotron_matrix_led > 0)) {
        ms_cyclotron_led_fade_in[i_led_cyclotron - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[i_led_cyclotron - cyclotron_led_start].go(i_brightness, cDelay, CIRCULAR_IN);
      }

      i_led_cyclotron--;

      if(b_cyclotron_simulate_ring == true) {
        if(i_led_cyclotron < cyclotron_led_start) {
          i_led_cyclotron = (i_powercell_leds + OUTER_CYCLOTRON_LED_MAX + i_nfilter_jewel_leds) - i_nfilter_jewel_leds - 1;
        }
        else if(i_current_ramp_speed <= i_2021_delay) {
          switch(i_cyclotron_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
              // Do nothing.
            break;

            case FRUTTO_CYCLOTRON_LED_COUNT:
              i_cyclotron_matrix_led = i_cyclotron_20led_matrix[i_led_cyclotron - cyclotron_led_start];

              if(i_cyclotron_matrix_led == 0) {
                for(uint8_t i = i_led_cyclotron; i > cyclotron_led_start; i--) {
                  if(i_cyclotron_20led_matrix[i - cyclotron_led_start] > 0) {
                    i_led_cyclotron = i;
                    break;
                  }
                }
              }
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
            default:
              i_cyclotron_matrix_led = i_cyclotron_12led_matrix[i_led_cyclotron - cyclotron_led_start];

              if(i_cyclotron_matrix_led == 0) {
                for(uint8_t i = i_led_cyclotron; i > cyclotron_led_start; i--) {
                  if(i_cyclotron_12led_matrix[i - cyclotron_led_start] > 0) {
                    i_led_cyclotron = i;
                    break;
                  }
                }
              }
            break;
          }
        }
      }
      else {
        if(i_led_cyclotron < cyclotron_led_start) {
          i_led_cyclotron = i_pack_num_leds - i_nfilter_jewel_leds - 1;
        }
      }
    }

  }
}

void cyclotron1984(int cDelay) {
  if(ms_cyclotron.justFinished()) {
    cDelay = cDelay / i_cyclotron_multiplier;

    if(b_1984_led_start != true) {
      cyclotron84LightOff(i_led_cyclotron);
    }

    if(b_1984_led_start == true) {
      b_1984_led_start = false;
    }

    if(b_clockwise == true) {
      i_1984_counter++;
    }
    else {
      i_1984_counter--;
    }

    if(i_1984_counter > 3) {
      i_1984_counter = 0;
    }
    else if(i_1984_counter < 0) {
      i_1984_counter = 3;
    }

    i_led_cyclotron = cyclotron_led_start + i_1984_cyclotron_leds[i_1984_counter];

    cyclotron84LightOn(i_led_cyclotron);

    if(b_2021_ramp_up == true) {
      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_up = false;
        ms_cyclotron.start(cDelay);

        i_current_ramp_speed = cDelay;

        i_vibration_level = i_vibration_idle_level_1984;
      }
      else {
        ms_cyclotron.start(r_2021_ramp.update());
        i_current_ramp_speed = r_2021_ramp.update();

        i_vibration_level = i_vibration_idle_level_1984;
      }
    }
    else if(b_2021_ramp_down == true) {
      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_down = false;
      }
      else {
        ms_cyclotron.start(r_2021_ramp.update());

        i_current_ramp_speed = r_2021_ramp.update();

        i_vibration_level = i_vibration_level - 1;

        if(i_vibration_level < i_vibration_lowest_level) {
          i_vibration_level = i_vibration_lowest_level;
        }
      }
    }
    else {
      ms_cyclotron.start(cDelay);
    }

    if(b_wand_firing != true && b_overheating != true && b_alarm != true) {
      vibrationPack(i_vibration_level);
    }
  }
}

void cyclotron1984Alarm() {
  uint8_t i_brightness = getBrightness(i_cyclotron_brightness);
  uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, FIRING_MODE, b_cyclotron_colour_toggle);
  uint8_t led1 = cyclotron_led_start + i_1984_cyclotron_leds[0];
  uint8_t led2 = cyclotron_led_start + i_1984_cyclotron_leds[1];
  uint8_t led3 = cyclotron_led_start + i_1984_cyclotron_leds[2];
  uint8_t led4 = cyclotron_led_start + i_1984_cyclotron_leds[3];

  if(b_fade_cyclotron_led != true) {
    pack_leds[led1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
    pack_leds[led2] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
    pack_leds[led3] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
    pack_leds[led4] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

    // Turn on all the other cyclotron LEDs if required.
    if(b_cyclotron_single_led != true) {
      pack_leds[led1 + 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

      if(led1 - 1 < cyclotron_led_start) {
        led1 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led1 = led1 - 1;
      }

      pack_leds[led1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
      pack_leds[led2 + 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

      if(led2 - 1 < cyclotron_led_start) {
        led2 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led2 = led2 - 1;
      }

      pack_leds[led2] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
      pack_leds[led3 + 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

      if(led3 - 1 < cyclotron_led_start) {
        led3 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led3 = led3 - 1;
      }

      pack_leds[led3] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
      pack_leds[led4 + 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

      if(led4 - 1 < cyclotron_led_start) {
        led4 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led4 = led4 - 1;
      }

      pack_leds[led4] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
    }
  }
  else {
    if(i_cyclotron_led_value[led1 - cyclotron_led_start] == 0) {
      ms_cyclotron_led_fade_in[led1 - cyclotron_led_start].go(0);
      ms_cyclotron_led_fade_in[led1 - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
    }

    if(i_cyclotron_led_value[led2 - cyclotron_led_start] == 0) {
      ms_cyclotron_led_fade_in[led2 - cyclotron_led_start].go(0);
      ms_cyclotron_led_fade_in[led2 - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
    }

    if(i_cyclotron_led_value[led3 - cyclotron_led_start] == 0) {
      ms_cyclotron_led_fade_in[led3 - cyclotron_led_start].go(0);
      ms_cyclotron_led_fade_in[led3 - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
    }

    if(i_cyclotron_led_value[led4 - cyclotron_led_start] == 0) {
      ms_cyclotron_led_fade_in[led4 - cyclotron_led_start].go(0);
      ms_cyclotron_led_fade_in[led4 - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
    }

    // Turn on all the other cyclotron LEDs if required.
    if(b_cyclotron_single_led != true) {
      if(i_cyclotron_led_value[led1 + 1 - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[led1 + 1 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led1 + 1 - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(led1 - 1 < cyclotron_led_start) {
        led1 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led1 = led1 - 1;
      }

      if(i_cyclotron_led_value[led1  - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[led1 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led1 - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(i_cyclotron_led_value[led2 + 1 - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[led2 + 1 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led2 + 1 - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(led2 - 1 < cyclotron_led_start) {
        led2 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led2 = led2 - 1;
      }

      if(i_cyclotron_led_value[led2 - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[led2 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led2 - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(i_cyclotron_led_value[led3 + 1 - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[led3 + 1 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led3 + 1 - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(led3 - 1 < cyclotron_led_start) {
        led3 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led3 = led3 - 1;
      }

      if(i_cyclotron_led_value[led3 - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[led3 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led3 - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(i_cyclotron_led_value[led4 + 1 - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[led4 + 1 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led4 + 1 - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(led4 - 1 < cyclotron_led_start) {
        led4 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led4 = led4 - 1;
      }

      if(i_cyclotron_led_value[led4 - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[led4 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[led4 - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }
    }
  }
}

void cyclotron84LightOn(int cLed) {
  uint8_t i_brightness = getBrightness(i_cyclotron_brightness);
  uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, FIRING_MODE, b_cyclotron_colour_toggle);

  // We override the colour changes when using stock HasLab Cyclotron LEDs, returning full white.
  // Changing the colour space with a CHSV Object affects the brightness slightly for non RGB pixels.
  if(i_cyclotron_leds == HASLAB_CYCLOTRON_LED_COUNT && b_cyclotron_haslab_chsv_colour_change != true) {
    i_colour_scheme = C_HASLAB;
  }

  if(b_fade_cyclotron_led != true) {
    pack_leds[cLed] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

    // Turn on the other 2 LEDs if we are allowing 3 to light up.
    if(b_cyclotron_single_led != true) {
      pack_leds[cLed+1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

      if(cLed - 1 < cyclotron_led_start) {
        cLed = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        cLed = cLed - 1;
      }

      pack_leds[cLed] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
    }
  }
  else {
    if(i_cyclotron_led_value[cLed - cyclotron_led_start] == 0) {
      ms_cyclotron_led_fade_in[cLed - cyclotron_led_start].go(0);
      ms_cyclotron_led_fade_in[cLed - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay / i_cyclotron_multiplier, CIRCULAR_IN);
    }

    // Turn on the other 2 LEDs if we are allowing 3 to light up.
    if(b_cyclotron_single_led != true) {
      if(i_cyclotron_led_value[cLed + 1 - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[cLed + 1 - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[cLed + 1 - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay / i_cyclotron_multiplier, CIRCULAR_IN);
      }

      if(cLed - 1 < cyclotron_led_start) {
        cLed = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        cLed = cLed - 1;
      }

      if(i_cyclotron_led_value[cLed - cyclotron_led_start] == 0) {
        ms_cyclotron_led_fade_in[cLed - cyclotron_led_start].go(0);
        ms_cyclotron_led_fade_in[cLed - cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay / i_cyclotron_multiplier, CIRCULAR_IN);
      }
    }
  }
}

void cyclotron84LightOff(int cLed) {
  uint8_t i_brightness = getBrightness(i_cyclotron_brightness); // Calculate desired brightness.

  if(b_fade_cyclotron_led != true) {
    pack_leds[cLed] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);

    // Turn off the other 2 LEDs if we are allowing 3 to light up.
    if(b_cyclotron_single_led != true) {
      pack_leds[cLed + 1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);

      if(cLed - 1 < cyclotron_led_start) {
        cLed = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        cLed = cLed - 1;
      }

      pack_leds[cLed] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
    }
  }
  else {
    if(i_cyclotron_led_value[cLed - cyclotron_led_start] == i_brightness) {
      ms_cyclotron_led_fade_out[cLed - cyclotron_led_start].go(i_brightness);
      ms_cyclotron_led_fade_out[cLed - cyclotron_led_start].go(0, i_1984_fade_out_delay / i_cyclotron_multiplier, CIRCULAR_OUT);
    }

    // Turn off the other 2 LEDs if we are allowing 3 to light up.
    if(b_cyclotron_single_led != true) {
      if(i_cyclotron_led_value[cLed + 1 - cyclotron_led_start] == i_brightness) {
        ms_cyclotron_led_fade_out[cLed + 1 - cyclotron_led_start].go(i_brightness);
        ms_cyclotron_led_fade_out[cLed + 1 - cyclotron_led_start].go(0, i_1984_fade_out_delay / i_cyclotron_multiplier, CIRCULAR_OUT);
      }

      if(cLed - 1 < cyclotron_led_start) {
        cLed = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        cLed = cLed - 1;
      }

      if(i_cyclotron_led_value[cLed - cyclotron_led_start] == i_brightness) {
        ms_cyclotron_led_fade_out[cLed - cyclotron_led_start].go(i_brightness);
        ms_cyclotron_led_fade_out[cLed - cyclotron_led_start].go(0, i_1984_fade_out_delay / i_cyclotron_multiplier, CIRCULAR_OUT);
      }
    }
  }
}

void cyclotronOverHeating() {
  if(b_overheat_sync_to_fan != true) {
    smokeControl(true);
  }

  if(ms_overheating.justFinished()) {
    playEffect(S_AIR_RELEASE);
    playEffect(S_VENT_SMOKE, false, i_volume_effects, true, 120);

    // Fade in the steam release loop.
    playEffect(S_STEAM_LOOP, true, i_volume_effects, true, 1000);

    switch(i_wand_power_level) {
      case 1:
      default:
        ms_overheating_length.start(i_ms_overheating_length_1);
      break;

      case 2:
        ms_overheating_length.start(i_ms_overheating_length_2);
      break;

      case 3:
        ms_overheating_length.start(i_ms_overheating_length_3);
      break;

      case 4:
        ms_overheating_length.start(i_ms_overheating_length_4);
      break;

      case 5:
        ms_overheating_length.start(i_ms_overheating_length_5);
      break;
    }

    if(b_overheat_sync_to_fan != true) {
      smokeControl(false);
    }
  }

  // The cyclotron lights during the entire overheating sequence
  switch (SYSTEM_YEAR) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      if(b_overheat_lights_off != true) {
        cyclotron2021(i_2021_delay * 10);
        vibrationPack(i_vibration_lowest_level * 2);
      }
      else if(b_overheat_lights_off == true) {
        if(i_powercell_led > 0) {
          cyclotron2021(i_2021_delay * 10);

          vibrationPack(i_vibration_lowest_level);
        }
        else {
          vibrationPack(0);
        }
      }

      innerCyclotronRing(i_2021_inner_delay * 14);
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      innerCyclotronRing(i_2021_inner_delay * 14);

      if(ms_alarm.justFinished()) {
        ms_alarm.start(i_1984_delay / 2);
        if(b_fade_cyclotron_led != true) {
          resetCyclotronLeds();
        }
        else {
          cyclotron84LightOff(i_1984_cyclotron_leds[0] + cyclotron_led_start);
          cyclotron84LightOff(i_1984_cyclotron_leds[1] + cyclotron_led_start);
          cyclotron84LightOff(i_1984_cyclotron_leds[2] + cyclotron_led_start);
          cyclotron84LightOff(i_1984_cyclotron_leds[3] + cyclotron_led_start);
        }
      }
      else {
        if(ms_alarm.remaining() < i_1984_delay / 4) {
          if(b_overheat_lights_off != true) {
            vibrationPack(i_vibration_lowest_level);
            cyclotron1984Alarm();
          }
          else if(b_overheat_lights_off == true && i_powercell_led > 0) {
            vibrationPack(i_vibration_lowest_level);

            cyclotron1984Alarm();
          }
          else {
            vibrationPack(0);
          }
        }
      }
    break;
  }

  // Time the N-Filter light to when the fan is running.
  //if(ms_fan_stop_timer.isRunning() && ms_fan_stop_timer.remaining() < 3000) {
  if(ms_overheating_length.isRunning()) {
    if(b_overheat_sync_to_fan == true) {
      smokeControl(true);
    }

    // Turn the fans on.
    fanControl(true);
    fanBooster(true);

    // For strobing the vent light.
    if(ms_vent_light_off.justFinished()) {
      ms_vent_light_off.stop();
      ms_vent_light_on.start(i_vent_light_delay);

      if(b_overheat_strobe == true) {
        ventLight(true);
      }
    }
    else if(ms_vent_light_on.justFinished()) {
      ms_vent_light_on.stop();
      ms_vent_light_off.start(i_vent_light_delay);

      if(b_overheat_strobe == true) {
        ventLight(false);
      }
    }

    // For non-strobing vent light option.
    if(b_overheat_strobe != true) {
      if(b_vent_light_on != true) {
        // Solid light on if strobe option turned off.
        ventLight(true);
      }
    }

    ventLightLEDW(true);
  }

  if(ms_overheating_length.justFinished()) {
    // Tell the Neutrona Wand the overheating is finished.
    packOverHeatingFinished();
  }
}

void packOverHeatingFinished() {
  packSerialSend(P_OVERHEATING_FINISHED);
  serial1Send(A_OVERHEATING_FINISHED);

  ms_overheating_length.stop();

  stopEffect(S_STEAM_LOOP);
  playEffect(S_VENT_DRY);
  playEffect(S_STEAM_LOOP_FADE_OUT);

  b_overheating = false;

  // Turn off the smoke.
  smokeControl(false);

  // Stop the fans.
  fanControl(false);
  fanBooster(false);

  // Reset the LEDs before resetting the alarm flag.
  if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
    resetCyclotronLeds();
  }

  b_alarm = false;

  if(b_overheat_lights_off == true) {
    cyclotronSpeedRevert();

    // Reset the ramp speeds.
    switch(SYSTEM_YEAR) {
      case SYSTEM_1984:
      case SYSTEM_1989:
          // Reset the ramp speeds.
          i_current_ramp_speed = i_1984_delay * 1.3;
          i_inner_current_ramp_speed = i_inner_ramp_delay;
      break;

      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        // Reset the ramp speeds.
        i_current_ramp_speed = i_2021_ramp_delay;
        i_inner_current_ramp_speed = i_inner_ramp_delay;
      break;
    }
  }

  reset2021RampUp();

  packStartup();

  // Turn off the vent lights
  ventLight(false);
  ventLightLEDW(false);
  ms_vent_light_off.stop();
  ms_vent_light_on.stop();

  ms_cyclotron.start(i_2021_delay);
}

void cyclotronNoCable() {
  switch (SYSTEM_YEAR) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      cyclotron2021(i_2021_delay * 10);
      innerCyclotronRing(i_2021_inner_delay * 14);

      if(ms_alarm.justFinished()) {
        ventLight(false);
        ventLightLEDW(false);
        ms_alarm.start(i_1984_delay);
      }
      else {
        if(ms_alarm.remaining() < i_1984_delay / 2) {
          ventLight(true);
          ventLightLEDW(true);
        }
      }

      vibrationPack(i_vibration_lowest_level * 3);
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      innerCyclotronRing(i_2021_inner_delay * 14);
      cyclotron1984(i_1984_delay * 3);

      if(ms_alarm.justFinished()) {
        ms_alarm.start(i_1984_delay / 2);

        // Turn off the N-Filter lights.
        ventLight(false);
        ventLightLEDW(false);

        vibrationPack(i_vibration_lowest_level);
      }
      else {
        if(ms_alarm.remaining() < i_1984_delay / 4) {
          vibrationPack(i_vibration_idle_level_1984);

          // Turn on the N-Filter lights.
          ventLight(true);
          ventLightLEDW(true);
        }
      }
    break;
  }
}

// Turns off the LEDs in the Cyclotron Lid only.
void cyclotronLidLedsOff() {
  if(b_fade_out != true) {
    uint8_t i_cyclotron_leds_total = i_pack_num_leds - i_nfilter_jewel_leds - cyclotron_led_start;

    if(b_cyclotron_simulate_ring == true) {
      i_cyclotron_leds_total = OUTER_CYCLOTRON_LED_MAX;
    }

    for(int i = 0; i < i_cyclotron_leds_total; i++) {
      if(b_cyclotron_simulate_ring == true) {
        switch(i_cyclotron_leds) {
          case OUTER_CYCLOTRON_LED_MAX:
            // For 40-element LED ring.
            pack_leds[i_cyclotron_40led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
          break;

          case FRUTTO_CYCLOTRON_LED_COUNT:
            // For Frutto Technology 20 LEDs.
            pack_leds[i_cyclotron_20led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
          break;

          case HASLAB_CYCLOTRON_LED_COUNT:
          default:
              pack_leds[i_cyclotron_12led_matrix[i] + cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
          break;
        }
      }
      else {
        pack_leds[i + cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      }
    }

    for(int i = 0; i < i_cyclotron_leds_total; i++) {
        ms_cyclotron_led_fade_out[i].go(0);
        ms_cyclotron_led_fade_in[i].go(0);

        i_cyclotron_led_on_status[i] = false;
    }
  }
}

void resetCyclotronLeds() {
  // Turn off optional N-Filter LED.
  digitalWrite(i_nfilter_led_pin, LOW);

  cyclotronLidLedsOff();

  // Only reset the start LED if the pack is off or just started.
  if(b_reset_start_led == true) {
    i_led_cyclotron = cyclotron_led_start;
  }

  // Keep the fade control fading out a light that is not on during startup.
  if(PACK_STATE == MODE_OFF){
    if(b_1984_led_start != true) {
      b_1984_led_start = true;
    }
  }

  // Tell the Inner Cyclotron to turn off the LEDs.
  if(b_cyclotron_lid_on == true) {
    innerCyclotronOff();
  }
  else if(b_alarm != true || PACK_STATE == MODE_OFF) {
    innerCyclotronOff();
  }

  cyclotronSpeedRevert();
}

void clearCyclotronFades() {
  if(b_fade_out != true) {
    for(int i = 0; i < OUTER_CYCLOTRON_LED_MAX; i++) {
      i_cyclotron_led_value[i] = 0;
    }
  }
}

void innerCyclotronOff() {
  for(int i = 0; i < i_inner_cyclotron_num_leds; i++) {
    cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
  }
}

// For NeoPixel rings, ramp up and ramp down the LEDs in the ring and set the speed. (optional)
void innerCyclotronRing(int cDelay) {
  if(ms_cyclotron_ring.justFinished()) {
    if(b_inner_ramp_up == true) {
      if(r_inner_ramp.isFinished()) {
        b_inner_ramp_up = false;
        ms_cyclotron_ring.start(cDelay);

        i_inner_current_ramp_speed = cDelay;
      }
      else {
        ms_cyclotron_ring.start(r_inner_ramp.update());
        i_inner_current_ramp_speed = r_inner_ramp.update();
      }
    }
    else if(b_inner_ramp_down == true) {
      if(r_inner_ramp.isFinished()) {
        b_inner_ramp_down = false;
      }
      else {
        ms_cyclotron_ring.start(r_inner_ramp.update());

        i_inner_current_ramp_speed = r_inner_ramp.update();
      }
    }
    else {
      i_inner_current_ramp_speed = cDelay;

      if(i_cyclotron_multiplier > 1) {
        if(i_cyclotron_multiplier > 4) {
          cDelay = cDelay - 4;
        }
        else {
          cDelay = cDelay - i_cyclotron_multiplier;
        }
      }

      if(cDelay < 1) {
        cDelay = 1;
      }

      ms_cyclotron_ring.start(cDelay);
    }

    // Colour control for the Inner Cyclotron LEDs.
    uint8_t i_brightness = getBrightness(i_cyclotron_inner_brightness);
    uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_INNER, FIRING_MODE, b_cyclotron_colour_toggle);

    if(i_cyclotron_multiplier > 1) {
      switch(i_cyclotron_multiplier) {
        case 6:
          cDelay = cDelay - 4;
        break;

        case 5:
          cDelay = cDelay - 3;
        break;

        case 4:
          cDelay = cDelay - 3;
        break;

        case 3:
          cDelay = cDelay - 2;
        break;

        case 2:
          cDelay = cDelay - 2;
        break;

        default:
          cDelay = cDelay - 1;
        break;
      }
    }
    else {
      cDelay = cDelay / i_cyclotron_multiplier;
    }

    if(cDelay < 2) {
      cDelay = 2;
    }

    if(b_clockwise == true) {
      if(b_cyclotron_lid_on != true) {
        if(b_grb_cyclotron == true) {
          // For GRB LEDs.
          cyclotron_leds[i_led_cyclotron_ring] = getHueAsGRB(CYCLOTRON_INNER, i_colour_scheme, i_brightness);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring] = getHueAsRGB(CYCLOTRON_INNER, i_colour_scheme, i_brightness);
        }

        if(i_led_cyclotron_ring == 0) {
          cyclotron_leds[i_inner_cyclotron_num_leds - 1] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring - 1] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
        }
      }

      i_led_cyclotron_ring++;

      if(i_led_cyclotron_ring > i_inner_cyclotron_num_leds - 1) {
        i_led_cyclotron_ring = 0;
      }
    }
    else {
      if(b_cyclotron_lid_on != true) {
        // For GRB LEDs.
        if(b_grb_cyclotron == true) {
          cyclotron_leds[i_led_cyclotron_ring] = getHueAsGRB(CYCLOTRON_INNER, i_colour_scheme, i_brightness);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring] = getHueAsRGB(CYCLOTRON_INNER, i_colour_scheme, i_brightness);
        }

        if(i_led_cyclotron_ring + 1 > i_inner_cyclotron_num_leds - 1) {
          cyclotron_leds[0] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring + 1] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
        }
      }

      i_led_cyclotron_ring--;

      if(i_led_cyclotron_ring < 0) {
        i_led_cyclotron_ring = i_inner_cyclotron_num_leds -1;
      }
    }
  }
}

void reset2021RampUp() {
  b_2021_ramp_up = true;
  b_2021_ramp_up_start = true;

  // Inner Cyclotron ring.
  b_inner_ramp_up = true;
}

void reset2021RampDown() {
  b_2021_ramp_down = true;
  b_2021_ramp_down_start = true;

  // Inner Cyclotron ring.
  b_inner_ramp_down = true;
}

void ventLightLEDW(bool b_on) {
  if(b_on == true) {
    digitalWrite(i_nfilter_led_pin, HIGH);
  }
  else {
    digitalWrite(i_nfilter_led_pin, LOW);
  }
}

void ventLight(bool b_on) {
  uint8_t i_colour_scheme = getDeviceColour(VENT_LIGHT, FIRING_MODE, true);
  b_vent_light_on = b_on;

  if(b_on == true) {
    // If doing firing smoke effects, let's change the light colours.
    if(b_wand_firing == true || b_overheating == true) {
      if(FIRING_MODE == PROTON || FIRING_MODE == VENTING) {
        // Override the N-Filter light colours for a proton stream.
        switch(i_wand_power_level) {
          case 1:
            i_colour_scheme = C_RED;

            if(FIRING_MODE == VENTING) {
              i_colour_scheme = C_RAINBOW;
            }
          break;

          case 2:
            i_colour_scheme = C_PINK;
          break;

          case 3:
            i_colour_scheme = C_YELLOW;
          break;

          case 4:
            i_colour_scheme = C_LIGHT_BLUE;
          break;

          case 5:
            i_colour_scheme = C_WHITE;
          break;

          default:
            i_colour_scheme = C_WHITE;
          break;
        }
      }
    }
    else if(b_alarm == true) {
      i_colour_scheme = C_RED;
    }

    for(int i = i_vent_light_start; i < i_pack_num_leds; i++) {
      pack_leds[i] = getHueAsRGB(VENT_LIGHT, i_colour_scheme); // Uses full brightness.
    }
  }
  else {
    for(int i = i_vent_light_start; i < i_pack_num_leds; i++) {
      pack_leds[i] = getHueAsRGB(VENT_LIGHT, C_BLACK);
    }
  }
}

// Only for Afterlife (2021) mode.
void checkCyclotronAutoSpeed() {
  // No need to start any timers until after any ramping has finished; only in Afterlife (2021) do we do the auto speed increases.
  if(b_wand_firing == true && b_2021_ramp_up != true && b_2021_ramp_down != true && (SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE)) {
    if(ms_cyclotron_auto_speed_timer.justFinished() && i_cyclotron_multiplier < 6) {
      // Increase the Cyclotron speed.
      i_cyclotron_multiplier++;

      // Increase the Cyclotron Switch Panel LEDs speed.
      i_cyclotron_switch_led_mulitplier++;

      // Restart the timer.
      ms_cyclotron_auto_speed_timer.stop();
      ms_cyclotron_auto_speed_timer.start(i_cyclotron_auto_speed_timer_length / i_wand_power_level);
    }
  }
}

void modeFireStartSounds() {
  // Adjust the gain with the Afterlife idling sound effect while firing.
  if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) && i_wand_power_level < 5) {
    if(ms_idle_fire_fade.remaining() < 3000) {
      adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - 2, true, 100);
    }
    else {
      adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - 2, true, ms_idle_fire_fade.remaining());
    }
  }

  if(FIRING_MODE != MESON) {
    if(SYSTEM_YEAR == SYSTEM_1989) {
      playEffect(S_FIRE_START_SPARK, false, i_volume_effects - 10);
    }
    else {
      playEffect(S_FIRE_START_SPARK);
    }
  }

  switch(FIRING_MODE) {
    case PROTON:
    default:
      if(SYSTEM_YEAR == SYSTEM_1989 && b_firing_intensify == true) {
        int8_t i_v_fire_start = i_volume_effects - 10;

        if(i_v_fire_start < i_volume_abs_min) {
          i_v_fire_start = i_volume_abs_min;
        }

        playEffect(S_FIRE_START, false, i_v_fire_start);
      }
      else {
        playEffect(S_FIRE_START);
      }

      switch(i_wand_power_level) {
        case 1 ... 4:
          if(b_firing_intensify == true) {
            if(SYSTEM_YEAR == SYSTEM_1989) {
              playEffect(S_GB2_FIRE_LOOP, true, i_volume_effects, true, 6500);
              playEffect(S_GB2_FIRE_START);
            }
            else {
              playEffect(S_GB1_FIRE_LOOP, true, i_volume_effects, true, 1000);
              playEffect(S_GB1_FIRE_START);
            }

            b_sound_firing_intensify_trigger = true;
          }
          else {
            b_sound_firing_intensify_trigger = false;
          }

          if(b_firing_alt == true) {
            playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 1000);

            if(SYSTEM_YEAR == SYSTEM_1989) {
              playEffect(S_GB2_FIRE_START);
            }

            b_sound_firing_alt_trigger = true;
          }
          else {
            b_sound_firing_alt_trigger = false;
          }
        break;

        case 5:
          switch(SYSTEM_YEAR) {
            case SYSTEM_1989:
              playEffect(S_GB2_FIRE_START);
            break;

            case SYSTEM_1984:
              playEffect(S_GB1_FIRE_START_HIGH_POWER, false, i_volume_effects);
              playEffect(S_GB1_FIRE_START);
            break;

            case SYSTEM_AFTERLIFE:
            case SYSTEM_FROZEN_EMPIRE:
            default:
              playEffect(S_AFTERLIFE_FIRE_START, false, i_volume_effects + 2);
            break;
          }

          if(b_firing_intensify == true) {
            // Reset some sound triggers.
            b_sound_firing_intensify_trigger = true;
            playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, true, 700);
          }
          else {
            b_sound_firing_intensify_trigger = false;
          }

          if(b_firing_alt == true) {
            // Reset some sound triggers.
            b_sound_firing_alt_trigger = true;

            playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 700);
          }
          else {
            b_sound_firing_alt_trigger = false;
          }
        break;
      }
    break;

    case SLIME:
      stopEffect(S_SLIME_END);
      playEffect(S_SLIME_START);
      playEffect(S_SLIME_LOOP, true, i_volume_effects, true, 1500);
    break;

    case STASIS:
      stopEffect(S_STASIS_END);
      playEffect(S_STASIS_START);
      playEffect(S_STASIS_LOOP, true, i_volume_effects, true, 1000);
    break;

    case MESON:
      playEffect(S_MESON_START);
      playEffect(S_MESON_FIRE_PULSE);

      switch(i_wand_power_level) {
        case 5:
          ms_meson_blast.start(i_meson_blast_delay_level_5);
        break;

        case 4:
          ms_meson_blast.start(i_meson_blast_delay_level_5);
        break;

        case 3:
          ms_meson_blast.start(i_meson_blast_delay_level_3);
        break;

        case 2:
          ms_meson_blast.start(i_meson_blast_delay_level_2);
        break;

        case 1:
        default:
          ms_meson_blast.start(i_meson_blast_delay_level_1);
        break;
      }
    break;

    case VENTING:
    case SETTINGS:
      // Nothing.
    break;
  }
}

void wandFiring() {
  modeFireStartSounds();

  b_wand_firing = true;

  serial1Send(A_FIRING);

  // Reset the Cyclotron auto speed up timers. Only for Afterlife (2021) mode.
  ms_cyclotron_auto_speed_timer.stop();

  if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
    ms_cyclotron_auto_speed_timer.start(i_cyclotron_auto_speed_timer_length / i_wand_power_level);
  }

  if(b_stream_effects == true) {
    unsigned int i_s_random = random(7,14) * 1000;
    ms_firing_sound_mix.start(i_s_random);
  }

  // Turn off any smoke.
  smokeControl(false);

  // Start a smoke timer to play a little bit of smoke while firing.
  ms_smoke_timer.start(i_smoke_timer[i_wand_power_level - 1]);
  ms_smoke_on.stop();

  vibrationPack(255);

  // Reset some vent light timers.
  ms_vent_light_off.stop();
  ms_vent_light_on.stop();
  ms_vent_light_off.start(i_vent_light_delay);

  // Reset vent sounds flag.
  b_vent_sounds = true;

  ms_firing_length_timer.start(i_firing_timer_length);
}

void modeFireStopSounds() {
  if(b_wand_firing == true) {
    // Adjust the gain with the Afterlife idling track.
    if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) && i_wand_power_level < 5) {
      if(ms_idle_fire_fade.remaining() < 1000) {
        adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects, true, 30);
      }
      else {
        adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects, true, ms_idle_fire_fade.remaining());
      }
    }

    switch(FIRING_MODE) {
      case PROTON:
      default:
        // Play different firing end stream sound depending on how long we have been firing for.
        if(ms_firing_length_timer.remaining() < 5000) {
          // Short tail end.
          playEffect(S_FIRING_END_GUN);
        }
        else if(ms_firing_length_timer.remaining() < 10000) {
          // Mid tail end.
          playEffect(S_FIRING_END_MID);
        }
        else {
          // Long tail end.
          playEffect(S_FIRING_END);
        }
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
        // Nothing
      break;
    }
  }

  wandStopFiringSounds();
}

void wandStoppedFiring() {
  ms_meson_blast.stop();

  modeFireStopSounds();

  ms_firing_sound_mix.stop();

  serial1Send(A_FIRING_STOPPED);

  // Stop the auto speed timer.
  ms_cyclotron_auto_speed_timer.stop();

  b_wand_firing = false;
  b_firing_alt = false;
  b_firing_intensify = false;

  // Reset some vent light timers.
  ms_vent_light_off.stop();
  ms_vent_light_on.stop();
  ventLight(false);
  ventLightLEDW(false);

  // Reset vent sounds flag.
  b_vent_sounds = true;

  // Turn off any smoke.
  smokeControl(false);

  // Turn off the fans.
  fanControl(false);
  fanBooster(false);

  ms_firing_length_timer.stop();
  ms_smoke_timer.stop();
  ms_smoke_on.stop();
}

void wandStopFiringSounds() {
  // Firing sounds.
  switch(FIRING_MODE) {
    case PROTON:
    default:
      if(SYSTEM_YEAR == SYSTEM_1989) {
        stopEffect(S_GB2_FIRE_START);
        stopEffect(S_GB2_FIRE_LOOP);
      }
      else {
        stopEffect(S_GB1_FIRE_START);
        stopEffect(S_GB1_FIRE_LOOP);
      }

      if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
        stopEffect(S_AFTERLIFE_FIRE_START);
      }

      stopEffect(S_FIRING_LOOP_GB1);
      stopEffect(S_GB1_FIRE_START_HIGH_POWER);
      stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);

      /*
      // Keep this for later.
      stopEffect(S_FIRE_START_SPARK);
      stopEffect(S_FIRE_START);
      */
    break;

    case SLIME:
      stopEffect(S_SLIME_START);
      stopEffect(S_SLIME_LOOP);
    break;

    case STASIS:
      stopEffect(S_STASIS_START);
      stopEffect(S_STASIS_LOOP);
    break;

    case MESON:
      // Nothing.
    break;

    case VENTING:
    case SETTINGS:
      // Nothing
    break;
  }

  switch(STATUS_CTS) {
    case CTS_FIRING_1984:
      STATUS_CTS = CTS_NOT_FIRING;

      stopEffect(S_CROSS_STREAMS_START);
      stopEffect(S_CROSS_STREAMS_END);

      playEffect(S_CROSS_STREAMS_END, false, i_volume_effects + 10);
    break;

    case CTS_FIRING_2021:
      STATUS_CTS = CTS_NOT_FIRING;

      stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
      stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);

      playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects + 10);
    break;

    case CTS_NOT_FIRING:
      // Do nothing.
    break;
  }

  b_sound_firing_intensify_trigger = false;
  b_sound_firing_alt_trigger = false;
}

void packAlarm() {
  modeFireStopSounds();

  // Pack sounds.
  if(SYSTEM_YEAR == SYSTEM_1989) {
    stopEffect(S_GB2_PACK_START);
    stopEffect(S_GB2_PACK_LOOP);
  }
  else if(SYSTEM_YEAR == SYSTEM_1984) {
    stopEffect(S_IDLE_LOOP);
    stopEffect(S_BOOTUP);
  }
  else {
    stopEffect(S_AFTERLIFE_PACK_STARTUP);
    stopEffect(S_AFTERLIFE_PACK_IDLE_LOOP);
  }

  playEffect(S_SHUTDOWN);

  if(SYSTEM_YEAR == SYSTEM_1989) {
    playEffect(S_GB2_PACK_OFF);
  }
  else {
    playEffect(S_PACK_SHUTDOWN);
  }

  switch(FIRING_MODE) {
    case SLIME:
      stopEffect(S_PACK_SLIME_TANK_LOOP);
    break;

    case STASIS:
      stopEffect(S_STASIS_IDLE_LOOP);
    break;

    case MESON:
      stopEffect(S_MESON_IDLE_LOOP);
    break;

    default:
      // Do nothing.
    break;
  }

  if(b_overheating != true) {
    switch(SYSTEM_YEAR) {
      case SYSTEM_1984:
      case SYSTEM_1989:
      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
        playEffect(S_PACK_RIBBON_ALARM_1, true);
        playEffect(S_ALARM_LOOP, true);
        playEffect(S_RIBBON_CABLE_START);
      break;

      // Not used.
      default:
        playEffect(S_PACK_BEEPING, true);
      break;
    }
  }
}

// LEDs for the 1984/2021 and vibration switches.
void cyclotronSwitchPlateLEDs() {
  if(switch_cyclotron_lid.isReleased()) {
    // Play sounds when lid is removed.
    stopEffect(S_VENT_SMOKE);
    stopEffect(S_MODE_SWITCH);
    stopEffect(S_CLICK);
    stopEffect(S_SPARKS_LOOP);
    stopEffect(S_BEEPS_BARGRAPH);

    playEffect(S_MODE_SWITCH);

    playEffect(S_VENT_SMOKE);

    // Play some spark sounds if the pack is running and the lid is removed.
    if(PACK_STATE == MODE_ON) {
      playEffect(S_SPARKS_LOOP);
    }
  }

  if(switch_cyclotron_lid.isPressed()) {
    // Play sounds when lid is mounted.
    stopEffect(S_CLICK);
    stopEffect(S_VENT_DRY);

    playEffect(S_CLICK);

    playEffect(S_VENT_DRY);

    // Play some spark sounds if the pack is running and the lid is put back on.
    if(PACK_STATE == MODE_ON) {
      playEffect(S_SPARKS_LOOP);
    }
  }

  if(switch_cyclotron_lid.getState() == LOW) {
    if(b_cyclotron_lid_on != true) {
      // The Cyclotron Lid is now on.
      b_cyclotron_lid_on = true;

      // Turn off Inner Cyclotron LEDs.
      innerCyclotronOff();
    }
  }
  else {
    if(b_cyclotron_lid_on == true) {
      // The Cyclotron Lid is now off.
      b_cyclotron_lid_on = false;

      // Make sure the Inner Cyclotron turns on if we are in the EEPROM LED menu.
      if(b_spectral_lights_on == true) {
        spectralLightsOn();
      }
    }
  }

  if(b_cyclotron_lid_on != true) {
    if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
      if(ms_cyclotron_switch_plate_leds.remaining() < i_cyclotron_switch_plate_leds_delay / 2) {
        digitalWrite(cyclotron_switch_led_green, HIGH);
      }
      else {
        digitalWrite(cyclotron_switch_led_green, LOW);
      }
    }
    else {
      digitalWrite(cyclotron_switch_led_green, HIGH);
    }

    if(b_vibration == true) {
      if(ms_cyclotron_switch_plate_leds.remaining() < i_cyclotron_switch_plate_leds_delay / 2) {
        digitalWrite(cyclotron_switch_led_yellow, HIGH);
      }
      else {
        digitalWrite(cyclotron_switch_led_yellow, LOW);
      }
    }
    else {
      digitalWrite(cyclotron_switch_led_yellow, HIGH);
    }
  }
  else {
    // Keep the Cyclotron switch LEDs off when the lid is on.
    digitalWrite(cyclotron_switch_led_green, LOW);
    digitalWrite(cyclotron_switch_led_yellow, LOW);
  }

  if(ms_cyclotron_switch_plate_leds.justFinished()) {
    ms_cyclotron_switch_plate_leds.start(i_cyclotron_switch_plate_leds_delay);
  }
}

void vibrationPack(int i_level) {
  if(b_vibration == true && b_vibration_enabled == true) {
    if(b_vibration_firing == true) {
      if(b_wand_firing == true) {
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

void cyclotronSpeedRevert() {
  // Stop overheat beeps.
  stopEffect(S_BEEP_8);

  i_cyclotron_multiplier = 1;
  i_cyclotron_switch_led_mulitplier = 1;
  i_powercell_multiplier = 1;
}

void cyclotronSpeedIncrease() {
  switch(SYSTEM_YEAR) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      if(i_cyclotron_multiplier < 9) {
        i_cyclotron_multiplier++;
      }

      if(i_cyclotron_switch_led_mulitplier < 9) {
        i_cyclotron_switch_led_mulitplier++;
      }

      if(i_powercell_multiplier < 6) {
        i_powercell_multiplier++;
      }
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      i_cyclotron_multiplier++;
      i_cyclotron_switch_led_mulitplier++;
      i_powercell_multiplier++;
    break;
  }
}

void adjustVolumeEffectsGain() {
  // Since adjusting only from the wand, only certain effects need to be adjusted on the fly.
  w_trig.trackGain(S_PACK_RIBBON_ALARM_1, i_volume_effects);
  w_trig.trackGain(S_ALARM_LOOP, i_volume_effects);
  w_trig.trackGain(S_RIBBON_CABLE_START, i_volume_effects);
  w_trig.trackGain(S_PACK_BEEPING, i_volume_effects); // Not used.
  w_trig.trackGain(S_BEEP_8, i_volume_effects);
  w_trig.trackGain(S_SHUTDOWN, i_volume_effects);
  w_trig.trackGain(S_GB2_PACK_START, i_volume_effects);
  w_trig.trackGain(S_GB2_PACK_LOOP, i_volume_effects);
  w_trig.trackGain(S_GB2_PACK_OFF, i_volume_effects);
  w_trig.trackGain(S_PACK_SHUTDOWN, i_volume_effects);
  w_trig.trackGain(S_PACK_SHUTDOWN_AFTERLIFE, i_volume_effects);
  w_trig.trackGain(S_IDLE_LOOP, i_volume_effects);
  w_trig.trackGain(S_BOOTUP, i_volume_effects);
  w_trig.trackGain(S_AFTERLIFE_PACK_STARTUP, i_volume_effects);
  w_trig.trackGain(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects);

  w_trig.trackGain(S_PACK_SLIME_TANK_LOOP, i_volume_effects);
  w_trig.trackGain(S_STASIS_IDLE_LOOP, i_volume_effects);
  w_trig.trackGain(S_MESON_IDLE_LOOP, i_volume_effects);

  w_trig.trackGain(S_AFTERLIFE_WAND_IDLE_2, i_volume_effects - 10);
  w_trig.trackGain(S_AFTERLIFE_WAND_RAMP_1, i_volume_effects - 10);
  w_trig.trackGain(S_AFTERLIFE_WAND_RAMP_2, i_volume_effects - 10);
  w_trig.trackGain(S_AFTERLIFE_WAND_RAMP_2_FADE_IN, i_volume_effects - 10);
  w_trig.trackGain(S_AFTERLIFE_WAND_IDLE_1, i_volume_effects - 10);
  w_trig.trackGain(S_AFTERLIFE_WAND_IDLE_2, i_volume_effects - 10);
  w_trig.trackGain(S_AFTERLIFE_WAND_RAMP_DOWN_2, i_volume_effects - 10);
  w_trig.trackGain(W_AFTERLIFE_GUN_RAMP_DOWN_2_FADE_OUT, i_volume_effects - 10);
  w_trig.trackGain(S_AFTERLIFE_WAND_RAMP_DOWN_1, i_volume_effects - 10);
}

void increaseVolumeEffects() {
  if(i_volume_percentage + VOLUME_EFFECTS_MULTIPLIER > 100) {
    i_volume_percentage = 100;

    // Provide feedback at maximum volume.
    stopEffect(S_BEEPS_ALT);
    playEffect(S_BEEPS_ALT);
  }
  else {
    i_volume_percentage = i_volume_percentage + VOLUME_EFFECTS_MULTIPLIER;
  }

  i_volume_effects = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_percentage / 100);

  adjustVolumeEffectsGain();
}

void decreaseVolumeEffects() {
  if(i_volume_percentage - VOLUME_EFFECTS_MULTIPLIER < 0) {
    i_volume_percentage = 0;

    // Provide feedback at minimum volume.
    stopEffect(S_BEEPS_ALT);
    playEffect(S_BEEPS_ALT, false, i_volume_master - 10);
  }
  else {
    i_volume_percentage = i_volume_percentage - VOLUME_EFFECTS_MULTIPLIER;
  }

  i_volume_effects = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_percentage / 100);

  adjustVolumeEffectsGain();
}


void increaseVolumeEEPROM() {
  if(i_volume_master_eeprom == i_volume_abs_min && MINIMUM_VOLUME > i_volume_master_eeprom) {
    i_volume_master_eeprom = MINIMUM_VOLUME;
  }

  if(i_volume_master_percentage + VOLUME_MULTIPLIER > 100) {
    i_volume_master_percentage = 100;
  }
  else {
    i_volume_master_percentage = i_volume_master_percentage + VOLUME_MULTIPLIER;
  }

  i_volume_master_eeprom = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
  i_volume_revert = i_volume_master_eeprom;

  if(b_pack_on != true && b_pack_shutting_down != true) {
    // Provide feedback when the pack is not running.
    stopEffect(S_BEEPS_ALT);
    playEffect(S_BEEPS_ALT, false, i_volume_master_eeprom);
  }

  i_volume_master = i_volume_master_eeprom;

  w_trig.masterGain(i_volume_master_eeprom);
}

void decreaseVolumeEEPROM() {
  if(i_volume_master_eeprom == i_volume_abs_min) {
    // Cannot go any lower.
  }
  else {
    if(i_volume_master_percentage - VOLUME_MULTIPLIER < 0) {
      i_volume_master_percentage = 0;
    }
    else {
      i_volume_master_percentage = i_volume_master_percentage - VOLUME_MULTIPLIER;
    }

    i_volume_master_eeprom = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
    i_volume_revert = i_volume_master_eeprom;

    i_volume_master = i_volume_master_eeprom;

    w_trig.masterGain(i_volume_master_eeprom);
  }

  if(b_pack_on != true && b_pack_shutting_down != true) {
    // Provide feedback when the pack is not running.
    stopEffect(S_BEEPS_ALT);
    playEffect(S_BEEPS_ALT, false, i_volume_master_eeprom);
  }
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

  if(b_pack_on != true && b_pack_shutting_down != true) {
    // Provide feedback when the pack is not running.
    stopEffect(S_BEEPS_ALT);
    playEffect(S_BEEPS_ALT, false, i_volume_master);
  }

  w_trig.masterGain(i_volume_master);
}

void decreaseVolume() {
  if(i_volume_master == i_volume_abs_min) {
    // Cannot go any lower.
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

  if(b_pack_on != true && b_pack_shutting_down != true) {
    // Provide feedback when the pack is not running.
    stopEffect(S_BEEPS_ALT);
    playEffect(S_BEEPS_ALT, false, i_volume_master);
  }
}

void readEncoder() {
  if(digitalRead(encoder_pin_a) == digitalRead(encoder_pin_b)) {
    i_encoder_pos++;
  }
  else {
    i_encoder_pos--;
  }

  i_val_rotary = i_encoder_pos / 2.5;
}

void checkRotaryEncoder() {
  if(i_val_rotary > i_last_val_rotary) {
    if(ms_volume_check.isRunning() != true) {
      increaseVolume();

      // Tell wand to increase volume.
      packSerialSend(P_VOLUME_INCREASE);

      ms_volume_check.start(50);
    }
  }

  if(i_val_rotary < i_last_val_rotary) {
    if(ms_volume_check.isRunning() != true) {
      decreaseVolume();

      // Tell wand to decrease volume.
      packSerialSend(P_VOLUME_DECREASE);

      ms_volume_check.start(50);
    }
  }

  i_last_val_rotary = i_val_rotary;

  if(ms_volume_check.justFinished()) {
    ms_volume_check.stop();
  }
}

// Smoke # 1. N-Filter cone outlet.
void smokeControl(bool b_smoke_on) {
  if(b_smoke_enabled == true) {
    if(b_smoke_on == true) {
      if(b_wand_firing == true && b_overheating != true && b_smoke_1_continuous_firing == true && b_smoke_continuous_mode[i_wand_power_level - 1] == true) {
        digitalWrite(smoke_pin, HIGH);
      }
      else if(b_overheating == true && b_wand_firing != true && b_smoke_1_overheat == true && b_smoke_overheat_mode[i_wand_power_level - 1] == true) {
        digitalWrite(smoke_pin, HIGH);
      }
      else {
        digitalWrite(smoke_pin, LOW);
      }
    }
    else {
      digitalWrite(smoke_pin, LOW);
    }

    smokeBooster(b_smoke_on);
  }
}

// Smoke #2. Good for putting smoke in the Booster Tube.
void smokeBooster(bool b_smoke_on) {
  if(b_smoke_enabled == true) {
    if(b_smoke_on == true) {
      if(b_wand_firing == true && b_overheating != true && b_smoke_2_continuous_firing == true && b_smoke_continuous_mode[i_wand_power_level - 1] == true) {
        digitalWrite(smoke_booster_pin, HIGH);
      }
      else if(b_overheating == true && b_smoke_2_overheat == true && b_wand_firing != true && b_smoke_overheat_mode[i_wand_power_level - 1] == true) {
        digitalWrite(smoke_booster_pin, HIGH);
      }
      else {
        digitalWrite(smoke_booster_pin, LOW);
      }
    }
    else {
      digitalWrite(smoke_booster_pin, LOW);
    }
  }
}

void fanBooster(bool b_fan_on) {
  if(b_smoke_enabled == true) {
    if(b_fan_on == true) {
      if(b_wand_firing == true && b_overheating != true && b_fan_booster_continuous_firing == true && b_smoke_continuous_mode[i_wand_power_level - 1] == true) {
        digitalWrite(fan_booster_pin, HIGH);
      }
      else if(b_overheating == true && b_wand_firing != true && b_fan_booster_overheat == true && b_smoke_overheat_mode[i_wand_power_level - 1] == true) {
        digitalWrite(fan_booster_pin, HIGH);
      }
      else {
        digitalWrite(fan_booster_pin, LOW);
      }
    }
    else {
      digitalWrite(fan_booster_pin, LOW);
    }
  }
}

// N-Filter Fan.
// Fan control. You can use this to switch on any device when properly hooked up with a transistor etc.
// A fan is a good idea for the N-Filter for example.
void fanControl(bool b_fan_on) {
  if(b_smoke_enabled == true) {
    if(b_fan_on == true) {
      if(b_wand_firing == true && b_overheating != true && b_fan_continuous_firing == true && b_smoke_continuous_mode[i_wand_power_level - 1] == true) {
        digitalWrite(fan_pin, HIGH);
      }
      else if(b_overheating == true && b_wand_firing != true && b_fan_overheat == true && b_smoke_overheat_mode[i_wand_power_level - 1] == true) {
        digitalWrite(fan_pin, HIGH);
      }
      else {
        digitalWrite(fan_pin, LOW);
      }
    }
    else {
      digitalWrite(fan_pin, LOW);
    }
  }
}

// Check if the Attenuator is still connected.
void serial1HandShake() {
  if(b_serial1_connected == true) {
    if(ms_serial1_handshake.justFinished()) {
      ms_serial1_handshake.start(i_serial1_handshake_delay);

      b_serial1_connected = false;

      // Where are you Attenuator?
      serial1Send(A_HANDSHAKE);
    }
    else if(ms_serial1_handshake_checking.justFinished()) {
      ms_serial1_handshake_checking.stop();

      // Ask the Attenuator if it is still connected.
      serial1Send(A_HANDSHAKE);
    }
  }
  else {
    if(ms_serial1_handshake.justFinished()) {
      // Ask the Attenuator if it is connected.
      serial1Send(A_HANDSHAKE);

      ms_serial1_handshake.start(i_serial1_handshake_delay / 5);
    }
  }
}

// Check if the wand is still connected.
void wandHandShake() {
  if(b_wand_connected == true) {
    if(ms_wand_handshake.justFinished()) {

      if(b_wand_firing == true) {
        wandStoppedFiring();
        cyclotronSpeedRevert();
      }

      ms_wand_handshake.start(i_wand_handshake_delay);

      b_wand_connected = false;
      b_wand_on = false;

      wandExtraSoundsStop();

      if(b_spectral_lights_on == true) {
        spectralLightsOff();
      }

      // Where are you wand?
      packSerialSend(P_HANDSHAKE);
    }
    else if(ms_wand_handshake_checking.justFinished()) {
      if(b_diagnostic == true) {
        // Play a beep sound to know if the wand is connected while in diagnostic mode.
        playEffect(S_VENT_BEEP, true);
      }

      ms_wand_handshake_checking.stop();

      // Ask the wand if it is still connected.
      packSerialSend(P_HANDSHAKE);
    }
  }
  else {
    b_wand_on = false;

    if(b_wand_firing == true) {
      wandStoppedFiring();
      cyclotronSpeedRevert();
    }

    // Turn off overheating if the wand gets disconnected.
    /*
    if(b_overheating == true) {
      packOverHeatingFinished();
    }
    */

    if(b_spectral_lights_on == true) {
      spectralLightsOff();
    }

    if(ms_wand_handshake.justFinished()) {
      // Ask the wand if it is connected.
      packSerialSend(P_HANDSHAKE);

      ms_wand_handshake.start(i_wand_handshake_delay / 5);
    }
  }
}

void wandExtraSoundsStop() {
  stopEffect(S_AFTERLIFE_WAND_RAMP_1);
  stopEffect(S_AFTERLIFE_WAND_IDLE_1);

  stopEffect(S_AFTERLIFE_WAND_RAMP_2);
  stopEffect(S_AFTERLIFE_WAND_IDLE_2);
  stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_1);
  stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2);

  stopEffect(S_AFTERLIFE_WAND_RAMP_2_FADE_IN);
  stopEffect(S_AFTERLIFE_WAND_RAMP_DOWN_2_FADE_OUT);
}

// Incoming messages from the extra Serial 1 port.
void checkSerial1() {
  while(serial1Coms.available() > 0) {
    serial1Coms.rxObj(dataStructR);

    if(!serial1Coms.currentPacketID()) {
      if(dataStructR.i > 0 && dataStructR.s == A_COM_START && dataStructR.e == A_COM_END) {
        if(b_serial1_connected == true) {
          switch(dataStructR.i) {
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

            case A_SYNC_START:
              //Serial.println("Serial1 Sync Start");
            break;

            case A_SYNC_END:
              //Serial.println("Serial1 Sync End");
            break;

            default:
              // Music track number to be played.
              if(i_music_count > 0 && dataStructR.i >= i_music_track_start) {
                if(b_playing_music == true) {
                  stopMusic(); // Stops current track before change.

                  // Only update after the music is stopped.
                  i_current_music_track = dataStructR.i;

                  // Play the appropriate track on pack and wand, and notify the serial1 device.
                  playMusic();
                }
                else {
                  i_current_music_track = dataStructR.i;

                  // Tell the wand which track to play.
                  packSerialSend(i_current_music_track);
                }
              }
            break;
          }
        }
        else {
          // Check if the Attenuator is telling us it is here after connecting it to the pack.
          // Then synchronise some settings between the pack and the Attenuator.
          if(dataStructR.i == A_SYNC_START && b_serial_1_syncing != true) {
            b_serial_1_syncing = true;

            serial1Send(A_SYNC_START);

            // Tell the Attenuator that the pack is here.
            serial1Send(A_PACK_CONNECTED);

            if(SYSTEM_YEAR == SYSTEM_1984) {
              serial1Send(A_YEAR_1984);
            }
            else if(SYSTEM_YEAR == SYSTEM_1989) {
              serial1Send(A_YEAR_1989);
            }
            /*
            else if(SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
              serial1Send(A_YEAR_FROZEN_EMPIRE);
            }
            */
            else if(SYSTEM_YEAR == SYSTEM_AFTERLIFE) {
              serial1Send(A_YEAR_AFTERLIFE);
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
                serial1Send(A_SPECTRAL_CUSTOM_MODE);
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

            serial1Send(A_SPECTRAL_COLOUR_DATA);

            if(SYSTEM_MODE == MODE_SUPER_HERO) {
              serial1Send(A_MODE_SUPER_HERO);
            }
            else {
              serial1Send(A_MODE_ORIGINAL);
            }

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
              serial1Send(A_MUSIC_IS_PLAYING);
            }
            else {
              serial1Send(A_MUSIC_IS_NOT_PLAYING);
            }

            if(b_music_paused == true) {
              serial1Send(A_MUSIC_IS_PAUSED);
            }
            else {
              serial1Send(A_MUSIC_IS_NOT_PAUSED);
            }

            serial1Send(A_MUSIC_TRACK_COUNT_SYNC);

            b_serial1_connected = true;
            b_serial_1_syncing = false;

            ms_serial1_handshake.start(i_serial1_handshake_delay);
            ms_serial1_handshake_checking.start(i_serial1_handshake_delay / 2);

            serial1Send(A_SYNC_END);
          }
        }
      }
    }
  }
}

// Incoming messages from the wand.
void checkWand() {
  while(packComs.available() > 0) {
    packComs.rxObj(comStruct);

    if(!packComs.currentPacketID()) {
      if(comStruct.i > 0 && comStruct.s == W_COM_START && comStruct.e == W_COM_END) {
        if(b_wand_connected == true) {
          switch(comStruct.i) {
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

              serial1Send(A_SPECTRAL_CUSTOM_MODE);
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
              i_volume_revert = i_volume_master;

              // Set the master volume to silent.
              i_volume_master = i_volume_abs_min;

              w_trig.masterGain(i_volume_master); // Reset the master gain.
            break;

            case W_VOLUME_REVERT:
              // Set the master volume to silent.
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
              if(i_spectral_cyclotron_inner_custom > 1 && i_spectral_cyclotron_inner_custom_saturation > 253) {
                i_spectral_cyclotron_inner_custom--;
              }
              else {
                i_spectral_cyclotron_inner_custom = 1;

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
              if(i_spectral_cyclotron_custom > 1 && i_spectral_cyclotron_custom_saturation > 253) {
                i_spectral_cyclotron_custom--;
              }
              else {
                i_spectral_cyclotron_custom = 1;

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
              if(i_spectral_powercell_custom > 1 && i_spectral_powercell_custom_saturation > 253) {
                i_spectral_powercell_custom--;
              }
              else {
                i_spectral_powercell_custom = 1;

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
              else if(i_spectral_powercell_custom < 253) {
                i_spectral_powercell_custom++;
              }
              else {
                i_spectral_powercell_custom = 254;
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
              else if(i_spectral_cyclotron_custom < 253) {
                i_spectral_cyclotron_custom++;
              }
              else {
                i_spectral_cyclotron_custom = 254;

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
              else if(i_spectral_cyclotron_inner_custom < 253) {
                i_spectral_cyclotron_inner_custom++;
              }
              else {
                i_spectral_cyclotron_inner_custom = 254;

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

                    resetCyclotronLeds();

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

                    // Reset the Cyclotron.
                    resetCyclotronLeds();

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

            case W_CLEAR_EEPROM_SETTINGS:
              clearLedEEPROM();

              stopEffect(S_VOICE_EEPROM_ERASE);
              playEffect(S_VOICE_EEPROM_ERASE);
            break;

            case W_SAVE_EEPROM_SETTINGS:
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

                  resetCyclotronLEDs();

                  playEffect(S_VOICE_CYCLOTRON_20);
                  packSerialSend(P_CYCLOTRON_LEDS_20);
                break;

                case FRUTTO_CYCLOTRON_LED_COUNT:
                default:
                  // Switch to 12 LEDs. Default HasLab.
                  i_cyclotron_leds = HASLAB_CYCLOTRON_LED_COUNT;

                  resetCyclotronLEDs();

                  playEffect(S_VOICE_CYCLOTRON_12);
                  packSerialSend(P_CYCLOTRON_LEDS_12);
                break;

                case HASLAB_CYCLOTRON_LED_COUNT:
                  // Switch to 40 LEDs.
                  i_cyclotron_leds = OUTER_CYCLOTRON_LED_MAX;

                  resetCyclotronLEDs();

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
                //if(SYSTEM_YEAR_TEMP == SYSTEM_FROZEN_EMPIRE) {
                if(SYSTEM_YEAR_TEMP == SYSTEM_AFTERLIFE) {
                  b_switch_mode_override = false;

                  //stopEffect(S_VOICE_FROZEN_EMPIRE);
                  stopEffect(S_VOICE_YEAR_MODE_DEFAULT);
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

            default:
              // Music track number to be played.
              if(i_music_count > 0 && comStruct.i >= i_music_track_start) {
                if(b_playing_music == true) {
                  stopMusic(); // Stops current track before change.

                  i_current_music_track = comStruct.i; // Change track AFTER stopping music.

                  // Play the appropriate track on pack and wand, and notify the serial1 device.
                  playMusic();
                }
                else {
                  i_current_music_track = comStruct.i;
                }
              }
            break;
          }
        }
        else {
          // Check if the wand is telling us it is here after connecting it to the pack.
          // Then synchronise some settings between the pack and the wand.
          if(comStruct.i == W_HANDSHAKE) {
            if(b_overheating == true) {
              packOverHeatingFinished();
            }

            packSerialSend(P_SYNC_START);

            // Tell the wand that the pack is here.
            packSerialSend(P_HANDSHAKE);

            // Make sure this is called before the P_YEAR is sent over to the Neutrona Wand.
            if(SYSTEM_MODE == MODE_SUPER_HERO) {
              packSerialSend(P_MODE_SUPER_HERO);
            }
            else {
              packSerialSend(P_MODE_ORIGINAL);
            }

            if(switch_power.getState() == LOW) {
              // Tell the Neutrona Wand that power to the Proton Pack is on.
              packSerialSend(P_MODE_ORIGINAL_RED_SWITCH_ON);
            }
            else {
              // Tell the Neutrona Wand that power to the Proton Pack is off.
              packSerialSend(P_MODE_ORIGINAL_RED_SWITCH_OFF);
            }

            if(SYSTEM_YEAR == SYSTEM_1984) {
              packSerialSend(P_YEAR_1984);
            }
            else if(SYSTEM_YEAR == SYSTEM_1989) {
              packSerialSend(P_YEAR_1989);
            }
            /*
            else if(SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
              packSerialSend(P_YEAR_FROZEN_EMPIRE);
            }
            */
            else {
              packSerialSend(P_YEAR_AFTERLIFE);
            }

            // Stop any music. Mainly for when flashing while connected to a computer with a running wand.
            //packSerialSend(P_MUSIC_STOP);

            // Sync the current music track.
            // If music is already playing on a pack while a wand is reconnected, the wand will start playing music when the current track ends.
            packSerialSend(i_current_music_track);

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

            // Put the wand into volume sync mode.
            packSerialSend(P_VOLUME_SYNC_MODE);

            // Sequence here is important. Synchronise the volume settings.
            packSerialSend(i_volume_percentage);

            packSerialSend(i_volume_master_percentage);

            packSerialSend(i_volume_music_percentage);

            if(i_volume_master == i_volume_abs_min) {
              // Telling the wand to be silent if required.
              packSerialSend(P_MASTER_AUDIO_SILENT_MODE);
            }
            else {
              packSerialSend(P_MASTER_AUDIO_NORMAL);
            }

            b_wand_connected = true;

            packSerialSend(P_SYNC_END);
          }
        }
      }
      else if(comStruct.i > 0 && comStruct.s == W_COM_START && comStruct.e == W_COM_SOUND_NUMBER) {
        // The Neutrona Wand is telling us to play a # number sound effect only. S_1 through S_60).
        stopEffect(comStruct.i + 1);

        if(comStruct.i - 1 > 0) {
          stopEffect(comStruct.i - 1);
        }

        stopEffect(comStruct.i);

        playEffect(comStruct.i);
      }
    }
  }
}

void serial1Send(int i_message) {
  dataStruct.s = A_COM_START;
  dataStruct.i = i_message;

  if(i_message == A_SPECTRAL_CUSTOM_MODE || i_message == A_SPECTRAL_COLOUR_DATA) {
    dataStruct.d1 = i_spectral_cyclotron_custom;
    dataStruct.d2 = i_spectral_cyclotron_custom_saturation;
  }
  else if(i_message == A_MUSIC_TRACK_COUNT_SYNC) {
    dataStruct.d1 = i_music_count;
  }
  else if(i_message == A_MUSIC_IS_PLAYING || i_message == A_MUSIC_IS_NOT_PLAYING) {
    dataStruct.d1 = i_current_music_track;
  }

  dataStruct.e = A_COM_END;

  serial1Coms.sendDatum(dataStruct);
}

void packSerialSend(int i_message) {
  sendStruct.s = P_COM_START;
  sendStruct.i = i_message;
  sendStruct.e = P_COM_END;

  packComs.sendDatum(sendStruct);
}

// It is very important that S_1 up to S_60 follow each other in order on the Micro SD Card and sound effects enum.
void overheatVoiceIndicator(unsigned int i_tmp_length) {
  i_tmp_length = i_tmp_length / i_overheat_delay_increment;

  unsigned int i_tmp_sound = (S_1 - 1) + i_tmp_length;

  stopEffect(i_tmp_sound - 1);
  stopEffect(i_tmp_sound);
  stopEffect(i_tmp_sound + 1);
  playEffect(i_tmp_sound);
}

void overheatIncrement(uint8_t i_tmp_power_level) {
  switch(i_tmp_power_level) {
    case 5:
      if(i_ms_overheating_length_5 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheating_length_5 = i_ms_overheating_length_5 + i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_5);
      }
    break;

    case 4:
      if(i_ms_overheating_length_4 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheating_length_4 = i_ms_overheating_length_4 + i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_4);
      }
    break;

    case 3:
      if(i_ms_overheating_length_3 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheating_length_3 = i_ms_overheating_length_3 + i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_3);
      }
    break;

    case 2:
      if(i_ms_overheating_length_2 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheating_length_2 = i_ms_overheating_length_2 + i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_2);
      }
    break;

    case 1:
    default:
      if(i_ms_overheating_length_1 + i_overheat_delay_increment <= i_overheat_delay_max) {
        i_ms_overheating_length_1 = i_ms_overheating_length_1 + i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_1);
      }
    break;
  }
}

void overheatDecrement(uint8_t i_tmp_power_level) {
  switch(i_tmp_power_level) {
    case 5:
      if(i_ms_overheating_length_5 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheating_length_5 = i_ms_overheating_length_5 - i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_5);
      }
    break;

    case 4:
      if(i_ms_overheating_length_4 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheating_length_4 = i_ms_overheating_length_4 - i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_4);
      }
    break;

    case 3:
      if(i_ms_overheating_length_3 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheating_length_3 = i_ms_overheating_length_3 - i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_3);
      }
    break;

    case 2:
      if(i_ms_overheating_length_2 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheating_length_2 = i_ms_overheating_length_2 - i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_2);
      }
    break;

    case 1:
    default:
      if(i_ms_overheating_length_1 - i_overheat_delay_increment >= i_overheat_delay_increment * 2) {
        i_ms_overheating_length_1 = i_ms_overheating_length_1 - i_overheat_delay_increment;

        overheatVoiceIndicator(i_ms_overheating_length_1);
      }
    break;
  }
}

// Update the LED counts for the Proton Pack.
void updateProtonPackLEDCounts() {
  i_pack_num_leds = i_powercell_leds + i_cyclotron_leds + i_nfilter_jewel_leds;
  i_vent_light_start = i_powercell_leds + i_cyclotron_leds;
  cyclotron_led_start = i_powercell_leds;
}

void resetCyclotronLEDs() {
  switch(i_cyclotron_leds) {
    // For a 40 LED Neopixel ring.
    case OUTER_CYCLOTRON_LED_MAX:
      i_2021_delay = CYCLOTRON_DELAY_2021_40_LED;
      i_1984_cyclotron_leds[0] = i_1984_cyclotron_40_leds[0];
      i_1984_cyclotron_leds[1] = i_1984_cyclotron_40_leds[1];
      i_1984_cyclotron_leds[2] = i_1984_cyclotron_40_leds[2];
      i_1984_cyclotron_leds[3] = i_1984_cyclotron_40_leds[3];
    break;

    // For Frutto Technology Cyclotron LEDs.
    case FRUTTO_CYCLOTRON_LED_COUNT:
      i_2021_delay = CYCLOTRON_DELAY_2021_20_LED;
      i_1984_cyclotron_leds[0] = i_1984_cyclotron_20_leds[0];
      i_1984_cyclotron_leds[1] = i_1984_cyclotron_20_leds[1];
      i_1984_cyclotron_leds[2] = i_1984_cyclotron_20_leds[2];
      i_1984_cyclotron_leds[3] = i_1984_cyclotron_20_leds[3];
    break;

    // Default HasLab LEDs.
    case HASLAB_CYCLOTRON_LED_COUNT:
    default:
      i_2021_delay = CYCLOTRON_DELAY_2021_12_LED;
      i_1984_cyclotron_leds[0] = i_1984_cyclotron_12_leds[0];
      i_1984_cyclotron_leds[1] = i_1984_cyclotron_12_leds[1];
      i_1984_cyclotron_leds[2] = i_1984_cyclotron_12_leds[2];
      i_1984_cyclotron_leds[3] = i_1984_cyclotron_12_leds[3];
    break;
  }
}

// Adjusts which year mode the Proton Pack and Neutrona Wand are in if switched by the Neutrona Wand.
void toggleYearModes() {
  // Toggle between the year modes.
  stopEffect(S_BEEPS_BARGRAPH);
  playEffect(S_BEEPS_BARGRAPH);

  switch(SYSTEM_YEAR_TEMP) {
    case SYSTEM_1984:
      SYSTEM_YEAR_TEMP = SYSTEM_1989;

      //stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1984);
      stopEffect(S_VOICE_1989);

      playEffect(S_VOICE_1989);

      // Tell the wand to play the 1989 sound effect.
      packSerialSend(P_MODE_1989);
    break;

    case SYSTEM_1989:
      SYSTEM_YEAR_TEMP = SYSTEM_AFTERLIFE;

      //stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1984);
      stopEffect(S_VOICE_1989);

      playEffect(S_VOICE_AFTERLIFE);

      // Tell the wand to play the Afterlife sound effect.
      packSerialSend(P_MODE_AFTERLIFE);
    break;

    case SYSTEM_AFTERLIFE:
      SYSTEM_YEAR_TEMP = SYSTEM_1984;
      //SYSTEM_YEAR_TEMP = SYSTEM_FROZEN_EMPIRE;

      //stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1984);
      stopEffect(S_VOICE_1989);

      playEffect(S_VOICE_1984);

      // Tell the wand to play the 1984 sound effect.
      packSerialSend(P_MODE_1984);

      //packSerialSend(P_MODE_FROZEN_EMPIRE);
    break;

    /*
    case SYSTEM_FROZEN_EMPIRE:
      SYSTEM_YEAR_TEMP = SYSTEM_FROZEN_EMPIRE;

      stopEffect(S_VOICE_FROZEN_EMPIRE);
      stopEffect(S_VOICE_AFTERLIFE);
      stopEffect(S_VOICE_1984);
      stopEffect(S_VOICE_1989);

      playEffect(S_VOICE_1984);

      // Tell the wand to play the 1984 sound effect.
      packSerialSend(P_MODE_1984);
    break;
    */

    default:
      // Nothing.
    break;
  }
}

// Helper method to play a sound effect using certain defaults.
void playEffect(int i_track_id, bool b_track_loop, int8_t i_track_volume, bool b_fade_in, unsigned int i_fade_time) {
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
  else {
    w_trig.trackLoop(i_track_id, 0);
  }
}

void stopEffect(int i_track_id) {
  w_trig.trackStop(i_track_id);
}

// Adjust the gain of a single track.
void adjustGainEffect(int i_track_id, int8_t i_track_volume, bool b_fade, unsigned int i_fade_time) {
  if(i_track_volume < i_volume_abs_min) {
    i_track_volume = i_volume_abs_min;
  }

  if(i_track_volume > 10) {
    i_track_volume = i_volume_abs_max;
  }

  if(b_fade == true) {
    w_trig.trackFade(i_track_id, i_track_volume, i_fade_time, 0);
  }
  else {
    w_trig.trackGain(i_track_id, i_track_volume);
  }
}

// Helper method to play a music track using certain defaults.
void playMusic() {
  if(b_music_paused != true) {
    b_playing_music = true;

    // Loop the music track.
    if(b_repeat_track == true) {
      w_trig.trackLoop(i_current_music_track, 1);
    }
    else {
      w_trig.trackLoop(i_current_music_track, 0);
    }

    w_trig.trackGain(i_current_music_track, i_volume_music);
    w_trig.trackPlayPoly(i_current_music_track, true);

    w_trig.update();

    ms_music_status_check.start(i_music_check_delay * 10);
    w_trig.resetTrackCounter(true);

    // Tell the Neutrona Wand which music track to change to and play it.
    packSerialSend(i_current_music_track);
    packSerialSend(P_MUSIC_START);

    // Tell connected serial device music playback has started.
    serial1Send(A_MUSIC_IS_PLAYING);
    serial1Send(A_MUSIC_IS_NOT_PAUSED);
  }
}

void stopMusic() {
  w_trig.trackStop(i_current_music_track);
  w_trig.update();

  b_music_paused = false;
  b_playing_music = false;

  // Tell the Neutrona Wand to stop music playback.
  packSerialSend(P_MUSIC_STOP);

  // Tell connected serial device music playback has stopped.
  serial1Send(A_MUSIC_IS_NOT_PLAYING);
  serial1Send(A_MUSIC_IS_NOT_PAUSED);
}

void pauseMusic() {
  if(b_playing_music == true) {
    // Pause music playback on the Proton Pack
    w_trig.trackPause(i_current_music_track);
    w_trig.update();
    b_music_paused = true;

    // Tell connected devices music playback is paused.
    packSerialSend(P_MUSIC_PAUSE);
    serial1Send(A_MUSIC_IS_PAUSED);
  }
}

void resumeMusic() {
  if(b_playing_music == true) {
    // Reset the music check timer.
    ms_music_status_check.start(i_music_check_delay * 10);
    w_trig.resetTrackCounter(true);

    // Resume music playback on the Proton Pack
    w_trig.trackResume(i_current_music_track);
    w_trig.update();
    b_music_paused = false;

    // Tell connected devices music playback is resumed.
    packSerialSend(P_MUSIC_RESUME);
    serial1Send(A_MUSIC_IS_NOT_PAUSED);
  }
}

void musicNextTrack() {
  unsigned int i_temp_track = i_current_music_track; // Used for music navigation.

  // Determine the next track.
  if(i_current_music_track + 1 > i_music_track_start + i_music_count - 1) {
    // Start at the first track if already on the last.
    i_temp_track = i_music_track_start;
  }
  else {
    i_temp_track++;
  }

  // Switch to the next track.
  if(b_playing_music == true) {
    // Stops music using the current track number as the identifier.
    stopMusic();

    i_current_music_track = i_temp_track; // Change only AFTER stopping music playback.

    // Play the appropriate track on pack and wand, and notify the serial1 device.
    playMusic();
  }
  else {
    // Set the new track.
    i_current_music_track = i_temp_track;

    serial1Send(A_MUSIC_IS_NOT_PLAYING); // Updates the music track on the attenuator.
  }
}

void musicPrevTrack() {
  unsigned int i_temp_track = i_current_music_track; // Used for music navigation.

  // Determine the previous track.
  if(i_current_music_track - 1 < i_music_track_start) {
    // Start at the last track if already on the first.
    i_temp_track = i_music_track_start + (i_music_count - 1);
  }
  else {
    i_temp_track--;
  }

  // Switch to the previous track.
  if(b_playing_music == true) {
    // Stops music using the current track number as the identifier.
    stopMusic();

    i_current_music_track = i_temp_track; // Change only AFTER stopping music playback.

    // Play the appropriate track on pack and wand, and notify the serial1 device.
    playMusic();
  }
  else {
    // Set the new track.
    i_current_music_track = i_temp_track;

    serial1Send(A_MUSIC_IS_NOT_PLAYING); // Updates the music track on the attenuator.
  }
}

void resetContinuousSmoke() {
  b_smoke_continuous_mode[0] = b_smoke_continuous_mode_1;
  b_smoke_continuous_mode[1] = b_smoke_continuous_mode_2;
  b_smoke_continuous_mode[2] = b_smoke_continuous_mode_3;
  b_smoke_continuous_mode[3] = b_smoke_continuous_mode_4;
  b_smoke_continuous_mode[4] = b_smoke_continuous_mode_5;
}

void readEEPROM() {
  // Get the stored CRC from the EEPROM.
  unsigned long l_crc_check;
  EEPROM.get(EEPROM.length() - sizeof(l_crc_size), l_crc_check);

  // Check if the calculated CRC matches the stored CRC value in the EEPROM.
  if(eepromCRC() == l_crc_check) {
    // Read our object from the EEPROM.
    objEEPROM obj_eeprom;
    EEPROM.get(i_eepromAddress, obj_eeprom);

    if(obj_eeprom.powercell_count > 0 && obj_eeprom.powercell_count != 255) {
      i_powercell_leds = obj_eeprom.powercell_count;

      switch(i_powercell_leds) {
        case HASLAB_POWERCELL_LED_COUNT:
          // 15 Power Cell LEDs.
          i_powercell_delay_1984 = 60;
          i_powercell_delay_2021 = 34;
        break;

        case FRUTTO_POWERCELL_LED_COUNT:
        default:
          // 13 Power Cell LEDs.
          i_powercell_delay_1984 = 75;
          i_powercell_delay_2021 = 40;
        break;
      }
    }

    if(obj_eeprom.cyclotron_count > 0 && obj_eeprom.cyclotron_count != 255) {
      i_cyclotron_leds = obj_eeprom.cyclotron_count;
      resetCyclotronLEDs();
    }

    if(obj_eeprom.inner_cyclotron_count > 0 && obj_eeprom.inner_cyclotron_count != 255) {
      i_inner_cyclotron_num_leds = obj_eeprom.inner_cyclotron_count;

      switch(i_inner_cyclotron_num_leds) {
        case 12:
          i_2021_inner_delay = 12;
          i_1984_inner_delay = 15;
        break;

        case 23:
        case 24:
          i_2021_inner_delay = 8;
          i_1984_inner_delay = 12;
        break;

        case 35:
        default:
          i_2021_inner_delay = 5;
          i_1984_inner_delay = 9;
        break;
      }
    }

    if(obj_eeprom.grb_inner_cyclotron > 0 && obj_eeprom.grb_inner_cyclotron != 255) {
      if(obj_eeprom.grb_inner_cyclotron > 1) {
        b_grb_cyclotron = true;
      }
      else {
        b_grb_cyclotron = false;
      }
    }

    if(obj_eeprom.powercell_spectral_custom > 0 && obj_eeprom.powercell_spectral_custom != 255) {
      i_spectral_powercell_custom = obj_eeprom.powercell_spectral_custom;
    }

    if(obj_eeprom.cyclotron_spectral_custom > 0 && obj_eeprom.cyclotron_spectral_custom != 255) {
      i_spectral_cyclotron_custom = obj_eeprom.cyclotron_spectral_custom;
    }

    if(obj_eeprom.cyclotron_inner_spectral_custom > 0 && obj_eeprom.cyclotron_inner_spectral_custom != 255) {
      i_spectral_cyclotron_inner_custom = obj_eeprom.cyclotron_inner_spectral_custom;
    }

    if(obj_eeprom.powercell_spectral_saturation_custom > 0 && obj_eeprom.powercell_spectral_saturation_custom != 255) {
      i_spectral_powercell_custom_saturation = obj_eeprom.powercell_spectral_saturation_custom;
    }

    if(obj_eeprom.cyclotron_spectral_saturation_custom > 0 && obj_eeprom.cyclotron_spectral_saturation_custom != 255) {
      i_spectral_cyclotron_custom_saturation = obj_eeprom.cyclotron_spectral_saturation_custom;
    }

    if(obj_eeprom.cyclotron_inner_spectral_saturation_custom > 0 && obj_eeprom.cyclotron_inner_spectral_saturation_custom != 255) {
      i_spectral_cyclotron_inner_custom_saturation = obj_eeprom.cyclotron_inner_spectral_saturation_custom;
    }

    // Update the LED counts for the Proton Pack.
    updateProtonPackLEDCounts();

    // Read our configuration object from the EEPROM.
    objConfigEEPROM obj_config_eeprom;
    unsigned int i_eepromConfigAddress = EEPROM.length() / 2;

    EEPROM.get(i_eepromConfigAddress, obj_config_eeprom);

    if(obj_config_eeprom.stream_effects > 0 && obj_config_eeprom.stream_effects != 255) {
      if(obj_config_eeprom.stream_effects > 1) {
        b_stream_effects = true;
      }
      else {
        b_stream_effects = false;
      }
    }

    if(obj_config_eeprom.simulate_ring > 0 && obj_config_eeprom.simulate_ring != 255) {
      if(obj_config_eeprom.simulate_ring > 1) {
        b_cyclotron_simulate_ring = true;
      }
      else {
        b_cyclotron_simulate_ring = false;
      }
    }

    if(obj_config_eeprom.cyclotron_direction > 0 && obj_config_eeprom.cyclotron_direction != 255) {
      if(obj_config_eeprom.cyclotron_direction > 1) {
        b_clockwise = true;
      }
      else {
        b_clockwise = false;
      }
    }

    if(obj_config_eeprom.smoke_setting > 0 && obj_config_eeprom.smoke_setting != 255) {
      if(obj_config_eeprom.smoke_setting > 1) {
        b_smoke_enabled = true;
      }
      else {
        b_smoke_enabled = false;
      }
    }

    if(obj_config_eeprom.overheat_strobe > 0 && obj_config_eeprom.overheat_strobe != 255) {
      if(obj_config_eeprom.overheat_strobe > 1) {
        b_overheat_strobe = true;
      }
      else {
        b_overheat_strobe = false;
      }
    }

    if(obj_config_eeprom.overheat_lights_off > 0 && obj_config_eeprom.overheat_lights_off != 255) {
      if(obj_config_eeprom.overheat_lights_off > 1) {
        b_overheat_lights_off = true;
      }
      else {
        b_overheat_lights_off = false;
      }
    }

    if(obj_config_eeprom.overheat_sync_to_fan > 0 && obj_config_eeprom.overheat_sync_to_fan != 255) {
      if(obj_config_eeprom.overheat_sync_to_fan > 1) {
        b_overheat_sync_to_fan = true;
      }
      else {
        b_overheat_sync_to_fan = false;
      }
    }

    if(obj_config_eeprom.year_mode > 0 && obj_config_eeprom.year_mode != 255) {
      if(obj_config_eeprom.year_mode > 1) {
        // 1 = toggle switch, 2 = 1984, 3 = 1989, 4 = Afterlife, 5 = Frozen Empire.
        switch(obj_config_eeprom.year_mode) {
          case 2:
            SYSTEM_YEAR = SYSTEM_1984;
          break;

          case 3:
            SYSTEM_YEAR = SYSTEM_1989;
          break;

          /*
          case 5:
            SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
          break;
          */
          case 4:
          default:
            SYSTEM_YEAR = SYSTEM_AFTERLIFE;
          break;
        }

        SYSTEM_YEAR_TEMP = SYSTEM_YEAR;
        SYSTEM_EEPROM_YEAR = SYSTEM_YEAR;

        // Set the switch override to true, so the toggle switch in the Proton Pack does not override the year settings during the bootup process.
        b_switch_mode_override = true;
      }
    }
    else {
      // 1 = toggle switch, 2 = 1984, 3 = 1989, 4 = Afterlife, 5 = Frozen Empire.
      SYSTEM_EEPROM_YEAR = SYSTEM_TOGGLE_SWITCH;
    }

    if(obj_config_eeprom.system_mode > 0 && obj_config_eeprom.system_mode != 255) {
      if(obj_config_eeprom.system_mode > 1) {
        SYSTEM_MODE = MODE_ORIGINAL;
      }
      else {
        SYSTEM_MODE = MODE_SUPER_HERO;
      }
    }

    if(obj_config_eeprom.vga_powercell > 0 && obj_config_eeprom.vga_powercell != 255) {
      if(obj_config_eeprom.vga_powercell > 1) {
        b_powercell_colour_toggle = true;
      }
      else {
        b_powercell_colour_toggle = false;
      }
    }

    if(obj_config_eeprom.vga_cyclotron > 0 && obj_config_eeprom.vga_cyclotron != 255) {
      if(obj_config_eeprom.vga_cyclotron > 1) {
        b_cyclotron_colour_toggle = true;
      }
      else {
        b_cyclotron_colour_toggle = false;
      }
    }

    if(obj_config_eeprom.demo_light_mode > 0 && obj_config_eeprom.demo_light_mode != 255) {
      if(obj_config_eeprom.demo_light_mode > 1) {
        b_demo_light_mode = true;
      }
      else {
        b_demo_light_mode = false;
      }
    }

    if(obj_config_eeprom.cyclotron_three_led_toggle > 0 && obj_config_eeprom.cyclotron_three_led_toggle != 255) {
      if(obj_config_eeprom.cyclotron_three_led_toggle > 1) {
        b_cyclotron_single_led = false;
      }
      else {
        b_cyclotron_single_led = true;
      }
    }

    if(obj_config_eeprom.default_system_volume > 0 && obj_config_eeprom.default_system_volume != 255) {
      i_volume_master_percentage = obj_config_eeprom.default_system_volume;
      i_volume_master_eeprom = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
      i_volume_revert = i_volume_master_eeprom;
      i_volume_master = i_volume_master_eeprom;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_5 > 0 && obj_config_eeprom.overheat_smoke_duration_level_5 != 255) {
      i_ms_overheating_length_5 = obj_config_eeprom.overheat_smoke_duration_level_5 * 1000;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_4 > 0 && obj_config_eeprom.overheat_smoke_duration_level_4 != 255) {
      i_ms_overheating_length_4 = obj_config_eeprom.overheat_smoke_duration_level_4 * 1000;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_3 > 0 && obj_config_eeprom.overheat_smoke_duration_level_3 != 255) {
      i_ms_overheating_length_3 = obj_config_eeprom.overheat_smoke_duration_level_3 * 1000;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_2 > 0 && obj_config_eeprom.overheat_smoke_duration_level_2 != 255) {
      i_ms_overheating_length_2 = obj_config_eeprom.overheat_smoke_duration_level_2 * 1000;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_1 > 0 && obj_config_eeprom.overheat_smoke_duration_level_1 != 255) {
      i_ms_overheating_length_1 = obj_config_eeprom.overheat_smoke_duration_level_1 * 1000;
    }

    if(obj_config_eeprom.smoke_continuous_mode_5 > 0 && obj_config_eeprom.smoke_continuous_mode_5 != 255) {
      if(obj_config_eeprom.smoke_continuous_mode_5 > 1) {
        b_smoke_continuous_mode_5 = true;
      }
      else {
        b_smoke_continuous_mode_5 = false;
      }
    }

    if(obj_config_eeprom.smoke_continuous_mode_4 > 0 && obj_config_eeprom.smoke_continuous_mode_4 != 255) {
      if(obj_config_eeprom.smoke_continuous_mode_4 > 1) {
        b_smoke_continuous_mode_4 = true;
      }
      else {
        b_smoke_continuous_mode_4 = false;
      }
    }

    if(obj_config_eeprom.smoke_continuous_mode_3 > 0 && obj_config_eeprom.smoke_continuous_mode_3 != 255) {
      if(obj_config_eeprom.smoke_continuous_mode_3 > 1) {
        b_smoke_continuous_mode_3 = true;
      }
      else {
        b_smoke_continuous_mode_3 = false;
      }
    }

    if(obj_config_eeprom.smoke_continuous_mode_2 > 0 && obj_config_eeprom.smoke_continuous_mode_2 != 255) {
      if(obj_config_eeprom.smoke_continuous_mode_2 > 1) {
        b_smoke_continuous_mode_2 = true;
      }
      else {
        b_smoke_continuous_mode_2 = false;
      }
    }

    if(obj_config_eeprom.smoke_continuous_mode_1 > 0 && obj_config_eeprom.smoke_continuous_mode_1 != 255) {
      if(obj_config_eeprom.smoke_continuous_mode_1 > 1) {
        b_smoke_continuous_mode_1 = true;
      }
      else {
        b_smoke_continuous_mode_1 = false;
      }
    }
  }

  resetContinuousSmoke();
}

void clearConfigEEPROM() {
  // Clear out the EEPROM data for the configuration settings only.
  unsigned int i_eepromConfigAddress = EEPROM.length() / 2;

  for(unsigned int i = 0 ; i < sizeof(objConfigEEPROM); i++) {
    EEPROM.put(i_eepromConfigAddress, 0);

    i_eepromConfigAddress++;
  }

  updateCRCEEPROM();
}

void saveConfigEEPROM() {
  // 1 = false, 2 = true;
  uint8_t i_proton_stream_effects = 2;
  uint8_t i_simulate_ring = 2;
  uint8_t i_cyclotron_direction = 2;
  uint8_t i_smoke_settings = 2;

  uint8_t i_overheat_strobe = 2;
  uint8_t i_overheat_lights_off = 2;
  uint8_t i_overheat_sync_to_fan = 2;
  uint8_t i_year_mode_eeprom = SYSTEM_EEPROM_YEAR;
  uint8_t i_system_mode = 1;

  uint8_t i_vga_powercell = 1;
  uint8_t i_vga_cyclotron = 1;
  uint8_t i_demo_light_mode = 1;
  uint8_t i_cyclotron_three_led_toggle = 1; // 1 = single led. 2 = three leds.
  uint8_t i_default_system_volume = 100; // <- i_volume_master_percentage
  uint8_t i_overheat_smoke_duration_level_5 = i_ms_overheating_length_5 / 1000;
  uint8_t i_overheat_smoke_duration_level_4 = i_ms_overheating_length_4 / 1000;
  uint8_t i_overheat_smoke_duration_level_3 = i_ms_overheating_length_3 / 1000;
  uint8_t i_overheat_smoke_duration_level_2 = i_ms_overheating_length_2 / 1000;
  uint8_t i_overheat_smoke_duration_level_1 = i_ms_overheating_length_1 / 1000;

  uint8_t i_smoke_continuous_mode_5 = 1;
  uint8_t i_smoke_continuous_mode_4 = 1;
  uint8_t i_smoke_continuous_mode_3 = 1;
  uint8_t i_smoke_continuous_mode_2 = 1;
  uint8_t i_smoke_continuous_mode_1 = 1;

  if(b_stream_effects != true) {
    i_proton_stream_effects = 1;
  }

  if(b_cyclotron_simulate_ring != true) {
    i_simulate_ring = 1;
  }

  if(b_clockwise != true) {
    i_cyclotron_direction = 1;
  }

  if(b_smoke_enabled != true) {
    i_smoke_settings = 1;
  }

  if(b_overheat_strobe != true) {
    i_overheat_strobe = 1;
  }

  if(b_overheat_lights_off != true) {
    i_overheat_lights_off = 1;
  }

  if(b_overheat_sync_to_fan != true) {
    i_overheat_sync_to_fan = 1;
  }

  if(SYSTEM_MODE == MODE_ORIGINAL) {
    i_system_mode = 2;
  }

  if(b_powercell_colour_toggle == true) {
    i_vga_powercell = 2; // 1 = false, 2 = true;
  }

  if(b_cyclotron_colour_toggle == true) {
    i_vga_cyclotron = 2;
  }

  if(b_demo_light_mode == true) {
    i_demo_light_mode = 2;
  }

  if(b_cyclotron_single_led != true) {
    i_cyclotron_three_led_toggle = 2;
  }

  if(i_volume_master_percentage > 0 && i_volume_master_percentage < 255) {
    i_default_system_volume = i_volume_master_percentage;
  }

  if(b_smoke_continuous_mode_5 == true) {
    i_smoke_continuous_mode_5 = 2;
  }

  if(b_smoke_continuous_mode_4 == true) {
    i_smoke_continuous_mode_4 = 2;
  }

  if(b_smoke_continuous_mode_3 == true) {
    i_smoke_continuous_mode_3 = 2;
  }

  if(b_smoke_continuous_mode_2 == true) {
    i_smoke_continuous_mode_2 = 2;
  }

  if(b_smoke_continuous_mode_1 == true) {
    i_smoke_continuous_mode_1 = 2;
  }

  unsigned int i_eepromConfigAddress = EEPROM.length() / 2;

  objConfigEEPROM obj_eeprom = {
    i_proton_stream_effects,
    i_cyclotron_direction,
    i_simulate_ring,
    i_smoke_settings,
    i_overheat_strobe,
    i_overheat_lights_off,
    i_overheat_sync_to_fan,
    i_year_mode_eeprom,
    i_system_mode,
    i_vga_powercell,
    i_vga_cyclotron,
    i_demo_light_mode,
    i_cyclotron_three_led_toggle,
    i_default_system_volume,
    i_overheat_smoke_duration_level_5,
    i_overheat_smoke_duration_level_4,
    i_overheat_smoke_duration_level_3,
    i_overheat_smoke_duration_level_2,
    i_overheat_smoke_duration_level_1,
    i_smoke_continuous_mode_5,
    i_smoke_continuous_mode_4,
    i_smoke_continuous_mode_3,
    i_smoke_continuous_mode_2,
    i_smoke_continuous_mode_1
  };

  // Save to the EEPROM.
  EEPROM.put(i_eepromConfigAddress, obj_eeprom);

  updateCRCEEPROM();
}

void clearLedEEPROM() {
  // Clear out the EEPROM only in the memory addresses used for our EEPROM data object.
  for(unsigned int i = 0 ; i < sizeof(objEEPROM); i++) {
    EEPROM.put(i, 0);
  }

  updateCRCEEPROM();

  updateProtonPackLEDCounts();
}

void saveLedEEPROM() {
  // Power Cell LEDs
  // Cyclotron LEDs
  // Inner Cyclotron LEDs
  // GRB / RGB Inner Cyclotron toggle flag

  uint8_t i_grb_cyclotron = 1;

  if(b_grb_cyclotron == true) {
    i_grb_cyclotron = 2;
  }

  // Write the data to the EEPROM if any of the values have changed.
  objEEPROM obj_eeprom = {
    i_powercell_leds,
    i_cyclotron_leds,
    i_inner_cyclotron_num_leds,
    i_grb_cyclotron,
    i_spectral_powercell_custom,
    i_spectral_cyclotron_custom,
    i_spectral_cyclotron_inner_custom,
    i_spectral_powercell_custom_saturation,
    i_spectral_cyclotron_custom_saturation,
    i_spectral_cyclotron_inner_custom_saturation,
  };

  // Save and update our object in the EEPROM.
  EEPROM.put(i_eepromAddress, obj_eeprom);

  updateCRCEEPROM();
}

// Update the CRC in the EEPROM.
void updateCRCEEPROM() {
  EEPROM.put(EEPROM.length() - sizeof(l_crc_size), eepromCRC());
}

unsigned long eepromCRC(void) {
  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  unsigned long crc = l_crc_size;

  for(unsigned int index = 0; index < EEPROM.length() - sizeof(crc); ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }

  return crc;
}


void setupWavTrigger() {
  // If the controller is powering the WAV Trigger, we should wait for the WAV Trigger to finish reset before trying to send commands.
  delay(1000);

  // WAV Trigger's startup at 57600
  w_trig.start();

  delay(10);

  w_trig.stopAllTracks();
  w_trig.samplerateOffset(0); // Reset our sample rate offset
  w_trig.masterGain(-70); // Reset the master gain db. Range is -70 to 0. Bootup the system at the lowest volume, then we reset it after the system is loaded.
  w_trig.setAmpPwr(b_onboard_amp_enabled);

  // Enable track reporting from the WAV Trigger
  w_trig.setReporting(true);

  // Allow time for the WAV Triggers to respond with the version string and number of tracks.
  delay(350);

  int w_num_tracks = w_trig.getNumTracks();

  // Build the music track count.
  i_music_count = w_num_tracks - i_last_effects_track;

  if(i_music_count > 0) {
    i_current_music_track = i_music_track_start; // Set the first track of music as file 500_
  }
}
