/**
 *   gpstar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Dustin Grau <dustin.grau@gmail.com>
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
 *
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
 * - Power Ramp: Uses the power level to ramp up/down to an equivalent position on the display
 *               The initial pattern will transition to a special ramp-up or ramp-down pattern
 *
 * States
 * - Off: Denotes bargraph is not in use and should not be animated
 * - On: Denotes bargraph may be mid-pattern, but state is unknown
 * - Empty: Denotes bargraph was last seen as completely empty (dark)
 * - Mid: Denotes bargraph pattern reached the middle of the display
 * - Full: Denotes bargraph was last seen as completely full (lit)
 */
enum BARGRAPH_PATTERNS { BG_RAMP_UP, BG_RAMP_DOWN, BG_OUTER_INNER, BG_INNER_PULSE, BG_POWER_RAMP, BG_POWER_DOWN, BG_POWER_UP };
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
    BARGRAPH_PATTERN = BG_POWER_UP;
  }
  else if(BARGRAPH_STATE == BG_FULL) {
    // When known full, ramp down.
    BARGRAPH_PATTERN = BG_POWER_DOWN;
  }

  // Ensure bargraph stops at the correct element based on a given power level.
  // Account for uneven division by using the remainder as the base for level 1.
  uint8_t i_bargraph_base = (i_bargraph_elements % i_bargraph_levels);
  // Remember, the passed level will be 0-based so we must add 1 for calculations.
  i_bargraph_sim_max = i_bargraph_base + (i_bargraph_levels * (i_level + 1));
}

// Performs update on bargraph elements based given a pattern.
void bargraphUpdate(uint8_t i_delay_divisor) {
  if(i_delay_divisor == 0) {
    i_delay_divisor = 1; // Avoid divide by zero.
  }

  if(BARGRAPH_PATTERN == BG_POWER_RAMP ||
     BARGRAPH_PATTERN == BG_POWER_DOWN ||
     BARGRAPH_PATTERN == BG_POWER_UP) {
    if(BARGRAPH_PATTERN == BG_POWER_RAMP){
      // Set the initial direction for the power ramp (up).
      BARGRAPH_PATTERN = BG_POWER_UP;
    }

    // Use the current power level to set some global variables, such as the simulated maximum elements.
    // This will determine whether to ramp up or down, and must be called prior to the switch statement below.
    bargraphPowerCheck(POWER_LEVEL);
  }

  // Set the current delay by dividing the base delay by some value (Min: 2).
  uint8_t i_current_delay = max(2, int(i_bargraph_delay / i_delay_divisor));

  // Adjust the delay based on the number of total elements to be illuminated.
  // Applies primarily to the BG_POWER_LEVEL pattern for levels 1-4.
  i_current_delay = i_current_delay + (i_bargraph_elements - i_bargraph_sim_max);

  // If bargraph is not in an OFF state and timer is off/finished, perform an update of element(s).
  if(BARGRAPH_STATE != BG_OFF && ms_bargraph.remaining() == 0) {

    // Animations should be based on a set pattern and logic here must only affect the bargraph device.
    switch(BARGRAPH_PATTERN) {
      case BG_RAMP_UP:
      case BG_POWER_UP:
        // Turn on only the current element.
        bargraphSetElement(i_bargraph_element, 1);

        // Increment to the next element.
        i_bargraph_element++;

        if((BARGRAPH_PATTERN == BG_POWER_UP && i_bargraph_element > i_bargraph_sim_max) ||
           (BARGRAPH_PATTERN == BG_RAMP_UP && i_bargraph_element > i_bargraph_elements)) {
          // Note that the bargraph is full;
          BARGRAPH_STATE = BG_FULL;

          if(BARGRAPH_PATTERN == BG_POWER_UP) {
            // Set an extra delay at end of sequence, before the ramp-down.
            ms_bargraph.start(i_current_delay * 3);
          }
          else {
            // If not in the power ramp pattern, bargraph is done.
            BARGRAPH_STATE = BG_OFF;
          }
        }
        else {
          // Reset timer for next iteration, increasing the delay as elements are lit (easing out).
          ms_bargraph.start(i_current_delay + int(i_bargraph_element / 2));
        }
      break;

      case BG_RAMP_DOWN:
      case BG_POWER_DOWN:
        // Turn off only the current element.
        bargraphSetElement(i_bargraph_element, 0);

        // Decrement to the next element.
        i_bargraph_element--;

        if(i_bargraph_element < 0) {
          // Make sure bargraph is cleared;
          bargraphClear();

          if(BARGRAPH_PATTERN == BG_POWER_DOWN) {
            // Set an extra delay at end of sequence, before ramp-up.
            ms_bargraph.start(i_current_delay * 3);
          }
          else {
            // If not in the power ramp pattern, bargraph is done.
            BARGRAPH_STATE = BG_OFF;
          }
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

        // Reset timer for next iteration, with slight delay as the steps increase.
        ms_bargraph.start(i_current_delay + i_bargraph_step);
      break;
    }
  }
}