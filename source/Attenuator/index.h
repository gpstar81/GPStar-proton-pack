const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale="1.0, user-scalable=no">
  <title>Proton Pack</title>
  <style>
  html { font-family: Open Sans; display: inline-block;, margin: 0px auto; text-align: center; }
  body { margin-top: 20px; }
  h1 { color: #333; margin: 20px auto 20px; }
  h3 { color: #555; margin-bottom: 20px; }
  .card{
    max-width: 400px;
    min-height: 250px;
    background: #02b875;
    padding: 30px;
    box-sizing: border-box;
    color: #FFF;
    margin:20px;
    box-shadow: 0px 2px 18px -4px rgba(0,0,0,0.75);
  }
  </style>
</head>
<body>

<h1>Equipment Status</h2><br>
<div class="card">
  <h3>Operating Mode: <span id="theme"></span></h3><br/>
</div>

<script>
  setInterval(function() {
    getData(); // Check for new data every 2 seconds
  }, 2000);

  function getData() {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        document.getElementById("theme").innerHTML = this.responseText;
      }
    };
    xhttp.open("GET", "readADC", true);
    xhttp.send();
  }
</script>
</body>
</html>
)=====";