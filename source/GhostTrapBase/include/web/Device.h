/**
 *   GPStar Ghost Trap - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Nomake Wan <-redacted->
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
  <link rel="icon" type="image/svg+xml" href="/favicon.svg"/>
  <link rel="shortcut icon" href="/favicon.ico"/>
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1 id="top">Ghost Trap Settings</h1>
  <div class="block left">
    <p>
      Change system configuration options using the available toggles and selectors.
      Use this screen to customize the private network used by the built-in WiFi network.
      <br/>
      You may also use <b>http://[PRIVATE_NETWORK].local</b> to access this device via your browser.
      Use the "Update Settings" button to save values to the device controller.
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
      <b>Status Display:</b>
      <select id="displayType" name="displayType">
        <option value="0">Text</option>
        <option value="1">Graphical</option>
        <option value="2">Both</option>
      </select>
    </div>
  </div>

  <h1>Door Triggers</h1>
  <div class="block left">
    <div class="setting">
      <label class="toggle-switchy" data-text="yesno" data-label="left">
        <input id="openedSmokeEnabled" name="openedSmokeEnabled" type="checkbox">
        <span class="toggle">
          <span class="switch"></span>
        </span>
        <span class="label">Smoke on Trap Open:</span>
      </label>
    </div>
    <div class="setting">
      <b>Duration when Opened (Seconds):</b><br/>
      <input type="range" id="openedSmokeDuration" name="openedSmokeDuration" min="1" max="10" value="1" step="1"
       oninput="durationOpenedOut.value=openedSmokeDuration.value"/>
      <output class="labelSlider" id="durationOpenedOut" for="openedSmokeDuration"></output>
    </div>
    <div class="setting">
      <label class="toggle-switchy" data-text="yesno" data-label="left">
        <input id="closedSmokeEnabled" name="closedSmokeEnabled" type="checkbox">
        <span class="toggle">
          <span class="switch"></span>
        </span>
        <span class="label">Smoke on Trap Close:</span>
      </label>
    </div>
    <div class="setting">
      <b>Duration when Closed (Seconds):</b><br/>
      <input type="range" id="closedSmokeDuration" name="closedSmokeDuration" min="1" max="10" value="1" step="1"
       oninput="durationClosedOut.value=closedSmokeDuration.value"/>
      <output class="labelSlider" id="durationClosedOut" for="closedSmokeDuration"></output>
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

    function getSettings() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var settings = JSON.parse(this.responseText);
          if (settings) {
            // Update fields with the current values, or supply an expected default as necessary.
            setValue("wifiName", settings.wifiName || "");
            setValue("displayType", settings.displayType || 0); // Default: 0 [Text]

            // Smoke settings for doors opening.
            setToggle("openedSmokeEnabled", settings.openedSmokeEnabled);
            setValue("openedSmokeDuration", settings.openedSmokeDuration || 1);
            setHtml("durationOpenedOut", getText("openedSmokeDuration"));

            // Smoke settings for doors closing.
            setToggle("closedSmokeEnabled", settings.closedSmokeEnabled);
            setValue("closedSmokeDuration", settings.closedSmokeDuration || 1);
            setHtml("durationClosedOut", getText("closedSmokeDuration"));
          }
        }
      };
      xhttp.open("GET", "/config/device", true);
      xhttp.send();
    }

    function saveSettings() {
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
        displayType: getInt("displayType"),
        openedSmokeEnabled: getToggle("openedSmokeEnabled"),
        closedSmokeEnabled: getToggle("closedSmokeEnabled"),
        openedSmokeDuration: getInt("openedSmokeDuration") || 1,
        closedSmokeDuration: getInt("closedSmokeDuration") || 1
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
