/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

/*
 * Function prototypes.
 */
void readEEPROM();
void clearConfigEEPROM();
void clearLEDEEPROM();
void saveConfigEEPROM();
void saveLEDEEPROM();
void loadConfigEEPROM();
void loadLEDEEPROM();
void updateCRCEEPROM(uint32_t);
uint32_t getCRCEEPROM(void);
uint32_t eepromCRC(void);
void bargraphYearModeUpdate();
void resetOverheatLevels();
void resetWhiteLEDBlinkRate();

// Include ESP32 Preferences library
#include <Preferences.h>

// Preferences for system configuration, which will use a "led" and "config" namespaces.
// For Wireless.h will store SSID and AP password within a "credentials" namespace.
Preferences preferences;

// Data structure for LED settings (stored in Preferences)
struct objLEDEEPROM {
  uint8_t barrelSpectralCustom;
  uint8_t barrelSpectralSaturationCustom;
  uint8_t numBarrelLeds;
  uint8_t numBargraphLeds;
} gObjLEDEEPROM;

// Data structure for configuration settings (stored in Preferences)
struct objConfigEEPROM {
  uint8_t crossTheStreams;
  uint8_t crossTheStreamsMix;
  uint8_t overheating;
  uint8_t extraProtonSounds;
  uint8_t neutronaWandSounds;
  uint8_t spectralMode;
  uint8_t holidayMode;
  uint8_t quickVent;
  uint8_t wandBootErrors;
  uint8_t ventLightAutoIntensity;
  uint8_t invertBargraph;
  uint8_t bargraphMode;
  uint8_t bargraphFiringAnimation;
  uint8_t bargraphOverheatBlinking;
  uint8_t neutronaWandYearMode;
  uint8_t ctsMode;
  uint8_t systemMode;
  uint8_t beepLoop;
  uint8_t defaultWandVolume;
  uint8_t overheatStartTimerLevel5;
  uint8_t overheatStartTimerLevel4;
  uint8_t overheatStartTimerLevel3;
  uint8_t overheatStartTimerLevel2;
  uint8_t overheatStartTimerLevel1;
  uint8_t overheatLevel5;
  uint8_t overheatLevel4;
  uint8_t overheatLevel3;
  uint8_t overheatLevel2;
  uint8_t overheatLevel1;
  uint8_t wandVibration;
} gObjConfigEEPROM;

// Save LED settings to Preferences using the global instance
void saveLEDEEPROM() {
  gObjLEDEEPROM.barrelSpectralCustom = i_spectral_wand_custom_colour;
  gObjLEDEEPROM.barrelSpectralSaturationCustom = i_spectral_wand_custom_saturation;
  gObjLEDEEPROM.numBarrelLeds = WAND_BARREL_LED_COUNT;
  gObjLEDEEPROM.numBargraphLeds = BARGRAPH_TYPE_EEPROM;

  preferences.begin("led", false);
  preferences.putBytes("led", &gObjLEDEEPROM, sizeof(gObjLEDEEPROM));
  preferences.end();

  updateCRCEEPROM(eepromCRC());
}

// Load LED settings from Preferences into the global instance
void loadLEDEEPROM() {
  if (preferences.begin("led", true)) {
    if (preferences.isKey("led")) {
      preferences.getBytes("led", &gObjLEDEEPROM, sizeof(gObjLEDEEPROM));
    }
    preferences.end();
  }
}

// Clear LED settings in Preferences
void clearLEDEEPROM() {
  if (preferences.begin("led", false)) {
    preferences.clear();
    preferences.end();
  }

  updateCRCEEPROM(eepromCRC());
}

// Save config settings to Preferences
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
  uint8_t i_wand_boot_errors = 1;
  uint8_t i_vent_light_auto_intensity = 2;
  uint8_t i_invert_bargraph = 1;
  uint8_t i_bargraph_mode = 1; // 1 = default, 2 = super hero, 3 = original.
  uint8_t i_bargraph_firing_animation = 1; // 1 = default, 2 = super hero, 3 = original.
  uint8_t i_bargraph_overheat_blinking = 1;
  uint8_t i_neutrona_wand_year_mode = 1; // 1 = default, 2 = 1984, 3 = 1989, 4 = Afterlife, 5 = Frozen Empire.
  uint8_t i_cts_mode = 1; // 1 = default, 2 = 1984, 3 = 1989, 4 = Afterlife, 5 = Frozen Empire.
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

  if(b_wand_boot_errors) {
    i_wand_boot_errors = 2;
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
      i_cts_mode = 4;
    break;
    case CTS_1984:
      i_cts_mode = 2;
    break;
    case CTS_DEFAULT:
    default:
      i_cts_mode = 1;
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

  gObjConfigEEPROM.crossTheStreams = i_cross_the_streams;
  gObjConfigEEPROM.crossTheStreamsMix = i_cross_the_streams_mix;
  gObjConfigEEPROM.overheating = i_overheating;
  gObjConfigEEPROM.extraProtonSounds = i_extra_proton_sounds;
  gObjConfigEEPROM.neutronaWandSounds = i_neutrona_wand_sounds;
  gObjConfigEEPROM.spectralMode = i_spectral;
  gObjConfigEEPROM.holidayMode = 0;
  gObjConfigEEPROM.quickVent = i_quick_vent;
  gObjConfigEEPROM.wandBootErrors = i_wand_boot_errors;
  gObjConfigEEPROM.ventLightAutoIntensity = i_vent_light_auto_intensity;
  gObjConfigEEPROM.invertBargraph = i_invert_bargraph;
  gObjConfigEEPROM.bargraphMode = i_bargraph_mode;
  gObjConfigEEPROM.bargraphFiringAnimation = i_bargraph_firing_animation;
  gObjConfigEEPROM.bargraphOverheatBlinking = i_bargraph_overheat_blinking;
  gObjConfigEEPROM.neutronaWandYearMode = i_neutrona_wand_year_mode;
  gObjConfigEEPROM.ctsMode = i_cts_mode;
  gObjConfigEEPROM.systemMode = i_system_mode;
  gObjConfigEEPROM.beepLoop = i_beep_loop;
  gObjConfigEEPROM.defaultWandVolume = i_default_wand_volume;
  gObjConfigEEPROM.overheatStartTimerLevel5 = i_overheat_start_timer_level_5;
  gObjConfigEEPROM.overheatStartTimerLevel4 = i_overheat_start_timer_level_4;
  gObjConfigEEPROM.overheatStartTimerLevel3 = i_overheat_start_timer_level_3;
  gObjConfigEEPROM.overheatStartTimerLevel2 = i_overheat_start_timer_level_2;
  gObjConfigEEPROM.overheatStartTimerLevel1 = i_overheat_start_timer_level_1;
  gObjConfigEEPROM.overheatLevel5 = i_overheat_level_5;
  gObjConfigEEPROM.overheatLevel4 = i_overheat_level_4;
  gObjConfigEEPROM.overheatLevel3 = i_overheat_level_3;
  gObjConfigEEPROM.overheatLevel2 = i_overheat_level_2;
  gObjConfigEEPROM.overheatLevel1 = i_overheat_level_1;
  gObjConfigEEPROM.wandVibration = i_wand_vibration;

  if (preferences.begin("config", false)) {
    preferences.putBytes("config", &gObjConfigEEPROM, sizeof(gObjConfigEEPROM));
    preferences.end();
  }

  updateCRCEEPROM(eepromCRC());
}

// Load config settings from Preferences
void loadConfigEEPROM() {
  if (preferences.begin("config", true)) {
    if (preferences.isKey("config")) {
      preferences.getBytes("config", &gObjConfigEEPROM, sizeof(gObjConfigEEPROM));
    }
    preferences.end();
  }
}

// Clear config settings in Preferences
void clearConfigEEPROM() {
  if (preferences.begin("config", false)) {
    preferences.clear();
    preferences.end();
  }

  updateCRCEEPROM(eepromCRC());
}

// CRC helpers for Preferences
void updateCRCEEPROM(uint32_t crc) {
  if (preferences.begin("crc", false)) {
    preferences.putUInt("crc", crc);
    preferences.end();
  }
}

uint32_t getCRCEEPROM() {
  uint32_t crc = 0;

  if (preferences.begin("crc", true)) {
    crc = preferences.getUInt("crc");
    preferences.end();
  }

  return crc;
}

// Calculate CRC for all stored preferences
uint32_t eepromCRC() {
  CRC32 crc;
  loadLEDEEPROM();
  loadConfigEEPROM();
  crc.update((uint8_t*)&gObjLEDEEPROM, sizeof(gObjLEDEEPROM));
  crc.update((uint8_t*)&gObjConfigEEPROM, sizeof(gObjConfigEEPROM));
  return crc.finalize();
}

// Read all user preferences from Preferences (ESP32)
void readEEPROM() {
  uint32_t storedCrc = getCRCEEPROM();
  uint32_t calcCrc = eepromCRC();
  if(storedCrc == calcCrc) {
    // Map loaded values to runtime variables as needed
    if(gObjConfigEEPROM.crossTheStreams == 2) {
      FIRING_MODE = CTS_MODE; // At least the CTS mode is enabled.

      if(gObjConfigEEPROM.crossTheStreamsMix == 2) {
        FIRING_MODE = CTS_MIX_MODE; // Upgrade to the CTS Mix mode.
      }

      // Remember this as the last firing mode as well.
      LAST_FIRING_MODE = FIRING_MODE;
    }

    if(gObjConfigEEPROM.overheating > 0 && gObjConfigEEPROM.overheating < 3) {
      if(gObjConfigEEPROM.overheating > 1) {
        b_overheat_enabled = true;
      }
      else {
        b_overheat_enabled = false;
      }
    }

    if(gObjConfigEEPROM.extraProtonSounds > 0 && gObjConfigEEPROM.extraProtonSounds < 3) {
      if(gObjConfigEEPROM.extraProtonSounds > 1) {
        b_stream_effects = true;
      }
      else {
        b_stream_effects = false;
      }
    }

    if(gObjConfigEEPROM.neutronaWandSounds > 0 && gObjConfigEEPROM.neutronaWandSounds < 3) {
      if(gObjConfigEEPROM.neutronaWandSounds > 1) {
        b_extra_pack_sounds = true;
      }
      else {
        b_extra_pack_sounds = false;
      }
    }

    if(gObjConfigEEPROM.spectralMode > 0 && gObjConfigEEPROM.spectralMode < 3) {
      if(gObjConfigEEPROM.spectralMode > 1) {
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

    if(gObjConfigEEPROM.quickVent > 0 && gObjConfigEEPROM.quickVent < 3) {
      if(gObjConfigEEPROM.quickVent > 1) {
        b_quick_vent = true;
      }
      else {
        b_quick_vent = false;
      }
    }

    if(gObjConfigEEPROM.wandBootErrors > 0 && gObjConfigEEPROM.wandBootErrors < 3) {
      if(gObjConfigEEPROM.wandBootErrors > 1) {
        b_wand_boot_errors = true;
      }
      else {
        b_wand_boot_errors = false;
      }
    }

    if(gObjConfigEEPROM.ventLightAutoIntensity > 0 && gObjConfigEEPROM.ventLightAutoIntensity < 3) {
      if(gObjConfigEEPROM.ventLightAutoIntensity > 1) {
        b_vent_light_control = true;
      }
      else {
        b_vent_light_control = false;
      }
    }

    if(gObjConfigEEPROM.invertBargraph > 0 && gObjConfigEEPROM.invertBargraph < 3) {
      if(gObjConfigEEPROM.invertBargraph > 1) {
        b_bargraph_invert = true;
      }
      else {
        b_bargraph_invert = false;
      }
    }

    if(gObjConfigEEPROM.bargraphMode > 0 && gObjConfigEEPROM.bargraphMode < 4) {
      switch(gObjConfigEEPROM.bargraphMode) {
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

    if(gObjConfigEEPROM.bargraphFiringAnimation > 0 && gObjConfigEEPROM.bargraphFiringAnimation < 4) {
      switch(gObjConfigEEPROM.bargraphFiringAnimation) {
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

    if(gObjConfigEEPROM.bargraphOverheatBlinking > 0 && gObjConfigEEPROM.bargraphOverheatBlinking < 3) {
      if(gObjConfigEEPROM.bargraphOverheatBlinking > 1) {
        b_overheat_bargraph_blink = true;
      }
      else {
        b_overheat_bargraph_blink = false;
      }
    }

    if(gObjConfigEEPROM.neutronaWandYearMode > 0 && gObjConfigEEPROM.neutronaWandYearMode < 6) {
      switch(gObjConfigEEPROM.neutronaWandYearMode) {
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

    if(gObjConfigEEPROM.ctsMode > 0 && gObjConfigEEPROM.ctsMode < 6) {
      switch(gObjConfigEEPROM.ctsMode) {
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

    if(gObjConfigEEPROM.systemMode > 0 && gObjConfigEEPROM.systemMode < 3 && b_gpstar_benchtest) {
      if(gObjConfigEEPROM.systemMode > 1) {
        SYSTEM_MODE = MODE_ORIGINAL;
      }
      else {
        SYSTEM_MODE = MODE_SUPER_HERO;
      }
    }

    if(gObjConfigEEPROM.beepLoop > 0 && gObjConfigEEPROM.beepLoop < 3) {
      if(gObjConfigEEPROM.beepLoop > 1) {
        b_beep_loop = true;
      }
      else {
        b_beep_loop = false;
      }
    }

    if(gObjConfigEEPROM.defaultWandVolume > 0 && gObjConfigEEPROM.defaultWandVolume < 102 && b_gpstar_benchtest) {
      // EEPROM value is from 1 to 101; subtract 1 to get the correct percentage.
      i_volume_master_percentage = gObjConfigEEPROM.defaultWandVolume - 1;
      i_volume_master_eeprom = MINIMUM_VOLUME - ((MINIMUM_VOLUME - i_volume_abs_max) * i_volume_master_percentage / 100);
      i_volume_revert = i_volume_master_eeprom;
      i_volume_master = i_volume_master_eeprom;
    }

    if(gObjConfigEEPROM.overheatStartTimerLevel5 > 0 && gObjConfigEEPROM.overheatStartTimerLevel5 < 61) {
      i_ms_overheat_initiate_level_5 = gObjConfigEEPROM.overheatStartTimerLevel5 * 1000;

      i_ms_overheat_initiate[4] = i_ms_overheat_initiate_level_5;
    }

    if(gObjConfigEEPROM.overheatStartTimerLevel4 > 0 && gObjConfigEEPROM.overheatStartTimerLevel4 < 61) {
      i_ms_overheat_initiate_level_4 = gObjConfigEEPROM.overheatStartTimerLevel4 * 1000;

      i_ms_overheat_initiate[3] = i_ms_overheat_initiate_level_4;
    }

    if(gObjConfigEEPROM.overheatStartTimerLevel3 > 0 && gObjConfigEEPROM.overheatStartTimerLevel3 < 61) {
      i_ms_overheat_initiate_level_3 = gObjConfigEEPROM.overheatStartTimerLevel3 * 1000;

      i_ms_overheat_initiate[2] = i_ms_overheat_initiate_level_3;
    }

    if(gObjConfigEEPROM.overheatStartTimerLevel2 > 0 && gObjConfigEEPROM.overheatStartTimerLevel2 < 61) {
      i_ms_overheat_initiate_level_2 = gObjConfigEEPROM.overheatStartTimerLevel2 * 1000;

      i_ms_overheat_initiate[1] = i_ms_overheat_initiate_level_2;
    }

    if(gObjConfigEEPROM.overheatStartTimerLevel1 > 0 && gObjConfigEEPROM.overheatStartTimerLevel1 < 61) {
      i_ms_overheat_initiate_level_1 = gObjConfigEEPROM.overheatStartTimerLevel1 * 1000;

      i_ms_overheat_initiate[0] = i_ms_overheat_initiate_level_1;
    }

    if(gObjConfigEEPROM.overheatLevel5 > 0 && gObjConfigEEPROM.overheatLevel5 < 3) {
      if(gObjConfigEEPROM.overheatLevel5 > 1) {
        b_overheat_level_5 = true;
      }
      else {
        b_overheat_level_5 = false;
      }
    }

    if(gObjConfigEEPROM.overheatLevel4 > 0 && gObjConfigEEPROM.overheatLevel4 < 3) {
      if(gObjConfigEEPROM.overheatLevel4 > 1) {
        b_overheat_level_4 = true;
      }
      else {
        b_overheat_level_4 = false;
      }
    }

    if(gObjConfigEEPROM.overheatLevel3 > 0 && gObjConfigEEPROM.overheatLevel3 < 3) {
      if(gObjConfigEEPROM.overheatLevel3 > 1) {
        b_overheat_level_3 = true;
      }
      else {
        b_overheat_level_3 = false;
      }
    }

    if(gObjConfigEEPROM.overheatLevel2 > 0 && gObjConfigEEPROM.overheatLevel2 < 3) {
      if(gObjConfigEEPROM.overheatLevel2 > 1) {
        b_overheat_level_2 = true;
      }
      else {
        b_overheat_level_2 = false;
      }
    }

    if(gObjConfigEEPROM.overheatLevel1 > 0 && gObjConfigEEPROM.overheatLevel1 < 3) {
      if(gObjConfigEEPROM.overheatLevel1 > 1) {
        b_overheat_level_1 = true;
      }
      else {
        b_overheat_level_1 = false;
      }
    }

    if(gObjConfigEEPROM.wandVibration > 0 && gObjConfigEEPROM.wandVibration < 5) {
      switch(gObjConfigEEPROM.wandVibration) {
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

    if(gObjLEDEEPROM.barrelSpectralCustom > 0 && gObjLEDEEPROM.barrelSpectralCustom != 255) {
      i_spectral_wand_custom_colour = gObjLEDEEPROM.barrelSpectralCustom;
    }

    if(gObjLEDEEPROM.barrelSpectralSaturationCustom > 0 && gObjLEDEEPROM.barrelSpectralSaturationCustom != 255) {
      i_spectral_wand_custom_saturation = gObjLEDEEPROM.barrelSpectralSaturationCustom;
    }

    if(gObjLEDEEPROM.numBarrelLeds == LEDS_2 || gObjLEDEEPROM.numBarrelLeds == LEDS_5 ||
      gObjLEDEEPROM.numBarrelLeds == LEDS_48 || gObjLEDEEPROM.numBarrelLeds == LEDS_50) {
      i_num_barrel_leds = gObjLEDEEPROM.numBarrelLeds;

      switch(i_num_barrel_leds) {
        case 2:
          WAND_BARREL_LED_COUNT = LEDS_2;
        break;

        case 5:
          WAND_BARREL_LED_COUNT = LEDS_5;
        break;

        case 48:
          WAND_BARREL_LED_COUNT = LEDS_48;
        break;

        case 50:
        default:
          WAND_BARREL_LED_COUNT = LEDS_50;
          i_num_barrel_leds = 48; // Need to reset it to 48. 2 are for the tip.
        break;
      }
    }

    if(gObjLEDEEPROM.numBargraphLeds == SEGMENTS_28 || gObjLEDEEPROM.numBargraphLeds == SEGMENTS_30) {
      if(gObjLEDEEPROM.numBargraphLeds < 30) {
        BARGRAPH_TYPE_EEPROM = SEGMENTS_28;
      }
      else {
        BARGRAPH_TYPE_EEPROM = SEGMENTS_30;
      }

      BARGRAPH_TYPE = BARGRAPH_TYPE_EEPROM;
    }
  }
  else {
    // CRC mismatch; clear preferences
    playEffect(S_VOICE_EEPROM_LOADING_FAILED_RESET);
    clearLEDEEPROM();
    clearConfigEEPROM();
  }
}
