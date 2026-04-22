/**
 *   DevicePrefs - Defines device preference structs.
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

// Includes for all ENUM declarations.
#include "Streams.h"
#include "Themes.h"
#include "Vibration.h"

// Preferences for the Proton Pack device.
struct __attribute__((packed)) PackPrefs {
  bool isESP32 = false;
  SYSTEM_MODES defaultSystemModePack = MODE_SUPER_HERO; // [1=SuperHero,2=ModeOriginal]
  SYSTEM_THEMES defaultYearThemePack = SYSTEM_TOGGLE_SWITCH; // [1=TOGGLE,2=1984,3=1989,4=AFTERLIFE,5=FROZEN_EMPIRE]
  SYSTEM_THEMES currentYearThemePack = SYSTEM_AFTERLIFE; // [2=1984,3=1989,4=AFTERLIFE,5=FROZEN_EMPIRE]
  VIBRATION_MODES packVibration = VIBRATION_DEFAULT; // [1=ALWAYS,2=FIRING_ONLY,3=NEVER,4=DEFAULT,5=CYCLOTRON_MOTOR]
  uint8_t defaultPackVolume = 50; // 5-100
  bool fadeoutIdleSounds = false;
  bool ribbonCableAlarm = false; // Ignore ribbon cable switch state
  bool wandQuickBootup = false;
  bool cyclotronDirection = false;
  bool demoLightMode = false;
  bool protonStreamEffects = false;
  bool brassStartupLoop = false;
  bool overheatStrobeNF = false;
  bool overheatSyncToFan = false;
  bool overheatLightsOff = false;
  uint8_t ledCycLidCount = 12; // [12,20,36,40]
  uint8_t ledCycLidHue = 2; // Spectral custom colour/hue 2-254
  uint8_t ledCycLidSat = 2; // Spectral custom saturation 2-254
  uint8_t ledCycLidLum = 50; // Brightness 20-100
  uint8_t ledCycLidCenter = 0; // [0=3,1=1]
  bool ledCycLidFade = false;
  bool ledCycLidSimRing = false;
  bool disableLidDetection = false; // Ignore cyclotron lid switch state
  uint8_t ledCycInnerPanel = 1; // [1=Individual,2=RGB-Static,3=RGB-Dynamic]
  uint8_t ledCycPanLum = 50; // Brightness 0-100
  uint8_t ledCycCakeCount = 12; // [12,23,24,26,35,36]
  uint8_t ledCycCakeHue = 2; // Spectral custom colour/hue 2-254
  uint8_t ledCycCakeSat = 2; // Spectral custom saturation 2-254
  uint8_t ledCycCakeLum = 50; // Brightness 20-100
  bool ledCycCakeGRB = false;
  uint8_t ledCycCavCount = 0; // Cyclotron cavity LEDs (0-20)
  uint8_t ledCycCavType = 0; // Cyclotron cavity LED Type
  bool ledVGCyclotron = false;
  uint8_t ledPowercellCount = 13; //[13,15]
  bool ledInvertPowercell = false;
  uint8_t ledPowercellHue = 2; // Spectral custom colour/hue 2-254
  uint8_t ledPowercellSat = 2; // Spectral custom saturation 2-254
  uint8_t ledPowercellLum = 50; // Brightness 20-100
  bool ledVGPowercell = false;
  bool gpstarAudioLed = false;
  bool isWiFiEnabled = false; // WiFi Enabled (true) or Disabled (false)
  bool resetWifiPassword = false;
};

// Output a compiler message if the final struct exceeds a specific size needed for SerialTransfer.
static_assert(sizeof(PackPrefs) < 85, "WARNING: PackPrefs has grown too large (>84 bytes)");

// Preferences for the Neutrona Wand device.
struct __attribute__((packed)) WandPrefs {
  bool isESP32 = false;
  uint8_t ledWandCount = 0; // [0=5 (Stock), 1=48 (Frutto), 2=50 (GPStar), 3=2 (Tip)]
  uint8_t ledWandHue = 2; // Spectral custom colour/hue 2-254
  uint8_t ledWandSat = 2; // Spectral custom saturation 2-254
  bool rgbVentEnabled = false;
  bool overheatEnabled = false;
  uint8_t streamFlags = FLAG_PROTON; // Represents STREAM_MODE_FLAGS (managed by a DeviceState)
  uint8_t defaultStreamMode = PROTON;
  uint8_t defaultFiringMode = 0; // [0=VG,1=CTS,3=CTS_MIX]
  VIBRATION_MODES wandVibration = VIBRATION_DEFAULT; // [1=ALWAYS,2=FIRING_ONLY,3=NEVER,4=DEFAULT]
  uint8_t barrelSwitchPolarity = 1; // [1=DEFAULT,2=INVERTED,3=DISABLED]
  bool wandSoundsToPack = false;
  bool quickVenting = false;
  bool rgbVentColours = false;
  bool autoVentLight = false;
  bool wandBeepLoop = false;
  bool wandBootError = false;
  SYSTEM_THEMES defaultYearModeWand = SYSTEM_TOGGLE_SWITCH; // [1=TOGGLE,2=1984,3=1989,4=AFTERLIFE,5=FROZEN_EMPIRE]
  uint8_t defaultYearModeCTS = 1; // [1=TOGGLE,2=1984,4=2021]
  uint8_t defaultWandVolume = 50; // 5-100
  uint8_t numBargraphSegments = 28; // [28=28-segment,30=30-segment]
  bool invertWandBargraph = false;
  bool bargraphOverheatBlink = false;
  uint8_t bargraphIdleAnimation = 1; // [1=System,2=SuperHero,3=ModeOriginal]
  uint8_t bargraphFireAnimation = 1; // [1=System,2=SuperHero,3=ModeOriginal]
  bool gpstarAudioLed = false;
  bool isWiFiEnabled = false; // WiFi Enabled (true) or Disabled (false)
  bool resetWifiPassword = false;
};

// Output a compiler message if the final struct exceeds a specific size needed for SerialTransfer.
static_assert(sizeof(WandPrefs) < 35, "WARNING: WandPrefs has grown too large (>34 bytes)");

// Preferences for smoke/overheat behavior.
struct __attribute__((packed)) SmokePrefs {
  bool smokeEnabled = false;
  bool overheatContinuous5 = false;
  bool overheatContinuous4 = false;
  bool overheatContinuous3 = false;
  bool overheatContinuous2 = false;
  bool overheatContinuous1 = false;
  uint8_t overheatDuration5 = 2; // 2-60 Seconds
  uint8_t overheatDuration4 = 2; // 2-60 Seconds
  uint8_t overheatDuration3 = 2; // 2-60 Seconds
  uint8_t overheatDuration2 = 2; // 2-60 Seconds
  uint8_t overheatDuration1 = 2; // 2-60 Seconds
  bool overheatLevel5 = false;
  bool overheatLevel4 = false;
  bool overheatLevel3 = false;
  bool overheatLevel2 = false;
  bool overheatLevel1 = false;
  uint8_t overheatDelay5 = 2; // 2-60 Seconds
  uint8_t overheatDelay4 = 2; // 2-60 Seconds
  uint8_t overheatDelay3 = 2; // 2-60 Seconds
  uint8_t overheatDelay2 = 2; // 2-60 Seconds
  uint8_t overheatDelay1 = 2; // 2-60 Seconds
};

// Output a compiler message if the final struct exceeds a specific size needed for SerialTransfer.
static_assert(sizeof(SmokePrefs) < 35, "WARNING: SmokePrefs has grown too large (>34 bytes)");

// Data for synchronizing the Neutrona Wand.
struct __attribute__((packed)) WandSyncData {
  SYSTEM_MODES systemMode = MODE_SUPER_HERO;
  SYSTEM_THEMES systemTheme = SYSTEM_AFTERLIFE;
  uint8_t streamFlags = FLAG_PROTON;
  STREAM_MODES streamMode = PROTON;
  bool ionArmSwitch = false; // Limited to a binary state for this purpose.
  bool cyclotronLidState = false;
  bool packOn = false;
  POWER_LEVELS powerLevel = LEVEL_5;
  bool vibrationToggle = false; // Only tracks the pack's physical toggle state.
  uint8_t packAudioVersion = 0;
  uint8_t effectsVolume = 50;
  bool masterMuted = false;
  uint8_t musicStatus = 0;
  bool repeatMusicTrack = false;
  bool shuffleMusicTracks = false;
};

// Output a compiler message if the final struct exceeds a specific size needed for SerialTransfer.
static_assert(sizeof(WandSyncData) < 35, "WARNING: WandSyncData has grown too large (>34 bytes)");

// Data for synchronizing the Attenuator.
struct __attribute__((packed)) AttenuatorSyncData {
  SYSTEM_MODES systemMode = MODE_SUPER_HERO;
  SYSTEM_THEMES systemTheme = SYSTEM_AFTERLIFE;
  uint8_t streamFlags = FLAG_PROTON;
  STREAM_MODES streamMode = PROTON;
  bool ionArmSwitch = false; // Limited to a binary state for this purpose.
  bool cyclotronLidState = false;
  bool packOn = false;
  POWER_LEVELS powerLevel = LEVEL_5;
  bool wandPresent = false;
  bool barrelExtended = false;
  bool wandFiring = false;
  bool overheatingNow = false;
  uint8_t speedMultiplier = 1;
  uint8_t spectralColour = 0;
  uint8_t spectralSaturation = 0;
  bool masterMuted = false;
  uint8_t masterVolume = 50;
  uint8_t effectsVolume = 50;
  uint8_t musicVolume = 50;
  bool musicPlaying = false;
  bool musicPaused = false;
  bool trackLooped = false;
  bool shuffleTracks = false;
  uint16_t currentTrack = 0;
  uint16_t musicCount = 0;
  uint16_t packAudioVersion = 0;
  uint16_t wandAudioVersion = 0;
  uint16_t packVoltage = 0;
  bool audioCorrupt = false;
  bool audioOutdated = false;
};

// Output a compiler message if the final struct exceeds a specific size needed for SerialTransfer.
static_assert(sizeof(AttenuatorSyncData) < 85, "WARNING: AttenuatorSyncData has grown too large (>84 bytes)");
