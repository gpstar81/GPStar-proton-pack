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
  <h1>Proton Pack Settings</h1>
  <div class="block">
    <p>
      Change system configuration options using the available toggles/selectors.
      Options can only be changed when the pack and wand are not powered/running!
    </p>
    <br/>
  </div>

  <h1>General Options</h1>
  <div class="block left">
    <div class="setting">
      <b>System Mode:</b>
      <select id="defaultSystemModePack" name="defaultSystemModePack">
        <option value="0">Super Hero</option>
        <option value="1">Mode Original</option>
      </select>
    </div>
    <div class="setting">
      <b>Theme Year:</b>
      <select id="defaultYearThemePack" name="defaultYearThemePack">
        <option value="1">System Toggle</option>
        <option value="2">1984</option>
        <option value="3">1989</option>
        <option value="4">Afterlife</option>
        <option value="5">Frozen Empire</option>
      </select>
    </div>
    <div class="setting">
      <b class="labelSlider">Startup Volume %:</b>
      <input type="range" id="defaultSystemVolume" name="defaultSystemVolume" min="0" max="100" value="100" step="2" oninput="masterVolOut.value=defaultSystemVolume.value"/>
      <output class="labelSlider" id="masterVolOut" for="defaultSystemVolume">100</output>
    </div>
    <div class="setting">
      <b class="labelSwitch">Proton Stream Impact Effects:</b>
      <label class="switch">
        <input id="protonStreamEffects" name="protonStreamEffects" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Enable Smoke Effects:</b>
      <label class="switch">
        <input id="smokeEnabled" name="smokeEnabled" type="checkbox">
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
      <b class="labelSwitch">Enable Demo Light Mode:</b>
      <label class="switch">
        <input id="demoLightMode" name="demoLightMode" type="checkbox">
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
      <b class="labelSlider">Custom Color (Hue):</b>
      <input type="range" id="ledCycLidHue" name="ledCycLidHue" min="0" max="360" value="360" step="2" oninput="cycHueOut.value=ledCycLidHue.value"/>
      <output class="labelSlider" id="cycHueOut" for="ledCycLidHue">360</output>
    </div>
    <div class="setting">
      <b class="labelSlider">Custom Saturation %:</b>
      <input type="range" id="ledCycLidSat" name="ledCycLidSat" min="0" max="100" value="100" step="2" oninput="cycSatOut.value=ledCycLidSat.value"/>
      <output class="labelSlider" id="cycSatOut" for="ledCycLidSat">100</output>
    </div>
    <div class="setting">
      <b>Direction:</b>
      <select id="cyclotronDirection" name="cyclotronDirection">
        <option value="0">Counter-Clockwise</option>
        <option value="1">Clockwise</option>
      </select>
    </div>
    <div class="setting">
      <b>Center LEDs:</b>
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
        <option value="12">12</option>
        <option value="23">23</option>
        <option value="24">24</option>
        <option value="35">35</option>
      </select>
    </div>
    <div class="setting">
      <b class="labelSlider">Custom Color (Hue):</b>
      <input type="range" id="ledCycCakeHue" name="ledCycCakeHue" min="0" max="360" value="360" step="2" oninput="cakeHueOut.value=ledCycCakeHue.value"/>
      <output class="labelSlider" id="cakeHueOut" for="ledCycCakeHue">360</output>
    </div>
    <div class="setting">
      <b class="labelSlider">Custom Saturation %:</b>
      <input type="range" id="ledCycCakeSat" name="ledCycCakeSat" min="0" max="100" value="100" step="2" oninput="cakeSatOut.value=ledCycCakeSat.value"/>
      <output class="labelSlider" id="cakeSatOut" for="ledCycCakeSat">100</output>
    </div>
      <div class="setting">
      <b class="labelSwitch">Swap Red/Green LEDs (GRB):</b>
      <label class="switch">
        <input id="ledCycCakeGRB" name="ledCycCakeGRB" type="checkbox">
        <span class="slider round"></span>
      </label>
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
      <b class="labelSlider">Custom Color (Hue):</b>
      <input type="range" id="ledPowercellHue" name="ledPowercellHue" min="0" max="360" value="360" step="2" oninput="pcHueOut.value=ledPowercellHue.value"/>
      <output class="labelSlider" id="pcHueOut" for="ledPowercellHue">360</output>
    </div>
    <div class="setting">
      <b class="labelSlider">Custom Saturation %:</b>
      <input type="range" id="ledPowercellSat" name="ledPowercellSat" min="0" max="100" value="100" step="2" oninput="pcSatOut.value=ledPowercellSat.value"/>
      <output class="labelSlider" id="pcSatOut" for="ledPowercellSat">100</output>
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
    &nbsp;&nbsp;&nbsp;
    <button type="button" class="green" onclick="saveSettings()">Save</button>
  </div>

  <script type="application/javascript">
    window.addEventListener("load", onLoad);

    function onLoad(event) {
      getSettings();
    }

    // Converts a value from one range to another: eg. convertRange(160, [2,254], [0,360])
    function convertRange(value, r1, r2) { 
      return Math.round((value - r1[0]) * (r2[1] - r2[0]) / (r1[1] - r1[0]) + r2[0]);
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

            /**
             * Note: Color (hue) value range uses the following scale, though HTML5 uses 0-360 for HSL color.
             *  0 = Red
             *  32 = Orange
             *  64 = Yellow
             *  96 = Green
             *  128 = Aqua
             *  160 = Blue
             *  160 = Blue
             *  192 = Purple
             *  224 = Pink
             *  254 = Red
             */

            // Update fields with the current values, or supply an expected default as necessary.
            document.getElementById("defaultSystemModePack").value = settings.defaultSystemModePack || 0;
            document.getElementById("defaultYearThemePack").value = settings.defaultYearThemePack || 1; // Value cannot be 0.
            document.getElementById("defaultSystemVolume").value = settings.defaultSystemVolume || 100; // Default to full volume.
            document.getElementById("masterVolOut").innerHTML = document.getElementById("defaultSystemVolume").value;
            document.getElementById("protonStreamEffects").value = settings.protonStreamEffects || 0;
            document.getElementById("smokeEnabled").checked = settings.smokeEnabled ? true: false;
            document.getElementById("overheatStrobeNF").checked = settings.overheatStrobeNF ? true: false;
            document.getElementById("overheatLightsOff").checked = settings.overheatLightsOff ? true: false;
            document.getElementById("overheatSyncToFan").checked = settings.overheatSyncToFan ? true: false;
            document.getElementById("demoLightMode").checked = settings.demoLightMode ? true: false;
            document.getElementById("ledCycLidCount").value = settings.ledCycLidCount || 12; // Haslab: 12
            document.getElementById("ledCycLidHue").value = convertRange(settings.ledCycLidHue || 254, [1,254], [0,360]); // Default: Red
            document.getElementById("cycHueOut").innerHTML = document.getElementById("ledCycLidHue").value;
            document.getElementById("ledCycLidSat").value = convertRange(settings.ledCycLidSat || 254, [1,254], [0,100]); // Full Saturation
            document.getElementById("cycSatOut").innerHTML = document.getElementById("ledCycLidSat").value;
            document.getElementById("cyclotronDirection").value = settings.cyclotronDirection || 0;
            document.getElementById("ledCycLidCenter").value = settings.ledCycLidCenter || 0;
            document.getElementById("ledVGCyclotron").checked = settings.ledVGCyclotron ? true: false;
            document.getElementById("ledCycLidSimRing").checked = settings.ledCycLidSimRing ? true: false;
            document.getElementById("ledCycCakeCount").value = settings.ledCycCakeCount || 35; // Default: 12
            document.getElementById("ledCycCakeHue").value = convertRange(settings.ledCycCakeHue || 254, [1,254], [0,360]); // Default: Red
            document.getElementById("cakeHueOut").innerHTML = document.getElementById("ledCycCakeHue").value;
            document.getElementById("ledCycCakeSat").value = convertRange(settings.ledCycCakeSat || 254, [1,254], [0,100]); // Full Saturation
            document.getElementById("cakeSatOut").innerHTML = document.getElementById("ledCycCakeSat").value;
            document.getElementById("ledCycCakeGRB").checked = settings.ledCycCakeGRB ? true: false;
            document.getElementById("ledPowercellCount").value = settings.ledPowercellCount || 13; // Haslab: 13
            document.getElementById("ledPowercellHue").value = convertRange(settings.ledPowercellHue || 160, [1,254], [0,360]); // Default: Blue
            document.getElementById("pcHueOut").innerHTML = document.getElementById("ledPowercellHue").value;
            document.getElementById("ledPowercellSat").value = convertRange(settings.ledPowercellSat || 254, [1,254], [0,100]); // Full Saturation
            document.getElementById("pcSatOut").innerHTML = document.getElementById("ledPowercellSat").value;
            document.getElementById("ledVGPowercell").checked = settings.ledVGPowercell ? true: false;
          }
        }
      };
      xhttp.open("GET", "/config/pack", true);
      xhttp.send();
    }

    function saveSettings() {
      var settings = {
        defaultSystemModePack: parseInt(document.getElementById("defaultSystemModePack").value || 0, 10),
        defaultYearThemePack: parseInt(document.getElementById("defaultYearThemePack").value || 1, 10),
        defaultSystemVolume: parseInt(document.getElementById("defaultSystemVolume").value || 0, 10),
        protonStreamEffects: document.getElementById("protonStreamEffects").checked ? 1 : 0,
        smokeEnabled: document.getElementById("smokeEnabled").checked ? 1 : 0,
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
        ledPowercellCount: parseInt(document.getElementById("ledPowercellCount").value || 13, 10),
        ledPowercellHue: convertRange(parseInt(document.getElementById("ledPowercellHue").value || 200, 10), [0,360], [1,254]),
        ledPowercellSat: convertRange(parseInt(document.getElementById("ledPowercellSat").value || 100, 10), [0,100], [1,254]),
        ledVGPowercell: document.getElementById("ledVGPowercell").checked ? 1 : 0
      };
      var body = JSON.stringify(settings);

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var jObj = JSON.parse(this.responseText);
          alert(jObj.status); // Always display status returned.
        }
      };
      xhttp.open("PUT", "/config/pack/save", true);
      xhttp.setRequestHeader("Content-Type", "application/json");
      xhttp.send(body);
    }
  </script>
</body>
</html>
)=====";