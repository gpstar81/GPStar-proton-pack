/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
 * Colours are based on the RGB pattern; for GRB the Red/Green values should be switched.
 * Brightness is based on varying the intensity of each (0-255) using a relative value.
 *
 * However, colours based on Hue use a colour degree, a Saturation, and Value (brightness).
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
  C_BLUEFADE,
  C_PASTEL,
  C_RAINBOW,
  C_CUSTOM_POWERCELL,
  C_CUSTOM_CYCLOTRON,
  C_CUSTOM_INNER_CYCLOTRON,
  C_HASLAB
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
uint8_t i_curr_colour[6] = { 0, 0, 0, 0, 0, 0 };
uint8_t i_count[6] = { 0, 0, 0, 0, 0, 0 };

uint8_t getDeviceColour(uint8_t i_device, uint8_t i_firing_mode, bool b_toggle) {
  // Toggle indicates use of Video Game colors, which is based on the firing mode.
  // Otherwise a default color will be used based on the device itself.
  if(b_toggle == true) {
    switch(i_firing_mode) {
      case PROTON:
        switch(i_device) {
          case POWERCELL:
            return C_MID_BLUE;
          break;

          case CYCLOTRON_OUTER:
          case CYCLOTRON_INNER:
          case CYCLOTRON_PANEL:
            return C_RED;
          break;

          case CYCLOTRON_CAVITY:
            // Cycles through 3 colors, changing on each call.
            // If starting at 0, value will increment to 1.
            // If value is above/divisible by 4, reset to 1.
            i_count[i_device]++;
            if(i_count[i_device] > 4 || i_count[i_device] % 4 == 0) {
              i_count[i_device] = 1; // Reset counter.
            }

            switch(i_count[i_device]) {
              case 1:
                return C_ORANGE;
              break;
              case 2:
                return C_WHITE;
              break;
              case 3:
              default:
                return C_YELLOW;
              break;
            }
          break;

          // VENT_LIGHT colour in PROTON mode will always be overridden by void ventLight()
          case VENT_LIGHT:
          default:
            return C_WHITE;
          break;
        }
      break;

      case SLIME:
        switch(i_device) {
          case POWERCELL:
          case CYCLOTRON_OUTER:
          case CYCLOTRON_INNER:
          case CYCLOTRON_PANEL:
          case VENT_LIGHT:
          default:
            if(SYSTEM_YEAR == SYSTEM_1989) {
              return C_PINK;
            }
            else {
              return C_GREEN;
            }
          break;
        }
      break;

      case STASIS:
        switch(i_device) {
          case POWERCELL:
          case CYCLOTRON_OUTER:
          case CYCLOTRON_INNER:
          case CYCLOTRON_PANEL:
          case VENT_LIGHT:
          default:
            return C_LIGHT_BLUE;
          break;
        }
      break;

      case MESON:
        switch(i_device) {
          case POWERCELL:
          case CYCLOTRON_OUTER:
          case CYCLOTRON_INNER:
          case CYCLOTRON_PANEL:
          case VENT_LIGHT:
          default:
            return C_ORANGE;
          break;
        }
      break;

      case SPECTRAL:
        switch(i_device) {
          case POWERCELL:
          case CYCLOTRON_OUTER:
          case CYCLOTRON_INNER:
          case CYCLOTRON_PANEL:
          case VENT_LIGHT:
          default:
            return C_RAINBOW;
          break;
        }
      break;

      case HOLIDAY:
        switch(i_device) {
          case POWERCELL:
          case CYCLOTRON_OUTER:
          case CYCLOTRON_INNER:
          case CYCLOTRON_PANEL:
          case VENT_LIGHT:
          default:
            return C_REDGREEN;
          break;
        }
      break;

      case SPECTRAL_CUSTOM:
        switch(i_device) {
          case POWERCELL:
            return C_CUSTOM_POWERCELL;
          break;

          case CYCLOTRON_OUTER:
            return C_CUSTOM_CYCLOTRON;
          break;

          case CYCLOTRON_INNER:
          case CYCLOTRON_PANEL:
            return C_CUSTOM_INNER_CYCLOTRON;
          break;

          case VENT_LIGHT:
          default:
            return C_CUSTOM_CYCLOTRON;
          break;
        }
      break;

      default:
        return C_MID_BLUE;
      break;
    }
  }
  else {
    switch(i_device) {
      case POWERCELL:
        return C_MID_BLUE;
      break;

      case CYCLOTRON_OUTER:
      case CYCLOTRON_INNER:
      case CYCLOTRON_PANEL:
        return C_RED;
      break;

      case CYCLOTRON_CAVITY:
        // Cycles through 3 colors, changing on each call.
        // If starting at 0, value will increment to 1.
        // If value is above/divisible by 4, reset to 1.
        i_count[i_device]++;
        if(i_count[i_device] > 4 || i_count[i_device] % 4 == 0) {
          i_count[i_device] = 1; // Reset counter.
        }

        switch(i_count[i_device]) {
          case 1:
            return C_ORANGE;
          break;
          case 2:
            return C_WHITE;
          break;
          case 3:
          default:
            return C_YELLOW;
          break;
        }
      break;

      // VENT_LIGHT colour in PROTON mode will always be overridden by void ventLight()
      case VENT_LIGHT:
      default:
        return C_WHITE;
      break;
    }
  }
}

CHSV getHue(uint8_t i_device, uint8_t i_colour, uint8_t i_brightness = 255, uint8_t i_saturation = 255) {
  // Brightness here is a value from 0-255 as limited by byte (uint8_t) type.

  // For colour cycles, i_cycle indicates how often to change colour.
  // This is device-dependent in order to provide a noticeable change.
  // Value must be >0 as this is used with modulo (cannot mod by 0).
  uint8_t i_cycle = 2;

  switch(i_device) {
    case CYCLOTRON_OUTER:
      i_cycle = 10;
    break;
    case CYCLOTRON_INNER:
    case CYCLOTRON_PANEL:
      i_cycle = 6;
    break;
  }

  // Returns a CHSV object with a hue (colour), full saturation, and stated brightness.
  switch(i_colour) {
    case C_HASLAB:
    case C_WHITE:
    default:
      return CHSV(100, 0, i_brightness); // Just "on", which is white.
    break;

    case C_CUSTOM_POWERCELL:
      return CHSV(i_spectral_powercell_custom_colour, i_spectral_powercell_custom_saturation, i_brightness); // b_spectral_powercell_custom is a global variable found in Configuration.h
    break;

    case C_CUSTOM_CYCLOTRON:
      return CHSV(i_spectral_cyclotron_custom_colour, i_spectral_cyclotron_custom_saturation, i_brightness); // b_spectral_cyclotron_custom is a global variable found in Configuration.h
    break;

    case C_CUSTOM_INNER_CYCLOTRON:
      return CHSV(i_spectral_cyclotron_inner_custom_colour, i_spectral_cyclotron_inner_custom_saturation, i_brightness); // b_spectral_cyclotron_inner_custom is a global variable found in Configuration.h
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

    case C_REDGREEN:
      // Alternate between red (0) and green (96).
      if(i_curr_colour[i_device] != 0 && i_curr_colour[i_device] != 96) {
        i_curr_colour[i_device] = 0; // Reset if out of range.
      }

      i_count[i_device]++;

      if(i_count[i_device] % i_cycle == 0) {
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

    case C_ORANGEPURPLE:
      // Alternate between orange (32) and purple (192).
      if(i_curr_colour[i_device] != 32 && i_curr_colour[i_device] != 192) {
        i_curr_colour[i_device] = 32; // Reset if out of range.
      }

      i_count[i_device]++;

      if(i_count[i_device] % i_cycle == 0) {
        if(i_curr_colour[i_device] == 32) {
          i_curr_colour[i_device] = 192;
          i_count[i_device] = 0; // Reset counter.
        }
        else {
          i_curr_colour[i_device] = 32;
        }
      }
      return CHSV(i_curr_colour[i_device], 255, i_brightness);
    break;

    case C_BLUEFADE:
      // Reset if out of range: blue (160) to light blue (146).
      // This is based on use of the 15-LED RGB Power Cell.
      if(i_count[i_device] < 146 || i_count[i_device] > 160) {
        i_count[i_device] = 160; // Reset if out of range.
      }

      // Cycles from dark to light blue (160-145) at full saturation.
      i_count[i_device]--;

      return CHSV(i_count[i_device], 255, i_brightness);
    break;

    case C_PASTEL:
      // Cycle through all colours (0-255) at half saturation.
      i_count[i_device]++;

      if(i_count[i_device] % i_cycle == 0) {
        i_curr_colour[i_device] = (i_curr_colour[i_device] + 5) % 255;
        i_count[i_device] = 0; // Reset counter.
      }

      return CHSV(i_curr_colour[i_device], 128, i_brightness);
    break;

    case C_RAINBOW:
      // Cycle through all colours (0-255) at full saturation.
      i_count[i_device]++;
      if(i_count[i_device] % i_cycle == 0) {
        i_curr_colour[i_device] = (i_curr_colour[i_device] + 5) % 255;
        i_count[i_device] = 0; // Reset counter.
      }

      return CHSV(i_curr_colour[i_device], 255, i_brightness);
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

CRGB getHueAsGBR(uint8_t i_device, uint8_t i_colour, uint8_t i_brightness = 255) {
  // Brightness here is a value from 0-255 as limited by byte (uint8_t) type.

  // Get the initial colour using the HSV scheme.
  CHSV hsv = getHue(i_device, i_colour, i_brightness);

  // Convert from HSV to RGB.
  CRGB rgb; // RGB Array as { r, g, b }
  hsv2rgb_rainbow(hsv, rgb);

  // Swap color values before returning.
  return CRGB(rgb[1], rgb[2], rgb[0]);
}