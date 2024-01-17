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
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Proton Pack</title>
  <link rel="icon" href="data:;base64,iVBORw0KGgo=">
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1>Equipment Status</h1>
  <div class="card">
    <p><b>Operation Mode:</b> <span class="info" id="mode">&mdash;</span></p>
    <p><b>Effects Theme:</b> <span class="info" id="theme">&mdash;</span></p>
    <br/>
    <p><b>Pack State:</b> <span class="info" id="pack">&mdash;</span></p>
    <p><b>Pack Armed:</b> <span class="info" id="switch">&mdash;</span></p>
    <p><b>Ribbon Cable:</b> <span class="info" id="cable">&mdash;</span></p>
    <p><b>Cyclotron State:</b> <span class="info" id="cyclotron">&mdash;</span></p>
    <p><b>Overheat State:</b> <span class="info" id="temperature">&mdash;</span></p>
    <br/>
    <p><b>Wand Presence:</b> <span class="info" id="wand">&mdash;</span></p>
    <p><b>Wand State:</b> <span class="info" id="wandPower">&mdash;</span></p>
    <p><b>Wand Armed:</b> <span class="info" id="safety">&mdash;</span></p>
    <p><b>System Mode:</b> <span class="info" id="wandMode">&mdash;</span></p>
    <p><b>Power Level:</b> <span class="info" id="power">&mdash;</span></p>
    <p><b>Firing State:</b> <span class="info" id="firing">&mdash;</span></p>
    <br/>
    <p>
      <b>Battery Health:</b> <span id="battHealth"></span>
      <span class="info" id="battVoltage">&mdash;</span>
      <span style="font-size: 0.6em">VDC</span>
    </p>
  </div>

  <h1>Audio Controls</h1>
  <div class="block">
    <h3>Master Volume: <span id="masterVolume"></span></h3>
    <button type="button" class="blue" onclick="volumeMasterDown()">- Down</button>
    <button type="button" class="orange" onclick="toggleMute()">Mute/Unmute</button>
    <button type="button" class="blue" onclick="volumeMasterUp()">Up +</button>
    <br/>
    <h3>Effects Volume: <span id="effectsVolume"></span></h3>
    <button type="button" class="blue" onclick="volumeEffectsDown()">- Down</button>
    <button type="button" class="blue" onclick="volumeEffectsUp()">Up +</button>
    <br/>
    <h3>Music Controls</h3>
    <div style="color:#444;"><b>Music Volume:</b> <span id="musicVolume"></span></div>
    <br/>
    <button type="button" class="green" onclick="startstopMusic()">Start/Stop</button>
    <br/>
    <select id="tracks" class="custom-select" onchange="musicSelect(this)"></select>
    <br/>
    <button type="button" class="blue" onclick="musicPrev()">&laquo; Prev</button>
    &nbsp;
    <button type="button" class="blue" onclick="musicNext()">Next &raquo;</button>
    <br/>
    <button type="button" class="green" onclick="pauseresumeMusic()"
     style="width:120px;margin-top:10px">Pause/Resume</button>
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
    <a href="/password">Change WiFi Password</a>
    <br/>
    <br/>
    <a href="javascript:doRestart()">Restart/Resync</a>
    <br/>
    <br/>
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

    function updateTrackListing(musicStart, musicEnd, musicCurrent) {
      // Continue if start/end values are sane and something actually changed.
      if (musicStart > 0 && musicEnd < 1000 && musicEnd > musicStart &&
         (musicTrackMax != musicEnd || musicTrackCurrent != musicCurrent)) {
        // Proceed if we have a starting track and valid end track, and if current track changed.
        musicTrackMax = musicEnd;
        musicTrackCurrent = musicCurrent;

        var trackList = document.getElementById("tracks");
        if (trackList) {
          removeOptions(trackList); // Clear previous options.

          // Generate an option for each track in the selection field.
          for (var i = musicStart; i <= musicEnd; i++) {
            var opt = document.createElement("option");
            opt.setAttribute("value", i);
            if (i == musicCurrent) {
              opt.setAttribute("selected", true);
            }
            var txt = document.createTextNode("Track #" + i);
            opt.appendChild(txt);
            trackList.appendChild(opt);
          }
        }
      }
    }

    function setButtonStates(mode, pack, wand, cyclotron) {
      if (mode == "Original" && pack == "Powered" && wand == "Powered") {
        // Cannot turn off pack remotely if in mode Original and pack/wand are Powered.
        document.getElementById("btnPackOff").disabled = true;
      } else {
        // Otherwise, this should be allowed.
        document.getElementById("btnPackOff").disabled = false;
      }

      if (mode == "Original" && pack == "Powered" && wand == "Powered") {
        // Cannot turn off pack remotely if in mode Original and pack/wand are Powered.
        document.getElementById("btnPackOn").disabled = true;
      } else {
        // Otherwise, this should be allowed.
        document.getElementById("btnPackOn").disabled = false;
      }

      if (mode == "Super Hero" && wand == "Powered" && cyclotron != "Recovery") {
        // Can only use manual vent if mode Super Hero and wand is Powered and not already venting.
        document.getElementById("btnVent").disabled = false;
      } else {
        // Otherwise, this action should NOT be allowed (button disabled).
        document.getElementById("btnVent").disabled = true;
      }

      if (cyclotron == "Warning" || cyclotron == "Critical") {
        // Can only attenuate if cyclotron is in certain states.
        document.getElementById("btnAttenuate").disabled = false;
      } else {
        // Otherwise, this should NOT be allowed.
        document.getElementById("btnAttenuate").disabled = true;
      }
    }

    function updateStatus(jObj) {
      // Update display if we have the expected data (containing mode and theme).
      if (jObj && jObj.mode && jObj.theme) {
        // Current Pack Status
        document.getElementById("mode").innerHTML = jObj.mode || "...";
        document.getElementById("theme").innerHTML = jObj.theme || "...";
        document.getElementById("switch").innerHTML = jObj.switch || "...";
        document.getElementById("pack").innerHTML = jObj.pack || "...";
        document.getElementById("power").innerHTML = jObj.power || "...";
        document.getElementById("wand").innerHTML = jObj.wand || "...";
        document.getElementById("cable").innerHTML = jObj.cable || "...";
        document.getElementById("cyclotron").innerHTML = jObj.cyclotron || "...";
        document.getElementById("temperature").innerHTML = jObj.temperature || "...";

        // Current Wand Status
        if (jObj.wand == "Connected") {
          // Only update if the wand is physically connected to the pack.
          document.getElementById("wandPower").innerHTML = jObj.wandPower || "...";
          document.getElementById("wandMode").innerHTML = jObj.wandMode || "...";
          document.getElementById("safety").innerHTML = jObj.safety || "...";
          document.getElementById("firing").innerHTML = jObj.firing || "...";
        } else {
          // Default to empty values when wand is not present.
          document.getElementById("wandPower").innerHTML = "...";
          document.getElementById("wandMode").innerHTML = "...";
          document.getElementById("safety").innerHTML = "...";
          document.getElementById("firing").innerHTML = "...";
        }

        if (jObj.battVoltage) {
          // Voltage should typically be <5.0 but >4.2 under normal use; anything below that indicates a possible problem.
          document.getElementById("battVoltage").innerHTML = parseFloat((jObj.battVoltage || 0).toFixed(2));
          if (jObj.battVoltage < 4.2) {
            document.getElementById("battHealth").innerHTML = "&#129707;"; // Low Battery
          } else {
            document.getElementById("battHealth").innerHTML = "&#128267;"; // Good Battery
          }
        }

        // Volume Information
        document.getElementById("masterVolume").innerHTML = (jObj.volMaster || 0) + "%";
        document.getElementById("effectsVolume").innerHTML = (jObj.volEffects || 0) + "%";
        document.getElementById("musicVolume").innerHTML = (jObj.volMusic || 0) + "%";

        // Update special UI elements based on the latest data values.
        setButtonStates(jObj.mode, jObj.pack, jObj.wandPower, jObj.cyclotron);
        updateTrackListing(jObj.musicStart, jObj.musicEnd, jObj.musicCurrent);
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
            setTimeout(function(){
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

    function startstopMusic() {
      sendCommand("/music/startstop");
    }

    function pauseresumeMusic() {
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