/**
 *   WirelessManager - WiFi management class for GPStar devices.
 *   Handles WiFi AP and external network connections.
 *   Copyright (C) 2023-2025 Michael Rajotte, Dustin Grau, Nomake Wan
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

#ifdef ESP32

#include <WiFi.h>
#include <WiFiAP.h>
#include <ESPmDNS.h>
#include <IPAddress.h>
#include <Preferences.h>

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

/**
 * Removes spaces and illegal characters from an SSID string.
 * Purpose: Cleans up SSID names for safe use in WiFi configuration.
 * Inputs:
 *   - input: The SSID string to sanitize.
 * Outputs:
 *   - String: Sanitized SSID.
 * Side Effects: None.
 */
String sanitizeSSID(const String input);

/**
 * WirelessManager
 * Purpose: Encapsulates WiFi configuration and connection logic for ESP32-based devices.
 */
class WirelessManager {
  public:
    WirelessManager(const String& deviceName, const String& deviceAddress);

    // Scan for available SSIDs and write them into caller-provided array.
    // Returns number of SSIDs written (0..maxResults). Blocking call while scan completes.
    uint8_t scanForSSIDs(String ssids[], uint8_t maxResults = 40);

    // Public getter for localNetworkName
    String getLocalNetworkName() const { return localNetworkName; }

    // Public getter for localPassword
    String getLocalPassword() const { return localPassword; }
    String getDefaultPassword() const { return String(AP_DEFAULT_PASSWORD); }

    // Public getter for the MDNS name (eg. "GPStar_<deviceName>.local")
    String getMdnsName() const;

    // Public getter for local IP information
    IPAddress getLocalAddress() const { return localAddress; }
    IPAddress getLocalSubnet() const { return localSubnet; }
    IPAddress getLocalGateway() const { return localGateway; }
    IPAddress getLocalDhcpStart() const { return localDhcpStart; }

    // Public getter for external IP information
    bool isExtWifiEnabled() const { return extWifiEnabled; }
    String getExtWifiNetworkName() const { return extWifiNetworkName; }
    String getExtWifiPassword() const { return extWifiPassword; }
    IPAddress getExtWifiAddress() const { return extWifiAddress; }
    IPAddress getExtWifiSubnet() const { return extWifiSubnet; }
    IPAddress getExtWifiGateway() const { return extWifiGateway; }

    // Store the default AP password in preferences
    bool resetWifiPassword();

    // Check if an IPAddress object is valid (not all 0s or all 255s)
    bool IsValidIP(const IPAddress& ip);

    // Check if the configured external WiFi settings are valid
    bool HasValidExtIP();

    // Obtain the external WiFi network information from the connection
    bool getExtWifiNetworkInfo();

    // Set a default gateway based on the external IP address
    void setDefaultExtWifiGateway();

    // Start the local MDNS responder service using the AP name
    bool startMdnsService();

  private:
    // Local AP Configuration
    static constexpr uint8_t MAX_ATTEMPTS = 3;
    static constexpr char AP_DEFAULT_PREFIX[] = "GPStar_";
    static constexpr char AP_DEFAULT_PASSWORD[] = "555-2368";

    // Local AP Configuration
    bool localWiFiStarted;
    String localDeviceName;
    String localNetworkName;
    String localPassword;
    IPAddress localAddress;
    IPAddress localSubnet;
    IPAddress localGateway;
    IPAddress localDhcpStart;

    // External WiFi Configuration
    bool extWifiEnabled;
    bool extWifiStarted;
    String extWifiNetworkName;
    String extWifiPassword;
    IPAddress extWifiAddress;
    IPAddress extWifiSubnet;
    IPAddress extWifiGateway;

    // Preferences for NVS Data
    Preferences preferences;

    // Internal Helper Functions
    void loadWirelessPreferences();
    IPAddress convertToIP(const String ipAddressString);
};

#endif // ESP32