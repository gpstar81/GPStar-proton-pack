/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2026 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                         & Dustin Grau <dustin.grau@gmail.com>
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

#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

// Declare the external binary data markers for embedded files.
// common.js
extern const uint8_t _binary_assets_common_js_gz_start[];
extern const uint8_t _binary_assets_common_js_gz_end[];
// equipment.svg
extern const uint8_t _binary_assets_equipment_svg_gz_start[];
extern const uint8_t _binary_assets_equipment_svg_gz_end[];
// favicon.ico
extern const uint8_t _binary_assets_favicon_ico_gz_start[];
extern const uint8_t _binary_assets_favicon_ico_gz_end[];
// favicon.svg
extern const uint8_t _binary_assets_favicon_svg_gz_start[];
extern const uint8_t _binary_assets_favicon_svg_gz_end[];
// style.css
extern const uint8_t _binary_assets_style_css_gz_start[];
extern const uint8_t _binary_assets_style_css_gz_end[];
// index.html
extern const uint8_t _binary_assets_index_html_gz_start[];
extern const uint8_t _binary_assets_index_html_gz_end[];
// index.js
extern const uint8_t _binary_assets_index_js_gz_start[];
extern const uint8_t _binary_assets_index_js_gz_end[];
// device.html
extern const uint8_t _binary_assets_device_html_gz_start[];
extern const uint8_t _binary_assets_device_html_gz_end[];
// network.html
extern const uint8_t _binary_assets_network_html_gz_start[];
extern const uint8_t _binary_assets_network_html_gz_end[];
// password.html
extern const uint8_t _binary_assets_password_html_gz_start[];
extern const uint8_t _binary_assets_password_html_gz_end[];
// pack.html
extern const uint8_t _binary_assets_pack_html_gz_start[];
extern const uint8_t _binary_assets_pack_html_gz_end[];
// wand.html
extern const uint8_t _binary_assets_wand_html_gz_start[];
extern const uint8_t _binary_assets_wand_html_gz_end[];
// smoke.html
extern const uint8_t _binary_assets_smoke_html_gz_start[];
extern const uint8_t _binary_assets_smoke_html_gz_end[];
// swaggerui.html
extern const uint8_t _binary_assets_swaggerui_html_gz_start[];
extern const uint8_t _binary_assets_swaggerui_html_gz_end[];

// Define standard ports and URI endpoints.
const uint16_t WS_PORT = 80; // Web Server (+WebSocket) port
const char WS_URI[] = "/ws"; // WebSocket endpoint URI
bool b_httpd_started = false; // Denotes the web server has been started.

// Define an asynchronous web server at TCP port 80.
AsyncWebServer httpServer(WS_PORT);

// Define a websocket endpoint for the async web server.
AsyncWebSocket ws(WS_URI);

// Create a server-side event source on /events.
AsyncEventSource events("/events");

// Track the number of connected WebSocket clients.
uint8_t i_ws_client_count = 0;

// Track time to refresh progress for OTA updates.
unsigned long i_progress_millis = 0;

// Create timer for WebSocket cleanup.
millisDelay ms_cleanup;
const uint16_t i_websocketCleanup = 5000;

// Forward function declarations.
bool canChangeStreamMode(); // From System.h
void getSpecialPreferences(); // From PreferencesESP.h
void sendDebug(const String& message); // From System.h
void registerWebRoutes(); // From Webrouting.h

/*
 * Text Helper Functions - Converts ENUM values to consistent, user-friendly text
 */

// Rounds a float to 2 decimal places.
float roundFloat(float value) {
  return roundf(value * 100.0f) / 100.0f;
}

String getCyclotronState() {
  switch(i_cyclotron_multiplier) {
    case 1:
       // Indicates an "idle" state, subject to the overheat status.
      return (b_overheating ? "Recovery" : "Normal");
    break;
    case 2:
      return "Active"; // After throwing a stream for an extended period.
    break;
    case 3:
      return "Warning"; // Considered to be in a "pre-overheat" state.
    break;
    default:
      return "Critical"; // For anything above a 3x speed increase.
    break;
  }
}

/**
 * JSON Body Helpers - Creates stringified JSON representations of device configurations
 */

String getDeviceConfig() {
  // Prepare a JSON object with information we have gleaned from the system.
  String equipSettings;
  JsonDocument jsonBody;

  // Provide current values for the device.
  jsonBody["displayType"] = DISPLAY_TYPE;
  jsonBody["useAnimation"] = b_enable_ui_animations;
  if(s_track_listing != "" && s_track_listing != "null") {
    jsonBody["songList"] = s_track_listing;
  }
  else {
    jsonBody["songList"] = "";
  }
  jsonBody["buildDate"] = build_date;
  jsonBody["audioVersion"] = i_audio_version;
  jsonBody["audioCorrupt"] = b_microsd_corrupt;
  jsonBody["audioOutdated"] = b_microsd_outdated;

  // Build list of available stream modes based on device configuration.
  JsonArray streamModes = jsonBody["streamModes"].to<JsonArray>();
  for(uint8_t i = 0; i <= LAST_SWITCHABLE_STREAM_MODE; i++) {
    STREAM_MODES mode = static_cast<STREAM_MODES>(i);
    if(gpstarPack.supportsStreamMode(mode)) {
      JsonObject streamMode = streamModes.add<JsonObject>();
      streamMode["value"] = gpstarPack.getStreamModeValue(mode);
      streamMode["label"] = gpstarPack.getStreamModeName(mode);
    }
  }

  jsonBody["wifiName"] = wirelessMgr->getLocalNetworkName();
  jsonBody["wifiNameExt"] = wirelessMgr->getExtWifiNetworkName();

  // Refresh external WiFi info when/if connected and get the values.
  if(wirelessMgr->getExtWifiNetworkInfo()) {
    jsonBody["extAddr"] = wirelessMgr->getExtWifiAddress().toString();
    jsonBody["extMask"] = wirelessMgr->getExtWifiSubnet().toString();
  } else {
    jsonBody["extAddr"] = "";
    jsonBody["extMask"] = "";
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getPackConfig() {
  // Prepare a JSON object with information we have gleaned from the system.
  String equipSettings;
  JsonDocument jsonBody;

  try {
    // Provide a flag to indicate prefs are directly available.
    jsonBody["prefsAvailable"] = true; // Always true for the immediate device.

    // Use the device-reported values so we can fine-tune the UI options.
    jsonBody["esp32Pack"] = packConfig.isESP32;
    jsonBody["isWiFiEnabled"] = packConfig.isWiFiEnabled;
    jsonBody["resetWifiPassword"] = false;
    jsonBody["gpstarAudio"] = (i_audio_version > 1);

    // Return current powered state for pack and wand.
    jsonBody["packPowered"] = (PACK_STATE == MODE_ON || b_pack_shutting_down);
    jsonBody["wandPowered"] = b_wand_on;

    // Proton Pack Runtime Options
    jsonBody["defaultSystemModePack"] = packConfig.defaultSystemModePack; // [1=SH,2=MO]
    jsonBody["defaultYearThemePack"] = packConfig.defaultYearThemePack; // [1=TOGGLE,2=1984,3=1989,4=2021,5=2024]
    jsonBody["currentYearThemePack"] = packConfig.currentYearThemePack; // [2=1984,3=1989,4=2021,5=2024]
    jsonBody["defaultPackVolume"] = packConfig.defaultPackVolume; // 5-100
    jsonBody["fadeoutIdleSounds"] = packConfig.fadeoutIdleSounds; // true|false
    jsonBody["packVibration"] = packConfig.packVibration; // [1=ALWAYS,2=FIRING,3=NEVER,4=TOGGLE,5=MOTORIZED_CYCLOTRON]
    jsonBody["protonStreamEffects"] = packConfig.protonStreamEffects; // true|false
    jsonBody["brassStartupLoop"] = packConfig.brassStartupLoop; // true|false
    jsonBody["ribbonCableAlarm"] = packConfig.ribbonCableAlarm; // true|false
    jsonBody["wandQuickBootup"] = packConfig.wandQuickBootup; // true|false
    jsonBody["overheatStrobeNF"] = packConfig.overheatStrobeNF; // true|false
    jsonBody["overheatLightsOff"] = packConfig.overheatLightsOff; // true|false
    jsonBody["overheatSyncToFan"] = packConfig.overheatSyncToFan;// true|false
    jsonBody["demoLightMode"] = packConfig.demoLightMode; // true|false

    // Proton Pack LED Options
    jsonBody["ledCycLidCount"] = packConfig.ledCycLidCount; // [12,20,36,40]
    jsonBody["ledCycLidHue"] = packConfig.ledCycLidHue; // Spectral custom colour/hue 2-254
    jsonBody["ledCycLidSat"] = packConfig.ledCycLidSat; // Spectral custom saturation 2-254
    jsonBody["ledCycLidLum"] = packConfig.ledCycLidLum; // Brightness 20-100
    jsonBody["cyclotronDirection"] = packConfig.cyclotronDirection; // [false=CCW,true=CW]
    jsonBody["ledCycLidCenter"] = packConfig.ledCycLidCenter; // [0=3,1=1]
    jsonBody["ledCycLidFade"] = packConfig.ledCycLidFade; // true|false
    jsonBody["ledVGCyclotron"] = packConfig.ledVGCyclotron; // true|false
    jsonBody["ledCycLidSimRing"] = packConfig.ledCycLidSimRing; // true|false
    jsonBody["disableLidDetection"] = packConfig.disableLidDetection; // true|false
    jsonBody["ledCycInnerPanel"] = packConfig.ledCycInnerPanel; // [1=Individual,2=RGB-Static,3=RGB-Dynamic]
    jsonBody["ledCycPanLum"] = packConfig.ledCycPanLum; // Brightness 0-100
    jsonBody["ledCycCakeCount"] = packConfig.ledCycCakeCount; // [12,23,24,26,35,36]
    jsonBody["ledCycCakeHue"] = packConfig.ledCycCakeHue; // Spectral custom colour/hue 2-254
    jsonBody["ledCycCakeSat"] = packConfig.ledCycCakeSat; // Spectral custom saturation 2-254
    jsonBody["ledCycCakeLum"] = packConfig.ledCycCakeLum; // Brightness 20-100
    jsonBody["ledCycCakeGRB"] = packConfig.ledCycCakeGRB; // Use GRB for cake LEDs true|false
    jsonBody["ledCycCavCount"] = packConfig.ledCycCavCount; // Cyclotron cavity LEDs (0-20)
    jsonBody["ledCycCavType"] = packConfig.ledCycCavType; // Cyclotron cavity LED Type
    jsonBody["ledPowercellCount"] = packConfig.ledPowercellCount; //[13,15]
    jsonBody["ledInvertPowercell"] = packConfig.ledInvertPowercell; // true|false
    jsonBody["ledPowercellHue"] = packConfig.ledPowercellHue; // Spectral custom colour/hue 2-254
    jsonBody["ledPowercellSat"] = packConfig.ledPowercellSat; // Spectral custom saturation 2-254
    jsonBody["ledPowercellLum"] = packConfig.ledPowercellLum; // Brightness 20-100
    jsonBody["ledVGPowercell"] = packConfig.ledVGPowercell; // true|false
    jsonBody["gpstarAudioLed"] = packConfig.gpstarAudioLed; // true|false
  }
  catch (...) {
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getWandConfig() {
  // Prepare a JSON object with information we have gleaned from the system.
  String equipSettings;
  JsonDocument jsonBody;

  try {
    // Provide a flag to indicate prefs were received via serial coms.
    jsonBody["prefsAvailable"] = b_received_prefs_wand;

    // Use the device-reported values so we can fine-tune the UI options.
    jsonBody["esp32Wand"] = wandConfig.isESP32;
    jsonBody["isWiFiEnabled"] = wandConfig.isWiFiEnabled;
    jsonBody["resetWifiPassword"] = false;
    jsonBody["gpstarAudio"] = (i_wand_audio_version > 1);

    // Return current powered state for pack and wand.
    jsonBody["packPowered"] = (PACK_STATE == MODE_ON || b_pack_shutting_down);
    jsonBody["wandPowered"] = b_wand_on;
    jsonBody["wandConnected"] = b_wand_connected;

    // Neutrona Wand LED Options
    jsonBody["ledWandCount"] = wandConfig.ledWandCount; // [0=5 (Stock), 1=48 (Frutto), 2=50 (GPStar), 3=2 (Tip)]
    jsonBody["ledWandHue"] = wandConfig.ledWandHue; // Spectral custom colour/hue 2-254
    jsonBody["ledWandSat"] = wandConfig.ledWandSat; // Spectral custom saturation 2-254

    // Unpack stream mode options into individual checkbox values
    jsonBody["stasisStream"] = (wandConfig.streamFlags & FLAG_STASIS) != 0;
    jsonBody["slimeStream"] = (wandConfig.streamFlags & FLAG_SLIME) != 0;
    jsonBody["mesonStream"] = (wandConfig.streamFlags & FLAG_MESON) != 0;
    jsonBody["spectralStream"] = (wandConfig.streamFlags & FLAG_SPECTRAL) != 0;
    jsonBody["spectralCustomStream"] = (wandConfig.streamFlags & FLAG_SPECTRAL_CUSTOM) != 0;
    jsonBody["halloweenStream"] = (wandConfig.streamFlags & FLAG_HOLIDAY_HALLOWEEN) != 0;
    jsonBody["christmasStream"] = (wandConfig.streamFlags & FLAG_HOLIDAY_CHRISTMAS) != 0;

    // Neutrona Wand Runtime Options
    jsonBody["systemMode"] = gpstarPack.getModeName(); // "Super Hero" or "Original"
    jsonBody["systemTheme"] = gpstarPack.getThemeName(); // 1984, 1989 (GB2), 2021 (AL), 2024 (FE)
    jsonBody["currentStreamMode"] = gpstarPack.getStreamModeName(); // String for current firing mode
    jsonBody["overheatEnabled"] = wandConfig.overheatEnabled; // true|false
    jsonBody["defaultStreamMode"] = wandConfig.defaultStreamMode; // [0=PROTON,STASIS=1,SLIME=2,MESON=3,SPECTRAL=4,HALLOWEEN=5,CHRISTMAS=6,HOLIDAYCUSTOM=7,CUSTOM=8]
    jsonBody["defaultFiringMode"] = wandConfig.defaultFiringMode; // [0=VG,1=CTS,3=CTS_MIX]
    jsonBody["wandVibration"] = wandConfig.wandVibration; // [1=ALWAYS,2=FIRING,3=NEVER,4=TOGGLE]
    jsonBody["barrelSwitchPolarity"] = wandConfig.barrelSwitchPolarity; // [1=DEFAULT,2=INVERTED,3=DISABLED]
    jsonBody["wandSoundsToPack"] = wandConfig.wandSoundsToPack; // true|false
    jsonBody["quickVenting"] = wandConfig.quickVenting; // true|false (Super-Hero Mode Only)
    jsonBody["rgbVentEnabled"] = wandConfig.rgbVentEnabled; // true|false
    jsonBody["rgbVentColours"] = wandConfig.rgbVentColours; // true|false
    jsonBody["autoVentLight"] = wandConfig.autoVentLight; // true|false
    jsonBody["gpstarAudioLed"] = wandConfig.gpstarAudioLed; // true|false
    jsonBody["wandBeepLoop"] = wandConfig.wandBeepLoop; // true|false (Afterlife/Frozen Empire Only)
    jsonBody["wandBootError"] = wandConfig.wandBootError; // true|false (Super-Hero Mode Only)
    jsonBody["defaultYearModeWand"] = wandConfig.defaultYearModeWand; // [1=TOGGLE,2=1984,3=1989,4=2021,5=2024]
    jsonBody["defaultYearModeCTS"] = wandConfig.defaultYearModeCTS; // [1=TOGGLE,2=1984,4=2021]
    jsonBody["defaultWandVolume"] = wandConfig.defaultWandVolume; // 5-100
    jsonBody["numBargraphSegments"] = wandConfig.numBargraphSegments; // [28=28-segment,30=30-segment]
    jsonBody["invertWandBargraph"] = wandConfig.invertWandBargraph; // true|false
    jsonBody["bargraphOverheatBlink"] = wandConfig.bargraphOverheatBlink; // true|false
    jsonBody["bargraphIdleAnimation"] = wandConfig.bargraphIdleAnimation; // [1=SYSTEM,2=SH,3=MO]
    jsonBody["bargraphFireAnimation"] = wandConfig.bargraphFireAnimation; // [1=SYSTEM,2=SH,3=MO]
  }
  catch (...) {
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getSmokeConfig() {
  // Prepare a JSON object with information we have gleaned from the system.
  String equipSettings;
  JsonDocument jsonBody;

  try {
    // Provide a flag to indicate prefs were received via serial coms.
    jsonBody["prefsAvailable"] = true; // Always true for the immediate device.

    // Return current powered state for pack and wand.
    jsonBody["packPowered"] = (PACK_STATE == MODE_ON || b_pack_shutting_down);
    jsonBody["wandPowered"] = b_wand_on;
    jsonBody["wandConnected"] = b_wand_connected;

    // Proton Pack
    jsonBody["smokeEnabled"] = smokeConfig.smokeEnabled; // true|false

    // Power Level 5
    jsonBody["overheatDuration5"] = smokeConfig.overheatDuration5; // 2-60 Seconds
    jsonBody["overheatContinuous5"] = smokeConfig.overheatContinuous5; // true|false
    // Power Level 4
    jsonBody["overheatDuration4"] = smokeConfig.overheatDuration4; // 2-60 Seconds
    jsonBody["overheatContinuous4"] = smokeConfig.overheatContinuous4; // true|false
    // Power Level 3
    jsonBody["overheatDuration3"] = smokeConfig.overheatDuration3; // 2-60 Seconds
    jsonBody["overheatContinuous3"] = smokeConfig.overheatContinuous3; // true|false
    // Power Level 2
    jsonBody["overheatDuration2"] = smokeConfig.overheatDuration2; // 2-60 Seconds
    jsonBody["overheatContinuous2"] = smokeConfig.overheatContinuous2; // true|false
    // Power Level 1
    jsonBody["overheatDuration1"] = smokeConfig.overheatDuration1; // 2-60 Seconds
    jsonBody["overheatContinuous1"] = smokeConfig.overheatContinuous1; // true|false

    // Neutrona Wand

    // Power Level 5
    jsonBody["overheatLevel5"] = smokeConfig.overheatLevel5; // true|false
    jsonBody["overheatDelay5"] = smokeConfig.overheatDelay5; // 2-60 Seconds
    // Power Level 4
    jsonBody["overheatLevel4"] = smokeConfig.overheatLevel4; // true|false
    jsonBody["overheatDelay4"] = smokeConfig.overheatDelay4; // 2-60 Seconds
    // Power Level 3
    jsonBody["overheatLevel3"] = smokeConfig.overheatLevel3; // true|false
    jsonBody["overheatDelay3"] = smokeConfig.overheatDelay3; // 2-60 Seconds
    // Power Level 2
    jsonBody["overheatLevel2"] = smokeConfig.overheatLevel2; // true|false
    jsonBody["overheatDelay2"] = smokeConfig.overheatDelay2; // 2-60 Seconds
    // Power Level 1
    jsonBody["overheatLevel1"] = smokeConfig.overheatLevel1; // true|false
    jsonBody["overheatDelay1"] = smokeConfig.overheatDelay1; // 2-60 Seconds
  }
  catch (...) {
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getEquipmentStatus() {
  // Prepare a JSON object with information we have gleaned from the system.
  String equipStatus;
  JsonDocument jsonBody;

  uint16_t i_music_track_min = 0;
  uint16_t i_music_track_max = 0;
  bool b_wand_firing_cts = STATUS_CTS != CTS_NOT_FIRING; // Indicates firing CTS in either 1984 or 2021 mode.

  if(i_music_track_count > 0) {
    i_music_track_min = i_music_track_start; // First music track possible (eg. 500)
    i_music_track_max = i_music_track_start + i_music_track_count - 1; // 500 + N - 1 to be inclusive of the offset value.
  }

  try {
    jsonBody["mode"] = gpstarPack.getModeName();
    jsonBody["modeID"] = gpstarPack.getSystemMode();
    jsonBody["theme"] = gpstarPack.getThemeName();
    jsonBody["themeID"] = gpstarPack.getSystemTheme();
    jsonBody["vgMode"] = (wandConfig.defaultFiringMode == FLAG_VG_MODE || !b_wand_connected);
    jsonBody["smoke"] = b_smoke_enabled;
    jsonBody["vibration"] = b_vibration_switch_on;
    jsonBody["direction"] = b_clockwise;
    jsonBody["switch"] = gpstarPack.getIonArmSwitchState();
    jsonBody["pack"] = (PACK_STATE == MODE_ON ? "Powered" : "Idle");
    jsonBody["ramping"] = b_pack_shutting_down;
    jsonBody["power"] = gpstarPack.getPowerLevelName();
    jsonBody["safety"] = gpstarPack.getBarrelStateName();
    jsonBody["wand"] = (b_wand_connected ? "Connected" : "Not Connected");
    jsonBody["wandPower"] = (b_wand_on ? "Powered" : "Idle");
    jsonBody["wandMode"] = gpstarPack.getStreamModeName();
    jsonBody["firing"] = (b_wand_firing ? "Firing" : "Idle");
    jsonBody["crossedStreams"] = (b_wand_firing && b_wand_firing_cts);
    jsonBody["lockout"] = b_wand_mash_lockout;
    jsonBody["cable"] = (ribbonCableAttached() ? "Connected" : "Disconnected");
    jsonBody["cyclotron"] = getCyclotronState();
    jsonBody["cyclotronLid"] = b_cyclotron_lid_on;
    jsonBody["temperature"] = (b_overheating ? "Venting" : "Normal");
    jsonBody["musicPlaying"] = b_playing_music;
    jsonBody["musicPaused"] = b_music_paused;
    jsonBody["musicLooping"] = b_repeat_track;
    jsonBody["musicShuffled"] = b_shuffle_tracks;
    jsonBody["musicCurrent"] = i_current_music_track;
    jsonBody["musicStart"] = i_music_track_min;
    jsonBody["musicEnd"] = i_music_track_max;
    jsonBody["volMuted"] = (i_volume_master == i_volume_abs_min);
    jsonBody["volMaster"] = i_volume_master_percentage;
    jsonBody["volEffects"] = i_volume_effects_percentage;
    jsonBody["volMusic"] = i_volume_music_percentage;
    jsonBody["battVoltage"] = roundFloat(f_batt_volts);
    jsonBody["packTempC"] = roundFloat(f_temperature_c);
    jsonBody["packTempF"] = roundFloat(f_temperature_f);
    jsonBody["wandAmps"] = roundFloat(f_wand_amps);
    jsonBody["apClients"] = i_ap_client_count;
    jsonBody["wsClients"] = i_ws_client_count;
    jsonBody["canChangeStream"] = canChangeStreamMode();
  }
  catch (...) {
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipStatus);
  return equipStatus;
}

String getWifiSettings() {
  // Prepare a JSON object with information stored in preferences (or a blank default).
  String wifiSettings;
  JsonDocument jsonBody;

  // Modern ArduinoJson: assign nested object for "active"
  JsonObject active = jsonBody["active"].to<JsonObject>();
  wirelessMgr->getExtWifiNetworkAsJson(active);

  // Modern ArduinoJson: assign nested array for "saved"
  JsonArray saved = jsonBody["saved"].to<JsonArray>();
  String savedNetworks = wirelessMgr->getPreferredNetworks();

  // Parse the saved networks JSON string into a temporary document
  JsonDocument tmpDoc;
  DeserializationError err = deserializeJson(tmpDoc, savedNetworks);
  if(!err && tmpDoc.is<JsonArray>()) {
    for(JsonVariant v : tmpDoc.as<JsonArray>()) {
      saved.add(v);
    }
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, wifiSettings);
  return wifiSettings;
}

/*
 * Web Handler Functions - Performs actions or returns data for web UI
 */

// Send notification to all websocket clients.
void notifyWSClients() {
  if(b_httpd_started) {
    // Send latest status to all connected clients.
    ws.textAll(getEquipmentStatus());
  }
}

void onWebSocketEventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch(type) {
    case WS_EVT_CONNECT:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        debugf("WebSocket[%s][%lu] Connect\n", server->url(), client->id());
      #endif
      i_ws_client_count++;
      notifyWSClients();
    break;

    case WS_EVT_DISCONNECT:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        debugf("WebSocket[%s][C:%lu] Disconnect\n", server->url(), client->id());
      #endif
      if(i_ws_client_count > 0) {
        i_ws_client_count--;
        notifyWSClients();
      }
    break;

    case WS_EVT_ERROR:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        debugf("WebSocket[%s][C:%lu] Error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
      #endif
    break;

    case WS_EVT_PONG:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        debugf("WebSocket[%s][C:%lu] Pong[L:%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
      #endif
    break;

    case WS_EVT_DATA:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        debugf("WebSocket[%s][C:%lu] Data[L:%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
      #endif
    break;
  }
}

void onOTAStart() {
  // Log when OTA has started
  debugln(F("OTA update started"));
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if(millis() - i_progress_millis > 1000) {
    i_progress_millis = millis();
    debugf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if(success) {
    debugln(F("OTA update finished successfully!"));
  }
  else {
    debugln(F("There was an error during OTA update!"));
  }
}

void startWebServer() {
  // Register all routes and handlers for the web server.
  registerWebRoutes();

  // Set the MDNS name (get it from your wireless manager)
  setDeviceMdnsName(wirelessMgr->getMdnsName());

  // Configures all URI endpoints using registered routes.
  setupRouting(httpServer);

  // Get preferences for the web UI.
  getSpecialPreferences();

  // Configure the WebSocket endpoint.
  ws.onEvent(onWebSocketEventHandler);
  httpServer.addHandler(&ws);

  // Handle web server Events for telemetry data.
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      debugf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
  });
  httpServer.addHandler(&events);

  // Configure the OTA firmware endpoint handler.
  ElegantOTA.begin(&httpServer);

  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);

  // Start the web server.
  httpServer.begin();

  // Denote that the web server should be started.
  b_httpd_started = true;

  #if defined(DEBUG_SEND_TO_CONSOLE)
    debugln(F("Async HTTP Server Started"));
  #endif
}

// Perform management if the AP and web server are started.
void webLoops() {
  if(b_local_ap_started && b_httpd_started) {
    if(ms_cleanup.remaining() < 1) {
      // Clean up oldest WebSocket connections.
      ws.cleanupClients();

      // Restart timer for next cleanup action.
      ms_cleanup.start(i_websocketCleanup);
    }

    if(ms_apclient.remaining() < 1) {
      // Update the current count of AP clients.
      i_ap_client_count = WiFi.softAPgetStationNum();

      // Restart timer for next count.
      ms_apclient.start(i_apClientCount);
    }

    if(ms_otacheck.remaining() < 1) {
      // Handles device reboot after an OTA update.
      ElegantOTA.loop();

      // Restart timer for next check.
      ms_otacheck.start(i_otaCheck);
    }
  }
}

// Stops the web server and disables WiFi to save power or for security.
void shutdownWireless() {
  if(WiFi.getMode() != WIFI_OFF) {
    // Close all websocket connections and stop the web server.
    ws.closeAll();
    httpServer.end();
    b_httpd_started = false;

    // Disconnect WiFi and turn off radio.
    WiFi.disconnect(true);
    delay(1);
    WiFi.mode(WIFI_OFF);
    delay(1);
    b_local_ap_started = false;
    b_ext_wifi_started = false;

    #if defined(DEBUG_WIRELESS_SETUP)
      debugln(F("Wireless and web server shut down."));
    #endif
  }
}

// Restarts WiFi and web server when needed.
void restartWireless() {
  if(!b_local_ap_started) {
    if(startWiFi()) {
      // Start the local web server.
      startWebServer();

      // Begin timer for remote client events.
      ms_cleanup.start(i_websocketCleanup);
      ms_apclient.start(i_apClientCount);
      ms_otacheck.start(i_otaCheck);

      #if defined(DEBUG_WIRELESS_SETUP)
        debugln(F("Wireless and web server restarted."));
      #endif
    }
  }
}

// Send a debug event to connected clients via Server-Sent Events (SSE).
void sendDebugEvent(const char* message) {
  events.send(message, "debug", millis());
}

/**
 * Standard Page Handlers - Delivers the main web pages and common content
 */

void handleRoot(AsyncWebServerRequest *request) {
  // Used for the root page (/ = index.html) from the web server.
  debugln(F("Sending -> Index HTML"));
  size_t i_file_len = embeddedFileSize(_binary_assets_index_html_gz_start, _binary_assets_index_html_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_HTML, _binary_assets_index_html_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handleRootJS(AsyncWebServerRequest *request) {
  // Used for the root page (/ = index.js) from the web server.
  debugln(F("Sending -> Index JavaScript"));
  size_t i_file_len = embeddedFileSize(_binary_assets_index_js_gz_start, _binary_assets_index_js_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_JAVASCRIPT, _binary_assets_index_js_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handleCommonJS(AsyncWebServerRequest *request) {
  // Used for all pages (common.js) from the web server.
  debugln(F("Sending -> Common JavaScript"));
  size_t i_file_len = embeddedFileSize(_binary_assets_common_js_gz_start, _binary_assets_common_js_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_JAVASCRIPT, _binary_assets_common_js_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handleStylesheet(AsyncWebServerRequest *request) {
  // Used for the common stylesheet of the web server.
  debugln(F("Sending -> Main StyleSheet"));
  size_t i_file_len = embeddedFileSize(_binary_assets_style_css_gz_start, _binary_assets_style_css_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_CSS, _binary_assets_style_css_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handleFavIco(AsyncWebServerRequest *request) {
  // Used for the favicon of the web server.
  debugln(F("Sending -> Favicon"));
  size_t i_file_len = embeddedFileSize(_binary_assets_favicon_ico_gz_start, _binary_assets_favicon_ico_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_ICON, _binary_assets_favicon_ico_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response); // Serve gzipped .ico file.
}

void handleFavSvg(AsyncWebServerRequest *request) {
  // Used for the favicon of the web server.
  debugln(F("Sending -> Favicon"));
  size_t i_file_len = embeddedFileSize(_binary_assets_favicon_svg_gz_start, _binary_assets_favicon_svg_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_SVG, _binary_assets_favicon_svg_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response); // Serve gzipped .svg file.
}

void handleEquipSvg(AsyncWebServerRequest *request) {
  // Used for the equipment view from the web server.
  debugln(F("Sending -> Equipment SVG"));
  size_t i_file_len = embeddedFileSize(_binary_assets_equipment_svg_gz_start, _binary_assets_equipment_svg_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_SVG, _binary_assets_equipment_svg_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response);
}

void handleNetwork(AsyncWebServerRequest *request) {
  // Used for the network page from the web server.
  debugln(F("Sending -> Network HTML"));
  size_t i_file_len = embeddedFileSize(_binary_assets_network_html_gz_start, _binary_assets_network_html_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_HTML, _binary_assets_network_html_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handlePassword(AsyncWebServerRequest *request) {
  // Used for the password page from the web server.
  debugln(F("Sending -> Password HTML"));
  size_t i_file_len = embeddedFileSize(_binary_assets_password_html_gz_start, _binary_assets_password_html_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_HTML, _binary_assets_password_html_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handleDeviceSettings(AsyncWebServerRequest *request) {
  // Used for the device page from the web server.
  debugln(F("Sending -> Device Settings HTML"));
  size_t i_file_len = embeddedFileSize(_binary_assets_device_html_gz_start, _binary_assets_device_html_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_HTML, _binary_assets_device_html_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handleSwagger(AsyncWebServerRequest *request) {
  // Used for the SwaggerUI page (/ = swaggerui.html) from the web server.
  debugln(F("Sending -> SwaggerUI HTML"));
  size_t i_file_len = embeddedFileSize(_binary_assets_swaggerui_html_gz_start, _binary_assets_swaggerui_html_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_HTML, _binary_assets_swaggerui_html_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

/**
 * Peripheral Page Handlers - Delivers the preference pages for available peripherals
 */

void handlePackSettings(AsyncWebServerRequest *request) {
  // Tell the pack that we'll need the latest pack EEPROM values.
  executeCommand(A_REQUEST_PREFERENCES_PACK);

  // Used for the settings page from the web server.
  debugln(F("Sending -> Pack Settings HTML"));
  size_t i_file_len = embeddedFileSize(_binary_assets_pack_html_gz_start, _binary_assets_pack_html_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_HTML, _binary_assets_pack_html_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handleWandSettings(AsyncWebServerRequest *request) {
  // Tell the pack that we'll need the latest wand EEPROM values.
  executeCommand(A_REQUEST_PREFERENCES_WAND);

  // Used for the settings page from the web server.
  debugln(F("Sending -> Wand Settings HTML"));
  size_t i_file_len = embeddedFileSize(_binary_assets_wand_html_gz_start, _binary_assets_wand_html_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_HTML, _binary_assets_wand_html_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handleSmokeSettings(AsyncWebServerRequest *request) {
  // Tell the pack that we'll need the latest smoke EEPROM values.
  executeCommand(A_REQUEST_PREFERENCES_SMOKE);

  // Used for the settings page from the web server.
  debugln(F("Sending -> Smoke Settings HTML"));
  size_t i_file_len = embeddedFileSize(_binary_assets_smoke_html_gz_start, _binary_assets_smoke_html_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_HTML, _binary_assets_smoke_html_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handleGetDeviceConfig(AsyncWebServerRequest *request) {
  // Return current device settings as a stringified JSON object.
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_JSON, getDeviceConfig());
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  request->send(response);
}

void handleGetPackConfig(AsyncWebServerRequest *request) {
  // Return current pack settings as a stringified JSON object.
  getPackPrefsObject(); // Call common function (also used by Attenuator)
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_JSON, getPackConfig());
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  request->send(response);
}

void handleGetWandConfig(AsyncWebServerRequest *request) {
  // Return current wand settings as a stringified JSON object.
  // Object should be waiting in memory after being returned.
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_JSON, getWandConfig());
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  request->send(response);
}

void handleGetSmokeConfig(AsyncWebServerRequest *request) {
  // Return current smoke settings as a stringified JSON object.
  getSmokePrefsObject(); // Call common function (also used by Attenuator)
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_JSON, getSmokeConfig());
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  request->send(response);
}

void handleGetStatus(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_JSON, getEquipmentStatus());
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  request->send(response);
}

void handleGetWifi(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_JSON, getWifiSettings());
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  request->send(response);
}

void handleGetSSIDs(AsyncWebServerRequest *request) {
  // Prepare a JSON object with an array of in-range 2.4 GHz WiFi networks.
  String wifiNetworks;
  String ssidList[40];
  JsonDocument jsonBody;

  // Return available SSIDs (up to 40) as a String array.
  uint8_t i_found = wirelessMgr->scanForSSIDs(ssidList, 40);

  // Make a single array property and add each discovered SSID.
  JsonArray arr = jsonBody["networks"].to<JsonArray>();
  for(uint8_t i = 0; i < i_found; ++i) {
    arr.add(ssidList[i]);
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, wifiNetworks);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_JSON, wifiNetworks);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  request->send(response);
}

// Handles DELETE /wifi/network/{index} to remove a saved WiFi network by index.
void handleDeleteNetwork(AsyncWebServerRequest *request) {
  int networkIndex = -1;
  String s_path = request->url();
  if(s_path.length() > 0) {
    int lastSlash = s_path.lastIndexOf('/');
    if(lastSlash >= 0 && lastSlash < s_path.length() - 1) {
      String segment = s_path.substring(lastSlash + 1);
      if(segment.length() == 0) {
        request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Missing network index."));
        return;
      }
      networkIndex = segment.toInt();
    }
  }

  int count = wirelessMgr->getPreferredNetworkCount();
  if(networkIndex < 0 || networkIndex >= count) {
    request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Invalid network index."));
    return;
  }

  bool removed = wirelessMgr->removePreferredNetwork((uint8_t)networkIndex);
  if(removed) {
    request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus("Saved network successfully removed."));
  } else {
    request->send(HTTP_STATUS_404, MIME_JSON, returnJsonStatus("Network not found or could not be removed."));
  }
}

void handleRestart(AsyncWebServerRequest *request) {
  // Performs a restart of the device.
  request->send(HTTP_STATUS_204, MIME_JSON, returnJsonStatus());
  delay(1000);
  ESP.restart();
}

/**
 * Action Handlers - Perform specific actions via web requests
 */

void handlePackOn(AsyncWebServerRequest *request) {
  if(((gpstarPack.getSystemMode() == MODE_SUPER_HERO && PACK_STATE == MODE_ON) || (gpstarPack.getSystemMode() == MODE_ORIGINAL && gpstarPack.getIonArmSwitch() == RED_SWITCH_ON)) || b_pack_shutting_down) {
    request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Pack is already powered on or is shutting down")); // 409 Conflict
    return;
  }

  debugln(F("Web: Turn Pack On"));
  executeCommand(A_TURN_PACK_ON);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handlePackOff(AsyncWebServerRequest *request) {
  if(((gpstarPack.getSystemMode() == MODE_SUPER_HERO && PACK_STATE != MODE_ON) || (gpstarPack.getSystemMode() == MODE_ORIGINAL && gpstarPack.getIonArmSwitch() == RED_SWITCH_OFF)) || b_pack_shutting_down) {
    request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Pack is already powered off or is shutting down")); // 409 Conflict
    return;
  }

  debugln(F("Web: Turn Pack Off"));
  executeCommand(A_TURN_PACK_OFF);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleAttenuatePack(AsyncWebServerRequest *request) {
  if(i_cyclotron_multiplier > 2) {
    // Only send command to pack if cyclotron is not "normal".
    debugln(F("Web: Cancel Overheat Warning"));
    executeCommand(A_WARNING_CANCELLED);
    request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  }
  else {
    // Tell the user why the requested action failed.
    request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("System not in overheat warning state")); // 400 Bad Request
  }
}

void handleManualVent(AsyncWebServerRequest *request) {
  if(PACK_STATE != MODE_ON) {
    request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Pack is not powered on")); // 409 Conflict
    return;
  }

  debugln(F("Web: Manual Vent Triggered"));
  executeCommand(A_MANUAL_OVERHEAT);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleCableOn(AsyncWebServerRequest *request) {
  if(b_use_ribbon_cable) {
    // We silently return from this so as not to annoy users with proper ribbon cables.
    request->send(HTTP_STATUS_201, MIME_JSON, returnJsonStatus());
    //request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Physical ribbon cable mode is selected")); // 409 Conflict
    return;
  }

  if(!b_manual_cable_alarm) {
    request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Ribbon cable is already attached")); // 409 Conflict
    return;
  }

  debugln(F("Web: Ribbon Cable Attach Triggered"));
  executeCommand(A_ALARM_OFF);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleCableOff(AsyncWebServerRequest *request) {
  if(b_use_ribbon_cable) {
    // We silently return from this so as not to annoy users with proper ribbon cables.
    request->send(HTTP_STATUS_201, MIME_JSON, returnJsonStatus());
    //request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Physical ribbon cable mode is selected")); // 409 Conflict
    return;
  }

  if(b_manual_cable_alarm) {
    request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Ribbon cable is already detached")); // 409 Conflict
    return;
  }

  debugln(F("Web: Ribbon Cable Detach Triggered"));
  executeCommand(A_ALARM_ON);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleMashLockout(AsyncWebServerRequest *request) {
  if(PACK_STATE != MODE_ON) {
    request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Pack is not powered on")); // 409 Conflict
    return;
  }
  else if(b_wand_mash_lockout) {
    request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Pack is already locked out")); // 409 Conflict
    return;
  }

  debugln(F("Web: Manual Button Mash Lockout Triggered"));
  executeCommand(A_SYSTEM_LOCKOUT);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleCancelLockout(AsyncWebServerRequest *request) {
  if(!b_wand_mash_lockout) {
    request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Pack is not locked out")); // 409 Conflict
    return;
  }

  debugln(F("Web: Button Mash Lockout Cancelled"));
  executeCommand(A_CANCEL_LOCKOUT);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleToggleSmoke(AsyncWebServerRequest *request) {
  debugln(F("Web: Smoke Toggle Triggered"));

  String s_path = request->url();
  if(s_path.length() > 0) {
    int lastSlash = s_path.lastIndexOf('/');
    if(lastSlash >= 0 && lastSlash < s_path.length() - 1) {
      String segment = s_path.substring(lastSlash + 1);
      if(segment == "on") {
        executeCommand(A_TOGGLE_SMOKE, 2);
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
      else if(segment == "off") {
        executeCommand(A_TOGGLE_SMOKE, 1);
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
    }
  }

  debugln(F("Invalid Smoke State"));
  request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Invalid Smoke State")); // 400 Bad Request
}

void handleToggleVibration(AsyncWebServerRequest *request) {
  debugln(F("Web: Vibration Toggle Triggered"));

  String s_path = request->url();
  if(s_path.length() > 0) {
    int lastSlash = s_path.lastIndexOf('/');
    if(lastSlash >= 0 && lastSlash < s_path.length() - 1) {
      String segment = s_path.substring(lastSlash + 1);
      if(segment == "on") {
        executeCommand(A_TOGGLE_VIBRATION, 2);
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
      else if(segment == "off") {
        executeCommand(A_TOGGLE_VIBRATION, 1);
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
    }
  }

  debugln(F("Invalid Vibration State"));
  request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Invalid Vibration State")); // 400 Bad Request
}

void handleCyclotronDirection(AsyncWebServerRequest *request) {
  debugln(F("Web: Cyclotron Direction Toggle Triggered"));

  String s_path = request->url();
  if(s_path.length() > 0) {
    int lastSlash = s_path.lastIndexOf('/');
    if(lastSlash >= 0 && lastSlash < s_path.length() - 1) {
      String segment = s_path.substring(lastSlash + 1);
      if(segment == "clockwise") {
        executeCommand(A_CYCLOTRON_DIRECTION_TOGGLE, 2);
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
      else if(segment == "counterclockwise") {
        executeCommand(A_CYCLOTRON_DIRECTION_TOGGLE, 1);
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
    }
  }

  debugln(F("Invalid Direction"));
  request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Invalid Direction")); // 400 Bad Request
}

uint16_t getYearFromPath(const String& s_path) {
  // Check that the path value is not empty.
  if(s_path.length() > 0) {
    int lastSlash = s_path.lastIndexOf('/');
    if(lastSlash >= 0 && lastSlash < s_path.length() - 1) {
      String segment = s_path.substring(lastSlash + 1);
      uint16_t year = segment.toInt();
      // Only return if segment is a valid theme year (1984, 1989, 2021, 2024)
      if(year == 1984 || year == 1989 || year == 2021 || year == 2024) {
        return year;
      }
    }
  }

  return 0; // Indicate no valid theme was set.
}

void handleThemeChange(AsyncWebServerRequest *request) {
  debugln(F("Web: Theme Change Triggered"));

  // Pre-check: Prevent theme change if pack or wand is running.
  if(PACK_STATE == MODE_ON || b_wand_on || b_pack_shutting_down) {
    request->send(409, "application/json", returnJsonStatus("Theme change not allowed while pack or wand is running.")); // 409 Conflict
    return;
  }

  uint16_t i_year = getYearFromPath(request->url());
  switch(i_year) {
    case 1984:
      executeCommand(A_YEAR_1984);
    break;
    case 1989:
      executeCommand(A_YEAR_1989);
    break;
    case 2021:
      executeCommand(A_YEAR_AFTERLIFE);
    break;
    case 2024:
      executeCommand(A_YEAR_FROZEN_EMPIRE);
    break;
    default:
      debugln(F("Invalid Theme Year"));
      request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Invalid Theme Year")); // 400 Bad Request
      return;
    break;
  }

  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

STREAM_MODES getStreamModeFromPath(const String& s_path) {
  // Default to the current stream mode, allowing an invalid stream name to become a no-op.
  STREAM_MODES newStreamMode = gpstarPack.getStreamMode();

  // Check that the path value is not empty.
  if(s_path.length() > 0) {
    int lastSlash = s_path.lastIndexOf('/');
    if(lastSlash >= 0 && lastSlash < s_path.length() - 1) {
      String segment = s_path.substring(lastSlash + 1);
      if(segment == "proton") {
        newStreamMode = PROTON;
      }
      else if(segment == "stasis") {
        newStreamMode = STASIS;
      }
      else if(segment == "slime") {
        newStreamMode = SLIME;
      }
      else if(segment == "meson") {
        newStreamMode = MESON;
      }
      else if(segment == "spectral") {
        newStreamMode = SPECTRAL;
      }
      else if(segment == "halloween") {
        newStreamMode = HOLIDAY_HALLOWEEN;
      }
      else if(segment == "christmas") {
        newStreamMode = HOLIDAY_CHRISTMAS;
      }
      else if(segment == "custom") {
        newStreamMode = SPECTRAL_CUSTOM;
      }
    }
  }

  return newStreamMode;
}

void handleStreamModeChange(AsyncWebServerRequest *request) {
  debugln(F("Web: Firing Mode Change Triggered"));

  // Pre-check: Prevent stream mode change when the system can't handle it.
  if(!canChangeStreamMode()) {
    request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Stream mode change not allowed while pack is firing or in error state.")); // 409 Conflict
    return;
  }

  // Get the requested stream mode from the URL path and map to a valid stream type.
  // The changeStreamMode() function will handle any further checks for options.
  changeStreamMode(getStreamModeFromPath(request->url()));
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleToggleMute(AsyncWebServerRequest *request) {
  debugln(F("Web: Toggle Mute"));

  String s_path = request->url();
  if(s_path.length() > 0) {
    int lastSlash = s_path.lastIndexOf('/');
    if(lastSlash >= 0 && lastSlash < s_path.length() - 1) {
      String segment = s_path.substring(lastSlash + 1);
      if(segment == "mute") {
        executeCommand(A_TOGGLE_MUTE, 2);
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
      else if(segment == "unmute") {
        executeCommand(A_TOGGLE_MUTE, 1);
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
    }
  }

  debugln(F("Invalid Action"));
  request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Invalid Action")); // 400 Bad Request
}

void handleMasterVolumeUp(AsyncWebServerRequest *request) {
  debugln(F("Web: Master Volume Up"));
  executeCommand(A_VOLUME_INCREASE);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleMasterVolumeDown(AsyncWebServerRequest *request) {
  debugln(F("Web: Master Volume Down"));
  executeCommand(A_VOLUME_DECREASE);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleMasterVolumeSet(AsyncWebServerRequest *request) {
  debugln(F("Web: Master Volume Set"));

  String s_path = request->url();
  if(s_path.length() > 0) {
    int lastSlash = s_path.lastIndexOf('/');
    if(lastSlash >= 0 && lastSlash < s_path.length() - 1) {
      String segment = s_path.substring(lastSlash + 1);

      // Check if segment is a valid number (0 is valid, or toInt() returns non-zero)
      if(segment == "0" || segment.toInt() != 0) {
        uint8_t volume = abs(segment.toInt());

        // Validate and constrain to 0-100 range
        if(volume <= 100) {
          // Set volume directly to the specified level
          if(setMasterVolumePercentage(volume)) {
            request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
            notifyWSClients();
            return;
          }
          else {
            debugln(F("Failed to set volume"));
            request->send(HTTP_STATUS_500, MIME_JSON, returnJsonStatus("Failed to set volume"));
            return;
          }
        }
      }
    }
  }

  debugln(F("Invalid Volume Level"));
  request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Invalid Volume Level (0-100)"));
}

void handleEffectsVolumeUp(AsyncWebServerRequest *request) {
  debugln(F("Web: Effects Volume Up"));
  executeCommand(A_VOLUME_SOUND_EFFECTS_INCREASE);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleEffectsVolumeDown(AsyncWebServerRequest *request) {
  debugln(F("Web: Effects Volume Down"));
  executeCommand(A_VOLUME_SOUND_EFFECTS_DECREASE);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleMusicVolumeUp(AsyncWebServerRequest *request) {
  debugln(F("Web: Music Volume Up"));
  executeCommand(A_VOLUME_MUSIC_INCREASE);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleMusicVolumeDown(AsyncWebServerRequest *request) {
  debugln(F("Web: Music Volume Down"));
  executeCommand(A_VOLUME_MUSIC_DECREASE);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleMusicStartStop(AsyncWebServerRequest *request) {
  debugln(F("Web: Music Start/Stop"));
  executeCommand(A_MUSIC_START_STOP);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleMusicPauseResume(AsyncWebServerRequest *request) {
  debugln(F("Web: Music Pause/Resume"));
  executeCommand(A_MUSIC_PAUSE_RESUME);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleNextMusicTrack(AsyncWebServerRequest *request) {
  debugln(F("Web: Next Music Track"));
  executeCommand(A_MUSIC_NEXT_TRACK);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handlePrevMusicTrack(AsyncWebServerRequest *request) {
  debugln(F("Web: Prev Music Track"));
  executeCommand(A_MUSIC_PREV_TRACK);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleLoopMusicTrack(AsyncWebServerRequest *request) {
  debugln(F("Web: Toggle Music Track Loop"));

  String s_path = request->url();
  if(s_path.length() > 0) {
    int lastSlash = s_path.lastIndexOf('/');
    if(lastSlash >= 0 && lastSlash < s_path.length() - 1) {
      String segment = s_path.substring(lastSlash + 1);
      if(segment == "one") {
        executeCommand(A_MUSIC_TRACK_LOOP_TOGGLE, 2);
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
      else if(segment == "all") {
        executeCommand(A_MUSIC_TRACK_LOOP_TOGGLE, 1);
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
    }
  }

  debugln(F("Invalid Looping Option"));
  request->send(HTTP_STATUS_400, MIME_JSON, "Invalid Looping Option"); // 400 Bad Request
}

void handleShuffleMusicTracks(AsyncWebServerRequest *request) {
  debugln(F("Web: Toggle Music Track Shuffling"));

  String s_path = request->url();
  if(s_path.length() > 0) {
    int lastSlash = s_path.lastIndexOf('/');
    if(lastSlash >= 0 && lastSlash < s_path.length() - 1) {
      String segment = s_path.substring(lastSlash + 1);
      if(segment == "on") {
        executeCommand(A_MUSIC_TRACK_SHUFFLE_TOGGLE, 2);
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
      else if(segment == "off") {
        executeCommand(A_MUSIC_TRACK_SHUFFLE_TOGGLE, 1);
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
    }
  }

  debugln(F("Invalid Shuffle Option"));
  request->send(HTTP_STATUS_400, MIME_JSON, "Invalid Shuffle Option"); // 400 Bad Request
}

void handleSelectMusicTrack(AsyncWebServerRequest *request) {
  String c_music_track = "";

  if(request->hasParam("track")) {
    // Get the parameter "track" if it exists (will be a String).
    c_music_track = request->getParam("track")->value();
  }

  if(c_music_track.toInt() != 0 && c_music_track.toInt() >= i_music_track_start) {
    uint16_t i_music_track = c_music_track.toInt();
    debugln(F("Web: Selected Music Track: ") + String(i_music_track));
    executeCommand(A_MUSIC_PLAY_TRACK, i_music_track); // Inform the pack of the new track.
    request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  }
  else {
    // Tell the user why the requested action failed.
    request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Invalid track number requested")); // 400 Bad Request
  }
}

void handleSaveAllEEPROM(AsyncWebServerRequest *request) {
  debugln(F("Web: Save All EEPROM"));
  executeCommand(A_SAVE_EEPROM_SETTINGS_PACK);
  executeCommand(A_SAVE_EEPROM_SETTINGS_WAND);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleSavePackEEPROM(AsyncWebServerRequest *request) {
  debugln(F("Web: Save Pack EEPROM"));
  executeCommand(A_SAVE_EEPROM_SETTINGS_PACK);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleResetPackEEPROM(AsyncWebServerRequest *request) {
  debugln(F("Web: Reset Pack EEPROM"));
  executeCommand(A_RESET_EEPROM_SETTINGS_PACK);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleSaveWandEEPROM(AsyncWebServerRequest *request) {
  debugln(F("Web: Save Wand EEPROM"));
  executeCommand(A_SAVE_EEPROM_SETTINGS_WAND);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleResetWandEEPROM(AsyncWebServerRequest *request) {
  debugln(F("Web: Reset Wand EEPROM"));
  executeCommand(A_RESET_EEPROM_SETTINGS_WAND);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

/**
 * Body Handler Methods - These handlers process JSON body content from POST requests
 */

// Handles the JSON body for the device settings save request.
AsyncCallbackJsonWebHandler *handleSaveDeviceConfig = new AsyncCallbackJsonWebHandler("/config/device/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  JsonDocument jsonBody;
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  } else {
    debugln(F("Body was not a JSON object"));
  }

  try {
    // First check if a new private WiFi network name has been chosen.
    String newSSID = jsonBody["wifiName"].as<String>();
    newSSID = sanitizeSSID(newSSID); // Jacques, clean him!
    bool b_ssid_changed = false;

    // Update the private network name ONLY if the new value differs from the current SSID.
    if(newSSID != "" && newSSID != wirelessMgr->getLocalNetworkName()){
      if(newSSID.length() >= 8 && newSSID.length() <= 32) {
        // Create Preferences object to handle non-volatile storage (NVS).
        Preferences preferences;

        // Accesses namespace in read/write mode.
        if(preferences.begin("credentials", false)) {
          #if defined(DEBUG_SEND_TO_CONSOLE)
            debugln(F("New Private SSID: "));
            debugln(newSSID);
          #endif
          preferences.putString("ssid", newSSID); // Store SSID in case this was altered.
          preferences.end();
        }

        b_ssid_changed = true; // This will cause a reboot of the device after saving.
      }
      else {
        // Immediately return an error if the network name was invalid.
        request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Error: Network name must be between 8 and 32 characters in length.")); // 400 Bad Request
      }
    }

    // General Options - Returned as unsigned integers
    if(jsonBody["displayType"].is<unsigned char>()) {
      switch(jsonBody["displayType"].as<unsigned char>()) {
        case 0:
          DISPLAY_TYPE = STATUS_TEXT;
        break;
        case 1:
          DISPLAY_TYPE = STATUS_GRAPHIC;
        break;
        case 2:
          DISPLAY_TYPE = STATUS_BOTH;
        break;
      }
    }

    if(jsonBody["useAnimation"].is<bool>()) {
      // Enable/disable UI animation effects.
      b_enable_ui_animations = jsonBody["useAnimation"].as<bool>();
    }

    // Get the track listing from the text field.
    String songList = jsonBody["songList"].as<String>();
    bool b_list_err = false;

    // Create Preferences object to handle non-volatile storage (NVS).
    Preferences preferences;

    // Accesses namespace in read/write mode.
    if(preferences.begin("device", false)) {
      preferences.putUChar("display_type", DISPLAY_TYPE);
      preferences.putBool("use_animations", b_enable_ui_animations);

      // Store the song list to preferences.
      if(songList.length() <= 2000) {
        if(songList == "null") {
          songList = "";
        }

        // Update song lists if contents are under 2000 bytes.
        #if defined(DEBUG_SEND_TO_CONSOLE)
          debugln(F("Song List Bytes: "));
          debugln(songList.length());
        #endif
        preferences.putString("track_list", songList);
        s_track_listing = songList;
      }
      else {
        // Max size for preferences is 4KB so we need to make reserve space for other items.
        // Also, there is a 2KB limit for a single item which is what we're storing here.
        b_list_err = true;
      }

      preferences.end();
    }

    if(b_list_err) {
      request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus("Settings updated, but song list exceeds the 2,000 bytes maximum and was not saved."));
    } else if(b_ssid_changed) {
      request->send(HTTP_STATUS_201, MIME_JSON, returnJsonStatus("Settings updated, restart required. Please use the new network name to connect to your device."));
    }
    else {
      request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus("Settings updated."));
    }
  }
  catch (...) {
    request->send(HTTP_STATUS_500, MIME_JSON, returnJsonStatus("An error was encountered while saving settings.")); // 500 Server Error
  }
}); // handleSaveDeviceConfig

// Handles the JSON body for the pack settings save request.
AsyncCallbackJsonWebHandler *handleSavePackConfig = new AsyncCallbackJsonWebHandler("/config/pack/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  JsonDocument jsonBody;
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    debugln(F("Body was not a JSON object"));
    request->send(HTTP_STATUS_500, MIME_JSON, returnJsonStatus("Body was not a JSON object.")); // 500 Server Error
    return;
  }

  if(PACK_STATE != MODE_ON && !b_wand_on) {
    try {
      // Numeric fields - General options
      packConfig.defaultSystemModePack = (SYSTEM_MODES)jsonBody["defaultSystemModePack"].as<uint8_t>();
      packConfig.defaultYearThemePack = (SYSTEM_THEMES)jsonBody["defaultYearThemePack"].as<uint8_t>();
      packConfig.currentYearThemePack = (SYSTEM_THEMES)jsonBody["currentYearThemePack"].as<uint8_t>();
      packConfig.packVibration = (VIBRATION_MODES)jsonBody["packVibration"].as<uint8_t>();
      packConfig.defaultPackVolume = jsonBody["defaultPackVolume"].as<uint8_t>();

      // Boolean fields - Effect toggles
      updateJsonBool(packConfig.ribbonCableAlarm, jsonBody, "ribbonCableAlarm");
      updateJsonBool(packConfig.wandQuickBootup, jsonBody, "wandQuickBootup");
      updateJsonBool(packConfig.protonStreamEffects, jsonBody, "protonStreamEffects");
      updateJsonBool(packConfig.brassStartupLoop, jsonBody, "brassStartupLoop");
      updateJsonBool(packConfig.overheatStrobeNF, jsonBody, "overheatStrobeNF");
      updateJsonBool(packConfig.overheatLightsOff, jsonBody, "overheatLightsOff");
      updateJsonBool(packConfig.overheatSyncToFan, jsonBody, "overheatSyncToFan");
      updateJsonBool(packConfig.demoLightMode, jsonBody, "demoLightMode");
      updateJsonBool(packConfig.fadeoutIdleSounds, jsonBody, "fadeoutIdleSounds");
      updateJsonBool(packConfig.gpstarAudioLed, jsonBody, "gpstarAudioLed");

      // Update certain operational values immediately.
      switch(packConfig.defaultSystemModePack) {
        case 1:
        default:
          gpstarPack.setSystemMode(MODE_SUPER_HERO);
          gpstarPack.setIonArmSwitch(RED_SWITCH_OFF);
        break;

        case 2:
          gpstarPack.setSystemMode(MODE_ORIGINAL);
          gpstarPack.setIonArmSwitch(RED_SWITCH_OFF);
        break;
      }

      // GPStar II WiFi Toggles
      updateJsonBool(packConfig.isWiFiEnabled, jsonBody, "isWiFiEnabled");
      updateJsonBool(packConfig.resetWifiPassword, jsonBody, "resetWifiPassword");

      // Numeric fields - Cyclotron Lid options
      packConfig.ledCycLidCount = jsonBody["ledCycLidCount"].as<uint8_t>();
      packConfig.ledCycLidHue = jsonBody["ledCycLidHue"].as<uint8_t>();
      packConfig.ledCycLidSat = jsonBody["ledCycLidSat"].as<uint8_t>();
      packConfig.ledCycLidLum = jsonBody["ledCycLidLum"].as<uint8_t>();
      packConfig.ledCycLidCenter = jsonBody["ledCycLidCenter"].as<uint8_t>();

      // Boolean fields - Cyclotron Lid toggles
      updateJsonBool(packConfig.cyclotronDirection, jsonBody, "cyclotronDirection");
      updateJsonBool(packConfig.ledCycLidFade, jsonBody, "ledCycLidFade");
      updateJsonBool(packConfig.ledVGCyclotron, jsonBody, "ledVGCyclotron");
      updateJsonBool(packConfig.ledCycLidSimRing, jsonBody, "ledCycLidSimRing");
      updateJsonBool(packConfig.disableLidDetection, jsonBody, "disableLidDetection");

      // Numeric fields - Inner Cyclotron options
      packConfig.ledCycInnerPanel = jsonBody["ledCycInnerPanel"].as<uint8_t>();
      packConfig.ledCycPanLum = jsonBody["ledCycPanLum"].as<uint8_t>();
      packConfig.ledCycCakeCount = jsonBody["ledCycCakeCount"].as<uint8_t>();
      packConfig.ledCycCakeHue = jsonBody["ledCycCakeHue"].as<uint8_t>();
      packConfig.ledCycCakeSat = jsonBody["ledCycCakeSat"].as<uint8_t>();
      packConfig.ledCycCakeLum = jsonBody["ledCycCakeLum"].as<uint8_t>();
      packConfig.ledCycCavCount = jsonBody["ledCycCavCount"].as<uint8_t>();
      if(packConfig.ledCycCavCount > 20) {
        packConfig.ledCycCavCount = 20; // Set maximum allowed.
      }
      packConfig.ledCycCavType = jsonBody["ledCycCavType"].as<uint8_t>();

      // Boolean fields - Inner Cyclotron toggles
      updateJsonBool(packConfig.ledCycCakeGRB, jsonBody, "ledCycCakeGRB");

      // Numeric fields - Power Cell options
      packConfig.ledPowercellCount = jsonBody["ledPowercellCount"].as<uint8_t>();
      packConfig.ledPowercellHue = jsonBody["ledPowercellHue"].as<uint8_t>();
      packConfig.ledPowercellSat = jsonBody["ledPowercellSat"].as<uint8_t>();
      packConfig.ledPowercellLum = jsonBody["ledPowercellLum"].as<uint8_t>();

      // Boolean field - Power Cell toggles
      updateJsonBool(packConfig.ledInvertPowercell, jsonBody, "ledInvertPowercell");
      updateJsonBool(packConfig.ledVGPowercell, jsonBody, "ledVGPowercell");

      handlePackPrefsUpdate(); // Have the pack save the new settings.
      request->send(HTTP_STATUS_201, MIME_JSON, returnJsonStatus("Settings updated, please test before saving to EEPROM."));
    }
    catch (...) {
      request->send(HTTP_STATUS_500, MIME_JSON, returnJsonStatus("An error was encountered while saving settings.")); // 500 Server Error
    }
  }
  else {
    // Tell the user why the requested action failed.
    request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Pack and/or Wand are running, save action cancelled")); // 409 Conflict
  }
}); // handleSavePackConfig

// Handles the JSON body for the wand settings save request.
AsyncCallbackJsonWebHandler *handleSaveWandConfig = new AsyncCallbackJsonWebHandler("/config/wand/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  JsonDocument jsonBody;
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    debugln(F("Body was not a JSON object"));
    request->send(HTTP_STATUS_500, MIME_JSON, returnJsonStatus("Body was not a JSON object.")); // 500 Server Error
    return;
  }

  if(PACK_STATE != MODE_ON && !b_wand_on) {
    try {
      // Numeric fields - LED options
      wandConfig.ledWandCount = jsonBody["ledWandCount"].as<uint8_t>();
      wandConfig.ledWandHue = jsonBody["ledWandHue"].as<uint8_t>();
      wandConfig.ledWandSat = jsonBody["ledWandSat"].as<uint8_t>();

      // Boolean fields - LED toggles
      updateJsonBool(wandConfig.rgbVentEnabled, jsonBody, "rgbVentEnabled");
      updateJsonBool(wandConfig.rgbVentColours, jsonBody, "rgbVentColours");
      updateJsonBool(wandConfig.autoVentLight, jsonBody, "autoVentLight");

      // Stream mode toggles - Update in the config object for the moment, and save back to the device's state object later.
      // Note that PROTON mode can neither be set nor unset (always enabled).
      (jsonBody["stasisStream"].as<bool>() || gpstarPack.inStreamMode(STASIS)) ? (wandConfig.streamFlags |= FLAG_STASIS) : (wandConfig.streamFlags &= ~FLAG_STASIS);
      (jsonBody["slimeStream"].as<bool>() || gpstarPack.inStreamMode(SLIME)) ? (wandConfig.streamFlags |= FLAG_SLIME) : (wandConfig.streamFlags &= ~FLAG_SLIME);
      (jsonBody["mesonStream"].as<bool>() || gpstarPack.inStreamMode(MESON)) ? (wandConfig.streamFlags |= FLAG_MESON) : (wandConfig.streamFlags &= ~FLAG_MESON);
      (jsonBody["spectralStream"].as<bool>() || gpstarPack.inStreamMode(SPECTRAL)) ? (wandConfig.streamFlags |= FLAG_SPECTRAL) : (wandConfig.streamFlags &= ~FLAG_SPECTRAL);
      (jsonBody["spectralCustomStream"].as<bool>() || gpstarPack.inStreamMode(SPECTRAL_CUSTOM)) ? (wandConfig.streamFlags |= FLAG_SPECTRAL_CUSTOM) : (wandConfig.streamFlags &= ~FLAG_SPECTRAL_CUSTOM);
      (jsonBody["halloweenStream"].as<bool>() || gpstarPack.inStreamMode(HOLIDAY_HALLOWEEN)) ? (wandConfig.streamFlags |= FLAG_HOLIDAY_HALLOWEEN) : (wandConfig.streamFlags &= ~FLAG_HOLIDAY_HALLOWEEN);
      (jsonBody["christmasStream"].as<bool>() || gpstarPack.inStreamMode(HOLIDAY_CHRISTMAS)) ? (wandConfig.streamFlags |= FLAG_HOLIDAY_CHRISTMAS) : (wandConfig.streamFlags &= ~FLAG_HOLIDAY_CHRISTMAS);

      // Numeric fields - General wand options
      wandConfig.defaultYearModeWand = (SYSTEM_THEMES)jsonBody["defaultYearModeWand"].as<uint8_t>();
      wandConfig.defaultYearModeCTS = jsonBody["defaultYearModeCTS"].as<uint8_t>();
      wandConfig.defaultStreamMode = jsonBody["defaultStreamMode"].as<uint8_t>();
      wandConfig.defaultFiringMode = jsonBody["defaultFiringMode"].as<uint8_t>();
      wandConfig.wandVibration = (VIBRATION_MODES)jsonBody["wandVibration"].as<uint8_t>();
      wandConfig.barrelSwitchPolarity = jsonBody["barrelSwitchPolarity"].as<uint8_t>();
      wandConfig.defaultWandVolume = jsonBody["defaultWandVolume"].as<uint8_t>();

      // Boolean fields - General wand toggles
      updateJsonBool(wandConfig.overheatEnabled, jsonBody, "overheatEnabled");
      updateJsonBool(wandConfig.wandSoundsToPack, jsonBody, "wandSoundsToPack");
      updateJsonBool(wandConfig.quickVenting, jsonBody, "quickVenting");
      updateJsonBool(wandConfig.wandBeepLoop, jsonBody, "wandBeepLoop");
      updateJsonBool(wandConfig.wandBootError, jsonBody, "wandBootError");
      updateJsonBool(wandConfig.gpstarAudioLed, jsonBody, "gpstarAudioLed");

      // Numeric fields - Bargraph options
      wandConfig.numBargraphSegments = jsonBody["numBargraphSegments"].as<uint8_t>();
      wandConfig.bargraphIdleAnimation = jsonBody["bargraphIdleAnimation"].as<uint8_t>();
      wandConfig.bargraphFireAnimation = jsonBody["bargraphFireAnimation"].as<uint8_t>();

      // Boolean fields - Bargraph toggles
      updateJsonBool(wandConfig.invertWandBargraph, jsonBody, "invertWandBargraph");
      updateJsonBool(wandConfig.bargraphOverheatBlink, jsonBody, "bargraphOverheatBlink");

      // GPStar II WiFi Toggles
      updateJsonBool(wandConfig.isWiFiEnabled, jsonBody, "isWiFiEnabled");
      updateJsonBool(wandConfig.resetWifiPassword, jsonBody, "resetWifiPassword");

      handleWandPrefsUpdate(); // Have the pack pass the new settings.
      request->send(HTTP_STATUS_201, MIME_JSON, returnJsonStatus("Settings updated, please test before saving to EEPROM."));
    }
    catch (...) {
      request->send(HTTP_STATUS_500, MIME_JSON, returnJsonStatus("An error was encountered while saving settings.")); // 500 Server Error
    }
  }
  else {
    // Tell the user why the requested action failed.
    request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Pack and/or Wand are running, save action cancelled")); // 409 Conflict
  }
}); // handleSaveWandConfig

// Handles the JSON body for the smoke settings save request.
AsyncCallbackJsonWebHandler *handleSaveSmokeConfig = new AsyncCallbackJsonWebHandler("/config/smoke/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  JsonDocument jsonBody;
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    debugln(F("Body was not a JSON object"));
  }

  if(PACK_STATE != MODE_ON && !b_wand_on) {
    try {
      // Boolean field - General smoke toggle
      updateJsonBool(smokeConfig.smokeEnabled, jsonBody, "smokeEnabled");

      // Numeric fields - Overheat duration values (seconds)
      smokeConfig.overheatDuration5 = jsonBody["overheatDuration5"].as<uint8_t>();
      smokeConfig.overheatDuration4 = jsonBody["overheatDuration4"].as<uint8_t>();
      smokeConfig.overheatDuration3 = jsonBody["overheatDuration3"].as<uint8_t>();
      smokeConfig.overheatDuration2 = jsonBody["overheatDuration2"].as<uint8_t>();
      smokeConfig.overheatDuration1 = jsonBody["overheatDuration1"].as<uint8_t>();

      // Boolean fields - Continuous smoke timers (seconds)
      updateJsonBool(smokeConfig.overheatContinuous5, jsonBody, "overheatContinuous5");
      updateJsonBool(smokeConfig.overheatContinuous4, jsonBody, "overheatContinuous4");
      updateJsonBool(smokeConfig.overheatContinuous3, jsonBody, "overheatContinuous3");
      updateJsonBool(smokeConfig.overheatContinuous2, jsonBody, "overheatContinuous2");
      updateJsonBool(smokeConfig.overheatContinuous1, jsonBody, "overheatContinuous1");

      // Boolean fields - Overheat by level toggles
      updateJsonBool(smokeConfig.overheatLevel5, jsonBody, "overheatLevel5");
      updateJsonBool(smokeConfig.overheatLevel4, jsonBody, "overheatLevel4");
      updateJsonBool(smokeConfig.overheatLevel3, jsonBody, "overheatLevel3");
      updateJsonBool(smokeConfig.overheatLevel2, jsonBody, "overheatLevel2");
      updateJsonBool(smokeConfig.overheatLevel1, jsonBody, "overheatLevel1");

      // Numeric fields - Overheat delay values (seconds)
      smokeConfig.overheatDelay5 = jsonBody["overheatDelay5"].as<uint8_t>();
      smokeConfig.overheatDelay4 = jsonBody["overheatDelay4"].as<uint8_t>();
      smokeConfig.overheatDelay3 = jsonBody["overheatDelay3"].as<uint8_t>();
      smokeConfig.overheatDelay2 = jsonBody["overheatDelay2"].as<uint8_t>();
      smokeConfig.overheatDelay1 = jsonBody["overheatDelay1"].as<uint8_t>();

      handleSmokePrefsUpdate(); // Have the pack save and pass the new settings.
      request->send(HTTP_STATUS_201, MIME_JSON, returnJsonStatus("Settings updated, please test before saving to EEPROM."));
    }
    catch (...) {
      request->send(HTTP_STATUS_500, MIME_JSON, returnJsonStatus("An error was encountered while saving settings.")); // 500 Server Error
    }
  }
  else {
    // Tell the user why the requested action failed.
    request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Pack and/or Wand are running, save action cancelled")); // 409 Conflict
  }
}); // handleSaveSmokeConfig

// Handles the JSON body for the password change request.
AsyncCallbackJsonWebHandler *passwordChangeHandler = new AsyncCallbackJsonWebHandler("/password/update", [](AsyncWebServerRequest *request, JsonVariant &json) {
  JsonDocument jsonBody;
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  } else {
    debugln(F("Body was not a JSON object"));
  }

  if(jsonBody["password"].is<const char*>()) {
    String newPasswd = jsonBody["password"].as<String>();

    // Password is used for the built-in Access Point ability, which will be used when a preferred network is not available.
    if(newPasswd.length() >= 8) {
      // Create Preferences object to handle non-volatile storage (NVS).
      Preferences preferences;

      // Accesses namespace in read/write mode.
      if(preferences.begin("credentials", false)) {
        #if defined(DEBUG_SEND_TO_CONSOLE)
          debug(F("New Private WiFi Password: "));
          debugln(newPasswd);
        #endif
        preferences.putString("password", newPasswd); // Store user-provided password.
        preferences.end();
      }

      request->send(HTTP_STATUS_201, MIME_JSON, returnJsonStatus("Password updated, restart required. Please enter your new WiFi password when prompted by your device."));
    }
    else {
      // Password must be at least 8 characters in length.
      request->send(HTTP_STATUS_417, MIME_JSON, returnJsonStatus("Password must be a minimum of 8 characters to meet WPA2 requirements.")); // 417 Expectation Failed
    }
  }
  else {
    debugln(F("No password in JSON body"));
    request->send(HTTP_STATUS_500, MIME_JSON, returnJsonStatus("Unable to update password.")); // 500 Server Error
  }
}); // passwordChangeHandler

// Handles the JSON body for the wifi network info.
AsyncCallbackJsonWebHandler *wifiChangeHandler = new AsyncCallbackJsonWebHandler("/wifi/update", [](AsyncWebServerRequest *request, JsonVariant &json) {
  JsonDocument jsonBody;
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  } else {
    debugln(F("Body was not a JSON object"));
  }

  // Check for 'active' property (object) and use it if present, else use top-level
  JsonObject activeObj;
  if(jsonBody["active"].is<JsonObject>()) {
    activeObj = jsonBody["active"].as<JsonObject>();
  } else {
    debugln(F("No 'active' object in JSON body"));
    request->send(HTTP_STATUS_204, MIME_JSON, returnJsonStatus("Unable to find expected network information in JSON body.")); // 204 No Content
    return;
  }

  if(activeObj["ssid"].is<const char*>() && activeObj["password"].is<const char*>()) {
    bool b_errors = false; // Assume false until otherwise indicated.
    bool b_enabled = wirelessMgr->isExtWifiEnabled(); // Default to the current state.
    updateJsonBool(b_enabled, activeObj, "enabled"); // Update var from JSON if present.
    String wifiNetwork = activeObj["ssid"].as<String>();
    String wifiPasswd = activeObj["password"].as<String>();

    // Handle staticIP logic: if false, blank the fields; if true, use provided string values if present
    bool b_static_ip = false;
    String localAddr = "";
    String subnetMask = "";
    String gatewayIP = "";

    if(activeObj["staticIP"].is<bool>()) {
      b_static_ip = activeObj["staticIP"].as<bool>();
    }

    if(b_static_ip) {
      if(activeObj["address"].is<const char*>()) {
        localAddr = activeObj["address"].as<String>();
      }
      if(activeObj["subnet"].is<const char*>()) {
        subnetMask = activeObj["subnet"].as<String>();
      }
      if(activeObj["gateway"].is<const char*>()) {
        gatewayIP = activeObj["gateway"].as<String>();
      }
    }

    if(!b_enabled) {
      // If disabled, update the stored preference immediately.
      wirelessMgr->disableExtWiFi();
    } else {
      // Check validity of provided values.
      if(wifiNetwork.length() >= 2 && wifiPasswd.length() >= 8) {
        // Clear old network IP info if SSID or password have been changed.
        String old_ssid = wirelessMgr->getExtWifiNetworkName();
        String old_passwd = wirelessMgr->getExtWifiPassword();
        if(old_ssid == "" || old_ssid != wifiNetwork || old_passwd == "" || old_passwd != wifiPasswd) {
          localAddr = "";
          subnetMask = "";
          gatewayIP = "";
        }

        // Continue saving static IP info only if network values are 7 characters or more (eg. N.N.N.N)
        bool b_valid_ip = true;
        if(!(localAddr.length() >= 7 && localAddr != wirelessMgr->getExtWifiAddress().toString())) {
          b_valid_ip = false;
        }
        if(!(subnetMask.length() >= 7 && subnetMask != wirelessMgr->getExtWifiSubnet().toString())) {
          b_valid_ip = false;
        }
        if(!(gatewayIP.length() >= 7 && gatewayIP != wirelessMgr->getExtWifiGateway().toString())) {
          b_valid_ip = false;
        }

        if(!b_valid_ip) {
          // If any of the above values were invalid, clear all three fields.
          localAddr = "";
          subnetMask = "";
          gatewayIP = "";
        }

        // Save and apply the new values as the current external network.
        if(wirelessMgr->savePreferredNetwork(wifiNetwork, wifiPasswd, b_static_ip, localAddr, subnetMask, gatewayIP)) {
          int8_t idx = wirelessMgr->getPreferredNetworkIndex(wifiNetwork);
          if(idx >= 0) {
            if(!wirelessMgr->applyPreferredNetwork((uint8_t)idx)) {
              request->send(HTTP_STATUS_500, MIME_JSON, returnJsonStatus("Unable to apply settings for the current network."));
              return;
            }
          }
          else {
            request->send(HTTP_STATUS_500, MIME_JSON, returnJsonStatus("Unable to locate the preferred network information."));
            return;
          }
        }
        else {
          request->send(HTTP_STATUS_500, MIME_JSON, returnJsonStatus("Unable to save preferred network, check total saved networks (must be 5 or less)."));
          return;
        }
      }
      else {
        b_errors = true; // General error for invalid SSID or password length.
      }
    }

    if(!b_errors) {
      // Disconnect from the WiFi network and re-apply any changes.
      WiFi.disconnect();
      b_ext_wifi_started = false;
      notifyWSClients();

      delay(100); // Delay needed.

      String s_reason = "";
      if(b_enabled) {
        b_ext_wifi_started = startExternalWifi(); // Restart and set global flag.

        if(b_ext_wifi_started) {
          s_reason = "Settings updated, WiFi connection restarted successfully.";
        }
        else {
          s_reason = "Settings updated, but WiFi connection was not successful.";
        }
      }
      else {
        s_reason = "Settings updated, and external WiFi has been disconnected.";
      }

      request->send(HTTP_STATUS_201, MIME_JSON, returnJsonStatus(s_reason));
    }
    else {
      request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus("Errors encountered while processing data. Please re-check submitted values and try again."));
    }
  }
  else {
    debugln(F("No password in JSON body"));
    request->send(HTTP_STATUS_204, MIME_JSON, returnJsonStatus("Unable to update password.")); // 204 No Content
  }
}); // wifiChangeHandler
