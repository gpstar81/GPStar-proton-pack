/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

const char DEVICE_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta http-equiv="Cache-control" content="public">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Special Device Settings</title>
  <link rel="icon" type="image/svg+xml" href="/favicon.svg"/>
  <link rel="shortcut icon" href="/favicon.ico"/>
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1 id="top">Special Device Settings</h1>
  <div class="block left">
    <p>
      Adjust settings unique to this device's web interface.
      Use this screen to customize the private network used by the built-in WiFi network.
      <br/>
      You may also use <b>http://[PRIVATE_NETWORK].local</b> to access this device via your browser.
      Use the "Update Settings" button to save values to the controller.
    </p>
    <br/>
  </div>
  <h1>General Options</h1>
  <div class="block left">
    <div class="setting">
      <b>Private Network:</b>
      <input type="text" id="wifiName" width="42" maxlength="32" placeholder="Custom SSID"
       title="Only letters, numbers, hyphens, and underscores are allowed, up to 32 characters."/>
    </div>
    <div class="setting">
      <b>MotionCal - Magnetic Offsets:</b><br/>
      <input type="text" id="hardIron1" width="8" maxlength="8" placeholder="X Axis"/><br/>
      <input type="text" id="hardIron2" width="8" maxlength="8" placeholder="Y Axis"/><br/>
      <input type="text" id="hardIron3" width="8" maxlength="8" placeholder="Z Axis"/><br/>
    </div>
    <div class="setting">
      <b>MotionCal - Magnetic Mapping:</b><br/>
      <input type="text" id="softIron1" width="8" maxlength="8" placeholder="Value 1"/>
      <input type="text" id="softIron2" width="8" maxlength="8" placeholder="Value 2"/>
      <input type="text" id="softIron3" width="8" maxlength="8" placeholder="Value 3"/>
      <br/>
      <input type="text" id="softIron4" width="8" maxlength="8" placeholder="Value 4"/>
      <input type="text" id="softIron5" width="8" maxlength="8" placeholder="Value 5"/>
      <input type="text" id="softIron6" width="8" maxlength="8" placeholder="Value 6"/>
      <br/>
      <input type="text" id="softIron7" width="8" maxlength="8" placeholder="Value 7"/>
      <input type="text" id="softIron8" width="8" maxlength="8" placeholder="Value 8"/>
      <input type="text" id="softIron9" width="8" maxlength="8" placeholder="Value 9"/>
    </div>
    <div class="setting">
      <b>MotionCal - Magnetic Field:</b><br/>
      <input type="text" id="magField" width="8" maxlength="8" placeholder="0"/>
    </div>
    <div class="setting">
      <b>Song List:</b> <span id="byteCount"></span><br/>
      <textarea id="songList" name="songList" rows="40" cols="38"
       style="text-align:left;" oninput="updateByteCount()"
       placeholder="Add a list of track names, 1 per line, up to 2000 Bytes in total"></textarea>
    </div>
  </div>

  <div class="block">
    <a href="#top">Top</a>
    <hr/>
    <a href="/">&laquo; Back</a>
    &nbsp;&nbsp;
    <button type="button" class="green" style="width:120px" onclick="saveSettings()">Update&nbsp;Settings</button>
    <br/>
    <br/>
  </div>

  <script type="application/javascript" src="/common.js"></script>
  <script type="application/javascript">
    window.addEventListener("load", onLoad);

    function onLoad(event) {
      // Wait 0.1s for page to fully load.
      setTimeout(getSettings, 100);
    }

    function updateByteCount() {
        var songList = getEl("songList");
        var byteCount = getEl("byteCount");
        var byteLength = new TextEncoder().encode(songList.value).length;
        byteCount.innerHTML = byteLength + "/2000 Bytes";
    }

    function getSettings() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var settings = JSON.parse(this.responseText);
          if (settings) {
            // Update fields with the current values, or supply an expected default as necessary.
            setValue("wifiName", settings.wifiName || "");
            setValue("hardIron1", settings.hardIron1 ?? 0);
            setValue("hardIron2", settings.hardIron2 ?? 0);
            setValue("hardIron3", settings.hardIron3 ?? 0);
            setValue("softIron1", settings.softIron1 ?? 0);
            setValue("softIron2", settings.softIron2 ?? 0);
            setValue("softIron3", settings.softIron3 ?? 0);
            setValue("softIron4", settings.softIron4 ?? 0);
            setValue("softIron5", settings.softIron5 ?? 0);
            setValue("softIron6", settings.softIron6 ?? 0);
            setValue("softIron7", settings.softIron7 ?? 0);
            setValue("softIron8", settings.softIron8 ?? 0);
            setValue("softIron9", settings.softIron9 ?? 0);
            setValue("magField", settings.magField ?? 0);
            setValue("songList", settings.songList || "");
            updateByteCount();
          }
        }
      };
      xhttp.open("GET", "/config/device", true);
      xhttp.send();
    }

    function saveSettings() {
      // Do not allow saving if track list is too large for allowed storage space.
      if (getValue("songList").length > 2000) {
        alert("Error: Unable to save track listing (exceeds allowed bytes).");
        return;
      }

      // Do not allow saving if the new SSID is too short/long, or illegal.
      var wifiName = getText("wifiName");
      if (wifiName.length < 8) {
        alert("Error: Network name must be more than 8 characters.");
        return;
      }
      if (wifiName.length > 32) {
        // Field input size should disallow this, but check just in case.
        alert("Error: Network name cannot exceed 32 characters.");
        return;
      }
      var ssidRegex = /^[a-zA-Z0-9-_]*$/;
      if (!ssidRegex.test(wifiName)) {
        // The name for the SSID must conform to RFC standards which limits the allowed characters.
        alert("Error: Network name may only contain letters, numbers, hyphens, and underscores.");
        return;
      }

      // Saves current settings to attenuator, updating runtime variables and making changes immediately effective.
      var settings = {
        wifiName: wifiName,
        hardIron1: getFloat("hardIron1"),
        hardIron2: getFloat("hardIron2"),
        hardIron3: getFloat("hardIron3"),
        softIron1: getFloat("softIron1"),
        softIron2: getFloat("softIron2"),
        softIron3: getFloat("softIron3"),
        softIron4: getFloat("softIron4"),
        softIron5: getFloat("softIron5"),
        softIron6: getFloat("softIron6"),
        softIron7: getFloat("softIron7"),
        softIron8: getFloat("softIron8"),
        softIron9: getFloat("softIron9"),
        magField: getFloat("magField"),
        songList: getText("songList")
      };
      var body = JSON.stringify(settings);

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4) {
          if (this.status == 200) {
            handleStatus(this.responseText);
          }

          if (this.status == 201) {
            handleStatus(this.responseText);

            if (confirm("Restart device now?")) {
              doRestart();
            }
          }
        }
      };
      xhttp.open("PUT", "/config/device/save", true);
      xhttp.setRequestHeader("Content-Type", "application/json");
      xhttp.send(body);
    }

    function doRestart() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 204) {
          // Reload the page after 2 seconds.
          setTimeout(function() {
            window.location.reload();
          }, 2000);
        }
      };
      xhttp.open("DELETE", "/restart", true);
      xhttp.send();
    }
  </script>
</body>
</html>
)=====";
