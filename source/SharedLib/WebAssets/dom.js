/** DOM Manipulation Utilities - Device-agnostic helpers for element access and manipulation **/

function getEl(id) {
  return document.getElementById(id);
}

function getInt(id) {
  return parseInt(getValue(id) ?? 0, 10);
}

function getFloat(id) {
  return parseFloat(getValue(id) ?? 0);
}

function getText(id) {
  return (getValue(id) || "").trim();
}

function getHtml(id) {
  return (getEl(id).innerHTML);
}

function getToggle(id) {
  var el = getEl(id);
  return el && el.checked ? true : false;
}

function getValue(id) {
  var el = getEl(id);
  return el && typeof el.value !== "undefined" ? el.value : null;
}

function setHtml(id, value) {
  getEl(id).innerHTML = value || "";
}

function setToggle(id, value) {
  var el = getEl(id);
  if (el) {
    if (el._lockout != true) el.checked = value === true;
  }
}

function setValue(id, value) {
  var el = getEl(id);
  if (el) el.value = value;
}

function hideEl(id) {
  getEl(id).style.display = "none";
}

function showEl(id) {
  getEl(id).style.display = "block";
}

function disableEl(id) {
  getEl(id).disabled = true;
}

function enableEl(id) {
  getEl(id).disabled = false;
}

function colorEl(id, red, green, blue, alpha = 0.5) {
  getEl(id).style.backgroundColor = "rgba(" + red + ", " + green + ", " + blue + ", " + alpha + ")";
}

function blinkEl(id, state) {
  if (state) {
    getEl(id).classList.add("blinking");
  } else {
    getEl(id).classList.remove("blinking");
  }
}

function disableControls() {
  // Disables all form controls (inputs, selects, buttons)
  var controls = document.querySelectorAll('input, select, button');
  for (var i = 0; i < controls.length; i++) {
    controls[i].disabled = true;
  }
}
