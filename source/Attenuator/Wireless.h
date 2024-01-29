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

#pragma once

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
#include "PackSettings.h" // PACK_SETTINGS_page
#include "WandSettings.h" // WAND_SETTINGS_page
#include "SmokeSettings.h" // SMOKE_SETTINGS_page
#include "Style.h" // STYLE_page

// Preferences for SSID and AP password, which will use a "credentials" namespace.
Preferences preferences;

// Set up values for the SSID and password for the WiFi access point (AP).
const String ap_ssid_prefix = "ProtonPack"; // This will be the base of the SSID name.
String ap_default_passwd = "555-2368"; // This will be the default password for the AP.
String ap_ssid; // Reserved for storing the built-in SSID for the AP to be set at startup.
String ap_pass; // Reserved for storing the built-in AP password set by the user.
String wifi_ssid; // Reserved for storing a preferred SSID to join.
String wifi_pass; // Reserved for storing the network password.
String wifi_address; // Reserved for storing the IP address to use for access.
String wifi_subnet; // Reserved for storing the subnet mask to use for access.
String wifi_gateway; // Reserved for storing the gateway for the network.
const uint8_t maxAttempts = 3; // Max attempts to establish a WiFi connection.

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

boolean startAccesPoint() {
  // Create an AP name unique to this device, to avoid stepping on similar hardware.
  String macAddr = String(WiFi.macAddress());
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

  // Start the WiFi radio as an Access Point using the SSID and password (as WPA2).
  // Additionally, sets radio to channel 6, don't hide SSID, and max 4 connections.
  // Note that the WiFi protocols available for use are 802.11b/g/n
  bool b_ap_started = WiFi.softAP(ap_ssid.c_str(), ap_pass.c_str(), 6, false, 4);
  #if defined(DEBUG_WIRELESS_SETUP)
    Serial.println(b_ap_started ? "AP Ready" : "AP Failed");
  #endif
  return b_ap_started;
}

boolean startWiFi() {
  // Begin some diagnostic information to console.
  #if defined(DEBUG_WIRELESS_SETUP)
    Serial.println();
    Serial.println("Starting WiFi...");
    Serial.print("Device WiFi MAC Address: ");
    Serial.println(WiFi.macAddress());
  #endif

  // Check for stored network preferences and attempt to connect as a client.
  preferences.begin("network", true); // Access namespace in read-only mode.
  #if defined(RESET_AP_SETTINGS)
    // Doesn't actually "reset" but forces default values which will allow
    // the WiFi preferences to be reset by the user, then re-flash after
    // commenting out the RESET_AP_SETTINGS definition in Configuration.h
  #else
    // Use either the stored preferences or an expected default value.
    wifi_ssid = preferences.getString("ssid", "");
    wifi_pass = preferences.getString("password", "");
    wifi_address = preferences.getString("address", "192.168.1.2");
    wifi_subnet = preferences.getString("subnet", "255.255.255.0");
    wifi_gateway = preferences.getString("gateway", "192.168.1.1");
  #endif
  preferences.end();

  if(wifi_ssid.length() >= 2 && wifi_pass.length() >= 8) {
    uint8_t attemptCount = 0;

    while (attemptCount < maxAttempts) {
      // Attempt to connect to a specified WiFi network.
      WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());

      // Wait for the connection to be established
      uint8_t attempt = 0;
      while (attempt < 30 && WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi...");
        attempt++;
      }

      if (WiFi.status() == WL_CONNECTED) {
        // Exit the loop if connected successfully
        return true;
      } else {
        #if defined(DEBUG_WIRELESS_SETUP)
          Serial.println("Failed to connect to WiFi. Retrying...");
        #endif
        attemptCount++;
      }
    }

    if (attemptCount == maxAttempts) {
      #if defined(DEBUG_WIRELESS_SETUP)
        Serial.println("Max connection attempts reached. Could not connect to WiFi.");
      #endif

      // As a fallback, start a built-in access point with the preferred credentials.
      return startAccesPoint();
    }
  }
  else {
    // Start a built-in access point with the preferred credentials.
    return startAccesPoint();
  }

  return false; // Not sure how we'd get here, but just in case return false.
}

IPAddress convertToIP(String ipAddressString) {
  uint16_t quads[4]; // Array to store 4 quads for the IP.
  uint8_t quadStartIndex = 0;
  int8_t quadEndIndex = 0;

  for (uint8_t i = 0; i < 4; i++) {
    // Find the index of the next dot
    quadEndIndex = ipAddressString.indexOf('.', quadStartIndex);

    if (quadEndIndex != -1) {
      // If a dot is found, extract and store the quad
      String quad = ipAddressString.substring(quadStartIndex, quadEndIndex);
      quads[i] = quad.toInt(); // Convert the quad string to an integer
      quadStartIndex = quadEndIndex + 1;
    } else {
      // If the dot is not found, this is the last quad
      String lastQuad = ipAddressString.substring(quadStartIndex);
      quads[i] = lastQuad.toInt();
    }
  }

  // Create an IPAddress object from the quads
  IPAddress ipAddress(quads[0], quads[1], quads[2], quads[3]);

  return ipAddress;
}

void configureNetwork() {
  // If connected to WiFi and the network information is available, set as necessary.
  if(WiFi.status() == WL_CONNECTED && wifi_address.length() >= 7 && wifi_subnet.length() >= 7 && wifi_gateway.length() >= 7) {
    IPAddress staticIP = convertToIP(wifi_address);
    IPAddress gateway = convertToIP(wifi_subnet);
    IPAddress subnet = convertToIP(wifi_gateway);

    // Set a static IP for this device via preferences.
    WiFi.config(staticIP, gateway, subnet);
    #if defined(DEBUG_WIRELESS_SETUP)
      IPAddress deviceIP = WiFi.localIP();
      Serial.println("Connected to WiFi");
      Serial.print("IP Address: ");
      Serial.println(deviceIP);
    #endif
  }
  else {
    // Simple networking info for the AP.
    IPAddress localIP(192, 168, 1, 2);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);

    // Set networking info and report to console.
    WiFi.softAPConfig(localIP, gateway, subnet);
    #if defined(DEBUG_WIRELESS_SETUP)
      IPAddress deviceIP = WiFi.softAPIP();
      Serial.print("Access Point IP Address: ");
      Serial.println(deviceIP);
      Serial.println("WiFi AP Started as " + ap_ssid);
      Serial.println("WiFi AP Password: " + ap_pass);
    #endif
  }
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
      // For anything above level 3.
      return "Critical";
    break;
  }
}

// Provide all handler functions for the API layer.
#include "Webhandler.h"

void setupRouting() {
  // Define the endpoints for the web server.

  // Static Pages
  httpServer.on("/", HTTP_GET, handleRoot);
  httpServer.on("/password", HTTP_GET, handlePassword);
  httpServer.on("/settings/pack", HTTP_GET, handlePackSettings);
  httpServer.on("/settings/wand", HTTP_GET, handleWandSettings);
  httpServer.on("/settings/smoke", HTTP_GET, handleSmokeSettings);
  httpServer.on("/style.css", HTTP_GET, handleStylesheet);
  httpServer.onNotFound(handleNotFound);

  // Get/Set Handlers
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

  // Body Handlers
  httpServer.addHandler(handleSavePackConfig); // /config/pack/save
  httpServer.addHandler(handleSaveWandConfig); // /config/wand/save
  httpServer.addHandler(handleSaveSmokeConfig); // /config/smoke/save
  httpServer.addHandler(passwordChangeHandler); // /password/update
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
