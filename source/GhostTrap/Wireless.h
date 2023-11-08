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
const unsigned int i_wifi_retry_wait = 500; // How long between attempts to find the wifi network
const unsigned int i_websocket_retry_wait = 5000; // How long between restoring the websocket
bool b_wifi_connected = false; // Denotes connection to expected wifi network
bool b_socket_config = false; // Denotes websocket configuration was performed

WebSocketsClient webSocket; // WebSocket client class instance

StaticJsonDocument<256> jsonDoc; // Allocate a static JSON document

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  if (type == WStype_CONNECTED) {
    Serial.println("WebSocket Connected");
  }

  if (type == WStype_DISCONNECTED) {
    Serial.println("WebSocket Disconnected");
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

      // Output for debug purposes only.
      Serial.print("Arming Mode: ");
      Serial.print(data_mode);
      Serial.print(" Operating Mode: ");
      Serial.println(data_theme);

      Serial.print("Pack Armed: ");
      Serial.print(data_switch);
      Serial.print(" Pack State: ");
      Serial.println(data_pack);

      Serial.print("Power Level: ");
      Serial.print(data_power);
      Serial.print("Safety State: ");
      Serial.println(data_safety);

      Serial.print("Wand Mode: ");
      Serial.print(data_wand);
      Serial.print(" Neutrona Wand: ");
      Serial.println(data_firing);

      Serial.print("Ribbon Cable: ");
      Serial.println(data_cable);

      Serial.print("Cyclotron State: ");
      Serial.print(data_ctron);
      Serial.print(" Overheat State: ");
      Serial.println(data_temp);
    }
  }
}