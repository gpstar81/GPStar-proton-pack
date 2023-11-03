/**
 *   gpstar GhostTrap - Ghostbusters Proton Pack & Neutrona Wand.
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
#include <WebServer.h>
#include <WebSocketsClient.h>
#include <WiFi.h>

const char* ap_ssid = "ProtonPack_9174";
const char* ap_pass = "12345678";

millisDelay ms_wifiretry;
const unsigned int i_wifi_retry_wait = 1000; // How long between attempts to find the wifi network
const unsigned int i_websocket_retry_wait = 5000; // How long between restoring the websocket
bool b_wifi_connected = false; // Denotes connection to expected wifi network
bool b_socket_config = false; // Denotes websocket configuration was performed

WebSocketsClient webSocket; // WebSocket client class instance

StaticJsonDocument<256> jsonDoc; // Allocate a static JSON document

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    // Deserialize incoming Json String
    DeserializationError error = deserializeJson(jsonDoc, payload); 
    if (error) { // Print erro msg if incomig String is not JSON formated
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    // Store values as a known datatype (String).
    String data_theme = jsonDoc["theme"];
    String data_mode = jsonDoc["mode"];
    String data_pack = jsonDoc["pack"];
    String data_power = jsonDoc["power"];
    String data_wand = jsonDoc["wand"];
    String data_cable = jsonDoc["cable"];
    String data_ctron = jsonDoc["cyclotron"];
    String data_temp = jsonDoc["temperature"];

    // Output for debug purposes only.
    Serial.print("Operating Mode: ");
    Serial.println(data_theme);
    Serial.print("Device Mode: ");
    Serial.println(data_mode);
    Serial.print("Pack State: ");
    Serial.println(data_pack);
    Serial.print("Power Level: ");
    Serial.println(data_power);
    Serial.print("Neutrona Wand: ");
    Serial.println(data_wand);
    Serial.print("Ribbon Cable: ");
    Serial.println(data_cable);
    Serial.print("Cyclotron State: ");
    Serial.println(data_ctron);
    Serial.print("Overheat State: ");
    Serial.println(data_temp);
  }
}