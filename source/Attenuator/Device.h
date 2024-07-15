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

const char DEVICE_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta http-equiv="Cache-control" content="public">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Device Preferences</title>
  <link rel="icon" href="data:;base64,iVBORw0KGgo=">
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1 id="top">Attenuator Settings</h1>
  <div class="block left">
    <p>
      Change system configuration options using the available toggles and selectors.
      Use the "Update Settings" button to save values to the Attenuator controller.
    </p>
    <br/>
  </div>
  <h1>General Options</h1>
  <div class="block left">
    <div class="setting">
      <b class="labelSwitch">Invert Device LED Order:</b>
      <label class="switch">
        <input id="invertLEDs" name="invertLEDs" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Enable Piezo Buzzer:</b>
      <label class="switch">
        <input id="buzzer" name="buzzer" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Enable Vibration:</b>
      <label class="switch">
        <input id="vibration" name="vibration" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Feedback on Overheat:</b>
      <label class="switch">
        <input id="overheat" name="overheat" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Feedback when Firing:</b>
      <label class="switch">
        <input id="firing" name="firing" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b>Rad Lens Idle:</b>
      <select id="radLensIdle" name="radLensIdle">
        <option value="0">Amber Pulse</option>
        <option value="1">Orange Fade</option>
      </select>
    </div>
    <div class="setting">
      <b>Status Display:</b>
      <select id="displayType" name="displayType">
        <option value="0">Text</option>
        <option value="1">Graphical</option>
        <option value="2">Both</option>
      </select>
    </div>
    <div class="setting">
      <b>Song List:</b> <span id="byteCount"></span><br/>
      <textarea id="songList" name="songList" rows="40" cols="38"
       style="text-align:left;" oninput="updateByteCount()"
       placeholder="Add a list of track names, 1 per line, up to 2000 Bytes in total"></textarea>
    </div>
  </div>

  <div class="block">
    <hr/>
    <a href="/">&laquo; Back</a>
    &nbsp;&nbsp;
    <button type="button" class="green" style="width:120px" onclick="saveSettings()">Update&nbsp;Settings</button>
    &nbsp;&nbsp;
    <a href="#top">Top</a>
    <br/>
    <br/>
  </div>

  <script type="application/javascript">
    window.addEventListener("load", onLoad);

    function onLoad(event) {
      // Wait 0.1s for page to fully load.
      setTimeout(getSettings, 100);
    }

    function getEl(id){
      return document.getElementById(id);
    }

    function updateByteCount() {
        var songList = getEl("songList");
        var byteCount = getEl("byteCount");
        var byteLength = new TextEncoder().encode(songList.value).length;
        byteCount.innerHTML = byteLength + "/2000 Bytes";
    }

    function isJsonString(str) {
      try {
        JSON.parse(str);
      } catch (e) {
        return false;
      }
      return true;
    }

    function handleStatus(response) {
      if (isJsonString(response || "")) {
        var jObj = JSON.parse(response || "");
        if (jObj.status && jObj.status != "success") {
          alert(jObj.status); // Report non-success status.
        }
      } else {
        alert(response); // Display plain text message.
      }
    }

    function getSettings() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var settings = JSON.parse(this.responseText);
          if (settings) {
            // Update fields with the current values, or supply an expected default as necessary.
            getEl("invertLEDs").checked = settings.invertLEDs ? true : false;
            getEl("buzzer").checked = settings.buzzer ? true : false;
            getEl("vibration").checked = settings.vibration ? true : false;
            getEl("overheat").checked = settings.overheat ? true : false;
            getEl("firing").checked = settings.firing ? true : false;
            getEl("radLensIdle").value = settings.radLensIdle || 0; // Default: 0 [Amber Pulse]
            getEl("displayType").value = settings.displayType || 0; // Default: 0 [Text]
            getEl("songList").value = settings.songList || "";
            updateByteCount();
          }
        }
      };
      xhttp.open("GET", "/config/attenuator", true);
      xhttp.send();
    }

    function saveSettings() {
      // Do not allow saving if track list is too large for allowed storage space.
      if (getEl("songList").value.length > 2000) {
        alert("Unable to save track listing; exceeds allowed bytes.");
        return;
      }

      // Saves current settings to attenuator, updating runtime variables and making changes immediately effective.
      var settings = {
        invertLEDs: getEl("invertLEDs").checked ? 1 : 0,
        buzzer: getEl("buzzer").checked ? 1 : 0,
        vibration: getEl("vibration").checked ? 1 : 0,
        overheat: getEl("overheat").checked ? 1 : 0,
        firing: getEl("firing").checked ? 1 : 0,
        radLensIdle: parseInt(getEl("radLensIdle").value || 0, 10),
        displayType: parseInt(getEl("displayType").value || 0, 10),
        songList: getEl("songList").value || ""
      };
      var body = JSON.stringify(settings);

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          handleStatus(this.responseText);
        }
      };
      xhttp.open("PUT", "/config/attenuator/save", true);
      xhttp.setRequestHeader("Content-Type", "application/json");
      xhttp.send(body);
    }

    function saveEEPROM() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          handleStatus(this.responseText);
        }
      };
      xhttp.open("PUT", "/eeprom/pack", true);
      xhttp.send();
    }
  </script>
</body>
</html>
)=====";