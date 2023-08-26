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

void setup() {
  Serial.begin(9600);

  // Enable Serial1 if compiling for the gpstar Neutrona Wand micro controller.
  #ifdef HAVE_HWSERIAL1
    Serial1.begin(9600);
    packComs.begin(Serial1);
  #else
    packComs.begin(Serial);
  #endif

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
  mainLoop();

}

void mainLoop() {
  // Monitor for interactions by user.
  checkPack();
  switchLoops();
  checkRotary();
  checkSwitches();

  /*
   * The left toggle activates the bargraph display.
   *
   * When idle, the user can change the pattern on the device.
   * When firing, an alternative pattern should be used, such
   * as the standard animation as used on the wand.
   */
  if(b_left_toggle == true){
    // Turn the bargraph on (using some pattern).
    bargraphRun();
    b_bargraph_on = true;
  }
  else {
    // Turn the bargraph off (clear all elements).
    if(!b_bargraph_on == true){
      bargraphOff();
    }
    b_bargraph_on = false;
  }

  /*
   * The right toggle activates the LED's on the device.
   *
   * Since this device will have a serial connection to the pack,
   * the light should ideally change colors based on user action.
   * Example: The upper LED could go from orange/amber to red as
   *          the pack begins to overheat, and back to yellow after
   *          the vent cycle reboots the pack.
   *          The lower LED could change color based on the firing
   *          mode current in use by the wand.
   */ 
  if(b_right_toggle == true){
    attenuator_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_ORANGE);
    // attenuator_leds[LOWER_LED] = getHueAsRGB(LOWER_LED, C_RED);
    attenuator_leds[LOWER_LED] = getHueAsRGB(LOWER_LED, C_RAINBOW);
    FastLED.show();
  }
  else {
    attenuator_leds[UPPER_LED] = getHueAsRGB(UPPER_LED, C_BLACK);
    attenuator_leds[LOWER_LED] = getHueAsRGB(LOWER_LED, C_BLACK);
    FastLED.show();
  }
}

void setupBargraph() {
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
  
  Serial.print(F("Bargraph Present -> "));
  Serial.println(b_28segment_bargraph);
  
  if(b_28segment_bargraph == true) {
    ht_bargraph.begin(0x00);
  }
}

void bargraphRun() {
  if(b_28segment_bargraph == true) {
    for(uint8_t i = 0; i < 28; i++) {
      ht_bargraph.setLedNow(i_bargraph[i]);
    }
  }
}

void bargraphOff() {
  if(b_28segment_bargraph == true) {
    for(uint8_t i = 0; i < 28; i++) {
      ht_bargraph.clearLedNow(i_bargraph[i]);
    }
  }
}

void checkRotary() {
  // This will eventually do something, such as changing the pattern for the bargraph.

}

void checkSwitches() {
  // Determine the toggle states.

  if(b_debug == true) {
    // Serial.print(F("D3 Left -> "));
    // Serial.println(switch_left.getState());

    // Serial.print(F("D4 Right -> "));
    // Serial.println(switch_right.getState());
  }

  // Set a variable which tells us if the toggle is on or off.
  if(switch_left.getState() == LOW) {
    if(b_debug == true && b_left_toggle == false) {
      Serial.println("left toggle on");
    }

    b_left_toggle = true;
  }
  else {
    b_left_toggle = false;
  }

  // Set a variable which tells us if the toggle is on or off.
  if(switch_right.getState() == LOW) {
    if(b_debug == true && b_right_toggle == false) {
      Serial.println("right toggle on");
    }

    b_right_toggle = true;
  }
  else {
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

// Pack communication to the device.
void checkPack() {
    if(packComs.available()) {

    }
}