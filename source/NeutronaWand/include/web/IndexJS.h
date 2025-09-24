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

function onLoad(event) {
  document.getElementsByClassName("tablinks")[0].click();
  hideEl("calInfo"); // Hide the calibration coverage info.
  disableSensorButtons(); // Set button states by default.
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
  console.log("WebSocket connection opened");

  // Clear the automated status interval timer.
  clearInterval(statusInterval);
}

function onClose(event) {
  console.log("WebSocket connection closed");
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
        switch(jObj.audioVersion || 0) {
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

function disableSensorButtons() {
  // Used to just disable all the sensor-related buttons initially
  getEl("btnRecenter").disabled = true;
  getEl("btnCalibrateOn").disabled = true;
  getEl("btnCalibrateOff").disabled = true;
}

function setButtonStates(sensorState) {
  // Use the sensor state to determine button states.
  switch(sensorState) {
    case "Calibration":
      // While in calibration mode only the "off" button is enabled.
      getEl("btnRecenter").disabled = true;
      getEl("btnCalibrateOn").disabled = true;
      getEl("btnCalibrateOff").disabled = false;
      break;
    case "Offsets":
      // While in offset calculation mode no buttons are enabled.
      getEl("btnRecenter").disabled = true;
      getEl("btnCalibrateOn").disabled = true;
      getEl("btnCalibrateOff").disabled = true;
      break;
    case "Telemetry":
      // While in telemetry mode only re-center and calibration enable buttons are enabled.
      getEl("btnRecenter").disabled = false;
      getEl("btnCalibrateOn").disabled = false;
      getEl("btnCalibrateOff").disabled = true;
      break;
    default:
      disableSensorButtons();
      break;
  }
}

function updateEquipment(jObj) {
  // Update display if we have the expected data (containing mode and theme at a minimum).
  if (jObj && jObj.mode && jObj.theme) {
    // Update button states based on sensor information.
    setButtonStates(jObj.sensors || "");

    // Enable/Disable Music Controls
    if (jObj.benchtest) {
      getEl("btnVolMusicUp").disabled = false;
      getEl("btnVolMusicDown").disabled = false;
      getEl("playbackStatus").disabled = false;
      getEl("tracks").disabled = false;
      getEl("btnMusicPrev").disabled = false;
      getEl("btnMusicStartStop").disabled = false;
      getEl("btnMusicPauseResume").disabled = false;
      getEl("btnMusicNext").disabled = false;
      getEl("toggleLoop").disabled = true;
    } else {
      getEl("btnVolMusicUp").disabled = true;
      getEl("btnVolMusicDown").disabled = true;
      getEl("playbackStatus").disabled = true;
      getEl("tracks").disabled = true;
      getEl("btnMusicPrev").disabled = true;
      getEl("btnMusicStartStop").disabled = true;
      getEl("btnMusicPauseResume").disabled = true;
      getEl("btnMusicNext").disabled = true;
      getEl("toggleLoop").disabled = false;
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
}

function parentWidth(elem) {
  return elem.parentElement.clientWidth;
}

function parentHeight(elem) {
  return elem.parentElement.clientHeight;
}

class Telemetry3DView {
  constructor(domId, geometryUrl) {
    this.el = document.getElementById(domId);
    this.width = parentWidth(this.el);
    this.height = parentHeight(this.el);
    this.aspect = this.width / this.height;

    // Create the scene with a transparent background.
    this.scene = new THREE.Scene();
    this.scene.background = null;

    // Set up renderer with a transparent background
    this.renderer = new THREE.WebGLRenderer({antialias: true, alpha: true});
    this.renderer.setSize(this.width, this.height);
    this.el.appendChild(this.renderer.domElement);

    // Add lights to the scene for realistic shading and visibility.
    // HemisphereLight simulates ambient light from the sky and ground, providing soft global illumination.
    const hemiLight = new THREE.HemisphereLight(0xffffff, 0x444444, 1);
    hemiLight.position.set(0,200,0); // Position above the scene (Y axis) for natural lighting effect
    this.scene.add(hemiLight);

    // DirectionalLight simulates sunlight, casting parallel rays and creating shadows and highlights.
    const dirLight = new THREE.DirectionalLight(0xffffff, 0.8);
    dirLight.position.set(100, 100, 100); // Position at an angle for dynamic shading (3/4 view)
    this.scene.add(dirLight);

    // Add lines for the XYZ axes as visual aid (X: red, Y: green, Z: blue) with 200 unit length.
    // const axesHelper = new THREE.AxesHelper(200);
    // this.scene.add(axesHelper);

    // Load geometry from JSON (converted from STL)
    fetch(geometryUrl)
      .then(res => res.json())
      .then(json => {
        const loader = new THREE.BufferGeometryLoader();
        const geometry = loader.parse(json);

        // Center the geometry itself so the mesh rotates around its center
        geometry.computeBoundingBox();
        const box = geometry.boundingBox;
        this.size = new THREE.Vector3();
        box.getSize(this.size); // Original size of the mesh in original units (assume: mm)
        const center = new THREE.Vector3();
        box.getCenter(center); // True center of the mesh itself using the bounding box
        geometry.translate(-center.x, -center.y, -center.z); // Center the object on the origin

        // Select a material and color then create the mesh for the scene
        const material = new THREE.MeshLambertMaterial({color: 0x00A000});
        this.mesh = new THREE.Mesh(geometry, material);
        this.scene.add(this.mesh);

        // Set up an orthographic camera; better for technical models without distortion.
        const frustumSize = Math.max(this.size.x, this.size.y, this.size.z) * 1.2; // Scale to fit mesh comfortably
        this.camera = new THREE.OrthographicCamera(
          (-frustumSize * this.aspect / 2), // Left
          (frustumSize * this.aspect / 2),  // Right
          (frustumSize / 2),                // Top
          (-frustumSize / 2),               // Bottom
          0.1,                              // Near clipping plane
          300                               // Far clipping plane
        );

        // Position camera and look at the center of the scene
        this.camera.position.set(0, this.size.y, frustumSize);

        // Camera positioning using the center of the mesh as the focal point
        this.camera.lookAt(new THREE.Vector3());
        this.render();
      });
  }

  render() {
    if (this.scene && this.camera) {
      this.width = parentWidth(this.el);
      this.height = parentHeight(this.el);
      this.aspect = this.width / this.height;
      this.camera.aspect = this.aspect;
      this.camera.updateProjectionMatrix();
      this.renderer.setSize(this.width, this.height);
      this.renderer.render(this.scene, this.camera);
    }
  }

  // Update mesh orientation using quaternion
  setQuaternion(qx, qy, qz, qw) {
    if (this.mesh) {
      this.mesh.quaternion.set(qx, qy, qz, qw);
      this.render();
    }
  }

  // Update camera position if needed
  setCameraPosition(x, y, z) {
    if (this.camera) {
      this.camera.position.set(x, y, z);
      this.camera.lookAt(0, 0, 0);
      this.render();
    }
  }
}

class Calibration3DView {
  constructor(domId) {
    this.el = document.getElementById(domId);
    this.width = parentWidth(this.el);
    this.height = parentHeight(this.el);
    this.aspect = this.width / this.height;

    // Create the scene with a transparent background.
    this.scene = new THREE.Scene();
    this.scene.background = null;

    // Set up renderer with antialiasing and alpha for transparency
    this.renderer = new THREE.WebGLRenderer({antialias: true, alpha: true});
    this.renderer.setSize(this.width, this.height);
    this.el.appendChild(this.renderer.domElement);

    // Add lights to the scene for realistic shading and visibility.
    // HemisphereLight simulates ambient light from the sky and ground, providing soft global illumination.
    const hemiLight = new THREE.HemisphereLight(0xffffff, 0x444444, 1);
    hemiLight.position.set(0,200,0);
    this.scene.add(hemiLight);

    // DirectionalLight simulates sunlight, casting parallel rays and creating shadows and highlights.
    const dirLight = new THREE.DirectionalLight(0xffffff, 0.8);
    dirLight.position.set(100, 100, 100);
    this.scene.add(dirLight);

    // Add lines for the XYZ axes as visual aid (X: red, Y: green, Z: blue) with 200 unit length.
    // const axesHelper = new THREE.AxesHelper(200);
    // this.scene.add(axesHelper);

    // Sphere geometry placed at the origin of the scene.
    const geometry = new THREE.SphereGeometry(5, 32, 32);
    const material = new THREE.MeshBasicMaterial({
      color: 0x222222,
      transparent: true,
      opacity: 0.4
    });
    this.mesh = new THREE.Mesh(geometry, material);
    this.scene.add(this.mesh);

    // Set up a perspective camera; better for spatial orientation.
    this.camera = new THREE.PerspectiveCamera(80, this.aspect, 0.1, 1000);

    // Position camera and look at the center of the scene
    this.camera.position.set(0, 5, 70);

    // Camera positioning using the center of the mesh as the focal point
    this.camera.lookAt(new THREE.Vector3());
    this.render();
  }

  /**
   * Renders the current scene from the camera's perspective.
   * Also handles resizing if the parent element changes size.
   */
  render() {
    if (this.scene && this.camera) {
      this.width = parentWidth(this.el);
      this.height = parentHeight(this.el);
      this.aspect = this.width / this.height;
      this.camera.aspect = this.aspect;
      this.camera.updateProjectionMatrix();
      this.renderer.setSize(this.width, this.height);
      this.renderer.render(this.scene, this.camera);
    }
  }

  /**
   * Visualize calibration points (array with {x, y, z} coordinates) as small red spheres in 3D space.
   */
  setPoints(points) {
    // Create the group if it doesn't exist
    if (!this.pointsGroup) {
      this.pointsGroup = new THREE.Group();
      this.scene.add(this.pointsGroup);
    }

    // Reuse or create meshes as needed
    const existing = this.pointsGroup.children.length;
    const needed = points ? points.length : 0;

    // Add new meshes only if needed (keeps a pool of meshes for efficiency)
    for (let i = existing; i < needed; i++) {
      const geometry = new THREE.SphereGeometry(0.8, 16, 16);
      const material = new THREE.MeshBasicMaterial({color: 0xff0000});
      const pointMesh = new THREE.Mesh(geometry, material);
      this.pointsGroup.add(pointMesh);
    }

    // Update positions and visibility from the pool of meshes
    for (let i = 0; i < this.pointsGroup.children.length; i++) {
      const childMesh = this.pointsGroup.children[i];

      if (i < needed) {
        const p = points[i];
        childMesh.position.set(p.x, p.y, p.z);
        childMesh.visible = true;
      } else {
        childMesh.visible = false;
      }
    }

    let center = this.getPointsCentroid();
    if (center && points.length > 10) {
      if (this.mesh) {
        // Move the default mesh (origin sphere) to the centroid
        this.mesh.position.set(center.x, center.y, center.z);
      }
      if (this.camera) {
        // Re-center the camera on the centroid of the points
        this.camera.lookAt(center.x, center.y, center.z);
      }
    }

    this.render();
  }

  /**
   * Removes all calibration points from the scene and disposes their geometry/material.
   */
  clearPoints() {
    if (this.pointsGroup) {
      // Remove group from scene
      this.scene.remove(this.pointsGroup);

      // Dispose geometry and material for each mesh
      this.pointsGroup.children.forEach(mesh => {
        if (mesh.geometry) mesh.geometry.dispose();
        if (mesh.material) mesh.material.dispose();
      });

      // Clear reference and render
      this.pointsGroup = null;
      this.render();
    }
  }

  /**
   * Calculates the centroid (arithmetic mean) of all visible calibration points.
   * Returns an object {x, y, z} or null if no points are visible.
   */
  getPointsCentroid() {
    if (!this.pointsGroup || this.pointsGroup.children.length === 0) return null;
    let sum = {x: 0, y: 0, z: 0};
    let count = 0;

    this.pointsGroup.children.forEach(mesh => {
      if (mesh.visible) {
        sum.x += mesh.position.x;
        sum.y += mesh.position.y;
        sum.z += mesh.position.z;
        count++;
      }
    });

    if (count === 0) return null;

    return {
      x: sum.x / count,
      y: sum.y / count,
      z: sum.z / count
    };
  }
}

function formatFloat(value) {
  value = Number(value);
  let absValue = Math.abs(value);
  let whole = Math.floor(absValue).toString();
  let frac = absValue.toFixed(1).split(".")[1];
  let sign = value.toFixed(1) < 0 ? "-" : "&nbsp;";
  let padCount = 3 - whole.length;
  let pad = "";
  for (let i = 0; i < padCount; i++) {
    pad += "&nbsp;";
  }
  // Final format: [sign][pad][whole].[frac]
  return sign + pad + whole + "." + frac;
}

let lastCoverage = 0;

if (!!window.EventSource) {
  // Create events for the sensor readings.
  var source = new EventSource("/events");

  source.addEventListener("open", function(e) {
    console.log("Server-Side Events connected");
  }, false);

  source.addEventListener("error", function(e) {
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Server-Side Events disconnected");
    }
  }, false);

  source.addEventListener("coverage", function(e) {
    if (e.data === undefined) return;

    // Update the calibration coverage percentage.
    lastCoverage = parseFloat(e.data) || 0;
    setHtml("coverage", formatFloat(lastCoverage) + "%");
  }, false);

  source.addEventListener("calibration", function(e) {
    var points = [];
    try {
      points = JSON.parse(e.data); // array of {x, y, z}
    } catch (e) { }

    if (calibration3D) {
      calibration3D.setPoints(points);
    }
  }, false);

  source.addEventListener("telemetry", function(e) {
    var obj = {};
    try {
      obj = JSON.parse(e.data);
    } catch (e) { }

    // Convert roll, pitch, and yaw from degrees to radians for Three.js
    var rollRads = (obj.roll || 0) * Math.PI / 180;
    var pitchRads = (obj.pitch || 0) * Math.PI / 180;
    var yawRads = (obj.yaw || 0) * Math.PI / 180;

    // Update the HTML elements with the telemetry data
    setHtml("gyroX",  formatFloat(obj.gyroX || 0)  + "&deg;/s");
    setHtml("gyroY",  formatFloat(obj.gyroY || 0)  + "&deg;/s");
    setHtml("gyroZ",  formatFloat(obj.gyroZ || 0)  + "&deg;/s");
    setHtml("accelX", formatFloat(obj.accelX || 0) + "m/s<sup>2</sup>");
    setHtml("accelY", formatFloat(obj.accelY || 0) + "m/s<sup>2</sup>");
    setHtml("accelZ", formatFloat(obj.accelZ || 0) + "m/s<sup>2</sup>");
    setHtml("roll",   formatFloat(obj.roll || 0)   + "&deg;");
    setHtml("pitch",  formatFloat(obj.pitch || 0)  + "&deg;");
    setHtml("yaw",    formatFloat(obj.yaw || 0)    + "&deg;");
    setHtml("gForce", formatFloat(obj.gForce || 0) + "");
    setHtml("angVel", formatFloat(obj.angVel || 0) + "&deg;/s");
    setHtml("shaken", "&nbsp;&nbsp;&nbsp;" + (obj.shaken ? "&oplus;" : "&mdash;"));

    // Proceed with updating the rendered scene if all objects are present.
    if (telemetry3D && telemetry3D.mesh) {
      // Change cube rotation after checking for the available data (quaternion preferred).
      // This uses a right-handed coordinate system with X (right), Y (up), and Z (towards viewer).
      // Map accordingly from device to view: Pitch (Y) -> X, Yaw (Z) -> Y, Roll (X) -> Z.

      // Use quaternion (x,y,z,w) calculations for more accurate orientation and avoid gimbal lock.
      telemetry3D.setQuaternion(-obj.qy, -obj.qz, obj.qx, obj.qw);

      // Move camera behind the object based on yaw
      const radius = 200; // Distance from object, adjust as needed
      const camX = radius * Math.sin(yawRads);
      const camZ = radius * Math.cos(yawRads);
      if (telemetry3D.size) {
        // Keep Y fixed just above the Z plane for a slight downward angle  
        telemetry3D.setCameraPosition(camX, telemetry3D.size.y * 2, camZ);
      } else {
        telemetry3D.setCameraPosition(camX, 0, camZ); // Keep Y fixed at 0 if size is not available
      }
    }
  }, false);
}

// Instances for each visualization
let telemetry3D, calibration3D;

// Initialize both visualizations
function init3D() {
  telemetry3D = new Telemetry3DView("3Dtelemetry", "/geometry.json");
  calibration3D = new Calibration3DView("3Dcalibration");
}

window.addEventListener("load", onLoad);

/** API Commands **/

function resetPosition() {
  sendCommand("/sensors/recenter");
}

function triggerInfrared() {
  sendCommand("/infrared/signal?type=ghostintrap");
}

function enableCalibration() {
  if (confirm("Are you sure you want to start calibration?")) {
    sendCommand("/sensors/calibrate/enable");
    calibration3D.clearPoints();
    showEl("calInfo");
  }
}

function disableCalibration() {
  if (lastCoverage < 60) {
    if (confirm("Coverage is less than 60%, do you wish to continue collecting data?")) {
      return; // Leave calibration mode active.
    }
  }

  if (confirm("Are you sure you are done collecting calibration data?")) {
    sendCommand("/sensors/calibrate/disable");
    hideEl("calInfo");
    calibration3D.clearPoints();
  }
}
)=====";
