/**
 *   GPStar BeltGizmo - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2024-2025 Dustin Grau <dustin.grau@gmail.com>
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
#include <WebSocketsClient.h>

// Declare the external binary data markers for embedded files.
// common.js
extern const uint8_t _binary_assets_common_js_gz_start[];
extern const uint8_t _binary_assets_common_js_gz_end[];
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

// Define standard ports and URI endpoints.
const uint16_t WS_PORT = 80; // Web Server (+WebSocket) port
const char WS_URI[] = "/ws"; // WebSocket endpoint URI
bool b_httpd_started = false; // Denotes the web server has been started.

/**
 * Define a WebSocket client connection and related variables.
 * This should be a standard GPStar Proton Pack wireless device at 192.168.1.2,
 * which means our local network needs to differ and so this device will be
 * available at 192.168.2.2 by default on the private (local) network.
 */
WebSocketsClient wsClient;
const char WS_HOST[] = "192.168.1.2";  // WebSocket server IP
bool b_socket_ready = false;           // WS client socket ready
uint16_t i_websocket_retry_wait = 500; // Delay for WS retry

// Define an asynchronous web server at TCP port 80.
AsyncWebServer httpServer(WS_PORT);

// Define a websocket endpoint for the async web server.
AsyncWebSocket ws(WS_URI);

// Track the number of connected WebSocket clients.
uint8_t i_ws_client_count = 0;

// Track time to refresh progress for OTA updates.
unsigned long i_progress_millis = 0;

// Create timer for WebSocket cleanup.
millisDelay ms_cleanup;
const uint16_t i_websocketCleanup = 5000;

// Forward function declarations.
void setupRouting();
void notifyWSClients();

/*
 * API Helper Functions
 */

/**
 * JSON Body Helpers - Creates stringified JSON representations of device configurations
 */

String getDeviceConfig() {
  // Prepare a JSON object with information we have gleaned from the system.
  String equipSettings;
  JsonDocument jsonBody;

  // Provide current values for the device.
  jsonBody["buildDate"] = build_date;
  jsonBody["wifiName"] = wirelessMgr->getLocalNetworkName();
  jsonBody["wifiNameExt"] = wirelessMgr->getExtWifiNetworkName();
  jsonBody["extAddr"] = wirelessMgr->getExtWifiAddress().toString();
  jsonBody["extMask"] = wirelessMgr->getExtWifiSubnet().toString();

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getEquipmentStatus() {
  // Prepare a JSON object with information we have gleaned from the system.
  String equipStatus;
  JsonDocument jsonBody;

  jsonBody["mode"] = wsData.mode;
  jsonBody["theme"] = wsData.theme;
  jsonBody["switch"] = wsData.switchState;
  jsonBody["pack"] = wsData.pack;
  jsonBody["safety"] = wsData.safety;
  jsonBody["power"] = wsData.wandPower;
  jsonBody["wandMode"] = wsData.wandMode;
  jsonBody["firing"] = wsData.firing;
  jsonBody["cable"] = wsData.cable;
  jsonBody["cyclotron"] = wsData.cyclotron;
  jsonBody["temperature"] = wsData.temperature;
  jsonBody["apClients"] = i_ap_client_count;
  jsonBody["wsClients"] = i_ws_client_count;
  jsonBody["extWifiEnabled"] = wirelessMgr->isExtWifiEnabled();
  jsonBody["extWifiPaused"] = b_ext_wifi_paused;
  jsonBody["extWifiStarted"] = b_ext_wifi_started;

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipStatus);
  return equipStatus;
}

String getWifiSettings() {
  // Prepare a JSON object with information stored in preferences (or a blank default).
  String wifiNetwork;
  JsonDocument jsonBody;

  // Create Preferences object to handle non-volatile storage (NVS).
  Preferences preferences;

  // Accesses namespace in read-only mode.
  if(preferences.begin("network", true)) {
    jsonBody["enabled"] = preferences.getBool("enabled", false);
    jsonBody["network"] = preferences.getString("ssid");
    jsonBody["password"] = preferences.getString("password");

    jsonBody["address"] = preferences.getString("address");
    if(jsonBody["address"].as<String>() == "") {
      jsonBody["address"] = wirelessMgr->getExtWifiAddress().toString();
    }

    jsonBody["subnet"] = preferences.getString("subnet");
    if(jsonBody["subnet"].as<String>() == "") {
      jsonBody["subnet"] = wirelessMgr->getExtWifiSubnet().toString();
    }

    jsonBody["gateway"] = preferences.getString("gateway");
    if(jsonBody["gateway"].as<String>() == "") {
      jsonBody["gateway"] = wirelessMgr->getExtWifiGateway().toString();
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

// Return a small JSON object with a "status" property: {"status":"<value>"}
// This returns the provided status string verbatim (no escaping or modification).
String returnJsonStatus(const String &status = String("success")) {
  String s_out;
  s_out.reserve(status.length() + 16); // Reserve space to avoid multiple allocations.
  s_out = "{\"status\":\"";
  s_out += status; // Append status value.
  s_out += "\"}";
  return s_out;
}

void startWebServer() {
  // Configures URI routing with function handlers.
  setupRouting();

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

// Act upon data sent via the websocket (as a client).
void webSocketClientEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      debugln("Client WebSocket Disconnected!");
      b_socket_ready = false;
      wsClient.disconnect();
      delay(200); // Short delay before reconnecting.
      wsClient.begin(WS_HOST, WS_PORT, WS_URI);
      wsClient.setReconnectInterval(i_websocket_retry_wait);
    break;

    case WStype_CONNECTED:
      debugf("WebSocket Connected to url: %s\n", payload);
      digitalWrite(BUILT_IN_LED, HIGH); // Turn on the built-in LED.
      b_socket_ready = true;
      wsClient.sendTXT("Hello from Belt Gizmo");
    break;
    case WStype_ERROR:
      debugf("WebSocket Error: %s\n", payload);
    break;

    case WStype_TEXT:
      /*
      * Deserialize incoming JSON String from remote websocket server.
      * NOTE: Some data from the Attenuator/Wireless may be plain text
      * which will cause an error to be thrown. Only continue when no
      * error is present from deserialization.
      */
      JsonDocument jsonBody;
      DeserializationError jsonError = deserializeJson(jsonBody, payload);
      if(!jsonError) {
        // Store values as a known datatype (String).
        wsData.mode = jsonBody["mode"].as<String>();
        wsData.theme = jsonBody["theme"].as<String>();
        wsData.switchState = jsonBody["switch"].as<String>();
        wsData.pack = jsonBody["pack"].as<String>();
        wsData.safety = jsonBody["safety"].as<String>();
        wsData.wandPower = jsonBody["power"].as<unsigned short>(); // Only integer value.
        wsData.wandMode = jsonBody["wandMode"].as<String>();
        wsData.firing = jsonBody["firing"].as<String>();
        wsData.cable = jsonBody["cable"].as<String>();
        wsData.cyclotron = jsonBody["cyclotron"].as<String>();
        wsData.temperature = jsonBody["temperature"].as<String>();

        // Output some data to the serial console when needed.
        debugln(wsData.wandMode + " is " + wsData.firing + " at level " + String(wsData.wandPower));

        // Change LED for testing
        if(wsData.firing == "Firing") {
          b_firing = true;
        }
        else {
          b_firing = false;
        }

        // Skip further mode changes if in self-test mode.
        if(b_testing) {
          return;
        }

        // Always keep up with the current stream mode.
        if(wsData.wandMode == "Proton Stream") {
          STREAM_MODE = PROTON;
        }
        else if(wsData.wandMode == "Plasm System") {
          STREAM_MODE = SLIME;
        }
        else if(wsData.wandMode == "Dark Matter Gen.") {
          STREAM_MODE = STASIS;
        }
        else if(wsData.wandMode == "Particle System") {
          STREAM_MODE = MESON;
        }
        else if(wsData.wandMode == "Spectral Stream") {
          STREAM_MODE = SPECTRAL;
        }
        else if(wsData.wandMode == "Halloween") {
          STREAM_MODE = HOLIDAY_HALLOWEEN;
        }
        else if(wsData.wandMode == "Christmas") {
          STREAM_MODE = HOLIDAY_CHRISTMAS;
        }
        else if(wsData.wandMode == "Settings") {
          STREAM_MODE = SETTINGS;
        }
        else {
          STREAM_MODE = SPECTRAL_CUSTOM; // Custom Stream
        }

        notifyWSClients(); // Update local WebSocket clients
      }
    break;
  }
}

// Function to setup WebSocket connection.
void setupWebSocketClient() {
  debugln(F("Initializing WebSocket Client Connection..."));
  wsClient.begin(WS_HOST, WS_PORT, WS_URI);
  wsClient.setReconnectInterval(i_websocket_retry_wait);
  wsClient.onEvent(webSocketClientEvent);
  b_socket_ready = true;
}

/**
 * Standard Page Handlers - Delivers the main web pages and common content
 */

// Function: embeddedFileSize
// Purpose:  Compute the size (in bytes) of an embedded binary asset using
//           the linker-provided start/end markers generated for each asset.
// Inputs:
//   - start: pointer to the first byte (e.g. _binary_assets_<file>_start)
//   - end:   pointer to the one-past-last byte (e.g. _binary_assets_<file>_end)
// Outputs:
//   - size_t: number of bytes in the embedded asset (0 on invalid pointers or if end <= start)
inline size_t embeddedFileSize(const uint8_t* start, const uint8_t* end) {
  if (start == nullptr || end == nullptr) return 0;
  if (end <= start) return 0;
  return (size_t)(end - start);
}

void handleRoot(AsyncWebServerRequest *request) {
  // Used for the root page (/ = index.html) from the web server.
  debugln("Sending -> Index HTML");
  size_t i_file_len = embeddedFileSize(_binary_assets_index_html_gz_start, _binary_assets_index_html_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", _binary_assets_index_html_gz_start, i_file_len);
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  response->addHeader("Content-Encoding", "gzip"); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handleRootJS(AsyncWebServerRequest *request) {
  // Used for the root page (/ = index.js) from the web server.
  debugln("Sending -> Index JavaScript");
  size_t i_file_len = embeddedFileSize(_binary_assets_index_js_gz_start, _binary_assets_index_js_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(200, "application/javascript; charset=UTF-8", _binary_assets_index_js_gz_start, i_file_len);
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  response->addHeader("Content-Encoding", "gzip"); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handleCommonJS(AsyncWebServerRequest *request) {
  // Used for all pages (common.js) from the web server.
  debugln("Sending -> Common JavaScript");
  size_t i_file_len = embeddedFileSize(_binary_assets_common_js_gz_start, _binary_assets_common_js_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(200, "application/javascript; charset=UTF-8", _binary_assets_common_js_gz_start, i_file_len);
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  response->addHeader("Content-Encoding", "gzip"); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handleStylesheet(AsyncWebServerRequest *request) {
  // Used for the common stylesheet of the web server.
  debugln("Sending -> Main StyleSheet");
  size_t i_file_len = embeddedFileSize(_binary_assets_style_css_gz_start, _binary_assets_style_css_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(200, "text/css", _binary_assets_style_css_gz_start, i_file_len);
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  response->addHeader("Content-Encoding", "gzip"); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handleFavIco(AsyncWebServerRequest *request) {
  // Used for the favicon of the web server.
  debugln("Sending -> Favicon");
  size_t i_file_len = embeddedFileSize(_binary_assets_favicon_ico_gz_start, _binary_assets_favicon_ico_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(200, "image/x-icon", _binary_assets_favicon_ico_gz_start, i_file_len);
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  response->addHeader("Content-Encoding", "gzip"); // Tell the client this is gzipped content.
  request->send(response); // Serve gzipped .ico file.
}

void handleFavSvg(AsyncWebServerRequest *request) {
  // Used for the favicon of the web server.
  debugln("Sending -> Favicon");
  size_t i_file_len = embeddedFileSize(_binary_assets_favicon_svg_gz_start, _binary_assets_favicon_svg_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(200, "image/svg+xml", _binary_assets_favicon_svg_gz_start, i_file_len);
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  response->addHeader("Content-Encoding", "gzip"); // Tell the client this is gzipped content.
  request->send(response); // Serve gzipped .svg file.
}

void handleNetwork(AsyncWebServerRequest *request) {
  // Used for the network page from the web server.
  debugln("Sending -> Network HTML");
  size_t i_file_len = embeddedFileSize(_binary_assets_network_html_gz_start, _binary_assets_network_html_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", _binary_assets_network_html_gz_start, i_file_len);
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  response->addHeader("Content-Encoding", "gzip"); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handlePassword(AsyncWebServerRequest *request) {
  // Used for the password page from the web server.
  debugln("Sending -> Password HTML");
  size_t i_file_len = embeddedFileSize(_binary_assets_password_html_gz_start, _binary_assets_password_html_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", _binary_assets_password_html_gz_start, i_file_len);
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  response->addHeader("Content-Encoding", "gzip"); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

void handleDeviceSettings(AsyncWebServerRequest *request) {
  // Used for the device page from the web server.
  debugln("Sending -> Device Settings HTML");
  size_t i_file_len = embeddedFileSize(_binary_assets_device_html_gz_start, _binary_assets_device_html_gz_end);
  AsyncWebServerResponse *response = request->beginResponse(200, "text/html", _binary_assets_device_html_gz_start, i_file_len);
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  response->addHeader("Content-Encoding", "gzip"); // Tell the client this is gzipped content.
  request->send(response); // Serve page content.
}

/**
 * Peripheral Page Handlers - Delivers the preference pages for available peripherals
 */

void handleGetDeviceConfig(AsyncWebServerRequest *request) {
  // Return current device settings as a stringified JSON object.
  request->send(200, "application/json", getDeviceConfig());
}

void handleGetStatus(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getEquipmentStatus());
}

void handleGetWifi(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getWifiSettings());
}

void handleGetSSIDs(AsyncWebServerRequest *request) {
  // Prepare a JSON object with an array of WiFi networks nearby.
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
  request->send(200, "application/json", wifiNetworks);
}

void handleRestart(AsyncWebServerRequest *request) {
  // Performs a restart of the device.
  request->send(204, "application/json", returnJsonStatus());
  delay(1000);
  ESP.restart();
}

/**
 * Action Handlers - Perform specific actions via web requests
 */

void handleRestartWiFi(AsyncWebServerRequest *request) {
  // Performs a restart of the external WiFi.

  // Disconnect from the WiFi network and re-apply any changes.
  WiFi.disconnect();
  b_ext_wifi_started = false;
  notifyWSClients();

  delay(100); // Delay needed.

  b_ext_wifi_started = startExternalWifi(); // Restart and set global flag.
  if(b_ext_wifi_started) {
    request->send(200, "application/json", returnJsonStatus("WiFi connection restarted successfully."));
  }
  else {
      request->send(200, "application/json", returnJsonStatus("WiFi connection was not successful."));
  }
}

void handleEnableSelfTest(AsyncWebServerRequest *request) {
  debugln("Web: Self Test Enabled");
  if(STREAM_MODE != SELFTEST) {
    STREAM_MODE_PREV = STREAM_MODE; // Save current mode.
    STREAM_MODE = SELFTEST; // Switch to self-test mode.
    b_testing = true; // Enable testing flag.

    // Simulate firing at full power for testing.
    wsData.wandPower = 5;
    b_firing = true;
  }
  request->send(200, "application/json", returnJsonStatus());
}

void handleDisableSelfTest(AsyncWebServerRequest *request) {
  debugln("Web: Self Test Disabled");
  if(STREAM_MODE == SELFTEST) {
    STREAM_MODE = STREAM_MODE_PREV; // Restore previous mode.
    b_testing = false; // Disable testing flag.

    // Return to default power level and firing state.
    wsData.wandPower = 1;
    b_firing = false;
  }
  request->send(200, "application/json", returnJsonStatus());
}

/**
 * Body Handler Methods - These handlers process JSON body content from POST requests
 */

// Handles the JSON body for the device settings save request.
AsyncCallbackJsonWebHandler *handleSaveDeviceConfig = new AsyncCallbackJsonWebHandler("/config/device/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  JsonDocument jsonBody;
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    debugln(F("Body was not a JSON object"));
  }

  String result;
  try {
    // First check if a new private WiFi network name has been chosen.
    String newSSID = jsonBody["wifiName"].as<String>();
    newSSID = sanitizeSSID(newSSID); // Jacques, clean him!
    bool b_ssid_changed = false;

    // Create Preferences object to handle non-volatile storage (NVS).
    Preferences preferences;

    // Update the private network name ONLY if the new value differs from the current SSID.
    if(newSSID != "" && newSSID != wirelessMgr->getLocalNetworkName()){
      if(newSSID.length() >= 8 && newSSID.length() <= 32) {
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
        request->send(200, "application/json", returnJsonStatus("Error: Network name must be between 8 and 32 characters in length."));
      }
    }

    if(b_ssid_changed){
      request->send(201, "application/json", returnJsonStatus("Settings updated, restart required. Please use the new network name to connect to your device."));
    }
    else {
      request->send(200, "application/json", returnJsonStatus("Settings updated."));
    }
  }
  catch (...) {
    request->send(200, "application/json", returnJsonStatus("An error was encountered while saving settings."));
  }
}); // handleSaveDeviceConfig

// Handles the JSON body for the password change request.
AsyncCallbackJsonWebHandler *passwordChangeHandler = new AsyncCallbackJsonWebHandler("/password/update", [](AsyncWebServerRequest *request, JsonVariant &json) {
  JsonDocument jsonBody;
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    debugln("Body was not a JSON object");
  }

  String result;
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

      request->send(201, "application/json", returnJsonStatus("Password updated, restart required. Please enter your new WiFi password when prompted by your device."));
    }
    else {
      // Password must be at least 8 characters in length.
      request->send(200, "application/json", returnJsonStatus("Password must be a minimum of 8 characters to meet WPA2 requirements."));
    }
  }
  else {
    debugln("No password in JSON body");
    request->send(200, "application/json", returnJsonStatus("Unable to update password."));
  }
}); // passwordChangeHandler

// Handles the JSON body for the wifi network info.
AsyncCallbackJsonWebHandler *wifiChangeHandler = new AsyncCallbackJsonWebHandler("/wifi/update", [](AsyncWebServerRequest *request, JsonVariant &json) {
  JsonDocument jsonBody;
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    debugln("Body was not a JSON object");
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

    // Create Preferences object to handle non-volatile storage (NVS).
    Preferences preferences;

    // Accesses namespace in read/write mode.
    if(preferences.begin("network", false)) {
      // Store the state of toggle switches regardless.
      preferences.putBool("enabled", b_enabled);

      if(wifiNetwork.length() >= 2 && wifiPasswd.length() >= 8) {
        // Clear old network IP info if SSID or password have been changed.
        String old_ssid = preferences.getString("ssid", "");
        String old_passwd = preferences.getString("password", "");
        if(old_ssid == "" || old_ssid != wifiNetwork || old_passwd == "" || old_passwd != wifiPasswd) {
          preferences.putString("address", "");
          preferences.putString("subnet", "");
          preferences.putString("gateway", "");
        }

        // Store the critical values to enable/disable the external WiFi.
        preferences.putString("ssid", wifiNetwork);
        preferences.putString("password", wifiPasswd);

        // Continue saving only if network values are 7 characters or more (eg. N.N.N.N)
        bool b_static_ip = true;
        if(localAddr.length() >= 7 && localAddr != wirelessMgr->getExtWifiAddress().toString()) {
          preferences.putString("address", localAddr);
        }
        else {
          b_static_ip = false;
        }
        if(subnetMask.length() >= 7 && subnetMask != wirelessMgr->getExtWifiSubnet().toString()) {
          preferences.putString("subnet", subnetMask);
        }
        else {
          b_static_ip = false;
        }
        if(gatewayIP.length() >= 7 && gatewayIP != wirelessMgr->getExtWifiGateway().toString()) {
          preferences.putString("gateway", gatewayIP);
        }
        else {
          b_static_ip = false;
        }
        if(!b_static_ip) {
          // If any of the above values were invalid, blank all three.
          preferences.putString("address", "");
          preferences.putString("subnet", "");
          preferences.putString("gateway", "");
        }
      }
      else {
        // Reset all values to defaults.
        preferences.putString("ssid", "");
        preferences.putString("password", "");
        preferences.putString("address", "");
        preferences.putString("subnet", "");
        preferences.putString("gateway", "");
      }

      preferences.end();
    }
    else {
      b_errors = true;
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

      request->send(200, "application/json", returnJsonStatus(s_reason));
    }
    else {
      request->send(200, "application/json", returnJsonStatus("Errors encountered while processing request data. Please re-check submitted values and try again."));
    }
  }
  else {
    debugln("No password in JSON body");
    request->send(200, "application/json", returnJsonStatus("Unable to update password."));
  }
}); // wifiChangeHandler

void handleNotFound(AsyncWebServerRequest *request) {
  // Returned for any invalid URL requested.
  debugln("Web page not found");
  request->send(404, "text/plain", "Not Found");
}

// Define all known URI endpoints for the web server.
// Declare this last as it uses all of the above functions.
void setupRouting() {
  // Static Pages
  httpServer.on("/", HTTP_GET, handleRoot);
  httpServer.on("/common.js", HTTP_GET, handleCommonJS);
  httpServer.on("/favicon.ico", HTTP_GET, handleFavIco);
  httpServer.on("/favicon.svg", HTTP_GET, handleFavSvg);
  httpServer.on("/style.css", HTTP_GET, handleStylesheet);
  httpServer.on("/index.js", HTTP_GET, handleRootJS);
  httpServer.on("/network", HTTP_GET, handleNetwork);
  httpServer.on("/password", HTTP_GET, handlePassword);
  httpServer.on("/settings/device", HTTP_GET, handleDeviceSettings);
  httpServer.onNotFound(handleNotFound);

  // Get/Set Handlers
  httpServer.on("/config/device", HTTP_GET, handleGetDeviceConfig);
  httpServer.on("/status", HTTP_GET, handleGetStatus);
  httpServer.on("/restart", HTTP_DELETE, handleRestart);
  httpServer.on("/wifi/restart", HTTP_GET, handleRestartWiFi);
  httpServer.on("/wifi/settings", HTTP_GET, handleGetWifi);
  httpServer.on("/wifi/networks", HTTP_GET, handleGetSSIDs);
  httpServer.on("/selftest/enable", HTTP_PUT, handleEnableSelfTest);
  httpServer.on("/selftest/disable", HTTP_PUT, handleDisableSelfTest);

  // Body Handlers
  httpServer.addHandler(handleSaveDeviceConfig); // /config/device/save
  httpServer.addHandler(passwordChangeHandler); // /password/update
  httpServer.addHandler(wifiChangeHandler); // /wifi/update
}
