void bargraphReset() {
  i_bargraph_element = 0;
  i_bargraph_step = 0;
  BARGRAPH_STATE = BG_UNKNOWN;
  ms_bargraph.stop();
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
    b_bargraph_present = true;
  }
  else {
    b_bargraph_present = false;
  }

  if(b_bargraph_present) {
    ht_bargraph.begin(0x00);
  }

  bargraphReset();
}

void bargraphSetElement(int i_element, bool b_power) {
  if(i_element < 0) {
    i_element = 0; // Keep value in range. 
  }
  if(i_element >= i_bargraph_elements) {
    // Do not exceed the addressable elements.
    i_element = i_bargraph_elements - 1;
  }

  if(b_bargraph_present) {
    if(b_power) {
      ht_bargraph.setLedNow(i_bargraph[i_element]);
    }
    else {
      ht_bargraph.clearLedNow(i_bargraph[i_element]);
    }
  }
}

void bargraphFull() {
  if(b_bargraph_present) {
    for(uint8_t i = 0; i < i_bargraph_elements; i++) {
      bargraphSetElement(i, 1);
    }
    BARGRAPH_STATE = BG_FULL;
  }
}

void bargraphClear() {
  if(b_bargraph_present) {
    ht_bargraph.clearAll();
  }
  BARGRAPH_STATE = BG_EMPTY;
}

void bargraphPowerCheck() {
  // Ramps the bargraph up and down.
  if(BARGRAPH_STATE == BG_EMPTY) {
    BARGRAPH_PATTERN = BG_RAMP_UP;
  }
  else {
    BARGRAPH_PATTERN = BG_RAMP_DOWN;
  }
}

void bargraphUpdate() {
  // Perform an update of element(s) when timer has completed.
  if(ms_bargraph.remaining() == 0) {
    switch(BARGRAPH_PATTERN) {
      case BG_RAMP_UP:
        if(BARGRAPH_STATE != BG_EMPTY) {
          // Make sure bargraph is empty before ramp up.
          bargraphClear();
        }

        // Update element only when timer is finished.
        bargraphSetElement(i_bargraph_element, 1);

        // Increment to the next element.
        i_bargraph_element++;

        if(i_bargraph_element >= i_bargraph_elements) {
          // Note that the bargraph is full;
          BARGRAPH_STATE = BG_FULL;

          // Set an extra delay at end of sequence.
          ms_bargraph.start(i_bargraph_delay * 2);
        }
        else {
          // Reset timer for next iteration.
          ms_bargraph.start(i_bargraph_delay);
        }
      break;

      case BG_RAMP_DOWN:
        if(BARGRAPH_STATE != BG_FULL) {
          // Make sure bargraph is full before ramp down.
          bargraphFull();
        }

        // Update element only when timer is finished.
        bargraphSetElement(i_bargraph_element, 0);

        // Decrement to the next element.
        i_bargraph_element--;

        if(i_bargraph_element <= 0) {
          // Make sure bargraph is cleared;
          bargraphClear();

          // Set an extra delay at end of sequence.
          ms_bargraph.start(i_bargraph_delay * 2);
        }
        else {
          // Reset timer for next iteration.
          ms_bargraph.start(i_bargraph_delay);
        }
      break;

      case BG_INNER_PULSE:
      case BG_OUTER_INNER:
        if(BARGRAPH_STATE != BG_EMPTY && BARGRAPH_STATE != BG_MID) {
          // Make sure bargraph is empty before starting the pattern.
          bargraphClear();

          // Prepare to begin on appropriate step.
          if(BARGRAPH_PATTERN == BG_INNER_PULSE) {
            i_bargraph_step = i_bargraph_steps - 1;
          }
          else {
            i_bargraph_step = 0;
          }
        }

        // Set special values when at either end of the bargraph.
        if(i_bargraph_step == (i_bargraph_steps - 1)) {
          BARGRAPH_STATE = BG_MID;
        }
        else if(i_bargraph_step == 0) {
          BARGRAPH_STATE = BG_EMPTY;
        }

        switch(i_bargraph_step) {
          case 0:
            bargraphSetElement(0, 1);
            bargraphSetElement(27, 1);

            if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
              bargraphSetElement(1, 0);
              bargraphSetElement(26, 0);
            }
          break;

          case 1:
            bargraphSetElement(1, 1);
            bargraphSetElement(26, 1);

            bargraphSetElement(0, 0);
            bargraphSetElement(27, 0);

            if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
              bargraphSetElement(2, 0);
              bargraphSetElement(25, 0);
            }
          break;

          case 2:
            bargraphSetElement(2, 1);
            bargraphSetElement(25, 1);

            bargraphSetElement(1, 0);
            bargraphSetElement(26, 0);

            if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
              bargraphSetElement(3, 0);
              bargraphSetElement(24, 0);
            }
          break;

          case 3:
            bargraphSetElement(3, 1);
            bargraphSetElement(24, 1);

            bargraphSetElement(2, 0);
            bargraphSetElement(25, 0);

            if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
              bargraphSetElement(4, 0);
              bargraphSetElement(23, 0);
            }
          break;

          case 4:
            bargraphSetElement(4, 1);
            bargraphSetElement(23, 1);

            bargraphSetElement(3, 0);
            bargraphSetElement(24, 0);

            if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
              bargraphSetElement(5, 0);
              bargraphSetElement(22, 0);
            }
          break;

          case 5:
            bargraphSetElement(5, 1);
            bargraphSetElement(22, 1);

            bargraphSetElement(4, 0);
            bargraphSetElement(23, 0);

            if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
              bargraphSetElement(6, 0);
              bargraphSetElement(21, 0);
            }
          break;

          case 6:
            bargraphSetElement(6, 1);
            bargraphSetElement(21, 1);

            bargraphSetElement(5, 0);
            bargraphSetElement(22, 0);

            if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
              bargraphSetElement(7, 0);
              bargraphSetElement(20, 0);
            }
          break;

          case 7:
            bargraphSetElement(7, 1);
            bargraphSetElement(20, 1);

            bargraphSetElement(6, 0);
            bargraphSetElement(21, 0);

            if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
              bargraphSetElement(8, 0);
              bargraphSetElement(19, 0);
            }
          break;

          case 8:
            bargraphSetElement(8, 1);
            bargraphSetElement(19, 1);

            bargraphSetElement(7, 0);
            bargraphSetElement(20, 0);

            if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
              bargraphSetElement(9, 0);
              bargraphSetElement(18, 0);
            }
          break;

          case 9:
            bargraphSetElement(9, 1);
            bargraphSetElement(18, 1);

            bargraphSetElement(8, 0);
            bargraphSetElement(19, 0);

            if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
              bargraphSetElement(10, 0);
              bargraphSetElement(17, 0);
            }
          break;

          case 10:
            bargraphSetElement(10, 1);
            bargraphSetElement(17, 1);

            bargraphSetElement(9, 0);
            bargraphSetElement(18, 0);

            if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
              bargraphSetElement(11, 0);
              bargraphSetElement(16, 0);
            }
          break;

          case 11:
            bargraphSetElement(11, 1);
            bargraphSetElement(16, 1);

            bargraphSetElement(10, 0);
            bargraphSetElement(17, 0);

            if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
              bargraphSetElement(12, 0);
              bargraphSetElement(15, 0);
            }
          break;

          case 12:
            bargraphSetElement(12, 1);
            bargraphSetElement(15, 1);

            bargraphSetElement(11, 0);
            bargraphSetElement(16, 0);

            if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
              bargraphSetElement(13, 0);
              bargraphSetElement(14, 0);
            }
          break;

          case 13:
            bargraphSetElement(13, 1);
            bargraphSetElement(14, 1);

            bargraphSetElement(12, 0);
            bargraphSetElement(15, 0);
          break;
        }

        if(BARGRAPH_STATE != BG_MID) {
          // Continue pattern until reaching midpoint.
          i_bargraph_step++;
        }
        else {
          // Reverse direction at midpoint state.
          i_bargraph_step--;
        }

        ms_bargraph.start(i_bargraph_delay);
      break;
    }
  }
}