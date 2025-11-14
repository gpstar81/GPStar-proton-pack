/**
 *   Communications - Serial communication packet definitions for GPStar devices.
 *   Provides common objects and enums for serial data exchange.
 *   Copyright (C) 2023-2025 Michael Rajotte, Dustin Grau, Nomake Wan
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
 * Data is sent between devices using the SerialTransfer library and struct objects.
 * Every struct uses packed data and each preference is stored as a single byte value,
 * though some values are sent as double-byte on rare occasion. Only ATMega and ESP32
 * are used and both are little-endian so no declaration of byte-order is provided.
 *
 * Note that SerialTransfer encapsulates all data packets with a 4-byte preamble and
 * a 2-byte postamble, adding 6 bytes of overhead to any transfer.
 *
 * Additionally, at 9600 baud it takes roughly 1 millisecond to transfer 1 byte of
 * data. By default, SerialTransfer considers a packet 'stale' after 50 milliseconds.
 * The exception is the Proton Pack to Attenuator connection, which doubles this to
 * 100 milliseconds. Thus it is important to keep the size of any payload plus
 * overhead to less than this timeout length in bytes.
 */

// Types of packets to be sent via serial communication.
enum PACKET_TYPE : uint8_t {
  PACKET_UNKNOWN = 0,
  PACKET_COMMAND = 1,
  PACKET_DATA = 2,
  PACKET_PACK = 3,
  PACKET_WAND = 4,
  PACKET_SMOKE = 5,
  PACKET_SYNC = 6
};

// For command signals (1 byte ID, 2 byte optional data).
struct __attribute__((packed)) CommandPacket {
  uint8_t s;
  uint8_t c;
  uint16_t d1; // Reserved for values over 255 (eg. current music track)
  uint8_t e;
};

// For generic data communication (1 byte ID, 3 byte array).
struct __attribute__((packed)) MessagePacket {
  uint8_t s;
  uint8_t m;
  uint8_t d[3]; // Reserved for multiple, arbitrary byte values.
  uint8_t e;
};

// Preferences for the Proton Pack device.
struct __attribute__((packed)) PackPrefs {
  uint8_t isESP32;
  uint8_t defaultSystemModePack;
  uint8_t defaultYearThemePack;
  uint8_t currentYearThemePack;
  uint8_t defaultPackVolume;
  uint8_t packVibration;
  uint8_t ribbonCableAlarm;
  uint8_t wandQuickBootup;
  uint8_t cyclotronDirection;
  uint8_t demoLightMode;
  uint8_t protonStreamEffects;
  uint8_t overheatStrobeNF;
  uint8_t overheatSyncToFan;
  uint8_t overheatLightsOff;
  uint8_t ledCycLidCount;
  uint8_t ledCycLidHue;
  uint8_t ledCycLidSat;
  uint8_t ledCycLidLum;
  uint8_t ledCycLidCenter;
  uint8_t ledCycLidFade;
  uint8_t ledCycLidSimRing;
  uint8_t ledCycInnerPanel;
  uint8_t ledCycPanLum;
  uint8_t ledCycCakeCount;
  uint8_t ledCycCakeHue;
  uint8_t ledCycCakeSat;
  uint8_t ledCycCakeLum;
  uint8_t ledCycCakeGRB;
  uint8_t ledCycCavCount;
  uint8_t ledCycCavType;
  uint8_t ledVGCyclotron;
  uint8_t ledPowercellCount;
  uint8_t ledInvertPowercell;
  uint8_t ledPowercellHue;
  uint8_t ledPowercellSat;
  uint8_t ledPowercellLum;
  uint8_t ledVGPowercell;
  uint8_t gpstarAudioLed;
  uint8_t wifiState;
  uint8_t resetWifiPassword;
} packConfig;

// Output a compiler message if the final struct exceeds a specific size needed for SerialTransfer.
static_assert(sizeof(packConfig) < 85, "WARNING: PackConfig has grown too large (>84 bytes)");

// Preferences for the Neutrona Wand device.
struct __attribute__((packed)) WandPrefs {
  uint8_t isESP32;
  uint8_t ledWandCount;
  uint8_t ledWandHue;
  uint8_t ledWandSat;
  uint8_t rgbVentEnabled;
  uint8_t spectralModesEnabled;
  uint8_t overheatEnabled;
  uint8_t defaultFiringMode;
  uint8_t wandVibration;
  uint8_t barrelSwitchPolarity;
  uint8_t wandSoundsToPack;
  uint8_t quickVenting;
  uint8_t autoVentLight;
  uint8_t wandBeepLoop;
  uint8_t wandBootError;
  uint8_t defaultYearModeWand;
  uint8_t defaultYearModeCTS;
  uint8_t defaultWandVolume;
  uint8_t numBargraphSegments;
  uint8_t invertWandBargraph;
  uint8_t bargraphOverheatBlink;
  uint8_t bargraphIdleAnimation;
  uint8_t bargraphFireAnimation;
  uint8_t gpstarAudioLed;
  uint8_t wifiState;
  uint8_t resetWifiPassword;
} wandConfig;

// Output a compiler message if the final struct exceeds a specific size needed for SerialTransfer.
static_assert(sizeof(wandConfig) < 35, "WARNING: WandPrefs has grown too large (>34 bytes)");

// Preferences for smoke/overheat behavior.
struct __attribute__((packed)) SmokePrefs {
  // Pack
  uint8_t smokeEnabled;
  uint8_t overheatContinuous5;
  uint8_t overheatContinuous4;
  uint8_t overheatContinuous3;
  uint8_t overheatContinuous2;
  uint8_t overheatContinuous1;
  uint8_t overheatDuration5;
  uint8_t overheatDuration4;
  uint8_t overheatDuration3;
  uint8_t overheatDuration2;
  uint8_t overheatDuration1;
  // Wand
  uint8_t overheatLevel5;
  uint8_t overheatLevel4;
  uint8_t overheatLevel3;
  uint8_t overheatLevel2;
  uint8_t overheatLevel1;
  uint8_t overheatDelay5;
  uint8_t overheatDelay4;
  uint8_t overheatDelay3;
  uint8_t overheatDelay2;
  uint8_t overheatDelay1;
} smokeConfig;

// Output a compiler message if the final struct exceeds a specific size needed for SerialTransfer.
static_assert(sizeof(smokeConfig) < 35, "WARNING: SmokePrefs has grown too large (>34 bytes)");

// Data for synchronizing the Neutrona Wand.
struct __attribute__((packed)) WandSyncData {
  uint8_t systemMode;
  uint8_t ionArmSwitch;
  uint8_t cyclotronLidState;
  uint8_t systemYear;
  uint8_t packOn;
  uint8_t powerLevel;
  uint8_t streamMode;
  uint8_t vibrationEnabled;
  uint8_t effectsVolume;
  uint8_t masterMuted;
  uint8_t musicStatus;
  uint8_t repeatMusicTrack;
} wandSyncData;

// Output a compiler message if the final struct exceeds a specific size needed for SerialTransfer.
static_assert(sizeof(wandSyncData) < 35, "WARNING: WandSyncData has grown too large (>34 bytes)");

// Data for synchronizing the Attenuator.
struct __attribute__((packed)) AttenuatorSyncData {
  uint8_t systemMode;
  uint8_t ionArmSwitch;
  uint8_t cyclotronLidState;
  uint8_t systemYear;
  uint8_t packOn;
  uint8_t powerLevel;
  uint8_t streamMode;
  uint8_t streamFlags;
  uint8_t wandPresent;
  uint8_t barrelExtended;
  uint8_t wandFiring;
  uint8_t overheatingNow;
  uint8_t speedMultiplier;
  uint8_t spectralColour;
  uint8_t spectralSaturation;
  uint8_t masterMuted;
  uint8_t masterVolume;
  uint8_t effectsVolume;
  uint8_t musicVolume;
  uint8_t musicPlaying;
  uint8_t musicPaused;
  uint8_t trackLooped;
  uint16_t currentTrack;
  uint16_t musicCount;
  uint16_t packAudioVersion;
  uint16_t wandAudioVersion;
  uint16_t packVoltage;
} attenuatorSyncData;

// Output a compiler message if the final struct exceeds a specific size needed for SerialTransfer.
static_assert(sizeof(attenuatorSyncData) < 85, "WARNING: AttenuatorSyncData has grown too large (>84 bytes)");

/*
 * These enum definitions must be kept in sync across the devices they communicate with, using the same dataype and ordering.
 * Enum values are internally considered integer values and here they are being given a distinct underlying datatype of uint8_t.
 * It is therefore important that the total number of elements per enum must remain below 254 to not overflow that (byte) type.
 */

 // Specifically for device synchronization.
enum DEVICE_ID : uint8_t {
  A_COM_START,
  P_COM_START,
  W_COM_START,
  A_COM_END,
  P_COM_END,
  W_COM_END
};

// Specifically for actions called from the Proton Pack.
enum PACK_MESSAGE : uint8_t {
  P_NULL,
  P_HANDSHAKE,
  P_SYNC_START,
  P_SYNC_DATA,
  P_SYNC_END,
  P_ON,
  P_OFF,
  P_ALARM_ON,
  P_ALARM_OFF,
  P_VIBRATION_ENABLED,
  P_VIBRATION_DISABLED,
  P_YEAR_1984,
  P_YEAR_1989,
  P_YEAR_AFTERLIFE,
  P_YEAR_FROZEN_EMPIRE,
  P_VOLUME_SOUND_EFFECTS_INCREASE,
  P_VOLUME_SOUND_EFFECTS_DECREASE,
  P_PACK_VIBRATION_ENABLED,
  P_PACK_VIBRATION_DISABLED,
  P_PACK_VIBRATION_FIRING_ENABLED,
  P_PACK_VIBRATION_DEFAULT,
  P_PACK_MOTORIZED_CYCLOTRON_ENABLED,
  P_VIDEO_GAME_MODE_COLOURS_ENABLED,
  P_VIDEO_GAME_MODE_POWER_CELL_ENABLED,
  P_VIDEO_GAME_MODE_CYCLOTRON_ENABLED,
  P_VIDEO_GAME_MODE_COLOURS_DISABLED,
  P_MODE_FROZEN_EMPIRE,
  P_MODE_AFTERLIFE,
  P_MODE_1989,
  P_MODE_1984,
  P_SET_STREAM_MODE,
  P_SMOKE_DISABLED,
  P_SMOKE_ENABLED,
  P_CYCLOTRON_COUNTER_CLOCKWISE,
  P_CYCLOTRON_CLOCKWISE,
  P_CYCLOTRON_SINGLE_LED,
  P_CYCLOTRON_THREE_LED,
  P_MUSIC_STATUS,
  P_MUSIC_LOOP_STATUS,
  P_MASTER_AUDIO_STATUS,
  P_POWERCELL_DIMMING,
  P_CYCLOTRON_DIMMING,
  P_INNER_CYCLOTRON_DIMMING,
  P_CYCLOTRON_PANEL_DIMMING,
  P_DIMMING,
  P_PROTON_STREAM_IMPACT_ENABLED,
  P_PROTON_STREAM_IMPACT_DISABLED,
  P_RGB_INNER_CYCLOTRON_LEDS,
  P_GRB_INNER_CYCLOTRON_LEDS,
  P_CYCLOTRON_LEDS_40,
  P_CYCLOTRON_LEDS_36,
  P_CYCLOTRON_LEDS_20,
  P_CYCLOTRON_LEDS_12,
  P_POWERCELL_LEDS_15,
  P_POWERCELL_LEDS_13,
  P_INNER_CYCLOTRON_LEDS_23,
  P_INNER_CYCLOTRON_LEDS_24,
  P_INNER_CYCLOTRON_LEDS_26,
  P_INNER_CYCLOTRON_LEDS_35,
  P_INNER_CYCLOTRON_LEDS_36,
  P_INNER_CYCLOTRON_LEDS_12,
  P_CYCLOTRON_FADING_DISABLED,
  P_CYCLOTRON_FADING_ENABLED,
  P_CYCLOTRON_SIMULATE_RING_DISABLED,
  P_CYCLOTRON_SIMULATE_RING_ENABLED,
  P_WARNING_CANCELLED,
  P_OVERHEAT_STROBE_ENABLED,
  P_OVERHEAT_STROBE_DISABLED,
  P_OVERHEAT_LIGHTS_OFF_ENABLED,
  P_OVERHEAT_LIGHTS_OFF_DISABLED,
  P_OVERHEAT_SYNC_FAN_DISABLED,
  P_OVERHEAT_SYNC_FAN_ENABLED,
  P_YEAR_MODE_DEFAULT,
  P_MODE_SUPER_HERO,
  P_MODE_ORIGINAL,
  P_ION_ARM_SWITCH_ON,
  P_ION_ARM_SWITCH_OFF,
  P_CYCLOTRON_LID_ON,
  P_CYCLOTRON_LID_OFF,
  P_MANUAL_OVERHEAT,
  P_OVERHEATING_FINISHED,
  P_VENTING_FINISHED,
  P_DEMO_LIGHT_MODE_ENABLED,
  P_DEMO_LIGHT_MODE_DISABLED,
  P_CONTINUOUS_SMOKE_5_ENABLED,
  P_CONTINUOUS_SMOKE_4_ENABLED,
  P_CONTINUOUS_SMOKE_3_ENABLED,
  P_CONTINUOUS_SMOKE_2_ENABLED,
  P_CONTINUOUS_SMOKE_1_ENABLED,
  P_CONTINUOUS_SMOKE_5_DISABLED,
  P_CONTINUOUS_SMOKE_4_DISABLED,
  P_CONTINUOUS_SMOKE_3_DISABLED,
  P_CONTINUOUS_SMOKE_2_DISABLED,
  P_CONTINUOUS_SMOKE_1_DISABLED,
  P_SOUND_SUPER_HERO,
  P_SOUND_MODE_ORIGINAL,
  P_SEND_PREFERENCES_WAND,
  P_SEND_PREFERENCES_SMOKE,
  P_SAVE_PREFERENCES_WAND,
  P_SAVE_PREFERENCES_SMOKE,
  P_SAVE_EEPROM_WAND,
  P_INNER_CYCLOTRON_PANEL_DISABLED,
  P_INNER_CYCLOTRON_PANEL_STATIC,
  P_INNER_CYCLOTRON_PANEL_DYNAMIC,
  P_POWERCELL_NOT_INVERTED,
  P_POWERCELL_INVERTED,
  P_PACK_GPSTAR_AUDIO_LED_DISABLED,
  P_PACK_GPSTAR_AUDIO_LED_ENABLED,
  P_QUICK_BOOTUP_ENABLED,
  P_QUICK_BOOTUP_DISABLED,
  P_TURN_WAND_ON,
  P_POST_FINISH,
  P_NO_OP
};

// Output a compiler message if the final ENUM exceeds the datatype expected.
static_assert(P_NO_OP < 255, "WARNING: PACK_MESSAGE has grown too large for uint8_t!");

// Specifically for actions called from the Neutrona Wand.
enum WAND_MESSAGE : uint8_t {
  W_NULL,
  W_HANDSHAKE,
  W_SYNC_NOW,
  W_SYNCHRONIZED,
  W_ON,
  W_OFF,
  W_FIRING,
  W_FIRING_STOPPED,
  W_BUTTON_MASHING,
  W_STREAM_FLAGS,
  W_PROTON_MODE,
  W_SLIME_MODE,
  W_STASIS_MODE,
  W_MESON_MODE,
  W_SPECTRAL_MODE,
  W_HALLOWEEN_MODE,
  W_CHRISTMAS_MODE,
  W_SPECTRAL_CUSTOM_MODE,
  W_SETTINGS_MODE,
  W_OVERHEATING,
  W_VENTING,
  W_CYCLOTRON_NORMAL_SPEED,
  W_CYCLOTRON_INCREASE_SPEED,
  W_BEEP_START,
  W_POWER_LEVEL_1,
  W_POWER_LEVEL_2,
  W_POWER_LEVEL_3,
  W_POWER_LEVEL_4,
  W_POWER_LEVEL_5,
  W_FIRING_INTENSIFY_MIX,
  W_FIRING_INTENSIFY_STOPPED_MIX,
  W_FIRING_ALT_MIX,
  W_FIRING_ALT_STOPPED_MIX,
  W_FIRING_CROSSING_THE_STREAMS_1984,
  W_FIRING_CROSSING_THE_STREAMS_MIX_1984,
  W_FIRING_CROSSING_THE_STREAMS_STOPPED_MIX_1984,
  W_FIRING_CROSSING_THE_STREAMS_2021,
  W_FIRING_CROSSING_THE_STREAMS_MIX_2021,
  W_FIRING_CROSSING_THE_STREAMS_STOPPED_MIX_2021,
  W_TOGGLE_MUTE,
  W_YEAR_MODES_CYCLE,
  W_VIDEO_GAME_MODE_COLOUR_TOGGLE,
  W_CROSS_THE_STREAMS,
  W_CROSS_THE_STREAMS_MIX,
  W_VIBRATION_DISABLED,
  W_VIBRATION_ENABLED,
  W_VIBRATION_FIRING_ENABLED,
  W_VIBRATION_DEFAULT,
  W_VIBRATION_CYCLE_TOGGLE,
  W_VIBRATION_CYCLE_TOGGLE_EEPROM,
  W_SMOKE_TOGGLE,
  W_VIDEO_GAME_MODE,
  W_CYCLOTRON_DIRECTION_TOGGLE,
  W_CYCLOTRON_LED_TOGGLE,
  W_OVERHEATING_DISABLED,
  W_OVERHEATING_ENABLED,
  W_MUSIC_TRACK_LOOP_TOGGLE,
  W_VOLUME_SOUND_EFFECTS_INCREASE,
  W_VOLUME_SOUND_EFFECTS_DECREASE,
  W_VOLUME_MUSIC_INCREASE,
  W_VOLUME_MUSIC_DECREASE,
  W_MUSIC_TOGGLE,
  W_VOLUME_DECREASE,
  W_VOLUME_INCREASE,
  W_MENU_LEVEL_1,
  W_MENU_LEVEL_2,
  W_MENU_LEVEL_3,
  W_MENU_LEVEL_4,
  W_MENU_LEVEL_5,
  W_DIMMING_TOGGLE,
  W_DIMMING_INCREASE,
  W_DIMMING_DECREASE,
  W_PROTON_STREAM_IMPACT_TOGGLE,
  W_CLEAR_LED_EEPROM_SETTINGS,
  W_SAVE_LED_EEPROM_SETTINGS,
  W_TOGGLE_CYCLOTRON_LEDS,
  W_TOGGLE_POWERCELL_LEDS,
  W_TOGGLE_INNER_CYCLOTRON_LEDS,
  W_TOGGLE_RGB_INNER_CYCLOTRON_LEDS,
  W_EEPROM_LED_MENU,
  W_EEPROM_CONFIG_MENU,
  W_CLEAR_CONFIG_EEPROM_SETTINGS,
  W_SAVE_CONFIG_EEPROM_SETTINGS,
  W_EXTRA_WAND_SOUNDS_STOP,
  W_AFTERLIFE_GUN_RAMP_1,
  W_AFTERLIFE_GUN_RAMP_2,
  W_AFTERLIFE_RAMP_LOOP_2_STOP,
  W_AFTERLIFE_GUN_LOOP_1,
  W_AFTERLIFE_GUN_LOOP_2,
  W_AFTERLIFE_GUN_RAMP_DOWN_2,
  W_AFTERLIFE_GUN_RAMP_DOWN_1,
  W_AFTERLIFE_GUN_RAMP_DOWN_2_FADE_OUT,
  W_AFTERLIFE_GUN_RAMP_2_FADE_IN,
  W_VOICE_NEUTRONA_WAND_SOUNDS_ENABLED,
  W_VOICE_NEUTRONA_WAND_SOUNDS_DISABLED,
  W_CYCLOTRON_SIMULATE_RING_TOGGLE,
  W_SPECTRAL_MODES_ENABLED,
  W_SPECTRAL_MODES_DISABLED,
  W_SPECTRAL_INNER_CYCLOTRON_CUSTOM_DECREASE,
  W_SPECTRAL_CYCLOTRON_CUSTOM_DECREASE,
  W_SPECTRAL_POWERCELL_CUSTOM_DECREASE,
  W_SPECTRAL_POWERCELL_CUSTOM_INCREASE,
  W_SPECTRAL_CYCLOTRON_CUSTOM_INCREASE,
  W_SPECTRAL_INNER_CYCLOTRON_CUSTOM_INCREASE,
  W_SPECTRAL_LIGHTS_ON,
  W_SPECTRAL_LIGHTS_OFF,
  W_QUICK_VENT_ENABLED,
  W_QUICK_VENT_DISABLED,
  W_BOOTUP_ERRORS_ENABLED,
  W_BOOTUP_ERRORS_DISABLED,
  W_BARREL_LEDS_2,
  W_BARREL_LEDS_5,
  W_BARREL_LEDS_48,
  W_BARREL_LEDS_50,
  W_BARGRAPH_INVERTED,
  W_BARGRAPH_NOT_INVERTED,
  W_OVERHEAT_STROBE_TOGGLE,
  W_OVERHEAT_LIGHTS_OFF_TOGGLE,
  W_OVERHEAT_SYNC_TO_FAN_TOGGLE,
  W_YEAR_MODES_CYCLE_EEPROM,
  W_BARREL_EXTENDED,
  W_BARREL_RETRACTED,
  W_MUSIC_NEXT_TRACK,
  W_MUSIC_PREV_TRACK,
  W_OVERHEAT_INCREASE_LEVEL_1,
  W_OVERHEAT_INCREASE_LEVEL_2,
  W_OVERHEAT_INCREASE_LEVEL_3,
  W_OVERHEAT_INCREASE_LEVEL_4,
  W_OVERHEAT_INCREASE_LEVEL_5,
  W_OVERHEAT_DECREASE_LEVEL_1,
  W_OVERHEAT_DECREASE_LEVEL_2,
  W_OVERHEAT_DECREASE_LEVEL_3,
  W_OVERHEAT_DECREASE_LEVEL_4,
  W_OVERHEAT_DECREASE_LEVEL_5,
  W_BARGRAPH_OVERHEAT_BLINK_ENABLED,
  W_BARGRAPH_OVERHEAT_BLINK_DISABLED,
  W_MODE_BEEP_LOOP_ENABLED,
  W_MODE_BEEP_LOOP_DISABLED,
  W_DEFAULT_BARGRAPH,
  W_MODE_ORIGINAL_BARGRAPH,
  W_SUPER_HERO_BARGRAPH,
  W_SUPER_HERO_FIRING_ANIMATIONS_BARGRAPH,
  W_MODE_ORIGINAL_FIRING_ANIMATIONS_BARGRAPH,
  W_DEFAULT_FIRING_ANIMATIONS_BARGRAPH,
  W_NEUTRONA_WAND_1984_MODE,
  W_NEUTRONA_WAND_1989_MODE,
  W_NEUTRONA_WAND_AFTERLIFE_MODE,
  W_NEUTRONA_WAND_FROZEN_EMPIRE_MODE,
  W_NEUTRONA_WAND_DEFAULT_MODE,
  W_DEMO_LIGHT_MODE_TOGGLE,
  W_CTS_DEFAULT,
  W_CTS_1984,
  W_CTS_AFTERLIFE,
  W_MODE_TOGGLE,
  W_OVERHEAT_LEVEL_5_ENABLED,
  W_OVERHEAT_LEVEL_4_ENABLED,
  W_OVERHEAT_LEVEL_3_ENABLED,
  W_OVERHEAT_LEVEL_2_ENABLED,
  W_OVERHEAT_LEVEL_1_ENABLED,
  W_OVERHEAT_LEVEL_5_DISABLED,
  W_OVERHEAT_LEVEL_4_DISABLED,
  W_OVERHEAT_LEVEL_3_DISABLED,
  W_OVERHEAT_LEVEL_2_DISABLED,
  W_OVERHEAT_LEVEL_1_DISABLED,
  W_CONTINUOUS_SMOKE_TOGGLE_5,
  W_CONTINUOUS_SMOKE_TOGGLE_4,
  W_CONTINUOUS_SMOKE_TOGGLE_3,
  W_CONTINUOUS_SMOKE_TOGGLE_2,
  W_CONTINUOUS_SMOKE_TOGGLE_1,
  W_VOLUME_DECREASE_EEPROM,
  W_VOLUME_INCREASE_EEPROM,
  W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_5,
  W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_4,
  W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_3,
  W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_2,
  W_SOUND_OVERHEAT_SMOKE_DURATION_LEVEL_1,
  W_SOUND_OVERHEAT_START_TIMER_LEVEL_5,
  W_SOUND_OVERHEAT_START_TIMER_LEVEL_4,
  W_SOUND_OVERHEAT_START_TIMER_LEVEL_3,
  W_SOUND_OVERHEAT_START_TIMER_LEVEL_2,
  W_SOUND_OVERHEAT_START_TIMER_LEVEL_1,
  W_SOUND_DEFAULT_SYSTEM_VOLUME_ADJUSTMENT,
  W_SEND_PREFERENCES_WAND,
  W_SEND_PREFERENCES_SMOKE,
  W_GB1_WAND_BARREL_EXTEND,
  W_AFTERLIFE_WAND_BARREL_EXTEND,
  W_WAND_BARREL_RETRACT,
  W_WAND_BOOTUP_SOUND,
  W_WAND_BOOTUP_SHORT_SOUND,
  W_WAND_SHUTDOWN_SOUND,
  W_WAND_MASH_ERROR_SOUND,
  W_WAND_BEEP_SOUNDS,
  W_WAND_BEEP_BARGRAPH,
  W_MODE_ORIGINAL_HEATUP_STOP,
  W_MODE_ORIGINAL_HEATUP,
  W_MODE_ORIGINAL_HEATDOWN_STOP,
  W_MODE_ORIGINAL_HEATDOWN,
  W_BEEPS_ALT,
  W_WAND_BEEP_STOP,
  W_WAND_BEEP_STOP_LOOP,
  W_WAND_BEEP_START,
  W_WAND_BEEP,
  W_MASH_ERROR_LOOP,
  W_MASH_ERROR_RESTART,
  W_BOSON_DART_SOUND,
  W_SHOCK_BLAST_SOUND,
  W_SLIME_TETHER_SOUND,
  W_MESON_COLLIDER_SOUND,
  W_MESON_FIRE_PULSE,
  W_TOGGLE_INNER_CYCLOTRON_PANEL,
  W_WAND_BOOTUP_1989,
  W_TOGGLE_POWERCELL_DIRECTION,
  W_TOGGLE_CYCLOTRON_FADING,
  W_TOGGLE_PACK_WIFI,
  W_RESET_WIFI_PASSWORD,
  W_WAND_WIFI_RESET,
  W_WAND_WIFI_DISABLED,
  W_WAND_WIFI_ENABLED,
  W_BARREL_ERROR_SOUND,
  W_BARREL_SWITCH_DEFAULT,
  W_BARREL_SWITCH_INVERTED,
  W_BARREL_SWITCH_DISABLED,
  W_BARGRAPH_28_SEGMENTS,
  W_BARGRAPH_30_SEGMENTS,
  W_RGB_VENT_DISABLED,
  W_RGB_VENT_ENABLED,
  W_AUTO_VENT_INTENSITY_DISABLED,
  W_AUTO_VENT_INTENSITY_ENABLED,
  W_GPSTAR_AUDIO_LED_TOGGLE,
  W_WAND_GPSTAR_AUDIO_LED_DISABLED,
  W_WAND_GPSTAR_AUDIO_LED_ENABLED,
  W_WAND_AUDIO_VERSION,
  W_QUICK_BOOTUP_TOGGLE,
  W_IMPACT_SOUND,
  W_COM_SOUND_NUMBER,
  W_NO_OP
};

// Output a compiler message if the final ENUM exceeds the datatype expected.
static_assert(W_NO_OP < 255, "WARNING: WAND_MESSAGE has grown too large for uint8_t!");

// Primarily for Attenuator communications but may become a more unified API list.
enum API_MESSAGE : uint8_t {
  A_NULL,
  A_HANDSHAKE,
  A_SYNC_START,
  A_SYNC_DATA,
  A_SYNC_END,
  A_WAND_ON,
  A_WAND_OFF,
  A_FIRING,
  A_FIRING_STOPPED,
  A_SYSTEM_LOCKOUT,
  A_CANCEL_LOCKOUT,
  A_STREAM_FLAGS,
  A_PROTON_MODE,
  A_STASIS_MODE,
  A_SLIME_MODE,
  A_MESON_MODE,
  A_SPECTRAL_MODE,
  A_HALLOWEEN_MODE,
  A_CHRISTMAS_MODE,
  A_SPECTRAL_CUSTOM_MODE,
  A_SETTINGS_MODE,
  A_VENTING,
  A_VENTING_FINISHED,
  A_OVERHEATING,
  A_OVERHEATING_FINISHED,
  A_WARNING_CANCELLED,
  A_CYCLOTRON_LID_ON,
  A_CYCLOTRON_LID_OFF,
  A_CYCLOTRON_NORMAL_SPEED,
  A_CYCLOTRON_INCREASE_SPEED,
  A_CYCLOTRON_DIRECTION_TOGGLE,
  A_POWER_LEVEL_1,
  A_POWER_LEVEL_2,
  A_POWER_LEVEL_3,
  A_POWER_LEVEL_4,
  A_POWER_LEVEL_5,
  A_MUSIC_TRACK_LOOP_TOGGLE,
  A_VOLUME_SOUND_EFFECTS_INCREASE,
  A_VOLUME_SOUND_EFFECTS_DECREASE,
  A_VOLUME_MUSIC_INCREASE,
  A_VOLUME_MUSIC_DECREASE,
  A_MUSIC_NEXT_TRACK,
  A_MUSIC_PREV_TRACK,
  A_VOLUME_DECREASE,
  A_VOLUME_INCREASE,
  A_VOLUME_SYNC,
  A_SAVE_EEPROM_SETTINGS_PACK,
  A_SAVE_EEPROM_SETTINGS_WAND,
  A_YEAR_FROZEN_EMPIRE,
  A_YEAR_AFTERLIFE,
  A_YEAR_1989,
  A_YEAR_1984,
  A_ALARM_ON,
  A_ALARM_OFF,
  A_PACK_ON,
  A_PACK_OFF,
  A_TURN_PACK_ON,
  A_TURN_PACK_OFF,
  A_SPECTRAL_COLOUR_DATA,
  A_MUSIC_START_STOP,
  A_TOGGLE_MUTE,
  A_TOGGLE_SMOKE,
  A_TOGGLE_VIBRATION,
  A_BARREL_EXTENDED,
  A_BARREL_RETRACTED,
  A_MODE_SUPER_HERO,
  A_MODE_ORIGINAL,
  A_ION_ARM_SWITCH_ON,
  A_ION_ARM_SWITCH_OFF,
  A_MANUAL_OVERHEAT,
  A_MUSIC_TRACK_COUNT_SYNC,
  A_MUSIC_PAUSE_RESUME,
  A_MUSIC_IS_PLAYING,
  A_MUSIC_IS_NOT_PLAYING,
  A_MUSIC_IS_PAUSED,
  A_MUSIC_IS_NOT_PAUSED,
  A_MUSIC_PLAY_TRACK,
  A_BATTERY_VOLTAGE_PACK,
  A_TEMPERATURE_PACK,
  A_WAND_POWER_AMPS,
  A_WAND_CONNECTED,
  A_WAND_DISCONNECTED,
  A_WAND_AUDIO_VERSION,
  A_RESET_WIFI_PASSWORD,
  A_REQUEST_PREFERENCES_PACK,
  A_REQUEST_PREFERENCES_WAND,
  A_REQUEST_PREFERENCES_SMOKE,
  A_SEND_PREFERENCES_PACK,
  A_SEND_PREFERENCES_WAND,
  A_SEND_PREFERENCES_SMOKE,
  A_SAVE_PREFERENCES_PACK,
  A_SAVE_PREFERENCES_WAND,
  A_SAVE_PREFERENCES_SMOKE,
  A_NO_OP
};

// Output a compiler message if the final ENUM exceeds the datatype expected.
static_assert(A_NO_OP < 255, "WARNING: API_MESSAGE has grown too large for uint8_t!");
