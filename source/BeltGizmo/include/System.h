/**
 *   GPStar BeltGizmo - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2024-2026 Dustin Grau <dustin.grau@gmail.com>
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

// Clear any prior information from the WebSocket client.
void resetWebSocketData() {
  wsData.mode = "";
  wsData.theme = "";
  wsData.switchState = "";
  wsData.pack = "";
  wsData.safety = "";
  wsData.wandPower = 5; // Default to max power.
  wsData.wandMode = "";
  wsData.firing = "";
  wsData.cable = "";
  wsData.cyclotron = "";
  wsData.temperature = "";
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

void ledsOff() {
  // Change all possible addressable LEDs to black.
  fill_solid(device_leds, DEVICE_MAX_LEDS, CRGB::Black);
}

// Helper: compute fixed-point scaled position (Q16) from 16-bit phase.
// Outputs:
//  - index: integer LED index (0..i_num_leds-1)
//  - frac:  fractional 0..255 part used for weighting adjacent LEDs
static inline void calculateScaledPos(uint16_t i_phase, uint8_t &index, uint8_t &frac) {
  uint32_t scaledPos = (uint32_t)i_phase * (uint32_t)i_num_leds; // Q16
  index = (uint8_t)(scaledPos >> 16);               // integer LED index
  frac  = (uint8_t)((scaledPos >> 8) & 0xFF);       // fractional 0..255
}

// Function to update the current colour based on stream mode.
void updateStreamColor() {
  switch(gpstarSystem.getStreamMode()) {
    case PROTON:
      i_stream_colour = C_RED;
    break;
    case SLIME:
      i_stream_colour = C_GREEN;
    break;
    case STASIS:
      i_stream_colour = C_BLUE;
    break;
    case MESON:
      i_stream_colour = C_ORANGE;
    break;
    case SPECTRAL:
      i_stream_colour = C_RAINBOW;
    break;
    case HOLIDAY_HALLOWEEN:
      i_stream_colour = C_ORANGEPURPLE;
    break;
    case HOLIDAY_CHRISTMAS:
      i_stream_colour = C_REDGREEN;
    break;
    case SELFTEST:
      // Initialize timer on first entry to self-test mode
      if(!ms_selftest_cycle.isRunning()) {
        ms_selftest_cycle.start(i_selftest_interval);
        i_selftest_colour = 0; // Reset to first colour
      }

      // Cycle through all available colours every 2 seconds during self-test
      if(ms_selftest_cycle.justFinished()) {
        sendDebug(String(F("Self-Test: Switching to Color #")) + String(i_selftest_colour));

        // Set current colour based on count of colours available (red, green, blue).
        switch(i_selftest_colour % i_colour_count) {
          case 0: i_stream_colour = C_WHITE; break;
          case 1: i_stream_colour = C_RED; break;
          case 2: i_stream_colour = C_GREEN; break;
          case 3: i_stream_colour = C_BLUE; break;
        }

        // Advance to next colour for the next cycle
        i_selftest_colour = (i_selftest_colour + 1) % i_colour_count;

        // Restart timer for next cycle.
        ms_selftest_cycle.restart();
      }
    break;
    default:
      i_stream_colour = C_WHITE;
    break;
  }
}

// Animate the LEDs in a wave-like pattern using a single colour.
void animateLights() {
  static uint16_t i_led_phase = 0; // 16-bit phase accumulator (high byte = 0..255 visible phase)

  if(!ms_anim_change.justFinished()) return; // nothing to do this frame

  ledsOff(); // Clear LEDs before updating animation.

  // Compute a full-bright CRGB once and scale per-LED with nscale8_video.
  CRGB baseColor;
  switch(LED_COLOR_TYPE) {
    case LED_GBR:
    default:
      baseColor = getHueAsGBR(PRIMARY_LED, i_stream_colour, 255);
    break;
    case LED_RGB:
      baseColor = getHueAsRGB(PRIMARY_LED, i_stream_colour, 255);
    break;
    case LED_GRB:
      baseColor = getHueAsGRB(PRIMARY_LED, i_stream_colour, 255);
    break;
  }

  // Compute fixed-point position and split between two LEDs for smooth sub-pixel motion.
  uint8_t i_index, i_frac;
  calculateScaledPos(i_led_phase, i_index, i_frac);

  // Split peak between i_index and the next LED
  uint8_t i_weightA = 255 - i_frac; // Weight for i_index
  uint8_t i_weightB = i_frac;       // Weight for i_index + 1

  // Apply weighted colours (nscale8_video expects 0..255)
  CRGB cA = baseColor; cA.nscale8_video(i_weightA);
  device_leds[i_index] = cA;

  uint8_t i_indexB = i_index + 1;
  if(i_indexB >= i_num_leds) {
    i_indexB = 0;
  }
  CRGB cB = baseColor; cB.nscale8_video(i_weightB);
  device_leds[i_indexB] = cB;

  // Slight blur to soften stepping (tune i_blurAmount)
  const uint8_t i_blur_amount = 32;
  blur1d(device_leds, i_num_leds, i_blur_amount);

  // IMPORTANT: advance the 16-bit phase so the visible (high) byte moves by i_animation_step.
  // Shift left 8 so visible phase increments by i_animation_step each frame.
  if(b_invert_animation) {
    i_led_phase -= (uint16_t)i_animation_step << 8;
  } else {
    i_led_phase += (uint16_t)i_animation_step << 8;
  }

  if(b_firing) {
    // Speed up animation when firing, based on power level.
    ms_anim_change.start(int(i_animation_duration / ((wsData.wandPower + 1) * 2)));
  } else {
    // Otherwise return to normal speed at idle.
    ms_anim_change.start(i_animation_duration);
  }
}
