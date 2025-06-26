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
void updateCRCEEPROM();
uint32_t eepromCRC(void);
void bargraphYearModeUpdate();
void resetOverheatLevels();
void resetWhiteLEDBlinkRate();

// Include ESP32 Preferences library
#include <Preferences.h>

// Data structure for LED settings (stored in Preferences)
struct objLEDEEPROM {
  uint8_t barrelSpectralCustom;
  uint8_t barrelSpectralSaturationCustom;
  uint8_t numBarrelLeds;
  uint8_t numBargraphLeds;
  uint8_t rgbVentLight;
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
  Preferences prefs;
  prefs.begin("led", false);
  prefs.putUChar("barrelSpectralCustom", gObjLEDEEPROM.barrelSpectralCustom);
  prefs.putUChar("barrelSpectralSaturationCustom", gObjLEDEEPROM.barrelSpectralSaturationCustom);
  prefs.putUChar("numBarrelLeds", gObjLEDEEPROM.numBarrelLeds);
  prefs.putUChar("numBargraphLeds", gObjLEDEEPROM.numBargraphLeds);
  prefs.putUChar("rgbVentLight", gObjLEDEEPROM.rgbVentLight);
  prefs.end();
}

// Load LED settings from Preferences into the global instance
void loadLEDEEPROM() {
  Preferences prefs;
  prefs.begin("led", true);
  gObjLEDEEPROM.barrelSpectralCustom = prefs.getUChar("barrelSpectralCustom", 0);
  gObjLEDEEPROM.barrelSpectralSaturationCustom = prefs.getUChar("barrelSpectralSaturationCustom", 0);
  gObjLEDEEPROM.numBarrelLeds = prefs.getUChar("numBarrelLeds", 5);
  gObjLEDEEPROM.numBargraphLeds = prefs.getUChar("numBargraphLeds", 28);
  gObjLEDEEPROM.rgbVentLight = prefs.getUChar("rgbVentLight", 1);
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
  prefs.putUChar("crossTheStreams", gObjConfigEEPROM.crossTheStreams);
  prefs.putUChar("crossTheStreamsMix", gObjConfigEEPROM.crossTheStreamsMix);
  prefs.putUChar("overheating", gObjConfigEEPROM.overheating);
  prefs.putUChar("extraProtonSounds", gObjConfigEEPROM.extraProtonSounds);
  prefs.putUChar("neutronaWandSounds", gObjConfigEEPROM.neutronaWandSounds);
  prefs.putUChar("spectralMode", gObjConfigEEPROM.spectralMode);
  prefs.putUChar("holidayMode", gObjConfigEEPROM.holidayMode);
  prefs.putUChar("quickVent", gObjConfigEEPROM.quickVent);
  prefs.putUChar("wandBootErrors", gObjConfigEEPROM.wandBootErrors);
  prefs.putUChar("ventLightAutoIntensity", gObjConfigEEPROM.ventLightAutoIntensity);
  prefs.putUChar("invertBargraph", gObjConfigEEPROM.invertBargraph);
  prefs.putUChar("bargraphMode", gObjConfigEEPROM.bargraphMode);
  prefs.putUChar("bargraphFiringAnimation", gObjConfigEEPROM.bargraphFiringAnimation);
  prefs.putUChar("bargraphOverheatBlinking", gObjConfigEEPROM.bargraphOverheatBlinking);
  prefs.putUChar("neutronaWandYearMode", gObjConfigEEPROM.neutronaWandYearMode);
  prefs.putUChar("ctsMode", gObjConfigEEPROM.ctsMode);
  prefs.putUChar("systemMode", gObjConfigEEPROM.systemMode);
  prefs.putUChar("beepLoop", gObjConfigEEPROM.beepLoop);
  prefs.putUChar("defaultWandVolume", gObjConfigEEPROM.defaultWandVolume);
  prefs.putUChar("overheatStartTimerLevel5", gObjConfigEEPROM.overheatStartTimerLevel5);
  prefs.putUChar("overheatStartTimerLevel4", gObjConfigEEPROM.overheatStartTimerLevel4);
  prefs.putUChar("overheatStartTimerLevel3", gObjConfigEEPROM.overheatStartTimerLevel3);
  prefs.putUChar("overheatStartTimerLevel2", gObjConfigEEPROM.overheatStartTimerLevel2);
  prefs.putUChar("overheatStartTimerLevel1", gObjConfigEEPROM.overheatStartTimerLevel1);
  prefs.putUChar("overheatLevel5", gObjConfigEEPROM.overheatLevel5);
  prefs.putUChar("overheatLevel4", gObjConfigEEPROM.overheatLevel4);
  prefs.putUChar("overheatLevel3", gObjConfigEEPROM.overheatLevel3);
  prefs.putUChar("overheatLevel2", gObjConfigEEPROM.overheatLevel2);
  prefs.putUChar("overheatLevel1", gObjConfigEEPROM.overheatLevel1);
  prefs.putUChar("wandVibration", gObjConfigEEPROM.wandVibration);
  prefs.end();
}

// Load config settings from Preferences
void loadConfigEEPROM() {
  Preferences prefs;
  prefs.begin("config", true);
  gObjConfigEEPROM.crossTheStreams = prefs.getUChar("crossTheStreams", 1);
  gObjConfigEEPROM.crossTheStreamsMix = prefs.getUChar("crossTheStreamsMix", 1);
  gObjConfigEEPROM.overheating = prefs.getUChar("overheating", 2);
  gObjConfigEEPROM.extraProtonSounds = prefs.getUChar("extraProtonSounds", 2);
  gObjConfigEEPROM.neutronaWandSounds = prefs.getUChar("neutronaWandSounds", 2);
  gObjConfigEEPROM.spectralMode = prefs.getUChar("spectralMode", 1);
  gObjConfigEEPROM.holidayMode = prefs.getUChar("holidayMode", 0);
  gObjConfigEEPROM.quickVent = prefs.getUChar("quickVent", 2);
  gObjConfigEEPROM.wandBootErrors = prefs.getUChar("wandBootErrors", 2);
  gObjConfigEEPROM.ventLightAutoIntensity = prefs.getUChar("ventLightAutoIntensity", 2);
  gObjConfigEEPROM.invertBargraph = prefs.getUChar("invertBargraph", 1);
  gObjConfigEEPROM.bargraphMode = prefs.getUChar("bargraphMode", 1);
  gObjConfigEEPROM.bargraphFiringAnimation = prefs.getUChar("bargraphFiringAnimation", 1);
  gObjConfigEEPROM.bargraphOverheatBlinking = prefs.getUChar("bargraphOverheatBlinking", 1);
  gObjConfigEEPROM.neutronaWandYearMode = prefs.getUChar("neutronaWandYearMode", 1);
  gObjConfigEEPROM.ctsMode = prefs.getUChar("ctsMode", 1);
  gObjConfigEEPROM.systemMode = prefs.getUChar("systemMode", 1);
  gObjConfigEEPROM.beepLoop = prefs.getUChar("beepLoop", 2);
  gObjConfigEEPROM.defaultWandVolume = prefs.getUChar("defaultWandVolume", 101);
  gObjConfigEEPROM.overheatStartTimerLevel5 = prefs.getUChar("overheatStartTimerLevel5", 0);
  gObjConfigEEPROM.overheatStartTimerLevel4 = prefs.getUChar("overheatStartTimerLevel4", 0);
  gObjConfigEEPROM.overheatStartTimerLevel3 = prefs.getUChar("overheatStartTimerLevel3", 0);
  gObjConfigEEPROM.overheatStartTimerLevel2 = prefs.getUChar("overheatStartTimerLevel2", 0);
  gObjConfigEEPROM.overheatStartTimerLevel1 = prefs.getUChar("overheatStartTimerLevel1", 0);
  gObjConfigEEPROM.overheatLevel5 = prefs.getUChar("overheatLevel5", 1);
  gObjConfigEEPROM.overheatLevel4 = prefs.getUChar("overheatLevel4", 1);
  gObjConfigEEPROM.overheatLevel3 = prefs.getUChar("overheatLevel3", 1);
  gObjConfigEEPROM.overheatLevel2 = prefs.getUChar("overheatLevel2", 1);
  gObjConfigEEPROM.overheatLevel1 = prefs.getUChar("overheatLevel1", 1);
  gObjConfigEEPROM.wandVibration = prefs.getUChar("wandVibration", 4);
  prefs.end();
}

// Clear config settings in Preferences
void clearConfigEEPROM() {
  Preferences prefs;
  prefs.begin("config", false);
  prefs.clear();
  prefs.end();
}

// CRC helpers for Preferences
void updateCRCEEPROM(uint32_t crc) {
  Preferences prefs;
  prefs.begin("crc", false);
  prefs.putUInt("crc", crc);
  prefs.end();
}

uint32_t getCRCEEPROM() {
  Preferences prefs;
  prefs.begin("crc", true);
  uint32_t crc = prefs.getUInt("crc", 0);
  prefs.end();
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

// Example: Read all user preferences from Preferences (ESP32)
void readEEPROM() {
  uint32_t storedCrc = getCRCEEPROM();
  uint32_t calcCrc = eepromCRC();
  if (storedCrc == calcCrc) {
    loadLEDEEPROM();
    loadConfigEEPROM();
    // Map loaded values to runtime variables as needed
  } else {
    // CRC mismatch: clear preferences
    clearLEDEEPROM();
    clearConfigEEPROM();
  }
}
