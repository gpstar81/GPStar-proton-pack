/**
 *   GPStar Stream Effects - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2024-2025 Dustin Grau <dustin.grau@gmail.com>
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

const char INDEXJS_page[] PROGMEM = R"=====(
var websocket;
var statusInterval;

window.addEventListener("load", onLoad);

function onLoad(event) {
  document.getElementsByClassName("tablinks")[0].click();
  getDevicePrefs(); // Get all preferences.
  initWebSocket(); // Open the WebSocket.
  getStatus(updateEquipment); // Get status immediately.
}

function initWebSocket() {
  console.log("Attempting to open a WebSocket connection...");
  let gateway = "ws://" + window.location.hostname + "/ws";
  websocket = new WebSocket(gateway);
  websocket.onopen = onOpen;
  websocket.onclose = onClose;
  websocket.onmessage = onMessage;
  doHeartbeat();
}

function doHeartbeat() {
  if (websocket.readyState == websocket.OPEN) {
    websocket.send("heartbeat"); // Send a specific message.
  }
  setTimeout(doHeartbeat, 8000);
}

function onOpen(event) {
  console.log("WebSocket connection opened");

  // Clear the automated status interval timer.
  clearInterval(statusInterval);
}

function onClose(event) {
  console.log("WebSocket connection closed");
  setTimeout(initWebSocket, 1000);

  // Fallback for when WebSocket is unavailable.
  if (!statusInterval) {
    statusInterval = setInterval(function() {
      getStatus(updateEquipment); // Check for status every X seconds
    }, 1000);
  }
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

function getDevicePrefs() {
  // This is updated once per page load as it is not subject to frequent changes.
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var jObj = JSON.parse(this.responseText);
      if (jObj) {
        // Device Info
        setHtml("buildDate", "Build: " + (jObj.buildDate || ""));
        setHtml("wifiName", jObj.wifiName || "");
        if ((jObj.wifiNameExt || "") != "" && (jObj.extAddr || "") != "" || (jObj.extMask || "") != "") {
          setHtml("extWifi", (jObj.wifiNameExt || "") + ": " + jObj.extAddr + " / " + jObj.extMask);
        }
      }
    }
  };
  xhttp.open("GET", "/config/device", true);
  xhttp.send();
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
    break;
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

function updateEquipment(jObj) {
  // Update display if we have the expected data (containing mode and theme at a minimum).
  if (jObj) {
    if (jObj.mode && jObj.theme) {
      // Current Pack Status
      setHtml("mode", jObj.mode || "...");
      setHtml("theme", jObj.theme || "...");
      setHtml("pack", jObj.pack || "...");
      setHtml("switch", jObj.switch || "...");
      setHtml("cable", jObj.cable || "...");
      setHtml("cyclotron", jObj.cyclotron || "...");
      setHtml("temperature", jObj.temperature || "...");

      // Current Wand Status
      setHtml("wandMode", jObj.wandMode || "...");
      setHtml("safety", jObj.safety || "...");
      setHtml("power", jObj.power || "...");
      setHtml("firing", jObj.firing || "...");
      updateBars(jObj.power || 0, jObj.wandMode || "");
    } else {
      // If no data, clear everything.
      setHtml("mode", "...");
      setHtml("theme", "...");
      setHtml("pack", "...");
      setHtml("switch", "...");
      setHtml("cable", "...");
      setHtml("cyclotron", "...");
      setHtml("temperature", "...");
      setHtml("wandMode", "...");
      setHtml("safety", "...");
      setHtml("power", "...");
      setHtml("firing", "...");
      updateBars(0, "");
    }

    // External WiFi Status
    if (jObj.extWifiEnabled) {
      setHtml("wifiStatus", jObj.extWifiStarted ? "Connected" : (jObj.extWifiPaused ? "Paused" : "Connecting..."));
    } else {
      setHtml("wifiStatus", "Disabled");
    }

    // Connected Wifi Clients - Private AP vs. WebSocket
    setHtml("clientInfo", "AP Clients: " + (jObj.apClients || 0) + " / WebSocket Clients: " + (jObj.wsClients || 0));
  }
}

function testOn() {
  sendCommand("/selftest/enable");
}

function testOff() {
  sendCommand("/selftest/disable");
}
)=====";
