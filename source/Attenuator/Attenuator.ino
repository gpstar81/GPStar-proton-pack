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

  // Setup the bargraph.
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

}

void loop() {

  mainLoop();

}

void mainLoop() {

  switchLoops();
  checkRotary();
  checkSwitches();

  // Update the device LEDs.
  if(ms_fast_led.justFinished()) {
    FastLED.show();
    ms_fast_led.stop();
  }
}

void checkRotary() {

}

void checkSwitches() {

}

void switchLoops() {
  switch_left.loop();
  switch_right.loop();
}
