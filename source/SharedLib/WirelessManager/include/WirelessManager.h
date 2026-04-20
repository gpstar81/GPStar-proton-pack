/**
 *   WirelessManager - WiFi management class for GPStar devices.
 *   Handles WiFi AP and external network connections.
 *   Copyright (C) 2023-2026 Michael Rajotte, Dustin Grau, Nomake Wan
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

// Only compile this code for ESP32 platforms
#ifdef ESP32

#include <WiFi.h>
#include <WiFiAP.h>
#include <ESPmDNS.h>
#include <IPAddress.h>
#include <Preferences.h>
#include <ArduinoJson.h>

/**
 * Wireless (WiFi) Communications for ESP32
 *
 * This device will use the SoftAP mode to act as a standalone WiFi access point, allowing
 * direct connections to the device without need for a full wireless network. All address
 * (IP) assignments will be handled as part of the code here.
 *
 * Additionally, the user may specify an external WiFi network (SSID and password) for the
 * device which will put the device into AP+STA mode, allowing it to connect to an existing
 * WiFi network while still hosting its own (private) AP for direct connections.
 *
 * External networks may be stored (up to 5) as "preferred networks" in JSON format within
 * the device's NVS (non-volatile storage) area. The user can sort these networks in order
 * of preference, which may allow networks to be joined automatically if in range.
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
String sanitizeSSID(const String& input);

// Define strongly-typed device types for MDNS records.
// Made public for ease of use in any device w/ WiFi.
enum WirelessDeviceType {
  ATTENUATOR,
  PROTON_PACK,
  NEUTRONA_WAND,
  SINGLESHOT,
  GHOST_TRAP,
  BELT_GIZMO,
  STREAM_EFFECTS,
  PSTT
};

/**
 * WirelessManager
 * Purpose: Encapsulates WiFi configuration and connection logic for ESP32-based devices.
 */
class WirelessManager {
  public:
    WirelessManager(WirelessDeviceType deviceType, const String& deviceAddress);

    // Set a default gateway based on the external IP address
    void setDefaultExtWifiGateway();

    // Start the local MDNS responder service using the AP name
    bool startMdnsService();

    // Check if WiFi is active (either external connection or local AP started)
    bool isWifiActive() const;

    // Set the internal flag to disable external WiFi connections
    void disableExtWiFi();

    // Scan for available SSIDs and write them into caller-provided array.
    // Returns number of SSIDs written (0..maxResults). Blocking call while scan completes.
    uint8_t scanForSSIDs(String ssids[], uint8_t maxResults = 40);

    // Public getter for localDeviceType
    WirelessDeviceType getDeviceType() const { return localDeviceType; }

    // Return the device type name enum as a string
    constexpr const char* getDeviceTypeName() {
      switch (localDeviceType) {
        case WirelessDeviceType::ATTENUATOR: return "Attenuator";
        case WirelessDeviceType::PROTON_PACK: return "Pack2";
        case WirelessDeviceType::NEUTRONA_WAND: return "Wand2";
        case WirelessDeviceType::GHOST_TRAP: return "Trap";
        case WirelessDeviceType::SINGLESHOT: return "Blaster";
        case WirelessDeviceType::BELT_GIZMO: return "BeltGizmo";
        case WirelessDeviceType::STREAM_EFFECTS: return "StreamEffects";
        case WirelessDeviceType::PSTT: return "PSTT";
        default: return "";
      }
    }

    // Public getter for localNetworkName
    String getLocalNetworkName() const { return localNetworkName; }

    // Public getter for localPassword
    String getLocalPassword() const { return localPassword; }
    String getDefaultPassword() const { return String(AP_DEFAULT_PASSWORD); }

    // Public getter for the MDNS name (eg. "GPStar_<deviceName>.local")
    String getMdnsName() const;

    // Get unique 12-bit device ID from WiFi MAC address (last 12 bits)
    uint16_t getDeviceID();

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

    // Device discovery via mDNS for WebSocket implementations
    bool discoverWebSocketServer();
    IPAddress getFirstDiscoveredDevice() const;

    // Store the default AP password in preferences
    bool resetWifiPassword();

    // Check if an IPAddress object is valid (not all 0s or all 255s)
    bool IsValidIP(const IPAddress& ip) const;

    // Check if the configured external WiFi settings are valid
    bool HasValidExtIP();

    // Obtain the external WiFi network information from the connection
    bool getExtWifiNetworkInfo();
    void getExtWifiNetworkAsJson(JsonObject& obj) const;

    // Preferred External Networks Management (JSON-based)
    bool savePreferredNetworks(const String& networksJson);
    String getPreferredNetworks();
    bool applyPreferredNetwork(uint8_t index);
    uint8_t getPreferredNetworkCount();
    bool savePreferredNetwork(const String& ssid, const String& password, bool staticIP, const String& address = "", const String& subnet = "", const String& gateway = "");
    bool removePreferredNetwork(uint8_t index);
    bool hasPreferredNetwork(const String& ssid);
    int8_t getPreferredNetworkIndex(const String& ssid);

  private:
    // Local AP Configuration
    static constexpr uint8_t MAX_ATTEMPTS = 3;
    static constexpr char AP_DEFAULT_PREFIX[] = "GPStar_";
    static constexpr char AP_DEFAULT_PASSWORD[] = "555-2368";

    // Preferred Networks Configuration
    static constexpr uint8_t MAX_PREFERRED_NETWORKS = 10;

    // Local AP Configuration
    WirelessDeviceType localDeviceType;
    String localDeviceName;
    String localNetworkName;
    String localPassword;
    IPAddress localAddress;
    IPAddress localSubnet;
    IPAddress localGateway;
    IPAddress localDhcpStart;

    // External WiFi Configuration
    bool extWifiEnabled;
    String extWifiNetworkName;
    String extWifiPassword;
    bool extWifiStaticIP;
    IPAddress extWifiAddress;
    IPAddress extWifiSubnet;
    IPAddress extWifiGateway;

    // Reset discovered device IPAddress array
    IPAddress discoveredCoreDevice[3];
    void resetDisoveredDevices();

    // Preferences for NVS Data
    Preferences preferences;

    // Internal Helper Functions
    void loadWirelessPreferences();
    IPAddress convertToIP(const String& ipAddressString);
};

#endif // ESP32