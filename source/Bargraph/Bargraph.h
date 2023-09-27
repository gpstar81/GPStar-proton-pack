/**
 * Library for testing bargraph animations.
 * Patterns should not rely on hard-set values for position, but rather be relative to the min/max elements available.
 */

/*
 * Bargraph Patterns and States
 *
 * Patterns
 * - Ramp Up: Turns on all elements, from bottom to top
 * - Ramp Down: Starts full and turns off top to bottom
 * - Outer-Inner: Standard firing sequence, with a single element moving from the top/bottom to middle then back again
 * - Inner Pulse: Pulses elements from the middle of the bargraph outward to the top/bottom and back inward again
 *
 * States
 * - Off: Denotes bargraph is not in use and should not be animated
 * - On: Denotes bargraph may be mid-pattern, but state is unknown
 * - Empty: Denotes bargraph was last seen as completely empty (dark)
 * - Mid: Denotes bargraph pattern reached the middle of the display
 * - Full: Denotes bargraph was last seen as completely full (lit)
 */
enum BARGRAPH_PATTERNS { BG_RAMP_UP, BG_RAMP_DOWN, BG_OUTER_INNER, BG_INNER_PULSE };
enum BARGRAPH_PATTERNS BARGRAPH_PATTERN;
enum BARGRAPH_STATES { BG_OFF, BG_ON, BG_EMPTY, BG_MID, BG_FULL };
enum BARGRAPH_STATES BARGRAPH_STATE;

/***** Helper Functions *****/

void bargraphSetElement(int i_element, bool b_power) {
  if(i_element < 0) {
    i_element = 0; // Keep byte value in usable range. 
  }
  else if(i_element >= i_bargraph_elements) {
    // Do not exceed the total addressable elements.
    i_element = i_bargraph_elements - 1;
  }

  if(b_bargraph_present) {
    // This simplifies the process of turning individual elements on or off.
    // Uses the mapping information which accounts for installation orientation.
    if(b_power) {
      ht_bargraph.setLedNow(i_bargraph[i_element]);
    }
    else {
      ht_bargraph.clearLedNow(i_bargraph[i_element]);
    }
  }
}

void bargraphReset() {
  // Sets the bargraph into a state where it can begin running.
  i_bargraph_element = 0;
  i_bargraph_step = 0;
  BARGRAPH_STATE = BG_ON;
  ms_bargraph.stop();
}

void bargraphFull() {
  // Illuminates all elements on the bargraph, marks state as full.
  if(b_bargraph_present) {
    for(uint8_t i = 0; i < i_bargraph_elements; i++) {
      bargraphSetElement(i, 1);
    }
  }
  i_bargraph_element = i_bargraph_elements - 1;
  BARGRAPH_STATE = BG_FULL; // Mark last known state.
}

void bargraphClear() {
  // Clears all elements from the bargraph, marks state as empty.
  if(b_bargraph_present) {
    ht_bargraph.clearAll();
  }
  i_bargraph_element = 0;
  BARGRAPH_STATE = BG_EMPTY; // Mark last known state.
}

void bargraphOff() {
  // Turns off the bargraph and prevents any animations.
  bargraphClear(); // Only clears elements, marks as empty.
  bargraphReset(); // Only clears timers, resets variables.
  BARGRAPH_STATE = BG_OFF;
}

/***** Core Setup - Declared after helper functions *****/

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

  bargraphOff(); // Turn off the bargraph.
}

/***** Animation Controls *****/

void bargraphPowerCheck(uint8_t i_level) {
  // Alternates between ramping up and down.
  if(BARGRAPH_STATE == BG_EMPTY) {
    // When known empty, ramp up.
    BARGRAPH_PATTERN = BG_RAMP_UP;
  }
  else if(BARGRAPH_STATE == BG_FULL) {
    // When known full, ramp down.
    BARGRAPH_PATTERN = BG_RAMP_DOWN;
  }
  else if(BARGRAPH_STATE != BG_OFF) {
    bargraphClear();
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

  // Adjust the delay based on the simulated max vs. actual max.
  i_current_delay = i_current_delay + (i_bargraph_elements - i_bargraph_sim_max);

  // If bargraph is not in an OFF state and timer is off/finished, perform an update of element(s).
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