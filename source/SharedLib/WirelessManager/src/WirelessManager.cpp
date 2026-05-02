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

// Only compile this code for ESP32 platforms
#ifdef ESP32

#include <WirelessManager.h>

/**
 * Function: sanitizeSSID
 * Purpose: Remove spaces and illegal characters meant for an SSID.
 * Inputs:
 *   - input: The input string to sanitize.
 * Outputs:
 *   - String: Sanitized string with only alphanumeric, hyphens, and underscores.
 */
String sanitizeSSID(const String& input) {
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
WirelessManager::WirelessManager(WirelessDeviceType deviceType, const String& deviceAddress)
  // Initialize member variables before constructor body runs.
  : localDeviceType(deviceType),                              // Sets the default device type
    localPassword(AP_DEFAULT_PASSWORD),                       // Sets default local AP password
    localAddress(convertToIP(deviceAddress)),                 // Converts and sets device IP address
    localSubnet(convertToIP("255.255.255.0")),                // Sets default subnet mask
    localGateway(convertToIP("0.0.0.0")),                     // Gateway 0.0.0.0 keeps iOS cellular active
    extWifiEnabled(false),                                    // External WiFi disabled by default
    dnsResponderActive(false)                                    // DNS server not started yet
{
  // Run the true constructor after member variables are initialized above.
  // Keep gateway at 0.0.0.0 - this signals "no internet route via WiFi"
  // iOS sees this and keeps cellular active (works for laptops too).
  // Android benefits from DNS hijacking + 204 responses to keep cellular active.
  localDhcpStart = IPAddress(localAddress[0], localAddress[1], localAddress[2], 100);
  localDeviceName = getDeviceTypeName(); // Set the default device name based on type enum.
  loadWirelessPreferences(); // Loads custom credentials and other values from Preferences if set by user.
}

/**
 * Function: startMdnsService
 * Purpose: Starts the local MDNS responder service using the AP name and DNS responder.
 * Inputs: None.
 * Outputs:
 *   - bool: True if MDNS service started successfully, false otherwise.
 * Side Effects: Also starts the DNS responder service for captive portal avoidance.
 */
bool WirelessManager::startMdnsService() {
  String networkName = getLocalNetworkName();

  // Start DNS responder to prevent captive portal detection and keep cellular active
  startDnsResponder();

  if(MDNS.begin(networkName.c_str())) {
    delay(100); // Small delay to ensure mDNS is fully initialized.

    // Prepare a unique device ID for identification regardless of IP address
    char deviceIdBuf[5]; // 4 hex digits + null terminator
    snprintf(deviceIdBuf, sizeof(deviceIdBuf), "%04X", getDeviceID());
    String deviceIdHex = String(deviceIdBuf); // Unique hardware identifier

    // Capabilities
    MDNS.addService("http", "tcp", 80); // Web server on port 80
    MDNS.addService("ws",   "tcp", 80); // WebSocket via port 80
    MDNS.addService("ota",  "tcp", 80); // OTA update on port 80

    // Metadata
    MDNS.addServiceTxt("http", "tcp", "path",  "/"); // Root path of the HTTP server
    MDNS.addServiceTxt("ws",   "tcp", "path",  "/ws"); // Location for WebSocket upgrade
    MDNS.addServiceTxt("ota",  "tcp", "path",  "/update"); // Location of the OTA endpoint
    MDNS.addServiceTxt("ws",   "tcp", "class", "gpstar"); // Indicates a GPStar-class device
    MDNS.addServiceTxt("ws",   "tcp", "type",  getDeviceTypeName()); // Distinct device type
    MDNS.addServiceTxt("ws",   "tcp", "device", deviceIdHex.c_str()); // Unique device ID

    return true;
  }

  return false;
}

/**
 * Function: startDnsResponder
 * Purpose: Starts a custom DNS response server to return a SERVFAIL response to clients.
 *          Clients of the softAP will be given the local device IP as the primary DNS server
 *          as part of their DHCP lease, but this DNS server has no route to the internet and
 *          will always fail. We don't want clients to keep retrying to resolve names so we
 *          must respond quickly with a failure notice. Intended to keep the device's WiFi
 *          network active but mark it as unusable for internet access.
 * Inputs: None.
 * Outputs:
 *   - bool: True if DNS server started successfully, false otherwise.
 * Side Effects: Sets dnsResponderActive flag.
 */
bool WirelessManager::startDnsResponder() {
  if(!dnsResponderActive) {
    // Start a UDP responder for DNS server on port 53.
    dnsResponderActive = dnsUDP.begin(53);
  }
  return dnsResponderActive;
}

/**
 * Function: handleDNS
 * Purpose: Processes pending DNS requests. Must be called frequently in the main loop.
 * Inputs: None.
 * Outputs: None.
 * Side Effects: Responds to DNS queries with a fast failure notice (SERVFAIL).
 */
void WirelessManager::handleDNS() {
  if(dnsResponderActive) {
    int packetSize = dnsUDP.parsePacket();
    if (packetSize <= 0) return;
    if (packetSize > sizeof(packetBuffer)) return;

    dnsUDP.read(packetBuffer, packetSize);

    // DNS header is first 12 bytes
    // [0..1] Transaction ID = preserve from request (required)
    // [2] Flags: QR|Opcode|AA|TC|RD
    // [3] Flags: RA|Z|RCODE
    // [4..5] Questions count
    // [6..7] Answer RRs count
    // [8..9] Authority RRs count
    // [10..11] Additional RRs count

    // Build minimal failure response to discourage client from using this DNS:
    // QR=1 (response), RD=0 (no recursion desired), RA=0 (no recursion available)
    // RCODE=2 (SERVFAIL) - fast failure to stop client from retrying this server
    packetBuffer[2] = 0x80;  // QR=1 (response), all other flags=0
    packetBuffer[3] = 0x02;  // RCODE=2 (SERVFAIL), RA=0

    // Zero all counts (questions, answers, authority, additional)
    packetBuffer[4]  = 0x00;
    packetBuffer[5]  = 0x00;
    packetBuffer[6]  = 0x00;
    packetBuffer[7]  = 0x00;
    packetBuffer[8]  = 0x00;
    packetBuffer[9]  = 0x00;
    packetBuffer[10] = 0x00;
    packetBuffer[11] = 0x00;

    // Send only the 12-byte header for SERVFAIL (no need to echo question section)
    dnsUDP.beginPacket(dnsUDP.remoteIP(), dnsUDP.remotePort());
    dnsUDP.write(packetBuffer, 12);
    dnsUDP.endPacket();
  }
}

/**
 * Function: getDeviceID
 * Purpose: Generate a unique 12-bit device ID from WiFi MAC address.
 *          Uses fallback methods if MAC is invalid or uninitialized.
 * Inputs: None.
 * Outputs:
 *   - uint16_t: A 12-bit unique device identifier (0x000-0xFFF).
 */
uint16_t WirelessManager::getDeviceID() {
  uint8_t mac[6];
  WiFi.macAddress(mac);

  // Extract 12-bit ID from MAC (lower 4 bits of mac[4] + all 8 bits of mac[5])
  uint16_t deviceId = ((mac[4] & 0x0F) << 8) | mac[5];

  // Check for invalid MAC address (all zeros or common invalid values)
  if(deviceId == 0x000 ||
     (mac[0] == 0 && mac[1] == 0 && mac[2] == 0 && mac[3] == 0 && mac[4] == 0 && mac[5] == 0) ||
     (mac[0] == 0xFF && mac[1] == 0xFF && mac[2] == 0xFF && mac[3] == 0xFF && mac[4] == 0xFF && mac[5] == 0xFF)) {

    // Fallback 1: Try ESP32 chip ID from eFuse MAC
    uint64_t chipId = ESP.getEfuseMac();
    deviceId = chipId & 0x0FFF;  // Last 12 bits

    // Fallback 2: If chip ID is also invalid, use pseudo-random based on millis()
    if(deviceId == 0x000 || deviceId == 0xFFF) {
      // Generate reproducible pseudo-random ID based on boot time and analog noise
      uint32_t seed = millis() + analogRead(A0);
      deviceId = (seed ^ (seed >> 12)) & 0x0FFF;

      // Ensure it's never 0x000 (reserve for broadcast/special use)
      if(deviceId == 0x000) {
        deviceId = 0x001;
      }
    }
  }

  return deviceId;
}

/**
 * Function: isWifiActive
 * Purpose: Determines if WiFi functionality is currently operational in any form.
 *          This method checks both local AP status and external WiFi connections.
 * Inputs: None.
 * Outputs:
 *   - bool: True if WiFi is active in any of these scenarios:
 *           * Local SoftAP is running (WiFi mode is WIFI_MODE_AP or WIFI_MODE_APSTA)
 *           * External WiFi is enabled in settings AND currently connected (WL_CONNECTED)
 *           Returns false when:
 *           * WiFi is completely off (mode: WIFI_OFF)
 *           * Only in STA mode but not connected to external network
 *           * External WiFi disabled in settings (even if technically connected)
 */
bool WirelessManager::isWifiActive() const {
  wifi_mode_t mode = WiFi.getMode();
  return (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA) || (isExtWifiEnabled() && WiFi.status() == WL_CONNECTED);
}

/**
 * Function: getNetworkStatus
 * Purpose: Returns current network configuration and statistics as JSON object.
 * Inputs:
 *   - obj: JsonObject reference to populate with network status data
 * Outputs: None (populates the provided JsonObject)
 * Side Effects: None.
 */
void WirelessManager::getNetworkStatus(JsonObject& obj) const {
  // WiFi Mode
  wifi_mode_t mode = WiFi.getMode();
  if(mode == WIFI_MODE_AP) obj["wifiMode"] = "AP";
  else if(mode == WIFI_MODE_STA) obj["wifiMode"] = "STA";
  else if(mode == WIFI_MODE_APSTA) obj["wifiMode"] = "AP+STA";
  else obj["wifiMode"] = "OFF";

  // Local AP Info
  JsonObject localAP = obj["localAP"].to<JsonObject>();
  localAP["ssid"] = localNetworkName;
  localAP["address"] = localAddress.toString();
  localAP["subnet"] = localSubnet.toString();
  localAP["gateway"] = localGateway.toString();
  localAP["dhcpStart"] = localDhcpStart.toString();
  localAP["mdnsName"] = getMdnsName();

  // DNS Server Info
  JsonObject dns = obj["dns"].to<JsonObject>();
  dns["active"] = dnsResponderActive;

  // External WiFi Info
  JsonObject extWifi = obj["extWifi"].to<JsonObject>();
  extWifi["enabled"] = extWifiEnabled;
  if(extWifiEnabled) {
    extWifi["ssid"] = extWifiNetworkName;
    extWifi["connected"] = (WiFi.status() == WL_CONNECTED);
    if(WiFi.status() == WL_CONNECTED) {
      extWifi["address"] = extWifiAddress.toString();
      extWifi["subnet"] = extWifiSubnet.toString();
      extWifi["gateway"] = extWifiGateway.toString();
    }
  }
}

/**
 * Function: disableExtWiFi
 * Purpose: Disables external WiFi by setting the enabled property of the 'network' namespace to false.
 * Inputs: None.
 * Outputs: None. Updates preferences and runtime value.
 */
void WirelessManager::disableExtWiFi() {
  if (preferences.begin("network", false)) {
    preferences.putBool("enabled", false);
    preferences.end();
  }
  extWifiEnabled = false;
}

/**
 * Function: IsValidIP
 * Purpose: Validates an IPAddress object to ensure it's not all zeros or all 255s.
 * Inputs:
 *   - ip: The IPAddress object to validate.
 * Outputs:
 *   - bool: True if the IP is valid, false otherwise.
 */
bool WirelessManager::IsValidIP(const IPAddress& ip) const {
  return !(ip == IPAddress(0, 0, 0, 0) || ip == IPAddress(255, 255, 255, 255));
}

/**
 * Function: HasValidExtIP
 * Purpose: Confirms that the external WiFi settings are valid.
 * Inputs: None.
 * Outputs:
 *   - bool: True if the external WiFi settings are valid, false otherwise.
 */
bool WirelessManager::HasValidExtIP() {
  return isExtWifiEnabled() &&
         getExtWifiNetworkName().length() >= 2 &&
         getExtWifiPassword().length() >= 8 &&
         IsValidIP(getExtWifiAddress()) &&
         IsValidIP(getExtWifiSubnet());
}

/**
 * Function: convertToIP
 * Purpose: Converts an IPv4 address string (e.g., "192.168.1.100") to an IPAddress object.
 * Inputs:
 *   - ipAddressString: IP address in string format.
 * Outputs:
 *   - IPAddress: Parsed IP address object. Returns 0.0.0.0 if input is invalid.
 */
IPAddress WirelessManager::convertToIP(const String& ipAddressString) {
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

/**
 * Function: loadWirelessPreferences
 * Purpose: Loads WiFi preferences from NVS storage for both local AP and external network settings.
 * Inputs: None.
 * Outputs: None. Updates member variables with stored preferences or defaults.
 */
void WirelessManager::loadWirelessPreferences() {
  // Create a default for the local network name based on device type to device name.
  localNetworkName = String(AP_DEFAULT_PREFIX) + localDeviceName;

  // Prepare to return either stored preferences or a default value for local SSID/password.
  if(preferences.begin("credentials", true)) {
    // Use either the stored preferences or an expected default value.
    localNetworkName = preferences.getString("ssid", localNetworkName);
    localNetworkName = sanitizeSSID(localNetworkName); // Jacques, clean him!
    localPassword = preferences.getString("password", AP_DEFAULT_PASSWORD);
    preferences.end();
  }
  else {
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

  // The external network is using a static IP only if any of the address,
  //  subnet, or gateway fields are valid (read: not 0.0.0.0).
  extWifiStaticIP = IsValidIP(extWifiAddress) || IsValidIP(extWifiSubnet) || IsValidIP(extWifiGateway);

  // Ensure the current external network is added to preferred networks if not already present.
  if (extWifiEnabled && extWifiNetworkName != "" && !hasPreferredNetwork(extWifiNetworkName)) {
    savePreferredNetwork(
      extWifiNetworkName,
      extWifiPassword,
      extWifiStaticIP,
      extWifiAddress.toString(),
      extWifiSubnet.toString(),
      extWifiGateway.toString()
    );
  }
}

/**
 * Function: setDefaultExtWifiGateway
 * Purpose: Sets a default gateway based on the external IP address if none is configured.
 * Inputs: None.
 * Outputs: None. Modifies extWifiGateway member variable.
 */
void WirelessManager::setDefaultExtWifiGateway() {
  // Set a default gateway based on the external IP address.
  if(IsValidIP(extWifiAddress)) {
    extWifiGateway = IPAddress(extWifiAddress[0], extWifiAddress[1], extWifiAddress[2], 1);
  } else {
    extWifiGateway = IPAddress(0, 0, 0, 0);
  }
}

/**
 * Function: getExtWifiNetworkInfo
 * Purpose: Obtains the external WiFi network information from the current connection.
 * Inputs: None.
 * Outputs:
 *   - bool: True if information was successfully obtained, false otherwise.
 */
bool WirelessManager::getExtWifiNetworkInfo() {
  if(isExtWifiEnabled() && WiFi.status() == WL_CONNECTED) {
    // Verify we're connected to the expected external network
    String connectedSSID = WiFi.SSID();
    if(connectedSSID == extWifiNetworkName && extWifiNetworkName.length() > 0) {
      // Get the actual network info from the current connection
      extWifiAddress = WiFi.localIP();
      extWifiSubnet = WiFi.subnetMask();
      extWifiGateway = WiFi.gatewayIP();
      return true;
    }
  }

  // Clear the stored values if we're not connected to the external network
  extWifiAddress = IPAddress(0, 0, 0, 0);
  extWifiSubnet = IPAddress(0, 0, 0, 0);
  extWifiGateway = IPAddress(0, 0, 0, 0);

  return false;
}

/**
 * Function: getExtNetworkAsJson
 * Purpose: Returns a JSON object string representing the current active external network configuration.
 * Inputs: None.
 * Outputs:
 *   - String: JSON object with fields: ssid, password, address, subnet, gateway
 */
void WirelessManager::getExtWifiNetworkAsJson(JsonObject& obj) const {
  obj["enabled"] = extWifiEnabled;
  obj["ssid"] = extWifiNetworkName;
  obj["password"] = extWifiPassword;
  obj["staticIP"] = extWifiStaticIP;
  obj["address"] = extWifiAddress.toString();
  obj["subnet"] = extWifiSubnet.toString();
  obj["gateway"] = extWifiGateway.toString();
}

// Private method to reset the member array of 3 IPAddress elements.
void WirelessManager::resetDisoveredDevices() {
  for (int8_t i = 0; i < 3; ++i) {
    discoveredCoreDevice[i] = IPAddress(0, 0, 0, 0);
  }
}

/**
 * Function: discoverWebSocketServer
 * Purpose: Uses mDNS to discover the WebSocket server IP for available GPStar devices.
 * Inputs: None.
 * Outputs: Return true on any successful discoveries, false otherwise.
 */
bool WirelessManager::discoverWebSocketServer() {
  int8_t i_num_discovered = MDNS.queryService("ws", "tcp"); // Query for WebSocket services on the current network.
  resetDisoveredDevices(); // IP's for discovered Attenuator, Pack2, and Wand2 devices.
  int8_t i_attenuator = -1; // Index of a discovered Attenuator device.
  int8_t i_protonpack = -1; // Index of a discovered Proton Pack device.
  int8_t i_neutronawand = -1; // Index of a discovered Neutrona Wand device.
  IPAddress hostIP;

  if (i_num_discovered == 0) {
    //Serial.println(F("mDNS: No WS services found on the external network."));
    return false;
  }

  // Iterate through discovered services to find the best match for GPStar host device.
  for (int8_t i = 0; i < i_num_discovered; i++) {
    String cls = MDNS.txt(i, "class"); // Get the class of device.
    String type = MDNS.txt(i, "type"); // Get the type of device.

    if(cls == "gpstar") {
      // For any GPStar-class device, check the type for best match (Attenuator > Pack2 > Wand2).
      if (type == "Attenuator") {
        i_attenuator = i; // Found a WebSocket via an Attenuator.
      } else if (type == "Pack2") {
        i_protonpack = i; // Found a WebSocket via a Proton Pack.
      } else if (type == "Wand2") {
        i_neutronawand = i; // Found a WebSocket via a Neutrona Wand.
      }
    }
  }

  if(i_attenuator == -1 && i_protonpack == -1 && i_neutronawand == -1) {
    //Serial.println(F("mDNS: No suitable WS services found on a GPStar device."));
    return false;
  }

  if(i_attenuator > -1) {
    String fqdn = MDNS.hostname(i_attenuator) + ".local"; // Must resolve using the full hostname.
    if(WiFi.hostByName(fqdn.c_str(), hostIP)) {
      discoveredCoreDevice[0] = hostIP;
    } else {
      //Serial.println(F("mDNS: WS service found on an Attenuator, but could not resolve hostname to an IP."));
      if(i_protonpack == -1 && i_neutronawand == -1) {
        // Only stop trying if no alternative devices exist.
        return false;
      }
    }
  }

  if(i_protonpack > -1) {
    String fqdn = MDNS.hostname(i_protonpack) + ".local"; // Must resolve using the full hostname.
    if(WiFi.hostByName(fqdn.c_str(), hostIP)) {
      discoveredCoreDevice[1] = hostIP;
    } else {
      //Serial.println(F("mDNS: WS service found on a Proton Pack, but could not resolve hostname to an IP."));
      if(i_neutronawand == -1) {
        // Only stop trying if no alternative devices exist.
        return false;
      }
    }
  }

  if(i_neutronawand > -1) {
    String fqdn = MDNS.hostname(i_neutronawand) + ".local"; // Must resolve using the full hostname.
    if(WiFi.hostByName(fqdn.c_str(), hostIP)) {
      discoveredCoreDevice[2] = hostIP;
    } else {
      //Serial.println(F("mDNS: WS service found on a Neutrona Wand, but could not resolve hostname to an IP."));
      return false;
    }
  }

  // Iterate over the found IPAddress values, if any are valid we can return true.
  for (int8_t i = 0; i < 3; ++i) {
    if(IsValidIP(discoveredCoreDevice[i])) {
      return true; // At least one valid IP address was found.
    }
  }

  //Serial.println(F("mDNS: WS service found, but resolved IP was invalid."));
  return false;
}

IPAddress WirelessManager::getFirstDiscoveredDevice() const {
  // Iterate over the found IPAddress values, the first valid IP will be used.
  // Ordering: 0=Attenuator, 1=Proton Pack, 2=Neutrona Wand
  for (int8_t i = 0; i < 3; ++i) {
    if(IsValidIP(discoveredCoreDevice[i])) {
      return discoveredCoreDevice[i];
      break; // Use the first valid IP found.
    }
  }
  return IPAddress(0, 0, 0, 0); // Return invalid IP if none found.
}

/**
 * Function: resetWifiPassword
 * Purpose: Resets the WiFi password to the default value.
 * Inputs: None.
 * Outputs:
 *   - bool: True if the password was successfully reset, false otherwise.
 */
bool WirelessManager::resetWifiPassword() {
  if(preferences.begin("credentials", false)) {
    preferences.putString("password", AP_DEFAULT_PASSWORD);
    preferences.end();
    return true;
  }
  return false;
}

/**
 * Function: getMdnsName
 * Purpose: Returns the mDNS hostname with ".local" suffix, with fallback safety check.
 * Inputs: None.
 * Outputs:
 *   - String: The mDNS hostname with ".local" suffix (e.g., "GPStar_Attenuator.local").
 *             Returns "MDNS_NOT_SET" if mDNS could not be initialized.
 * Side Effects: None.
 */
String WirelessManager::getMdnsName() const {
  // Simply return the local network name with .local suffix as used with MDNS.begin()
  String hostname = getLocalNetworkName();

  if(hostname.length() > 0) {
    return hostname + ".local";
  }

  return "MDNS_NOT_SET";
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

/**
 * Function: savePreferredNetworks
 * Purpose: Saves a JSON array of network configurations to preferences.
 * Inputs:
 *   - networksJson: JSON string containing array of network objects.
 *     Expected format:
 *     [
 *       {
 *         "ssid": "NetworkName1",
 *         "password": "password123",
 *         "address": "192.168.1.100",    // Optional static IP
 *         "subnet": "255.255.255.0",     // Optional subnet mask
 *         "gateway": "192.168.1.1"       // Optional gateway IP
 *       },
 *       {
 *         "ssid": "NetworkName2",
 *         "password": "anotherpass",
 *         "address": "",                 // Empty strings for DHCP
 *         "subnet": "",
 *         "gateway": ""
 *       }
 *     ]
 *     Maximum 10 network objects. SSID and password are required fields.
 * Outputs:
 *   - bool: True if successfully saved, false otherwise.
 */
bool WirelessManager::savePreferredNetworks(const String& networksJson) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, networksJson);

  if (error) {
    return false;
  }

  // Validate it's an array and doesn't exceed max networks
  if (!doc.is<JsonArray>() || doc.size() > MAX_PREFERRED_NETWORKS) {
    return false;
  }

  // Validate each network object has required fields
  for (JsonVariant network : doc.as<JsonArray>()) {
    if (!network.is<JsonObject>() ||
        !network["ssid"].is<const char*>() ||
        !network["password"].is<const char*>()) {
      return false;
    }
  }

  // Save to preferences
  if (preferences.begin("networks", false)) {
    preferences.putString("list", networksJson);
    preferences.end();
    return true;
  }

  return false;
}

/**
 * Function: getPreferredNetworks
 * Purpose: Retrieves the preferred networks as a JSON string.
 * Inputs: None.
 * Outputs:
 *   - String: JSON array of preferred network configurations, or empty array if none saved.
 *     Returned format:
 *     [
 *       {
 *         "ssid": "NetworkName1",
 *         "password": "password123",
 *         "address": "192.168.1.100",    // Static IP or empty string
 *         "subnet": "255.255.255.0",     // Subnet mask or empty string
 *         "gateway": "192.168.1.1"       // Gateway IP or empty string
 *       },
 *       ...
 *     ]
 *     Returns "[]" if no networks are saved or on error.
 */
String WirelessManager::getPreferredNetworks() {
  if (preferences.begin("networks", true)) {
    String result = preferences.getString("list", "[]");
    preferences.end();
    return result;
  }
  return "[]";
}

/**
 * Function: applyPreferredNetwork
 * Purpose: Applies a preferred network configuration to the current "network" namespace.
 * Inputs:
 *   - index: Index of the preferred network to apply (0-based)
 * Outputs:
 *   - bool: True if successfully applied, false otherwise.
 */
bool WirelessManager::applyPreferredNetwork(uint8_t index) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, getPreferredNetworks());

  if (error || !doc.is<JsonArray>() || index >= doc.size()) {
    return false;
  }

  JsonObject network = doc[index].as<JsonObject>();

  // Apply to current network preferences
  if (preferences.begin("network", false)) {
    preferences.putBool("enabled", true);
    preferences.putString("ssid", network["ssid"].as<String>());
    preferences.putString("password", network["password"].as<String>());
    preferences.putString("address", network["address"] | "");
    preferences.putString("subnet", network["subnet"] | "");
    preferences.putString("gateway", network["gateway"] | "");
    preferences.end();

    // Update current runtime values
    extWifiEnabled = true;
    extWifiNetworkName = network["ssid"].as<String>();
    extWifiPassword = network["password"].as<String>();
    extWifiAddress = convertToIP(network["address"] | "");
    extWifiSubnet = convertToIP(network["subnet"] | "");
    extWifiGateway = convertToIP(network["gateway"] | "");

    return true;
  }

  return false;
}

/**
 * Function: getPreferredNetworkCount
 * Purpose: Returns the number of preferred network configurations.
 * Inputs: None.
 * Outputs:
 *   - uint8_t: Number of preferred networks (0-MAX_PREFERRED_NETWORKS).
 */
uint8_t WirelessManager::getPreferredNetworkCount() {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, getPreferredNetworks());

  if (error || !doc.is<JsonArray>()) {
    return 0;
  }

  return doc.size();
}

/**
 * Function: savePreferredNetwork
 * Purpose: Adds a new (or updates existing) network configuration in the preferred list.
 * Inputs:
 *   - ssid: Network SSID
 *   - password: Network password
 *   - staticIP: Whether to use static IP configuration
 *   - address: Static IP address (optional)
 *   - subnet: Subnet mask (optional)
 *   - gateway: Gateway IP (optional)
 * Outputs:
 *   - bool: True if successfully added/updated, false if list is full or error occurred.
 */
bool WirelessManager::savePreferredNetwork(const String& ssid, const String& password, bool staticIP, const String& address, const String& subnet, const String& gateway) {
  if (ssid.length() < 2 || password.length() < 8) {
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, getPreferredNetworks());

  if (error) {
    doc.to<JsonArray>(); // Initialize as empty array
  }

  JsonArray networks = doc.as<JsonArray>();

  // Check for existing SSID and update if found
  for (JsonVariant network : networks) {
    if (network["ssid"].as<String>() == ssid) {
      network["password"] = password;
      network["staticIP"] = staticIP;
      network["address"] = address;
      network["subnet"] = subnet;
      network["gateway"] = gateway;

      String result;
      serializeJson(doc, result);

      return savePreferredNetworks(result);
    }
  }

  // Check if we're at capacity (only for adding a network)
  if (networks.size() >= MAX_PREFERRED_NETWORKS) {
    return false;
  }

  // Add the new network
  JsonObject newNetwork = networks.add<JsonObject>();
  newNetwork["ssid"] = ssid;
  newNetwork["password"] = password;
  newNetwork["staticIP"] = staticIP;
  newNetwork["address"] = address;
  newNetwork["subnet"] = subnet;
  newNetwork["gateway"] = gateway;

  String result;
  serializeJson(doc, result);

  return savePreferredNetworks(result);
}

/**
 * Function: removePreferredNetwork
 * Purpose: Removes a preferred network configuration by index.
 * Inputs:
 *   - index: Index of the network to remove (0-based)
 * Outputs:
 *   - bool: True if successfully removed, false otherwise.
 */
bool WirelessManager::removePreferredNetwork(uint8_t index) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, getPreferredNetworks());

  if (error || !doc.is<JsonArray>() || index >= doc.size()) {
    return false;
  }

  JsonArray networks = doc.as<JsonArray>();
  networks.remove(index);

  String result;
  serializeJson(doc, result);

  return savePreferredNetworks(result);
}

/**
 * Function: getPreferredNetworkIndex
 * Purpose: Returns the index of a preferred network by SSID, or -1 if not found.
 * Inputs:
 *   - ssid: Network SSID to search for
 * Outputs:
 *   - int8_t: Index of the network if found, -1 otherwise.
 */
int8_t WirelessManager::getPreferredNetworkIndex(const String& ssid) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, getPreferredNetworks());
  if (error || !doc.is<JsonArray>()) {
    return -1;
  }
  JsonArray networks = doc.as<JsonArray>();
  int8_t idx = 0;
  for (JsonVariant network : networks) {
    if (network["ssid"].as<String>() == ssid) {
      return idx;
    }
    ++idx;
  }
  return -1;
}

/**
 * Function: hasPreferredNetwork
 * Purpose: Checks if a network with the given SSID exists in the preferred networks list.
 * Inputs:
 *   - ssid: Network SSID to search for
 * Outputs:
 *   - bool: True if SSID exists in preferred networks, false otherwise.
 */
bool WirelessManager::hasPreferredNetwork(const String& ssid) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, getPreferredNetworks());

  if (error || !doc.is<JsonArray>()) {
    return false;
  }

  JsonArray networks = doc.as<JsonArray>();

  // Check for matching SSID
  for (JsonVariant network : networks) {
    if (network["ssid"].as<String>() == ssid) {
      return true;
    }
  }

  return false;
}

#endif // ESP32