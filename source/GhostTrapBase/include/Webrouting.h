/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2026 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                         & Dustin Grau <dustin.grau@gmail.com>
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

#pragma once

/**
 * Register all web routes for this device prior to calling setupRouting().
 *
 * Uses the common WebRouter.h include which provides all common ENUM and
 * HTTP types along with the addSimpleRoute() and addBodyRoute() functions.
 *
 * Relies on all URI handlers having been defined in Webhandler.h first.
 */
void registerWebRoutes() {
  // Body Handler Routes - POST endpoints that accept JSON request bodies

  addBodyRoute(
    "/config/device/save",
    handleSaveDeviceConfig,
    "Save device configuration",
    "Saves device settings including WiFi name, LED options, buzzer/vibration settings, and music track list",
    TAG_CONFIGURATION,
    "JSON object containing device configuration fields for WiFi name and other localized preferences.",
    RESP_SETTINGS_SAVED
  );

  addBodyRoute(
    "/password/update",
    passwordChangeHandler,
    "Update WiFi password",
    "Updates the WiFi access point password (minimum 8 characters for WPA2)",
    TAG_WIFI,
    "JSON object with 'password' field"
  );

  addBodyRoute(
    "/wifi/update",
    wifiChangeHandler,
    "Update WiFi network settings",
    "Updates the active WiFi network configuration including SSID, password, and optional static IP settings",
    TAG_WIFI,
    "JSON object with 'active' network and saved networks array"
  );

  // Simple Routes - Standard HTTP methods

  // Static Pages
  addSimpleRoute("/", HTTP_GET, handleRoot, "Root page", "Serves the main index.html page", TAG_PAGES, RESP_HTML_PAGE);
  addSimpleRoute("/common.js", HTTP_GET, handleCommonJS, "Common JavaScript", "Serves common JavaScript for all pages", TAG_ASSETS, RESP_JAVASCRIPT_FILE);
  addSimpleRoute("/favicon.ico", HTTP_GET, handleFavIco, "Favicon ICO", "Serves the favicon as ICO format", TAG_ASSETS, RESP_ICON_FILE);
  addSimpleRoute("/favicon.svg", HTTP_GET, handleFavSvg, "Favicon SVG", "Serves the favicon as SVG format", TAG_ASSETS, RESP_SVG_FILE);
  addSimpleRoute("/style.css", HTTP_GET, handleStylesheet, "Main stylesheet", "Serves the main CSS stylesheet", TAG_ASSETS, RESP_CSS_FILE);
  addSimpleRoute("/index.js", HTTP_GET, handleRootJS, "Index JavaScript", "Serves the JavaScript for the index page", TAG_ASSETS, RESP_JAVASCRIPT_FILE);
  addSimpleRoute("/network", HTTP_GET, handleNetwork, "Network page", "Serves the network configuration page", TAG_PAGES, RESP_HTML_PAGE);
  addSimpleRoute("/password", HTTP_GET, handlePassword, "Password page", "Serves the password change page", TAG_PAGES, RESP_HTML_PAGE);
  addSimpleRoute("/settings/device", HTTP_GET, handleDeviceSettings, "Device settings page", "Serves the device settings configuration page", TAG_PAGES, RESP_HTML_PAGE);
  addSimpleRoute("/equipment.svg", HTTP_GET, handleEquipSvg, "Equipment SVG", "Serves the equipment visualization SVG", TAG_ASSETS, RESP_SVG_FILE);

  // Configuration Endpoints
  addSimpleRoute("/config/device", HTTP_GET, handleGetDeviceConfig, "Get device config", "Returns current device configuration as JSON", TAG_CONFIGURATION, RESP_CONFIG_OBJECT);

  // System Status and Control
  addSimpleRoute("/status", HTTP_GET, handleGetStatus, "Get system status as JSON", "Returns current system status including mode, theme, and connected device info", TAG_SYSTEM, RESP_SYSTEM_STATUS);
  addSimpleRoute("/restart", HTTP_DELETE, handleRestart, "Restart device", "Performs a restart of the device", TAG_SYSTEM, RESP_NO_CONTENT_RESTART);

  // Device Control
  addSimpleRoute("/selftest/enable", HTTP_PUT, handleEnableSelfTest, "Enable self-test", "Enable device self-test mode", TAG_DEVICE_CONTROL);
  addSimpleRoute("/selftest/disable", HTTP_PUT, handleDisableSelfTest, "Disable self-test", "Disable device self-test mode", TAG_DEVICE_CONTROL);
  addSimpleRoute("/smoke/enable", HTTP_PUT, handleSmokeEnable, "Enable smoke", "Enable device smoke", TAG_DEVICE_CONTROL);
  addSimpleRoute("/smoke/disable", HTTP_PUT, handleSmokeDisable, "Disable smoke", "Disable device smoke", TAG_DEVICE_CONTROL);
  addSimpleRoute("/light/on", HTTP_PUT, handleLightOn, "Turn on lights", "Turn on device lights", TAG_DEVICE_CONTROL);
  addSimpleRoute("/light/off", HTTP_PUT, handleLightOff, "Turn off lights", "Turn off device lights", TAG_DEVICE_CONTROL);

  // WiFi Management
  addSimpleRoute("/wifi/restart", HTTP_GET, handleRestartWiFi, "Restart WiFi", "Restart the WiFi networking", TAG_WIFI, RESP_WIFI_SETTINGS);
  addSimpleRoute("/wifi/settings", HTTP_GET, handleGetWifi, "Get WiFi settings", "Returns current WiFi configuration", TAG_WIFI, RESP_WIFI_SETTINGS);
  addSimpleRoute("/wifi/networks", HTTP_GET, handleGetSSIDs, "Get available networks", "Returns list of nearby 2.4 GHz WiFi networks", TAG_WIFI, RESP_NETWORK_ARRAY);
  addSimpleRoute("/wifi/network/*", HTTP_DELETE, handleDeleteNetwork, "Delete saved network", "Deletes a saved WiFi network by index", TAG_WIFI);

  // OpenAPI Documentation
  addSimpleRoute("/swaggerui", HTTP_GET, handleSwagger, "Swagger UI", "Serves the Swagger UI page for API documentation", TAG_PAGES, RESP_HTML_PAGE);
  addSimpleRoute("/openapi.json", HTTP_GET, handleOpenAPISpec, "OpenAPI specification", "Returns the OpenAPI 3.x specification for this API", TAG_DOCUMENTATION, RESP_OPENAPI_SPEC);
}
