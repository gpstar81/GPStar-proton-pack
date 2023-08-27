/**
 *   gpstar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
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

  // RGB LED's for effects (upper/lower).
  FastLED.addLeds<NEOPIXEL, ATTENUATOR_LED_PIN>(attenuator_leds, ATTENUATOR_NUM_LEDS);

  // Debounce the toggle switches.
  switch_left.setDebounceTime(switch_debounce_time);
  switch_right.setDebounceTime(switch_debounce_time);

  // Rotary encoder on the top of the attenuator.
  pinMode(r_encoderA, INPUT_PULLUP);
  pinMode(r_encoderB, INPUT_PULLUP);

  // Setup the bargraph after a brief delay.
  delay(10);
  setupBargraph();
}

void loop() {
  if(b_wait_for_pack == true) {
    // Handshake with the pack. Telling the pack that we are here.
    packSerialSend(W_HANDSHAKE);

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
  checkRotary();
  checkSwitches();

  /*
   * The left toggle activates the bargraph display manually.
   * When paired with the gpstar Proton Pack, the bargraph
   * will automatically enable and display an animation.
   *
   * When idle, the user can change the pattern on the device.
   * When firing, an alternative pattern should be used, such
   * as the standard animation as used on the wand.
   */
  if(b_left_toggle == true || b_pack_on == true) {
    // Turn the bargraph on (using some pattern).
    bargraphRun();
  }
  else {
    // Turn the bargraph off (clear all elements).
    bargraphOff();
  }

  /*
   * The right toggle activates the LED's on the device manually.
   *
   * Since this device will have a serial connection to the pack,
   * the lights will change colors based on user interactions.
   */ 
  if(b_right_toggle == true || b_pack_on == true) {
    // Set upper LED based on overheating state, if available.
    attenuator_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_AMBER_PULSE);

    // Set lower LED based on firing mode, if available.
    uint8_t i_scheme = C_RED;
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
        i_scheme = C_RAINBOW;
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
    FastLED.show();
  }
  else {
    // Turn off the LED's by setting to black.
    attenuator_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_BLACK);
    attenuator_leds[LOWER_LED] = getHueAsRGB(LOWER_LED, C_BLACK);
    FastLED.show();
  }

  // Update the device LEDs.
  // if(ms_fast_led.justFinished()) {
  //   FastLED.show();
  //   ms_fast_led.start(i_fast_led_delay);
  // }
}

void checkRotary() {
  // This will eventually do something, such as changing the pattern for the bargraph.

}

void checkSwitches() {
  // Determine the toggle states.

  // if(b_debug == true) {
  //   Serial.print(F("D3 Left -> "));
  //   Serial.println(switch_left.getState());

  //   Serial.print(F("D4 Right -> "));
  //   Serial.println(switch_right.getState());
  // }

  // Set a variable which tells us if the toggle is on or off.
  if(switch_left.getState() == LOW) {
    if(b_debug == true && b_left_toggle == false) {
      Serial.println("Left Toggle On");
    }

    b_left_toggle = true;
  }
  else {
    if(b_debug == true && b_left_toggle == true) {
      Serial.println("Left Toggle Off");
    }

    b_left_toggle = false;
  }

  // Set a variable which tells us if the toggle is on or off.
  if(switch_right.getState() == LOW) {
    if(b_debug == true && b_right_toggle == false) {
      Serial.println("Right Toggle On");
    }

    b_right_toggle = true;
  }
  else {
    if(b_debug == true && b_right_toggle == true) {
      Serial.println("Right Toggle Off");
    }

    b_right_toggle = false;
  }
}

void switchLoops() {
  // Perform debounce and get button/switch states.
  switch_left.loop();
  switch_right.loop();
}

void packSerialSend(int i_message) {
  sendStruct.i = i_message;
  sendStruct.s = W_COM_START;
  sendStruct.e = W_COM_END;

  packComs.sendDatum(sendStruct);
}

void checkPack() {
  // Pack communication to the Attenuator device.
  if(packComs.available()) {
    packComs.rxObj(comStruct);

    if(!packComs.currentPacketID()) {
      if(comStruct.i > 0 && comStruct.s == P_COM_START && comStruct.e == P_COM_END) {
        // Use the passed communication flag to set the proper state for this device.
        switch(comStruct.i) {
          case P_ON:
            // Pack is on.
            if(b_debug && b_pack_on == false) {
              Serial.println("Pack On");
            }
            b_pack_on = true;
          break;

          case P_OFF:
            // Pack is off.
            if(b_debug && b_pack_on == true) {
              Serial.println("Pack Off");
            }
            b_pack_on = false;
          break;

          case P_SYNC_START:
            b_sync = true;
          break;

          case P_SYNC_END:
            b_sync = false;
          break;

          case P_ALARM_ON:
            // Alarm is on.
            if(b_debug && b_pack_alarm == false) {
              Serial.println("Alarm On");
            }
            b_pack_alarm = true;
          break;

          case P_ALARM_OFF:
            // Alarm is off.
            if(b_debug && b_pack_alarm == true) {
              Serial.println("Alarm Off");
            }
            b_pack_alarm = false;
          break;

          case P_HANDSHAKE:
            // The pack is asking us if we are still here. Respond back.
            packSerialSend(W_HANDSHAKE);
          break;

          case P_YEAR_1984:
            if(b_debug && i_mode_year != 1984) {
              Serial.println("Mode 1984");
            }
            i_mode_year = 1984;
          break;

          case P_YEAR_1989:
            if(b_debug && i_mode_year != 1989) {
              Serial.println("Mode 1989");
            }
            i_mode_year = 1989;
          break;

          case P_YEAR_AFTERLIFE:
            if(b_debug && i_mode_year != 2021) {
              Serial.println("Mode 2021");
            }
            i_mode_year = 2021;
          break;

          case P_PROTON_MODE:
            FIRING_MODE = PROTON;
          break;

          case P_SLIME_MODE:
            FIRING_MODE = SLIME;
          break;

          case P_STASIS_MODE:
            FIRING_MODE = STASIS;
          break;

          case P_MESON_MODE:
            FIRING_MODE = MESON;
          break;

          case P_SPECTRAL_CUSTOM_MODE:
            FIRING_MODE = SPECTRAL_CUSTOM;
          break;

          case P_SPECTRAL_MODE:
            FIRING_MODE = SPECTRAL;
          break;

          case P_HOLIDAY_MODE:
            FIRING_MODE = HOLIDAY;
          break;

          case P_VENTING_MODE:
            FIRING_MODE = VENTING;
          break;

          case P_SETTINGS_MODE:
            FIRING_MODE = SETTINGS;
          break;

          case P_POWER_LEVEL_1:
            POWER_LEVEL = LEVEL_1;
          break;

          case P_POWER_LEVEL_2:
            POWER_LEVEL = LEVEL_2;
          break;

          case P_POWER_LEVEL_3:
            POWER_LEVEL = LEVEL_4;
          break;

          case P_POWER_LEVEL_4:
            POWER_LEVEL = LEVEL_4;
          break;

          case P_POWER_LEVEL_5:
            POWER_LEVEL = LEVEL_5;
          break;
        }
      }

      comStruct.i = 0;
      comStruct.s = 0;
    }
  }
}