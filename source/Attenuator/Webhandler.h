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

/*
 * Web Handler Functions - Performs actions or returns data for web UI
 */
StaticJsonDocument<1024> jsonDoc; // Used for processing JSON body data.
StaticJsonDocument<16> jsonSuccess; // Used for sending JSON status as success.
String status; // Holder for simple "status: success" response.

void handleRoot(AsyncWebServerRequest *request) {
  // Used for the root page (/) from the web server.
  //debug("Web Root HTML Requested");
  String s = INDEX_page; // Read HTML page into String.
  request->send(200, "text/html", s); // Serve page content.
}

void handlePassword(AsyncWebServerRequest *request) {
  // Used for the password page from the web server.
  //debug("Password HTML Requested");
  String s = PASSWORD_page; // Read HTML page into String.
  request->send(200, "text/html", s); // Serve page content.
}

void handlePackSettings(AsyncWebServerRequest *request) {
  // Used for the settings page from the web server.
  //debug("Pack Settings HTML Requested");
  String s = PACK_SETTINGS_page; // Read HTML page into String.
  request->send(200, "text/html", s); // Serve page content.

  // Tell the pack that we'll need the latest pack EEPROM values.
  attenuatorSerialSend(A_SEND_PREFERENCES_PACK);
}

void handleWandSettings(AsyncWebServerRequest *request) {
  // Used for the settings page from the web server.
  //debug("Wand Settings HTML Requested");
  String s = WAND_SETTINGS_page; // Read HTML page into String.
  request->send(200, "text/html", s); // Serve page content.

  // Tell the pack that we'll need the latest wand EEPROM values.
  attenuatorSerialSend(A_SEND_PREFERENCES_PACK);
}

void handleSmokeSettings(AsyncWebServerRequest *request) {
  // Used for the settings page from the web server.
  //debug("Smoke Settings HTML Requested");
  String s = SMOKE_SETTINGS_page; // Read HTML page into String.
  request->send(200, "text/html", s); // Serve page content.

  // Tell the pack that we'll need the latest smoke EEPROM values.
  attenuatorSerialSend(A_SEND_PREFERENCES_PACK);
}

void handleStyle(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  //debug("Main StyleSheet Requested");
  String s = STYLE_page; // Read CSS page into String.
  request->send(200, "text/css", s); // Serve page content.
}

String getPackConfig() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonDoc.clear();

  if(!b_wait_for_pack) {
    // Return current powered state for pack and wand.
    jsonDoc["packPowered"] = (b_pack_on ? true : false);
    jsonDoc["wandPowered"] = (b_wand_on ? true : false);

    // Proton Pack Runtime Options
    jsonDoc["defaultSystemModePack"] = packConfig.defaultSystemModePack; // [0=SH,1=MO]
    jsonDoc["defaultYearThemePack"] = packConfig.defaultYearThemePack; // [1=TOGGLE,2=1984,3=1989,4=2021,5=2024]
    jsonDoc["defaultSystemVolume"] = packConfig.defaultSystemVolume; // 0-100
    jsonDoc["protonStreamEffects"] = packConfig.protonStreamEffects; // true|false
    jsonDoc["smokeEnabled"] = packConfig.smokeEnabled; // true|false
    jsonDoc["overheatStrobeNF"] = packConfig.overheatStrobeNF; // true|false
    jsonDoc["overheatLightsOff"] = packConfig.overheatLightsOff; // true|false
    jsonDoc["overheatSyncToFan"] = packConfig.overheatSyncToFan;// true|false
    jsonDoc["demoLightMode"] = packConfig.demoLightMode; // true|false

    // Proton Pack LED Options
    jsonDoc["ledCycLidCount"] = packConfig.ledCycLidCount; // [12,20,40]
    jsonDoc["ledCycLidHue"] = packConfig.ledCycLidHue; // Spectral custom color/hue 1-254
    jsonDoc["ledCycLidSat"] = packConfig.ledCycLidSat; // Spectral custom saturation 1-254
    jsonDoc["cyclotronDirection"] = packConfig.cyclotronDirection; // [0=CCW,1=CW]
    jsonDoc["ledCycLidCenter"] = packConfig.ledCycLidCenter; // [0=3,1=1]
    jsonDoc["ledVGCyclotron"] = packConfig.ledVGCyclotron; // true|false
    jsonDoc["ledCycLidSimRing"] = packConfig.ledCycLidSimRing; // true|false
    jsonDoc["ledCycCakeCount"] = packConfig.ledCycCakeCount; // [12,23,24,35]
    jsonDoc["ledCycCakeHue"] = packConfig.ledCycCakeHue; // Spectral custom color/hue 1-254
    jsonDoc["ledCycCakeSat"] = packConfig.ledCycCakeSat; // Spectral custom saturation 1-254
    jsonDoc["ledCycCakeGRB"] = packConfig.ledCycCakeGRB; // Use GRB for cake LEDs true|false
    jsonDoc["ledPowercellCount"] = packConfig.ledPowercellCount; //[13,15]
    jsonDoc["ledPowercellHue"] = packConfig.ledPowercellHue; // Spectral custom color/hue 1-254
    jsonDoc["ledPowercellSat"] = packConfig.ledPowercellSat; // Spectral custom saturation 1-254
    jsonDoc["ledVGPowercell"] = packConfig.ledVGPowercell; // true|false
  }

  // Serialize JSON object to string.
  serializeJson(jsonDoc, equipSettings);
  return equipSettings;
}

String getWandConfig() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonDoc.clear();

  if(!b_wait_for_pack) {
    // Return current powered state for pack and wand.
    jsonDoc["packPowered"] = (b_pack_on ? true : false);
    jsonDoc["wandPowered"] = (b_wand_on ? true : false);

    // Neutrona Wand LED Options
    jsonDoc["ledWandCount"] = wandConfig.ledWandCount; // [5,48,60]
    jsonDoc["ledWandHue"] = wandConfig.ledWandHue; // Spectral custom color/hue 1-254
    jsonDoc["ledWandSat"] = wandConfig.ledWandSat; // Spectral custom saturation 1-254
    jsonDoc["spectralModeEnabled"] = wandConfig.spectralModeEnabled; // true|false
    jsonDoc["spectralHolidayMode"] = wandConfig.spectralHolidayMode; // true|false

    // Neutrona Wand Runtime Options
    jsonDoc["overheatEnabled"] = wandConfig.overheatEnabled; // true|false
    jsonDoc["defaultFiringMode"] = wandConfig.defaultFiringMode; // [0=VG,1=CTS,2=SYSTEM]
    jsonDoc["wandSoundsToPack"] = wandConfig.wandSoundsToPack; // true|false
    jsonDoc["quickVenting"] = wandConfig.quickVenting; // true|false (Super-Hero Mode Only)
    jsonDoc["autoVentLight"] = wandConfig.autoVentLight; // true|false
    jsonDoc["wandBeepLoop"] = wandConfig.wandBeepLoop; // true|false (Afterlife/Frozen Empire Only)
    jsonDoc["wandBootError"] = wandConfig.wandBootError; // true|false (Super-Hero Mode Only)
    jsonDoc["defaultYearModeWand"] = wandConfig.defaultYearModeWand; // [1=TOGGLE,2=1984,3=1989,4=2021,5=2024]
    jsonDoc["defaultYearModeCTS"] = wandConfig.defaultYearModeCTS; // [1=TOGGLE,2=1984,3=1989,4=2021,5=2024]
    jsonDoc["invertWandBargraph"] = wandConfig.invertWandBargraph; // true|false
    jsonDoc["bargraphOverheatBlink"] = wandConfig.bargraphOverheatBlink; // true|false
    jsonDoc["bargraphIdleAnimation"] = wandConfig.bargraphIdleAnimation; // [1=SYSTEM,2=SH,3=MO]
    jsonDoc["bargraphFireAnimation"] = wandConfig.bargraphFireAnimation; // [1=SYSTEM,2=SH,3=MO]
  }

  // Serialize JSON object to string.
  serializeJson(jsonDoc, equipSettings);
  return equipSettings;
}

String getSmokeConfig() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonDoc.clear();

  if(!b_wait_for_pack) {
    // Return current powered state for pack and wand.
    jsonDoc["packPowered"] = (b_pack_on ? true : false);
    jsonDoc["wandPowered"] = (b_wand_on ? true : false);

    // Proton Pack

    // Power Level 5
    jsonDoc["overheatDuration5"] = smokeConfig.overheatDuration5; // 2-60
    jsonDoc["overheatContinuous5"] = smokeConfig.overheatContinuous5; // true|false
    // Power Level 4
    jsonDoc["overheatDuration4"] = smokeConfig.overheatDuration4; // 2-60
    jsonDoc["overheatContinuous4"] = smokeConfig.overheatContinuous4; // true|false
    // Power Level 3
    jsonDoc["overheatDuration3"] = smokeConfig.overheatDuration3; // 2-60
    jsonDoc["overheatContinuous3"] = smokeConfig.overheatContinuous3; // true|false
    // Power Level 2
    jsonDoc["overheatDuration2"] = smokeConfig.overheatDuration2; // 2-60
    jsonDoc["overheatContinuous2"] = smokeConfig.overheatContinuous2; // true|false
    // Power Level 1
    jsonDoc["overheatDuration1"] = smokeConfig.overheatDuration1; // 2-60
    jsonDoc["overheatContinuous1"] = smokeConfig.overheatContinuous1; // true|false

    // Neutrona Wand

    // Power Level 5
    jsonDoc["overheatLevel5"] = smokeConfig.overheatLevel5; // true|false
    jsonDoc["overheatDelay5"] = smokeConfig.overheatDelay5; // 2-60
    // Power Level 4
    jsonDoc["overheatLevel4"] = smokeConfig.overheatLevel4; // true|false
    jsonDoc["overheatDelay4"] = smokeConfig.overheatDelay4; // 2-60
    // Power Level 3
    jsonDoc["overheatLevel3"] = smokeConfig.overheatLevel3; // true|false
    jsonDoc["overheatDelay3"] = smokeConfig.overheatDelay3; // 2-60
    // Power Level 2
    jsonDoc["overheatLevel2"] = smokeConfig.overheatLevel2; // true|false
    jsonDoc["overheatDelay2"] = smokeConfig.overheatDelay2; // 2-60
    // Power Level 1
    jsonDoc["overheatLevel1"] = smokeConfig.overheatLevel1; // true|false
    jsonDoc["overheatDelay1"] = smokeConfig.overheatDelay1; // 2-60
  }

  // Serialize JSON object to string.
  serializeJson(jsonDoc, equipSettings);
  return equipSettings;
}

String getEquipmentStatus() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipStatus;
  jsonDoc.clear();

  if(!b_wait_for_pack) {
    // Only prepare status when not waiting on the pack
    jsonDoc["mode"] = getMode();
    jsonDoc["theme"] = getTheme();
    jsonDoc["switch"] = getRedSwitch();
    jsonDoc["pack"] = (b_pack_on ? "Powered" : "Idle");
    jsonDoc["power"] = getPower();
    jsonDoc["safety"] = getSafety();
    jsonDoc["wand"] = (b_wand_on ? "Powered" : "Idle");
    jsonDoc["wandMode"] = getWandMode();
    jsonDoc["firing"] = (b_firing ? "Firing" : "Idle");
    jsonDoc["cable"] = (b_pack_alarm ? "Disconnected" : "Connected");
    jsonDoc["cyclotron"] = getCyclotronState();
    jsonDoc["temperature"] = (b_overheating ? "Venting" : "Normal");
    jsonDoc["musicPlaying"] = b_playing_music;
    jsonDoc["musicPaused"] = b_music_paused;
    jsonDoc["musicCurrent"] = i_music_track_current;
    jsonDoc["musicStart"] = i_music_track_min;
    jsonDoc["musicEnd"] = i_music_track_max;
  }

  // Serialize JSON object to string.
  serializeJson(jsonDoc, equipStatus);
  return equipStatus;
}

void handleGetPackConfig(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getPackConfig());
}

void handleGetWandConfig(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getWandConfig());
}

void handleGetSmokeConfig(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getSmokeConfig());
}

void handleGetStatus(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getEquipmentStatus());
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
  if(i_speed_multiplier > 1) {
    // Only send command to pack if cyclotron is not "normal".
    debug("Cancel Overheat Warning");
    attenuatorSerialSend(A_WARNING_CANCELLED);
    request->send(200, "application/json", status);
  } else {
    // Tell the user why the requested action failed.
    String result;
    jsonDoc.clear();
    jsonDoc["status"] = "System not in overheat warning";
    serializeJson(jsonDoc, result); // Serialize to string.
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
    jsonDoc.clear();
    jsonDoc["status"] = "Invalid track number requested";
    serializeJson(jsonDoc, result); // Serialize to string.
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
AsyncCallbackJsonWebHandler *handleSavePackConfig = new AsyncCallbackJsonWebHandler("/config/pack/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  StaticJsonDocument<512> jsonBody;
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    Serial.print("Body was not a JSON object");
  }

  String result;
  if(!b_pack_on && !b_wand_on) {
    // General Options
    packConfig.defaultSystemModePack = jsonBody["defaultSystemModePack"];
    packConfig.defaultYearThemePack = jsonBody["defaultYearThemePack"];
    packConfig.defaultSystemVolume = jsonBody["defaultSystemVolume"];
    packConfig.protonStreamEffects = jsonBody["protonStreamEffects"];
    packConfig.smokeEnabled = jsonBody["smokeEnabled"];
    packConfig.overheatStrobeNF = jsonBody["overheatStrobeNF"];
    packConfig.overheatLightsOff = jsonBody["overheatLightsOff"];
    packConfig.overheatSyncToFan = jsonBody["overheatSyncToFan"];
    packConfig.demoLightMode = jsonBody["demoLightMode"];

    // Cyclotron Lid
    packConfig.ledCycLidCount = jsonBody["ledCycLidCount"];
    packConfig.ledCycLidHue = jsonBody["ledCycLidHue"];
    packConfig.ledCycLidSat = jsonBody["ledCycLidSat"];
    packConfig.cyclotronDirection = jsonBody["cyclotronDirection"];
    packConfig.ledCycLidCenter = jsonBody["ledCycLidCenter"];
    packConfig.ledVGCyclotron = jsonBody["ledVGCyclotron"];
    packConfig.ledCycLidSimRing = jsonBody["ledCycLidSimRing"];

    // Inner Cyclotron
    packConfig.ledCycCakeCount = jsonBody["ledCycCakeCount"];
    packConfig.ledCycCakeHue = jsonBody["ledCycCakeHue"];
    packConfig.ledCycCakeSat = jsonBody["ledCycCakeSat"];
    packConfig.ledCycCakeGRB = jsonBody["ledCycCakeGRB"];

    // Power Cell
    packConfig.ledPowercellCount = jsonBody["ledPowercellCount"];
    packConfig.ledPowercellHue = jsonBody["ledPowercellHue"];
    packConfig.ledPowercellSat = jsonBody["ledPowercellSat"];
    packConfig.ledVGPowercell = jsonBody["ledVGPowercell"];

    jsonBody.clear();
    jsonBody["status"] = "Settings updated, please test before saving to EEPROM.";
    serializeJson(jsonBody, result); // Serialize to string.
    attenuatorSerialSend(A_SAVE_PREFERENCES_PACK); // Tell the pack to save the new settings.
    request->send(200, "application/json", result);
  }
  else {
    // Tell the user why the requested action failed.
    String result;
    jsonDoc.clear();
    jsonDoc["status"] = "Pack and/or Wand are running, save action cancelled";
    serializeJson(jsonDoc, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
});

// Handles the JSON body for the wand settings save request.
AsyncCallbackJsonWebHandler *handleSaveWandConfig = new AsyncCallbackJsonWebHandler("/config/wand/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  StaticJsonDocument<512> jsonBody;
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    Serial.print("Body was not a JSON object");
  }

  String result;
  if(!b_pack_on && !b_wand_on) {
    wandConfig.ledWandCount = jsonBody["ledWandCount"];
    wandConfig.ledWandHue = jsonBody["ledWandHue"];
    wandConfig.ledWandSat = jsonBody["ledWandSat"];
    wandConfig.spectralModeEnabled = jsonBody["spectralModeEnabled"];
    wandConfig.spectralHolidayMode = jsonBody["spectralHolidayMode"];
    wandConfig.overheatEnabled = jsonBody["overheatEnabled"];
    wandConfig.defaultFiringMode = jsonBody["defaultFiringMode"];
    wandConfig.wandSoundsToPack = jsonBody["wandSoundsToPack"];
    wandConfig.quickVenting = jsonBody["quickVenting"];
    wandConfig.autoVentLight = jsonBody["autoVentLight"];
    wandConfig.wandBeepLoop = jsonBody["wandBeepLoop"];
    wandConfig.wandBootError = jsonBody["wandBootError"];
    wandConfig.defaultYearModeWand = jsonBody["defaultYearModeWand"];
    wandConfig.defaultYearModeCTS = jsonBody["defaultYearModeCTS"];
    wandConfig.invertWandBargraph = jsonBody["invertWandBargraph"];
    wandConfig.bargraphOverheatBlink = jsonBody["bargraphOverheatBlink"];
    wandConfig.bargraphIdleAnimation = jsonBody["bargraphIdleAnimation"];
    wandConfig.bargraphFireAnimation = jsonBody["bargraphFireAnimation"];

    jsonBody.clear();
    jsonBody["status"] = "Settings updated, please test before saving to EEPROM.";
    serializeJson(jsonBody, result); // Serialize to string.
    attenuatorSerialSend(A_SAVE_PREFERENCES_WAND); // Tell the wand (via pack) to save the new settings.
    request->send(200, "application/json", result);
  }
  else {
    // Tell the user why the requested action failed.
    String result;
    jsonDoc.clear();
    jsonDoc["status"] = "Pack and/or Wand are running, save action cancelled";
    serializeJson(jsonDoc, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
});

// Handles the JSON body for the smoke settings save request.
AsyncCallbackJsonWebHandler *handleSaveSmokeConfig = new AsyncCallbackJsonWebHandler("/config/wand/save", [](AsyncWebServerRequest *request, JsonVariant &json) {
  StaticJsonDocument<512> jsonBody;
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    Serial.print("Body was not a JSON object");
  }

  String result;
  if(!b_pack_on && !b_wand_on) {
    smokeConfig.overheatDuration5 = jsonBody["overheatDuration5"];
    smokeConfig.overheatDuration4 = jsonBody["overheatDuration4"];
    smokeConfig.overheatDuration3 = jsonBody["overheatDuration3"];
    smokeConfig.overheatDuration2 = jsonBody["overheatDuration2"];
    smokeConfig.overheatDuration1 = jsonBody["overheatDuration1"];

    smokeConfig.overheatContinuous5 = jsonBody["overheatContinuous5"];
    smokeConfig.overheatContinuous4 = jsonBody["overheatContinuous4"];
    smokeConfig.overheatContinuous3 = jsonBody["overheatContinuous3"];
    smokeConfig.overheatContinuous2 = jsonBody["overheatContinuous2"];
    smokeConfig.overheatContinuous1 = jsonBody["overheatContinuous1"];

    smokeConfig.overheatLevel5 = jsonBody["overheatLevel5"];
    smokeConfig.overheatLevel4 = jsonBody["overheatLevel4"];
    smokeConfig.overheatLevel3 = jsonBody["overheatLevel3"];
    smokeConfig.overheatLevel2 = jsonBody["overheatLevel2"];
    smokeConfig.overheatLevel1 = jsonBody["overheatLevel1"];

    smokeConfig.overheatDelay5 = jsonBody["overheatDelay5"];
    smokeConfig.overheatDelay4 = jsonBody["overheatDelay4"];
    smokeConfig.overheatDelay3 = jsonBody["overheatDelay3"];
    smokeConfig.overheatDelay2 = jsonBody["overheatDelay2"];
    smokeConfig.overheatDelay1 = jsonBody["overheatDelay1"];

    jsonBody.clear();
    jsonBody["status"] = "Settings updated, please test before saving to EEPROM.";
    serializeJson(jsonBody, result); // Serialize to string.
    attenuatorSerialSend(A_SAVE_PREFERENCES_SMOKE); // Tell the pack and wand to save the new settings.
    request->send(200, "application/json", result);
  }
  else {
    // Tell the user why the requested action failed.
    String result;
    jsonDoc.clear();
    jsonDoc["status"] = "Pack and/or Wand are running, save action cancelled";
    serializeJson(jsonDoc, result); // Serialize to string.
    request->send(200, "application/json", result);
  }
});

// Handles the JSON body for the password change request.
AsyncCallbackJsonWebHandler *passwordChangeHandler = new AsyncCallbackJsonWebHandler("/password/update", [](AsyncWebServerRequest *request, JsonVariant &json) {
  StaticJsonDocument<256> jsonBody;
  if(json.is<JsonObject>()) {
    jsonBody = json.as<JsonObject>();
  }
  else {
    Serial.print("Body was not a JSON object");
  }

  String result;
  if(jsonBody.containsKey("password")) {
    String newPasswd = jsonBody["password"];
    //Serial.println("New AP Password: " + newPasswd);

    if(newPasswd != "") {
      preferences.begin("credentials", false); // Access namespace in read/write mode.
      preferences.putString("ssid", ap_ssid); // Store SSID in case this was changed.
      preferences.putString("password", newPasswd); // Store user-provided password.
      preferences.end();

      jsonBody.clear();
      jsonBody["status"] = "Password updated, rebooting controller. Please enter your new WiFi password when prompted by your device.";
      serializeJson(jsonBody, result); // Serialize to string.
      request->send(200, "application/json", result);
      delay(1000); // Pause to allow response to flow.
      ESP.restart(); // Reboot device
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