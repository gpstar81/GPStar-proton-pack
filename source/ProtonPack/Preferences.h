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
void resetCyclotronLEDs();
void resetContinuousSmoke();
void updateProtonPackLEDCounts();

/*
 * General EEPROM Variables
 */
unsigned int i_eepromAddress = 0; // The address in the EEPROM to start reading from.
unsigned long l_crc_size = ~0L; // The 4 last bytes are reserved for storing the CRC.

/*
 * Data structure object for LED settings which are saved into the EEPROM memory.
 */
struct objLEDEEPROM {
  uint8_t powercell_count;
  uint8_t cyclotron_count;
  uint8_t inner_cyclotron_count;
  uint8_t grb_inner_cyclotron;
  uint8_t powercell_spectral_custom;
  uint8_t cyclotron_spectral_custom;
  uint8_t cyclotron_inner_spectral_custom;
  uint8_t powercell_spectral_saturation_custom;
  uint8_t cyclotron_spectral_saturation_custom;
  uint8_t cyclotron_inner_spectral_saturation_custom;
};

/*
 * Data structure object for customizations which are saved into the EEPROM memory.
 */
struct objConfigEEPROM {
  uint8_t stream_effects;
  uint8_t cyclotron_direction;
  uint8_t simulate_ring; // Primarily for the Afterlife/Frozen Empire themes.
  uint8_t smoke_setting;
  uint8_t overheat_strobe;
  uint8_t overheat_lights_off;
  uint8_t overheat_sync_to_fan;

  uint8_t year_mode; // 1984, 1989, Afterlife, Frozen Empire or the Proton Pack toggle switch default.
  uint8_t system_mode; // Super Hero or Mode Original.
  uint8_t vga_powercell; // For disabling or enabling video game colours for the Power Cell.
  uint8_t vga_cyclotron; // For disabling or enabling video game colours for the Cyclotron.
  uint8_t demo_light_mode; // Enables pack startup automatically at bootup (battery power-on).
  uint8_t cyclotron_three_led_toggle; // Toggles between the 1-LED or 3-LED for 84/89 modes.
  uint8_t default_system_volume; // Default master volume at bootup (battery power-on)

  uint8_t overheat_smoke_duration_level_5;
  uint8_t overheat_smoke_duration_level_4;
  uint8_t overheat_smoke_duration_level_3;
  uint8_t overheat_smoke_duration_level_2;
  uint8_t overheat_smoke_duration_level_1;

  uint8_t smoke_continuous_mode_5;
  uint8_t smoke_continuous_mode_4;
  uint8_t smoke_continuous_mode_3;
  uint8_t smoke_continuous_mode_2;
  uint8_t smoke_continuous_mode_1;
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
    objLEDEEPROM obj_eeprom;
    EEPROM.get(i_eepromAddress, obj_eeprom);

    if(obj_eeprom.powercell_count > 0 && obj_eeprom.powercell_count != 255) {
      i_powercell_leds = obj_eeprom.powercell_count;

      switch(i_powercell_leds) {
        case HASLAB_POWERCELL_LED_COUNT:
          // 15 Power Cell LEDs.
          i_powercell_delay_1984 = 60;
          i_powercell_delay_2021 = 34;
        break;

        case FRUTTO_POWERCELL_LED_COUNT:
        default:
          // 13 Power Cell LEDs.
          i_powercell_delay_1984 = 75;
          i_powercell_delay_2021 = 40;
        break;
      }
    }

    if(obj_eeprom.cyclotron_count > 0 && obj_eeprom.cyclotron_count != 255) {
      i_cyclotron_leds = obj_eeprom.cyclotron_count;
      resetCyclotronLEDs();
    }

    if(obj_eeprom.inner_cyclotron_count > 0 && obj_eeprom.inner_cyclotron_count != 255) {
      i_inner_cyclotron_num_leds = obj_eeprom.inner_cyclotron_count;

      switch(i_inner_cyclotron_num_leds) {
        case 12:
          i_2021_inner_delay = 12;
          i_1984_inner_delay = 15;
        break;

        case 23:
        case 24:
          i_2021_inner_delay = 8;
          i_1984_inner_delay = 12;
        break;

        case 35:
        default:
          i_2021_inner_delay = 5;
          i_1984_inner_delay = 9;
        break;
      }
    }

    if(obj_eeprom.grb_inner_cyclotron > 0 && obj_eeprom.grb_inner_cyclotron != 255) {
      if(obj_eeprom.grb_inner_cyclotron > 1) {
        b_grb_cyclotron = true;
      }
      else {
        b_grb_cyclotron = false;
      }
    }

    if(obj_eeprom.powercell_spectral_custom > 0 && obj_eeprom.powercell_spectral_custom != 255) {
      i_spectral_powercell_custom = obj_eeprom.powercell_spectral_custom;
    }

    if(obj_eeprom.cyclotron_spectral_custom > 0 && obj_eeprom.cyclotron_spectral_custom != 255) {
      i_spectral_cyclotron_custom = obj_eeprom.cyclotron_spectral_custom;
    }

    if(obj_eeprom.cyclotron_inner_spectral_custom > 0 && obj_eeprom.cyclotron_inner_spectral_custom != 255) {
      i_spectral_cyclotron_inner_custom = obj_eeprom.cyclotron_inner_spectral_custom;
    }

    if(obj_eeprom.powercell_spectral_saturation_custom > 0 && obj_eeprom.powercell_spectral_saturation_custom != 255) {
      i_spectral_powercell_custom_saturation = obj_eeprom.powercell_spectral_saturation_custom;
    }

    if(obj_eeprom.cyclotron_spectral_saturation_custom > 0 && obj_eeprom.cyclotron_spectral_saturation_custom != 255) {
      i_spectral_cyclotron_custom_saturation = obj_eeprom.cyclotron_spectral_saturation_custom;
    }

    if(obj_eeprom.cyclotron_inner_spectral_saturation_custom > 0 && obj_eeprom.cyclotron_inner_spectral_saturation_custom != 255) {
      i_spectral_cyclotron_inner_custom_saturation = obj_eeprom.cyclotron_inner_spectral_saturation_custom;
    }

    // Update the LED counts for the Proton Pack.
    updateProtonPackLEDCounts();

    // Read our configuration object from the EEPROM.
    objConfigEEPROM obj_config_eeprom;
    unsigned int i_eepromConfigAddress = EEPROM.length() / 2;

    EEPROM.get(i_eepromConfigAddress, obj_config_eeprom);

    if(obj_config_eeprom.stream_effects > 0 && obj_config_eeprom.stream_effects != 255) {
      if(obj_config_eeprom.stream_effects > 1) {
        b_stream_effects = true;
      }
      else {
        b_stream_effects = false;
      }
    }

    if(obj_config_eeprom.simulate_ring > 0 && obj_config_eeprom.simulate_ring != 255) {
      if(obj_config_eeprom.simulate_ring > 1) {
        b_cyclotron_simulate_ring = true;
      }
      else {
        b_cyclotron_simulate_ring = false;
      }
    }

    if(obj_config_eeprom.cyclotron_direction > 0 && obj_config_eeprom.cyclotron_direction != 255) {
      if(obj_config_eeprom.cyclotron_direction > 1) {
        b_clockwise = true;
      }
      else {
        b_clockwise = false;
      }
    }

    if(obj_config_eeprom.smoke_setting > 0 && obj_config_eeprom.smoke_setting != 255) {
      if(obj_config_eeprom.smoke_setting > 1) {
        b_smoke_enabled = true;
      }
      else {
        b_smoke_enabled = false;
      }
    }

    if(obj_config_eeprom.overheat_strobe > 0 && obj_config_eeprom.overheat_strobe != 255) {
      if(obj_config_eeprom.overheat_strobe > 1) {
        b_overheat_strobe = true;
      }
      else {
        b_overheat_strobe = false;
      }
    }

    if(obj_config_eeprom.overheat_lights_off > 0 && obj_config_eeprom.overheat_lights_off != 255) {
      if(obj_config_eeprom.overheat_lights_off > 1) {
        b_overheat_lights_off = true;
      }
      else {
        b_overheat_lights_off = false;
      }
    }

    if(obj_config_eeprom.overheat_sync_to_fan > 0 && obj_config_eeprom.overheat_sync_to_fan != 255) {
      if(obj_config_eeprom.overheat_sync_to_fan > 1) {
        b_overheat_sync_to_fan = true;
      }
      else {
        b_overheat_sync_to_fan = false;
      }
    }

    if(obj_config_eeprom.year_mode > 0 && obj_config_eeprom.year_mode != 255) {
      if(obj_config_eeprom.year_mode > 1) {
        // 1 = toggle switch, 2 = 1984, 3 = 1989, 4 = Afterlife, 5 = Frozen Empire.
        switch(obj_config_eeprom.year_mode) {
          case 2:
            SYSTEM_YEAR = SYSTEM_1984;
          break;

          case 3:
            SYSTEM_YEAR = SYSTEM_1989;
          break;

          /*
          case 5:
            SYSTEM_YEAR = SYSTEM_FROZEN_EMPIRE;
          break;
          */
          case 4:
          default:
            SYSTEM_YEAR = SYSTEM_AFTERLIFE;
          break;
        }

        SYSTEM_YEAR_TEMP = SYSTEM_YEAR;
        SYSTEM_EEPROM_YEAR = SYSTEM_YEAR;

        // Set the switch override to true, so the toggle switch in the Proton Pack does not override the year settings during the bootup process.
        b_switch_mode_override = true;
      }
    }
    else {
      // 1 = toggle switch, 2 = 1984, 3 = 1989, 4 = Afterlife, 5 = Frozen Empire.
      SYSTEM_EEPROM_YEAR = SYSTEM_TOGGLE_SWITCH;
    }

    if(obj_config_eeprom.system_mode > 0 && obj_config_eeprom.system_mode != 255) {
      if(obj_config_eeprom.system_mode > 1) {
        SYSTEM_MODE = MODE_ORIGINAL;
      }
      else {
        SYSTEM_MODE = MODE_SUPER_HERO;
      }
    }

    if(obj_config_eeprom.vga_powercell > 0 && obj_config_eeprom.vga_powercell != 255) {
      if(obj_config_eeprom.vga_powercell > 1) {
        b_powercell_colour_toggle = true;
      }
      else {
        b_powercell_colour_toggle = false;
      }
    }

    if(obj_config_eeprom.vga_cyclotron > 0 && obj_config_eeprom.vga_cyclotron != 255) {
      if(obj_config_eeprom.vga_cyclotron > 1) {
        b_cyclotron_colour_toggle = true;
      }
      else {
        b_cyclotron_colour_toggle = false;
      }
    }

    if(obj_config_eeprom.demo_light_mode > 0 && obj_config_eeprom.demo_light_mode != 255) {
      if(obj_config_eeprom.demo_light_mode > 1) {
        b_demo_light_mode = true;
      }
      else {
        b_demo_light_mode = false;
      }
    }

    if(obj_config_eeprom.cyclotron_three_led_toggle > 0 && obj_config_eeprom.cyclotron_three_led_toggle != 255) {
      if(obj_config_eeprom.cyclotron_three_led_toggle > 1) {
        b_cyclotron_single_led = false;
      }
      else {
        b_cyclotron_single_led = true;
      }
    }

    if(obj_config_eeprom.default_system_volume > 0 && obj_config_eeprom.default_system_volume != 255) {
      i_volume_master_percentage = obj_config_eeprom.default_system_volume;
      i_volume_master_eeprom = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
      i_volume_revert = i_volume_master_eeprom;
      i_volume_master = i_volume_master_eeprom;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_5 > 0 && obj_config_eeprom.overheat_smoke_duration_level_5 != 255) {
      i_ms_overheating_length_5 = obj_config_eeprom.overheat_smoke_duration_level_5 * 1000;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_4 > 0 && obj_config_eeprom.overheat_smoke_duration_level_4 != 255) {
      i_ms_overheating_length_4 = obj_config_eeprom.overheat_smoke_duration_level_4 * 1000;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_3 > 0 && obj_config_eeprom.overheat_smoke_duration_level_3 != 255) {
      i_ms_overheating_length_3 = obj_config_eeprom.overheat_smoke_duration_level_3 * 1000;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_2 > 0 && obj_config_eeprom.overheat_smoke_duration_level_2 != 255) {
      i_ms_overheating_length_2 = obj_config_eeprom.overheat_smoke_duration_level_2 * 1000;
    }

    if(obj_config_eeprom.overheat_smoke_duration_level_1 > 0 && obj_config_eeprom.overheat_smoke_duration_level_1 != 255) {
      i_ms_overheating_length_1 = obj_config_eeprom.overheat_smoke_duration_level_1 * 1000;
    }

    if(obj_config_eeprom.smoke_continuous_mode_5 > 0 && obj_config_eeprom.smoke_continuous_mode_5 != 255) {
      if(obj_config_eeprom.smoke_continuous_mode_5 > 1) {
        b_smoke_continuous_mode_5 = true;
      }
      else {
        b_smoke_continuous_mode_5 = false;
      }
    }

    if(obj_config_eeprom.smoke_continuous_mode_4 > 0 && obj_config_eeprom.smoke_continuous_mode_4 != 255) {
      if(obj_config_eeprom.smoke_continuous_mode_4 > 1) {
        b_smoke_continuous_mode_4 = true;
      }
      else {
        b_smoke_continuous_mode_4 = false;
      }
    }

    if(obj_config_eeprom.smoke_continuous_mode_3 > 0 && obj_config_eeprom.smoke_continuous_mode_3 != 255) {
      if(obj_config_eeprom.smoke_continuous_mode_3 > 1) {
        b_smoke_continuous_mode_3 = true;
      }
      else {
        b_smoke_continuous_mode_3 = false;
      }
    }

    if(obj_config_eeprom.smoke_continuous_mode_2 > 0 && obj_config_eeprom.smoke_continuous_mode_2 != 255) {
      if(obj_config_eeprom.smoke_continuous_mode_2 > 1) {
        b_smoke_continuous_mode_2 = true;
      }
      else {
        b_smoke_continuous_mode_2 = false;
      }
    }

    if(obj_config_eeprom.smoke_continuous_mode_1 > 0 && obj_config_eeprom.smoke_continuous_mode_1 != 255) {
      if(obj_config_eeprom.smoke_continuous_mode_1 > 1) {
        b_smoke_continuous_mode_1 = true;
      }
      else {
        b_smoke_continuous_mode_1 = false;
      }
    }
  }

  resetContinuousSmoke();
}

void clearLedEEPROM() {
  // Clear out the EEPROM only in the memory addresses used for our EEPROM data object.
  for(unsigned int i = 0 ; i < sizeof(objLEDEEPROM); i++) {
    EEPROM.put(i, 0);
  }

  updateCRCEEPROM();

  updateProtonPackLEDCounts();
}

void saveLedEEPROM() {
  // Power Cell LEDs
  // Cyclotron LEDs
  // Inner Cyclotron LEDs
  // GRB / RGB Inner Cyclotron toggle flag

  uint8_t i_grb_cyclotron = 1;

  if(b_grb_cyclotron == true) {
    i_grb_cyclotron = 2;
  }

  // Write the data to the EEPROM if any of the values have changed.
  objLEDEEPROM obj_eeprom = {
    i_powercell_leds,
    i_cyclotron_leds,
    i_inner_cyclotron_num_leds,
    i_grb_cyclotron,
    i_spectral_powercell_custom,
    i_spectral_cyclotron_custom,
    i_spectral_cyclotron_inner_custom,
    i_spectral_powercell_custom_saturation,
    i_spectral_cyclotron_custom_saturation,
    i_spectral_cyclotron_inner_custom_saturation,
  };

  // Save and update our object in the EEPROM.
  EEPROM.put(i_eepromAddress, obj_eeprom);

  updateCRCEEPROM();
}

void clearConfigEEPROM() {
  // Clear out the EEPROM data for the configuration settings only.
  unsigned int i_eepromConfigAddress = EEPROM.length() / 2;

  for(unsigned int i = 0 ; i < sizeof(objConfigEEPROM); i++) {
    EEPROM.put(i_eepromConfigAddress, 0);

    i_eepromConfigAddress++;
  }

  updateCRCEEPROM();
}

void saveConfigEEPROM() {
  // 1 = false, 2 = true;
  uint8_t i_proton_stream_effects = 2;
  uint8_t i_simulate_ring = 2;
  uint8_t i_cyclotron_direction = 2;
  uint8_t i_smoke_settings = 2;

  uint8_t i_overheat_strobe = 2;
  uint8_t i_overheat_lights_off = 2;
  uint8_t i_overheat_sync_to_fan = 2;
  uint8_t i_year_mode_eeprom = SYSTEM_EEPROM_YEAR;
  uint8_t i_system_mode = 1;

  uint8_t i_vga_powercell = 1;
  uint8_t i_vga_cyclotron = 1;
  uint8_t i_demo_light_mode = 1;
  uint8_t i_cyclotron_three_led_toggle = 1; // 1 = single led. 2 = three leds.
  uint8_t i_default_system_volume = 100; // <- i_volume_master_percentage
  uint8_t i_overheat_smoke_duration_level_5 = i_ms_overheating_length_5 / 1000;
  uint8_t i_overheat_smoke_duration_level_4 = i_ms_overheating_length_4 / 1000;
  uint8_t i_overheat_smoke_duration_level_3 = i_ms_overheating_length_3 / 1000;
  uint8_t i_overheat_smoke_duration_level_2 = i_ms_overheating_length_2 / 1000;
  uint8_t i_overheat_smoke_duration_level_1 = i_ms_overheating_length_1 / 1000;

  uint8_t i_smoke_continuous_mode_5 = 1;
  uint8_t i_smoke_continuous_mode_4 = 1;
  uint8_t i_smoke_continuous_mode_3 = 1;
  uint8_t i_smoke_continuous_mode_2 = 1;
  uint8_t i_smoke_continuous_mode_1 = 1;

  if(b_stream_effects != true) {
    i_proton_stream_effects = 1;
  }

  if(b_cyclotron_simulate_ring != true) {
    i_simulate_ring = 1;
  }

  if(b_clockwise != true) {
    i_cyclotron_direction = 1;
  }

  if(b_smoke_enabled != true) {
    i_smoke_settings = 1;
  }

  if(b_overheat_strobe != true) {
    i_overheat_strobe = 1;
  }

  if(b_overheat_lights_off != true) {
    i_overheat_lights_off = 1;
  }

  if(b_overheat_sync_to_fan != true) {
    i_overheat_sync_to_fan = 1;
  }

  if(SYSTEM_MODE == MODE_ORIGINAL) {
    i_system_mode = 2;
  }

  if(b_powercell_colour_toggle == true) {
    i_vga_powercell = 2; // 1 = false, 2 = true;
  }

  if(b_cyclotron_colour_toggle == true) {
    i_vga_cyclotron = 2;
  }

  if(b_demo_light_mode == true) {
    i_demo_light_mode = 2;
  }

  if(b_cyclotron_single_led != true) {
    i_cyclotron_three_led_toggle = 2;
  }

  if(i_volume_master_percentage > 0 && i_volume_master_percentage < 255) {
    i_default_system_volume = i_volume_master_percentage;
  }

  if(b_smoke_continuous_mode_5 == true) {
    i_smoke_continuous_mode_5 = 2;
  }

  if(b_smoke_continuous_mode_4 == true) {
    i_smoke_continuous_mode_4 = 2;
  }

  if(b_smoke_continuous_mode_3 == true) {
    i_smoke_continuous_mode_3 = 2;
  }

  if(b_smoke_continuous_mode_2 == true) {
    i_smoke_continuous_mode_2 = 2;
  }

  if(b_smoke_continuous_mode_1 == true) {
    i_smoke_continuous_mode_1 = 2;
  }

  unsigned int i_eepromConfigAddress = EEPROM.length() / 2;

  objConfigEEPROM obj_eeprom = {
    i_proton_stream_effects,
    i_cyclotron_direction,
    i_simulate_ring,
    i_smoke_settings,
    i_overheat_strobe,
    i_overheat_lights_off,
    i_overheat_sync_to_fan,
    i_year_mode_eeprom,
    i_system_mode,
    i_vga_powercell,
    i_vga_cyclotron,
    i_demo_light_mode,
    i_cyclotron_three_led_toggle,
    i_default_system_volume,
    i_overheat_smoke_duration_level_5,
    i_overheat_smoke_duration_level_4,
    i_overheat_smoke_duration_level_3,
    i_overheat_smoke_duration_level_2,
    i_overheat_smoke_duration_level_1,
    i_smoke_continuous_mode_5,
    i_smoke_continuous_mode_4,
    i_smoke_continuous_mode_3,
    i_smoke_continuous_mode_2,
    i_smoke_continuous_mode_1
  };

  // Save to the EEPROM.
  EEPROM.put(i_eepromConfigAddress, obj_eeprom);

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