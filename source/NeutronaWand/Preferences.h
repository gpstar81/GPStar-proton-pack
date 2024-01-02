/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
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

/*
 * User Preference Storage/Retrieval via EEPROM
 *
 * This library of functions controls the storing, clearing, reading, and management
 * of user preferences by way of the EEPROM storage area of the ATMega2560 chipset.
 * Values are stored a simple integer types, though they are mapped to boolean and
 * other datatypes as necessary during the reading/storing actions. Additionally,
 * a user may not even be using the EEPROM storage area and thus defaults may be
 * set when values are not present.
 */

/*
 * Function prototypes.
 */
void readEEPROM();
void clearConfigEEPROM();
void clearLedEEPROM();
void saveConfigEEPROM();
void saveLedEEPROM();
void updateCRCEEPROM();
unsigned long eepromCRC(void);
void bargraphYearModeUpdate();
void resetOverHeatModes();
void setBargraphOrientation();

/*
 * General EEPROM Variables
 */
unsigned int i_eepromAddress = 0; // The address in the EEPROM to start reading from.
unsigned long l_crc_size = ~0L; // The 4 last bytes are reserved for storing the CRC.

/*
 * Data structure object for LED settings which are saved into the EEPROM memory.
 */
struct objLEDEEPROM {
  uint8_t barrel_spectral_custom;
  uint8_t barrel_spectral_saturation_custom;
};

/*
 * Data structure object for customizations which are saved into the EEPROM memory.
 */
struct objEEPROM {
  uint8_t cross_the_streams;
  uint8_t cross_the_streams_mix;
  uint8_t overheating;
  uint8_t neutrona_wand_sounds;
  uint8_t spectral_mode;
  uint8_t holiday_mode;

  uint8_t quick_vent;
  uint8_t wand_boot_errors;
  uint8_t vent_light_auto_intensity;
  uint8_t num_barrel_leds;

  uint8_t invert_bargraph;
  uint8_t bargraph_mode;
  uint8_t bargraph_firing_animation;
  uint8_t bargraph_overheat_blinking;

  uint8_t neutrona_wand_year_mode;
  uint8_t CTS_mode;
  uint8_t beep_loop;

  uint8_t overheat_start_timer_level_5;
  uint8_t overheat_start_timer_level_4;
  uint8_t overheat_start_timer_level_3;
  uint8_t overheat_start_timer_level_2;
  uint8_t overheat_start_timer_level_1;

  uint8_t overheat_level_5;
  uint8_t overheat_level_4;
  uint8_t overheat_level_3;
  uint8_t overheat_level_2;
  uint8_t overheat_level_1;
};

/*
 * Read all user preferences from Proton Pack controller EEPROM.
 */
void readEEPROM() {
  // Get the stored CRC from the EEPROM.
  unsigned long l_crc_check;
  EEPROM.get(EEPROM.length() - sizeof(l_crc_size), l_crc_check);

  // Check if the calculated CRC matches the stored CRC value in the EEPROM.
  if(eepromCRC() == l_crc_check) {
    // Read our object from the EEPROM.
    objEEPROM obj_eeprom;
    EEPROM.get(i_eepromAddress, obj_eeprom);

    if(obj_eeprom.cross_the_streams > 0 && obj_eeprom.cross_the_streams != 255) {
      if(obj_eeprom.cross_the_streams > 1) {
        b_cross_the_streams = true;
      }
      else {
        b_cross_the_streams = false;
      }
    }

    if(obj_eeprom.cross_the_streams_mix > 0 && obj_eeprom.cross_the_streams_mix != 255) {
      if(obj_eeprom.cross_the_streams_mix > 1) {
        b_cross_the_streams_mix = true;
      }
      else {
        b_cross_the_streams_mix = false;
      }
    }

    if(obj_eeprom.overheating > 0 && obj_eeprom.overheating != 255) {
      if(obj_eeprom.overheating > 1) {
        b_overheat_enabled = true;
      }
      else {
        b_overheat_enabled = false;
      }
    }

    if(obj_eeprom.neutrona_wand_sounds > 0 && obj_eeprom.neutrona_wand_sounds != 255) {
      if(obj_eeprom.neutrona_wand_sounds > 1) {
        b_extra_pack_sounds = true;
      }
      else {
        b_extra_pack_sounds = false;
      }
    }

    if(obj_eeprom.spectral_mode > 0 && obj_eeprom.spectral_mode != 255) {
      if(obj_eeprom.spectral_mode > 1) {
        b_spectral_mode_enabled = true;
        b_spectral_custom_mode_enabled = true;
      }
      else {
        b_spectral_mode_enabled = false;
        b_spectral_custom_mode_enabled = false;
      }
    }

    if(obj_eeprom.holiday_mode > 0 && obj_eeprom.holiday_mode != 255) {
      if(obj_eeprom.holiday_mode > 1) {
        b_holiday_mode_enabled = true;
      }
      else {
        b_holiday_mode_enabled = false;
      }
    }

    if(obj_eeprom.quick_vent > 0 && obj_eeprom.quick_vent != 255) {
      if(obj_eeprom.quick_vent > 1) {
        b_quick_vent = true;
      }
      else {
        b_quick_vent = false;
      }
    }

    if(obj_eeprom.wand_boot_errors > 0 && obj_eeprom.wand_boot_errors != 255) {
      if(obj_eeprom.wand_boot_errors > 1) {
        b_wand_boot_errors = true;
      }
      else {
        b_wand_boot_errors = false;
      }
    }

    if(obj_eeprom.vent_light_auto_intensity > 0 && obj_eeprom.vent_light_auto_intensity != 255) {
      if(obj_eeprom.vent_light_auto_intensity > 1) {
        b_vent_light_control = true;
      }
      else {
        b_vent_light_control = false;
      }
    }

    if(obj_eeprom.num_barrel_leds > 0 && obj_eeprom.num_barrel_leds != 255) {
      /*
      i_num_barrel_leds = obj_eeprom.num_barrel_leds; // Keep it disabled for now until new barrel leds are ready.

      switch(i_num_barrel_leds) {
        case 5:
        default:
          WAND_BARREL_LED_COUNT = LEDS_5;
        case 48:
          WAND_BARREL_LED_COUNT = LEDS_48;
        break;

        case 60:
          WAND_BARREL_LED_COUNT = LEDS_60;
        break;
      }
      */
    }

    if(obj_eeprom.invert_bargraph > 0 && obj_eeprom.invert_bargraph != 255) {
      if(obj_eeprom.invert_bargraph > 1) {
        b_bargraph_invert = true;
      }
      else {
        b_bargraph_invert = false;
      }

      setBargraphOrientation();
    }

    if(obj_eeprom.bargraph_mode > 0 && obj_eeprom.bargraph_mode != 255) {
      switch(obj_eeprom.bargraph_mode) {
        case 3:
          BARGRAPH_MODE = BARGRAPH_ORIGINAL;
          BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_ORIGINAL;
        break;

        case 2:
          BARGRAPH_MODE = BARGRAPH_SUPER_HERO;
          BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_SUPER_HERO;
        break;

        case 1:
        default:
          BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_DEFAULT;
        break;
      }
    }

    if(obj_eeprom.bargraph_firing_animation > 0 && obj_eeprom.bargraph_mode != 255) {
      switch(obj_eeprom.bargraph_firing_animation) {
        case 3:
          BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_ORIGINAL;
          BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_ORIGINAL;
        break;

        case 2:
          BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_SUPER_HERO;
          BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_SUPER_HERO;
        break;

        case 1:
        default:
          BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_DEFAULT;
        break;
      }
    }

    if(obj_eeprom.bargraph_overheat_blinking > 0 && obj_eeprom.bargraph_overheat_blinking != 255) {
      if(obj_eeprom.bargraph_overheat_blinking > 1) {
        b_overheat_bargraph_blink = true;
      }
      else {
        b_overheat_bargraph_blink = false;
      }
    }

    if(obj_eeprom.neutrona_wand_year_mode > 0 && obj_eeprom.neutrona_wand_year_mode != 255) {
      switch(obj_eeprom.neutrona_wand_year_mode) {
        case 5:
          WAND_YEAR_MODE = YEAR_FROZEN_EMPIRE;
        break;

        case 4:
          WAND_YEAR_MODE = YEAR_AFTERLIFE;
        break;

        case 3:
          WAND_YEAR_MODE = YEAR_1989;
        break;

        case 2:
          WAND_YEAR_MODE = YEAR_1984;
        break;

        case 1:
        default:
          WAND_YEAR_MODE = YEAR_DEFAULT;
        break;
      }
    }

    if(obj_eeprom.CTS_mode > 0 && obj_eeprom.CTS_mode != 255) {
      switch(obj_eeprom.CTS_mode) {
        case 5:
          WAND_YEAR_CTS = CTS_FROZEN_EMPIRE;
        break;

        case 4:
          WAND_YEAR_CTS = CTS_AFTERLIFE;
        break;

        case 3:
          WAND_YEAR_CTS = CTS_1989;
        break;

        case 2:
          WAND_YEAR_CTS = CTS_1984;
        break;

        case 1:
        default:
          WAND_YEAR_CTS = CTS_DEFAULT;
        break;
      }
    }

    if(obj_eeprom.beep_loop > 0 && obj_eeprom.beep_loop != 255) {
      if(obj_eeprom.beep_loop > 1) {
        b_beep_loop = true;
      }
      else {
        b_beep_loop = false;
      }
    }

    if(obj_eeprom.overheat_start_timer_level_5 > 0 && obj_eeprom.overheat_start_timer_level_5 != 255) {
      i_ms_overheat_initiate_mode_5 = obj_eeprom.overheat_start_timer_level_5 * 1000;

      i_ms_overheat_initiate[4] = i_ms_overheat_initiate_mode_5;
    }

    if(obj_eeprom.overheat_start_timer_level_4 > 0 && obj_eeprom.overheat_start_timer_level_4 != 255) {
      i_ms_overheat_initiate_mode_4 = obj_eeprom.overheat_start_timer_level_4 * 1000;

      i_ms_overheat_initiate[3] = i_ms_overheat_initiate_mode_4;
    }

    if(obj_eeprom.overheat_start_timer_level_3 > 0 && obj_eeprom.overheat_start_timer_level_3 != 255) {
      i_ms_overheat_initiate_mode_3 = obj_eeprom.overheat_start_timer_level_3 * 1000;

      i_ms_overheat_initiate[2] = i_ms_overheat_initiate_mode_3;
    }

    if(obj_eeprom.overheat_start_timer_level_2 > 0 && obj_eeprom.overheat_start_timer_level_2 != 255) {
      i_ms_overheat_initiate_mode_2 = obj_eeprom.overheat_start_timer_level_2 * 1000;

      i_ms_overheat_initiate[1] = i_ms_overheat_initiate_mode_2;
    }

    if(obj_eeprom.overheat_start_timer_level_1 > 0 && obj_eeprom.overheat_start_timer_level_1 != 255) {
      i_ms_overheat_initiate_mode_1 = obj_eeprom.overheat_start_timer_level_1 * 1000;

      i_ms_overheat_initiate[0] = i_ms_overheat_initiate_mode_1;
    }

    if(obj_eeprom.overheat_level_5 > 0 && obj_eeprom.overheat_level_5 != 255) {
      if(obj_eeprom.overheat_level_5 > 1) {
        b_overheat_mode_5 = true;
      }
      else {
        b_overheat_mode_5 = false;
      }
    }

    if(obj_eeprom.overheat_level_4 > 0 && obj_eeprom.overheat_level_4 != 255) {
      if(obj_eeprom.overheat_level_4 > 1) {
        b_overheat_mode_4 = true;
      }
      else {
        b_overheat_mode_4 = false;
      }
    }

    if(obj_eeprom.overheat_level_3 > 0 && obj_eeprom.overheat_level_3 != 255) {
      if(obj_eeprom.overheat_level_3 > 1) {
        b_overheat_mode_3 = true;
      }
      else {
        b_overheat_mode_3 = false;
      }
    }

    if(obj_eeprom.overheat_level_2 > 0 && obj_eeprom.overheat_level_2 != 255) {
      if(obj_eeprom.overheat_level_2 > 1) {
        b_overheat_mode_2 = true;
      }
      else {
        b_overheat_mode_2 = false;
      }
    }

    if(obj_eeprom.overheat_level_1 > 0 && obj_eeprom.overheat_level_1 != 255) {
      if(obj_eeprom.overheat_level_1 > 1) {
        b_overheat_mode_1 = true;
      }
      else {
        b_overheat_mode_1 = false;
      }
    }

    // Update the bargraph settings again after loading EEPROM setting data for it.
    bargraphYearModeUpdate();

    // Rebuild the over heat enabled modes.
    resetOverHeatModes();

    // Read our LED object from the EEPROM.
    objLEDEEPROM obj_led_eeprom;
    unsigned int i_eepromLEDAddress = EEPROM.length() / 2;

    EEPROM.get(i_eepromLEDAddress, obj_led_eeprom);
    if(obj_led_eeprom.barrel_spectral_custom > 0 && obj_led_eeprom.barrel_spectral_custom != 255) {
      i_spectral_wand_custom_colour = obj_led_eeprom.barrel_spectral_custom;
    }

    if(obj_led_eeprom.barrel_spectral_saturation_custom > 0 && obj_led_eeprom.barrel_spectral_saturation_custom != 255) {
      i_spectral_wand_custom_saturation = obj_led_eeprom.barrel_spectral_saturation_custom;
    }
  }
}

void clearLEDEEPROM() {
  // Clear out the EEPROM data for the configuration settings only.
  unsigned int i_eepromLEDAddress = EEPROM.length() / 2;

  for(unsigned int i = 0 ; i < sizeof(objLEDEEPROM); i++) {
    EEPROM.put(i_eepromLEDAddress, 0);

    i_eepromLEDAddress++;
  }

  updateCRCEEPROM();
}

void saveLedEEPROM() {
  unsigned int i_eepromLEDAddress = EEPROM.length() / 2;

  // For now we are just saving the Spectral Custom colour.
  objLEDEEPROM obj_eeprom = {
    i_spectral_wand_custom_colour,
    i_spectral_wand_custom_saturation,
  };

  // Save to the EEPROM.
  EEPROM.put(i_eepromLEDAddress, obj_eeprom);

  updateCRCEEPROM();
}

void clearConfigEEPROM() {
  // Clear out the EEPROM only in the memory addresses used for our EEPROM data object.
  for(unsigned int i = 0 ; i < sizeof(objEEPROM); i++) {
    EEPROM.put(i, 0);
  }

  updateCRCEEPROM();
}

void saveConfigEEPROM() {
  // (Video Game Modes) + Cross The Streams / Cross The Streams Mix / Overheating
  uint8_t i_cross_the_streams = 1;
  uint8_t i_cross_the_streams_mix = 1;
  uint8_t i_overheating = 1;
  uint8_t i_neutrona_wand_sounds = 1;
  uint8_t i_spectral = 1;
  uint8_t i_holiday = 1;
  uint8_t i_quick_vent = 1;
  uint8_t i_wand_boot_errors = 1;
  uint8_t i_vent_light_auto_intensity = 1;
  uint8_t i_invert_bargraph = 1;
  uint8_t i_bargraph_mode = 1;
  uint8_t i_bargraph_firing_animation = 1;
  uint8_t i_bargraph_overheat_blinking = 1;
  uint8_t i_neutrona_wand_year_mode = 1;
  uint8_t i_CTS_mode = 1;
  uint8_t i_beep_loop = 2;
  uint8_t i_overheat_start_timer_level_5 = i_ms_overheat_initiate_mode_5 / 1000;
  uint8_t i_overheat_start_timer_level_4 = i_ms_overheat_initiate_mode_4 / 1000;
  uint8_t i_overheat_start_timer_level_3 = i_ms_overheat_initiate_mode_3 / 1000;
  uint8_t i_overheat_start_timer_level_2 = i_ms_overheat_initiate_mode_2 / 1000;
  uint8_t i_overheat_start_timer_level_1 = i_ms_overheat_initiate_mode_1 / 1000;
  uint8_t i_overheat_level_5 = 1;
  uint8_t i_overheat_level_4 = 1;
  uint8_t i_overheat_level_3 = 1;
  uint8_t i_overheat_level_2 = 1;
  uint8_t i_overheat_level_1 = 1;

  if(b_cross_the_streams == true) {
    i_cross_the_streams = 2;
  }

  if(b_cross_the_streams_mix == true) {
    i_cross_the_streams_mix = 2;
  }

  if(b_overheat_enabled == true) {
    i_overheating = 2;
  }

  if(b_extra_pack_sounds == true) {
    i_neutrona_wand_sounds = 2;
  }

  if(b_spectral_mode_enabled == true || b_holiday_mode_enabled == true) {
    i_spectral = 2;
    i_holiday = 2;
  }

  if(b_quick_vent == true) {
    i_quick_vent = 2;
  }

  if(b_wand_boot_errors == true) {
    i_wand_boot_errors = 2;
  }

  if(b_vent_light_control == true) {
    i_vent_light_auto_intensity = 2;
  }

  if(b_bargraph_invert == true) {
    i_invert_bargraph = 2;
  }

  switch(BARGRAPH_MODE_EEPROM) {
    case BARGRAPH_EEPROM_SUPER_HERO:
      // 2 = BARGRAPH_EEPROM_SUPER_HERO
      i_bargraph_mode = 2;
    break;

    case BARGRAPH_EEPROM_ORIGINAL:
      // 3 = BARGRAPH_EEPROM_ORIGINAL
      i_bargraph_mode = 3;
    break;

    case BARGRAPH_EEPROM_DEFAULT:
    default:
      // The system chooses which bargraph mode when set to this. This is the default setting.
      // 1 = BARGRAPH_EEPROM_DEFAULT
      i_bargraph_mode = 1;
    break;
  }

  switch(BARGRAPH_EEPROM_FIRING_ANIMATION) {
    case BARGRAPH_EEPROM_ANIMATION_SUPER_HERO:
      // 2 = BARGRAPH_EEPROM_ANIMATION_SUPER_HERO
      i_bargraph_firing_animation = 2;
    break;

    case BARGRAPH_EEPROM_ANIMATION_ORIGINAL:
      // 3 = BARGRAPH_EEPROM_ANIMATION_ORIGINAL
      i_bargraph_firing_animation = 3;
    break;

    case BARGRAPH_EEPROM_ANIMATION_DEFAULT:
    default:
      // The system chooses which bargraph firing animation when set to this. This is the default setting.
      // 1 = BARGRAPH_EEPROM_ANIMATION_DEFAULT
      i_bargraph_firing_animation = 1;
    break;
  }

  if(b_overheat_bargraph_blink == true) {
    i_bargraph_overheat_blinking = 2;
  }

  switch(WAND_YEAR_MODE) {
    case YEAR_FROZEN_EMPIRE:
      i_neutrona_wand_year_mode = 5;
    break;

    case YEAR_AFTERLIFE:
      i_neutrona_wand_year_mode = 4;
    break;

    case YEAR_1989:
      i_neutrona_wand_year_mode = 3;
    break;

    case YEAR_1984:
      i_neutrona_wand_year_mode = 2;
    break;

    case YEAR_DEFAULT:
    default:
      i_neutrona_wand_year_mode = 1;
    break;
  }

  switch(WAND_YEAR_CTS) {
    case CTS_FROZEN_EMPIRE:
      i_CTS_mode = 5;
    break;

    case CTS_AFTERLIFE:
      i_CTS_mode = 4;
    break;

    case CTS_1989:
      i_CTS_mode = 3;
    break;

    case CTS_1984:
      i_CTS_mode = 2;
    break;

    case CTS_DEFAULT:
    default:
      i_CTS_mode = 1;
    break;
  }

  if(b_beep_loop != true) {
    i_beep_loop = 1;
  }

  if(b_overheat_mode_5 == true) {
    i_overheat_level_5 = 2;
  }

  if(b_overheat_mode_4 == true) {
    i_overheat_level_4 = 2;
  }

  if(b_overheat_mode_3 == true) {
    i_overheat_level_3 = 2;
  }

  if(b_overheat_mode_2 == true) {
    i_overheat_level_2 = 2;
  }

  if(b_overheat_mode_1 == true) {
    i_overheat_level_1 = 2;
  }

  // Write the data to the EEPROM if any of the values have changed.
  objEEPROM obj_eeprom = {
    i_cross_the_streams,
    i_cross_the_streams_mix,
    i_overheating,
    i_neutrona_wand_sounds,
    i_spectral,
    i_holiday,
    i_quick_vent,
    i_wand_boot_errors,
    i_vent_light_auto_intensity,
    i_num_barrel_leds,
    i_invert_bargraph,
    i_bargraph_mode,
    i_bargraph_firing_animation,
    i_bargraph_overheat_blinking,
    i_neutrona_wand_year_mode,
    i_CTS_mode,
    i_beep_loop,
    i_overheat_start_timer_level_5,
    i_overheat_start_timer_level_4,
    i_overheat_start_timer_level_3,
    i_overheat_start_timer_level_2,
    i_overheat_start_timer_level_1,
    i_overheat_level_5,
    i_overheat_level_4,
    i_overheat_level_3,
    i_overheat_level_2,
    i_overheat_level_1
  };

  // Save and update our object in the EEPROM.
  EEPROM.put(i_eepromAddress, obj_eeprom);

  updateCRCEEPROM();
}

// Update the CRC in the EEPROM.
void updateCRCEEPROM() {
  EEPROM.put(EEPROM.length() - sizeof(l_crc_size), eepromCRC());
}

unsigned long eepromCRC(void) {
  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  unsigned long crc = l_crc_size;

  for(unsigned int index = 0; index < EEPROM.length() - sizeof(crc); ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }

  return crc;
}