/**
 *   GPStar Stream Effects - Ghostbusters Props, Mods, and Kits.
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

function isJsonString(str) {
  try {
    JSON.parse(str);
  } catch (e) {
    return false;
  }
  return true;
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

/** Common Data Handling **/

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

/** Common API Commands **/

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
