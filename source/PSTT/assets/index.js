/**
 *   GPStar Proton Stream Target Trainer
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

// Target health thresholds (loaded from device config)
var targetMaxHealth = 1000;
var targetLowHealth = 440;
var targetExtremeLowHealth = 220;

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
  // Create events for the sensor readings.
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

  source.addEventListener(
    "irData",
    function (e) {
      if (e.data === undefined) return;

      try {
        // Parse JSON event data
        const eventData = JSON.parse(e.data);

        // Update health bar
        updateHealthBar(eventData);

        // Update hit log for relevant events
        if (eventData.eventType === "target_hit" || eventData.eventType === "target_defeated" || eventData.eventType === "ir_received") {
          addHitLogEntry(eventData);
        }
      } catch (err) {
        // If not JSON, display as plain text for backwards compatibility
        let logContainer = getEl("dataConsole");

        // Clear default "Awaiting IR data..." text on first message
        if (logContainer.children.length === 0 && logContainer.textContent.includes("Awaiting")) {
          logContainer.textContent = "";
        }

        if (logContainer.children.length > 0) {
          logContainer.insertAdjacentHTML("afterbegin", '<div class="hit-entry">' + e.data + "</div>");
        } else {
          logContainer.innerHTML = '<div class="hit-entry">' + e.data + "</div>";
        }
        limitConsoleEntries();
      }
    },
    false,
  );
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

function getDevicePrefs() {
  // This is updated once per page load as it is not subject to frequent changes.
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status >= 200 && this.status < 300) {
      var jObj = JSON.parse(this.responseText);
      if (jObj) {
        // Store target health thresholds for health bar calculations
        if (jObj.maxHealth) targetMaxHealth = jObj.maxHealth;
        if (jObj.lowHealth) targetLowHealth = jObj.lowHealth;
        if (jObj.extremeLowHealth) targetExtremeLowHealth = jObj.extremeLowHealth;

        if (jObj.songList && jObj.songList != "") {
          musicTrackList = jObj.songList.split("\n");
          updateTrackListing();
        }

        // Device Info
        setHtml("buildDate", "Build: " + (jObj.buildDate || ""));
        setHtml("wifiName", "Private Network: " + jObj.wifiName || "");
        if ((jObj.wifiNameExt || "") != "" && (jObj.extAddr || "") != "" && (jObj.extMask || "") != "") {
          setHtml("extWifi", (jObj.wifiNameExt || "") + ": " + jObj.extAddr + " / " + jObj.extMask);
        }
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
      }
    } else if (this.readyState == 4) {
      // Handle error responses
      handleStatus(this.responseText);
    }
  };
  xhttp.open("GET", "/config/device", true);
  xhttp.send();
}

function disableActionButtons() {
  // Used to just disable all the buttons, for instance if pack de-synchronizes.
  disableEl("btnPsttDrop");
  disableEl("btnPsttReset");
}

function setButtonStates(statusObj) {
  // Assume all direct user actions are not possible, then override as necessary.
  disableActionButtons();

  // Consolidate common state checks into simple boolean values with user-friendly names.
  const targetDefeated = !!(statusObj.targetDefeated ?? 0); // Convert to boolean, handling undefined/null.

  if (targetDefeated) {
    // Can reset the target if it is defeated.
    enableEl("btnPsttReset");
  } else {
    // Can force the target to be defeated if it is ready.
    enableEl("btnPsttDrop");
  }
}

// Update the health bar display based on current health
function updateHealthBar(eventData) {
  if (!eventData.health || !eventData.maxHealth) return;

  const healthPercent = eventData.healthPercent || (eventData.health / eventData.maxHealth) * 100;
  const healthBar = getEl("healthBar");
  const healthText = getEl("healthText");

  if (healthBar && healthText) {
    // Update bar height
    healthBar.style.height = healthPercent + "%";

    // Calculate threshold percentages based on configured values
    const extremeLowPercent = (targetExtremeLowHealth / targetMaxHealth) * 100;
    const lowPercent = (targetLowHealth / targetMaxHealth) * 100;

    // Update color based on dynamic thresholds
    healthBar.className = "health-bar";
    if (healthPercent <= extremeLowPercent) {
      healthBar.classList.add("red");
    } else if (healthPercent <= lowPercent) {
      healthBar.classList.add("orange");
    } else if (healthPercent <= 70) {
      healthBar.classList.add("amber");
    }

    // Update text display
    healthText.textContent = Math.round(healthPercent) + "%";
  }
}

// Add a new entry to the hit log
function addHitLogEntry(eventData) {
  const logContainer = getEl("dataConsole");
  if (!logContainer) return;

  // Clear default "Awaiting IR data..." text on first message
  if (logContainer.children.length === 0 && logContainer.textContent.includes("Awaiting")) {
    logContainer.textContent = "";
  }

  let entryClass = "hit-entry";
  let entryText = "";
  let showRxMessage = false;

  // Format based on event type using template literals for cleaner assembly (instead of string concatenation).
  if (eventData.eventType === "target_hit") {
    // Indicate a hit from a device on the target.
    entryClass += " damage";
    entryText = `<strong>HIT:</strong> ${eventData.deviceType || "UNKNOWN"} [${eventData.deviceId || "?"}]<br/>`;
    entryText += `Stream: ${eventData.streamType || "?"} | Power: ${eventData.powerLevel || "?"}<br/>`;
    entryText += `Damage: <strong>${eventData.damage || 0}</strong><br/>`;
  } else if (eventData.eventType === "target_defeated") {
    // Indicate the final takedown of the target via direct hit.
    entryClass += " defeated";
    entryText = `<strong>DEFEATED:</strong> ${eventData.deviceType || "UNKNOWN"} [${eventData.deviceId || "?"}]<br/>`;
    entryText += `Stream: ${eventData.streamType || "?"} | Power: ${eventData.powerLevel || "?"}<br/>`;
    entryText += `Final Damage: <strong>${eventData.damage || 0}</strong>`;

    // Ensure health bar is updated to reflect defeat (0% health)
    updateHealthBar({
      health: 0,
      maxHealth: eventData.maxHealth || targetMaxHealth,
      healthPercent: 0,
    });
  } else if (eventData.eventType === "ir_received" && showRxMessage) {
    // Return raw data if enabled, useful for debugging.
    entryText = `<strong>IR RX:</strong> ${eventData.deviceType || "UNKNOWN"} [${eventData.deviceId || "?"}]`;
    if (eventData.message) {
      entryText += `<br/>${eventData.message}`;
    }
  }

  // Insert message at the top (latest first).
  if (entryText) {
    const newEntry = document.createElement("div");
    newEntry.className = entryClass;
    newEntry.innerHTML = entryText;

    if (logContainer.children.length > 0) {
      logContainer.insertBefore(newEntry, logContainer.firstChild);
    } else {
      logContainer.appendChild(newEntry);
    }

    // Limit number of entries based on container height.
    limitConsoleEntries();

    // Fade out and remove entry after 1 minute
    setTimeout(() => {
      newEntry.style.transition = "opacity 1s";
      newEntry.style.opacity = "0";
      setTimeout(() => {
        if (newEntry.parentNode === logContainer) {
          logContainer.removeChild(newEntry);
        }
      }, 1000); // Wait for fade animation to complete
    }, 60000); // 1 minute
  }
}

// Remove entries that exceed container height or max count
function limitConsoleEntries() {
  const logContainer = getEl("dataConsole");
  if (!logContainer) return;

  const maxEntries = 8; // Maximum number of entries to keep
  const children = logContainer.children;

  // Remove excess entries
  while (children.length > maxEntries) {
    logContainer.removeChild(logContainer.lastChild);
  }
}

function updateEquipment(jObj) {
  // Update display if we have the expected data (containing mode and theme at a minimum).
  if (jObj?.targetDefeated != null) {
    // Current PSTT Status

    // Update special UI elements based on the latest data values.
    setButtonStates(jObj);

    // Update the current track info.
    //musicTrackStart = jObj.musicStart || 0;
    //musicTrackMax = jObj.musicEnd || 0;
    //if (musicTrackCurrent != (jObj.musicCurrent ?? 0)) {
    //  musicTrackCurrent = jObj.musicCurrent || 0;
    //  updateTrackListing();
    //}

    // Connected Wifi Clients - Private AP vs. WebSocket
    setHtml("clientInfo", "AP Clients: " + (jObj?.apClients ?? 0) + " / WebSocket Clients: " + (jObj?.wsClients ?? 0));
  }

  // Always run logic to update the graphics, even if we don't have the expected data.
  //updateGraphics(jObj);
}
