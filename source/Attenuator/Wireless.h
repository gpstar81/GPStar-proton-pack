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
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#include "index.h" // Web page header file

// Set up values for the SSID and password for the WiFi access point (AP).
const String ap_ssid_prefix = "ProtonPack"; // This will be the base of the SSID name.
String ap_default_passwd = "555-2368"; // This will be the default password for the AP.
String ap_ssid; // Reserved for storing the true SSID for the AP to be set at startup.

// Define the web server object globally.
WebServer httpServer(80);

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

void handleRoot() {
  // Used for the root page (/) of the web server.
  Serial.println("Web Root Requested");
  String s = MAIN_page; // Read HTML contents from .h file.
  httpServer.send(200, "text/html", s); // Send index page.
}

void handleStatus() {
  // Return data for AJAX request by index.
  StaticJsonDocument<400> json;
  json["theme"] = getTheme();
  json["mode"] = getMode();
  json["pack"] = (b_pack_on ? "Powered" : "Idle");
  json["power"] = getPower();
  json["wand"] = (b_firing ? "Firing" : "Idle");
  json["cable"] = (b_pack_alarm ? "Disconnected" : "Connected");
  json["cyclotron"] = getCyclotronState();
  json["temperature"] = (b_overheating ? "Venting" : "Normal");
  String status;
  serializeJson(json, status); // Serialize to string.
  httpServer.send(200, "application/json", status);
}

void handlePackOn() {
  Serial.println("Pack On");
  attenuatorSerialSend(A_TURN_PACK_ON);
  httpServer.send(200, "text/plain", "OK");
}

void handlePackOff() {
  Serial.println("Pack Off");
  attenuatorSerialSend(A_TURN_PACK_OFF);
  httpServer.send(200, "text/plain", "OK");
}

void handleToggleMute() {
  Serial.println("Toggle Mute");
  attenuatorSerialSend(A_TOGGLE_MUTE);
  httpServer.send(200, "text/plain", "OK");
}

void handleMasterVolumeUp() {
  Serial.println("Master Volume Up");
  attenuatorSerialSend(A_VOLUME_INCREASE);
  httpServer.send(200, "text/plain", "OK");
}

void handleMasterVolumeDown() {
  Serial.println("Master Volume Down");
  attenuatorSerialSend(A_VOLUME_DECREASE);
  httpServer.send(200, "text/plain", "OK");
}

void handleEffectsVolumeUp() {
  Serial.println("Effects Volume Up");
  attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_INCREASE);
  httpServer.send(200, "text/plain", "OK");
}

void handleEffectsVolumeDown() {
  Serial.println("Effects Volume Down");
  attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_DECREASE);
  httpServer.send(200, "text/plain", "OK");
}

void handleMusicStartStop() {
  Serial.println("Music Start/Stop");
  attenuatorSerialSend(A_MUSIC_START_STOP);
  httpServer.send(200, "text/plain", "OK");
}

void handleNextMusicTrack() {
  Serial.println("Next Music Track");
  attenuatorSerialSend(A_MUSIC_NEXT_TRACK);
  httpServer.send(200, "text/plain", "OK");
}

void handlePrevMusicTrack() {
  Serial.println("Prev Music Track");
  attenuatorSerialSend(A_MUSIC_PREV_TRACK);
  httpServer.send(200, "text/plain", "OK");
}

void handleCancelWarning() {
  Serial.println("Cancel Overheat Warning");
  attenuatorSerialSend(A_WARNING_CANCELLED);
  httpServer.send(200, "text/plain", "OK");
}

void handleNotFound() {
  // Returned for any invalid URL requested.
  Serial.println("Web Not Found");
  httpServer.send(404, "text/plain", "Not Found");
}

boolean startAccessPoint() {
  // Begin some diagnostic information to console.
  Serial.println();
  Serial.print("Starting Wireless Access Point: ");
  String macAddr = String(WiFi.macAddress());
  Serial.print("Device WiFi MAC Address: ");
  Serial.println(macAddr);

  // Create an AP name unique to this device, to avoid stepping on others.
  String ap_ssid_suffix = macAddr.substring(12, 14) + macAddr.substring(15);
  ap_ssid = ap_ssid_prefix + "_" + ap_ssid_suffix; // Update AP broadcast name.
  return WiFi.softAP(ap_ssid, ap_default_passwd);
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
}

// Define server actions after declaring all functions for URL routing.
void startWebServer() {
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
  httpServer.onNotFound(handleNotFound);
  httpServer.begin();
  Serial.println("HTTP Server Started");
}
