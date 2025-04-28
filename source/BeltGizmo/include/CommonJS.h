/**
 *   GPStar BeltGizmo - Ghostbusters Props, Mods, and Kits.
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
)=====";
