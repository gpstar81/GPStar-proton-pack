/**
 *   Streams - Types, features, and power levels for effect streams.
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
#include <stdint.h>

/**
 * Power levels for stream modes.
 */
enum POWER_LEVELS : uint8_t {
  LEVEL_1 = 1,
  LEVEL_2 = 2,
  LEVEL_3 = 3,
  LEVEL_4 = 4,
  LEVEL_5 = 5
};

// Declare the minimum power level for the system for conditional checks.
constexpr uint8_t MIN_POWER_LEVEL = LEVEL_1;

// Declare the maximum power level for the system for conditional checks.
constexpr uint8_t MAX_POWER_LEVEL = LEVEL_5;

inline POWER_LEVELS operator++(POWER_LEVELS& level, int) {
  if (level + 1 > MAX_POWER_LEVEL) {}
  else { level = static_cast<POWER_LEVELS>(level + 1); }
  return level;
}

inline POWER_LEVELS operator--(POWER_LEVELS& level, int) {
  if (level - 1 < MIN_POWER_LEVEL) {}
  else { level = static_cast<POWER_LEVELS>(level - 1); }
  return level;
}

/**
 * Stream modes available, in order of cycling.
 *
 * Types of particle streams which can be fired by the wand:
 *   PROTON: Proton Stream (Default).
 *   STASIS: Dark Matter Generator.
 *   SLIME: Plasm Distribution System.
 *   MESON: Composite Particle System.
 *   SPECTRAL: Spectral (rainbow) stream mode.
 *   HOLIDAY_HALLOWEEN: Halloween holiday stream mode.
 *   HOLIDAY_CHRISTMAS: Christmas holiday stream mode.
 *   SPECTRAL_CUSTOM: Custom Spectral stream mode.
 *
 * Special modes (preempts current stream mode):
 *   SETTINGS: Settings mode (not a stream).
 *   SELFTEST: Self-test mode (not a stream).
 */
enum STREAM_MODES : uint8_t {
  PROTON = 0, // Default stream mode.
  STASIS = 1,
  SLIME = 2,
  MESON = 3,
  SPECTRAL = 4,
  HOLIDAY_HALLOWEEN = 5,
  HOLIDAY_CHRISTMAS = 6,
  SPECTRAL_CUSTOM = 7,
  SETTINGS = 64,
  SELFTEST = 128,
  NULLPARAM = 255
};

// Declare the last switchable stream mode for cycling purposes.
constexpr uint8_t LAST_SWITCHABLE_STREAM_MODE = SPECTRAL_CUSTOM;

/**
 * Flags for stream mode availability.
 *
 * FLAG_PROTON: Proton only, no other streams allowed.
 * FLAG_STASIS: Enables the Dark Matter Generator stream.
 * FLAG_SLIME: Enables the Plasm Distribution System stream.
 * FLAG_MESON: Enables the Composite Particle System stream.
 * FLAG_SPECTRAL: Enables Spectral streams.
 * FLAG_HOLIDAY_HALLOWEEN: Enables Halloween holiday stream.
 * FLAG_HOLIDAY_CHRISTMAS: Enables Christmas holiday stream.
 * FLAG_SPECTRAL_CUSTOM: Enables custom Spectral stream.
 */
enum STREAM_MODE_FLAGS : uint8_t {
  FLAG_PROTON = 0, // Default flag (no special streams).
  FLAG_STASIS = 1,
  FLAG_SLIME = 2,
  FLAG_MESON = 4,
  FLAG_SPECTRAL = 8,
  FLAG_HOLIDAY_HALLOWEEN = 16,
  FLAG_HOLIDAY_CHRISTMAS = 32,
  FLAG_SPECTRAL_CUSTOM = 64
};

/**
 * Firing modes for streams.
 *
 * Note: Cross the Streams [CTS] modes only support a PROTON stream.
 *
 * VG = Supports all possible streams modes, including "Spectral" if Video Game [VG] Modes and Spectral Modes are enabled.
 *      This mode requires the replacement of most stock LEDs with RGB LEDs to function properly.
 *
 * CTS = Pressing the Barrel Wing Button together at the same time as the Intensify button does a cross the streams firing.
 *       You can release one of the two firing buttons and the Neutrona Wand will still continue to cross the streams.
 *
 * CTS Mix = Pressing the Barrel Wing Button together at the same time as the Intensify button does a cross the streams firing.
 *           Releasing the Barrel Wing Switch returns to Proton Stream, and releasing Intensify stops firing completely.
 *           This is an "upgrade" of the CTS mode and requires that flag to be set as well.
 */
enum FIRING_MODE_FLAGS : uint8_t {
  FLAG_VG_MODE = 0,
  FLAG_CTS_MODE = 1,
  FLAG_CTS_MIX_MODE = 2
};

/**
 * Barrel states
 */
enum BARREL_STATES : uint8_t {
  BARREL_UNKNOWN = 0,
  BARREL_RETRACTED = 1,
  BARREL_EXTENDED = 2
};
