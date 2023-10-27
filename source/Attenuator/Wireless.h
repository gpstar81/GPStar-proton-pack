/**
 *   gpstar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
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
#include <Preferences.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Web page files (HTML as char[])
#include "index.h"

// Preferences for SSID and AP password, which will use a "credentials" namespace.
Preferences preferences;

// Set up values for the SSID and password for the WiFi access point (AP).
const String ap_ssid_prefix = "ProtonPack"; // This will be the base of the SSID name.
String ap_default_passwd = "555-2368"; // This will be the default password for the AP.
String ap_ssid; // Reserved for storing the true SSID for the AP to be set at startup.
String ap_pass; // Reserved for storing the true AP password set by the user.

// Define the web server object globally, answering to TCP port 80.
WebServer httpServer(80);

boolean startAccessPoint() {
  // Begin some diagnostic information to console.
  Serial.println();
  Serial.println("Starting Wireless Access Point");
  String macAddr = String(WiFi.macAddress());
  Serial.print("Device WiFi MAC Address: ");
  Serial.println(macAddr);

  // Create an AP name unique to this device, to avoid stepping on others.
  String ap_ssid_suffix = macAddr.substring(12, 14) + macAddr.substring(15);
  //ap_ssid = ap_ssid_prefix + "_" + ap_ssid_suffix; // Update AP broadcast name.

  // Prepare to return either stored preferences or a default value for SSID/password.
  preferences.begin("credentials", true); // Access namespace in read-only mode.
  ap_ssid = preferences.getString("ssid", ap_ssid_prefix + "_" + ap_ssid_suffix);
  ap_pass = preferences.getString("password", ap_default_passwd);
  preferences.end();

  // Start the access point using the SSID and password.
  return WiFi.softAP(ap_ssid.c_str(), ap_pass.c_str());
}

void configureNetwork() {
  // Simple networking info for the AP.
  IPAddress local_ip(192, 168, 1, 2);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);

  // Set networking info and report to console.
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);
  Serial.print("Access Point IP Address: ");
  IPAddress IP = WiFi.softAPIP();
  Serial.println(IP);
  Serial.print("WiFi AP Started as ");
  Serial.println(ap_ssid);
  Serial.print("WiFi AP Password: ");
  Serial.println(ap_pass);
}

/*
 * Text Helper Functions - Converts ENUM values to user-friendly text
 */

String getTheme() {
  switch(YEAR_MODE) {
    case YEAR_1984:
      return "1984";
    break;
    case YEAR_1989:
      return "1989";
    break;
    case YEAR_2021:
      return "Afterlife";
    break;
    default:
      return "Unknown";
    break;
  }
}

String getMode() {
  switch(FIRING_MODE) {
    case PROTON:
      return "Proton";
    break;
    case SLIME:
      return "Slime";
    break;
    case STASIS:
      return "Stasis";
    break;
    case MESON:
      return "Meson";
    break;
    case SPECTRAL:
      return "Spectral";
    break;
    case HOLIDAY:
      return "Holiday";
    break;
    case SPECTRAL_CUSTOM:
      return "Custom";
    break;
    case VENTING:
      return "Venting";
    break;
    case SETTINGS:
      return "Settings";
    break;
    default:
      return "Unknown";
    break;
  }
}

String getPower() {
  switch(POWER_LEVEL) {
    case LEVEL_1:
      return "1";
    break;
    case LEVEL_2:
      return "2";
    break;
    case LEVEL_3:
      return "3";
    break;
    case LEVEL_4:
      return "4";
    break;
    case LEVEL_5:
      return "5";
    break;
    default:
      return "-";
    break;
  }
}

String getCyclotronState() {
  switch(i_speed_multiplier) {
    case 1:
      return (b_overheating ? "Recovery" : "Normal");
    break;
    case 2:
      return "Active";
    break;
    case 3:
      return "Warning";
    break;
    default:
      return "Critical";
    break;
  }
}

/*
 * Web Handler Functions - Performs actions or returns data for web UI
 */
StaticJsonDocument<250> jsonDoc; // Used for processing JSON data.

void handleRoot() {
  // Used for the root page (/) of the web server.
  Serial.println("Web Root HTML Requested");
  String s = MAIN_page; // Read HTML contents from .h file.
  httpServer.send(200, "text/html", s); // Send index page.
}

void handleStatus() {
  // Return data for AJAX requests by the index page.
  jsonDoc.clear();
  jsonDoc["theme"] = getTheme();
  jsonDoc["mode"] = getMode();
  jsonDoc["pack"] = (b_pack_on ? "Powered" : "Idle");
  jsonDoc["power"] = getPower();
  jsonDoc["wand"] = (b_firing ? "Firing" : "Idle");
  jsonDoc["cable"] = (b_pack_alarm ? "Disconnected" : "Connected");
  jsonDoc["cyclotron"] = getCyclotronState();
  jsonDoc["temperature"] = (b_overheating ? "Venting" : "Normal");
  String status;
  serializeJson(jsonDoc, status); // Serialize to string.
  httpServer.send(200, "application/json", status);
}

void handlePackOn() {
  Serial.println("Turn Pack On");
  attenuatorSerialSend(A_TURN_PACK_ON);
  httpServer.send(200, "application/json", "{}");
}

void handlePackOff() {
  Serial.println("Turn Pack Off");
  attenuatorSerialSend(A_TURN_PACK_OFF);
  httpServer.send(200, "application/json", "{}");
}

void handleToggleMute() {
  Serial.println("Toggle Mute");
  attenuatorSerialSend(A_TOGGLE_MUTE);
  httpServer.send(200, "application/json", "{}");
}

void handleMasterVolumeUp() {
  Serial.println("Master Volume Up");
  attenuatorSerialSend(A_VOLUME_INCREASE);
  httpServer.send(200, "application/json", "{}");
}

void handleMasterVolumeDown() {
  Serial.println("Master Volume Down");
  attenuatorSerialSend(A_VOLUME_DECREASE);
  httpServer.send(200, "application/json", "{}");
}

void handleEffectsVolumeUp() {
  Serial.println("Effects Volume Up");
  attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_INCREASE);
  httpServer.send(200, "application/json", "{}");
}

void handleEffectsVolumeDown() {
  Serial.println("Effects Volume Down");
  attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_DECREASE);
  httpServer.send(200, "application/json", "{}");
}

void handleMusicStartStop() {
  Serial.println("Music Start/Stop");
  attenuatorSerialSend(A_MUSIC_START_STOP);
  httpServer.send(200, "application/json", "{}");
}

void handleNextMusicTrack() {
  Serial.println("Next Music Track");
  attenuatorSerialSend(A_MUSIC_NEXT_TRACK);
  httpServer.send(200, "application/json", "{}");
}

void handlePrevMusicTrack() {
  Serial.println("Prev Music Track");
  attenuatorSerialSend(A_MUSIC_PREV_TRACK);
  httpServer.send(200, "application/json", "{}");
}

void handleCancelWarning() {
  Serial.println("Cancel Overheat Warning");
  attenuatorSerialSend(A_WARNING_CANCELLED);
  httpServer.send(200, "application/json", "{}");
}

void handlePassword() {
  if (httpServer.hasArg("plain") == false) {
    Serial.println("No arg 'plain' for body");
  }

  String body = httpServer.arg("plain");
  jsonDoc.clear();
  deserializeJson(jsonDoc, body);

  String newPasswd = jsonDoc["password"];
  Serial.print("New AP Password: ");
  Serial.println(newPasswd);

  if (newPasswd != "") {
    preferences.begin("credentials", false); // Access namespace in read/write mode.
    preferences.putString("ssid", ap_ssid);
    preferences.putString("password", newPasswd);
    preferences.end();

    jsonDoc.clear();
    jsonDoc["response"] = "Password updated, rebooting controller. Please enter your new WiFi password when prompted by your device.";
    String result;
    serializeJson(jsonDoc, result); // Serialize to string.
    httpServer.send(200, "application/json", result);
    delay(100);
    ESP.restart(); // Reboot device
  }
}

void handleNotFound() {
  // Returned for any invalid URL requested.
  Serial.println("Web Not Found");
  httpServer.send(404, "text/plain", "Not Found");
}

void setupRouting() {
  // Define the endpoints for the web server.
  httpServer.on("/", handleRoot);
  httpServer.on("/status", handleStatus);
  httpServer.on("/pack/on", handlePackOn);
  httpServer.on("/pack/off", handlePackOff);
  httpServer.on("/pack/cancel", handleCancelWarning);
  httpServer.on("/volume/mute", handleToggleMute);
  httpServer.on("/volume/master/up", handleMasterVolumeUp);
  httpServer.on("/volume/master/down", handleMasterVolumeDown);
  httpServer.on("/volume/effects/up", handleEffectsVolumeUp);
  httpServer.on("/volume/effects/down", handleEffectsVolumeDown);
  httpServer.on("/music/toggle", handleMusicStartStop);
  httpServer.on("/music/next", handleNextMusicTrack);
  httpServer.on("/music/prev", handlePrevMusicTrack);
  httpServer.on("/password", HTTP_POST, handlePassword);
  httpServer.onNotFound(handleNotFound);
}

// Define server actions after declaring all functions for URL routing.
void startWebServer() {
  setupRouting(); // Set URI's with handlers.
  httpServer.begin(); // Start the daemon.
  Serial.println("HTTP Server Started");
}
