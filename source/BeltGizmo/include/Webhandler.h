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

// Web page files (defines all text as char[] variable)
#include "CommonJS.h" // COMMONJS_page
#include "Index.h" // INDEX_page
#include "IndexJS.h" // INDEXJS_page
#include "Device.h" // DEVICE_page
#include "ExtWiFi.h" // NETWORK_page
#include "Password.h" // PASSWORD_page
#include "Style.h" // STYLE_page
#include "Icon.h" // FAVICON_ico, FAVICON_svg

// Forward function declarations.
void setupRouting();

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
        Serial.printf("WebSocket[%s][%lu] Connect\n", server->url(), client->id());
      #endif
      i_ws_client_count++;
    break;

    case WS_EVT_DISCONNECT:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        Serial.printf("WebSocket[%s][C:%lu] Disconnect\n", server->url(), client->id());
      #endif
      if(i_ws_client_count > 0) {
        i_ws_client_count--;
      }
    break;

    case WS_EVT_ERROR:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        Serial.printf("WebSocket[%s][C:%lu] Error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
      #endif
    break;

    case WS_EVT_PONG:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        Serial.printf("WebSocket[%s][C:%lu] Pong[L:%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
      #endif
    break;

    case WS_EVT_DATA:
      #if defined(DEBUG_SEND_TO_CONSOLE)
        Serial.printf("WebSocket[%s][C:%lu] Data[L:%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
      #endif
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

  // Denote that the web server should be started.
  b_ws_started = true;

  #if defined(DEBUG_SEND_TO_CONSOLE)
    Serial.println(F("Async HTTP Server Started"));
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

void handleStylesheet(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  debug("Sending -> Main StyleSheet");
  AsyncWebServerResponse *response = request->beginResponse(200, "text/css", (const uint8_t*)STYLE_page, strlen(STYLE_page));
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  request->send(response); // Serve page content.
}

void handleFavIco(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  debug("Sending -> Favicon");
  AsyncWebServerResponse *response = request->beginResponse(200, "image/x-icon", FAVICON_ico, sizeof(FAVICON_ico));
  response->addHeader("Cache-Control", "no-cache, must-revalidate");
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
}

void handleFavSvg(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  debug("Sending -> Favicon");
  AsyncWebServerResponse *response = request->beginResponse(200, "image/svg+xml", FAVICON_svg, sizeof(FAVICON_svg));
  response->addHeader("Content-Encoding", "gzip");
  request->send(response);
}

String getDeviceConfig() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonBody.clear();

  // Provide current values for the device.
  jsonBody["buildDate"] = build_date;
  jsonBody["wifiName"] = ap_ssid;
  jsonBody["wifiNameExt"] = wifi_ssid;
  jsonBody["extAddr"] = wifi_address;
  jsonBody["extMask"] = wifi_subnet;

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
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

void handleGetDeviceConfig(AsyncWebServerRequest *request) {
  // Return current device settings as a stringified JSON object.
  request->send(200, "application/json", getDeviceConfig());
}

void handleGetWifi(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getWifiSettings());
}

void handleRestartDevice(AsyncWebServerRequest *request) {
  // Performs a restart of the device.
  request->send(204, "application/json", status);
  delay(1000);
  ESP.restart();
}

void handleRestartWiFi(AsyncWebServerRequest *request) {
  // Performs a restart of the external WiFi.
  jsonBody.clear();

  // Disconnect from the WiFi network and re-apply any changes.
  WiFi.disconnect();
  b_ext_wifi_started = false;

  delay(100); // Delay needed.

  b_ext_wifi_started = startExternalWifi(); // Restart and set global flag.
  if(b_ext_wifi_started) {
    jsonBody["status"] = "WiFi connection restarted successfully.";
  }
  else {
    jsonBody["status"] = "WiFi connection was not successful.";
  }

  String result;
  serializeJson(jsonBody, result); // Serialize to string.
  request->send(200, "application/json", result);
}

// Handles the JSON body for the pack settings save request.
AsyncCallbackJsonWebHandler *handleSaveDeviceConfig = new AsyncCallbackJsonWebHandler("/config/device/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
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
    newSSID = sanitizeSSID(newSSID); // Jacques, clean him!
    bool b_ssid_changed = false;

    // Update the private network name ONLY if the new value differs from the current SSID.
    if(newSSID != ap_ssid){
      if(newSSID.length() >= 8 && newSSID.length() <= 32) {
        // Accesses namespace in read/write mode.
        if(preferences.begin("credentials", false)) {
          #if defined(DEBUG_SEND_TO_CONSOLE)
            Serial.print(F("New Private SSID: "));
            Serial.println(newSSID);
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

    if(b_ssid_changed){
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
      // Accesses namespace in read/write mode.
      if(preferences.begin("credentials", false)) {
        #if defined(DEBUG_SEND_TO_CONSOLE)
          Serial.print(F("New Private WiFi Password: "));
          Serial.println(newPasswd);
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
  httpServer.on("/style.css", HTTP_GET, handleStylesheet);
  httpServer.onNotFound(handleNotFound);

  // Get/Set Handlers
  httpServer.on("/config/device", HTTP_GET, handleGetDeviceConfig);
  httpServer.on("/restart", HTTP_DELETE, handleRestartDevice);
  httpServer.on("/wifi/restart", HTTP_GET, handleRestartWiFi);
  httpServer.on("/wifi/settings", HTTP_GET, handleGetWifi);

  // Body Handlers
  httpServer.addHandler(handleSaveDeviceConfig); // /config/device/save
  httpServer.addHandler(passwordChangeHandler); // /password/update
  httpServer.addHandler(wifiChangeHandler); // /wifi/update
}

// Act upon data sent via the websocket (as a client).
void webSocketClientEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      debug("Client WebSocket Disconnected!");
      digitalWrite(BUILT_IN_LED, LOW); // Turn off the built-in LED.
      WiFi.disconnect();
      b_ext_wifi_started = false;
      delay(100); // Delay needed.
    break;

    case WStype_CONNECTED:
      Serial.printf("WebSocket Connected to url: %s\n", payload);
      digitalWrite(BUILT_IN_LED, HIGH); // Turn on the built-in LED.
      b_socket_ready = true;
      wsClient.sendTXT("Hello from Belt Gizmo");
    break;
    case WStype_ERROR:
      Serial.printf("WebSocket Error: %s\n", payload);
    break;

    case WStype_TEXT:
      /*
      * Deserialize incoming JSON String from remote websocket server.
      * NOTE: Some data from the Attenuator/Wireless may be plain text
      * which will cause an error to be thrown. Only continue when no
      * error is present from deserialization.
      */
      DeserializationError jsonError = deserializeJson(jsonBody, payload);
      if (!jsonError) {
        // Store values as a known datatype (String).
        String data_mode = jsonBody["mode"];
        String data_theme = jsonBody["theme"];
        String data_switch = jsonBody["switch"];
        String data_pack = jsonBody["pack"];
        String data_safety = jsonBody["safety"];
        String data_wandPower = jsonBody["wandPower"];
        String data_wandMode = jsonBody["wandMode"];
        String data_firing = jsonBody["firing"];
        String data_cable = jsonBody["cable"];
        String data_ctron = jsonBody["cyclotron"];
        String data_temp = jsonBody["temperature"];

        // Convert power (1-5) to an integer.
        i_power = (int)jsonBody["power"];

        // Output some data to the serial console when needed.
        String dataMessage = data_wandMode + " is " + data_firing + " at level " + i_power;
        debug(dataMessage);

        // Change LED for testing
        if(data_firing == "Firing") {
          b_firing = true;
        }
        else {
          b_firing = false;
        }

        // Always keep up with the current stream mode.
        if(data_wandMode == "Proton Stream") {
          STREAM_MODE = PROTON;
        }
        else if(data_wandMode == "Plasm System") {
          STREAM_MODE = SLIME;
        }
        else if(data_wandMode == "Dark Matter Gen.") {
          STREAM_MODE = STASIS;
        }
        else if(data_wandMode == "Particle System") {
          STREAM_MODE = MESON;
        }
        else if(data_wandMode == "Spectral Stream") {
          STREAM_MODE = SPECTRAL;
        }
        else if(data_wandMode == "Halloween") {
          STREAM_MODE = HOLIDAY_HALLOWEEN;
        }
        else if(data_wandMode == "Christmas") {
          STREAM_MODE = HOLIDAY_CHRISTMAS;
        }
        else if(data_wandMode == "Settings") {
          STREAM_MODE = SETTINGS;
        }
        else {
          STREAM_MODE = SPECTRAL_CUSTOM; // Custom Stream
        }
      }
    break;
  }
}

// Function to setup WebSocket connection.
void setupWebSocketClient() {
  debug(F("Initializing WebSocket Client Connection..."));
  wsClient.begin(ws_host, ws_port, ws_uri);
  wsClient.setReconnectInterval(i_websocket_retry_wait);
  wsClient.onEvent(webSocketClientEvent);
  b_socket_ready = true;
}