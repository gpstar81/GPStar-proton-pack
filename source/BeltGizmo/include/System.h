/**
 *   GPStar BeltGizmo - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2024-2025 Dustin Grau <dustin.grau@gmail.com>
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

void debug(const String message) {
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

  if(iterator == nullptr) {
    Serial.println(F("No partitions found."));
    return;
  }

  Serial.println(F("Partitions:"));
  while(iterator != nullptr) {
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
  static uint8_t i_led_position = 0; // 8-bit Phase Wrap
  uint8_t i_color;

  // Time to update the animation of the LEDs
  if(ms_anim_change.justFinished()) {
    ledsOff(); // Clear LEDs before updating animation.

    // Determine the color once per animation sequence as based on the current stream mode.
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

    // Compute a full-bright CRGB once and scale per-LED with nscale8_video.
    CRGB baseColor = b_use_gbr ? getHueAsGBR(PRIMARY_LED, i_color, 255) : getHueAsRGB(PRIMARY_LED, i_color, 255);

    // Fixed-point position across DEVICE_NUM_LEDS (Q8 fractional part)
    uint16_t scaledPos = (uint16_t)i_led_position * (uint16_t)DEVICE_NUM_LEDS;
    uint8_t i_index = (uint8_t)(scaledPos >> 8); // Integer LED index
    uint8_t i_frac = (uint8_t)(scaledPos & 0xFF); // 0..255 fractional part

    // Split peak between i_index and the next LED
    uint8_t i_weightA = 255 - i_frac; // Weight for i_index
    uint8_t i_weightB = i_frac;       // Weight for i_index+1

    // Apply weights (nscale8_video expects 0..255)
    CRGB cA = baseColor; cA.nscale8_video(i_weightA);
    device_leds[i_index] = cA;

    // Wrap next index manually (avoids modulo)
    uint8_t i_indexB = i_index + 1;
    if (i_indexB >= DEVICE_NUM_LEDS) i_indexB = 0;
    CRGB cB = baseColor; cB.nscale8_video(i_weightB);
    device_leds[i_indexB] = cB;

    // Smooth the result slightly by using a blur on the hot pixel.
    const uint8_t i_blurAmount = 32;
    blur1d(device_leds, DEVICE_NUM_LEDS, i_blurAmount);

    // Move the wave position by shifting position for the next update.
    i_led_position += i_animation_step; // Implicitly wraps at 256 upon overflow

    if(b_firing) {
      // Speed up animation when firing, based on power level.
      ms_anim_change.start(i_animation_duration / ((i_power + 1) * 2));
    }
    else {
      // Otherwise return to normal speed at idle.
      ms_anim_change.start(i_animation_duration);
    }
  }
}
