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

// Set to 1 to enable built-in debug messages
#define DEBUG 0

// Debug macros
#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

// PROGMEM macro
#define PROGMEM_READU32(x) pgm_read_dword_near(&(x))
#define PROGMEM_READU16(x) pgm_read_word_near(&(x))
#define PROGMEM_READU8(x) pgm_read_byte_near(&(x))

// 3rd-Party Libraries
#include <CRC32.h>
#include <digitalWriteFast.h>
#include <EEPROM.h>
#include <millisDelay.h>
#include <FastLED.h>
#include <ezButton.h>
#include <Ramp.h>
#include <SerialTransfer.h>
#include <Wire.h>

// Local Files
#include "Configuration.h"
#include "MusicSounds.h"
#include "Communication.h"
#include "Header.h"
#include "Colours.h"
#include "Audio.h"
#include "Preferences.h"
#include "WandSensor.h"

void setup() {
  // Setup i2c.
  Wire.begin();

  Serial.begin(9600); // Standard serial (USB) console.
  Serial1.begin(9600); // Add-on Serial1 communication.
  Serial2.begin(9600); // Communication to the Neutrona Wand.

  // Search for and setup the optional Neutrona Wand sensor.
  wandSensorSetup();

  // Connect the serial ports.
  serial1Coms.begin(Serial1, false); // Attenuator/Wireless
  packComs.begin(Serial2, false); // Neutrona Wand

  // Setup the audio device for this controller.
  setupAudioDevice();

  // Rotary encoder for volume control.
  pinModeFast(encoder_pin_a, INPUT_PULLUP);
  pinModeFast(encoder_pin_b, INPUT_PULLUP);

  // Status indicator LED on the v1.5 GPStar Proton Pack Board.
  pinModeFast(led_pack_status, OUTPUT);

  // Configure the various switches on the pack.
  switch_alarm.setDebounceTime(50);
  switch_mode.setDebounceTime(50);
  switch_vibration.setDebounceTime(50);
  switch_cyclotron_direction.setDebounceTime(50);
  switch_cyclotron_lid.setDebounceTime(50);
  switch_smoke.setDebounceTime(50);

  // Adjust the PWM frequency of the vibration motor.
  TCCR5B = (TCCR5B & B11111000) | (B00000100);  // for PWM frequency of 122.55 Hz

  // Vibration motor
  pinMode(vibration, OUTPUT); // Vibration motor is PWM, so fallback to default pinMode just to be safe.

  // Smoke motor for the N-Filter.
  pinModeFast(smoke_pin, OUTPUT);

  // Fan pin for the N-Filter smoke.
  pinModeFast(fan_pin, OUTPUT);

  // Second smoke motor (booster tube)
  pinModeFast(smoke_booster_pin, OUTPUT);

  // A fan pin that goes off at the same time as the booster tube smoke pin.
  pinModeFast(fan_booster_pin, OUTPUT);

  // Another optional N-Filter LED.
  pinModeFast(i_nfilter_led_pin, OUTPUT);

  // Power Cell, Cyclotron Lid, and N-Filter.
  FastLED.addLeds<NEOPIXEL, PACK_LED_PIN>(pack_leds, FRUTTO_POWERCELL_LED_COUNT + OUTER_CYCLOTRON_LED_MAX + JEWEL_NFILTER_LED_COUNT);

  // Inner Cyclotron LEDs (Inner Panel + Cyclotron + Cavity).
  FastLED.addLeds<NEOPIXEL, CYCLOTRON_LED_PIN>(cyclotron_leds, INNER_CYCLOTRON_LED_PANEL_MAX + INNER_CYCLOTRON_CAKE_LED_MAX + INNER_CYCLOTRON_CAVITY_LED_MAX);

  // Other FastLED Options
  FastLED.setDither(0); // Disables the "temporal dithering" feature as this software will set brightness on a per-pixel level by device.
  //FastLED.setMaxPowerInVoltsAndMilliamps(5, 800); // Limit draw to 800mA at 5v of power. Enabling this can cause some flickering of the LEDs.

  // Cyclotron Switch Panel LEDs
  pinModeFast(cyclotron_sw_plate_led_r1, OUTPUT);
  pinModeFast(cyclotron_sw_plate_led_r2, OUTPUT);
  pinModeFast(cyclotron_sw_plate_led_y1, OUTPUT);
  pinModeFast(cyclotron_sw_plate_led_y2, OUTPUT);
  pinModeFast(cyclotron_sw_plate_led_g1, OUTPUT);
  pinModeFast(cyclotron_sw_plate_led_g2, OUTPUT);
  pinModeFast(cyclotron_switch_led_green, OUTPUT);
  pinModeFast(cyclotron_switch_led_yellow, OUTPUT);

  // Default mode is Super Hero (for simpler controls).
  SYSTEM_MODE = MODE_SUPER_HERO;

  // Bootup the pack into Proton mode, the same as the wand.
  STREAM_MODE = PROTON;

  // Set the CTS to not firing.
  STATUS_CTS = CTS_NOT_FIRING;

  // Set default year selection to toggle switch.
  SYSTEM_EEPROM_YEAR = SYSTEM_TOGGLE_SWITCH;

  // Set default vibration mode.
  VIBRATION_MODE_EEPROM = VIBRATION_DEFAULT;

  // Configure the vibration state.
  if(switch_vibration.getState() == LOW) {
    b_vibration_switch_on = true;
  }
  else {
    b_vibration_switch_on = false;
  }

  // Configure the year mode, though this will be modified
  // as based on the user's stored preferences in EEPROM.
  if(switch_mode.getState() == LOW) {
    SYSTEM_YEAR = SYSTEM_1984;
  }
  else {
    SYSTEM_YEAR = SYSTEM_AFTERLIFE;
  }

  SYSTEM_YEAR_TEMP = SYSTEM_YEAR;

  // Load any saved settings stored in the EEPROM memory of the Proton Pack.
  if(b_eeprom == true) {
    readEEPROM();
  }

  // Setup and configure the Inner Cyclotron LEDs.
  resetInnerCyclotronLEDs();
  updateProtonPackLEDCounts();

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

  // Reset cyclotron ramps.
  resetRampSpeeds();

  // Start some timers
  ms_battcheck.start(500);
  ms_fast_led.start(i_fast_led_delay);
  ms_check_music.start(i_music_check_delay);
  ms_serial1_handshake.start(i_serial1_handshake_delay);
  ms_cyclotron_switch_plate_leds.start(i_cyclotron_switch_plate_leds_delay);

  // Perform initial pack reset.
  packOffReset();

  if(SYSTEM_MODE == MODE_SUPER_HERO) {
    // Auto start the pack if it is in demo light mode.
    if(b_demo_light_mode == true) {
      // Turn the pack on.
      PACK_ACTION_STATE = ACTION_ACTIVATE;
    }
  }

  // Reset the master volume. Important to keep this as we startup the system at the lowest volume.
  // Then the EEPROM reads any settings if required, then we reset the volume.
  resetMasterVolume();

  // Perform power-on sequence if demo light mode is not enabled per user preferences.
  if(b_demo_light_mode != true && b_pack_post_finish != true) {
    // System Power On Self Test
    playEffect(S_POWER_ON);
    ms_delay_post.start(1);
  }
  else {
    b_pack_post_finish = true;
  }
}

void loop() {
  updateAudio();
  updateWandSensor();

  // Voltage Check
  if(ms_battcheck.justFinished()) {
    doVoltageCheck(); // Obtains the latest value and pushes the data to serial1, if available.
    ms_battcheck.start(i_ms_battcheck_delay);
  }

  // Check for any new serial commands were received from the Neutrona Wand.
  checkWand();

  // Check if the wand is considered to have been disconnected.
  wandDisconnectCheck();

  // Check if serial1 device is present.
  serial1HandShake();

  // Check if any new serial commands were received.
  checkSerial1();

  checkMusic();
  checkSwitches();
  checkRotaryEncoder();
  checkMenuVibration();

  if(b_pack_post_finish == true) {
    switch (PACK_STATE) {
      case MODE_OFF:
        // Turn on the status indicator LED.
        digitalWriteFast(led_pack_status, HIGH);

        if(b_pack_on == true) {
          b_2021_ramp_up = false;
          b_2021_ramp_up_start = false;
          b_inner_ramp_up = false;
          b_fade_out = true;

          reset2021RampDown();

          b_pack_shutting_down = true;

          ms_fadeout.start(0);

          switch(SYSTEM_MODE) {
            case MODE_ORIGINAL:
              if(switch_power.getState() == HIGH) {
                // Tell the Neutrona Wand that power to the Proton Pack is off.
                if(b_wand_connected) {
                  packSerialSend(P_MODE_ORIGINAL_RED_SWITCH_OFF);
                }

                // Tell the Attenuator or any other device that the power to the Proton Pack is off.
                if(b_serial1_connected) {
                  serial1Send(A_MODE_ORIGINAL_RED_SWITCH_OFF);
                }
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

          b_pack_on = false;
        }

        if(b_2021_ramp_down == true && b_overheating == false && b_alarm == false) {
          // If we enter the LED EEPROM menu while the pack is ramping off, stop it right away.
          if(b_spectral_lights_on == true) {
            packOffReset();
            spectralLightsOn();
          }
          else {
            cyclotronControl();
            cyclotronSwitchLEDLoop();
            powercellLoop();
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

            if(b_reset_start_led == false && ms_fadeout.isRunning() != true) {
              packOffReset();
            }
          }
        }
      break;

      case MODE_ON:
        // Turn off the status indicator LED.
        digitalWriteFast(led_pack_status, LOW);

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

        if(ribbonCableAttached() == true && b_overheating == false) {
          if(b_alarm == true) {
            if(!usingSlimeCyclotron()) {
              if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
                // Reset the LEDs before resetting the alarm flag.
                resetCyclotronState();
                ms_cyclotron.start(0);
              }
              else {
                ms_cyclotron.start(i_outer_current_ramp_speed);
              }
            }

            ms_cyclotron_ring.start(i_inner_current_ramp_speed);

            ventLight(false);
            ventLightLEDW(false);

            b_alarm = false;

            reset2021RampUp();

            stopEffect(S_PACK_RECOVERY);
            playEffect(S_PACK_RECOVERY);

            packStartup(false);
          }
        }

        checkCyclotronAutoSpeed();

        // Play a little bit of smoke and N-Filter vent lights while firing and other misc sound effects.
        if(b_wand_firing == true) {
          // Mix some impact sound effects.
          if(ms_firing_sound_mix.justFinished() && STREAM_MODE == PROTON && STATUS_CTS == CTS_NOT_FIRING && b_stream_effects == true) {
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
                // If no firing effect has played yet.
                i_random = 3;
              break;
            }

            uint16_t i_s_random = random(2,4) * 1000; // 2 or 3 seconds

            switch (i_random) {
              case 3:
                playEffect(S_FIRE_SPARKS, false, i_volume_effects, false, 0, false);
                i_last_firing_effect_mix = S_FIRE_SPARKS;

                ms_firing_sound_mix.start(i_s_random * 5);
              break;

              case 2:
                playEffect(S_FIRE_SPARKS_4, false, i_volume_effects, false, 0, false);
                i_last_firing_effect_mix = S_FIRE_SPARKS_4;

                ms_firing_sound_mix.start(i_s_random);
              break;

              case 1:
                playEffect(S_FIRE_SPARKS_3, false, i_volume_effects, false, 0, false);
                i_last_firing_effect_mix = S_FIRE_SPARKS_3;

                ms_firing_sound_mix.start(i_s_random);
              break;

              case 0:
                playEffect(S_FIRE_SPARKS_2, false, i_volume_effects, false, 0, false);
                playEffect(S_FIRE_SPARKS_5, false, i_volume_effects, false, 0, false);
                i_last_firing_effect_mix = S_FIRE_SPARKS_5;

                ms_firing_sound_mix.start(1800);
              break;

              default:
                // This will never trigger because i_random will only ever be 0~3.
                playEffect(S_FIRE_SPARKS_2, false, i_volume_effects, false, 0, false);
                i_last_firing_effect_mix = S_FIRE_SPARKS_2;

                ms_firing_sound_mix.start(500);
              break;
            }
          }

          if(ms_smoke_on.justFinished()) {
            ms_smoke_on.stop();
            ms_smoke_timer.start(PROGMEM_READU32(i_smoke_timer[i_wand_power_level - 1]));
            b_vent_sounds = true;
          }

          if(ms_smoke_timer.justFinished()) {
            if(ms_smoke_on.isRunning() != true) {
              ms_smoke_on.start(PROGMEM_READU32(i_smoke_on_time[i_wand_power_level - 1]));
            }
          }

          if(ms_smoke_on.isRunning() == true) {
            // Turn on some smoke and play some vent sounds if smoke is enabled.
            if(b_smoke_enabled == true) {
              // Turn on some smoke.
              smokeNFilter(true);

              // Play some sounds with the smoke and vent lighting.
              if(b_vent_sounds == true) {
                playVentSounds();

                b_vent_sounds = false;
              }

              fanNFilter(true);
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
            smokeNFilter(false);
            ventLight(false);
            ventLightLEDW(false);
            fanNFilter(false);
            fanBooster(false);
          }
        }

        if(b_venting == true) {
          packVenting();
        }

        cyclotronControl();
        cyclotronSwitchLEDLoop();

        if(b_overheating == true && b_overheat_lights_off == true) {
          powercellRampDown();
        }
        else {
          powercellLoop();
        }
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
        packStartup(true);
      break;
    }
  }
  else {
    systemPOST();
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

void systemPOST() {
  uint8_t i_tmp_led1 = i_cyclotron_led_start + cyclotron84LookupTable(0);
  uint8_t i_tmp_led2 = i_cyclotron_led_start + cyclotron84LookupTable(1);
  uint8_t i_tmp_led3 = i_cyclotron_led_start + cyclotron84LookupTable(2);
  uint8_t i_tmp_led4 = i_cyclotron_led_start + cyclotron84LookupTable(3);
  uint8_t i_tmp_led5 = i_pack_num_leds - round(i_nfilter_jewel_leds / 2);

  if(i_post_powercell_up < i_powercell_leds && ms_delay_post.justFinished()) {
    pack_leds[i_post_powercell_up] = getHueAsRGB(POWERCELL, C_MID_BLUE);

    if((i_post_powercell_up % 5) == 0) {
      pack_leds[i_tmp_led1] = getHueAsRGB(CYCLOTRON_OUTER, C_RED);
      pack_leds[i_tmp_led2] = getHueAsRGB(CYCLOTRON_OUTER, C_RED);
      pack_leds[i_tmp_led3] = getHueAsRGB(CYCLOTRON_OUTER, C_RED);
      pack_leds[i_tmp_led4] = getHueAsRGB(CYCLOTRON_OUTER, C_RED);
      pack_leds[i_tmp_led5] = getHueAsRGB(CYCLOTRON_OUTER, C_WHITE);
    }
    else {
      pack_leds[i_tmp_led1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led2] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led3] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led4] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led5] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
    }

    i_post_powercell_up++;

    if(i_post_powercell_up == i_powercell_leds) {
      ms_delay_post.stop();
      ms_delay_post_2.start(30);
    }
    else {
      ms_delay_post.start(30);
    }
  }

  if(i_post_powercell_down < i_powercell_leds && ms_delay_post_2.justFinished()) {
    pack_leds[i_post_powercell_down] = getHueAsRGB(POWERCELL, C_BLACK);

    if((i_post_powercell_down % 5) == 0) {
      pack_leds[i_tmp_led1] = getHueAsRGB(CYCLOTRON_OUTER, C_RED);
      pack_leds[i_tmp_led2] = getHueAsRGB(CYCLOTRON_OUTER, C_RED);
      pack_leds[i_tmp_led3] = getHueAsRGB(CYCLOTRON_OUTER, C_RED);
      pack_leds[i_tmp_led4] = getHueAsRGB(CYCLOTRON_OUTER, C_RED);
      pack_leds[i_tmp_led5] = getHueAsRGB(CYCLOTRON_OUTER, C_WHITE);
    }
    else {
      pack_leds[i_tmp_led1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led2] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led3] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led4] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led5] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
    }

    if(b_inner_cyclotron_led_panel == true) {
      if(i_post_powercell_down <= i_ic_panel_end) {
        cyclotron_leds[i_post_powercell_down] = getHueAsRGB(CYCLOTRON_PANEL, C_RED);

        if(i_post_powercell_down > i_ic_panel_start) {
          cyclotron_leds[i_post_powercell_down - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        }
      }

      if(i_post_powercell_down > i_ic_panel_end) {
        cyclotron_leds[i_ic_panel_end] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
      }
    }

    i_post_powercell_down++;

    if(i_post_powercell_down == i_powercell_leds) {
      ms_delay_post_2.stop();
      ms_delay_post_3.start(30);
    }
    else {
      ms_delay_post_2.start(30);
    }
  }

  if(i_post_fade > 0 && ms_delay_post_3.justFinished()) {
    pack_leds[i_tmp_led1] = getHueAsRGB(CYCLOTRON_OUTER, C_RED, i_post_fade);
    pack_leds[i_tmp_led2] = getHueAsRGB(CYCLOTRON_OUTER, C_RED, i_post_fade);
    pack_leds[i_tmp_led3] = getHueAsRGB(CYCLOTRON_OUTER, C_RED, i_post_fade);
    pack_leds[i_tmp_led4] = getHueAsRGB(CYCLOTRON_OUTER, C_RED, i_post_fade);
    pack_leds[i_tmp_led5] = getHueAsRGB(CYCLOTRON_OUTER, C_WHITE, i_post_fade);

    i_post_fade--;

    if(i_post_fade == 0) {
      ms_delay_post_3.stop();

      pack_leds[i_tmp_led1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led2] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led3] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led4] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      pack_leds[i_tmp_led5] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);

      cyclotronSwitchLEDOff();

      b_pack_post_finish = true;
    }
    else {
      ms_delay_post_3.start(5);
    }
  }
}

bool fadeOutLights() {
  bool b_return = false;

  if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) && !usingSlimeCyclotron()) {
    uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, STREAM_MODE, b_cyclotron_colour_toggle);

    // We override the colour changes when using stock HasLab Cyclotron LEDs.
    // Changing the colour space with a CHSV Object affects the brightness slightly for non RGB pixels.
    if(i_cyclotron_leds == HASLAB_CYCLOTRON_LED_COUNT && b_cyclotron_haslab_chsv_colour_change != true) {
      i_colour_scheme = C_HASLAB;
    }

    uint8_t i_cyclotron_leds_total = i_pack_num_leds - i_nfilter_jewel_leds - i_cyclotron_led_start;

    if(b_cyclotron_simulate_ring == true) {
      i_cyclotron_leds_total = OUTER_CYCLOTRON_LED_MAX;
    }

    for(uint8_t i = 0; i < i_cyclotron_leds_total; i++) {
      uint8_t i_curr_brightness = i_cyclotron_led_value[i] - 10;

      if(i_curr_brightness > i_cyclotron_led_value[i]) {
        i_curr_brightness = 0;
      }

      i_cyclotron_led_value[i] = i_curr_brightness;

      if(i_curr_brightness > 0) {

        b_return = true;

        if(b_cyclotron_simulate_ring == true) {
          pack_leds[cyclotronLookupTable(i) + i_cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
        }
        else {
          pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
        }
      }
      else {
        if(b_cyclotron_simulate_ring == true) {
          pack_leds[cyclotronLookupTable(i) + i_cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, C_BLACK);
        }
        else {
          pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, C_BLACK);
        }
      }
    }
  }

  return b_return;
}

bool ribbonCableAttached() {
  if(b_use_ribbon_cable == true) {
    if(switch_alarm.getState() == LOW) {
      // Ribbon cable is attached.
      return true;
    }
    else {
      // Ribbon cable is detached.
      return false;
    }
  }
  else {
    // If no ribbon cable switch is used, always return true.
    return true;
  }
}

void playVentSounds() {
  playEffect(S_VENT_SMOKE);
  playEffect(S_SPARKS_LOOP);
}

void packStartup(bool firstStart) {
  PACK_STATE = MODE_ON;
  PACK_ACTION_STATE = ACTION_IDLE;

  if(ribbonCableAttached() != true) {
    if((SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) && !usingSlimeCyclotron()) {
      ms_cyclotron.start(0);
      ms_alarm.start(0);
    }

    packAlarm();

    // Tell the wand and add-on device the pack ribbon cable alarm is on.
    packSerialSend(P_ALARM_ON);
    serial1Send(A_ALARM_ON);
  }
  else {
    if(!firstStart) {
      // Tell the wand the pack alarm is off.
      packSerialSend(P_ALARM_OFF);

      // Tell any add-on devices that the alarm is off.
      serial1Send(A_ALARM_OFF);
    }

    stopEffect(S_PACK_RIBBON_ALARM_1);
    stopEffect(S_ALARM_LOOP);
    stopEffect(S_RIBBON_CABLE_START);
    stopEffect(S_PACK_SHUTDOWN_AFTERLIFE_ALT); // This is a long track which may still be playing.

    switch(SYSTEM_YEAR) {
      case SYSTEM_1984:
        playEffect(S_GB1_1984_BOOT_UP);
        playEffect(S_GB1_1984_PACK_LOOP, true, i_volume_effects, true, 3800);
      break;

      case SYSTEM_1989:
        playEffect(S_GB2_PACK_START);
        playEffect(S_GB2_PACK_LOOP, true, i_volume_effects, true, 3000);
      break;

      case SYSTEM_AFTERLIFE:
      default:
        if(firstStart) {
          if(STREAM_MODE == SLIME) {
            playEffect(S_AFTERLIFE_PACK_STARTUP, false, i_volume_effects - 30);
            playEffect(S_AFTERLIFE_PACK_IDLE_LOOP, true, i_volume_effects - 40, true, 18000);
          }
          else {
            playEffect(S_AFTERLIFE_PACK_STARTUP, false, i_volume_effects);
            playEffect(S_AFTERLIFE_PACK_IDLE_LOOP, true, i_volume_effects, true, 18000);
          }

          ms_idle_fire_fade.start(18000);
        }
        else {
          if(STREAM_MODE == SLIME) {
            playEffect(S_BOOTUP, false, i_volume_effects - 30);
            playEffect(S_AFTERLIFE_PACK_IDLE_LOOP, true, i_volume_effects - 40, true, 500);
          }
          else {
            playEffect(S_BOOTUP, false, i_volume_effects);
            playEffect(S_AFTERLIFE_PACK_IDLE_LOOP, true, i_volume_effects, true, 500);
          }

          ms_idle_fire_fade.start(0);
        }
      break;

      case SYSTEM_FROZEN_EMPIRE:
        if(STREAM_MODE == SLIME) {
          playEffect(S_BOOTUP, false, i_volume_effects - 30);
          playEffect(S_AFTERLIFE_PACK_IDLE_LOOP, true, i_volume_effects - 40, true, 500);
        }
        else {
          playEffect(S_BOOTUP, false, i_volume_effects);
          playEffect(S_AFTERLIFE_PACK_IDLE_LOOP, true, i_volume_effects, true, 500);
        }

        // Cyclotron lid is off, play the Frozen Empire sound effect.
        if(SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE && STREAM_MODE == PROTON && !b_cyclotron_lid_on) {
          playEffect(S_FROZEN_EMPIRE_BOOT_EFFECT, true, i_volume_effects, true, 2000);
          b_brass_pack_sound_loop = true;
        }

        ms_idle_fire_fade.start(0);
      break;
    }

    switch(STREAM_MODE) {
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

  switch(SYSTEM_YEAR) {
    case SYSTEM_1984:
    case SYSTEM_1989:
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      stopEffect(S_PACK_RECOVERY);
      stopEffect(S_PACK_RIBBON_ALARM_1);
      stopEffect(S_ALARM_LOOP);
      stopEffect(S_RIBBON_CABLE_START);
    break;
  }

  if(b_wand_firing == true) {
    // Preemptively stop firing.
    wandStoppedFiring();
    cyclotronSpeedRevert();
  }
  else {
    // Turn off the vent lights if they were on.
    ventLight(false);
    ventLightLEDW(false);
    ms_vent_light_off.stop();
    ms_vent_light_on.stop();

    // Turn off any smoke.
    smokeNFilter(false);
    ms_smoke_timer.stop();
    ms_smoke_on.stop();

    // Turn off the fans.
    fanNFilter(false);
    fanBooster(false);

    // Turn off the Cyclotron auto speed timer.
    ms_cyclotron_auto_speed_timer.stop();

    // Reset vent sounds flag.
    b_vent_sounds = true;
  }

  wandExtraSoundsStop();
  wandExtraSoundsBeepLoopStop();

  stopEffect(S_SHUTDOWN);
  stopEffect(S_STEAM_LOOP);
  stopEffect(S_SLIME_REFILL);

  if(STREAM_MODE == SLIME) {
    stopEffect(S_PACK_SLIME_TANK_LOOP);
  }

  if(STREAM_MODE == STASIS) {
    stopEffect(S_STASIS_IDLE_LOOP);
  }

  if(STREAM_MODE == MESON) {
    stopEffect(S_MESON_IDLE_LOOP);
  }

  if(SYSTEM_YEAR == SYSTEM_1989) {
    stopEffect(S_GB2_PACK_START);
    stopEffect(S_GB2_PACK_LOOP);
    stopEffect(S_GB2_PACK_OFF);
  }

  if(SYSTEM_YEAR == SYSTEM_1984) {
    stopEffect(S_PACK_SHUTDOWN);
    stopEffect(S_GB1_1984_PACK_LOOP);
    stopEffect(S_GB1_1984_BOOT_UP);
  }

  if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
    if(b_powercell_sound_loop) {
      stopEffect(S_POWERCELL); // Just in case a shutdown happens and not a ramp down.
      b_powercell_sound_loop = false;
    }

    stopEffect(S_PACK_SHUTDOWN_AFTERLIFE_ALT);
    stopEffect(S_AFTERLIFE_PACK_STARTUP);
    stopEffect(S_AFTERLIFE_PACK_IDLE_LOOP);

    if(SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
      stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT);
    }
  }

  // Need to play the 'close' SFX if we already played the open one.
  if(b_overheating == true) {
    stopEffect(S_SLIME_EMPTY);

    stopEffect(S_VENT_OPEN);

    if(STREAM_MODE != SLIME) {
      stopEffect(S_PACK_PRE_VENT);

      if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
        stopEffect(S_PACK_OVERHEAT_HOT);
      }

      playEffect(S_VENT_CLOSE);
      playEffect(S_STEAM_LOOP_FADE_OUT);
    }
  }
  else if(b_venting == true) {
    stopEffect(S_SLIME_EMPTY);
    stopEffect(S_QUICK_VENT_OPEN);
    playEffect(S_QUICK_VENT_CLOSE);
  }

  if(b_alarm != true) {
    switch(SYSTEM_YEAR) {
      case SYSTEM_1984:
        playEffect(S_PACK_SHUTDOWN);
      break;

      case SYSTEM_1989:
        playEffect(S_GB2_PACK_OFF);
      break;

      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        playEffect(S_PACK_SHUTDOWN_AFTERLIFE_ALT);
      break;
    }
  }
  else {
    playEffect(S_SHUTDOWN);
  }
}

void packOffReset() {
  powercellOff();
  cyclotronSwitchLEDOff();

  ms_overheating_length.stop();
  b_overheating = false;
  b_venting = false;
  b_2021_ramp_down = false;
  b_2021_ramp_down_start = false;
  b_inner_ramp_down = false;
  b_reset_start_led = true; // Reset the start LED of the Cyclotron.

  resetCyclotronState();
  reset2021RampUp();

  // Update Power Cell LED timer delay and optional Cyclotron LED switch plate LED timer delays.
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

  // Reset the Power Cell timer.
  ms_powercell.stop();
  ms_powercell.start(i_powercell_delay);

  // Reset the Cyclotron LED switch timer.
  ms_cyclotron_switch_led.stop();
  ms_cyclotron_switch_led.start(i_cyclotron_switch_led_delay);

  // Need to reset the Cyclotron timers.
  if(!usingSlimeCyclotron()) {
    ms_cyclotron.start(i_2021_delay);
  }

  ms_cyclotron_ring.start(i_inner_ramp_delay);

  // Vibration motor off.
  vibrationOff();
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

void setYearModeByToggle() {
  // We have 4 year modes but only 2 toggle states, so these get grouped by their Haslab defaults.
  // Toggling the switch up/down will cycle through 1984 -> Afterlife -> 1989 -> Frozen Empire.
  if(switch_mode.getState() == LOW) {
    if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
      // When currently in Afterlife/Frozen Empire we switch to 1984 or 1989.
      if(SYSTEM_YEAR == SYSTEM_AFTERLIFE) {
        SYSTEM_YEAR = SYSTEM_1989;
        SYSTEM_YEAR_TEMP = SYSTEM_YEAR;

        // Tell the wand/serial1 to switch to 1989 mode.
        packSerialSend(P_YEAR_1989);
        serial1Send(A_YEAR_1989);

        // Play audio cue confirming the change. Only play the audio queue when the user physically flicks the switch.
        if(switch_mode.isPressed() || switch_mode.isReleased()) {
          playEffect(S_VOICE_1989);
        }
      }
      else {
        SYSTEM_YEAR = SYSTEM_1984;
        SYSTEM_YEAR_TEMP = SYSTEM_YEAR;

        // Tell the wand/serial1 to switch to 1984 mode.
        packSerialSend(P_YEAR_1984);
        serial1Send(A_YEAR_1984);

        // Play audio cue confirming the change. Only play the audio queue when the user physically flicks the switch.
        if(switch_mode.isPressed() || switch_mode.isReleased()) {
          playEffect(S_VOICE_1984);
        }
      }

      // Reset the pack variables to match the new year mode.
      resetRampSpeeds();
      packOffReset();
    }
  }
  else {
    if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
      // When currently in 1984/1989 we switch to Afterlife or Frozen Empire.
      if(SYSTEM_YEAR == SYSTEM_1984) {
        SYSTEM_YEAR = SYSTEM_AFTERLIFE;
        SYSTEM_YEAR_TEMP = SYSTEM_YEAR;

        // Tell the wand/serial1 to switch to Afterlife mode.
        packSerialSend(P_YEAR_AFTERLIFE);
        serial1Send(A_YEAR_AFTERLIFE);

        // Play audio cue confirming the change. Only play the audio queue when the user physically flicks the switch.
        if(switch_mode.isPressed() || switch_mode.isReleased()) {
          playEffect(S_VOICE_AFTERLIFE);
        }
      }
      else {
        SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
        SYSTEM_YEAR_TEMP = SYSTEM_YEAR;

        // Tell the wand/serial1 to switch to Afterlife mode.
        packSerialSend(P_YEAR_FROZEN_EMPIRE);
        serial1Send(A_YEAR_FROZEN_EMPIRE);

        // Play audio cue confirming the change. Only play the audio queue when the user physically flicks the switch.
        if(switch_mode.isPressed() || switch_mode.isReleased()) {
          playEffect(S_VOICE_FROZEN_EMPIRE);
        }
      }

      // Reset the pack variables to match the new year mode.
      resetRampSpeeds();
      packOffReset();
    }
  }
}

void checkSwitches() {
  // Perform loop() needed by ezButton.
  switch_alarm.loop();
  switch_cyclotron_lid.loop();
  switch_cyclotron_direction.loop();
  switch_mode.loop();
  switch_power.loop();
  switch_smoke.loop();
  switch_vibration.loop();

  cyclotronSwitchPlateLEDs();

  // Cyclotron direction toggle switch.
  if(switch_cyclotron_direction.isPressed() || switch_cyclotron_direction.isReleased()) {
    stopEffect(S_BEEPS);
    stopEffect(S_BEEPS_ALT);
    stopEffect(S_VOICE_CYCLOTRON_CLOCKWISE);
    stopEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);

    if(b_clockwise == true) {
      b_clockwise = false;

      playEffect(S_BEEPS_ALT);
      playEffect(S_VOICE_CYCLOTRON_COUNTER_CLOCKWISE);

      // Tell wand to play Cyclotron counter clockwise voice.
      packSerialSend(P_CYCLOTRON_COUNTER_CLOCKWISE);
    }
    else {
      b_clockwise = true;

      playEffect(S_BEEPS);
      playEffect(S_VOICE_CYCLOTRON_CLOCKWISE);

      // Tell wand to play Cyclotron clockwise voice.
      packSerialSend(P_CYCLOTRON_CLOCKWISE);
    }
  }

  // Smoke
  if(switch_smoke.isPressed() || switch_smoke.isReleased()) {
    stopEffect(S_VOICE_SMOKE_DISABLED);
    stopEffect(S_VOICE_SMOKE_ENABLED);

    if(b_smoke_enabled == true) {
      b_smoke_enabled = false;

      stopEffect(S_VENT_DRY);

      playEffect(S_VENT_DRY);
      playEffect(S_VOICE_SMOKE_DISABLED);

      // Tell wand to play smoke disabled voice.
      packSerialSend(P_SMOKE_DISABLED);
    }
    else {
      b_smoke_enabled = true;

      stopEffect(S_VENT_SMOKE);

      playEffect(S_VENT_SMOKE);
      playEffect(S_VOICE_SMOKE_ENABLED);

      // Tell wand to play smoke enabled voice.
      packSerialSend(P_SMOKE_ENABLED);
    }
  }

  // Vibration toggle switch.
  if(switch_vibration.isPressed() || switch_vibration.isReleased()) {
    stopEffect(S_VOICE_VIBRATION_ENABLED);
    stopEffect(S_VOICE_VIBRATION_DISABLED);

    if(switch_vibration.getState() == LOW) {
      if(b_vibration_switch_on == false) {
        // Tell the wand to enable vibration.
        packSerialSend(P_VIBRATION_ENABLED);

        b_vibration_switch_on = true;

        playEffect(S_VOICE_VIBRATION_ENABLED);
      }
    }
    else {
      if(b_vibration_switch_on == true) {
        // Tell the wand to disable vibration.
        packSerialSend(P_VIBRATION_DISABLED);

        b_vibration_switch_on = false;

        playEffect(S_VOICE_VIBRATION_DISABLED);
      }
    }
  }

  if(switch_mode.isPressed() || switch_mode.isReleased()) {
    // Play a beep confirmation when the switch is flipped.
    stopEffect(S_BEEPS_BARGRAPH);
    playEffect(S_BEEPS_BARGRAPH);

    // Turn off the year mode override flag controlled by the Proton Pack.
    b_switch_mode_override = false;
  }

  if(b_use_ribbon_cable && (switch_alarm.isPressed() || switch_alarm.isReleased())) {
    // Play a sound when the ribbon cable is attached or detached.
    if(ribbonCableAttached()) {
      // Only play this sound if the pack is off to match Frozen Empire.
      if(PACK_STATE == MODE_OFF) {
        stopEffect(S_CLICK);
        playEffect(S_CLICK);
      }
    }
    else {
      stopEffect(S_RIBBON_CABLE_DETACH);
      playEffect(S_RIBBON_CABLE_DETACH);
    }
  }

  switch(PACK_STATE) {
    case MODE_OFF:
      switch(SYSTEM_MODE) {
        case MODE_ORIGINAL:
          if(switch_power.isPressed() || switch_power.isReleased()) {
            // When the ion arm switch is used to turn the Proton Pack on, play a extra sound effect in Afterlife or Frozen Empire.
            switch(SYSTEM_YEAR) {
              case SYSTEM_AFTERLIFE:
              case SYSTEM_FROZEN_EMPIRE:
                stopEffect(S_ION_ARM_SWITCH_ALT);
                playEffect(S_ION_ARM_SWITCH_ALT);
              break;

              case SYSTEM_1984:
              case SYSTEM_1989:
              default:
                // Do nothing.
              break;
            }

            if(switch_power.getState() == LOW) {
              // Tell the Neutrona Wand that power to the Proton Pack is on.
              if(b_wand_connected) {
                packSerialSend(P_MODE_ORIGINAL_RED_SWITCH_ON);
              }

              // Tell the Attenuator or any other device that the power to the Proton Pack is on.
              if(b_serial1_connected) {
                serial1Send(A_MODE_ORIGINAL_RED_SWITCH_ON);
              }
            }
            else {
              // Tell the Neutrona Wand that power to the Proton Pack is off.
              if(b_wand_connected) {
                packSerialSend(P_MODE_ORIGINAL_RED_SWITCH_OFF);
              }

              // Tell the Attenuator or any other device that the power to the Proton Pack is off.
              if(b_serial1_connected) {
                serial1Send(A_MODE_ORIGINAL_RED_SWITCH_OFF);
              }
            }
          }
        break;

        case MODE_SUPER_HERO:
        default:
          if(switch_power.isPressed() || switch_power.isReleased()) {
            // When the ion arm switch is used, play an extra sound effect in Afterlife or Frozen Empire.
            switch(SYSTEM_YEAR) {
              case SYSTEM_AFTERLIFE:
              case SYSTEM_FROZEN_EMPIRE:
                stopEffect(S_ION_ARM_SWITCH_ALT);
                playEffect(S_ION_ARM_SWITCH_ALT);
              break;

              case SYSTEM_1984:
              case SYSTEM_1989:
              default:
                // Do nothing.
              break;
            }

            // Turn the pack on if switch is moved to on position.
            if(switch_power.getState() == LOW) {
              PACK_ACTION_STATE = ACTION_ACTIVATE;
            }
          }
        break;
      }

      // Year mode. Best to adjust it only when the pack is off.
      if(b_pack_shutting_down != true && b_pack_on == false && b_spectral_lights_on != true) {
        // If switching manually by the pack toggle switch.
        if(b_switch_mode_override != true) {
          setYearModeByToggle();
        }
        else {
          // If the Neutrona Wand sub menu setting told the Proton Pack to change years.
          switch(SYSTEM_YEAR_TEMP) {
            case SYSTEM_1984:
              if(SYSTEM_YEAR != SYSTEM_YEAR_TEMP) {
                // Tell the wand to switch to 1984 mode.
                packSerialSend(P_YEAR_1984);

                SYSTEM_YEAR = SYSTEM_1984;
                SYSTEM_YEAR_TEMP = SYSTEM_YEAR;

                serial1Send(A_YEAR_1984);
              }
            break;

            case SYSTEM_1989:
              if(SYSTEM_YEAR != SYSTEM_YEAR_TEMP) {
                // Tell the wand to switch to 1989 mode.
                packSerialSend(P_YEAR_1989);

                SYSTEM_YEAR = SYSTEM_1989;
                SYSTEM_YEAR_TEMP = SYSTEM_YEAR;

                serial1Send(A_YEAR_1989);
              }
            break;

            case SYSTEM_FROZEN_EMPIRE:
              if(SYSTEM_YEAR != SYSTEM_YEAR_TEMP) {
                // Tell the wand to switch to Frozen Empire mode.
                packSerialSend(P_YEAR_FROZEN_EMPIRE);

                SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
                SYSTEM_YEAR_TEMP = SYSTEM_YEAR;

                serial1Send(A_YEAR_FROZEN_EMPIRE);
              }
            break;

            case SYSTEM_AFTERLIFE:
            default:
              if(SYSTEM_YEAR != SYSTEM_YEAR_TEMP) {
                // Tell the wand to switch to Afterlife mode.
                packSerialSend(P_YEAR_AFTERLIFE);

                SYSTEM_YEAR = SYSTEM_AFTERLIFE;
                SYSTEM_YEAR_TEMP = SYSTEM_YEAR;

                serial1Send(A_YEAR_AFTERLIFE);
              }
            break;
          }

          resetRampSpeeds();
          packOffReset();
        }
      }
    break;

    case MODE_ON:
      if(switch_power.isReleased() || switch_power.isPressed()) {
        // Turn the pack off if switch is moved to off position.
        if(switch_power.getState() == HIGH) {
          PACK_ACTION_STATE = ACTION_OFF;
        }

        // When the ion arm switch is used, play an extra sound effect in Afterlife or Frozen Empire.
        switch(SYSTEM_YEAR) {
          case SYSTEM_AFTERLIFE:
          case SYSTEM_FROZEN_EMPIRE:
            stopEffect(S_ION_ARM_SWITCH_ALT);
            playEffect(S_ION_ARM_SWITCH_ALT);
          break;

          case SYSTEM_1984:
          case SYSTEM_1989:
          default:
            if(switch_power.getState() == HIGH) {
              // If shutting down from the ion arm switch in 84/89, play the extra shutdown sound.
              playEffect(S_SHUTDOWN);
            }
          break;
        }
      }
    break;
  }
}

void resetRampSpeeds() {
  switch(SYSTEM_YEAR) {
    case SYSTEM_1984:
    case SYSTEM_1989:
      i_outer_current_ramp_speed = (uint16_t)(i_1984_delay * 1.3);
      i_inner_current_ramp_speed = i_inner_ramp_delay;
    break;

    case SYSTEM_AFTERLIFE:
    default:
      i_outer_current_ramp_speed = i_2021_ramp_delay;
      i_inner_current_ramp_speed = i_inner_ramp_delay;
    break;

    case SYSTEM_FROZEN_EMPIRE:
      i_outer_current_ramp_speed = (uint16_t)(i_2021_ramp_delay / 1.5);
      i_inner_current_ramp_speed = i_inner_ramp_delay;
    break;
  }
}

void cyclotronSwitchLEDOff() {
  digitalWriteFast(cyclotron_sw_plate_led_r1, LOW);
  digitalWriteFast(cyclotron_sw_plate_led_r2, LOW);

  digitalWriteFast(cyclotron_sw_plate_led_y1, LOW);
  digitalWriteFast(cyclotron_sw_plate_led_y2, LOW);

  digitalWriteFast(cyclotron_sw_plate_led_g1, LOW);
  digitalWriteFast(cyclotron_sw_plate_led_g2, LOW);

  i_cyclotron_sw_led = 0;

  innerCyclotronLEDPanelOff();
}

void cyclotronSwitchLEDUpdate() {
  // When lid is off, updates the switch panel lights using either the stock connectors for individual LEDs,
  // or via the addressable LEDs if the user has installed the custom PCB between the Pack Controller and Cake.
  if(b_cyclotron_lid_on != true) {
    uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_PANEL, STREAM_MODE, b_cyclotron_colour_toggle);
    uint8_t i_brightness = getBrightness(i_cyclotron_panel_brightness);

    if(b_alarm == true) {
      if(i_cyclotron_sw_led > 0) {
        digitalWriteFast(cyclotron_sw_plate_led_r1, HIGH);
        digitalWriteFast(cyclotron_sw_plate_led_r2, HIGH);

        digitalWriteFast(cyclotron_sw_plate_led_y1, HIGH);
        digitalWriteFast(cyclotron_sw_plate_led_y2, HIGH);

        digitalWriteFast(cyclotron_sw_plate_led_g1, HIGH);
        digitalWriteFast(cyclotron_sw_plate_led_g2, HIGH);

        if(b_inner_cyclotron_led_panel == true) {
          for(uint8_t i = i_ic_panel_start; i <= i_ic_panel_end - 2; i++) {
            cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_PANEL, C_RED, i_brightness);
          }
        }
      }
      else {
        digitalWriteFast(cyclotron_sw_plate_led_r1, LOW);
        digitalWriteFast(cyclotron_sw_plate_led_r2, LOW);

        digitalWriteFast(cyclotron_sw_plate_led_y1, LOW);
        digitalWriteFast(cyclotron_sw_plate_led_y2, LOW);

        digitalWriteFast(cyclotron_sw_plate_led_g1, LOW);
        digitalWriteFast(cyclotron_sw_plate_led_g2, LOW);

        if(b_inner_cyclotron_led_panel == true) {
          for(uint8_t i = i_ic_panel_start; i <= i_ic_panel_end - 2; i++) {
            cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
          }
        }
      }
    }
    else {
      switch(i_cyclotron_sw_led) {
        case 0: // All Off
          digitalWriteFast(cyclotron_sw_plate_led_r1, LOW);
          digitalWriteFast(cyclotron_sw_plate_led_r2, LOW);

          digitalWriteFast(cyclotron_sw_plate_led_y1, LOW);
          digitalWriteFast(cyclotron_sw_plate_led_y2, LOW);

          digitalWriteFast(cyclotron_sw_plate_led_g1, LOW);
          digitalWriteFast(cyclotron_sw_plate_led_g2, LOW);

          if(b_inner_cyclotron_led_panel == true) {
            // All but the switch LEDs are turned off
            for(uint8_t i = i_ic_panel_start; i <= i_ic_panel_end - 2; i++) {
              cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
            }
          }
        break;

        case 1: // Add Green/Bottom
          digitalWriteFast(cyclotron_sw_plate_led_r1, LOW);
          digitalWriteFast(cyclotron_sw_plate_led_r2, LOW);

          digitalWriteFast(cyclotron_sw_plate_led_y1, LOW);
          digitalWriteFast(cyclotron_sw_plate_led_y2, LOW);

          digitalWriteFast(cyclotron_sw_plate_led_g1, HIGH);
          digitalWriteFast(cyclotron_sw_plate_led_g2, HIGH);

          if(b_inner_cyclotron_led_panel == true) {
            cyclotron_leds[4] = getHueAsRGB(CYCLOTRON_PANEL, i_colour_scheme, i_brightness);
            cyclotron_leds[5] = getHueAsRGB(CYCLOTRON_PANEL, i_colour_scheme, i_brightness);
          }
        break;

        case 2: // Add Yellow/Middle
          digitalWriteFast(cyclotron_sw_plate_led_r1, LOW);
          digitalWriteFast(cyclotron_sw_plate_led_r2, LOW);

          digitalWriteFast(cyclotron_sw_plate_led_y1, HIGH);
          digitalWriteFast(cyclotron_sw_plate_led_y2, HIGH);

          digitalWriteFast(cyclotron_sw_plate_led_g1, HIGH);
          digitalWriteFast(cyclotron_sw_plate_led_g2, HIGH);

          if(b_inner_cyclotron_led_panel == true) {
            cyclotron_leds[2] = getHueAsRGB(CYCLOTRON_PANEL, i_colour_scheme, i_brightness);
            cyclotron_leds[3] = getHueAsRGB(CYCLOTRON_PANEL, i_colour_scheme, i_brightness);
          }
        break;

        case 3: // Add Red/Top
          digitalWriteFast(cyclotron_sw_plate_led_r1, HIGH);
          digitalWriteFast(cyclotron_sw_plate_led_r2, HIGH);

          digitalWriteFast(cyclotron_sw_plate_led_y1, HIGH);
          digitalWriteFast(cyclotron_sw_plate_led_y2, HIGH);

          digitalWriteFast(cyclotron_sw_plate_led_g1, HIGH);
          digitalWriteFast(cyclotron_sw_plate_led_g2, HIGH);

          if(b_inner_cyclotron_led_panel == true) {
            cyclotron_leds[0] = getHueAsRGB(CYCLOTRON_PANEL, i_colour_scheme, i_brightness);
            cyclotron_leds[1] = getHueAsRGB(CYCLOTRON_PANEL, i_colour_scheme, i_brightness);
          }
        break;

        case 4: // All Illuminated (Pause)
          digitalWriteFast(cyclotron_sw_plate_led_r1, HIGH);
          digitalWriteFast(cyclotron_sw_plate_led_r2, HIGH);

          digitalWriteFast(cyclotron_sw_plate_led_y1, HIGH);
          digitalWriteFast(cyclotron_sw_plate_led_y2, HIGH);

          digitalWriteFast(cyclotron_sw_plate_led_g1, HIGH);
          digitalWriteFast(cyclotron_sw_plate_led_g2, HIGH);

          if(b_inner_cyclotron_led_panel == true) {
            for(uint8_t i = i_ic_panel_start; i <= i_ic_panel_end - 2; i++) {
              cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_PANEL, i_colour_scheme, i_brightness);
            }
          }
        break;

        case 5: // Remove Green/Bottom
          digitalWriteFast(cyclotron_sw_plate_led_r1, HIGH);
          digitalWriteFast(cyclotron_sw_plate_led_r2, HIGH);

          digitalWriteFast(cyclotron_sw_plate_led_y1, HIGH);
          digitalWriteFast(cyclotron_sw_plate_led_y2, HIGH);

          digitalWriteFast(cyclotron_sw_plate_led_g1, LOW);
          digitalWriteFast(cyclotron_sw_plate_led_g2, LOW);

          if(b_inner_cyclotron_led_panel == true) {
            cyclotron_leds[4] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
            cyclotron_leds[5] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
          }
        break;

        case 6: // Remove Yellow/Middle
          digitalWriteFast(cyclotron_sw_plate_led_r1, HIGH);
          digitalWriteFast(cyclotron_sw_plate_led_r2, HIGH);

          digitalWriteFast(cyclotron_sw_plate_led_y1, LOW);
          digitalWriteFast(cyclotron_sw_plate_led_y2, LOW);

          digitalWriteFast(cyclotron_sw_plate_led_g1, LOW);
          digitalWriteFast(cyclotron_sw_plate_led_g2, LOW);

          if(b_inner_cyclotron_led_panel == true) {
            cyclotron_leds[2] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
            cyclotron_leds[3] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
          }
        break;

        case 7:// Remove Red/Top
          digitalWriteFast(cyclotron_sw_plate_led_r1, LOW);
          digitalWriteFast(cyclotron_sw_plate_led_r2, LOW);

          digitalWriteFast(cyclotron_sw_plate_led_y1, LOW);
          digitalWriteFast(cyclotron_sw_plate_led_y2, LOW);

          digitalWriteFast(cyclotron_sw_plate_led_g1, LOW);
          digitalWriteFast(cyclotron_sw_plate_led_g2, LOW);

          if(b_inner_cyclotron_led_panel == true) {
            cyclotron_leds[0] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
            cyclotron_leds[1] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
          }
        break;
      }
    }
  }
}

void cyclotronSwitchLEDLoop() {
  if(ms_cyclotron_switch_led.justFinished()) {
    if(b_cyclotron_lid_on != true) {
      if(b_alarm == true) {
        if(i_cyclotron_sw_led > 0) {
          i_cyclotron_sw_led = 0;
        }
        else {
          i_cyclotron_sw_led++;
        }
      }
      else {
        if(i_cyclotron_sw_led >= 7) {
          i_cyclotron_sw_led = 0;
        }
        else {
          i_cyclotron_sw_led++;
        }
      }

      // Frozen Empire brass pack sound is handled here.
      if(SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE && STREAM_MODE == PROTON && !b_alarm && !b_overheating && !b_2021_ramp_down) {
        if(!b_brass_pack_sound_loop) {
          playEffect(S_FROZEN_EMPIRE_BOOT_EFFECT, true, i_volume_effects, true, 2000);
          b_brass_pack_sound_loop = true;
        }
      }
      else if(b_brass_pack_sound_loop) {
        stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT);
        b_brass_pack_sound_loop = false;
      }

      // Update the LEDs.
      cyclotronSwitchLEDUpdate();
    }
    else {
      // No need to have the Inner Cyclotron switch plate LEDs on when the lid is on.
      cyclotronSwitchLEDOff();

      // Stop the brass pack sound if it is playing.
      if(b_brass_pack_sound_loop) {
        stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT);
        b_brass_pack_sound_loop = false;
      }
    }

    // Setup the delays again.
    uint16_t i_cyc_led_delay = i_cyclotron_switch_led_delay / i_cyclotron_switch_led_mulitplier;

    switch(SYSTEM_YEAR) {
      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        if(ms_idle_fire_fade.remaining() > 0) {
          if(b_2021_ramp_up == true) {
            i_cyc_led_delay = i_cyclotron_switch_led_delay + (i_2021_ramp_delay - r_2021_ramp.update());
          }
          else if(b_2021_ramp_down == true) {
            i_cyc_led_delay = i_cyclotron_switch_led_delay + r_2021_ramp.update();
          }
        }
        else {
          if(b_2021_ramp_up == true) {
            i_cyc_led_delay = i_cyclotron_switch_led_delay + ((i_2021_ramp_delay / 2) - r_2021_ramp.update());
          }
          else if(b_2021_ramp_down == true) {
            i_cyc_led_delay = i_cyclotron_switch_led_delay + r_2021_ramp.update();
          }
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
    if(i_powercell_led < 0) {
      // Do Nothing.
    }
    else {
      pack_leds[i_powercell_led] = getHueAsRGB(POWERCELL, C_BLACK);

      i_powercell_led--;
    }

    if((b_overheating == true || b_2021_ramp_down == true || b_alarm == true) && b_powercell_sound_loop == true) {
      audio.trackLoop(S_POWERCELL, 0); // Turn off looping which stops the track.
      b_powercell_sound_loop = false;
    }

    // Setup the delays again.
    uint16_t i_pc_delay = i_powercell_delay;

    switch(SYSTEM_YEAR) {
      case SYSTEM_1984:
      case SYSTEM_1989:
        if(b_2021_ramp_up == true || b_2021_ramp_down == true) {
          i_pc_delay = i_powercell_delay + (r_2021_ramp.update() - i_1984_delay);
        }
      break;

      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        if(b_2021_ramp_up == true || b_2021_ramp_down == true) {
          i_pc_delay = i_powercell_delay + r_2021_ramp.update();
        }
      break;
    }

    if(b_alarm == true) {
      i_pc_delay = i_powercell_delay * 3;
    }

    ms_powercell.start(i_pc_delay);
  }
}

void powercellLoop() {
  if(ms_powercell.justFinished()) {
    uint16_t i_extra_delay = 0;

    // Power Cell
    if(i_powercell_led >= i_powercell_leds) {
      powercellOff();

      i_powercell_led = 0;
    }
    else {
      if(b_powercell_updating != true) {
        if(((SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE && b_cyclotron_lid_on) || SYSTEM_YEAR == SYSTEM_AFTERLIFE) && i_powercell_led == 0 && !b_2021_ramp_up && !b_2021_ramp_down && !b_wand_firing && !b_alarm && !b_overheating) {
          if(b_powercell_sound_loop != true) {
            b_powercell_sound_loop = true;
            stopEffect(S_POWERCELL);
            playEffect(S_POWERCELL, true, i_volume_effects, true, 1400);
          }
        }

        powercellDraw(i_powercell_led); // Update starting at a specific LED.

        // Add a small delay to pause the Power Cell when all Power Cell LEDs are lit up, to match Afterlife and Frozen Empire.
        if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) && !b_alarm && i_powercell_led == i_powercell_leds - 1) {
          i_extra_delay = 350;
        }

        i_powercell_led++;
      }
    }

    if((b_overheating || b_2021_ramp_down || b_alarm || (SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE && !b_cyclotron_lid_on)) && b_powercell_sound_loop) {
      audio.trackLoop(S_POWERCELL, 0); // Turn off looping which stops the track.
      b_powercell_sound_loop = false;
    }

    // Setup the delays again.
    uint16_t i_pc_delay = i_powercell_delay;

    switch(SYSTEM_YEAR) {
      case SYSTEM_1984:
      case SYSTEM_1989:
        if(b_2021_ramp_up == true || b_2021_ramp_down == true) {
          i_pc_delay = i_powercell_delay + (r_2021_ramp.update() - i_1984_delay);
        }
      break;

      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        if(b_2021_ramp_up == true || b_2021_ramp_down == true) {
          i_pc_delay = i_powercell_delay + r_2021_ramp.update();
        }
      break;
    }

    if(b_alarm == true) {
      i_pc_delay = i_powercell_delay * 5;
    }

    // Speed up the Power Cell when the cyclotron speeds up before a overheat.
    uint16_t i_multiplier = 0;

    if(i_powercell_multiplier > 1) {
      switch(i_powercell_multiplier) {
        case 2:
          if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
            if(i_pc_delay + i_extra_delay > 5) {
              i_multiplier = 5;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
          else {
            if(i_pc_delay + i_extra_delay > 10) {
              i_multiplier = 10;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
        break;

        case 3:
          if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
            if(i_pc_delay + i_extra_delay > 10) {
              i_multiplier = 10;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
          else {
            if(i_pc_delay + i_extra_delay > 20) {
              i_multiplier = 20;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
        break;

        case 4:
          if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
            if(i_pc_delay + i_extra_delay > 15) {
              i_multiplier = 15;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
          else {
            if(i_pc_delay + i_extra_delay > 30) {
              i_multiplier = 30;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
        break;

        case 5:
          if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
            if(i_pc_delay + i_extra_delay > 25) {
              i_multiplier = 25;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
          else {
            if(i_pc_delay + i_extra_delay > 40) {
              if(i_pc_delay + i_extra_delay > 40) {
                i_multiplier = 40;
              }
              else {
                i_multiplier = i_pc_delay + i_extra_delay;
              }
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
        break;

        case 6:
          if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
            i_multiplier = 30;
          }
          else {
            if(i_pc_delay + i_extra_delay > 50) {
              i_multiplier = 50;
            }
            else {
              i_multiplier = i_pc_delay + i_extra_delay;
            }
          }
        break;
      }
    }

    ms_powercell.start((i_pc_delay + i_extra_delay) - i_multiplier);
  }
}

void powercellOn() {
  i_powercell_led = i_powercell_leds - 1;

  powercellDraw();
}

void powercellOff() {
  for(uint8_t i = 0; i < i_powercell_leds; i++) {
    pack_leds[i] = getHueAsRGB(POWERCELL, C_BLACK);
  }

  i_powercell_led = 0;
}

void spectralLightsOff() {
  b_spectral_lights_on = false;

  for(uint8_t i = 0; i <= i_max_pack_leds - 1; i++) {
    pack_leds[i] = getHueAsRGB(POWERCELL, C_BLACK);
  }

  for(uint8_t i = i_ic_cake_start; i <= i_ic_cake_end; i++) {
    if(b_grb_cyclotron_cake == true) {
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
  for(uint8_t i = 0; i < i_powercell_leds; i++) {
    pack_leds[i] = getHueAsRGB(POWERCELL, i_colour_scheme);
  }

  uint8_t i_max = i_pack_num_leds - i_nfilter_jewel_leds - i_cyclotron_led_start;
  i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, SPECTRAL_CUSTOM, true);
  for(uint8_t i = 0; i < i_max; i++) {
    pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme);
  }

  i_colour_scheme = getDeviceColour(CYCLOTRON_INNER, SPECTRAL_CUSTOM, true);
  for(uint8_t i = i_ic_cake_start; i <= i_ic_cake_end; i++) {
    if(b_grb_cyclotron_cake == true) {
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
  uint8_t i_colour_scheme = getDeviceColour(POWERCELL, STREAM_MODE, b_powercell_colour_toggle);

  // Sets the colour for each Power Cell LED, subject to colour toggle setting.
  for(uint8_t i = i_start; i <= i_powercell_led; i++) {
    if(i_powercell_led < i_powercell_leds) {
      // Note: Always assumed to be RGB for built-in or Frutto LEDs.
      pack_leds[i] = getHueAsRGB(POWERCELL, i_colour_scheme, i_brightness);
    }
  }
}

// This function handles returning 1984 Cyclotron lookup table values.
uint8_t cyclotron84LookupTable(uint8_t index) {
  // First include a sanity check that will reject indexes above 3.
  if(index > 3) {
    index = 0;
  }

  if(b_clockwise) {
    switch(i_cyclotron_leds) {
      case HASLAB_CYCLOTRON_LED_COUNT:
      default:
        // 1984 CW 12 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_12_leds_cw[index]);
      break;

      case FRUTTO_CYCLOTRON_LED_COUNT:
        // 1984 CW 20 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_20_leds_cw[index]);
      break;

      case FRUTTO_MAX_CYCLOTRON_LED_COUNT:
        // 1984 CW 36 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_36_leds_cw[index]);
      break;

      case OUTER_CYCLOTRON_LED_MAX:
        // 1984 CW 40 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_40_leds_cw[index]);
      break;
    }
  }
  else {
    switch(i_cyclotron_leds) {
      case HASLAB_CYCLOTRON_LED_COUNT:
      default:
        // 1984 CCW 12 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_12_leds_ccw[index]);
      break;

      case FRUTTO_CYCLOTRON_LED_COUNT:
        // 1984 CCW 20 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_20_leds_ccw[index]);
      break;

      case FRUTTO_MAX_CYCLOTRON_LED_COUNT:
        // 1984 CCW 36 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_36_leds_ccw[index]);
      break;

      case OUTER_CYCLOTRON_LED_MAX:
        // 1984 CCW 40 LED array.
        return PROGMEM_READU8(i_1984_cyclotron_40_leds_ccw[index]);
      break;
    }
  }
}

// This function handles returning ring-simulated Cyclotron lookup table values.
uint8_t cyclotronLookupTable(uint8_t index) {
  switch(i_cyclotron_leds) {
    case HASLAB_CYCLOTRON_LED_COUNT:
    default:
      // Hasbro 12 LED array.
      return PROGMEM_READU8(i_cyclotron_12led_matrix[index]);
    break;

    case FRUTTO_CYCLOTRON_LED_COUNT:
      // Frutto 20 LED array.
      return PROGMEM_READU8(i_cyclotron_20led_matrix[index]);
    break;

    case FRUTTO_MAX_CYCLOTRON_LED_COUNT:
      // Frutto Max 36 LED array.
      return PROGMEM_READU8(i_cyclotron_36led_matrix[index]);
    break;

    case OUTER_CYCLOTRON_LED_MAX:
      // NeoPixel Ring 40 LED array.
      return PROGMEM_READU8(i_cyclotron_40led_matrix[index]);
    break;
  }
}

// Reset the Cyclotron LED colours.
void cyclotronColourReset() {
  uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, STREAM_MODE, b_cyclotron_colour_toggle);

  // We override the colour changes when using stock HasLab Cyclotron LEDs, returning full white.
  // Changing the colour space with a CHSV Object affects the brightness slightly for non RGB pixels.
  if(i_cyclotron_leds == HASLAB_CYCLOTRON_LED_COUNT && b_cyclotron_haslab_chsv_colour_change != true) {
    i_colour_scheme = C_HASLAB;
  }

  // Accounts for a total # of LEDs minus the N-Filter jewel and whatever precedes the Cyclotron.
  uint8_t i_max = i_pack_num_leds - i_nfilter_jewel_leds - i_cyclotron_led_start;
  for(uint8_t i = 0; i < i_max; i++) {
    if(b_cyclotron_led_on_status[i] == true) {
      // Note: Always assumed to be RGB for built-in or Frutto LEDs.
      // Sets 0-index <i> plus the position of the first Cyclotron LED.
      pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_cyclotron_led_value[i]);
    }
  }
}

void cyclotronControl() {
  // Only reset the starting LED when the pack is first started up.
  if(b_reset_start_led == true) {
    b_reset_start_led = false;

    switch(SYSTEM_YEAR) {
      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
      default:
        if(b_clockwise) {
          i_led_cyclotron = i_cyclotron_led_start;
        }
        else {
          i_led_cyclotron = i_cyclotron_led_start + 2; // Start on LED #2 in counter-clockwise mode in 2021 mode.
        }
      break;

      case SYSTEM_1984:
      case SYSTEM_1989:
        i_1984_counter = 3;
      break;
    }
  }

  if(ribbonCableAttached() != true && PACK_STATE != MODE_OFF && b_2021_ramp_down_start != true && b_overheating == false) {
    if(b_alarm == false) {
      b_2021_ramp_up = false;
      b_inner_ramp_up = false;
      b_alarm = true;

      if((SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) && !usingSlimeCyclotron()) {
        resetCyclotronState();
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
      b_2021_ramp_up = false;
      b_inner_ramp_up = false;

      if((SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) && !usingSlimeCyclotron()) {
        resetCyclotronState();
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

    cyclotronOverheating();
  }
  else {
    if(b_2021_ramp_up_start == true) {
      b_2021_ramp_up_start = false;

      switch(SYSTEM_YEAR) {
        case SYSTEM_1984:
        case SYSTEM_1989:
          r_2021_ramp.go(i_outer_current_ramp_speed); // Reset the ramp.
          r_2021_ramp.go(i_1984_delay, i_1984_ramp_length, CIRCULAR_OUT);
          r_inner_ramp.go(i_inner_current_ramp_speed); // Inner Cyclotron ramp reset.
          r_inner_ramp.go(i_1984_inner_delay, i_1984_ramp_length, CIRCULAR_OUT);
        break;

        case SYSTEM_AFTERLIFE:
        case SYSTEM_FROZEN_EMPIRE:
        default:
          if(ms_idle_fire_fade.remaining() > 0) {
            r_2021_ramp.go(i_outer_current_ramp_speed); // Reset the ramp.
            r_2021_ramp.go(i_2021_delay, i_2021_ramp_length, QUARTIC_OUT);
            r_inner_ramp.go(i_inner_current_ramp_speed); // Inner Cyclotron ramp reset.
            r_inner_ramp.go(i_2021_inner_delay, i_2021_ramp_length, QUARTIC_OUT);
          }
          else {
            r_2021_ramp.go(i_outer_current_ramp_speed); // Reset the ramp.
            r_2021_ramp.go(i_2021_delay, (uint16_t)(i_2021_ramp_length / 1.5), QUADRATIC_OUT);
            r_inner_ramp.go(i_inner_current_ramp_speed); // Inner Cyclotron ramp reset.
            r_inner_ramp.go(i_2021_inner_delay, i_2021_ramp_length, QUADRATIC_OUT);
          }
        break;
      }
    }
    else if(b_2021_ramp_down_start == true) {
      b_2021_ramp_down_start = false;

      r_2021_ramp.go(i_outer_current_ramp_speed); // Reset the ramp.
      r_inner_ramp.go(i_inner_current_ramp_speed); // Reset the Inner Cyclotron ramp.

      if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
        r_2021_ramp.go((uint16_t)(i_1984_delay * 1.3), i_1984_ramp_down_length, CIRCULAR_IN);
        r_inner_ramp.go(i_inner_ramp_delay, i_1984_ramp_down_length, CIRCULAR_IN);
      }
      else {
        r_2021_ramp.go(i_2021_ramp_delay, i_2021_ramp_down_length, QUARTIC_IN);
        r_inner_ramp.go(i_inner_ramp_delay, i_2021_ramp_down_length, QUARTIC_IN);
      }
    }

    if(!usingSlimeCyclotron()) {
      if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
        cyclotron1984(i_outer_current_ramp_speed);
      }
      else {
        cyclotron2021(i_outer_current_ramp_speed);
      }
    }

    innerCyclotronRingUpdate(i_inner_current_ramp_speed);
  }

  if(b_cyclotron_lid_on && !usingSlimeCyclotron()) {
    cyclotronFade();
  }
}

void cyclotronFade() {
  uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, STREAM_MODE, b_cyclotron_colour_toggle);

  // We override the colour changes when using stock HasLab Cyclotron LEDs.
  // Changing the colour space with a CHSV Object affects the brightness slightly for non RGB pixels.
  if(i_cyclotron_leds == HASLAB_CYCLOTRON_LED_COUNT && b_cyclotron_haslab_chsv_colour_change != true) {
    i_colour_scheme = C_HASLAB;
  }

  uint8_t i_cyclotron_leds_total = i_pack_num_leds - i_nfilter_jewel_leds - i_cyclotron_led_start;

  if(b_cyclotron_simulate_ring == true) {
    i_cyclotron_leds_total = OUTER_CYCLOTRON_LED_MAX;
  }

  switch (SYSTEM_YEAR) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      for(uint8_t i = 0; i < i_cyclotron_leds_total; i++) {
        if(ms_cyclotron_led_fade_in[i].isRunning()) {
          b_cyclotron_led_on_status[i] = true;

          uint8_t i_curr_brightness = ms_cyclotron_led_fade_in[i].update();
          i_cyclotron_led_value[i] = i_curr_brightness;

          if(b_cyclotron_simulate_ring == true) {
            pack_leds[cyclotronLookupTable(i) + i_cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
          }
          else {
            pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
          }
        }

        uint8_t i_new_brightness = getBrightness(i_cyclotron_brightness);
        if(ms_cyclotron_led_fade_in[i].isFinished() && i_cyclotron_led_value[i] > (i_new_brightness - 1) && b_cyclotron_led_on_status[i] == true) {
          i_cyclotron_led_value[i] = i_new_brightness;
          b_cyclotron_led_on_status[i] = false;

          ms_cyclotron_led_fade_out[i].go(i_new_brightness);

          switch(i_cyclotron_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
            case FRUTTO_CYCLOTRON_LED_COUNT:
            case FRUTTO_MAX_CYCLOTRON_LED_COUNT:
              ms_cyclotron_led_fade_out[i].go(0, i_outer_current_ramp_speed * 3, CIRCULAR_OUT);
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
            default:
              ms_cyclotron_led_fade_out[i].go(0, i_outer_current_ramp_speed * 2, CIRCULAR_OUT);
            break;
          }

          if(b_cyclotron_simulate_ring == true) {
            pack_leds[cyclotronLookupTable(i) + i_cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_new_brightness);
          }
          else {
            pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_new_brightness);
          }
        }

        if(ms_cyclotron_led_fade_out[i].isRunning() && b_cyclotron_led_on_status[i] == false) {
          uint8_t i_curr_brightness = ms_cyclotron_led_fade_out[i].update();
          i_cyclotron_led_value[i] = i_curr_brightness;

          if(b_cyclotron_simulate_ring == true) {
            pack_leds[cyclotronLookupTable(i) + i_cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
          }
          else {
            pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
          }
        }

        if(ms_cyclotron_led_fade_out[i].isFinished() && b_cyclotron_led_on_status[i] == false) {
          i_cyclotron_led_value[i] = 0;
          b_cyclotron_led_on_status[i] = true;

          if(b_cyclotron_simulate_ring == true) {
            pack_leds[cyclotronLookupTable(i) + i_cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
          }
          else {
            pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
          }
        }
      }
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      if(b_fade_cyclotron_led == true) {
        for(uint8_t i = 0; i < i_pack_num_leds - i_nfilter_jewel_leds - i_cyclotron_led_start; i++) {
          if(ms_cyclotron_led_fade_in[i].isRunning()) {
            b_cyclotron_led_on_status[i] = true;
            uint8_t i_curr_brightness = ms_cyclotron_led_fade_in[i].update();

            pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
            i_cyclotron_led_value[i] = i_curr_brightness;
          }

          uint8_t i_new_brightness = getBrightness(i_cyclotron_brightness);

          if(ms_cyclotron_led_fade_in[i].isFinished() && i_cyclotron_led_value[i] > (i_new_brightness - 1) && b_cyclotron_led_on_status[i] == true) {
            pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_new_brightness);
            i_cyclotron_led_value[i] = i_new_brightness;
          }

          if(ms_cyclotron_led_fade_out[i].isRunning()) {
            uint8_t i_curr_brightness = ms_cyclotron_led_fade_out[i].update();

            if(i_curr_brightness < 30) {
              ms_cyclotron_led_fade_out[i].go(0);

              pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
              i_cyclotron_led_value[i] = 0;
              b_cyclotron_led_on_status[i] = true;
            }
            else {
              pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_curr_brightness);
              i_cyclotron_led_value[i] = i_curr_brightness;
              b_cyclotron_led_on_status[i] = false;
            }
          }

          if(ms_cyclotron_led_fade_out[i].isFinished() && b_cyclotron_led_on_status[i] == false) {
            pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
            i_cyclotron_led_value[i] = 0;
            b_cyclotron_led_on_status[i] = true;
          }
        }
      }
    break;
  }
}

void cyclotron2021(uint16_t iRampDelay) {
  uint8_t i_brightness = getBrightness(i_cyclotron_brightness); // Calculate desired brightness.

  if(ms_cyclotron.justFinished()) {
    uint8_t i_cyclotron_matrix_led = cyclotronLookupTable(i_led_cyclotron - i_cyclotron_led_start);

    if(b_2021_ramp_up == true) {
      i_fast_led_delay = FAST_LED_UPDATE_MS;

      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_up = false;
        i_outer_current_ramp_speed = iRampDelay;

        if(b_cyclotron_simulate_ring == true) {
          switch(i_cyclotron_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
              ms_cyclotron.start(i_outer_current_ramp_speed);
            break;

            case FRUTTO_MAX_CYCLOTRON_LED_COUNT:
              if(i_cyclotron_matrix_led > 0) {
                ms_cyclotron.start(i_outer_current_ramp_speed);
              }
              else {
                ms_cyclotron.start(i_outer_current_ramp_speed * 9);
              }
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
            case FRUTTO_CYCLOTRON_LED_COUNT:
            default:
              if(i_cyclotron_matrix_led > 0) {
                ms_cyclotron.start(i_outer_current_ramp_speed);
              }
              else {
                ms_cyclotron.start(i_outer_current_ramp_speed - i_2021_delay);
              }
            break;
          }
        }
        else {
          ms_cyclotron.start(i_outer_current_ramp_speed);
        }

        i_vibration_level = i_vibration_idle_level_2021;
      }
      else {
        i_outer_current_ramp_speed = r_2021_ramp.update();

        if(b_cyclotron_simulate_ring == true) {
          switch(i_cyclotron_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
              ms_cyclotron.start(i_outer_current_ramp_speed);
            break;

            case FRUTTO_MAX_CYCLOTRON_LED_COUNT:
              if(i_cyclotron_matrix_led > 0) {
                ms_cyclotron.start(i_outer_current_ramp_speed);
              }
              else {
                if(i_outer_current_ramp_speed * 9 > 1200) {
                  ms_cyclotron.start(i_outer_current_ramp_speed * 6);
                }
                else {
                  ms_cyclotron.start(i_outer_current_ramp_speed * 9);
                }
              }
            break;

            case FRUTTO_CYCLOTRON_LED_COUNT:
              if(i_cyclotron_matrix_led > 0) {
                ms_cyclotron.start(i_outer_current_ramp_speed);
              }
              else {
                ms_cyclotron.start(i_outer_current_ramp_speed - i_2021_delay);
              }
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
            default:
              if(i_cyclotron_matrix_led > 0) {
                ms_cyclotron.start(i_outer_current_ramp_speed);
              }
              else {
                ms_cyclotron.start(i_outer_current_ramp_speed - i_2021_delay);
              }
            break;
          }
        }
        else {
          ms_cyclotron.start(i_outer_current_ramp_speed);
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
      i_fast_led_delay = FAST_LED_UPDATE_MS;

      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_down = false;
      }
      else {
        i_outer_current_ramp_speed = r_2021_ramp.update();

        if(b_cyclotron_simulate_ring == true) {
          switch(i_cyclotron_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
              ms_cyclotron.start(i_outer_current_ramp_speed);
            break;

            case FRUTTO_MAX_CYCLOTRON_LED_COUNT:
              if(i_cyclotron_matrix_led > 0) {
                ms_cyclotron.start(i_outer_current_ramp_speed);
              }
              else {
                ms_cyclotron.start(i_outer_current_ramp_speed * 9);
              }
            break;

            case FRUTTO_CYCLOTRON_LED_COUNT:
              if(i_cyclotron_matrix_led > 0) {
                ms_cyclotron.start(i_outer_current_ramp_speed);
              }
              else {
                ms_cyclotron.start(i_outer_current_ramp_speed - i_2021_delay);
              }
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
            default:
              if(i_cyclotron_matrix_led > 0) {
                ms_cyclotron.start(i_outer_current_ramp_speed);
              }
              else {
                ms_cyclotron.start(i_outer_current_ramp_speed - i_2021_delay);
              }
            break;
          }
        }
        else {
          ms_cyclotron.start(i_outer_current_ramp_speed);
        }

        if(i_outer_current_ramp_speed > 40 && i_vibration_level > i_vibration_lowest_level + 20) {
          i_vibration_level = i_vibration_level - 1;
        }
        else if(i_outer_current_ramp_speed > 100 && i_vibration_level > i_vibration_lowest_level) {
          i_vibration_level = i_vibration_level - 1;
        }

        if(i_vibration_level < i_vibration_lowest_level) {
          i_vibration_level = i_vibration_lowest_level;
        }
      }
    }
    else {
      i_outer_current_ramp_speed = iRampDelay;

      uint16_t t_iRampDelay = iRampDelay;

      switch(i_cyclotron_leds) {
        case OUTER_CYCLOTRON_LED_MAX:
        case FRUTTO_MAX_CYCLOTRON_LED_COUNT:
          if(i_cyclotron_multiplier > 1) {
            if(t_iRampDelay - i_cyclotron_multiplier < t_iRampDelay) {
              t_iRampDelay = t_iRampDelay - i_cyclotron_multiplier;
            }
            else {
              t_iRampDelay = 0;
            }

            if(b_cyclotron_lid_on == true) {
              i_fast_led_delay = FAST_LED_UPDATE_MS + i_cyclotron_multiplier;
            }
            else {
              i_fast_led_delay = FAST_LED_UPDATE_MS;
            }
          }
          else {
            i_fast_led_delay = FAST_LED_UPDATE_MS;
          }

          if(i_fast_led_delay > 10) {
            i_fast_led_delay = 10;
          }
        break;

        case FRUTTO_CYCLOTRON_LED_COUNT:
        case HASLAB_CYCLOTRON_LED_COUNT:
        default:
          i_fast_led_delay = FAST_LED_UPDATE_MS;

          if(i_cyclotron_multiplier > 1) {
            if(t_iRampDelay - i_cyclotron_multiplier < t_iRampDelay) {
              t_iRampDelay = t_iRampDelay - i_cyclotron_multiplier;
            }
            else {
              t_iRampDelay = 0;
            }
          }
        break;
      }

      if(t_iRampDelay < 1) {
        t_iRampDelay = 1;
      }

      if(b_cyclotron_simulate_ring == true) {
        switch(i_cyclotron_leds) {
          case OUTER_CYCLOTRON_LED_MAX:
            ms_cyclotron.start(t_iRampDelay);
          break;

          case FRUTTO_MAX_CYCLOTRON_LED_COUNT:
            if(i_cyclotron_matrix_led > 0) {
              ms_cyclotron.start(t_iRampDelay);
            }
            else if(i_outer_current_ramp_speed > i_2021_delay && t_iRampDelay - i_2021_delay < t_iRampDelay) {
              ms_cyclotron.start((t_iRampDelay - i_2021_delay) * 9); // This will simulate the fake LEDs during overheat and ribbon cable alarms.
            }
            else {
              ms_cyclotron.start(0);
            }
          break;

          case FRUTTO_CYCLOTRON_LED_COUNT:
            if(i_cyclotron_matrix_led > 0) {
              ms_cyclotron.start(t_iRampDelay);
            }
            else if(i_outer_current_ramp_speed > i_2021_delay && t_iRampDelay - i_2021_delay < t_iRampDelay) {
              ms_cyclotron.start(t_iRampDelay - i_2021_delay); // This will simulate the fake LEDs during overheat and ribbon cable alarms.
            }
            else {
              ms_cyclotron.start(0);
            }
          break;

          case HASLAB_CYCLOTRON_LED_COUNT:
          default:
            if(i_cyclotron_matrix_led > 0) {
              ms_cyclotron.start(t_iRampDelay);
            }
            else if(i_outer_current_ramp_speed > i_2021_delay && t_iRampDelay - i_2021_delay < t_iRampDelay) {
              ms_cyclotron.start(t_iRampDelay - i_2021_delay); // This will simulate the fake LEDs during overheat and ribbon cable alarms.
            }
            else {
              ms_cyclotron.start(0);
            }
          break;
        }
      }
      else {
        ms_cyclotron.start(t_iRampDelay);
      }
    }

    if(b_wand_firing != true && b_overheating != true && b_alarm != true) {
      vibrationPack(i_vibration_level);
    }

    switch(i_cyclotron_leds) {
      case FRUTTO_MAX_CYCLOTRON_LED_COUNT:
        if(i_cyclotron_multiplier > 1) {
          if(iRampDelay - i_cyclotron_multiplier < iRampDelay) {
            iRampDelay = iRampDelay - i_cyclotron_multiplier;
          }
          else {
            iRampDelay = 0;
          }
        }
        else {
          iRampDelay = iRampDelay / i_cyclotron_multiplier;

          if(b_2021_ramp_up == true || b_2021_ramp_down == true) {
            iRampDelay = iRampDelay * 1;
          }
          else {
            iRampDelay = iRampDelay * 3;
          }
        }
      break;

      case OUTER_CYCLOTRON_LED_MAX:
      case FRUTTO_CYCLOTRON_LED_COUNT:
        if(i_cyclotron_multiplier > 1) {
          if(iRampDelay - i_cyclotron_multiplier < iRampDelay) {
            iRampDelay = iRampDelay - i_cyclotron_multiplier;
          }
          else {
            iRampDelay = 0;
          }
        }
        else {
          iRampDelay = iRampDelay / i_cyclotron_multiplier;

          iRampDelay = iRampDelay * 3;
        }
      break;

      case HASLAB_CYCLOTRON_LED_COUNT:
      default:
        if(i_cyclotron_multiplier > 1) {
          if(iRampDelay - i_cyclotron_multiplier < iRampDelay) {
            iRampDelay = iRampDelay - i_cyclotron_multiplier;
          }
          else {
            iRampDelay = 0;
          }
        }
        else {
          iRampDelay = iRampDelay / i_cyclotron_multiplier;

          iRampDelay = iRampDelay * 2;
        }
      break;
    }

    if(iRampDelay < 1) {
      iRampDelay = 1;
    }

    if((i_cyclotron_led_value[i_led_cyclotron - i_cyclotron_led_start] == 0 && b_cyclotron_simulate_ring != true) || (i_cyclotron_led_value[i_led_cyclotron - i_cyclotron_led_start] == 0 && b_cyclotron_simulate_ring == true && i_cyclotron_matrix_led > 0)) {
      ms_cyclotron_led_fade_in[i_led_cyclotron - i_cyclotron_led_start].go(0);
      ms_cyclotron_led_fade_in[i_led_cyclotron - i_cyclotron_led_start].go(i_brightness, iRampDelay, CIRCULAR_IN);
    }

    // If we are in slime mode, call the slime effect functions instead.
    if(usingSlimeCyclotron()) {
      if(PACK_STATE == MODE_ON && !ms_cyclotron_slime_effect.isRunning()) {
        // Make sure we've started the slime effect timer if it hasn't been started already.
        ms_cyclotron_slime_effect.start(0);
      }

      slimeCyclotronEffect();
      return;
    }

    if(b_clockwise == true) {
      i_led_cyclotron++;

      if(b_cyclotron_simulate_ring == true) {
        if(i_led_cyclotron > (i_powercell_leds + OUTER_CYCLOTRON_LED_MAX + i_nfilter_jewel_leds) - i_nfilter_jewel_leds - 1) {
          i_led_cyclotron = i_cyclotron_led_start;
        }
        else if(i_outer_current_ramp_speed <= i_2021_delay) {
          switch(i_cyclotron_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
              // Do nothing.
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
            case FRUTTO_CYCLOTRON_LED_COUNT:
            case FRUTTO_MAX_CYCLOTRON_LED_COUNT:
            default:
              i_cyclotron_matrix_led = cyclotronLookupTable(i_led_cyclotron - i_cyclotron_led_start);

              if(i_cyclotron_matrix_led == 0) {
                for(uint8_t i = i_led_cyclotron; i < OUTER_CYCLOTRON_LED_MAX; i++) {
                  if(cyclotronLookupTable(i - i_cyclotron_led_start) > 0) {
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
          i_led_cyclotron = i_cyclotron_led_start;
        }
      }
    }
    else {
      i_led_cyclotron--;

      if(b_cyclotron_simulate_ring == true) {
        if(i_led_cyclotron < i_cyclotron_led_start) {
          i_led_cyclotron = (i_powercell_leds + OUTER_CYCLOTRON_LED_MAX + i_nfilter_jewel_leds) - i_nfilter_jewel_leds - 1;
        }
        else if(i_outer_current_ramp_speed <= i_2021_delay) {
          switch(i_cyclotron_leds) {
            case OUTER_CYCLOTRON_LED_MAX:
              // Do nothing.
            break;

            case HASLAB_CYCLOTRON_LED_COUNT:
            case FRUTTO_CYCLOTRON_LED_COUNT:
            case FRUTTO_MAX_CYCLOTRON_LED_COUNT:
            default:
              i_cyclotron_matrix_led = cyclotronLookupTable(i_led_cyclotron - i_cyclotron_led_start);

              if(i_cyclotron_matrix_led == 0) {
                for(uint8_t i = i_led_cyclotron; i > i_cyclotron_led_start; i--) {
                  if(cyclotronLookupTable(i - i_cyclotron_led_start) > 0) {
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
        if(i_led_cyclotron < i_cyclotron_led_start) {
          i_led_cyclotron = i_pack_num_leds - i_nfilter_jewel_leds - 1;
        }
      }
    }
  }
}

void cyclotron1984(uint16_t iRampDelay) {
  i_fast_led_delay = FAST_LED_UPDATE_MS;

  if(ms_cyclotron.justFinished()) {
    iRampDelay = iRampDelay / i_cyclotron_multiplier;

    if(b_2021_ramp_up == true) {
      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_up = false;

        ms_cyclotron.start(iRampDelay);
        i_outer_current_ramp_speed = iRampDelay;


        i_vibration_level = i_vibration_idle_level_1984;
      }
      else {
        ms_cyclotron.start(r_2021_ramp.update());
        i_outer_current_ramp_speed = r_2021_ramp.update();

        i_vibration_level = i_vibration_idle_level_1984;
      }
    }
    else if(b_2021_ramp_down == true) {
      if(r_2021_ramp.isFinished()) {
        b_2021_ramp_down = false;
      }
      else {
        ms_cyclotron.start(r_2021_ramp.update());
        i_outer_current_ramp_speed = r_2021_ramp.update();

        i_vibration_level = i_vibration_level - 1;

        if(i_vibration_level < i_vibration_lowest_level) {
          i_vibration_level = i_vibration_lowest_level;
        }
      }
    }
    else {
      ms_cyclotron.start(iRampDelay);
    }

    if(b_wand_firing != true && b_overheating != true && b_alarm != true) {
      vibrationPack(i_vibration_level);
    }

    // If we are in slime mode, call the slime effect functions instead.
    if(usingSlimeCyclotron()) {
      if(PACK_STATE == MODE_ON && !ms_cyclotron_slime_effect.isRunning()) {
        // Make sure we've started the slime effect timer if it hasn't been started already.
        ms_cyclotron_slime_effect.start(0);
      }

      slimeCyclotronEffect();
      return;
    }

    if(b_1984_led_start != true) {
      cyclotron84LightOff(i_led_cyclotron);
    }
    else {
      b_1984_led_start = false;
    }

    i_1984_counter++;

    if(i_1984_counter > 3) {
      i_1984_counter = 0;
    }

    i_led_cyclotron = i_cyclotron_led_start + cyclotron84LookupTable(i_1984_counter);

    cyclotron84LightOn(i_led_cyclotron);
  }
}

void cyclotron1984Alarm() {
  uint8_t i_brightness = getBrightness(i_cyclotron_brightness);
  uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, STREAM_MODE, b_cyclotron_colour_toggle);
  uint8_t led1 = i_cyclotron_led_start + cyclotron84LookupTable(0);
  uint8_t led2 = i_cyclotron_led_start + cyclotron84LookupTable(1);
  uint8_t led3 = i_cyclotron_led_start + cyclotron84LookupTable(2);
  uint8_t led4 = i_cyclotron_led_start + cyclotron84LookupTable(3);

  if(b_fade_cyclotron_led != true) {
    pack_leds[led1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
    pack_leds[led2] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
    pack_leds[led3] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
    pack_leds[led4] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

    // Turn on all the other cyclotron LEDs if required.
    if(b_cyclotron_single_led != true) {
      pack_leds[led1 + 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

      if(led1 - 1 < i_cyclotron_led_start) {
        led1 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led1 = led1 - 1;
      }

      pack_leds[led1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
      pack_leds[led2 + 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

      if(led2 - 1 < i_cyclotron_led_start) {
        led2 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led2 = led2 - 1;
      }

      pack_leds[led2] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
      pack_leds[led3 + 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

      if(led3 - 1 < i_cyclotron_led_start) {
        led3 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led3 = led3 - 1;
      }

      pack_leds[led3] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
      pack_leds[led4 + 1] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);

      if(led4 - 1 < i_cyclotron_led_start) {
        led4 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led4 = led4 - 1;
      }

      pack_leds[led4] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
    }
  }
  else {
    uint8_t i_brightness_tmp = 0;

    if(i_cyclotron_led_value[led1 - i_cyclotron_led_start] == i_brightness_tmp) {
      ms_cyclotron_led_fade_in[led1 - i_cyclotron_led_start].go(i_brightness_tmp);
      ms_cyclotron_led_fade_in[led1 - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
    }

    if(i_cyclotron_led_value[led2 - i_cyclotron_led_start] == i_brightness_tmp) {
      ms_cyclotron_led_fade_in[led2 - i_cyclotron_led_start].go(i_brightness_tmp);
      ms_cyclotron_led_fade_in[led2 - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
    }

    if(i_cyclotron_led_value[led3 - i_cyclotron_led_start] == i_brightness_tmp) {
      ms_cyclotron_led_fade_in[led3 - i_cyclotron_led_start].go(i_brightness_tmp);
      ms_cyclotron_led_fade_in[led3 - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
    }

    if(i_cyclotron_led_value[led4 - i_cyclotron_led_start] == i_brightness_tmp) {
      ms_cyclotron_led_fade_in[led4 - i_cyclotron_led_start].go(i_brightness_tmp);
      ms_cyclotron_led_fade_in[led4 - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
    }

    // Turn on all the other cyclotron LEDs if required.
    if(b_cyclotron_single_led != true) {
      if(i_cyclotron_led_value[led1 + 1 - i_cyclotron_led_start] == i_brightness_tmp) {
        ms_cyclotron_led_fade_in[led1 + 1 - i_cyclotron_led_start].go(i_brightness_tmp);
        ms_cyclotron_led_fade_in[led1 + 1 - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(led1 - 1 < i_cyclotron_led_start) {
        led1 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led1 = led1 - 1;
      }

      if(i_cyclotron_led_value[led1  - i_cyclotron_led_start] == i_brightness_tmp) {
        ms_cyclotron_led_fade_in[led1 - i_cyclotron_led_start].go(i_brightness_tmp);
        ms_cyclotron_led_fade_in[led1 - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(i_cyclotron_led_value[led2 + 1 - i_cyclotron_led_start] == i_brightness_tmp) {
        ms_cyclotron_led_fade_in[led2 + 1 - i_cyclotron_led_start].go(i_brightness_tmp);
        ms_cyclotron_led_fade_in[led2 + 1 - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(led2 - 1 < i_cyclotron_led_start) {
        led2 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led2 = led2 - 1;
      }

      if(i_cyclotron_led_value[led2 - i_cyclotron_led_start] == i_brightness_tmp) {
        ms_cyclotron_led_fade_in[led2 - i_cyclotron_led_start].go(i_brightness_tmp);
        ms_cyclotron_led_fade_in[led2 - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(i_cyclotron_led_value[led3 + 1 - i_cyclotron_led_start] == i_brightness_tmp) {
        ms_cyclotron_led_fade_in[led3 + 1 - i_cyclotron_led_start].go(i_brightness_tmp);
        ms_cyclotron_led_fade_in[led3 + 1 - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(led3 - 1 < i_cyclotron_led_start) {
        led3 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led3 = led3 - 1;
      }

      if(i_cyclotron_led_value[led3 - i_cyclotron_led_start] == i_brightness_tmp) {
        ms_cyclotron_led_fade_in[led3 - i_cyclotron_led_start].go(i_brightness_tmp);
        ms_cyclotron_led_fade_in[led3 - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(i_cyclotron_led_value[led4 + 1 - i_cyclotron_led_start] == i_brightness_tmp) {
        ms_cyclotron_led_fade_in[led4 + 1 - i_cyclotron_led_start].go(i_brightness_tmp);
        ms_cyclotron_led_fade_in[led4 + 1 - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }

      if(led4 - 1 < i_cyclotron_led_start) {
        led4 = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        led4 = led4 - 1;
      }

      if(i_cyclotron_led_value[led4 - i_cyclotron_led_start] == i_brightness_tmp) {
        ms_cyclotron_led_fade_in[led4 - i_cyclotron_led_start].go(i_brightness_tmp);
        ms_cyclotron_led_fade_in[led4 - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay, CIRCULAR_IN);
      }
    }
  }
}

void cyclotron84LightOn(uint8_t cLed) {
  uint8_t i_brightness = getBrightness(i_cyclotron_brightness);
  uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, STREAM_MODE, b_cyclotron_colour_toggle);

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

      if(cLed - 1 < i_cyclotron_led_start) {
        cLed = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        cLed = cLed - 1;
      }

      pack_leds[cLed] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, i_brightness);
    }
  }
  else {
    uint8_t i_brightness_tmp = 0;

    if(i_cyclotron_led_value[cLed - i_cyclotron_led_start] == i_brightness_tmp) {
      ms_cyclotron_led_fade_in[cLed - i_cyclotron_led_start].go(i_brightness_tmp);
      ms_cyclotron_led_fade_in[cLed - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay / i_cyclotron_multiplier, CIRCULAR_IN);
    }

    // Turn on the other 2 LEDs if we are allowing 3 to light up.
    if(b_cyclotron_single_led != true) {
      if(i_cyclotron_led_value[cLed + 1 - i_cyclotron_led_start] == i_brightness_tmp) {
        ms_cyclotron_led_fade_in[cLed + 1 - i_cyclotron_led_start].go(i_brightness_tmp);
        ms_cyclotron_led_fade_in[cLed + 1 - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay / i_cyclotron_multiplier, CIRCULAR_IN);
      }

      if(cLed - 1 < i_cyclotron_led_start) {
        cLed = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        cLed = cLed - 1;
      }

      if(i_cyclotron_led_value[cLed - i_cyclotron_led_start] == i_brightness_tmp) {
        ms_cyclotron_led_fade_in[cLed - i_cyclotron_led_start].go(i_brightness_tmp);
        ms_cyclotron_led_fade_in[cLed - i_cyclotron_led_start].go(i_brightness, i_1984_fade_in_delay / i_cyclotron_multiplier, CIRCULAR_IN);
      }
    }
  }
}

void cyclotron84LightOff(uint8_t cLed) {
  uint8_t i_brightness = getBrightness(i_cyclotron_brightness); // Calculate desired brightness.

  if(b_fade_cyclotron_led != true) {
    pack_leds[cLed] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);

    // Turn off the other 2 LEDs if we are allowing 3 to light up.
    if(b_cyclotron_single_led != true) {
      pack_leds[cLed + 1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);

      if(cLed - 1 < i_cyclotron_led_start) {
        cLed = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        cLed = cLed - 1;
      }

      pack_leds[cLed] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
    }
  }
  else {
    uint8_t i_brightness_tmp = 0;

    if(i_cyclotron_led_value[cLed - i_cyclotron_led_start] == i_brightness) {
      ms_cyclotron_led_fade_out[cLed - i_cyclotron_led_start].go(i_brightness);
      ms_cyclotron_led_fade_out[cLed - i_cyclotron_led_start].go(i_brightness_tmp, i_1984_fade_out_delay / i_cyclotron_multiplier, CIRCULAR_OUT);
    }

    // Turn off the other 2 LEDs if we are allowing 3 to light up.
    if(b_cyclotron_single_led != true) {
      if(i_cyclotron_led_value[cLed + 1 - i_cyclotron_led_start] == i_brightness) {
        ms_cyclotron_led_fade_out[cLed + 1 - i_cyclotron_led_start].go(i_brightness);
        ms_cyclotron_led_fade_out[cLed + 1 - i_cyclotron_led_start].go(i_brightness_tmp, i_1984_fade_out_delay / i_cyclotron_multiplier, CIRCULAR_OUT);
      }

      if(cLed - 1 < i_cyclotron_led_start) {
        cLed = i_pack_num_leds - i_nfilter_jewel_leds - 1;
      }
      else {
        cLed = cLed - 1;
      }

      if(i_cyclotron_led_value[cLed - i_cyclotron_led_start] == i_brightness) {
        ms_cyclotron_led_fade_out[cLed - i_cyclotron_led_start].go(i_brightness);
        ms_cyclotron_led_fade_out[cLed - i_cyclotron_led_start].go(i_brightness_tmp, i_1984_fade_out_delay / i_cyclotron_multiplier, CIRCULAR_OUT);
      }
    }
  }
}

// Returns whether we should be using the slime cyclotron effect or not.
bool usingSlimeCyclotron() {
  if(STREAM_MODE == SLIME && b_cyclotron_colour_toggle) {
    return true;
  }
  else {
    return false;
  }
}

// Controls the slime cyclotron effect.
void slimeCyclotronEffect() {
  if(ms_cyclotron_slime_effect.justFinished()) {
    if(PACK_STATE == MODE_OFF && b_2021_ramp_down) {
      slimeCyclotronFadeout();
      return;
    }

    uint8_t i_cyclotron_leds_total = i_pack_num_leds - i_nfilter_jewel_leds - i_cyclotron_led_start;
    uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_OUTER, STREAM_MODE, b_cyclotron_colour_toggle);
    uint8_t i_random_lower = 50;
    uint8_t i_random_upper = 121;

    if(b_wand_firing) {
      i_random_lower = 40;

      switch(i_wand_power_level) {
        case 1:
        default:
          i_random_upper = 121;
        break;

        case 2:
          i_random_upper = 151;
        break;

        case 3:
          i_random_upper = 181;
        break;

        case 4:
          i_random_upper = 201;
        break;

        case 5:
          i_random_upper = 221;
        break;
      }
    }
    else if(b_alarm || b_overheating || b_venting) {
      i_random_lower = 20;
      i_random_upper = 41;
    }

    for(uint8_t i = 0; i < i_cyclotron_leds_total; i++) {
      pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, i_colour_scheme, random(i_random_lower, i_random_upper));
    }

    if(i_random_lower == 50 && i_random_upper == 121) {
      ms_cyclotron_slime_effect.start(random(70,101));
    }
    else {
      switch(i_wand_power_level) {
        case 1:
        default:
          ms_cyclotron_slime_effect.start(100);
        break;

        case 2:
          ms_cyclotron_slime_effect.start(90);
        break;

        case 3:
          ms_cyclotron_slime_effect.start(80);
        break;

        case 4:
          ms_cyclotron_slime_effect.start(70);
        break;

        case 5:
          ms_cyclotron_slime_effect.start(60);
        break;
      }
    }
  }

  if(b_wand_firing != true && b_overheating != true && b_alarm != true) {
    vibrationPack(i_vibration_level);
  }
}

// Controls the slime cyclotron fadeout effect.
void slimeCyclotronFadeout() {
  //if(ms_cyclotron_slime_effect.justFinished()) {
    uint8_t i_cyclotron_leds_total = i_pack_num_leds - i_nfilter_jewel_leds - i_cyclotron_led_start;
    bool b_leds_fading = false;

    for(uint8_t i = 0; i < i_cyclotron_leds_total; i++) {
      pack_leds[i + i_cyclotron_led_start].fadeToBlackBy(1);

      if(!b_leds_fading && pack_leds[i + i_cyclotron_led_start]) {
        b_leds_fading = true;
      }
    }

    if(b_leds_fading) {
      // At least one LED not off yet.
      ms_cyclotron_slime_effect.start(30);
    }
    else {
      // All LEDs faded to black.
      ms_cyclotron_slime_effect.stop();
      b_2021_ramp_down = false;
    }
  //}
}

void packVenting() {
  if(b_overheat_sync_to_fan != true && STREAM_MODE != SLIME) {
    smokeNFilter(true);
  }

  if(ms_overheating.justFinished()) {
    if(STREAM_MODE == SLIME) {
      // Play the sound of slime refilling the tank.
      playEffect(S_SLIME_REFILL, true);
    }
    else {
      uint8_t i_smoke_random = random(5);

      switch(i_smoke_random) {
        case 4:
          playEffect(S_VENT_SMOKE_4, false, i_volume_effects, true, 120);
        break;

        case 3:
          playEffect(S_VENT_SMOKE_3, false, i_volume_effects, true, 120);
        break;

        case 2:
          playEffect(S_VENT_SMOKE_2, false, i_volume_effects, true, 120);
        break;

        case 1:
          playEffect(S_VENT_SMOKE_1, false, i_volume_effects, true, 120);
        break;

        case 0:
        default:
          playEffect(S_VENT_SMOKE, false, i_volume_effects, true, 120);
        break;
      }

      // Fade in the steam release loop.
      playEffect(S_STEAM_LOOP, true, i_volume_effects, true, 1000);
    }

    switch(i_wand_power_level) {
      case 1:
      default:
        ms_overheating_length.start(i_ms_overheating_length_1 >= 4000 ? i_ms_overheating_length_1 / 2 : 2000);

        if(b_overheat_sync_to_fan != true && STREAM_MODE != SLIME) {
          ms_smoke_on.stop();
          ms_smoke_on.start(i_ms_overheating_length_1 >= 4000 ? i_ms_overheating_length_1 / 2 : 2000);
        }
      break;

      case 2:
        ms_overheating_length.start(i_ms_overheating_length_2 >= 4000 ? i_ms_overheating_length_2 / 2 : 2000);

        if(b_overheat_sync_to_fan != true && STREAM_MODE != SLIME) {
          ms_smoke_on.stop();
          ms_smoke_on.start(i_ms_overheating_length_1 >= 4000 ? i_ms_overheating_length_1 / 2 : 2000);
        }
      break;

      case 3:
        ms_overheating_length.start(i_ms_overheating_length_3 >= 4000 ? i_ms_overheating_length_3 / 2 : 2000);

        if(b_overheat_sync_to_fan != true && STREAM_MODE != SLIME) {
          ms_smoke_on.stop();
          ms_smoke_on.start(i_ms_overheating_length_1 >= 4000 ? i_ms_overheating_length_1 / 2 : 2000);
        }
      break;

      case 4:
        ms_overheating_length.start(i_ms_overheating_length_4 >= 4000 ? i_ms_overheating_length_4 / 2 : 2000);

        if(b_overheat_sync_to_fan != true && STREAM_MODE != SLIME) {
          ms_smoke_on.stop();
          ms_smoke_on.start(i_ms_overheating_length_1 >= 4000 ? i_ms_overheating_length_1 / 2 : 2000);
        }
      break;

      case 5:
        ms_overheating_length.start(i_ms_overheating_length_5 >= 4000 ? i_ms_overheating_length_5 / 2 : 2000);

        if(b_overheat_sync_to_fan != true && STREAM_MODE != SLIME) {
          ms_smoke_on.stop();
          ms_smoke_on.start(i_ms_overheating_length_1 >= 4000 ? i_ms_overheating_length_1 / 2 : 2000);
        }
      break;
    }
  }

  if(ms_smoke_on.remaining() < 1) {
    smokeNFilter(false);
  }

  if(ms_overheating_length.isRunning() && STREAM_MODE != SLIME) {
    if(b_overheat_sync_to_fan == true) {
      smokeNFilter(true);
    }

    // Turn the fans on.
    fanNFilter(true);
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
    // Tell the Neutrona Wand the venting is finished.
    packVentingFinished();
  }
}

void cyclotronOverheating() {
  if(b_overheat_sync_to_fan != true && STREAM_MODE != SLIME) {
    smokeNFilter(true);
  }

  if(ms_overheating.justFinished()) {
    if(STREAM_MODE == SLIME) {
      // Play the sound of slime refilling the tank.
      playEffect(S_SLIME_REFILL, true);
    }
    else {
      playEffect(S_AIR_RELEASE);

      uint8_t i_smoke_random = random(5);

      switch(i_smoke_random) {
        case 4:
          playEffect(S_VENT_SMOKE_4, false, i_volume_effects, true, 120);
        break;

        case 3:
          playEffect(S_VENT_SMOKE_3, false, i_volume_effects, true, 120);
        break;

        case 2:
          playEffect(S_VENT_SMOKE_2, false, i_volume_effects, true, 120);
        break;

        case 1:
          playEffect(S_VENT_SMOKE_1, false, i_volume_effects, true, 120);
        break;

        case 0:
        default:
          playEffect(S_VENT_SMOKE, false, i_volume_effects, true, 120);
        break;
      }

      // Fade in the steam release loop.
      playEffect(S_STEAM_LOOP, true, i_volume_effects, true, 1000);
    }

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
      smokeNFilter(false);
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
          vibrationOff();
        }
      }

      innerCyclotronRingUpdate(i_2021_inner_delay * 16);
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      innerCyclotronRingUpdate(i_2021_inner_delay * 16);

      if(ms_alarm.justFinished()) {
        ms_alarm.start(i_1984_delay / 2);
        if(b_fade_cyclotron_led != true && STREAM_MODE != SLIME) {
          resetCyclotronState();
        }
        else {
          for(uint8_t i = 0; i < 4; i++) {
            cyclotron84LightOff(cyclotron84LookupTable(i) + i_cyclotron_led_start);
          }
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
            vibrationOff();
          }
        }
      }
    break;
  }

  if(ms_overheating_length.isRunning() && STREAM_MODE != SLIME) {
    if(b_overheat_sync_to_fan == true) {
      smokeNFilter(true);
    }

    // Turn the fans on.
    fanNFilter(true);
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
    packOverheatingFinished();
  }
}

void packOverheatingFinished() {
  if(b_wand_syncing != true) {
    packSerialSend(P_OVERHEATING_FINISHED);
  }

  serial1Send(A_OVERHEATING_FINISHED);

  ms_overheating_length.stop();

  stopEffect(S_STEAM_LOOP);
  stopEffect(S_SLIME_REFILL);

  if(STREAM_MODE != SLIME) {
    switch(SYSTEM_YEAR) {
      case SYSTEM_AFTERLIFE:
      case SYSTEM_FROZEN_EMPIRE:
        playEffect(S_VENT_CLOSE_ALT);
      break;

      case SYSTEM_1984:
      case SYSTEM_1989:
      default:
        playEffect(S_VENT_CLOSE);
      break;
    }

    if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
      stopEffect(S_PACK_OVERHEAT_HOT);
    }

    playEffect(S_STEAM_LOOP_FADE_OUT);
  }

  b_overheating = false;

  // Turn off the smoke.
  smokeNFilter(false);

  // Stop the fans.
  fanNFilter(false);
  fanBooster(false);

  // Reset the LEDs before resetting the alarm flag.
  if((SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) && !usingSlimeCyclotron()) {
    resetCyclotronState();
  }

  b_alarm = false;

  if(b_overheat_lights_off == true) {
    cyclotronSpeedRevert();

    // Reset the cyclotron ramp speeds.
    resetRampSpeeds();
  }

  reset2021RampUp();

  packStartup(false);

  // Turn off the vent lights
  ventLight(false);
  ventLightLEDW(false);
  ms_vent_light_off.stop();
  ms_vent_light_on.stop();

  if(!usingSlimeCyclotron()) {
    ms_cyclotron.start(i_2021_delay);
  }
}

void packVentingFinished() {
  packSerialSend(P_VENTING_FINISHED);
  serial1Send(A_VENTING_FINISHED);

  ms_overheating_length.stop();
  ms_smoke_on.stop();

  stopEffect(S_STEAM_LOOP);
  stopEffect(S_SLIME_REFILL);
  stopEffect(S_QUICK_VENT_OPEN);
  playEffect(S_QUICK_VENT_CLOSE);

  if(STREAM_MODE == SLIME) {
    playEffect(S_PACK_SLIME_TANK_LOOP, true, i_volume_effects, true, 1500);
  }
  else {
    playEffect(S_STEAM_LOOP_FADE_OUT);
  }

  b_venting = false;

  // Turn off the smoke.
  smokeNFilter(false);

  // Stop the fans.
  fanNFilter(false);
  fanBooster(false);

  // Turn off the vent lights
  ventLight(false);
  ventLightLEDW(false);
  ms_vent_light_off.stop();
  ms_vent_light_on.stop();
}

void cyclotronNoCable() {
  switch (SYSTEM_YEAR) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      if(!usingSlimeCyclotron()) {
        cyclotron2021(i_2021_delay * 10);
      }

      innerCyclotronRingUpdate(i_2021_inner_delay * 16);

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
      if(!usingSlimeCyclotron()) {
        cyclotron1984(i_1984_delay * 3);
      }

      innerCyclotronRingUpdate(i_2021_inner_delay * 16);

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
    uint8_t i_cyclotron_leds_total = i_pack_num_leds - i_nfilter_jewel_leds - i_cyclotron_led_start;

    if(b_cyclotron_simulate_ring == true) {
      i_cyclotron_leds_total = OUTER_CYCLOTRON_LED_MAX;
    }

    for(uint8_t i = 0; i < i_cyclotron_leds_total; i++) {
      if(b_cyclotron_simulate_ring == true) {
        pack_leds[cyclotronLookupTable(i) + i_cyclotron_led_start - 1] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      }
      else {
        pack_leds[i + i_cyclotron_led_start] = getHueAsRGB(CYCLOTRON_OUTER, C_BLACK);
      }
    }

    for(uint8_t i = 0; i < i_cyclotron_leds_total; i++) {
      ms_cyclotron_led_fade_out[i].go(0);
      ms_cyclotron_led_fade_in[i].go(0);

      b_cyclotron_led_on_status[i] = false;
    }
  }
}

void resetCyclotronState() {
  // Turn off optional N-Filter LED.
  digitalWriteFast(i_nfilter_led_pin, LOW);

  // Stop the slime Cyclotron effect timer.
  ms_cyclotron_slime_effect.stop();

  cyclotronLidLedsOff();

  // Only reset the start LED if the pack is off or just started.
  if(b_reset_start_led == true) {
    i_led_cyclotron = i_cyclotron_led_start;
  }

  // Keep the fade control fading out a light that is not on during startup.
  if(PACK_STATE == MODE_OFF) {
    if(b_1984_led_start != true) {
      b_1984_led_start = true;
    }
  }

  // Tell the Inner Cyclotron to turn off the LEDs.
  if(b_cyclotron_lid_on == true) {
    innerCyclotronCakeOff();
    innerCyclotronCavityOff();
  }
  else if(b_alarm != true || PACK_STATE == MODE_OFF) {
    innerCyclotronCakeOff();
    innerCyclotronCavityOff();
  }

  cyclotronSpeedRevert();
}

void clearCyclotronFades() {
  //if(b_fade_out != true) {
    for(uint8_t i = 0; i < OUTER_CYCLOTRON_LED_MAX; i++) {
      i_cyclotron_led_value[i] = 0;
      ms_cyclotron_led_fade_out[i].go(0);
      ms_cyclotron_led_fade_in[i].go(0);
    }
  //}
}

void innerCyclotronLEDPanelOff() {
  if(b_inner_cyclotron_led_panel == true) {
    if(b_cyclotron_lid_on == true) {
      // All lights turn off while the cyclotron lid is on.
      for(uint8_t i = i_ic_panel_start; i <= i_ic_panel_end; i++) {
        cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
      }
    }
    else {
      // Otherwise the 2 switch panel lights remain on when lid is removed.
      for(uint8_t i = i_ic_panel_start; i <= i_ic_panel_end - 2; i++) {
        cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
      }
    }
  }
}

void innerCyclotronCakeOff() {
  for(uint8_t i = i_ic_cake_start; i <= i_ic_cake_end; i++) {
    cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
  }
}

void innerCyclotronCavityOff() {
  if(i_inner_cyclotron_cavity_num_leds > 0) {
    for(uint8_t i = i_ic_cavity_start; i <= i_ic_cavity_end; i++) {
      cyclotron_leds[i] = getHueAsRGB(CYCLOTRON_CAVITY, C_BLACK);
    }
  }
}

void innerCyclotronCavityUpdate(uint16_t iRampDelay) {
  // Map the value from the inner cake to the cavity lights to get current position.
  uint8_t i_midpoint = i_ic_cavity_start + (i_inner_cyclotron_cavity_num_leds / 2) - 1;
  uint8_t i_colour_scheme = C_BLACK; // Colour scheme for lighting, to be set later.
  uint8_t i_brightness = getBrightness(i_cyclotron_inner_brightness);

  // Cannot go lower than the starting point for this segment of LEDs.
  if(i_led_cyclotron_cavity < i_ic_cavity_start) {
    i_led_cyclotron_cavity = i_ic_cavity_start;
  }

  if(SYSTEM_YEAR != SYSTEM_FROZEN_EMPIRE || STREAM_MODE != PROTON) {
    // This produces the "sparking" effect as seen in GB:FE only for the Proton stream,
    // so the effect is essentially disabled for all other themes and firing modes.
    i_colour_scheme = C_BLACK;
  }
  else {
    if(i_led_cyclotron_cavity < i_midpoint) {
      i_colour_scheme = C_YELLOW; // Always keep the lower half of LEDs yellow.
    }
    else {
      // Light spiraling higher than the lower half will have variable colours.
      i_colour_scheme = getDeviceColour(CYCLOTRON_CAVITY, STREAM_MODE, false);
    }
  }

  if(b_clockwise == true) {
    if(iRampDelay < 40 && b_cyclotron_lid_on != true) {
      if(b_gbr_cyclotron_cavity == true) {
        cyclotron_leds[i_led_cyclotron_cavity] = getHueAsGBR(CYCLOTRON_CAVITY, i_colour_scheme, i_brightness);
      }
      else {
        cyclotron_leds[i_led_cyclotron_cavity] = getHueAsRGB(CYCLOTRON_CAVITY, i_colour_scheme, i_brightness);
      }

      if(i_led_cyclotron_cavity == i_ic_cavity_start) {
        cyclotron_leds[i_ic_cavity_end] = getHueAsRGB(CYCLOTRON_CAVITY, C_BLACK);
      }
      else {
        cyclotron_leds[i_led_cyclotron_cavity - 1] = getHueAsRGB(CYCLOTRON_CAVITY, C_BLACK);
      }
    }

    i_led_cyclotron_cavity++;

    if(i_led_cyclotron_cavity > i_ic_cavity_end) {
      i_led_cyclotron_cavity = i_ic_cavity_start;
    }
  }
  else {
    if(iRampDelay < 40 && b_cyclotron_lid_on != true) {
      if(b_gbr_cyclotron_cavity == true) {
        cyclotron_leds[i_led_cyclotron_cavity] = getHueAsGBR(CYCLOTRON_CAVITY, i_colour_scheme, i_brightness);
      }
      else {
        cyclotron_leds[i_led_cyclotron_cavity] = getHueAsRGB(CYCLOTRON_CAVITY, i_colour_scheme, i_brightness);
      }

      if(i_led_cyclotron_cavity + 1 > i_ic_cavity_end) {
        cyclotron_leds[i_ic_cavity_start] = getHueAsRGB(CYCLOTRON_CAVITY, C_BLACK);
      }
      else {
        cyclotron_leds[i_led_cyclotron_cavity + 1] = getHueAsRGB(CYCLOTRON_CAVITY, C_BLACK);
      }
    }

    i_led_cyclotron_cavity--;

    if(i_led_cyclotron_cavity < i_ic_cavity_start) {
      i_led_cyclotron_cavity = i_ic_cavity_end;
    }
  }
}

// For NeoPixel rings, ramp up and ramp down the LEDs in the ring and set the speed. (optional)
void innerCyclotronRingUpdate(uint16_t iRampDelay) {
  if(ms_cyclotron_ring.justFinished()) {
    if(b_inner_ramp_up == true) {
      if(r_inner_ramp.isFinished()) {
        b_inner_ramp_up = false;
        ms_cyclotron_ring.start(iRampDelay);

        i_inner_current_ramp_speed = iRampDelay;
      }
      else {
        ms_cyclotron_ring.start(r_inner_ramp.update());
        i_inner_current_ramp_speed = r_inner_ramp.update();
      }
    }
    else if(b_inner_ramp_down == true) {
      innerCyclotronCavityOff(); // Turn off (sparking) cavity lights.

      if(r_inner_ramp.isFinished()) {
        b_inner_ramp_down = false;
      }
      else {
        ms_cyclotron_ring.start(r_inner_ramp.update());

        i_inner_current_ramp_speed = r_inner_ramp.update();
      }
    }
    else {
      i_inner_current_ramp_speed = iRampDelay;

      if(i_cyclotron_multiplier > 1) {
        if(i_cyclotron_multiplier > 4) {
          iRampDelay = iRampDelay - 4;
        }
        else {
          iRampDelay = iRampDelay - i_cyclotron_multiplier;
        }
      }

      if(iRampDelay < 1 || iRampDelay > i_inner_current_ramp_speed) {
        iRampDelay = 1;
      }

      ms_cyclotron_ring.start(iRampDelay);
    }

    switch(i_cyclotron_multiplier) {
      case 9:
      case 8:
      case 7:
      case 6:
        // A value of 6 should be the max, but just in case this value goes higher let's catch those possible cases.
        if(iRampDelay - 4 < iRampDelay) {
          iRampDelay = iRampDelay - 4;
        }
        else {
          iRampDelay = 2;
        }
      break;

      case 5:
      case 4:
        if(iRampDelay - 3 < iRampDelay) {
          iRampDelay = iRampDelay - 3;
        }
        else {
          iRampDelay = 2;
        }
      break;

      case 3:
      case 2:
        if(iRampDelay - 2 < iRampDelay) {
          iRampDelay = iRampDelay - 2;
        }
        else {
          iRampDelay = 2;
        }
      break;

      case 1:
      default:
        // A value of 1 is considered the "normal" speed so treat it as the default.
        if(iRampDelay - 1 < iRampDelay) {
          iRampDelay = iRampDelay - 1;
        }
        else {
          iRampDelay = 2;
        }
      break;

      case 0:
        // We should never have this value, but just in case make sure there's a known delay calculated.
        iRampDelay = iRampDelay / i_cyclotron_multiplier;
        break;
    }

    if(iRampDelay < 2) {
      iRampDelay = 2;
    }

    // Colour control for the Inner Cyclotron LEDs.
    uint8_t i_brightness = getBrightness(i_cyclotron_inner_brightness);
    uint8_t i_colour_scheme = getDeviceColour(CYCLOTRON_INNER, STREAM_MODE, b_cyclotron_colour_toggle);

    if(SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE && STREAM_MODE == PROTON) {
      // As a "sparking" effect is predominant in GB:FE during the Proton stream,
      // the inner LED colour/brightness is altered for this mode.
      i_brightness = getBrightness(i_cyclotron_inner_brightness / 2);
      i_colour_scheme = C_ORANGE;
    }

    if(b_clockwise == true) {
      if(b_cyclotron_lid_on != true) {
        if(b_grb_cyclotron_cake == true) {
          cyclotron_leds[i_led_cyclotron_ring] = getHueAsGRB(CYCLOTRON_INNER, i_colour_scheme, i_brightness);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring] = getHueAsRGB(CYCLOTRON_INNER, i_colour_scheme, i_brightness);
        }

        if(i_led_cyclotron_ring == i_ic_cake_start) {
          cyclotron_leds[i_ic_cake_end] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring - 1] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
        }
      }

      i_led_cyclotron_ring++;

      if(i_led_cyclotron_ring > i_ic_cake_end) {
        i_led_cyclotron_ring = i_ic_cake_start;
      }
    }
    else {
      if(b_cyclotron_lid_on != true) {
        if(b_grb_cyclotron_cake == true) {
          cyclotron_leds[i_led_cyclotron_ring] = getHueAsGRB(CYCLOTRON_INNER, i_colour_scheme, i_brightness);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring] = getHueAsRGB(CYCLOTRON_INNER, i_colour_scheme, i_brightness);
        }

        if(i_led_cyclotron_ring + 1 > i_ic_cake_end) {
          cyclotron_leds[i_ic_cake_start] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
        }
        else {
          cyclotron_leds[i_led_cyclotron_ring + 1] = getHueAsRGB(CYCLOTRON_INNER, C_BLACK);
        }
      }

      i_led_cyclotron_ring--;

      if(i_led_cyclotron_ring < i_ic_cake_start) {
        i_led_cyclotron_ring = i_ic_cake_end;
      }
    }

    // Update the sparking effect only half as often as the cake is updated.
    if(i_inner_cyclotron_cavity_num_leds > 0 && (i_led_cyclotron_ring % 2) == 0) {
      // Update the inner cyclotron cavity LEDs for Frozen Empire w/ a Proton stream.
      // The delay value is just used to determine when to begin the sparking effect.
      innerCyclotronCavityUpdate(iRampDelay);
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
    digitalWriteFast(i_nfilter_led_pin, HIGH);
  }
  else {
    digitalWriteFast(i_nfilter_led_pin, LOW);
  }
}

void ventLight(bool b_on) {
  uint8_t i_colour_scheme = getDeviceColour(VENT_LIGHT, STREAM_MODE, true);
  b_vent_light_on = b_on;

  if(b_on == true) {
    // If doing firing smoke effects, let's change the light colours.
    if(b_wand_firing == true || b_overheating == true) {
      if(STREAM_MODE == PROTON) {
        // Override the N-Filter light colours for a proton stream.
        switch(i_wand_power_level) {
          case 1:
            i_colour_scheme = C_RED;
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

    for(uint8_t i = i_vent_light_start; i < i_pack_num_leds; i++) {
      pack_leds[i] = getHueAsRGB(VENT_LIGHT, i_colour_scheme); // Uses full brightness.
    }
  }
  else {
    for(uint8_t i = i_vent_light_start; i < i_pack_num_leds; i++) {
      pack_leds[i] = getHueAsRGB(VENT_LIGHT, C_BLACK);
    }
  }
}

// Only for Afterlife and Frozen Empire mode.
void checkCyclotronAutoSpeed() {
  // No need to start any timers until after any ramping has finished; only in Afterlife and Frozen Empire do we do the auto speed increases.
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
  switch(STREAM_MODE) {
    case PROTON:
    default:
      switch(i_wand_power_level) {
        case 1 ... 4:
        default:
          if(b_firing_intensify == true) {
            switch(SYSTEM_YEAR) {
              case SYSTEM_1984:
                playEffect(S_GB1_1984_FIRE_START_SHORT, false, i_volume_effects, false, 0, false);
                playEffect(S_GB1_1984_FIRE_LOOP_PACK, true, i_volume_effects, true, 1350, false);
              break;

              case SYSTEM_1989:
                playEffect(S_GB2_FIRE_START, false, i_volume_effects, false, 0, false);
                playEffect(S_GB2_FIRE_LOOP, true, i_volume_effects, true, 6500, false);
              break;

              case SYSTEM_AFTERLIFE:
              default:
                playEffect(S_GB1_FIRE_START, false, i_volume_effects, false, 0, false);
                playEffect(S_GB1_1984_FIRE_LOOP_PACK, true, i_volume_effects, true, 1000, false);
              break;

              case SYSTEM_FROZEN_EMPIRE:
                playEffect(S_FROZEN_EMPIRE_FIRE_START, false, i_volume_effects, false, 0, false);
                playEffect(S_GB1_1984_FIRE_LOOP_PACK, true, i_volume_effects, true, 1000, false);
              break;
            }

            b_sound_firing_intensify_trigger = true;
          }
          else {
            b_sound_firing_intensify_trigger = false;
          }

          if(b_firing_alt == true) {
            if(SYSTEM_YEAR == SYSTEM_1989) {
              playEffect(S_GB2_FIRE_START, false, i_volume_effects, false, 0, false);
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 6500, false);
            }
            else if(SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
              playEffect(S_FROZEN_EMPIRE_FIRE_START, false, i_volume_effects, false, 0, false);
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 800, false);
            }
            else {
              playEffect(S_FIRE_START, false, i_volume_effects, false, 0, false);
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 1000, false);
            }

            b_sound_firing_alt_trigger = true;
          }
          else {
            b_sound_firing_alt_trigger = false;
          }
        break;

        case 5:
          switch(SYSTEM_YEAR) {
            case SYSTEM_1984:
              playEffect(S_GB1_1984_FIRE_START_HIGH_POWER, false, i_volume_effects, false, 0, false);
            break;

            case SYSTEM_1989:
              playEffect(S_GB1_FIRE_START_HIGH_POWER, false, i_volume_effects, false, 0, false);
            break;

            case SYSTEM_AFTERLIFE:
            default:
              playEffect(S_AFTERLIFE_FIRE_START, false, i_volume_effects, false, 0, false);
            break;

            case SYSTEM_FROZEN_EMPIRE:
              playEffect(S_FROZEN_EMPIRE_FIRE_START, false, i_volume_effects, false, 0, false);
            break;
          }

          if(b_firing_intensify == true) {
            // Reset some sound triggers.
            b_sound_firing_intensify_trigger = true;
            if(SYSTEM_YEAR == SYSTEM_1984) {
              playEffect(S_GB1_1984_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, true, 1700, false);
            }
            else if(SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
              playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, true, 800, false);
            }
            else {
              playEffect(S_GB1_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, true, 1500, false);
            }
          }
          else {
            b_sound_firing_intensify_trigger = false;
          }

          if(b_firing_alt == true) {
            // Reset some sound triggers.
            b_sound_firing_alt_trigger = true;
            if(SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 800, false);
            }
            else if(SYSTEM_YEAR == SYSTEM_1984) {
              playEffect(S_GB1_1984_FIRE_HIGH_POWER_LOOP, true, i_volume_effects, true, 1700, false);
            }
            else {
              playEffect(S_FIRING_LOOP_GB1, true, i_volume_effects, true, 700, false);
            }
          }
          else {
            b_sound_firing_alt_trigger = false;
          }
        break;
      }
    break;

    case SLIME:
      stopEffect(S_SLIME_END);
      playEffect(S_SLIME_START, false, i_volume_effects, false, 0, false);
      playEffect(S_SLIME_LOOP, true, i_volume_effects, true, 1500, false);
    break;

    case STASIS:
      stopEffect(S_STASIS_END);
      playEffect(S_STASIS_START, false, i_volume_effects, false, 0, false);
      playEffect(S_STASIS_LOOP, true, i_volume_effects, true, 1000, false);
    break;

    case MESON:
      playEffect(S_MESON_FIRE_PULSE, false, i_volume_effects, false, 0, false);
    break;
  }

  // Adjust the gain with the Afterlife idling sound effect while firing.
  if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) && i_wand_power_level < 5) {
    if(ms_idle_fire_fade.remaining() < 3000) {
      if(STREAM_MODE == SLIME) {
        adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - 40, true, 100);
      }
      else {
        adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - 2, true, 100);
      }
    }
    else {
      if(STREAM_MODE == SLIME) {
        adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - 40, true, ms_idle_fire_fade.remaining());
      }
      else {
        adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - 2, true, ms_idle_fire_fade.remaining());
      }
    }
  }
}

void wandFiring() {
  if(b_powercell_sound_loop == true) {
    b_powercell_sound_loop = false;
    stopEffect(S_POWERCELL); // Turn off the powercell sound effect.
  }

  modeFireStartSounds();

  b_wand_firing = true;
  serial1Send(A_FIRING);

  // Reset the Cyclotron auto speed up timers. Only for Afterlife (2021) mode.
  ms_cyclotron_auto_speed_timer.stop();

  if(SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) {
    ms_cyclotron_auto_speed_timer.start(i_cyclotron_auto_speed_timer_length / i_wand_power_level);
  }

  if(b_stream_effects == true && STATUS_CTS == CTS_NOT_FIRING) {
    uint16_t i_s_random = random(7,15) * 1000;
    ms_firing_sound_mix.start(i_s_random);
  }

  // Turn off any smoke.
  smokeNFilter(false);

  // Start a smoke timer to play a little bit of smoke while firing.
  if(STREAM_MODE != SLIME) {
    ms_smoke_timer.start(PROGMEM_READU32(i_smoke_timer[i_wand_power_level - 1]));
    ms_smoke_on.stop();
  }

  // Just in case a semi-auto was fired before we started firing a stream, stop its vibration timer.
  ms_menu_vibration.stop();

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
  wandStopFiringSounds();

  if(b_wand_firing == true) {
    if(b_wand_mash_lockout != true) {
      switch(STREAM_MODE) {
        case PROTON:
        default:
          switch(SYSTEM_YEAR) {
            case SYSTEM_1984:
              if(i_wand_power_level != i_wand_power_level_max) {
                // Play different firing end stream sound depending on how long we have been firing for.
                if(ms_firing_length_timer.remaining() < 5000) {
                  // Long firing tail end.
                  playEffect(S_FIRING_END_MID, false, i_volume_effects, false, 0, false);
                }
                else if(ms_firing_length_timer.remaining() < 10000) {
                  // Mid firing tail end.
                  playEffect(S_FIRING_END, false, i_volume_effects, false, 0, false);
                }
                else {
                  // Short firing tail end.
                  playEffect(S_GB1_1984_FIRE_END_SHORT, false, i_volume_effects, false, 0, false);
                }
              }
              else {
                // Play different firing end stream sound depending on how long we have been firing for.
                if(ms_firing_length_timer.remaining() < 5000) {
                  // Long tail end.
                  playEffect(S_GB1_1984_FIRE_END_HIGH_POWER, false, i_volume_effects, false, 0, false);
                }
                else if(ms_firing_length_timer.remaining() < 10000) {
                  // Mid tail end.
                  playEffect(S_GB1_1984_FIRE_END_MID_HIGH_POWER, false, i_volume_effects, false, 0, false);
                }
                else {
                  // Short tail end.
                  playEffect(S_GB1_1984_FIRE_END_SHORT_HIGH_POWER, false, i_volume_effects, false, 0, false);
                }
              }
            break;

            case SYSTEM_1989:
              // Play different firing end stream sound depending on how long we have been firing for.
              if(ms_firing_length_timer.remaining() < 5000) {
                // Long tail end.
                playEffect(S_FIRING_END_GUN, false, i_volume_effects, false, 0, false);
              }
              else if(ms_firing_length_timer.remaining() < 10000) {
                // Mid tail end.
                playEffect(S_FIRING_END_MID, false, i_volume_effects, false, 0, false);
              }
              else {
                // Short tail end.
                playEffect(S_FIRING_END, false, i_volume_effects, false, 0, false);
              }
            break;

            case SYSTEM_AFTERLIFE:
            default:
              // Play different firing end stream sound depending on how long we have been firing for.
              if(ms_firing_length_timer.remaining() < 5000) {
                // Long firing tail end.
                playEffect(S_AFTERLIFE_FIRE_END_LONG, false, i_volume_effects, false, 0, false);
              }
              else if(ms_firing_length_timer.remaining() < 10000) {
                // Mid firing tail end.
                playEffect(S_AFTERLIFE_FIRE_END_MID, false, i_volume_effects, false, 0, false);
              }
              else {
                // Short firing tail end.
                playEffect(S_AFTERLIFE_FIRE_END_SHORT, false, i_volume_effects, false, 0, false);
              }
            break;

            case SYSTEM_FROZEN_EMPIRE:
              // Frozen Empire replaces all firing tail sounds with just a "thump".
              playEffect(S_FROZEN_EMPIRE_FIRE_END, false, i_volume_effects, false, 0, false);
            break;
          }
        break;

        case SLIME:
          playEffect(S_SLIME_END, false, i_volume_effects, false, 0, false);
        break;

        case STASIS:
          playEffect(S_STASIS_END, false, i_volume_effects, false, 0, false);
        break;

        case MESON:
          // Nothing.
        break;
      }
    }

    // Adjust the gain with the Afterlife idling track.
    if((SYSTEM_YEAR == SYSTEM_AFTERLIFE || SYSTEM_YEAR == SYSTEM_FROZEN_EMPIRE) && i_wand_power_level < 5) {
      if(ms_idle_fire_fade.remaining() < 1000) {
        if(STREAM_MODE == SLIME) {
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - 40, true, 30);
        }
        else {
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects, true, 30);
        }
      }
      else {
        if(STREAM_MODE == SLIME) {
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects - 40, true, ms_idle_fire_fade.remaining());
        }
        else {
          adjustGainEffect(S_AFTERLIFE_PACK_IDLE_LOOP, i_volume_effects, true, ms_idle_fire_fade.remaining());
        }
      }
    }
  }

  b_wand_mash_lockout = false;
}

void wandStoppedFiring() {
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
  smokeNFilter(false);

  // Turn off the fans.
  fanNFilter(false);
  fanBooster(false);

  ms_firing_length_timer.stop();
  ms_smoke_timer.stop();
  ms_smoke_on.stop();

  // Stop overheat beeps.
  switch(SYSTEM_YEAR) {
    case SYSTEM_AFTERLIFE:
    case SYSTEM_FROZEN_EMPIRE:
    default:
      stopEffect(S_PACK_BEEPS_OVERHEAT);
    break;

    case SYSTEM_1984:
    case SYSTEM_1989:
      stopEffect(S_BEEP_8);
    break;
  }
}

void wandStopFiringSounds() {
  // Stop all firing sounds.
  switch(STREAM_MODE) {
    case PROTON:
    default:
      switch(i_wand_power_level) {
        case 1 ... 4:
        default:
          switch(SYSTEM_YEAR) {
            case SYSTEM_1984:
              stopEffect(S_GB1_1984_FIRE_START_SHORT);
              stopEffect(S_GB1_1984_FIRE_LOOP_PACK);
            break;
            case SYSTEM_1989:
              stopEffect(S_GB2_FIRE_START);
              stopEffect(S_GB2_FIRE_LOOP);
            break;
            case SYSTEM_AFTERLIFE:
            default:
              stopEffect(S_GB1_FIRE_START);
              stopEffect(S_GB1_1984_FIRE_LOOP_PACK);
            break;
            case SYSTEM_FROZEN_EMPIRE:
              stopEffect(S_FROZEN_EMPIRE_FIRE_START);
              stopEffect(S_GB1_1984_FIRE_LOOP_PACK);
            break;
          }
        break;
        case 5:
          switch(SYSTEM_YEAR) {
            case SYSTEM_1984:
              stopEffect(S_GB1_1984_FIRE_START_HIGH_POWER);
              stopEffect(S_GB1_1984_FIRE_HIGH_POWER_LOOP);
            break;
            case SYSTEM_1989:
              stopEffect(S_GB1_FIRE_START_HIGH_POWER);
            break;
            case SYSTEM_AFTERLIFE:
            default:
              stopEffect(S_AFTERLIFE_FIRE_START);
            break;
            case SYSTEM_FROZEN_EMPIRE:
              stopEffect(S_FROZEN_EMPIRE_FIRE_START);
            break;
          }
        break;
      }

      if(b_stream_effects) {
        stopEffect(S_FIRE_LOOP_IMPACT);
      }

      stopEffect(S_FIRING_LOOP_GB1);
      stopEffect(S_GB1_FIRE_HIGH_POWER_LOOP);
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
  }

  switch(STATUS_CTS) {
    case CTS_FIRING_1984:
      STATUS_CTS = CTS_NOT_FIRING;

      if(AUDIO_DEVICE != A_GPSTAR_AUDIO) {
        stopEffect(S_CROSS_STREAMS_START);
        stopEffect(S_CROSS_STREAMS_END);
      }

      if(b_wand_mash_lockout != true) {
        playEffect(S_CROSS_STREAMS_END, false, i_volume_effects, false, 0, false);
      }
    break;

    case CTS_FIRING_2021:
      STATUS_CTS = CTS_NOT_FIRING;

      if(AUDIO_DEVICE != A_GPSTAR_AUDIO) {
        stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_START);
        stopEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END);
      }

      if(b_wand_mash_lockout != true) {
        playEffect(S_AFTERLIFE_CROSS_THE_STREAMS_END, false, i_volume_effects, false, 0, false);
      }
    break;

    case CTS_NOT_FIRING:
      // Do nothing.
    break;
  }

  b_sound_firing_intensify_trigger = false;
  b_sound_firing_alt_trigger = false;
}

void packAlarm() {
  if(b_wand_firing == true) {
    // Preemptively stop firing sounds.
    wandStopFiringSounds();
    cyclotronSpeedRevert();
  }

  // Stop Pack sounds.
  if(SYSTEM_YEAR == SYSTEM_1989) {
    stopEffect(S_GB2_PACK_START);
    stopEffect(S_GB2_PACK_LOOP);
  }
  else if(SYSTEM_YEAR == SYSTEM_1984) {
    stopEffect(S_GB1_1984_PACK_LOOP);
    stopEffect(S_GB1_1984_BOOT_UP);
  }
  else {
    stopEffect(S_AFTERLIFE_PACK_STARTUP);
    stopEffect(S_AFTERLIFE_PACK_IDLE_LOOP);

    if(b_brass_pack_sound_loop) {
      stopEffect(S_FROZEN_EMPIRE_BOOT_EFFECT);
      b_brass_pack_sound_loop = false;
    }
  }

  playEffect(S_SHUTDOWN);

  if(SYSTEM_YEAR == SYSTEM_1989) {
    playEffect(S_GB2_PACK_OFF);
  }
  else {
    playEffect(S_PACK_SHUTDOWN);
  }

  switch(STREAM_MODE) {
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
      default:
        playEffect(S_PACK_RIBBON_ALARM_1, true);
        playEffect(S_ALARM_LOOP, true);
        playEffect(S_RIBBON_CABLE_START);
      break;
    }
  }

  // Turn off LEDs within the Cyclotron cavity if lid is not attached.
  if(b_cyclotron_lid_on != true) {
    innerCyclotronCavityOff();
  }
}

// LEDs for the 1984/2021 and vibration switches.
void cyclotronSwitchPlateLEDs() {
  if(switch_cyclotron_lid.isReleased()) {
    // Play sounds when lid is removed.
    stopEffect(S_VENT_SMOKE);
    stopEffect(S_VENT_SMOKE_1);
    stopEffect(S_VENT_SMOKE_2);
    stopEffect(S_VENT_SMOKE_3);
    stopEffect(S_VENT_SMOKE_4);
    stopEffect(S_MODE_SWITCH);
    stopEffect(S_CLICK);
    stopEffect(S_SPARKS_LOOP);
    stopEffect(S_BEEPS_BARGRAPH);

    playEffect(S_MODE_SWITCH);

    uint8_t i_smoke_random = random(5);

    switch(i_smoke_random) {
      case 4:
        playEffect(S_VENT_SMOKE_4, false, i_volume_effects, true, 120);
      break;

      case 3:
        playEffect(S_VENT_SMOKE_3, false, i_volume_effects, true, 120);
      break;

      case 2:
        playEffect(S_VENT_SMOKE_2, false, i_volume_effects, true, 120);
      break;

      case 1:
        playEffect(S_VENT_SMOKE_1, false, i_volume_effects, true, 120);
      break;

      case 0:
      default:
        playEffect(S_VENT_SMOKE, false, i_volume_effects, true, 120);
      break;
    }

    // Play some spark sounds if the pack is running and the lid is removed.
    if(PACK_STATE == MODE_ON) {
      playEffect(S_SPARKS_LOOP);
    }
    else {
      // Make sure we reset the cyclotron LED status if not in the EEPROM LED menu.
      if(b_spectral_lights_on != true) {
        b_reset_start_led = false;
      }
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
    else {
      // Make sure we reset the cyclotron LED status if not in the EEPROM LED menu.
      if(b_spectral_lights_on != true) {
        b_reset_start_led = false;
      }
    }
  }

  if(switch_cyclotron_lid.getState() == LOW) {
    if(b_cyclotron_lid_on != true) {
      // The Cyclotron Lid is now on.
      b_cyclotron_lid_on = true;

      // Tell the connected devices.
      packSerialSend(P_CYCLOTRON_LID_ON);
      serial1Send(A_CYCLOTRON_LID_ON);

      // Turn off Inner Cyclotron LEDs.
      innerCyclotronCakeOff();
      innerCyclotronCavityOff();
    }
  }
  else {
    if(b_cyclotron_lid_on == true) {
      // The Cyclotron Lid is now off.
      b_cyclotron_lid_on = false;

      // Tell the connected devices.
      packSerialSend(P_CYCLOTRON_LID_OFF);
      serial1Send(A_CYCLOTRON_LID_OFF);

      // Make sure the Inner Cyclotron turns on if we are in the EEPROM LED menu.
      if(b_spectral_lights_on == true) {
        spectralLightsOn();
      }
    }
  }

  if(b_cyclotron_lid_on != true) {
    uint8_t i_brightness = getBrightness(i_cyclotron_panel_brightness);

    if(SYSTEM_YEAR == SYSTEM_1984 || SYSTEM_YEAR == SYSTEM_1989) {
      if(ms_cyclotron_switch_plate_leds.remaining() < i_cyclotron_switch_plate_leds_delay / 2) {
        digitalWriteFast(cyclotron_switch_led_green, HIGH);

        if(b_inner_cyclotron_led_panel == true) {
          cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_RED, i_brightness);
        }
      }
      else {
        digitalWriteFast(cyclotron_switch_led_green, LOW);

        if(b_inner_cyclotron_led_panel == true) {
          cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        }
      }
    }
    else {
      digitalWriteFast(cyclotron_switch_led_green, HIGH);

      if(b_inner_cyclotron_led_panel == true) {
        cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_RED, i_brightness);
      }
    }

    if(b_vibration_switch_on == true) {
      if(ms_cyclotron_switch_plate_leds.remaining() < i_cyclotron_switch_plate_leds_delay / 2) {
        digitalWriteFast(cyclotron_switch_led_yellow, HIGH);

        if(b_inner_cyclotron_led_panel == true) {
          cyclotron_leds[i_ic_panel_end] = getHueAsRGB(CYCLOTRON_PANEL, C_YELLOW, i_brightness);
        }
      }
      else {
        digitalWriteFast(cyclotron_switch_led_yellow, LOW);

        if(b_inner_cyclotron_led_panel == true) {
          cyclotron_leds[i_ic_panel_end] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
        }
      }
    }
    else {
      digitalWriteFast(cyclotron_switch_led_yellow, HIGH);

      if(b_inner_cyclotron_led_panel == true) {
        cyclotron_leds[i_ic_panel_end] = getHueAsRGB(CYCLOTRON_PANEL, C_YELLOW, i_brightness);
      }
    }
  }
  else {
    // Keep the Cyclotron switch LEDs off when the lid is on.
    digitalWriteFast(cyclotron_switch_led_green, LOW);
    digitalWriteFast(cyclotron_switch_led_yellow, LOW);

    if(b_inner_cyclotron_led_panel == true) {
      cyclotron_leds[i_ic_panel_end - 1] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
      cyclotron_leds[i_ic_panel_end] = getHueAsRGB(CYCLOTRON_PANEL, C_BLACK);
    }
  }

  if(ms_cyclotron_switch_plate_leds.justFinished()) {
    ms_cyclotron_switch_plate_leds.repeat();
  }
}

void vibrationPack(uint8_t i_level) {
  if(b_vibration_enabled == true && b_vibration_switch_on == true && i_level > 0) {
    if(b_vibration_firing == true) {
      if(b_wand_firing == true) {
        if(i_level != i_vibration_level_prev) {
          i_vibration_level_prev = i_level;
          analogWrite(vibration, i_level);
        }
      }
      else {
        vibrationOff();
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
    vibrationOff();
  }
}

void checkMenuVibration() {
  if(ms_menu_vibration.justFinished()) {
    vibrationOff();
  }
  else if(ms_menu_vibration.isRunning()) {
    if(PACK_STATE == MODE_OFF) {
      // If we're off we must be in the EEPROM Config Menu; vibrate at 59%.
      analogWrite(vibration, 150);
    }
    else {
      // If we're on we must be firing a semi-auto blast; vibrate at 100%.
      analogWrite(vibration, 255);
    }
  }
}

void vibrationOff() {
  ms_menu_vibration.stop();
  i_vibration_level_prev = 0;
  analogWrite(vibration, 0);
}

void cyclotronSpeedRevert() {
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

int8_t readRotary() {
  static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};

  prev_next_code <<= 2;

  if(digitalReadFast(encoder_pin_b)) {
    prev_next_code |= 0x02;
  }

  if(digitalReadFast(encoder_pin_a)) {
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

void checkRotaryEncoder() {
 static int8_t c, val;

  if((val = readRotary())) {
    c += val;

    // Clockwise
    if(prev_next_code == 0x0b) {
      if(ms_volume_check.isRunning() != true) {
        increaseVolume();

        // Tell wand to increase volume.
        packSerialSend(P_VOLUME_INCREASE);

        ms_volume_check.start(50);
      }
    }

    // Counter Clockwise
    if(prev_next_code == 0x07) {
      if(ms_volume_check.isRunning() != true) {
        decreaseVolume();

        // Tell wand to decrease volume.
        packSerialSend(P_VOLUME_DECREASE);

        ms_volume_check.start(50);
      }
    }
  }

  if(ms_volume_check.justFinished()) {
    ms_volume_check.stop();
  }
}

// Smoke # 1. N-Filter cone outlet.
void smokeNFilter(bool b_smoke_on) {
  if(b_smoke_enabled == true) {
    if(b_smoke_on == true) {
      if(b_wand_firing == true && b_overheating != true && b_smoke_1_continuous_firing == true && b_smoke_continuous_level[i_wand_power_level - 1] == true) {
        digitalWriteFast(smoke_pin, HIGH);
      }
      else if(b_overheating == true && b_wand_firing != true && b_smoke_1_overheat == true && b_smoke_overheat_level[i_wand_power_level - 1] == true) {
        digitalWriteFast(smoke_pin, HIGH);
      }
      else if(b_venting == true) {
        digitalWriteFast(smoke_pin, HIGH);
      }
      else {
        digitalWriteFast(smoke_pin, LOW);
      }
    }
    else {
      digitalWriteFast(smoke_pin, LOW);
    }

    smokeBooster(b_smoke_on);
  }
}

// Smoke #2. Good for putting smoke in the Booster Tube.
void smokeBooster(bool b_smoke_on) {
  if(b_smoke_enabled == true) {
    if(b_smoke_on == true) {
      if(b_wand_firing == true && b_overheating != true && b_smoke_2_continuous_firing == true && b_smoke_continuous_level[i_wand_power_level - 1] == true) {
        digitalWriteFast(smoke_booster_pin, HIGH);
      }
      else if(b_overheating == true && b_smoke_2_overheat == true && b_wand_firing != true && b_smoke_overheat_level[i_wand_power_level - 1] == true) {
        digitalWriteFast(smoke_booster_pin, HIGH);
      }
      else if(b_venting == true) {
        digitalWriteFast(smoke_booster_pin, HIGH);
      }
      else {
        digitalWriteFast(smoke_booster_pin, LOW);
      }
    }
    else {
      digitalWriteFast(smoke_booster_pin, LOW);
    }
  }
}

// N-Filter Fan.
// Fan control. You can use this to switch on any device when properly hooked up with a transistor etc.
// A fan is a good idea for the N-Filter for example.
void fanNFilter(bool b_fan_on) {
  if(b_smoke_enabled == true) {
    if(b_fan_on == true) {
      if(b_wand_firing == true && b_overheating != true && b_fan_continuous_firing == true && b_smoke_continuous_level[i_wand_power_level - 1] == true) {
        digitalWriteFast(fan_pin, HIGH);
      }
      else if(b_overheating == true && b_wand_firing != true && b_fan_overheat == true && b_smoke_overheat_level[i_wand_power_level - 1] == true) {
        digitalWriteFast(fan_pin, HIGH);
      }
      else if(b_venting == true) {
        digitalWriteFast(fan_pin, HIGH);
      }
      else {
        digitalWriteFast(fan_pin, LOW);
      }
    }
    else {
      digitalWriteFast(fan_pin, LOW);
    }
  }
}

void fanBooster(bool b_fan_on) {
  if(b_smoke_enabled == true) {
    if(b_fan_on == true) {
      if(b_wand_firing == true && b_overheating != true && b_fan_booster_continuous_firing == true && b_smoke_continuous_level[i_wand_power_level - 1] == true) {
        digitalWriteFast(fan_booster_pin, HIGH);
      }
      else if(b_overheating == true && b_wand_firing != true && b_fan_booster_overheat == true && b_smoke_overheat_level[i_wand_power_level - 1] == true) {
        digitalWriteFast(fan_booster_pin, HIGH);
      }
      else if(b_venting == true) {
        digitalWriteFast(fan_booster_pin, HIGH);
      }
      else {
        digitalWriteFast(fan_booster_pin, LOW);
      }
    }
    else {
      digitalWriteFast(fan_booster_pin, LOW);
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
void wandDisconnectCheck() {
  // A wand was previously considered to be connected.
  if(b_wand_connected == true) {
    if(ms_wand_check.justFinished()) {
      // Timer just ran out, so we must assume the wand was disconnected.
      if(b_diagnostic == true) {
        // While in diagnostic mode, play a sound to indicate the wand is disconnected.
        playEffect(S_VENT_BEEP);
      }

      b_wand_connected = false; // Cause the next handshake to trigger a sync.
      b_wand_syncing = false; // If there is no wand we cannot be syncing with one.
      b_wand_on = false; // No wand means the device is no longer powered on.
      b_wand_mash_lockout = false;

      // Tell the serial1 device the wand was disconnected.
      serial1Send(A_WAND_DISCONNECTED);

      if(b_wand_firing == true) {
        // Reset the pack to a non-firing state.
        wandStoppedFiring();
        cyclotronSpeedRevert();
      }

      stopEffect(S_SMASH_ERROR_LOOP);
      stopEffect(S_SMASH_ERROR_RESTART);

      wandExtraSoundsStop();
      wandExtraSoundsBeepLoopStop();

      // Turn off overheating if the wand gets disconnected.
      if(b_overheating == true) {
        packOverheatingFinished();
      }

      if(b_spectral_lights_on == true) {
        spectralLightsOff();
      }
    }
    else {
      if(ms_wand_check.remaining() < 1500 && !b_wand_syncing) {
        // If we haven't received a handshake from the wand in over 6.5 seconds, force a handshake with the wand.
        // This is because the wand is supposed to handshake every 3.25 seconds and we haven't heard back in two pings.
        // This should be a last-resort check to make sure it's available and responding.
        b_wand_syncing = true;
        packSerialSend(P_HANDSHAKE);
      }
    }
  }
}

void wandExtraSoundsBeepLoop() {
  if(b_overheating != true) {
    switch(i_wand_power_level) {
      case 1:
        playEffect(S_AFTERLIFE_BEEP_WAND_S1, true, i_volume_effects - i_wand_beep_level);
      break;

      case 2:
        playEffect(S_AFTERLIFE_BEEP_WAND_S2, true, i_volume_effects - i_wand_beep_level);
      break;

      case 3:
        playEffect(S_AFTERLIFE_BEEP_WAND_S3, true, i_volume_effects - i_wand_beep_level);
      break;

      case 4:
        playEffect(S_AFTERLIFE_BEEP_WAND_S4, true, i_volume_effects - i_wand_beep_level);
      break;

      case 5:
        playEffect(S_AFTERLIFE_BEEP_WAND_S5, true, i_volume_effects - i_wand_beep_level);
      break;
    }
  }
}

void wandExtraSoundsBeepLoopStop() {
  stopEffect(S_AFTERLIFE_BEEP_WAND_S1);
  stopEffect(S_AFTERLIFE_BEEP_WAND_S2);
  stopEffect(S_AFTERLIFE_BEEP_WAND_S3);
  stopEffect(S_AFTERLIFE_BEEP_WAND_S4);
  stopEffect(S_AFTERLIFE_BEEP_WAND_S5);
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

  stopEffect(S_WAND_BOOTUP);

  if(b_wand_mash_lockout == true || PACK_STATE == MODE_OFF) {
    stopEffect(S_SMASH_ERROR_LOOP);
    stopEffect(S_SMASH_ERROR_RESTART);
  }
}

// It is very important that S_1 up to S_60 follow each other in order on the Micro SD Card and sound effects enum.
void overheatVoiceIndicator(uint16_t i_tmp_length) {
  i_tmp_length = i_tmp_length / i_overheat_delay_increment;

  uint16_t i_tmp_sound = (S_1 - 1) + i_tmp_length;

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
  // Calculate the "pack" which is the standard Powercell plus Cyclotron Lid,
  // and optionally an N-filter LED jewel array at the end of that chain
  i_pack_num_leds = i_powercell_leds + i_cyclotron_leds + i_nfilter_jewel_leds;
  i_cyclotron_led_start = i_powercell_leds;
  i_vent_light_start = i_powercell_leds + i_cyclotron_leds;

  // Calculate the inner cyclotron which may consist of the optional components:
  // [in order...] Switch Panel + Cake Lights + Cavity Lights
  if(b_inner_cyclotron_led_panel == true) {
    i_ic_panel_end = i_inner_cyclotron_panel_num_leds - 1;
    i_ic_cake_start = i_ic_panel_end + 1;
    i_ic_cake_end = i_ic_cake_start + i_inner_cyclotron_cake_num_leds - 1;
    i_ic_cavity_start = i_ic_cake_end + 1;
    i_ic_cavity_end = i_ic_cavity_start + i_inner_cyclotron_cavity_num_leds - 1;
  }
  else {
    i_ic_panel_end = 0;
    i_ic_cake_start = 0;
    i_ic_cake_end = i_ic_cake_start + i_inner_cyclotron_cake_num_leds - 1;
    i_ic_cavity_start = i_ic_cake_end + 1;
    i_ic_cavity_end = i_ic_cavity_start + i_inner_cyclotron_cavity_num_leds - 1;
  }
}

// Update the LED counts for the inner cyclotron, if we are using the addon LED panel or not.
void resetInnerCyclotronLEDs() {
  if(b_inner_cyclotron_led_panel == true) {
    // For clarity, these are added in the order by which the devices would be connected in the chain.
    i_inner_cyclotron_panel_num_leds = 8; // Maximum is 8 (2 above switches, 6 on the side)
  }
  else {
    // Without the inner panel we just use the dedicated LED ports on the controller for single-colour LEDs.
    // The inner chain just reduces down to the inner cake plus extra cavity lights for the "sparking" FX.
    i_inner_cyclotron_panel_num_leds = 0; // Set to 0 if not enabled.
  }
}

void resetCyclotronLEDs() {
  switch(i_cyclotron_leds) {
    // For a 40 LED Neopixel ring.
    case OUTER_CYCLOTRON_LED_MAX:
      i_2021_delay = CYCLOTRON_DELAY_2021_40_LED;
    break;

    // For Frutto Technology Max Cyclotron (36) LEDs.
    case FRUTTO_MAX_CYCLOTRON_LED_COUNT:
      i_2021_delay = CYCLOTRON_DELAY_2021_36_LED;
    break;

    // For Frutto Technology Cyclotron (20) LEDs.
    case FRUTTO_CYCLOTRON_LED_COUNT:
      i_2021_delay = CYCLOTRON_DELAY_2021_20_LED;
    break;

    // Default HasLab (12) LEDs.
    case HASLAB_CYCLOTRON_LED_COUNT:
    default:
      i_2021_delay = CYCLOTRON_DELAY_2021_12_LED;
    break;
  }
}

void resetContinuousSmoke() {
  b_smoke_continuous_level[0] = b_smoke_continuous_level_1;
  b_smoke_continuous_level[1] = b_smoke_continuous_level_2;
  b_smoke_continuous_level[2] = b_smoke_continuous_level_3;
  b_smoke_continuous_level[3] = b_smoke_continuous_level_4;
  b_smoke_continuous_level[4] = b_smoke_continuous_level_5;
}

// Sourced from https://community.particle.io/t/battery-voltage-checking/5467
// Obtains the ATMega chip's actual Vcc voltage value, using internal bandgap reference.
// This demonstrates ability to read processors Vcc voltage and the ability to maintain A/D calibration with changing Vcc.
void doVoltageCheck() {
  // REFS1 REFS0               --> 0 1, AVcc internal ref. -Selects AVcc reference
  // MUX4 MUX3 MUX2 MUX1 MUX0  --> 11110 1.1V (VBG)        -Selects channel 30, bandgap voltage, to measure
  ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR)| (0<<MUX5) | (1<<MUX4) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);

  // This appears to work without the delay, but for more accurate readings it may be necessary.
  // delay(50); // Let mux settle a little to get a more stable A/D conversion.

  ADCSRA |= _BV( ADSC ); // Start a conversion.
  while( ( (ADCSRA & (1<<ADSC)) != 0 ) ); // Wait for conversion to complete...

  // Scale the value, which returns the actual value of Vcc x 100
  const long InternalReferenceVoltage = 1115L; // Adjust this value to your boards specific internal BG voltage x1000.
  i_batt_volts = (((InternalReferenceVoltage * 1023L) / ADC) + 5L) / 10L; // Calculates for straight line value.

  // Send current voltage value to the serial1 device, if connected.
  if(b_serial1_connected) {
    serial1Send(A_BATTERY_VOLTAGE_PACK, i_batt_volts);
  }
}

// Included last as the contained logic will control all aspects of the pack using the defined functions above.
#include "Serial.h"
