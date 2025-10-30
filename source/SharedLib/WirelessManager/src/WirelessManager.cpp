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

#ifdef ESP32

#include <WirelessManager.h>

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

/**
 * Constructor: WirelessManager
 * Purpose: Initializes WiFi manager with device-specific AP SSID and IP address.
 * Inputs:
 *   - deviceName: The device name to append to the AP prefix.
 *   - deviceIpAddress: The device's IP address as a string (e.g., "192.168.1.100").
 */
WirelessManager::WirelessManager(const String& deviceName, const String& deviceAddress)
  // Initialize member variables before constructor body runs.
  : localDeviceName(deviceName),                              // Sets the default device name
    localNetworkName(String(AP_DEFAULT_PREFIX) + deviceName), // Sets AP SSID to "GPStar_<deviceName>"
    localPassword(AP_DEFAULT_PASSWORD),                       // Sets default local AP password
    localAddress(convertToIP(deviceAddress)),                 // Converts and sets device IP address
    localSubnet(convertToIP("255.255.255.0")),            // Sets default subnet mask
    localGateway(convertToIP("0.0.0.0")),                     // Sets default gateway
    localWiFiStarted(false),                                  // Local AP not started yet
    extWifiEnabled(false),                                    // External WiFi disabled by default
    extWifiStarted(false)                                     // External WiFi not started yet
{
  // Run the true constructor after member variables are initialized above.
  localDhcpStart = IPAddress(localAddress[0], localAddress[1], localAddress[2], 100);
  loadWirelessPreferences(); // Loads custom credentials and other values from Preferences if set by user
}

// Function: IsValidIP
// Purpose: Validates an IPAddress object to ensure it's not all zeros or all 255s.
// Inputs:
//   - ip: The IPAddress object to validate.
// Outputs:
//   - bool: True if the IP is valid, false otherwise.
bool WirelessManager::IsValidIP(const IPAddress& ip) {
  return !(ip == IPAddress(0, 0, 0, 0) || ip == IPAddress(255, 255, 255, 255));
}

// Function: HasValidExtIP
// Purpose: Confirms that the external WiFi settings are valid.
// Inputs: None.
// Outputs:
//   - bool: True if the external WiFi settings are valid, false otherwise.
bool WirelessManager::HasValidExtIP() {
  return isExtWifiEnabled() &&
         getExtWifiNetworkName().length() >= 2 &&
         getExtWifiPassword().length() >= 8 &&
         IsValidIP(getExtWifiAddress()) &&
         IsValidIP(getExtWifiSubnet());
}

// Function: convertToIP
// Purpose: Converts an IPv4 address string (e.g., "192.168.1.100") to an IPAddress object.
// Inputs:
//   - ipAddressString: IP address in string format.
// Outputs:
//   - IPAddress: Parsed IP address object. Returns 0.0.0.0 if input is invalid.
IPAddress WirelessManager::convertToIP(const String ipAddressString) {
  uint8_t quads[4] = {0, 0, 0, 0}; // Store 4 octets for the IP address
  int lastIndex = 0;
  int nextIndex = 0;

  for(uint8_t i = 0; i < 4; i++) {
    nextIndex = ipAddressString.indexOf('.', lastIndex);
    String quadStr;
    if(nextIndex != -1 && i < 3) {
      quadStr = ipAddressString.substring(lastIndex, nextIndex);
      lastIndex = nextIndex + 1;
    } else {
      quadStr = ipAddressString.substring(lastIndex);
    }
    quads[i] = quadStr.toInt();
  }

  // Basic validation: ensure all quads are in 0-255 range
  for(uint8_t i = 0; i < 4; i++) {
    if(quads[i] > 255) return IPAddress(0, 0, 0, 0);
  }

  return IPAddress(quads[0], quads[1], quads[2], quads[3]);
}

void WirelessManager::loadWirelessPreferences() {
  // Prepare to return either stored preferences or a default value for local SSID/password.
  if(preferences.begin("credentials", true)) {
    // Use either the stored preferences or an expected default value.
    localNetworkName = preferences.getString("ssid", String(AP_DEFAULT_PREFIX) + localDeviceName);
    localNetworkName = sanitizeSSID(localNetworkName); // Jacques, clean him!
    localPassword = preferences.getString("password", AP_DEFAULT_PASSWORD);
    preferences.end();
  }
  else {
    localNetworkName = String(AP_DEFAULT_PREFIX) + localDeviceName; // Use default SSID.
    localPassword = AP_DEFAULT_PASSWORD; // Force use of the local default WiFi password.

    // If namespace is not initialized, open in read/write mode and set defaults.
    if(preferences.begin("credentials", false)) {
      preferences.putString("ssid", localNetworkName);
      preferences.putString("password", localPassword);
      preferences.end();
    }
  }

  // Use either the stored preferences or an expected default value.
  if(preferences.begin("network", true)) {
    extWifiEnabled = preferences.getBool("enabled", false);
    extWifiNetworkName = preferences.getString("ssid", "");
    extWifiPassword = preferences.getString("password", "");
    extWifiAddress = convertToIP(preferences.getString("address", ""));
    extWifiSubnet = convertToIP(preferences.getString("subnet", ""));
    extWifiGateway = convertToIP(preferences.getString("gateway", ""));
    preferences.end();
  } else {
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
}

// Function: setDefaultExtWifiGateway
// Purpose: Sets a default gateway based on the external IP address if none is configured.
// Inputs: None.
// Outputs: None. Modifies extWifiGateway member variable.
void WirelessManager::setDefaultExtWifiGateway() {
  // Set a default gateway based on the external IP address.
  if(IsValidIP(extWifiAddress)) {
    extWifiGateway = IPAddress(extWifiAddress[0], extWifiAddress[1], extWifiAddress[2], 1);
  } else {
    extWifiGateway = IPAddress(0, 0, 0, 0);
  }
}

// Function: getExtWifiNetworkInfo
// Purpose: Obtains the external WiFi network information from the current connection.
// Inputs: None.
// Outputs:
//   - bool: True if information was successfully obtained, false otherwise.
bool WirelessManager::getExtWifiNetworkInfo() {
  if(WiFi.status() == WL_CONNECTED) {
    // Get the IP address for this device on the external network.
    extWifiAddress = WiFi.localIP();
    extWifiSubnet = WiFi.subnetMask();
    extWifiGateway = WiFi.gatewayIP();
    return true;
  }
  return false;
}

// Function: resetWifiPassword
// Purpose: Resets the WiFi password to the default value.
// Inputs: None.
// Outputs: None. Modifies the stored password in preferences.
bool WirelessManager::resetWifiPassword() {
  if(preferences.begin("credentials", false)) {
    preferences.putString("password", AP_DEFAULT_PASSWORD);
    preferences.end();
    return true;
  }
  return false;
}

// Function: getMdnsName
// Purpose: Returns the mDNS hostname with ".local" suffix, with fallback safety check.
// Inputs: None.
// Outputs:
//   - String: The mDNS hostname with ".local" suffix (e.g., "GPStar_Attenuator.local").
//             Returns "MDNS_NOT_SET" if mDNS could not be initialized.
// Side Effects: None.
String WirelessManager::getMdnsName() const {
  // Try to get the primary hostname directly (index 0), or will be an empty string.
  String mdnsHostname = MDNS.hostname(0);

  // Check if hostname is valid and not empty.
  if(mdnsHostname.length() > 0) {
    return mdnsHostname + ".local";
  }

  // Return clear debug indicator if mDNS isn't ready or returns empty.
  return "MDNS_NOT_SET";
}

// Function: startMdnsService
// Purpose: Starts the local MDNS responder service using the AP name.
// Inputs: None.
// Outputs:
//   - bool: True if MDNS service started successfully, false otherwise.
bool WirelessManager::startMdnsService() {
  if(MDNS.begin(getLocalNetworkName().c_str())) {
    MDNS.addService("http", "tcp", 80);
    return true;
  }

  return false;
}

/**
 * Function: scanForSSIDs
 * Purpose: Performs a WiFi scan and writes unique 2.4GHz SSIDs into a caller-provided array.
 * Inputs:
 *   - ssids: preallocated array of String objects to receive SSIDs
 *   - maxResults: capacity of ssids array
 * Outputs:
 *   - uint8_t: number of unique SSIDs written into ssids
 *
 * Notes:
 *   - Blocking call while the scan runs.
 *   - Ensures STA capability is enabled for scanning but does not force-disable AP.
 *   - Filters results to channels 1..14 (2.4 GHz).
 *   - Ensures results are unique by SSID string (duplicates skipped).
 */
uint8_t WirelessManager::scanForSSIDs(String ssids[], uint8_t maxResults) {
  if(maxResults == 0) return 0;

  // Ensure STA capability is enabled for scanning; enable AP+STA only if STA bit missing.
  if((WiFi.getMode() & WIFI_MODE_STA) == 0) {
    WiFi.mode(WIFI_AP_STA);
  }

  // Clear previous scan results to free memory.
  WiFi.scanDelete();

  // NOTE: This is a blocking call while the scan runs.
  int i_found = WiFi.scanNetworks();
  if(i_found <= 0) {
    WiFi.scanDelete();
    return 0;
  }

  uint8_t i_count = 0;
  for(int i = 0; i < i_found && i_count < maxResults; ++i) {
    // IEEE 802.11 defines 2.4 GHz Wi‑Fi channels as channel numbers 1-14, which is what we want.
    int i_channel = WiFi.channel(i);
    if(i_channel < 1 || i_channel > 14) {
      continue; // Skip 5 GHz or unknown-channel networks.
    }

    String s_ssid = WiFi.SSID(i);
    if(s_ssid.length() == 0) {
      continue;
    }

    // Check for duplicates: only add SSID if not already present in ssids[]
    bool b_duplicate = false;
    for(uint8_t i_j = 0; i_j < i_count; ++i_j) {
      if(ssids[i_j] == s_ssid) {
        b_duplicate = true;
        break;
      }
    }
    if(b_duplicate) {
      continue;
    }

    ssids[i_count++] = s_ssid; // Add unique SSID
  }

  WiFi.scanDelete();
  return i_count;
}

#endif // ESP32