const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale="1.0, user-scalable=no">
  <title>Proton Pack</title>
  <style>
  html { font-family: Open Sans; display: inline-block;, margin: 0px auto; text-align: center; }
  body { margin-top: 10px; }
  h1 { color: #222; margin: 10px auto 10px; }
  p { font-size: 20px; margin-bottom: 5px; }
  .card{
    background: #ddd;
    box-sizing: border-box;
    box-shadow: 0px 2px 18px -4px rgba(0, 0, 0, 0.75);
    color: #444;
    margin: 20px 5px 20px 10px;
    max-width: 400px;
    min-height: 200px;
    padding: 20px 5px 20px 10px;
    text-align: left;
  }
  </style>
</head>
<body>

<h1>Equipment Status</h2><br>
<div class="card">
  <p><b>Operating Mode:</b> <span id="theme">...</span></p>
  <p><b>Device Mode:</b> <span id="mode">...</span></p>
  <p><b>Pack State:</b> <span id="pack">...</span></p>
  <p><b>Power Level:</b> <span id="power">...</span></p>
  <p><b>Neutrona Wand:</b> <span id="wand">...</span></p>
  <p><b>Ribbon Cable:</b> <span id="cable">...</span></p>
  <p><b>Cyclotron State:</b> <span id="cyclotron">...</span></p>
  <p><b>Overheat State:</b> <span id="temperature">...</span></p>
</div>

<script>
  setInterval(function() {
    getData(); // Check for new data every X seconds
  }, 1000);

  function getData() {
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
    xhttp.open("GET", "/data", true);
    xhttp.send();
  }
</script>
</body>
</html>
)=====";