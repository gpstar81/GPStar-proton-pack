/**
 *   GPStar Ghost Trap - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Nomake Wan <-redacted->
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

const char INDEXJS_page[] PROGMEM = R"=====(
var websocket;
var statusInterval;
var musicTrackStart = 0, musicTrackMax = 0, musicTrackCurrent = 0, musicTrackList = [];

window.addEventListener("load", onLoad);

function onLoad(event) {
  document.getElementsByClassName("tablinks")[0].click();
  getDevicePrefs(); // Get all preferences.
  initWebSocket(); // Open the WebSocket.
  getStatus(); // Get status immediately.
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

function onMessage(event) {
  if (isJsonString(event.data)) {
    // If JSON, use as status update.
    updateEquipment(JSON.parse(event.data));
  } else {
    // Anything else gets sent to console.
    console.log(event.data);
  }
}

function setButtonStates(smokeEnabled) {
  // Assume all functions are not possible, override as necessary.
  getEl("btnSmoke2").disabled = true;
  getEl("btnSmoke5").disabled = true;
  getEl("btnSmokeEnable").disabled = true;
  getEl("btnSmokeDisable").disabled = true;

  if (smokeEnabled) {
    // Enable specific buttons only when smoke is enabled.
    getEl("btnSmoke2").disabled = false;
    getEl("btnSmoke5").disabled = false;
    getEl("btnSmokeDisable").disabled = false;
  } else {
    // Otherwise, make sure the user can re-enable smoke.
    getEl("btnSmokeEnable").disabled = false;
  }
}

function updateGraphics(jObj){
  // Update display if we have the expected data (containing door state at a minimum).
  if (jObj && jObj.doorState) {
    if (jObj.doorState == "Opened") {
      colorEl("doorOverlay", 0, 150, 0);
    } else {
      colorEl("doorOverlay", 255, 0, 0);
    }
  } else {
    // Reset all screen elements to their defaults to indicate no data available.
    colorEl("doorOverlay", 100, 100, 100);
  }
}

function updateEquipment(jObj) {
  // Update display if we have the expected data (containing door state at a minimum).
  if (jObj && jObj.doorState) {
    // Current Pack Status
    setHtml("doorState", jObj.doorState || "...");

    // Update special UI elements based on the latest data values.
    setButtonStates(jObj.smokeEnabled);

    // Connected Wifi Clients - Private AP vs. WebSocket
    setHtml("clientInfo", "AP Clients: " + (jObj.apClients || 0) + " / WebSocket Clients: " + (jObj.wsClients || 0));

    updateGraphics(jObj);
  }
}

function getStatus() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      // Update the equipment (text) display, which will also update graphical elements.
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
        // Device Info
        setHtml("buildDate", "Build: " + (jObj.buildDate || ""));
        setHtml("wifiName", jObj.wifiName || "");
        if ((jObj.wifiNameExt || "") != "" && (jObj.extAddr || "") != "" || (jObj.extMask || "") != "") {
          setHtml("extWifi", (jObj.wifiNameExt || "") + ": " + jObj.extAddr + " / " + jObj.extMask);
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
  xhttp.open("GET", "/config/device", true);
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

function runSmoke(msDuration) {
  sendCommand("/smoke/run?duration=" + parseInt(msDuration, 10));
}

function enableSmoke() {
  sendCommand("/smoke/enable");
}

function disableSmoke() {
  sendCommand("/smoke/disable");
}

function lightOn() {
  sendCommand("/light/on");
}

function lightOff() {
  sendCommand("/light/off");
}
)=====";
