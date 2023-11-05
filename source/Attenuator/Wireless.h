/**
 *   gpstar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
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
#include <ESPAsyncWebSrv.h>
#include <Preferences.h>
#include <WiFi.h>

// Web page files (HTML as char[])
#include "index.h"
#include "password.h"

// Preferences for SSID and AP password, which will use a "credentials" namespace.
Preferences preferences;

// Set up values for the SSID and password for the WiFi access point (AP).
const String ap_ssid_prefix = "ProtonPack"; // This will be the base of the SSID name.
String ap_default_passwd = "555-2368"; // This will be the default password for the AP.
String ap_ssid; // Reserved for storing the true SSID for the AP to be set at startup.
String ap_pass; // Reserved for storing the true AP password set by the user.

// Define an asynchronous web server at TCP port 80.
AsyncWebServer httpServer(80);

// Define a websocket endpoint for the async web server.
AsyncWebSocket ws("/ws");

// Track the number of connected WebSocket clients.
uint8_t i_ws_client_count = 0;

boolean startAccessPoint() {
  // Begin some diagnostic information to console.
  Serial.println();
  Serial.println("Starting Wireless Access Point");
  String macAddr = String(WiFi.macAddress());
  Serial.print("Device WiFi MAC Address: ");
  Serial.println(macAddr);

  // Create an AP name unique to this device, to avoid stepping on others.
  String ap_ssid_suffix = macAddr.substring(12, 14) + macAddr.substring(15);
  //ap_ssid = ap_ssid_prefix + "_" + ap_ssid_suffix; // Update AP broadcast name.

  // Prepare to return either stored preferences or a default value for SSID/password.
  preferences.begin("credentials", true); // Access namespace in read-only mode.
  ap_ssid = preferences.getString("ssid", ap_ssid_prefix + "_" + ap_ssid_suffix);
  ap_pass = preferences.getString("password", ap_default_passwd);
  preferences.end();

  // Start the access point using the SSID and password.
  return WiFi.softAP(ap_ssid.c_str(), ap_pass.c_str());
}

void configureNetwork() {
  // Simple networking info for the AP.
  IPAddress local_ip(192, 168, 1, 2);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);

  // Set networking info and report to console.
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  Serial.print("Access Point IP Address: ");
  IPAddress IP = WiFi.softAPIP();
  Serial.println(IP);
  Serial.print("WiFi AP Started as ");
  Serial.println(ap_ssid);
  Serial.print("WiFi AP Password: ");
  Serial.println(ap_pass);
}

// Create timers for server-push events.
millisDelay ms_cleanup;
const unsigned int i_websocketCleanup = 5000;

/*
 * Text Helper Functions - Converts ENUM values to user-friendly text
 */

String getTheme() {
  switch(YEAR_MODE) {
    case YEAR_1984:
      return "1984";
    break;
    case YEAR_1989:
      return "1989";
    break;
    case YEAR_2021:
      return "Afterlife";
    break;
    default:
      return "Unknown";
    break;
  }
}

String getMode() {
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
StaticJsonDocument<256> jsonDoc; // Used for processing JSON data.

void handleRoot(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  Serial.println("Web Root HTML Requested");
  String s = MAIN_page; // Read HTML contents from .h file.
  request->send(200, "text/html", s); // Send index page.
}

void handlePassword(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  Serial.println("Password HTML Requested");
  String s = PSWD_page; // Read HTML contents from .h file.
  request->send(200, "text/html", s); // Send password page.
}

String getStatus() {
  jsonDoc.clear();
  jsonDoc["theme"] = getTheme();
  jsonDoc["mode"] = getMode();
  jsonDoc["pack"] = (b_pack_on ? "Powered" : "Idle");
  jsonDoc["power"] = getPower();
  jsonDoc["wand"] = (b_firing ? "Firing" : "Idle");
  jsonDoc["cable"] = (b_pack_alarm ? "Disconnected" : "Connected");
  jsonDoc["cyclotron"] = getCyclotronState();
  jsonDoc["temperature"] = (b_overheating ? "Venting" : "Normal");
  String status;
  serializeJson(jsonDoc, status); // Serialize to string.
  return status;
}

void handleStatus(AsyncWebServerRequest *request) {
  // Return data for AJAX requests by the index page.
  request->send(200, "application/json", getStatus());
}

void handlePackOn(AsyncWebServerRequest *request) {
  Serial.println("Turn Pack On");
  attenuatorSerialSend(A_TURN_PACK_ON);
  request->send(200, "application/json", "{}");
}

void handlePackOff(AsyncWebServerRequest *request) {
  Serial.println("Turn Pack Off");
  attenuatorSerialSend(A_TURN_PACK_OFF);
  request->send(200, "application/json", "{}");
}

void handleCancelWarning(AsyncWebServerRequest *request) {
  if (i_speed_multiplier > 1) {
    // Only send command to pack if cyclotron is not "normal".
    Serial.println("Cancel Overheat Warning");
    attenuatorSerialSend(A_WARNING_CANCELLED);
  }
  request->send(200, "application/json", "{}");
}

void handleToggleMute(AsyncWebServerRequest *request) {
  Serial.println("Toggle Mute");
  attenuatorSerialSend(A_TOGGLE_MUTE);
  request->send(200, "application/json", "{}");
}

void handleMasterVolumeUp(AsyncWebServerRequest *request) {
  Serial.println("Master Volume Up");
  attenuatorSerialSend(A_VOLUME_INCREASE);
  request->send(200, "application/json", "{}");
}

void handleMasterVolumeDown(AsyncWebServerRequest *request) {
  Serial.println("Master Volume Down");
  attenuatorSerialSend(A_VOLUME_DECREASE);
  request->send(200, "application/json", "{}");
}

void handleEffectsVolumeUp(AsyncWebServerRequest *request) {
  Serial.println("Effects Volume Up");
  attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_INCREASE);
  request->send(200, "application/json", "{}");
}

void handleEffectsVolumeDown(AsyncWebServerRequest *request) {
  Serial.println("Effects Volume Down");
  attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_DECREASE);
  request->send(200, "application/json", "{}");
}

void handleMusicStartStop(AsyncWebServerRequest *request) {
  Serial.println("Music Start/Stop");
  attenuatorSerialSend(A_MUSIC_START_STOP);
  request->send(200, "application/json", "{}");
}

void handleNextMusicTrack(AsyncWebServerRequest *request) {
  Serial.println("Next Music Track");
  attenuatorSerialSend(A_MUSIC_NEXT_TRACK);
  request->send(200, "application/json", "{}");
}

void handlePrevMusicTrack(AsyncWebServerRequest *request) {
  Serial.println("Prev Music Track");
  attenuatorSerialSend(A_MUSIC_PREV_TRACK);
  request->send(200, "application/json", "{}");
}

void handleNotFound(AsyncWebServerRequest *request) {
  // Returned for any invalid URL requested.
  Serial.println("Web page not found");
  request->send(404, "text/plain", "Not Found");
}

void setupRouting() {
  // Define the endpoints for the web server.

  // Static Pages
  httpServer.on("/", HTTP_GET, handleRoot);
  httpServer.on("/password", HTTP_GET, handlePassword);

  // AJAX Handlers
  httpServer.on("/status", HTTP_GET, handleStatus);
  httpServer.on("/pack/on", HTTP_GET, handlePackOn);
  httpServer.on("/pack/off", HTTP_GET, handlePackOff);
  httpServer.on("/pack/cancel", HTTP_GET, handleCancelWarning);
  httpServer.on("/volume/toggle", HTTP_GET, handleToggleMute);
  httpServer.on("/volume/master/up", HTTP_GET, handleMasterVolumeUp);
  httpServer.on("/volume/master/down", HTTP_GET, handleMasterVolumeDown);
  httpServer.on("/volume/effects/up", HTTP_GET, handleEffectsVolumeUp);
  httpServer.on("/volume/effects/down", HTTP_GET, handleEffectsVolumeDown);
  httpServer.on("/music/toggle", HTTP_GET, handleMusicStartStop);
  httpServer.on("/music/next", HTTP_GET, handleNextMusicTrack);
  httpServer.on("/music/prev", HTTP_GET, handlePrevMusicTrack);

  // Handle the JSON body for the password change request.
  AsyncCallbackJsonWebHandler *passwordChangeHandler = new AsyncCallbackJsonWebHandler("/password/update", [](AsyncWebServerRequest *request, JsonVariant &json) {
    StaticJsonDocument<256> jsonData;
    if (json.is<JsonObject>()) {
      jsonData = json.as<JsonObject>();
    }
    else {
      Serial.print("Body was not a JSON object");
    }

    String result;
    if (jsonData.containsKey("password")) {
      String newPasswd = jsonData["password"];
      Serial.print("New AP Password: ");
      Serial.println(newPasswd);

      if (newPasswd != "") {
        preferences.begin("credentials", false); // Access namespace in read/write mode.
        preferences.putString("ssid", ap_ssid);
        preferences.putString("password", newPasswd);
        preferences.end();

        jsonData.clear();
        jsonData["response"] = "Password updated, rebooting controller. Please enter your new WiFi password when prompted by your device.";
        serializeJson(jsonData, result); // Serialize to string.
        request->send(200, "application/json", result);
        delay(1000); // Pause to allow response to flow.
        ESP.restart(); // Reboot device
      }
    }
    else {
      Serial.println("No password in JSON body");
      jsonData.clear();
      jsonData["response"] = "Unable to update password.";
      serializeJson(jsonData, result); // Serialize to string.
      request->send(200, "application/json", result);
    }
  });
  httpServer.addHandler(passwordChangeHandler);

  httpServer.onNotFound(handleNotFound);
}

void onWebSocketEventHandler(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
  switch(type) {
    case WS_EVT_CONNECT:
      Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
      i_ws_client_count++;
    break;

    case WS_EVT_DISCONNECT:
      Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
      if (i_ws_client_count > 0) {
        i_ws_client_count--;
      }
    break;

    case WS_EVT_ERROR:
      Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
    break;

    case WS_EVT_PONG:
      Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
    break;

    case WS_EVT_DATA:
      Serial.println("WebSocket Data Received");
      // Do something when data is received via WebSocket.
    break;
  }
}

// Define server actions after declaring all functions for URL routing.
void startWebServer() {
  setupRouting(); // Set URI's with handlers.
  ws.onEvent(onWebSocketEventHandler);
  httpServer.addHandler(&ws);
  httpServer.begin(); // Start the daemon.
  Serial.println("HTTP Server Started");
}

// Send notification to all websocket clients.
void notifyWSClients() {
  // Send latest status to all connected clients.
  ws.textAll(getStatus());
}
