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

const char INDEX_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Proton Pack</title>

  <style>
    html {
      font-family: Tahoma, Verdana, Arial;
      display: inline-block;
      margin: 0px;
      text-align: center;
    }

    body {
      margin: 0px 0px 20px 0px;
    }

    h1 {
        background: #222;
        color: #eee;
        margin: 0px;
        padding: 8px;
        width: 100%;
    }

    h3 { color: #333; margin: 10px; }

    a { text-decoration: none; }

    p { font-size: 18px; margin-bottom: 5px; }

    .block {
      margin: 10px 5px 30px 5px;
      padding: 10px 5px 10px 5px;
    }

    .card {
      background: #ddd;
      box-sizing: border-box;
      box-shadow: 0px 2px 18px -4px rgba(0, 0, 0, 0.75);
      color: #444;
      margin: 20px auto;
      max-width: 400px;
      min-height: 200px;
      padding: 10px 20px 10px 20px;
      text-align: left;
    }

    .info {
      font-family: courier, courier new, serif;
    }

    button {
      background-color: #555;
      border: 2px solid #333;
      border-radius: 8px;
      box-shadow: 0 8px 16px 0 rgba(0,0,0,0.2), 0 6px 20px 0 rgba(0,0,0,0.19);
      color: white;
      margin: 5px;
      padding: 12px 14px;
      max-width: 120px;
      text-align: center;
      touch-action: manipulation;
      width: 100px;
    }

    button:disabled,
    button[disabled] {
      border: 1px solid #999999 !important;
      background-color: #cccccc !important;
      color: #555 !important;
    }

    input {
      border: 2px solid #333;
      border-radius: 4px;
      box-sizing: border-box;
      font-size: 18px;
      margin: 4px 0;
      padding: 6px 10px;
      width: 50%;
    }

    select {
      background-color: #999;
      border: 2px solid #333;
      border-radius: 8px;
      box-shadow: 0 8px 16px 0 rgba(0,0,0,0.2), 0 6px 20px 0 rgba(0,0,0,0.19);
      color: white;
      height: 40px;
      margin: 5px;
      padding: 12px 14px;
      max-width: 260px;
      text-align: center;
      touch-action: manipulation;
      width: 260px;
    }

    .red {
      background-color: #f44336;
      border: 2px solid #d42316;
    }
    .orange {
      background-color: #ffac1c;
      border: 2px solid #dd9a0a;
    }
    .green {
      background-color: #4CAF50;
      border: 2px solid #2C8F30;
    }
    .blue {
      background-color: #008CBA;
      border: 2px solid #006C9A;
    }
  </style>

  <script type="application/javascript">
    var hostname = window.location.hostname;
    var gateway = "ws://" + hostname + "/ws";
    var websocket;
    var statusInterval;
    var musicTrackMax = 0;

    window.addEventListener("load", onLoad);

    function onLoad(event) {
      initWebSocket();
      getStatus();
    }

    function initWebSocket() {
      console.log("Trying to open a WebSocket connection...");
      websocket = new WebSocket(gateway);
      websocket.onopen = onOpen;
      websocket.onclose = onClose;
      websocket.onmessage = onMessage;
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
      statusInterval = setInterval(function() {
        getStatus(); // Check for status every X seconds
      }, 1000);
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

    function updateStatus(jObj) {
      if (jObj) {
        document.getElementById("mode").innerHTML = jObj.mode || "...";
        document.getElementById("theme").innerHTML = jObj.theme || "...";
        document.getElementById("switch").innerHTML = jObj.switch || "...";
        document.getElementById("pack").innerHTML = jObj.pack || "...";
        document.getElementById("power").innerHTML = jObj.power || "...";
        document.getElementById("safety").innerHTML = jObj.safety || "...";
        document.getElementById("wand").innerHTML = jObj.wand || "...";
        document.getElementById("wandMode").innerHTML = jObj.wandMode || "...";
        document.getElementById("firing").innerHTML = jObj.firing || "...";
        document.getElementById("cable").innerHTML = jObj.cable || "...";
        document.getElementById("cyclotron").innerHTML = jObj.cyclotron || "...";
        document.getElementById("temperature").innerHTML = jObj.temperature || "...";

        if (jObj.mode == "Original" && jObj.pack == "Powered") {
          // Cannot turn off pack remotely if in mode Original and pack is Powered.
          document.getElementById("btnPackOff").disabled = true;
        } else {
          // Otherwise, this should be allowed.
          document.getElementById("btnPackOff").disabled = false;
        }

        if (jObj.mode == "Super Hero" && jObj.wand == "Powered") {
          // Can only use manual vent if mode Super Hero and wand is Powered.
          document.getElementById("btnVent").disabled = false;
        } else {
          // Otherwise, this should NOT be allowed.
          document.getElementById("btnVent").disabled = true;
        }

        if (jObj.musicStart > 0 && jObj.musicEnd > jObj.musicStart && musicTrackMax != jObj.musicEnd) {
          // Proceed if we have a starting track and valid end track.
          musicTrackMax = jObj.musicEnd;
          var trackList = document.getElementById("tracks");
          removeOptions(trackList);
          for (var i = jObj.musicStart; i <= jObj.musicEnd; i++) {
              var opt = document.createElement("option");
              opt.value = i;
              opt.innerHTML = i;
              trackList.appendChild(opt);
          }
        }
      }
    }

    function getStatus() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var data = JSON.parse(this.responseText);
          updateStatus(data);
        }
      };
      xhttp.open("GET", "/status", true);
      xhttp.send();
    }

    function packOn() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/pack/on", true);
      xhttp.send();
    }

    function packOff() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/pack/off", true);
      xhttp.send();
    }

    function cancelWarning() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/pack/cancel", true);
      xhttp.send();
    }

    function beginVenting() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/pack/vent", true);
      xhttp.send();
    }

    function toggleMute() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/volume/toggle", true);
      xhttp.send();
    }

    function volumeMasterUp() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/volume/master/up", true);
      xhttp.send();
    }

    function volumeMasterDown() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/volume/master/down", true);
      xhttp.send();
    }

    function volumeEffectsUp() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/volume/effects/up", true);
      xhttp.send();
    }

    function volumeEffectsDown() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/volume/effects/down", true);
      xhttp.send();
    }

    function toggleMusic() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/music/toggle", true);
      xhttp.send();
    }

    function musicNext() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/music/next", true);
      xhttp.send();
    }

    function musicSelect(caller) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/music/select?track=" + caller.value, true);
      xhttp.send();
    }

    function musicPrev() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/music/prev", true);
      xhttp.send();
    }
  </script>
</head>
<body>
  <h1>Equipment Status</h1>
  <div class="card">
    <p><b>System Mode:</b> <span class="info" id="mode">&mdash;</span></p>
    <p><b>Theme Mode:</b> <span class="info" id="theme">&mdash;</span></p>
    <br/>
    <p><b>Pack State:</b> <span class="info" id="pack">&mdash;</span></p>
    <p><b>Pack Armed:</b> <span class="info" id="switch">&mdash;</span></p>
    <p><b>Ribbon Cable:</b> <span class="info" id="cable">&mdash;</span></p>
    <p><b>Cyclotron State:</b> <span class="info" id="cyclotron">&mdash;</span></p>
    <p><b>Overheat State:</b> <span class="info" id="temperature">&mdash;</span></p>
    <br/>
    <p><b>Wand State:</b> <span class="info" id="wand">&mdash;</span></p>
    <p><b>Wand Armed:</b> <span class="info" id="safety">&mdash;</span></p>
    <p><b>Wand Mode:</b> <span class="info" id="wandMode">&mdash;</span></p>
    <p><b>Power Level:</b> <span class="info" id="power">&mdash;</span></p>
    <p><b>Firing State:</b> <span class="info" id="firing">&mdash;</span></p>
  </div>

  <h1>Audio Controls</h1>
  <div class="block">
    <h3>Master Volume</h3>
    <button type="button" class="blue" onclick="volumeMasterDown()">- Down</button>
    <button type="button" class="orange" onclick="toggleMute()">Mute/Unmute</button>
    <button type="button" class="blue" onclick="volumeMasterUp()">Up +</button>
    <br/>
    <h3>Music Playback</h3>
    <button type="button" class="blue" onclick="musicPrev()">&laquo; Prev</button>
    <button type="button" class="green" onclick="toggleMusic()">Start/Stop</button>
    <button type="button" class="blue" onclick="musicNext()">Next &raquo;</button>
    <br/>
    <h3>Play Music Track</h3>
    <select id="tracks" onchange="musicSelect(this)"></select>
    <br/>
    <h3>Effects Volume</h3>
    <button type="button" class="blue" onclick="volumeEffectsDown()">- Down</button>
    &nbsp;&nbsp;
    <button type="button" class="blue" onclick="volumeEffectsUp()">Up +</button>
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
    <button type="button" class="blue" onclick="cancelWarning()">Attenuate</button>
  </div>

  <h1>Administration</h1>
  <div class="block">
    <a href="/password">Change WiFi Password</a>
    &nbsp;&nbsp;&nbsp;
    <a href="/update">Update Firmware</a>
  </div>
</body>
</html>
)=====";