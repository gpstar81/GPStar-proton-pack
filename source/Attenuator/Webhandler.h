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

  // Tell the pack that we'll need the latest EEPROM values.
  Serial.println("Sending request for preferences");
  attenuatorSerialSend(A_SEND_PREFERENCES_PACK);
  Serial.println("Request sent");
}

void handleStyle(AsyncWebServerRequest *request) {
  // Used for the root page (/) of the web server.
  //debug("Main StyleSheet Requested");
  String s = STYLE_page; // Read CSS page into String.
  request->send(200, "text/css", s); // Serve page content.
}

String getPackPreferences() {
  // Prepare a JSON object with information we have gleamed from the system.
  String equipSettings;
  jsonDoc.clear();

  if(!b_wait_for_pack) {
    Serial.println("Getting Preferences");

    // Return current powered state for pack and wand.
    jsonDoc["packPowered"] = (b_pack_on ? true : false);
    jsonDoc["wandPowered"] = (b_wand_on ? true : false);

    // Proton Pack LED Options
    jsonDoc["ledCycLidCount"] = 12; // [12,20,40]
    jsonDoc["ledCycLidHue"] = 0; // Spectral custom color/hue 1-254
    jsonDoc["ledCycLidSat"] = 0; // Spectral custom saturation 1-254
    jsonDoc["ledCycLidCenter"] = 1; // [0=3,1=1]
    jsonDoc["ledCycLidSimRing"] = true; // true|false
    jsonDoc["ledCycCakeCount"] = 35; // [12,23,24,35]
    jsonDoc["ledCycCakeHue"] = 0; // Spectral custom color/hue 1-254
    jsonDoc["ledCycCakeSat"] = 0; // Spectral custom saturation 1-254
    jsonDoc["ledCycCakeGRB"] = true; // Use GRB for cake LEDs true|false
    jsonDoc["ledPowercellCount"] = 13; //[13,15]
    jsonDoc["ledPowercellHue"] = 0; // Spectral custom color/hue 1-254
    jsonDoc["ledPowercellSat"] = 0; // Spectral custom saturation 1-254

    // Proton Pack Runtime Options
    jsonDoc["defaultSystemModePack"] = 0; // [0=SH,1=MO]
    jsonDoc["defaultYearModePack"] = 4; // [1=TOGGLE,2=1984,3=1989,4=2021,5=2024]
    jsonDoc["defaultSystemVolume"] = 100; // 0-100
    jsonDoc["cyclotronDirection"] = 1; // [0=CCW,1=CW]
    jsonDoc["vgCyclotron"] = true; // true|false
    jsonDoc["vgPowercell"] = true; // true|false
    jsonDoc["demoLightMode"] = false; // true|false
    jsonDoc["protonStreamEffects"] = true; // true|false
    jsonDoc["smokeEnabled"] = true; // true|false
    jsonDoc["overheatStrobeNF"] = true; // true|false
    jsonDoc["overheatSyncToFan"] = false;// true|false
    jsonDoc["overheatLightsOff"] = true; // true|false

    // Power Level 5
    jsonDoc["overheatDuration5"] = 6; // 2-60
    jsonDoc["overheatContinuous5"] = true; // true|false
    // Power Level 4
    jsonDoc["overheatDuration4"] = 5; // 2-60
    jsonDoc["overheatContinuous4"] = false; // true|false
    // Power Level 3
    jsonDoc["overheatDuration3"] = 4; // 2-60
    jsonDoc["overheatContinuous3"] = false; // true|false
    // Power Level 2
    jsonDoc["overheatDuration2"] = 3; // 2-60
    jsonDoc["overheatContinuous2"] = false; // true|false
    // Power Level 1
    jsonDoc["overheatDuration1"] = 2; // 2-60
    jsonDoc["overheatContinuous1"] = false; // true|false

    // Neutrona Wand LED Options
    jsonDoc["ledWandCount"] = 5; // [5,48,60]
    jsonDoc["ledWandHue"] = 0; // Spectral custom color/hue 0-255
    jsonDoc["ledWandSat"] = 0; // Spectral custom saturation 0-255
    jsonDoc["spectralModeEnabled"] = false; // true|false
    jsonDoc["spectralHolidayMode"] = false; // true|false

    // Neutrona Wand Runtime Options
    jsonDoc["overheatEnabled"] = true; // true|false
    jsonDoc["defaultFiringMode"] = "SYSTEM"; // [VG,CTS,SYSTEM]
    jsonDoc["wandSoundsToPack"] = false; // true|false
    jsonDoc["quickVenting"] = false; // true|false (Super-Hero Mode Only)
    jsonDoc["autoVentLight"] = false; // true|false
    jsonDoc["wandBeepLoop"] = true; // true|false (Afterlife/Frozen Empire Only)
    jsonDoc["wandBootError"] = true; // true|false (Super-Hero Mode Only)
    jsonDoc["defaultYearModeWand"] = 2021; // [1984,1989,2021,2024,DEFAULT]
    jsonDoc["defaultYearModeCTS"] = 2021; // [1984,1989,2021,2024,DEFAULT]
    jsonDoc["invertWandBargraph"] = false; // true|false
    jsonDoc["bargraphOverheatBlink"] = true; // true|false
    jsonDoc["bargraphIdleAnimation"] = "SYSTEM"; // [SH,MO,SYSTEM]
    jsonDoc["bargraphFireAnimation"] = "SYSTEM"; // [SH,MO,SYSTEM]

    // Power Level 5
    jsonDoc["overheatEnabled5"] = true; // true|false
    jsonDoc["overheatStartDelay5"] = 12; // 2-60
    // Power Level 4
    jsonDoc["overheatEnabled4"] = false; // true|false
    jsonDoc["overheatStartDelay4"] = 15; // 2-60
    // Power Level 3
    jsonDoc["overheatEnabled3"] = false; // true|false
    jsonDoc["overheatStartDelay3"] = 20; // 2-60
    // Power Level 2
    jsonDoc["overheatEnabled2"] = false; // true|false
    jsonDoc["overheatStartDelay2"] = 30; // 2-60
    // Power Level 1
    jsonDoc["overheatEnabled1"] = false; // true|false
    jsonDoc["overheatStartDelay1"] = 60; // 2-60
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

void handleGetPackPrefs(AsyncWebServerRequest *request) {
  // Return current system status as a stringified JSON object.
  request->send(200, "application/json", getPackPreferences());
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
    attenuatorSerialSend(i_music_track); // Inform the pack of the new track.
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