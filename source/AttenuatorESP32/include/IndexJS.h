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

const char INDEXJS_page[] PROGMEM = R"=====(
var websocket;
var statusInterval;
var musicTrackStart = 0, musicTrackMax = 0, musicTrackCurrent = 0, musicTrackList = [];

window.addEventListener("load", onLoad);

function onLoad(event) {
  document.getElementsByClassName("tablinks")[0].click();
  setDefaultOverlays(); // Set graphics to defaults.
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
        if (jObj.songList && jObj.songList != "") {
          musicTrackList = jObj.songList.split("\n");
          updateTrackListing();
        }

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

function setDefaultOverlays() {
  // Reset colors on all graphic overlay elements to a default state.
  colorEl("ionOverlay", 255, 0, 0);
  colorEl("boostOverlay", 100, 100, 100);
  colorEl("pcellOverlay", 100, 100, 100);
  hideEl("cableOverlay");
  blinkEl("cableOverlay", false);
  colorEl("cycOverlay", 100, 100, 100);
  blinkEl("cycOverlay", false);
  hideEl("cyclotronLid");
  colorEl("filterOverlay", 100, 100, 100);
  blinkEl("filterOverlay", false);
  hideEl("barrelOverlay");
  blinkEl("barrelOverlay", false);
  setHtml("powerLevel", "");
  colorEl("safetyOverlay", 100, 100, 100);
  setHtml("battVoltage", "");
  hideEl("cyclotronLid");

  // Clear special text elements until we know the system state.
  setHtml("equipTitle", "");
  setHtml("streamMode", "");
}

function disableActionButtons() {
  // Used to just disable all the buttons, for instance if pack desyncs
  getEl("btnPackOff").disabled = true;
  getEl("btnPackOn").disabled = true;
  getEl("btnVent").disabled = true;
  getEl("btnAttenuate").disabled = true;
  //getEl("btnLOStart").disabled = true;
  //getEl("btnLOCancel").disabled = true;
}

function setButtonStates(mode, pack, wand, cyclotron, ionswitch, firing) {
  // Assume all direct user actions are not possible, then override as necessary.
  disableActionButtons();

  if ((pack == "Powered" || (mode == "Original" && ionswitch == "Ready")) && wand != "Powered") {
    // Can only turn off the pack, so long as the wand is not powered.
    getEl("btnPackOff").disabled = false;
  }

  if ((mode == "Super Hero" && pack != "Powered") || (mode == "Original" && ionswitch != "Ready")) {
    // Can turn on the pack if not already powered (implies wand is not powered).
    getEl("btnPackOn").disabled = false;
  }

  if (pack == "Powered" && (cyclotron == "Normal" || cyclotron == "Active") && firing != "Firing") {
    // Can only use manual vent if pack is not already venting, and not currently firing.
    // eg. Cyclotron is not in the Warning, Critical, or Recovery states.
    getEl("btnVent").disabled = false;
  }

  if (pack == "Powered" && (cyclotron == "Normal" || cyclotron == "Active") && wand != "Powered") {
    // Can only use manual lockout if pack is on, not already venting, and wand is off.
    //getEl("btnLOStart").disabled = false;
    //getEl("btnLOCancel").disabled = false;
  }

  if (cyclotron == "Warning" || cyclotron == "Critical") {
    // Can only attenuate if cyclotron is in the pre-overheat states.
    getEl("btnAttenuate").disabled = false;
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
  // Update display if we have the expected data (containing mode and theme at a minimum).
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
    setHtml("equipTitle", header);

    if (jObj.switch == "Ready") {
      colorEl("ionOverlay", 0, 150, 0);
    } else {
      colorEl("ionOverlay", 255, 0, 0);
    }

    if (jObj.pack == "Powered") {
      colorEl("pcellOverlay", 0, 150, 0);
    } else {
      colorEl("pcellOverlay", 100, 100, 100);
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
        colorEl("cycOverlay", 255, 230, 0);
        blinkEl("cycOverlay", false);
        break;
      case "Warning":
        colorEl("cycOverlay", 255, 100, 0);
        blinkEl("cycOverlay", false);
        break;
      case "Critical":
        colorEl("cycOverlay", 255, 0, 0);
        blinkEl("cycOverlay", true);
        break;
      case "Recovery":
        colorEl("cycOverlay", 0, 0, 255);
        blinkEl("cycOverlay", false);
        break;
      default:
        if (jObj.pack == "Powered") {
          // Also covers cyclotron state of "Normal"
          colorEl("cycOverlay", 0, 150, 0);
        } else {
          colorEl("cycOverlay", 100, 100, 100);
        }
        blinkEl("cycOverlay", false);
    }

    if (jObj.pack == "Powered") {
      if (jObj.temperature == "Venting") {
        colorEl("filterOverlay", 255, 0, 0);
        blinkEl("filterOverlay", true);
      } else {
        colorEl("filterOverlay", 0, 150, 0);
        blinkEl("filterOverlay", false);
      }
    } else {
      colorEl("filterOverlay", 100, 100, 100);
      blinkEl("filterOverlay", false);
    }

    // Current Wand Status
    if (jObj.wand == "Connected") {
      // Only update if the wand is physically connected to the pack.
      setHtml("streamMode", jObj.wandMode || "");
      setHtml("powerLevel", "L-" + (jObj.power || "0"));
      showEl("barrelOverlay");
      colorEl("barrelOverlay", color[0], color[1], color[2], "0." + Math.round(jObj.power * 1.2, 10));
      if (jObj.firing == "Firing") {
        blinkEl("barrelOverlay", true);
      } else {
        blinkEl("barrelOverlay", false);
      }

      if (jObj.wandPower == "Powered") {
        if (jObj.safety == "Safety Off") {
          colorEl("safetyOverlay", 0, 150, 0);
        } else {
          colorEl("safetyOverlay", 255, 0, 0);
        }
      } else {
        colorEl("safetyOverlay", 100, 100, 100);
      }
    } else {
      // Wand is considered "disconnected" as no serial communication exists.
      setHtml("powerLevel", "");
      if (parseFloat(jObj.wandAmps || 0) > 0.01) {
        // If we have a non-zero amperage reading, display that as it means a stock wand is attached.
        setHtml("streamMode", "Stream: " + parseFloat((jObj.wandAmps || 0)).toFixed(2) + " GW");
      } else {
        // Otherwise we consider a wand to be "disengaged" as it could be inactive or detached.
        setHtml("streamMode", "- Disengaged -");
      }
      hideEl("barrelOverlay");
      colorEl("safetyOverlay", 100, 100, 100);
    }

    if (parseFloat(jObj.battVoltage || 0) > 1) {
      // Voltage should typically be ~5.0 at idle and >=4.2 under normal use; anything below that indicates a possible problem.
      setHtml("battVoltage", "Output:<br/>" + parseFloat((jObj.battVoltage || 0)).toFixed(2) + " GeV");
      if (jObj.battVoltage < 4.2) {
        colorEl("boostOverlay", 255, 0, 0);
      } else {
        colorEl("boostOverlay", 0, 150, 0);
      }
    } else {
      setHtml("battVoltage", "0.00 GeV");
    }

    if(jObj.cyclotron && !jObj.cyclotronLid) {
      showEl("cyclotronLid");
    } else {
      hideEl("cyclotronLid");
    }
  } else if (getText("equipTitle") != "- Desynchronized -") {
    // Reset all screen elements to their defaults to indicate no data available.
    setDefaultOverlays();
    disableActionButtons();

    // Set special text elements based on the current equipment state.
    setHtml("equipTitle", "- Desynchronized -");
    setHtml("streamMode", "- Disengaged -");
  }
}

function updateEquipment(jObj) {
  // Update display if we have the expected data (containing mode and theme at a minimum).
  if (jObj && jObj.mode && jObj.theme) {
    // Current Pack Status
    setHtml("mode", jObj.mode || "...");
    setHtml("theme", jObj.theme || "...");
    setHtml("pack", jObj.pack || "...");
    setHtml("switch", jObj.switch || "...");
    setHtml("cable", jObj.cable || "...");
    if(jObj.cyclotron && !jObj.cyclotronLid) {
      setHtml("cyclotron", (jObj.cyclotron || "") + " &#9762;");
    } else {
      setHtml("cyclotron", jObj.cyclotron || "...");
    }
    setHtml("temperature", jObj.temperature || "...");
    setHtml("wand", jObj.wand || "...");

    // Current Wand Status
    if (jObj.wand == "Connected") {
      // Only update if the wand is physically connected to the pack.
      setHtml("wandPower", jObj.wandPower || "...");
      setHtml("wandMode", jObj.wandMode || "...");
      setHtml("safety", jObj.safety || "...");
      setHtml("power", jObj.power || "...");
      setHtml("firing", jObj.firing || "...");
      updateBars(jObj.power || 0, jObj.wandMode || "");
    } else {
      // Default to empty values when wand is not present.
      setHtml("wandPower", "...");
      setHtml("wandMode", "...");
      setHtml("safety", "...");
      setHtml("power", "...");
      setHtml("firing", "...");
      updateBars(0, "");
    }

    if (jObj.battVoltage) {
      // Voltage should typically be <5.0 but >4.2 under normal use; anything below that indicates high drain.
      setHtml("battVoltageTXT", parseFloat((jObj.battVoltage || 0)).toFixed(2));
      if (jObj.battVoltage < 4.2) {
        setHtml("battHealth", "&#129707;"); // Draining Battery
      } else {
        setHtml("battHealth", "&#128267;"); // Healthy Battery
      }
    } else {
      setHtml("battHealth", "");
    }

    // Volume Information
    setHtml("masterVolume", (jObj.volMaster || 0) + "%");
    if ((jObj.volMaster || 0) == 0) {
      setHtml("masterVolume", "Min");
    }
    setHtml("effectsVolume", (jObj.volEffects || 0) + "%");
    if ((jObj.volEffects || 0) == 0) {
      setHtml("effectsVolume", "Min");
    }
    setHtml("musicVolume", (jObj.volMusic || 0) + "%");
    if ((jObj.volMusic || 0) == 0) {
      setHtml("musicVolume", "Min");
    }

    // Music Playback Status
    if (jObj.musicPlaying && !jObj.musicPaused) {
      // If music is playing (but not paused), show that status.
      setHtml("playbackStatus", "Music Playing");
    } else if (jObj.musicPaused) {
      // If music is playing AND paused, show that status.
      setHtml("playbackStatus", "Music Paused");
    } else {
      // If no music is playing or paused, show a default message.
      setHtml("playbackStatus", "No Music Playing");
    }

    // Update special UI elements based on the latest data values.
    setButtonStates(jObj.mode, jObj.pack, jObj.wandPower, jObj.cyclotron, jObj.switch, jObj.firing);

    // Update the current track info.
    musicTrackStart = jObj.musicStart || 0;
    musicTrackMax = jObj.musicEnd || 0;
    if (musicTrackCurrent != (jObj.musicCurrent || 0)) {
      musicTrackCurrent = jObj.musicCurrent || 0;
      updateTrackListing();
    }

    // Connected Wifi Clients - Private AP vs. WebSocket
    setHtml("clientInfo", "AP Clients: " + (jObj.apClients || 0) + " / WebSocket Clients: " + (jObj.wsClients || 0));
  }

  // Always run logic to update the graphics, even if we don't have the expected data.
  updateGraphics(jObj);
}
)=====";
