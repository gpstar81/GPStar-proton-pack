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

const char SMOKE_SETTINGS_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Smoke Settings</title>
  <link rel="icon" href="data:;base64,iVBORw0KGgo=">
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1>Smoke Settings</h1>
  <div class="block left">
    <p>
      Change system configuration options using the available toggles and selectors.
      Options may only be changed when the pack and wand are not powered and running.
      Use the "Update Settings" button to save values to your equipment.
      Test your changes before using the "Save to EEPROM" to store as permanent defaults.
    </p>
    <br/>
  </div>

  <div class="block">
    <hr/>
    <a href="/">&laquo; Back</a>
    &nbsp;&nbsp;
    <button type="button" class="green" style="width:120px" onclick="saveSettings()">Update&nbsp;Settings</button>
    &nbsp;&nbsp;
    <button type="button" class="orange" style="width:120px" onclick="saveEEPROM()">Save&nbsp;to&nbsp;EPROM</button>
  </div>

  <script type="application/javascript">
    window.addEventListener("load", onLoad);

    function onLoad(event) {
      getSettings();
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
            if (settings.packPowered || settings.wandPowered) {
              alert("Pack and/or Wand are currently powered up. Changes to settings will not be allowed. Turn off system via toggle switches and reload the page to obtain the latest settings.");
            }

          }
        }
      };
      xhttp.open("GET", "/config/smoke", true);
      xhttp.send();
    }

    function saveSettings() {
      // Saves current settings to pack/wand, updating runtime variables and making changes immediately effective.
      // This does NOT save to the EEPROM automatically as the user is encouraged to test prior to that action.
      var settings = {
        overheatDuration5: 0,
        overheatDuration4: 0,
        overheatDuration3: 0,
        overheatDuration2: 0,
        overheatDuration1: 0,
        overheatContinuous5: false,
        overheatContinuous4: false,
        overheatContinuous3: false,
        overheatContinuous2: false,
        overheatContinuous1: false,
        overheatLevel5: false,
        overheatLevel4: false,
        overheatLevel3: false,
        overheatLevel2: false,
        overheatLevel1: false,
        overheatDelay5: 0,
        overheatDelay4: 0,
        overheatDelay3: 0,
        overheatDelay2: 0,
        overheatDelay1: 0
      };
      var body = JSON.stringify(settings);

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          handleStatus(this.responseText);
        }
      };
      xhttp.open("PUT", "/config/smoke/save", true);
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
      xhttp.open("PUT", "/eeprom/smoke", true);
      xhttp.send();
    }
  </script>
</body>
</html>
)=====";