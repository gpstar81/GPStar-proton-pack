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
 * Wireless Communications for ESP32 - WiFi & Bluetooth
 */
#include <WiFi.h>
#include <WebServer.h>

// Set up the SSID and password for the WiFi access point (AP).
const char* ap_ssid_prefix = "ProtonPack"; // This will be the base of the SSID name.
char* ap_default_passwd = "555-2368"; // This will be the default password for the AP.

// Simple networking info
IPAddress local_ip(192, 168, 1, 2);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer httpServer(80);

String startHTML() {
  // Common header for HTML content from the web server.
  String htmlCode = "<!DOCTYPE html>\n";
  htmlCode += "<html>\n";
  htmlCode += "<head>\n";
  htmlCode += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  htmlCode += "<meta http-equiv=\"refresh\" content=\"2\"/>";
  htmlCode += "<title>Proton Pack Attenuator</title>\n";
  htmlCode += "<style>\n";
  htmlCode += "html { font-family: Open Sans; display: inline-block;, margin: 0px auto; text-align: center; }\n";
  htmlCode += "body { margin-top: 100px; }\n";
  htmlCode += "h1 { color: #333; margin: 30px auto 30px; }\n";
  htmlCode += "h3 { color: #555; margin-bottom: 50px; }\n";
  htmlCode += "p { font-size: 18px; color: #888; margin-bottom: 10px; }\n";
  htmlCode += "</style>\n";
  htmlCode += "</head>\n";
  htmlCode += "<body>\n";
  return htmlCode;
}

String endHTML() {
  // Common footer for HTML content from the web server.
  String htmlCode = "</body>\n";
  htmlCode += "</html>";
  return htmlCode;
}

String getRootHTML() {
  String htmlCode = startHTML(); // Start the HTML document

  htmlCode += "<h1>Attenuator Control</h1>\n";

  htmlCode += "<p><b>Year Theme:</b> ";
  switch(YEAR_MODE) {
    case YEAR_1984:
      htmlCode += "1984";
    break;
    case YEAR_1989:
      htmlCode += "1989";
    break;
    case YEAR_2021:
      htmlCode += "Afterlife";
    break;
    default:
      htmlCode += "Unknown";
    break;
  }
  htmlCode += "</p></br>\n";

  htmlCode += "<p><b>Firing Mode:</b> ";
  switch(FIRING_MODE) {
    case PROTON:
      htmlCode += "Proton";
    break;
    case SLIME:
      htmlCode += "Slime";
    break;
    case STASIS:
      htmlCode += "Stasis";
    break;
    case MESON:
      htmlCode += "Meson";
    break;
    case SPECTRAL:
      htmlCode += "Spectral";
    break;
    case HOLIDAY:
      htmlCode += "Holiday";
    break;
    case SPECTRAL_CUSTOM:
      htmlCode += "Custom";
    break;
    case VENTING:
      htmlCode += "Venting";
    break;
    case SETTINGS:
      htmlCode += "Settings";
    break;
    default:
      htmlCode += "Unknown";
    break;
  }
  htmlCode += "</p></br>\n";

  htmlCode += "<p><b>Power Level:</b> ";
  switch(POWER_LEVEL) {
    case LEVEL_1:
      htmlCode += "1";
    break;
    case LEVEL_2:
      htmlCode += "2";
    break;
    case LEVEL_3:
      htmlCode += "3";
    break;
    case LEVEL_4:
      htmlCode += "4";
    break;
    case LEVEL_5:
      htmlCode += "5";
    break;
  }
  htmlCode += "</p></br>\n";

  htmlCode += endHTML(); // Close the HTML document
  return htmlCode;
}

void handleRoot() {
  // Used for the root page (/) of the web server.
  Serial.println("Web Root Requested");
  httpServer.send(200, "text/html", getRootHTML());
}

void handleNotFound() {
  // Returned for any invalid URL requested.
  Serial.println("Web Not Found");
  httpServer.send(404, "text/plain", "Not Found");
}
