/**
 *   GPStar Stream Effects - Ghostbusters Props, Mods, and Kits.
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

const char STYLE_page[] PROGMEM = R"=====(
@font-face {
  font-family:'power_symbol';
  src:url(data:application/octet-stream;base64,d09GMgABAAAAAALMAA4AAAAABiQAAAJ0AAEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAP0ZGVE0cGiYGVgCCSggEEQgKghCCFAsKAAE2AiQDEAQgBYJiBzcbJAUgjpQuUybx8P92j7/PHUTQvADoZJRxFVdZafGhooRdCjlkQuTz3hqPGibwAbxLL9wBtFkvDDigjHoBHTcFeHICAXpAlLWLNteDkthZ1+dIebklAj5796UavNH67pfwdb8vPxBoiNY6S0LIhBwJgUDUqeKaQl4MK24RnePjsvdsTs1JinoAoCvA3M0zNprc4+8ef5eb/9vsubqUCECGDB0FmC4T6CjXF6VcSPT4+///ob85H9cVAFESgEhIAIAQ9z69aIb4/38J5KAucszBNkCOwRJIed58SIMTTY6tb1Sv4YkLb1M6+87F9yLqxLl362+sfz0uPN/27bfhTrIuv+cGvfj2i/UvgPxBq3eCI+UX3n0XF0tuPPJ6umJR16zRR9NuvnJz8JjDVdOukqimv56ZHVJtmtr980GbPq8vWLny5/MXd/777LP/7Wr9385nn7PJxfOoIxGv92nTaOtai86vHDj34MTR11878tvrR4+8duLBgXOvdG5RTQUeD0PGP1En98vQx8viY82y5Cia0cKR8zL+YVmkVwsEex47FGuajP+1boO6FsPa6ZSzyt6oK4Ggrv//8lxdVJgpAAB5ncp0qAv2M/wgkJsigYQQmGwwoLGHAkASIBSaCJDkugiQ6Wi1ADlOCVBo4LoApcJDmcjrAYRQX4akrlbIjNYTOU6g0NrpTqX67sFeexyxyX62GsFU0OyxgyeHeWbaY7eDxSH7bbFJV0MNVulqrK4uXTPGIMOdPVQlR/Bqk1Ts37Kp69DBVdexXY2pjRk0fNDBngGLR1YHbBtEdVXrOHS9Kq0NrmZYJG0ANTlCitTw61SCP1wGAAA=)format('woff2');
  unicode-range:U+23FB;
}

html {
  display:inline-block;
  font-family:Tahoma,Verdana,Arial;
  margin:0;
  overflow-x:hidden;
  text-align:center;
  width:100%;
}

body {
  font-size:100%;
  margin:0 0 20px;
  position:relative;
}

h1 {
  background:#222;
  color:#eee;
  margin:0;
  padding:8px;
  width:100%;
}

h3 {
  color:#333;
  margin:10px;
}

a {
  text-decoration:none;
}

p {
  font-size:16px;
  margin:4px 0 6px;
}

.dark {
  background-color:#444;
}

.blinking {
  animation:blink-animation .5s steps(5,start) infinite;
}

@keyframes blink-animation {
  to {
    visibility:hidden;
  }
}

.footer {
  color:#ccc;
  font-size:.8em;
}

.left {
  text-align:left;
}

.block {
  margin:10px 5px 30px;
  padding:10px 5px;
  width:99%;
}

.card {
  background:#ddd;
  border-radius:12px;
  box-sizing:border-box;
  box-shadow:0 2px 18px -4px rgba(0,0,0,0.75);
  color:#444;
  margin:20px auto;
  max-width:360px;
  min-height:200px;
  padding:10px 15px;
  text-align:left;
  width:360px;
}

.centered {
  align-items:center;
  display:flex;
  justify-content:center;
  position:relative;
}

.infoLabel {
  font-family:Tahoma,Verdana,Arial;
  font-size:.9em;
  font-weight:600;
  min-width:40px;
}

.infoState {
  font-family:Courier New,Courier,sans-serif;
  font-size:0.9em;
}

button {
  background-color:#555;
  border:2px solid #333;
  border-radius:8px;
  box-shadow:0 8px 16px rgba(0,0,0,0.2),0 6px 20px rgba(0,0,0,0.19);
  color:#fff;
  margin:5px;
  padding:12px 8px;
  text-align:center;
  touch-action:manipulation;
  width:100px;
  transition:background-color .3s ease;
}

button:disabled,
button[disabled] {
  border:1px solid #999!important;
  background-color:#ccc!important;
  color:#555!important;
}

button:active {
  box-shadow:7px 6px 28px 1px rgba(0,0,0,0.24);
  transform:translateY(4px);
}

input,
select,
textarea {
  border:2px solid #333;
  border-radius:4px;
  box-sizing:border-box;
  font-size:18px;
  margin:4px 0;
  padding:6px 10px;
}

select {
  background-color:#555;
  border-radius:8px;
  box-shadow:0 8px 16px rgba(0,0,0,0.2),0 6px 20px rgba(0,0,0,0.19);
  color:#fff;
  cursor:pointer;
  font-size:1.15rem;
  height:40px;
  margin:5px;
  max-width:320px;
  min-width:160px;
  text-align:center;
  padding:0px 0px;
  width:160px;
}

.custom-select {
  position:relative;
}

.custom-select::before,
.custom-select::after {
  --size:.3rem;
  content:"";
  right:1rem;
  pointer-events:none;
  position:absolute;
}

.custom-select::before {
  border-left:var(--size) solid transparent;
  border-right:var(--size) solid transparent;
  border-bottom:var(--size) solid #000;
  top:40%;
}

.custom-select::after {
  border-left:var(--size) solid transparent;
  border-right:var(--size) solid transparent;
  border-top:var(--size) solid #000;
  top:55%;
}

.red,
.orange,
.green,
.blue {
  border:2px solid;
}

.red {
  background-color:#f44336;
  border-color:#d42316;
}

.orange {
  background-color:#ffac1c;
  border-color:#dd9a0a;
}

.green {
  background-color:#4CAF50;
  border-color:#2C8F30;
}

.blue {
  background-color:#008CBA;
  border-color:#006C9A;
}

.setting {
  margin:10px 0;
}

.labelSlider {
  position:relative;
  top:-10px;
}

.labelSwitch {
  display:inline-block;
  position:relative;
  text-align:right;
  top:6px;
  width:280px;
}

input[type=range] {
  width:300px;
}

input[type=text] {
  width:200px;
}

.swatch {
  border:1px solid #000;
  height:40px;
  width:60px;
}

.bar-container {
  display:flex;
  margin:0 auto;
  max-width:360px;
  width:360px;
}

.bar-container {
  display:inline-flex;
  margin-left:20px;
  position:relative;
  top:3px;
  width:120px;
}

.bar {
  background-color:lightgray;
  border:1px solid #222;
  height:15px;
  width:20px;
}

.tab {
  display:none;
}

.tabs {
  border-bottom:1px solid #000;
  display:flex;
  margin:0 auto;
  max-width:400px;
}

.tabs button {
  align-items:center;
  border:1px solid #333;
  border-radius:0;
  cursor:pointer;
  flex:1;
  font-family:monospace,power_symbol;
  font-size:2.2em;
  padding:0px;
  line-height:60px;
  height:60px;
  position:relative;
  margin:0;
  transition:background-color .3s ease;
  width:85px;
  -webkit-filter:grayscale(100%);
  filter:grayscale(100%);
}

.tabs button:hover {
  background:#aaa;
}

.tabs button.active {
  background:#999;
}

/* https://github.com/adamculpepper/toggle-switchy */
.toggle-switchy {color:#fff;}
.toggle-switchy > input + .toggle:before {content:'ON';}
.toggle-switchy > input + .toggle:after {content:'OFF';}
.toggle-switchy > input + .toggle > .switch {background:#fff;}
.toggle-switchy > input + .toggle + .label {color:#000;}
.toggle-switchy > input:checked + .toggle {background:#3498db;}
.toggle-switchy > input:not(:checked) + .toggle {background:#ccc;}
.toggle-switchy > input:checked + .toggle > .switch {border:3px solid #3498db;}
.toggle-switchy > input:not(:checked) + .toggle > .switch {border:3px solid #ccc;}
.toggle-switchy > input + .toggle {border-radius:4px;}
.toggle-switchy > input + .toggle .switch {border-radius:6px;}
.toggle-switchy {display:inline-flex; align-items:center; user-select:none; position:relative; vertical-align:middle; margin-bottom:0;}
.toggle-switchy:hover {cursor:pointer;}
.toggle-switchy > input {position:absolute; opacity:0;}
.toggle-switchy > input + .toggle {align-items:center; position:relative;}
.toggle-switchy > input + .toggle {overflow:hidden; position:relative; flex-shrink:0;}
.toggle-switchy > input[disabled] + .toggle {opacity:0.5;}
.toggle-switchy > input[disabled] + .toggle:hover {cursor:not-allowed;}
.toggle-switchy > input + .toggle {width:100%; height:100%; margin:0; cursor:pointer;}
.toggle-switchy > input + .toggle > .switch {display:block; height:100%; position:absolute; right:0; z-index:3; box-sizing:border-box;}
.toggle-switchy > input + .toggle:before,
.toggle-switchy > input + .toggle:after {display:flex; align-items:center; position:absolute; z-index:2; height:100%;}
.toggle-switchy > input + .toggle + .label {margin-left:10px;}
.toggle-switchy[data-label='left'] > input + .toggle {order:2;}
.toggle-switchy[data-label='left'] > input + .toggle + .label {order:1; margin-left:0; margin-right:10px; font-weight:bold}
.toggle-switchy > input + .toggle:before {opacity:0;}
.toggle-switchy > input:checked + .toggle:before {opacity:1;}
.toggle-switchy > input:checked + .toggle:after {opacity:0;}
.toggle-switchy > input + .toggle {transition:background 200ms linear, box-shadow 200ms linear;}
.toggle-switchy > input + .toggle:before,
.toggle-switchy > input + .toggle:after {transition:all 200ms linear;}
.toggle-switchy > input + .toggle > .switch {transition:right 200ms linear, border-color 200ms linear;}
.toggle-switchy > input + .toggle {width:65px; height:30px;}
.toggle-switchy > input + .toggle > .switch {width:30px;}
.toggle-switchy > input + .toggle:before,
.toggle-switchy > input + .toggle:after {font-size:0.8rem;}
.toggle-switchy > input:not(:checked) + .toggle > .switch {right:calc(100% - 30px);}
.toggle-switchy > input + .toggle,
.toggle-switchy > input + .toggle > .switch {border-radius:50px;}
.toggle-switchy > input + .toggle:before {right:50%;}
.toggle-switchy > input + .toggle:after {left:50%;}
.toggle-switchy[data-text='repeat'] > input + .toggle:before {content:'ONE';}
.toggle-switchy[data-text='repeat'] > input + .toggle:after {content:'ALL';}
.toggle-switchy[data-text='yesno'] > input + .toggle:before {content:'YES';}
.toggle-switchy[data-text='yesno'] > input + .toggle:after {content:'NO';}
)=====";
