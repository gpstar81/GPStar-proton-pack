/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2024 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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
  <h1 id="top">Pack Settings</h1>
  <div class="block left">
    <p>
      Change system configuration options using the available toggles and selectors.
      Options may only be changed when the pack and wand are not powered and running.
      Use the "Update Settings" button to save values to your equipment.
    </p>
    <br/>
  </div>

  <h1>General Options</h1>
  <div class="block left">
    <div class="setting">
      <b>Operation Mode:</b>
      <select id="defaultSystemModePack" name="defaultSystemModePack" style="width:180px">
        <option value="0">Super Hero</option>
        <option value="1">Mode Original</option>
      </select>
    </div>
    <div class="setting">
      <b>&nbsp;&nbsp;&nbsp;Default Theme:</b>
      <select id="defaultYearThemePack" name="defaultYearThemePack" style="width:180px">
        <option value="1">System Toggle</option>
        <option value="2">1984</option>
        <option value="3">1989</option>
        <option value="4">Afterlife</option>
        <option value="5">Frozen Empire</option>
      </select>
    </div>
    <div class="setting">
      <b>&nbsp;&nbsp;&nbsp;Current Theme:</b>
      <select id="currentYearThemePack" name="currentYearThemePack" style="width:180px">
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
        <option value="5">Motorized Cyclotron</option>
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
      <select id="ledCycLidCount" name="ledCycLidCount" style="width:200px">
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
      <select id="cyclotronDirection" name="cyclotronDirection" style="width:200px">
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
      <b class="labelSwitch">Center LED Fade Effect:</b>
      <label class="switch">
        <input id="ledCycLidFade" name="ledCycLidFade" type="checkbox">
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
    <div class="setting">
      <b class="labelSwitch">Enable Video Game Colours:</b>
      <label class="switch">
        <input id="ledVGCyclotron" name="ledVGCyclotron" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
  </div>

  <h1>Inner Cyclotron</h1>
  <div class="block left">
    <div class="setting">
      <b>Ring LED Count:</b>
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
      <b>Inner Panel:</b>
      <select id="ledCycInnerPanel" name="ledCycInnerPanel" style="width:180px">
        <option value="1">Individual LEDs</option>
        <option value="2">Frutto (Static Colours)</option>
        <option value="3">Frutto (Dynamic Colours)</option>
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
        <option value="15">15 - Frutto/GPStar</option>
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
      <b class="labelSwitch">Invert Power Cell Animation:</b>
      <label class="switch">
        <input id="ledInvertPowercell" name="ledInvertPowercell" type="checkbox">
        <span class="slider round"></span>
      </label>
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
      // Wait 0.4s for serial communications between devices.
      setTimeout(getSettings, 400);

      // Disable the save button until we obtain settings.
      getEl("btnSave").disabled = true;
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
              alert("An unexpected error occurred and preferences could not be downloaded. Please refresh the page to try again.");
              return;
            }

            if (settings.packPowered || settings.wandPowered) {
              alert("Pack and/or Wand are currently running. Changes to settings will not be allowed. Turn off devices via toggle switches and reload the page to obtain the latest settings.");
              return;
            }

            // Valid settings were received and both the pack and wand are off, so allow updating settings.
            getEl("btnSave").disabled = false;

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
            setValue("defaultSystemModePack", settings.defaultSystemModePack || 0);
            setValue("defaultYearThemePack", settings.defaultYearThemePack || 1); // Value cannot be 0.
            setValue("currentYearThemePack", settings.currentYearThemePack || 4); // Value cannot be 0.
            setValue("defaultSystemVolume", settings.defaultSystemVolume || 100); // Default to full volume.
            setValue("packVibration", settings.packVibration || 4);
            setHtml("masterVolOut", getValue("defaultSystemVolume"));
            setToggle("protonStreamEffects", settings.protonStreamEffects);
            setToggle("ribbonCableAlarm", settings.ribbonCableAlarm);
            setToggle("overheatStrobeNF", settings.overheatStrobeNF);
            setToggle("overheatLightsOff", settings.overheatLightsOff);
            setToggle("overheatSyncToFan", settings.overheatSyncToFan);
            setToggle("demoLightMode", settings.demoLightMode);

            setValue("ledCycLidCount", settings.ledCycLidCount || 12); // Haslab: 12
            setValue("ledCycLidHue", convertRange(settings.ledCycLidHue || 254, [1,254], [0,360])); // Default: Red
            setValue("ledCycLidSat", convertRange(settings.ledCycLidSat || 254, [1,254], [0,100])); // Full Saturation
            setValue("cyclotronDirection", settings.cyclotronDirection || 0);
            setValue("ledCycLidCenter", settings.ledCycLidCenter || 0);
            setToggle("ledCycLidFade", settings.ledCycLidFade);
            setToggle("ledVGCyclotron", settings.ledVGCyclotron);
            setToggle("ledCycLidSimRing", settings.ledCycLidSimRing);

            setValue("ledCycCakeCount", settings.ledCycCakeCount || 35); // Default: 35
            setValue("ledCycCakeHue", convertRange(settings.ledCycCakeHue || 254, [1,254], [0,360])); // Default: Red
            setValue("ledCycCakeSat", convertRange(settings.ledCycCakeSat || 254, [1,254], [0,100])); // Full Saturation
            setValue("ledCycInnerPanel", settings.ledCycInnerPanel || 1); // Default: Individual
            setToggle("ledCycCakeGRB", settings.ledCycCakeGRB);
            setValue("ledCycCavCount", settings.ledCycCavCount || 0); // Default: 0
            setHtml("ledCycCavCountOut", getValue("ledCycCavCount"));

            setValue("ledPowercellCount", settings.ledPowercellCount || 13); // Haslab: 13
            setToggle("ledInvertPowercell", settings.ledInvertPowercell);
            setValue("ledPowercellHue", convertRange(settings.ledPowercellHue || 160, [1,254], [0,360])); // Default: Blue
            setValue("ledPowercellSat", convertRange(settings.ledPowercellSat || 254, [1,254], [0,100])); // Full Saturation
            setToggle("ledVGPowercell", settings.ledVGPowercell);

            // Update colour preview and value display for hue/saturation sliders.
            updateColour("cycColourPreview", "cycHueOut", "cycSatOut", getValue("ledCycLidHue"), getValue("ledCycLidSat"));
            updateColour("cakeColourPreview", "cakeHueOut", "cakeSatOut", getValue("ledCycCakeHue"), getValue("ledCycCakeSat"));
            updateColour("pcColourPreview", "pcHueOut", "pcSatOut", getValue("ledPowercellHue"), getValue("ledPowercellSat"));
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
        defaultSystemModePack: getInt("defaultSystemModePack"),
        defaultYearThemePack: getInt("defaultYearThemePack") || 1,
        currentYearThemePack: getInt("currentYearThemePack") || 4,
        defaultSystemVolume: getInt("defaultSystemVolume") || 100,
        packVibration: getInt("packVibration") || 4,
        protonStreamEffects: getToggle("protonStreamEffects"),
        ribbonCableAlarm: getToggle("ribbonCableAlarm"),
        overheatStrobeNF: getToggle("overheatStrobeNF"),
        overheatLightsOff: getToggle("overheatLightsOff"),
        overheatSyncToFan: getToggle("overheatSyncToFan"),
        demoLightMode: getToggle("demoLightMode"),

        ledCycLidCount: getInt("ledCycLidCount") || 12,
        ledCycLidHue: convertRange(getInt("ledCycLidHue") || 360, [0,360], [1,254]),
        ledCycLidSat: convertRange(getInt("ledCycLidSat") || 100, [0,100], [1,254]),
        cyclotronDirection: getInt("cyclotronDirection"),
        ledCycLidCenter: getInt("ledCycLidCenter"),
        ledCycLidFade: getToggle("ledCycLidFade"),
        ledVGCyclotron: getToggle("ledVGCyclotron"),
        ledCycLidSimRing: getToggle("ledCycLidSimRing"),

        ledCycCakeCount: getInt("ledCycCakeCount") || 35,
        ledCycCakeHue: convertRange(getInt("ledCycCakeHue") || 360, [0,360], [1,254]),
        ledCycCakeSat: convertRange(getInt("ledCycCakeSat") || 100, [0,100], [1,254]),
        ledCycInnerPanel: getInt("ledCycInnerPanel") || 1,
        ledCycCakeGRB: getToggle("ledCycCakeGRB"),
        ledCycCavCount: getInt("ledCycCavCount"),

        ledPowercellCount: getInt("ledPowercellCount") || 13,
        ledInvertPowercell: getToggle("ledInvertPowercell"),
        ledPowercellHue: convertRange(getInt("ledPowercellHue") || 200, [0,360], [1,254]),
        ledPowercellSat: convertRange(getInt("ledPowercellSat") || 100, [0,100], [1,254]),
        ledVGPowercell: getToggle("ledVGPowercell")
      };
      var body = JSON.stringify(settings);

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          handleStatus(this.responseText);
          getSettings(); // Get latest settings.

          if (confirm("Settings successfully updated. Do you want to store the latest settings to the pack EEPROM?")) {
            saveEEPROM(); // Perform action only if the user answers OK to the confirmation.
          }
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
