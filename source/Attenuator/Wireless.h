/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

/**
 * Wireless (WiFi) Communications for ESP32
 *
 * This device will use the SoftAP mode to act as a standalone WiFi access point, allowing
 * direct connections to the device without need for a full wireless network. All address
 * (IP) assignments will be handled as part of the code here.
 *
 * Note that per the Expressif programming guide: "ESP32 has only one 2.4 GHz ISM band RF
 * module, which is shared by Bluetooth (BT & BLE) and Wi-Fi, so Bluetooth can’t receive
 * or transmit data while Wi-Fi is receiving or transmitting data and vice versa. Under
 * such circumstances, ESP32 uses the time-division multiplexing method to receive and
 * transmit packets."
 *
 * Essentially performance suffers when both WiFi and Bluetooth are enabled and so we
 * must choose which is more useful to the operation of this device. Decision: WiFi.
 *
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/coexist.html
 */
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <AsyncTCP.h>
#include <ElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WiFi.h>

// Web page files (defines all text as char[] variable)
#include "Index.h" // INDEX_page
#include "Password.h" // PASSWORD_page
#include "Style.h" // STYLE_page

// Preferences for SSID and AP password, which will use a "credentials" namespace.
Preferences preferences;

// Set up values for the SSID and password for the WiFi access point (AP).
const String ap_ssid_prefix = "ProtonPack"; // This will be the base of the SSID name.
String ap_default_passwd = "555-2368"; // This will be the default password for the AP.
String ap_ssid; // Reserved for storing the true SSID for the AP to be set at startup.
String ap_pass; // Reserved for storing the true AP password set by the user.

// Define an asynchronous web server at TCP port 80.
// Docs: https://github.com/me-no-dev/ESPAsyncWebServer
AsyncWebServer httpServer(80);

// Define a websocket endpoint for the async web server.
AsyncWebSocket ws("/ws");

// Track the number of connected WebSocket clients.
uint8_t i_ws_client_count = 0;

// Track time to refresh progress for OTA updates.
unsigned long i_progress_millis = 0;

// Create timer for WebSocket cleanup.
millisDelay ms_cleanup;
const unsigned int i_websocketCleanup = 5000;

boolean startWiFi() {
  // Begin some diagnostic information to console.
  String macAddr = String(WiFi.macAddress());
  #if defined(DEBUG_WIRELESS_SETUP)
    Serial.println();
    Serial.println("Starting Wireless Access Point");
    Serial.print("Device WiFi MAC Address: ");
    Serial.println(macAddr);
  #endif

  // Create an AP name unique to this device, to avoid stepping on others.
  String ap_ssid_suffix = macAddr.substring(12, 14) + macAddr.substring(15);

  // Prepare to return either stored preferences or a default value for SSID/password.
  preferences.begin("credentials", true); // Access namespace in read-only mode.
  #if defined(RESET_AP_SETTINGS)
    // Doesn't actually "reset" but forces default values for SSID and password.
    // Meant to allow the user to reset their credentials then re-flash after
    // commenting out the RESET_AP_SETTINGS definition in Configuration.h
    ap_ssid = ap_ssid_prefix + "_" + ap_ssid_suffix;
    ap_pass = ap_default_passwd;
  #else
    // Use either the stored preferences or an expected default value.
    ap_ssid = preferences.getString("ssid", ap_ssid_prefix + "_" + ap_ssid_suffix);
    ap_pass = preferences.getString("password", ap_default_passwd);
  #endif
  preferences.end();

  // Start the access point using the SSID and password.
  bool b_ap_started = WiFi.softAP(ap_ssid.c_str(), ap_pass.c_str());
  #if defined(DEBUG_WIRELESS_SETUP)
    Serial.println(b_ap_started ? "AP Ready" : "AP Failed");
  #endif
  return b_ap_started;
}

void configureNetwork() {
  // Simple networking info for the AP.
  IPAddress local_ip(192, 168, 1, 2);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);

  // Set networking info and report to console.
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  #if defined(DEBUG_WIRELESS_SETUP)
    IPAddress IP = WiFi.softAPIP();
    Serial.print("Access Point IP Address: ");
    Serial.println(IP);
    Serial.println("WiFi AP Started as " + ap_ssid);
    Serial.println("WiFi AP Password: " + ap_pass);
  #endif
}

/*
 * Text Helper Functions - Converts ENUM values to user-friendly text
 */

String getMode() {
  switch(ARMING_MODE) {
    case MODE_SUPERHERO:
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
  if(ARMING_MODE == MODE_ORIGINAL) {
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
    // Otherwise, just "ready".
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
  switch(FIRING_MODE) {
    case PROTON:
      return "Proton";
    break;
    case SLIME:
      return "Slime";
    break;
    case STASIS:
      return "Stasis";
    break;
    case MESON:
      return "Meson";
    break;
    case SPECTRAL:
      return "Spectral";
    break;
    case HOLIDAY:
      return "Holiday";
    break;
    case SPECTRAL_CUSTOM:
      return "Custom";
    break;
    case VENTING:
      return "Venting";
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
      return (b_overheating ? "Recovery" : "Normal");
    break;
    case 2:
      return "Active";
    break;
    case 3:
      return "Warning";
    break;
    default:
      return "Critical";
    break;
  }
}

/*
 * Web Handler Functions - Performs actions or returns data for web UI
 */
StaticJsonDocument<512> jsonDoc; // Used for processing JSON body data.
StaticJsonDocument<16> jsonSuccess; // Used for sending JSON status as success.
String status; // Holder for simple "status: success" response.

void handleRoot(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  debug("Web Root HTML Requested");
  String s = INDEX_page; // Read HTML page into String.
  request->send(200, "text/html", s); // Serve page content.
}

void handlePassword(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  debug("Password HTML Requested");
  String s = PASSWORD_page; // Read HTML page into String.
  request->send(200, "text/html", s); // Serve page content.
}

void handleStyle(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  debug("Main StyleSheet Requested");
  String s = STYLE_page; // Read CSS page into String.
  request->send(200, "text/css", s); // Serve page content.
}

String getEquipmentStatus() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipStatus;
  jsonDoc.clear();
  jsonDoc["mode"] = getMode();
  jsonDoc["theme"] = getTheme();
  jsonDoc["switch"] = getRedSwitch();
  jsonDoc["pack"] = (b_pack_on ? "Powered" : "Idle");
  jsonDoc["power"] = getPower();
  jsonDoc["safety"] = getSafety();
  jsonDoc["wand"] = (b_wand_on ? "Powered" : "Idle");
  jsonDoc["wandMode"] = getWandMode();
  jsonDoc["firing"] = (b_firing ? "Firing" : "Idle");
  jsonDoc["cable"] = (b_pack_alarm ? "Disconnected" : "Connected");
  jsonDoc["cyclotron"] = getCyclotronState();
  jsonDoc["temperature"] = (b_overheating ? "Venting" : "Normal");
  jsonDoc["musicStart"] = i_music_track_min;
  jsonDoc["musicEnd"] = i_music_track_max;
  serializeJson(jsonDoc, equipStatus); // Serialize to string.
  return equipStatus;
}

void handleStatus(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getEquipmentStatus());
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
  if(i_speed_multiplier > 1) {
    // Only send command to pack if cyclotron is not "normal".
    debug("Cancel Overheat Warning");
    attenuatorSerialSend(A_WARNING_CANCELLED);
    request->send(200, "application/json", status);
  } else {
    // Tell the user why the requested action failed.
    String result;
    jsonDoc.clear();
    jsonDoc["status"] = "System not in overheat warning";
    serializeJson(jsonDoc, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
}

void handleManualVent(AsyncWebServerRequest *request) {
  debug("Manual Vent Triggered");
  attenuatorSerialSend(A_MANUAL_OVERHEAT);
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

void handleMusicStartStop(AsyncWebServerRequest *request) {
  debug("Music Start/Stop");
  attenuatorSerialSend(A_MUSIC_START_STOP);
  request->send(200, "application/json", status);
}

void handleMusicPauseResume(AsyncWebServerRequest *request) {
  debug("Music Start/Stop");
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

void handleSelectMusicTrack(AsyncWebServerRequest *request) {
  String c_music_track = "";

  if(request->hasParam("track")) {
    // Get the parameter "track" if it exists (will be a String).
    c_music_track = request->getParam("track")->value();
  }

  if(c_music_track.toInt() != 0 && c_music_track.toInt() >= i_music_track_min) {
    uint16_t i_music_track = c_music_track.toInt();
    debug("Selected Music Track: " + String(i_music_track));
    attenuatorSerialSend(i_music_track); // Inform the pack of the new track.
    request->send(200, "application/json", status);
  }
  else {
    // Tell the user why the requested action failed.
    String result;
    jsonDoc.clear();
    jsonDoc["status"] = "Invalid track number requested";
    serializeJson(jsonDoc, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
}

void handleNotFound(AsyncWebServerRequest *request) {
  // Returned for any invalid URL requested.
  debug("Web page not found");
  request->send(404, "text/plain", "Not Found");
}

void setupRouting() {
  // Define the endpoints for the web server.

  // Static Pages
  httpServer.on("/", HTTP_GET, handleRoot);
  httpServer.on("/password", HTTP_GET, handlePassword);
  httpServer.on("/style.css", HTTP_GET, handleStyle);

  // AJAX Handlers
  httpServer.on("/status", HTTP_GET, handleStatus);
  httpServer.on("/pack/on", HTTP_PUT, handlePackOn);
  httpServer.on("/pack/off", HTTP_PUT, handlePackOff);
  httpServer.on("/pack/attenuate", HTTP_PUT, handleAttenuatePack);
  httpServer.on("/pack/vent", HTTP_PUT, handleManualVent);
  httpServer.on("/volume/toggle", HTTP_PUT, handleToggleMute);
  httpServer.on("/volume/master/up", HTTP_PUT, handleMasterVolumeUp);
  httpServer.on("/volume/master/down", HTTP_PUT, handleMasterVolumeDown);
  httpServer.on("/volume/effects/up", HTTP_PUT, handleEffectsVolumeUp);
  httpServer.on("/volume/effects/down", HTTP_PUT, handleEffectsVolumeDown);
  httpServer.on("/music/startstop", HTTP_PUT, handleMusicStartStop);
  httpServer.on("/music/pauseresume", HTTP_PUT, handleMusicPauseResume);
  httpServer.on("/music/next", HTTP_PUT, handleNextMusicTrack);
  httpServer.on("/music/select", HTTP_PUT, handleSelectMusicTrack);
  httpServer.on("/music/prev", HTTP_PUT, handlePrevMusicTrack);

  // Handle the JSON body for the password change request.
  AsyncCallbackJsonWebHandler *passwordChangeHandler = new AsyncCallbackJsonWebHandler("/password/update", [](AsyncWebServerRequest *request, JsonVariant &json) {
    StaticJsonDocument<256> jsonBody;
    if(json.is<JsonObject>()) {
      jsonBody = json.as<JsonObject>();
    }
    else {
      Serial.print("Body was not a JSON object");
    }

    String result;
    if(jsonBody.containsKey("password")) {
      String newPasswd = jsonBody["password"];
      //Serial.println("New AP Password: " + newPasswd);

      if(newPasswd != "") {
        preferences.begin("credentials", false); // Access namespace in read/write mode.
        preferences.putString("ssid", ap_ssid); // Store SSID in case this was changed.
        preferences.putString("password", newPasswd); // Store user-provided password.
        preferences.end();

        jsonBody.clear();
        jsonBody["status"] = "Password updated, rebooting controller. Please enter your new WiFi password when prompted by your device.";
        serializeJson(jsonBody, result); // Serialize to string.
        request->send(200, "application/json", result);
        delay(1000); // Pause to allow response to flow.
        ESP.restart(); // Reboot device
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
  httpServer.addHandler(passwordChangeHandler);

  httpServer.onNotFound(handleNotFound);
}

void onWebSocketEventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
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
        Serial.println("WebSocket Data Received");
      #endif
      // Do something when data is received via WebSocket.
    break;
  }
}

void onOTAStart() {
  // Log when OTA has started
  debug("OTA update started");
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - i_progress_millis > 1000) {
    i_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    debug("OTA update finished successfully!");
  } else {
    debug("There was an error during OTA update!");
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

// Send notification to all websocket clients.
void notifyWSClients() {
  // Send latest status to all connected clients.
  ws.textAll(getEquipmentStatus());
}
