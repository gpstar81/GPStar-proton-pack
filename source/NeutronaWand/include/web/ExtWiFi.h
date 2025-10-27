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
      Configure and enable a preferred external WiFi network for this device to join when in range (eg. your home network).
      Enabling this feature allows you to make use of a preferred WiFi network such as those used by your mobile device(s).
      <br/>
      You may optionally configure a static IP address along with a subnet and gateway for that network, if desired.
      Once joined, you may return to this screen to view the IP address assigned by the external WiFi network.
    </p>
  </div>

  <div class="block left">
    <div class="setting">
      <label class="toggle-switchy" data-text="yesno" data-label="left">
        <input id="enabled" name="enabled" type="checkbox">
        <span class="toggle">
          <span class="switch"></span>
        </span>
        <span class="label">Use External WiFi Network:</span>
      </label>
    </div>
    <b>2.4GHz Networks:</b> <select id="ssidSelect"></select>
    <br/>
    &nbsp;&nbsp;&nbsp;<b>WiFi Network:</b>
    <input type="text" id="network" size="60" maxlength="32" placeholder="External SSID"
     title="Only letters, numbers, hyphens, and underscores are allowed, up to 32 characters."/>
    <br/>
    &nbsp;<b>WiFi Password:</b> <input type="text" id="password" size="60" maxlength="63" placeholder="External Password"/>
    <br/>
    <br/>
    <br/>
    If necessary, you may toggle the switch below to specify a static IP address, subnet,
    and gateway to be used by the controller on the preferred WiFi network. Note that any
    changes to the network name or password will clear previously-entered values.
    <div class="setting">
      <label class="toggle-switchy" data-text="yesno" data-label="left">
        <input id="useStaticIP" name="useStaticIP" type="checkbox">
        <span class="toggle">
          <span class="switch"></span>
        </span>
        <span class="label">Set Static IP Address</span>
      </label>
    </div>
    &nbsp;&nbsp;&nbsp;&nbsp;<b>IP Address:</b> <input type="text" id="address" size="20" maxlength="15" disabled/>
    <br/>
    &nbsp;<b>Subnet Mask:</b> <input type="text" id="subnet" size="20" maxlength="15" disabled/>
    <br/>
    &nbsp;&nbsp;&nbsp;<b>Gateway IP:</b> <input type="text" id="gateway" size="20" maxlength="15" disabled/>
  </div>

  <div class="block">
    <hr/>
    <a href="/">&laquo; Back</a>
    &nbsp;&nbsp;&nbsp;
    <button type="button" class="green" onclick="saveSettings()">Save</button>
    &nbsp;&nbsp;&nbsp;
    <button type="button" class="orange" onclick="getNetworks()" id="refreshNetworks" style="width:160px;">Refresh Networks</button>
    <br/>
    <br/>
  </div>

  <script type="application/javascript" src="/common.js"></script>
  <script type="application/javascript">
    window.addEventListener("load", onLoad);

    function onLoad(event) {
      // Wait for page to fully load.
      setTimeout(function() {
        getSettings();
        getNetworks();
      }, 50);
    }

    // Ensure the select element reflects the current network input value.
    // If the current network isn't present in the options, add the value as the first selectable option
    // so the UI shows the selected value regardless of which API returned first.
    function selectCurrentNetwork() {
      var ssidSelect = getEl("ssidSelect");
      if (!ssidSelect) return;
      var currentNetwork = getText("network") || "";

      if (!currentNetwork) {
        // If no current network, select the placeholder
        ssidSelect.value = "";
        return;
      }

      // Try to select matching option
      ssidSelect.value = currentNetwork;

      // If no option matched, add it as the first non-placeholder option and select it
      var found = false;
      for (var j = 0; j < ssidSelect.options.length; ++j) {
        if (ssidSelect.options[j].value === currentNetwork) {
          found = true;
          break;
        }
      }
      if (!found) {
        var opt = document.createElement("option");
        opt.value = currentNetwork;
        opt.text = currentNetwork;
        // insert at index 1 (after placeholder) so scanned entries remain below
        try {
          ssidSelect.add(opt, 1);
        } catch (e) {
          ssidSelect.add(opt);
        }
        ssidSelect.value = currentNetwork;
      }
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
            // Ensure the select reflects the current network even if networks haven't been loaded yet.
            selectCurrentNetwork();
          }
        }
      };
      xhttp.open("GET", "/wifi/settings", true);
      xhttp.send();
    }

    function getNetworks() {
      // Disable the refresh button while the request is running.
      disableEl("refreshNetworks");

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4) {
          if (this.status == 200) {
            var resp = JSON.parse(this.responseText);
            if (resp) {
              populateNetworks(resp.networks || []);
            }
          }

          // Restore button interactivity
          enableEl("refreshNetworks");
        }
      };
      xhttp.open("GET", "/wifi/networks", true);
      xhttp.send();
    }

    function populateNetworks(networks) {
      var ssidSelect = getEl("ssidSelect");
      if (!ssidSelect) return;

      // Clear existing options except the placeholder at index 0
      while (ssidSelect.options.length > 1) {
        ssidSelect.remove(1);
      }

      if (!networks || !Array.isArray(networks)) {
        // Attempt to sync with any known current network even if networks aren't present
        selectCurrentNetwork();
        return;
      }

      var currentNetwork = getText("network") || "";

      for (var i = 0; i < networks.length; ++i) {
        var name = networks[i] || "";
        var opt = document.createElement("option");
        opt.value = name;
        opt.text = name;
        ssidSelect.add(opt);
      }

      // If there is an existing selected network, mark it selected. If it's missing, helper will add it.
      if (currentNetwork) {
        ssidSelect.value = currentNetwork;
        if (ssidSelect.value !== currentNetwork) {
          selectCurrentNetwork();
        }
      }

      // Wire change handler (idempotent)
      ssidSelect.onchange = function() {
        var chosen = ssidSelect.value || "";
        // Set network input to selected value
        setValue("network", chosen);

        // Clear other fields as requested
        setValue("password", "");
        setValue("address", "");
        setValue("subnet", "");
        setValue("gateway", "");
        setToggle("useStaticIP", false);

        // Optionally clear static IP UI fields (they are disabled when toggle false)
        getEl("address").value = "";
        getEl("subnet").value = "";
        getEl("gateway").value = "";
      };

      // Final sync to ensure selection regardless of which API responded first
      selectCurrentNetwork();
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

      // If input matches an option in the select, make it selected; otherwise clear select.
      var ssidSelect = getEl("ssidSelect");
      if (ssidSelect) {
        ssidSelect.value = this.value || "";
      }
    });

    getEl("password").addEventListener("input", function() {
      // Clear fields based on input changes.
      addressInput.value = "";
      subnetInput.value = "";
      gatewayInput.value = "";
    });

    getEl("useStaticIP").addEventListener("change", function() {
      // Get the checkbox state to enable the IP fields.
      var editEnabled = getToggle("useStaticIP");

      // Enable or disable based on checkbox state.
      addressInput.disabled = !editEnabled;
      subnetInput.disabled = !editEnabled;
      gatewayInput.disabled = !editEnabled;
    });

    function saveSettings() {
      var wEnabled = getToggle("enabled");
      var wNetwork = getText("network");
      var wPassword = getText("password");
      if (wEnabled) {
        if (wNetwork.length < 2) {
          alert("The WiFi network must be a minimum of 2 characters.");
          return;
        }

        if (wPassword.length < 8) {
          alert("The WiFi password must be a minimum of 8 characters to meet WPA2 requirements.");
          return;
        }
      }
      else {
        wPassword = "";
        wNetwork = "";
      }

      var wStatic = getToggle("useStaticIP");
      if (wStatic) {
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
      }
      else {
        wAddress = "";
        wSubnet = "";
        wGateway = "";
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
