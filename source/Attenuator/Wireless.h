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
  htmlCode += "<title>Proton Pack</title>\n";
  htmlCode += "<style>\n";
  htmlCode += "html { font-family: Open Sans; display: inline-block;, margin: 0px auto; text-align: center; }\n";
  htmlCode += "body { margin-top: 100px; }\n";
  htmlCode += "h1 { color: #333; margin: 30px auto 30px; }\n";
  htmlCode += "h3 { color: #555; margin-bottom: 50px; }\n";
  htmlCode += "proton { color: red; }\n";
  htmlCode += "slime { color: green; }\n";
  htmlCode += "statis { color: blue; }\n";
  htmlCode += "meson { color: orange; }\n";
  htmlCode += "spectral { color: purple; }\n";
  htmlCode += "holiday { color: purple; }\n";
  htmlCode += "custom { color: purple; }\n";
  htmlCode += "venting { color: maroon; }\n";
  htmlCode += "settings { color: gray; }\n";
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

  htmlCode += "<h1>Equipment Status</h1>\n";

  htmlCode += "<p><b>Operating Mode:</b> ";
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
      htmlCode += "<span class=\"proton\">Proton</span>";
    break;
    case SLIME:
      htmlCode += "<span class=\"slime\">Slime</span>";
    break;
    case STASIS:
      htmlCode += "<span class=\"stasis\">Stasis</span>";
    break;
    case MESON:
      htmlCode += "<span class=\"meson\">Meson</span>";
    break;
    case SPECTRAL:
      htmlCode += "<span class=\"spectral\">Spectral</span>";
    break;
    case HOLIDAY:
      htmlCode += "<span class=\"holiday\">Holiday</span>";
    break;
    case SPECTRAL_CUSTOM:
      htmlCode += "<span class=\"custom\">Custom</span>";
    break;
    case VENTING:
      htmlCode += "<span class=\"venting\">Venting</span>";
    break;
    case SETTINGS:
      htmlCode += "<span class=\"settings\">Settings</span>";
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

  htmlCode += "<p><b>Neutrona Wand State:</b> ";
  htmlCode += (b_firing ? "Firing" : "Idle");
  htmlCode += "</p></br>\n";

  htmlCode += "<p><b>Ribbon Cable State:</b> ";
  htmlCode += (b_pack_alarm ? "Disconnected" : "Connected");
  htmlCode += "</p></br>\n";

  htmlCode += "<p><b>Cyclotron State:</b> ";
  switch(i_speed_multiplier) {
    case 1:
      htmlCode += "Normal";
    break;
    case 2:
      htmlCode += "Active";
    break;
    case 3:
      htmlCode += "Warning";
    break;
    case 4:
    case 5:
      htmlCode += "Critical";
    break;
  }
  htmlCode += "</p></br>\n";

  htmlCode += "<p><b>Overheat State:</b> ";
  htmlCode += (b_overheating ? "Venting" : "Normal");
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
