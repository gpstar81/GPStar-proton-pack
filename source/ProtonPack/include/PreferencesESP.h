/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
void resetCyclotronLEDs();
void resetInnerCyclotronLEDs();
void resetContinuousSmoke();
void updateProtonPackLEDCounts();

// Include ESP32 Preferences library
#include <Preferences.h>

// Data structure for LED settings (stored in Preferences)
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
  uint8_t powercell_brightness;
  uint8_t cyclotron_brightness;
  uint8_t inner_cyclotron_brightness;
  uint8_t inner_panel_brightness;
  uint8_t cyclotron_cavity_count;
  uint8_t cyclotron_cavity_type;
  uint8_t inner_cyclotron_led_panel;
  uint8_t powercell_inverted;
} gObjLEDEEPROM;

struct objConfigEEPROM {
  uint8_t stream_effects;
  uint8_t cyclotron_direction;
  uint8_t center_led_fade;
  uint8_t simulate_ring;
  uint8_t smoke_setting;
  uint8_t overheat_strobe;
  uint8_t overheat_lights_off;
  uint8_t overheat_sync_to_fan;
  uint8_t year_mode;
  uint8_t system_mode;
  uint8_t vg_powercell;
  uint8_t vg_cyclotron;
  uint8_t demo_light_mode;
  uint8_t cyclotron_three_led_toggle;
  uint8_t default_system_volume;
  uint8_t overheat_smoke_duration_level_5;
  uint8_t overheat_smoke_duration_level_4;
  uint8_t overheat_smoke_duration_level_3;
  uint8_t overheat_smoke_duration_level_2;
  uint8_t overheat_smoke_duration_level_1;
  uint8_t smoke_continuous_level_5;
  uint8_t smoke_continuous_level_4;
  uint8_t smoke_continuous_level_3;
  uint8_t smoke_continuous_level_2;
  uint8_t smoke_continuous_level_1;
  uint8_t pack_vibration;
  uint8_t use_ribbon_cable;
} gObjConfigEEPROM;

// Save LED settings to Preferences
void saveLEDEEPROM() {
  Preferences prefs;
  prefs.begin("led", false);
  prefs.putUChar("powercell_count", gObjLEDEEPROM.powercell_count);
  prefs.putUChar("cyclotron_count", gObjLEDEEPROM.cyclotron_count);
  prefs.putUChar("inner_cyclotron_count", gObjLEDEEPROM.inner_cyclotron_count);
  prefs.putUChar("grb_inner_cyclotron", gObjLEDEEPROM.grb_inner_cyclotron);
  prefs.putUChar("powercell_spectral_custom", gObjLEDEEPROM.powercell_spectral_custom);
  prefs.putUChar("cyclotron_spectral_custom", gObjLEDEEPROM.cyclotron_spectral_custom);
  prefs.putUChar("cyclotron_inner_spectral_custom", gObjLEDEEPROM.cyclotron_inner_spectral_custom);
  prefs.putUChar("powercell_spectral_saturation_custom", gObjLEDEEPROM.powercell_spectral_saturation_custom);
  prefs.putUChar("cyclotron_spectral_saturation_custom", gObjLEDEEPROM.cyclotron_spectral_saturation_custom);
  prefs.putUChar("cyclotron_inner_spectral_saturation_custom", gObjLEDEEPROM.cyclotron_inner_spectral_saturation_custom);
  prefs.putUChar("powercell_brightness", gObjLEDEEPROM.powercell_brightness);
  prefs.putUChar("cyclotron_brightness", gObjLEDEEPROM.cyclotron_brightness);
  prefs.putUChar("inner_cyclotron_brightness", gObjLEDEEPROM.inner_cyclotron_brightness);
  prefs.putUChar("inner_panel_brightness", gObjLEDEEPROM.inner_panel_brightness);
  prefs.putUChar("cyclotron_cavity_count", gObjLEDEEPROM.cyclotron_cavity_count);
  prefs.putUChar("cyclotron_cavity_type", gObjLEDEEPROM.cyclotron_cavity_type);
  prefs.putUChar("inner_cyclotron_led_panel", gObjLEDEEPROM.inner_cyclotron_led_panel);
  prefs.putUChar("powercell_inverted", gObjLEDEEPROM.powercell_inverted);
  prefs.end();
}

// Load LED settings from Preferences
void loadLEDEEPROM() {
  Preferences prefs;
  prefs.begin("led", true);
  gObjLEDEEPROM.powercell_count = prefs.getUChar("powercell_count", 13);
  gObjLEDEEPROM.cyclotron_count = prefs.getUChar("cyclotron_count", 4);
  gObjLEDEEPROM.inner_cyclotron_count = prefs.getUChar("inner_cyclotron_count", 12);
  gObjLEDEEPROM.grb_inner_cyclotron = prefs.getUChar("grb_inner_cyclotron", 1);
  gObjLEDEEPROM.powercell_spectral_custom = prefs.getUChar("powercell_spectral_custom", 0);
  gObjLEDEEPROM.cyclotron_spectral_custom = prefs.getUChar("cyclotron_spectral_custom", 0);
  gObjLEDEEPROM.cyclotron_inner_spectral_custom = prefs.getUChar("cyclotron_inner_spectral_custom", 0);
  gObjLEDEEPROM.powercell_spectral_saturation_custom = prefs.getUChar("powercell_spectral_saturation_custom", 0);
  gObjLEDEEPROM.cyclotron_spectral_saturation_custom = prefs.getUChar("cyclotron_spectral_saturation_custom", 0);
  gObjLEDEEPROM.cyclotron_inner_spectral_saturation_custom = prefs.getUChar("cyclotron_inner_spectral_saturation_custom", 0);
  gObjLEDEEPROM.powercell_brightness = prefs.getUChar("powercell_brightness", 100);
  gObjLEDEEPROM.cyclotron_brightness = prefs.getUChar("cyclotron_brightness", 100);
  gObjLEDEEPROM.inner_cyclotron_brightness = prefs.getUChar("inner_cyclotron_brightness", 100);
  gObjLEDEEPROM.inner_panel_brightness = prefs.getUChar("inner_panel_brightness", 100);
  gObjLEDEEPROM.cyclotron_cavity_count = prefs.getUChar("cyclotron_cavity_count", 0);
  gObjLEDEEPROM.cyclotron_cavity_type = prefs.getUChar("cyclotron_cavity_type", 2);
  gObjLEDEEPROM.inner_cyclotron_led_panel = prefs.getUChar("inner_cyclotron_led_panel", 2);
  gObjLEDEEPROM.powercell_inverted = prefs.getUChar("powercell_inverted", 1);
  prefs.end();
}

// Clear LED settings in Preferences
void clearLEDEEPROM() {
  Preferences prefs;
  prefs.begin("led", false);
  prefs.clear();
  prefs.end();
}

// Save config settings to Preferences
void saveConfigEEPROM() {
  Preferences prefs;
  prefs.begin("config", false);
  prefs.putBytes("config", &gObjConfigEEPROM, sizeof(gObjConfigEEPROM));
  prefs.end();
}

// Load config settings from Preferences
void readEEPROM() {
  Preferences prefs;
  prefs.begin("config", true);
  prefs.getBytes("config", &gObjConfigEEPROM, sizeof(gObjConfigEEPROM));
  prefs.end();
}

// Clear config settings in Preferences (reset to defaults)
void clearConfigEEPROM() {
  Preferences prefs;
  prefs.begin("config", false);
  prefs.clear();
  prefs.end();
}

// Update CRC for config (stub for compatibility, ESP32 NVS is already robust)
void updateCRCEEPROM() {
  // ESP32 Preferences/NVS handles integrity, but you can implement CRC if needed.
}

// Calculate CRC for config (stub for compatibility)
uint32_t eepromCRC(void) {
  // Example CRC32 calculation over gObjConfigEEPROM
  const uint8_t* data = reinterpret_cast<const uint8_t*>(&gObjConfigEEPROM);
  uint32_t crc = 0xFFFFFFFF;
  for (size_t i = 0; i < sizeof(gObjConfigEEPROM); ++i) {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; ++j) {
      if (crc & 1)
        crc = (crc >> 1) ^ 0xEDB88320;
      else
        crc >>= 1;
    }
  }
  return crc ^ 0xFFFFFFFF;
}
