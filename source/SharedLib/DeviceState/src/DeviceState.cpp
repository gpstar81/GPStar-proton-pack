/**
 *   DeviceState - Object definition for current state of a device.
 *   Provides helper methods for managing changes to properties/states.
 *   Copyright (C) 2023-2026 Michael Rajotte, Dustin Grau, Nomake Wan
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

/**
 * Every device (pack, wand, etc.) should have a global instance of DeviceState
 * to track the current state of the device or the overall system. This will
 * allow devices to remain in sync as modes and themes are changed when data
 * is synchronized across serial connections or via WebSocket communications.
 */

// Standard library includes for math and string functions
#include <math.h>
#include <string.h>
#include <stdio.h>

// Library Header
#include "DeviceState.h"

// Create the single global instances of each config/sync object.
PackPrefs packConfig;
WandPrefs wandConfig;
SmokePrefs smokeConfig;
WandSyncData wandSyncData;
AttenuatorSyncData attenuatorSyncData;

// Constructor - Initializes all default device settings
DeviceState::DeviceState()
  : systemMode(MODE_SUPER_HERO), // Default to Super Hero mode for all stream modes.
    ionArmSwitch(RED_SWITCH_OFF), // Assume the Ion Arm switch is off.
    systemTheme(SYSTEM_AFTERLIFE), // Default to Afterlife theme until changed.
    streamMode(PROTON), // [Private] Always begin in Proton stream mode.
    streamModePrevious(PROTON), // [Private] Remember the previous stream mode.
    streamModeOpts(FLAG_STASIS | FLAG_SLIME | FLAG_MESON | FLAG_SPECTRAL | FLAG_HOLIDAY_HALLOWEEN | FLAG_HOLIDAY_CHRISTMAS | FLAG_SPECTRAL_CUSTOM), // [Private] By default, enable all standard streams.
    powerLevel(LEVEL_5), // Always begin at maximum power level.
    powerLevelPrevious(LEVEL_5), // Remember the default power level.
    firingMode(FLAG_VG_MODE), // By default, the only firing mode is VG Mode.
    firingModePrevious(FLAG_VG_MODE), // Remember the default firing mode value.
    barrelState(BARREL_UNKNOWN), // Set to unknown for bootup to prevent sounds from playing erroneously.
    vibrationMode(VIBRATION_NEVER) // Do not assume that vibration is enabled.
{
  // Put custom initialization code here if needed.
}

// Getter for systemMode (private variable)
SYSTEM_MODES DeviceState::getSystemMode() const {
  return systemMode;
}

/**
 * Set the system mode along with all related flags/modes.
 */
bool DeviceState::setSystemMode(SYSTEM_MODES mode) {
  switch(mode) {
    case MODE_DEFAULT:
    case MODE_SUPER_HERO:
      systemMode = MODE_SUPER_HERO;
      restorePreviousFiringMode();
      return true;
    break;
    case MODE_ORIGINAL:
      // Super Hero mode can retain the current firing mode.
      systemMode = MODE_ORIGINAL;
      ionArmSwitch = RED_SWITCH_OFF; // Force red switch off in Original mode.
      streamMode = PROTON; // Force stream mode to PROTON.
      if(firingMode == FLAG_VG_MODE) {
        firingMode = FLAG_CTS_MODE; // Force to a known base firing mode.
      }
      return true;
    break;
    default:
      return false; // Invalid mode specified.
    break;
  }
}

// Getter for ionArmSwitch (private variable)
RED_SWITCH_MODES DeviceState::getIonArmSwitch() const {
  if(systemMode == MODE_SUPER_HERO) {
    return RED_SWITCH_ON; // For Super Hero mode, always report switch as ON.
  }

  return ionArmSwitch;
}

// Setter for ionArmSwitch (private variable)
bool DeviceState::setIonArmSwitch(RED_SWITCH_MODES state) {
  ionArmSwitch = state;
  return true;
}

// Getter for systemTheme (private variable)
SYSTEM_THEMES DeviceState::getSystemTheme() const {
  return systemTheme;
}

// Special getter for related system themes (1980s themes)
bool DeviceState::isTheme80s() const {
  return (systemTheme == SYSTEM_1984 || systemTheme == SYSTEM_1989);
}

// Special getter for related system themes (2000s themes)
bool DeviceState::isThemeModern() const {
  return (systemTheme == SYSTEM_AFTERLIFE || systemTheme == SYSTEM_FROZEN_EMPIRE);
}

// Setter for systemTheme (private variable)
void DeviceState::setSystemTheme(SYSTEM_THEMES theme) {
  systemTheme = theme;
}

// Getter for streamModeOpts (private variable)
uint8_t DeviceState::getStreamModeOpts() const {
  return streamModeOpts;
}

// Setter for streamModeOpts (private variable)
void DeviceState::setStreamModeOpts(uint8_t value) {
    streamModeOpts = value;
}

// Resets all stream mode options to none.
void DeviceState::clearStreamFlags() {
  streamModeOpts = FLAG_PROTON;
}

// Enable all of the VG stream flags.
void DeviceState::enableVGStreams() {
  if(!isFiringModeVG()) { return; } // Only allowed in VG firing mode (which implies SUPER_HERO system mode).
  streamModeOpts |= (FLAG_STASIS | FLAG_SLIME | FLAG_MESON);
}

// Enable all spectral and holiday stream flags.
void DeviceState::enableAllSpectralStreams() {
  if(!isFiringModeVG()) { return; } // Only allowed in VG firing mode (which implies SUPER_HERO system mode).
  streamModeOpts |= (FLAG_SPECTRAL | FLAG_SPECTRAL_CUSTOM | FLAG_HOLIDAY_HALLOWEEN | FLAG_HOLIDAY_CHRISTMAS);
}

// Enable only the STASIS flag.
void DeviceState::enableStasisStream() {
  if(!isFiringModeVG()) { return; } // Only allowed in VG firing mode (which implies SUPER_HERO system mode).
  streamModeOpts |= FLAG_STASIS;
}

// Enable only the SLIME flag.
void DeviceState::enableSlimeStream() {
  if(!isFiringModeVG()) { return; } // Only allowed in VG firing mode (which implies SUPER_HERO system mode).
  streamModeOpts |= FLAG_SLIME;
}

// Enable only the MESON flag.
void DeviceState::enableMesonStream() {
  if(!isFiringModeVG()) { return; } // Only allowed in VG firing mode (which implies SUPER_HERO system mode).
  streamModeOpts |= FLAG_MESON;
}

// Enable only the SPECTRAL flag.
void DeviceState::enableSpectralStream() {
  if(!isFiringModeVG()) { return; } // Only allowed in VG firing mode (which implies SUPER_HERO system mode).
  streamModeOpts |= FLAG_SPECTRAL;
}

// Enable only the SPECTRAL_CUSTOM flag.
void DeviceState::enableSpectralCustomStream() {
  if(!isFiringModeVG()) { return; } // Only allowed in VG firing mode (which implies SUPER_HERO system mode).
  streamModeOpts |= FLAG_SPECTRAL_CUSTOM;
}

// Enable only the HOLIDAY_HALLOWEEN flag.
void DeviceState::enableHalloweenStream() {
  if(!isFiringModeVG()) { return; } // Only allowed in VG firing mode (which implies SUPER_HERO system mode).
  streamModeOpts |= FLAG_HOLIDAY_HALLOWEEN;
}

// Enable only the HOLIDAY_CHRISTMAS flag.
void DeviceState::enableChristmasStream() {
  if(!isFiringModeVG()) { return; } // Only allowed in VG firing mode (which implies SUPER_HERO system mode).
  streamModeOpts |= FLAG_HOLIDAY_CHRISTMAS;
}

// Disable all of the VG stream flags.
void DeviceState::disableVGStreams() {
  streamModeOpts &= ~(FLAG_STASIS | FLAG_SLIME | FLAG_MESON);
}

// Disable only the STASIS flag.
void DeviceState::disableStasisStream() {
  streamModeOpts &= ~FLAG_STASIS;
}

// Disable only the SLIME flag.
void DeviceState::disableSlimeStream() {
  streamModeOpts &= ~FLAG_SLIME;
}

// Disable only the MESON flag.
void DeviceState::disableMesonStream() {
  streamModeOpts &= ~FLAG_MESON;
}

// Disable only the SPECTRAL flag.
void DeviceState::disableSpectralStream() {
  streamModeOpts &= ~FLAG_SPECTRAL;
}

// Disable only the SPECTRAL_CUSTOM flag.
void DeviceState::disableSpectralCustomStream() {
  streamModeOpts &= ~FLAG_SPECTRAL_CUSTOM;
}

// Disable only the HOLIDAY_HALLOWEEN flag.
void DeviceState::disableHalloweenStream() {
  streamModeOpts &= ~FLAG_HOLIDAY_HALLOWEEN;
}

// Disable only the HOLIDAY_CHRISTMAS flag.
void DeviceState::disableChristmasStream() {
  streamModeOpts &= ~FLAG_HOLIDAY_CHRISTMAS;
}

// Remove spectral and holiday stream flags (keeps PROTON and VG streams).
void DeviceState::removeAllSpectralStreams() {
  // Use bitwise AND with inverted mask to clear only spectral/holiday flags.
  // Step 1: OR together all spectral flags to create a mask (bits set to 1 for flags to remove).
  // Step 2: Invert the mask with ~ so those bits become 0 and all others become 1.
  // Step 3: AND with streamModeOpts to clear only the spectral flags while preserving others.
  streamModeOpts &= ~(FLAG_SPECTRAL | FLAG_SPECTRAL_CUSTOM | FLAG_HOLIDAY_HALLOWEEN | FLAG_HOLIDAY_CHRISTMAS);
}

// Returns the required streamModeOpts flag for a given stream mode.
STREAM_MODE_FLAGS DeviceState::getRequiredStreamFlag(STREAM_MODES mode) const {
  switch (mode) {
    case PROTON:
    default:
      return FLAG_PROTON;
    break;
    case STASIS:
      return FLAG_STASIS;
    break;
    case SLIME:
      return FLAG_SLIME;
    break;
    case MESON:
      return FLAG_MESON;
    break;
    case SPECTRAL:
      return FLAG_SPECTRAL;
    break;
    case SPECTRAL_CUSTOM:
      return FLAG_SPECTRAL_CUSTOM;
    break;
    case HOLIDAY_HALLOWEEN:
      return FLAG_HOLIDAY_HALLOWEEN;
    break;
    case HOLIDAY_CHRISTMAS:
      return FLAG_HOLIDAY_CHRISTMAS;
    break;
  }
}

// Return a boolean result if a specific stream mode flag is enabled or not.
bool DeviceState::hasStreamFlag(STREAM_MODE_FLAGS flag) const {
  if(flag == FLAG_PROTON) {
    return true; // No flag required, always supported.
  }

  // Otherwise check if the flag is set in streamModeOpts.
  return !!(streamModeOpts & flag);
}

// Return a boolean result if the device state supports a specific stream mode.
bool DeviceState::supportsStreamMode(STREAM_MODES mode) const {
  if(systemMode == MODE_ORIGINAL && mode != PROTON) { return false; } // Not allowed in Mode Original.
  if(firingMode != FLAG_VG_MODE && mode != PROTON) { return false; } // Not allowed in CTS/CTS Mix.
  return hasStreamFlag(getRequiredStreamFlag(mode));
}

// Return a boolean result if the device state supports any/all VG streams.
bool DeviceState::supportsVGStreams() const {
  if(systemMode == MODE_ORIGINAL) { return false; } // Not allowed in Mode Original.
  if(firingMode != FLAG_VG_MODE) { return false; } // Not allowed in CTS/CTS Mix.
  return (streamModeOpts & (FLAG_STASIS | FLAG_SLIME | FLAG_MESON)) != 0;
}

// Return a boolean result if all VG streams are enabled.
bool DeviceState::supportsAllVGStreams() const {
  if(systemMode == MODE_ORIGINAL) { return false; } // Not allowed in Mode Original.
  if(firingMode != FLAG_VG_MODE) { return false; } // Not allowed in CTS/CTS Mix.
  return (streamModeOpts == (FLAG_STASIS | FLAG_SLIME | FLAG_MESON));
}

// Return a boolean result if any spectral or holiday stream modes are enabled.
bool DeviceState::supportsSpectralStreams() const {
  if(systemMode == MODE_ORIGINAL) { return false; } // Not allowed in Mode Original.
  if(firingMode != FLAG_VG_MODE) { return false; } // Not allowed in CTS/CTS Mix.
  return (streamModeOpts & (FLAG_SPECTRAL | FLAG_SPECTRAL_CUSTOM | FLAG_HOLIDAY_HALLOWEEN | FLAG_HOLIDAY_CHRISTMAS)) != 0;
}

// Return a boolean result if all spectral and holiday stream modes are enabled.
bool DeviceState::supportsAllSpectralStreams() const {
  if(systemMode == MODE_ORIGINAL) { return false; } // Not allowed in Mode Original.
  if(firingMode != FLAG_VG_MODE) { return false; } // Not allowed in CTS/CTS Mix.
  return (streamModeOpts == (FLAG_SPECTRAL | FLAG_SPECTRAL_CUSTOM | FLAG_HOLIDAY_HALLOWEEN | FLAG_HOLIDAY_CHRISTMAS));
}

// Return a boolean result if any stream other than Proton is enabled.
bool DeviceState::supportsAnyAlternateStreams() const {
  if(systemMode == MODE_ORIGINAL) { return false; } // Not allowed in Mode Original.
  if(firingMode != FLAG_VG_MODE) { return false; } // Not allowed in CTS/CTS Mix.
  return (streamModeOpts != FLAG_PROTON);
}

// Check if the requested stream mode is already the current mode.
bool DeviceState::inStreamMode(STREAM_MODES mode) const {
  return streamMode == mode;
}

// Return the current stream mode as the expected ENUM value.
STREAM_MODES DeviceState::getStreamMode() const {
  return streamMode;
}

// Return the previous stream mode as the expected ENUM value.
STREAM_MODES DeviceState::getPreviousStreamMode() const {
  return streamModePrevious;
}

/**
 * Return the current stream mode as a uint8_t (needed for API calls).
 */
uint8_t DeviceState::getStreamModeByte() const {
  return (uint8_t)streamMode;
}

// Check if recently switched from a specific stream mode.
bool DeviceState::switchedFromStream(STREAM_MODES mode) const {
  return streamModePrevious == mode;
}

/**
 * Attempt to set a distinct stream mode.
 * Only allows change if the required streamModeOpts flag is set.
 * Returns true on success, false if required flag is not enabled or mode already set.
 */
bool DeviceState::setStreamMode(STREAM_MODES mode) {
  // If already in the requested mode, no change is possible.
  if(inStreamMode(mode)) { return false; }

  // If the target stream mode is not supported, no change is possible.
  if(!supportsStreamMode(mode)) { return false; }

  // Change stream mode and return true.
  streamModePrevious = streamMode;
  streamMode = mode;
  return true;
}

/**
 * Returns the next supported stream mode in the range 0-16, wrapping around if needed.
 *
 * Starts from the current streamMode and searches for the next valid mode.
 * If the end of the range is reached, wraps around to the beginning.
 * Only modes supported by the device (as determined by supportsStreamMode) are returned.
 */
STREAM_MODES DeviceState::nextStreamMode() const {
  // Start from the current stream mode.
  STREAM_MODES candidate = streamMode;

  do {
    // Move to next mode, wrap to first mode if we go past LAST_SWITCHABLE_STREAM_MODE.
    candidate = static_cast<STREAM_MODES>(
      (candidate + 1) > LAST_SWITCHABLE_STREAM_MODE ? PROTON : candidate + 1
    );

    // supportsStreamMode checks if the candidate mode is available for this device.
  } while (!supportsStreamMode(candidate));

  return candidate;
}

/**
 * Returns the previous supported stream mode in the range 0-16, wrapping around if needed.
 *
 * Starts from the current streamMode and searches for the previous valid mode.
 * If the beginning of the range is reached, wraps around to the end.
 * Only modes supported by the device (as determined by supportsStreamMode) are returned.
 */
STREAM_MODES DeviceState::previousStreamMode() const {
  // Start from the current stream mode.
  STREAM_MODES candidate = streamMode;

  do {
    // Move to previous mode, wrap to LAST_SWITCHABLE_STREAM_MODE if we go below first mode.
    candidate = static_cast<STREAM_MODES>(
      (candidate == PROTON) ? LAST_SWITCHABLE_STREAM_MODE : candidate - 1
    );

    // supportsStreamMode checks if the candidate mode is available for this device.
  } while (!supportsStreamMode(candidate));

  return candidate;
}

// Getter for firingMode (private variable)
uint8_t DeviceState::getFiringMode() const {
  return firingMode;
}

// Is the current firing mode Video Game [VG] Mode?
bool DeviceState::isFiringModeVG() const {
  return firingMode == FLAG_VG_MODE; // Effectively no flags set.
}

// Is the current firing mode Cross-The-Streams [CTS] Mode?
bool DeviceState::isFiringModeCTS() const {
  return firingMode == FLAG_CTS_MODE; // Only checks for CTS Mode flag.
}

// Is the current firing mode Cross-The-Streams Mix [CTSMix] Mode?
bool DeviceState::isFiringModeCTSMix() const {
  // Must have both CTS Mode and CTS Mix Mode flags set.
  return firingMode == (FLAG_CTS_MODE | FLAG_CTS_MIX_MODE);
}

// Set the firing mode to Video Game [VG] Mode.
void DeviceState::setFiringModeVG() {
  systemMode = MODE_SUPER_HERO; // Must force back into Super Hero mode.
  firingMode = FLAG_VG_MODE; // Clears the firing mode back to defaults.
  firingModePrevious = firingMode; // Resets the last-known firing mode.
}

// Set the firing mode to Cross-The-Streams [CTS] Mode.
void DeviceState::setFiringModeCTS() {
  firingMode = FLAG_CTS_MODE; // Only sets the CTS Mode flag.
  if(systemMode == MODE_SUPER_HERO) {
    firingModePrevious = firingMode; // Only update if in MODE_SUPER_HERO.
  }
}

// Set the firing mode to Cross-The-Streams Mix [CTSMix] Mode.
void DeviceState::setFiringModeCTSMix() {
  firingMode = (FLAG_CTS_MODE | FLAG_CTS_MIX_MODE); // Sets both CTS Mode and CTS Mix Mode flags.
  if(systemMode == MODE_SUPER_HERO) {
    firingModePrevious = firingMode; // Only update if in MODE_SUPER_HERO.
  }
}

// Restore the previous firing mode.
void DeviceState::restorePreviousFiringMode() {
  if(firingMode != firingModePrevious) {
    if(firingModePrevious == FLAG_VG_MODE) {
      setFiringModeVG();
    }
    else if(firingModePrevious == FLAG_CTS_MODE) {
      setFiringModeCTS();
    }
    else if(firingModePrevious == (FLAG_CTS_MODE | FLAG_CTS_MIX_MODE)) {
      setFiringModeCTSMix();
    }
    else {
      setFiringModeVG(); // Default fallback
    }
  }
}

// Get the current power level.
POWER_LEVELS DeviceState::getPowerLevel() const {
  return powerLevel;
}

POWER_LEVELS DeviceState::getPreviousPowerLevel() const {
  return powerLevelPrevious;
}

// Set the power level, returning true on success.
bool DeviceState::setPowerLevel(POWER_LEVELS level) {
  if(level < LEVEL_1 || level > LEVEL_5) {
    return false; // Invalid power level specified.
  }

  powerLevelPrevious = powerLevel; // Store previous power level.
  powerLevel = level;
  return true;
}

// Restores previously-set power level, returning true on success.
bool DeviceState::restorePowerLevel() {
  if(powerLevelPrevious < MIN_POWER_LEVEL || powerLevelPrevious > MAX_POWER_LEVEL) { return false; }
  POWER_LEVELS temp = powerLevel;
  powerLevel = powerLevelPrevious;
  powerLevelPrevious = temp;
  return true;
}

// Increase the power level by 1, returning true on success.
bool DeviceState::increasePowerLevel() {
  if(powerLevel == MAX_POWER_LEVEL) { return false; }
  powerLevelPrevious = powerLevel;
  powerLevel++;
  return true;
}

// Decrease the power level by 1, returning true on success.
bool DeviceState::decreasePowerLevel() {
  if(powerLevel == MIN_POWER_LEVEL) { return false; }
  powerLevelPrevious = powerLevel;
  powerLevel--;
  return true;
}

// Getter for barrelState (private variable)
BARREL_STATES DeviceState::getBarrelState() const {
  return barrelState;
}

// Setter for barrelState (private variable)
bool DeviceState::setBarrelState(BARREL_STATES state) {
  barrelState = state;
  return true;
}

// Getter for vibrationMode (private variable)
VIBRATION_MODES DeviceState::getVibrationMode() const {
  return vibrationMode;
}

// Setter for vibrationMode (private variable)
bool DeviceState::setVibrationMode(VIBRATION_MODES mode) {
  vibrationMode = mode;
  return true;
}

/**
 * Export current DeviceState to WandSyncData struct.
 * Converts enum values to appropriate types for synchronization.
 */
void DeviceState::exportData(WandSyncData& syncData) const {
  syncData.systemMode = getSystemMode();
  syncData.systemTheme = getSystemTheme();
  syncData.streamFlags = getStreamModeOpts();
  syncData.streamMode = getStreamMode();
  syncData.ionArmSwitch = (getIonArmSwitch() == RED_SWITCH_ON);
  syncData.powerLevel = getPowerLevel();
  // Note: Other fields like cyclotronLidState, packOn, effectsVolume, etc.
  // are not part of DeviceState and should be set by the calling code.
}

/**
 * Export current DeviceState to AttenuatorSyncData struct.
 * Converts enum values to appropriate types for synchronization.
 */
void DeviceState::exportData(AttenuatorSyncData& syncData) const {
  syncData.systemMode = getSystemMode();
  syncData.systemTheme = getSystemTheme();
  syncData.streamFlags = getStreamModeOpts();
  syncData.streamMode = getStreamMode();
  syncData.ionArmSwitch = (getIonArmSwitch() == RED_SWITCH_ON);
  syncData.powerLevel = getPowerLevel();
  syncData.barrelExtended = (getBarrelState() == BARREL_EXTENDED);
  // Note: Other fields like wandPresent, wandFiring, volumes, etc.
  // are not part of DeviceState and should be set by the calling code.
}

/**
 * Import WandSyncData struct into current DeviceState.
 * Applies values in proper order using existing setter methods for validation.
 */
void DeviceState::importData(const WandSyncData& syncData) {
  // Apply system mode first as it may reset other values
  setSystemMode(syncData.systemMode);

  // Apply system theme
  setSystemTheme(syncData.systemTheme);

  // Apply stream mode flags (options)
  //setStreamModeOpts(syncData.streamFlags);

  // Apply stream mode
  setStreamMode(syncData.streamMode);

  // Apply ion arm switch
  setIonArmSwitch(syncData.ionArmSwitch ? RED_SWITCH_ON : RED_SWITCH_OFF);

  // Apply power level
  setPowerLevel(syncData.powerLevel);
}

/**
 * Import AttenuatorSyncData struct into current DeviceState.
 * Applies values in proper order using existing setter methods for validation.
 */
void DeviceState::importData(const AttenuatorSyncData& syncData) {
  // Apply system mode first as it may reset other values
  setSystemMode(syncData.systemMode);

  // Apply system theme
  setSystemTheme(syncData.systemTheme);

  // Apply stream flags (options)
  setStreamModeOpts(syncData.streamFlags);

  // Apply stream mode
  setStreamMode(syncData.streamMode);

  // Apply ion arm switch
  setIonArmSwitch(syncData.ionArmSwitch ? RED_SWITCH_ON : RED_SWITCH_OFF);

  // Apply power level
  setPowerLevel(syncData.powerLevel);

  // Apply barrel state
  setBarrelState(syncData.barrelExtended ? BARREL_EXTENDED : BARREL_RETRACTED);
}

/**
 * Helper functions to convert current DeviceState to human-readable strings.
 */

// Convert the system mode to a user-friendly string.
const char* DeviceState::getModeName() const {
  switch(getSystemMode()) {
    case MODE_SUPER_HERO:
      return "Super Hero";
    break;
    case MODE_ORIGINAL:
      return "Original";
    break;
    default:
      return "Unknown";
    break;
  }
}

// Convert the theme mode to a user-friendly string.
const char* DeviceState::getThemeName(SYSTEM_THEMES yearTheme) const {
  if(yearTheme == SYSTEM_EMPTY) {
    // Return the string for whatever mode the system is currently in.
    switch(getSystemTheme()) {
      case SYSTEM_1984:
        return "1984";
      break;
      case SYSTEM_1989:
        return "1989";
      break;
      case SYSTEM_AFTERLIFE:
        return "Afterlife";
      break;
      case SYSTEM_FROZEN_EMPIRE:
        return "Frozen Empire";
      break;
      default:
        return "Unknown";
      break;
    }
  }
  else {
    // Return the string for a specific mode passed as a parameter.
    switch(yearTheme) {
      case SYSTEM_1984:
        return "1984";
      break;
      case SYSTEM_1989:
        return "1989";
      break;
      case SYSTEM_AFTERLIFE:
        return "Afterlife";
      break;
      case SYSTEM_FROZEN_EMPIRE:
        return "Frozen Empire";
      break;
      default:
        return "Unknown";
      break;
    }
  }
}

// Convert the ion arm switch to a user-friendly string.
const char* DeviceState::getIonArmSwitchState() const {
  if(getSystemMode() == MODE_ORIGINAL) {
    // Switch state only matters for mode "Original".
    switch(getIonArmSwitch()) {
      case RED_SWITCH_OFF:
        return "Standby";
      break;
      case RED_SWITCH_ON:
        return "Ready";
      break;
      default:
        return "Unknown";
      break;
    }
  }
  else {
    // Otherwise, just "Ready" (eg. Super Hero mode).
    return "Ready";
  }
}

// Convert the stream mode to a user-friendly string.
const char* DeviceState::getStreamModeName(STREAM_MODES mode) const {
  switch(mode == NULLPARAM ? getStreamMode() : mode) {
    case PROTON:
    default:
      return "Proton Stream";
    break;
    case SLIME:
      // Plasm Distribution System
      return "Plasm System";
    break;
    case STASIS:
      // Dark Matter Generator
      return "Dark Matter Gen.";
    break;
    case MESON:
      // Composite Particle System
      return "Particle System";
    break;
    case SPECTRAL:
      return "Spectral Stream";
    break;
    case HOLIDAY_HALLOWEEN:
      return "Halloween";
    break;
    case HOLIDAY_CHRISTMAS:
      return "Christmas";
    break;
    case SPECTRAL_CUSTOM:
      return "Custom Stream";
    break;
    case SETTINGS:
      return "Settings";
    break;
    case SELFTEST:
      return "Self Test";
    break;
  }
}

// Returns the HTML value string for a given stream mode.
const char* DeviceState::getStreamModeValue(STREAM_MODES mode) const {
  switch(mode) {
    case PROTON:
    default:
      return "proton";
    break;
    case STASIS:
      return "stasis";
    break;
    case SLIME:
      return "slime";
    break;
    case MESON:
      return "meson";
    break;
    case SPECTRAL:
      return "spectral";
    break;
    case HOLIDAY_HALLOWEEN:
      return "holiday/halloween";
    break;
    case HOLIDAY_CHRISTMAS:
      return "holiday/christmas";
    break;
    case SPECTRAL_CUSTOM:
      return "spectral/custom";
    break;
  }
}

// Convert the power level to a user-friendly string.
const char* DeviceState::getPowerLevelName() const {
  switch(getPowerLevel()) {
    case LEVEL_1:
      return "1";
    break;
    case LEVEL_2:
      return "2";
    break;
    case LEVEL_3:
      return "3";
    break;
    case LEVEL_4:
      return "4";
    break;
    case LEVEL_5:
    default:
      return "5";
    break;
  }
}

// Convert the barrel retraction state to a user-friendly string.
const char* DeviceState::getBarrelStateName() const {
  switch(getBarrelState()) {
    case BARREL_RETRACTED:
      return "Safety On";
    break;
    case BARREL_EXTENDED:
      return "Safety Off";
    break;
    default:
      return "Unknown";
    break;
  }
}
