/**
 *   GPStar Stream Effects - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2024 Dustin Grau <dustin.grau@gmail.com>
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

void ledsOff() {
  fill_solid(device_leds, DEVICE_NUM_LEDS, CRGB::Black);
}

void animateLights() {
  static uint8_t paletteIndex = 0; // Tracks the current base color index in the palette
  static uint16_t wavePosition = 0; // Tracks the position of the wave
  
  if (ms_anim_change.justFinished()) {
    ms_anim_change.start(i_animation_time);

    // Iterate through all LEDs
    for (uint16_t i = 0; i < DEVICE_NUM_LEDS; i++) {
      // Calculate a brightness factor based on the wave position
      uint8_t brightness = sin8((wavePosition + i * 20) % 255);

      // Use the brightness to modulate the color from the palette
      CRGB color = ColorFromPalette(cp_StreamPalette, paletteIndex + i * 5);
      device_leds[i] = color;
      device_leds[i].nscale8(brightness); // Scale the color by brightness
    }

    // Increment the palette index and wave position for the next frame
    paletteIndex += (i_animation_step / 2) * i_power; // Adjust this step size for smoother or faster transitions
    wavePosition += i_animation_step * i_power; // Adjust this step size to control wave speed
  }
}
