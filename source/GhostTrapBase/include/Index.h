/**
 *   GPStar Ghost Trap - Ghostbusters Props, Mods, and Kits.
 *   Copyright (C) 2025 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
 *                    & Nomake Wan <-redacted->
 *                    & Dustin Grau <dustin.grau@gmail.com>
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
  <meta name="apple-mobile-web-app-title" content="Ghost Trap"/>
  <title>Ghost Trap</title>
  <link rel="icon" type="image/svg+xml" href="/favicon.svg"/>
  <link rel="shortcut icon" href="/favicon.ico"/>
  <link rel="stylesheet" href="/style.css">
</head>
<body class="dark">
  <div class="tabs dark">
    <button class="tablinks" onclick="openTab(event, 'tab1')">&#9883;</button>
    <button class="tablinks" onclick="openTab(event, 'tab2')">&#9836;</button>
    <button class="tablinks" onclick="openTab(event, 'tab3')">&#9211;</button>
    <button class="tablinks" onclick="openTab(event, 'tab4')">&#9881;</button>
  </div>

  <div id="tab1" class="tab">
    <div id="equipCRT" class="equipment">
      <div id="doorOverlay" class="overlay door-state"></div>
    </div>

    <div id="equipTXT" class="card">
      <p><span class="infoLabel">Door State:</span> <span class="infoState" id="doorState">&mdash;</span></p>
    </div>
  </div>

  <div id="tab2" class="tab">
    <div>
      <div class="volume-container">

      </div>
    </div>
  </div>

  <div id="tab3" class="tab">
    <div class="card" style="text-align:center;">
      <br/>
      <button type="button" class="green" onclick="enableSmoke()" id="btnSmokeEnable">Enable</button>
      &nbsp;&nbsp;
      <button type="button" class="red" onclick="disableSmoke()" id="btnSmokeDisable">Disable</button>
      <br/>
      <br/>
      <button type="button" class="orange" onclick="runSmoke(2000)" id="btnSmoke2">Smoke 2sec.</button>
      &nbsp;&nbsp;
      <button type="button" class="orange" onclick="runSmoke(5000)" id="btnSmoke5">Smoke 5sec.</button>
      <br/>
      <br/>
      <button type="button" class="blue" onclick="lightOn()" id="btnLightOn">LED On</button>
      &nbsp;&nbsp;
      <button type="button" class="blue" onclick="lightOff()" id="btnLightOff">LED Off</button>
      <br/>
      <br/>
      <br/>
    </div>
  </div>

  <div id="tab4" class="tab">
    <div class="card" style="text-align:center;">
      <a href="/settings/device">Ghost Trap Settings</a>
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
</body>
</html>
)=====";
