/**
 *   GPStar Ghost Trap - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Nomake Wan <-redacted->
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

#pragma once

// Web page files (defines all text as char[] variable)
#include "CommonJS.h" // COMMONJS_page
#include "Index.h" // INDEX_page
#include "IndexJS.h" // INDEXJS_page
#include "Device.h" // DEVICE_page
#include "ExtWiFi.h" // NETWORK_page
#include "Password.h" // PASSWORD_page
#include "Style.h" // STYLE_page
#include "Equip.h" // EQUIP_svg
#include "Icon.h" // FAVICON_ico, FAVICON_svg

// Forward function declarations.
void notifyWSClients();
void setupRouting();
void startSmoke(uint16_t i_duration);
void stopSmoke();

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
  debug("Sending -> Index JavaScript");
  request->send(200, "application/javascript", String(COMMONJS_page)); // Serve page content.
}

void handleRoot(AsyncWebServerRequest *request) {
  // Used for the root page (/) from the web server.
  debug("Sending -> Index HTML");
  request->send(200, "text/html", String(INDEX_page)); // Serve page content.
}

void handleRootJS(AsyncWebServerRequest *request) {
  // Used for the root page (/) from the web server.
  debug("Sending -> Index JavaScript");
  request->send(200, "application/javascript", String(INDEXJS_page)); // Serve page content.
}

void handleNetwork(AsyncWebServerRequest *request) {
  // Used for the network page from the web server.
  debug("Sending -> Network HTML");
  request->send(200, "text/html", String(NETWORK_page)); // Serve page content.
}

void handlePassword(AsyncWebServerRequest *request) {
  // Used for the password page from the web server.
  debug("Sending -> Password HTML");
  request->send(200, "text/html", String(PASSWORD_page)); // Serve page content.
}

void handleDeviceSettings(AsyncWebServerRequest *request) {
  // Used for the device page from the web server.
  debug("Sending -> Device Settings HTML");
  request->send(200, "text/html", String(DEVICE_page)); // Serve page content.
}

void handleStylesheet(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  debug("Sending -> Main StyleSheet");
  request->send(200, "text/css", String(STYLE_page)); // Serve page content.
}

void handleEquipSvg(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  debug("Sending -> Equipment SVG");
  //AsyncWebServerResponse *response = request->beginResponse(200, "image/svg+xml", EQUIP_svg, sizeof(EQUIP_svg));
  //response->addHeader("Content-Encoding", "gzip");
  //request->send(response);
  request->send(200, "image/svg+xml", String(EQUIP_svg)); // Serve file contents as SVG/XML.
}

void handleFavIco(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  debug("Sending -> Favicon");
  AsyncWebServerResponse *response = request->beginResponse(200, "image/x-icon", FAVICON_ico, sizeof(FAVICON_ico));
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
  jsonBody["displayType"] = DISPLAY_TYPE;
  jsonBody["buildDate"] = build_date;
  jsonBody["wifiName"] = ap_ssid;
  jsonBody["wifiNameExt"] = wifi_ssid;
  jsonBody["extAddr"] = wifi_address;
  jsonBody["extMask"] = wifi_subnet;
  jsonBody["openedSmokeEnabled"] = b_smoke_opened_enabled;
  jsonBody["closedSmokeEnabled"] = b_smoke_closed_enabled;
  jsonBody["openedSmokeDuration"] = i_smoke_opened_duration / 1000; // Convert MS to Seconds.
  jsonBody["closedSmokeDuration"] = i_smoke_closed_duration / 1000; // Convert MS to Seconds.

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getEquipmentStatus() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipStatus;
  jsonBody.clear();

  jsonBody["smokeEnabled"] = b_smoke_enabled;
  jsonBody["doorState"] = (DOOR_STATE == DOORS_OPENED) ? "Opened" : "Closed";
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

void handleRestart(AsyncWebServerRequest *request) {
  // Performs a restart of the device.
  request->send(204, "application/json", status);
  delay(1000);
  ESP.restart();
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

    if(jsonBody["openedSmokeEnabled"].is<unsigned short>()) {
      b_smoke_opened_enabled = jsonBody["openedSmokeEnabled"].as<bool>();
    }
    else {
      b_smoke_opened_enabled = false; // Default to off if value is invalid.
    }

    if(jsonBody["closedSmokeEnabled"].is<unsigned short>()) {
      b_smoke_closed_enabled = jsonBody["closedSmokeEnabled"].as<bool>();
    }
    else {
      b_smoke_closed_enabled = false; // Default to off if value is invalid.
    }

    if(jsonBody["openedSmokeDuration"].is<uint8_t>()) {
      i_smoke_opened_duration = jsonBody["openedSmokeDuration"].as<uint8_t>() * 1000; // Convert to MS.
    }

    if(jsonBody["closedSmokeDuration"].is<uint8_t>()) {
      i_smoke_closed_duration = jsonBody["closedSmokeDuration"].as<uint8_t>() * 1000; // Convert to MS.
    }

    // Accesses namespace in read/write mode.
    if(preferences.begin("device", false)) {
      preferences.putShort("display_type", DISPLAY_TYPE);
      preferences.putBool("smoke_opened", b_smoke_opened_enabled);
      preferences.putBool("smoke_closed", b_smoke_closed_enabled);
      preferences.putShort("smoke_op_dur", i_smoke_opened_duration);
      preferences.putShort("smoke_cl_dur", i_smoke_closed_duration);
      preferences.end();
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

void handleSmokeEnable(AsyncWebServerRequest *request) {
  b_smoke_enabled = true;
  request->send(200, "application/json", status);
  notifyWSClients();
}

void handleSmokeDisable(AsyncWebServerRequest *request) {
  b_smoke_enabled = false;
  request->send(200, "application/json", status);
  notifyWSClients();
}

void handleSmokeRun(AsyncWebServerRequest *request) {
  String c_smoke_duration = "";
  uint16_t i_smoke_duration = 0;

  if(request->hasParam("duration")) {
    // Make sure the duration is a sane value (in milliseconds).
    c_smoke_duration = request->getParam("duration")->value();
    debug("Web: Run Smoke, Duration: " + c_smoke_duration);
    i_smoke_duration = c_smoke_duration.toInt();
  }

  if(i_smoke_duration >= i_smoke_duration_min && i_smoke_duration <= i_smoke_duration_max) {
    // Stop any running smoke.
    stopSmoke();

    // Run smoke for some duration.
    startSmoke(i_smoke_duration);

    request->send(200, "application/json", status);
  }
  else {
    // Tell the user why the requested action failed.
    String result;
    jsonBody.clear();
    jsonBody["status"] = "Invalid duration specified";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
}

void handleLightOn(AsyncWebServerRequest *request) {
  ms_light.stop();
  ms_light.start(20000); // Turn on for 20 seconds steady.
  request->send(200, "application/json", status);
}

void handleLightOff(AsyncWebServerRequest *request) {
  ms_light.stop();
  ms_light.start(1); // Set a short timer to force light off.
  request->send(200, "application/json", status);
}

void setupRouting() {
  // Define the endpoints for the web server.

  // Static Pages
  httpServer.on("/", HTTP_GET, handleRoot);
  httpServer.on("/common.js", HTTP_GET, handleCommonJS);
  httpServer.on("/equipment.svg", HTTP_GET, handleEquipSvg);
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
  httpServer.on("/status", HTTP_GET, handleGetStatus);
  httpServer.on("/restart", HTTP_DELETE, handleRestart);
  httpServer.on("/wifi/settings", HTTP_GET, handleGetWifi);
  httpServer.on("/smoke/enable", HTTP_PUT, handleSmokeEnable);
  httpServer.on("/smoke/disable", HTTP_PUT, handleSmokeDisable);
  httpServer.on("/smoke/run", HTTP_PUT, handleSmokeRun);
  httpServer.on("/light/on", HTTP_PUT, handleLightOn);
  httpServer.on("/light/off", HTTP_PUT, handleLightOff);

  // Body Handlers
  httpServer.addHandler(handleSaveDeviceConfig); // /config/device/save
  httpServer.addHandler(passwordChangeHandler); // /password/update
  httpServer.addHandler(wifiChangeHandler); // /wifi/update
}

// Send notification to all websocket clients.
void notifyWSClients() {
  // Send latest status to all connected clients.
  ws.textAll(getEquipmentStatus());
}
