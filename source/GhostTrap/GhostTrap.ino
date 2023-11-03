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

// Suppress warning about SPI hardware pins
#define FASTLED_INTERNAL

// 3rd-Party Libraries
#include <FastLED.h>
#include <millisDelay.h>

// Local Files
#include "Header.h"
#include "Wireless.h"

void setup(){
  Serial.begin(9600);

  Serial.println(""); // Blank line.
  Serial.print("WiFi Network: ");
  Serial.println(ap_ssid);
  Serial.print("WiFi Password: ");
  Serial.println(ap_pass);
  WiFi.begin(ap_ssid, ap_pass);
  Serial.println("Waiting for WiFi connection...");

  // Begin waiting, so we can keep moving in the main loop.
  ms_wifiretry.start(i_wifi_retry_wait);

  pinMode(TEST_LED_PIN, OUTPUT);
}

void loop(){
  if (ms_wifiretry.remaining() < 1) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("."); // Note that we're still waiting.
      ms_wifiretry.start(i_wifi_retry_wait);
      b_wifi_connected = false;
      b_socket_config= false;
    }
    else {
      if (!b_wifi_connected) {
        // On first connection, output some status indicating as such.
        Serial.println("WiFi Connected");
        Serial.println(WiFi.localIP());
        b_wifi_connected = true;
        digitalWrite(TEST_LED_PIN, HIGH);
      }
    }
  }

  // When the wifi connection is established, proceed with websocket.
  if (b_wifi_connected) {
    if (!b_socket_config) {
      // Connect to the Attenuator device which is at a known IP address.
      webSocket.begin("192.168.1.2", 80, "/ws");

      webSocket.onEvent(webSocketEvent); // WebSocket event handler

      // If connection broken/failed then retry every X seconds.
      webSocket.setReconnectInterval(i_websocket_retry_wait);

      // Denote that we configured the websocket connection.
      b_socket_config = true;
    }

    webSocket.loop(); // Keep the socket alive.
  }
}
