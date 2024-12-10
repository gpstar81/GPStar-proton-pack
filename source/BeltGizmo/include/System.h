/**
 *   GPStar BeltGizmo - Ghostbusters Props, Mods, and Kits.
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

// Animates the LEDs in a wave-like pattern
void animateLights() {
  static uint16_t i_led_position = 0;
  uint8_t i_color;

  // Update timer interval in case i_power changes
  if (ms_anim_change.justFinished()) {
    if(b_firing) {
      // Speed up animation only when firing.
      ms_anim_change.start(i_animation_duration / ((i_power + 1) * 2));
    }
    else {
      // Otherwise return to normal speed.
      ms_anim_change.start(i_animation_duration);
    }

    for (int i = 0; i < DEVICE_NUM_LEDS; i++) {
      uint8_t i_brightness = map(sin8((i_led_position + i * 32) % 255), 0, 255, i_min_brightness, i_max_brightness);
      
      switch(STREAM_MODE) {
        case PROTON:
          i_color = C_RED;
        break;
        case SLIME:
          i_color = C_GREEN;
        break;
        case STASIS:
          i_color = C_BLUE;
        break;
        case MESON:
          i_color = C_ORANGE;
        break;
        case SPECTRAL:
          i_color = C_RAINBOW;
        break;
        case HOLIDAY_HALLOWEEN:
          i_color = C_ORANGEPURPLE;
        break;
        case HOLIDAY_CHRISTMAS:
          i_color = C_REDGREEN;
        break;
        default:
          i_color = C_WHITE;
        break;
      }

      //device_leds[i] = getHueAsRGB(PRIMARY_LED, i_color, 255 - i_brightness);
      device_leds[i] = getHueAsGBR(PRIMARY_LED, i_color, 255 - i_brightness);
    }

    i_led_position += i_animation_step; // Move the wave position by shifting position for the next update.
  }
}
