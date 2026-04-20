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

#pragma once

// Standard library includes for integer type definitions
#include <stdint.h>  // Provides uint8_t, uint16_t, etc.

// LED_RGB: Platform-independent RGB color representation.
// Example: LED_RGB red = {255, 0, 0};
struct LED_RGB {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

// LED_HSV: Platform-independent HSV color representation.
// Example: LED_HSV cyan = {128, 255, 200}; (hue, saturation, brightness)
struct LED_HSV {
  uint8_t h; // Hue: 0-255 (0=red, 85=green, 170=blue)
  uint8_t s; // Saturation: 0-255 (0=white, 255=full color)
  uint8_t v; // Value (brightness): 0-255
};

// ColorOrder: LED strip color channel ordering.
// Different LED strips use different channel orders.
// WS2812B strips use ORDER_GRB, most others use ORDER_RGB.
enum ColorOrder : uint8_t {
  ORDER_RGB = 0,
  ORDER_GRB = 1,
  ORDER_GBR = 2
};

// SingleColor: Common static colors used across GPStar devices.
// These colors have fixed HSV values and no animation state.
// Devices can extend this enum for common, custom colors.
enum SingleColor {
  C_BLACK,
  C_WHITE,
  C_WARM_WHITE,
  C_PINK,
  C_PASTEL_PINK,
  C_RED,
  C_LIGHT_RED,
  C_RED2,
  C_RED3,
  C_RED4,
  C_RED5,
  C_ORANGE,
  C_BEIGE,
  C_YELLOW,
  C_CHARTREUSE,
  C_GREEN,
  C_DARK_GREEN,
  C_MINT,
  C_AQUA,
  C_LIGHT_BLUE,
  C_MID_BLUE,
  C_NAVY_BLUE,
  C_BLUE,
  C_PURPLE
};

// DynamicColor: Animated/stateful colors that change over time.
// These require state tracking and are managed by static methods.
enum DynamicColor {
  C_REDGREEN,
  C_ORANGEPURPLE,
  C_BLUEGREEN,
  C_REDPURPLE,
  C_AMBER_PULSE,
  C_ORANGE_FADE,
  C_RED_FADE,
  C_PASTEL,
  C_RAINBOW
};

// Maximum number of independent devices that can have dynamic colors.
// eg. ProtonPack has 6 devices (POWERCELL, CYCLOTRON_OUTER, CYCLOTRON_INNER, etc.)
#define MAX_DYNAMIC_COLOR_DEVICES 6

/**
 * Lighting: Utility class for LED color operations.
 *
 * This class provides:
 * - Standard color definitions used across all GPStar devices
 * - HSV color lookup for predefined static colors
 * - HSV color lookup for dynamic/animated colors with state tracking
 * - HSV to RGB color conversion (without FastLED dependency)
 * - Color channel reordering for different LED strip types
 * - Brightness percentage conversion
 *
 * Example usage:
 *   // Static colors:
 *   LED_HSV red_hsv = Lighting::getColorHSV(C_RED, 255, 255);
 *   LED_RGB red_rgb = Lighting::hsv2rgb(red_hsv);
 *
 *   // Dynamic colors (animated):
 *   LED_HSV rainbow = Lighting::getDynamicColorHSV(0, C_RAINBOW, 255);
 *   LED_RGB rgb = Lighting::hsv2rgb(rainbow);
 *
 *   // Apply color ordering for GRB strips:
 *   LED_RGB grb = Lighting::applyColorOrder(rgb, ORDER_GRB);
 *
 * Note: This class is entirely static - no instantiation needed.
 */
class Lighting {
  private:
    // State tracking for dynamic colors (one per device slot)
    static uint8_t s_dynamicHue[MAX_DYNAMIC_COLOR_DEVICES];
    static uint8_t s_dynamicBright[MAX_DYNAMIC_COLOR_DEVICES];
    static int16_t s_dynamicNextBright[MAX_DYNAMIC_COLOR_DEVICES];
    static uint8_t s_dynamicCounter[MAX_DYNAMIC_COLOR_DEVICES];

  public:
    // Reset all dynamic color state to initial values
    static void resetDynamicColors();

    // Get HSV color values for standard (static) colors.
    // Parameters:
    //   color: SingleColor enum value
    //   brightness: 0-255 (default: 255 = full brightness)
    //   saturation: 0-255 (default: 255 = full saturation)
    // Returns: LED_HSV with hue, saturation, and brightness
    // Example: LED_HSV blue = Lighting::getColorHSV(C_BLUE, 200, 255);
    static LED_HSV getColorHSV(SingleColor color, uint8_t brightness = 255, uint8_t saturation = 255);

    // Get HSV color values for dynamic (animated) colors.
    // Parameters:
    //   deviceSlot: [0-5] - Which device (6 Total)
    //   color: DynamicColor - Which pattern
    //   brightness: [0-255] - Target brightness (may be overridden by fade effects)
    //   saturation: [0-255] - Color saturation (default: 255)
    // Returns: LED_HSV for the current animation frame
    // Example: LED_HSV hsv = Lighting::getDynamicColorHSV(0, C_RAINBOW, 255);
    static LED_HSV getDynamicColorHSV(uint8_t deviceSlot, DynamicColor color, uint8_t brightness = 255, uint8_t saturation = 255);

    // Convert HSV color to RGB using rainbow algorithm for smooth color transitions.
    // Example: LED_RGB rgb = Lighting::hsv2rgb({128, 255, 200});
    static LED_RGB hsv2rgb(const LED_HSV &hsv);

    // Returns reordered RGB channels of a single color value (eg. RGB to GRB).
    // Example: LED_RGB grb = Lighting::applyColorOrder(rgb, ORDER_GRB);
    static LED_RGB applyColorOrder(const LED_RGB &color, ColorOrder order);

    // Convert brightness percentage (0-100) to byte value (0-255).
    // Example: uint8_t val = Lighting::getBrightness(50); // Returns 127
    static uint8_t getBrightness(uint8_t percent);

    // Scale RGB color by brightness factor (0-255).
    // Example: LED_RGB dimmed = Lighting::scaleBrightness(rgb, 128); // 50% brightness
    static LED_RGB scaleBrightness(const LED_RGB &color, uint8_t brightness);
};
