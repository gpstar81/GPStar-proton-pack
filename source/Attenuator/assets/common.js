/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
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

/** Common JavaScript - Device Specific **/

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

function cableOn() {
  sendCommand("/pack/cable/on");
}

function cableOff() {
  sendCommand("/pack/cable/off");
}

function toggleSmoke(el) {
  handleToggle(el, "/pack/smoke/on", "/pack/smoke/off");
}

function toggleVibration(el) {
  handleToggle(el, "/pack/vibration/on", "/pack/vibration/off");
}

function cyclotronDirection(el) {
  handleToggle(el, "/pack/cyclotron/clockwise", "/pack/cyclotron/counterclockwise");
}

function themeSelect(caller) {
  // Change the theme via selected option: /pack/theme/<year>
  sendCommand("/pack/theme/" + caller.value);
}

function streamModeSelect(caller) {
  // Change the stream mode via selected option: /pack/stream/<stream_mode>
  sendCommand("/pack/stream/" + caller.value);
}

/**
 * Appended shared JavaScript functions from SharedLib/WebAssets/*.js below:
 *  - api.js
 *  - dom.js
 *  - help.js
 *  - utils.js
 */
