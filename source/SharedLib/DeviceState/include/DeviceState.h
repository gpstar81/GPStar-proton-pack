/**
 *   DeviceState - Object definition for current state of a device.
 *   Provides common properties and enums for operations of hardware.
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

#pragma once

// Includes for common attributes and runtime features.
#include "Streams.h"
#include "Themes.h"
#include "Vibration.h"
#include "DeviceData.h"

class DeviceState {
public:
  // Constructor
  DeviceState();

  // Getters for private variables
  SYSTEM_MODES getSystemMode() const;
  RED_SWITCH_MODES getIonArmSwitch() const;
  SYSTEM_THEMES getSystemTheme() const;
  STREAM_MODES getStreamMode() const;
  STREAM_MODES getPreviousStreamMode() const;
  uint8_t getStreamModeByte() const;
  uint8_t getStreamModeOpts() const;
  POWER_LEVELS getPowerLevel() const;
  POWER_LEVELS getPreviousPowerLevel() const;
  uint8_t getFiringMode() const;
  BARREL_STATES getBarrelState() const;
  VIBRATION_MODES getVibrationMode() const;

  // Theme mode helpers
  bool isTheme80s() const;
  bool isThemeModern() const;

  // Setters for private variables
  bool setSystemMode(SYSTEM_MODES mode);
  bool setIonArmSwitch(RED_SWITCH_MODES state);
  void setSystemTheme(SYSTEM_THEMES theme);
  void setStreamModeOpts(uint8_t value);
  bool setPowerLevel(POWER_LEVELS level);
  bool restorePowerLevel();
  bool increasePowerLevel();
  bool decreasePowerLevel();
  bool setBarrelState(BARREL_STATES state);
  bool setVibrationMode(VIBRATION_MODES mode);

  // Stream flag manipulation helpers
  void clearStreamFlags();
  void enableVGStreams();
  void enableAllSpectralStreams();
  void enableStasisStream();
  void enableSlimeStream();
  void enableMesonStream();
  void enableSpectralStream();
  void enableSpectralCustomStream();
  void enableHalloweenStream();
  void enableChristmasStream();
  void disableVGStreams();
  void disableStasisStream();
  void disableSlimeStream();
  void disableMesonStream();
  void disableSpectralStream();
  void disableSpectralCustomStream();
  void disableHalloweenStream();
  void disableChristmasStream();
  void removeAllSpectralStreams();

  // Stream mode detection/change helpers
  STREAM_MODE_FLAGS getRequiredStreamFlag(STREAM_MODES mode) const;
  bool hasStreamFlag(STREAM_MODE_FLAGS flag) const;
  bool supportsStreamMode(STREAM_MODES mode) const;
  bool supportsVGStreams() const;
  bool supportsAllVGStreams() const;
  bool supportsSpectralStreams() const;
  bool supportsAllSpectralStreams() const;
  bool supportsAnyAlternateStreams() const;
  bool inStreamMode(STREAM_MODES mode) const;
  bool setStreamMode(STREAM_MODES mode);
  bool switchedFromStream(STREAM_MODES mode) const;
  STREAM_MODES nextStreamMode() const;
  STREAM_MODES previousStreamMode() const;

  // Firing mode helpers
  bool isFiringModeVG() const;
  bool isFiringModeCTS() const;
  bool isFiringModeCTSMix() const;
  void setFiringModeVG();
  void setFiringModeCTS();
  void setFiringModeCTSMix();
  void restorePreviousFiringMode();

  // Sync data export methods (DeviceState -> Sync structs)
  void exportData(WandSyncData& syncData) const;
  void exportData(AttenuatorSyncData& syncData) const;

  // Sync data import methods (Sync structs -> DeviceState)
  void importData(const WandSyncData& syncData);
  void importData(const AttenuatorSyncData& syncData);

  // Human-readable string helpers
  const char* getModeName() const;
  const char* getThemeName(SYSTEM_THEMES yearTheme = SYSTEM_EMPTY) const;
  const char* getIonArmSwitchState() const;
  const char* getStreamModeName(STREAM_MODES mode = NULLPARAM) const;
  const char* getStreamModeValue(STREAM_MODES mode) const;
  const char* getPowerLevelName() const;
  const char* getBarrelStateName() const;

private:
  // Private instance properties, either as an ENUM or bytes (for flags).
  SYSTEM_MODES systemMode;
  RED_SWITCH_MODES ionArmSwitch;
  SYSTEM_THEMES systemTheme;
  STREAM_MODES streamMode;
  STREAM_MODES streamModePrevious;
  uint8_t streamModeOpts;
  POWER_LEVELS powerLevel;
  POWER_LEVELS powerLevelPrevious;
  uint8_t firingMode;
  uint8_t firingModePrevious;
  BARREL_STATES barrelState;
  VIBRATION_MODES vibrationMode;
};
