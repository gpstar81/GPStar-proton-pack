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
    "/config/pack/save",
    handleSavePackConfig,
    "Save pack configuration",
    "Saves Proton Pack settings including LED configurations and effects toggles",
    TAG_CONFIGURATION,
    "JSON object containing pack configuration fields"
  );

  addBodyRoute(
    "/config/wand/save",
    handleSaveWandConfig,
    "Save wand configuration",
    "Saves Neutrona Wand settings including LED configurations and effects toggles",
    TAG_CONFIGURATION,
    "JSON object containing wand configuration fields"
  );

  addBodyRoute(
    "/config/smoke/save",
    handleSaveSmokeConfig,
    "Save smoke configuration",
    "Saves smoke effect settings including overheat durations, continuous smoke settings, and per-level configurations",
    TAG_CONFIGURATION,
    "JSON object containing smoke configuration fields"
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
  addSimpleRoute("/settings/pack", HTTP_GET, handlePackSettings, "Pack settings page", "Serves the pack settings configuration page", TAG_PAGES, RESP_HTML_PAGE);
  addSimpleRoute("/settings/smoke", HTTP_GET, handleSmokeSettings, "Smoke settings page", "Serves the smoke settings configuration page", TAG_PAGES, RESP_HTML_PAGE);
  addSimpleRoute("/settings/wand", HTTP_GET, handleWandSettings, "Wand settings page", "Serves the wand settings configuration page", TAG_PAGES, RESP_HTML_PAGE);
  addSimpleRoute("/equipment.svg", HTTP_GET, handleEquipSvg, "Equipment SVG", "Serves the equipment visualization SVG", TAG_ASSETS, RESP_SVG_FILE);
  addSimpleRoute("/help.json", HTTP_GET, handleContextHelp, "Help JSON", "Serves the help JSON file", TAG_ASSETS, RESP_JSON_OBJECT);

  // Configuration Endpoints
  addSimpleRoute("/config/device", HTTP_GET, handleGetDeviceConfig, "Get device config", "Returns current device configuration as JSON", TAG_CONFIGURATION, RESP_CONFIG_OBJECT);
  addSimpleRoute("/config/pack", HTTP_GET, handleGetPackConfig, "Get pack config", "Returns current pack configuration as JSON", TAG_CONFIGURATION, RESP_CONFIG_OBJECT);
  addSimpleRoute("/config/smoke", HTTP_GET, handleGetSmokeConfig, "Get smoke config", "Returns current smoke configuration as JSON", TAG_CONFIGURATION, RESP_CONFIG_OBJECT);
  addSimpleRoute("/config/wand", HTTP_GET, handleGetWandConfig, "Get wand config", "Returns current wand configuration as JSON", TAG_CONFIGURATION, RESP_CONFIG_OBJECT);

  // EEPROM Operations
  addSimpleRoute("/eeprom/all", HTTP_PUT, handleSaveAllEEPROM, "Save pack/wand to EEPROM", "Saves settings to EEPROM for pack and wand", TAG_EEPROM);
  addSimpleRoute("/eeprom/pack", HTTP_PUT, handleSavePackEEPROM, "Save pack to EEPROM", "Saves pack settings to EEPROM", TAG_EEPROM);
  addSimpleRoute("/eeprom/pack", HTTP_DELETE, handleResetPackEEPROM, "Reset pack to factory defaults", "Resets pack settings to factory defaults", TAG_EEPROM);
  addSimpleRoute("/eeprom/wand", HTTP_PUT, handleSaveWandEEPROM, "Save wand to EEPROM", "Saves wand settings to EEPROM", TAG_EEPROM);
  addSimpleRoute("/eeprom/wand", HTTP_DELETE, handleResetWandEEPROM, "Reset wand to factory defaults", "Resets wand settings to factory defaults", TAG_EEPROM);

  // System Status and Control
  addSimpleRoute("/status", HTTP_GET, handleGetStatus, "Get system status as JSON", "Returns current system status including mode, theme, and connected device info", TAG_SYSTEM, RESP_SYSTEM_STATUS);
  addSimpleRoute("/restart", HTTP_DELETE, handleRestart, "Restart device", "Performs a restart of the device", TAG_SYSTEM, RESP_NO_CONTENT_RESTART);

  // Device Control
  addSimpleRoute("/pack/on", HTTP_PUT, handlePackOn, "Turn pack on", "Powers on the proton pack", TAG_DEVICE_CONTROL);
  addSimpleRoute("/pack/off", HTTP_PUT, handlePackOff, "Turn pack off", "Powers off the proton pack", TAG_DEVICE_CONTROL);
  addSimpleRoute("/pack/attenuate", HTTP_PUT, handleAttenuatePack, "Attenuate pack", "Cancels overheat warning on pack", TAG_DEVICE_CONTROL);
  addSimpleRoute("/pack/vent", HTTP_PUT, handleManualVent, "Manual vent", "Triggers manual vent sequence", TAG_DEVICE_CONTROL);
  addSimpleRoute("/pack/cable/on", HTTP_PUT, handleCableOn, "Ribbon cable on", "Cancels ribbon cable alarm", TAG_DEVICE_CONTROL);
  addSimpleRoute("/pack/cable/off", HTTP_PUT, handleCableOff, "Ribbon cable off", "Activates ribbon cable alarm", TAG_DEVICE_CONTROL);
  addSimpleRoute("/pack/lockout", HTTP_PUT, handleMashLockout, "Button mash lockout", "Triggers manual button mash lockout", TAG_DEVICE_CONTROL);
  addSimpleRoute("/pack/restart", HTTP_PUT, handleCancelLockout, "Cancel lockout", "Cancels button mash lockout", TAG_DEVICE_CONTROL);
  addSimpleRoute("/pack/cyclotron/clockwise", HTTP_PUT, handleCyclotronDirection, "Cyclotron clockwise", "Sets cyclotron rotation to clockwise", TAG_DEVICE_CONTROL);
  addSimpleRoute("/pack/cyclotron/counterclockwise", HTTP_PUT, handleCyclotronDirection, "Cyclotron counterclockwise", "Sets cyclotron rotation to counterclockwise", TAG_DEVICE_CONTROL);
  addSimpleRoute("/pack/smoke/on", HTTP_PUT, handleToggleSmoke, "Smoke on", "Enables smoke effects", TAG_DEVICE_CONTROL);
  addSimpleRoute("/pack/smoke/off", HTTP_PUT, handleToggleSmoke, "Smoke off", "Disables smoke effects", TAG_DEVICE_CONTROL);
  addSimpleRoute("/pack/vibration/on", HTTP_PUT, handleToggleVibration, "Vibration on", "Enables vibration feedback", TAG_DEVICE_CONTROL);
  addSimpleRoute("/pack/vibration/off", HTTP_PUT, handleToggleVibration, "Vibration off", "Disables vibration feedback", TAG_DEVICE_CONTROL);

  // Stream Mode Control
  addSimpleRoute("/pack/stream/proton", HTTP_PUT, handleStreamModeChange, "Proton stream", "Changes stream mode to standard Proton [default]", TAG_STREAM_MODES);
  addSimpleRoute("/pack/stream/stasis", HTTP_PUT, handleStreamModeChange, "Stasis stream", "Changes stream mode to Dark Matter Generator (Stasis)", TAG_STREAM_MODES);
  addSimpleRoute("/pack/stream/slime", HTTP_PUT, handleStreamModeChange, "Slime stream", "Changes stream mode to Plasm Distribution System (Slime)", TAG_STREAM_MODES);
  addSimpleRoute("/pack/stream/meson", HTTP_PUT, handleStreamModeChange, "Meson stream", "Changes stream mode to Composite Particle System (Meson)", TAG_STREAM_MODES);
  addSimpleRoute("/pack/stream/spectral", HTTP_PUT, handleStreamModeChange, "Spectral stream", "Changes stream mode to Spectral (Rainbow)", TAG_STREAM_MODES);
  addSimpleRoute("/pack/stream/holiday/halloween", HTTP_PUT, handleStreamModeChange, "Halloween stream", "Changes stream mode to Holiday: Halloween", TAG_STREAM_MODES);
  addSimpleRoute("/pack/stream/holiday/christmas", HTTP_PUT, handleStreamModeChange, "Christmas stream", "Changes stream mode to Holiday: Christmas", TAG_STREAM_MODES);
  addSimpleRoute("/pack/stream/spectral/custom", HTTP_PUT, handleStreamModeChange, "Custom spectral stream", "Changes stream mode to Spectral Custom", TAG_STREAM_MODES);

  // Theme Control
  addSimpleRoute("/pack/theme/1984", HTTP_PUT, handleThemeChange, "Ghostbusters (1984) Theme", "Changes system theme to 1984", TAG_THEMES);
  addSimpleRoute("/pack/theme/1989", HTTP_PUT, handleThemeChange, "Ghostbusters II (1989) Theme", "Changes system theme to 1989 (Ghostbusters II)", TAG_THEMES);
  addSimpleRoute("/pack/theme/2021", HTTP_PUT, handleThemeChange, "Ghostbusters: Afterlife (2021) Theme", "Changes system theme to 2021 (Afterlife)", TAG_THEMES);
  addSimpleRoute("/pack/theme/2024", HTTP_PUT, handleThemeChange, "Ghostbusters: Frozen Empire (2024) Theme", "Changes system theme to 2024 (Frozen Empire)", TAG_THEMES);

  // Volume Control
  addSimpleRoute("/volume/mute", HTTP_PUT, handleToggleMute, "Mute audio", "Mutes all audio output", TAG_VOLUME_CONTROL);
  addSimpleRoute("/volume/unmute", HTTP_PUT, handleToggleMute, "Unmute audio", "Unmutes all audio output", TAG_VOLUME_CONTROL);
  addSimpleRoute("/volume/master/up", HTTP_PUT, handleMasterVolumeUp, "Master volume up", "Increases master volume level", TAG_VOLUME_CONTROL);
  addSimpleRoute("/volume/master/down", HTTP_PUT, handleMasterVolumeDown, "Master volume down", "Decreases master volume level", TAG_VOLUME_CONTROL);
  addSimpleRoute("/volume/master/set/*", HTTP_PUT, handleMasterVolumeSet, "Set master volume", "Sets master volume level to a specific value (0-100)", TAG_VOLUME_CONTROL);
  addSimpleRoute("/volume/effects/up", HTTP_PUT, handleEffectsVolumeUp, "Effects volume up", "Increases sound effects volume", TAG_VOLUME_CONTROL);
  addSimpleRoute("/volume/effects/down", HTTP_PUT, handleEffectsVolumeDown, "Effects volume down", "Decreases sound effects volume", TAG_VOLUME_CONTROL);
  addSimpleRoute("/volume/music/up", HTTP_PUT, handleMusicVolumeUp, "Music volume up", "Increases music volume", TAG_VOLUME_CONTROL);
  addSimpleRoute("/volume/music/down", HTTP_PUT, handleMusicVolumeDown, "Music volume down", "Decreases music volume", TAG_VOLUME_CONTROL);

  // Music Control
  addSimpleRoute("/music/startstop", HTTP_PUT, handleMusicStartStop, "Start/stop music", "Toggles music playback on/off", TAG_MUSIC_CONTROL);
  addSimpleRoute("/music/pauseresume", HTTP_PUT, handleMusicPauseResume, "Pause/resume music", "Toggles music pause state", TAG_MUSIC_CONTROL);
  addSimpleRoute("/music/next", HTTP_PUT, handleNextMusicTrack, "Next track", "Skips to next music track", TAG_MUSIC_CONTROL);
  addSimpleRoute("/music/select", HTTP_PUT, handleSelectMusicTrack, "Select track", "Selects specific music track by number (query param: track [int])", TAG_MUSIC_CONTROL);
  addSimpleRoute("/music/prev", HTTP_PUT, handlePrevMusicTrack, "Previous track", "Returns to previous music track", TAG_MUSIC_CONTROL);
  addSimpleRoute("/music/loop/all", HTTP_PUT, handleLoopMusicTrack, "Loop all tracks", "Sets music to loop all tracks", TAG_MUSIC_CONTROL);
  addSimpleRoute("/music/loop/one", HTTP_PUT, handleLoopMusicTrack, "Loop one track", "Sets music to loop current track", TAG_MUSIC_CONTROL);
  addSimpleRoute("/music/shuffle/on", HTTP_PUT, handleShuffleMusicTracks, "Shuffle on", "Enables track shuffle mode", TAG_MUSIC_CONTROL);
  addSimpleRoute("/music/shuffle/off", HTTP_PUT, handleShuffleMusicTracks, "Shuffle off", "Disables track shuffle mode", TAG_MUSIC_CONTROL);

  // WiFi Management
  addSimpleRoute("/wifi/settings", HTTP_GET, handleGetWifi, "Get WiFi settings", "Returns current WiFi configuration", TAG_WIFI, RESP_WIFI_SETTINGS);
  addSimpleRoute("/wifi/networks", HTTP_GET, handleGetSSIDs, "Get available networks", "Returns list of nearby 2.4 GHz WiFi networks", TAG_WIFI, RESP_NETWORK_ARRAY);
  addSimpleRoute("/wifi/network/*", HTTP_DELETE, handleDeleteNetwork, "Delete saved network", "Deletes a saved WiFi network by index", TAG_WIFI);

  // OpenAPI Documentation
  addSimpleRoute("/swaggerui", HTTP_GET, handleSwagger, "Swagger UI", "Serves the Swagger UI page for API documentation", TAG_PAGES, RESP_HTML_PAGE);
  addSimpleRoute("/openapi.json", HTTP_GET, handleOpenAPISpec, "OpenAPI specification", "Returns the OpenAPI 3.x specification for this API", TAG_DOCUMENTATION, RESP_OPENAPI_SPEC);

  // Captive Portal / Connectivity Check Endpoints
  // DNS hijacking redirects OS connectivity checks to the device.
  // By returning proper HTTP responses (204 for Android, Success HTML for iOS),
  // we signal "captive portal authenticated" which allows cellular to stay active.
  // Gateway 0.0.0.0 ensures iOS keeps cellular active (no internet route via WiFi).
  addSimpleRoute("/success.txt", HTTP_GET, handleConnectivityCheck, "Android connectivity test", "Captive portal detection endpoint", TAG_PORTAL, RESP_CONNECTIVITY_CHECK);
  addSimpleRoute("/generate_204", HTTP_GET, handleConnectivityCheck, "Android connectivity check", "Captive portal detection endpoint", TAG_PORTAL, RESP_CONNECTIVITY_CHECK);
  addSimpleRoute("/gen_204", HTTP_GET, handleConnectivityCheck, "Google connectivity check", "Captive portal detection endpoint", TAG_PORTAL, RESP_CONNECTIVITY_CHECK);
  addSimpleRoute("/ncsi.txt", HTTP_GET, handleConnectivityCheck, "Windows connectivity check", "Captive portal detection endpoint", TAG_PORTAL, RESP_CONNECTIVITY_CHECK);
  addSimpleRoute("/hotspot-detect.html", HTTP_GET, handleConnectivityCheck, "iOS captive portal check", "Captive portal detection endpoint", TAG_PORTAL, RESP_CONNECTIVITY_CHECK);
  addSimpleRoute("/library/test/success.html", HTTP_GET, handleConnectivityCheck, "iOS connectivity check", "Captive portal detection endpoint", TAG_PORTAL, RESP_CONNECTIVITY_CHECK);
  addSimpleRoute("/connecttest.txt", HTTP_GET, handleConnectivityCheck, "Windows connectivity test", "Captive portal detection endpoint", TAG_PORTAL, RESP_CONNECTIVITY_CHECK);
  addSimpleRoute("/redirect", HTTP_GET, handleConnectivityCheck, "Windows redirect check", "Captive portal detection endpoint", TAG_PORTAL, RESP_CONNECTIVITY_CHECK);
}
