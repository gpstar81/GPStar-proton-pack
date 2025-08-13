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

const char COMMONJS_page[] PROGMEM = R"=====(
function getEl(id){
  return document.getElementById(id);
}

function getInt(id){
  return parseInt(getValue(id) || 0, 10);
}

function getText(id){
  return (getValue(id) || "").trim();
}

function getToggle(id){
  return (getEl(id).checked ? 1 : 0);
}

function getValue(id){
  return getEl(id).value;
}

function setHtml(id, value){
  getEl(id).innerHTML = value || "";
}

function setToggle(id, value){
  getEl(id).checked = (value ? true : false);
}

function setValue(id, value){
  getEl(id).value = value;
}

function hideEl(id){
  getEl(id).style.display = "none";
}

function showEl(id){
  getEl(id).style.display = "block";
}

function colorEl(id, red, green, blue, alpha = 0.5){
  getEl(id).style.backgroundColor = "rgba(" + red + ", " + green + ", " + blue + ", " + alpha + ")";
}

function blinkEl(id, state) {
  if(state) {
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

function packAttenuate() {
  sendCommand("/pack/attenuate");
}

function packVent() {
  sendCommand("/pack/vent");
}

function packLOStart() {
  sendCommand("/pack/lockout/start");
}

function packLOCancel() {
  sendCommand("/pack/lockout/cancel");
}

function toggleMute() {
  sendCommand("/volume/toggle");
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
  sendCommand("/music/select?track=" + caller.value);
}

function musicPrev() {
  sendCommand("/music/prev");
}

function musicNext() {
  sendCommand("/music/next");
}

function musicLoop() {
  sendCommand("/music/loop");
}

function getStatus(callbackFunc) {
  // This function expects a JSON response from the server which must be parsed and sent to the callback function.
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
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
)=====";
