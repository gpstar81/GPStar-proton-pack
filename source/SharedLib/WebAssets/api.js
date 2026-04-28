/** General API Helpers - Common functions for device control **/

function sendCommand(apiUri) {
  // Sends an action command to the server (device) using a PUT request.
  // These commands have no response data, so we just handle the status.
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4) {
      handleStatus(this.responseText);
    }
  };
  xhttp.open("PUT", apiUri, true);
  xhttp.send();
}

function volSysUp() {
  sendCommand("/volume/master/up");
}

function volSysDown() {
  sendCommand("/volume/master/down");
}

function volFxUp() {
  sendCommand("/volume/effects/up");
}

function volFxDown() {
  sendCommand("/volume/effects/down");
}

function volMusicUp() {
  sendCommand("/volume/music/up");
}

function volMusicDown() {
  sendCommand("/volume/music/down");
}

function musicStartStop() {
  sendCommand("/music/startstop");
}

function musicPauseResume() {
  sendCommand("/music/pauseresume");
}

function musicSelect(caller) {
  // Change the music track by selected option: /music/select?track=<#>
  sendCommand("/music/select?track=" + caller.value);
}

function musicPrev() {
  sendCommand("/music/prev");
}

function musicNext() {
  sendCommand("/music/next");
}

function handleToggle(el, apiOn, apiOff) {
  if (el._lockout) return;
  el._lockout = true;

  const switchEl = el.parentElement.querySelector(".switch");

  function onTransitionEnd(e) {
    if (e.propertyName === "right") {
      sendCommand(el.checked ? apiOn : apiOff);
      el._lockout = false;
      switchEl.removeEventListener("transitionend", onTransitionEnd);
    }
  }

  switchEl.addEventListener("transitionend", onTransitionEnd);
}

function toggleMute(el) {
  handleToggle(el, "/volume/mute", "/volume/unmute");
}

function musicLoop(el) {
  handleToggle(el, "/music/loop/one", "/music/loop/all");
}

function musicShuffle(el) {
  handleToggle(el, "/music/shuffle/on", "/music/shuffle/off");
}

function getStatus(callbackFunc) {
  // This function expects a JSON response from the server which must be parsed and sent to the callback function.
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function () {
    if (this.readyState == 4 && this.status >= 200 && this.status < 300) {
      if (callbackFunc && typeof callbackFunc === "function") {
        // If a callback function is provided, call it with the JSON response.
        callbackFunc(JSON.parse(this.responseText));
      } else {
        // Otherwise display a message that no callback function was provided.
        console.warn("No callback function provided for getStatus response.");
      }
    }
  };
  xhttp.open("GET", "/status", true);
  xhttp.send();
}

function doRestart() {
  // A special command which requires user confirmation before proceeding.
  if (confirm("Are you sure you wish to restart the serial device?")) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
      if (this.readyState == 4 && this.status >= 200 && this.status < 300) {
        // Reload the page after 2 seconds.
        setTimeout(function () {
          window.location.reload();
        }, 2000);
      }
    };
    xhttp.open("DELETE", "/restart", true);
    xhttp.send();
  }
}
