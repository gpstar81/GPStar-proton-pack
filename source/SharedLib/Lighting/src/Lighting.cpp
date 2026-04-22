/**
 *   Lighting - LED color abstraction utilities for GPStar devices.
 *   Provides LED-library-independent color types and conversions.
 *   Copyright (C) 2023-2026 Michael Rajotte, Dustin Grau, Nomake Wan
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

// Library Header
#include <Lighting.h>

// Static member initialization for dynamic color state
uint8_t Lighting::s_dynamicHue[MAX_DYNAMIC_COLOR_DEVICES] = {0};
uint8_t Lighting::s_dynamicBright[MAX_DYNAMIC_COLOR_DEVICES] = {0};
int16_t Lighting::s_dynamicNextBright[MAX_DYNAMIC_COLOR_DEVICES] = {-1, -1, -1, -1, -1, -1};
uint8_t Lighting::s_dynamicCounter[MAX_DYNAMIC_COLOR_DEVICES] = {1, 1, 1, 1, 1, 1};

// Reset all dynamic color state
void Lighting::resetDynamicColors() {
  for(uint8_t i = 0; i < MAX_DYNAMIC_COLOR_DEVICES; i++) {
    s_dynamicHue[i] = 0;
    s_dynamicBright[i] = 0;
    s_dynamicNextBright[i] = -1;
    s_dynamicCounter[i] = 1;
  }
}

// Get HSV color values for standard colors.
LED_HSV Lighting::getColorHSV(SingleColor color, uint8_t brightness, uint8_t saturation) {
  // Returns LED_HSV with appropriate hue, saturation, and brightness.
  // Some colors override saturation or brightness with fixed values.

  switch(color) {
    case C_WHITE:
    default:
      return {100, 0, brightness}; // White = no saturation

    case C_BLACK:
      return {0, 0, 0}; // Black = all zeros (overrides brightness)

    case C_WARM_WHITE:
      return {36, 183, brightness};

    case C_PINK:
      return {244, saturation, brightness};

    case C_PASTEL_PINK:
      return {244, 128, brightness}; // Fixed saturation

    case C_RED:
      return {0, saturation, brightness};

    case C_LIGHT_RED:
      return {0, 192, brightness}; // Fixed saturation

    case C_RED2:
      return {5, saturation, brightness};

    case C_RED3:
      return {10, saturation, brightness};

    case C_RED4:
      return {15, saturation, brightness};

    case C_RED5:
      return {20, saturation, brightness};

    case C_ORANGE:
      return {32, saturation, brightness};

    case C_BEIGE:
      return {43, 128, brightness}; // Fixed saturation

    case C_YELLOW:
      return {64, saturation, brightness};

    case C_CHARTREUSE:
      return {80, saturation, brightness};

    case C_GREEN:
      return {96, saturation, brightness};

    case C_DARK_GREEN:
      return {96, saturation, 128}; // Fixed brightness

    case C_MINT:
      return {112, 120, brightness}; // Fixed saturation

    case C_AQUA:
      return {128, saturation, brightness};

    case C_LIGHT_BLUE:
      return {145, saturation, brightness};

    case C_MID_BLUE:
      return {160, saturation, brightness};

    case C_NAVY_BLUE:
      return {170, 200, 112}; // Fixed saturation and brightness

    case C_BLUE:
      return {180, saturation, brightness};

    case C_PURPLE:
      return {192, saturation, brightness};
  }
}

// Convert brightness percentage (0-100) to byte value (0-255).
uint8_t Lighting::getBrightness(uint8_t percent) {
  // Brightness as percentage (0-100), converted to range 0-255
  if(percent > 100) {
    percent = 100;
  }
  return (uint8_t)((255 * percent) / 100);
}

// Scale RGB color by brightness factor (0-255).
LED_RGB Lighting::scaleBrightness(const LED_RGB &color, uint8_t brightness) {
  // Scale each channel by brightness factor (0-255)
  LED_RGB result;
  result.r = (uint8_t)((color.r * brightness) / 255);
  result.g = (uint8_t)((color.g * brightness) / 255);
  result.b = (uint8_t)((color.b * brightness) / 255);
  return result;
}

// Color Channel Ordering

// Apply color channel ordering for different LED strip types.
LED_RGB Lighting::applyColorOrder(const LED_RGB &color, ColorOrder order) {
  switch(order) {
    case ORDER_GRB:
      // Swap red and green channels
      return {color.g, color.r, color.b};

    case ORDER_GBR:
      // Rotate channels: G->R, B->G, R->B
      return {color.g, color.b, color.r};

    case ORDER_RGB:
    default:
      return color;
  }
}

// Convert HSV color to RGB color using a standard rainbow algorithm.
LED_RGB Lighting::hsv2rgb(const LED_HSV &hsv) {
  LED_RGB rgb;

  // If saturation is 0, the color is a shade of gray
  if(hsv.s == 0) {
    rgb.r = hsv.v;
    rgb.g = hsv.v;
    rgb.b = hsv.v;
    return rgb;
  }

  // Divide hue into 6 regions (0-5) for the color wheel
  // Each region is 256/6 ≈ 43 hue units
  uint8_t region = hsv.h / 43;
  uint8_t remainder = (hsv.h - (region * 43)) * 6;

  // Calculate intermediate values for smooth transitions
  uint8_t p = (hsv.v * (255 - hsv.s)) >> 8;
  uint8_t q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
  uint8_t t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

  // Map region to RGB values for rainbow effect
  switch(region) {
    case 0:
      rgb.r = hsv.v;
      rgb.g = t;
      rgb.b = p;
    break;
    case 1:
      rgb.r = q;
      rgb.g = hsv.v;
      rgb.b = p;
    break;
    case 2:
      rgb.r = p;
      rgb.g = hsv.v;
      rgb.b = t;
    break;
    case 3:
      rgb.r = p;
      rgb.g = q;
      rgb.b = hsv.v;
    break;
    case 4:
      rgb.r = t;
      rgb.g = p;
      rgb.b = hsv.v;
    break;
    case 5:
    default:
      rgb.r = hsv.v;
      rgb.g = p;
      rgb.b = q;
    break;
  }

  return rgb;
}

// Get HSV color values for dynamic/animated colors
//
// This function uses frame counting instead of real-time delays (avoiding timers).
// Each call to this function = 1 frame. The s_dynamicCounter[] increments every call.
// When counter reaches the cycle value (counter % cycle == 0), the animation advances.
// Frames will be dependent on the update speed used for "show" of the LEDs.
//
// - No dependency on timing libraries (millis(), millisDelay, etc.)
// - Simpler state management - just increment a counter
// - Self-regulating speed: animation rate = LED update rate
// - If you call this every 10ms, a cycle of 50 = 500ms between changes
//
// CYCLE SPEEDS (frames between animation updates):
// - cycle = 2  (C_RAINBOW, C_PASTEL): Very fast, changes every 2 calls
// - cycle = 5  (C_AMBER_PULSE): Fast pulse
// - cycle = 7  (C_ORANGEPURPLE, C_REDPURPLE): Medium speed alternation
// - cycle = 8  (C_RED_FADE): Medium fade speed
// - cycle = 10 (C_ORANGE_FADE): Slower fade
// - cycle = 50 (C_REDGREEN, C_BLUEGREEN): Slow alternation
//
// Call this function every time you update a chain of LEDs.
LED_HSV Lighting::getDynamicColorHSV(uint8_t deviceSlot, DynamicColor color, uint8_t brightness, uint8_t saturation) {
  // Ensure deviceSlot is within bounds
  if(deviceSlot >= MAX_DYNAMIC_COLOR_DEVICES) {
    deviceSlot = 0;
  }

  // Cycle rate for counter-based timing (frames between changes).
  uint8_t cycle = 2; // Initial value, each pattern sets its own cycle rate.

  switch(color) {
    case C_REDGREEN:
      // Alternate between red (0) and green (96)
      if(s_dynamicHue[deviceSlot] != 0 && s_dynamicHue[deviceSlot] != 96) {
        s_dynamicHue[deviceSlot] = 0; // Reset if out of range
      }

      cycle = 50;
      s_dynamicCounter[deviceSlot]++;

      if(s_dynamicCounter[deviceSlot] % cycle == 0) {
        s_dynamicHue[deviceSlot] = (s_dynamicHue[deviceSlot] == 0) ? 96 : 0;
        s_dynamicCounter[deviceSlot] = 1;
      }

      return {s_dynamicHue[deviceSlot], 255, brightness};
    // END C_REDGREEN

    case C_ORANGEPURPLE:
      // Alternate between orange (15) and purple (210)
      if(s_dynamicHue[deviceSlot] != 15 && s_dynamicHue[deviceSlot] != 210) {
        s_dynamicHue[deviceSlot] = 15; // Reset if out of range
      }

      cycle = 7;
      s_dynamicCounter[deviceSlot]++;

      if(s_dynamicCounter[deviceSlot] % cycle == 0) {
        s_dynamicHue[deviceSlot] = (s_dynamicHue[deviceSlot] == 15) ? 210 : 15;
        s_dynamicCounter[deviceSlot] = 1;
      }

      return {s_dynamicHue[deviceSlot], 255, brightness};
    // END C_ORANGEPURPLE

    case C_BLUEGREEN:
      // Alternate between blue (145) and green (96)
      if(s_dynamicHue[deviceSlot] != 145 && s_dynamicHue[deviceSlot] != 96) {
        s_dynamicHue[deviceSlot] = 145; // Reset if out of range
      }

      cycle = 50;
      s_dynamicCounter[deviceSlot]++;

      if(s_dynamicCounter[deviceSlot] % cycle == 0) {
        s_dynamicHue[deviceSlot] = (s_dynamicHue[deviceSlot] == 96) ? 145 : 96;
        s_dynamicCounter[deviceSlot] = 1;
      }

      return {s_dynamicHue[deviceSlot], 255, brightness};
    // END C_BLUEGREEN

    case C_REDPURPLE:
      // Alternate between red (0) and purple (210)
      if(s_dynamicHue[deviceSlot] != 0 && s_dynamicHue[deviceSlot] != 210) {
        s_dynamicHue[deviceSlot] = 0; // Reset if out of range
      }

      cycle = 7;
      s_dynamicCounter[deviceSlot]++;

      if(s_dynamicCounter[deviceSlot] % cycle == 0) {
        s_dynamicHue[deviceSlot] = (s_dynamicHue[deviceSlot] == 0) ? 210 : 0;
        s_dynamicCounter[deviceSlot] = 1;
      }

      return {s_dynamicHue[deviceSlot], 255, brightness};
    // END C_REDPURPLE

    case C_AMBER_PULSE:
      // Pulse between amber (24) and orange (32)
      if(s_dynamicHue[deviceSlot] < 20 || s_dynamicHue[deviceSlot] > 32) {
        s_dynamicHue[deviceSlot] = 24; // Reset if out of range
        s_dynamicNextBright[deviceSlot] = 1; // Start incrementing
      }

      cycle = 5;
      s_dynamicCounter[deviceSlot]++;

      if(s_dynamicCounter[deviceSlot] % cycle == 0) {
        s_dynamicHue[deviceSlot] += s_dynamicNextBright[deviceSlot];

        // Reverse direction at boundaries
        if(s_dynamicHue[deviceSlot] >= 32) {
          s_dynamicNextBright[deviceSlot] = -1;
        }
        else if(s_dynamicHue[deviceSlot] <= 20) {
          s_dynamicNextBright[deviceSlot] = 1;
        }

        s_dynamicCounter[deviceSlot] = 1;
      }

      return {s_dynamicHue[deviceSlot], 255, brightness};
    // END C_AMBER_PULSE

    case C_ORANGE_FADE:
      // Fade brightness on orange hue (28)
      if(s_dynamicBright[deviceSlot] == 0) {
        s_dynamicBright[deviceSlot] = 50;
        s_dynamicNextBright[deviceSlot] = 5; // Start incrementing
      }

      cycle = 10;
      s_dynamicCounter[deviceSlot]++;

      if(s_dynamicCounter[deviceSlot] % cycle == 0) {
        s_dynamicBright[deviceSlot] += s_dynamicNextBright[deviceSlot];

        // Reverse direction at boundaries
        if(s_dynamicBright[deviceSlot] >= 250) {
          s_dynamicNextBright[deviceSlot] = -5;
        }
        else if(s_dynamicBright[deviceSlot] <= 50) {
          s_dynamicNextBright[deviceSlot] = 5;
        }

        s_dynamicCounter[deviceSlot] = 1;
      }

      return {28, 255, s_dynamicBright[deviceSlot]};
    // END C_ORANGE_FADE

    case C_RED_FADE:
      // Fade brightness on red hue (0)
      if(s_dynamicBright[deviceSlot] == 0) {
        s_dynamicBright[deviceSlot] = 50;
        s_dynamicNextBright[deviceSlot] = 5; // Start incrementing
      }

      cycle = 8;
      s_dynamicCounter[deviceSlot]++;

      if(s_dynamicCounter[deviceSlot] % cycle == 0) {
        s_dynamicBright[deviceSlot] += s_dynamicNextBright[deviceSlot];

        // Reverse direction at boundaries
        if(s_dynamicBright[deviceSlot] >= 250) {
          s_dynamicNextBright[deviceSlot] = -5;
        }
        else if(s_dynamicBright[deviceSlot] <= 50) {
          s_dynamicNextBright[deviceSlot] = 5;
        }

        s_dynamicCounter[deviceSlot] = 1;
      }

      return {0, 255, s_dynamicBright[deviceSlot]};
    // END C_RED_FADE

    case C_PASTEL:
      // Cycle through all hues (0-255) at half saturation
      s_dynamicCounter[deviceSlot]++;

      if(s_dynamicCounter[deviceSlot] % cycle == 0) {
        s_dynamicHue[deviceSlot] = (s_dynamicHue[deviceSlot] + 5) % 256;
        s_dynamicCounter[deviceSlot] = 1;
      }

      return {s_dynamicHue[deviceSlot], 128, brightness};
    // END C_PASTEL

    case C_RAINBOW:
    default:
      // Cycle through all hues (0-255) at full saturation
      s_dynamicCounter[deviceSlot]++;

      if(s_dynamicCounter[deviceSlot] % cycle == 0) {
        s_dynamicHue[deviceSlot] = (s_dynamicHue[deviceSlot] + 5) % 256;
        s_dynamicCounter[deviceSlot] = 1;
      }

      return {s_dynamicHue[deviceSlot], 255, brightness};
    // END C_RAINBOW
  }
}
