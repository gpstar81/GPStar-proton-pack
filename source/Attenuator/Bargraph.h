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

/**
 * Control patterns displayed by the 28-segment bargraph device.
 */

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

    b_bargraph_on = true; // Denote that the device is "on".
  }
}

void bargraphOff() {
  if(b_28segment_bargraph == true && b_bargraph_on == true) {
    for(uint8_t i = 0; i < 28; i++) {
      ht_bargraph.clearLedNow(i_bargraph[i]);
    }

    b_bargraph_on = false; // Denote that the device is "off".
  }
}