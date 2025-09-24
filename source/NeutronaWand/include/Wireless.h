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
#include <WiFi.h>
#include <WiFiAP.h>
#include <ESPmDNS.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

// Set up values for the SSID and password for the built-in WiFi access point (AP).
const uint8_t i_max_attempts = 3; // Max attempts to establish a external WiFi connection.
const char AP_DEFAULT_SSID[] = "GPStar_Wand2"; // This will be the base of the SSID name.
String ap_default_passwd = "555-2368"; // This will be the default password for the AP.
String ap_ssid; // Reserved for holding the full, private AP name for this device.
bool b_ap_started = false; // Denotes the softAP network has been started.
bool b_ws_started = false; // Denotes the web server has been started.
bool b_ext_wifi_started = false; // Denotes external WiFi was joined.

// Local variables for connecting to a preferred WiFi network (when available).
bool b_wifi_enabled = false; // Denotes user wishes to join/use external WiFi.
String wifi_ssid;    // Preferred network SSID for external WiFi
String wifi_pass;    // Preferred network password for external WiFi
String wifi_address; // Static IP for external WiFi network
String wifi_subnet;  // Subnet for external WiFi network
String wifi_gateway; // Gateway IP for external WiFi network

// Define standard ports and URI endpoints.
const uint16_t WS_PORT = 80; // Web Server (+WebSocket) port
const char WS_URI[] = "/ws"; // WebSocket endpoint URI

// Define an asynchronous web server at TCP port 80.
AsyncWebServer httpServer(WS_PORT);

// Define a websocket endpoint for the async web server.
AsyncWebSocket ws(WS_URI);

// Create a server-side event source on /events.
AsyncEventSource events("/events");

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

// Convert an IP address string to an IPAddress object.
IPAddress convertToIP(const String ipAddressString) {
  uint16_t quads[4]; // Array to store 4 quads for the IP.
  uint8_t quadStartIndex = 0;
  int8_t quadEndIndex = 0;

  for(uint8_t i = 0; i < 4; i++) {
    // Find the index of the next dot
    quadEndIndex = ipAddressString.indexOf('.', quadStartIndex);

    if(quadEndIndex != -1) {
      // If a dot is found, extract and store the quad
      String quad = ipAddressString.substring(quadStartIndex, quadEndIndex);
      quads[i] = quad.toInt(); // Convert the quad string to an integer
      quadStartIndex = quadEndIndex + 1;
    }
    else {
      // If the dot is not found, this is the last quad
      String lastQuad = ipAddressString.substring(quadStartIndex);
      quads[i] = lastQuad.toInt();
    }
  }

  // Create an IPAddress object from the quads
  IPAddress ipAddress(quads[0], quads[1], quads[2], quads[3]);

  return ipAddress;
}

// Remove spaces and illegal characters meant for an SSID.
String sanitizeSSID(const String input) {
  String result = "";

  for(size_t i = 0; i < input.length(); i++) {
    char c = input[i];

    // Only allow alphanumeric, hyphens, and underscores
    if(isalnum(c) || c == '-' || c == '_') {
      result += c;
    }
  }

  return result;
}

// Reset the AP password in case the user forgot it.
void resetWifiPassword() {
  if(preferences.begin("credentials", false)) {
    preferences.putString("password", ap_default_passwd);
    preferences.end();
  }
}

/*
 * WiFi Management Functions
 */

bool startAccesPoint() {
  // Report some diagnostic data which will be necessary for this portion of setup.
  #if defined(DEBUG_WIRELESS_SETUP)
    debugln();
    debug(F("Device WiFi MAC Address: "));
    debugln(WiFi.macAddress());
  #endif

  // Create an AP name unique to this device, to avoid stepping on similar hardware.
  String macAddr = String(WiFi.macAddress());
  String ap_pass; // Local variable for stored AP password.

  // Prepare to return either stored preferences or a default value for SSID/password.
  // Accesses namespace in read-only mode.
  if(preferences.begin("credentials", true)) {
    #if defined(RESET_AP_SETTINGS)
      // Doesn't actually "reset" but forces default values for SSID and password.
      // Meant to allow the user to reset their credentials then re-flash after
      // commenting out the RESET_AP_SETTINGS definition in Configuration.h
      ap_ssid = String(AP_DEFAULT_SSID); // Use default SSID.
      ap_pass = ap_default_passwd; // Force use of the default WiFi password.
    #else
      // Use either the stored preferences or an expected default value.
      ap_ssid = preferences.getString("ssid", String(AP_DEFAULT_SSID));
      ap_ssid = sanitizeSSID(ap_ssid); // Jacques, clean him!
      ap_pass = preferences.getString("password", ap_default_passwd);
    #endif
    preferences.end();
  }
  else {
    ap_ssid = String(AP_DEFAULT_SSID); // Use default SSID.
    ap_pass = ap_default_passwd; // Force use of the default WiFi password.

    // If namespace is not initialized, open in read/write mode and set defaults.
    if(preferences.begin("credentials", false)) {
      preferences.putString("ssid", ap_ssid);
      preferences.putString("password", ap_pass);
      preferences.end();
    }
  }

  #if defined(DEBUG_WIRELESS_SETUP)
    debugln();
    debugln(F("Starting Private WiFi Configuration"));
    debug(F("Stored Private SSID: "));
    debugln(ap_ssid);
    debug(F("Stored Private PASS: "));
    debugln(ap_pass);
  #endif

  // Start the WiFi radio as an Access Point using the SSID and password (as WPA2).
  // Additionally, sets radio to channel 1, don't hide SSID, and max 4 connections.
  // Note that the WiFi protocols available for use are 802.11 b/g/n over 2.4GHz.
  bool b_success = false;
  b_success = WiFi.softAP(ap_ssid.c_str(), ap_pass.c_str(), 1, false, 4);

  #if defined(DEBUG_WIRELESS_SETUP)
    debugln(b_success ? "AP Ready" : "AP Failed");
  #endif

  if(b_success) {
    delay(300); // Wait briefly before configuring network.

    // Simple networking IP info exclusively for the AP.
    IPAddress localIP(192, 168, 1, 6);
    IPAddress gateway(0, 0, 0, 0); // Not needed for AP.
    IPAddress subnet(255, 255, 255, 0);
    IPAddress dhcpStart(192, 168, 1, 100);

    // Set networking info and report to console.
    WiFi.softAPConfig(localIP, gateway, subnet, dhcpStart);
    WiFi.softAPsetHostname(ap_ssid.c_str()); // Hostname is the same as SSID.
    WiFi.softAPbandwidth(WIFI_BW_HT20); // Use 20MHz for range/compatibility.
    WiFi.softAPenableIPv6(false); // Just here to ensure IPv6 is not enabled.
    #if defined(DEBUG_WIRELESS_SETUP)
      debug(F("AP Name (SSID): "));
      debugln(WiFi.softAPSSID());
      debug(F("AP     Channel: "));
      debugln(WiFi.channel());
      debug(F("AP IP Addr/Sub: "));
      debug(WiFi.softAPIP());
      debug(F(" / "));
      debugln(WiFi.softAPSubnetCIDR());
      debug(F("AP     Network: "));
      debugln(WiFi.softAPNetworkID());
      debug(F("AP   Broadcast: "));
      debugln(WiFi.softAPBroadcastIP());
      debug(F("AP    Hostname: "));
      debugln(WiFi.softAPgetHostname());
      debug(F("AP Mac Address: "));
      debugln(WiFi.softAPmacAddress());
      debug(F("AP  Gateway IP: "));
      debugln(WiFi.gatewayIP());
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
      debugln();
      debugln(F("Attempting External WiFi Configuration"));
      debug(F("Stored External SSID: "));
      debugln(wifi_ssid);
      debug(F("Stored External PASS: "));
      debugln(wifi_pass);
    #endif

    // Provide adequate attempts to connect to the external WiFi network.
    while(i_curr_attempt < i_max_attempts) {
      WiFi.persistent(false); // Don't write SSID/Password to flash memory.

      // Attempt to connect to a specified WiFi network.
      WiFi.begin(wifi_ssid.c_str(), wifi_pass.c_str());

      // Limit Tx power to save battery and reduce interference.
      WiFi.setTxPower(WIFI_POWER_7dBm); // Set to 7 dBm (default is ~20 dBm).

      // Wait for the connection to be established.
      uint8_t attempt = 0;
      while(attempt < i_max_attempts && WiFi.status() != WL_CONNECTED) {
        delay(500);
        #if defined(DEBUG_WIRELESS_SETUP)
          debug(F("Connecting to external WiFi network, attempt #"));
          debugln(attempt);
        #endif
        attempt++;
      }

      if(WiFi.status() == WL_CONNECTED) {
        // Configure static IP values for tis device on the preferred network.
        if(wifi_address.length() >= 7 && wifi_subnet.length() >= 7 && wifi_gateway.length() >= 7) {
          #if defined(DEBUG_WIRELESS_SETUP)
            debug(F("Using Stored IP: "));
            debug(wifi_address);
            debug(F(" / "));
            debugln(wifi_subnet);
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
          debug(F("WiFi IP Address: "));
          debug(localIP);
          debug(F(" / "));
          debugln(subnetMask);
        #endif

        WiFi.setAutoReconnect(false); // Don't try to reconnect, wait for a power cycle.

        return true; // Exit the loop if connected successfully.
      }
      else {
        #if defined(DEBUG_WIRELESS_SETUP)
          debugln(F("Failed to connect to WiFi. Retrying..."));
        #endif
        i_curr_attempt++;
      }
    }

    if(i_curr_attempt == i_max_attempts) {
      #if defined(DEBUG_WIRELESS_SETUP)
        debugln(F("Max connection attempts reached."));
        debugln(F("Cannot connect to external WiFi."));
      #endif
    }
  }

  return false; // If we reach this point the connection has failed.
}

bool startWiFi() {
  // Begin some diagnostic information to console.
  #if defined(DEBUG_WIRELESS_SETUP)
    debugln();
    debugln(F("Begin WiFi Configuration"));
  #endif

  // Enable WiFi power save mode (via the esp_wifi_set_ps function).
  WiFi.setSleep(true);
  delay(100);

  // Attempt connection to an external (preferred) WiFi as a client.
  b_ext_wifi_started = startExternalWifi();

  if(!b_wifi_enabled || !b_ext_wifi_started) {
    #if defined(DEBUG_WIRELESS_SETUP)
      debugln(F("External WiFi not available, switching to SoftAP mode..."));
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
    if(b_mdns_started) {
      debug(F("mDNS Responder Started: "));
      debugln(ap_ssid + ".local");
    }
    else {
      debugln(F("Error Starting mDNS Responder!"));
    }
  #else
    // Suppress unused variable warning.
    (void)b_mdns_started;
  #endif
  delay(200);

  return b_ap_started; // At least return whether the soft AP started successfully.
}

// Provide all handler functions for the API layer.
#include "Webhandler.h"

// Stops the web server and disables WiFi to save power or for security.
void shutdownWireless() {
  if(WiFi.getMode() != WIFI_OFF) {
    // Close all websocket connections and stop the web server.
    ws.closeAll();
    httpServer.end();
    b_ws_started = false;

    // Disconnect WiFi and turn off radio.
    WiFi.disconnect(true);
    delay(1);
    WiFi.mode(WIFI_OFF);
    delay(1);
    b_ap_started = false;
    b_ext_wifi_started = false;

    #if defined(DEBUG_WIRELESS_SETUP)
      debugln(F("Wireless and web server shut down."));
    #endif
  }
}

// Restarts WiFi and web server when needed.
void restartWireless() {
  if(!b_ap_started) {
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
