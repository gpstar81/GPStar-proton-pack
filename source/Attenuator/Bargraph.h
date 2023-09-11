/**
 *   gpstar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com> & Dustin Grau
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

void bargraphFull() {
  if(b_28segment_bargraph) {
    for(uint8_t i = 0; i < 28; i++) {
      ht_bargraph.setLedNow(i_bargraph[i]);
    }
  }
}

void bargraphClear() {
  if(b_28segment_bargraph) {
    ht_bargraph.clearAll();

    i_bargraph_status = 0; // Reset the position/sequence.

    ms_bargraph.stop();
    ms_bargraph_firing.stop();
  }
}

void bargraphPowerCheck2021Alt(bool b_override) {
  if((!b_firing && FIRING_MODE != SETTINGS && !b_overheating) || b_override) {
    if(POWER_LEVEL != POWER_LEVEL_PREV) {
      if(POWER_LEVEL > POWER_LEVEL_PREV) {
        b_bargraph_up = true;
      }
      else {
        b_bargraph_up = false;
      }
      switch(POWER_LEVEL) {
        case LEVEL_5:
          ms_bargraph_alt.start(i_bargraph_wait / 3);
        break;

        case LEVEL_4:
          ms_bargraph_alt.start(i_bargraph_wait / 4);
        break;

        case LEVEL_3:
          ms_bargraph_alt.start(i_bargraph_wait / 5);
        break;

        case LEVEL_2:
          ms_bargraph_alt.start(i_bargraph_wait / 6);
        break;

        case LEVEL_1:
          ms_bargraph_alt.start(i_bargraph_wait / 7);
        break;
      }
    }
  }
}

void bargraphYearModeUpdate() {
  switch(YEAR_MODE) {
    case YEAR_2021:
      i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_2021;
    break;

    case YEAR_1984:
    case YEAR_1989:
      i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_1984;
    break;
  }
}

void bargraphRampIdle() {
  if(b_28segment_bargraph) {
    if(ms_bargraph.justFinished()) {
      uint8_t i_bargraph_multiplier[5] = { 7, 6, 5, 4, 3 };

      if(b_bargraph_up) {
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

        if(b_bargraph_up) {
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
  if(b_28segment_bargraph) {
    switch(i_bargraph_status_alt) {
      case 0 ... 27:
        ht_bargraph.setLedNow(i_bargraph[i_bargraph_status_alt]);

        i_bargraph_status_alt++;

        if(i_bargraph_status_alt == 28) {
          // A little pause when we reach the top.
          ms_bargraph.start(i_bargraph_wait / 2);

          // Adjust the ramp down speed if necessary.
          switch(YEAR_MODE) {
            case YEAR_2021:
              i_bargraph_multiplier_current  = i_bargraph_multiplier_ramp_2021 / 2;
            break;

            case YEAR_1984:
            case YEAR_1989:
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

        if(b_overheating || b_pack_alarm) {
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
          if((POWER_LEVEL < 5 && YEAR_MODE == YEAR_2021) || YEAR_MODE == YEAR_1984 || YEAR_MODE == YEAR_1989) {
            ht_bargraph.clearLedNow(i_bargraph[i_tmp]);
          }

          switch(YEAR_MODE) {
            case YEAR_1984:
            case YEAR_1989:
              // Bargraph has ramped up and down. In 1984 mode we want to start the ramping.
              if(i_bargraph_status_alt == 54) {
                ms_bargraph_alt.start(i_bargraph_interval); // Start the alternate bargraph to ramp up and down continuously.
                ms_bargraph.stop();
                b_bargraph_up = true;
                i_bargraph_status_alt = 0;
                bargraphYearModeUpdate();
              }
              else {
                ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
                i_bargraph_status_alt++;
              }

            break;

            case YEAR_2021:
              switch(POWER_LEVEL) {
                case LEVEL_5:
                  // Stop any power check in 2021 if we are already in level 5.
                  ms_bargraph_alt.stop();

                  ms_bargraph.stop();
                  b_bargraph_up = false;
                  i_bargraph_status_alt = 27;
                  bargraphYearModeUpdate();
                break;

                case LEVEL_4:
                  if(i_bargraph_status_alt == 31) {
                    ms_bargraph.stop();
                    b_bargraph_up = false;
                    i_bargraph_status_alt = 23;
                    bargraphYearModeUpdate();
                  }
                  else {
                    ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
                    i_bargraph_status_alt++;
                  }
                break;

                case LEVEL_3:
                  if(i_bargraph_status_alt == 37) {
                    ms_bargraph.stop();
                    b_bargraph_up = false;
                    i_bargraph_status_alt = 17;
                    bargraphYearModeUpdate();
                  }
                  else {
                    ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
                    i_bargraph_status_alt++;
                  }
                break;

                case LEVEL_2:
                  if(i_bargraph_status_alt == 43) {
                    ms_bargraph.stop();
                    b_bargraph_up = false;
                    i_bargraph_status_alt = 11;
                    bargraphYearModeUpdate();
                  }
                  else {
                    ms_bargraph.start(i_bargraph_interval * i_bargraph_multiplier_current);
                    i_bargraph_status_alt++;
                  }
                break;

                case LEVEL_1:
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
    uint8_t t_bargraph_ramp_multiplier = 1;

    if(b_overheating || b_pack_alarm) {
      t_bargraph_ramp_multiplier = 2;
    }

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
        if(i_bargraph_status + 1 == POWER_LEVEL && !b_overheating) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          i_bargraph_status++;
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        }
      break;

      case 5:
        if(i_bargraph_status - 1 == POWER_LEVEL && !b_overheating) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          i_bargraph_status++;
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        }
      break;

      case 6:
        if(i_bargraph_status - 3 == POWER_LEVEL && !b_overheating) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          i_bargraph_status++;
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        }
      break;

      case 7:
        if(i_bargraph_status - 5 == POWER_LEVEL && !b_overheating) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          i_bargraph_status++;
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
        }
      break;

      case 8:
        if(i_bargraph_status - 7 == POWER_LEVEL && !b_overheating) {
          ms_bargraph.stop();
          i_bargraph_status = 0;
        }
        else {
          ms_bargraph.start(d_bargraph_ramp_interval * t_bargraph_ramp_multiplier);
          i_bargraph_status++;
        }
      break;

      case 9:
        ms_bargraph.stop();
        i_bargraph_status = 0;
      break;
    }
  }
}

void bargraphRampFiring() {
  if(b_28segment_bargraph) {
    // Start ramping up and down from the middle to the top/bottom and back to the middle again.
    switch(i_bargraph_status_alt) {
      case 0:
        ht_bargraph.setLedNow(i_bargraph[13]);
        ht_bargraph.setLedNow(i_bargraph[14]);

        i_bargraph_status_alt++;

        if(b_bargraph_up == false) {
          ht_bargraph.clearLedNow(i_bargraph[12]);
          ht_bargraph.clearLedNow(i_bargraph[15]);
        }

        b_bargraph_up = true;
      break;

      case 1:
        ht_bargraph.setLedNow(i_bargraph[12]);
        ht_bargraph.setLedNow(i_bargraph[15]);

        if(b_bargraph_up) {
          ht_bargraph.clearLedNow(i_bargraph[13]);
          ht_bargraph.clearLedNow(i_bargraph[14]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[11]);
          ht_bargraph.clearLedNow(i_bargraph[16]);

          i_bargraph_status_alt--;
        }
      break;

      case 2:
        ht_bargraph.setLedNow(i_bargraph[11]);
        ht_bargraph.setLedNow(i_bargraph[16]);

        if(b_bargraph_up) {
          ht_bargraph.clearLedNow(i_bargraph[12]);
          ht_bargraph.clearLedNow(i_bargraph[15]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[10]);
          ht_bargraph.clearLedNow(i_bargraph[17]);

          i_bargraph_status_alt--;
        }
      break;

      case 3:
        ht_bargraph.setLedNow(i_bargraph[10]);
        ht_bargraph.setLedNow(i_bargraph[17]);

        if(b_bargraph_up) {
          ht_bargraph.clearLedNow(i_bargraph[11]);
          ht_bargraph.clearLedNow(i_bargraph[16]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[9]);
          ht_bargraph.clearLedNow(i_bargraph[18]);

          i_bargraph_status_alt--;
        }
      break;

      case 4:
        ht_bargraph.setLedNow(i_bargraph[9]);
        ht_bargraph.setLedNow(i_bargraph[18]);

        if(b_bargraph_up) {
          ht_bargraph.clearLedNow(i_bargraph[10]);
          ht_bargraph.clearLedNow(i_bargraph[17]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[8]);
          ht_bargraph.clearLedNow(i_bargraph[19]);

          i_bargraph_status_alt--;
        }

      break;

      case 5:
        ht_bargraph.setLedNow(i_bargraph[8]);
        ht_bargraph.setLedNow(i_bargraph[19]);

        if(b_bargraph_up) {
          ht_bargraph.clearLedNow(i_bargraph[9]);
          ht_bargraph.clearLedNow(i_bargraph[18]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[7]);
          ht_bargraph.clearLedNow(i_bargraph[20]);

          i_bargraph_status_alt--;
        }
      break;

      case 6:
        ht_bargraph.setLedNow(i_bargraph[7]);
        ht_bargraph.setLedNow(i_bargraph[20]);

        if(b_bargraph_up) {
          ht_bargraph.clearLedNow(i_bargraph[8]);
          ht_bargraph.clearLedNow(i_bargraph[19]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[6]);
          ht_bargraph.clearLedNow(i_bargraph[21]);

          i_bargraph_status_alt--;
        }
      break;

      case 7:
        ht_bargraph.setLedNow(i_bargraph[6]);
        ht_bargraph.setLedNow(i_bargraph[21]);

        if(b_bargraph_up) {
          ht_bargraph.clearLedNow(i_bargraph[7]);
          ht_bargraph.clearLedNow(i_bargraph[20]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[5]);
          ht_bargraph.clearLedNow(i_bargraph[22]);

          i_bargraph_status_alt--;
        }
      break;

      case 8:
        ht_bargraph.setLedNow(i_bargraph[5]);
        ht_bargraph.setLedNow(i_bargraph[22]);

        if(b_bargraph_up) {
          ht_bargraph.clearLedNow(i_bargraph[6]);
          ht_bargraph.clearLedNow(i_bargraph[21]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[4]);
          ht_bargraph.clearLedNow(i_bargraph[23]);

          i_bargraph_status_alt--;
        }
      break;

      case 9:
        ht_bargraph.setLedNow(i_bargraph[4]);
        ht_bargraph.setLedNow(i_bargraph[23]);

        if(b_bargraph_up) {
          ht_bargraph.clearLedNow(i_bargraph[5]);
          ht_bargraph.clearLedNow(i_bargraph[22]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[3]);
          ht_bargraph.clearLedNow(i_bargraph[24]);

          i_bargraph_status_alt--;
        }
      break;

      case 10:
        ht_bargraph.setLedNow(i_bargraph[3]);
        ht_bargraph.setLedNow(i_bargraph[24]);

        if(b_bargraph_up) {
          ht_bargraph.clearLedNow(i_bargraph[4]);
          ht_bargraph.clearLedNow(i_bargraph[23]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[2]);
          ht_bargraph.clearLedNow(i_bargraph[25]);

          i_bargraph_status_alt--;
        }
      break;

      case 11:
        ht_bargraph.setLedNow(i_bargraph[2]);
        ht_bargraph.setLedNow(i_bargraph[25]);

        if(b_bargraph_up) {
          ht_bargraph.clearLedNow(i_bargraph[3]);
          ht_bargraph.clearLedNow(i_bargraph[24]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[1]);
          ht_bargraph.clearLedNow(i_bargraph[26]);

          i_bargraph_status_alt--;
        }
      break;

      case 12:
        ht_bargraph.setLedNow(i_bargraph[1]);
        ht_bargraph.setLedNow(i_bargraph[26]);

        if(b_bargraph_up) {
          ht_bargraph.clearLedNow(i_bargraph[2]);
          ht_bargraph.clearLedNow(i_bargraph[25]);

          i_bargraph_status_alt++;
        }
        else {
          ht_bargraph.clearLedNow(i_bargraph[0]);
          ht_bargraph.clearLedNow(i_bargraph[27]);

          i_bargraph_status_alt--;
        }
      break;

      case 13:
        ht_bargraph.setLedNow(i_bargraph[0]);
        ht_bargraph.setLedNow(i_bargraph[27]);

        ht_bargraph.clearLedNow(i_bargraph[1]);
        ht_bargraph.clearLedNow(i_bargraph[26]);

        i_bargraph_status_alt--;

        b_bargraph_up = false;
      break;
    }
  }

  int i_ramp_interval = d_bargraph_ramp_interval;

  if(b_28segment_bargraph) {
    i_ramp_interval = d_bargraph_ramp_interval_alt;
  }

  // If in a power mode on the wand that can overheat, change the speed of the bargraph ramp during firing based on time remaining before we overheat.
  if(i_speed_multiplier > 1) {
    if(i_speed_multiplier > 5) {
      if(b_28segment_bargraph) {
        ms_bargraph_firing.start(i_ramp_interval / i_ramp_interval);
      }
    }
    else if(i_speed_multiplier > 4) {
      if(b_28segment_bargraph) {
        ms_bargraph_firing.start(i_ramp_interval / 9);
      }
    }
    else if(i_speed_multiplier > 3) {
      if(b_28segment_bargraph) {
        ms_bargraph_firing.start(i_ramp_interval / 7);
      }
    }
    else if(i_speed_multiplier > 2) {
      if(b_28segment_bargraph) {
        ms_bargraph_firing.start(i_ramp_interval / 5);
      }
    }
    else if(i_speed_multiplier > 1) {
      if(b_28segment_bargraph) {
        ms_bargraph_firing.start(i_ramp_interval / 3);
      }
    }
    else {
      if(b_28segment_bargraph) {
        switch(POWER_LEVEL) {
          case LEVEL_5:
            ms_bargraph_firing.start((i_ramp_interval / 2) - 7); // 13
          break;

          case LEVEL_4:
            ms_bargraph_firing.start((i_ramp_interval / 2) - 3); // 15
          break;

          case LEVEL_3:
            ms_bargraph_firing.start(i_ramp_interval / 2); // 20
          break;

          case LEVEL_2:
            ms_bargraph_firing.start((i_ramp_interval / 2) + 7); // 30
          break;

          case LEVEL_1:
            ms_bargraph_firing.start((i_ramp_interval / 2) + 12); // 35
          break;
        }
      }
    }
  }
  else {
    if(b_28segment_bargraph) {
      switch(POWER_LEVEL) {
        case LEVEL_5:
          ms_bargraph_firing.start((i_ramp_interval / 2) - 7); // 13
        break;

        case LEVEL_4:
          ms_bargraph_firing.start((i_ramp_interval / 2) - 3); // 15
        break;

        case LEVEL_3:
          ms_bargraph_firing.start(i_ramp_interval / 2); // 20
        break;

        case LEVEL_2:
          ms_bargraph_firing.start((i_ramp_interval / 2) + 7); // 25
        break;

        case LEVEL_1:
          ms_bargraph_firing.start((i_ramp_interval / 2) + 12); // 30
        break;
      }
    }
  }
}

void bargraphPowerCheck() {
  // Control for the 28 segment barmeter bargraph.
  if(b_28segment_bargraph) {
    if(ms_bargraph_alt.justFinished()) {
      uint8_t i_bargraph_multiplier[5] = { 7, 6, 5, 4, 3 };

      if(YEAR_MODE == 2021) {
        for(uint8_t i = 0; i <= 4; i++) {
          i_bargraph_multiplier[i] = 10;
        }
      }

      if(b_bargraph_up) {
        ht_bargraph.setLedNow(i_bargraph[i_bargraph_status_alt]);

        switch(POWER_LEVEL) {
          case LEVEL_5:
            if(i_bargraph_status_alt > 26) {
              b_bargraph_up = false;

              i_bargraph_status_alt = 27;

              if(YEAR_MODE == 2021) {
                // In 2021 mode, we stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                // A little pause when we reach the top.
                ms_bargraph_alt.start(i_bargraph_wait / 2);
              }
            }
            else {
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[4]);
            }
          break;

          case LEVEL_4:
            if(i_bargraph_status_alt > 21) {
              b_bargraph_up = false;

              if(YEAR_MODE == 2021) {
                // In 2021 mode, we stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                // A little pause when we reach the top.
                ms_bargraph_alt.start(i_bargraph_wait / 2);
              }
            }
            else {
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[3]);
            }
          break;

          case LEVEL_3:
            if(i_bargraph_status_alt > 16) {
              b_bargraph_up = false;
              if(YEAR_MODE == 2021) {
                // In 2021 mode, we stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                // A little pause when we reach the top.
                ms_bargraph_alt.start(i_bargraph_wait / 2);
              }
            }
            else {
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[2]);
            }
          break;

          case LEVEL_2:
            if(i_bargraph_status_alt > 10) {
              b_bargraph_up = false;
              if(YEAR_MODE == 2021) {
                // In 2021 mode, we stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                // A little pause when we reach the top.
                ms_bargraph_alt.start(i_bargraph_wait / 2);
              }
            }
            else {
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[1]);
            }
          break;

          case LEVEL_1:
            if(i_bargraph_status_alt > 4) {
              b_bargraph_up = false;
              if(YEAR_MODE == 2021) {
                // In 2021 mode, we stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                // A little pause when we reach the top.
                ms_bargraph_alt.start(i_bargraph_wait / 2);
              }
            }
            else {
              ms_bargraph_alt.start(i_bargraph_interval * i_bargraph_multiplier[0]);
            }
          break;
        }

        if(b_bargraph_up) {
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

          switch(POWER_LEVEL) {
            case LEVEL_5:
              if(YEAR_MODE == 2021 && i_bargraph_status_alt < 27) {
                // In 2021 mode, we stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                ms_bargraph_alt.start(i_bargraph_interval * 3);
              }
            break;

            case LEVEL_4:
              if(YEAR_MODE == 2021 && i_bargraph_status_alt < 22) {
                // In 2021 mode, we stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                ms_bargraph_alt.start(i_bargraph_interval * 4);
              }
            break;

            case LEVEL_3:
              if(YEAR_MODE == 2021 && i_bargraph_status_alt < 17) {
                // In 2021 mode, we stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                ms_bargraph_alt.start(i_bargraph_interval * 5);
              }
            break;

            case LEVEL_2:
              if(YEAR_MODE == 2021 && i_bargraph_status_alt < 11) {
                // In 2021 mode, we stop when we reach our target.
                ms_bargraph_alt.stop();
              }
              else {
                ms_bargraph_alt.start(i_bargraph_interval * 6);
              }
            break;

            case LEVEL_1:
              if(YEAR_MODE == 2021 && i_bargraph_status_alt < 5) {
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
}

void bargraphClearAlt() {
  if(b_28segment_bargraph) {
    ht_bargraph.clearAll();

    i_bargraph_status_alt = 0;
  }
}

void prepBargraphRampDown() {
  // If bargraph is set to ramp down during overheat, we need to set a few things.
  // Reset some bargraph levels before we ramp the bargraph down.
  i_bargraph_status_alt = 28; // For 28 segment bargraph

  i_bargraph_status = 5; // For Hasbro 5 LED bargraph.

  bargraphFull();

  ms_bargraph.start(d_bargraph_ramp_interval);

  // Prepare to make the bargraph ramp down now.
  bargraphRampUp();
}

void prepBargraphRampUp() {
  bargraphClearAlt();

  ms_settings_blinking.stop();

  bool b_overheat_bargraph_blink = false;

  // Prepare a few things before ramping the bargraph back up from a full ramp down.
  if(!b_overheat_bargraph_blink) {
    if(YEAR_MODE == 2021) {
      bargraphYearModeUpdate();
    }
    else {
      i_bargraph_multiplier_current = i_bargraph_multiplier_ramp_1984 * 2;
    }

    // If using the 28 segment bargraph, in Afterlife, we need to redraw the segments.
    // 1984/1989 years will go in to a auto ramp and do not need a manual refresh.
    if(YEAR_MODE == YEAR_2021 && b_28segment_bargraph) {
      bargraphPowerCheck2021Alt(false);
    }

    bargraphRampUp();
  }
}

void settingsBlinkingLights() {
  if(ms_settings_blinking.justFinished()) {
     ms_settings_blinking.start(i_settings_blinking_delay);
  }

  if(ms_settings_blinking.remaining() < i_settings_blinking_delay / 2) {
    bool b_solid_five = false;
    bool b_solid_one = false;

    if(b_28segment_bargraph) {
      if(b_solid_five) {
        for(uint8_t i = 0; i < 16; i++) {
          if(b_solid_one && i < 2) {
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
      else if(b_solid_one) {
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
  }
  else {
    if(b_28segment_bargraph) {
      // 18 for the 5 level menu system.
      uint8_t i_leds = 18;

      if(b_overheating || b_pack_alarm) {
        // All the segments.
        i_leds = 28;
      }

      // NOTE: If you draw all 28 segments at once often, you can overflow the serial buffer after around 5 seconds.
      for(uint8_t i = 0; i < i_leds; i++) {
        if(b_overheating || b_pack_alarm) {
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
    /*
    switch(i_wand_menu) {
      case 5:
        if(b_28segment_bargraph) {
          // 18 for the 5 level menu system.
          uint8_t i_leds = 18;

          if(b_overheating || b_pack_alarm) {
            // All the segments.
            i_leds = 28;
          }

          // NOTE: If you draw all 28 segments at once often, you can overflow the serial buffer after around 5 seconds.
          for(uint8_t i = 0; i < i_leds; i++) {
            if(b_overheating || b_pack_alarm) {
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
      break;

      case 4:
        if(b_28segment_bargraph) {
          for(uint8_t i = 0; i < 14; i++) {
            if(b_overheating || b_pack_alarm) {
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
      break;

      case 3:
        if(b_28segment_bargraph) {
          for(uint8_t i = 0; i < 10; i++) {
            if(b_overheating || b_pack_alarm) {
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
      break;

      case 2:
        if(b_28segment_bargraph) {
          for(uint8_t i = 0; i < 6; i++) {
            if(b_overheating || b_pack_alarm) {
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
      break;

      case 1:
        if(b_28segment_bargraph) {
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
      break;
    }
    */
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
  
  if(b_28segment_bargraph) {
    ht_bargraph.begin(0x00);

    bargraphYearModeUpdate();
  }
}