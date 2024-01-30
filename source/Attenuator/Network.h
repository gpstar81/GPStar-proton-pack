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
      Configure and enable a preferred external WiFi network for this device to join when in range.
      Enabling this feature allows you to make use of a preferred WiFi network such as those used by your mobile device(s).
      You may optionally configure a static IP address (with a subnet and gateway), if desired.
      Otherwise, you may return to this screen to view the IP address assigned by your WiFi network.
    </p>
  </div>

  <div class="block left">
    <div class="setting">
      <b class="labelSwitch">Use External WiFi Network:</b>
      <label class="switch">
        <input id="enabled" name="enabled" type="checkbox">
        <span class="slider round"></span>
      </label>  
    </div>
    <br/>
    &nbsp;&nbsp;<b>WiFi Network:</b> <input type="text" id="network" width="100" maxlength="30"/>
    <br/>
    <b>WiFi Password:</b> <input type="text" id="password" width="100" maxlength="30"/>
    <br/>
    <br/>
    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<b>Static IP:</b> <input type="text" id="address" width="100" maxlength="15"/>
    <br/>
    &nbsp;<b>Subnet Mask:</b> <input type="text" id="subnet" width="100" maxlength="15"/>
    <br/>
    &nbsp;&nbsp;&nbsp;<b>Gateway IP:</b> <input type="text" id="gateway" width="100" maxlength="15"/>
  </div>

  <div class="block">
    <hr/>
    <a href="/">&laquo; Back</a>
    &nbsp;&nbsp;&nbsp;
    <button type="button" class="green" onclick="saveSettings()">Save</button>
    <br/>
    <br/>
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

    function isValidIP(ipAddress) {  
      if (/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(ipAddress)) {  
        return true;
      }
      return false;  
    } 

    function saveSettings() {
      var wEnabled = document.getElementById("enabled").checked ? true : false;

      var wNetwork = (document.getElementById("network").value || "").trim();
      if (wEnabled && wNetwork.length < 2) {
        alert("The WiFi network must be a minimum of 2 characters.");
        return;
      }

      var wPassword = (document.getElementById("password").value || "").trim();
      if (wEnabled && wPassword.length < 8) {
        alert("The WiFi password must be a minimum of 8 characters to meet WPA2 requirements.");
        return;
      }

      var wAddress = (document.getElementById("address").value || "").trim();
      if (wAddress != "" && !isValidIP(wAddress)) {
        alert("IP Address is invalid, please correct and try again.");
        return;
      }

      var wSubnet = (document.getElementById("subnet").value || "").trim();
      if (wSubnet != "" && !isValidIP(wSubnet)) {
        alert("Subnet Mask is invalid, please correct and try again.");
        return;
      }

      var wGateway = (document.getElementById("gateway").value || "").trim();
      if (wGateway != "" && !isValidIP(wGateway)) {
        alert("Gateway IP is invalid, please correct and try again.");
        return;
      }

      var body = JSON.stringify({
        enabled: wEnabled,
        password: wPassword,
        network: wNetwork,
        address: wAddress,
        subnet: wSubnet,
        gateway: wGateway
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