/**
 *   GPStar Single-Shot Blaster
 *   Copyright (C) 2024-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
 * Dedicated library for managing delayed execution of functions with optional repeating behavior.
 * This allows execution of a passed function after a specified delay, with the option to repeat
 * the execution at regular intervals. Useful for timing-based actions without blocking.
 */

 // Function pointer type for delayed execution callbacks (no parameters, no return value).
typedef void (*DelayedCallback)();

// Structure for managing delayed execution with optional repeating behavior
struct DelayedExecution {
  millisDelay ms_timer; // Timer to track initial delay (and/or interval)
  DelayedCallback callback = nullptr; // Function to call when timer expires
  uint16_t i_interval_ms = 0; // Store the original interval for repeating
  bool b_active = false; // Flag to indicate if this timer is active
  bool b_repeating = false; // Flag to indicate if this should repeat
};

// Maximum number of concurrent delayed executions (limits resource usage).
const uint8_t MAX_DELAYED_EXECUTIONS = 8;

// Array to hold multiple delayed executions up to the maximum.
DelayedExecution delayed_executions[MAX_DELAYED_EXECUTIONS] = {};

// Function: executeDelayed
// Purpose: Execute a callback function after a specified delay, optionally repeating at intervals
// Inputs:
//   - DelayedCallback callback: Function to execute when timer expires
//   - uint16_t i_delay_ms: Delay in milliseconds before first execution
//   - bool b_repeat: If true, continues executing at intervals; if false, runs once only
// Outputs:
//   - uint8_t: Unique ID (index) for this delayed execution (0 = failed to create)
uint8_t executeDelayed(DelayedCallback callback, uint16_t i_delay_ms, bool b_repeat = false) {
  // Find an available slot
  for(uint8_t i = 0; i < MAX_DELAYED_EXECUTIONS; i++) {
    if(!delayed_executions[i].b_active) {
      // Initialize the delayed execution
      delayed_executions[i].callback = callback;
      delayed_executions[i].i_interval_ms = i_delay_ms;
      delayed_executions[i].b_active = true;
      delayed_executions[i].b_repeating = b_repeat;
      delayed_executions[i].ms_timer.start(i_delay_ms);
      
      // Return array index + 1 (so 0 can indicate failure)
      return i + 1;
    }
  }
  
  // No available slots
  return 0;
}

// Function: stopDelayedExecution
// Purpose: Stops a specific delayed execution timer and clears its active state
// Inputs:
//   - uint8_t i_timer_id: Unique ID (index) returned by executeDelayed()
// Outputs:
//   - bool: True if timer was found and stopped, false if ID not found
bool stopDelayedExecution(uint8_t i_timer_index) {
  if(i_timer_index == 0 || i_timer_index > MAX_DELAYED_EXECUTIONS) {
    return false; // Invalid index
  }
  
  // Convert back to array index (subtract 1)
  uint8_t i_array_index = i_timer_index - 1;
  
  if(delayed_executions[i_array_index].b_active) {
    delayed_executions[i_array_index].b_active = false;
    delayed_executions[i_array_index].ms_timer.stop();
    return true;
  }
  
  return false; // Timer not active
}

// Function: stopAllDelayedExecutions
// Purpose: Stops all currently running delayed execution timers
void stopAllDelayedExecutions() {
  for(uint8_t i = 0; i < MAX_DELAYED_EXECUTIONS; i++) {
    if(delayed_executions[i].b_active) {
      delayed_executions[i].b_active = false;
      delayed_executions[i].ms_timer.stop();
    }
  }
}

// Function: checkDelayedExecutions
// Purpose: Monitors all delayed execution timers and handles both one-time and repeating callbacks
// Note: Add this to your main loop or checkGeneralTimers()
void checkDelayedExecutions() {
  for(uint8_t i = 0; i < MAX_DELAYED_EXECUTIONS; i++) {
    if(delayed_executions[i].b_active && delayed_executions[i].ms_timer.justFinished()) {
      // Execute the callback function
      if(delayed_executions[i].callback != nullptr) {
        delayed_executions[i].callback();
      }
      
      // If this is a repeating timer, restart it; otherwise deactivate
      if(delayed_executions[i].b_repeating) {
        delayed_executions[i].ms_timer.start(delayed_executions[i].i_interval_ms);
      } 
      else {
        delayed_executions[i].b_active = false;
      }
    }
  }
}