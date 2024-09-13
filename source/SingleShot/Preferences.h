/**
 *   GPStar Single-Shot Blaster
 *   Copyright (C) 2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
void saveConfigEEPROM();
void updateCRCEEPROM();
uint32_t eepromCRC(void);
void resetOverheatLevels();
void resetWhiteLEDBlinkRate();

/*
 * General EEPROM Variables
 */
uint16_t i_eepromAddress = 0; // The address in the EEPROM to start reading from.

/*
 * Data structure object for customizations which are saved into the EEPROM memory.
 */
struct objConfigEEPROM {
  uint8_t device_boot_errors;
  uint8_t vent_light_auto_intensity;
  uint8_t invert_bargraph;
  uint8_t default_system_volume;
  uint8_t device_vibration;
};

/*
 * Read all user preferences from device controller EEPROM.
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

    if(obj_config_eeprom.device_boot_errors > 0 && obj_config_eeprom.device_boot_errors != 255) {
      if(obj_config_eeprom.device_boot_errors > 1) {
        b_device_boot_errors = true;
      }
      else {
        b_device_boot_errors = false;
      }
    }

    if(obj_config_eeprom.vent_light_auto_intensity > 0 && obj_config_eeprom.vent_light_auto_intensity != 255) {
      if(obj_config_eeprom.vent_light_auto_intensity > 1) {
        b_vent_light_control = true;
      }
      else {
        b_vent_light_control = false;
      }
    }

    if(obj_config_eeprom.invert_bargraph > 0 && obj_config_eeprom.invert_bargraph != 255) {
      if(obj_config_eeprom.invert_bargraph > 1) {
        b_bargraph_invert = true;
      }
      else {
        b_bargraph_invert = false;
      }
    }

    if(obj_config_eeprom.default_system_volume > 0 && obj_config_eeprom.default_system_volume <= 100) {
      i_volume_master_percentage = obj_config_eeprom.default_system_volume;
      i_volume_master_eeprom = MINIMUM_VOLUME - (MINIMUM_VOLUME * i_volume_master_percentage / 100);
      i_volume_revert = i_volume_master_eeprom;
      i_volume_master = i_volume_master_eeprom;
    }

    if(obj_config_eeprom.device_vibration > 0 && obj_config_eeprom.device_vibration != 255) {
      switch(obj_config_eeprom.device_vibration) {
        case 3:
          VIBRATION_MODE_EEPROM = VIBRATION_NONE;
          VIBRATION_MODE = VIBRATION_MODE_EEPROM;
        break;

        case 2:
          VIBRATION_MODE_EEPROM = VIBRATION_FIRING_ONLY;
          VIBRATION_MODE = VIBRATION_MODE_EEPROM;
        break;

        case 1:
        default:
          VIBRATION_MODE_EEPROM = VIBRATION_ALWAYS;
          VIBRATION_MODE = VIBRATION_MODE_EEPROM;
        break;
      }
    }
  }
  else {
    // CRC doesn't match; let's clear the EEPROMs to be safe.
    playEffect(S_VOICE_EEPROM_LOADING_FAILED_RESET);

    clearConfigEEPROM();
  }
}

void clearConfigEEPROM() {
  // Clear out the EEPROM only in the memory addresses used for our EEPROM data object.
  for(uint16_t i = 0; i < sizeof(objConfigEEPROM); i++) {
    EEPROM.update(i, 0);
  }

  updateCRCEEPROM();
}

void saveConfigEEPROM() {
  // 1 = false, 2 = true.
  uint8_t i_device_boot_errors = 2; // Assumed true by default.
  uint8_t i_vent_light_auto_intensity = 2; // Assumed true by default.
  uint8_t i_invert_bargraph = 1; // Assumed false by default.
  uint8_t i_default_system_volume = 100; // <- i_volume_master_percentage
  uint8_t i_device_vibration = 4; // 1 = always, 2 = when firing, 3 = off, 4 = default.

  if(!b_device_boot_errors) {
    i_device_boot_errors = 1;
  }

  if(!b_vent_light_control) {
    i_vent_light_auto_intensity = 1;
  }

  if(b_bargraph_invert) {
    i_invert_bargraph = 2;
  }

  if(i_volume_master_percentage <= 100) {
    // Minimum volume in EEPROM is 1; maybe change later?
    if(i_volume_master_percentage == 0) {
      i_default_system_volume = 1;
    }
    else {
      i_default_system_volume = i_volume_master_percentage;
    }
  }

  switch(VIBRATION_MODE_EEPROM) {
    case VIBRATION_ALWAYS:
    default:
      i_device_vibration = 1;
    break;

    case VIBRATION_FIRING_ONLY:
      i_device_vibration = 2;
    break;

    case VIBRATION_NONE:
      i_device_vibration = 3;
    break;
  }

  // Write the data to the EEPROM if any of the values have changed.
  objConfigEEPROM obj_config_eeprom = {
    i_device_boot_errors,
    i_vent_light_auto_intensity,
    i_invert_bargraph,
    i_default_system_volume,
    i_device_vibration
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

  for(uint16_t index = 0; index < (i_eepromAddress + sizeof(objConfigEEPROM)); index++) {
    crc.update(EEPROM[index]);
  }

  crc.update(sizeof(objConfigEEPROM));

  return (uint32_t)crc.finalize();
}
