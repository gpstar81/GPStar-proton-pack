/**
 *   GPStar Ghost Trap - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Nomake Wan <nomake_wan@yahoo.co.jp>
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

// Writes a debug message to the serial console or sends to the WebSocket.
void sendDebug(const String message) {
  #if defined(DEBUG_SEND_TO_CONSOLE)
    debugln(message); // Print to serial console.
  #endif
  #if defined(DEBUG_SEND_TO_WEBSOCKET)
    ws.textAll(message); // Send a copy to the WebSocket.
  #endif
}

// Obtain a list of partitions for this device.
void printPartitions() {
  const esp_partition_t *partition;
  esp_partition_iterator_t iterator = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

  if(iterator == nullptr) {
    debugln(F("No partitions found."));
    return;
  }

  debugln(F("Partitions:"));
  while(iterator != nullptr) {
    partition = esp_partition_get(iterator);
    debugf("Label: %s, Size: %lu bytes, Address: 0x%08lx\n",
           partition->label,
           partition->size,
           partition->address);
    iterator = esp_partition_next(iterator);
  }

  esp_partition_iterator_release(iterator);  // Release the iterator once done
}

/*
 * Determine the current state of any LEDs before next FastLED refresh.
 */
void updateLEDs() {  // Static variable to use for choice of LED color.
  if(ms_light.isRunning()) {
    if(ms_top_leds.justFinished()) {
      ms_top_leds.repeat(); // Restart the delay
    }
  }

  if(ms_light.justFinished()) {
    sendDebug(F("LED Off"));
    ms_light.repeat();
  }
}

/*
 * Determine the current state of the blower.
 */
void checkBlower() {
  if(ms_blower.isRunning()) {
    // If timer is active but power is not applied, turn on the device AFTER the delay period has elapsed.
    if((millis() - ms_blower.getStartTime()) >= i_blower_start_delay) {
      sendDebug(F("Blower On"));
    }
  }

  if(ms_blower.justFinished()) {
    sendDebug(F("Blower Off"));
  }
}

/*
 * Determine the current state of the smoke device.
 */
void checkSmoke() {
  if(ms_smoke.isRunning()) {
    // If timer is active but power is not applied, turn on the device immediately.
    sendDebug(F("Smoke On"));
  }

  if(ms_smoke.justFinished()) {
    sendDebug(F("Smoke Off"));
  }
}

/*
 * Perform debounce and get current button/switch states.
 *
 * Required by the ezButton objects.
 */
void switchLoops() {

}

/*
 * Monitor for interactions by user input.
 */
void checkDoors() {
  // Determine whether the trap doors are currently opened or closed.

}

/*
 * Stop a running smoke sequence.
 */
void stopSmoke() {
  // Stop any existing timers before proceeding.
  ms_blower.stop();
  ms_light.stop();
  ms_smoke.stop();
}

/*
 * Execute a smoke sequence for a given duration.
 */
void startSmoke(uint16_t i_duration) {
  if(!ms_smoke.isRunning()) {
    // Check lower/upper limits for duration.
    if(i_duration < i_smoke_duration_min) {
      i_duration = i_smoke_duration_min;
    }
    if(i_duration > i_smoke_duration_max) {
      i_duration = i_smoke_duration_max;
    }

    // If enabled, begin setting timers for the various devices (LED, blower, and smoke).
    if(b_smoke_enabled && i_duration >= i_smoke_duration_min && i_duration <= i_smoke_duration_max) {
      ms_blower.start(i_duration * 2); // Run the blower twice as long as the smoke duration.
      ms_light.start(i_duration * 1.5); // Keep the LED lit only 1.5x the smoke duration.
      ms_smoke.start(i_duration); // Only run smoke for as long as the system will allow.
    }
  }
}
