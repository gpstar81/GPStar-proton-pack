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

const char SMOKE_SETTINGS_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta http-equiv="Cache-control" content="public">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Smoke Settings</title>
  <link rel="icon" href="data:;base64,iVBORw0KGgo=">
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1 id="top">Smoke Settings</h1>
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
      <b class="labelSwitch">Enable Smoke Effects:</b>
      <label class="switch">
        <input id="smokeEnabled" name="smokeEnabled" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
  </div>

  <h1>Power Level 5</h1>
  <div class="block left">
    <div class="setting">
      <b class="labelSwitch">Overheating Enabled:</b>
      <label class="switch">
        <input id="overheatLevel5" name="overheatLevel5" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Continuous Firing Smoke:</b>
      <label class="switch">
        <input id="overheatContinuous5" name="overheatContinuous5" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b>Overheat Start Delay (Seconds):</b><br/>
      <input type="range" id="overheatDelay5" name="overheatDelay5" min="2" max="60" value="2" step="1"
       oninput="delay5Out.value=overheatDelay5.value"/>
      <output class="labelSlider" id="delay5Out" for="overheatDelay5"></output>
    </div>
    <div class="setting">
      <b>Overheat Duration (Seconds):</b><br/>
      <input type="range" id="overheatDuration5" name="overheatDuration5" min="2" max="60" value="2" step="1"
       oninput="duration5Out.value=overheatDuration5.value"/>
      <output class="labelSlider" id="duration5Out" for="overheatDuration5"></output>
    </div>
  </div>

  <h1>Power Level 4</h1>
  <div class="block left">
    <div class="setting">
    <div class="setting">
      <b class="labelSwitch">Overheating Enabled:</b>
      <label class="switch">
        <input id="overheatLevel4" name="overheatLevel4" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
      <b class="labelSwitch">Continuous Firing Smoke:</b>
      <label class="switch">
        <input id="overheatContinuous4" name="overheatContinuous4" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b>Overheat Start Delay (Seconds):</b><br/>
      <input type="range" id="overheatDelay4" name="overheatDelay4" min="2" max="60" value="2" step="1"
       oninput="delay4Out.value=overheatDelay4.value"/>
      <output class="labelSlider" id="delay4Out" for="overheatDelay4"></output>
    </div>
    <div class="setting">
      <b>Overheat Duration (Seconds):</b><br/>
      <input type="range" id="overheatDuration4" name="overheatDuration4" min="2" max="60" value="2" step="1"
       oninput="duration4Out.value=overheatDuration4.value"/>
      <output class="labelSlider" id="duration4Out" for="overheatDuration4"></output>
    </div>
  </div>

  <h1>Power Level 3</h1>
  <div class="block left">
    <div class="setting">
      <b class="labelSwitch">Overheating Enabled:</b>
      <label class="switch">
        <input id="overheatLevel3" name="overheatLevel3" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Continuous Firing Smoke:</b>
      <label class="switch">
        <input id="overheatContinuous3" name="overheatContinuous3" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b>Overheat Start Delay (Seconds):</b><br/>
      <input type="range" id="overheatDelay3" name="overheatDelay3" min="2" max="60" value="2" step="1"
       oninput="delay3Out.value=overheatDelay3.value"/>
      <output class="labelSlider" id="delay3Out" for="overheatDelay3"></output>
    </div>
    <div class="setting">
      <b>Overheat Duration (Seconds):</b><br/>
      <input type="range" id="overheatDuration3" name="overheatDuration3" min="2" max="60" value="2" step="1"
       oninput="duration3Out.value=overheatDuration3.value"/>
      <output class="labelSlider" id="duration3Out" for="overheatDuration3"></output>
    </div>
  </div>

  <h1>Power Level 2</h1>
  <div class="block left">
    <div class="setting">
      <b class="labelSwitch">Overheating Enabled:</b>
      <label class="switch">
        <input id="overheatLevel2" name="overheatLevel2" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Continuous Firing Smoke:</b>
      <label class="switch">
        <input id="overheatContinuous2" name="overheatContinuous2" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b>Overheat Start Delay (Seconds):</b><br/>
      <input type="range" id="overheatDelay2" name="overheatDelay2" min="2" max="60" value="2" step="1"
       oninput="delay2Out.value=overheatDelay2.value"/>
      <output class="labelSlider" id="delay2Out" for="overheatDelay2"></output>
    </div>
    <div class="setting">
      <b>Overheat Duration (Seconds):</b><br/>
      <input type="range" id="overheatDuration2" name="overheatDuration2" min="2" max="60" value="2" step="1"
       oninput="duration2Out.value=overheatDuration2.value"/>
      <output class="labelSlider" id="duration2Out" for="overheatDuration2"></output>
    </div>
  </div>

  <h1>Power Level 1</h1>
  <div class="block left">
    <div class="setting">
      <b class="labelSwitch">Overheating Enabled:</b>
      <label class="switch">
        <input id="overheatLevel1" name="overheatLevel1" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b class="labelSwitch">Continuous Firing Smoke:</b>
      <label class="switch">
        <input id="overheatContinuous1" name="overheatContinuous1" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
      <b>Overheat Start Delay (Seconds):</b><br/>
      <input type="range" id="overheatDelay1" name="overheatDelay1" min="2" max="60" value="2" step="1"
       oninput="delay1Out.value=overheatDelay1.value"/>
      <output class="labelSlider" id="delay1Out" for="overheatDelay1"></output>
    </div>
    <div class="setting">
      <b>Overheat Duration (Seconds):</b><br/>
      <input type="range" id="overheatDuration1" name="overheatDuration1" min="2" max="60" value="2" step="1"
       oninput="duration1Out.value=overheatDuration1.value"/>
      <output class="labelSlider" id="duration1Out" for="overheatDuration1"></output>
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

  <script type="application/javascript">
    window.addEventListener("load", onLoad);

    function onLoad(event) {
      // Wait 0.5s for serial communications between devices.
      setTimeout(getSettings, 500);
    }

    function getEl(id){
      return document.getElementById(id);
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

            getEl("smokeEnabled").checked = settings.smokeEnabled ? true: false;

            getEl("overheatContinuous5").checked = settings.overheatContinuous5 || 0;
            getEl("overheatLevel5").checked = settings.overheatLevel5 || 0;
            getEl("overheatDelay5").value = settings.overheatDelay5 || 2;
            getEl("delay5Out").innerHTML = getEl("overheatDelay5").value;
            getEl("overheatDuration5").value = settings.overheatDuration5 || 2;
            getEl("duration5Out").innerHTML = getEl("overheatDuration5").value;

            getEl("overheatContinuous4").checked = settings.overheatContinuous4 || 0;
            getEl("overheatLevel4").checked = settings.overheatLevel4 || 0;
            getEl("overheatDelay4").value = settings.overheatDelay4 || 2;
            getEl("delay4Out").innerHTML = getEl("overheatDelay4").value;
            getEl("overheatDuration4").value = settings.overheatDuration4 || 2;
            getEl("duration4Out").innerHTML = getEl("overheatDuration4").value;

            getEl("overheatContinuous3").checked = settings.overheatContinuous3 || 0;
            getEl("overheatLevel3").checked = settings.overheatLevel3 || 0;
            getEl("overheatDelay3").value = settings.overheatDelay3 || 2;
            getEl("delay3Out").innerHTML = getEl("overheatDelay3").value;
            getEl("overheatDuration3").value = settings.overheatDuration3 || 2;
            getEl("duration3Out").innerHTML = getEl("overheatDuration3").value;

            getEl("overheatContinuous2").checked = settings.overheatContinuous2 || 0;
            getEl("overheatLevel2").checked = settings.overheatLevel2 || 0;
            getEl("overheatDelay2").value = settings.overheatDelay2 || 2;
            getEl("delay2Out").innerHTML = getEl("overheatDelay2").value;
            getEl("overheatDuration2").value = settings.overheatDuration2 || 2;
            getEl("duration2Out").innerHTML = getEl("overheatDuration2").value;

            getEl("overheatContinuous1").checked = settings.overheatContinuous1 || 0;
            getEl("overheatLevel1").checked = settings.overheatLevel1 || 0;
            getEl("overheatDelay1").value = settings.overheatDelay1 || 2;
            getEl("delay1Out").innerHTML = getEl("overheatDelay1").value;
            getEl("overheatDuration1").value = settings.overheatDuration1 || 2;
            getEl("duration1Out").innerHTML = getEl("overheatDuration1").value;
          }
        }
      };
      xhttp.open("GET", "/config/smoke", true);
      xhttp.send();
    }

    function saveSettings() {
      // Saves current settings to pack/wand, updating runtime variables and making changes immediately effective.
      // This does NOT save to the EEPROM automatically as the user is encouraged to test prior to that action.
      var settings = {
        smokeEnabled: getEl("smokeEnabled").checked ? 1 : 0,
        overheatDuration5: parseInt(getEl("overheatDuration5").value || 2, 10),
        overheatDuration4: parseInt(getEl("overheatDuration4").value || 2, 10),
        overheatDuration3: parseInt(getEl("overheatDuration3").value || 2, 10),
        overheatDuration2: parseInt(getEl("overheatDuration2").value || 2, 10),
        overheatDuration1: parseInt(getEl("overheatDuration1").value || 2, 10),
        overheatContinuous5: getEl("overheatContinuous5").checked ? 1 : 0,
        overheatContinuous4: getEl("overheatContinuous4").checked ? 1 : 0,
        overheatContinuous3: getEl("overheatContinuous3").checked ? 1 : 0,
        overheatContinuous2: getEl("overheatContinuous2").checked ? 1 : 0,
        overheatContinuous1: getEl("overheatContinuous1").checked ? 1 : 0,
        overheatLevel5: getEl("overheatLevel5").checked ? 1 : 0,
        overheatLevel4: getEl("overheatLevel4").checked ? 1 : 0,
        overheatLevel3: getEl("overheatLevel3").checked ? 1 : 0,
        overheatLevel2: getEl("overheatLevel2").checked ? 1 : 0,
        overheatLevel1: getEl("overheatLevel1").checked ? 1 : 0,
        overheatDelay5: parseInt(getEl("overheatDelay5").value || 2, 10),
        overheatDelay4: parseInt(getEl("overheatDelay4").value || 2, 10),
        overheatDelay3: parseInt(getEl("overheatDelay3").value || 2, 10),
        overheatDelay2: parseInt(getEl("overheatDelay2").value || 2, 10),
        overheatDelay1: parseInt(getEl("overheatDelay1").value || 2, 10)
      };
      var body = JSON.stringify(settings);

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          handleStatus(this.responseText);
          getSettings(); // Get latest settings.

          if (confirm("Settings successfully updated. Do you want to store the latest settings to the pack/wand EEPROM?")) {
            saveEEPROM(); // Perform action only if the user answers OK to the confirmation.
          }
        }
      };
      xhttp.open("PUT", "/config/smoke/save", true);
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
      xhttp.open("PUT", "/eeprom/all", true);
      xhttp.send();
    }
  </script>
</body>
</html>
)=====";
