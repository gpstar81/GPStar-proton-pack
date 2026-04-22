/**
 *   Vibration - Operation modes and settings for vibration feedback.
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

// Vibration operation modes.
enum VIBRATION_MODES : uint8_t {
  VIBRATION_EMPTY = 0,
  VIBRATION_ALWAYS = 1,
  VIBRATION_FIRING_ONLY = 2,
  VIBRATION_NEVER = 3,
  VIBRATION_DEFAULT = 4,
  CYCLOTRON_MOTOR = 5
};
