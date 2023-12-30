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
      <br/>
      Options can only be changed when the pack and wand are not powered/running!
    </p>
    <br/>

    <h1>General Options</h1>
    <div class="block left">
      <b>System Mode:</b>
      <select id="defaultSystemModePack" name="defaultSystemModePack">
        <option value="0">Super Hero</option>
        <option value="1">Mode Original</option>
      </select>
      </br>
      <b>Theme Year:</b>
      <select id="defaultYearThemePack" name="defaultYearThemePack">
        <option value="1">System Toggle</option>
        <option value="2">1984</option>
        <option value="3">1989</option>
        <option value="4">Afterlife</option>
        <option value="5">Frozen Empire</option>
      </select>
      </br>
      <b>Default Master Volume:</b>
      <input type="range" id="defaultSystemVolume" name="defaultSystemVolume" min="0" max="100" value="100" step="2" oninput="masterVolOut.value=defaultSystemVolume.value"/>
      <output id="masterVolOut" for="defaultSystemVolume">100</output>
      </br>
      <b>Proton Stream Impact Effects:</b>
      <label class="switch">
        <input id="protonStreamEffects" name="protonStreamEffects" type="checkbox">
        <span class="slider round"></span>
      </label>
      </br>
      <b>Smoke Effects:</b>
      <label class="switch">
        <input id="smokeEnabled" name="smokeEnabled" type="checkbox">
        <span class="slider round"></span>
      </label>
      </br>
      <b>Strobe N-Filter on Overheat:</b>
      <label class="switch">
        <input id="overheatStrobeNF" name="overheatStrobeNF" type="checkbox">
        <span class="slider round"></span>
      </label>
      </br>
      <b>Lights Off During Overheat:</b>
      <label class="switch">
        <input id="overheatLightsOff" name="overheatLightsOff" type="checkbox">
        <span class="slider round"></span>
      </label>
      </br>
      <b>Overheat Sync to Fan:</b>
      <label class="switch">
        <input id="overheatSyncToFan" name="overheatSyncToFan" type="checkbox">
        <span class="slider round"></span>
      </label>
      </br>
      <b>Demo Light Mode:</b>
      <label class="switch">
        <input id="demoLightMode" name="demoLightMode" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>

    <h1>Cyclotron Lid</h1>
    <div class="block left">
      <b>LED Count:</b>
      <select id="ledCycLidCount" name="ledCycLidCount">
        <option value="12">12 - Stock</option>
        <option value="20">20 - Frutto</option>
        <option value="40">40 - Ring</option>
      </select>
      <br/>
      <b>Spectral Custom Color (Hue):</b>
      <input type="range" id="ledCycLidHue" name="ledCycLidHue" min="2" max="254" value="254" step="2" oninput="cycHueOut.value=ledCycLidHue.value"/>
      <output id="cycHueOut" for="ledCycLidHue">254</output>
      <br/>
      <b>Spectral Custom Saturation:</b>
      <input type="range" id="ledCycLidSat" name="ledCycLidSat" min="2" max="254" value="254" step="2" oninput="cycSatOut.value=ledCycLidSat.value"/>
      <output id="cycSatOut" for="ledCycLidSat">254</output>
      <br/>
      <b>Direction:</b>
      <select id="cyclotronDirection" name="cyclotronDirection">
        <option value="0">Counter-Clockwise</option>
        <option value="1">Clockwise</option>
      </select>
      </br>
      <b>Center LEDs:</b>
      <select id="ledCycLidCenter" name="ledCycLidCenter">
        <option value="0">3 LED</option>
        <option value="1">1 LED</option>
      </select>
      </br>
      <b>Enable Video Game Colors:</b>
      <label class="switch">
        <input id="ledVGCyclotron" name="ledVGCyclotron" type="checkbox">
        <span class="slider round"></span>
      </label>
      <br/>
      <b>Simulate Ring:</b>
      <label class="switch">
        <input id="ledCycLidSimRing" name="ledCycLidSimRing" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>

    <h1>Cyclotron Cake</h1>
    <div class="block left">
      <b>LED Count:</b>
      <select id="ledCycCakeCount" name="ledCycCakeCount">
        <option value="12">12</option>
        <option value="23">23</option>
        <option value="24">24</option>
        <option value="35">35</option>
      </select>
      <br/>
      <b>Spectral Custom Color (Hue):</b>
      <input type="range" id="ledCycCakeHue" name="ledCycCakeHue" min="2" max="254" value="254" step="2" oninput="cakeHueOut.value=ledCycCakeHue.value"/>
      <output id="cakeHueOut" for="ledCycCakeHue">254</output>
      <br/>
      <b>Spectral Custom Saturation:</b>
      <input type="range" id="ledCycCakeSat" name="ledCycCakeSat" min="2" max="254" value="254" step="2" oninput="cakeSatOut.value=ledCycCakeSat.value"/>
      <output id="cakeSatOut" for="ledCycCakeSat">254</output>
      <br/>
      <b>Swap Red/Green LEDs (GRB):</b>
      <label class="switch">
        <input id="ledCycCakeGRB" name="ledCycCakeGRB" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>

    <h1>Power Cell</h1>
    <div class="block left">
      <b>LED Count:</b>
      <select id="ledPowercellCount" name="ledPowercellCount">
        <option value="13">13 - Stock</option>
        <option value="15">15 - Frutto</option>
      </select>
      <br/>
      <b>Spectral Custom Color (Hue):</b>
      <input type="range" id="ledPowercellHue" name="ledPowercellHue" min="2" max="254" value="254" step="2" oninput="pcHueOut.value=ledPowercellHue.value"/>
      <output id="pcHueOut" for="ledPowercellHue">254</output>
      <br/>
      <b>Spectral Custom Saturation:</b>
      <input type="range" id="ledPowercellSat" name="ledPowercellSat" min="2" max="254" value="254" step="2" oninput="pcSatOut.value=ledPowercellSat.value"/>
      <output id="pcSatOut" for="ledPowercellSat">254</output>
      </br>
      <b>Enable Video Game Colors:</b>
      <label class="switch">
        <input id="ledVGPowercell" name="ledVGPowercell" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>

    <br/>
    <a href="/">&laquo; Back</a>
    &nbsp;&nbsp;&nbsp;
    <button type="button" class="green" onclick="saveSettings()">Save</button>
  </div>

  <script type="application/javascript">
    window.addEventListener("load", onLoad);

    function onLoad(event) {
      getSettings();
    }

    function getSettings() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var settings = JSON.parse(this.responseText);
          if (settings) {
            console.log(settings);

            if (settings.packPowered || settings.wandPowered) {
              alert("Pack and/or Wand are currently powered up. Changes to settings will not be allowed. Turn off system via toggle switches and reload the page to obtain the latest settings.");
            }

            /**
             * Note: Color (hue) value range uses the following scale:
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
            document.getElementById("defaultSystemVolume").value = settings.defaultSystemVolume || 100; // Use full volume.
            document.getElementById("protonStreamEffects").value = settings.protonStreamEffects || 0;
            document.getElementById("smokeEnabled").value = settings.smokeEnabled || 0;
            document.getElementById("overheatStrobeNF").value = settings.overheatStrobeNF || 0;
            document.getElementById("overheatLightsOff").value = settings.overheatLightsOff || 0;
            document.getElementById("overheatSyncToFan").value = settings.overheatSyncToFan || 0;
            document.getElementById("demoLightMode").value = settings.demoLightMode || 0;
            document.getElementById("ledCycLidCount").value = settings.ledCycLidCount || 12; // Haslab: 12
            document.getElementById("ledCycLidHue").value = settings.ledCycLidHue || 254; // Default: Red
            document.getElementById("ledCycLidSat").value = settings.ledCycLidSat || 254; // Full Saturation
            document.getElementById("cyclotronDirection").value = settings.cyclotronDirection || 0;
            document.getElementById("ledCycLidCenter").value = settings.ledCycLidCenter || 0;
            document.getElementById("ledVGCyclotron").value = settings.ledVGCyclotron || 0;
            document.getElementById("ledCycLidSimRing").value = settings.ledCycLidSimRing || 0;
            document.getElementById("ledCycCakeCount").value = settings.ledCycCakeCount || 35; // Default: 12
            document.getElementById("ledCycCakeHue").value = settings.ledCycCakeHue || 254; // Default: Red
            document.getElementById("ledCycCakeSat").value = settings.ledCycCakeSat || 254; // Full Saturation
            document.getElementById("ledCycCakeGRB").value = settings.ledCycCakeGRB || 0;
            document.getElementById("ledPowercellCount").value = settings.ledPowercellCount || 13; // Haslab: 13
            document.getElementById("ledPowercellHue").value = settings.ledPowercellHue || 160; // Default: Blue
            document.getElementById("ledPowercellSat").value = settings.ledPowercellSat || 254; // Full Saturation
            document.getElementById("ledVGPowercell").value = settings.ledVGPowercell || 0;
          }
        }
      };
      xhttp.open("GET", "/config/pack", true);
      xhttp.send();
    }

    function saveSettings() {
      var settings = {
        defaultSystemModePack: document.getElementById("defaultSystemModePack").value || 0,
        defaultYearThemePack: document.getElementById("defaultYearThemePack").value || 1,
        defaultSystemVolume: document.getElementById("defaultSystemVolume").value || 0,
        protonStreamEffects: document.getElementById("protonStreamEffects").value || 0,
        smokeEnabled: document.getElementById("smokeEnabled").value || 0,
        overheatStrobeNF: document.getElementById("overheatStrobeNF").value || 0,
        overheatLightsOff: document.getElementById("overheatLightsOff").value || 0,
        overheatSyncToFan: document.getElementById("overheatSyncToFan").value || 0,
        demoLightMode: document.getElementById("demoLightMode").value || 0,
        ledCycLidCount: document.getElementById("ledCycLidCount").value || 12,
        ledCycLidHue: document.getElementById("ledCycLidHue").value || 254,
        ledCycLidSat: document.getElementById("ledCycLidSat").value || 254,
        cyclotronDirection: document.getElementById("cyclotronDirection").value || 0,
        ledCycLidCenter: document.getElementById("ledCycLidCenter").value || 0,
        ledVGCyclotron: document.getElementById("ledVGCyclotron").value || 0,
        ledCycLidSimRing: document.getElementById("ledCycLidSimRing").value || 0,
        ledCycCakeCount: document.getElementById("ledCycCakeCount").value || 35,
        ledCycCakeHue: document.getElementById("ledCycCakeHue").value || 254,
        ledCycCakeSat: document.getElementById("ledCycCakeSat").value || 254,
        ledCycCakeGRB: document.getElementById("ledCycCakeGRB").value || 0,
        ledPowercellCount: document.getElementById("ledPowercellCount").value || 13,
        ledPowercellHue: document.getElementById("ledPowercellHue").value || 160,
        ledPowercellSat: document.getElementById("ledPowercellSat").value || 254,
        ledVGPowercell: document.getElementById("ledVGPowercell").value || 0
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