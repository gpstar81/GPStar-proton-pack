/**
 *   GPStar Single-Shot Blaster
 *   Copyright (C) 2024-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Dustin Grau <dustin.grau@gmail.com>
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

/*
 * Micro SD Card sound files in order. If you have no sound, your SD card might be too slow, try a faster one.
 * File naming 000_ is important. For music, it is 500_ and higher.
 */

enum sound_fx {
  S_EMPTY,
  S_BOOTUP,
  S_SHUTDOWN,
  S_IDLE_LOOP,
  S_FIRE_BLAST,
  S_BEEPS,
  S_BEEPS_LOW,
  S_BEEPS_ALT,
  S_VOICE_EEPROM_LOADING_FAILED_RESET,
  S_VOICE_EEPROM_ERASE,
  S_VOICE_EEPROM_SAVE,
  S_VOICE_LEVEL_1,
  S_VOICE_LEVEL_2,
  S_VOICE_LEVEL_3,
  S_VOICE_LEVEL_4,
  S_VOICE_LEVEL_5,
  S_CLICK,
  S_VOICE_EEPROM_CONFIG_MENU,
  S_DEVICE_READY,
  S_VOICE_VENT_AUTO_INTENSITY_ENABLED,
  S_VOICE_VENT_AUTO_INTENSITY_DISABLED,
  S_VOICE_RGB_VENT_LIGHTS_ENABLED,
  S_VOICE_RGB_VENT_LIGHTS_DISABLED
};

/*
 * Need to keep track which is the last sound effect, so we can iterate over the effects to adjust the volume gain on them.
 */
const uint16_t i_last_effects_track = S_VOICE_RGB_VENT_LIGHTS_DISABLED;