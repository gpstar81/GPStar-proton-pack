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

/*
 * Mini Cyclotron
 * Alternates between a pair of LEDs in the jewel, fading in by some number of steps per update of the timer.
 */
millisDelay ms_cyclotron;
const uint8_t i_cyclotron_leds[i_num_cyclotron_leds] = {0, 1, 2, 3, 4, 5, 6}; // Note: 0 is the dead center of the jewel
const uint8_t i_cyclotron_max_steps = 12; // Set a reusable constant for the maximum number of steps to cycle through
// Sequence: 1, 4, 2, 5, 3, 6, 4, 1, 5, 2, 6, 3
const uint8_t i_cyclotron_pair[i_cyclotron_max_steps][2] = {
  {1, 3}, // 1:in, 3:out,
  {1, 4}, // 1:out, 4:in,
  {2, 4}, // 2:in, 4:out,
  {2, 5}, // 2:out, 5:in,
  {3, 5}, // 3:in, 5:out,
  {3, 6}, // 3:out, 6:in,
  {4, 6}, // 4:in, 6:out,
  {4, 1}, // 4:out, 1:in,
  {5, 1}, // 5:in, 1:out,
  {5, 2}, // 5:out, 2:in,
  {6, 2}, // 6:in, 2:out,
  {6, 3}  // 6:out, 3:in,
};
const uint16_t i_base_cyclotron_delay = 30; // Set delay between LED updates at normal speed, at the lowest power level
const uint16_t i_min_cyclotron_delay = 10;  // Set the minimum (fastest) transition time desired for a cyclotron update
const uint8_t i_cyc_fade_step = 15; // Step size for each fade-in increment (must be a divisor of 255: 3, 5, 15, 17, 51, 85)
const uint8_t i_cyclotron_min_brightness = 0;   // Minimum brightness for each LED (use fade step for changes)
const uint8_t i_cyclotron_max_brightness = 255; // Maximum brightness for each LED (use fade step for changes)
uint16_t i_dynamic_cyclotron_delay; // Calculated cyclotron spin delay based on power level.

uint16_t getCyclotronDelay() {
  return (uint16_t) (i_base_cyclotron_delay - (POWER_LEVEL * (i_base_cyclotron_delay - i_min_cyclotron_delay) / 4));
}

// Manage lights in pairs to move in a predefined sequence, fading each light in and out.
void updateCyclotron(uint8_t i_colour) {
  static bool sb_toggle = true; // Static toggle to remain scoped to this function between calls
  static uint8_t sb_pairing = 0; // Which pair of LEDs to use for each "cycle" of fade in/out actions
  static uint8_t si_brightness_in = i_cyclotron_min_brightness; // Static brightness variable for fade-in effect
  static uint8_t si_brightness_out = i_cyclotron_max_brightness; // Static brightness variable for fade-out effect

  if(ms_cyclotron.justFinished()) {
    // Change the timing (delay) based on the power level selected.
    ms_cyclotron.start(getCyclotronDelay());

    // Increment brightness for fade-in effect
    if(si_brightness_in < i_cyclotron_max_brightness - 1) {
      si_brightness_in += i_cyc_fade_step;

      if(si_brightness_in > i_cyclotron_max_brightness - 1) {
        si_brightness_in = i_cyclotron_max_brightness;
      }
    }

    // Decrement brightness for fade-out effect
    if(si_brightness_out > i_cyclotron_min_brightness + 1) {
      si_brightness_out -= i_cyc_fade_step;

      if(si_brightness_out < i_cyclotron_min_brightness + 1) {
        si_brightness_out = i_cyclotron_min_brightness;
      }
    }

    // Toggle between the LEDs in the i_cyclotron_pair using the given color.
    if(sb_toggle) {
      system_leds[i_cyclotron_pair[sb_pairing][0]] = getHueAsRGB(i_colour).nscale8(si_brightness_in);  // Fade in LED 1 in the pair
      system_leds[i_cyclotron_pair[sb_pairing][1]] = getHueAsRGB(i_colour).nscale8(si_brightness_out); // Fade out LED 2 in the pair
    }
    else {
      system_leds[i_cyclotron_pair[sb_pairing][0]] = getHueAsRGB(i_colour).nscale8(si_brightness_out); // Fade out LED 1 in the pair
      system_leds[i_cyclotron_pair[sb_pairing][1]] = getHueAsRGB(i_colour).nscale8(si_brightness_in);  // Fade in LED 2 in the pair
    }

    // Toggle state and reset brightness variables after fade-in is complete.
    if (si_brightness_in == i_cyclotron_max_brightness && si_brightness_out == i_cyclotron_min_brightness) {
      sb_toggle = !sb_toggle;
      si_brightness_in = i_cyclotron_min_brightness;
      si_brightness_out = i_cyclotron_max_brightness;
      sb_pairing = (sb_pairing + 1) % i_cyclotron_max_steps; // Change to next pair on each toggle.
    }
  }
}