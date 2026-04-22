/**
 *   GPStar Stream Effects - Ghostbusters Props, Mods, and Kits.
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

void sendDebug(const String& message);

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

void initializePalettes() {
  paletteProton = CRGBPalette16(
    CRGB::Red, CRGB::Red, CRGB::Maroon, CRGB::Maroon,
    CRGB::Orange, CRGB::Red, CRGB::Red, CRGB::Black,
    CRGB::Red, CRGB::Red, CRGB::Maroon, CRGB::Maroon,
    CRGB::Orange, CRGB::Red, CRGB::Red, CRGB::Black
  );

  paletteSlime = CRGBPalette16(
    CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green,
    CRGB::LimeGreen, CRGB::LimeGreen, CRGB::Black, CRGB::Black,
    CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green,
    CRGB::LimeGreen, CRGB::LimeGreen, CRGB::Black, CRGB::Black
  );

  paletteStasis = CRGBPalette16(
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
    CRGB::Indigo, CRGB::Indigo, CRGB::Black, CRGB::Black,
    CRGB::Blue, CRGB::Blue, CRGB::Blue, CRGB::Blue,
    CRGB::Indigo, CRGB::Indigo, CRGB::Black, CRGB::Black
  );

  paletteMeson = CRGBPalette16(
    CRGB::Yellow, CRGB::Yellow, CRGB::Orange, CRGB::Orange,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
    CRGB::Yellow, CRGB::Yellow, CRGB::Orange, CRGB::Orange,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black
  );

  paletteSpectral = CRGBPalette16(
    CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green,
    CRGB::Blue, CRGB::Indigo, CRGB::Violet, CRGB::Black,
    CRGB::Red, CRGB::Orange, CRGB::Yellow, CRGB::Green,
    CRGB::Blue, CRGB::Indigo, CRGB::Violet, CRGB::Black
  );

  paletteHalloween = CRGBPalette16(
    CRGB::Orange, CRGB::Orange, CRGB::Orange, CRGB::Orange,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
    CRGB::Purple, CRGB::Purple, CRGB::Purple, CRGB::Purple,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black
  );

  paletteChristmas = CRGBPalette16(
    CRGB::Red, CRGB::Red, CRGB::Red, CRGB::Red,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
    CRGB::Green, CRGB::Green, CRGB::Green, CRGB::Green,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black
  );

  paletteBrass = CRGBPalette16(
    CRGB::Chartreuse , CRGB::Chartreuse , CRGB::Chartreuse , CRGB::Chartreuse ,
    CRGB::Orange, CRGB::Orange, CRGB::Black, CRGB::Black,
    CRGB::Chartreuse , CRGB::Chartreuse , CRGB::Chartreuse , CRGB::Chartreuse ,
    CRGB::Orange, CRGB::Orange, CRGB::Black, CRGB::Black
  );

  paletteWhite = CRGBPalette16(
    CRGB::GhostWhite, CRGB::GhostWhite, CRGB::Gainsboro, CRGB::Gainsboro,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black,
    CRGB::GhostWhite, CRGB::GhostWhite, CRGB::Gainsboro, CRGB::Gainsboro,
    CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black
  );
}

// Function to update the current palette based on stream mode.
void updateStreamPalette() {
  switch(gpstarSystem.getStreamMode()) {
    case PROTON:
      if(gpstarSystem.getSystemTheme() == SYSTEM_FROZEN_EMPIRE && !wsData.cyclotronLid && wsData.ctsActive) {
        cp_StreamPalette = paletteBrass;
      }
      else {
        cp_StreamPalette = paletteProton;
      }
    break;
    case SLIME:
      cp_StreamPalette = paletteSlime;
    break;
    case STASIS:
      cp_StreamPalette = paletteStasis;
    break;
    case MESON:
      cp_StreamPalette = paletteMeson;
    break;
    case SPECTRAL:
      cp_StreamPalette = paletteSpectral;
    break;
    case HOLIDAY_HALLOWEEN:
      cp_StreamPalette = paletteHalloween;
    break;
    case HOLIDAY_CHRISTMAS:
      cp_StreamPalette = paletteChristmas;
    break;
    case SELFTEST:
      // Initialize timer on first entry to self-test mode
      if(!ms_selftest_cycle.isRunning()) {
        ms_selftest_cycle.start(i_selftest_interval);
        i_selftest_palette = 0; // Reset to first palette
      }

      // Cycle through all available palettes every 2 seconds during self-test
      if(ms_selftest_cycle.justFinished()) {
        sendDebug(String(F("Self-Test: Switching to Palette #")) + String(i_selftest_palette) + String(F(" w/ Power Level ")) + String(wsData.wandPower));

        // Set current palette based on count of palettes available
        switch(i_selftest_palette % i_palette_count) {
          case 0: cp_StreamPalette = paletteWhite; break;
          case 1: cp_StreamPalette = paletteProton; break;
          case 2: cp_StreamPalette = paletteSlime; break;
          case 3: cp_StreamPalette = paletteStasis; break;
          case 4: cp_StreamPalette = paletteMeson; break;
          case 5: cp_StreamPalette = paletteSpectral; break;
          case 6: cp_StreamPalette = paletteHalloween; break;
          case 7: cp_StreamPalette = paletteChristmas; break;
          case 8: cp_StreamPalette = paletteBrass; break;
        }

        // Advance to next palette for the next cycle
        i_selftest_palette = (i_selftest_palette + 1) % i_palette_count;

        // Restart timer for next cycle.
        ms_selftest_cycle.restart();
      }
    break;
    default:
      cp_StreamPalette = paletteWhite;
    break;
  }
}

// Animate the LEDs using FastLED's built-in palette system for smooth colour transitions.
void animateLights() {
  static uint8_t i_palette_start_index = 0; // Starting index for palette distribution across LEDs.

  // Use FastLED's fill_palette function for automatic colour distribution and blending
  // Parameters: LED array, number of LEDs, starting palette index, delta between LEDs, palette, brightness, blending mode.
  fill_palette(device_leds, i_num_leds, i_palette_start_index, 255 / i_num_leds, cp_StreamPalette, 255, LINEARBLEND);

  // Handle LED ordering ordering as necessary.
  switch(LED_COLOR_TYPE) {
    case LED_RGB:
    default:
      // No-op
    break;
    case LED_GRB:
      for(uint16_t i = 0; i < i_num_leds; i++) {
        CRGB b_temp_colour = device_leds[i];
        device_leds[i] = CRGB(b_temp_colour.g, b_temp_colour.r, b_temp_colour.b);
      }
    break;
    case LED_GBR:
      for(uint16_t i = 0; i < i_num_leds; i++) {
        CRGB b_temp_colour = device_leds[i];
        device_leds[i] = CRGB(b_temp_colour.g, b_temp_colour.b, b_temp_colour.r);
      }
    break;
  }

  // Increment starting index to create flowing animation effect using the wand power level.
  i_palette_start_index += wsData.wandPower;
}
