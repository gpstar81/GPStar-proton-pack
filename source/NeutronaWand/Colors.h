/**
 *   gpstar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gmail.com>
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
 * Colors are based on the RGB pattern; for GRB the Red/Green values should be switched.
 * Brightness is based on varying the intensity of each (0-255) using a relative value.
 *
 * However, colors based on Hue use a color degree, a Saturation, and Value (brightness).
*/

enum colors {
  C_BLACK,
  C_WHITE,
  C_PINK,
  C_RED,
  C_ORANGE,
  C_YELLOW,
  C_GREEN,
  C_AQUA,
  C_BLUE,
  C_PURPLE
};

int getBrightness(uint8_t i_percent = 100){
  // Brightness here is a percentage, to be convered to a range 0-255.
  if (i_percent > 100) {
    i_percent = 100;
  }
  return (int) ((255 * i_percent) / 100);
}

CRGB getColor(uint8_t i_color, uint8_t i_brightness = 255, bool b_grb = false) {
  // Brightness here is a value from 0-255 as limited by byte (uint8_t) type.

  // Colors will be relative to pre-calculated full or half brightness values.
  uint8_t v_full = (int) ((255 * i_brightness) / 100);
  uint8_t v_half = (int) ((128 * i_brightness) / 100);

  // Returns a CRGB object with the correct color values for Red, Green, and Blue.
  switch (i_color) {
    case C_BLACK:
      return CRGB(0, 0, 0);
      break;
    case C_WHITE:
      return CRGB(v_full, v_full, v_full);
      break;
    case C_PINK:
      if (b_grb) {
        return CRGB(v_half, v_full, v_half);
      }
      return CRGB(v_full, v_half, v_half);
      break;
    case C_RED:
      if (b_grb) {
        return CRGB(0, v_full, 0);
      }
      return CRGB(v_full, 0, 0);
      break;
    case C_ORANGE:
      if (b_grb) {
        return CRGB(v_half, v_full, 0);
      }
      return CRGB(v_full, v_half, 0);
      break;
    case C_YELLOW:
      return CRGB(v_full, v_full, 0);
      break;
    case C_GREEN:
      if (b_grb) {
        return CRGB(v_full, 0, 0);
      }
      return CRGB(0, v_full, 0);
      break;
    case C_AQUA:
      if (b_grb) {
        return CRGB(0, v_full, v_full);
      }
      return CRGB(0, v_full, v_full);
      break;
    case C_BLUE:
      return CRGB(0, 0, v_full);
      break;
    case C_PURPLE:
      if (b_grb) {
        return CRGB(0, v_full, v_full);
      }
      return CRGB(v_full, 0, v_full);
      break;
  }

  return CRGB(0, 0, 0); // Return default if no case matches.
}

CHSV getHue(uint8_t i_color, uint8_t i_brightness = 255, uint8_t i_saturation = 255) {
  // Brightness here is a value from 0-255 as limited by byte (uint8_t) type.

  // Returns a CHSV object with a hue (color), full saturation, and stated brightness.
  switch (i_color) {
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
    case C_AQUA:
      return CHSV(128, i_saturation, i_brightness);
      break;
    case C_BLUE:
      return CHSV(160, i_saturation, i_brightness);
      break;
    case C_PURPLE:
      return CHSV(192, i_saturation, i_brightness);
      break;
  }

  return CHSV(0, 0, 0); // Return default if no case matches.
}