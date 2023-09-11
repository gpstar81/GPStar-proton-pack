/**
 *   gpstar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com> & Dustin Grau
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

/**
 * Colours based on Hue use a colour degree, a Saturation, and Value (brightness).
 * A conversion to RGB values is done to ensure consistent colors across devices.
 */

enum colours {
  C_BLACK,
  C_WHITE,
  C_PINK,
  C_RED,
  C_ORANGE,
  C_YELLOW,
  C_GREEN,
  C_MINT,
  C_AQUA,
  C_LIGHT_BLUE,
  C_BLUE,
  C_PURPLE,
  C_AMBER_PULSE,
  C_RED_FADE,
  C_REDGREEN,
  C_RAINBOW,
  C_SPECTRAL_CUSTOM
};

int getBrightness(uint8_t i_percent = 100) {
  // Brightness here is a percentage, to be convered to a range 0-255.
  if(i_percent > 100) {
    i_percent = 100;
  }
  return (int) ((255 * i_percent) / 100);
}

// Special values for colour cycles: current hue (colour) and when to change colour.
// This must match the number of device ENUM entries (though that is rarely changed).
uint8_t i_curr_colour[2] = { 0, 0 };
uint8_t i_curr_bright[2] = { 0, 0 };
uint8_t i_next_bright[2] = { -1, -1 };
uint8_t i_count[2] = { 0, 0 };

CHSV getHue(uint8_t i_device, uint8_t i_colour, uint8_t i_brightness = 255, uint8_t i_saturation = 255) {
  // Brightness here is a value from 0-255 as limited by byte (uint8_t) type.

  // Returns a CHSV object with a hue (colour), full saturation, and stated brightness.
  switch(i_colour) {
    case C_BLACK:
      return CHSV(0, 0, 0); // Overrides brightness.
    break;

    case C_WHITE:
      return CHSV(100, 0, i_brightness);
    break;

    case C_PINK:
      return CHSV(244, i_saturation, i_brightness);
    break;

    case C_RED:
      return CHSV(0, i_saturation, i_brightness);
    break;

    case C_ORANGE:
      return CHSV(32, i_saturation, i_brightness);
    break;

    case C_YELLOW:
      return CHSV(64, i_saturation, i_brightness);
    break;

    case C_GREEN:
      return CHSV(96, i_saturation, i_brightness);
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

    case C_BLUE:
      return CHSV(160, i_saturation, i_brightness);
    break;

    case C_PURPLE:
      return CHSV(192, i_saturation, i_brightness);
    break;

    case C_AMBER_PULSE:
      // Fade between amber (24) and orange (32).
      if(i_curr_colour[i_device] < 24 || i_curr_colour[i_device] > 32) {
        i_curr_colour[i_device] = 24; // Reset if out of range.
      }

      i_count[i_device]++;
      if(i_count[i_device] % 32 == 0) {
        i_curr_colour[i_device] = (i_curr_colour[i_device] + 1) % 32;
        i_count[i_device] = 24; // Reset counter.
      }

      return CHSV(i_curr_colour[i_device], 255, i_brightness);
    break;

    case C_RED_FADE:
      if(i_curr_bright[i_device] <= 1) {
        // Prime for the climb back to full brightness.
        i_curr_bright[i_device] = 1;
        i_next_bright[i_device] = 2;
      }
      if(i_curr_bright[i_device] >= 254) {
        // Prime for the climb back to full darkness.
        i_curr_bright[i_device] = 254;
        i_next_bright[i_device] = -2;
      }

      // Increments brightness by X steps on each processor loop.
      // Uses the +/- value to increment or decrement by the given value.
      i_count[i_device]++;
      if(i_count[i_device] % 2 == 0) {
        i_curr_bright[i_device] = i_curr_bright[i_device] + i_next_bright[i_device];
        i_count[i_device] = 0; // Reset counter.
      }

      return CHSV(0, 255, i_curr_bright[i_device]);
    break;

    case C_REDGREEN:
      // Alternate between red (0) and green (96) every X loops.
      if(i_curr_colour[i_device] != 0 && i_curr_colour[i_device] != 96) {
        i_curr_colour[i_device] = 0; // Reset if out of range.
      }

      i_count[i_device]++;

      if(i_count[i_device] % 200 == 0) {
        if(i_curr_colour[i_device] == 0) {
          i_curr_colour[i_device] = 96;
          i_count[i_device] = 0; // Reset counter.
        }
        else {
          i_curr_colour[i_device] = 0;
        }
      }

      return CHSV(i_curr_colour[i_device], 255, i_brightness);
    break;

    case C_RAINBOW:
      // Cycle through all colours (0-255) at full saturation every X loops.
      i_count[i_device]++;
      if(i_count[i_device] % 10 == 0) {
        // Increments color by 1 for more gradual fade between cycles.
        i_curr_colour[i_device] = (i_curr_colour[i_device] + 1) % 255;
        i_count[i_device] = 0; // Reset counter.
      }

      return CHSV(i_curr_colour[i_device], 255, i_brightness);
    break;

    case C_SPECTRAL_CUSTOM:
      return CHSV(i_spectral_custom, i_spectral_custom_saturation, i_brightness);
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
  // Forward to getHueAsRGB() with the flag set for GRB color swap.
  return getHueAsRGB(i_device, i_colour, i_brightness, true);
}