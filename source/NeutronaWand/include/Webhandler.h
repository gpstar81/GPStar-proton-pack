/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

// Web page files (defines all text as char[] variable)
#include "web/CommonJS.h" // COMMONJS_page
#include "web/Index.h" // INDEX_page
#include "web/IndexJS.h" // INDEXJS_page
#include "web/Device.h" // DEVICE_page
#include "web/ExtWiFi.h" // NETWORK_page
#include "web/Password.h" // PASSWORD_page
#include "web/WandSettings.h" // WAND_SETTINGS_page
#include "web/Style.h" // STYLE_page
#include "web/Icon.h" // FAVICON_ico, FAVICON_svg
#include "web/Geometry.h" // GEOMETRY_json
#include "web/ThreeJS.h" // THREEJS_page

// Forward function declarations.
void setupRouting();
void getSpecialPreferences();

// Rounds a float to 2 decimal places.
float roundFloat(float value) {
  return roundf(value * 100.0f) / 100.0f;
}

/*
 * Text Helper Functions - Converts ENUM values to user-friendly text
 */

String getMode() {
  switch(SYSTEM_MODE) {
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

String getTheme() {
  switch(SYSTEM_YEAR) {
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

String getRedSwitch() {
  if(SYSTEM_MODE == MODE_ORIGINAL) {
    // Switch state only matters for mode "Original".
    switch(RED_SWITCH_MODE) {
      case SWITCH_ON:
        return "Ready";
      break;
      case SWITCH_OFF:
        return "Standby";
      break;
      default:
        return "Unknown";
      break;
    }
  }
  else {
    // Otherwise, just "Ready".
    return "Ready";
  }
}

String getSafety() {
  switch(BARREL_STATE) {
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

String getWandMode() {
  switch(STREAM_MODE) {
    case PROTON:
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
    default:
      return "Unknown";
    break;
  }
}

String getPower() {
  switch(POWER_LEVEL) {
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
      return "5";
    break;
    default:
      return "-";
    break;
  }
}

/*
 * Web Handler Functions - Performs actions or returns data for web UI
 */
JsonDocument jsonBody; // Used for processing JSON body/payload data.
JsonDocument jsonSuccess; // Used for sending JSON status as success.
JsonDocument jsonTelemetry; // Used for sending JSON telemetry data.
String status; // Holder for simple "status: success" response.

void onWebSocketEventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch(type) {
    case WS_EVT_CONNECT:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        debugf("WebSocket[%s][%lu] Connect\n", server->url(), client->id());
      #endif
      i_ws_client_count++;
    break;

    case WS_EVT_DISCONNECT:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        debugf("WebSocket[%s][C:%lu] Disconnect\n", server->url(), client->id());
      #endif
      if(i_ws_client_count > 0) {
        i_ws_client_count--;
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
  debug(F("OTA update started"));
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - i_progress_millis > 1000) {
    i_progress_millis = millis();
    debugf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    debug(F("OTA update finished successfully!"));
  } else {
    debug(F("There was an error during OTA update!"));
  }
}

void startWebServer() {
  // Configures URI routing with function handlers.
  setupRouting();

  // Get preferences for the web UI.
  getSpecialPreferences();

  // Prepare a standard "success" message for responses.
  jsonSuccess.clear();
  jsonSuccess["status"] = "success";
  serializeJson(jsonSuccess, status);

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
  b_ws_started = true;

  #if defined(DEBUG_SEND_TO_CONSOLE)
    debugln(F("Async HTTP Server Started"));
  #endif
}

void handleCommonJS(AsyncWebServerRequest *request) {
  // Used for the root page (/) from the web server.
  debug("Sending -> Common JavaScript");
  AsyncWebServerResponse *response = request->beginResponse(200, "application/javascript; charset=UTF-8", (const uint8_t*)COMMONJS_page, strlen(COMMONJS_page));
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  request->send(response); // Serve page content.
}

void handleRoot(AsyncWebServerRequest *request) {
  // Used for the root page (/) from the web server.
  debug("Sending -> Index HTML");
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", (const uint8_t*)INDEX_page, strlen(INDEX_page));
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  request->send(response); // Serve page content.
}

void handleRootJS(AsyncWebServerRequest *request) {
  // Used for the root page (/) from the web server.
  debug("Sending -> Index JavaScript");
  AsyncWebServerResponse *response = request->beginResponse(200, "application/javascript; charset=UTF-8", (const uint8_t*)INDEXJS_page, strlen(INDEXJS_page));
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  request->send(response); // Serve page content.
}

void handleNetwork(AsyncWebServerRequest *request) {
  // Used for the network page from the web server.
  debug("Sending -> Network HTML");
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", (const uint8_t*)NETWORK_page, strlen(NETWORK_page));
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  request->send(response); // Serve page content.
}

void handlePassword(AsyncWebServerRequest *request) {
  // Used for the password page from the web server.
  debug("Sending -> Password HTML");
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", (const uint8_t*)PASSWORD_page, strlen(PASSWORD_page));
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  request->send(response); // Serve page content.
}

void handleDeviceSettings(AsyncWebServerRequest *request) {
  // Used for the device page from the web server.
  debug("Sending -> Device Settings HTML");
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", (const uint8_t*)DEVICE_page, strlen(DEVICE_page));
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  request->send(response); // Serve page content.
}

void handleWandSettings(AsyncWebServerRequest *request) {
  // Tell the pack that we'll need the latest wand EEPROM values.
  //executeCommand(A_REQUEST_PREFERENCES_WAND);

  // Used for the settings page from the web server.
  debug("Sending -> Wand Settings HTML");
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", (const uint8_t*)WAND_SETTINGS_page, strlen(WAND_SETTINGS_page));
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  request->send(response); // Serve page content.
}

void handleStylesheet(AsyncWebServerRequest *request) {
  // Used for the common stylesheet of the web server.
  debug("Sending -> Main StyleSheet");
  AsyncWebServerResponse *response = request->beginResponse(200, "text/css", (const uint8_t*)STYLE_page, strlen(STYLE_page));
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  request->send(response); // Serve page content.
}

void handleFavIco(AsyncWebServerRequest *request) {
  // Used for the favicon of the web server.
  debug("Sending -> Favicon");
  AsyncWebServerResponse *response = request->beginResponse(200, "image/x-icon", FAVICON_ico, sizeof(FAVICON_ico));
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  response->addHeader("Content-Encoding", "gzip");
  request->send(response); // Serve gzipped .ico file.
}

void handleFavSvg(AsyncWebServerRequest *request) {
  // Used for the favicon of the web server.
  debug("Sending -> Favicon");
  AsyncWebServerResponse *response = request->beginResponse(200, "image/svg+xml", FAVICON_svg, sizeof(FAVICON_svg));
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  response->addHeader("Content-Encoding", "gzip");
  request->send(response); // Serve gzipped .svg file.
}

void handleGeometry(AsyncWebServerRequest *request) {
  // Used for the model geometry (/geometry.json) from the web server.
  debug("Sending -> STL Geometry");
  AsyncWebServerResponse *response = request->beginResponse(200, "application/json; charset=UTF-8", GEOMETRY_json, sizeof(GEOMETRY_json));
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  response->addHeader("Content-Encoding", "gzip");
  request->send(response); // Serve gzipped JSON content.
}

void handleThreeJS(AsyncWebServerRequest *request) {
  // Used for the root page (/three.js) from the web server.
  debug("Sending -> Three.js Library");
  AsyncWebServerResponse *response = request->beginResponse(200, "application/javascript; charset=UTF-8", THREEJS_page, sizeof(THREEJS_page));
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  response->addHeader("Content-Encoding", "gzip");
  request->send(response); // Serve page content.
}

String getDeviceConfig() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonBody.clear();

  // Provide current values for the device.
  if(s_track_listing != "" && s_track_listing != "null") {
    jsonBody["songList"] = s_track_listing;
  }
  else {
    jsonBody["songList"] = "";
  }
  jsonBody["buildDate"] = build_date;
  jsonBody["wifiName"] = ap_ssid;
  jsonBody["wifiNameExt"] = wifi_ssid;
  jsonBody["extAddr"] = wifi_address;
  jsonBody["extMask"] = wifi_subnet;
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

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getWandConfig() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonBody.clear();

  // Provide a flag to indicate prefs are directly available.
  jsonBody["prefsAvailable"] = true;

  // Return current powered state for pack and wand.
  jsonBody["packPowered"] = (b_pack_on ? true : false);
  jsonBody["wandPowered"] = (WAND_STATUS == MODE_ON);
  jsonBody["wandConnected"] = (WAND_CONN_STATE == PACK_CONNECTED);

  // Neutrona Wand LED Options
  jsonBody["ledWandCount"] = wandConfig.ledWandCount; // [0=5 (Stock), 1=48 (Frutto), 2=50 (GPStar), 3=2 (Tip)]
  jsonBody["ledWandHue"] = wandConfig.ledWandHue; // Spectral custom colour/hue 2-254
  jsonBody["ledWandSat"] = wandConfig.ledWandSat; // Spectral custom saturation 2-254
  jsonBody["spectralModesEnabled"] = wandConfig.spectralModesEnabled; // true|false

  // Neutrona Wand Runtime Options
  jsonBody["overheatEnabled"] = wandConfig.overheatEnabled; // true|false
  jsonBody["defaultFiringMode"] = wandConfig.defaultFiringMode; // [1=VG,2=CTS,3=CTS_MIX]
  jsonBody["wandVibration"] = wandConfig.wandVibration; // [1=ALWAYS,2=FIRING,3=NEVER,4=TOGGLE]
  jsonBody["wandSoundsToPack"] = wandConfig.wandSoundsToPack; // true|false
  jsonBody["quickVenting"] = wandConfig.quickVenting; // true|false (Super-Hero Mode Only)
  jsonBody["rgbVentEnabled"] = wandConfig.rgbVentEnabled; // true|false
  jsonBody["autoVentLight"] = wandConfig.autoVentLight; // true|false
  jsonBody["wandBeepLoop"] = wandConfig.wandBeepLoop; // true|false (Afterlife/Frozen Empire Only)
  jsonBody["wandBootError"] = wandConfig.wandBootError; // true|false (Super-Hero Mode Only)
  jsonBody["defaultYearModeWand"] = wandConfig.defaultYearModeWand; // [1=TOGGLE,2=1984,3=1989,4=2021,5=2024]
  jsonBody["defaultYearModeCTS"] = wandConfig.defaultYearModeCTS; // [1=TOGGLE,2=1984,4=2021]
  jsonBody["numBargraphSegments"] = wandConfig.numBargraphSegments; // [28=28-segment,30=30-segment]
  jsonBody["invertWandBargraph"] = wandConfig.invertWandBargraph; // true|false
  jsonBody["bargraphOverheatBlink"] = wandConfig.bargraphOverheatBlink; // true|false
  jsonBody["bargraphIdleAnimation"] = wandConfig.bargraphIdleAnimation; // [1=SYSTEM,2=SH,3=MO]
  jsonBody["bargraphFireAnimation"] = wandConfig.bargraphFireAnimation; // [1=SYSTEM,2=SH,3=MO]

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getEquipmentStatus() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipStatus;
  jsonBody.clear();

  uint16_t i_music_track_min = 0;
  uint16_t i_music_track_max = 0;

  if(i_music_track_count > 0) {
    i_music_track_min = i_music_track_start; // First music track possible (eg. 500)
    i_music_track_max = i_music_track_start + i_music_track_count - 1; // 500 + N - 1 to be inclusive of the offset value.
  }

#ifdef ESP32
  jsonBody["pcb"] = "ESP32";
#else
  jsonBody["pcb"] = "ATMega";
#endif
  jsonBody["mode"] = getMode();
  jsonBody["modeID"] = (SYSTEM_MODE == MODE_SUPER_HERO) ? 1 : 0;
  jsonBody["theme"] = getTheme();
  jsonBody["themeID"] = SYSTEM_YEAR;
  jsonBody["switch"] = getRedSwitch();
  jsonBody["power"] = getPower();
  jsonBody["safety"] = getSafety();
  jsonBody["wandPower"] = (WAND_STATUS == MODE_ON ? "Powered" : "Idle");
  jsonBody["wandMode"] = getWandMode();
  jsonBody["firing"] = (b_firing ? "Firing" : "Idle");
  jsonBody["musicPlaying"] = b_playing_music;
  jsonBody["musicPaused"] = b_music_paused;
  jsonBody["musicCurrent"] = i_current_music_track;
  jsonBody["musicStart"] = i_music_track_min;
  jsonBody["musicEnd"] = i_music_track_max;
  jsonBody["volMaster"] = i_volume_master_percentage;
  jsonBody["volEffects"] = i_volume_effects_percentage;
  jsonBody["volMusic"] = i_volume_music_percentage;
  jsonBody["apClients"] = i_ap_client_count;
  jsonBody["wsClients"] = i_ws_client_count;

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipStatus);
  return equipStatus;
}

String getWifiSettings() {
  // Prepare a JSON object with information stored in preferences (or a blank default).
  String wifiNetwork;
  jsonBody.clear();

  // Accesses namespace in read-only mode.
  if(preferences.begin("network", true)) {
    jsonBody["enabled"] = preferences.getBool("enabled", false);
    jsonBody["network"] = preferences.getString("ssid");
    jsonBody["password"] = preferences.getString("password");

    jsonBody["address"] = preferences.getString("address");
    if(jsonBody["address"].as<String>() == "") {
      jsonBody["address"] = wifi_address;
    }

    jsonBody["subnet"] = preferences.getString("subnet");
    if(jsonBody["subnet"].as<String>() == "") {
      jsonBody["subnet"] = wifi_subnet;
    }

    jsonBody["gateway"] = preferences.getString("gateway");
    if(jsonBody["gateway"].as<String>() == "") {
      jsonBody["gateway"] = wifi_gateway;
    }

    preferences.end();
  }
  else {
    if(preferences.begin("network", false)) {
      preferences.putBool("enabled", false);
      preferences.putString("ssid", "");
      preferences.putString("password", "");
      preferences.putString("address", "");
      preferences.putString("subnet", "");
      preferences.putString("gateway", "");
      preferences.end();
    }
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, wifiNetwork);
  return wifiNetwork;
}

String getTelemetry() {
  // Prepare a JSON object with magnetometer and gyroscope/acceleration data.
  String telemetryData;
  jsonTelemetry.clear();

  // Magnetometer in microteslas (uT) converted to a heading in degrees.
  jsonTelemetry["heading"] = roundFloat(filteredMotionData.heading);
  // Acceleration in meters/second^2 (m/s^2)
  jsonTelemetry["accelX"] = roundFloat(filteredMotionData.accelX);
  jsonTelemetry["accelY"] = roundFloat(filteredMotionData.accelY);
  jsonTelemetry["accelZ"] = roundFloat(filteredMotionData.accelZ);
  // Gyroscope in radians/second (rads/s)
  jsonTelemetry["gyroX"] = roundFloat(filteredMotionData.gyroX);
  jsonTelemetry["gyroY"] = roundFloat(filteredMotionData.gyroY);
  jsonTelemetry["gyroZ"] = roundFloat(filteredMotionData.gyroZ);
  // Spatial data in Euler angles (degrees).
  jsonTelemetry["pitch"] = roundFloat(spatialData.pitch);
  jsonTelemetry["yaw"] = roundFloat(spatialData.yaw);
  jsonTelemetry["roll"] = roundFloat(spatialData.roll);
  jsonTelemetry["qw"] = roundFloat(spatialData.quaternion[0]);
  jsonTelemetry["qx"] = roundFloat(spatialData.quaternion[1]);
  jsonTelemetry["qy"] = roundFloat(spatialData.quaternion[2]);
  jsonTelemetry["qz"] = roundFloat(spatialData.quaternion[3]);

  // Serialize JSON object to string.
  serializeJson(jsonTelemetry, telemetryData);
  return telemetryData;
}

void handleGetDeviceConfig(AsyncWebServerRequest *request) {
  // Return current device settings as a stringified JSON object.
  request->send(200, "application/json", getDeviceConfig());
}

void handleGetWandConfig(AsyncWebServerRequest *request) {
  // Return current wand settings as a stringified JSON object.
  request->send(200, "application/json", getWandConfig());
}

void handleGetStatus(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getEquipmentStatus());
}

void handleGetWifi(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getWifiSettings());
}

void handleResetSensors(AsyncWebServerRequest *request) {
  // Re-center by resetting all current telemetry data for motion sensors.
  // This allows all motion data to be zeroed out and begin a new average.
  resetAllMotionData();
  request->send(200, "application/json", status);
}

void handleCalibrateSensorsEnabled(AsyncWebServerRequest *request) {
  // Turn on calibration mode for the motion sensors.
  SENSOR_READ_TARGET = CALIBRATION;
  request->send(200, "application/json", status);
}

void handleCalibrateSensorsDisabled(AsyncWebServerRequest *request) {
  // Turn off calibration mode for the motion sensors.
  SENSOR_READ_TARGET = OFFSETS;
  resetAllMotionData();
  request->send(200, "application/json", status);
}

void handleInfraredSignal(AsyncWebServerRequest *request) {
  String c_signal_type = "";

  if(request->hasParam("type")) {
    // Get the parameter "track" if it exists (will be a String).
    c_signal_type = request->getParam("type")->value();
    sendInfraredCommand(c_signal_type);
  }

  request->send(200, "application/json", status);
}

void handleRestart(AsyncWebServerRequest *request) {
  // Performs a restart of the device.
  request->send(204, "application/json", status);
  delay(1000);
  ESP.restart();
}

void handleToggleMute(AsyncWebServerRequest *request) {
  debug("Web: Toggle Mute");
  //executeCommand(A_TOGGLE_MUTE);
  request->send(200, "application/json", status);
}

void handleMasterVolumeUp(AsyncWebServerRequest *request) {
  debug("Web: Master Volume Up");
  //executeCommand(A_VOLUME_INCREASE);
  request->send(200, "application/json", status);
}

void handleMasterVolumeDown(AsyncWebServerRequest *request) {
  debug("Web: Master Volume Down");
  //executeCommand(A_VOLUME_DECREASE);
  request->send(200, "application/json", status);
}

void handleEffectsVolumeUp(AsyncWebServerRequest *request) {
  debug("Web: Effects Volume Up");
  executeCommand(P_VOLUME_SOUND_EFFECTS_INCREASE);
  request->send(200, "application/json", status);
}

void handleEffectsVolumeDown(AsyncWebServerRequest *request) {
  debug("Web: Effects Volume Down");
  executeCommand(P_VOLUME_SOUND_EFFECTS_DECREASE);
  request->send(200, "application/json", status);
}

void handleMusicVolumeUp(AsyncWebServerRequest *request) {
  debug("Web: Music Volume Up");
  //executeCommand(A_VOLUME_MUSIC_INCREASE);
  request->send(200, "application/json", status);
}

void handleMusicVolumeDown(AsyncWebServerRequest *request) {
  debug("Web: Music Volume Down");
  //executeCommand(A_VOLUME_MUSIC_DECREASE);
  request->send(200, "application/json", status);
}

void handleMusicStartStop(AsyncWebServerRequest *request) {
  debug("Web: Music Start/Stop");
  //executeCommand(A_MUSIC_START_STOP);
  request->send(200, "application/json", status);
}

void handleMusicPauseResume(AsyncWebServerRequest *request) {
  debug("Web: Music Pause/Resume");
  //executeCommand(A_MUSIC_PAUSE_RESUME);
  request->send(200, "application/json", status);
}

void handleNextMusicTrack(AsyncWebServerRequest *request) {
  debug("Web: Next Music Track");
  //executeCommand(A_MUSIC_NEXT_TRACK);
  request->send(200, "application/json", status);
}

void handlePrevMusicTrack(AsyncWebServerRequest *request) {
  debug("Web: Prev Music Track");
  //executeCommand(A_MUSIC_PREV_TRACK);
  request->send(200, "application/json", status);
}

void handleLoopMusicTrack(AsyncWebServerRequest *request) {
  debug("Web: Toggle Music Track Loop");
  //executeCommand(A_MUSIC_TRACK_LOOP_TOGGLE);
  request->send(200, "application/json", status);
}

void handleSelectMusicTrack(AsyncWebServerRequest *request) {
  String c_music_track = "";

  if(request->hasParam("track")) {
    // Get the parameter "track" if it exists (will be a String).
    c_music_track = request->getParam("track")->value();
  }

  if(c_music_track.toInt() != 0 && c_music_track.toInt() >= i_music_track_start) {
    uint16_t i_music_track = c_music_track.toInt();
    debug("Web: Selected Music Track: " + String(i_music_track));
    //executeCommand(A_MUSIC_PLAY_TRACK, i_music_track); // Inform the pack of the new track.
    request->send(200, "application/json", status);
  }
  else {
    // Tell the user why the requested action failed.
    String result;
    jsonBody.clear();
    jsonBody["status"] = "Invalid track number requested";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
}

void handleSaveWandEEPROM(AsyncWebServerRequest *request) {
  debug("Web: Save Wand EEPROM");
  executeCommand(P_SAVE_EEPROM_WAND);
  request->send(200, "application/json", status);
}

// Handles the JSON body for the Attenuator settings save request.
AsyncCallbackJsonWebHandler *handleSaveDeviceConfig = new AsyncCallbackJsonWebHandler("/config/device/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  jsonBody.clear();
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    debug("Body was not a JSON object");
  }

  String result;
  try {
    // First check if a new private WiFi network name has been chosen.
    String newSSID = jsonBody["wifiName"].as<String>();
    newSSID = sanitizeSSID(newSSID); // Jacques, clean him!
    bool b_ssid_changed = false;

    // Update the private network name ONLY if the new value differs from the current SSID.
    if(newSSID != ap_ssid){
      if(newSSID.length() >= 8 && newSSID.length() <= 32) {
        // Accesses namespace in read/write mode.
        if(preferences.begin("credentials", false)) {
          #if defined(DEBUG_SEND_TO_CONSOLE)
            debug(F("New Private SSID: "));
            debugln(newSSID);
          #endif
          preferences.putString("ssid", newSSID); // Store SSID in case this was altered.
          preferences.end();
        }

        b_ssid_changed = true; // This will cause a reboot of the device after saving.
      }
      else {
        // Immediately return an error if the network name was invalid.
        jsonBody.clear();
        jsonBody["status"] = "Error: Network name must be between 8 and 32 characters in length.";
        serializeJson(jsonBody, result); // Serialize to string.
        request->send(200, "application/json", result);
      }
    }

    // Set the current magnetic calibration values.
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

    // Get the track listing from the text field.
    String songList = jsonBody["songList"].as<String>();
    bool b_list_err = false;

    // Accesses namespace in read/write mode.
    if(preferences.begin("device", false)) {
      // Store the magnetic calibration struct (object) to preferences.
      preferences.putBytes("mag_cal", &magCalData, sizeof(magCalData));

      // Store the song list to preferences.
      if(songList.length() <= 2000) {
        if(songList == "null") {
          songList = "";
        }

        // Update song lists if contents are under 2000 bytes.
        #if defined(DEBUG_SEND_TO_CONSOLE)
          debug(F("Song List Bytes: "));
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

    if(b_list_err){
      jsonBody.clear();
      jsonBody["status"] = "Settings updated, but song list exceeds the 2,000 bytes maximum and was not saved.";
      serializeJson(jsonBody, result); // Serialize to string.
      request->send(200, "application/json", result);
    }
    else if(b_ssid_changed){
      jsonBody.clear();
      jsonBody["status"] = "Settings updated, restart required. Please use the new network name to connect to your device.";
      serializeJson(jsonBody, result); // Serialize to string.
      request->send(201, "application/json", result);
    }
    else {
      jsonBody.clear();
      jsonBody["status"] = "Settings updated.";
      serializeJson(jsonBody, result); // Serialize to string.
      request->send(200, "application/json", result);
    }
  }
  catch (...) {
    jsonBody.clear();
    jsonBody["status"] = "An error was encountered while saving settings.";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
});

// Handles the JSON body for the wand settings save request.
AsyncCallbackJsonWebHandler *handleSaveWandConfig = new AsyncCallbackJsonWebHandler("/config/wand/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  jsonBody.clear();
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    debug("Body was not a JSON object");
  }

  String result;
  if(WAND_STATUS == MODE_OFF) {
    try {
      wandConfig.ledWandCount = jsonBody["ledWandCount"].as<uint8_t>();
      wandConfig.ledWandHue = jsonBody["ledWandHue"].as<uint8_t>();
      wandConfig.ledWandSat = jsonBody["ledWandSat"].as<uint8_t>();
      wandConfig.rgbVentEnabled = jsonBody["rgbVentEnabled"].as<uint8_t>();
      wandConfig.spectralModesEnabled = jsonBody["spectralModesEnabled"].as<uint8_t>();
      wandConfig.overheatEnabled = jsonBody["overheatEnabled"].as<uint8_t>();
      wandConfig.defaultFiringMode = jsonBody["defaultFiringMode"].as<uint8_t>();
      wandConfig.wandVibration = jsonBody["wandVibration"].as<uint8_t>();
      wandConfig.wandSoundsToPack = jsonBody["wandSoundsToPack"].as<uint8_t>();
      wandConfig.quickVenting = jsonBody["quickVenting"].as<uint8_t>();
      wandConfig.autoVentLight = jsonBody["autoVentLight"].as<uint8_t>();
      wandConfig.wandBeepLoop = jsonBody["wandBeepLoop"].as<uint8_t>();
      wandConfig.wandBootError = jsonBody["wandBootError"].as<uint8_t>();
      wandConfig.defaultYearModeWand = jsonBody["defaultYearModeWand"].as<uint8_t>();
      wandConfig.defaultYearModeCTS = jsonBody["defaultYearModeCTS"].as<uint8_t>();
      wandConfig.numBargraphSegments = jsonBody["numBargraphSegments"].as<uint8_t>();
      wandConfig.invertWandBargraph = jsonBody["invertWandBargraph"].as<uint8_t>();
      wandConfig.bargraphOverheatBlink = jsonBody["bargraphOverheatBlink"].as<uint8_t>();
      wandConfig.bargraphIdleAnimation = jsonBody["bargraphIdleAnimation"].as<uint8_t>();
      wandConfig.bargraphFireAnimation = jsonBody["bargraphFireAnimation"].as<uint8_t>();

      jsonBody.clear();
      jsonBody["status"] = "Settings updated, please test before saving to EEPROM.";
      serializeJson(jsonBody, result); // Serialize to string.
      handleWandPrefsUpdate(); // Have the wand the new settings.
      request->send(200, "application/json", result);
    }
    catch (...) {
      jsonBody.clear();
      jsonBody["status"] = "An error was encountered while saving settings.";
      serializeJson(jsonBody, result); // Serialize to string.
      request->send(200, "application/json", result);
    }
  }
  else {
    // Tell the user why the requested action failed.
    jsonBody.clear();
    jsonBody["status"] = "Pack and/or Wand are running, save action cancelled";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
});

// Handles the JSON body for the password change request.
AsyncCallbackJsonWebHandler *passwordChangeHandler = new AsyncCallbackJsonWebHandler("/password/update", [](AsyncWebServerRequest *request, JsonVariant &json) {
  jsonBody.clear();
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    debug("Body was not a JSON object");
  }

  String result;
  if(jsonBody["password"].is<const char*>()) {
    String newPasswd = jsonBody["password"].as<String>();

    // Password is used for the built-in Access Point ability, which will be used when a preferred network is not available.
    if(newPasswd.length() >= 8) {
      // Accesses namespace in read/write mode.
      if(preferences.begin("credentials", false)) {
        #if defined(DEBUG_SEND_TO_CONSOLE)
          debug(F("New Private WiFi Password: "));
          debugln(newPasswd);
        #endif
        preferences.putString("password", newPasswd); // Store user-provided password.
        preferences.end();
      }

      jsonBody.clear();
      jsonBody["status"] = "Password updated, restart required. Please enter your new WiFi password when prompted by your device.";
      serializeJson(jsonBody, result); // Serialize to string.
      request->send(201, "application/json", result);
    }
    else {
      // Password must be at least 8 characters in length.
      jsonBody.clear();
      jsonBody["status"] = "Password must be a minimum of 8 characters to meet WPA2 requirements.";
      serializeJson(jsonBody, result); // Serialize to string.
      request->send(200, "application/json", result);
    }
  }
  else {
    debug("No password in JSON body");
    jsonBody.clear();
    jsonBody["status"] = "Unable to update password.";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
});

// Handles the JSON body for the wifi network info.
AsyncCallbackJsonWebHandler *wifiChangeHandler = new AsyncCallbackJsonWebHandler("/wifi/update", [](AsyncWebServerRequest *request, JsonVariant &json) {
  jsonBody.clear();
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    debug("Body was not a JSON object");
  }

  String result;
  if(jsonBody["network"].is<const char*>() && jsonBody["password"].is<const char*>()) {
    bool b_errors = false; // Assume false until otherwise indicated.
    bool b_enabled = jsonBody["enabled"].as<bool>();
    String wifiNetwork = jsonBody["network"].as<String>();
    String wifiPasswd = jsonBody["password"].as<String>();
    String localAddr = jsonBody["address"].as<String>();
    String subnetMask = jsonBody["subnet"].as<String>();
    String gatewayIP = jsonBody["gateway"].as<String>();

    // If no errors encountered, continue with storing a preferred network (with credentials and IP information).
    if(wifiNetwork.length() >= 2 && wifiPasswd.length() >= 8) {
      // Accesses namespace in read/write mode.
      if(preferences.begin("network", false)) {
        // Clear old network IP info if SSID or password have been changed.
        String old_ssid = preferences.getString("ssid", "");
        String old_passwd = preferences.getString("password", "");
        if(old_ssid == "" || old_ssid != wifiNetwork || old_passwd == "" || old_passwd != wifiPasswd) {
          preferences.putString("address", "");
          preferences.putString("subnet", "");
          preferences.putString("gateway", "");
        }

        // Store the critical values to enable/disable the external WiFi.
        preferences.putBool("enabled", b_enabled);
        preferences.putString("ssid", wifiNetwork);
        preferences.putString("password", wifiPasswd);

        // Continue saving only if network values are 7 characters or more (eg. N.N.N.N)
        if(localAddr.length() >= 7 && localAddr != wifi_address) {
          preferences.putString("address", localAddr);
        }
        if(subnetMask.length() >= 7 && subnetMask != wifi_subnet) {
          preferences.putString("subnet", subnetMask);
        }
        if(gatewayIP.length() >= 7 && gatewayIP != wifi_gateway) {
          preferences.putString("gateway", gatewayIP);
        }

        preferences.end();
      }
    }

    if(!b_errors) {
      jsonBody.clear();

      // Disconnect from the WiFi network and re-apply any changes.
      WiFi.disconnect();
      b_ext_wifi_started = false;

      delay(100); // Delay needed.

      if(b_enabled) {
        b_ext_wifi_started = startExternalWifi(); // Restart and set global flag.

        if(b_ext_wifi_started) {
          jsonBody["status"] = "Settings updated, WiFi connection restarted successfully.";
        }
        else {
          jsonBody["status"] = "Settings updated, but WiFi connection was not successful.";
        }
      }
      else {
        jsonBody["status"] = "Settings updated, and external WiFi has been disconnected.";
      }

      serializeJson(jsonBody, result); // Serialize to string.
      request->send(200, "application/json", result);
    }
    else {
      jsonBody.clear();
      jsonBody["status"] = "Errors encountered while processing request data. Please re-check submitted values and try again.";
      serializeJson(jsonBody, result); // Serialize to string.
      request->send(200, "application/json", result);
    }
  }
  else {
    debug("No password in JSON body");
    jsonBody.clear();
    jsonBody["status"] = "Unable to update password.";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
});

void handleNotFound(AsyncWebServerRequest *request) {
  // Returned for any invalid URL requested.
  debug("Web page not found");
  request->send(404, "text/plain", "Not Found");
}

void setupRouting() {
  // Define the endpoints for the web server.

  // Static Pages
  httpServer.on("/", HTTP_GET, handleRoot);
  httpServer.on("/common.js", HTTP_GET, handleCommonJS);
  httpServer.on("/favicon.ico", HTTP_GET, handleFavIco);
  httpServer.on("/favicon.svg", HTTP_GET, handleFavSvg);
  httpServer.on("/index.js", HTTP_GET, handleRootJS);
  httpServer.on("/network", HTTP_GET, handleNetwork);
  httpServer.on("/password", HTTP_GET, handlePassword);
  httpServer.on("/settings/device", HTTP_GET, handleDeviceSettings);
  httpServer.on("/settings/wand", HTTP_GET, handleWandSettings);
  httpServer.on("/style.css", HTTP_GET, handleStylesheet);
  httpServer.on("/geometry.json", HTTP_GET, handleGeometry);
  httpServer.on("/three.js", HTTP_GET, handleThreeJS);
  httpServer.onNotFound(handleNotFound);

  // Get/Set Handlers
  httpServer.on("/config/device", HTTP_GET, handleGetDeviceConfig);
  httpServer.on("/config/wand", HTTP_GET, handleGetWandConfig);
  httpServer.on("/eeprom/wand", HTTP_PUT, handleSaveWandEEPROM);
  httpServer.on("/status", HTTP_GET, handleGetStatus);
  httpServer.on("/restart", HTTP_DELETE, handleRestart);
  httpServer.on("/volume/master/up", HTTP_PUT, handleMasterVolumeUp);
  httpServer.on("/volume/master/down", HTTP_PUT, handleMasterVolumeDown);
  httpServer.on("/volume/effects/up", HTTP_PUT, handleEffectsVolumeUp);
  httpServer.on("/volume/effects/down", HTTP_PUT, handleEffectsVolumeDown);
  httpServer.on("/volume/music/up", HTTP_PUT, handleMusicVolumeUp);
  httpServer.on("/volume/music/down", HTTP_PUT, handleMusicVolumeDown);
  httpServer.on("/music/startstop", HTTP_PUT, handleMusicStartStop);
  httpServer.on("/music/pauseresume", HTTP_PUT, handleMusicPauseResume);
  httpServer.on("/music/next", HTTP_PUT, handleNextMusicTrack);
  httpServer.on("/music/select", HTTP_PUT, handleSelectMusicTrack);
  httpServer.on("/music/prev", HTTP_PUT, handlePrevMusicTrack);
  httpServer.on("/music/loop", HTTP_PUT, handleLoopMusicTrack);
  httpServer.on("/wifi/settings", HTTP_GET, handleGetWifi);
  httpServer.on("/sensors/recenter", HTTP_PUT, handleResetSensors);
  httpServer.on("/sensors/calibrate/enable", HTTP_PUT, handleCalibrateSensorsEnabled);
  httpServer.on("/sensors/calibrate/disable", HTTP_PUT, handleCalibrateSensorsDisabled);
  httpServer.on("/infrared/signal", HTTP_PUT, handleInfraredSignal);

  // Body Handlers
  httpServer.addHandler(handleSaveDeviceConfig); // /config/device/save
  httpServer.addHandler(handleSaveWandConfig); // /config/wand/save
  httpServer.addHandler(passwordChangeHandler); // /password/update
  httpServer.addHandler(wifiChangeHandler); // /wifi/update
}

// Send notification to all websocket clients.
void notifyWSClients() {
  if(b_ws_started) {
    // Send latest status to all connected clients.
    ws.textAll(getEquipmentStatus());
  }
}

void sendTelemetryData() {
  if(b_ws_started) {
    // Gather the latest filtered motion data, serialize it to a JSON string,
    // and send it to all connected EventSource (SSE) clients as a "telemetry"
    // event name (using the current time as a unique event identifier).
    events.send(getTelemetry().c_str(), "telemetry", millis());
  }
}

// Perform management if the AP and web server are started.
void webLoops() {
  if(b_ap_started && b_ws_started) {
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