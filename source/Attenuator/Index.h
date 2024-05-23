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
  <title>Proton Pack</title>
  <link rel="icon" href="data:;base64,iVBORw0KGgo=">
  <link rel="stylesheet" href="/style.css">
  <link rel="stylesheet" href="/style2.css">
</head>
<body>
  <h1>Equipment Status</h1>
  <div class="card">
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
  <div class="equipment">
    <div id="pcOverlay" class="overlay power-box" style="background:rgba(0, 0, 255, 0.5);"></div>
    <div id="cycOverlay" class="overlay cyc-circle" style="background:rgba(255, 0, 0, 0.5);"></div>
    <div id="barrelOverlay" class="overlay barrel-box" style="background:rgba(0, 255, 0, 0.5);"></div>
    <div id="warnOverlay" class="overlay ribbon-warning"><div class="exclamation">!</div></div>
  </div>

  <h1>Audio Controls</h1>
  <div>
    <div class="volume-container">
      <div class="volume-control">
        <h3>System</h3>
        <button type="button" onclick="volumeMasterUp()">+</button>
        <span id="masterVolume"></span>
        <button type="button" onclick="volumeMasterDown()">&minus;</button>
      </div>
      <div class="volume-control">
        <h3>Effects</h3>
        <button type="button" onclick="volumeEffectsUp()">+</button>
        <span id="effectsVolume"></span>
        <button type="button" onclick="volumeEffectsDown()">&minus;</button>
      </div>
      <div class="volume-control">
        <h3>Music</h3>
        <button type="button" onclick="volumeMusicUp()">+</button>
        <span id="musicVolume"></span>
        <button type="button" onclick="volumeMusicDown()">&minus;</button>
      </div>
    </div>
    <button type="button" class="orange" onclick="toggleMute()">Mute/Unmute</button>
    <br/>
    <h3>Music Navigation</h3>
    <div class="music-navigation">
      <button type="music-button" onclick="musicPrev()" title="Previous Track">&#9664;&#9664;</button>
      <button type="music-button" onclick="musicStartStop()" title="Start/Stop">&#9634;&nbsp;&#9654;</button>
      <button type="music-button" onclick="musicPauseResume()" title="Play/Pause">&#9646;&#9646;&nbsp;&#9654;</button>
      <button type="music-button" onclick="musicNext()" title="Next Track">&#9654;&#9654;</button>
    </div>
    <select id="tracks" class="custom-select" onchange="musicSelect(this)" style="width:300px"></select>
    <br/>
    <br/>
  </div>

  <h1>Pack Controls</h1>
  <div class="block">
    <button type="button" class="red" onclick="packOff()" id="btnPackOff">Pack Off</button>
    &nbsp;&nbsp;
    <button type="button" class="green" onclick="packOn()" id="btnPackOn">Pack On</button>
    <br/>
    <br/>
    <button type="button" class="orange" onclick="beginVenting()" id="btnVent">Vent</button>
    &nbsp;&nbsp;
    <button type="button" class="blue" onclick="attenuatePack()" id="btnAttenuate">Attenuate</button>
  </div>

  <h1>Preferences</h1>
  <div class="block">
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
  </div>

  <h1>Administration</h1>
  <div class="block">
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
    <br/>
    <br/>
    <div class="footer">
      <span id="buildDate"></span>
      &mdash;
      <span id="wifiName"></span>
      <br/>
      <span id="extWifi"></span>
    </div>
  </div>

  <script type="application/javascript">
    var hostname = window.location.hostname;
    var gateway = "ws://" + hostname + "/ws";
    var websocket;
    var statusInterval;
    var musicTrackCurrent = 0;
    var musicTrackMax = 0;

    window.addEventListener("load", onLoad);

    function onLoad(event) {
      initWebSocket(); // Open the WebSocket for server-push data.
      getStatus(); // Get status immediately while WebSocket opens.
    }

    function getEl(id){
      return document.getElementById(id);
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
        updateStatus(JSON.parse(event.data));
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

    function updateTrackListing(musicStart, musicEnd, musicCurrent, songList) {
      // Continue if start/end values are sane and something actually changed.
      if (musicStart > 0 && musicEnd < 1000 && musicEnd >= musicStart &&
         (musicTrackMax != musicEnd || musicTrackCurrent != musicCurrent)) {
        // Proceed if we have a starting track and valid end track, and if current track changed.
        musicTrackMax = musicEnd;
        musicTrackCurrent = musicCurrent;

        // Prepare for track names, if available.
        var tracks = [];
        var trackNum = 0;
        var trackName = "";
        if (songList != "") {
          tracks = songList.split('\n');
        }

        // Update the list of options for track selection.
        var trackList = getEl("tracks");
        if (trackList) {
          removeOptions(trackList); // Clear previous options.

          // Generate an option for each track in the selection field.
          for (var i = musicStart; i <= musicEnd; i++) {
            var opt = document.createElement("option");
            opt.setAttribute("value", i);
            if (i == musicCurrent) {
              opt.setAttribute("selected", true);
            }

            trackName = tracks[trackNum] || "";
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

    function getColor(cMode) {
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
      var powerBars = getEl("powerBars");
      if (powerBars) {
        powerBars.innerHTML = ""; // Clear previous bars if any

        if (iPower > 0) {
          var color = getColor(cMode);
          for (var i = 1; i <= iPower; i++) {
            var bar = document.createElement("div");
            bar.className = "bar";
            bar.style.backgroundColor = "rgba(" + color[0] + ", " + color[1] + ", " + color[2] + ", 0." + Math.round(i * 1.8, 10) + ")";
            powerBars.appendChild(bar);
          }
        }
      }
    }

    function updateStatus(jObj) {
      // Update display if we have the expected data (containing mode and theme).
      if (jObj && jObj.mode && jObj.theme) {
        // Current Pack Status
        getEl("mode").innerHTML = jObj.mode || "...";
        getEl("theme").innerHTML = jObj.theme || "...";
        getEl("switch").innerHTML = jObj.switch || "...";
        getEl("pack").innerHTML = jObj.pack || "...";
        getEl("cable").innerHTML = jObj.cable || "...";
        getEl("cyclotron").innerHTML = jObj.cyclotron || "...";
        getEl("temperature").innerHTML = jObj.temperature || "...";
        getEl("wand").innerHTML = jObj.wand || "...";

        // Current Wand Status
        if (jObj.wand == "Connected") {
          // Only update if the wand is physically connected to the pack.
          getEl("wandPower").innerHTML = jObj.wandPower || "...";
          getEl("wandMode").innerHTML = jObj.wandMode || "...";
          getEl("safety").innerHTML = jObj.safety || "...";
          getEl("power").innerHTML = jObj.power || "...";
          getEl("firing").innerHTML = jObj.firing || "...";
          updateBars(jObj.power || 0, jObj.wandMode || "");
        } else {
          // Default to empty values when wand is not present.
          getEl("wandPower").innerHTML = "...";
          getEl("wandMode").innerHTML = "...";
          getEl("safety").innerHTML = "...";
          getEl("power").innerHTML = "...";
          getEl("firing").innerHTML = "...";
          updateBars(0, jObj.wandMode || "");
        }

        if (jObj.battVoltage) {
          // Voltage should typically be <5.0 but >4.2 under normal use; anything below that indicates a possible problem.
          getEl("battVoltage").innerHTML = parseFloat((jObj.battVoltage || 0).toFixed(2));
          if (jObj.battVoltage < 4.2) {
            getEl("battHealth").innerHTML = "&#129707;"; // Low Battery
          } else {
            getEl("battHealth").innerHTML = "&#128267;"; // Good Battery
          }
        }

        // Volume Information
        getEl("masterVolume").innerHTML = (jObj.volMaster || 0) + "%";
        getEl("effectsVolume").innerHTML = (jObj.volEffects || 0) + "%";
        getEl("musicVolume").innerHTML = (jObj.volMusic || 0) + "%";

        // Device Info
        getEl("buildDate").innerHTML = jObj.buildDate || "";
        getEl("wifiName").innerHTML = jObj.wifiName || "";
        if ((jObj.extAddr || "") != "" || (jObj.extMask || "") != "") {
          getEl("extWifi").innerHTML = jObj.extAddr + " / " + jObj.extMask;
        }

        // Update special UI elements based on the latest data values.
        setButtonStates(jObj.mode, jObj.pack, jObj.wandPower, jObj.cyclotron);
        updateTrackListing(jObj.musicStart, jObj.musicEnd, jObj.musicCurrent, jObj.songList || 0);
      }
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
          updateStatus(JSON.parse(this.responseText));
        }
      };
      xhttp.open("GET", "/status", true);
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

    function attenuatePack() {
      sendCommand("/pack/attenuate");
    }

    function beginVenting() {
      sendCommand("/pack/vent");
    }

    function toggleMute() {
      sendCommand("/volume/toggle");
    }

    function volumeMasterUp() {
      sendCommand("/volume/master/up");
    }

    function volumeMasterDown() {
      sendCommand("/volume/master/down");
    }

    function volumeEffectsUp() {
      sendCommand("/volume/effects/up");
    }

    function volumeEffectsDown() {
      sendCommand("/volume/effects/down");
    }

    function volumeMusicUp() {
      sendCommand("/volume/music/up");
    }

    function volumeMusicDown() {
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
