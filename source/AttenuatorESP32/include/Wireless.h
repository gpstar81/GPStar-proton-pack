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
#include <AsyncJson.h>
#include <ElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <ESPmDNS.h>

// Preferences for SSID and AP password, which will use a "credentials" namespace.
Preferences preferences;

// Set up values for the SSID and password for the built-in WiFi access point (AP).
const uint8_t i_max_attempts = 3; // Max attempts to establish a external WiFi connection.
const String ap_ssid_prefix = "ProtonPack"; // This will be the base of the SSID name.
String ap_default_passwd = "555-2368"; // This will be the default password for the AP.
String ap_ssid; // Reserved for holding the full, private AP name for this device.
bool b_ap_started = false; // Denotes the softAP network has been started.
bool b_ws_started = false; // Denotes the web server has been started.

// Local variables for connecting to a preferred WiFi network (when available).
bool b_wifi_enabled = false; // Denotes user wishes to join/use external WiFi.
String wifi_ssid;    // Preferred network SSID for external WiFi
String wifi_pass;    // Preferred network password for external WiFi
String wifi_address; // Static IP for external WiFi network
String wifi_subnet;  // Subnet for external WiFi network
String wifi_gateway; // Gateway IP for external WiFi network

// Define an asynchronous web server at TCP port 80.
// Docs: https://github.com/me-no-dev/ESPAsyncWebServer
AsyncWebServer httpServer(80);

// Define a websocket endpoint for the async web server.
AsyncWebSocket ws("/ws");

// Track the number of connected WiFi (AP) clients.
uint8_t i_ap_client_count = 0;

// Track the number of connected WebSocket clients.
uint8_t i_ws_client_count = 0;

// Track time to refresh progress for OTA updates.
unsigned long i_progress_millis = 0;

// Create timer for WebSocket cleanup.
millisDelay ms_cleanup;
const uint16_t i_websocketCleanup = 5000;

// Create timer for checking connections.
millisDelay ms_apclient;
const uint16_t i_apClientCount = 200;

// Create timer for OTA updates.
millisDelay ms_otacheck;
const uint16_t i_otaCheck = 100;

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

/*
 * WiFi Management Functions
 */

bool startAccesPoint() {
  // Report some diagnostic data which will be necessary for this portion of setup.
  #if defined(DEBUG_WIRELESS_SETUP)
    Serial.println();
    Serial.print(F("Device WiFi MAC Address: "));
    Serial.println(WiFi.macAddress());
  #endif

  // Create an AP name unique to this device, to avoid stepping on similar hardware.
  String macAddr = String(WiFi.macAddress());
  String ap_ssid_suffix = macAddr.substring(12, 14) + macAddr.substring(15);
  String ap_pass; // Local variable for stored AP password.

  // Prepare to return either stored preferences or a default value for SSID/password.
  // Accesses namespace in read-only mode.
  if(preferences.begin("credentials", true)) {
    #if defined(RESET_AP_SETTINGS)
      // Doesn't actually "reset" but forces default values for SSID and password.
      // Meant to allow the user to reset their credentials then re-flash after
      // commenting out the RESET_AP_SETTINGS definition in Configuration.h
      ap_ssid = ap_ssid_prefix + "_" + ap_ssid_suffix; // Use default SSID.
      ap_pass = ap_default_passwd; // Force use of the default WiFi password.
    #else
      // Use either the stored preferences or an expected default value.
      ap_ssid = preferences.getString("ssid", ap_ssid_prefix + "_" + ap_ssid_suffix);
      ap_pass = preferences.getString("password", ap_default_passwd);
    #endif
    preferences.end();
  }
  else {
    ap_ssid = ap_ssid_prefix + "_" + ap_ssid_suffix; // Use default SSID.
    ap_pass = ap_default_passwd; // Force use of the default WiFi password.

    // If namespace is not initialized, open in read/write mode and set defaults.
    if(preferences.begin("credentials", false)) {
      preferences.putString("ssid", ap_ssid);
      preferences.putString("password", ap_pass);
      preferences.end();
    }
  }

  #if defined(DEBUG_WIRELESS_SETUP)
    Serial.println();
    Serial.println(F("Starting Private WiFi Configuration"));
    Serial.print(F("Stored Private SSID: "));
    Serial.println(ap_ssid);
    Serial.print(F("Stored Private PASS: "));
    Serial.println(ap_pass);
  #endif

  // Start the WiFi radio as an Access Point using the SSID and password (as WPA2).
  // Additionally, sets radio to channel 1, don't hide SSID, and max 4 connections.
  // Note that the WiFi protocols available for use are 802.11 b/g/n over 2.4GHz.
  bool b_success = false;
  if(encoder_center.getStateRaw() == LOW) {
    // If encoder post is being pressed during boot-up then bypass setting a password.
    b_success = WiFi.softAP(ap_ssid.c_str(), NULL, 1, false, 4);

    // Always output a serial message in case anything is listening, as this is important.
    Serial.println(F("WARNING: User bypassed WPA2 security, SoftAP running without password!"));
  }
  else {
    // Otherwise, set the password as desired by the user (or the default).
    b_success = WiFi.softAP(ap_ssid.c_str(), ap_pass.c_str(), 1, false, 4);
  }
  #if defined(DEBUG_WIRELESS_SETUP)
    Serial.println(b_success ? "AP Ready" : "AP Failed");
  #endif

  if(b_success) {
    delay(300); // Wait briefly before configuring network.

    // Simple networking IP info exclusively for the AP.
    IPAddress localIP(192, 168, 1, 2);
    IPAddress gateway(0, 0, 0, 0); // Not needed for AP.
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dhcpStart(192, 168, 1, 100);

    // Set networking info and report to console.
    WiFi.softAPConfig(localIP, gateway, subnet, dhcpStart);
    WiFi.softAPsetHostname(ap_ssid.c_str()); // Hostname is the same as SSID.
    WiFi.softAPbandwidth(WIFI_BW_HT20); // Use 20MHz for range/compatibility.
    WiFi.softAPenableIPv6(false); // Just here to ensure IPv6 is not enabled.
    #if defined(DEBUG_WIRELESS_SETUP)
      Serial.print(F("AP Name (SSID): "));
      Serial.println(WiFi.softAPSSID());
      Serial.print(F("AP     Channel: "));
      Serial.println(WiFi.channel());
      Serial.print(F("AP IP Addr/Sub: "));
      Serial.print(WiFi.softAPIP());
      Serial.print(F(" / "));
      Serial.println(WiFi.softAPSubnetCIDR());
      Serial.print(F("AP     Network: "));
      Serial.println(WiFi.softAPNetworkID());
      Serial.print(F("AP   Broadcast: "));
      Serial.println(WiFi.softAPBroadcastIP());
      Serial.print(F("AP    Hostname: "));
      Serial.println(WiFi.softAPgetHostname());
      Serial.print(F("AP Mac Address: "));
      Serial.println(WiFi.softAPmacAddress());
      Serial.print(F("AP  Gateway IP: "));
      Serial.println(WiFi.gatewayIP());
    #endif
  }

  return b_success;
}

bool startExternalWifi() {
  // Check for stored network preferences and attempt to connect as a client.
  #if defined(RESET_AP_SETTINGS)
    // Doesn't actually "reset" but forces default values which will allow
    // the WiFi preferences to be reset by the user, then re-flash after
    // commenting out the RESET_AP_SETTINGS definition in Configuration.h
  #else
    // Use either the stored preferences or an expected default value.
    // Accesses namespace in read-only mode.
    if(preferences.begin("network", true)) {
      b_wifi_enabled = preferences.getBool("enabled", false);
      wifi_ssid = preferences.getString("ssid", user_wifi_ssid);
      wifi_pass = preferences.getString("password", user_wifi_pass);
      wifi_address = preferences.getString("address", "");
      wifi_subnet = preferences.getString("subnet", "");
      wifi_gateway = preferences.getString("gateway", "");
      preferences.end();
    }
    else {
      // If namespace is not initialized, open in read/write mode and set defaults.
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
  #endif

  // User wants to utilize the external WiFi network and has valid SSID and password.
  if(b_wifi_enabled && wifi_ssid.length() >= 2 && wifi_pass.length() >= 8) {
    uint8_t i_curr_attempt = 0;

    // When external WiFi is desired, enable simultaneous SoftAP + Station mode.
    WiFi.mode(WIFI_MODE_APSTA);
    delay(300);

    #if defined(DEBUG_WIRELESS_SETUP)
      Serial.println();
      Serial.println(F("Starting External WiFi Configuration"));
      Serial.print(F("Stored External SSID: "));
      Serial.println(wifi_ssid);
      Serial.print(F("Stored External PASS: "));
      Serial.println(wifi_pass);
    #endif

    // Provide adequate attempts to connect to the external WiFi network.
    while (i_curr_attempt < i_max_attempts) {
      WiFi.persistent(false); // Don't write SSID/Password to flash memory.

      // Attempt to connect to a specified WiFi network.
      WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());

      // Wait for the connection to be established.
      uint8_t attempt = 0;
      while (attempt < i_max_attempts && WiFi.status() != WL_CONNECTED) {
        delay(500);
        #if defined(DEBUG_WIRELESS_SETUP)
          Serial.print(F("Connecting to external WiFi network, attempt #"));
          Serial.println(attempt);
        #endif
        attempt++;
      }

      if (WiFi.status() == WL_CONNECTED) {
        // Configure static IP values for tis device on the preferred network.
        if(wifi_address.length() >= 7 && wifi_subnet.length() >= 7 && wifi_gateway.length() >= 7) {
          #if defined(DEBUG_WIRELESS_SETUP)
            Serial.print(F("Using Stored IP: "));
            Serial.print(wifi_address);
            Serial.print(F(" / "));
            Serial.println(wifi_subnet);
          #endif

          if(wifi_gateway.length() < 7) {
            wifi_gateway = wifi_address;
          }

          IPAddress staticIP = convertToIP(wifi_address);
          IPAddress gateway = convertToIP(wifi_gateway);
          IPAddress subnet = convertToIP(wifi_subnet);

          // Set a static IP for this device.
          WiFi.config(staticIP, gateway, subnet);
        }

        // Get the IP address for this device on the preferred network.
        IPAddress localIP = WiFi.localIP();
        IPAddress subnetMask = WiFi.subnetMask();
        IPAddress gatewayIP = WiFi.gatewayIP();
        wifi_address = localIP.toString();
        wifi_subnet = subnetMask.toString();
        wifi_gateway = gatewayIP.toString();

        #if defined(DEBUG_WIRELESS_SETUP)
          Serial.print(F("WiFi IP Address: "));
          Serial.print(localIP);
          Serial.print(F(" / "));
          Serial.println(subnetMask);
        #endif

        WiFi.setAutoReconnect(false); // Don't try to reconnect, wait for a power cycle.

        return true; // Exit the loop if connected successfully.
      } else {
        #if defined(DEBUG_WIRELESS_SETUP)
          Serial.println(F("Failed to connect to WiFi. Retrying..."));
        #endif
        i_curr_attempt++;
      }
    }

    if (i_curr_attempt == i_max_attempts) {
      #if defined(DEBUG_WIRELESS_SETUP)
        Serial.println(F("Max connection attempts reached."));
        Serial.println(F("Cannot connect to external WiFi."));
      #endif
    }
  }

  return false; // If we reach this point the connection has failed.
}

bool startWiFi() {
  bool b_ext_wifi_started = false;

  // Begin some diagnostic information to console.
  #if defined(DEBUG_WIRELESS_SETUP)
    Serial.println();
    Serial.println(F("Begin WiFi Configuration"));
  #endif

  // Disable WiFi power save mode (via the esp_wifi_set_ps function).
  WiFi.setSleep(false);
  delay(100);

  // Attempt connection to an external (preferred) WiFi as a client.
  b_ext_wifi_started = startExternalWifi();

  if(!b_wifi_enabled || !b_ext_wifi_started) {
    #if defined(DEBUG_WIRELESS_SETUP)
      Serial.println(F("External WiFi not available, switching to SoftAP mode..."));
    #endif

    // When external WiFi is unavailable, switch to only use the SoftAP mode.
    WiFi.mode(WIFI_MODE_AP);
    delay(300);
  }

  // Start the built-in access point (softAP) with the preferred credentials.
  // This should ALWAYS be available for direct connections to the device.
  if(!b_ap_started) {
    b_ap_started = startAccesPoint();
  }

  // Set the mDNS hostname to "ProtonPack_NNNN.local" just like the private AP name.
  bool b_mdns_started = MDNS.begin(ap_ssid.c_str());
  #if defined(DEBUG_WIRELESS_SETUP)
    if (b_mdns_started) {
      Serial.print(F("mDNS Responder Started: "));
      Serial.println(ap_ssid + ".local");
    }
    else {
      Serial.println(F("Error Starting mDNS Responder!"));
    }
  #else
    // Suppress unused variable warning.
    (void)b_mdns_started;
  #endif
  delay(200);

  return b_ap_started; // At least return whether the soft AP started successfully.
}

void onOTAStart() {
  // Log when OTA has started
  debug(F("OTA update started"));
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
    debug(F("OTA update finished successfully!"));
  } else {
    debug(F("There was an error during OTA update!"));
  }
}

// Provide all handler functions for the API layer.
#include "Webhandler.h"
