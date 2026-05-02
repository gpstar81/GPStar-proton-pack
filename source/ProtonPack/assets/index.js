/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023-2026 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

var websocket;
var statusInterval;
var displayType = 1; // Display Preference: 0=Text-Only, 1=Graphical, 2=Both
var useAnimation = 1; // Enable/Disable special animations on the UI when using graphical display
var lastCyclotronState = ""; // Track previous cyclotron state to avoid redundant effect updates
var lastPackState = ""; // Track previous pack power state to differentiate idle updates
var musicTrackStart = 0,
  musicTrackMax = 0,
  musicTrackCurrent = 0,
  musicTrackList = [];

window.addEventListener("load", onLoad);

function onLoad(event) {
  document.getElementsByClassName("tablinks")[0].click();
  setDefaultOverlays(); // Set graphics to defaults.
  getDevicePrefs(); // Get all preferences.
  getNetworkInfo(); // Get networking info.        
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
  getNetworkInfo(); // Refresh network statistics.
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
    statusInterval = setInterval(function () {
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

if (!!window.EventSource) {
  // Create events for one-way communication.
  var source = new EventSource("/events");

  source.addEventListener(
    "open",
    function (e) {
      console.log("Server-Side Events connected");
    },
    false,
  );

  source.addEventListener(
    "error",
    function (e) {
      if (e.target.readyState != EventSource.OPEN) {
        console.log("Server-Side Events disconnected");
      }
    },
    false,
  );

  source.addEventListener(
    "debug",
    function (e) {
      if (e.data === undefined) return;
      console.log("Debug: ", e.data);
    },
    false,
  );
}

function getDevicePrefs() {
  // This is updated once per page load as it is not subject to frequent changes.
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status >= 200 && this.status < 300) {
      var jObj = JSON.parse(this.responseText);
      if (jObj) {
        if (jObj.songList && jObj.songList != "") {
          musicTrackList = jObj.songList.split("\n");
          updateTrackListing();
        }

        // Device Info
        setHtml("buildDate", "Build: " + (jObj.buildDate || ""));

        switch (jObj.audioVersion ?? 0) {
          case 0:
            setHtml("audioInfo", "No Audio Detected");
            break;
          case 1:
            setHtml("audioInfo", "WAV Trigger");
            break;
          case 100:
            setHtml("audioInfo", "GPStar Audio v100");
            break;
          default:
            setHtml("audioInfo", "GPStar Audio v" + (jObj.audioVersion || ""));
            break;
        }

        // Display Preference
        displayType = jObj.displayType ?? 1;
        switch (displayType) {
          case 0:
          default:
            // Text-Only Display
            hideEl("equipCRT");
            showEl("equipTXT");
            break;
          case 1:
            // Graphical Display (Default)
            showEl("equipCRT");
            hideEl("equipTXT");
            break;
          case 2:
            // Both graphical and text
            showEl("equipCRT");
            showEl("equipTXT");
            break;
        }
        useAnimation = jObj.useAnimation ?? 1;

        // Update stream mode dropdown if available
        if (jObj.streamModes && Array.isArray(jObj.streamModes)) {
          updateStreamModeListing(jObj.streamModes);
        }

        // microSD Warnings
        if (Boolean(jObj.audioCorrupt)) {
          alert("Corruption has been detected on the microSD card. Please reformat the card as FAT32 and reload audio files.");
        } else if (Boolean(jObj.audioOutdated)) {
          // The file count on the microSD card does not match firmware; alert the user.
          alert("Contents of microSD card do not match current firmware. Please make sure to update your microSD cards after updating firmware.");
        }
      }
    } else if (this.readyState == 4) {
      // Handle error responses
      handleStatus(this.responseText);
    }
  };
  xhttp.open("GET", "/config/device", true);
  xhttp.send();
}

function getNetworkInfo() {
  // Fetch network configuration and statistics from dedicated endpoint.
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status >= 200 && this.status < 300) {
      var jObj = JSON.parse(this.responseText);
      if (jObj) {
        // Display local AP network name
        if (jObj.localAP && jObj.localAP.ssid) {
          setHtml("wifiName", "Private Network: " + jObj.localAP.ssid);
        }
        
        // Display client counts
        var clientText = "AP Clients: " + (jObj.apClients ?? 0) + " / WebSocket Clients: " + (jObj.wsClients ?? 0);
        setHtml("clientInfo", clientText);
        
        // Display external WiFi info if connected
        if (jObj.extWifi && jObj.extWifi.enabled && jObj.extWifi.connected) {
          var extInfo = jObj.extWifi.ssid + ": " + jObj.extWifi.address + " / " + jObj.extWifi.subnet;
          setHtml("extWifi", extInfo);
        } else {
          setHtml("extWifi", ""); // Clear if not connected
        }
      }
    } else if (this.readyState == 4) {
      // Handle error responses
      console.log("Failed to fetch network info:", this.responseText);
    }
  };
  xhttp.open("GET", "/wifi/status", true);
  xhttp.send();
}

function removeOptions(selectElement) {
  var i,
    len = selectElement.options.length - 1;
  for (i = len; i >= 0; i--) {
    selectElement.remove(i);
  }
}

function updateStreamModeListing(streamModes) {
  var streamModeSelect = getEl("streamMode");
  if (streamModeSelect && streamModes.length > 0) {
    removeOptions(streamModeSelect); // Clear previous options.

    // Generate an option for each available stream mode.
    for (var i = 0; i < streamModes.length; i++) {
      var mode = streamModes[i];
      var opt = document.createElement("option");
      opt.setAttribute("value", mode.value);
      opt.appendChild(document.createTextNode(mode.label));
      streamModeSelect.appendChild(opt);
    }
  }
}

function updateTrackListing() {
  // Continue if start/end values are sane and something actually changed.
  if (musicTrackStart >= 500 && musicTrackMax < 4596 && musicTrackMax >= musicTrackStart) {
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
  setHtml("streamStatus", "");

  // Clear all glitch effects on page load
  clearAllEffects();
}

function clearAllEffects() {
  // Only apply effects if graphical display is enabled (displayType 1 or 2)
  if (displayType === 0) return;

  // Disable all CRT glitch effects
  var equipCRT = getEl("equipCRT");
  if (equipCRT) {
    equipCRT.setAttribute("effect-rgb", false);
    equipCRT.setAttribute("effect-scan", false);
    equipCRT.setAttribute("effect-flicker", false);
    equipCRT.setAttribute("effect-distort", false);
    equipCRT.setAttribute("effect-shake", false);
    equipCRT.setAttribute("effect-degauss", false);
    equipCRT.setAttribute("effect-icing", false);
  }
}

function disableActionButtons() {
  // Used to just disable all the buttons, for instance if pack de-synchronizes.
  disableEl("btnPackOff");
  disableEl("btnPackOn");
  disableEl("btnVent");
  disableEl("btnAttenuate");
}

function setButtonStates(statusObj) {
  // Assume all direct user actions are not possible, then override as necessary.
  disableActionButtons();

  // Consolidate common state checks into simple boolean values with user-friendly names.
  const packPowered = statusObj.pack === "Powered";
  const wandPowered = statusObj.wand === "Connected" && statusObj.wandPower === "Powered";
  const cyclotronNormal = statusObj.cyclotron === "Normal" || statusObj.cyclotron === "Active";
  const cyclotronOverheat = statusObj.cyclotron === "Warning" || statusObj.cyclotron === "Critical";
  const modeSuperHero = statusObj.modeID != 2;
  const ionswitchReady = statusObj.switch === "Ready";
  const firingActive = statusObj.firing === "Firing";
  const rampingActive = Boolean(statusObj.ramping);
  const vgmodeActive = Boolean(statusObj.vgMode);

  if ((packPowered || (!modeSuperHero && ionswitchReady)) && !firingActive) {
    // Can turn off the pack if already powered, but wand is not firing.
    enableEl("btnPackOff");
  }

  if ((modeSuperHero && !packPowered) || (!modeSuperHero && !ionswitchReady)) {
    // Can turn on the pack if not already powered (implies wand is not powered).
    enableEl("btnPackOn");
  }

  // Set the toggles to their current state.
  setToggle("toggleSmoke", statusObj.smoke);
  setToggle("toggleVibration", statusObj.vibration);
  setToggle("cyclotronDirection", statusObj.direction);

  // Set the theme drop-down to the current theme.
  switch (statusObj.themeID) {
    case 2:
    default:
      setValue("themes", "1984");
      break;
    case 3:
      setValue("themes", "1989");
      break;
    case 4:
      setValue("themes", "2021");
      break;
    case 5:
      setValue("themes", "2024");
      break;
  }

  // Start by enabling the firing mode selector; it will be disabled as necessary below.
  if (statusObj.canChangeStream) {
    enableEl("streamMode");
  } else {
    disableEl("streamMode");
  }

  switch (statusObj.wandMode) {
    case "Proton Stream":
      setValue("streamMode", "proton");
      break;
    case "Plasm System":
      setValue("streamMode", "slime");
      break;
    case "Dark Matter Gen.":
      setValue("streamMode", "stasis");
      break;
    case "Particle System":
      setValue("streamMode", "meson");
      break;
    case "Spectral Stream":
      setValue("streamMode", "spectral");
      break;
    case "Halloween":
      setValue("streamMode", "holiday/halloween");
      break;
    case "Christmas":
      setValue("streamMode", "holiday/christmas");
      break;
    case "Custom Stream":
      setValue("streamMode", "spectral/custom");
      break;
    default:
      // Invalid mode specified, so reset to Proton.
      setValue("streamMode", "proton");
      break;
  }

  if (packPowered || wandPowered || rampingActive) {
    // If either the pack or wand is powered, we cannot change themes.
    disableEl("themes");
  } else {
    // Only allow theme change if both pack and wand are off.
    enableEl("themes");
  }

  if (packPowered && cyclotronNormal && !firingActive) {
    // Can only use manual vent if pack is not already venting, and not currently firing.
    // Additionally, the Cyclotron is not in the Warning, Critical, or Recovery states.
    enableEl("btnVent");
  }

  if (cyclotronOverheat && wandPowered) {
    // Can only attenuate if cyclotron is in the pre-overheat (Warning, Critical) states.
    enableEl("btnAttenuate");
  }
}

function getStreamColor(cMode, iTheme, iCustomVal = 200, iCustomSat = 254) {
  var color = [0, 0, 0];

  // Use this to do our colour-change for spectral streams.
  var tickSeconds = new Date().getSeconds();

  switch (cMode) {
    case "Plasm System":
      if (iTheme == 3) {
        // Pink
        color[0] = 200;
        color[2] = 180;
      } else {
        // Dark Green
        color[1] = 80;
      }
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
    case "Halloween":
      if (tickSeconds % 2) {
        // Orange
        color[0] = 255;
        color[1] = 140;
      } else {
        // Purple
        color[0] = 200;
        color[2] = 240;
      }
      break;
    case "Christmas":
      if (tickSeconds % 2) {
        // Red
        color[0] = 180;
      } else {
        // Green
        color[1] = 180;
      }
      break;
    case "Spectral Stream":
      switch (tickSeconds % 8) {
        case 0:
        default:
          // Red
          color[0] = 180;
          break;
        case 1:
          // Orange
          color[0] = 255;
          color[1] = 140;
          break;
        case 2:
          // Yellow
          color[0] = 240;
          color[1] = 220;
          break;
        case 3:
          // Green
          color[1] = 180;
          break;
        case 4:
          // Light Blue
          color[1] = 60;
          color[2] = 255;
          break;
        case 5:
          // Blue
          color[2] = 180;
          break;
        case 6:
          // Indigo
          color[0] = 90;
          color[2] = 240;
          break;
        case 7:
          // Purple
          color[0] = 200;
          color[2] = 240;
          break;
      }
      break;
    case "Custom Stream":
    default:
      // Proton Stream(s) as Red
      color[0] = 180;
      break;
  }

  return color;
}

function updateBars(iPower, cMode, iTheme) {
  var color = getStreamColor(cMode, iTheme);
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

function updateCyclotronGraphics(packState, cyclotronState) {
  // Skip all graphical updates if display is text-only.
  if (displayType === 0) {
    return;
  }

  // Skip update if state hasn't changed (prevents unnecessary animation restarts).
  if (lastCyclotronState === cyclotronState && lastPackState === packState) {
    return;
  }
  lastCyclotronState = cyclotronState;
  lastPackState = packState;

  // Update the overlay color and blink state based on cyclotron status.
  // Also update the equipCRT border/text color to match.
  var equipCRT = getEl("equipCRT");
  if (equipCRT) {
    // Remove all existing "state-*" classes from the equipment element.
    equipCRT.className = equipCRT.className.replace(/state-\w+/g, "").trim();
  }

  // Clear all CRT glitch effects before applying new state.
  clearAllEffects();

  // Available Effects:
  //   effect-rgb: RGB color separation (chromatic aberration with red/cyan shift)
  //   effect-scan: Horizontal scan jitter (left/right shaking)
  //   effect-flicker: Brightness flicker (slow opacity/brightness changes)
  //   effect-distort: Clip distortion (sections disappear/reappear)
  //   effect-shake: Screen shake (full screen vibration)
  //   effect-degauss: Degaussing wave (slow rolling magenta/cyan/yellow interference band)

  switch (cyclotronState) {
    case "Active":
      colorEl("cycOverlay", 255, 230, 0);
      blinkEl("cycOverlay", false);
      if (equipCRT) {
        // Set a yellow border/text for the CRT
        equipCRT.classList.add("state-active");

        if (useAnimation) {
          // Enable multiple effects for active state
          equipCRT.setAttribute("effect-degauss", true);
          equipCRT.setAttribute("effect-flicker", true);
        }
      }
      break;
    case "Warning":
      colorEl("cycOverlay", 255, 100, 0);
      blinkEl("cycOverlay", false);
      if (equipCRT) {
        equipCRT.classList.add("state-warning"); // Set an orange border/text for the CRT

        if (useAnimation) {
          // Enable multiple effects for warning state
          equipCRT.setAttribute("effect-rgb", true);
          equipCRT.setAttribute("effect-flicker", true);
        }
      }
      break;
    case "Critical":
      colorEl("cycOverlay", 255, 0, 0);
      blinkEl("cycOverlay", true);
      if (equipCRT) {
        // Set a red border/text for the CRT
        equipCRT.classList.add("state-critical");

        if (useAnimation) {
          // Enable multiple effects for critical state
          equipCRT.setAttribute("effect-rgb", true);
          equipCRT.setAttribute("effect-shake", true);
        }
      }
      break;
    case "Recovery":
      colorEl("cycOverlay", 0, 0, 255);
      blinkEl("cycOverlay", false);
      if (equipCRT) {
        equipCRT.classList.add("state-recovery"); // Set a blue border/text for the CRT

        if (useAnimation) {
          equipCRT.setAttribute("effect-flicker", true); // Enable subtle flicker for recovery state
        }
      }
      break;
    default:
      if (packState == "Powered") {
        // Also covers cyclotron state of "Normal"
        colorEl("cycOverlay", 0, 150, 0); // Set a brighter green border/text for the CRT

        if (useAnimation) {
          equipCRT.setAttribute("effect-degauss", true); // Enable a degauss wave for active state
        }
      } else {
        colorEl("cycOverlay", 100, 100, 100);
      }
      blinkEl("cycOverlay", false);
      break;
  }
}

function updateGraphics(jObj) {
  // Update display if we have the expected data (containing mode and theme at a minimum).
  if (jObj && jObj.mode && jObj.theme) {
    var color = getStreamColor(jObj.wandMode || "", jObj.themeID ?? 0);

    var header = ""; // Used for the title on the display.
    switch (jObj.modeID ?? 1) {
      case 1:
        header = "Upgraded"; // aka. Super Hero
        break;
      case 2:
        header = "Standard"; // aka. Mode Original
        break;
      default:
        header = "- Disabled -";
        break;
    }
    switch (jObj.themeID ?? 0) {
      case 2:
        header += " / V1.9.84"; // 1984
        break;
      case 3:
        header += " / V1.9.89"; // 1989
        break;
      case 4:
        header += " / V2.0.21"; // 2021
        break;
      case 5:
        header += " / V2.0.24"; // 2024
        break;
      default:
        header += " / V0.0.00"; // Unknown
        break;
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

    // Update the cyclotron and other graphics based on current state.
    if (jObj.lockout == true) {
      // Clear all CRT glitch effects before applying new state.
      clearAllEffects();

      // During a wand mash lockout we want to show a visual result in the UI.
      if (jObj.themeID == 5) {
        // For the Frozen Empire theme we want a effect that resembles cracked ice.
        colorEl("cycOverlay", 0, 0, 255);
        equipCRT.classList.add("state-recovery");
        equipCRT.setAttribute("effect-icing", true);
      } else {
        // Otherwise we want to show a visual tearing of the UI due to the user input.
        colorEl("cycOverlay", 255, 0, 0);
        equipCRT.classList.add("state-critical");
        equipCRT.setAttribute("effect-distort", true);
      }

      // Reset state tracking to force update when lockout ends
      lastCyclotronState = "";
      lastPackState = "";
    } else {
      // Otherwise update the cyclotron graphics based on normal state values.
      updateCyclotronGraphics(jObj.pack, jObj.cyclotron);
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
      setHtml("streamStatus", jObj.wandMode || "");
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
      if (parseFloat(jObj.wandAmps ?? 0) > 0.01) {
        // If we have a non-zero amperage reading, display that as it means a stock wand is attached.
        setHtml("streamStatus", "Stream: " + parseFloat(jObj.wandAmps ?? 0).toFixed(2) + " GW");
      } else {
        // Otherwise we consider a wand to be "disengaged" as it could be inactive or detached.
        setHtml("streamStatus", "- Disengaged -");
      }
      hideEl("barrelOverlay");
      colorEl("safetyOverlay", 100, 100, 100);
    }

    if (parseFloat(jObj.battVoltage ?? 0) > 1) {
      // Voltage should typically be ~5.0 at idle and >=4.2 under normal use; anything below that indicates a possible problem.
      setHtml("battVoltage", "Output:<br/>" + parseFloat(jObj.battVoltage ?? 0).toFixed(2) + " GeV");
      if (jObj.battVoltage < 4.2) {
        colorEl("boostOverlay", 255, 0, 0);
      } else {
        colorEl("boostOverlay", 0, 150, 0);
      }
    } else {
      setHtml("battVoltage", "0.00 GeV");
    }

    if (jObj.cyclotron && !jObj.cyclotronLid) {
      showEl("cyclotronLid");
    } else {
      hideEl("cyclotronLid");
    }
  } else if (getHtml("equipTitle") != "- Desynchronized -") {
    // Reset all screen elements to their defaults to indicate no data available.
    setDefaultOverlays();
    disableActionButtons();

    // Set special text elements based on the current equipment state.
    setHtml("equipTitle", "- Desynchronized -");
    setHtml("streamStatus", "- Disengaged -");
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
    if (jObj.cyclotron && !jObj.cyclotronLid) {
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
      updateBars(jObj.power ?? 0, jObj.wandMode || "", jObj.themeID ?? 0);
    } else {
      // Default to empty values when wand is not present.
      setHtml("wandPower", "...");
      setHtml("wandMode", "...");
      setHtml("safety", "...");
      setHtml("power", "...");
      setHtml("firing", "...");
      updateBars(0, "", 0);
    }

    if (jObj.battVoltage) {
      // Voltage should typically be <5.0 but >4.2 under normal use; anything below that indicates high drain.
      setHtml("battVoltageTXT", parseFloat(jObj.battVoltage ?? 0).toFixed(2));
      if (jObj.battVoltage < 4.2) {
        setHtml("battHealth", "&#129707;"); // Draining Battery
      } else {
        setHtml("battHealth", "&#128267;"); // Healthy Battery
      }
    } else {
      setHtml("battHealth", "");
    }

    // Volume Information
    setHtml("masterVolume", (jObj.volMaster ?? 0) + "%");
    if ((jObj.volMaster ?? 0) == 0) {
      setHtml("masterVolume", "Min");
    }
    setHtml("effectsVolume", (jObj.volEffects ?? 0) + "%");
    if ((jObj.volEffects ?? 0) == 0) {
      setHtml("effectsVolume", "Min");
    }
    setHtml("musicVolume", (jObj.volMusic ?? 0) + "%");
    if ((jObj.volMusic ?? 0) == 0) {
      setHtml("musicVolume", "Min");
    }
    setToggle("toggleMute", jObj.volMuted);

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
    setToggle("toggleLoop", jObj.musicLooping);
    setToggle("toggleShuffle", jObj.musicShuffled);

    // Update special UI elements based on the latest data values.
    setButtonStates(jObj);

    // Update the current track info.
    musicTrackStart = jObj.musicStart || 0;
    musicTrackMax = jObj.musicEnd || 0;
    if (musicTrackCurrent != (jObj.musicCurrent ?? 0)) {
      musicTrackCurrent = jObj.musicCurrent || 0;
      updateTrackListing();
    }
  }

  // Always run logic to update the graphics, even if we don't have the expected data.
  updateGraphics(jObj);
}

function packToggleIonArm(event) {
  event.stopPropagation(); // Prevent bubbling up to the parent element.

  // Allow the user to toggle the pack power state based on button states.
  // Mimics the behavior of the Ion Arm switch on the physical pack.
  if (getEl("btnAttenuate").disabled == true) {
    // Only allowed if in a state where attenuation is NOT possible.
    if (getEl("btnPackOn").disabled == false) {
      packOn(); // Button enabled, allow power-on event.
    } else if (getEl("btnPackOff").disabled == false) {
      packOff(); // Button enabled, allow power-off event.
    }
  }
}

function packVentSilent(event) {
  event.stopPropagation(); // Prevent bubbling up to the parent element.

  // Only perform API call if the Vent button is enabled.
  // This avoids an alert box when venting is not possible.
  if (getEl("btnVent").disabled == false) {
    // Not already venting, not currently firing, and cyclotron is normal.
    packVent();
  }
}

function packAttenuateSilent(event) {
  // Only perform API call if the Attenuate button is enabled.
  // This avoids an alert box when attenuation is not possible.
  if (getEl("btnAttenuate").disabled == false) {
    // Cyclotron is in the pre-overheat (Warning, Critical) states.
    packAttenuate();
  }
}

function packRibbonCableToggleSilent(event) {
  event.stopPropagation(); // Prevent bubbling up to the parent element.

  // Allow the user to toggle the ribbon cable alarm state based on button states.
  // Mimics the behavior of the Ribbon Cable switch on the physical pack.
  if (getEl("btnAttenuate").disabled == true) {
    // Only allowed if in a state where attenuation is NOT possible.
    if (getHtml("cable") == "Disconnected") {
      cableOn(); // Element visible, allow cable attach event.
    } else if (getHtml("cable") == "Connected") {
      cableOff(); // Element hidden, allow cable detach event.
    }
  }
}
