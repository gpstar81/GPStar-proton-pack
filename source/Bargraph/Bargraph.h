/**
 * Library for testing bargraph animations.
 * Patterns should not rely on hard-set values for position, but rather be relative to the min/max elements available.
 */

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
    i_element = 0; // Keep byte value in range. 
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

void bargraphPowerCheck(uint8_t i_level) {
  // Alternates between ramping up and down.
  if(BARGRAPH_STATE == BG_EMPTY) {
    BARGRAPH_PATTERN = BG_RAMP_UP;
  }
  else {
    BARGRAPH_PATTERN = BG_RAMP_DOWN;
  }

  // Ensure bargraph stops at the correct element based on a given power level.
  // Account for uneven division by using the remainder as the base for level 1.
  uint8_t i_bargraph_base = (i_bargraph_elements % i_bargraph_levels);
  // Remember, the passed level will be 0-based so we must add 1 for calculations.
  i_bargraph_sim_max = i_bargraph_base + (i_bargraph_levels * (i_level + 1));
}

void bargraphUpdate(uint8_t i_delay_divisor) {
  if(i_delay_divisor == 0) {
    i_delay_divisor = 1; // Avoid divide by zero.
  }
  // Set the current delay by dividing the base delay by some value.
  uint8_t i_current_delay = int(i_bargraph_delay / i_delay_divisor);

  // Perform an update of element(s) when timer has completed,
  // only if bargraph was not explicitly set to an OFF state.
  if(BARGRAPH_STATE != BG_OFF && ms_bargraph.remaining() == 0) {
    // Animations should be based on a set pattern and logic here must only affect the bargraph device.
    switch(BARGRAPH_PATTERN) {
      case BG_RAMP_UP:
        if(BARGRAPH_STATE != BG_EMPTY) {
          // Make sure bargraph is empty before ramp up.
          bargraphClear();
        }

        // Turn on only the current element.
        bargraphSetElement(i_bargraph_element, 1);

        // Increment to the next element.
        i_bargraph_element++;

        if(i_bargraph_element >= i_bargraph_sim_max) {
          // Note that the bargraph is full;
          BARGRAPH_STATE = BG_FULL;

          // Set an extra delay at end of sequence.
          ms_bargraph.start(i_current_delay * 2);
        }
        else {
          // Reset timer for next iteration.
          ms_bargraph.start(i_current_delay);
        }
      break;

      case BG_RAMP_DOWN:
        if(BARGRAPH_STATE != BG_FULL) {
          // Make sure bargraph is full before ramp down.
          bargraphFull();
        }

        // Turn off only the current element.
        bargraphSetElement(i_bargraph_element, 0);

        // Decrement to the next element.
        i_bargraph_element--;

        if(i_bargraph_element <= 0) {
          // Make sure bargraph is cleared;
          bargraphClear();

          // Set an extra delay at end of sequence.
          ms_bargraph.start(i_current_delay * 2);
        }
        else {
          // Reset timer for next iteration.
          ms_bargraph.start(i_current_delay);
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

        uint8_t i_element_max = i_bargraph_sim_max - 1;
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

          // Clear the next outer elements at N-1 steps.
          bargraphSetElement(i_bargraph_step - 1, 0);
          bargraphSetElement(i_element_max - (i_bargraph_step - 1), 0);

          // Clear the next inner elements at N+1 steps.
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

        // Reset timer for next iteration.
        ms_bargraph.start(i_current_delay);
      break;
    }
  }
}