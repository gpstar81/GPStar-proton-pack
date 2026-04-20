/**
 *   Themes - Definitions for available themes in GPStar devices.
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

/*
 * System modes.
 *
 * Super Hero:
 *   - A idealised system based on the close up of the Super Hero Proton Pack and Neutrona Wand in the 1984 Rooftop closeup scene and what is shown in Afterlife.
 *   - Offers different toggle switch sequences for turning on the pack and wand.
 * Mode Original:
 *   - Based on the original operational manual during production of GB1.
 *   - Wand toggle switches must be on before the cyclotron can turn on from the Wand only.
 *
 * Super Hero will be the default system mode.
 */
enum SYSTEM_MODES : uint8_t {
  MODE_DEFAULT = 0,
  MODE_SUPER_HERO = 1,
  MODE_ORIGINAL = 2
};

/**
 * Special state for the Ion Arm switch for Mode Original.
 * The state of this mode is disregarded for the Super Hero mode.
 */
enum RED_SWITCH_MODES : uint8_t {
  RED_SWITCH_OFF = 0,
  RED_SWITCH_ON = 1
};

/**
 * Themes for the system.
 *
 * SYSTEM_EMPTY: No theme selected, used for testing.
 * SYSTEM_TOGGLE_SWITCH: Theme is determined by the physical toggle switch on the Proton Pack.
 * SYSTEM_1984: Theme based on the 1984 movie (GB).
 * SYSTEM_1989: Theme based on the 1989 movie (GB2).
 * SYSTEM_AFTERLIFE: Theme based on the Afterlife (2021) movie; Default theme.
 * SYSTEM_FROZEN_EMPIRE: Theme based on the Frozen Empire (2024) movie.
 */
enum SYSTEM_THEMES : uint8_t {
  SYSTEM_EMPTY = 0,
  SYSTEM_TOGGLE_SWITCH = 1,
  SYSTEM_1984 = 2,
  SYSTEM_1989 = 3,
  SYSTEM_AFTERLIFE = 4,
  SYSTEM_FROZEN_EMPIRE = 5
};
