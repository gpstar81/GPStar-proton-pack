/**
 *   GPStar Single-Shot Blaster
 *   Copyright (C) 2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

#pragma once

/**
 * Purpose: Control patterns displayed by the 28-segment bargraph device (mostly) independent of external factors.
 *
 * Patterns should not rely on hard-set values for position, but rather be relative to the min/max elements available.
 * Control of elements should be checked on each main loop of the program, but only updated at designated intervals as
 * set using a timer. This allows elements to linger in an on or off state for more than a single cycle, and offers
 * a chance to update timing values for the next iteration/change as necessary. Ideally, just setting a pattern name
 * should change behavior, though in some cases it may be necessary to clear, reset, or illuminate all elements prior
 * to the next change of the selected pattern. When possible, that should be done in the main program, not as part of
 * the pattern itself (though in some cases this is unavoidable or distinctly necessary).
 */

/*
 * Bargraph Patterns and States
 *
 * Patterns
 * - Ramp Up: Turns on all elements, from bottom to top
 * - Ramp Down: Starts full and turns off top to bottom
 * - Outer-Inner: Moves a pair of single elements starting from the top/bottom to middle then back outward again
 * - Inner Pulse: Pulses elements from the middle of the bargraph outward to the top/bottom and back inward again
 * - Power Ramp: Uses the power level to ramp up/down to an equivalent position on the display (as levels 1-5)
 *               The initial pattern name will trigger transition to special ramp-up or ramp-down patterns
 *
 * States
 * - Off: Denotes bargraph is not in use and should not be animated
 * - On: Denotes bargraph may be mid-pattern, but state is unknown
 * - Empty: Denotes bargraph was last seen as completely empty (dark)
 * - Mid: Denotes bargraph pattern reached the middle of the display
 * - Full: Denotes bargraph was last seen as completely full (lit)
 */
enum BARGRAPH_PATTERNS { BG_NONE, BG_RAMP_UP, BG_RAMP_DOWN, BG_OUTER_INNER, BG_INNER_PULSE, BG_POWER_RAMP, BG_POWER_DOWN, BG_POWER_UP };
enum BARGRAPH_STATES { BG_OFF, BG_ON, BG_EMPTY, BG_MID, BG_FULL, BG_BARS };

/*
 * Barmeter 28 segment bargraph configuration and timers.
 * Part #: BL28Z-3005SA04Y
 * This will use the following pins for i2c serial communication:
 * Arduino Nano
 *   SDA -> A4
 *   SCL -> A5
 * ESP32
 *   SDA -> GPIO 21
 *   SCL -> GPIO 22
 */
struct Bargraph {
  const static uint8_t Elements = 28; // Maximum elements for bargraph device; not likely to change but adjustable just in case.
  const static uint8_t MaxLevels = 5; // Reflects the count of POWER_LEVELS elements (the only dependency on other device behavior).
  const static uint8_t UpdateDelay = 8; // Base delay (ms) for bargraph refresh (this should be a value evenly divisible by 2, 3, or 4).

  private:
    const uint8_t OrientationNormal[Bargraph::Elements] = {0, 16, 32, 48, 1, 17, 33, 49, 2, 18, 34, 50, 3, 19, 35, 51, 4, 20, 36, 52, 5, 21, 37, 53, 6, 22, 38, 54};
    const uint8_t OrientationInvert[Bargraph::Elements] = {54, 38, 22, 6, 53, 37, 21, 5, 52, 36, 20, 4, 51, 35, 19, 3, 50, 34, 18, 2, 49, 33, 17, 1, 48, 32, 16, 0};
    const uint8_t Bar_1[Bargraph::Elements] = {1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    const uint8_t Bar_2[Bargraph::Elements] = {1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    const uint8_t Bar_3[Bargraph::Elements] = {1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    const uint8_t Bar_4[Bargraph::Elements] = {1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
    const uint8_t Bar_5[Bargraph::Elements] = {1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0};

  public:
    HT16K33 device; // Singular bargraph object instance using the HT16K33 matrix driver.
    uint8_t simulate = Bargraph::Elements; // Simulated maximum for patterns which may be dependent on other factors.
    uint8_t steps = Bargraph::Elements / 2; // Steps for patterns (1/2 max) which are bilateral/mirrored.
    uint8_t step = 0; // Indicates current step for bilateral/mirrored patterns.
    int element = 0; // Indicates current LED element for adjustment.
    bool inverted = false; // Whether the order of the device elements should be considered inverted.
    bool present = false; // Denotes that i2c bus found the bargraph device.
    millisDelay ms_bargraph; // Timer to control bargraph updates consistently.

    enum BARGRAPH_PATTERNS PATTERN = BG_NONE;
    enum BARGRAPH_STATES STATE = BG_OFF;

    void initialize(bool b_invert = false) {
      byte by_error, by_address;
      uint8_t i_i2c_devices = 0;

      // Scan i2c for any devices (28 segment bargraph).
      for(by_address = 1; by_address < 127; by_address++ ) {
        Wire.beginTransmission(by_address);
        by_error = Wire.endTransmission();

        if(by_error == 0) {
          // Device found at address.
          i_i2c_devices++;
        }
      }

      if(i_i2c_devices > 0) {
        present = true;
      }
      else {
        present = false;
      }

      if(present) {
        device.begin(0x00);
      }

      inverted = (b_invert == true);
    }

    uint8_t mapElement() {
      // Returns the true element on the device, as based on the mapped orientation.
      if(inverted) {
        return OrientationInvert[element];
      }
      else {
        return OrientationNormal[element];
      }
    }

    void showBars(uint8_t i_count) {
      off(); // Clear previous state and display.

      PATTERN = BG_NONE;
      STATE = BG_BARS;

      // Display evenly spaced indicators for values 1-5.
      bool b_power = 0;
      for(uint8_t i = 0; i < Bargraph::Elements; i++) {
        switch(i_count) {
          case 1:
            b_power = Bar_1[i];
          break;
          case 2:
            b_power = Bar_2[i];
          break;
          case 3:
            b_power = Bar_3[i];
          break;
          case 4:
            b_power = Bar_4[i];
          break;
          case 5:
            b_power = Bar_5[i];
          break;
        }

        setElement(i, b_power);
      }

      commit(); // Immediately change state.
    }

    void setElement(int8_t i_element, bool b_power) {
      // This updates the element on the bargraph but does not immediately alter to the bargraph itself.
      if(i_element < 0) {
         // Keep byte value in usable range.
         element = 0;
      }
      else if(i_element > Bargraph::Elements - 1) {
        // Do not exceed the total addressable elements.
        element = Bargraph::Elements - 1;
      }
      else {
        // Otherwise use the element as given.
        element = i_element;
      }

      if(present) {
        // This simplifies the process of turning individual elements on or off.
        // Uses mapping information which accounts for installation orientation.
        if(b_power) {
          device.setLed(mapElement()); // Turn the mapped element on.
        }
        else {
          device.clearLed(mapElement()); // Turn the mapped element off.
        }
      }
    }

    void commit() {
      // This commits any changes created by bargraph.setElement to the bargraph.
      if(present) {
        device.sendLed();
      }
    }

    void clear() {
      // Clears all elements from the bargraph, marks state as empty.
      if(present) {
        device.clearAll();
      }
      element = 0;
      STATE = BG_EMPTY; // Mark last known state.
    }

    void reset() {
      // Sets the bargraph into a state where it can begin running.
      element = 0;
      step = 0;
      STATE = BG_ON;
      ms_bargraph.stop();
    }

    void full() {
      // Illuminates all elements on the bargraph, marks state as full.
      for(uint8_t i = 0; i < Bargraph::Elements; i++) {
        setElement(i, 1);
      }
      STATE = BG_FULL; // Mark last known state.
    }

    void off() {
      // Turns off the bargraph and prevents any animations.
      clear(); // Only clears elements, marks as empty.
      reset(); // Only clears timers, resets variables.
      STATE = BG_OFF;
    }
} bargraph;

/***** Core Setup - Declared after helper functions *****/

void setupBargraph() {
  Wire.begin();
  Wire.setClock(400000UL); // Sets the i2c bus to 400kHz

  bargraph.initialize(); // Initialize device, if available

  bargraph.off(); // Turn off the bargraph.
}

/***** Animation Controls *****/

void bargraphPowerCheck() {
  // Alternates between ramping up and down.
  if(bargraph.STATE == BG_EMPTY) {
    // When known empty, ramp up.
    bargraph.PATTERN = BG_POWER_UP;
  }
  else if(bargraph.STATE == BG_FULL) {
    // When known full, ramp down.
    bargraph.PATTERN = BG_POWER_DOWN;
  }

  // Ensure bargraph stops at the correct element based on a given power level.
  // Account for uneven division by using the remainder as the base for level 1.
  uint8_t i_bargraph_base = (Bargraph::Elements % Bargraph::MaxLevels);
  // Remember, the passed level will be 0-based so we must add 1 for calculations.
  bargraph.simulate = i_bargraph_base + (Bargraph::MaxLevels * (uint8_t)(POWER_LEVEL));
}

// Performs update on bargraph elements based given a pattern.
void bargraphUpdate(uint8_t i_delay_divisor = 1) {
  if(i_delay_divisor == 0) {
    i_delay_divisor = 1; // Avoid divide by zero.
  }

  if(bargraph.PATTERN == BG_POWER_RAMP ||
     bargraph.PATTERN == BG_POWER_DOWN ||
     bargraph.PATTERN == BG_POWER_UP) {
    // Use the current power level to set some global variables, such as the simulated maximum elements.
    // This will determine whether to ramp up or down, and must be called prior to the switch statement below.
    bargraphPowerCheck();
  }

  // Set the current delay by dividing the base delay by some value (Min: 2).
  // For most normal usage, the divisor will be 1 (thus, no change in delay).
  uint8_t i_current_delay = max(2, int(Bargraph::UpdateDelay / i_delay_divisor));

  // Adjust the delay based on the number of total elements to be illuminated.
  // Primarily affects BG_POWER_RAMP at levels 1-4 to slow the ramp animation.
  i_current_delay = i_current_delay + (Bargraph::Elements - bargraph.simulate);

  // If bargraph is not in an OFF state and timer is off/finished, perform an update of element(s).
  if(bargraph.STATE != BG_OFF && bargraph.ms_bargraph.remaining() == 0) {

    // Animations should be based on a set pattern and logic here must only affect the bargraph device.
    switch(bargraph.PATTERN) {
      case BG_NONE:
      default:
        // No-Op.
      break;

      case BG_POWER_RAMP:
        bargraph.PATTERN = BG_POWER_UP; // Set the initial direction for the power ramp (up).
      break;

      case BG_RAMP_UP:
        // This is intended to be a single action, ramping the bargraph up then stopping animations.

        // Turn on only the current element.
        bargraph.setElement(bargraph.element, 1);
        bargraph.commit();

        // Increment to the next element.
        bargraph.element++;

        if(bargraph.element >= Bargraph::Elements) {
          // Make note that the bargraph is full;
          bargraph.STATE = BG_FULL;

          // Once the ramp up is complete the bargraph should be off.
          bargraph.STATE = BG_OFF;
        }
        else {
          // Reset timer for next iteration, increasing the delay as elements are lit (easing out).
          bargraph.ms_bargraph.start(i_current_delay + int(bargraph.element / 2));
        }
      break;

      case BG_POWER_UP:
        // This is part of a continuous animation BG_POWER_RAMP which ramps up then back down.

        // Turn on only the current element.
        bargraph.setElement(bargraph.element, 1);
        bargraph.commit();

        // Increment to the next element.
        bargraph.element++;

        if(bargraph.element >= bargraph.simulate) {
          // Make note that the bargraph is full;
          bargraph.STATE = BG_FULL;

          // Set an extra delay at end of sequence, before the ramp-down.
          bargraph.ms_bargraph.start(i_current_delay * 3);
        }
        else {
          // Reset timer for next iteration, increasing the delay as elements are lit (easing out).
          bargraph.ms_bargraph.start(i_current_delay + int(bargraph.element / 2));
        }
      break;

      case BG_RAMP_DOWN:
        // This is intended to be a single action, ramping the bargraph down then stopping animations.

        // Turn off only the current element.
        bargraph.setElement(bargraph.element, 0);
        bargraph.commit();

        // Add a significant slowdown to this ramp-down.
        i_current_delay = i_current_delay * 4;

        // Decrement to the next element.
        bargraph.element--;

        if(bargraph.element < 0) {
          // Make sure bargraph is cleared;
          bargraph.clear();

          // Once the ramp up is complete the bargraph should be off.
          bargraph.STATE = BG_OFF;
        }
        else {
          // Reset timer for next iteration.
          bargraph.ms_bargraph.start(i_current_delay);
        }
      break;

      case BG_POWER_DOWN:
        // This is part of a continuous animation BG_POWER_RAMP which ramps up then back down.

        // Turn off only the current element.
        bargraph.setElement(bargraph.element, 0);
        bargraph.commit();

        if(bargraph.PATTERN == BG_RAMP_DOWN) {
          // Add a significant slowdown to a standalone ramp-down.
          i_current_delay = i_current_delay * 4;
        }

        // Decrement to the next element.
        bargraph.element--;

        if(bargraph.element < 0) {
          // Make sure bargraph is cleared;
          bargraph.clear();

          // Set an extra delay at end of sequence, before ramp-up.
          bargraph.ms_bargraph.start(i_current_delay * 3);
        }
        else {
          // Reset timer for next iteration.
          bargraph.ms_bargraph.start(i_current_delay);
        }
      break;

      case BG_INNER_PULSE:
      case BG_OUTER_INNER:
        if(bargraph.STATE != BG_EMPTY && bargraph.STATE != BG_MID) {
          // Make sure bargraph is empty before starting the pattern.
          bargraph.clear();

          // Prepare to begin on appropriate step for the pattern.
          if(bargraph.PATTERN == BG_INNER_PULSE) {
            // This pattern begins at the ends and so it must step backwards once it reaches the midpoint.
            bargraph.step = bargraph.steps - 1;
          }
          else {
            // All other patterns (read: BG_INNER_PULSE) starts and ends its steps at the midpoint.
            bargraph.step = 0;
          }
        }

        uint8_t i_element_max = Bargraph::Elements - 1;
        uint8_t i_step_mid = bargraph.steps - 1;

        // Set special values when at either end of the bargraph.
        if(bargraph.step == 0) {
          // Denote that the bargraph is now empty, meaning it likely has not yet begun or just completed.
          bargraph.STATE = BG_EMPTY;

          // Illuminate the first and last elements.
          bargraph.setElement(bargraph.step, 1);
          bargraph.setElement(i_element_max, 1);

          if(bargraph.PATTERN == BG_OUTER_INNER) {
            // Clear the next inner elements.
            bargraph.setElement(bargraph.step + 1, 0);
            bargraph.setElement(i_element_max - 1, 0);
          }

          bargraph.commit();
        }
        else if(bargraph.step == i_step_mid) {
          // Denote that we are at the midpoint step, which is technically the endpoint for these patterns.
          bargraph.STATE = BG_MID;

          // Illuminate the middle elements.
          bargraph.setElement(i_step_mid, 1);
          bargraph.setElement(i_step_mid + 1, 1);

          // Clear the next middle elements.
          bargraph.setElement(i_step_mid - 1, 0);
          bargraph.setElement(i_step_mid + 2, 0);

          bargraph.commit();
        }
        else {
          // This covers all steps between the starting point and endpoint for the patterns.

          // Illuminate elements N steps from each end of the device.
          bargraph.setElement(bargraph.step, 1);
          bargraph.setElement(i_element_max - bargraph.step, 1);

          // Clear the next outer elements at N-1 steps.
          bargraph.setElement(bargraph.step - 1, 0);
          bargraph.setElement(i_element_max - (bargraph.step - 1), 0);

          // Clear the next inner elements at N+1 steps.
          if(bargraph.PATTERN == BG_OUTER_INNER) {
            bargraph.setElement(bargraph.step + 1, 0);
            bargraph.setElement(i_element_max - (bargraph.step + 1), 0);
          }

          bargraph.commit();
        }

        if(bargraph.STATE != BG_MID) {
          // Continue the pattern until it reaches the midpoint.
          bargraph.step++;
        }
        else {
          // Reverse direction at the midpoint state.
          bargraph.step--;
        }

        // Reset timer for next iteration, with slight delay as the steps increase.
        bargraph.ms_bargraph.start(i_current_delay + bargraph.step);
      break;
    }
  }
}
