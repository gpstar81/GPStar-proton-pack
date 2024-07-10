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
  <meta charset="UTF-8">
  <meta http-equiv="Cache-control" content="public">
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
      <b>&nbsp;&nbsp;&nbsp;Default Theme:</b>
      <select id="defaultYearThemePack" name="defaultYearThemePack">
        <option value="1">System Toggle</option>
        <option value="2">1984</option>
        <option value="3">1989</option>
        <option value="4">Afterlife</option>
        <option value="5">Frozen Empire</option>
      </select>
    </div>
    <div class="setting">
      <b>&nbsp;&nbsp;&nbsp;Current Theme:</b>
      <select id="currentYearThemePack" name="currentYearThemePack">
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
        <option value="20">20 - Frutto (4x5)</option>
        <option value="36">36 - Frutto (4x9)</option>
        <option value="40">40 - RGB Ring</option>
      </select>
    </div>
    <div class="setting">
      <b>Custom Colour (Hue):</b><br/>
      <input type="range" id="ledCycLidHue" name="ledCycLidHue" min="0" max="360" value="360" step="2"
       oninput="updateColour('cycColourPreview', 'cycHueOut', 'cycSatOut', ledCycLidHue.value, ledCycLidSat.value)"/>
      <output class="labelSlider" id="cycHueOut" for="ledCycLidHue"></output>
      <br/>
      <div id="cycColourPreview" class="swatch"></div>
    </div>
    <div class="setting">
      <b>Custom Saturation %:</b><br/>
      <input type="range" id="ledCycLidSat" name="ledCycLidSat" min="0" max="100" value="100" step="2"
       oninput="updateColour('cycColourPreview', 'cycHueOut', 'cycSatOut', ledCycLidHue.value, ledCycLidSat.value)"/>
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
      <b class="labelSwitch">Enable Video Game Colours:</b>
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
        <option value="36">36 - GPStar</option>
        <option value="35">35</option>
        <option value="26">26</option>
        <option value="24">24</option>
        <option value="23">23</option>
        <option value="12">12</option>
      </select>
    </div>
    <div class="setting">
      <b class="labelSwitch">Use Frutto RGB LED Panel:</b>
      <label class="switch">
        <input id="ledCycInnerPanel" name="ledCycInnerPanel" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Swap Red/Green LEDs (GRB):</b>
      <label class="switch">
        <input id="ledCycCakeGRB" name="ledCycCakeGRB" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b>Custom Colour (Hue):</b><br/>
      <input type="range" id="ledCycCakeHue" name="ledCycCakeHue" min="0" max="360" value="360" step="2"
       oninput="updateColour('cakeColourPreview', 'cakeHueOut', 'cakeSatOut', ledCycCakeHue.value, ledCycCakeSat.value)"/>
      <output class="labelSlider" id="cakeHueOut" for="ledCycCakeHue"></output>
      <br/>
      <div id="cakeColourPreview" class="swatch"></div>
    </div>
    <div class="setting">
      <b>Custom Saturation %:</b><br/>
      <input type="range" id="ledCycCakeSat" name="ledCycCakeSat" min="0" max="100" value="100" step="2"
       oninput="updateColour('cakeColourPreview', 'cakeHueOut', 'cakeSatOut', ledCycCakeHue.value, ledCycCakeSat.value)"/>
      <output class="labelSlider" id="cakeSatOut" for="ledCycCakeSat"></output>
    </div>
    <div class="setting">
      <b>Cyclotron Cavity Lights:</b><br/>
      <input type="range" id="ledCycCavCount" name="ledCycCavCount" min="0" max="20" value="0" step="2"
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
      <b>Custom Colour (Hue):</b><br/>
      <input type="range" id="ledPowercellHue" name="ledPowercellHue" min="0" max="360" value="360" step="2"
       oninput="updateColour('pcColourPreview', 'pcHueOut', 'pcSatOut', ledPowercellHue.value, ledPowercellSat.value)"/>
      <output class="labelSlider" id="pcHueOut" for="ledPowercellHue"></output>
      <br/>
      <div id="pcColourPreview" class="swatch"></div>
    </div>
    <div class="setting">
      <b>Custom Saturation %:</b><br/>
      <input type="range" id="ledPowercellSat" name="ledPowercellSat" min="0" max="100" value="100" step="2"
       oninput="updateColour('pcColourPreview', 'pcHueOut', 'pcSatOut', ledPowercellHue.value, ledPowercellSat.value)"/>
      <output class="labelSlider" id="pcSatOut" for="ledPowercellSat"></output>
    </div>
    <div class="setting">
      <b class="labelSwitch">Enable Video Game Colours:</b>
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

    function getEl(id){
      return document.getElementById(id);
    }

    // Converts a value from one range to another: eg. convertRange(160, [2,254], [0,360])
    function convertRange(value, r1, r2) {
      return Math.round((value - r1[0]) * (r2[1] - r2[0]) / (r1[1] - r1[0]) + r2[0]);
    }

    function updateColour(colourPreviewID, hueLabelID, satLabelID, hueValue, satValue) {
      // Updates the slider values and preview the selected colour using HSL.
      getEl(hueLabelID).innerHTML = hueValue;
      getEl(satLabelID).innerHTML = satValue;
      var lightness = convertRange(100 - parseInt(satValue, 10), [0,100], [50,100]);
      getEl(colourPreviewID).style.backgroundColor = "hsl(" + parseInt(hueValue, 10) + ", " + parseInt(satValue, 10) + "%, " + lightness + "%)";
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
             * Note: Colour (hue) value range for FastLED uses the following scale, though CSS uses 0-360 for HSL colour.
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
            getEl("defaultSystemModePack").value = settings.defaultSystemModePack || 0;
            getEl("defaultYearThemePack").value = settings.defaultYearThemePack || 1; // Value cannot be 0.
            getEl("currentYearThemePack").value = settings.currentYearThemePack || 4; // Value cannot be 0.
            getEl("defaultSystemVolume").value = settings.defaultSystemVolume || 100; // Default to full volume.
            getEl("packVibration").value = settings.packVibration || 4;
            getEl("masterVolOut").innerHTML = getEl("defaultSystemVolume").value;
            getEl("protonStreamEffects").checked = settings.protonStreamEffects ? true : false;
            getEl("ribbonCableAlarm").checked = settings.ribbonCableAlarm ? true : false;
            getEl("overheatStrobeNF").checked = settings.overheatStrobeNF ? true : false;
            getEl("overheatLightsOff").checked = settings.overheatLightsOff ? true : false;
            getEl("overheatSyncToFan").checked = settings.overheatSyncToFan ? true : false;
            getEl("demoLightMode").checked = settings.demoLightMode ? true : false;

            getEl("ledCycLidCount").value = settings.ledCycLidCount || 12; // Haslab: 12
            getEl("ledCycLidHue").value = convertRange(settings.ledCycLidHue || 254, [1,254], [0,360]); // Default: Red
            getEl("ledCycLidSat").value = convertRange(settings.ledCycLidSat || 254, [1,254], [0,100]); // Full Saturation
            getEl("cyclotronDirection").value = settings.cyclotronDirection || 0;
            getEl("ledCycLidCenter").value = settings.ledCycLidCenter || 0;
            getEl("ledVGCyclotron").checked = settings.ledVGCyclotron ? true : false;
            getEl("ledCycLidSimRing").checked = settings.ledCycLidSimRing ? true : false;

            getEl("ledCycCakeCount").value = settings.ledCycCakeCount || 35; // Default: 35
            getEl("ledCycCakeHue").value = convertRange(settings.ledCycCakeHue || 254, [1,254], [0,360]); // Default: Red
            getEl("ledCycCakeSat").value = convertRange(settings.ledCycCakeSat || 254, [1,254], [0,100]); // Full Saturation
            getEl("ledCycInnerPanel").checked = settings.ledCycInnerPanel ? true : false;
            getEl("ledCycCakeGRB").checked = settings.ledCycCakeGRB ? true : false;
            getEl("ledCycCavCount").value = settings.ledCycCavCount || 0; // Default: 0
            getEl("ledCycCavCountOut").innerHTML = getEl("ledCycCavCount").value;

            getEl("ledPowercellCount").value = settings.ledPowercellCount || 13; // Haslab: 13
            getEl("ledPowercellHue").value = convertRange(settings.ledPowercellHue || 160, [1,254], [0,360]); // Default: Blue
            getEl("ledPowercellSat").value = convertRange(settings.ledPowercellSat || 254, [1,254], [0,100]); // Full Saturation
            getEl("ledVGPowercell").checked = settings.ledVGPowercell ? true : false;

            // Update colour preview and value display for hue/saturation sliders.
            updateColour("cycColourPreview", "cycHueOut", "cycSatOut", getEl("ledCycLidHue").value, getEl("ledCycLidSat").value);
            updateColour("cakeColourPreview", "cakeHueOut", "cakeSatOut", getEl("ledCycCakeHue").value, getEl("ledCycCakeSat").value);
            updateColour("pcColourPreview", "pcHueOut", "pcSatOut", getEl("ledPowercellHue").value, getEl("ledPowercellSat").value);
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
        defaultSystemModePack: parseInt(getEl("defaultSystemModePack").value || 0, 10),
        defaultYearThemePack: parseInt(getEl("defaultYearThemePack").value || 1, 10),
        currentYearThemePack: parseInt(getEl("currentYearThemePack").value || 4, 10),
        defaultSystemVolume: parseInt(getEl("defaultSystemVolume").value || 0, 10),
        packVibration: parseInt(getEl("packVibration").value || 4, 10),
        protonStreamEffects: getEl("protonStreamEffects").checked ? 1 : 0,
        ribbonCableAlarm: getEl("ribbonCableAlarm").checked ? 1 : 0,
        overheatStrobeNF: getEl("overheatStrobeNF").checked ? 1 : 0,
        overheatLightsOff: getEl("overheatLightsOff").checked ? 1 : 0,
        overheatSyncToFan: getEl("overheatSyncToFan").checked ? 1 : 0,
        demoLightMode: getEl("demoLightMode").checked ? 1 : 0,

        ledCycLidCount: parseInt(getEl("ledCycLidCount").value || 12, 10),
        ledCycLidHue: convertRange(parseInt(getEl("ledCycLidHue").value || 360, 10), [0,360], [1,254]),
        ledCycLidSat: convertRange(parseInt(getEl("ledCycLidSat").value || 100, 10), [0,100], [1,254]),
        cyclotronDirection: parseInt(getEl("cyclotronDirection").value || 0, 10),
        ledCycLidCenter: parseInt(getEl("ledCycLidCenter").value || 0, 10),
        ledVGCyclotron: getEl("ledVGCyclotron").checked ? 1 : 0,
        ledCycLidSimRing: getEl("ledCycLidSimRing").checked ? 1 : 0,

        ledCycCakeCount: parseInt(getEl("ledCycCakeCount").value || 35, 10),
        ledCycCakeHue: convertRange(parseInt(getEl("ledCycCakeHue").value || 360, 10), [0,360], [1,254]),
        ledCycCakeSat: convertRange(parseInt(getEl("ledCycCakeSat").value || 100, 10), [0,100], [1,254]),
        ledCycInnerPanel: getEl("ledCycInnerPanel").checked ? 1 : 0,
        ledCycCakeGRB: getEl("ledCycCakeGRB").checked ? 1 : 0,
        ledCycCavCount: parseInt(getEl("ledCycCavCount").value || 0, 10),

        ledPowercellCount: parseInt(getEl("ledPowercellCount").value || 13, 10),
        ledPowercellHue: convertRange(parseInt(getEl("ledPowercellHue").value || 200, 10), [0,360], [1,254]),
        ledPowercellSat: convertRange(parseInt(getEl("ledPowercellSat").value || 100, 10), [0,100], [1,254]),
        ledVGPowercell: getEl("ledVGPowercell").checked ? 1 : 0
      };
      var body = JSON.stringify(settings);

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          handleStatus(this.responseText);
          getSettings(); // Get latest settings.
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
