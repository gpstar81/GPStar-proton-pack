/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
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

const char SMOKE_SETTINGS_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta http-equiv="Cache-control" content="public">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Smoke Settings</title>
  <link rel="icon" type="image/svg+xml" href="/favicon.svg"/>
  <link rel="shortcut icon" href="/favicon.ico"/>
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
      <b class="labelSwitch">Overheating Enabled:</b>
      <label class="switch">
        <input id="overheatLevel4" name="overheatLevel4" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    <div class="setting">
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
    <button type="button" class="green" style="width:120px" onclick="saveSettings()" id="btnSave">Update&nbsp;Settings</button>
    <br/>
    <br/>
  </div>

  <script type="application/javascript" src="/common.js"></script>
  <script type="application/javascript">
    window.addEventListener("load", onLoad);

    function onLoad(event) {
      // Wait 0.5s for serial communications between devices.
      setTimeout(getSettings, 500);

      // Disable the save button until we obtain settings.
      getEl("btnSave").disabled = true;
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

			if (!settings.wandConnected) {
              getEl("overheatLevel1").disabled = true;
              getEl("overheatLevel2").disabled = true;
              getEl("overheatLevel3").disabled = true;
              getEl("overheatLevel4").disabled = true;
              getEl("overheatLevel5").disabled = true;
              getEl("overheatDelay1").disabled = true;
              getEl("overheatDelay2").disabled = true;
              getEl("overheatDelay3").disabled = true;
              getEl("overheatDelay4").disabled = true;
              getEl("overheatDelay5").disabled = true;
              alert("GPStar Neutrona Wand not detected; only partial smoke settings are available.");
            }
			else {
              getEl("overheatLevel1").disabled = false;
              getEl("overheatLevel2").disabled = false;
              getEl("overheatLevel3").disabled = false;
              getEl("overheatLevel4").disabled = false;
              getEl("overheatLevel5").disabled = false;
              getEl("overheatDelay1").disabled = false;
              getEl("overheatDelay2").disabled = false;
              getEl("overheatDelay3").disabled = false;
              getEl("overheatDelay4").disabled = false;
              getEl("overheatDelay5").disabled = false;
            }

            // Valid settings were received and both the pack and wand are off, so allow updating settings.
            getEl("btnSave").disabled = false;

            setToggle("smokeEnabled", settings.smokeEnabled);

            setToggle("overheatContinuous5", settings.overheatContinuous5 || 0);
            setToggle("overheatLevel5", settings.overheatLevel5 || 0);
            setValue("overheatDelay5", settings.overheatDelay5 || 30);
            setHtml("delay5Out", getText("overheatDelay5"));
            setValue("overheatDuration5", settings.overheatDuration5 || 6);
            setHtml("duration5Out", getText("overheatDuration5"));

            setToggle("overheatContinuous4", settings.overheatContinuous4 || 0);
            setToggle("overheatLevel4", settings.overheatLevel4 || 0);
            setValue("overheatDelay4", settings.overheatDelay4 || 35);
            setHtml("delay4Out", getText("overheatDelay4"));
            setValue("overheatDuration4", settings.overheatDuration4 || 5);
            setHtml("duration4Out", getText("overheatDuration4"));

            setToggle("overheatContinuous3", settings.overheatContinuous3 || 0);
            setToggle("overheatLevel3", settings.overheatLevel3 || 0);
            setValue("overheatDelay3", settings.overheatDelay3 || 40);
            setHtml("delay3Out", getText("overheatDelay3"));
            setValue("overheatDuration3", settings.overheatDuration3 || 4);
            setHtml("duration3Out", getText("overheatDuration3"));

            setToggle("overheatContinuous2", settings.overheatContinuous2 || 0);
            setToggle("overheatLevel2", settings.overheatLevel2 || 0);
            setValue("overheatDelay2", settings.overheatDelay2 || 50);
            setHtml("delay2Out", getText("overheatDelay2"));
            setValue("overheatDuration2", settings.overheatDuration2 || 3);
            setHtml("duration2Out", getText("overheatDuration2"));

            setToggle("overheatContinuous1", settings.overheatContinuous1 || 0);
            setToggle("overheatLevel1", settings.overheatLevel1 || 0);
            setValue("overheatDelay1", settings.overheatDelay1 || 60);
            setHtml("delay1Out", getText("overheatDelay1"));
            setValue("overheatDuration1", settings.overheatDuration1 || 2);
            setHtml("duration1Out", getText("overheatDuration1"));
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
        smokeEnabled: getToggle("smokeEnabled"),
        overheatDuration5: getInt("overheatDuration5") || 6,
        overheatDuration4: getInt("overheatDuration4") || 5,
        overheatDuration3: getInt("overheatDuration3") || 4,
        overheatDuration2: getInt("overheatDuration2") || 3,
        overheatDuration1: getInt("overheatDuration1") || 2,
        overheatContinuous5: getToggle("overheatContinuous5"),
        overheatContinuous4: getToggle("overheatContinuous4"),
        overheatContinuous3: getToggle("overheatContinuous3"),
        overheatContinuous2: getToggle("overheatContinuous2"),
        overheatContinuous1: getToggle("overheatContinuous1"),
        overheatLevel5: getToggle("overheatLevel5"),
        overheatLevel4: getToggle("overheatLevel4"),
        overheatLevel3: getToggle("overheatLevel3"),
        overheatLevel2: getToggle("overheatLevel2"),
        overheatLevel1: getToggle("overheatLevel1"),
        overheatDelay5: getInt("overheatDelay5") || 30,
        overheatDelay4: getInt("overheatDelay4") || 35,
        overheatDelay3: getInt("overheatDelay3") || 40,
        overheatDelay2: getInt("overheatDelay2") || 50,
        overheatDelay1: getInt("overheatDelay1") || 60
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
