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

const char WAND_SETTINGS_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Neutrona Wand Settings</title>
  <link rel="icon" href="data:;base64,iVBORw0KGgo=">
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1>Wand Settings</h1>
  <div class="block left">
    <p>
      Change system configuration options using the available toggles and selectors.
      Options may only be changed when the pack and wand are not powered and running.
      Use the "Update Settings" button to save values to your equipment.
      Test your changes before using the "Save to EEPROM" to store as permanent defaults.
    </p>
    <br/>
  </div>

  <h1>General Options</h1>
  <div class="block left">
    <div class="setting">
      <b>Stream/Firing Mode:</b>
      <select id="defaultFiringMode" name="defaultFiringMode">
        <option value="1">Video Game</option>
        <option value="2">CTS</option>
        <option value="3">CTS Mix</option>
      </select>
    </div>
    <div class="setting">
      &nbsp;&nbsp;&nbsp;<b>Wand Year Theme:</b>
      <select id="defaultYearModeWand" name="defaultYearModeWand">
        <option value="1">Via System</option>
        <option value="2">1984</option>
        <option value="3">1989</option>
        <option value="4">Afterlife</option>
        <option value="5">Frozen Empire</option>
      </select>
    </div>
    <div class="setting">
      &nbsp;&nbsp;&nbsp;&nbsp;<b>Default CTS Mode:</b>
      <select id="defaultYearModeCTS" name="defaultYearModeCTS">
        <option value="1">Via System</option>
        <option value="2">1984</option>
        <option value="3">1989</option>
        <option value="4">Afterlife</option>
        <option value="5">Frozen Empire</option>
      </select>
    </div>
    <div class="setting">
      &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<b>Use Vibration:</b>
      <select id="wandVibration" name="wandVibration">
        <option value="4">Via Toggle</option>
        <option value="3">Never</option>
        <option value="2">When Firing</option>
        <option value="1">Always</option>
      </select>
    </div>
    <div class="setting">
      <b class="labelSwitch">Auto Vent Light Brightness:</b>
      <label class="switch">
        <input id="autoVentLight" name="autoVentLight" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Lights Off During Overheat:</b>
      <label class="switch">
        <input id="overheatLightsOff" name="overheatLightsOff" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Spectral Modes Enabled:</b>
      <label class="switch">
        <input id="spectralModeEnabled" name="spectralModeEnabled" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Spectral Mode - Holiday:</b>
      <label class="switch">
        <input id="spectralHolidayMode" name="spectralHolidayMode" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Overheating Enabled:</b>
      <label class="switch">
        <input id="overheatEnabled" name="overheatEnabled" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Quick Venting Enabled:</b>
      <label class="switch">
        <input id="quickVenting" name="quickVenting" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Send Sounds to Pack:</b>
      <label class="switch">
        <input id="wandSoundsToPack" name="wandSoundsToPack" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">&nbsp;Wand Beep Loop:</b>
      <label class="switch">
        <input id="wandBeepLoop" name="wandBeepLoop" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Wand Boot Errors:</b>
      <label class="switch">
        <input id="wandBootError" name="wandBootError" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
  </div>

  <h1>Bargraph Options</h1>
  <div class="block left">
    <div class="setting">
      <b>&nbsp;&nbsp;&nbsp;Idle Animation:</b>
      <select id="bargraphIdleAnimation" name="bargraphIdleAnimation">
        <option value="1">Default</option>
        <option value="2">Super Hero</option>
        <option value="3">Mode Original</option>
      </select>
    </div>
    <div class="setting">
      <b>Firing Animation:</b>
      <select id="bargraphFireAnimation" name="bargraphFireAnimation">
        <option value="1">Default</option>
        <option value="2">Super Hero</option>
        <option value="3">Mode Original</option>
      </select>
    </div>
    <div class="setting">
      <b class="labelSwitch">Blink on Overheat:</b>
      <label class="switch">
        <input id="bargraphOverheatBlink" name="bargraphOverheatBlink" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Invert Animations:</b>
      <label class="switch">
        <input id="invertWandBargraph" name="invertWandBargraph" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
  </div>

  <h1>Barrel LEDs</h1>
  <div class="block left">
    <div class="setting">
      <b>LED Count:</b>
      <select id="ledWandCount" name="ledWandCount">
        <option value="0">5 - Stock</option>
        <option value="1">48 - Frutto</option>
      </select>
    </div>
    <div class="setting">
      <b>Custom Color (Hue):</b><br/>
      <input type="range" id="ledWandHue" name="ledWandHue" min="0" max="360" value="360" step="2"
       oninput="updateColor('wandColorPreview', 'wandHueOut', 'wandSatOut', ledWandHue.value, ledWandSat.value)"/>
      <output class="labelSlider" id="wandHueOut" for="ledWandHue"></output>
      <br/>
      <div id="wandColorPreview" class="swatch"></div>
    </div>
    <div class="setting">
      <b>Custom Saturation %:</b><br/>
      <input type="range" id="ledWandSat" name="ledWandSat" min="0" max="100" value="100" step="2"
       oninput="updateColor('wandColorPreview', 'wandHueOut', 'wandSatOut', ledWandHue.value, ledWandSat.value)"/>
      <output class="labelSlider" id="wandSatOut" for="ledWandSat"></output>
    </div>
  </div>

  <div class="block">
    <hr/>
    <a href="/">&laquo; Back</a>
    &nbsp;&nbsp;
    <button type="button" class="green" style="width:120px" onclick="saveSettings()">Update&nbsp;Settings</button>
    &nbsp;
    <button type="button" class="orange" style="width:120px" onclick="saveEEPROM()">Save&nbsp;to&nbsp;EPROM</button>
    <br/>
    <br/>
  </div>

  <script type="application/javascript">
    window.addEventListener("load", onLoad);

    function onLoad(event) {
      // Wait 0.4s for serial communications between devices.
      setTimeout(getSettings, 400);
    }

    // Converts a value from one range to another: eg. convertRange(160, [2,254], [0,360])
    function convertRange(value, r1, r2) {
      return Math.round((value - r1[0]) * (r2[1] - r2[0]) / (r1[1] - r1[0]) + r2[0]);
    }

    function updateColor(colorPreviewID, hueLabelID, satLabelID, hueValue, satValue) {
      // Updates the slider values and preview the selected color using HSL.
      document.getElementById(hueLabelID).innerHTML = hueValue;
      document.getElementById(satLabelID).innerHTML = satValue;
      var lightness = convertRange(100 - parseInt(satValue, 10), [0,100], [50,100]);
      document.getElementById(colorPreviewID).style.backgroundColor = "hsl(" + parseInt(hueValue, 10) + ", " + parseInt(satValue, 10) + "%, " + lightness + "%)";
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
            if (!settings.prefsAvailable) {
              alert("An unexpected error occurred and preferences could not be downloaded. Please refresh the page to try again.");
              return;
            }

            if (settings.packPowered || settings.wandPowered) {
              alert("Pack and/or Wand are currently running. Changes to settings will not be allowed. Turn off devices via toggle switches and reload the page to obtain the latest settings.");
            }

            /**
             * Note: Color (hue) value range for FastLED uses the following scale, though CSS uses 0-360 for HSL color.
             *  0 = Red
             *  32 = Orange
             *  64 = Yellow
             *  96 = Green
             *  128 = Aqua
             *  160 = Blue
             *  192 = Purple
             *  224 = Pink
             *  254 = Red
             */

            document.getElementById("ledWandCount").value = settings.ledWandCount || 0; // Haslab: 0 (5 LED)
            document.getElementById("ledWandHue").value = convertRange(settings.ledWandHue || 254, [1,254], [0,360]); // Default: Red
            document.getElementById("ledWandSat").value = convertRange(settings.ledWandSat || 254, [1,254], [0,100]); // Full Saturation
            document.getElementById("spectralModeEnabled").checked = settings.spectralModeEnabled ? true: false;
            document.getElementById("spectralHolidayMode").checked = settings.spectralHolidayMode ? true: false;
            document.getElementById("overheatEnabled").checked = settings.overheatEnabled ? true: false;
            document.getElementById("defaultFiringMode").value = settings.defaultFiringMode || 1;
            document.getElementById("wandVibration").value = settings.wandVibration || 4;
            document.getElementById("wandSoundsToPack").checked = settings.wandSoundsToPack ? true: false;
            document.getElementById("quickVenting").checked = settings.quickVenting ? true: false;
            document.getElementById("autoVentLight").checked = settings.autoVentLight ? true: false;
            document.getElementById("wandBeepLoop").checked = settings.wandBeepLoop ? true: false;
            document.getElementById("wandBootError").checked = settings.wandBootError ? true: false;
            document.getElementById("defaultYearModeWand").value = settings.defaultYearModeWand || 1;
            document.getElementById("defaultYearModeCTS").value = settings.defaultYearModeCTS || 1;
            document.getElementById("invertWandBargraph").checked = settings.invertWandBargraph ? true: false;
            document.getElementById("bargraphOverheatBlink").checked = settings.bargraphOverheatBlink ? true: false;
            document.getElementById("bargraphIdleAnimation").value = settings.bargraphIdleAnimation || 1;
            document.getElementById("bargraphFireAnimation").value = settings.bargraphFireAnimation || 1;

            // Update color preview and value display for hue/saturation sliders.
            updateColor("wandColorPreview", "wandHueOut", "wandSatOut", document.getElementById("ledWandHue").value, document.getElementById("ledWandSat").value);
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
        ledWandCount: parseInt(document.getElementById("ledWandCount").value || 0, 10),
        ledWandHue: convertRange(parseInt(document.getElementById("ledWandHue").value || 360, 10), [0,360], [1,254]),
        ledWandSat: convertRange(parseInt(document.getElementById("ledWandSat").value || 100, 10), [0,100], [1,254]),
        spectralModeEnabled: document.getElementById("spectralModeEnabled").checked ? 1 : 0,
        spectralHolidayMode: document.getElementById("spectralHolidayMode").checked ? 1 : 0,
        overheatEnabled: document.getElementById("overheatEnabled").checked ? 1 : 0,
        defaultFiringMode: parseInt(document.getElementById("defaultFiringMode").value || 1, 10),
        wandVibration: parseInt(document.getElementById("wandVibration").value || 4, 10),
        wandSoundsToPack: document.getElementById("wandSoundsToPack").checked ? 1 : 0,
        quickVenting: document.getElementById("quickVenting").checked ? 1 : 0,
        autoVentLight: document.getElementById("autoVentLight").checked ? 1 : 0,
        wandBeepLoop: document.getElementById("wandBeepLoop").checked ? 1 : 0,
        wandBootError: document.getElementById("wandBootError").checked ? 1 : 0,
        defaultYearModeWand: parseInt(document.getElementById("defaultYearModeWand").value || 1, 10),
        defaultYearModeCTS: parseInt(document.getElementById("defaultYearModeCTS").value || 1, 10),
        invertWandBargraph: document.getElementById("invertWandBargraph").checked ? 1 : 0,
        bargraphOverheatBlink: document.getElementById("bargraphOverheatBlink").checked ? 1 : 0,
        bargraphIdleAnimation: parseInt(document.getElementById("bargraphIdleAnimation").value || 1, 10),
        bargraphFireAnimation: parseInt(document.getElementById("bargraphFireAnimation").value || 1, 10)
      };
      var body = JSON.stringify(settings);

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          handleStatus(this.responseText);
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