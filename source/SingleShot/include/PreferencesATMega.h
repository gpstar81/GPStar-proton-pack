/**
 *   GPStar Single-Shot Blaster
 *   Copyright (C) 2024-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

/*
 * General EEPROM Variables
 */
const uint16_t i_eepromAddress = 0; // The address in the EEPROM to start reading from.

/*
 * Read all user preferences from device controller EEPROM.
 */
void readEEPROM() {
  bool b_config_loaded = false;
  
  // Check if EEPROM contains valid data by reading a size marker.
  uint16_t i_stored_size;
  EEPROM.get(i_eepromAddress, i_stored_size);
  
  if(i_stored_size == sizeof(blasterConfig)) {
    // Size matches - safe to load configuration directly
    EEPROM.get(i_eepromAddress + sizeof(uint16_t), blasterConfig);
    b_config_loaded = true;
  }
  
  if(b_config_loaded) {
    // Successfully loaded a valid configuration, apply to other variables.
    i_volume_master_percentage = blasterConfig.defaultSystemVolume;
    i_volume_master = MINIMUM_VOLUME - ((MINIMUM_VOLUME - i_volume_abs_max) * i_volume_master_percentage / 100);
    i_volume_revert = i_volume_master;
    setAudioLED(blasterConfig.gpstarAudioLed);
  }
  else {
    // Failed to load valid config; reset to defaults
    playEffect(S_VOICE_EEPROM_LOADING_FAILED_RESET);
    clearConfigEEPROM();
  }
}

void clearConfigEEPROM() {
  // Clear out the EEPROM for size marker + blasterConfig struct.
  for(uint16_t i = i_eepromAddress; i < (sizeof(uint16_t) + sizeof(blasterConfig)); i++) {
    EEPROM.update(i, 0xFF); // Write 0xFF to each address
  }
  
  // Write invalid size marker to indicate no valid data.
  uint16_t i_invalid_size = 0;
  EEPROM.put(i_eepromAddress, i_invalid_size);
}

void saveConfigEEPROM() {
  // Store size marker first, then the blasterConfig struct
  uint16_t i_config_size = sizeof(blasterConfig);
  EEPROM.put(i_eepromAddress, i_config_size);
  EEPROM.put(i_eepromAddress + sizeof(uint16_t), blasterConfig);
}
