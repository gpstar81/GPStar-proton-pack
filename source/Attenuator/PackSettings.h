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

const char PACK_SETTINGS_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Proton Pack Settings</title>
  <link rel="icon" href="data:;base64,iVBORw0KGgo=">
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1>Pack Settings</h1>
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
      <b>Operation Mode:</b>
      <select id="defaultSystemModePack" name="defaultSystemModePack">
        <option value="0">Super Hero</option>
        <option value="1">Mode Original</option>
      </select>
    </div>
    <div class="setting">
      <b>&nbsp;&nbsp;&nbsp;Effects Theme:</b>
      <select id="defaultYearThemePack" name="defaultYearThemePack">
        <option value="1">System Toggle</option>
        <option value="2">1984</option>
        <option value="3">1989</option>
        <option value="4">Afterlife</option>
        <option value="5">Frozen Empire</option>
      </select>
    </div>
    <div class="setting">
      <b>&nbsp;&nbsp;&nbsp;&nbsp;Use Vibration:</b>
      <select id="packVibration" name="packVibration">
        <option value="1">Always</option>
        <option value="2">When Firing</option>
        <option value="3">Never</option>
        <option value="4">Via Toggle</option>
      </select>
    </div>
    <div class="setting">
      <b>Master Volume % at Startup:</b><br/>
      <input type="range" id="defaultSystemVolume" name="defaultSystemVolume" min="0" max="100" value="100" step="5"
       oninput="masterVolOut.value=defaultSystemVolume.value"/>
      <output class="labelSlider" id="masterVolOut" for="defaultSystemVolume"></output>
    </div>
    <div class="setting">
      <b class="labelSwitch">Lights Off During Overheat:</b>
      <label class="switch">
        <input id="overheatLightsOff" name="overheatLightsOff" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Overheat Sync Smoke to Fan:</b>
      <label class="switch">
        <input id="overheatSyncToFan" name="overheatSyncToFan" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Proton Stream Impact Effects:</b>
      <label class="switch">
        <input id="protonStreamEffects" name="protonStreamEffects" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Ribbon Cable Alarm:</b>
      <label class="switch">
        <input id="ribbonCableAlarm" name="ribbonCableAlarm" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Startup Light Mode Enabled:</b>
      <label class="switch">
        <input id="demoLightMode" name="demoLightMode" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Strobe N-Filter on Overheat:</b>
      <label class="switch">
        <input id="overheatStrobeNF" name="overheatStrobeNF" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
  </div>

  <h1>Cyclotron Lid</h1>
  <div class="block left">
    <div class="setting">
      <b>LED Count:</b>
      <select id="ledCycLidCount" name="ledCycLidCount">
        <option value="12">12 - Stock</option>
        <option value="20">20 - Frutto</option>
        <option value="40">40 - Ring</option>
      </select>
    </div>
    <div class="setting">
      <b>Custom Color (Hue):</b><br/>
      <input type="range" id="ledCycLidHue" name="ledCycLidHue" min="0" max="360" value="360" step="2"
       oninput="updateColor('cycColorPreview', 'cycHueOut', 'cycSatOut', ledCycLidHue.value, ledCycLidSat.value)"/>
      <output class="labelSlider" id="cycHueOut" for="ledCycLidHue"></output>
      <br/>
      <div id="cycColorPreview" class="swatch"></div>
    </div>
    <div class="setting">
      <b>Custom Saturation %:</b><br/>
      <input type="range" id="ledCycLidSat" name="ledCycLidSat" min="0" max="100" value="100" step="2"
       oninput="updateColor('cycColorPreview', 'cycHueOut', 'cycSatOut', ledCycLidHue.value, ledCycLidSat.value)"/>
      <output class="labelSlider" id="cycSatOut" for="ledCycLidSat"></output>
    </div>
    <div class="setting">
      <b>Spin Direction:</b>
      <select id="cyclotronDirection" name="cyclotronDirection">
        <option value="0">Counter-Clockwise</option>
        <option value="1">Clockwise</option>
      </select>
    </div>
    <div class="setting">
      <b>&nbsp;&nbsp;&nbsp;Center LEDs:</b>
      <select id="ledCycLidCenter" name="ledCycLidCenter">
        <option value="0">3 LED</option>
        <option value="1">1 LED</option>
      </select>
    </div>
    <div class="setting">
      <b class="labelSwitch">Enable Video Game Colors:</b>
      <label class="switch">
        <input id="ledVGCyclotron" name="ledVGCyclotron" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Simulate Ring Spin Effect:</b>
      <label class="switch">
        <input id="ledCycLidSimRing" name="ledCycLidSimRing" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
  </div>

  <h1>Inner Cyclotron</h1>
  <div class="block left">
    <div class="setting">
      <b>LED Count:</b>
      <select id="ledCycCakeCount" name="ledCycCakeCount">
        <option value="35">35 - GPStar</option>
        <option value="24">24</option>
        <option value="23">23</option>
        <option value="12">12</option>
      </select>
    </div>
    <div class="setting">
      <b class="labelSwitch">Swap Red/Green LEDs (GRB):</b>
      <label class="switch">
        <input id="ledCycCakeGRB" name="ledCycCakeGRB" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b>Custom Color (Hue):</b><br/>
      <input type="range" id="ledCycCakeHue" name="ledCycCakeHue" min="0" max="360" value="360" step="2"
       oninput="updateColor('cakeColorPreview', 'cakeHueOut', 'cakeSatOut', ledCycCakeHue.value, ledCycCakeSat.value)"/>
      <output class="labelSlider" id="cakeHueOut" for="ledCycCakeHue"></output>
      <br/>
      <div id="cakeColorPreview" class="swatch"></div>
    </div>
    <div class="setting">
      <b>Custom Saturation %:</b><br/>
      <input type="range" id="ledCycCakeSat" name="ledCycCakeSat" min="0" max="100" value="100" step="2"
       oninput="updateColor('cakeColorPreview', 'cakeHueOut', 'cakeSatOut', ledCycCakeHue.value, ledCycCakeSat.value)"/>
      <output class="labelSlider" id="cakeSatOut" for="ledCycCakeSat"></output>
    </div>
    <div class="setting">
      <b>Cyclotron Cavity Lights:</b><br/>
      <input type="range" id="ledCycCavCount" name="ledCycCavCount" min="0" max="30" value="0" step="2"
       oninput="ledCycCavCountOut.value=ledCycCavCount.value"/>
      <output class="labelSlider" id="ledCycCavCountOut" for="ledCycCavCount"></output>
    </div>
  </div>

  <h1>Power Cell</h1>
  <div class="block left">
    <div class="setting">
      <b>LED Count:</b>
      <select id="ledPowercellCount" name="ledPowercellCount">
        <option value="13">13 - Stock</option>
        <option value="15">15 - Frutto</option>
      </select>
    </div>
    <div class="setting">
      <b>Custom Color (Hue):</b><br/>
      <input type="range" id="ledPowercellHue" name="ledPowercellHue" min="0" max="360" value="360" step="2"
       oninput="updateColor('pcColorPreview', 'pcHueOut', 'pcSatOut', ledPowercellHue.value, ledPowercellSat.value)"/>
      <output class="labelSlider" id="pcHueOut" for="ledPowercellHue"></output>
      <br/>
      <div id="pcColorPreview" class="swatch"></div>
    </div>
    <div class="setting">
      <b>Custom Saturation %:</b><br/>
      <input type="range" id="ledPowercellSat" name="ledPowercellSat" min="0" max="100" value="100" step="2"
       oninput="updateColor('pcColorPreview', 'pcHueOut', 'pcSatOut', ledPowercellHue.value, ledPowercellSat.value)"/>
      <output class="labelSlider" id="pcSatOut" for="ledPowercellSat"></output>
    </div>
    <div class="setting">
      <b class="labelSwitch">Enable Video Game Colors:</b>
      <label class="switch">
        <input id="ledVGPowercell" name="ledVGPowercell" type="checkbox">
        <span class="slider round"></span>
      </label>
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

            // Update fields with the current values, or supply an expected default as necessary.
            document.getElementById("defaultSystemModePack").value = settings.defaultSystemModePack || 0;
            document.getElementById("defaultYearThemePack").value = settings.defaultYearThemePack || 1; // Value cannot be 0.
            document.getElementById("defaultSystemVolume").value = settings.defaultSystemVolume || 100; // Default to full volume.
            document.getElementById("packVibration").value = settings.packVibration || 4;
            document.getElementById("masterVolOut").innerHTML = document.getElementById("defaultSystemVolume").value;
            document.getElementById("protonStreamEffects").checked = settings.protonStreamEffects ? true: false;
            document.getElementById("ribbonCableAlarm").checked = settings.ribbonCableAlarm ? true: false;
            document.getElementById("overheatStrobeNF").checked = settings.overheatStrobeNF ? true: false;
            document.getElementById("overheatLightsOff").checked = settings.overheatLightsOff ? true: false;
            document.getElementById("overheatSyncToFan").checked = settings.overheatSyncToFan ? true: false;
            document.getElementById("demoLightMode").checked = settings.demoLightMode ? true: false;

            document.getElementById("ledCycLidCount").value = settings.ledCycLidCount || 12; // Haslab: 12
            document.getElementById("ledCycLidHue").value = convertRange(settings.ledCycLidHue || 254, [1,254], [0,360]); // Default: Red
            document.getElementById("ledCycLidSat").value = convertRange(settings.ledCycLidSat || 254, [1,254], [0,100]); // Full Saturation
            document.getElementById("cyclotronDirection").value = settings.cyclotronDirection || 0;
            document.getElementById("ledCycLidCenter").value = settings.ledCycLidCenter || 0;
            document.getElementById("ledVGCyclotron").checked = settings.ledVGCyclotron ? true: false;
            document.getElementById("ledCycLidSimRing").checked = settings.ledCycLidSimRing ? true: false;

            document.getElementById("ledCycCakeCount").value = settings.ledCycCakeCount || 35; // Default: 12
            document.getElementById("ledCycCakeHue").value = convertRange(settings.ledCycCakeHue || 254, [1,254], [0,360]); // Default: Red
            document.getElementById("ledCycCakeSat").value = convertRange(settings.ledCycCakeSat || 254, [1,254], [0,100]); // Full Saturation
            document.getElementById("ledCycCakeGRB").checked = settings.ledCycCakeGRB ? true: false;
            document.getElementById("ledCycCavCount").value = settings.ledCycCavCount || 0; // Default: 0
            document.getElementById("ledCycCavCountOut").innerHTML = document.getElementById("ledCycCavCount").value;

            document.getElementById("ledPowercellCount").value = settings.ledPowercellCount || 13; // Haslab: 13
            document.getElementById("ledPowercellHue").value = convertRange(settings.ledPowercellHue || 160, [1,254], [0,360]); // Default: Blue
            document.getElementById("ledPowercellSat").value = convertRange(settings.ledPowercellSat || 254, [1,254], [0,100]); // Full Saturation
            document.getElementById("ledVGPowercell").checked = settings.ledVGPowercell ? true: false;

            // Update color preview and value display for hue/saturation sliders.
            updateColor("cycColorPreview", "cycHueOut", "cycSatOut", document.getElementById("ledCycLidHue").value, document.getElementById("ledCycLidSat").value);
            updateColor("cakeColorPreview", "cakeHueOut", "cakeSatOut", document.getElementById("ledCycCakeHue").value, document.getElementById("ledCycCakeSat").value);
            updateColor("pcColorPreview", "pcHueOut", "pcSatOut", document.getElementById("ledPowercellHue").value, document.getElementById("ledPowercellSat").value);
          }
        }
      };
      xhttp.open("GET", "/config/pack", true);
      xhttp.send();
    }

    function saveSettings() {
      // Saves current settings to pack, updating runtime variables and making changes immediately effective.
      // This does NOT save to the EEPROM automatically as the user is encouraged to test prior to that action.
      var settings = {
        defaultSystemModePack: parseInt(document.getElementById("defaultSystemModePack").value || 0, 10),
        defaultYearThemePack: parseInt(document.getElementById("defaultYearThemePack").value || 1, 10),
        defaultSystemVolume: parseInt(document.getElementById("defaultSystemVolume").value || 0, 10),
        packVibration: parseInt(document.getElementById("packVibration").value || 4, 10),
        protonStreamEffects: document.getElementById("protonStreamEffects").checked ? 1 : 0,
        ribbonCableAlarm: document.getElementById("ribbonCableAlarm").checked ? 1 : 0,
        overheatStrobeNF: document.getElementById("overheatStrobeNF").checked ? 1 : 0,
        overheatLightsOff: document.getElementById("overheatLightsOff").checked ? 1 : 0,
        overheatSyncToFan: document.getElementById("overheatSyncToFan").checked ? 1 : 0,
        demoLightMode: document.getElementById("demoLightMode").checked ? 1 : 0,

        ledCycLidCount: parseInt(document.getElementById("ledCycLidCount").value || 12, 10),
        ledCycLidHue: convertRange(parseInt(document.getElementById("ledCycLidHue").value || 360, 10), [0,360], [1,254]),
        ledCycLidSat: convertRange(parseInt(document.getElementById("ledCycLidSat").value || 100, 10), [0,100], [1,254]),
        cyclotronDirection: parseInt(document.getElementById("cyclotronDirection").value || 0, 10),
        ledCycLidCenter: parseInt(document.getElementById("ledCycLidCenter").value || 0, 10),
        ledVGCyclotron: document.getElementById("ledVGCyclotron").checked ? 1 : 0,
        ledCycLidSimRing: document.getElementById("ledCycLidSimRing").checked ? 1 : 0,

        ledCycCakeCount: parseInt(document.getElementById("ledCycCakeCount").value || 35, 10),
        ledCycCakeHue: convertRange(parseInt(document.getElementById("ledCycCakeHue").value || 360, 10), [0,360], [1,254]),
        ledCycCakeSat: convertRange(parseInt(document.getElementById("ledCycCakeSat").value || 100, 10), [0,100], [1,254]),
        ledCycCakeGRB: document.getElementById("ledCycCakeGRB").checked ? 1 : 0,
        ledCycCavCount: parseInt(document.getElementById("ledCycCavCount").value || 0, 10),

        ledPowercellCount: parseInt(document.getElementById("ledPowercellCount").value || 13, 10),
        ledPowercellHue: convertRange(parseInt(document.getElementById("ledPowercellHue").value || 200, 10), [0,360], [1,254]),
        ledPowercellSat: convertRange(parseInt(document.getElementById("ledPowercellSat").value || 100, 10), [0,100], [1,254]),
        ledVGPowercell: document.getElementById("ledVGPowercell").checked ? 1 : 0
      };
      var body = JSON.stringify(settings);

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          handleStatus(this.responseText);
        }
      };
      xhttp.open("PUT", "/config/pack/save", true);
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