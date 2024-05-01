/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
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

#pragma once

// Forward function declarations.
bool startAccesPoint();
bool startExternalWifi();

/*
 * Web Handler Functions - Performs actions or returns data for web UI
 */
JsonDocument jsonBody; // Used for processing JSON body/payload data.
JsonDocument jsonSuccess; // Used for sending JSON status as success.
String status; // Holder for simple "status: success" response.

void handleRoot(AsyncWebServerRequest *request) {
  // Used for the root page (/) from the web server.
  //debug("Web Root HTML Requested");
  String s = INDEX_page; // Read HTML page into String.
  request->send(200, "text/html", s); // Serve page content.
}

void handleNetwork(AsyncWebServerRequest *request) {
  // Used for the network page from the web server.
  //debug("Network HTML Requested");
  String s = NETWORK_page; // Read HTML page into String.
  request->send(200, "text/html", s); // Serve page content.
}

void handlePassword(AsyncWebServerRequest *request) {
  // Used for the password page from the web server.
  //debug("Password HTML Requested");
  String s = PASSWORD_page; // Read HTML page into String.
  request->send(200, "text/html", s); // Serve page content.
}

void handleAttenuatorSettings(AsyncWebServerRequest *request) {
  // Used for the device page from the web server.
  //debug("Attenuator Settings HTML Requested");
  String s = DEVICE_page; // Read HTML page into String.
  request->send(200, "text/html", s); // Serve page content.
}

void handlePackSettings(AsyncWebServerRequest *request) {
  // Tell the pack that we'll need the latest pack EEPROM values.
  b_received_prefs_pack = false;
  attenuatorSerialSend(A_REQUEST_PREFERENCES_PACK);

  // Used for the settings page from the web server.
  //debug("Pack Settings HTML Requested");
  String s = PACK_SETTINGS_page; // Read HTML page into String.
  request->send(200, "text/html", s); // Serve page content.
}

void handleWandSettings(AsyncWebServerRequest *request) {
  // Tell the pack that we'll need the latest wand EEPROM values.
  b_received_prefs_wand = false;
  attenuatorSerialSend(A_REQUEST_PREFERENCES_WAND);

  // Used for the settings page from the web server.
  //debug("Wand Settings HTML Requested");
  String s = WAND_SETTINGS_page; // Read HTML page into String.
  request->send(200, "text/html", s); // Serve page content.
}

void handleSmokeSettings(AsyncWebServerRequest *request) {
  // Tell the pack that we'll need the latest smoke EEPROM values.
  b_received_prefs_smoke = false;
  attenuatorSerialSend(A_REQUEST_PREFERENCES_SMOKE);

  // Used for the settings page from the web server.
  //debug("Smoke Settings HTML Requested");
  String s = SMOKE_SETTINGS_page; // Read HTML page into String.
  request->send(200, "text/html", s); // Serve page content.
}

void handleStylesheet(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  //debug("Main StyleSheet Requested");
  String s = STYLE_page; // Read CSS page into String.
  request->send(200, "text/css", s); // Serve page content.
}

String getAttenuatorConfig() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonBody.clear();

  // Provide current values for the Attenuator device.
  jsonBody["invertLEDs"] = b_invert_leds;
  jsonBody["buzzer"] = b_enable_buzzer;
  jsonBody["vibration"] = b_enable_vibration;
  jsonBody["overheat"] = b_overheat_feedback;
  jsonBody["firing"] = b_firing_feedback;
  jsonBody["radLensIdle"] = RAD_LENS_IDLE;

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getPackConfig() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonBody.clear();

  if(!b_wait_for_pack) {
    // Provide a flag to indicate prefs were received via serial coms.
    jsonBody["prefsAvailable"] = b_received_prefs_pack;

    // Return current powered state for pack and wand.
    jsonBody["packPowered"] = (b_pack_on ? true : false);
    jsonBody["wandPowered"] = (b_wand_on ? true : false);

    // Proton Pack Runtime Options
    jsonBody["defaultSystemModePack"] = packConfig.defaultSystemModePack; // [0=SH,1=MO]
    jsonBody["defaultYearThemePack"] = packConfig.defaultYearThemePack; // [1=TOGGLE,2=1984,3=1989,4=2021,5=2024]
    jsonBody["defaultSystemVolume"] = packConfig.defaultSystemVolume; // 0-100
    jsonBody["packVibration"] = packConfig.packVibration; // [1=ALWAYS,2=FIRING,3=NEVER,4=TOGGLE]
    jsonBody["protonStreamEffects"] = packConfig.protonStreamEffects; // true|false
    jsonBody["ribbonCableAlarm"] = packConfig.ribbonCableAlarm; // true|false
    jsonBody["overheatStrobeNF"] = packConfig.overheatStrobeNF; // true|false
    jsonBody["overheatLightsOff"] = packConfig.overheatLightsOff; // true|false
    jsonBody["overheatSyncToFan"] = packConfig.overheatSyncToFan;// true|false
    jsonBody["demoLightMode"] = packConfig.demoLightMode; // true|false

    // Proton Pack LED Options
    jsonBody["ledCycLidCount"] = packConfig.ledCycLidCount; // [12,20,36,40]
    jsonBody["ledCycLidHue"] = packConfig.ledCycLidHue; // Spectral custom color/hue 1-254
    jsonBody["ledCycLidSat"] = packConfig.ledCycLidSat; // Spectral custom saturation 1-254
    jsonBody["cyclotronDirection"] = packConfig.cyclotronDirection; // [0=CCW,1=CW]
    jsonBody["ledCycLidCenter"] = packConfig.ledCycLidCenter; // [0=3,1=1]
    jsonBody["ledVGCyclotron"] = packConfig.ledVGCyclotron; // true|false
    jsonBody["ledCycLidSimRing"] = packConfig.ledCycLidSimRing; // true|false
    jsonBody["ledCycCakeCount"] = packConfig.ledCycCakeCount; // [12,23,24,35]
    jsonBody["ledCycCakeHue"] = packConfig.ledCycCakeHue; // Spectral custom color/hue 1-254
    jsonBody["ledCycCakeSat"] = packConfig.ledCycCakeSat; // Spectral custom saturation 1-254
    jsonBody["ledCycCakeGRB"] = packConfig.ledCycCakeGRB; // Use GRB for cake LEDs true|false
    jsonBody["ledCycCavCount"] = packConfig.ledCycCavCount; // Cyclotron cavity LEDs (0-30)
    jsonBody["ledPowercellCount"] = packConfig.ledPowercellCount; //[13,15]
    jsonBody["ledPowercellHue"] = packConfig.ledPowercellHue; // Spectral custom color/hue 1-254
    jsonBody["ledPowercellSat"] = packConfig.ledPowercellSat; // Spectral custom saturation 1-254
    jsonBody["ledVGPowercell"] = packConfig.ledVGPowercell; // true|false
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getWandConfig() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonBody.clear();

  if(!b_wait_for_pack) {
    // Provide a flag to indicate prefs were received via serial coms.
    jsonBody["prefsAvailable"] = b_received_prefs_wand;

    // Return current powered state for pack and wand.
    jsonBody["packPowered"] = (b_pack_on ? true : false);
    jsonBody["wandPowered"] = (b_wand_on ? true : false);

    // Neutrona Wand LED Options
    jsonBody["ledWandCount"] = wandConfig.ledWandCount; // [0=5,1=29,2=48]
    jsonBody["ledWandHue"] = wandConfig.ledWandHue; // Spectral custom color/hue 1-254
    jsonBody["ledWandSat"] = wandConfig.ledWandSat; // Spectral custom saturation 1-254
    jsonBody["spectralModeEnabled"] = wandConfig.spectralModeEnabled; // true|false
    jsonBody["spectralHolidayMode"] = wandConfig.spectralHolidayMode; // true|false

    // Neutrona Wand Runtime Options
    jsonBody["overheatEnabled"] = wandConfig.overheatEnabled; // true|false
    jsonBody["defaultFiringMode"] = wandConfig.defaultFiringMode; // [0=VG,1=CTS,2=SYSTEM]
    jsonBody["wandVibration"] = wandConfig.wandVibration; // [1=ALWAYS,2=FIRING,3=NEVER,4=TOGGLE]
    jsonBody["wandSoundsToPack"] = wandConfig.wandSoundsToPack; // true|false
    jsonBody["quickVenting"] = wandConfig.quickVenting; // true|false (Super-Hero Mode Only)
    jsonBody["autoVentLight"] = wandConfig.autoVentLight; // true|false
    jsonBody["wandBeepLoop"] = wandConfig.wandBeepLoop; // true|false (Afterlife/Frozen Empire Only)
    jsonBody["wandBootError"] = wandConfig.wandBootError; // true|false (Super-Hero Mode Only)
    jsonBody["defaultYearModeWand"] = wandConfig.defaultYearModeWand; // [1=TOGGLE,2=1984,3=1989,4=2021,5=2024]
    jsonBody["defaultYearModeCTS"] = wandConfig.defaultYearModeCTS; // [1=TOGGLE,2=1984,3=1989,4=2021,5=2024]
    jsonBody["invertWandBargraph"] = wandConfig.invertWandBargraph; // true|false
    jsonBody["bargraphOverheatBlink"] = wandConfig.bargraphOverheatBlink; // true|false
    jsonBody["bargraphIdleAnimation"] = wandConfig.bargraphIdleAnimation; // [1=SYSTEM,2=SH,3=MO]
    jsonBody["bargraphFireAnimation"] = wandConfig.bargraphFireAnimation; // [1=SYSTEM,2=SH,3=MO]
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getSmokeConfig() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonBody.clear();

  if(!b_wait_for_pack) {
    // Provide a flag to indicate prefs were received via serial coms.
    jsonBody["prefsAvailable"] = b_received_prefs_smoke;

    // Return current powered state for pack and wand.
    jsonBody["packPowered"] = (b_pack_on ? true : false);
    jsonBody["wandPowered"] = (b_wand_on ? true : false);

    // Proton Pack
    jsonBody["smokeEnabled"] = smokeConfig.smokeEnabled; // true|false

    // Power Level 5
    jsonBody["overheatDuration5"] = smokeConfig.overheatDuration5; // 2-60
    jsonBody["overheatContinuous5"] = smokeConfig.overheatContinuous5; // true|false
    // Power Level 4
    jsonBody["overheatDuration4"] = smokeConfig.overheatDuration4; // 2-60
    jsonBody["overheatContinuous4"] = smokeConfig.overheatContinuous4; // true|false
    // Power Level 3
    jsonBody["overheatDuration3"] = smokeConfig.overheatDuration3; // 2-60
    jsonBody["overheatContinuous3"] = smokeConfig.overheatContinuous3; // true|false
    // Power Level 2
    jsonBody["overheatDuration2"] = smokeConfig.overheatDuration2; // 2-60
    jsonBody["overheatContinuous2"] = smokeConfig.overheatContinuous2; // true|false
    // Power Level 1
    jsonBody["overheatDuration1"] = smokeConfig.overheatDuration1; // 2-60
    jsonBody["overheatContinuous1"] = smokeConfig.overheatContinuous1; // true|false

    // Neutrona Wand

    // Power Level 5
    jsonBody["overheatLevel5"] = smokeConfig.overheatLevel5; // true|false
    jsonBody["overheatDelay5"] = smokeConfig.overheatDelay5; // 2-60
    // Power Level 4
    jsonBody["overheatLevel4"] = smokeConfig.overheatLevel4; // true|false
    jsonBody["overheatDelay4"] = smokeConfig.overheatDelay4; // 2-60
    // Power Level 3
    jsonBody["overheatLevel3"] = smokeConfig.overheatLevel3; // true|false
    jsonBody["overheatDelay3"] = smokeConfig.overheatDelay3; // 2-60
    // Power Level 2
    jsonBody["overheatLevel2"] = smokeConfig.overheatLevel2; // true|false
    jsonBody["overheatDelay2"] = smokeConfig.overheatDelay2; // 2-60
    // Power Level 1
    jsonBody["overheatLevel1"] = smokeConfig.overheatLevel1; // true|false
    jsonBody["overheatDelay1"] = smokeConfig.overheatDelay1; // 2-60
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipSettings);
  return equipSettings;
}

String getEquipmentStatus() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipStatus;
  jsonBody.clear();

  if(!b_wait_for_pack) {
    // Only prepare status when not waiting on the pack
    jsonBody["mode"] = getMode();
    jsonBody["theme"] = getTheme();
    jsonBody["switch"] = getRedSwitch();
    jsonBody["pack"] = (b_pack_on ? "Powered" : "Idle");
    jsonBody["power"] = getPower();
    jsonBody["safety"] = getSafety();
    jsonBody["wand"] = (b_wand_present ? "Connected" : "Not Connected");
    jsonBody["wandPower"] = (b_wand_on ? "Powered" : "Idle");
    jsonBody["wandMode"] = getWandMode();
    jsonBody["firing"] = (b_firing ? "Firing" : "Idle");
    jsonBody["cable"] = (b_pack_alarm ? "Disconnected" : "Connected");
    jsonBody["cyclotron"] = getCyclotronState();
    jsonBody["temperature"] = (b_overheating ? "Venting" : "Normal");
    jsonBody["musicPlaying"] = b_playing_music;
    jsonBody["musicPaused"] = b_music_paused;
    jsonBody["musicCurrent"] = i_music_track_current;
    jsonBody["musicStart"] = i_music_track_min;
    jsonBody["musicEnd"] = i_music_track_max;
    jsonBody["volMaster"] = i_volume_master_percentage;
    jsonBody["volEffects"] = i_volume_effects_percentage;
    jsonBody["volMusic"] = i_volume_music_percentage;
    jsonBody["battVoltage"] = f_batt_volts;
  }

  // Serialize JSON object to string.
  serializeJson(jsonBody, equipStatus);
  return equipStatus;
}

String getWifiSettings() {
  // Prepare a JSON object with information stored in preferences (or a blank default).
  String wifiNetwork;
  jsonBody.clear();

  preferences.begin("network", true); // Access namespace in read-only mode.
  jsonBody["enabled"] = preferences.getBool("enabled", false);
  jsonBody["network"] = preferences.getString("ssid");
  jsonBody["password"] = preferences.getString("password");
  jsonBody["address"] = preferences.getString("address");
  if(jsonBody["address"] == "") {
    jsonBody["address"] = wifi_address;
  }
  jsonBody["subnet"] = preferences.getString("subnet");
  if(jsonBody["subnet"] == "") {
    jsonBody["subnet"] = wifi_subnet;
  }
  jsonBody["gateway"] = preferences.getString("gateway");
  if(jsonBody["gateway"] == "") {
    jsonBody["gateway"] = wifi_gateway;
  }
  preferences.end();

  // Serialize JSON object to string.
  serializeJson(jsonBody, wifiNetwork);
  return wifiNetwork;
}

void handleGetAttenuatorConfig(AsyncWebServerRequest *request) {
  // Return current attenuator settings as a stringified JSON object.
  request->send(200, "application/json", getAttenuatorConfig());
}

void handleGetPackConfig(AsyncWebServerRequest *request) {
  // Return current pack settings as a stringified JSON object.
  request->send(200, "application/json", getPackConfig());
}

void handleGetWandConfig(AsyncWebServerRequest *request) {
  // Return current wand settings as a stringified JSON object.
  request->send(200, "application/json", getWandConfig());
}

void handleGetSmokeConfig(AsyncWebServerRequest *request) {
  // Return current smoke settings as a stringified JSON object.
  request->send(200, "application/json", getSmokeConfig());
}

void handleGetStatus(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getEquipmentStatus());
}

void handleGetWifi(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getWifiSettings());
}

void handleRestart(AsyncWebServerRequest *request) {
  // Performs a restart of the device.
  request->send(204, "application/json", status);
  delay(1000);
  ESP.restart();
}

void handlePackOn(AsyncWebServerRequest *request) {
  debug("Turn Pack On");
  attenuatorSerialSend(A_TURN_PACK_ON);
  request->send(200, "application/json", status);
}

void handlePackOff(AsyncWebServerRequest *request) {
  debug("Turn Pack Off");
  attenuatorSerialSend(A_TURN_PACK_OFF);
  request->send(200, "application/json", status);
}

void handleAttenuatePack(AsyncWebServerRequest *request) {
  if(i_speed_multiplier > 2) {
    // Only send command to pack if cyclotron is not "normal".
    debug("Cancel Overheat Warning");
    attenuatorSerialSend(A_WARNING_CANCELLED);
    request->send(200, "application/json", status);
  } else {
    // Tell the user why the requested action failed.
    String result;
    jsonBody.clear();
    jsonBody["status"] = "System not in overheat warning";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
}

void handleManualVent(AsyncWebServerRequest *request) {
  debug("Manual Vent Triggered");
  attenuatorSerialSend(A_MANUAL_OVERHEAT);
  request->send(200, "application/json", status);
}

void handleToggleMute(AsyncWebServerRequest *request) {
  debug("Toggle Mute");
  attenuatorSerialSend(A_TOGGLE_MUTE);
  request->send(200, "application/json", status);
}

void handleMasterVolumeUp(AsyncWebServerRequest *request) {
  debug("Master Volume Up");
  attenuatorSerialSend(A_VOLUME_INCREASE);
  request->send(200, "application/json", status);
}

void handleMasterVolumeDown(AsyncWebServerRequest *request) {
  debug("Master Volume Down");
  attenuatorSerialSend(A_VOLUME_DECREASE);
  request->send(200, "application/json", status);
}

void handleEffectsVolumeUp(AsyncWebServerRequest *request) {
  debug("Effects Volume Up");
  attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_INCREASE);
  request->send(200, "application/json", status);
}

void handleEffectsVolumeDown(AsyncWebServerRequest *request) {
  debug("Effects Volume Down");
  attenuatorSerialSend(A_VOLUME_SOUND_EFFECTS_DECREASE);
  request->send(200, "application/json", status);
}

void handleMusicStartStop(AsyncWebServerRequest *request) {
  debug("Music Start/Stop");
  attenuatorSerialSend(A_MUSIC_START_STOP);
  request->send(200, "application/json", status);
}

void handleMusicPauseResume(AsyncWebServerRequest *request) {
  debug("Music Pause/Resume");
  attenuatorSerialSend(A_MUSIC_PAUSE_RESUME);
  request->send(200, "application/json", status);
}

void handleNextMusicTrack(AsyncWebServerRequest *request) {
  debug("Next Music Track");
  attenuatorSerialSend(A_MUSIC_NEXT_TRACK);
  request->send(200, "application/json", status);
}

void handlePrevMusicTrack(AsyncWebServerRequest *request) {
  debug("Prev Music Track");
  attenuatorSerialSend(A_MUSIC_PREV_TRACK);
  request->send(200, "application/json", status);
}

void handleSelectMusicTrack(AsyncWebServerRequest *request) {
  String c_music_track = "";

  if(request->hasParam("track")) {
    // Get the parameter "track" if it exists (will be a String).
    c_music_track = request->getParam("track")->value();
  }

  if(c_music_track.toInt() != 0 && c_music_track.toInt() >= i_music_track_min) {
    uint16_t i_music_track = c_music_track.toInt();
    debug("Selected Music Track: " + String(i_music_track));
    attenuatorSerialSend(A_MUSIC_PLAY_TRACK, i_music_track); // Inform the pack of the new track.
    request->send(200, "application/json", status);
  }
  else {
    // Tell the user why the requested action failed.
    String result;
    jsonBody.clear();
    jsonBody["status"] = "Invalid track number requested";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
}

void handleSaveAllEEPROM(AsyncWebServerRequest *request) {
  debug("Save All EEPROM");
  attenuatorSerialSend(A_SAVE_EEPROM_SETTINGS_PACK);
  attenuatorSerialSend(A_SAVE_EEPROM_SETTINGS_WAND);
  request->send(200, "application/json", status);
}

void handleSavePackEEPROM(AsyncWebServerRequest *request) {
  debug("Save Pack EEPROM");
  attenuatorSerialSend(A_SAVE_EEPROM_SETTINGS_PACK);
  request->send(200, "application/json", status);
}

void handleSaveWandEEPROM(AsyncWebServerRequest *request) {
  debug("Save Wand EEPROM");
  attenuatorSerialSend(A_SAVE_EEPROM_SETTINGS_WAND);
  request->send(200, "application/json", status);
}

// Handles the JSON body for the pack settings save request.
AsyncCallbackJsonWebHandler *handleSaveAttenuatorConfig = new AsyncCallbackJsonWebHandler("/config/attenuator/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  jsonBody.clear();
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    Serial.print("Body was not a JSON object");
  }

  String result;
  try {
    // General Options - Returned as unsigned integers
    if(jsonBody["invertLEDs"].is<unsigned short>()) {
      // Inverts the order of the LEDs as seen by the device.
      b_invert_leds = jsonBody["invertLEDs"].as<boolean>();
    }
    if(jsonBody["buzzer"].is<unsigned short>()) {
      // Enable/disable the buzzer completely.
      b_enable_buzzer = jsonBody["buzzer"].as<boolean>();
    }
    if(jsonBody["vibration"].is<unsigned short>()) {
      // Enable/disable vibration completely.
      b_enable_vibration = jsonBody["vibration"].as<boolean>();
    }
    if(jsonBody["overheat"].is<unsigned short>()) {
      // Enable/disable all buzzer/vibration feedback during overheat/alarm.
      b_overheat_feedback = jsonBody["overheat"].as<boolean>();
    }
    if(jsonBody["firing"].is<unsigned short>()) {
      // Enable/disable vibration when throwing a stream.
      b_firing_feedback = jsonBody["firing"].as<boolean>();
    }
    if(jsonBody["radLensIdle"].is<unsigned short>()) {
      switch(jsonBody["radLensIdle"].as<unsigned short>()) {
        case 0:
          RAD_LENS_IDLE = AMBER_PULSE;
        break;
        case 1:
          RAD_LENS_IDLE = ORANGE_FADE;
        break;
        case 2:
          RAD_LENS_IDLE = RED_FADE;
        break;
      }
    }

    preferences.begin("device", false); // Access namespace in read/write mode.
    preferences.putBool("invert_led", b_invert_leds);
    preferences.putBool("buzzer_enabled", b_enable_buzzer);
    preferences.putBool("vibration_enabled", b_enable_vibration);
    preferences.putBool("overheat_feedback", b_overheat_feedback);
    preferences.putBool("firing_feedback", b_firing_feedback);
    preferences.putShort("radiation_idle", RAD_LENS_IDLE);
    preferences.end();

    jsonBody.clear();
    jsonBody["status"] = "Settings updated on Attenuator";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
  catch (...) {
    jsonBody.clear();
    jsonBody["status"] = "An error was encountered while saving settings.";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
});

// Handles the JSON body for the pack settings save request.
AsyncCallbackJsonWebHandler *handleSavePackConfig = new AsyncCallbackJsonWebHandler("/config/pack/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  jsonBody.clear();
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    Serial.print("Body was not a JSON object");
  }

  String result;
  if(!b_pack_on && !b_wand_on) {
    try {
      // General Options
      packConfig.defaultSystemModePack = jsonBody["defaultSystemModePack"].as<uint8_t>();
      packConfig.defaultYearThemePack = jsonBody["defaultYearThemePack"].as<uint8_t>();
      packConfig.defaultSystemVolume = jsonBody["defaultSystemVolume"].as<uint8_t>();
      packConfig.packVibration = jsonBody["packVibration"].as<uint8_t>();
      packConfig.ribbonCableAlarm = jsonBody["ribbonCableAlarm"].as<uint8_t>();
      packConfig.protonStreamEffects = jsonBody["protonStreamEffects"].as<uint8_t>();
      packConfig.overheatStrobeNF = jsonBody["overheatStrobeNF"].as<uint8_t>();
      packConfig.overheatLightsOff = jsonBody["overheatLightsOff"].as<uint8_t>();
      packConfig.overheatSyncToFan = jsonBody["overheatSyncToFan"].as<uint8_t>();
      packConfig.demoLightMode = jsonBody["demoLightMode"].as<uint8_t>();

      // Update certain operational values immediately.
      switch(packConfig.defaultSystemModePack) {
        case 0:
        default:
          SYSTEM_MODE = MODE_SUPER_HERO;
          RED_SWITCH_MODE = SWITCH_OFF;
        break;

        case 1:
          SYSTEM_MODE = MODE_ORIGINAL;
          RED_SWITCH_MODE = SWITCH_OFF;
        break;
      }

      // Cyclotron Lid
      packConfig.ledCycLidCount = jsonBody["ledCycLidCount"].as<uint8_t>();
      packConfig.ledCycLidHue = jsonBody["ledCycLidHue"].as<uint8_t>();
      packConfig.ledCycLidSat = jsonBody["ledCycLidSat"].as<uint8_t>();
      packConfig.cyclotronDirection = jsonBody["cyclotronDirection"].as<uint8_t>();
      packConfig.ledCycLidCenter = jsonBody["ledCycLidCenter"].as<uint8_t>();
      packConfig.ledVGCyclotron = jsonBody["ledVGCyclotron"].as<uint8_t>();
      packConfig.ledCycLidSimRing = jsonBody["ledCycLidSimRing"].as<uint8_t>();

      // Inner Cyclotron
      packConfig.ledCycCakeCount = jsonBody["ledCycCakeCount"].as<uint8_t>();
      packConfig.ledCycCakeHue = jsonBody["ledCycCakeHue"].as<uint8_t>();
      packConfig.ledCycCakeSat = jsonBody["ledCycCakeSat"].as<uint8_t>();
      packConfig.ledCycCakeGRB = jsonBody["ledCycCakeGRB"].as<uint8_t>();
      packConfig.ledCycCavCount = jsonBody["ledCycCavCount"].as<uint8_t>();

      // Power Cell
      packConfig.ledPowercellCount = jsonBody["ledPowercellCount"].as<uint8_t>();
      packConfig.ledPowercellHue = jsonBody["ledPowercellHue"].as<uint8_t>();
      packConfig.ledPowercellSat = jsonBody["ledPowercellSat"].as<uint8_t>();
      packConfig.ledVGPowercell = jsonBody["ledVGPowercell"].as<uint8_t>();

      jsonBody.clear();
      jsonBody["status"] = "Settings updated, please test before saving to EEPROM.";
      serializeJson(jsonBody, result); // Serialize to string.
      attenuatorSerialSendData(A_SAVE_PREFERENCES_PACK); // Tell the pack to save the new settings.
      request->send(200, "application/json", result);
    }
    catch (...) {
      jsonBody.clear();
      jsonBody["status"] = "An error was encountered while saving settings.";
      serializeJson(jsonBody, result); // Serialize to string.
      request->send(200, "application/json", result);
    }
  }
  else {
    // Tell the user why the requested action failed.
    String result;
    jsonBody.clear();
    jsonBody["status"] = "Pack and/or Wand are running, save action cancelled";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
});

// Handles the JSON body for the wand settings save request.
AsyncCallbackJsonWebHandler *handleSaveWandConfig = new AsyncCallbackJsonWebHandler("/config/wand/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  jsonBody.clear();
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    Serial.print("Body was not a JSON object");
  }

  String result;
  if(!b_pack_on && !b_wand_on) {
    try {
      wandConfig.ledWandCount = jsonBody["ledWandCount"].as<uint8_t>();
      wandConfig.ledWandHue = jsonBody["ledWandHue"].as<uint8_t>();
      wandConfig.ledWandSat = jsonBody["ledWandSat"].as<uint8_t>();
      wandConfig.spectralModeEnabled = jsonBody["spectralModeEnabled"].as<uint8_t>();
      wandConfig.spectralHolidayMode = jsonBody["spectralHolidayMode"].as<uint8_t>();
      wandConfig.overheatEnabled = jsonBody["overheatEnabled"].as<uint8_t>();
      wandConfig.defaultFiringMode = jsonBody["defaultFiringMode"].as<uint8_t>();
      wandConfig.wandVibration = jsonBody["wandVibration"].as<uint8_t>();
      wandConfig.wandSoundsToPack = jsonBody["wandSoundsToPack"].as<uint8_t>();
      wandConfig.quickVenting = jsonBody["quickVenting"].as<uint8_t>();
      wandConfig.autoVentLight = jsonBody["autoVentLight"].as<uint8_t>();
      wandConfig.wandBeepLoop = jsonBody["wandBeepLoop"].as<uint8_t>();
      wandConfig.wandBootError = jsonBody["wandBootError"].as<uint8_t>();
      wandConfig.defaultYearModeWand = jsonBody["defaultYearModeWand"].as<uint8_t>();
      wandConfig.defaultYearModeCTS = jsonBody["defaultYearModeCTS"].as<uint8_t>();
      wandConfig.invertWandBargraph = jsonBody["invertWandBargraph"].as<uint8_t>();
      wandConfig.bargraphOverheatBlink = jsonBody["bargraphOverheatBlink"].as<uint8_t>();
      wandConfig.bargraphIdleAnimation = jsonBody["bargraphIdleAnimation"].as<uint8_t>();
      wandConfig.bargraphFireAnimation = jsonBody["bargraphFireAnimation"].as<uint8_t>();

      jsonBody.clear();
      jsonBody["status"] = "Settings updated, please test before saving to EEPROM.";
      serializeJson(jsonBody, result); // Serialize to string.
      attenuatorSerialSendData(A_SAVE_PREFERENCES_WAND); // Tell the wand (via pack) to save the new settings.
      request->send(200, "application/json", result);
    }
    catch (...) {
      jsonBody.clear();
      jsonBody["status"] = "An error was encountered while saving settings.";
      serializeJson(jsonBody, result); // Serialize to string.
      request->send(200, "application/json", result);
    }
  }
  else {
    // Tell the user why the requested action failed.
    String result;
    jsonBody.clear();
    jsonBody["status"] = "Pack and/or Wand are running, save action cancelled";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
});

// Handles the JSON body for the smoke settings save request.
AsyncCallbackJsonWebHandler *handleSaveSmokeConfig = new AsyncCallbackJsonWebHandler("/config/smoke/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  jsonBody.clear();
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    Serial.print("Body was not a JSON object");
  }

  String result;
  if(!b_pack_on && !b_wand_on) {
    try {
      smokeConfig.smokeEnabled = jsonBody["smokeEnabled"].as<uint8_t>();

      smokeConfig.overheatDuration5 = jsonBody["overheatDuration5"].as<uint8_t>();
      smokeConfig.overheatDuration4 = jsonBody["overheatDuration4"].as<uint8_t>();
      smokeConfig.overheatDuration3 = jsonBody["overheatDuration3"].as<uint8_t>();
      smokeConfig.overheatDuration2 = jsonBody["overheatDuration2"].as<uint8_t>();
      smokeConfig.overheatDuration1 = jsonBody["overheatDuration1"].as<uint8_t>();

      smokeConfig.overheatContinuous5 = jsonBody["overheatContinuous5"].as<uint8_t>();
      smokeConfig.overheatContinuous4 = jsonBody["overheatContinuous4"].as<uint8_t>();
      smokeConfig.overheatContinuous3 = jsonBody["overheatContinuous3"].as<uint8_t>();
      smokeConfig.overheatContinuous2 = jsonBody["overheatContinuous2"].as<uint8_t>();
      smokeConfig.overheatContinuous1 = jsonBody["overheatContinuous1"].as<uint8_t>();

      smokeConfig.overheatLevel5 = jsonBody["overheatLevel5"].as<uint8_t>();
      smokeConfig.overheatLevel4 = jsonBody["overheatLevel4"].as<uint8_t>();
      smokeConfig.overheatLevel3 = jsonBody["overheatLevel3"].as<uint8_t>();
      smokeConfig.overheatLevel2 = jsonBody["overheatLevel2"].as<uint8_t>();
      smokeConfig.overheatLevel1 = jsonBody["overheatLevel1"].as<uint8_t>();

      smokeConfig.overheatDelay5 = jsonBody["overheatDelay5"].as<uint8_t>();
      smokeConfig.overheatDelay4 = jsonBody["overheatDelay4"].as<uint8_t>();
      smokeConfig.overheatDelay3 = jsonBody["overheatDelay3"].as<uint8_t>();
      smokeConfig.overheatDelay2 = jsonBody["overheatDelay2"].as<uint8_t>();
      smokeConfig.overheatDelay1 = jsonBody["overheatDelay1"].as<uint8_t>();

      jsonBody.clear();
      jsonBody["status"] = "Settings updated, please test before saving to EEPROM.";
      serializeJson(jsonBody, result); // Serialize to string.
      attenuatorSerialSendData(A_SAVE_PREFERENCES_SMOKE); // Tell the pack and wand to save the new settings.
      request->send(200, "application/json", result);
    }
    catch (...) {
      jsonBody.clear();
      jsonBody["status"] = "An error was encountered while saving settings.";
      serializeJson(jsonBody, result); // Serialize to string.
      request->send(200, "application/json", result);
    }
  }
  else {
    // Tell the user why the requested action failed.
    String result;
    jsonBody.clear();
    jsonBody["status"] = "Pack and/or Wand are running, save action cancelled";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
});

// Handles the JSON body for the password change request.
AsyncCallbackJsonWebHandler *passwordChangeHandler = new AsyncCallbackJsonWebHandler("/password/update", [](AsyncWebServerRequest *request, JsonVariant &json) {
  jsonBody.clear();
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    Serial.print("Body was not a JSON object");
  }

  String result;
  if(jsonBody.containsKey("password")) {
    String newPasswd = jsonBody["password"];

    // Password is used for the built-in Access Point ability, which will be used when a preferred network is not available.
    if(newPasswd.length() >= 8) {
      preferences.begin("credentials", false); // Access namespace in read/write mode.
      preferences.putString("ssid", ap_ssid); // Store SSID in case this was altered.
      preferences.putString("password", newPasswd); // Store user-provided password.
      preferences.end();

      jsonBody.clear();
      jsonBody["status"] = "Password updated, restarting controller. Please enter your new WiFi password when prompted by your device.";
      serializeJson(jsonBody, result); // Serialize to string.
      request->send(200, "application/json", result);

      // Pause to allow response to flow, then restart the device.
      delay(1000);
      ESP.restart();
    }
    else {
      // Password must be at least 8 characters in length.
      jsonBody.clear();
      jsonBody["status"] = "Password must be a minimum of 8 characters to meet WPA2 requirements.";
      serializeJson(jsonBody, result); // Serialize to string.
      request->send(200, "application/json", result);
    }
  }
  else {
    debug("No password in JSON body");
    jsonBody.clear();
    jsonBody["status"] = "Unable to update password.";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
});

// Handles the JSON body for the wifi network info.
AsyncCallbackJsonWebHandler *wifiChangeHandler = new AsyncCallbackJsonWebHandler("/wifi/update", [](AsyncWebServerRequest *request, JsonVariant &json) {
  jsonBody.clear();
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    Serial.print("Body was not a JSON object");
  }

  String result;
  if(jsonBody.containsKey("network") && jsonBody.containsKey("password")) {
    bool b_errors = false; // Assume false until otherwise indicated.
    bool b_enabled = jsonBody["enabled"].as<bool>();
    String wifiNetwork = jsonBody["network"];
    String wifiPasswd = jsonBody["password"];
    String localAddr = jsonBody["address"];
    String subnetMask = jsonBody["subnet"];
    String gatewayIP = jsonBody["gateway"];

    // If no errors encountered, continue with storing a preferred network (with credentials and IP information).
    if(wifiNetwork.length() >= 2 && wifiPasswd.length() >= 8) {
      preferences.begin("network", false); // Access namespace in read/write mode.

      // Clear old network IP info if SSID or password have been changed.
      if(preferences.getString("ssid") != wifiNetwork || preferences.getString("password") != wifiPasswd) {
        preferences.putString("address", "");
        preferences.putString("subnet", "");
        preferences.putString("gateway", "");
      }

      // Store the critical values to enable/disable the external WiFi.
      preferences.putBool("enabled", b_enabled);
      preferences.putString("ssid", wifiNetwork);
      preferences.putString("password", wifiPasswd);

      // Continue saving only if network values are 7 characters or more (eg. N.N.N.N)
      if(localAddr.length() >= 7 && localAddr != wifi_address) {
        preferences.putString("address", localAddr);
      }
      if(subnetMask.length() >= 7 && subnetMask != wifi_subnet) {
        preferences.putString("subnet", subnetMask);
      }
      if(gatewayIP.length() >= 7 && gatewayIP != wifi_gateway) {
        preferences.putString("gateway", gatewayIP);
      }

      preferences.end();
    }

    if(!b_errors) {
      jsonBody.clear();

      // Disconnect from the WiFi network and re-apply any changes.
      WiFi.disconnect();
      delay(100);
      if(b_enabled) {
        if(startExternalWifi()) {
          jsonBody["status"] = "Settings updated, WiFi connection restarted successfully.";
        }
        else {
          jsonBody["status"] = "Settings updated, but WiFi connection was not successful.";
        }
      }
      else {
        jsonBody["status"] = "Settings updated, and external WiFi has been disconnected.";
      }

      serializeJson(jsonBody, result); // Serialize to string.
      request->send(200, "application/json", result);
    }
    else {
      jsonBody.clear();
      jsonBody["status"] = "Errors encountered while processing request data. Please re-check submitted values and try again.";
      serializeJson(jsonBody, result); // Serialize to string.
      request->send(200, "application/json", result);
    }
  }
  else {
    debug("No password in JSON body");
    jsonBody.clear();
    jsonBody["status"] = "Unable to update password.";
    serializeJson(jsonBody, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
});

void handleNotFound(AsyncWebServerRequest *request) {
  // Returned for any invalid URL requested.
  debug("Web page not found");
  request->send(404, "text/plain", "Not Found");
}