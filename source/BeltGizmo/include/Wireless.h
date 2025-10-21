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

// Declare external reference to WirelessManager pointer (allocated in main.cpp after NVS init)
extern WirelessManager* wirelessMgr;

// Set up values for the SSID and password for the built-in WiFi access point (AP).
const uint8_t i_max_attempts = 3; // Max attempts to establish a external WiFi connection.
bool b_local_ap_started = false; // Denotes the softAP network has been started.
bool b_ext_wifi_paused = false; // Denotes retry attempts are paused.
bool b_ext_wifi_started = false; // Denotes external WiFi was joined.

// Create timer for checking connections.
millisDelay ms_apclient;
const uint16_t i_apClientCount = 200;

// Track the number of connected WiFi (AP) clients.
uint8_t i_ap_client_count = 0;

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

/*
 * WiFi Management Functions
 */

bool startAccesPoint() {
  #if defined(DEBUG_WIRELESS_SETUP)
    debugln();
    debugln(F("Starting Private WiFi Configuration"));
    debug(F("Stored Private SSID: "));
    debugln(wirelessMgr->getLocalNetworkName());
    debug(F("Stored Private PASS: "));
    debugln(wirelessMgr->getLocalPassword());
  #endif

  // Start the WiFi radio as an Access Point using the SSID and password (as WPA2).
  // Additionally, sets radio to channel 1, don't hide SSID, and max 4 connections.
  // Note that the WiFi protocols available for use are 802.11 b/g/n over 2.4GHz.
  bool b_success = false;

  // Otherwise, set the password as desired by the user (or the default).
  b_success = WiFi.softAP(wirelessMgr->getLocalNetworkName().c_str(), wirelessMgr->getLocalPassword().c_str(), 1, false, 4);

  #if defined(DEBUG_WIRELESS_SETUP)
    debugln(b_success ? "AP Ready" : "AP Failed");
  #endif

  if(b_success) {
    delay(300); // Wait briefly before configuring network.

    // Set networking IP info and report WiFi properties to console.
    WiFi.softAPConfig(wirelessMgr->getLocalAddress(), wirelessMgr->getLocalGateway(), wirelessMgr->getLocalSubnet(), wirelessMgr->getLocalDhcpStart());
    WiFi.softAPsetHostname(wirelessMgr->getLocalNetworkName().c_str()); // Hostname is the same as SSID.
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

  // User wants to utilize the external WiFi network and has valid SSID and password.
  if(wirelessMgr->isExtWifiEnabled() && wirelessMgr->getExtWifiNetworkName().length() >= 2 && wirelessMgr->getExtWifiPassword().length() >= 8) {
    uint8_t i_curr_attempt = 0;

    // When external WiFi is desired, enable simultaneous SoftAP + Station mode.
    WiFi.mode(WIFI_MODE_APSTA);
    delay(300);

    #if defined(DEBUG_WIRELESS_SETUP)
      debugln();
      debugln(F("Attempting External WiFi Configuration"));
      debug(F("Stored External SSID: "));
      debugln(wirelessMgr->getExtWifiNetworkName());
      debug(F("Stored External PASS: "));
      debugln(wirelessMgr->getExtWifiPassword());
    #endif

    // Provide adequate attempts to connect to the external WiFi network.
    while(i_curr_attempt < i_max_attempts) {
      WiFi.persistent(false); // Don't write SSID/Password to flash memory.

      // Attempt to connect to a specified WiFi network.
      WiFi.begin(wirelessMgr->getExtWifiNetworkName().c_str(), wirelessMgr->getExtWifiPassword().c_str());

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
        // Configure static IP values for this device on the preferred network.
        if(wirelessMgr->HasValidExtIP()) {
          #if defined(DEBUG_WIRELESS_SETUP)
            debug(F("Using Stored IP: "));
            debug(String(wirelessMgr->getExtWifiAddress()));
            debug(F(" / "));
            debugln(String(wirelessMgr->getExtWifiSubnet()));
          #endif

          if(!wirelessMgr->IsValidIP(wirelessMgr->getExtWifiGateway())) {
            // If no gateway is set, set a default gateway based on the IP.
            wirelessMgr->setDefaultExtWifiGateway();
          }

          // Set a static IP for this device using stored preferences.
          WiFi.config(wirelessMgr->getExtWifiAddress(), wirelessMgr->getExtWifiGateway(), wirelessMgr->getExtWifiSubnet());
        }

        // Get the IP address/subnet/gateway for this device on the preferred network.
        wirelessMgr->getExtWifiNetworkInfo();

        #if defined(DEBUG_WIRELESS_SETUP)
          debug(F("WiFi IP Address: "));
          debug(wirelessMgr->getExtWifiAddress());
          debug(F(" / "));
          debugln(wirelessMgr->getExtWifiSubnet());
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
      b_ext_wifi_paused = true; // Pause retries after exhausting attempts.
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

  // Disable WiFi power save mode (via the esp_wifi_set_ps function).
  WiFi.setSleep(false);
  delay(100);

  // Attempt connection to an external (preferred) WiFi as a client.
  b_ext_wifi_started = startExternalWifi();

  if(!wirelessMgr->isExtWifiEnabled() || !b_ext_wifi_started) {
    #if defined(DEBUG_WIRELESS_SETUP)
      debugln(F("External WiFi not available, switching to SoftAP mode..."));
    #endif

    // When external WiFi is unavailable, switch to only use the SoftAP mode.
    WiFi.mode(WIFI_MODE_AP);
    delay(300);
  }

  // Start the built-in access point (softAP) with the preferred credentials.
  // This should ALWAYS be available for direct connections to the device.
  if(!b_local_ap_started) {
    b_local_ap_started = startAccesPoint();
  }

  // Set the mDNS hostname to "<ssid>.local" just like the private AP name.
  bool b_mdns_started = wirelessMgr->startMdnsService();
  #if defined(DEBUG_WIRELESS_SETUP)
    if(b_mdns_started) {
      debug(F("mDNS Responder Started: "));
      debugln(wirelessMgr->getMdnsName());
    }
    else {
      debugln(F("Error Starting mDNS Responder!"));
    }
  #else
    // Suppress unused variable warning.
    (void)b_mdns_started;
  #endif
  delay(200);

  return b_local_ap_started; // At least return whether the soft AP started successfully.
}
