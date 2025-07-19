/**
 *   GPStar Ghost Trap - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Nomake Wan <-redacted->
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

void debug(String message) {
  // Writes a debug message to the serial console.
  #if defined(DEBUG_SEND_TO_CONSOLE)
    Serial.println(message); // Print to serial console.
  #endif
  #if defined(DEBUG_SEND_TO_WEBSOCKET)
    ws.textAll(message); // Send a copy to the WebSocket.
  #endif
}

// Obtain a list of partitions for this device.
void printPartitions() {
  const esp_partition_t *partition;
  esp_partition_iterator_t iterator = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);

  if (iterator == nullptr) {
    Serial.println(F("No partitions found."));
    return;
  }

  Serial.println(F("Partitions:"));
  while (iterator != nullptr) {
    partition = esp_partition_get(iterator);
    Serial.printf("Label: %s, Size: %lu bytes, Address: 0x%08lx\n",
                  partition->label,
                  partition->size,
                  partition->address);
    iterator = esp_partition_next(iterator);
  }

  esp_partition_iterator_release(iterator);  // Release the iterator once done
}

void updateTopLEDs() {
  static uint8_t i_random = 0;

  // First black out all pixels to make the changes more obvious.
  fill_solid(top_leds, NUM_TOP_PIXELS, CRGB::Black);

  // Set the top RGB LEDs to some color, randomizing with flashes of white.
  for (int i = 0; i < NUM_TOP_PIXELS; i++) {
    i_random = random(0, 2);
    switch (i_random) {
      case 0:
        top_leds[i] = getHueAsGRB(i, C_GREEN);
      break;
      case 1:
        top_leds[i] = getHueAsGRB(i, C_WHITE);
      break;
    }
  }
}
/*
 * Determine the current state of any LEDs before next FastLED refresh.
 */
void updateLEDs() {  // Static variable to use for choice of LED color.
  if(b_ap_started && b_ws_started) {
    #if defined(USE_ESP32_S3)
      // Set the built-in LED to green to indicate the device is fully ready.
      device_leds[0] = getHueAsRGB(0, C_GREEN, 128);
    #else
      // Turn on the LED to indicate the device is fully ready.
      digitalWrite(BUILT_IN_LED, HIGH);
    #endif
  } else {
    #if defined(USE_ESP32_S3)
      // Set the built-in LED to red while the WiFi and WebSocket are not ready.
      device_leds[0] = getHueAsRGB(0, C_RED, 128);
    #else
      // Keep the LED off to indicate the device is not ready.
      digitalWrite(BUILT_IN_LED, LOW);
    #endif
  }

  if (ms_light.isRunning()) {
    if (digitalRead(TOP_2WHITE) == LOW) {
      // While the timer is active, keep the top 2 white LEDs lit.
      debug(F("LED On"));
      digitalWrite(TOP_2WHITE, HIGH); // Set to HIGH (on)
      ms_top_leds.start(i_top_leds_delay); // Start the delay for top LEDs.
    }

    if (ms_top_leds.justFinished()) {
      ms_top_leds.repeat(); // Restart the delay
      updateTopLEDs(); // Call the function to alter LEDs
    }
  }

  if (ms_light.justFinished()) {
    debug(F("LED Off"));
    digitalWrite(TOP_2WHITE, LOW); // Set to LOW (off)

    // Turn off the LEDs (set to black) using FastLED.
    fill_solid(top_leds, NUM_TOP_PIXELS, CRGB::Black);
  }
}

/*
 * Determine the current state of the blower.
 */
void checkBlower() {
  if (ms_blower.isRunning() && digitalRead(BLOWER_PIN) == LOW) {
    // If timer is active but power is not applied, turn on the device AFTER the delay period has elapsed.
    if ((millis() - ms_blower.getStartTime()) >= i_blower_start_delay) {
      debug(F("Blower On"));
      digitalWrite(BLOWER_PIN, HIGH); // Set to HIGH (on)
    }
  }

  if (ms_blower.justFinished()) {
    debug(F("Blower Off"));
    digitalWrite(BLOWER_PIN, LOW); // Set to LOW (off)
  }
}

/*
 * Determine the current state of the smoke device.
 */
void checkSmoke() {
  if (ms_smoke.isRunning() && digitalRead(SMOKE_PIN) == LOW) {
    // If timer is active but power is not applied, turn on the device immediately.
    debug(F("Smoke On"));
    digitalWrite(SMOKE_PIN, HIGH); // Set to HIGH (on)
  }

  if (ms_smoke.justFinished()) {
    debug(F("Smoke Off"));
    digitalWrite(SMOKE_PIN, LOW); // Set to LOW (off)
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
 * Returns true when doors are closed.
 */
bool doorsClosed() {
  return (digitalRead(DOOR_CLOSED_PIN) == 1 && digitalRead(DOOR_OPENED_PIN) == 0);
}

/*
 * Returns true when doors are opened.
 */
bool doorsOpened() {
  return (digitalRead(DOOR_CLOSED_PIN) == 0 && digitalRead(DOOR_OPENED_PIN) == 1);
}

/*
 * Monitor for interactions by user input.
 */
void checkDoors() {
  // Determine whether the trap doors are currently opened or closed.
  if (doorsClosed()) {
    DOOR_STATE = DOORS_CLOSED;
  }
  if (doorsOpened()) {
    DOOR_STATE = DOORS_OPENED;
  }
}

/*
 * Stop a running smoke sequence.
 */
void stopSmoke() {
  // Stop any existing timers before proceeding.
  ms_blower.stop();
  ms_light.stop();
  ms_smoke.stop();

  // Shut down any running devices.
  digitalWrite(BLOWER_PIN, LOW);
  digitalWrite(SMOKE_PIN, LOW);
  digitalWrite(TOP_2WHITE, LOW);

  // Turn off the LEDs (set to black) using FastLED.
  fill_solid(top_leds, NUM_TOP_PIXELS, CRGB::Black);
}

/*
 * Execute a smoke sequence for a given duration.
 */
void startSmoke(uint16_t i_duration) {
  if (!ms_smoke.isRunning()) {
    // Check lower/upper limits for duration.
    if (i_duration < i_smoke_duration_min) {
      i_duration = i_smoke_duration_min;
    }
    if (i_duration > i_smoke_duration_max) {
      i_duration = i_smoke_duration_max;
    }

    // If enabled, begin setting timers for the various devices (LED, blower, and smoke).
    if (b_smoke_enabled && i_duration >= i_smoke_duration_min && i_duration <= i_smoke_duration_max) {
      ms_blower.start(i_duration * 2); // Run the blower twice as long as the smoke duration.
      ms_light.start(i_duration * 1.5); // Keep the LED lit only 1.5x the smoke duration.
      ms_smoke.start(i_duration); // Only run smoke for as long as the system will allow.
    }
  }
}
