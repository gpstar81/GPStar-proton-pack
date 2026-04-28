/* Combined JavaScript - Generated automatically */
/* DO NOT EDIT - Modify source files instead */

/* === api.js === */
/** General API Helpers - Common functions for device control **/

function sendCommand(apiUri) {
  // Sends an action command to the server (device) using a PUT request.
  // These commands have no response data, so we just handle the status.
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4) {
      handleStatus(this.responseText);
    }
  };
  xhttp.open("PUT", apiUri, true);
  xhttp.send();
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

function musicSelect(caller) {
  // Change the music track by selected option: /music/select?track=<#>
  sendCommand("/music/select?track=" + caller.value);
}

function musicPrev() {
  sendCommand("/music/prev");
}

function musicNext() {
  sendCommand("/music/next");
}

function handleToggle(el, apiOn, apiOff) {
  if (el._lockout) return;
  el._lockout = true;

  const switchEl = el.parentElement.querySelector(".switch");

  function onTransitionEnd(e) {
    if (e.propertyName === "right") {
      sendCommand(el.checked ? apiOn : apiOff);
      el._lockout = false;
      switchEl.removeEventListener("transitionend", onTransitionEnd);
    }
  }

  switchEl.addEventListener("transitionend", onTransitionEnd);
}

function toggleMute(el) {
  handleToggle(el, "/volume/mute", "/volume/unmute");
}

function musicLoop(el) {
  handleToggle(el, "/music/loop/one", "/music/loop/all");
}

function musicShuffle(el) {
  handleToggle(el, "/music/shuffle/on", "/music/shuffle/off");
}

function getStatus(callbackFunc) {
  // This function expects a JSON response from the server which must be parsed and sent to the callback function.
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status >= 200 && this.status < 300) {
      if (callbackFunc && typeof callbackFunc === "function") {
        // If a callback function is provided, call it with the JSON response.
        callbackFunc(JSON.parse(this.responseText));
      } else {
        // Otherwise display a message that no callback function was provided.
        console.warn("No callback function provided for getStatus response.");
      }
    }
  };
  xhttp.open("GET", "/status", true);
  xhttp.send();
}

function doRestart() {
  // A special command which requires user confirmation before proceeding.
  if (confirm("Are you sure you wish to restart the serial device?")) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status >= 200 && this.status < 300) {
        // Reload the page after 2 seconds.
        setTimeout(function () {
          window.location.reload();
        }, 2000);
      }
    };
    xhttp.open("DELETE", "/restart", true);
    xhttp.send();
  }
}

/* === dom.js === */
/** DOM Manipulation Utilities - Device-agnostic helpers for element access and manipulation **/

function getEl(id) {
  return document.getElementById(id);
}

function getInt(id) {
  return parseInt(getValue(id) ?? 0, 10);
}

function getFloat(id) {
  return parseFloat(getValue(id) ?? 0);
}

function getText(id) {
  return (getValue(id) || "").trim();
}

function getHtml(id) {
  return (getEl(id).innerHTML);
}

function getToggle(id) {
  var el = getEl(id);
  return el && el.checked ? true : false;
}

function getValue(id) {
  var el = getEl(id);
  return el && typeof el.value !== "undefined" ? el.value : null;
}

function setHtml(id, value) {
  getEl(id).innerHTML = value || "";
}

function setToggle(id, value) {
  var el = getEl(id);
  if (el) {
    if (el._lockout != true) el.checked = value === true;
  }
}

function setValue(id, value) {
  var el = getEl(id);
  if (el) el.value = value;
}

function hideEl(id) {
  getEl(id).style.display = "none";
}

function showEl(id) {
  getEl(id).style.display = "block";
}

function disableEl(id) {
  getEl(id).disabled = true;
}

function enableEl(id) {
  getEl(id).disabled = false;
}

function colorEl(id, red, green, blue, alpha = 0.5) {
  getEl(id).style.backgroundColor = "rgba(" + red + ", " + green + ", " + blue + ", " + alpha + ")";
}

function blinkEl(id, state) {
  if (state) {
    getEl(id).classList.add("blinking");
  } else {
    getEl(id).classList.remove("blinking");
  }
}

function disableControls() {
  // Disables all form controls (inputs, selects, buttons)
  var controls = document.querySelectorAll('input, select, button');
  for (var i = 0; i < controls.length; i++) {
    controls[i].disabled = true;
  }
}

/* === help.js === */
/** Contextual Help System - Relies on a local help.json file **/

var helpData = null; // Cached help data from /help.json

function loadHelpData(callback) {
  // Fetch and cache the help.json file
  if (helpData !== null) {
    if (callback) callback();
    return;
  }

  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status >= 200 && this.status < 300) {
      try {
        helpData = JSON.parse(this.responseText);
        if (callback) callback();
      } catch (e) {
        console.error("Failed to parse help.json:", e);
      }
    }
  };
  xhttp.open("GET", "/help.json", true);
  xhttp.send();
}

function showHelpModal(title, text) {
  // Create or show the help modal with the provided text
  var modal = getEl("helpModal");
  if (!modal) {
    // Create modal structure
    modal = document.createElement("div");
    modal.id = "helpModal";
    modal.className = "help-modal";
    modal.innerHTML =
      '<div class="help-modal-content">' +
      '<span class="help-modal-close">&times;</span>' +
      '<h3 id="helpModalTitle"></h3>' +
      '<p id="helpModalText"></p>' +
      "</div>";
    document.body.appendChild(modal);

    // Close button handler
    modal.querySelector(".help-modal-close").onclick = function () {
      modal.style.display = "none";
    };

    // Click outside to close
    modal.onclick = function (event) {
      if (event.target === modal) {
        modal.style.display = "none";
      }
    };
  }

  // Set content and show
  getEl("helpModalTitle").textContent = title || "Help";
  getEl("helpModalText").textContent = text || "No help available.";
  modal.style.display = "block";
}

function createHelpIcon(labelText, helpText) {
  // Create an info icon that shows help when clicked
  var icon = document.createElement("span");
  icon.className = "help-icon";
  icon.innerHTML = "&#9432;"; // Unicode info symbol (i in circle)
  icon.title = "Click for help";
  icon.onclick = function (e) {
    e.preventDefault();
    e.stopPropagation();
    showHelpModal(labelText, helpText);
  };
  return icon;
}

function initializeHelp(section) {
  // Initialize help icons for all fields in the specified section
  loadHelpData(function () {
    if (!helpData || !helpData[section]) {
      return;
    }

    var sectionHelp = helpData[section];
    for (var fieldId in sectionHelp) {
      if (sectionHelp.hasOwnProperty(fieldId)) {
        var element = getEl(fieldId);
        if (element) {
          var helpText = sectionHelp[fieldId];
          
          // Skip if helpText is null or empty (no help available)
          if (!helpText) {
            continue;
          }
          
          // Find the parent setting div or label to insert the icon
          var parent = element.closest(".setting");
          if (parent) {
            // Find the label text
            var label = parent.querySelector("b, .label");
            var labelText = fieldId; // Fallback to field ID
            
            if (label) {
              // Extract and clean the label text
              labelText = label.textContent || label.innerText || fieldId;
              labelText = labelText.trim().replace(/:$/, ""); // Remove trailing colon
              
              // Create and add the help icon
              var helpIcon = createHelpIcon(labelText, helpText);
              label.appendChild(document.createTextNode(" "));
              label.appendChild(helpIcon);
            } else {
              // For toggle switches, find the label span
              var toggleLabel = parent.querySelector("label .label");
              if (toggleLabel) {
                labelText = toggleLabel.textContent || toggleLabel.innerText || fieldId;
                labelText = labelText.trim().replace(/:$/, "");
                
                var helpIcon = createHelpIcon(labelText, helpText);
                toggleLabel.appendChild(document.createTextNode(" "));
                toggleLabel.appendChild(helpIcon);
              }
            }
          }
        }
      }
    }
  });
}

/* === utils.js === */
/** General Utility Functions - Device-agnostic helper functions **/

function isJsonString(str) {
  try {
    JSON.parse(str);
  } catch (e) {
    return false;
  }
  return true;
}

function handleStatus(response) {
  // Generic handler for a JSON response with a "status" field.
  // If a response is not JSON then the full text is displayed.
  if (isJsonString(response || "")) {
    var jObj = JSON.parse(response || "");
    if (jObj.status && jObj.status != "success") {
      alert(jObj.status); // Report non-success status.
    }
  } else {
    alert(response); // Display plain text message.
  }
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

/* === common.js === */
/**
 *   GPStar BeltGizmo - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2024-2026 Dustin Grau <dustin.grau@gmail.com>
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

/**
 * NOTICE: Some required functions exist in shared JS files:
 * - api.js
 * - dom.js
 * - help.js
 * - utils.js
 */

// No device-specific functions

