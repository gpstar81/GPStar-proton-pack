/**
 *   GPStar Stream Effects - Ghostbusters Props, Mods, and Kits.
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
  C_REDGREEN,
  C_ORANGEPURPLE,
  C_AMBER_PULSE,
  C_ORANGE_FADE,
  C_RED_FADE,
  C_RAINBOW,
  C_SPECTRAL_CUSTOM
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
uint8_t i_curr_colour[1] = { 0 };
uint8_t i_curr_bright[1] = { 0 };
int16_t i_next_bright[1] = { -1 }; // Uses int to allow negative steps.
uint8_t i_count[1] = { 0 }; // Counter-based changes for certain themes.
millisDelay ms_colour_change[1]; // Timers for changing colours for certain themes.
uint16_t i_change_delay[1] = { 10 }; // Default delay time for changes.

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

    case C_AMBER_PULSE:
      // Fade between amber (24) and orange (32).
      if(i_curr_colour[i_device] < 20 || i_curr_colour[i_device] > 32) {
        i_curr_colour[i_device] = 20; // Reset if out of range.
      }

      // Set the time delay for colour changes per device.
      i_change_delay[i_device] = 5;

      if(ms_colour_change[i_device].remaining() < 1) {
        i_count[i_device]++;
        if(i_count[i_device] % 32 == 0) {
          i_curr_colour[i_device] = (i_curr_colour[i_device] + 1) % 32;
          i_count[i_device] = 20; // Reset counter.
        }
        ms_colour_change[i_device].start(i_change_delay[i_device]);
      }

      return CHSV(i_curr_colour[i_device], 255, i_brightness);
    break;

    case C_ORANGE_FADE:
      // Set the time delay for colour changes per device.
      i_change_delay[i_device] = 10;

      // Increments brightness by X steps every X milliseconds.
      // Uses the +/- value to increment or decrement by the given value.
      if(ms_colour_change[i_device].remaining() < 1) {
        if(i_curr_bright[i_device] <= 1) {
          // Prime for the climb back to full brightness.
          i_curr_bright[i_device] = 1;
          i_next_bright[i_device] = 3;
        }
        if(i_curr_bright[i_device] >= 254) {
          // Prime for the climb back to full darkness.
          i_curr_bright[i_device] = 254;
          i_next_bright[i_device] = -3;
        }
        i_curr_bright[i_device] = i_curr_bright[i_device] + i_next_bright[i_device];
        ms_colour_change[i_device].start(i_change_delay[i_device]);
      }

      return CHSV(28, 255, i_curr_bright[i_device]);
    break;

    case C_RED_FADE:
      // Set the time delay for colour changes per device.
      i_change_delay[i_device] = 8;

      // Increments brightness by X steps every X milliseconds.
      // Uses the +/- value to increment or decrement by the given value.
      if(ms_colour_change[i_device].remaining() < 1) {
        if(i_curr_bright[i_device] <= 1) {
          // Prime for the climb back to full brightness.
          i_curr_bright[i_device] = 1;
          i_next_bright[i_device] = 3;
        }
        if(i_curr_bright[i_device] >= 254) {
          // Prime for the climb back to full darkness.
          i_curr_bright[i_device] = 254;
          i_next_bright[i_device] = -3;
        }
        i_curr_bright[i_device] = i_curr_bright[i_device] + i_next_bright[i_device];
        ms_colour_change[i_device].start(i_change_delay[i_device]);
      }

      return CHSV(0, 255, i_curr_bright[i_device]);
    break;

    case C_REDGREEN:
      // Alternate between red (0) and green (96) every X milliseconds.
      if(i_curr_colour[i_device] != 0 && i_curr_colour[i_device] != 96) {
        i_curr_colour[i_device] = 0; // Reset if out of range.
      }

      // Set the time delay for colour changes per device.
      i_change_delay[i_device] = 800;

      if(ms_colour_change[i_device].remaining() < 1) {
        // Swap colours and restart the timer.
        if(i_curr_colour[i_device] == 0) {
          i_curr_colour[i_device] = 96;
        }
        else {
          i_curr_colour[i_device] = 0;
        }
        ms_colour_change[i_device].start(i_change_delay[i_device]);
      }

      return CHSV(i_curr_colour[i_device], 255, i_brightness);
    break;

    case C_ORANGEPURPLE:
      // Alternate between orange (15) and purple (210) every X milliseconds.
      if(i_curr_colour[i_device] != 15 && i_curr_colour[i_device] != 210) {
        i_curr_colour[i_device] = 15; // Reset if out of range.
      }

      // Set the time delay for colour changes per device.
      i_change_delay[i_device] = 800;

      if(ms_colour_change[i_device].remaining() < 1) {
        // Swap colours and restart the timer.
        if(i_curr_colour[i_device] == 15) {
          i_curr_colour[i_device] = 210;
        }
        else {
          i_curr_colour[i_device] = 15;
        }
        ms_colour_change[i_device].start(i_change_delay[i_device]);
      }

      return CHSV(i_curr_colour[i_device], 255, i_brightness);
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

    case C_SPECTRAL_CUSTOM:
      return CHSV(i_spectral_custom_colour, i_spectral_custom_saturation, i_brightness);
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

CRGB getHueAsGBR(uint8_t i_device, uint8_t i_colour, uint8_t i_brightness = 255) {
  // Brightness here is a value from 0-255 as limited by byte (uint8_t) type.

  // Get the initial colour using the HSV scheme.
  CHSV hsv = getHue(i_device, i_colour, i_brightness);

  // Convert from HSV to RGB.
  CRGB rgb; // RGB Array as { r, g, b }
  hsv2rgb_rainbow(hsv, rgb);

  // Swap colour values before returning.
  return CRGB(rgb[1], rgb[2], rgb[0]);
}