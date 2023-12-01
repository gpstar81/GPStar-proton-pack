/**
 *   gpstar External - Ghostbusters Proton Pack & Neutrona Wand.
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
// Requires library from https://github.com/Links2004/arduinoWebSockets
#include <ArduinoJson.h>
#include <Preferences.h>
#include <WebServer.h>
#include <WebSocketsClient.h>
#include <WiFi.h>

// Preferences for SSID and AP password, which will use a "credentials" namespace.
Preferences preferences;

// Set up values for the SSID and password for the WiFi access point (AP).
const String ap_default_ssid = "ProtonPack_D418"; // This will be the base of the SSID name.
String ap_default_passwd = "12345678"; // This will be the default password for the AP.
String ap_ssid; // Reserved for storing the true SSID for the AP to be set at startup.
String ap_pass; // Reserved for storing the true AP password set by the user.

// Prepare variables for client connections
millisDelay ms_wifiretry;
const unsigned int i_wifi_retry_wait = 1000; // How long between attempts to find the wifi network
const unsigned int i_websocket_retry_wait = 3000; // How long between restoring the websocket
bool b_wifi_connected = false; // Denotes connection to expected wifi network
bool b_socket_config = false; // Denotes websocket configuration was performed

WebSocketsClient webSocket; // WebSocket client class instance

StaticJsonDocument<256> jsonDoc; // Allocate a static JSON document

boolean startWiFi() {
  // Begin some diagnostic information to console.
  Serial.println();
  Serial.println("Starting Wireless Client");
  String macAddr = String(WiFi.macAddress());
  Serial.print("Device WiFi MAC Address: ");
  Serial.println(macAddr);

  // Prepare to return either stored preferences or a default value for SSID/password.
  preferences.begin("credentials", true); // Access namespace in read-only mode.
  ap_ssid = preferences.getString("ssid", ap_default_ssid);
  ap_pass = preferences.getString("password", ap_default_passwd);
  preferences.end();

  Serial.print("WiFi Network: ");
  Serial.println(ap_ssid);
  Serial.print("WiFi Password: ");
  Serial.println(ap_pass);

  // Start the access point using the SSID and password.
  return WiFi.begin(ap_ssid.c_str(), ap_pass.c_str());
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.println("WebSocket Connected");
    digitalWrite(BUILT_IN_LED, HIGH);
  }

  if (type == WStype_DISCONNECTED) {
    Serial.println("WebSocket Disconnected");
    digitalWrite(BUILT_IN_LED, LOW);
  }

  if (type == WStype_TEXT) {
    // Deserialize incoming JSON String from remote websocket server.
    DeserializationError error = deserializeJson(jsonDoc, payload); 
    if (error) {
      // Print error msg if incomig String is not JSON formatted.
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
    }
    else {
      // Store values as a known datatype (String).
      String data_mode = jsonDoc["mode"];
      String data_theme = jsonDoc["theme"];
      String data_switch = jsonDoc["switch"];
      String data_pack = jsonDoc["pack"];
      String data_power = jsonDoc["power"];
      String data_safety = jsonDoc["safety"];
      String data_wand = jsonDoc["wand"];
      String data_firing = jsonDoc["firing"];
      String data_cable = jsonDoc["cable"];
      String data_ctron = jsonDoc["cyclotron"];
      String data_temp = jsonDoc["temperature"];

      // Change LED for testing
      if(data_firing == "Firing") {
        Serial.println(data_firing);
        digitalWrite(LED_R_PIN, HIGH);
      }
      else {
        Serial.println(data_firing);
        digitalWrite(LED_R_PIN, LOW);
      }
    }
  }
}