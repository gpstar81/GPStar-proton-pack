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

        uint8_t i_element_max = i_bargraph_elements - 1;
        uint8_t i_step_mid = i_bargraph_steps - 1;

        // Set special values when at either end of the bargraph.
        if(i_bargraph_step == 0) {
          BARGRAPH_STATE = BG_EMPTY;

          // Illuminate the first and last elements.
          bargraphSetElement(i_bargraph_step, 1);
          bargraphSetElement(i_element_max, 1);

          if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
            // Clear the next inner elements.
            bargraphSetElement(i_bargraph_step + 1, 0);
            bargraphSetElement(i_element_max - 1, 0);
          }
        }
        else if(i_bargraph_step == i_step_mid) {
          BARGRAPH_STATE = BG_MID;

          // Illuminate the middle elements.
          bargraphSetElement(i_step_mid, 1);
          bargraphSetElement(i_step_mid + 1, 1);

          // Clear the next middle elements.
          bargraphSetElement(i_step_mid - 1, 0);
          bargraphSetElement(i_step_mid + 2, 0);
        }
        else {
          // Illuminate elements N steps from each end.
          bargraphSetElement(i_bargraph_step, 1);
          bargraphSetElement(i_element_max - i_bargraph_step, 1);

          // Clear the next outer elements from N steps.
          bargraphSetElement(i_bargraph_step - 1, 0);
          bargraphSetElement(i_element_max - (i_bargraph_step - 1), 0);

          // Clear the next inner elements from N steps.
          if(BARGRAPH_PATTERN == BG_OUTER_INNER) {
            bargraphSetElement(i_bargraph_step + 1, 0);
            bargraphSetElement(i_element_max - (i_bargraph_step + 1), 0);
          }
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