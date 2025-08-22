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
    <button class="tablinks" onclick="openTab(event, 'tab3')">&#9881;</button>
  </div>

  <div id="tab1" class="tab">
    <div id="equipCRT" class="equipment">
      <div class="telemetry">
        <p><span class="infoLabel">Heading:</span> <span class="infoState" id="heading">&mdash;</span></p>
        <p><span class="infoLabel">&nbsp;&nbsp;&nbsp;&nbsp;Rot. X:</span> <span class="infoState" id="gyroX">&mdash;</span></p>
        <p><span class="infoLabel">&nbsp;&nbsp;&nbsp;&nbsp;Rot. Y:</span> <span class="infoState" id="gyroY">&mdash;</span></p>
        <p><span class="infoLabel">&nbsp;&nbsp;&nbsp;&nbsp;Rot. Z:</span> <span class="infoState" id="gyroZ">&mdash;</span></p>
        <p><span class="infoLabel">&nbsp;Accel. X:</span> <span class="infoState" id="accelX">&mdash;</span></p>
        <p><span class="infoLabel">&nbsp;Accel. Y:</span> <span class="infoState" id="accelY">&mdash;</span></p>
        <p><span class="infoLabel">&nbsp;Accel. Z:</span> <span class="infoState" id="accelZ">&mdash;</span></p>
        <p><span class="infoLabel">&nbsp;&nbsp;Roll (X):</span> <span class="infoState" id="roll">&mdash;</span></p>
        <p><span class="infoLabel">Pitch (Y):</span> <span class="infoState" id="pitch">&mdash;</span></p>
        <p><span class="infoLabel">&nbsp;&nbsp;Yaw (Z):</span> <span class="infoState" id="yaw">&mdash;</span></p>
      </div>
      <div class="viz-content">
        <div id="3Dobj"></div>
      </div>
    </div>

    <button type="button" class="orange" onclick="resetPosition()">Re-center</button>
    <button type="button" class="red" onclick="triggerIfrared()">Infrared</button>
  </div>

  <div id="tab2" class="tab">
    <div>
      <div class="volume-container">
        <div class="volume-control">
          <h3>System</h3>
          <button type="button" onclick="volSysUp()">+</button>
          <span id="masterVolume"></span>
          <button type="button" onclick="volSysDown()">&minus;</button>
        </div>
        <div class="volume-control">
          <h3>Effects</h3>
          <button type="button" onclick="volFxUp()">+</button>
          <span id="effectsVolume"></span>
          <button type="button" onclick="volFxDown()">&minus;</button>
        </div>
        <div class="volume-control">
          <h3>Music</h3>
          <button type="button" onclick="volMusicUp()">+</button>
          <span id="musicVolume"></span>
          <button type="button" onclick="volMusicDown()">&minus;</button>
        </div>
      </div>
      <button type="button" class="orange" onclick="toggleMute()">Mute/Unmute</button>
      <!--
      <button type="button" class="orange" onclick="musicLoop()">Loop Track</button>
      -->
    </div>
    <div class="card">
      <h3 class="centered">Music Navigation</h3>
      <div class="music-navigation">
        <button type="button" onclick="musicPrev()" title="Previous Track">&#9664;&#9664;</button>
        <button type="button" onclick="musicStartStop()" title="Start/Stop">&#9634;&nbsp;&#9654;</button>
        <button type="button" onclick="musicPauseResume()" title="Play/Pause">&#9646;&#9646;&nbsp;&#9654;</button>
        <button type="button" onclick="musicNext()" title="Next Track">&#9654;&#9654;</button>
      </div>
      <div class="music-playback">
        <span id="playbackStatus" class="infoLabel">&mdash;</span>
      </div>
      <select id="tracks" class="custom-select" onchange="musicSelect(this)"></select>
    </div>
  </div>

  <div id="tab3" class="tab">
    <div class="card" style="text-align:center;">
      <a href="/settings/device">Special Settings</a>
      <br/>
      <br/>
      <a href="/settings/wand">Neutrona Wand Settings</a>
      <br/>
      <br/>
      <hr/>
      <br/>
      <a href="/update">Update ESP32 Firmware</a>
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
