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

const char NETWORK_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta http-equiv="Cache-control" content="public">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>WiFi Settings</title>
  <link rel="icon" type="image/svg+xml" href="/favicon.svg"/>
  <link rel="shortcut icon" href="/favicon.ico"/>
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <h1>WiFi Settings</h1>
  <div class="block left">
    <p>
      Configure and enable a parent device external WiFi network for this device to join when in range (eg. a Proton Pack).
      <br/>
      You may optionally configure a static IP address along with a subnet and gateway for that network, if desired.
      Once joined, you may return to this screen to view the IP address assigned by the external WiFi network.
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
    &nbsp;&nbsp;&nbsp;<b>WiFi Network:</b>
    <input type="text" id="network" width="100" maxlength="32" placeholder="External SSID"
     title="Only letters, numbers, hyphens, and underscores are allowed, up to 32 characters."/>
    <br/>
    &nbsp;<b>WiFi Password:</b> <input type="text" id="password" width="100" maxlength="63" placeholder="External Password"/>
    <br/>
    <br/>
    <br/>
    If necessary, you may toggle the switch below to specify a static IP address, subnet,
    and gateway to be used by the controller on the preferred WiFi network. Note that any
    changes to the network name or password will clear previously-entered values.
    <div class="setting">
      <b class="labelSwitch">Edit IP Address Values:</b>
      <label class="switch">
        <input id="editIP" name="editIP" type="checkbox">
        <span class="slider round"></span>
      </label>
    </div>
    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<b>Static IP:</b> <input type="text" id="address" width="100" maxlength="15" disabled/>
    <br/>
    &nbsp;<b>Subnet Mask:</b> <input type="text" id="subnet" width="100" maxlength="15" disabled/>
    <br/>
    &nbsp;&nbsp;&nbsp;<b>Gateway IP:</b> <input type="text" id="gateway" width="100" maxlength="15" disabled/>
  </div>

  <div class="block">
    <hr/>
    <a href="/">&laquo; Back</a>
    &nbsp;&nbsp;&nbsp;
    <button type="button" class="green" onclick="saveSettings()">Save</button>
    &nbsp;&nbsp;&nbsp;
    <button type="button" class="orange" onclick="restartWiFi()">Rejoin</button>
    <br/>
    <br/>
  </div>

  <script type="application/javascript" src="/common.js"></script>
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
            setToggle("enabled", settings.enabled);
            setValue("network", settings.network || "");
            setValue("password", settings.password || "");
            setValue("address", settings.address || "");
            setValue("subnet", settings.subnet || "");
            setValue("gateway", settings.gateway || "");
          }
        }
      };
      xhttp.open("GET", "/wifi/settings", true);
      xhttp.send();
    }

    function restartWiFi() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var jObj = JSON.parse(this.responseText);
          alert(jObj.status); // Always display status returned.
        }
      };
      xhttp.open("GET", "/wifi/restart", true);
      xhttp.send();
    }

    function isValidIP(ipAddress) {
      if (/^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/.test(ipAddress)) {
        return true;
      }
      return false;
    }

    // Set up some variables to fields which will be controlled via certain actions.
    var addressInput = getEl("address");
    var subnetInput = getEl("subnet");
    var gatewayInput = getEl("gateway");

    getEl("network").addEventListener("input", function() {
      // Clear fields based on input changes.
      addressInput.value = "";
      subnetInput.value = "";
      gatewayInput.value = "";
    });

    getEl("password").addEventListener("input", function() {
      // Clear fields based on input changes.
      addressInput.value = "";
      subnetInput.value = "";
      gatewayInput.value = "";
    });

    getEl("editIP").addEventListener("change", function() {
      // Get the checkbox state to enable the IP fields.
      var editEnabled = getToggle("editIP");

      // Enable or disable based on checkbox state.
      addressInput.disabled = !editEnabled;
      subnetInput.disabled = !editEnabled;
      gatewayInput.disabled = !editEnabled;
    });

    function saveSettings() {
      var wEnabled = getToggle("enabled");

      var wNetwork = getText("network");
      if (wEnabled && wNetwork.length < 2) {
        alert("The WiFi network must be a minimum of 2 characters.");
        return;
      }

      var wPassword = getText("password");
      if (wEnabled && wPassword.length < 8) {
        alert("The WiFi password must be a minimum of 8 characters to meet WPA2 requirements.");
        return;
      }

      var wAddress = getText("address");
      if (wAddress != "" && !isValidIP(wAddress)) {
        alert("IP Address is invalid, please correct and try again.");
        return;
      }

      var wSubnet = getText("subnet");
      if (wSubnet != "" && !isValidIP(wSubnet)) {
        alert("Subnet Mask is invalid, please correct and try again.");
        return;
      }

      var wGateway = getText("gateway");
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
          getSettings(); // Refresh the current network settings.
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
