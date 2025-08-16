/**
 *   GPStar Neutrona Wand - Ghostbusters Proton Pack & Neutrona Wand.
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
window.addEventListener("resize", onWindowResize);

function onLoad(event) {
  document.getElementsByClassName("tablinks")[0].click();
  getDevicePrefs(); // Get all preferences.
  initWebSocket(); // Open the WebSocket.
  getStatus(updateEquipment); // Get status immediately.
  init3D(); // Initialize 3D representation.
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

function updateEquipment(jObj) {
  // Logic TBD for wand
}

// https://randomnerdtutorials.com/esp32-mpu-6050-web-server/

let scene, camera, rendered, cube;

function parentWidth(elem) {
  return elem.parentElement.clientWidth;
}

function parentHeight(elem) {
  return elem.parentElement.clientHeight;
}

function init3D(){
  scene = new THREE.Scene();
  //scene.background = new THREE.Color(0xffffff);
  scene.background = null; // Set background to transparent

  camera = new THREE.PerspectiveCamera(75, parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube")), 0.1, 1000);

  //renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true }); // For transparent background
  renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));

  document.getElementById('3Dcube').appendChild(renderer.domElement);

  // Create a geometry (order: width, height, depth)
  const geometry = new THREE.BoxGeometry(2, 1, 5);

  // Materials for each cube face (order: right, left, top, bottom, front, back)
  var cubeMaterials = [
    new THREE.MeshBasicMaterial({color:0x009000}), // Right face  - darker green
    new THREE.MeshBasicMaterial({color:0x009000}), // Left face   - darker green
    new THREE.MeshBasicMaterial({color:0x00C000}), // Top face    - lighter green
    new THREE.MeshBasicMaterial({color:0x007000}), // Bottom face - darkest green
    new THREE.MeshBasicMaterial({color:0x00A000}), // Front face  - base green
    new THREE.MeshBasicMaterial({color:0x00A000}), // Back face   - base green
  ];

  cube = new THREE.Mesh(geometry, cubeMaterials);
  scene.add(cube);
  camera.position.z = 5;
  renderer.render(scene, camera);
}

// Resize the 3D object when the browser window changes size
function onWindowResize(){
  camera.aspect = parentWidth(document.getElementById("3Dcube")) / parentHeight(document.getElementById("3Dcube"));
  //camera.aspect = window.innerWidth /  window.innerHeight;
  camera.updateProjectionMatrix();
  //renderer.setSize(window.innerWidth, window.innerHeight);
  renderer.setSize(parentWidth(document.getElementById("3Dcube")), parentHeight(document.getElementById("3Dcube")));
}

function resetPosition() {
  sendCommand("/sensors/reset");
}

// Create events for the sensor readings
if (!!window.EventSource) {
  var source = new EventSource("/events");

  source.addEventListener("open", function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener("error", function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
    }
  }, false);

  source.addEventListener("telemetry", function(e) {
    var obj = JSON.parse(e.data);

    // Update the HTML elements with the telemetry data
    setHtml("heading", "&nbsp;&nbsp;Heading: " + parseFloat(obj.heading || 0).toFixed(2) + "&deg;");
    setHtml("gyroX",   "&nbsp;&nbsp;&nbsp;Rot. X: " + parseFloat(obj.gyroX || 0).toFixed(2) + " rads/s");
    setHtml("gyroY",   "&nbsp;&nbsp;&nbsp;Rot. Y: " + parseFloat(obj.gyroY || 0).toFixed(2) + " rads/s");
    setHtml("gyroZ",   "&nbsp;&nbsp;&nbsp;Rot. Z: " + parseFloat(obj.gyroZ || 0).toFixed(2) + " rads/s");
    setHtml("accelX",  "&nbsp;&nbsp;X (L-R): " + parseFloat(obj.accelX || 0).toFixed(2) + " m/s<sup>2</sup>");
    setHtml("accelY",  "&nbsp;&nbsp;Y (F-B): " + parseFloat(obj.accelY || 0).toFixed(2) + " m/s<sup>2</sup>");
    setHtml("accelZ",  "&nbsp;&nbsp;Z (U-D): " + parseFloat(obj.accelZ || 0).toFixed(2) + " m/s<sup>2</sup>");
    setHtml("pitch",   "Pitch (Y): " + parseFloat(obj.pitch || 0).toFixed(2) + "&deg;");
    setHtml("roll",    "&nbsp;Roll (X): " + parseFloat(obj.roll || 0).toFixed(2) + "&deg;");
    setHtml("yaw",     "&nbsp;&nbsp;Yaw (Z): " + parseFloat(obj.yaw || 0).toFixed(2) + "&deg;");

    // Change cube rotation after receiving the readings
    if (cube) {
      // cube.rotation.x = (obj.roll || 0); // Roll
      // cube.rotation.y = (obj.pitch || 0); // Pitch
      // cube.rotation.z = (obj.yaw || 0); // Yaw
      cube.rotation.x = (obj.gyroX || 0);
      cube.rotation.y = (obj.gyroZ || 0) * -1;
      cube.rotation.z = (obj.gyroY || 0);
      renderer.render(scene, camera);
    }
  }, false);
}
)=====";
