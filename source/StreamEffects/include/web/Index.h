/**
 *   GPStar Stream Effects - Ghostbusters Props, Mods, and Kits.
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

const char INDEX_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta http-equiv="Cache-control" content="public">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <meta name="apple-mobile-web-app-capable" content="yes"/>
  <meta name="apple-mobile-web-app-title" content="Stream Effects"/>
  <title>Stream Effects</title>
  <link rel="icon" type="image/svg+xml" href="/favicon.svg"/>
  <link rel="shortcut icon" href="/favicon.ico"/>
  <link rel="stylesheet" href="/style.css">
</head>
<body class="dark">
  <div class="tabs dark">
    <button class="tablinks" onclick="openTab(event, 'tab1')">&#9883;</button>
    <button class="tablinks" onclick="openTab(event, 'tab2')">&#9211;</button>
    <button class="tablinks" onclick="openTab(event, 'tab3')">&#9881;</button>
  </div>

  <div id="tab1" class="tab">
    <div id="equipTXT" class="card">
      <p><span class="infoLabel">Operation Mode:</span> <span class="infoState" id="mode">&mdash;</span></p>
      <p><span class="infoLabel">Effects Theme:</span> <span class="infoState" id="theme">&mdash;</span></p>
      <br/>
      <p><span class="infoLabel">Pack State:</span> <span class="infoState" id="pack">&mdash;</span></p>
      <p><span class="infoLabel">Pack Armed:</span> <span class="infoState" id="switch">&mdash;</span></p>
      <p><span class="infoLabel">Ribbon Cable:</span> <span class="infoState" id="cable">&mdash;</span></p>
      <p><span class="infoLabel">Cyclotron State:</span> <span class="infoState" id="cyclotron">&mdash;</span></p>
      <p><span class="infoLabel">Overheat State:</span> <span class="infoState" id="temperature">&mdash;</span></p>
      <br/>
      <p><span class="infoLabel">Wand Armed:</span> <span class="infoState" id="safety">&mdash;</span></p>
      <p><span class="infoLabel">System Mode:</span> <span class="infoState" id="wandMode">&mdash;</span></p>
      <div style="display:inline-flex;margin:0;">
        <p style="margin:0;">
          <span class="infoLabel">Power Level:</span>&nbsp;<span class="infoState" id="power">&mdash;</span>
        </p>
        <div class="bar-container" id="powerBars"></div>
      </div>
      <p><span class="infoLabel">Firing State:</span> <span class="infoState" id="firing">&mdash;</span></p>
      <br/>
      <p><span class="infoLabel">Ext. WiFi Status:</span> <span class="infoState" id="wifiStatus">&mdash;</span></p>
      <br/>
    </div>
  </div>

  <div id="tab2" class="tab">
    <div class="card" style="text-align:center;">
      <br/>
      <h2>Lighting Self-Test</h2>
      <br/>
      <br/>
      <button type="button" class="green" onclick="testOn()" id="btnTestOn">Test On</button>
      &nbsp;&nbsp;
      <button type="button" class="red" onclick="testOff()" id="btnTestOff">Test Off</button>
      <br/>
      <br/>
      <br/>
      <br/>
      <br/>
    </div>
  </div>

  <div id="tab3" class="tab">
    <div class="card" style="text-align:center;">
      <a href="/settings/device">Device Settings</a>
      <br/>
      <br/>
      <hr/>
      <br/>
      <a href="/update">Update Device Firmware</a>
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
