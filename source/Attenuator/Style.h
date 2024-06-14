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

const char STYLE_page[] PROGMEM = R"=====(
html {
  display: inline-block;
  font-family: Tahoma, Verdana, Arial;
  margin: 0;
  overflow-x: hidden;
  text-align: center;
  width: 100%;
}

body {
  margin: 0 0 20px;
  position: relative;
}

h1 {
  background: #222;
  colour: #eee;
  margin: 0;
  padding: 8px;
  width: 100%;
}

h3 {
  colour: #333;
  margin: 10px;
}

a {
  text-decoration: none;
}

p {
  font-size: 18px;
  margin: 5px 0 8px;
}

.dark {
  background-colour: #444;
}

.blinking {
  animation: blink-animation 0.5s steps(5, start) infinite;
}

@keyframes blink-animation {
  to {
    visibility: hidden;
  }
}

.footer {
  colour: #ccc;
  font-size: 0.8em;
}

.left {
  text-align: left;
}

.block {
  margin: 10px 5px 30px;
  padding: 10px 5px;
  width: 99%;
}

.card {
  background: #ddd;
  border-radius: 12px;
  box-sizing: border-box;
  box-shadow: 0 2px 18px -4px rgba(0, 0, 0, 0.75);
  colour: #444;
  margin: 20px auto;
  max-width: 360px;
  min-height: 200px;
  padding: 10px 15px;
  text-align: left;
  width: 360px;
}

.centered {
  align-items: center;
  display: flex;
  justify-content: center;
  position: relative;
}

.infoLabel {
  font-family: Tahoma, Verdana, Arial;
  font-size: 0.9em;
  font-weight: 600;
}

.infoState {
  font-family: Courier New, Courier, sans-serif;
  font-size: 1em;
}

.equipment {
  background-colour: black;
  background-image:url('/equipment.svg');
  background-position: center;
  background-repeat: no-repeat;
  background-size: 80%;
  border: 1px solid green;
  border-radius: 12px;
  box-sizing: border-box;
  box-shadow: 0 2px 18px -4px rgba(0, 0, 0, 0.75);
  colour: rgba(0, 160, 0, 1);
  height: 500px;
  margin: 20px auto;
  max-width: 360px;
  min-height: 200px;
  position: relative;
  text-align: left;
  text-shadow: #060 1px 0 5px;
  width: 360px;
  z-index: 0;
}

.equipment::before {
  background: linear-gradient(rgba(18, 16, 16, 0) 40%, rgba(200, 200, 200, 0.25) 50%), linear-gradient(90deg, rgba(255, 0, 0, 0.06), rgba(0, 255, 0, 0.02), rgba(0, 0, 255, 0.06));
  background-size: 100% 2px, 3px 100%;
  border-radius: 12px;
  bottom: 0;
  content: "";
  display: block;
  left: 0;
  position: absolute;
  pointer-events: none;
  right: 0;
  top: 0;
  z-index: 1;
}

.equip-title {
  font-size: 1.2em;
  font-weight: bold;
  padding-top: 10px;
  position: relative;
  top: 15px;
  z-index: 3;
}

.power-title {
  display: flex;
  font-weight: bold;
  left: 310px;
  margin-right: 8px;
  top: 350px;
  z-index: 3;
}

.stream-title {
  display: flex;
  font-weight: bold;
  left: 194px;
  margin-right: 8px;
  top: 421px;
  z-index: 3;
}

.batt-title {
  font-size: 0.9em;
  font-weight: bold;
  left: 8px;
  top: 406px;
  z-index: 3;
}

.overlay {
  position: absolute;
}

.barrel-box {
  border-radius: 5px;
  left: 268px;
  height: 60px;
  top: 338px;
  width: 20px;
  z-index: 2;
}

.booster-box {
  border-radius: 2px;
  left: 92.5px;
  height: 96px;
  top: 79.5px;
  width: 20px;
  z-index: 2;
}

.cyc-circle {
  border-radius: 50%;
  left: 77.5px;
  height: 140px;
  top: 280px;
  width: 140px;
  z-index: 2;
}

.cyc-circle::before {
  background-colour: white;
  border-radius: 50%;
  bottom: -1.7%;
  content: "";
  height: 29%;
  left: 67.3%;
  position: absolute;
  width: 29%;
  transform: translateX(-50%);
}

.filter-circle {
  border-radius: 50%;
  left: 150.7px;
  height: 42px;
  top: 381.2px;
  width: 42px;
  z-index: 3;
}

.ion-switch {
  border-radius: 2px;
  left: 52px;
  height: 38px;
  top: 80px;
  width: 30px;
  z-index: 2;
}

.power-box {
  border-radius: 2px;
  left: 37px;
  height: 74px;
  top: 133px;
  width: 35px;
  z-index: 2;
}

.safety-box {
  border-radius: 5px;
  left: 305px;
  height: 32px;
  top: 156px;
  width: 16px;
  z-index: 2;
}

.cable-warn {
  colour: yellow;
  display: none;
  font-family: monospace;
  font-size: 6em;
  left: 55px;
  text-shadow: #880 1px 0 5px;
  top: 206px;
  z-index: 2;
}

.rad-warn {
  font-weight: bold;
  left: 40px;
  position: relative;
  top: 425px;
  z-index: 2;
}

button {
  background-colour: #555;
  border: 2px solid #333;
  border-radius: 8px;
  box-shadow: 0 8px 16px rgba(0, 0, 0, 0.2), 0 6px 20px rgba(0, 0, 0, 0.19);
  colour: white;
  margin: 5px;
  padding: 12px 8px;
  text-align: center;
  touch-action: manipulation;
  width: 100px;
  transition: background-colour 0.3s ease;
}

button:disabled,
button[disabled] {
  border: 1px solid #999 !important;
  background-colour: #ccc !important;
  colour: #555 !important;
}

button:active {
  box-shadow: 7px 6px 28px 1px rgba(0, 0, 0, 0.24);
  transform: translateY(4px);
}

input, select, textarea {
  border: 2px solid #333;
  border-radius: 4px;
  box-sizing: border-box;
  font-size: 18px;
  margin: 4px 0;
  padding: 6px 10px;
}

select {
  background-colour: #555;
  border-radius: 8px;
  box-shadow: 0 8px 16px rgba(0, 0, 0, 0.2), 0 6px 20px rgba(0, 0, 0, 0.19);
  colour: white;
  cursor: pointer;
  font-size: 1.15rem;
  height: 40px;
  margin: 5px;
  max-width: 320px;
  min-width: 160px;
  text-align: center;
  width: 160px;
}

.custom-select {
  position: relative;
}

.custom-select::before,
.custom-select::after {
  --size: 0.3rem;
  content: "";
  right: 1rem;
  pointer-events: none;
  position: absolute;
}

.custom-select::before {
  border-left: var(--size) solid transparent;
  border-right: var(--size) solid transparent;
  border-bottom: var(--size) solid black;
  top: 40%;
}

.custom-select::after {
  border-left: var(--size) solid transparent;
  border-right: var(--size) solid transparent;
  border-top: var(--size) solid black;
  top: 55%;
}

.red, .orange, .green, .blue {
  border: 2px solid;
}

.red { background-colour: #f44336; border-colour: #d42316; }
.orange { background-colour: #ffac1c; border-colour: #dd9a0a; }
.green { background-colour: #4CAF50; border-colour: #2C8F30; }
.blue { background-colour: #008CBA; border-colour: #006C9A; }

.setting {
  margin: 10px 0;
}

.labelSlider {
  position: relative;
  top: -10px;
}

.labelSwitch {
  display: inline-block;
  position: relative;
  text-align: right;
  top: 6px;
  width: 280px;
}

input[type=range] {
  width: 300px;
}

input[type=text] {
  width: 200px;
}

.swatch {
  border: 1px solid black;
  height: 40px;
  width: 60px;
}

.switch {
  display: inline-block;
  position: relative;
  width: 60px;
  height: 34px;
}

.switch input {
  opacity: 0;
  width: 0;
  height: 0;
}

.slider {
  background-colour: #ccc;
  bottom: 0;
  cursor: pointer;
  left: 0;
  position: absolute;
  right: 0;
  top: 0;
  transition: .4s;
}

.slider:before {
  background-colour: white;
  bottom: 4px;
  content: "";
  height: 26px;
  left: 4px;
  position: absolute;
  transition: .4s;
  width: 26px;
}

input:checked + .slider {
  background-colour: #2196F3;
}

input:focus + .slider {
  box-shadow: 0 0 1px #2196F3;
}

input:checked + .slider:before {
  transform: translateX(26px);
}

.slider.round {
  border-radius: 34px;
}

.slider.round:before {
  border-radius: 50%;
}

.bar-container, .volume-container, .music-navigation {
  display: flex;
  margin: 0 auto;
  max-width: 360px;
  width: 360px;
}

.bar-container {
  display: inline-flex;
  margin-left: 20px;
  position: relative;
  top: 3px;
  width: 120px;
}

.bar {
  background-colour: lightgray;
  border: 1px solid #222;
  height: 15px;
  width: 20px;
}

.volume-control {
  align-items: center;
  background-colour: #fff;
  border-radius: 5px;
  box-shadow: 0 2px 5px rgba(0, 0, 0, 0.2);
  display: flex;
  flex-direction: column;
  justify-content: center;
  margin: 20px auto;
  padding: 10px;
  width: 80px;
}

.volume-control h3 {
  margin: 2px auto;
}

.volume-control button {
  align-items: center;
  background-colour: #008CBA;
  border: 1px solid #006C9A;
  cursor: pointer;
  display: flex;
  font-size: 20px;
  height: 50px;
  justify-content: center;
  margin: 5px 0;
  transition: background-colour 0.3s ease;
  width: 50px;
}

.volume-control button:hover {
  background-colour: #009DCB;
}

.volume-control button:active {
  transform: scale(0.95);
}

.volume-label {
  font-weight: bold;
  margin-bottom: 5px;
}

.music-navigation {
  justify-content: space-between;
  margin: 10px auto;
  width: 320px;
}

.music-navigation button {
  align-items: center;
  background-colour: #008CBA;
  border: 1px solid #006C9A;
  border-radius: 0;
  cursor: pointer;
  display: flex;
  font-family: monospace;
  font-size: 1.8em;
  height: 40px;
  justify-content: center;
  margin: 0;
  transition: background-colour 0.3s ease;
  width: 85px;
}

.music-navigation button:first-child {
  border-radius: 5px 0 0 5px;
}

.music-navigation button:last-child {
  border-radius: 0 5px 5px 0;
}

.music-navigation button:hover {
  background-colour: #009DCB;
}

.music-navigation button:active {
  transform: scale(0.95);
}

#tracks {
  margin-top: 15px;
  width: 320px;
}

.tab {
  display: none;
}

.tabs {
  border-bottom: 1px solid #000;
  display: flex;
  margin: 0 auto;
  max-width: 400px;
}

.tabs button {
  align-items: center;
  border: 1px solid #333;
  border-radius: 0;
  cursor: pointer;
  flex: 1;
  font-family: monospace;
  font-size: 2.2em;
  height: 60px;
  position: relative;
  margin: 0;
  transition: background-colour 0.3s ease;
  width: 85px;
  -webkit-filter: grayscale(100%);
}

.tabs button:hover {
  background: #aaa;
}

.tabs button.active {
  background: #999;
}
)=====";
