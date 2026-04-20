/**
 *   GPStar Proton Stream Target Trainer
 *   Copyright (C) 2023-2026 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *
 *   Adapted for ESP32 Preferences API.
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

// Include ESP32 Preferences library
#include <Preferences.h>

// Used to get UI preferences from the device namespace.
void getSpecialPreferences() {
  // Create Preferences object to handle non-volatile storage (NVS).
  Preferences preferences;

  // Load target configuration from the "device" namespace
  bool b_config_loaded = false;

  if(preferences.begin("device", true)) {
    if(preferences.isKey("targetConfig")) {
      // Validate stored size matches current struct size
      size_t i_stored_size = preferences.getBytesLength("targetConfig");
      if(i_stored_size == sizeof(targetConfig)) {
        size_t i_bytes_read = preferences.getBytes("targetConfig", &targetConfig, sizeof(targetConfig));
        b_config_loaded = (i_bytes_read == sizeof(targetConfig));
      }
    }

    // Future: Add any other device-specific preference loading here

    preferences.end();
  }

  // If namespace is not initialized or config not loaded, initialize with defaults
  if(!b_config_loaded) {
    if(preferences.begin("device", false)) {
      // Save default target configuration on first boot
      preferences.putBytes("targetConfig", &targetConfig, sizeof(targetConfig));
      preferences.end();
    }
  }
}
