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

const char INDEX_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta http-equiv="Cache-control" content="public">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <meta name="apple-mobile-web-app-capable" content="yes"/>
  <meta name="apple-mobile-web-app-title" content="Neutrona Wand"/>
  <title>Neutrona Wand</title>
  <link rel="icon" type="image/svg+xml" href="/favicon.svg"/>
  <link rel="shortcut icon" href="/favicon.ico"/>
  <link rel="stylesheet" href="/style.css">
</head>
<body class="dark">
  <div class="tabs dark">
    <button class="tablinks" onclick="openTab(event, 'tab1')">&#9883;</button>
    <button class="tablinks" onclick="openTab(event, 'tab2')">&#9836;</button>
    <button class="tablinks" onclick="openTab(event, 'tab3')">&#9737;</button>
    <button class="tablinks" onclick="openTab(event, 'tab4')">&#9881;</button>
  </div>

  <div id="tab1" class="tab">
    <div id="equipCRT" class="equipment">
      <div class="telemetry">
        <p id="gyro">
          <span class="infoLabel">&nbsp;&nbsp;X: &orarr;</span> <span class="infoState" id="gyroX">&mdash;</span><br/>
          <span class="infoLabel">&nbsp;&nbsp;Y: &orarr;</span> <span class="infoState" id="gyroY">&mdash;</span><br/>
          <span class="infoLabel">&nbsp;&nbsp;Z: &orarr;</span> <span class="infoState" id="gyroZ">&mdash;</span>
        </p>
        <p id="accel">
          <span class="infoLabel">&nbsp;&varr;&nbsp;</span> <span class="infoState" id="accelX">&mdash;</span><br/>
          <span class="infoLabel">&harr;</span> <span class="infoState" id="accelY">&mdash;</span><br/>
          <span class="infoLabel">&#9737;</span> <span class="infoState" id="accelZ">&mdash;</span>
        </p>
        <p id="ahrs">
          <span class="infoLabel">&nbsp;&nbsp;Roll &Phi;:</span> <span class="infoState" id="roll">&mdash;</span><br/>
          <span class="infoLabel">Pitch &Theta;:</span> <span class="infoState" id="pitch">&mdash;</span><br/>
          <span class="infoLabel">&nbsp;Yaw &Psi;:</span> <span class="infoState" id="yaw">&mdash;</span>
        </p>
        <p id="calc">
          <span class="infoLabel">&omega;:</span> <span class="infoState" id="angVel">&mdash;</span><br/>
          <span class="infoLabel"><i>g</i>:</span> <span class="infoState" id="gForce">&mdash;</span><br/>
          <span class="infoLabel">&Delta;:</span> <span class="infoState" id="shaken">&mdash;</span>
        </p>
      </div>
      <div class="viz-content">
        <div id="3Dtelemetry"></div>
      </div>
    </div>
    <button type="button" id="btnRecenter" class="blue" onclick="resetPosition()">Re-center</button>
    &nbsp;&nbsp;&nbsp;
    <button type="button" id="btnInfrared" class="orange" onclick="triggerInfrared()">Infrared</button>
    <br/>
  </div>

  <div id="tab2" class="tab">
    <div>
      <div class="volume-container">
        <div class="volume-control">
          <h3>Master</h3>
          <button type="button" id="btnVolSysUp" onclick="volSysUp()">+</button>
          <span id="masterVolume"></span>
          <button type="button" id="btnVolSysDown" onclick="volSysDown()">&minus;</button>
        </div>
        <div class="volume-control">
          <h3>Effects</h3>
          <button type="button" id="btnVolFxUp" onclick="volFxUp()">+</button>
          <span id="effectsVolume"></span>
          <button type="button" id="btnVolFxDown" onclick="volFxDown()">&minus;</button>
        </div>
        <div class="volume-control">
          <h3>Music</h3>
          <button type="button" id="btnVolMusicUp" onclick="volMusicUp()">+</button>
          <span id="musicVolume"></span>
          <button type="button" id="btnVolMusicDown" onclick="volMusicDown()">&minus;</button>
        </div>
      </div>
    </div>

    <div id="audio-toggles" class="card">
      <div style="display: inline-block">
        <h3>Master Mute</h3>
        <label class="toggle-switchy">
          <input id="toggleMute" name="toggleMute" type="checkbox" onchange="toggleMute(this)">
          <span class="toggle">
            <span class="switch"></span>
          </span>
        </label>
      </div>
      <div style="display: inline-block">
        <h3>Music Repeat</h3>
        <label class="toggle-switchy" data-text="repeat">
          <input id="toggleLoop" name="toggleLoop" type="checkbox" onchange="musicLoop(this)">
          <span class="toggle">
            <span class="switch"></span>
          </span>
        </label>
      </div>
    </div>

    <div class="card">
      <h3 class="centered">Music Navigation</h3>
      <div class="music-navigation">
        <button type="button" id="btnMusicPrev" onclick="musicPrev()" title="Previous Track">&#9664;&#9664;</button>
        <button type="button" id="btnMusicStartStop" onclick="musicStartStop()" title="Start/Stop">&#9634;&nbsp;&#9654;</button>
        <button type="button" id="btnMusicPauseResume" onclick="musicPauseResume()" title="Play/Pause">&#9646;&#9646;&nbsp;&#9654;</button>
        <button type="button" id="btnMusicNext" onclick="musicNext()" title="Next Track">&#9654;&#9654;</button>
      </div>
      <div class="music-playback">
        <span id="playbackStatus" class="infoLabel">&mdash;</span>
      </div>
      <select id="tracks" class="custom-select" onchange="musicSelect(this)"></select>
    </div>
  </div>

  <div id="tab3" class="tab">
    <div class="card" style="text-align:center;">
      <div class="block left">
        Magnetic calibration is critical to obtaining a correct reading from sensors and must be performed only when all components are fully installed.
        <ol style="padding-left:20px;">
          <li>Press "Enable Calibration" to begin.</li>
          <li>Rotate your Neutrona Wand slowly in all directions to collect calibration data (represented as dots in a sphere).</li>
          <li>Fill the sphere with dots until the coverage is as close to 100% as possible.</li>
          <li>Press "Disable Calibration" to stop and store the calculated values.</li>
        </ol>
      </div>
      <button type="button" id="btnCalibrateOn" class="green" onclick="enableCalibration()">Enable Calibration</button>
      &nbsp;&nbsp;&nbsp;
      <button type="button" id="btnCalibrateOff" class="red" onclick="disableCalibration()">Disable Calibration</button>
      <br/>
      <br/>
      <div class="telemetry" id="calInfo">
        <p>
          <span class="infoLabel">Coverage:</span> <span class="infoState" id="coverage">&mdash;</span>
        </p>
        <div class="viz-content">
          <div id="3Dcalibration"></div>
        </div>
      </div>
    </div>
  </div>

  <div id="tab4" class="tab">
    <div class="card" style="text-align:center;">
      <a href="/settings/device">Special Device Settings</a>
      <br/>
      <br/>
      <a href="/settings/wand">Neutrona Wand Settings</a>
      <br/>
      <br/>
      <hr/>
      <br/>
      <a href="/update">Update Neutrona Wand Firmware</a>
      <br/>
      <br/>
      <a href="/password">Secure Device WiFi</a>
      <br/>
      <br/>
      <a href="/network">Change WiFi Settings</a>
      <br/>
      <br/>
      <a href="javascript:doRestart()">Restart/Resync</a>
    </div>
    <div class="footer">
      <span id="buildDate"></span>
      &mdash;
      <span id="wifiName"></span>
      <br/>
      <span id="audioInfo"></span>
      <br/>
      <span id="clientInfo"></span>
      <br/>
      <span id="extWifi"></span>
    </div>
  </div>

  <script type="application/javascript" src="/common.js"></script>
  <script type="application/javascript" src="/index.js"></script>
  <script type="application/javascript" src="/three.js"></script>
</body>
</html>
)=====";
