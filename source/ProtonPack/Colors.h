/**
 *   gpstar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
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
 * However, colors based on Hue use a color spectrum, a Saturation, and Value (brightness).
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

CRGB getColor(int8_t color, int8_t i_brightness = 255) {
  // Returns a CRGB object with the correct color values for Red, Green, and Blue.
  uint8_t v_full = (int) ((255 * i_cyclotron_inner_brightness) / 100);
  uint8_t v_half = (int) ((128 * i_cyclotron_inner_brightness) / 100);

  switch (color) {
    case C_BLACK:
      return CRGB(0, 0, 0);
      break;
    case C_WHITE:
      return CRGB(v_full, v_full, v_full);
      break;
    case C_PINK:
      return CRGB(v_full, v_half, v_half);
      break;
    case C_RED:
      return CRGB(v_full, 0, 0);
      break;
    case C_ORANGE:
      return CRGB(v_full, v_half, 0);
      break;
    case C_YELLOW:
      return CRGB(v_full, v_full, 0);
      break;
    case C_GREEN:
      return CRGB(0, v_full, 0);
      break;
    case C_AQUA:
      return CRGB(0, v_full, v_full);
      break;
    case C_BLUE:
      return CRGB(0, 0, v_full);
      break;
    case C_PURPLE:
      return CRGB(v_full, 0, v_full);
      break;
  }
}

CHSV getHue(int8_t color, int8_t i_brightness = 255) {
  // Returns a CHSV object with a hue (color), full saturation, and stated brightness.

  switch (color) {
    case C_BLACK:
      return CHSV(0, 0, 0);
      break;
    case C_WHITE:
      return CHSV(100, 0, i_brightness);
      break;
    case C_PINK:
      return CHSV(244, 255, i_brightness);
      break;
    case C_RED:
      return CHSV(0, 255, i_brightness);
      break;
    case C_ORANGE:
      return CHSV(32, 255, i_brightness);
      break;
    case C_YELLOW:
      return CHSV(64, 255, i_brightness);
      break;
    case C_GREEN:
      return CHSV(96, 255, i_brightness);
      break;
    case C_AQUA:
      return CHSV(128, 255, i_brightness);
      break;
    case C_BLUE:
      return CHSV(160, 255, i_brightness);
      break;
    case C_PURPLE:
      return CHSV(192, 255, i_brightness);
      break;
  }
}