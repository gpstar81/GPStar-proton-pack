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

void bargraphFull() {
  if(b_28segment_bargraph == true) {
    for(uint8_t i = 0; i < 28; i++) {
      ht_bargraph.setLedNow(i_bargraph[i]);
    }    
  }
}

void bargraphClear() {
  if(b_28segment_bargraph == true) {
    ht_bargraph.clearAll();

    i_bargraph_status = 0; // Reset the position/sequence.

    ms_bargraph.stop();
    ms_bargraph_firing.stop();
  }
}

void bargraphRampIdle() {
  if(b_28segment_bargraph == true) {
    if(ms_bargraph.justFinished()) {
      uint8_t i_bargraph_multiplier[5] = { 7, 6, 5, 4, 3 };

      if(b_bargraph_up == true) {
        ht_bargraph.setLedNow(i_bargraph[i_bargraph_status]);

        if(i_bargraph_status > 26) {
          b_bargraph_up = false;

          i_bargraph_status = 27;

          // A little pause when we reach the top.
          ms_bargraph.start(i_bargraph_wait / 2);
        }
        else {
          ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier[4]);
        }

        if(b_bargraph_up == true) {
          i_bargraph_status++;
        }
      }
      else {
        ht_bargraph.clearLedNow(i_bargraph[i_bargraph_status]);

        if(i_bargraph_status == 0) {
          i_bargraph_status = 0;
          b_bargraph_up = true;
          // A little pause when we reach the bottom.
          ms_bargraph.start(i_bargraph_wait / 2);
        }
        else {
          i_bargraph_status--;

          ms_bargraph.start(i_bargraph_interval * 3);
        }
      }
    }
  }
}

void bargraphRampUp() {
  if(b_28segment_bargraph == true) {
    #ifdef GPSTAR_NEUTRONA_WAND_PCB
      switch(i_bargraph_status) {
        case 0 ... 27:
          ht_bargraph.setLedNow(i_bargraph[i_bargraph_status]);

          i_bargraph_status++;

          if(i_bargraph_status == 28) {
            // A little pause when we reach the top.
            ms_bargraph.start(i_bargraph_wait / 2);
          }
          else {
            ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
          }
        break;

        case 28 ... 56:
          uint8_t i_tmp = i_bargraph_status - 27;
          i_tmp = 28 - i_tmp;

          ht_bargraph.clearLedNow(i_bargraph[i_tmp]);

          if(i_bargraph_status == 54) {
            ms_bargraph_alt.start(i_bargraph_interval); // Start the alternate bargraph to ramp up and down continiuously.
            ms_bargraph.stop();
            b_bargraph_up = true;
            i_bargraph_status = 0;
          }
          else {
            ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
            i_bargraph_status++;
          }
        break;
      }
    #endif
  }
  else {
    uint8_t t_bargraph_ramp_multiplier = 1;

    switch(i_bargraph_status) {
      case 0:
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 1:
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 2:
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 3:
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 4:
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 5:
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 6:
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 7:
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 8:
        ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        i_bargraph_status++;
      break;

      case 9:
        ms_bargraph.stop();
        i_bargraph_status = 0;
      break;
    }
  }
}

void prepBargraphRampDown() {
  i_bargraph_status = 28; // For 28 segment bargraph

  bargraphFull();

  ms_bargraph.start(d_bargraph_ramp_interval);

  // Prepare to make the bargraph ramp down now.
  bargraphRampUp();
}

void bargraphRampFiring() {
  if(b_28segment_bargraph == true) {
    int i_ramp_interval = d_bargraph_ramp_interval; // Start with the default value.

    // Start ramping up and down from the middle to the top/bottom and back to the middle again.
    switch(i_bargraph_status) {
      case 0:
        ht_bargraph.setLedNow(i_bargraph[13]);
        ht_bargraph.setLedNow(i_bargraph[14]);

        i_bargraph_status++;

        if(b_bargraph_up == false) {
          ht_bargraph.clearLedNow(i_bargraph[12]);
          ht_bargraph.clearLedNow(i_bargraph[15]);
        }

        b_bargraph_up = true;
      break;

      case 1:
        ht_bargraph.setLedNow(i_bargraph[12]);
        ht_bargraph.setLedNow(i_bargraph[15]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[13]);
          ht_bargraph.clearLedNow(i_bargraph[14]);

          i_bargraph_status++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[11]);
          ht_bargraph.clearLedNow(i_bargraph[16]);

          i_bargraph_status--;
        }
      break;

      case 2:
        ht_bargraph.setLedNow(i_bargraph[11]);
        ht_bargraph.setLedNow(i_bargraph[16]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[12]);
          ht_bargraph.clearLedNow(i_bargraph[15]);

          i_bargraph_status++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[10]);
          ht_bargraph.clearLedNow(i_bargraph[17]);

          i_bargraph_status--;
        }
      break;

      case 3:
        ht_bargraph.setLedNow(i_bargraph[10]);
        ht_bargraph.setLedNow(i_bargraph[17]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[11]);
          ht_bargraph.clearLedNow(i_bargraph[16]);

          i_bargraph_status++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[9]);
          ht_bargraph.clearLedNow(i_bargraph[18]);

          i_bargraph_status--;
        }
      break;

      case 4:
        ht_bargraph.setLedNow(i_bargraph[9]);
        ht_bargraph.setLedNow(i_bargraph[18]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[10]);
          ht_bargraph.clearLedNow(i_bargraph[17]);

          i_bargraph_status++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[8]);
          ht_bargraph.clearLedNow(i_bargraph[19]);

          i_bargraph_status--;
        }
      break;

      case 5:
        ht_bargraph.setLedNow(i_bargraph[8]);
        ht_bargraph.setLedNow(i_bargraph[19]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[9]);
          ht_bargraph.clearLedNow(i_bargraph[18]);

          i_bargraph_status++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[7]);
          ht_bargraph.clearLedNow(i_bargraph[20]);

          i_bargraph_status--;
        }
      break;

      case 6:
        ht_bargraph.setLedNow(i_bargraph[7]);
        ht_bargraph.setLedNow(i_bargraph[20]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[8]);
          ht_bargraph.clearLedNow(i_bargraph[19]);

          i_bargraph_status++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[6]);
          ht_bargraph.clearLedNow(i_bargraph[21]);

          i_bargraph_status--;
        }
      break;

      case 7:
        ht_bargraph.setLedNow(i_bargraph[6]);
        ht_bargraph.setLedNow(i_bargraph[21]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[7]);
          ht_bargraph.clearLedNow(i_bargraph[20]);

          i_bargraph_status++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[5]);
          ht_bargraph.clearLedNow(i_bargraph[22]);

          i_bargraph_status--;
        }
      break;

      case 8:
        ht_bargraph.setLedNow(i_bargraph[5]);
        ht_bargraph.setLedNow(i_bargraph[22]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[6]);
          ht_bargraph.clearLedNow(i_bargraph[21]);

          i_bargraph_status++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[4]);
          ht_bargraph.clearLedNow(i_bargraph[23]);

          i_bargraph_status--;
        }
      break;

      case 9:
        ht_bargraph.setLedNow(i_bargraph[4]);
        ht_bargraph.setLedNow(i_bargraph[23]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[5]);
          ht_bargraph.clearLedNow(i_bargraph[22]);

          i_bargraph_status++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[3]);
          ht_bargraph.clearLedNow(i_bargraph[24]);

          i_bargraph_status--;
        }
      break;

      case 10:
        ht_bargraph.setLedNow(i_bargraph[3]);
        ht_bargraph.setLedNow(i_bargraph[24]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[4]);
          ht_bargraph.clearLedNow(i_bargraph[23]);

          i_bargraph_status++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[2]);
          ht_bargraph.clearLedNow(i_bargraph[25]);

          i_bargraph_status--;
        }
      break;

      case 11:
        ht_bargraph.setLedNow(i_bargraph[2]);
        ht_bargraph.setLedNow(i_bargraph[25]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[3]);
          ht_bargraph.clearLedNow(i_bargraph[24]);

          i_bargraph_status++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[1]);
          ht_bargraph.clearLedNow(i_bargraph[26]);

          i_bargraph_status--;
        }
      break;

      case 12:
        ht_bargraph.setLedNow(i_bargraph[1]);
        ht_bargraph.setLedNow(i_bargraph[26]);

        if(b_bargraph_up == true) {
          ht_bargraph.clearLedNow(i_bargraph[2]);
          ht_bargraph.clearLedNow(i_bargraph[25]);

          i_bargraph_status++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[0]);
          ht_bargraph.clearLedNow(i_bargraph[27]);

          i_bargraph_status--;
        }
      break;

      case 13:
        ht_bargraph.setLedNow(i_bargraph[0]);
        ht_bargraph.setLedNow(i_bargraph[27]);

        ht_bargraph.clearLedNow(i_bargraph[1]);
        ht_bargraph.clearLedNow(i_bargraph[26]);

        i_bargraph_status--;

        b_bargraph_up = false;
      break;
    }

    switch(POWER_LEVEL) {
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
      default:
        ms_bargraph_firing.start((i_ramp_interval / 2) + 12); // 30
      break;
    }
  }
}
