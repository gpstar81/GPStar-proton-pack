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

// Reference global instances defined elsewhere
extern Axis3F accelOffsets;
extern Axis3F gyroOffsets;

// Include ESP32 Preferences library
#include <Preferences.h>

// Save config settings to Preferences
void saveConfigEEPROM() {
  Preferences preferences;

  if(preferences.begin("config", false)) {
    preferences.putBytes("config", &blasterConfig, sizeof(blasterConfig));
    preferences.end();
  }
}

// Clear configuration preferences in NVS (ESP32)
void clearConfigEEPROM() {
  Preferences preferences;

  if(preferences.begin("config", false)) {
    preferences.clear();
    preferences.end();
  }
}

// Read configuration preferences from NVS (ESP32)
void readEEPROM() {
  Preferences preferences;
  bool b_config_loaded = false;

  if(preferences.begin("config", true)) {
    if(preferences.isKey("config")) {
      // Validate stored size matches current struct size
      size_t i_stored_size = preferences.getBytesLength("config");
      if(i_stored_size == sizeof(blasterConfig)) {
        size_t i_bytes_read = preferences.getBytes("config", &blasterConfig, sizeof(blasterConfig));
        b_config_loaded = (i_bytes_read == sizeof(blasterConfig));
      }
    }
    preferences.end();
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

// Used to get UI preferences from the device namespace.
void getSpecialPreferences() {
  Preferences preferences;

  /*
   * Get Local Device Preferences
   * Accesses the "device" namespace in read-only mode under the "nvs" partition.
   */
  bool b_namespace_opened = preferences.begin("device", true);
  if(b_namespace_opened) {
    // Return stored values if available, otherwise use a default value.
    s_track_listing = preferences.getString("track_list", "");

    // Restore the magnetometer calibration data from preferences.
    if(preferences.isKey("orientation")) {
      switch(preferences.getShort("orientation", 0)) {
        case 0:
        default:
          INSTALL_ORIENTATION = COMPONENTS_NOT_ORIENTED;
        break;
        case 1:
          INSTALL_ORIENTATION = COMPONENTS_UP_USB_FRONT;
        break;
        case 2:
          INSTALL_ORIENTATION = COMPONENTS_UP_USB_REAR;
        break;
        case 3:
          // Default for Haslab.
          INSTALL_ORIENTATION = COMPONENTS_DOWN_USB_FRONT;
        break;
        case 4:
          INSTALL_ORIENTATION = COMPONENTS_DOWN_USB_REAR;
        break;
        case 5:
          INSTALL_ORIENTATION = COMPONENTS_LEFT_USB_FRONT;
        break;
        case 6:
          INSTALL_ORIENTATION = COMPONENTS_LEFT_USB_REAR;
        break;
        case 7:
          // Default for Mack's Factory.
          INSTALL_ORIENTATION = COMPONENTS_RIGHT_USB_FRONT;
        break;
        case 8:
          INSTALL_ORIENTATION = COMPONENTS_RIGHT_USB_REAR;
        break;
        case 9:
          // Special debug mode for bench testing without orientation.
          INSTALL_ORIENTATION = COMPONENTS_FACTORY_DEFAULT;
        break;
      }
    }

    // Restore the magnetometer calibration data from preferences.
    if(preferences.isKey("mag_cal")) {
      preferences.getBytes("mag_cal", &magCalData, sizeof(magCalData));

      size_t readA = preferences.getBytes("accel_cal", &accelOffsets, sizeof(accelOffsets));
      if(readA == sizeof(accelOffsets)) {
        calibratedOffsets.accelX = accelOffsets.x;
        calibratedOffsets.accelY = accelOffsets.y;
        calibratedOffsets.accelZ = accelOffsets.z;
      }

      size_t readG = preferences.getBytes("gyro_cal", &gyroOffsets, sizeof(gyroOffsets));
      if(readG == sizeof(gyroOffsets)) {
        calibratedOffsets.gyroX = gyroOffsets.x;
        calibratedOffsets.gyroY = gyroOffsets.y;
        calibratedOffsets.gyroZ = gyroOffsets.z;
      }
    }

    preferences.end();
  }
  else {
    // If namespace is not initialized, open in read/write mode and set defaults.
    if(preferences.begin("device", false)) {
      preferences.putString("track_list", "");
      preferences.putShort("orientation", 3); // COMPONENTS_DOWN_USB_FRONT
      preferences.putBytes("mag_cal", &magCalData, sizeof(magCalData));
      preferences.end();
    }
  }

  // Fallback to the Haslab as default if not set.
  if(INSTALL_ORIENTATION == COMPONENTS_NOT_ORIENTED) {
    INSTALL_ORIENTATION = COMPONENTS_DOWN_USB_FRONT;
  }
}
