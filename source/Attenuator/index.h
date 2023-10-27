const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale="1.0, user-scalable=no">
  <title>Proton Pack</title>

  <style>
    html { font-family: Tahoma, Verdana, Arial; display: inline-block;, margin: 0px auto; text-align: center; }
    body { margin-top: 10px; }
    h1 { color: #222; margin: 15px; }
    h3 { color: #333; margin: 10px; }
    p { font-size: 18px; margin-bottom: 5px; }
    .card {
      background: #ddd;
      box-sizing: border-box;
      box-shadow: 0px 2px 18px -4px rgba(0, 0, 0, 0.75);
      color: #444;
      margin: 10px 20px 10px 20px;
      max-width: 400px;
      min-height: 200px;
      padding: 10px 20px 10px 20px;
      text-align: left;
    }
    .info {
      font-family: courier, courier new, serif;
    }
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
    setInterval(function() {
      getStatus(); // Check for new data every X seconds
    }, 1000);

    function getStatus() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var jObj = JSON.parse(this.responseText);
          document.getElementById("theme").innerHTML = jObj.theme || "...";
          document.getElementById("mode").innerHTML = jObj.mode || "...";
          document.getElementById("pack").innerHTML = jObj.pack || "...";
          document.getElementById("power").innerHTML = jObj.power || "...";
          document.getElementById("wand").innerHTML = jObj.wand || "...";
          document.getElementById("cable").innerHTML = jObj.cable || "...";
          document.getElementById("cyclotron").innerHTML = jObj.cyclotron || "...";
          document.getElementById("temperature").innerHTML = jObj.temperature || "...";
        }
      };
      xhttp.open("GET", "/status", true);
      xhttp.send();
    }

    function packOn() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/pack/on", true);
      xhttp.send();
    }

    function packOff() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/pack/off", true);
      xhttp.send();
    }

    function cancelWarning() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/pack/cancel", true);
      xhttp.send();
    }

    function toggleMute() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/volume/mute", true);
      xhttp.send();
    }

    function volumeMasterUp() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/volume/master/up", true);
      xhttp.send();
    }

    function volumeMasterDown() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/volume/master/down", true);
      xhttp.send();
    }

    function volumeEffectsUp() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/volume/effects/up", true);
      xhttp.send();
    }

    function volumeEffectsDown() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/volume/effects/down", true);
      xhttp.send();
    }

    function toggleMusic() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/music/toggle", true);
      xhttp.send();
    }

    function musicNext() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/music/next", true);
      xhttp.send();
    }

    function musicPrev() {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          // var jObj = JSON.parse(this.responseText);
        }
      };
      xhttp.open("GET", "/music/prev", true);
      xhttp.send();
    }

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
      xhttp.open("POST", "/password", true);
      xhttp.setRequestHeader("Content-Type", "application/json");
      xhttp.send(body);
    }
  </script>
</head>
<body>
  <h1>Equipment Status</h1>
  <div class="card">
    <p><b>Operating Mode:</b> <span class="info" id="theme">&mdash;</span></p>
    <p><b>Device Mode:</b> <span class="info" id="mode">&mdash;</span></p>
    <p><b>Pack State:</b> <span class="info" id="pack">&mdash;</span></p>
    <p><b>Power Level:</b> <span class="info" id="power">&mdash;</span></p>
    <p><b>Neutrona Wand:</b> <span class="info" id="wand">&mdash;</span></p>
    <p><b>Ribbon Cable:</b> <span class="info" id="cable">&mdash;</span></p>
    <p><b>Cyclotron State:</b> <span class="info" id="cyclotron">&mdash;</span></p>
    <p><b>Overheat State:</b> <span class="info" id="temperature">&mdash;</span></p>
  </div>
  <br/>
  <h1>Audio Controls</h1>
  <h3>Master Volume</h3>
  <button type="button" class="blue" onclick="volumeMasterDown()">Down -</button>
  <button type="button" onclick="toggleMute()">Mute/Unmute</button>
  <button type="button" class="blue" onclick="volumeMasterUp()">Up +</button>
  <br/>
  <h3>Music Playback</h3>
  <button type="button" class="blue" onclick="musicPrev()">&laquo; Prev</button>
  <button type="button" class="green" onclick="toggleMusic()">Start/Stop</button>
  <button type="button" class="blue" onclick="musicNext()">Next &raquo;</button>
  <br/>
  <h3>Effects Volume</h3>
  <button type="button" class="blue" onclick="volumeEffectsDown()">Down -</button>
  <button type="button" class="blue" onclick="volumeEffectsUp()">Up +</button>
  <br/>
  <br/>
  <h1>Pack Controls</h1>
  <button type="button" class="red" onclick="packOff()">Pack Off</button>
  <button type="button" onclick="cancelWarning()">Cancel</button>
  <button type="button" class="green" onclick="packOn()">Pack On</button>
  <br/>
  <br/>
  <br/>
  <h1>WiFi Password</h1>
  <b>New AP Password:</b> <input type="text" id="password" width="120"/><br/>
  <b>Confirm Password:</b> <input type="text" id="password2" width="120"/><br/>
  <button type="button" onclick="updatePassword()">Update</button>
  <br/>
  <br/>
</body>
</html>
)=====";