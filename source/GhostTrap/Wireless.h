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
bool b_wifi_connected = false; // Denotes connection to wifi network

WebSocketsClient webSocket; // WebSocket client class instance

StaticJsonDocument<256> doc; // Allocate a static JSON document
