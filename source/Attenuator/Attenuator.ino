/**
 *   gpstar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
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

// 3rd-Party Libraries
#include <millisDelay.h>
#include <FastLED.h>
#include <ezButton.h>
#include <ht16k33.h>
#include <Wire.h>
#include <SerialTransfer.h>

// Local Files
#include "Configuration.h"
#include "Communication.h"
#include "Header.h"
#include "Colours.h"
#include "Bargraph.h"

void setup() {
  Serial.begin(9600);

  // Enable Serial connection for communication with gpstar Proton Pack PCB.
  packComs.begin(Serial);

  // Bootup into proton mode (default for pack and wand).
  FIRING_MODE = PROTON;
  POWER_LEVEL = LEVEL_1;

  // RGB LED's for effects (upper/lower).
  FastLED.addLeds<NEOPIXEL, ATTENUATOR_LED_PIN>(attenuator_leds, ATTENUATOR_NUM_LEDS);

  // Debounce the toggle switches and encoder pushbutton.
  switch_left.setDebounceTime(switch_debounce_time);
  switch_right.setDebounceTime(switch_debounce_time);
  encoder_center.setDebounceTime(switch_debounce_time);

  // Rotary encoder on the top of the attenuator.
  pinMode(r_encoderA, INPUT_PULLUP);
  pinMode(r_encoderB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(r_encoderA), readEncoder, CHANGE);

  // Setup the bargraph after a brief delay.
  delay(10);
  setupBargraph();

  ms_fast_led.start(1);
  ms_bargraph.start(1);
}

void loop() {
  if(b_wait_for_pack == true) {
    // Handshake with the pack. Telling the pack that we are here.
    attenuatorSerialSend(A_HANDSHAKE);

    // Synchronise some settings with the pack.
    checkPack();

    delay(10);
  }
  else {
    mainLoop();
  }
}

void mainLoop() {
  // Monitor for interactions by user.
  checkPack();
  switchLoops();
  checkRotaryEncoder();

  // For now, use a press of the dial to start/stop the music.
  if(encoder_center.isReleased()) {
    attenuatorSerialSend(A_MUSIC_START_STOP);    
    //attenuatorSerialSend(A_TOGGLE_MUTE);    
  }

  /*
   * Left Toggle
   * When paired with the gpstar Proton Pack, will turn the
   * pack on or off. When the pack is on the bargraph will
   * automatically enable and display an animation which
   * matches the Neutrona Wand bargraph.
   *
   * TODO: Allow the user to select a bargraph pattern, or
   * simply control certain pack/wand behavior as desired.
   */

  // Turns the pack on or off (when paired) via left toggle.
  if(switch_left.isPressed() || switch_left.isReleased()) {
    if(switch_left.getState() == LOW) {
      attenuatorSerialSend(A_TURN_PACK_ON);
    }
    else {
      attenuatorSerialSend(A_TURN_PACK_OFF);
    }
  }

  if(b_pack_on == true) {
    if(b_pack_alarm != true) {
      // Turn the bargraph on (using some pattern).
      if(b_firing == true) {
        if(ms_bargraph_firing.justFinished()) {
          bargraphRampFiring();
        }
      }
      else {
        if(ms_bargraph.justFinished()) {
          bargraphRampUp();
        }
        else if(ms_bargraph.isRunning() != true && b_overheating != true && FIRING_MODE != SETTINGS) {
          // Bargraph idling loop.
          bargraphPowerCheck();
        }
      }
    }
    else {
      // This is going to cause the bargraph to ramp down.
      if(ms_bargraph.justFinished()) {
        bargraphRampUp();
      }
    }
  }
  else {
    // Clear all bargraph elements.
    bargraphClear();
  }

  /*
   * The right toggle activates the LED's on the device manually.
   *
   * Since this device will have a serial connection to the pack,
   * the lights will change colors based on user interactions.
   */
  if(switch_right.getState() == LOW) {
    if((b_firing == true && i_speed_multiplier > 1) || b_overheating == true || b_pack_alarm == true) {
      if(ms_blink_leds.justFinished()) {
        ms_blink_leds.start(i_blink_leds / i_speed_multiplier);
      }

      if(ms_blink_leds.isRunning() == true) {
        if(ms_blink_leds.remaining() < (i_blink_leds / i_speed_multiplier) / 2) {
          // Only blink the lower LED as we will use a fade effect for the upper LED.
          //attenuator_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_BLACK);
          attenuator_leds[LOWER_LED] = getHueAsRGB(LOWER_LED, C_BLACK);
        }
        else {
          controlLEDs();
        }
      }
    }
    else {
      controlLEDs();
    }
  }
  else {
    // Turn off the LED's by setting to black.
    attenuator_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_BLACK);
    attenuator_leds[LOWER_LED] = getHueAsRGB(LOWER_LED, C_BLACK);
  }

  // Update the device LEDs.
  if(ms_fast_led.justFinished()) {
    FastLED.show();
    ms_fast_led.start(i_fast_led_delay);
  }
}

void controlLEDs() {
  // Set upper LED based on alarm or overheating state, when active.
  if(b_pack_alarm == true || b_overheating == true) {
    attenuator_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_RED_FADE);
  }
  else {
    attenuator_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_AMBER_PULSE);
  }

  // Set lower LED based on firing mode.
  uint8_t i_scheme;
  switch(FIRING_MODE) {
    case SLIME:
      i_scheme = C_GREEN;
    break;
    case STASIS:
      i_scheme = C_LIGHT_BLUE;
    break;
    case MESON:
      i_scheme = C_ORANGE;
    break;
    case SPECTRAL:
      i_scheme = C_RAINBOW;
    break;
    case HOLIDAY:
      i_scheme = C_REDGREEN;
    break;
    case SPECTRAL_CUSTOM:
      i_scheme = C_SPECTRAL_CUSTOM;
    break;
    case SETTINGS:
      i_scheme = C_WHITE;
    break;
    case PROTON:
    case VENTING:
    default:
      i_scheme = C_RED;
    break;
  }

  attenuator_leds[LOWER_LED] = getHueAsRGB(LOWER_LED, i_scheme);
}

void readEncoder() {
  if(digitalRead(r_encoderA) == digitalRead(r_encoderB)) {
    i_encoder_pos++;
  }
  else {
    i_encoder_pos--;
  }

  i_val_rotary = i_encoder_pos / 2.5;
}

void checkRotaryEncoder() {
  if(i_val_rotary > i_last_val_rotary) {
    if(ms_rotary_debounce.isRunning() != true) {
      // Tell wand to increase volume.
      attenuatorSerialSend(A_VOLUME_INCREASE);

      ms_rotary_debounce.start(rotary_debounce_time);
    }
  }

  if(i_val_rotary < i_last_val_rotary) {
    if(ms_rotary_debounce.isRunning() != true) {
      // Tell wand to decrease the volume.
      attenuatorSerialSend(A_VOLUME_DECREASE);

      ms_rotary_debounce.start(rotary_debounce_time);
    }
  }

  i_last_val_rotary = i_val_rotary;

  if(ms_rotary_debounce.justFinished()) {
    ms_rotary_debounce.stop();
  }
}

void switchLoops() {
  // Perform debounce and get button/switch states.
  switch_left.loop();
  switch_right.loop();
  encoder_center.loop();
}

void attenuatorSerialSend(int i_message) {
  sendStruct.i = i_message;
  sendStruct.s = A_COM_START;
  sendStruct.e = A_COM_END;

  packComs.sendDatum(sendStruct);
}

void checkPack() {
  // Pack communication to the Attenuator device.
  if(packComs.available()) {
    packComs.rxObj(comStruct);

    if(!packComs.currentPacketID()) {
      if(comStruct.i > 0 && comStruct.s == A_COM_START && comStruct.e == A_COM_END) {
        // Use the passed communication flag to set the proper state for this device.
        switch(comStruct.i) {
          case A_SYNC_START:
            b_wait_for_pack = true;
            i_speed_multiplier = 1;
          break;

          case A_SYNC_END:
            b_wait_for_pack = false;
          break;

          case A_PACK_ON:
            // Pack is on.
            b_pack_on = true;

            if(b_pack_alarm != true) {
              bargraphRampUp();
            }
          break;

          case A_PACK_OFF:
            // Pack is off.
            b_pack_on = false;

            i_bargraph_status_alt = 0;
            i_bargraph_status = 0;
          break;

          case A_PACK_CONNECTED:
            // The Proton Pack is connected.
          break;

          case A_HANDSHAKE:
            // The pack is asking us if we are still here. Respond back.
            attenuatorSerialSend(A_HANDSHAKE);
          break;

          case A_YEAR_1984:
            i_mode_year = 1984;

            bargraphYearModeUpdate();
          break;

          case A_YEAR_1989:
            i_mode_year = 1989;

            bargraphYearModeUpdate();
          break;

          case A_YEAR_AFTERLIFE:
            i_mode_year = 2021;

            bargraphYearModeUpdate();
          break;

          case A_PROTON_MODE:
            FIRING_MODE = PROTON;
          break;

          case A_SLIME_MODE:
            FIRING_MODE = SLIME;
          break;

          case A_STASIS_MODE:
            FIRING_MODE = STASIS;
          break;

          case A_MESON_MODE:
            FIRING_MODE = MESON;
          break;

          case A_SPECTRAL_CUSTOM_MODE:
            FIRING_MODE = SPECTRAL_CUSTOM;

            if(comStruct.d1 > 0) {
              i_spectral_custom = comStruct.d1;
            }

            if(comStruct.d2 > 0) {
              i_spectral_custom_saturation = comStruct.d2;
            }
          break;

          case A_SPECTRAL_COLOUR_DATA:
            if(comStruct.d1 > 0) {
              i_spectral_custom = comStruct.d1;
            }

            if(comStruct.d2 > 0) {
              i_spectral_custom_saturation = comStruct.d2;
            }
          break;

          case A_SPECTRAL_MODE:
            FIRING_MODE = SPECTRAL;
          break;

          case A_HOLIDAY_MODE:
            FIRING_MODE = HOLIDAY;
          break;

          case A_VENTING_MODE:
            FIRING_MODE = VENTING;
          break;

          case A_SETTINGS_MODE:
            FIRING_MODE = SETTINGS;
          break;

          case A_POWER_LEVEL_1:
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_1;

            if(i_mode_year == 2021 && b_28segment_bargraph == true) {
              bargraphPowerCheck2021Alt(false);
            }
          break;

          case A_POWER_LEVEL_2:
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_2;

            if(i_mode_year == 2021 && b_28segment_bargraph == true) {
              bargraphPowerCheck2021Alt(false);
            }
          break;

          case A_POWER_LEVEL_3:
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_3;

            if(i_mode_year == 2021 && b_28segment_bargraph == true) {
              bargraphPowerCheck2021Alt(false);
            }
          break;

          case A_POWER_LEVEL_4:
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_4;

            if(i_mode_year == 2021 && b_28segment_bargraph == true) {
              bargraphPowerCheck2021Alt(false);
            }
          break;

          case A_POWER_LEVEL_5:
            POWER_LEVEL_PREV = POWER_LEVEL;
            POWER_LEVEL = LEVEL_5;

            if(i_mode_year == 2021 && b_28segment_bargraph == true) {
              bargraphPowerCheck2021Alt(false);
            }
          break;

          case A_ALARM_ON:
            // Alarm is on.
            b_pack_alarm = true;

            if(b_pack_on == true) {
              ms_blink_leds.start(i_blink_leds);

              bargraphFull();

              // Reset some bargraph levels before we ramp the bargraph down.
              i_bargraph_status_alt = 28;
              i_bargraph_status = 5;

              bargraphFull();

              ms_bargraph.start(d_bargraph_ramp_interval);
            }
          break;

          case A_ALARM_OFF:
            // Alarm is off.
            b_pack_alarm = false;

            if(b_pack_on == true) {
              ms_blink_leds.stop();

              bargraphYearModeUpdate();

              bargraphClearAlt();

              bargraphRampUp();
            }
          break;

          case A_OVERHEATING:
            // Pack is overheating.
            b_overheating = true;
            ms_blink_leds.start(i_blink_leds);

            // Reset some bargraph levels before we ramp the bargraph down.
            i_bargraph_status_alt = 28;
            i_bargraph_status = 5;

            bargraphFull();

            ms_bargraph.start(d_bargraph_ramp_interval);
          break;

          case A_OVERHEATING_FINISHED:
            b_overheating = false;
            ms_blink_leds.stop();

            bargraphYearModeUpdate();

            bargraphClearAlt();

            bargraphRampUp();
          break;

          case A_FIRING:
            b_firing = true;
            ms_blink_leds.start(i_blink_leds / i_speed_multiplier);

            bargraphClearAlt();

            ms_bargraph_alt.stop();
            i_bargraph_status_alt = 0;
            b_bargraph_up = false;

            i_bargraph_status = 1;

            // Stop any bargraph ramps.
            ms_bargraph.stop();

            ms_bargraph_firing.start(1);
          break;

          case A_FIRING_STOPPED:
            b_firing = false;
            ms_blink_leds.stop();
            i_speed_multiplier = 1;

            ms_bargraph_firing.stop();

            ms_bargraph_alt.stop(); // Stop the 1984 24 segment optional bargraph timer.

            b_bargraph_up = false;

            i_bargraph_status = POWER_LEVEL - 1;

            i_bargraph_status_alt = 0;
            i_bargraph_status = 0; //??
            bargraphClearAlt();

            switch(i_mode_year) {
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
          break;

          case A_CYCLOTRON_INCREASE_SPEED:
            i_speed_multiplier++;
          break;

          case A_CYCLOTRON_NORMAL_SPEED:
            i_speed_multiplier = 1;
          break;
        }
      }

      comStruct.i = 0;
      comStruct.s = 0;
    }
  }
}