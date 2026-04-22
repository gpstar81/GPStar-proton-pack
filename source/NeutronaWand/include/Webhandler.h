/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
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

// Extern declaration for global infrared handler
extern InfraredManager* irManager;

// Declare the external binary data markers for embedded files.
// common.js
extern const uint8_t _binary_assets_common_js_gz_start[];
extern const uint8_t _binary_assets_common_js_gz_end[];
// geometry.stl
extern const uint8_t _binary_assets_geometry_stl_gz_start[];
extern const uint8_t _binary_assets_geometry_stl_gz_end[];
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
// three.js
extern const uint8_t _binary_assets_three_min_js_gz_start[];
extern const uint8_t _binary_assets_three_min_js_gz_end[];
// device.html
extern const uint8_t _binary_assets_device_html_gz_start[];
extern const uint8_t _binary_assets_device_html_gz_end[];
// network.html
extern const uint8_t _binary_assets_network_html_gz_start[];
extern const uint8_t _binary_assets_network_html_gz_end[];
// password.html
extern const uint8_t _binary_assets_password_html_gz_start[];
extern const uint8_t _binary_assets_password_html_gz_end[];
// wand.html
extern const uint8_t _binary_assets_wand_html_gz_start[];
extern const uint8_t _binary_assets_wand_html_gz_end[];
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
void getSpecialPreferences(); // From PreferencesESP.h
void toggleStandaloneMode(bool); // From System.h
void registerWebRoutes(); // From Webrouting.h

/*
 * Text Helper Functions - Converts ENUM values to consistent, user-friendly text
 */

// Rounds a float to 3 decimal places.
float roundFloat(float value) {
  // Shifts the decimal point, rounds, then shifts back.
  return roundf(value * 1000.0f) / 1000.0f;
}

// Rounds a double to 3 decimal places.
float roundDouble(double value) {
  // Shifts the decimal point, rounds, then shifts back.
  return (float)round(value * 1000.0) / 1000.0;
}

String getSensorState() {
  switch(SENSOR_READ_TARGET) {
    case MAG_CALIBRATION:
      return "Magnetometer Calibration";
    break;
    case GYRO_CALIBRATION:
      return "Gyro Calibration";
    break;
    case OFFSETS:
      return "Offsets";
    break;
    case TELEMETRY:
      return "Telemetry";
    break;
    default:
      return "Unknown";
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

  jsonBody["hardIron1"] = magCalData.mag_hardiron[0];
  jsonBody["hardIron2"] = magCalData.mag_hardiron[1];
  jsonBody["hardIron3"] = magCalData.mag_hardiron[2];
  jsonBody["softIron1"] = magCalData.mag_softiron[0];
  jsonBody["softIron2"] = magCalData.mag_softiron[1];
  jsonBody["softIron3"] = magCalData.mag_softiron[2];
  jsonBody["softIron4"] = magCalData.mag_softiron[3];
  jsonBody["softIron5"] = magCalData.mag_softiron[4];
  jsonBody["softIron6"] = magCalData.mag_softiron[5];
  jsonBody["softIron7"] = magCalData.mag_softiron[6];
  jsonBody["softIron8"] = magCalData.mag_softiron[7];
  jsonBody["softIron9"] = magCalData.mag_softiron[8];
  jsonBody["magField"] = magCalData.mag_field;

  // Map the installation orientation to a number for the web UI.
  switch(INSTALL_ORIENTATION) {
    case COMPONENTS_UP_USB_FRONT:
      jsonBody["orientation"] = 1;
    break;
    case COMPONENTS_UP_USB_REAR:
      jsonBody["orientation"] = 2;
    break;
    case COMPONENTS_DOWN_USB_FRONT:
    default:
      jsonBody["orientation"] = 3;
    break;
    case COMPONENTS_DOWN_USB_REAR:
      jsonBody["orientation"] = 4;
    break;
    case COMPONENTS_LEFT_USB_FRONT:
      jsonBody["orientation"] = 5;
    break;
    case COMPONENTS_LEFT_USB_REAR:
      jsonBody["orientation"] = 6;
    break;
    case COMPONENTS_RIGHT_USB_FRONT:
      jsonBody["orientation"] = 7;
    break;
    case COMPONENTS_RIGHT_USB_REAR:
      jsonBody["orientation"] = 8;
    break;
    case COMPONENTS_FACTORY_DEFAULT:
      jsonBody["orientation"] = 9;
    break;
  }

  // Report the magnetometer self test results.
  JsonObject selfTestObj = jsonBody["magSelfTest"].to<JsonObject>();

  // Group XYZ values as arrays for each category
  JsonArray baseline = selfTestObj["baseline"].to<JsonArray>();
  baseline.add(magSelfTest.baselineX);
  baseline.add(magSelfTest.baselineY);
  baseline.add(magSelfTest.baselineZ);

  JsonArray testResult = selfTestObj["results"].to<JsonArray>();
  testResult.add(magSelfTest.selfTestX);
  testResult.add(magSelfTest.selfTestY);
  testResult.add(magSelfTest.selfTestZ);

  JsonArray delta = selfTestObj["delta"].to<JsonArray>();
  delta.add(magSelfTest.deltaX);
  delta.add(magSelfTest.deltaY);
  delta.add(magSelfTest.deltaZ);

  JsonArray passResult = selfTestObj["pass"].to<JsonArray>();
  passResult.add(magSelfTest.passX);
  passResult.add(magSelfTest.passY);
  passResult.add(magSelfTest.passZ);

  // Report the magnetometer configuration.
  JsonObject magConfig = jsonBody["magConfig"].to<JsonObject>();

  // Add user-friendly config options
  magConfig["performanceMode"] = magConfigInfo.performanceMode;
  magConfig["dataRate"] = magConfigInfo.dataRate;
  magConfig["range"] = magConfigInfo.range;
  magConfig["operationMode"] = magConfigInfo.operationMode;

  // Add raw register values as an array of objects
  JsonArray registers = magConfig["registers"].to<JsonArray>();
  for(size_t i = 0; i < sizeof(magConfigInfo.rawRegisters) / sizeof(magConfigInfo.rawRegisters[0]); ++i) {
    JsonObject regObj = registers[i].to<JsonObject>();
    regObj["name"] = magConfigInfo.rawRegisters[i].name;
    regObj["address"] = magConfigInfo.rawRegisters[i].address;
    regObj["value"] = magConfigInfo.rawRegisters[i].value;
  }

  // Report the current standalone mode setting.
  jsonBody["useStandalone"] = b_wand_standalone;

  jsonBody["irWhileFiring"] = b_ir_while_firing;

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getWandConfig() {
  // Prepare a JSON object with information we have gleaned from the system.
  String equipSettings;
  JsonDocument jsonBody;

  try {
    // Provide a flag to indicate prefs are directly available.
    jsonBody["prefsAvailable"] = true; // Always true for the immediate device.

    // Use the device-reported values so we can fine-tune the UI options.
    jsonBody["esp32Wand"] = wandConfig.isESP32;
    jsonBody["isWiFiEnabled"] = wandConfig.isWiFiEnabled;
    jsonBody["resetWifiPassword"] = false;
    jsonBody["gpstarAudio"] = (i_audio_version > 1);

    // Return current powered state for pack and wand.
    jsonBody["packPowered"] = (b_pack_on || b_pack_shutting_down);
    jsonBody["wandPowered"] = (WAND_STATUS == MODE_ON);
    jsonBody["wandConnected"] = (WAND_CONN_STATE == PACK_CONNECTED);

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
    jsonBody["systemMode"] = gpstarWand.getModeName(); // "Super Hero" or "Original"
    jsonBody["systemTheme"] = gpstarWand.getThemeName(); // 1984, 1989 (GB2), 2021 (AL), 2024 (FE)
    jsonBody["currentStreamMode"] = gpstarWand.getStreamModeName(); // String for current firing mode
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

String getEquipmentStatus() {
  // Prepare a JSON object with information we have gleaned from the system.
  String equipStatus;
  JsonDocument jsonBody;

  uint16_t i_music_track_min = 0;
  uint16_t i_music_track_max = 0;

  if(i_music_track_count > 0) {
    i_music_track_min = i_music_track_start; // First music track possible (eg. 500)
    i_music_track_max = i_music_track_start + i_music_track_count - 1; // 500 + N - 1 to be inclusive of the offset value.
  }

  try {
    jsonBody["standalone"] = b_wand_standalone;
    jsonBody["mode"] = gpstarWand.getModeName();
    jsonBody["modeID"] = gpstarWand.getSystemMode();
    jsonBody["theme"] = gpstarWand.getThemeName(getSystemYearMode());
    jsonBody["themeID"] = getSystemYearMode();
    jsonBody["switch"] = gpstarWand.getIonArmSwitchState();
    jsonBody["pack"] = (b_pack_on ? "Powered" : "Idle");
    jsonBody["power"] = gpstarWand.getPowerLevelName();
    jsonBody["safety"] = gpstarWand.getBarrelStateName();
    jsonBody["wandPower"] = (WAND_STATUS == MODE_ON ? "Powered" : "Idle");
    jsonBody["wandMode"] = gpstarWand.getStreamModeName();
    jsonBody["firing"] = (b_firing ? "Firing" : "Idle");
    jsonBody["crossedStreams"] = (b_firing && b_firing_cross_streams);
    jsonBody["lockout"] = b_wand_mash_lockout;
    jsonBody["cable"] = (b_ribbon_cable_attached ? "Connected" : "Disconnected");
    jsonBody["cyclotronLid"] = b_pack_cyclotron_lid_on;
    jsonBody["temperature"] = (WAND_ACTION_STATUS == ACTION_OVERHEATING ? "Venting" : "Normal");
    jsonBody["musicPlaying"] = b_playing_music;
    jsonBody["musicPaused"] = b_music_paused;
    jsonBody["musicLooping"] = b_repeat_track;
    jsonBody["musicShuffled"] = b_shuffle_tracks;
    jsonBody["musicCurrent"] = i_current_music_track;
    jsonBody["musicStart"] = i_music_track_min;
    jsonBody["musicEnd"] = i_music_track_max;
    jsonBody["volMaster"] = i_volume_master_percentage;
    jsonBody["volEffects"] = i_volume_effects_percentage;
    jsonBody["volMusic"] = i_volume_music_percentage;
    jsonBody["sensors"] = getSensorState();
    jsonBody["apClients"] = i_ap_client_count;
    jsonBody["wsClients"] = i_ws_client_count;
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

String getGyroCalJSON() {
  String calibrationData;

  // Create a JSON object with the countdown timer value.
  JsonDocument jsonCalibration;

  // Time remaining in seconds, rounded to 3 decimal places.
  jsonCalibration["t"] = roundFloat(ms_gyro_calibration.remaining() / 1000.0f);

  // Current averaged values for offsets the gyro and accelerometer.
  jsonCalibration["aX"] = roundFloat(calibratedOffsets.accelX);
  jsonCalibration["aY"] = roundFloat(calibratedOffsets.accelY);
  jsonCalibration["aZ"] = roundFloat(calibratedOffsets.accelZ);
  jsonCalibration["gX"] = roundFloat(calibratedOffsets.gyroX);
  jsonCalibration["gY"] = roundFloat(calibratedOffsets.gyroY);
  jsonCalibration["gZ"] = roundFloat(calibratedOffsets.gyroZ);

  // Serialize JSON object to string.
  serializeJson(jsonCalibration, calibrationData);
  return calibrationData;
}

// Prepare a JSON object with magnetometer calibration data points for visualization.
// Function: getMagCalJSON
// Purpose: Prepare JSON object with magnetometer calibration data and complete bin distribution arrays
// Inputs: Logical indicating to send all points (accesses the global magCal object for data)
// Outputs: String containing JSON data with coverage, points, and complete bin distribution arrays
//
// This function creates a comprehensive calibration data payload that includes:
// - Coverage percentage for progress monitoring
// - Coordinate points for 3D visualization
// - Complete elevation bin distribution (all bins, 0 for empty)
// - Complete azimuth bin distribution (all bins, 0 for empty)
// The complete arrays preserve index-to-degree mapping for client-side processing.
String getMagCalJSON(bool b_update_points = false) {
  String calibrationData;
  const char* statusMsg = magCal.getStatusMessage();

  // Create a JSON object with calibration data and bin distribution information.
  JsonDocument jsonCalibration;
  float f_last_coverage = magCal.getCoveragePercent();
  jsonCalibration["c"] = roundFloat(f_last_coverage);

  // Provide audio feedback at every 10% coverage milestone.
  static int16_t i_last_milestone = -1;
  int16_t i_current = (int16_t)f_last_coverage / 10.0f;
  if(i_last_milestone == -1) {
    i_last_milestone = i_current; // Init on first call.
  }
  else if(i_current != i_last_milestone) {
    i_last_milestone = i_current;
    playEffect(S_BEEPS_ALT);
  }

  // Add status message if provided and not blank
  if(statusMsg && statusMsg[0] != '\0') {
    jsonCalibration["s"] = statusMsg;
  }

  // Add the last sample as a separate array for reference.
  JsonArray magValue = jsonCalibration["v"].to<JsonArray>();
  MagData lastSample = magCal.getLastSample();
  magValue.add(roundFloat(lastSample.x));
  magValue.add(roundFloat(lastSample.y));
  magValue.add(roundFloat(lastSample.z));

  // Add complete elevation bin distribution data for vertical coverage analysis.
  // Purpose: Shows sample counts for ALL elevation bins, preserving index-to-degree mapping
  // Array index directly corresponds to elevation bin number for degree calculation
  const uint16_t* elevationCounts;
  uint8_t numElevationBins = magCal.getElevationBinDistribution(elevationCounts);
  JsonArray elevationArray = jsonCalibration["e"].to<JsonArray>();

  // Send ALL elevation bins (including empty ones as 0) to preserve index mapping
  for(uint8_t i = 0; i < numElevationBins; i++) {
    elevationArray.add(elevationCounts[i]); // Include all bins: filled and empty
  }

  // Add complete azimuth bin distribution data for horizontal coverage analysis.
  // Purpose: Shows sample counts for ALL azimuth bins, preserving index-to-degree mapping
  // Array index directly corresponds to azimuth bin number for degree calculation
  const uint16_t* azimuthCounts;
  uint8_t numAzimuthBins = magCal.getAzimuthBinDistribution(azimuthCounts);
  JsonArray azimuthArray = jsonCalibration["a"].to<JsonArray>();

  // Send ALL azimuth bins (including empty ones as 0) to preserve index mapping
  for(uint8_t i = 0; i < numAzimuthBins; i++) {
    azimuthArray.add(azimuthCounts[i]); // Include all bins: filled and empty
  }

  // The points arrays can be large so only update when necessary.
  if(b_update_points) {
    // Arrays of data points for magnetometer calibration visualization.
    const double* xPtr;
    const double* yPtr;
    const double* zPtr;

    // Get the visualization points from the magnetometer calibration object.
    uint16_t numPoints = magCal.getVisPoints(xPtr, yPtr, zPtr);

    // Add points as coordinate arrays [x, y, z] for compact JSON representation.
    JsonArray pointsArray = jsonCalibration["p"].to<JsonArray>();
    for(uint16_t i = 0; i < numPoints; i++) {
      JsonArray point = pointsArray.add<JsonArray>();
      point.add(roundDouble(xPtr[i])); // X coordinate
      point.add(roundDouble(yPtr[i])); // Y coordinate
      point.add(roundDouble(zPtr[i])); // Z coordinate
    }
  }

  // Serialize JSON object to string.
  serializeJson(jsonCalibration, calibrationData);
  return calibrationData;
}

String getTelemetryJSON() {
  // Prepare a JSON object with magnetometer and gyroscope/acceleration data.
  String telemetryData;
  JsonDocument jsonTelemetry;

  // Acceleration in meters/second^2 (m/s^2).
  jsonTelemetry["aX"] = roundFloat(filteredMotionData.accelX);
  jsonTelemetry["aY"] = roundFloat(filteredMotionData.accelY);
  jsonTelemetry["aZ"] = roundFloat(filteredMotionData.accelZ);
  // Gyroscope in degrees/second (deg/s).
  jsonTelemetry["gX"] = roundFloat(filteredMotionData.gyroX);
  jsonTelemetry["gY"] = roundFloat(filteredMotionData.gyroY);
  jsonTelemetry["gZ"] = roundFloat(filteredMotionData.gyroZ);
  // Magnetometer in microteslas (uT).
  jsonTelemetry["mX"] = roundFloat(filteredMotionData.magX);
  jsonTelemetry["mY"] = roundFloat(filteredMotionData.magY);
  jsonTelemetry["mZ"] = roundFloat(filteredMotionData.magZ);
  // Special calculated values (g-force and angular velocity)
  jsonTelemetry["gForce"] = roundFloat(filteredMotionData.gForce);
  jsonTelemetry["angVel"] = roundFloat(filteredMotionData.angVel);
  jsonTelemetry["shaken"] = filteredMotionData.shaken;
  // Spatial data in Euler angles (degrees).
  jsonTelemetry["roll"] = roundFloat(spatialData.roll);
  jsonTelemetry["pitch"] = roundFloat(spatialData.pitch);
  jsonTelemetry["yaw"] = roundFloat(spatialData.yaw);
  // Spatial data in quaternion (w, x, y, z).
  jsonTelemetry["qW"] = roundFloat(spatialData.quaternion[0]);
  jsonTelemetry["qX"] = roundFloat(spatialData.quaternion[1]);
  jsonTelemetry["qY"] = roundFloat(spatialData.quaternion[2]);
  jsonTelemetry["qZ"] = roundFloat(spatialData.quaternion[3]);

  // Serialize JSON object to string.
  serializeJson(jsonTelemetry, telemetryData);
  return telemetryData;
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

void sendGyroCalData() {
  if(b_httpd_started && SENSOR_READ_TARGET == GYRO_CALIBRATION) {
    // Gather the latest countdown timer data, serialize it to a JSON string,
    // and send it to all connected EventSource (SSE) clients as a "gyroCal"
    // event name (using the current ms time as a unique event identifier).
    events.send(getGyroCalJSON().c_str(), "gyroCal", millis());
  }
}

void sendMagCalData(bool b_update_points) {
  if(b_httpd_started && SENSOR_READ_TARGET == MAG_CALIBRATION) {
    // Gather the latest filtered motion data, serialize it to a JSON string,
    // and send it to all connected EventSource (SSE) clients as a "magCal"
    // event name (using the current ms time as a unique event identifier).
    events.send(getMagCalJSON(b_update_points).c_str(), "magCal", millis());
  }
}

void sendTelemetryData() {
  if(b_httpd_started && SENSOR_READ_TARGET == TELEMETRY) {
    // Gather the latest filtered motion data, serialize it to a JSON string,
    // and send it to all connected EventSource (SSE) clients as a "telemetry"
    // event name (using the current ms time as a unique event identifier).
    events.send(getTelemetryJSON().c_str(), "telemetry", millis());
  }
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

void handleGeometry(AsyncWebServerRequest *request) {
  // Used for the model geometry (assets/geometry.stl.gz) from the web server.
  debugln(F("Sending -> STL Geometry"));

  // Calculate file size from the embedded binary data and serve the file to the requesting client.
  size_t i_file_len = embeddedFileSize(_binary_assets_geometry_stl_gz_start, _binary_assets_geometry_stl_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_STL, _binary_assets_geometry_stl_gz_start, i_file_len);
  response->addHeader(HEADER_CACHE_CONTROL, CACHE_NO_CACHE);
  response->addHeader(HEADER_CONTENT_ENCODING, ENCODING_GZIP); // Tell the client this is gzipped content.
  request->send(response);
}

void handleThreeJS(AsyncWebServerRequest *request) {
  // Used for the root page (/three.min.js) from the web server.
  debugln(F("Sending -> Three.js Library"));

  // Calculate file size from the embedded binary data and serve the file to the requesting client.
  size_t i_file_len = embeddedFileSize(_binary_assets_three_min_js_gz_start, _binary_assets_three_min_js_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_JAVASCRIPT, _binary_assets_three_min_js_gz_start, i_file_len);
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

void handleWandSettings(AsyncWebServerRequest *request) {
  // Used for the settings page from the web server.
  debugln(F("Sending -> Wand Settings HTML"));
  size_t i_file_len = embeddedFileSize(_binary_assets_wand_html_gz_start, _binary_assets_wand_html_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_HTML, _binary_assets_wand_html_gz_start, i_file_len);
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

void handleGetWandConfig(AsyncWebServerRequest *request) {
  // Return current wand settings as a stringified JSON object.
  getWandPrefsObject(); // Call common function (also used by Pack/Attenuator)
  AsyncWebServerResponse *response = request->beginResponse(HTTP_STATUS_200, MIME_JSON, getWandConfig());
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

void handleToggleMute(AsyncWebServerRequest *request) {
  debugln(F("Web: Toggle Mute"));

  String s_path = request->url();
  if(s_path.length() > 0) {
    int lastSlash = s_path.lastIndexOf('/');
    if(lastSlash >= 0 && lastSlash < s_path.length() - 1) {
      String segment = s_path.substring(lastSlash + 1);
      if(segment == "mute") {
        toggleMute(2);
        notifyWSClients();
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
      else if(segment == "unmute") {
        toggleMute(1);
        notifyWSClients();
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
  increaseVolume();
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  notifyWSClients();
}

void handleMasterVolumeDown(AsyncWebServerRequest *request) {
  debugln(F("Web: Master Volume Down"));
  decreaseVolume();
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  notifyWSClients();
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
  increaseVolumeEffects();
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  notifyWSClients();
}

void handleEffectsVolumeDown(AsyncWebServerRequest *request) {
  debugln(F("Web: Effects Volume Down"));
  decreaseVolumeEffects();
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  notifyWSClients();
}

void handleMusicVolumeUp(AsyncWebServerRequest *request) {
  debugln(F("Web: Music Volume Up"));
  increaseVolumeMusic();
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  notifyWSClients();
}

void handleMusicVolumeDown(AsyncWebServerRequest *request) {
  debugln(F("Web: Music Volume Down"));
  decreaseVolumeMusic();
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  notifyWSClients();
}

void handleMusicStartStop(AsyncWebServerRequest *request) {
  debugln(F("Web: Music Start/Stop"));
  if(!b_playing_music && !b_music_paused) {
    playMusic();
  }
  else {
    stopMusic();
  }
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  notifyWSClients();
}

void handleMusicPauseResume(AsyncWebServerRequest *request) {
  debugln(F("Web: Music Pause/Resume"));
  if(b_playing_music) {
    if(b_music_paused) {
      resumeMusic();
    }
    else {
      pauseMusic();
    }
  }
  else {
    playMusic();
  }
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  notifyWSClients();
}

void handleNextMusicTrack(AsyncWebServerRequest *request) {
  debugln(F("Web: Next Music Track"));
  musicNextTrack();
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  notifyWSClients();
}

void handlePrevMusicTrack(AsyncWebServerRequest *request) {
  debugln(F("Web: Prev Music Track"));
  musicPrevTrack();
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  notifyWSClients();
}

void handleLoopMusicTrack(AsyncWebServerRequest *request) {
  debugln(F("Web: Toggle Music Track Loop"));

  String s_path = request->url();
  if(s_path.length() > 0) {
    int lastSlash = s_path.lastIndexOf('/');
    if(lastSlash >= 0 && lastSlash < s_path.length() - 1) {
      String segment = s_path.substring(lastSlash + 1);
      if(segment == "one") {
        toggleMusicLoop(2);
        notifyWSClients();
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
      else if(segment == "all") {
        toggleMusicLoop(1);
        notifyWSClients();
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
        toggleMusicShuffle(2);
        notifyWSClients();
        request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
        return;
      }
      else if(segment == "off") {
        toggleMusicShuffle(1);
        notifyWSClients();
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
    debugln("Web: Selected Music Track: " + String(i_music_track));
    playMusic(); // Start playing music.
    request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  }
  else {
    // Tell the user why the requested action failed.
    request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Invalid track number requested")); // 400 Bad Request
  }

  notifyWSClients();
}

void handleSaveWandEEPROM(AsyncWebServerRequest *request) {
  debugln(F("Web: Save Wand EEPROM"));
  executeCommand(P_SAVE_EEPROM_WAND);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleResetWandEEPROM(AsyncWebServerRequest *request) {
  debugln(F("Web: Reset Wand EEPROM"));
  executeCommand(P_RESET_EEPROM_WAND);
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
}

void handleResetSensors(AsyncWebServerRequest *request) {
  // Re-center by resetting all current telemetry data for motion sensors.
  // This allows all motion data to be zeroed out and begin a new average.
  resetAllMotionData(true); // Clear and re-calibrate (quick).
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  notifyWSClients();
}

void handleCalibrateGyroSensor(AsyncWebServerRequest *request) {
  // Turn on calibration mode for the gyroscope sensor.
  beginGyroCalibration(30); // Run calibration for 30 seconds.
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  notifyWSClients();
}

void handleMagCalEnabled(AsyncWebServerRequest *request) {
  // Turn on calibration mode for the magnetometer.
  resetAllMotionData(false); // Clear but don't re-calibrate.
  SENSOR_READ_TARGET = MAG_CALIBRATION; // Enables collection of magnetometer data.
  magCal.beginCalibration(); // Start collection of samples, clears counters.
  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  notifyWSClients();
}

void handleMagCalDisabled(AsyncWebServerRequest *request) {
  // Determine if proper coverage was achieved before calculating and storing data.
  float coverage = magCal.getCoveragePercent();
  if(coverage >= 60.0f) {
    // Compute calibration data for the standard calibration object.
    magCalData = magCal.computeCalibration();

    // Create Preferences object to handle non-volatile storage (NVS).
    Preferences preferences;

    // Save the calibration data (as an object) to preferences.
    if(preferences.begin("device", false)) {
      preferences.putBytes("mag_cal", &magCalData, sizeof(magCalData));
      preferences.end();
    }
  }

  // Turn off calibration mode for the motion sensors.
  SENSOR_READ_TARGET = OFFSETS; // Switch to offsets mode for brief collection.
  resetAllMotionData(true); // Reset and re-calibrate with fresh offsets.

  request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
  notifyWSClients();
}

void handleInfraredSignal(AsyncWebServerRequest *request) {
  String c_signal_type = "";
  JsonDocument jsonResponse;

  if(request->hasParam("type")) {
    // Get the parameter "type" if it exists (will be a String).
    c_signal_type = request->getParam("type")->value();

    // Convert string to IR command type enum
    IR_COMMAND_TYPE commandType;
    if(InfraredManager::stringToCommandType(c_signal_type, commandType)) {
      jsonResponse = irManager->sendCommand(commandType);
      jsonResponse["deviceName"] = wirelessMgr->getLocalNetworkName();
    }
    else {
      // Invalid command type
      jsonResponse["status"] = "failure";
      jsonResponse["error"] = "Invalid command type: " + c_signal_type;
    }
  }
  else {
    // No type parameter provided
    jsonResponse["status"] = "failure";
    jsonResponse["error"] = "Missing 'type' parameter";
  }

  // Serialize and send the response
  String responseString;
  serializeJson(jsonResponse, responseString);
  request->send(HTTP_STATUS_200, MIME_JSON, responseString);
}

void handlePowerLevelSet(AsyncWebServerRequest *request) {
  debugln(F("Web: Power Level Set"));

  String s_path = request->url();
  if(s_path.length() > 0) {
    int lastSlash = s_path.lastIndexOf('/');
    if(lastSlash >= 0 && lastSlash < s_path.length() - 1) {
      String segment = s_path.substring(lastSlash + 1);

      // Check if segment is a valid number (toInt() returns non-zero)
      if(segment.toInt() != 0) {
        uint8_t pLevel = abs(segment.toInt());

        // Validate and constrain to 1-5 range
        if(pLevel >= MIN_POWER_LEVEL && pLevel <= MAX_POWER_LEVEL) {
          // Set power level to the one specified.
          switch(pLevel) {
            case LEVEL_1:
              if(gpstarWand.getPowerLevel() == LEVEL_1) {
                // Tell the user why the requested action failed.
                request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Already in PL1")); // 400 Bad Request
              }
              else if(WAND_ACTION_STATUS != ACTION_FIRING || (WAND_ACTION_STATUS == ACTION_FIRING && gpstarWand.getPowerLevel() != LEVEL_5)) {
                debugln(F("Web: Power Level 1"));
                gpstarWand.setPowerLevel(LEVEL_1);
                updatePackPowerLevel();
                request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
                notifyWSClients();
              }
              else {
                // Tell the user why the requested action failed.
                request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Unable to switch from PL5 while firing")); // 400 Bad Request
              }
            break;
            case LEVEL_2:
              if(gpstarWand.getPowerLevel() == LEVEL_2) {
                // Tell the user why the requested action failed.
                request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Already in PL2")); // 400 Bad Request
              }
              else if(WAND_ACTION_STATUS != ACTION_FIRING || (WAND_ACTION_STATUS == ACTION_FIRING && gpstarWand.getPowerLevel() != LEVEL_5)) {
                debugln(F("Web: Power Level 2"));
                gpstarWand.setPowerLevel(LEVEL_2);
                updatePackPowerLevel();
                request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
                notifyWSClients();
              }
              else {
                // Tell the user why the requested action failed.
                request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Unable to switch from PL5 while firing")); // 400 Bad Request
              }
            break;
            case LEVEL_3:
              if(gpstarWand.getPowerLevel() == LEVEL_3) {
                // Tell the user why the requested action failed.
                request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Already in PL3")); // 400 Bad Request
              }
              else if(WAND_ACTION_STATUS != ACTION_FIRING || (WAND_ACTION_STATUS == ACTION_FIRING && gpstarWand.getPowerLevel() != LEVEL_5)) {
                debugln(F("Web: Power Level 3"));
                gpstarWand.setPowerLevel(LEVEL_3);
                updatePackPowerLevel();
                request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
                notifyWSClients();
              }
              else {
                // Tell the user why the requested action failed.
                request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Unable to switch from PL5 while firing")); // 400 Bad Request
              }
            break;
            case LEVEL_4:
              if(gpstarWand.getPowerLevel() == LEVEL_4) {
                // Tell the user why the requested action failed.
                request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Already in PL4")); // 400 Bad Request
              }
              else if(WAND_ACTION_STATUS != ACTION_FIRING || (WAND_ACTION_STATUS == ACTION_FIRING && gpstarWand.getPowerLevel() != LEVEL_5)) {
                debugln(F("Web: Power Level 4"));
                gpstarWand.setPowerLevel(LEVEL_4);
                updatePackPowerLevel();
                request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
                notifyWSClients();
              }
              else {
                // Tell the user why the requested action failed.
                request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Unable to switch from PL5 while firing")); // 400 Bad Request
              }
            break;
            case LEVEL_5:
              if(gpstarWand.getPowerLevel() == LEVEL_5) {
                // Tell the user why the requested action failed.
                request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Already in PL5")); // 400 Bad Request
              }
              else if(WAND_ACTION_STATUS != ACTION_FIRING) {
                debugln(F("Web: Power Level 5"));
                gpstarWand.setPowerLevel(LEVEL_5);
                updatePackPowerLevel();
                request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus());
                notifyWSClients();
              }
              else {
                // Tell the user why the requested action failed.
                request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Unable to switch to PL5 while firing")); // 400 Bad Request
              }
            break;
            default:
              // We can't be here, so do nothing.
            break;
          }

          return;
        }
      }
    }
  }

  debugln(F("Invalid Power Level"));
  request->send(HTTP_STATUS_400, MIME_JSON, returnJsonStatus("Invalid Power Level (1-5)"));
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

    // Orientation Option - Check that it meets the expected type before attempting to change the value.
    bool b_orientation_changed = false;
    INSTALL_ORIENTATIONS PREVIOUS_ORIENTATION = INSTALL_ORIENTATION;
    if(jsonBody["orientation"].is<unsigned char>()) {
      switch(jsonBody["orientation"].as<unsigned char>()) {
        case 1:
          INSTALL_ORIENTATION = COMPONENTS_UP_USB_FRONT;
        break;
        case 2:
          INSTALL_ORIENTATION = COMPONENTS_UP_USB_REAR;
        break;
        case 3:
          INSTALL_ORIENTATION = COMPONENTS_DOWN_USB_FRONT;
        break;
        case 4:
          INSTALL_ORIENTATION = COMPONENTS_DOWN_USB_REAR;
        break;
        case 5:
          INSTALL_ORIENTATION = COMPONENTS_LEFT_USB_FRONT;
        break;
        case 6:
          INSTALL_ORIENTATION = COMPONENTS_LEFT_USB_REAR;
        break;
        case 7:
          INSTALL_ORIENTATION = COMPONENTS_RIGHT_USB_FRONT;
        break;
        case 8:
          INSTALL_ORIENTATION = COMPONENTS_RIGHT_USB_REAR;
        break;
        case 9:
          INSTALL_ORIENTATION = COMPONENTS_FACTORY_DEFAULT;
        break;
        default:
          // Do not change orientation if an invalid value was provided.
        break;
      }

      if(INSTALL_ORIENTATION != PREVIOUS_ORIENTATION) {
        // Reset the magnetic calibration values to defaults on orientation change.
        magCalData = magCal.getDefaultCalibration();
        b_orientation_changed = true;
      } else {
        // Set the current magnetic calibration values when orientation is unchanged.
        magCalData.mag_hardiron[0] = jsonBody["hardIron1"].as<float>();
        magCalData.mag_hardiron[1] = jsonBody["hardIron2"].as<float>();
        magCalData.mag_hardiron[2] = jsonBody["hardIron3"].as<float>();
        magCalData.mag_softiron[0] = jsonBody["softIron1"].as<float>();
        magCalData.mag_softiron[1] = jsonBody["softIron2"].as<float>();
        magCalData.mag_softiron[2] = jsonBody["softIron3"].as<float>();
        magCalData.mag_softiron[3] = jsonBody["softIron4"].as<float>();
        magCalData.mag_softiron[4] = jsonBody["softIron5"].as<float>();
        magCalData.mag_softiron[5] = jsonBody["softIron6"].as<float>();
        magCalData.mag_softiron[6] = jsonBody["softIron7"].as<float>();
        magCalData.mag_softiron[7] = jsonBody["softIron8"].as<float>();
        magCalData.mag_softiron[8] = jsonBody["softIron9"].as<float>();
        magCalData.mag_field = jsonBody["magField"].as<float>();
      }
    }

    // Check for Standalone Mode mode switch.
    bool b_standalone_toggled = jsonBody["useStandalone"].as<bool>() | false;
    bool b_restart_required = (!b_standalone_toggled && b_wand_standalone);
    if(b_standalone_toggled && !b_wand_standalone) {
      // Switch to Standalone Mode.
      toggleStandaloneMode(true);
    }

    // Set our IR while firing flag appropriately.
    b_ir_while_firing = jsonBody["irWhileFiring"].as<bool>() | false;

    // Get the track listing from the text field.
    String songList = jsonBody["songList"].as<String>();
    bool b_list_err = false;

    // Create Preferences object to handle non-volatile storage (NVS).
    Preferences preferences;

    // Accesses namespace in read/write mode.
    if(preferences.begin("device", false)) {
      // Store the standalone mode setting to preferences.
      preferences.putBool("standalone", b_standalone_toggled);

      // Store the IR while firing setting to preferences.
      preferences.putBool("ir_while_firing", b_ir_while_firing);

      // Store the orientation value to preferences if changed.
      if(INSTALL_ORIENTATION != PREVIOUS_ORIENTATION) {
        preferences.putUChar("orientation", INSTALL_ORIENTATION);
      }

      // Store the magnetic calibration struct (object) to preferences.
      preferences.putBytes("mag_cal", &magCalData, sizeof(magCalData));

      if(b_orientation_changed) {
        resetMotionOffsets(calibratedOffsets); // Clear previous offsets set/collected.
        resetAllMotionData(true); // Reset and re-calibrate with fresh, quick offsets.

        // Reset the accelerometer offsets.
        accelOffsets.x = 0;
        accelOffsets.y = 0;
        accelOffsets.z = 0;

        // Reset the gyroscope offsets.
        gyroOffsets.x = 0;
        gyroOffsets.y = 0;
        gyroOffsets.z = 0;

        // Save the new empty offsets to preferences.
        preferences.putBytes("accel_cal", &accelOffsets, sizeof(accelOffsets));
        preferences.putBytes("gyro_cal", &gyroOffsets, sizeof(gyroOffsets));
      }

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
    } else if(b_restart_required) {
      request->send(HTTP_STATUS_201, MIME_JSON, returnJsonStatus("Settings updated, restart required to disable Standalone Mode."));
    }
    else {
      request->send(HTTP_STATUS_200, MIME_JSON, returnJsonStatus("Settings updated."));
    }
  }
  catch (...) {
    request->send(HTTP_STATUS_500, MIME_JSON, returnJsonStatus("An error was encountered while saving settings.")); // 500 Server Error
  }
}); // handleSaveDeviceConfig

// Handles the JSON body for the wand settings save request.
AsyncCallbackJsonWebHandler *handleSaveWandConfig = new AsyncCallbackJsonWebHandler("/config/wand/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  JsonDocument jsonBody;
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  } else {
    debugln(F("Body was not a JSON object"));
    request->send(HTTP_STATUS_500, MIME_JSON, returnJsonStatus("Body was not a JSON object.")); // 500 Server Error
    return;
  }

  if(WAND_STATUS == MODE_OFF) {
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
      (jsonBody["stasisStream"].as<bool>() || gpstarWand.inStreamMode(STASIS)) ? (wandConfig.streamFlags |= FLAG_STASIS) : (wandConfig.streamFlags &= ~FLAG_STASIS);
      (jsonBody["slimeStream"].as<bool>() || gpstarWand.inStreamMode(SLIME)) ? (wandConfig.streamFlags |= FLAG_SLIME) : (wandConfig.streamFlags &= ~FLAG_SLIME);
      (jsonBody["mesonStream"].as<bool>() || gpstarWand.inStreamMode(MESON)) ? (wandConfig.streamFlags |= FLAG_MESON) : (wandConfig.streamFlags &= ~FLAG_MESON);
      (jsonBody["spectralStream"].as<bool>() || gpstarWand.inStreamMode(SPECTRAL)) ? (wandConfig.streamFlags |= FLAG_SPECTRAL) : (wandConfig.streamFlags &= ~FLAG_SPECTRAL);
      (jsonBody["spectralCustomStream"].as<bool>() || gpstarWand.inStreamMode(SPECTRAL_CUSTOM)) ? (wandConfig.streamFlags |= FLAG_SPECTRAL_CUSTOM) : (wandConfig.streamFlags &= ~FLAG_SPECTRAL_CUSTOM);
      (jsonBody["halloweenStream"].as<bool>() || gpstarWand.inStreamMode(HOLIDAY_HALLOWEEN)) ? (wandConfig.streamFlags |= FLAG_HOLIDAY_HALLOWEEN) : (wandConfig.streamFlags &= ~FLAG_HOLIDAY_HALLOWEEN);
      (jsonBody["christmasStream"].as<bool>() || gpstarWand.inStreamMode(HOLIDAY_CHRISTMAS)) ? (wandConfig.streamFlags |= FLAG_HOLIDAY_CHRISTMAS) : (wandConfig.streamFlags &= ~FLAG_HOLIDAY_CHRISTMAS);

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

      handleWandPrefsUpdate(); // Have the wand pass the new settings.
      request->send(HTTP_STATUS_201, MIME_JSON, returnJsonStatus("Settings updated, please test before saving to EEPROM."));
    }
    catch (...) {
      request->send(HTTP_STATUS_500, MIME_JSON, returnJsonStatus("An error was encountered while saving settings.")); // 500 Server Error
    }
  }
  else {
    // Tell the user why the requested action failed.
    request->send(HTTP_STATUS_409, MIME_JSON, returnJsonStatus("Wand is running, save action cancelled")); // 409 Conflict
  }
}); // handleSaveWandConfig

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
