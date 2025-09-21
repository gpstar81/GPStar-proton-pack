/**
 *   GPStar Proton Pack - Ghostbusters Proton Pack & Neutrona Wand.
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

const char INDEX_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta http-equiv="Cache-control" content="public">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <meta name="apple-mobile-web-app-capable" content="yes"/>
  <meta name="apple-mobile-web-app-title" content="Proton Pack"/>
  <title>Proton Pack</title>
  <link rel="icon" type="image/svg+xml" href="/favicon.svg"/>
  <link rel="shortcut icon" href="/favicon.ico"/>
  <link rel="stylesheet" href="/style.css">
</head>
<body class="dark">
  <div class="tabs dark">
    <button class="tablinks" onclick="openTab(event, 'tab1')">&#9883;</button>
    <button class="tablinks" onclick="openTab(event, 'tab2')">&#9836;</button>
    <button class="tablinks" onclick="openTab(event, 'tab3')">&#9211;</button>
    <button class="tablinks" onclick="openTab(event, 'tab4')">&#9881;</button>
  </div>

  <div id="tab1" class="tab">
    <div id="equipCRT" class="equipment">
      <div id="equipTitle" class="infoState equip-title centered"></div>
      <div id="ionOverlay" class="overlay ion-switch"></div>
      <div id="boostOverlay" class="overlay booster-box"></div>
      <div id="pcellOverlay" class="overlay power-box"></div>
      <div id="cableOverlay" class="overlay cable-warn">&#9888;</div>
      <div id="cycOverlay" class="overlay cyc-circle"></div>
      <div id="filterOverlay" class="overlay filter-circle"></div>
      <div id="barrelOverlay" class="overlay barrel-box"></div>
      <div id="powerLevel" class="overlay infoState power-title"></div>
      <div id="streamMode" class="overlay infoState stream-title"></div>
      <div id="safetyOverlay" class="overlay safety-box"></div>
      <div id="battVoltage" class="overlay infoState batt-title"></div>
      <div id="cyclotronLid" class="infoState rad-warn">
        <span style="font-size:1.2em">&#9762;</span> Cyclotron Exposure Warning
      </div>
    </div>

    <div id="equipTXT" class="card">
      <p><span class="infoLabel">Operation Mode:</span> <span class="infoState" id="mode">&mdash;</span></p>
      <p><span class="infoLabel">Effects Theme:</span> <span class="infoState" id="theme">&mdash;</span></p>
      <br/>
      <p><span class="infoLabel">Pack State:</span> <span class="infoState" id="pack">&mdash;</span></p>
      <p><span class="infoLabel">Pack Armed:</span> <span class="infoState" id="switch">&mdash;</span></p>
      <p><span class="infoLabel">Ribbon Cable:</span> <span class="infoState" id="cable">&mdash;</span></p>
      <p><span class="infoLabel">Cyclotron State:</span> <span class="infoState" id="cyclotron">&mdash;</span></p>
      <p><span class="infoLabel">Overheat State:</span> <span class="infoState" id="temperature">&mdash;</span></p>
      <br/>
      <p><span class="infoLabel">Wand Presence:</span> <span class="infoState" id="wand">&mdash;</span></p>
      <p><span class="infoLabel">Wand State:</span> <span class="infoState" id="wandPower">&mdash;</span></p>
      <p><span class="infoLabel">Wand Armed:</span> <span class="infoState" id="safety">&mdash;</span></p>
      <p><span class="infoLabel">System Mode:</span> <span class="infoState" id="wandMode">&mdash;</span></p>
      <div style="display:inline-flex;margin:0;">
        <p style="margin:0;">
          <span class="infoLabel">Power Level:</span>&nbsp;<span class="infoState" id="power">&mdash;</span>
        </p>
        <div class="bar-container" id="powerBars"></div>
      </div>
      <p><span class="infoLabel">Firing State:</span> <span class="infoState" id="firing">&mdash;</span></p>
      <br/>
      <p>
        <span class="infoLabel">Powercell:</span>
        <span id="battHealth"></span>
        <span class="infoState" id="battVoltageTXT">&mdash;</span>
        <span style="font-size: 0.8em">GeV</span>
      </p>
    </div>
  </div>

  <div id="tab2" class="tab">
    <div>
      <div class="volume-container">
        <div class="volume-control">
          <h3>Master</h3>
          <button type="button" onclick="volSysUp()">+</button>
          <span id="masterVolume"></span>
          <button type="button" onclick="volSysDown()">&minus;</button>
        </div>
        <div class="volume-control">
          <h3>Effects</h3>
          <button type="button" onclick="volFxUp()">+</button>
          <span id="effectsVolume"></span>
          <button type="button" onclick="volFxDown()">&minus;</button>
        </div>
        <div class="volume-control">
          <h3>Music</h3>
          <button type="button" onclick="volMusicUp()">+</button>
          <span id="musicVolume"></span>
          <button type="button" onclick="volMusicDown()">&minus;</button>
        </div>
      </div>
    </div>

    <div id="audio-toggles" class="card">
      <div style="display: inline-block">
        <h3>Master Mute</h3>
        <label class="toggle-switchy">
          <input id="toggleMute" name="toggleMute" type="checkbox" onchange="toggleMute(this)">
          <span class="toggle">
            <span class="switch"></span>
          </span>
        </label>
      </div>
      <div style="display: inline-block">
        <h3>Music Repeat</h3>
        <label class="toggle-switchy" data-text="repeat">
          <input id="toggleLoop" name="toggleLoop" type="checkbox" onchange="musicLoop(this)">
          <span class="toggle">
            <span class="switch"></span>
          </span>
        </label>
      </div>
    </div>

    <div class="card">
      <h3 class="centered">Music Navigation</h3>
      <div class="music-navigation">
        <button type="button" onclick="musicPrev()" title="Previous Track">&#9664;&#9664;</button>
        <button type="button" onclick="musicStartStop()" title="Start/Stop">&#9634;&nbsp;&#9654;</button>
        <button type="button" onclick="musicPauseResume()" title="Play/Pause">&#9646;&#9646;&nbsp;&#9654;</button>
        <button type="button" onclick="musicNext()" title="Next Track">&#9654;&#9654;</button>
      </div>
      <div class="music-playback">
        <span id="playbackStatus" class="infoLabel">&mdash;</span>
      </div>
      <select id="tracks" class="custom-select" onchange="musicSelect(this)"></select>
    </div>
  </div>

  <div id="tab3" class="tab">
    <div class="card" style="text-align:center;">
      <br/>
      <button type="button" class="red" onclick="packOff()" id="btnPackOff">Pack Off</button>
      &nbsp;&nbsp;
      <button type="button" class="green" onclick="packOn()" id="btnPackOn">Pack On</button>
      <br/>
      <br/>
      <br/>
      <button type="button" class="orange" onclick="packVent()" id="btnVent">Vent</button>
      &nbsp;&nbsp;
      <button type="button" class="blue" onclick="packAttenuate()" id="btnAttenuate">Attenuate</button>
      <br/>
      <br/>
      <br/>
      <div style="display: inline-block">
        <h3>Smoke</h3>
        <label class="toggle-switchy">
          <input id="toggleSmoke" name="toggleSmoke" type="checkbox" onchange="toggleSmoke(this)">
          <span class="toggle">
            <span class="switch"></span>
          </span>
        </label>
      </div>
      <div style="display: inline-block">
        <h3>Vibration</h3>
        <label class="toggle-switchy">
          <input id="toggleVibration" name="toggleVibration" type="checkbox" onchange="toggleVibration(this)">
          <span class="toggle">
            <span class="switch"></span>
          </span>
        </label>
      </div>
      <br/>
      <div style="display: inline-block">
        <h3>Cyclotron Direction</h3>
        <label class="toggle-switchy" data-text="direction">
          <input id="cyclotronDirection" name="cyclotronDirection" type="checkbox" onchange="cyclotronDirection(this)">
          <span class="toggle">
            <span class="switch"></span>
          </span>
        </label>
      </div>
      <br/>
      <br/>
      <br/>
      <select id="themes" class="custom-select" onchange="themeSelect(this)">
        <option value="1984">GB1 - 1984</option>
        <option value="1989">GB2 - 1989</option>
        <option value="2021">Afterlife</option>
        <option value="2024">Frozen Empire</option>
      </select>
      <br/>
      <br/>
      <br/>
      <br/>
      <br/>
    </div>
  </div>

  <div id="tab4" class="tab">
    <div class="card" style="text-align:center;">
      <a href="/settings/device">Special Device Settings</a>
      <br/>
      <br/>
      <a href="/settings/pack">Proton Pack Settings</a>
      <br/>
      <br/>
      <a href="/settings/wand">Neutrona Wand Settings</a>
      <br/>
      <br/>
      <a href="/settings/smoke">Overheat/Smoke Settings</a>
      <br/>
      <br/>
      <hr/>
      <br/>
      <a href="/update">Update Proton Pack Firmware</a>
      <br/>
      <br/>
      <a href="/password">Secure Device WiFi</a>
      <br/>
      <br/>
      <a href="/network">Change WiFi Settings</a>
      <br/>
      <br/>
      <a href="javascript:doRestart()">Restart/Resync</a>
    </div>
    <div class="footer">
      <span id="buildDate"></span>
      &mdash;
      <span id="wifiName"></span>
      <br/>
      <span id="audioInfo"></span>
      <br/>
      <span id="clientInfo"></span>
      <br/>
      <span id="extWifi"></span>
    </div>
  </div>

  <script type="application/javascript" src="/common.js"></script>
  <script type="application/javascript" src="/index.js"></script>
</body>
</html>
)=====";
