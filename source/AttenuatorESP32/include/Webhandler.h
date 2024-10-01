/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

// Web page files (defines all text as char[] variable)
#include "Index.h" // INDEX_page
#include "IndexJS.h" // INDEXJS_page
#include "Device.h" // DEVICE_page
#include "ExtWiFi.h" // NETWORK_page
#include "Password.h" // PASSWORD_page
#include "PackSettings.h" // PACK_SETTINGS_page
#include "WandSettings.h" // WAND_SETTINGS_page
#include "SmokeSettings.h" // SMOKE_SETTINGS_page
#include "Style.h" // STYLE_page
#include "Equip.h" // EQUIP_svg

// Forward function declarations.
void setupRouting();

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
    case HOLIDAY:
      return "Holiday Stream";
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

String getCyclotronState() {
  switch(i_speed_multiplier) {
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

/*
 * Web Handler Functions - Performs actions or returns data for web UI
 */
JsonDocument jsonBody; // Used for processing JSON body/payload data.
JsonDocument jsonSuccess; // Used for sending JSON status as success.
String status; // Holder for simple "status: success" response.

void onWebSocketEventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch(type) {
    case WS_EVT_CONNECT:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        Serial.printf("WebSocket[%s][%u] Connect\n", server->url(), client->id());
      #endif
      i_ws_client_count++;
    break;

    case WS_EVT_DISCONNECT:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        Serial.printf("WebSocket[%s][%u] Disconnect\n", server->url(), client->id());
      #endif
      if(i_ws_client_count > 0) {
        i_ws_client_count--;
      }
    break;

    case WS_EVT_ERROR:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        Serial.printf("WebSocket[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
      #endif
    break;

    case WS_EVT_PONG:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        Serial.printf("WebSocket[%s][%u] Pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
      #endif
    break;

    case WS_EVT_DATA:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        Serial.println(F("WebSocket Data Received"));
      #endif
      // Do something when data is received via WebSocket.
    break;
  }
}

void startWebServer() {
  // Configures URI routing with function handlers.
  setupRouting();

  // Prepare a standard "success" message for responses.
  jsonSuccess.clear();
  jsonSuccess["status"] = "success";
  serializeJson(jsonSuccess, status);

  // Configure the WebSocket endpoint.
  ws.onEvent(onWebSocketEventHandler);
  httpServer.addHandler(&ws);

  // Configure the OTA firmware endpoint handler.
  ElegantOTA.begin(&httpServer);

  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);

  // Start the web server.
  httpServer.begin();
  //Serial.println("Async HTTP Server Started");
}

void handleRoot(AsyncWebServerRequest *request) {
  // Used for the root page (/) from the web server.
  //debug("Web Root HTML Requested");
  request->send(200, "text/html", String(INDEX_page)); // Serve page content.
}

void handleRootJS(AsyncWebServerRequest *request) {
  // Used for the root page (/) from the web server.
  //debug("Web Root JavaScript Requested");
  request->send(200, "application/javascript", String(INDEXJS_page)); // Serve page content.
}

void handleNetwork(AsyncWebServerRequest *request) {
  // Used for the network page from the web server.
  //debug("Network HTML Requested");
  request->send(200, "text/html", String(NETWORK_page)); // Serve page content.
}

void handlePassword(AsyncWebServerRequest *request) {
  // Used for the password page from the web server.
  //debug("Password HTML Requested");
  request->send(200, "text/html", String(PASSWORD_page)); // Serve page content.
}

void handleAttenuatorSettings(AsyncWebServerRequest *request) {
  // Used for the device page from the web server.
  //debug("Attenuator Settings HTML Requested");
  request->send(200, "text/html", String(DEVICE_page)); // Serve page content.
}

void handlePackSettings(AsyncWebServerRequest *request) {
  // Tell the pack that we'll need the latest pack EEPROM values.
  b_received_prefs_pack = false;
  attenuatorSerialSend(A_REQUEST_PREFERENCES_PACK);

  // Used for the settings page from the web server.
  //debug("Pack Settings HTML Requested");
  request->send(200, "text/html", String(PACK_SETTINGS_page)); // Serve page content.
}

void handleWandSettings(AsyncWebServerRequest *request) {
  // Tell the pack that we'll need the latest wand EEPROM values.
  b_received_prefs_wand = false;
  attenuatorSerialSend(A_REQUEST_PREFERENCES_WAND);

  // Used for the settings page from the web server.
  //debug("Wand Settings HTML Requested");
  request->send(200, "text/html", String(WAND_SETTINGS_page)); // Serve page content.
}

void handleSmokeSettings(AsyncWebServerRequest *request) {
  // Tell the pack that we'll need the latest smoke EEPROM values.
  b_received_prefs_smoke = false;
  attenuatorSerialSend(A_REQUEST_PREFERENCES_SMOKE);

  // Used for the settings page from the web server.
  //debug("Smoke Settings HTML Requested");
  request->send(200, "text/html", String(SMOKE_SETTINGS_page)); // Serve page content.
}

void handleStylesheet(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  //debug("Main StyleSheet Requested");
  request->send(200, "text/css", String(STYLE_page)); // Serve page content.
}

void handleSvgImage(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  //debug("Equipment SVG Requested");
  request->send(200, "image/svg+xml", String(EQUIP_svg)); // Serve page content.
}

String getAttenuatorConfig() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonBody.clear();

  // Provide current values for the Attenuator device.
  jsonBody["invertLEDs"] = b_invert_leds;
  jsonBody["buzzer"] = b_enable_buzzer;
  jsonBody["vibration"] = b_enable_vibration;
  jsonBody["overheat"] = b_overheat_feedback;
  jsonBody["firing"] = b_firing_feedback;
  jsonBody["radLensIdle"] = RAD_LENS_IDLE;
  jsonBody["displayType"] = DISPLAY_TYPE;
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

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getPackConfig() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonBody.clear();

  if(!b_wait_for_pack) {
    // Provide a flag to indicate prefs were received via serial coms.
    jsonBody["prefsAvailable"] = b_received_prefs_pack;

    // Return current powered state for pack and wand.
    jsonBody["packPowered"] = (b_pack_on ? true : false);
    jsonBody["wandPowered"] = (b_wand_on ? true : false);

    // Proton Pack Runtime Options
    jsonBody["defaultSystemModePack"] = packConfig.defaultSystemModePack; // [0=SH,1=MO]
    jsonBody["defaultYearThemePack"] = packConfig.defaultYearThemePack; // [1=TOGGLE,2=1984,3=1989,4=2021,5=2024]
    jsonBody["currentYearThemePack"] = packConfig.currentYearThemePack; // [2=1984,3=1989,4=2021,5=2024]
    jsonBody["defaultSystemVolume"] = packConfig.defaultSystemVolume; // 0-100
    jsonBody["packVibration"] = packConfig.packVibration; // [1=ALWAYS,2=FIRING,3=NEVER,4=TOGGLE,5=MOTORIZED_CYCLOTRON]
    jsonBody["protonStreamEffects"] = packConfig.protonStreamEffects; // true|false
    jsonBody["ribbonCableAlarm"] = packConfig.ribbonCableAlarm; // true|false
    jsonBody["overheatStrobeNF"] = packConfig.overheatStrobeNF; // true|false
    jsonBody["overheatLightsOff"] = packConfig.overheatLightsOff; // true|false
    jsonBody["overheatSyncToFan"] = packConfig.overheatSyncToFan;// true|false
    jsonBody["demoLightMode"] = packConfig.demoLightMode; // true|false

    // Proton Pack LED Options
    jsonBody["ledCycLidCount"] = packConfig.ledCycLidCount; // [12,20,36,40]
    jsonBody["ledCycLidHue"] = packConfig.ledCycLidHue; // Spectral custom colour/hue 1-254
    jsonBody["ledCycLidSat"] = packConfig.ledCycLidSat; // Spectral custom saturation 1-254
    jsonBody["cyclotronDirection"] = packConfig.cyclotronDirection; // [0=CCW,1=CW]
    jsonBody["ledCycLidCenter"] = packConfig.ledCycLidCenter; // [0=3,1=1]
    jsonBody["ledCycLidFade"] = packConfig.ledCycLidFade; // true|false
    jsonBody["ledVGCyclotron"] = packConfig.ledVGCyclotron; // true|false
    jsonBody["ledCycLidSimRing"] = packConfig.ledCycLidSimRing; // true|false
    jsonBody["ledCycInnerPanel"] = packConfig.ledCycInnerPanel; // [1=Individual,2=RGB-Static,3=RGB-Dynamic]
    jsonBody["ledCycCakeCount"] = packConfig.ledCycCakeCount; // [12,23,24,26,35,36]
    jsonBody["ledCycCakeHue"] = packConfig.ledCycCakeHue; // Spectral custom colour/hue 1-254
    jsonBody["ledCycCakeSat"] = packConfig.ledCycCakeSat; // Spectral custom saturation 1-254
    jsonBody["ledCycCakeGRB"] = packConfig.ledCycCakeGRB; // Use GRB for cake LEDs true|false
    jsonBody["ledCycCavCount"] = packConfig.ledCycCavCount; // Cyclotron cavity LEDs (0-20)
    jsonBody["ledPowercellCount"] = packConfig.ledPowercellCount; //[13,15]
    jsonBody["ledInvertPowercell"] = packConfig.ledInvertPowercell; // true|false
    jsonBody["ledPowercellHue"] = packConfig.ledPowercellHue; // Spectral custom colour/hue 1-254
    jsonBody["ledPowercellSat"] = packConfig.ledPowercellSat; // Spectral custom saturation 1-254
    jsonBody["ledVGPowercell"] = packConfig.ledVGPowercell; // true|false
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getWandConfig() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonBody.clear();

  if(!b_wait_for_pack) {
    // Provide a flag to indicate prefs were received via serial coms.
    jsonBody["prefsAvailable"] = b_received_prefs_wand;

    // Return current powered state for pack and wand.
    jsonBody["packPowered"] = (b_pack_on ? true : false);
    jsonBody["wandPowered"] = (b_wand_on ? true : false);

    // Neutrona Wand LED Options
    jsonBody["ledWandCount"] = wandConfig.ledWandCount; // [0=5,1=29,2=48]
    jsonBody["ledWandHue"] = wandConfig.ledWandHue; // Spectral custom colour/hue 1-254
    jsonBody["ledWandSat"] = wandConfig.ledWandSat; // Spectral custom saturation 1-254
    jsonBody["spectralModesEnabled"] = wandConfig.spectralModesEnabled; // true|false

    // Neutrona Wand Runtime Options
    jsonBody["overheatEnabled"] = wandConfig.overheatEnabled; // true|false
    jsonBody["defaultFiringMode"] = wandConfig.defaultFiringMode; // [1=VG,2=CTS,3=CTS_MIX]
    jsonBody["wandVibration"] = wandConfig.wandVibration; // [1=ALWAYS,2=FIRING,3=NEVER,4=TOGGLE]
    jsonBody["wandSoundsToPack"] = wandConfig.wandSoundsToPack; // true|false
    jsonBody["quickVenting"] = wandConfig.quickVenting; // true|false (Super-Hero Mode Only)
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
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getSmokeConfig() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonBody.clear();

  if(!b_wait_for_pack) {
    // Provide a flag to indicate prefs were received via serial coms.
    jsonBody["prefsAvailable"] = b_received_prefs_smoke;

    // Return current powered state for pack and wand.
    jsonBody["packPowered"] = (b_pack_on ? true : false);
    jsonBody["wandPowered"] = (b_wand_on ? true : false);

    // Proton Pack
    jsonBody["smokeEnabled"] = (smokeConfig.smokeEnabled == 1); // true|false

    // Power Level 5
    jsonBody["overheatDuration5"] = smokeConfig.overheatDuration5; // 2-60 Seconds
    jsonBody["overheatContinuous5"] = (smokeConfig.overheatContinuous5 == 1); // true|false
    // Power Level 4
    jsonBody["overheatDuration4"] = smokeConfig.overheatDuration4; // 2-60 Seconds
    jsonBody["overheatContinuous4"] = (smokeConfig.overheatContinuous4 == 1); // true|false
    // Power Level 3
    jsonBody["overheatDuration3"] = smokeConfig.overheatDuration3; // 2-60 Seconds
    jsonBody["overheatContinuous3"] = (smokeConfig.overheatContinuous3 == 1); // true|false
    // Power Level 2
    jsonBody["overheatDuration2"] = smokeConfig.overheatDuration2; // 2-60 Seconds
    jsonBody["overheatContinuous2"] = (smokeConfig.overheatContinuous2 == 1); // true|false
    // Power Level 1
    jsonBody["overheatDuration1"] = smokeConfig.overheatDuration1; // 2-60 Seconds
    jsonBody["overheatContinuous1"] = (smokeConfig.overheatContinuous1 == 1); // true|false

    // Neutrona Wand

    // Power Level 5
    jsonBody["overheatLevel5"] = (smokeConfig.overheatLevel5 == 1); // true|false
    jsonBody["overheatDelay5"] = smokeConfig.overheatDelay5; // 2-60 Seconds
    // Power Level 4
    jsonBody["overheatLevel4"] = (smokeConfig.overheatLevel4 == 1); // true|false
    jsonBody["overheatDelay4"] = smokeConfig.overheatDelay4; // 2-60 Seconds
    // Power Level 3
    jsonBody["overheatLevel3"] = (smokeConfig.overheatLevel3 == 1); // true|false
    jsonBody["overheatDelay3"] = smokeConfig.overheatDelay3; // 2-60 Seconds
    // Power Level 2
    jsonBody["overheatLevel2"] = (smokeConfig.overheatLevel2 == 1); // true|false
    jsonBody["overheatDelay2"] = smokeConfig.overheatDelay2; // 2-60 Seconds
    // Power Level 1
    jsonBody["overheatLevel1"] = (smokeConfig.overheatLevel1 == 1); // true|false
    jsonBody["overheatDelay1"] = smokeConfig.overheatDelay1; // 2-60 Seconds
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getEquipmentStatus() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipStatus;
  jsonBody.clear();

  if(!b_wait_for_pack) {
    // Only prepare status when not waiting on the pack
    jsonBody["mode"] = getMode();
    jsonBody["modeID"] = (SYSTEM_MODE == MODE_SUPER_HERO) ? 1 : 0;
    jsonBody["theme"] = getTheme();
    jsonBody["themeID"] = SYSTEM_YEAR;
    jsonBody["switch"] = getRedSwitch();
    jsonBody["pack"] = (b_pack_on ? "Powered" : "Idle");
    jsonBody["power"] = getPower();
    jsonBody["safety"] = getSafety();
    jsonBody["wand"] = (b_wand_present ? "Connected" : "Not Connected");
    jsonBody["wandPower"] = (b_wand_on ? "Powered" : "Idle");
    jsonBody["wandMode"] = getWandMode();
    jsonBody["firing"] = (b_firing ? "Firing" : "Idle");
    jsonBody["cable"] = (b_pack_alarm ? "Disconnected" : "Connected");
    jsonBody["cyclotron"] = getCyclotronState();
    jsonBody["cyclotronLid"] = b_cyclotron_lid_on;
    jsonBody["temperature"] = (b_overheating ? "Venting" : "Normal");
    jsonBody["musicPlaying"] = b_playing_music;
    jsonBody["musicPaused"] = b_music_paused;
    jsonBody["musicCurrent"] = i_music_track_current;
    jsonBody["musicStart"] = i_music_track_min;
    jsonBody["musicEnd"] = i_music_track_max;
    jsonBody["volMaster"] = i_volume_master_percentage;
    jsonBody["volEffects"] = i_volume_effects_percentage;
    jsonBody["volMusic"] = i_volume_music_percentage;
    jsonBody["battVoltage"] = f_batt_volts;
    jsonBody["wandAmps"] = f_wand_amps;
    jsonBody["apClients"] = i_ap_client_count;
    jsonBody["wsClients"] = i_ws_client_count;
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipStatus);
  return equipStatus;
}

String getWifiSettings() {
  // Prepare a JSON object with information stored in preferences (or a blank default).
  String wifiNetwork;
  jsonBody.clear();

  preferences.begin("network", true, "nvs"); // Access namespace in read-only mode.

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

  // Serialize JSON object to string.
  serializeJson(jsonBody, wifiNetwork);
  return wifiNetwork;
}

void handleGetAttenuatorConfig(AsyncWebServerRequest *request) {
  // Return current attenuator settings as a stringified JSON object.
  request->send(200, "application/json", getAttenuatorConfig());
}

void handleGetPackConfig(AsyncWebServerRequest *request) {
  // Return current pack settings as a stringified JSON object.
  request->send(200, "application/json", getPackConfig());
}

void handleGetWandConfig(AsyncWebServerRequest *request) {
  // Return current wand settings as a stringified JSON object.
  request->send(200, "application/json", getWandConfig());
}

void handleGetSmokeConfig(AsyncWebServerRequest *request) {
  // Return current smoke settings as a stringified JSON object.
  request->send(200, "application/json", getSmokeConfig());
}

void handleGetStatus(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getEquipmentStatus());
}

void handleGetWifi(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getWifiSettings());
}

void handleRestart(AsyncWebServerRequest *request) {
  // Performs a restart of the device.
  request->send(204, "application/json", status);
  delay(1000);
  ESP.restart();
}

void handlePackOn(AsyncWebServerRequest *request) {
  debug("Turn Pack On");
  attenuatorSerialSend(A_TURN_PACK_ON);
  request->send(200, "application/json", status);
}

void handlePackOff(AsyncWebServerRequest *request) {
  debug("Turn Pack Off");
  attenuatorSerialSend(A_TURN_PACK_OFF);
  request->send(200, "application/json", status);
}

void handleAttenuatePack(AsyncWebServerRequest *request) {
  if(i_speed_multiplier > 2) {
    // Only send command to pack if cyclotron is not "normal".
    debug("Cancel Overheat Warning");
    attenuatorSerialSend(A_WARNING_CANCELLED);
    request->send(200, "application/json", status);
  } else {
    // Tell the user why the requested action failed.
    String result;
    jsonBody.clear();
    jsonBody["status"] = "System not in overheat warning";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
}

void handleManualVent(AsyncWebServerRequest *request) {
  debug("Manual Vent Triggered");
  attenuatorSerialSend(A_MANUAL_OVERHEAT);
  request->send(200, "application/json", status);
}

void handleManualLockout(AsyncWebServerRequest *request) {
  debug("Manual Lockout Triggered");
  attenuatorSerialSend(A_SYSTEM_LOCKOUT);
  request->send(200, "application/json", status);
}

void handleCancelLockout(AsyncWebServerRequest *request) {
  debug("Cancel Lockout Triggered");
  attenuatorSerialSend(A_CANCEL_LOCKOUT);
  request->send(200, "application/json", status);
}

void handleToggleMute(AsyncWebServerRequest *request) {
  debug("Toggle Mute");
  attenuatorSerialSend(A_TOGGLE_MUTE);
  request->send(200, "application/json", status);
}

void handleMasterVolumeUp(AsyncWebServerRequest *request) {
  debug("Master Volume Up");
  attenuatorSerialSend(A_VOLUME_INCREASE);
  request->send(200, "application/json", status);
}

void handleMasterVolumeDown(AsyncWebServerRequest *request) {
  debug("Master Volume Down");
  attenuatorSerialSend(A_VOLUME_DECREASE);
  request->send(200, "application/json", status);
}

void handleEffectsVolumeUp(AsyncWebServerRequest *request) {
  debug("Effects Volume Up");
  attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_INCREASE);
  request->send(200, "application/json", status);
}

void handleEffectsVolumeDown(AsyncWebServerRequest *request) {
  debug("Effects Volume Down");
  attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_DECREASE);
  request->send(200, "application/json", status);
}

void handleMusicVolumeUp(AsyncWebServerRequest *request) {
  debug("Music Volume Up");
  attenuatorSerialSend(A_VOLUME_MUSIC_INCREASE);
  request->send(200, "application/json", status);
}

void handleMusicVolumeDown(AsyncWebServerRequest *request) {
  debug("Music Volume Down");
  attenuatorSerialSend(A_VOLUME_MUSIC_DECREASE);
  request->send(200, "application/json", status);
}

void handleMusicStartStop(AsyncWebServerRequest *request) {
  debug("Music Start/Stop");
  attenuatorSerialSend(A_MUSIC_START_STOP);
  request->send(200, "application/json", status);
}

void handleMusicPauseResume(AsyncWebServerRequest *request) {
  debug("Music Pause/Resume");
  attenuatorSerialSend(A_MUSIC_PAUSE_RESUME);
  request->send(200, "application/json", status);
}

void handleNextMusicTrack(AsyncWebServerRequest *request) {
  debug("Next Music Track");
  attenuatorSerialSend(A_MUSIC_NEXT_TRACK);
  request->send(200, "application/json", status);
}

void handlePrevMusicTrack(AsyncWebServerRequest *request) {
  debug("Prev Music Track");
  attenuatorSerialSend(A_MUSIC_PREV_TRACK);
  request->send(200, "application/json", status);
}

void handleLoopMusicTrack(AsyncWebServerRequest *request) {
  debug("Toggle Music Track Loop");
  attenuatorSerialSend(A_MUSIC_TRACK_LOOP_TOGGLE);
  request->send(200, "application/json", status);
}

void handleSelectMusicTrack(AsyncWebServerRequest *request) {
  String c_music_track = "";

  if(request->hasParam("track")) {
    // Get the parameter "track" if it exists (will be a String).
    c_music_track = request->getParam("track")->value();
  }

  if(c_music_track.toInt() != 0 && c_music_track.toInt() >= i_music_track_min) {
    uint16_t i_music_track = c_music_track.toInt();
    debug("Selected Music Track: " + String(i_music_track));
    attenuatorSerialSend(A_MUSIC_PLAY_TRACK, i_music_track); // Inform the pack of the new track.
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

void handleSaveAllEEPROM(AsyncWebServerRequest *request) {
  debug("Save All EEPROM");
  attenuatorSerialSend(A_SAVE_EEPROM_SETTINGS_PACK);
  attenuatorSerialSend(A_SAVE_EEPROM_SETTINGS_WAND);
  request->send(200, "application/json", status);
}

void handleSavePackEEPROM(AsyncWebServerRequest *request) {
  debug("Save Pack EEPROM");
  attenuatorSerialSend(A_SAVE_EEPROM_SETTINGS_PACK);
  request->send(200, "application/json", status);
}

void handleSaveWandEEPROM(AsyncWebServerRequest *request) {
  debug("Save Wand EEPROM");
  attenuatorSerialSend(A_SAVE_EEPROM_SETTINGS_WAND);
  request->send(200, "application/json", status);
}

// Handles the JSON body for the pack settings save request.
AsyncCallbackJsonWebHandler *handleSaveAttenuatorConfig = new AsyncCallbackJsonWebHandler("/config/attenuator/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  jsonBody.clear();
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    Serial.print("Body was not a JSON object");
  }

  String result;
  try {
    // First check if a new private WiFi network name has been chosen.
    String newSSID = jsonBody["wifiName"].as<String>();
    bool b_ssid_changed = false;

    // Update the private network name ONLY if the new value differs from the current SSID.
    if(newSSID != ap_ssid){
      if(newSSID.length() >= 8 && newSSID.length() <= 32) {
        preferences.begin("credentials", false, "nvs"); // Access namespace in read/write mode.
        #if defined(DEBUG_SEND_TO_CONSOLE)
          Serial.print(F("New Private SSID: "));
          Serial.println(newSSID);
        #endif
        preferences.putString("ssid", newSSID); // Store SSID in case this was altered.
        preferences.end();

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

    // General Options - Returned as unsigned integers
    if(jsonBody["invertLEDs"].is<unsigned short>()) {
      // Inverts the order of the LEDs as seen by the device.
      b_invert_leds = jsonBody["invertLEDs"].as<bool>();
    }

    if(jsonBody["buzzer"].is<unsigned short>()) {
      // Enable/disable the buzzer completely.
      b_enable_buzzer = jsonBody["buzzer"].as<bool>();
    }

    if(jsonBody["vibration"].is<unsigned short>()) {
      // Enable/disable vibration completely.
      b_enable_vibration = jsonBody["vibration"].as<bool>();
    }

    if(jsonBody["overheat"].is<unsigned short>()) {
      // Enable/disable all buzzer/vibration feedback during overheat/alarm.
      b_overheat_feedback = jsonBody["overheat"].as<bool>();
    }

    if(jsonBody["firing"].is<unsigned short>()) {
      // Enable/disable vibration when throwing a stream.
      b_firing_feedback = jsonBody["firing"].as<bool>();
    }

    if(jsonBody["radLensIdle"].is<unsigned short>()) {
      switch(jsonBody["radLensIdle"].as<unsigned short>()) {
        case 0:
          RAD_LENS_IDLE = AMBER_PULSE;
        break;
        case 1:
          RAD_LENS_IDLE = ORANGE_FADE;
        break;
        case 2:
          RAD_LENS_IDLE = RED_FADE;
        break;
      }
    }
    if(jsonBody["displayType"].is<unsigned short>()) {
      switch(jsonBody["displayType"].as<unsigned short>()) {
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

    // Get the track listing from the text field.
    String songList = jsonBody["songList"].as<String>();
    bool b_list_err = false;

    preferences.begin("device", false, "nvs"); // Access namespace in read/write mode.
    preferences.putBool("invert_led", b_invert_leds);
    preferences.putBool("use_buzzer", b_enable_buzzer);
    preferences.putBool("use_vibration", b_enable_vibration);
    preferences.putBool("use_overheat", b_overheat_feedback);
    preferences.putBool("fire_feedback", b_firing_feedback);
    preferences.putShort("radiation_idle", RAD_LENS_IDLE);
    preferences.putShort("display_type", DISPLAY_TYPE);
    if(songList.length() <= 2000) {
      if(songList == "null") {
        songList = "";
      }

      // Update song lists if contents are under 2000 bytes.
      #if defined(DEBUG_SEND_TO_CONSOLE)
        Serial.print(F("Song List Bytes: "));
        Serial.println(songList.length());
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

// Handles the JSON body for the pack settings save request.
AsyncCallbackJsonWebHandler *handleSavePackConfig = new AsyncCallbackJsonWebHandler("/config/pack/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  jsonBody.clear();
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    Serial.print("Body was not a JSON object");
  }

  String result;
  if(!b_pack_on && !b_wand_on) {
    try {
      // General Options
      packConfig.defaultSystemModePack = jsonBody["defaultSystemModePack"].as<uint8_t>();
      packConfig.defaultYearThemePack = jsonBody["defaultYearThemePack"].as<uint8_t>();
      packConfig.currentYearThemePack = jsonBody["currentYearThemePack"].as<uint8_t>();
      packConfig.defaultSystemVolume = jsonBody["defaultSystemVolume"].as<uint8_t>();
      packConfig.packVibration = jsonBody["packVibration"].as<uint8_t>();
      packConfig.ribbonCableAlarm = jsonBody["ribbonCableAlarm"].as<uint8_t>();
      packConfig.protonStreamEffects = jsonBody["protonStreamEffects"].as<uint8_t>();
      packConfig.overheatStrobeNF = jsonBody["overheatStrobeNF"].as<uint8_t>();
      packConfig.overheatLightsOff = jsonBody["overheatLightsOff"].as<uint8_t>();
      packConfig.overheatSyncToFan = jsonBody["overheatSyncToFan"].as<uint8_t>();
      packConfig.demoLightMode = jsonBody["demoLightMode"].as<uint8_t>();

      // Update certain operational values immediately.
      switch(packConfig.defaultSystemModePack) {
        case 0:
        default:
          SYSTEM_MODE = MODE_SUPER_HERO;
          RED_SWITCH_MODE = SWITCH_OFF;
        break;

        case 1:
          SYSTEM_MODE = MODE_ORIGINAL;
          RED_SWITCH_MODE = SWITCH_OFF;
        break;
      }

      // Cyclotron Lid
      packConfig.ledCycLidCount = jsonBody["ledCycLidCount"].as<uint8_t>();
      packConfig.ledCycLidHue = jsonBody["ledCycLidHue"].as<uint8_t>();
      packConfig.ledCycLidSat = jsonBody["ledCycLidSat"].as<uint8_t>();
      packConfig.cyclotronDirection = jsonBody["cyclotronDirection"].as<uint8_t>();
      packConfig.ledCycLidCenter = jsonBody["ledCycLidCenter"].as<uint8_t>();
      packConfig.ledCycLidFade = jsonBody["ledCycLidFade"].as<uint8_t>();
      packConfig.ledVGCyclotron = jsonBody["ledVGCyclotron"].as<uint8_t>();
      packConfig.ledCycLidSimRing = jsonBody["ledCycLidSimRing"].as<uint8_t>();

      // Inner Cyclotron
      packConfig.ledCycInnerPanel = jsonBody["ledCycInnerPanel"].as<uint8_t>();
      packConfig.ledCycCakeCount = jsonBody["ledCycCakeCount"].as<uint8_t>();
      packConfig.ledCycCakeHue = jsonBody["ledCycCakeHue"].as<uint8_t>();
      packConfig.ledCycCakeSat = jsonBody["ledCycCakeSat"].as<uint8_t>();
      packConfig.ledCycCakeGRB = jsonBody["ledCycCakeGRB"].as<uint8_t>();
      packConfig.ledCycCavCount = jsonBody["ledCycCavCount"].as<uint8_t>();
      if(packConfig.ledCycCavCount > 20) {
        packConfig.ledCycCavCount = 20; // Set maximum allowed.
      }

      // Power Cell
      packConfig.ledPowercellCount = jsonBody["ledPowercellCount"].as<uint8_t>();
      packConfig.ledInvertPowercell = jsonBody["ledInvertPowercell"].as<uint8_t>();
      packConfig.ledPowercellHue = jsonBody["ledPowercellHue"].as<uint8_t>();
      packConfig.ledPowercellSat = jsonBody["ledPowercellSat"].as<uint8_t>();
      packConfig.ledVGPowercell = jsonBody["ledVGPowercell"].as<uint8_t>();

      jsonBody.clear();
      jsonBody["status"] = "Settings updated, please test before saving to EEPROM.";
      serializeJson(jsonBody, result); // Serialize to string.
      attenuatorSerialSendData(A_SAVE_PREFERENCES_PACK); // Tell the pack to save the new settings.
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
    String result;
    jsonBody.clear();
    jsonBody["status"] = "Pack and/or Wand are running, save action cancelled";
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
    Serial.print("Body was not a JSON object");
  }

  String result;
  if(!b_pack_on && !b_wand_on) {
    try {
      wandConfig.ledWandCount = jsonBody["ledWandCount"].as<uint8_t>();
      wandConfig.ledWandHue = jsonBody["ledWandHue"].as<uint8_t>();
      wandConfig.ledWandSat = jsonBody["ledWandSat"].as<uint8_t>();
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
      attenuatorSerialSendData(A_SAVE_PREFERENCES_WAND); // Tell the wand (via pack) to save the new settings.
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
    String result;
    jsonBody.clear();
    jsonBody["status"] = "Pack and/or Wand are running, save action cancelled";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
});

// Handles the JSON body for the smoke settings save request.
AsyncCallbackJsonWebHandler *handleSaveSmokeConfig = new AsyncCallbackJsonWebHandler("/config/smoke/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  jsonBody.clear();
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    Serial.print("Body was not a JSON object");
  }

  String result;
  if(!b_pack_on && !b_wand_on) {
    try {
      smokeConfig.smokeEnabled = jsonBody["smokeEnabled"].as<uint8_t>();

      smokeConfig.overheatDuration5 = jsonBody["overheatDuration5"].as<uint8_t>();
      smokeConfig.overheatDuration4 = jsonBody["overheatDuration4"].as<uint8_t>();
      smokeConfig.overheatDuration3 = jsonBody["overheatDuration3"].as<uint8_t>();
      smokeConfig.overheatDuration2 = jsonBody["overheatDuration2"].as<uint8_t>();
      smokeConfig.overheatDuration1 = jsonBody["overheatDuration1"].as<uint8_t>();

      smokeConfig.overheatContinuous5 = jsonBody["overheatContinuous5"].as<uint8_t>();
      smokeConfig.overheatContinuous4 = jsonBody["overheatContinuous4"].as<uint8_t>();
      smokeConfig.overheatContinuous3 = jsonBody["overheatContinuous3"].as<uint8_t>();
      smokeConfig.overheatContinuous2 = jsonBody["overheatContinuous2"].as<uint8_t>();
      smokeConfig.overheatContinuous1 = jsonBody["overheatContinuous1"].as<uint8_t>();

      smokeConfig.overheatLevel5 = jsonBody["overheatLevel5"].as<uint8_t>();
      smokeConfig.overheatLevel4 = jsonBody["overheatLevel4"].as<uint8_t>();
      smokeConfig.overheatLevel3 = jsonBody["overheatLevel3"].as<uint8_t>();
      smokeConfig.overheatLevel2 = jsonBody["overheatLevel2"].as<uint8_t>();
      smokeConfig.overheatLevel1 = jsonBody["overheatLevel1"].as<uint8_t>();

      smokeConfig.overheatDelay5 = jsonBody["overheatDelay5"].as<uint8_t>();
      smokeConfig.overheatDelay4 = jsonBody["overheatDelay4"].as<uint8_t>();
      smokeConfig.overheatDelay3 = jsonBody["overheatDelay3"].as<uint8_t>();
      smokeConfig.overheatDelay2 = jsonBody["overheatDelay2"].as<uint8_t>();
      smokeConfig.overheatDelay1 = jsonBody["overheatDelay1"].as<uint8_t>();

      jsonBody.clear();
      jsonBody["status"] = "Settings updated, please test before saving to EEPROM.";
      serializeJson(jsonBody, result); // Serialize to string.
      attenuatorSerialSendData(A_SAVE_PREFERENCES_SMOKE); // Tell the pack and wand to save the new settings.
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
    String result;
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
    Serial.print("Body was not a JSON object");
  }

  String result;
  if(jsonBody["password"].is<const char*>()) {
    String newPasswd = jsonBody["password"].as<String>();

    // Password is used for the built-in Access Point ability, which will be used when a preferred network is not available.
    if(newPasswd.length() >= 8) {
      preferences.begin("credentials", false, "nvs"); // Access namespace in read/write mode.
      #if defined(DEBUG_SEND_TO_CONSOLE)
        Serial.print(F("New Private WiFi Password: "));
        Serial.println(newPasswd);
      #endif
      preferences.putString("password", newPasswd); // Store user-provided password.
      preferences.end();

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
    Serial.print("Body was not a JSON object");
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
      preferences.begin("network", false, "nvs"); // Access namespace in read/write mode.

      // Clear old network IP info if SSID or password have been changed.
      if(preferences.getString("ssid") != wifiNetwork || preferences.getString("password") != wifiPasswd) {
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

    if(!b_errors) {
      jsonBody.clear();

      // Disconnect from the WiFi network and re-apply any changes.
      WiFi.disconnect();

      delay(100); // Delay needed.

      if(b_enabled) {
        if(startExternalWifi()) {
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
  httpServer.on("/index.js", HTTP_GET, handleRootJS);
  httpServer.on("/network", HTTP_GET, handleNetwork);
  httpServer.on("/password", HTTP_GET, handlePassword);
  httpServer.on("/settings/attenuator", HTTP_GET, handleAttenuatorSettings);
  httpServer.on("/settings/pack", HTTP_GET, handlePackSettings);
  httpServer.on("/settings/wand", HTTP_GET, handleWandSettings);
  httpServer.on("/settings/smoke", HTTP_GET, handleSmokeSettings);
  httpServer.on("/style.css", HTTP_GET, handleStylesheet);
  httpServer.on("/equipment.svg", HTTP_GET, handleSvgImage);
  httpServer.onNotFound(handleNotFound);

  // Get/Set Handlers
  httpServer.on("/config/attenuator", HTTP_GET, handleGetAttenuatorConfig);
  httpServer.on("/config/pack", HTTP_GET, handleGetPackConfig);
  httpServer.on("/config/wand", HTTP_GET, handleGetWandConfig);
  httpServer.on("/config/smoke", HTTP_GET, handleGetSmokeConfig);
  httpServer.on("/eeprom/all", HTTP_PUT, handleSaveAllEEPROM);
  httpServer.on("/eeprom/pack", HTTP_PUT, handleSavePackEEPROM);
  httpServer.on("/eeprom/wand", HTTP_PUT, handleSaveWandEEPROM);
  httpServer.on("/status", HTTP_GET, handleGetStatus);
  httpServer.on("/restart", HTTP_DELETE, handleRestart);
  httpServer.on("/pack/on", HTTP_PUT, handlePackOn);
  httpServer.on("/pack/off", HTTP_PUT, handlePackOff);
  httpServer.on("/pack/attenuate", HTTP_PUT, handleAttenuatePack);
  httpServer.on("/pack/vent", HTTP_PUT, handleManualVent);
  httpServer.on("/pack/lockout/start", HTTP_PUT, handleManualLockout);
  httpServer.on("/pack/lockout/cancel", HTTP_PUT, handleCancelLockout);
  httpServer.on("/volume/toggle", HTTP_PUT, handleToggleMute);
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

  // Body Handlers
  httpServer.addHandler(handleSaveAttenuatorConfig); // /config/attenuator/save
  httpServer.addHandler(handleSavePackConfig); // /config/pack/save
  httpServer.addHandler(handleSaveWandConfig); // /config/wand/save
  httpServer.addHandler(handleSaveSmokeConfig); // /config/smoke/save
  httpServer.addHandler(passwordChangeHandler); // /password/update
  httpServer.addHandler(wifiChangeHandler); // /wifi/update
}

// Send notification to all websocket clients.
void notifyWSClients() {
  // Send latest status to all connected clients.
  ws.textAll(getEquipmentStatus());
}
