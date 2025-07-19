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

/**
 * Colours based on Hue use a colour degree, a Saturation, and Value (brightness).
 * A conversion to RGB values is done to ensure consistent colours across devices.
 */

enum colours {
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
  C_PURPLE,
  C_RAINBOW
};

uint8_t getBrightness(uint8_t i_percent = 100) {
  // Brightness here is a percentage, to be converted to a range 0-255.
  if(i_percent > 100) {
    i_percent = 100;
  }
  return (uint8_t) ((255 * i_percent) / 100);
}

// Special values for colour cycles: current hue (colour) and when to change colour.
// This must match the number of device ENUM entries (though that is rarely changed).
uint8_t i_curr_colour[DEVICE_NUM_LEDS] = { 0 };
millisDelay ms_colour_change[DEVICE_NUM_LEDS]; // Timers for changing colours for certain themes.
uint16_t i_change_delay[DEVICE_NUM_LEDS] = { 10 }; // Default delay time for changes.

CHSV getHue(uint8_t i_device, uint8_t i_colour, uint8_t i_brightness = 255, uint8_t i_saturation = 255) {
  // Brightness here is a value from 0-255 as limited by byte (uint8_t) type.

  // Returns a CHSV object with a hue (colour), full saturation, and stated brightness.
  switch(i_colour) {
    case C_WHITE:
      return CHSV(100, 0, i_brightness);
    break;

    case C_BLACK:
      return CHSV(0, 0, 0); // Overrides brightness.
    break;

    case C_WARM_WHITE:
      return CHSV(22, 155, i_brightness);
    break;

    case C_PINK:
      return CHSV(244, i_saturation, i_brightness);
    break;

    case C_PASTEL_PINK:
      return CHSV(244, 128, i_brightness);
    break;

    case C_RED:
      return CHSV(0, i_saturation, i_brightness);
    break;

    case C_LIGHT_RED:
      return CHSV(0, 192, i_brightness);
    break;

    case C_RED2:
      return CHSV(5, i_saturation, i_brightness);
    break;

    case C_RED3:
      return CHSV(10, i_saturation, i_brightness);
    break;

    case C_RED4:
      return CHSV(15, i_saturation, i_brightness);
    break;

    case C_RED5:
      return CHSV(20, i_saturation, i_brightness);
    break;

    case C_ORANGE:
      return CHSV(32, i_saturation, i_brightness);
    break;

    case C_BEIGE:
      return CHSV(43, 128, i_brightness);
    break;

    case C_YELLOW:
      return CHSV(64, i_saturation, i_brightness);
    break;

    case C_CHARTREUSE:
      return CHSV(80, i_saturation, i_brightness);
    break;

    case C_GREEN:
      return CHSV(96, i_saturation, i_brightness);
    break;

    case C_DARK_GREEN:
      return CHSV(96, i_saturation, 128);
    break;

    case C_MINT:
      return CHSV(112, 120, i_brightness);
    break;

    case C_AQUA:
      return CHSV(128, i_saturation, i_brightness);
    break;

    case C_LIGHT_BLUE:
      return CHSV(145, i_saturation, i_brightness);
    break;

    case C_MID_BLUE:
      return CHSV(160, i_saturation, i_brightness);
    break;

    case C_NAVY_BLUE:
      return CHSV(170, 200, 112);
    break;

    case C_BLUE:
      return CHSV(180, i_saturation, i_brightness);
    break;

    case C_PURPLE:
      return CHSV(192, i_saturation, i_brightness);
    break;

    case C_RAINBOW:
      // Set the time delay for colour changes per device.
      i_change_delay[i_device] = 10;

      // Cycle through all colours (0-255) at full saturation every X milliseconds.
      if(ms_colour_change[i_device].remaining() < 1) {
        // Increment colour and restart the timer.
        i_curr_colour[i_device] = (i_curr_colour[i_device] + 1) % 255;
        ms_colour_change[i_device].start(i_change_delay[i_device]);
      }

      return CHSV(i_curr_colour[i_device], 255, i_brightness);
    break;

    default:
      return CHSV(100, 0, i_brightness); // White on no match.
    break;
  }
}

CRGB getHueAsRGB(uint8_t i_device, uint8_t i_colour, uint8_t i_brightness = 255, bool b_grb = false) {
  // Brightness here is a value from 0-255 as limited by byte (uint8_t) type.

  // Get the initial colour using the HSV scheme.
  CHSV hsv = getHue(i_device, i_colour, i_brightness);

  // Convert from HSV to RGB.
  CRGB rgb; // RGB Array as { r, g, b }
  hsv2rgb_rainbow(hsv, rgb);

  if(b_grb) {
    // Swap red/green values before returning.
    return CRGB(rgb[1], rgb[0], rgb[2]);
  }
  else {
    return rgb; // Return RGB object.
  }
}

CRGB getHueAsGRB(uint8_t i_device, uint8_t i_colour, uint8_t i_brightness = 255) {
  // Forward to getHueAsRGB() with the flag set for GRB colour swap.
  return getHueAsRGB(i_device, i_colour, i_brightness, true);
}
