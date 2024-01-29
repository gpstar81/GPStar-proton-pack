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

const char NETWORK_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>WiFi Settings</title>
  <link rel="icon" href="data:;base64,iVBORw0KGgo=">
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1>WiFi Settings</h1>
  <div class="block">
    <p>
      This screen allows you to configure a preferred external WiFi network for your Proton Pack to join.
      Enabling this allows you to make use of a mobile hotspot or other preferred WiFi network used by your mobile device.
      You may optionally configure a static IP address (with a subnet and gateway), if desired.
    </p>
    <br/>
    <div class="setting">
      <b class="labelSwitch">Use External WiFi Network:</b>
      <label class="switch">
        <input id="enabled" name="enabled" type="checkbox">
        <span class="slider round"></span>
      </label>  
    </div>
    <br/>
    <b>WiFi Network:</b> <input type="text" id="network" width="100"/>
    <br/>
    <b>WiFi Password:</b> <input type="text" id="password" width="100"/>
    <br/>
    <br/>
    <b>Static IP Address:</b> <input type="text" id="address" width="100"/>
    <br/>
    <b>Subnet Mask:</b> <input type="text" id="subnet" width="100"/>
    <br/>
    <b>Gateway IP:</b> <input type="text" id="gateway" width="100"/>
    <br/>
    <br/>
    <a href="/">&laquo; Back</a>
    &nbsp;&nbsp;&nbsp;
    <button type="button" class="green" onclick="saveSettings()">Save</button>
  </div>

  <script type="application/javascript">
    window.addEventListener("load", onLoad);

    function onLoad(event) {
      // Wait for page to fully load.
      setTimeout(getSettings, 50);
    }

    function getSettings() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var settings = JSON.parse(this.responseText);
          if (settings) {
            document.getElementById("enabled").checked = settings.enabled ? true: false;
            document.getElementById("network").value = settings.network || "";
            document.getElementById("password").value = settings.password || "";
            document.getElementById("address").value = settings.address || "";
            document.getElementById("subnet").value = settings.subnet || "";
            document.getElementById("gateway").value = settings.gateway || "";
          }
        }
      };
      xhttp.open("GET", "/wifi/settings", true);
      xhttp.send();
    }

    function saveSettings() {
      var wNetwork = (document.getElementById("network").value || "").trim();
      if (wNetwork.length < 2) {
        alert("The WiFi network must be a minimum of 2 characters.");
        return;
      }

      var wPassword = (document.getElementById("password").value || "").trim();
      if (wPassword.length < 8) {
        alert("The WiFi password must be a minimum of 8 characters to meet WPA2 requirements.");
        return;
      }

      var body = JSON.stringify({
        enabled: document.getElementById("enabled").checked ? 1 : 0,
        password: wPassword,
        network: wNetwork,
        address: document.getElementById("address").value || "",
        subnet: document.getElementById("subnet").value || "",
        gateway: document.getElementById("gateway").value || ""
      });

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var jObj = JSON.parse(this.responseText);
          alert(jObj.status); // Always display status returned.
        }
      };
      xhttp.open("PUT", "/wifi/update", true);
      xhttp.setRequestHeader("Content-Type", "application/json");
      xhttp.send(body);
    }
  </script>
</body>
</html>
)=====";