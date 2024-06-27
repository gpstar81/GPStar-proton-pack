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

const char INDEX_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta http-equiv="Cache-control" content="public">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <meta name="apple-mobile-web-app-capable" content="yes"/>
  <title>Proton Pack</title>
  <link rel="icon" href="data:;base64,iVBORw0KGgo=">
  <link rel="stylesheet" href="/style.css">
</head>
<body class="dark">
  <div class="tabs dark">
    <button class="tablinks" onclick="openTab(event, 'tab1')">&#9883;</button>
    <button class="tablinks" onclick="openTab(event, 'tab2')">&#9836;</button>
    <button class="tablinks" onclick="openTab(event, 'tab3')">&#9211;</button>
    <button class="tablinks" onclick="openTab(event, 'tab4')">&#9881;</button>
  </div>

  <div id="tab1" class="tab">
    <div id="equipCRT" class="equipment">
      <div id="equipTitle" class="infoState equip-title centered"></div>
      <div id="ionOverlay" class="overlay ion-switch"></div>
      <div id="boostOverlay" class="overlay booster-box"></div>
      <div id="pcellOverlay" class="overlay power-box"></div>
      <div id="cableOverlay" class="overlay cable-warn">&#9888;</div>
      <div id="cycOverlay" class="overlay cyc-circle"></div>
      <div id="filterOverlay" class="overlay filter-circle"></div>
      <div id="barrelOverlay" class="overlay barrel-box"></div>
      <div id="powerLevel" class="overlay infoState power-title"></div>
      <div id="streamMode" class="overlay infoState stream-title"></div>
      <div id="safetyOverlay" class="overlay safety-box"></div>
      <div id="battOutput" class="overlay infoState batt-title"></div>
      <div id="cyclotronLid" class="infoState rad-warn">
        <span style="font-size:1.2em">&#9762;</span> Cyclotron Exposure Warning
      </div>
    </div>

    <div id="equipTXT" class="card">
      <p><span class="infoLabel">Operation Mode:</span> <span class="infoState" id="mode">&mdash;</span></p>
      <p><span class="infoLabel">Effects Theme:</span> <span class="infoState" id="theme">&mdash;</span></p>
      <br/>
      <p><span class="infoLabel">Pack State:</span> <span class="infoState" id="pack">&mdash;</span></p>
      <p><span class="infoLabel">Pack Armed:</span> <span class="infoState" id="switch">&mdash;</span></p>
      <p><span class="infoLabel">Ribbon Cable:</span> <span class="infoState" id="cable">&mdash;</span></p>
      <p><span class="infoLabel">Cyclotron State:</span> <span class="infoState" id="cyclotron">&mdash;</span></p>
      <p><span class="infoLabel">Overheat State:</span> <span class="infoState" id="temperature">&mdash;</span></p>
      <br/>
      <p><span class="infoLabel">Wand Presence:</span> <span class="infoState" id="wand">&mdash;</span></p>
      <p><span class="infoLabel">Wand State:</span> <span class="infoState" id="wandPower">&mdash;</span></p>
      <p><span class="infoLabel">Wand Armed:</span> <span class="infoState" id="safety">&mdash;</span></p>
      <p><span class="infoLabel">System Mode:</span> <span class="infoState" id="wandMode">&mdash;</span></p>
      <div style="display:inline-flex;margin:0;">
        <p style="margin:0;">
          <span class="infoLabel">Power Level:</span>&nbsp;<span class="infoState" id="power">&mdash;</span>
        </p>
        <div class="bar-container" id="powerBars"></div>
      </div>
      <p><span class="infoLabel">Firing State:</span> <span class="infoState" id="firing">&mdash;</span></p>
      <br/>
      <p>
        <span class="infoLabel">Powercell:</span>
        <span id="battHealth"></span>
        <span class="infoState" id="battVoltage">&mdash;</span>
        <span style="font-size: 0.8em">GeV</span>
      </p>
    </div>
  </div>

  <div id="tab2" class="tab">
    <div>
      <div class="volume-container">
        <div class="volume-control">
          <h3>System</h3>
          <button type="button" onclick="volSysUp()">+</button>
          <span id="masterVolume"></span>
          <button type="button" onclick="volSysDown()">&minus;</button>
        </div>
        <div class="volume-control">
          <h3>Effects</h3>
          <button type="button" onclick="volFxUp()">+</button>
          <span id="effectsVolume"></span>
          <button type="button" onclick="volFxDown()">&minus;</button>
        </div>
        <div class="volume-control">
          <h3>Music</h3>
          <button type="button" onclick="volMusicUp()">+</button>
          <span id="musicVolume"></span>
          <button type="button" onclick="volMusicDown()">&minus;</button>
        </div>
      </div>
      <button type="button" class="orange" onclick="toggleMute()">Mute/Unmute</button>
    </div>
    <div class="card">
      <h3 class="centered">Music Navigation</h3>
      <div class="music-navigation">
        <button type="button" onclick="musicPrev()" title="Previous Track">&#9664;&#9664;</button>
        <button type="button" onclick="musicStartStop()" title="Start/Stop">&#9634;&nbsp;&#9654;</button>
        <button type="button" onclick="musicPauseResume()" title="Play/Pause">&#9646;&#9646;&nbsp;&#9654;</button>
        <button type="button" onclick="musicNext()" title="Next Track">&#9654;&#9654;</button>
      </div>
      <select id="tracks" class="custom-select" onchange="musicSelect(this)"></select>
    </div>
  </div>

  <div id="tab3" class="tab">
    <div class="card" style="text-align:center;">
      <br/>
      <button type="button" class="red" onclick="packOff()" id="btnPackOff">Pack Off</button>
      &nbsp;&nbsp;
      <button type="button" class="green" onclick="packOn()" id="btnPackOn">Pack On</button>
      <br/>
      <br/>
      <br/>
      <button type="button" class="orange" onclick="packVent()" id="btnVent">Vent</button>
      &nbsp;&nbsp;
      <button type="button" class="blue" onclick="packAttenuate()" id="btnAttenuate">Attenuate</button>
    </div>
  </div>

  <div id="tab4" class="tab">
    <div class="card" style="text-align:center;">
      <a href="/settings/attenuator">Attenuator Settings</a>
      <br/>
      <br/>
      <a href="/settings/pack">Proton Pack Settings</a>
      <br/>
      <br/>
      <a href="/settings/wand">Neutrona Wand Settings</a>
      <br/>
      <br/>
      <a href="/settings/smoke">Overheat/Smoke Settings</a>
      <br/>
      <br/>
      <hr/>
      <br/>
      <a href="/update">Update ESP32 Firmware</a>
      <br/>
      <br/>
      <a href="/password">Secure Device WiFi</a>
      <br/>
      <br/>
      <a href="/network">Change WiFi Settings</a>
      <br/>
      <br/>
      <a href="javascript:doRestart()">Restart/Resync</a>
    </div>
    <div class="footer">
      <span id="buildDate"></span>
      &mdash;
      <span id="wifiName"></span>
      <br/>
      <span id="clientInfo"></span>
      <br/>
      <span id="extWifi"></span>
    </div>
  </div>

  <script type="application/javascript">
    var hostname = window.location.hostname;
    var gateway = "ws://" + hostname + "/ws";
    var websocket;
    var statusInterval;
    var musicTrackStart = 0;
    var musicTrackMax = 0;
    var musicTrackCurrent = 0;
    var musicTrackList = [];

    window.addEventListener("load", onLoad);

    function onLoad(event) {
      document.getElementsByClassName("tablinks")[0].click();
      getDevicePrefs(); // Get the device preferences on load.
      initWebSocket(); // Open the WebSocket for server-push data.
      getStatus(); // Get status immediately while WebSocket opens.
    }

    function openTab(evt, tabName) {
      // Hide all tab contents
      var tabs = document.getElementsByClassName("tab");
      for (var i = 0; i < tabs.length; i++) {
          tabs[i].style.display = "none";
      }

      // Remove the active class from all tab links
      var tablinks = document.getElementsByClassName("tablinks");
      for (i = 0; i < tablinks.length; i++) {
          tablinks[i].className = tablinks[i].className.replace(" active", "");
      }

      // Show the current tab and add an "active" class to the button that opened the tab
      showEl(tabName);
      evt.currentTarget.className += " active";
    }

    function getEl(id){
      return document.getElementById(id);
    }

    function setEl(id, value){
      getEl(id).innerHTML = value || "";
    }

    function colorEl(id, red, green, blue, alpha){
      getEl(id).style.backgroundColor = "rgba(" + red + ", " + green + ", " + blue + ", " + alpha + ")";
    }

    function hideEl(id){
      getEl(id).style.display = "none";
    }

    function showEl(id){
      getEl(id).style.display = "block";
    }

    function blinkEl(id, state) {
      if(state) {
        getEl(id).classList.add("blinking");
      } else {
        getEl(id).classList.remove("blinking");
      }
    }

    function initWebSocket() {
      console.log("Attempting to open a WebSocket connection...");
      websocket = new WebSocket(gateway);
      websocket.onopen = onOpen;
      websocket.onclose = onClose;
      websocket.onmessage = onMessage;
      heartbeat();
    }

    function heartbeat() {
      if (websocket.readyState == websocket.OPEN) {
        websocket.send("heartbeat");
      }
      setTimeout(heartbeat, 6000);
    }

    function onOpen(event) {
      console.log("Connection opened");

      // Clear the automated status interval timer.
      clearInterval(statusInterval);
    }

    function onClose(event) {
      console.log("Connection closed");
      setTimeout(initWebSocket, 1000);

      // Fallback for when WebSocket is unavailable.
      if (!statusInterval) {
        statusInterval = setInterval(function() {
          getStatus(); // Check for status every X seconds
        }, 1000);
      }
    }

    function isJsonString(str) {
      try {
        JSON.parse(str);
      } catch (e) {
        return false;
      }
      return true;
    }

    function onMessage(event) {
      if (isJsonString(event.data)) {
        // If JSON, use as status update.
        updateEquipment(JSON.parse(event.data));
      } else {
        // Anything else gets sent to console.
        console.log(event.data);
      }
    }

    function removeOptions(selectElement) {
      var i, len = selectElement.options.length - 1;
      for(i = len; i >= 0; i--) {
        selectElement.remove(i);
      }
    }

    function updateTrackListing() {
      // Continue if start/end values are sane and something actually changed.
      if (musicTrackStart > 0 && musicTrackMax < 1000 && musicTrackMax >= musicTrackStart) {
        // Prepare for track names, if available.
        var trackNum = 0;
        var trackName = "";

        // Update the list of options for track selection.
        var trackList = getEl("tracks");
        if (trackList) {
          removeOptions(trackList); // Clear previous options.

          // Generate an option for each track in the selection field.
          for (var i = musicTrackStart; i <= musicTrackMax; i++) {
            var opt = document.createElement("option");
            opt.setAttribute("value", i);
            if (i == musicTrackCurrent) {
              opt.setAttribute("selected", true);
            }

            trackName = musicTrackList[trackNum] || "";
            if (trackName != "") {
              opt.appendChild(document.createTextNode("#" + i + " " + trackName));
            } else {
              opt.appendChild(document.createTextNode("Track #" + i));
            }

            trackList.appendChild(opt); // Add the option.
            trackNum++; // Advance for the track name array.
          }
        }
      }
    }

    function setButtonStates(mode, pack, wand, cyclotron) {
      // Assume remote on/off is not possible, override as necessary.
      getEl("btnPackOff").disabled = false;
      getEl("btnPackOn").disabled = false;

      // Assume remote venting is not possible, override as necessary.
      getEl("btnVent").disabled = true;

      if (mode == "Original") {
        // Rules for Mode Original

        if (pack == "Powered" && wand == "Powered") {
          // Cannot turn on/off pack remotely if in mode Original and pack+wand are Powered.
          getEl("btnPackOff").disabled = true;
          getEl("btnPackOn").disabled = true;
        }
      } else {
        // Rules for Super Hero

        if (pack == "Powered" && wand == "Powered") {
          // Cannot turn on/off pack remotely if in mode Original and pack+wand are Powered.
          getEl("btnPackOff").disabled = true;
          getEl("btnPackOn").disabled = true;
        } else {
          // Otherwise, buttons can be enabled based on pack/wand status.
          if (pack == "Powered" && wand != "Powered") {
            // Can only turn off the pack, so long as the wand is not powered.
            getEl("btnPackOff").disabled = false;
          }
          if (pack != "Powered") {
            // Can turn on the pack if not already powered (implies wand is not powered).
            getEl("btnPackOn").disabled = false;
          }
        }

        if (wand == "Powered" && (cyclotron == "Normal" || cyclotron == "Active")) {
          // Can only use manual vent if wand is Powered and pack is not already venting.
          // eg. Cyclotron is not in the Warning, Critical, or Recovery states.
          getEl("btnVent").disabled = false;
        }
      }

      // Attenuate action works for either Operation Mode available.
      if (cyclotron == "Warning" || cyclotron == "Critical") {
        // Can only attenuate if cyclotron is in the pre-overheat states.
        getEl("btnAttenuate").disabled = false;
      } else {
        // Otherwise, this should NOT be allowed for any other state.
        getEl("btnAttenuate").disabled = true;
      }
    }

    function getStreamColor(cMode) {
      var color = [0, 0, 0];

      switch(cMode){
        case "Plasm System":
          // Dark Green
          color[1] = 80;
          break;
        case "Dark Matter Gen.":
          // Light Blue
          color[1] = 60;
          color[2] = 255;
          break;
        case "Particle System":
          // Orange
          color[0] = 255;
          color[1] = 140;
          break;
        case "Settings":
          // Gray
          color[0] = 40;
          color[1] = 40;
          color[2] = 40;
          break;
        default:
          // Proton Stream(s) as Red
          color[0] = 180;
      }

      return color;
    }

    function updateBars(iPower, cMode) {
      var color = getStreamColor(cMode);
      var powerBars = getEl("powerBars");
      if (powerBars) {
        powerBars.innerHTML = ""; // Clear previous bars if any

        if (iPower > 0) {
          for (var i = 1; i <= iPower; i++) {
            var bar = document.createElement("div");
            bar.className = "bar";
            bar.style.backgroundColor = "rgba(" + color[0] + ", " + color[1] + ", " + color[2] + ", 0." + Math.round(i * 1.8, 10) + ")";
            powerBars.appendChild(bar);
          }
        }
      }
    }

    function updateGraphics(jObj){
      // Update display if we have the expected data (containing mode and theme).
      if (jObj && jObj.mode && jObj.theme) {
        var color = getStreamColor(jObj.wandMode || "");

        var header = ""; // Used for the title on the display.
        switch(jObj.modeID || 0){
          case 0:
            header = "Standard"; // aka. Mode Original
          break;
          case 1:
            header = "Upgraded"; // aka. Super Hero
          break;
          default:
            header = "- Disabled -";
        }
        switch(jObj.themeID || 0) {
          case 2:
            header += " / V1.9.84";
          break;
          case 3:
            header += " / V1.9.89";
          break;
          case 4:
            header += " / V2.0.21";
          break;
          case 5:
            header += " / V2.0.24";
          break;
          default:
            header += " / V0.0.00";
        }
        setEl("equipTitle", header);

        if (jObj.switch == "Ready") {
          colorEl("ionOverlay", 0, 150, 0, 0.5);
        } else {
          colorEl("ionOverlay", 255, 0, 0, 0.5);
        }

        if (jObj.pack == "Powered") {
          colorEl("pcellOverlay", 0, 150, 0, 0.5);
        } else {
          colorEl("pcellOverlay", 100, 100, 100, 0.5);
        }

        if (jObj.cable == "Disconnected") {
          showEl("cableOverlay");
          blinkEl("cableOverlay", true);
        } else {
          hideEl("cableOverlay");
          blinkEl("cableOverlay", false);
        }

        switch(jObj.cyclotron){
          case "Active":
            colorEl("cycOverlay", 255, 230, 0, 0.5);
            blinkEl("cycOverlay", false);
            break;
          case "Warning":
            colorEl("cycOverlay", 255, 100, 0, 0.5);
            blinkEl("cycOverlay", false);
            break;
          case "Critical":
            colorEl("cycOverlay", 255, 0, 0, 0.5);
            blinkEl("cycOverlay", true);
            break;
          case "Recovery":
            colorEl("cycOverlay", 0, 0, 255, 0.5);
            blinkEl("cycOverlay", false);
            break;
          default:
            if (jObj.pack == "Powered") {
              // Also covers cyclotron state of "Normal"
              colorEl("cycOverlay", 0, 150, 0, 0.5);
            } else {
              colorEl("cycOverlay", 100, 100, 100, 0.5);
            }
            blinkEl("cycOverlay", false);
        }

        if (jObj.pack == "Powered") {
          if (jObj.temperature == "Venting") {
            colorEl("filterOverlay", 255, 0, 0, 0.5);
            blinkEl("filterOverlay", true);
          } else {
            colorEl("filterOverlay", 0, 150, 0, 0.5);
            blinkEl("filterOverlay", false);
          }
        } else {
          colorEl("filterOverlay", 100, 100, 100, 0.5);
          blinkEl("filterOverlay", false);
        }

        // Current Wand Status
        if (jObj.wand == "Connected") {
          // Only update if the wand is physically connected to the pack.
          setEl("streamMode", jObj.wandMode || "");
          setEl("powerLevel", "L-" + (jObj.power || "0"));
          showEl("barrelOverlay");
          colorEl("barrelOverlay", color[0], color[1], color[2], "0." + Math.round(jObj.power * 1.2, 10));
          if (jObj.firing == "Firing") {
            blinkEl("barrelOverlay", true);
          } else {
            blinkEl("barrelOverlay", false);
          }

          if (jObj.wandPower == "Powered") {
            if (jObj.safety == "Safety Off") {
              colorEl("safetyOverlay", 0, 150, 0, 0.5);
            } else {
              colorEl("safetyOverlay", 255, 0, 0, 0.5);
            }
          } else {
            colorEl("safetyOverlay", 100, 100, 100, 0.5);
          }
        } else {
          setEl("powerLevel", "&mdash;");
          setEl("streamMode", "- Disconnected -");
          hideEl("barrelOverlay");
          colorEl("safetyOverlay", 100, 100, 100, 0.5);
        }

        if (jObj.battVoltage) {
          // Voltage should typically be <5.0 but >4.2 under normal use; anything below that indicates a possible problem.
          setEl("battOutput", "Output:<br/>" + parseFloat((jObj.battVoltage || 0).toFixed(2)) + " GeV");
          if (jObj.battVoltage < 4.2) {
            colorEl("boostOverlay", 255, 0, 0, 0.5);
          } else {
            colorEl("boostOverlay", 0, 150, 0, 0.5);
          }
        } else {
          setEl("battOutput", "");
        }

        if(jObj.cyclotron && !jObj.cyclotronLid) {
          showEl("cyclotronLid");
        } else {
          hideEl("cyclotronLid");
        }
      } else {
        // Reset all screen elements to their defaults to indicate no data available.
        setEl("equipTitle", "- Desynchronized -");
        colorEl("ionOverlay", 255, 0, 0, 0.5);
        colorEl("boostOverlay", 100, 100, 100, 0.5);
        colorEl("pcellOverlay", 100, 100, 100, 0.5);
        hideEl("cableOverlay");
        blinkEl("cableOverlay", false);
        colorEl("cycOverlay", 100, 100, 100, 0.5);
        blinkEl("cycOverlay", false);
        hideEl("cyclotronLid");
        colorEl("filterOverlay", 100, 100, 100, 0.5);
        blinkEl("filterOverlay", false);
        hideEl("barrelOverlay");
        blinkEl("barrelOverlay", false);
        setEl("powerLevel", "&mdash;");
        setEl("streamMode", "- Disconnected -");
        colorEl("safetyOverlay", 100, 100, 100, 0.5);
        setEl("battOutput", "");
        hideEl("cyclotronLid");
      }
    }

    function updateEquipment(jObj) {
      // Update display if we have the expected data (containing mode and theme).
      if (jObj && jObj.mode && jObj.theme) {
        // Current Pack Status
        setEl("mode", jObj.mode || "...");
        setEl("theme", jObj.theme || "...");
        setEl("pack", jObj.pack || "...");
        setEl("switch", jObj.switch || "...");
        setEl("cable", jObj.cable || "...");
        if(jObj.cyclotron && !jObj.cyclotronLid) {
          setEl("cyclotron", (jObj.cyclotron || "") + " &#9762;");
        } else {
          setEl("cyclotron", jObj.cyclotron || "...");
        }
        setEl("temperature", jObj.temperature || "...");
        setEl("wand", jObj.wand || "...");

        // Current Wand Status
        if (jObj.wand == "Connected") {
          // Only update if the wand is physically connected to the pack.
          setEl("wandPower", jObj.wandPower || "...");
          setEl("wandMode", jObj.wandMode || "...");
          setEl("safety", jObj.safety || "...");
          setEl("power", jObj.power || "...");
          setEl("firing", jObj.firing || "...");
          updateBars(jObj.power || 0, jObj.wandMode || "");
        } else {
          // Default to empty values when wand is not present.
          setEl("wandPower", "...");
          setEl("wandMode", "...");
          setEl("safety", "...");
          setEl("power", "...");
          setEl("firing", "...");
          updateBars(0, "");
        }

        if (jObj.battVoltage) {
          // Voltage should typically be <5.0 but >4.2 under normal use; anything below that indicates high drain.
          setEl("battVoltage", parseFloat((jObj.battVoltage || 0).toFixed(2)));
          if (jObj.battVoltage < 4.2) {
            setEl("battHealth", "&#129707;"); // Draining Battery
          } else {
            setEl("battHealth", "&#128267;"); // Healthy Battery
          }
        } else {
          setEl("battVoltage", "...");
          setEl("battHealth", "");
        }

        // Volume Information
        setEl("masterVolume", (jObj.volMaster || 0) + "%");
        setEl("effectsVolume", (jObj.volEffects || 0) + "%");
        setEl("musicVolume", (jObj.volMusic || 0) + "%");

        // Update special UI elements based on the latest data values.
        setButtonStates(jObj.mode, jObj.pack, jObj.wandPower, jObj.cyclotron);

        // Update the current track info.
        musicTrackStart = jObj.musicStart || 0;
        musicTrackMax = jObj.musicEnd || 0;
        if (musicTrackCurrent != (jObj.musicCurrent || 0)) {
          musicTrackCurrent = jObj.musicCurrent || 0;
          updateTrackListing();
        }
      }

      // Connected Wifi Clients - Private AP vs. WebSocket
      setEl("clientInfo", "AP Clients: " + (jObj.apClients || 0) + " / WebSocket Clients: " + (jObj.wsClients || 0));

      updateGraphics(jObj);
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

    function getStatus() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          updateEquipment(JSON.parse(this.responseText));
        }
      };
      xhttp.open("GET", "/status", true);
      xhttp.send();
    }

    function getDevicePrefs() {
      // This is updated once per page load as it is not subject to frequent changes.
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var jObj = JSON.parse(this.responseText);
          if (jObj) {
            if (jObj.songList && jObj.songList != "") {
              musicTrackList = jObj.songList.split("\n");
              updateTrackListing();
            }

            // Device Info
            setEl("buildDate", "Build: " + (jObj.buildDate || ""));
            setEl("wifiName", jObj.wifiName || "");
            if ((jObj.wifiNameExt || "") != "" && (jObj.extAddr || "") != "" || (jObj.extMask || "") != "") {
              setEl("extWifi", (jObj.wifiNameExt || "") + ": " + jObj.extAddr + " / " + jObj.extMask);
            }

            // Display Preference
            switch(jObj.displayType || 0) {
              case 0:
                // Text-Only Display
                hideEl("equipCRT");
                showEl("equipTXT");
                break;
              case 1:
                // Graphical Display
                showEl("equipCRT");
                hideEl("equipTXT");
                break;
              case 2:
                // Both graphical and text
                showEl("equipCRT");
                showEl("equipTXT");
                break;
            }
          }
        }
      };
      xhttp.open("GET", "/config/attenuator", true);
      xhttp.send();
    }

    function doRestart() {
      if (confirm("Are you sure you wish to restart the serial device?")) {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 204) {
            // Reload the page after 2 seconds.
            setTimeout(function() {
              window.location.reload();
            }, 2000);
          }
        };
        xhttp.open("DELETE", "/restart", true);
        xhttp.send();
      }
    }

    function sendCommand(apiUri) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          handleStatus(this.responseText);
        }
      };
      xhttp.open("PUT", apiUri, true);
      xhttp.send();
    }

    function packOn() {
      sendCommand("/pack/on");
    }

    function packOff() {
      sendCommand("/pack/off");
    }

    function packAttenuate() {
      sendCommand("/pack/attenuate");
    }

    function packVent() {
      sendCommand("/pack/vent");
    }

    function toggleMute() {
      sendCommand("/volume/toggle");
    }

    function volSysUp() {
      sendCommand("/volume/master/up");
    }

    function volSysDown() {
      sendCommand("/volume/master/down");
    }

    function volFxUp() {
      sendCommand("/volume/effects/up");
    }

    function volFxDown() {
      sendCommand("/volume/effects/down");
    }

    function volMusicUp() {
      sendCommand("/volume/music/up");
    }

    function volMusicDown() {
      sendCommand("/volume/music/down");
    }

    function musicStartStop() {
      sendCommand("/music/startstop");
    }

    function musicPauseResume() {
      sendCommand("/music/pauseresume");
    }

    function musicNext() {
      sendCommand("/music/next");
    }

    function musicSelect(caller) {
      sendCommand("/music/select?track=" + caller.value);
    }

    function musicPrev() {
      sendCommand("/music/prev");
    }
  </script>
</body>
</html>
)=====";
