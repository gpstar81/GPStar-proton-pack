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

const char INDEX_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta http-equiv="Cache-control" content="public">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <meta name="apple-mobile-web-app-capable" content="yes"/>
  <meta name="apple-mobile-web-app-title" content="Belt Gizmo"/>
  <title>Belt Gizmo</title>
  <link rel="icon" type="image/svg+xml" href="/favicon.svg"/>
  <link rel="shortcut icon" href="/favicon.ico"/>
  <link rel="stylesheet" href="/style.css">
</head>
<body class="dark">
  <div class="card" style="text-align:center;">
    <a href="/settings/device">Device Settings</a>
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

  <script type="application/javascript" src="/common.js"></script>
  <script type="application/javascript" src="/index.js"></script>
</body>
</html>
)=====";
