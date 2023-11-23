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

const char STYLE_page[] PROGMEM = R"=====(
html {
  font-family: Tahoma, Verdana, Arial;
  display: inline-block;
  margin: 0px;
  text-align: center;
}

body {
  margin: 0px 0px 20px 0px;
}

h1 {
    background: #222;
    color: #eee;
    margin: 0px;
    padding: 8px;
    width: 100%;
}

h3 { color: #333; margin: 10px; }

a { text-decoration: none; }

p { font-size: 18px; margin-bottom: 5px; }

.block {
  margin: 10px 5px 30px 5px;
  padding: 10px 5px 10px 5px;
}

.card {
  background: #ddd;
  box-sizing: border-box;
  box-shadow: 0px 2px 18px -4px rgba(0, 0, 0, 0.75);
  color: #444;
  margin: 20px auto;
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
  padding: 12px 8px;
  text-align: center;
  touch-action: manipulation;
  width: 100px;
}

button:disabled,
button[disabled] {
  border: 1px solid #999999 !important;
  background-color: #cccccc !important;
  color: #555 !important;
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

select {
  background-color: #555;
  border: 2px solid #333;
  border-radius: 8px;
  box-shadow: 0 8px 16px 0 rgba(0,0,0,0.2), 0 6px 20px 0 rgba(0,0,0,0.19);
  color: white;
  cursor: pointer;
  font-size: 1.15rem;
  height: 50px;
  margin: 5px;
  padding: 6px 8px;
  max-width: 260px;
  min-width: 260px;
  text-align: center;
  touch-action: manipulation;
  width: 260px;
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

.red {
  background-color: #f44336;
  border: 2px solid #d42316;
}

.orange {
  background-color: #ffac1c;
  border: 2px solid #dd9a0a;
}

.green {
  background-color: #4CAF50;
  border: 2px solid #2C8F30;
}

.blue {
  background-color: #008CBA;
  border: 2px solid #006C9A;
}
)=====";