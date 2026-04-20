/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
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
void updateOverheatLevels();
void resetWhiteLEDBlinkRate();

// Reference global instances defined elsewhere
extern Axis3F accelOffsets;
extern Axis3F gyroOffsets;

// Include ESP32 Preferences library
#include <Preferences.h>

// Data structure for LED settings (stored in Preferences)
struct objLEDEEPROM {
  uint8_t barrelSpectralCustom;
  uint8_t barrelSpectralSaturationCustom; // Currently unused
  uint8_t ventLightAutoIntensity;
  uint8_t numBarrelLeds;
  uint8_t numBargraphLeds;
  // Note there is no RGB Vent Light setting for ESP32 as it must always be enabled
  uint8_t ventLightStreamColours;
  uint8_t gpstarAudioLed;
} gObjLEDEEPROM;

// Data structure for configuration settings (stored in Preferences)
struct objConfigEEPROM {
  uint8_t crossTheStreams;
  uint8_t crossTheStreamsMix;
  uint8_t streamModeOpts;
  uint8_t defaultStreamMode;
  uint8_t overheating;
  uint8_t extraProtonSounds;
  uint8_t neutronaWandSounds;
  uint8_t quickVent;
  uint8_t wandBootErrors;
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
  uint8_t barrelSwitchPolarity;
  uint8_t wandVibration;
} gObjConfigEEPROM;

// Save LED settings to Preferences using the global instance
void saveLEDEEPROM() {
  gObjLEDEEPROM.barrelSpectralCustom = i_spectral_wand_custom_colour;
  gObjLEDEEPROM.barrelSpectralSaturationCustom = i_spectral_wand_custom_saturation;
  gObjLEDEEPROM.ventLightAutoIntensity = b_vent_light_control ? 2 : 1;
  gObjLEDEEPROM.ventLightStreamColours = b_vent_light_stream_colours ? 2 : 1;
  gObjLEDEEPROM.numBarrelLeds = WAND_BARREL_LED_COUNT;
  gObjLEDEEPROM.numBargraphLeds = BARGRAPH_TYPE_EEPROM;
  gObjLEDEEPROM.gpstarAudioLed = b_gpstar_audio_led_enabled ? 2 : 1;

  // Create Preferences object to handle non-volatile storage (NVS).
  Preferences preferences;

  if(preferences.begin("led", false)) {
    preferences.putBytes("led", &gObjLEDEEPROM, sizeof(gObjLEDEEPROM));
    preferences.end();
  }

  updateCRCEEPROM(eepromCRC());
}

// Load LED settings from Preferences into the global instance
void loadLEDEEPROM() {
  // Create Preferences object to handle non-volatile storage (NVS).
  Preferences preferences;

  if(preferences.begin("led", true)) {
    if(preferences.isKey("led")) {
      preferences.getBytes("led", &gObjLEDEEPROM, sizeof(gObjLEDEEPROM));
    }
    preferences.end();
  }
}

// Clear LED settings in Preferences
void clearLEDEEPROM() {
  // Create Preferences object to handle non-volatile storage (NVS).
  Preferences preferences;

  if(preferences.begin("led", false)) {
    preferences.clear();
    preferences.end();
  }

  gObjLEDEEPROM = {};

  updateCRCEEPROM(eepromCRC());
}

// Save config settings to Preferences
void saveConfigEEPROM() {
  // Convert the current EEPROM volume value into a percentage.
  uint8_t i_eeprom_volume_master_percentage = 100 * (MINIMUM_VOLUME - i_volume_master_eeprom) / MINIMUM_VOLUME;

  uint8_t i_bargraph_mode = 1; // 1 = default, 2 = super hero, 3 = original.
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

  uint8_t i_bargraph_firing_animation = 1; // 1 = default, 2 = super hero, 3 = original.
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

  uint8_t i_neutrona_wand_year_mode = 1; // 1 = default, 2 = 1984, 3 = 1989, 4 = Afterlife, 5 = Frozen Empire.
  switch(WAND_YEAR_MODE) {
    case YEAR_FROZEN_EMPIRE:
      i_neutrona_wand_year_mode = (uint8_t)SYSTEM_FROZEN_EMPIRE;
    break;

    case YEAR_AFTERLIFE:
      i_neutrona_wand_year_mode = (uint8_t)SYSTEM_AFTERLIFE;
    break;

    case YEAR_1989:
      i_neutrona_wand_year_mode = (uint8_t)SYSTEM_1989;
    break;

    case YEAR_1984:
      i_neutrona_wand_year_mode = (uint8_t)SYSTEM_1984;
    break;

    case YEAR_DEFAULT:
    default:
      i_neutrona_wand_year_mode = (uint8_t)SYSTEM_TOGGLE_SWITCH;
    break;
  }

  uint8_t i_cts_mode = 1; // 1 = default, 2 = 1984, 3 = 1989, 4 = Afterlife, 5 = Frozen Empire.
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

  uint8_t i_barrel_switch_polarity = 1; // 1 = default, 2 = inverted, 3 = disabled.
  switch(BARREL_SWITCH_POLARITY) {
    case SWITCH_DEFAULT:
    default:
      i_barrel_switch_polarity = 1;
    break;

    case SWITCH_INVERTED:
      i_barrel_switch_polarity = 2;
    break;

    case SWITCH_DISABLED:
      i_barrel_switch_polarity = 3;
    break;
  }

  uint8_t i_wand_vibration = 4; // 1 = always, 2 = when firing, 3 = off, 4 = default.
  switch(VIBRATION_MODE_EEPROM) {
    case VIBRATION_ALWAYS:
      i_wand_vibration = 1;
    break;

    case VIBRATION_FIRING_ONLY:
      i_wand_vibration = 2;
    break;

    case VIBRATION_NEVER:
      i_wand_vibration = 3;
    break;

    case VIBRATION_DEFAULT:
    default:
      i_wand_vibration = 4;
    break;
  }

  gObjConfigEEPROM.crossTheStreams = (gpstarWand.isFiringModeCTS() || gpstarWand.isFiringModeCTSMix()) ? 2 : 1;
  gObjConfigEEPROM.crossTheStreamsMix = gpstarWand.isFiringModeCTSMix() ? 2 : 1;
  gObjConfigEEPROM.streamModeOpts = gpstarWand.getStreamModeOpts() + 1; // Have to add 1 to distinguish set and unset values
  gObjConfigEEPROM.defaultStreamMode = (uint8_t)DEFAULT_STREAM_MODE;
  gObjConfigEEPROM.overheating = b_overheat_enabled ? 2 : 1;
  gObjConfigEEPROM.extraProtonSounds = b_stream_effects ? 2 : 1;
  gObjConfigEEPROM.neutronaWandSounds = b_extra_pack_sounds ? 2 : 1;
  gObjConfigEEPROM.quickVent = b_quick_vent ? 2 : 1;
  gObjConfigEEPROM.wandBootErrors = b_wand_boot_errors ? 2 : 1;
  gObjConfigEEPROM.invertBargraph = b_bargraph_invert ? 2 : 1;
  gObjConfigEEPROM.bargraphMode = i_bargraph_mode;
  gObjConfigEEPROM.bargraphFiringAnimation = i_bargraph_firing_animation;
  gObjConfigEEPROM.bargraphOverheatBlinking = b_overheat_bargraph_blink ? 2 : 1;
  gObjConfigEEPROM.neutronaWandYearMode = i_neutrona_wand_year_mode;
  gObjConfigEEPROM.ctsMode = i_cts_mode;
  gObjConfigEEPROM.systemMode = gpstarWand.getSystemMode() == MODE_ORIGINAL ? 2 : 1;
  gObjConfigEEPROM.beepLoop = b_beep_loop ? 2 : 1;
  gObjConfigEEPROM.defaultWandVolume = (i_eeprom_volume_master_percentage < 101) ? (i_eeprom_volume_master_percentage + 1) : 101;
  gObjConfigEEPROM.overheatStartTimerLevel5 = i_ms_overheat_initiate_level_5 / 1000;
  gObjConfigEEPROM.overheatStartTimerLevel4 = i_ms_overheat_initiate_level_4 / 1000;
  gObjConfigEEPROM.overheatStartTimerLevel3 = i_ms_overheat_initiate_level_3 / 1000;
  gObjConfigEEPROM.overheatStartTimerLevel2 = i_ms_overheat_initiate_level_2 / 1000;
  gObjConfigEEPROM.overheatStartTimerLevel1 = i_ms_overheat_initiate_level_1 / 1000;
  gObjConfigEEPROM.overheatLevel5 = b_overheat_level_5 ? 2 : 1;
  gObjConfigEEPROM.overheatLevel4 = b_overheat_level_4 ? 2 : 1;
  gObjConfigEEPROM.overheatLevel3 = b_overheat_level_3 ? 2 : 1;
  gObjConfigEEPROM.overheatLevel2 = b_overheat_level_2 ? 2 : 1;
  gObjConfigEEPROM.overheatLevel1 = b_overheat_level_1 ? 2 : 1;
  gObjConfigEEPROM.barrelSwitchPolarity = i_barrel_switch_polarity;
  gObjConfigEEPROM.wandVibration = i_wand_vibration;

  // Create Preferences object to handle non-volatile storage (NVS).
  Preferences preferences;

  if(preferences.begin("config", false)) {
    preferences.putBytes("config", &gObjConfigEEPROM, sizeof(gObjConfigEEPROM));
    preferences.end();
  }

  updateCRCEEPROM(eepromCRC());
}

// Load config settings from Preferences
void loadConfigEEPROM() {
  // Create Preferences object to handle non-volatile storage (NVS).
  Preferences preferences;

  if(preferences.begin("config", true)) {
    if(preferences.isKey("config")) {
      preferences.getBytes("config", &gObjConfigEEPROM, sizeof(gObjConfigEEPROM));
    }
    preferences.end();
  }
}

// Clear config settings in Preferences
void clearConfigEEPROM() {
  // Create Preferences object to handle non-volatile storage (NVS).
  Preferences preferences;

  if(preferences.begin("config", false)) {
    preferences.clear();
    preferences.end();
  }

  gObjConfigEEPROM = {};

  updateCRCEEPROM(eepromCRC());
}

// CRC helpers for Preferences
void updateCRCEEPROM(uint32_t crc) {
  // Create Preferences object to handle non-volatile storage (NVS).
  Preferences preferences;

  if(preferences.begin("crc", false)) {
    preferences.putUInt("crc", crc);
    preferences.end();
  }
}

uint32_t getCRCEEPROM() {
  uint32_t crc = 0;

  // Create Preferences object to handle non-volatile storage (NVS).
  Preferences preferences;

  if(preferences.begin("crc", true)) {
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
      gpstarWand.setFiringModeCTS(); // At least the CTS mode is enabled.

      if(gObjConfigEEPROM.crossTheStreamsMix == 2) {
        gpstarWand.setFiringModeCTSMix(); // Upgrade to the CTS Mix mode.
      }
    }

    if(gObjConfigEEPROM.overheating > 0 && gObjConfigEEPROM.overheating < 3) {
      b_overheat_enabled = (gObjConfigEEPROM.overheating > 1);
    }

    if(gObjConfigEEPROM.extraProtonSounds > 0 && gObjConfigEEPROM.extraProtonSounds < 3) {
      b_stream_effects = (gObjConfigEEPROM.extraProtonSounds > 1);
    }

    if(gObjConfigEEPROM.neutronaWandSounds > 0 && gObjConfigEEPROM.neutronaWandSounds < 3) {
      b_extra_pack_sounds = (gObjConfigEEPROM.neutronaWandSounds > 1);
    }

    // Load stream mode opts directly (bit-packed flags)
    if(gObjConfigEEPROM.streamModeOpts > 0 && gObjConfigEEPROM.streamModeOpts < 129) {
      // Remember to subtract 1 to get the correct flags.
      gpstarWand.setStreamModeOpts(gObjConfigEEPROM.streamModeOpts - 1);
    }

    if(gObjConfigEEPROM.defaultStreamMode > 0 && gObjConfigEEPROM.defaultStreamMode < 9) {
      DEFAULT_STREAM_MODE = (STREAM_MODES)gObjConfigEEPROM.defaultStreamMode;
      gpstarWand.setStreamMode(DEFAULT_STREAM_MODE);

      if(gpstarWand.inStreamMode(MESON)) {
        if(AUDIO_DEVICE == A_GPSTAR_AUDIO || AUDIO_DEVICE == A_GPSTAR_AUDIO_ADV) {
          // Tell GPStar Audio we need short audio mode.
          audio.gpstarShortTrackOverload(false);
        }
      }
    }

    if(gObjConfigEEPROM.quickVent > 0 && gObjConfigEEPROM.quickVent < 3) {
      b_quick_vent = (gObjConfigEEPROM.quickVent > 1);
    }

    if(gObjConfigEEPROM.wandBootErrors > 0 && gObjConfigEEPROM.wandBootErrors < 3) {
      b_wand_boot_errors = (gObjConfigEEPROM.wandBootErrors > 1);
    }

    if(gObjConfigEEPROM.invertBargraph > 0 && gObjConfigEEPROM.invertBargraph < 3) {
      b_bargraph_invert = (gObjConfigEEPROM.invertBargraph > 1);
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
      b_overheat_bargraph_blink = (gObjConfigEEPROM.bargraphOverheatBlinking > 1);
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

    if(gObjConfigEEPROM.systemMode > 0 && gObjConfigEEPROM.systemMode < 3 && b_wand_standalone) {
      gpstarWand.setSystemMode(gObjConfigEEPROM.systemMode > 1 ? MODE_ORIGINAL : MODE_SUPER_HERO);
    }

    if(gObjConfigEEPROM.beepLoop > 0 && gObjConfigEEPROM.beepLoop < 3) {
      b_beep_loop = (gObjConfigEEPROM.beepLoop > 1);
    }

    if(gObjConfigEEPROM.defaultWandVolume > 0 && gObjConfigEEPROM.defaultWandVolume < 102) {
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
      b_overheat_level_5 = (gObjConfigEEPROM.overheatLevel5 > 1);
    }

    if(gObjConfigEEPROM.overheatLevel4 > 0 && gObjConfigEEPROM.overheatLevel4 < 3) {
      b_overheat_level_4 = (gObjConfigEEPROM.overheatLevel4 > 1);
    }

    if(gObjConfigEEPROM.overheatLevel3 > 0 && gObjConfigEEPROM.overheatLevel3 < 3) {
      b_overheat_level_3 = (gObjConfigEEPROM.overheatLevel3 > 1);
    }

    if(gObjConfigEEPROM.overheatLevel2 > 0 && gObjConfigEEPROM.overheatLevel2 < 3) {
      b_overheat_level_2 = (gObjConfigEEPROM.overheatLevel2 > 1);
    }

    if(gObjConfigEEPROM.overheatLevel1 > 0 && gObjConfigEEPROM.overheatLevel1 < 3) {
      b_overheat_level_1 = (gObjConfigEEPROM.overheatLevel1 > 1);
    }

    if(gObjConfigEEPROM.barrelSwitchPolarity > 0 && gObjConfigEEPROM.barrelSwitchPolarity < 4) {
      switch(gObjConfigEEPROM.barrelSwitchPolarity) {
        case 1:
        default:
          BARREL_SWITCH_POLARITY = SWITCH_DEFAULT;
        break;

        case 2:
          BARREL_SWITCH_POLARITY = SWITCH_INVERTED;
        break;

        case 3:
          BARREL_SWITCH_POLARITY = SWITCH_DISABLED;
        break;
      }
    }

    if(gObjConfigEEPROM.wandVibration > 0 && gObjConfigEEPROM.wandVibration < 5) {
      switch(gObjConfigEEPROM.wandVibration) {
        case 4:
        default:
          // Do nothing. Readings are taken from the vibration toggle switch from the Proton pack or configuration setting in stand alone mode.
          VIBRATION_MODE_EEPROM = VIBRATION_DEFAULT;
          if(b_wand_standalone) {
            gpstarWand.setVibrationMode(VIBRATION_NEVER);
          }
          else {
            gpstarWand.setVibrationMode(VIBRATION_FIRING_ONLY);
          }
        break;

        case 3:
          VIBRATION_MODE_EEPROM = VIBRATION_NEVER;
          gpstarWand.setVibrationMode(VIBRATION_MODE_EEPROM);
        break;

        case 2:
          b_vibration_switch_on = true; // Override the Proton Pack vibration toggle switch.
          VIBRATION_MODE_EEPROM = VIBRATION_FIRING_ONLY;
          gpstarWand.setVibrationMode(VIBRATION_MODE_EEPROM);
        break;

        case 1:
          b_vibration_switch_on = true; // Override the Proton Pack vibration toggle switch.
          VIBRATION_MODE_EEPROM = VIBRATION_ALWAYS;
          gpstarWand.setVibrationMode(VIBRATION_MODE_EEPROM);
        break;
      }
    }

    // Rebuild the overheat enabled power levels.
    updateOverheatLevels();

    // Reset the blinking white LED interval.
    resetWhiteLEDBlinkRate();

    if(gObjLEDEEPROM.barrelSpectralCustom > 0 && gObjLEDEEPROM.barrelSpectralCustom != 255) {
      i_spectral_wand_custom_colour = gObjLEDEEPROM.barrelSpectralCustom;
    }

    if(gObjLEDEEPROM.barrelSpectralSaturationCustom > 0 && gObjLEDEEPROM.barrelSpectralSaturationCustom != 255) {
      i_spectral_wand_custom_saturation = gObjLEDEEPROM.barrelSpectralSaturationCustom;
    }

    if(gObjLEDEEPROM.ventLightAutoIntensity > 0 && gObjLEDEEPROM.ventLightAutoIntensity < 3) {
      b_vent_light_control = (gObjLEDEEPROM.ventLightAutoIntensity > 1);
    }

    if(gObjLEDEEPROM.ventLightStreamColours > 0 && gObjLEDEEPROM.ventLightStreamColours < 3) {
      b_vent_light_stream_colours = (gObjLEDEEPROM.ventLightStreamColours > 1);
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
      BARGRAPH_TYPE_EEPROM = (gObjLEDEEPROM.numBargraphLeds < 30 ? SEGMENTS_28 : SEGMENTS_30);
      BARGRAPH_TYPE = BARGRAPH_TYPE_EEPROM;
    }

    if(gObjLEDEEPROM.gpstarAudioLed > 0 && gObjLEDEEPROM.gpstarAudioLed < 3) {
      b_gpstar_audio_led_enabled = (gObjLEDEEPROM.gpstarAudioLed > 1);
      setAudioLED(b_gpstar_audio_led_enabled);
    }
  }
  else {
    // CRC mismatch; clear preferences
    playEffect(S_VOICE_EEPROM_LOADING_FAILED_RESET);
    clearLEDEEPROM();
    clearConfigEEPROM();
  }
}

// Used to get UI preferences from the device namespace.
void getSpecialPreferences() {
  // Create Preferences object to handle non-volatile storage (NVS).
  Preferences preferences;

  // Accesses the "device" namespace in read-only mode.
  if(preferences.begin("device", true)) {
    // Restore the standalone mode flag from preferences.
    b_wand_standalone = preferences.getBool("standalone", false);

    // Restore the IR while firing flag from preferences.
    b_ir_while_firing = preferences.getBool("ir_while_firing", false);

    // Return stored values if available, otherwise use a default value.
    s_track_listing = preferences.getString("track_list", "");

    // Restore the magnetometer calibration data from preferences.
    if(preferences.isKey("orientation")) {
      switch(preferences.getUChar("orientation", 0)) {
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
      preferences.putBool("standalone", b_wand_standalone);
      preferences.putBool("ir_while_firing", b_ir_while_firing);
      preferences.putString("track_list", "");
      preferences.putUChar("orientation", 3); // COMPONENTS_DOWN_USB_FRONT
      preferences.putBytes("mag_cal", &magCalData, sizeof(magCalData));
      preferences.end();
    }
  }

  // Fallback to the Haslab as default if not set.
  if(INSTALL_ORIENTATION == COMPONENTS_NOT_ORIENTED) {
    INSTALL_ORIENTATION = COMPONENTS_DOWN_USB_FRONT;
  }
}
