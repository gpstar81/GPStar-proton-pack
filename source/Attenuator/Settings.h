/**
 *   GPStar Attenuator - Ghostbusters Proton Pack & Neutrona Wand.
 *   Copyright (C) 2023 Michael Rajotte <michael.rajotte@gpstartechnologies.com>
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

const char SETTINGS_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Equipment Settings</title>
  <link rel="icon" href="data:;base64,iVBORw0KGgo=">
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1>Modify Equipment Settings</h1>
  <div class="block">
    <p>
      Change system configuration options using the available toggles/selectors.
    </p>
    <br/>
    <b>Power Cell LEDs:</b>
    <select type="text" id="password">
    </select>
    <br/>
    <a href="/">&laquo; Back</a>
    &nbsp;&nbsp;&nbsp;
    <button type="button" class="green" onclick="saveSettings()">Save</button>
  </div>

  <script type="application/javascript">
    window.addEventListener("load", onLoad);

    function onLoad(event) {
      getStatus();
    }

    function getStatus() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // Use the current settings to determine what's configured.
          var data = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/status", true);
      xhttp.send();
    }

    function saveSettings() {
      var body = JSON.stringify({name: value});

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var jObj = JSON.parse(this.responseText);
          alert(jObj.status); // Always display status returned.
        }
      };
      xhttp.open("PUT", "/settings/update", true);
      xhttp.setRequestHeader("Content-Type", "application/json");
      xhttp.send(body);
    }
  </script>
</body>
</html>
)=====";