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

const char WAND_SETTINGS_page[] PROGMEM = R"=====(
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

  <h1>Bargraph Options</h1>
  <div class="block left">
    <div class="setting">
      <label class="toggle-switchy" data-text="yesno" data-label="left">
        <input id="invertWandBargraph" name="invertWandBargraph" type="checkbox">
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
      getEl("autoVentLight").disabled = true;
      getEl("deviceBootError").disabled = true;
      getEl("invertWandBargraph").disabled = true;
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

            if (settings.wandPowered) {
              disableControls();
              alert("Blaster is currently running. Changes to settings will not be allowed. Turn off devices via toggle switches and reload the page to obtain the latest settings.");
              return;
            }

            // Valid settings were received and both the pack and wand are off, so allow updating settings.
            getEl("btnSave").disabled = false;

            setToggle("autoVentLight", settings.autoVentLight);
            setToggle("deviceBootError", settings.deviceBootError);
            setToggle("invertWandBargraph", settings.invertWandBargraph);

            // Update colour preview and value display for hue/saturation sliders.
            updateColour("wandColourPreview", "wandHueOut", "wandSatOut", getEl("ledWandHue").value, getEl("ledWandSat").value);
          }
        }
      };
      xhttp.open("GET", "/config/wand", true);
      xhttp.send();
    }

    function saveSettings() {
      // Saves current settings to wand, updating runtime variables and making changes immediately effective.
      // This does NOT save to the EEPROM automatically as the user is encouraged to test prior to that action.
      var settings = {
        autoVentLight: getToggle("autoVentLight"),
        deviceBootError: getToggle("deviceBootError"),
        invertWandBargraph: getToggle("invertWandBargraph")
      };
      var body = JSON.stringify(settings);

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4) {
          if (this.status == 200) {
            handleStatus(this.responseText);
            setTimeout(getSettings, 400); // Get latest settings.

            if (confirm("Settings successfully updated. Do you want to store the latest settings to the blaster EEPROM?")) {
              saveEEPROM(); // Perform action only if the user answers OK to the confirmation.
            }
          } else {
            handleStatus(this.responseText);
          }
        }
      };
      xhttp.open("PUT", "/config/wand/save", true);
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
      xhttp.open("PUT", "/eeprom/wand", true);
      xhttp.send();
    }
  </script>
</body>
</html>
)=====";
