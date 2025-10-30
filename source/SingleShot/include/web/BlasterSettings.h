/**
 *   GPStar Single Shot Blaster - Ghostbusters Proton Pack & Neutrona Wand.
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

const char BLASTER_SETTINGS_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta http-equiv="Cache-control" content="public">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Single Shot Blaster Settings</title>
  <link rel="icon" type="image/svg+xml" href="/favicon.svg"/>
  <link rel="shortcut icon" href="/favicon.ico"/>
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1 id="top">Single Shot Blaster Settings</h1>
  <div class="block left">
    <p>
      Change system configuration options using the available toggles and selectors.
      Options may only be changed when the blaster is not powered or running.
      Use the "Update Settings" button to save values to your equipment.
    </p>
    <br/>
  </div>

  <h1>General Options</h1>
  <div class="block left">
    <div class="setting">
      <b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Use Vibration:</b>
      <select id="deviceVibration" name="deviceVibration" style="width:170px">
        <option value="3">Never</option>
        <option value="2">When Firing</option>
        <option value="1">Always</option>
      </select>
    </div>
    <div class="setting">
      <label class="toggle-switchy" data-label="left">
        <input id="autoVentLight" name="autoVentLight" type="checkbox">
        <span class="toggle">
          <span class="switch"></span>
        </span>
        <span class="label">Auto Vent Light Brightness:</span>
      </label>
    </div>
    <div class="setting">
      <label class="toggle-switchy" data-label="left">
        <input id="deviceBootError" name="deviceBootError" type="checkbox">
        <span class="toggle">
          <span class="switch"></span>
        </span>
        <span class="label">Boot Errors:</span>
      </label>
    </div>
  </div>

  <h1>Audio Options</h1>
  <div class="block left">
    <div class="setting">
      <b>Master Volume % at Startup:</b><br/>
      <input type="range" id="defaultSystemVolume" name="defaultSystemVolume" min="5" max="100" value="100" step="5"
       oninput="masterVolOut.value=defaultSystemVolume.value"/>
      <output class="labelSlider" id="masterVolOut" for="defaultSystemVolume"></output>
    </div>
    <div class="setting" id="gpstarAudioLedToggle">
      <label class="toggle-switchy" data-label="left">
        <input id="gpstarAudioLed" name="gpstarAudioLed" type="checkbox">
        <span class="toggle">
          <span class="switch"></span>
        </span>
        <span class="label">GPStar Audio Status LED:</span>
      </label>
    </div>
  </div>

  <h1>Bargraph Options</h1>
  <div class="block left">
    <div class="setting">
      <label class="toggle-switchy" data-text="yesno" data-label="left">
        <input id="invertBlasterBargraph" name="invertBlasterBargraph" type="checkbox">
        <span class="toggle">
          <span class="switch"></span>
        </span>
        <span class="label">Invert Animations:</span>
      </label>
    </div>
  </div>

  <div class="block">
    <a href="#top">Top</a>
    <hr/>
    <a href="/">&laquo; Back</a>
    &nbsp;&nbsp;
    <button type="button" class="green" style="width:120px" onclick="saveSettings()" id="btnSave">Update&nbsp;Settings</button>
    <br/>
    <br/>
  </div>

  <script type="application/javascript" src="/common.js"></script>
  <script type="application/javascript">
    window.addEventListener("load", onLoad);

    function onLoad(event) {
      // Wait 0.1s for serial communications between devices.
      setTimeout(getSettings, 100);

      // Disable the save button until we obtain settings.
      getEl("btnSave").disabled = true;
    }

    function disableControls() {
      // Disables all controls.
      disableEl("autoVentLight");
      disableEl("defaultSystemVolume");
      disableEl("deviceBootError");
      disableEl("deviceVibration");
      disableEl("gpstarAudioLed");
      disableEl("invertBlasterBargraph");
      hideEl("gpstarAudioLedToggle");
    }

    // Converts a value from one range to another: eg. convertRange(160, [2,254], [0,360])
    function convertRange(value, r1, r2) {
      return Math.round((value - r1[0]) * (r2[1] - r2[0]) / (r1[1] - r1[0]) + r2[0]);
    }

    function updateColour(colourPreviewID, hueLabelID, satLabelID, hueValue, satValue) {
      // Updates the slider values and preview the selected colour using HSL.
      setHtml(hueLabelID, hueValue);
      setHtml(satLabelID, satValue);
      var lightness = convertRange(100 - parseInt(satValue, 10), [0,100], [50,100]);
      getEl(colourPreviewID).style.backgroundColor = "hsl(" + parseInt(hueValue, 10) + ", " + parseInt(satValue, 10) + "%, " + lightness + "%)";
    }

    function getSettings() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var settings = JSON.parse(this.responseText);
          if (settings) {
            if (!settings.prefsAvailable) {
              disableControls();
              alert("Preferences could not be downloaded. Please refresh the page to try again.");
              return;
            }

            if (settings.blasterPowered) {
              disableControls();
              alert("Blaster is currently running. Changes to settings will not be allowed. Turn off devices via toggle switches and reload the page to obtain the latest settings.");
              return;
            }

            if (!settings.gpstarAudio) {
              // Hide the GPStar Audio LED Status toggle if wand is not using GPStar Audio.
              hideEl("gpstarAudioLedToggle");
              disableEl("gpstarAudioLed");
            }

            // Valid settings were received and both the pack and blaster are off, so allow updating settings.
            enableEl("btnSave");

            setToggle("autoVentLight", settings.autoVentLight);
            setValue("defaultSystemVolume", settings.defaultSystemVolume || 100);
            setToggle("deviceBootError", settings.deviceBootError);
            setValue("deviceVibration", settings.wandVibration || 1);
            setToggle("gpstarAudioLed", settings.gpstarAudioLed);
            setToggle("invertBlasterBargraph", settings.invertBlasterBargraph);

            // Update colour preview and value display for hue/saturation sliders.
            //updateColour("blasterColourPreview", "blasterHueOut", "blasterSatOut", getEl("ledBlasterHue").value, getEl("ledBlasterSat").value);
          }
        }
      };
      xhttp.open("GET", "/config/blaster", true);
      xhttp.send();
    }

    function saveSettings() {
      // Saves current settings to blaster, updating runtime variables and making changes immediately effective.
      // This does NOT save to the EEPROM automatically as the user is encouraged to test prior to that action.
      var settings = {
        autoVentLight: getToggle("autoVentLight"),
        defaultSystemVolume: getInt("defaultSystemVolume") || 100,
        deviceBootError: getToggle("deviceBootError"),
        deviceVibration: getInt("deviceVibration") || 1,
        gpstarAudioLed: getToggle("gpstarAudioLed"),
        invertBlasterBargraph: getToggle("invertBlasterBargraph")
      };
      var body = JSON.stringify(settings);

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4) {
          if (this.status == 200) {
            handleStatus(this.responseText);
            setTimeout(getSettings, 400); // Get latest settings.
          } else {
            handleStatus(this.responseText);
          }
        }
      };
      xhttp.open("PUT", "/config/blaster/save", true);
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
      xhttp.open("PUT", "/eeprom/blaster", true);
      xhttp.send();
    }
  </script>
</body>
</html>
)=====";
