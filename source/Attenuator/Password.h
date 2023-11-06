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

const char PASSWORD_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale="1.0, user-scalable=no">
  <title>WiFi Password</title>

  <style>
    html { font-family: Tahoma, Verdana, Arial; display: inline-block;, margin: 0px auto; text-align: center; }
    body { margin-top: 10px; }
    h1 { color: #222; margin: 15px; }
    h3 { color: #333; margin: 10px; }
    p { font-size: 18px; margin-bottom: 5px; text-align: left; }
    button {
      background-color: #555;
      border: 2px solid #333;
      border-radius: 8px;
      box-shadow: 0 8px 16px 0 rgba(0,0,0,0.2), 0 6px 20px 0 rgba(0,0,0,0.19);
      color: white;
      margin: 5px;
      padding: 12px 14px;
      max-width: 100px;
      text-align: center;
      width: 120px;
    }
    .blue {
      background-color: #008CBA;
      border: 2px solid #006C9A;
    }
    .green {
      background-color: #4CAF50;
      border: 2px solid #2C8F30;
    }
    .red {
      background-color: #f44336;
      border: 2px solid #d42316;
    }
    input {
      border: 2px solid #333;
      border-radius: 4px;
      box-sizing: border-box;
      font-size: 18px;
      margin: 4px 0;
      padding: 6px 10px;
      width: 50%;
    }
  </style>

  <script type="application/javascript">

    function updatePassword() {
      var newPass = (document.getElementById("password").value || "").trim();
      var confPW = (document.getElementById("password2").value || "").trim();
      if (newPass.length < 8) {
        alert("Your new password must be a minimum of 8 characters.");
        return;
      }
      if (newPass != confPW) {
        alert("Password and confirmation do not match. Please try again.");
        return;
      }

      var body = JSON.stringify({password: newPass});

      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var jObj = JSON.parse(this.responseText);
          alert(jObj.response);
        }
      };
      xhttp.open("POST", "/password/update", true);
      xhttp.setRequestHeader("Content-Type", "application/json");
      xhttp.send(body);
    }
  </script>
</head>
<body>
  <h1>WiFi Password</h1>
  <p>Use this page to update your preferred password for WiFi access. Note that this value is stored in controller's permanent memory,
  and firmware updates will have no effect on resetting this value. After updating, any devices which previously stored the password
  for the WiFi network will require you to update to the new password.</p>
  <br/>
  <b>New AP Password:</b> <input type="text" id="password" width="120"/><br/>
  <b>Confirm Password:</b> <input type="text" id="password2" width="120"/><br/>
  <a href="/">&laquo; Back</a>
  &nbsp;&nbsp;&nbsp;
  <button type="button" onclick="updatePassword()">Update</button>
  <br/>
  <br/>
</body>
</html>
)=====";