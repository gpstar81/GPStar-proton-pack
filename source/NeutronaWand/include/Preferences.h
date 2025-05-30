/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
void clearLEDEEPROM();
void saveConfigEEPROM();
void saveLEDEEPROM();
void updateCRCEEPROM();
uint32_t eepromCRC(void);
void bargraphYearModeUpdate();
void resetOverheatLevels();
void resetWhiteLEDBlinkRate();

/*
 * General EEPROM Variables
 */
const uint16_t i_eepromAddress = 0; // The address in the EEPROM to start reading from.

/*
 * Data structure object for LED settings which are saved into the EEPROM memory.
 */
struct objLEDEEPROM {
  uint8_t barrel_spectral_custom;
  uint8_t barrel_spectral_saturation_custom; // Unused
  uint8_t num_barrel_leds;
  uint8_t num_bargraph_leds;
  uint8_t rgb_vent_light;
};

/*
 * Data structure object for customizations which are saved into the EEPROM memory.
 */
struct objConfigEEPROM {
  uint8_t cross_the_streams;
  uint8_t cross_the_streams_mix;
  uint8_t overheating;
  uint8_t extra_proton_sounds;
  uint8_t neutrona_wand_sounds;
  uint8_t spectral_mode;
  uint8_t holiday_mode; // This will be deprecated in 6.0 as part of a new menu refactoring.
  uint8_t quick_vent;
  uint8_t wand_boot_errors;
  uint8_t vent_light_auto_intensity;
  uint8_t invert_bargraph;
  uint8_t bargraph_mode;
  uint8_t bargraph_firing_animation;
  uint8_t bargraph_overheat_blinking;
  uint8_t neutrona_wand_year_mode;
  uint8_t CTS_mode;
  uint8_t system_mode;
  uint8_t beep_loop;
  uint8_t default_wand_volume;
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
  uint8_t wand_vibration;
};

/*
 * Read all user preferences from Proton Pack controller EEPROM.
 */
void readEEPROM() {
  // Get the stored CRC from the EEPROM.
  uint32_t l_crc_check;
  EEPROM.get(EEPROM.length() - sizeof(eepromCRC()), l_crc_check);

  // Check if the calculated CRC matches the stored CRC value in the EEPROM.
  if(eepromCRC() == l_crc_check) {
    // Read our object from the EEPROM.
    objConfigEEPROM obj_config_eeprom;
    EEPROM.get(i_eepromAddress, obj_config_eeprom);

    if(obj_config_eeprom.cross_the_streams == 2) {
      FIRING_MODE = CTS_MODE; // At least the CTS mode is enabled.

      if(obj_config_eeprom.cross_the_streams_mix == 2) {
        FIRING_MODE = CTS_MIX_MODE; // Upgrade to the CTS Mix mode.
      }

      // Remember this as the last firing mode as well.
      LAST_FIRING_MODE = FIRING_MODE;
    }

    if(obj_config_eeprom.overheating > 0 && obj_config_eeprom.overheating < 3) {
      if(obj_config_eeprom.overheating > 1) {
        b_overheat_enabled = true;
      }
      else {
        b_overheat_enabled = false;
      }
    }

    if(obj_config_eeprom.extra_proton_sounds > 0 && obj_config_eeprom.extra_proton_sounds < 3) {
      if(obj_config_eeprom.extra_proton_sounds > 1) {
        b_stream_effects = true;
      }
      else {
        b_stream_effects = false;
      }
    }

    if(obj_config_eeprom.neutrona_wand_sounds > 0 && obj_config_eeprom.neutrona_wand_sounds < 3) {
      if(obj_config_eeprom.neutrona_wand_sounds > 1) {
        b_extra_pack_sounds = true;
      }
      else {
        b_extra_pack_sounds = false;
      }
    }

    if(obj_config_eeprom.spectral_mode > 0 && obj_config_eeprom.spectral_mode < 3) {
      if(obj_config_eeprom.spectral_mode > 1) {
        b_spectral_mode_enabled = true;
        b_spectral_custom_mode_enabled = true;
        b_holiday_mode_enabled = true;
      }
      else {
        b_spectral_mode_enabled = false;
        b_spectral_custom_mode_enabled = false;
        b_holiday_mode_enabled = false;
      }
    }

    if(obj_config_eeprom.quick_vent > 0 && obj_config_eeprom.quick_vent < 3) {
      if(obj_config_eeprom.quick_vent > 1) {
        b_quick_vent = true;
      }
      else {
        b_quick_vent = false;
      }
    }

    if(obj_config_eeprom.wand_boot_errors > 0 && obj_config_eeprom.wand_boot_errors < 3) {
      if(obj_config_eeprom.wand_boot_errors > 1) {
        b_wand_boot_errors = true;
      }
      else {
        b_wand_boot_errors = false;
      }
    }

    if(obj_config_eeprom.vent_light_auto_intensity > 0 && obj_config_eeprom.vent_light_auto_intensity < 3) {
      if(obj_config_eeprom.vent_light_auto_intensity > 1) {
        b_vent_light_control = true;
      }
      else {
        b_vent_light_control = false;
      }
    }

    if(obj_config_eeprom.invert_bargraph > 0 && obj_config_eeprom.invert_bargraph < 3) {
      if(obj_config_eeprom.invert_bargraph > 1) {
        b_bargraph_invert = true;
      }
      else {
        b_bargraph_invert = false;
      }
    }

    if(obj_config_eeprom.bargraph_mode > 0 && obj_config_eeprom.bargraph_mode < 4) {
      switch(obj_config_eeprom.bargraph_mode) {
        case 1:
        default:
          BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_DEFAULT;
        break;

        case 2:
          BARGRAPH_MODE = BARGRAPH_SUPER_HERO;
          BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_SUPER_HERO;
        break;

        case 3:
          BARGRAPH_MODE = BARGRAPH_ORIGINAL;
          BARGRAPH_MODE_EEPROM = BARGRAPH_EEPROM_ORIGINAL;
        break;
      }
    }

    if(obj_config_eeprom.bargraph_firing_animation > 0 && obj_config_eeprom.bargraph_mode < 4) {
      switch(obj_config_eeprom.bargraph_firing_animation) {
        case 1:
        default:
          BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_DEFAULT;
        break;

        case 2:
          BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_SUPER_HERO;
          BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_SUPER_HERO;
        break;

        case 3:
          BARGRAPH_FIRING_ANIMATION = BARGRAPH_ANIMATION_ORIGINAL;
          BARGRAPH_EEPROM_FIRING_ANIMATION = BARGRAPH_EEPROM_ANIMATION_ORIGINAL;
        break;
      }
    }

    if(obj_config_eeprom.bargraph_overheat_blinking > 0 && obj_config_eeprom.bargraph_overheat_blinking < 3) {
      if(obj_config_eeprom.bargraph_overheat_blinking > 1) {
        b_overheat_bargraph_blink = true;
      }
      else {
        b_overheat_bargraph_blink = false;
      }
    }

    if(obj_config_eeprom.neutrona_wand_year_mode > 0 && obj_config_eeprom.neutrona_wand_year_mode < 6) {
      switch(obj_config_eeprom.neutrona_wand_year_mode) {
        case 1:
        default:
          WAND_YEAR_MODE = YEAR_DEFAULT;
        break;
        case 2:
          WAND_YEAR_MODE = YEAR_1984;
        break;
        case 3:
          WAND_YEAR_MODE = YEAR_1989;
        break;
        case 4:
          WAND_YEAR_MODE = YEAR_AFTERLIFE;
        break;
        case 5:
          WAND_YEAR_MODE = YEAR_FROZEN_EMPIRE;
        break;
      }
    }

    if(obj_config_eeprom.CTS_mode > 0 && obj_config_eeprom.CTS_mode < 6) {
      switch(obj_config_eeprom.CTS_mode) {
        case 1:
        default:
          WAND_YEAR_CTS = CTS_DEFAULT;
        break;
        case 2:
          WAND_YEAR_CTS = CTS_1984;
        break;
        case 4:
          WAND_YEAR_CTS = CTS_AFTERLIFE;
        break;
      }
    }

    if(obj_config_eeprom.system_mode > 0 && obj_config_eeprom.system_mode < 3 && b_gpstar_benchtest) {
      if(obj_config_eeprom.system_mode > 1) {
        SYSTEM_MODE = MODE_ORIGINAL;
      }
      else {
        SYSTEM_MODE = MODE_SUPER_HERO;
      }
    }

    if(obj_config_eeprom.beep_loop > 0 && obj_config_eeprom.beep_loop < 3) {
      if(obj_config_eeprom.beep_loop > 1) {
        b_beep_loop = true;
      }
      else {
        b_beep_loop = false;
      }
    }

    if(obj_config_eeprom.default_wand_volume > 0 && obj_config_eeprom.default_wand_volume < 102 && b_gpstar_benchtest) {
      // EEPROM value is from 1 to 101; subtract 1 to get the correct percentage.
      i_volume_master_percentage = obj_config_eeprom.default_wand_volume - 1;
      i_volume_master_eeprom = MINIMUM_VOLUME - ((MINIMUM_VOLUME - i_volume_abs_max) * i_volume_master_percentage / 100);
      i_volume_revert = i_volume_master_eeprom;
      i_volume_master = i_volume_master_eeprom;
    }

    if(obj_config_eeprom.overheat_start_timer_level_5 > 0 && obj_config_eeprom.overheat_start_timer_level_5 < 61) {
      i_ms_overheat_initiate_level_5 = obj_config_eeprom.overheat_start_timer_level_5 * 1000;

      i_ms_overheat_initiate[4] = i_ms_overheat_initiate_level_5;
    }

    if(obj_config_eeprom.overheat_start_timer_level_4 > 0 && obj_config_eeprom.overheat_start_timer_level_4 < 61) {
      i_ms_overheat_initiate_level_4 = obj_config_eeprom.overheat_start_timer_level_4 * 1000;

      i_ms_overheat_initiate[3] = i_ms_overheat_initiate_level_4;
    }

    if(obj_config_eeprom.overheat_start_timer_level_3 > 0 && obj_config_eeprom.overheat_start_timer_level_3 < 61) {
      i_ms_overheat_initiate_level_3 = obj_config_eeprom.overheat_start_timer_level_3 * 1000;

      i_ms_overheat_initiate[2] = i_ms_overheat_initiate_level_3;
    }

    if(obj_config_eeprom.overheat_start_timer_level_2 > 0 && obj_config_eeprom.overheat_start_timer_level_2 < 61) {
      i_ms_overheat_initiate_level_2 = obj_config_eeprom.overheat_start_timer_level_2 * 1000;

      i_ms_overheat_initiate[1] = i_ms_overheat_initiate_level_2;
    }

    if(obj_config_eeprom.overheat_start_timer_level_1 > 0 && obj_config_eeprom.overheat_start_timer_level_1 < 61) {
      i_ms_overheat_initiate_level_1 = obj_config_eeprom.overheat_start_timer_level_1 * 1000;

      i_ms_overheat_initiate[0] = i_ms_overheat_initiate_level_1;
    }

    if(obj_config_eeprom.overheat_level_5 > 0 && obj_config_eeprom.overheat_level_5 < 3) {
      if(obj_config_eeprom.overheat_level_5 > 1) {
        b_overheat_level_5 = true;
      }
      else {
        b_overheat_level_5 = false;
      }
    }

    if(obj_config_eeprom.overheat_level_4 > 0 && obj_config_eeprom.overheat_level_4 < 3) {
      if(obj_config_eeprom.overheat_level_4 > 1) {
        b_overheat_level_4 = true;
      }
      else {
        b_overheat_level_4 = false;
      }
    }

    if(obj_config_eeprom.overheat_level_3 > 0 && obj_config_eeprom.overheat_level_3 < 3) {
      if(obj_config_eeprom.overheat_level_3 > 1) {
        b_overheat_level_3 = true;
      }
      else {
        b_overheat_level_3 = false;
      }
    }

    if(obj_config_eeprom.overheat_level_2 > 0 && obj_config_eeprom.overheat_level_2 < 3) {
      if(obj_config_eeprom.overheat_level_2 > 1) {
        b_overheat_level_2 = true;
      }
      else {
        b_overheat_level_2 = false;
      }
    }

    if(obj_config_eeprom.overheat_level_1 > 0 && obj_config_eeprom.overheat_level_1 < 3) {
      if(obj_config_eeprom.overheat_level_1 > 1) {
        b_overheat_level_1 = true;
      }
      else {
        b_overheat_level_1 = false;
      }
    }

    if(obj_config_eeprom.wand_vibration > 0 && obj_config_eeprom.wand_vibration < 5) {
      switch(obj_config_eeprom.wand_vibration) {
        case 4:
        default:
          // Do nothing. Readings are taken from the vibration toggle switch from the Proton pack or configuration setting in stand alone mode.
          VIBRATION_MODE_EEPROM = VIBRATION_DEFAULT;
          VIBRATION_MODE = VIBRATION_FIRING_ONLY;
        break;

        case 3:
          VIBRATION_MODE_EEPROM = VIBRATION_NONE;
          VIBRATION_MODE = VIBRATION_MODE_EEPROM;
        break;

        case 2:
          b_vibration_switch_on = true; // Override the Proton Pack vibration toggle switch.
          VIBRATION_MODE_EEPROM = VIBRATION_FIRING_ONLY;
          VIBRATION_MODE = VIBRATION_MODE_EEPROM;
        break;

        case 1:
          b_vibration_switch_on = true; // Override the Proton Pack vibration toggle switch.
          VIBRATION_MODE_EEPROM = VIBRATION_ALWAYS;
          VIBRATION_MODE = VIBRATION_MODE_EEPROM;
        break;
      }
    }

    // Rebuild the overheat enabled power levels.
    resetOverheatLevels();

    // Reset the blinking white LED interval.
    resetWhiteLEDBlinkRate();

    // Read our LED object from the EEPROM.
    objLEDEEPROM obj_led_eeprom;
    uint16_t i_eepromLEDAddress = i_eepromAddress + sizeof(objConfigEEPROM);

    EEPROM.get(i_eepromLEDAddress, obj_led_eeprom);

    if(obj_led_eeprom.barrel_spectral_custom > 0 && obj_led_eeprom.barrel_spectral_custom != 255) {
      i_spectral_wand_custom_colour = obj_led_eeprom.barrel_spectral_custom;
    }

    if(obj_led_eeprom.barrel_spectral_saturation_custom > 0 && obj_led_eeprom.barrel_spectral_saturation_custom != 255) {
      i_spectral_wand_custom_saturation = obj_led_eeprom.barrel_spectral_saturation_custom;
    }

    if(obj_led_eeprom.num_barrel_leds == 2 || obj_led_eeprom.num_barrel_leds == 5 ||
      obj_led_eeprom.num_barrel_leds == 48 || obj_led_eeprom.num_barrel_leds == 50) {
      i_num_barrel_leds = obj_led_eeprom.num_barrel_leds;

      switch(i_num_barrel_leds) {
        case 5:
        default:
          WAND_BARREL_LED_COUNT = LEDS_5;
        break;

        case 2:
          WAND_BARREL_LED_COUNT = LEDS_2;
        break;

        case 48:
          WAND_BARREL_LED_COUNT = LEDS_48;
        break;

        case 50:
          WAND_BARREL_LED_COUNT = LEDS_50;
          i_num_barrel_leds = 48; // Need to reset it to 48. 2 are for the tip.
        break;
      }
    }

    if(obj_led_eeprom.num_bargraph_leds == 28 || obj_led_eeprom.num_bargraph_leds == 30) {
      if(obj_led_eeprom.num_bargraph_leds < 30) {
        BARGRAPH_TYPE_EEPROM = SEGMENTS_28;
      }
      else {
        BARGRAPH_TYPE_EEPROM = SEGMENTS_30;
      }

      // Only override the bargraph LED count if we are not using a stock bargraph.
      BARGRAPH_TYPE = BARGRAPH_TYPE_EEPROM;
    }

    if(obj_led_eeprom.rgb_vent_light > 0 && obj_led_eeprom.rgb_vent_light < 3) {
      if(obj_led_eeprom.rgb_vent_light > 1) {
        b_rgb_vent_light = true;
      }
      else {
        b_rgb_vent_light = false;
      }
    }
  }
  else {
    // CRC doesn't match; let's clear the EEPROMs to be safe.
    playEffect(S_VOICE_EEPROM_LOADING_FAILED_RESET);

    clearConfigEEPROM();
    clearLEDEEPROM();
  }
}

void clearLEDEEPROM() {
  // Clear out the EEPROM data for the configuration settings only.
  uint16_t i_eepromLEDAddress = i_eepromAddress + sizeof(objConfigEEPROM);

  for(; i_eepromLEDAddress < sizeof(objLEDEEPROM); i_eepromLEDAddress++) {
    EEPROM.update(i_eepromLEDAddress, 0xFF); // Write 0xFF to each address
  }

  updateCRCEEPROM();
}

void saveLEDEEPROM() {
  uint16_t i_eepromLEDAddress = i_eepromAddress + sizeof(objConfigEEPROM);

  uint8_t i_barrel_led_count = 5; // 5 = Hasbro, 50 = GPStar Neutrona Barrel, 2 = GPStar Barrel LED Mini, 48 = Frutto.
  uint8_t i_bargraph_led_count = 28; // 28 segment, 30 segment.
  uint8_t i_rgb_vent_light = 1; // 1 = RGB Vent Light disabled, 2 = RGB Vent Light enabled

  if(WAND_BARREL_LED_COUNT == LEDS_48) {
    i_barrel_led_count = 48;
  }
  else if(WAND_BARREL_LED_COUNT == LEDS_50) {
    i_barrel_led_count = 50;
  }
  else if(WAND_BARREL_LED_COUNT == LEDS_2) {
    i_barrel_led_count = 2;
  }

  if(BARGRAPH_TYPE_EEPROM == SEGMENTS_30) {
    i_bargraph_led_count = 30;
  }

  if(b_rgb_vent_light) {
    i_rgb_vent_light = 2;
  }

  // Build the LED EEPROM object with the new data.
  objLEDEEPROM obj_led_eeprom = {
    i_spectral_wand_custom_colour,
    i_spectral_wand_custom_saturation,
    i_barrel_led_count,
    i_bargraph_led_count,
    i_rgb_vent_light
  };

  // Save to the EEPROM.
  EEPROM.put(i_eepromLEDAddress, obj_led_eeprom);

  updateCRCEEPROM();
}

void clearConfigEEPROM() {
  // Clear out the EEPROM only in the memory addresses used for our EEPROM data object.
  for(uint16_t i = i_eepromAddress; i < sizeof(objConfigEEPROM); i++) {
    EEPROM.update(i, 0xFF); // Write 0xFF to each address
  }

  updateCRCEEPROM();
}

void saveConfigEEPROM() {
  // Convert the current EEPROM volume value into a percentage.
  uint8_t i_eeprom_volume_master_percentage = 100 * (MINIMUM_VOLUME - i_volume_master_eeprom) / MINIMUM_VOLUME;

  // 1 = false, 2 = true.
  uint8_t i_cross_the_streams = 1;
  uint8_t i_cross_the_streams_mix = 1;
  uint8_t i_overheating = 2;
  uint8_t i_extra_proton_sounds = 2;
  uint8_t i_neutrona_wand_sounds = 2;
  uint8_t i_spectral = 1;
  uint8_t i_quick_vent = 2;
  uint8_t i_wand_boot_errors = 2;
  uint8_t i_vent_light_auto_intensity = 2;
  uint8_t i_invert_bargraph = 1;
  uint8_t i_bargraph_mode = 1; // 1 = default, 2 = super hero, 3 = original.
  uint8_t i_bargraph_firing_animation = 1; // 1 = default, 2 = super hero, 3 = original.
  uint8_t i_bargraph_overheat_blinking = 1;
  uint8_t i_neutrona_wand_year_mode = 1; // 1 = default, 2 = 1984, 3 = 1989, 4 = Afterlife, 5 = Frozen Empire.
  uint8_t i_CTS_mode = 1; // 1 = default, 2 = 1984, 3 = 1989, 4 = Afterlife, 5 = Frozen Empire.
  uint8_t i_system_mode = 1; // 1 = super hero, 2 = original.
  uint8_t i_beep_loop = 2;
  uint8_t i_default_wand_volume = 101; // <- i_eeprom_volume_master_percentage + 1
  uint8_t i_overheat_start_timer_level_5 = i_ms_overheat_initiate_level_5 / 1000;
  uint8_t i_overheat_start_timer_level_4 = i_ms_overheat_initiate_level_4 / 1000;
  uint8_t i_overheat_start_timer_level_3 = i_ms_overheat_initiate_level_3 / 1000;
  uint8_t i_overheat_start_timer_level_2 = i_ms_overheat_initiate_level_2 / 1000;
  uint8_t i_overheat_start_timer_level_1 = i_ms_overheat_initiate_level_1 / 1000;
  uint8_t i_overheat_level_5 = 1;
  uint8_t i_overheat_level_4 = 1;
  uint8_t i_overheat_level_3 = 1;
  uint8_t i_overheat_level_2 = 1;
  uint8_t i_overheat_level_1 = 1;
  uint8_t i_wand_vibration = 4; // 1 = always, 2 = when firing, 3 = off, 4 = default.

  if(FIRING_MODE == CTS_MODE || FIRING_MODE == CTS_MIX_MODE) {
    i_cross_the_streams = 2;
  }

  if(FIRING_MODE == CTS_MIX_MODE) {
    i_cross_the_streams_mix = 2;
  }

  if(!b_overheat_enabled) {
    i_overheating = 1;
  }

  if(!b_stream_effects) {
    i_extra_proton_sounds = 1;
  }

  if(!b_extra_pack_sounds) {
    i_neutrona_wand_sounds = 1;
  }

  if(b_spectral_mode_enabled) {
    i_spectral = 2;
  }

  if(!b_quick_vent) {
    i_quick_vent = 1;
  }

  if(!b_wand_boot_errors) {
    i_wand_boot_errors = 1;
  }

  if(!b_vent_light_control) {
    i_vent_light_auto_intensity = 1;
  }

  if(b_bargraph_invert) {
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

  if(b_overheat_bargraph_blink) {
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
    case CTS_AFTERLIFE:
      i_CTS_mode = 4;
    break;
    case CTS_1984:
      i_CTS_mode = 2;
    break;
    case CTS_DEFAULT:
    default:
      i_CTS_mode = 1;
    break;
  }

  if(SYSTEM_MODE == MODE_ORIGINAL) {
    i_system_mode = 2;
  }

  if(i_eeprom_volume_master_percentage <= 100) {
    // Need to add 1 to this because the EEPROM cannot contain a 0 value.
    i_default_wand_volume = i_eeprom_volume_master_percentage + 1;
  }

  if(!b_beep_loop) {
    i_beep_loop = 1;
  }

  if(b_overheat_level_5) {
    i_overheat_level_5 = 2;
  }

  if(b_overheat_level_4) {
    i_overheat_level_4 = 2;
  }

  if(b_overheat_level_3) {
    i_overheat_level_3 = 2;
  }

  if(b_overheat_level_2) {
    i_overheat_level_2 = 2;
  }

  if(b_overheat_level_1) {
    i_overheat_level_1 = 2;
  }

  switch(VIBRATION_MODE_EEPROM) {
    case VIBRATION_ALWAYS:
      i_wand_vibration = 1;
    break;

    case VIBRATION_FIRING_ONLY:
      i_wand_vibration = 2;
    break;

    case VIBRATION_NONE:
      i_wand_vibration = 3;
    break;

    case VIBRATION_DEFAULT:
    default:
      i_wand_vibration = 4;
    break;
  }

  // Write the data to the EEPROM if any of the values have changed.
  objConfigEEPROM obj_config_eeprom = {
    i_cross_the_streams,
    i_cross_the_streams_mix,
    i_overheating,
    i_extra_proton_sounds,
    i_neutrona_wand_sounds,
    i_spectral,
    0,
    i_quick_vent,
    i_wand_boot_errors,
    i_vent_light_auto_intensity,
    i_invert_bargraph,
    i_bargraph_mode,
    i_bargraph_firing_animation,
    i_bargraph_overheat_blinking,
    i_neutrona_wand_year_mode,
    i_CTS_mode,
    i_system_mode,
    i_beep_loop,
    i_default_wand_volume,
    i_overheat_start_timer_level_5,
    i_overheat_start_timer_level_4,
    i_overheat_start_timer_level_3,
    i_overheat_start_timer_level_2,
    i_overheat_start_timer_level_1,
    i_overheat_level_5,
    i_overheat_level_4,
    i_overheat_level_3,
    i_overheat_level_2,
    i_overheat_level_1,
    i_wand_vibration
  };

  // Save and update our object in the EEPROM.
  EEPROM.put(i_eepromAddress, obj_config_eeprom);

  updateCRCEEPROM();
}

// Update the CRC in the EEPROM.
void updateCRCEEPROM() {
  EEPROM.put(EEPROM.length() - sizeof(eepromCRC()), eepromCRC());
}

uint32_t eepromCRC(void) {
  CRC32 crc;

  for(uint16_t index = i_eepromAddress; index < (i_eepromAddress + sizeof(objConfigEEPROM) + sizeof(objLEDEEPROM)); index++) {
    crc.update(EEPROM[index]);
  }

  crc.update(sizeof(objConfigEEPROM));
  crc.update(sizeof(objLEDEEPROM));

  return (uint32_t)crc.finalize();
}
